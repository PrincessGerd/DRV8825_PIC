#include "motion_planer.h"
#include "axis_stepper.h"
// core
#include "../core/types.h"
#include "../core/task_manager.h"
#include "../core/system.h"
#include "../core/interrupts.h"
#include "../core/dma_descriptor.h"
#include "../core/gpio.h"
//
#include "../inc/math.h"
#include "../inc/cordic.h"
#include "../inc/ringbuffer.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <xc.h>

#define MOVEMENT_BUFER_SIZE 32
#define CMD_BUFFER_LEN 16
//__attribute__((packed, aligned(9)))

struct segment{
    int16_t dx, dy, dz;
    int32_t length;

    int16_t v0;
    int16_t v1;
    int16_t accel;
    int16_t exit_vel;
    int16_t accel_steps;
    int16_t decel_steps;
};

struct motion_planer{
    task_t super;
    event_t evt;
    uint16_t target_speed
    uint16_t initial_period;
    uint16_t max_period;

    struct segment* curr;
    struct segment* next;
    struct segment* prev;
};

bool is_junction(struct segment* curr, struct segment* next, int16_t threshold){
    if(next == 0) return false;
    int32_t len = (curr->length * next->length);    // TODO: make this propper q15
    int16_t inv_len = (FP_ONE /  len) >> Q15_BITS;
    int16_t dot = ((next->dx * curr->dx) + (next->dy * curr->dy) + (next->dz * curr->dz));
    cos_theta = mul_i32_q15(dot, len);
    return (cos_theta < threshold) ? true : false;
}

void filter_pass_forward(struct motion_planer* mp){
    struct segment* curr = mp->curr;
    struct segment* next = mp->next;
    // decelerate to nexts v1 or this v0 if the junction is sharper than the threshold
    int16_t end_vel = (next && !is_junction(curr,next, (Q15_ONE >> 2))) ? next->v1 : curr->v0;
    if(curr->v1 <= end_vel){
        curr->decel_steps = 0;
    } else {
        curr->decel_steps = (curr->v1*curr->v1 - end_vel*end_vel) / (curr->accel << 1);
        if(curr->decel_steps > (curr->length >> 1))
            curr->decel_steps = (curr->length >> 1);
    }
}

void filter_pass_backward(struct motion_planer* mp){
    struct segment* curr = mp->curr;
    struct segment* prev = mp->prev;
    int16_t start_vel = prev ? prev->exit_vel : curr->v1;
    if(start_vel >= curr->v1){
        curr->accel_steps = 0;
    } else {
        // standard equations of motion (V - V0 = 2as)
        curr->accel_steps = (curr->v1*curr->v1 - start_vel*start_vel) / (curr->accel << 1);
        if(curr->accel_steps > (curr->length >> 1))
            curr->accel_steps = (curr->length >> 1);
    }
}

void compensate_triangle(struct motion_planer* mp){
    struct segment* curr = mp->curr;
    int16_t plateau = curr->length - curr->accel_steps - curr->decel_steps;
    if(plateau < 0){
        int32_t total_len = curr->accel_steps + curr->decel_steps;
        curr->accel_steps = (curr->accel_steps * curr->length) / total_len;
        curr->decel_steps = (curr->length - curr->accel_steps);
    }
}

// only check the prev and next segments. inspired by the kernel filter in 1D signal processing. 
//  "sliding" the 1x3 kernel across the signal
void apply_filter(struct motion_planer* mp){
    if(mp == 0) return;
    // forward pass to make sure the decelleration of current lines up with the acceleration of next
    filter_pass_forward(mp);
    // backward pass to make sure the acceleration of current lines up with the decelleration of prev
    filter_pass_backward(mp);
    // compensates for accel and decel steps being the whole of the segment lenght. 
        // maybe not needed, but i assume acceleration straight into deceleration will cause jitter
    compensate_triangle(mp);
}

// -2s^3 + 3s^2
/*
static inline fp15_t smoothstep_cubic(fp15_t s) {
    fp15_t s2 = fp15_mul(s,s);
    fp15_t c1 = 3 * Q15_ONE;
    fp15_t c2 = fp15_mul((2u *Q15_ONE), s);
    return fp15_mul(s2, (c1 - c2));
}

static uint16_t next_period_accel(struct motion_planer* self) {
    fp15_t s = ((uint32_t)self->step_count << Q15_BITS) / self->accel_steps;
    if (s > Q15_ONE) s = Q15_ONE;
    fp15_t scale = smoothstep_cubic(s);
    uint32_t delta_f = self->initial_period - self->max_period;
    uint32_t period  = self->initial_period - ((delta_f * scale) >> 16);
    return (uint16_t)period;
}

static uint16_t next_period_const(struct motion_planer* self){
    return (uint16_t)self->max_period;
}

static uint16_t next_period_deccel(struct motion_planer* self){
    fp15_t s = ((uint32_t)self->steps_remaining << Q15_BITS) / self->accel_steps;
    if (s > Q15_ONE) s = Q15_ONE;
    fp15_t scale = smoothstep_cubic(s);
    uint32_t delta_f = self->initial_period - self->max_period;
    uint32_t period  = self->initial_period - ((delta_f * scale) >> 16);
    return (uint16_t)period;
}*/



void stepper_create(task_t** self){
    static struct motion_planer mp_inst = {0};
    task_create(&mp_inst.super,
        (event_handler_t)&motion_planer_init,
        (event_handler_t)&motion_planer_dispatch);

    *self = &mp_inst.super;
}

static void motion_planer_init(task_t* const super, event_t const* const ie){
    struct motion_planer* self = container_of(super, struct motion_planer, super); 
    struct stepper_initEvt* initial_event = (struct stepper_initEvt*)ie;
    axis_stepper_init(self->axes, self->axis_count, &LATC, port.mask);  
    move_queue_init();  
}

static void motion_planer_dispatch(task_t* const super, event_t* const e){
    struct motion_planer* self = container_of(super, struct motion_planer, super); 
    switch (e->signal) {
        ////////////////////////////////////////////////////////
        case EV_WORK_SIG:{
            struct motion_planer_workEvt* event = (struct motion_planer_workEvt*)e;
            // push the old prev out to motion controller

            task_event_post(self->super, self->evt);
            // shift everything left
            self->prev = self->curr;
            self->curr = self->next;
            // push the new segment into the 3x1 kernel
            self->next->dx = event->dx;
            self->next->dy = event->dy;
            self->next->dz = event->dz;
            self->next->length = isqrt32(
                (event->dx*event->dx) + 
                (event->dy*event->dy) + 
                (event->dz*event->dz));
        } break;
        case EV_IDLE_SIG: {
            // do nothing. just defined for tracing
        }break;
    default:
        break;
    }
    return;
}