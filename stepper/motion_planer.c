#include "motion_planer.h"
#include "stepper_device.h"
#include "axis_stepper.h"
// core
#include "../core/types.h"
#include "../core/task_manager.h"
#include "../core/system.h"
#include "../core/interrupts.h"
#include "../core/dma_descriptor.h"
//
#include "../inc/math.h"
#include <stdint.h>
#include <stdbool.h>
#include <xc.h>

typedef enum{
    ACCELERATE = 0,
    CONSTANT_SPEED,
    DEACCELERATE,
    IDLE
}motion_states_e;

#define BUFFER_SIZE 32
#define MAX_ACCEL_STEPS 0x5FF

struct motion_profile {
    uint32_t total_steps;
    uint32_t accel_steps;
    uint32_t step_count;

    uint16_t min_period;
    uint16_t max_period;
    uint16_t delta;
    motion_states_e motion_state;
};

struct motion_planer{
    task_t super;
    event_t evt;
    struct motion_profile profile;
    struct axis_stepper_t* axis_x;
    struct axis_stepper_t* axis_y;
    fp15_t nx;
    fp15_t ny;
    uint32_t tick_frequency;
};


static void stepper_init(task_t* const super, event_t const* const ie);
static void stepper_dispatch(task_t* const super, event_t* const e);

void stepper_create(task_t** self){
    static struct motion_planer mp_inst;
    axis_stepper_instance(&mp_inst.axis_x, &mp_inst.super,0);
    axis_stepper_instance(&mp_inst.axis_y, &mp_inst.super,1);
    task_create(&mp_inst.super,
        (event_handler_t)&stepper_init,
        (event_handler_t)&stepper_dispatch);
    *self = &mp_inst.super;
}

static void stepper_init(task_t* const super, event_t const* const ie){
    struct motion_planer* self = container_of(super, struct motion_planer, super); 
    struct stepper_initEvt* initial_event = (struct stepper_initEvt*)ie;
    axis_stepper_init(self->axis_x);    
    axis_stepper_init(self->axis_y); 
    self->tick_frequency = 64000000/32; // ie->tick_frequency;
}

// -2s^3 + 3s^2
static inline fp15_t smoothstep_cubic(fp15_t s) {
    fp15_t s2 = fp_mul(s,s);
    fp15_t c1 = 3 * Q15_ONE;
    fp15_t c2 = fp_mul((2u *Q15_ONE), s);
    return fp_mul(s2, (c1 - c2));
}

static uint16_t next_period_accel(struct motion_profile* mp) {
    fp15_t s = ((uint32_t)mp->step_count << Q15_BITS) / mp->accel_steps;
    if (s > Q15_ONE) s = Q15_ONE;
    fp15_t scale = smoothstep_cubic(s);
    uint32_t period  = mp->max_period - ((mp->delta * scale) >> Q15_BITS);
    return (uint16_t)period;
}

static uint16_t next_period_const(struct motion_profile* mp){
    return (uint16_t)mp->max_period;
}

static uint16_t next_period_deccel(struct motion_profile* mp){
    fp15_t s = ((uint32_t)(mp->total_steps - mp->step_count) << Q15_BITS) / mp->accel_steps;
    if (s > Q15_ONE) s = Q15_ONE;
    fp15_t scale = smoothstep_cubic(s);
    uint32_t period  = mp->max_period - ((mp->delta * scale) >> Q15_BITS);
    return (uint16_t)period;
}
typedef uint16_t(*next_period_func_t)(struct motion_profile* mp);
static void refill_stepper(struct motion_planer* self, next_period_func_t next_period){
    uint16_t* bx = axis_stepper_get_fill_buffer(self->axis_x);
    uint16_t* by = axis_stepper_get_fill_buffer(self->axis_y);
    for(int i = 0; i < BUFFER_SIZE; i++) {
        uint16_t p = next_period(&self->profile);
        bx[i] = (p * self->nx) >> Q15_BITS;
        by[i] = (p * self->ny) >> Q15_BITS;
        self->profile.step_count++;
    }
}

static void stepper_dispatch(task_t* const super, event_t* const e){
    struct motion_planer* self = container_of(super, struct motion_planer, super); 
    switch (e->signal) {
        ////////////////////////////////////////////////////////
        case STEPPER_WORK_SIG:{
            struct stepper_workEvt* event = (struct stepper_workEvt*)e;
            uint32_t dx = event->steps_x;   // steps in y axis
            uint32_t dy = event->steps_y;   // steps in x axis
            uint32_t mag = 15000; //isqrt32(dx*dx + dy*dy);
            self->nx = fp_div((dx << Q15_BITS), mag); // calculate normal for x 
            self->ny = fp_div((dy << Q15_BITS), mag); // calculate normal for y

            self->profile.step_count     = 0;
            self->profile.total_steps    = mag;
            self->profile.accel_steps    = (self->profile.total_steps >> 2) > 
                                            MAX_ACCEL_STEPS ? MAX_ACCEL_STEPS : (self->profile.total_steps >> 2); // steps required for acceleration
            self->profile.min_period     = (uint16_t)(self->tick_frequency / event->desired_speed);        // desired speed (steps/tick)    
            self->profile.max_period     = (uint16_t)(self->tick_frequency / 200);                  // start speed   (steps/tick) 
            self->profile.motion_state   = ACCELERATE;
            self->profile.delta = self->profile.max_period - self->profile.min_period;
            //refill_stepper(self,&next_period_accel);            
            axis_stepper_start_move(self->axis_x,dx);
            axis_stepper_start_move(self->axis_y,dy);
            self->evt.signal = STEPPER_UPDATE_SIG;   
            task_event_post(super,&self->evt);
        } break;
        ////////////////////////////////////////////////////////
        case STEPPER_UPDATE_SIG:{
            switch (self->profile.motion_state) {
                case ACCELERATE:{
                    if(self->profile.step_count < self->profile.accel_steps){
                        refill_stepper(self, &next_period_accel);
                    }else{
                        self->profile.motion_state = CONSTANT_SPEED;
                        self->evt.signal = STEPPER_UPDATE_SIG;   
                        task_event_post(super,&self->evt);
                    }
                }break;
                case CONSTANT_SPEED:{
                    if((self->profile.total_steps - self->profile.step_count) > self->profile.accel_steps){
                        refill_stepper(self, &next_period_const);
                    }else{
                        self->profile.motion_state = DEACCELERATE;
                        self->evt.signal = STEPPER_UPDATE_SIG;   
                        task_event_post(super,& self->evt);
                    }
                }break;
                case DEACCELERATE:{
                    if((self->profile.total_steps - self->profile.step_count) > 0){
                        refill_stepper(self, &next_period_deccel);
                    } else{
                        self->profile.motion_state = IDLE;
                        self->evt.signal = STEPPER_DONE_SIG;   
                    }
                    task_event_post(super,&self->evt);
                }break;
                default:    // IDLE case
                    return;
            }
        } break;
        ////////////////////////////////////////////////////////
        case STEPPER_DONE_SIG:{
            TRISCbits.TRISC7 = 1;
        }break;

    default: // IDLE, do nothing
        break;
    }
    return;
}