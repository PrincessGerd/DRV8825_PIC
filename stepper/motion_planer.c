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
#include <xc.h>
#include <string.h>

#define MOVEMENT_BUFER_SIZE 32
#define CMD_BUFFER_LEN 16
//__attribute__((packed, aligned(9)))

struct move_state{
    int32_t e1[2], e2[2];       // basis vectors
    int32_t c[2];               // center point
    int32_t steps_remaining;    //
    int32_t x;
    int32_t y; 
    int16_t dcos;               //
    int16_t dsin;               //
    bool clockwise;             // rotation direction
};

struct motion_planer{
    task_t super;
    event_t evt;
    uint32_t tick_frequency;
    struct axis_stepper* axes;
    uint8_t axis_count;
    uint8_t active;
    int16_t last_pos[2];
    uint8_t maskx;
    uint8_t masky;
    struct move_cmd* curr_move;
    struct move_state ms;
};

RING_BUFFER_DECLARE(move_queue,MOVEMENT_BUFER_SIZE,sizeof(struct move_cmd))
static void stepper_init(task_t* const super, event_t const* const ie);
static void stepper_dispatch(task_t* const super, event_t* const e);

int gcode_line(
    int16_t dx, int16_t dy,
    uint8_t maskx,
    uint8_t masky,
    uint8_t* out_buffer, int16_t offset)
{
    int16_t sx = (dx >= 0) ? 1 : -1;
    int16_t sy = (dy >= 0) ? 1 : -1;

    dx = (dx >= 0) ? dx : -dx;
    dy = (dy >= 0) ? dy : -dy;

    int16_t countx = 0;
    int16_t county = 0;

    int16_t err = dx - dy;
    int16_t i = offset;

    while (i < AXIS_STEPPER_BUFFER_SIZE) {
        int16_t e2 = (err << 1);
        uint8_t point = 0;

        if (countx == dx && county == dy)
            break;

        if (e2 > -dy) {
            err -= dy;
            countx++;
            point |= maskx;
        }

        if (e2 < dx) {
            err += dx;
            county++;
            point |= masky;
        }
        out_buffer[i] = point;
        i++;
        out_buffer[i] &= ~point;
        i++;
    }
    return (i - offset);
}

#define MAX_DTHETA_Q15 (1 << 10)
void arc_move_init(
    int16_t u[2],
    int16_t X, int16_t Y,
    int16_t I, int16_t J,
    bool clockwise,
    struct move_state* out_state) {
        X = 200;
        Y = 0;
        I = 0;
        J = -200;
        int32_t v[2] = {X, Y};
        int32_t c[2] = {u[0]+I, u[1]+J};

        int32_t umc[2] = {(u[0] - c[0]), (u[1] - c[1])};
        int32_t vmc[2] = {(v[0] - c[0]), (v[1] - c[1])};
    
        int16_t r;
        cordic_hypot(umc[0], umc[1], &r);
        r = r < 0 ? -r : r;

        int32_t sin_theta = (int16_t)((int32_t)umc[0]*vmc[1] - (int32_t)umc[1]*vmc[0]); // cross
        int32_t cos_theta = (int16_t)((int32_t)umc[0]*vmc[0] + (int32_t)umc[1]*vmc[1]); // dot
        int32_t theta; // total angle between start and end positions
        cordic_atan2(sin_theta, cos_theta, &theta);
    
        if (clockwise)
            theta = -theta;
            
        int32_t steps =
            (((theta > 0) ? theta : -theta) + (MAX_DTHETA_Q15 << 1) - 1) / MAX_DTHETA_Q15;
        steps = max(1, steps);
        // basis vectors
        // second bais is perpendicular to e1
        int16_t su[2] = {fp15_div(umc[0],r), fp15_div(umc[1],r)};
        int16_t e1[2] = {su[0], su[1]};
        int16_t e2[2] = {-e1[1], e1[0]};
        if (clockwise) {
            e2[0] =  e1[1];
            e2[1] = -e1[0];
        }
        int16_t dtheta = (theta / steps);
        fp15_t dcos=0, dsin=0;
        cordic_sincos(dtheta,&dcos,&dsin);
        if (clockwise)
            dsin = -dsin;
            
        out_state->e1[0] = e1[0];
        out_state->e2[0] = e2[0];
        out_state->e1[1] = e1[1];
        out_state->e2[1] = e2[1];
        out_state->steps_remaining = steps;
        out_state->dcos = dcos;
        out_state->dsin = dsin;
        out_state->x = ((int32_t)r) << Q15_BITS;
        out_state->y = 0;
        out_state->c[0] = c[0];
        out_state->c[1] = c[1];
    }

void arc_move(int16_t* u[2], struct move_state* ms, uint8_t maskx, uint8_t masky, uint8_t* out_buffer){
    int N = (AXIS_STEPPER_BUFFER_SIZE < ms->steps_remaining) ? 
        AXIS_STEPPER_BUFFER_SIZE : ms->steps_remaining;
    int16_t offset = 0;
    int16_t dx = 0, dy = 0;
    for(int16_t i = 0; i < N; i++){
        int32_t x_new = mul_i32_q15(ms->x,ms->dcos) - mul_i32_q15(ms->y,ms->dsin);
        int32_t y_new = mul_i32_q15(ms->x,ms->dsin) + mul_i32_q15(ms->y,ms->dcos);
        dx = (int16_t)((ms->c[0] + mul_i32_q15((x_new - ms->x), ms->e1[0]) + mul_i32_q15((y_new - ms->y),ms->e2[0])) >> Q15_BITS);
        dy = (int16_t)((ms->c[1] + mul_i32_q15((x_new - ms->x), ms->e1[1]) + mul_i32_q15((y_new - ms->y),ms->e2[1])) >> Q15_BITS);
        ms->x = x_new;
        ms->y = y_new;
        offset +=  gcode_line(dx,dy, maskx, masky, out_buffer, offset);
        ms->steps_remaining--;
    }
}

void stepper_create(task_t** self){
    static struct motion_planer mp_inst = {0};
    axis_stepper_instance(&mp_inst.axes, &mp_inst.super,0);
    task_create(&mp_inst.super,
        (event_handler_t)&stepper_init,
        (event_handler_t)&stepper_dispatch);
    static struct move_cmd tmp_mc = {0};
    mp_inst.curr_move = &tmp_mc;
    *self = &mp_inst.super;
}

static void stepper_init(task_t* const super, event_t const* const ie){
    struct motion_planer* self = container_of(super, struct motion_planer, super); 
    struct stepper_initEvt* initial_event = (struct stepper_initEvt*)ie;
    self->axis_count = 2;
    self->tick_frequency = 64000000/32; // ie->tick_frequency;
    self->active = 0;
    self->last_pos[0] = 0;
    self->last_pos[1] = 0;
    uint8_t port_mask = 0b01111011;
    self->maskx = (1 << 4); // RC4
    self->masky = (1 << 3); // RC5
    axis_stepper_init(self->axes, self->axis_count, &LATC, port_mask);  
    move_queue_init();  
}

static void stepper_dispatch(task_t* const super, event_t* const e){
    struct motion_planer* self = container_of(super, struct motion_planer, super); 
    switch (e->signal) {
        ////////////////////////////////////////////////////////
        case EV_WORK_SIG:{
            struct stepper_workEvt* event = (struct stepper_workEvt*)e;
            int16_t dx = event->X - self->last_pos[0];   // steps in y axis
            int16_t dy = event->Y - self->last_pos[1];   // steps in x axis
            int16_t dominant = (dx < dy) ? dy : dx;
            struct move_cmd next_move = {
                .mode = G_ARC_CW,
                .X = 200,
                .Y = 0,
                .I = 0,
                .J = -200
            };
            memcpy(&next_move,PTR_ADD(&event->super, sizeof(event_t)),sizeof(event_t));
            ring_buffer_enqueue(move_queue, &next_move);
            if(self->active == 0){
                self->curr_move = &next_move;
                self->evt.signal = EV_DONE_SIG;   
                task_event_post(super,&self->evt);
            }  

        } break;
        ////////////////////////////////////////////////////////
        case EV_BUFFER_FILL_SIG:{
            int8_t* fill;
            axis_stepper_get_fill_buffer(self->axes, &fill);
            switch (self->curr_move->mode) {
                case G_ARC_CW: // G_ARC_CCW || 
                    //TRISCbits.TRISC7 = 0;
                    arc_move(
                        self->last_pos,&self->ms, self->maskx, self->masky, fill);
                    break;
                case G_LINE:
                    int16_t dx = self->curr_move->X - self->last_pos[0];   // steps in y axis
                    int16_t dy = self->curr_move->Y - self->last_pos[1];   // steps in x axis
                    gcode_line(
                        dx, dy, self->maskx, self->masky, fill, 0);
                    break;         
                case G_DWELL:
                    break; // not sure how to handle this yet. dont want to waste compute time
                default:
                    break;
            }
        } break;
        ////////////////////////////////////////////////////////
        case EV_DONE_SIG:{
            struct move_cmd next_move;
            ring_buffer_dequeue(move_queue, (uint8_t*)(&next_move));  
            if(&next_move == 0) return;
            self->curr_move = &next_move;
            uint8_t* fill;
            axis_stepper_get_fill_buffer(self->axes, &fill);
            int16_t dx = self->curr_move->X - self->last_pos[0];   // steps in y axis
            int16_t dy = self->curr_move->Y - self->last_pos[1];   // steps in x axis
            switch (self->curr_move->mode) {
                case G_ARC_CW:
                    arc_move_init(self->last_pos, 
                        self->curr_move->X, self->curr_move->Y,
                        self->curr_move->I, self->curr_move->J,
                        self->curr_move->mode, &self->ms);
                    arc_move(
                        self->last_pos,&self->ms, self->maskx, self->masky, fill);
                    break;
                case G_LINE:
                    gcode_line(
                        dx, dy, self->maskx, self->masky, fill, 0);
                    break;         
                case G_DWELL:
                    break; // not sure how to handle this yet. dont want to waste compute time
                default:
                    break;
            }
            int16_t dominant = (dx < dy) ? dy : dx;
            axis_stepper_start_move(self->axes,dominant);
            self->evt.signal = EV_BUFFER_FILL_SIG;   
            task_event_post(super,&self->evt);
        }break;
        ////////////////////////////////////////////////////////
        case EV_IDLE_SIG: {
            // do nothing. just defined for tracing
        }break;
    default:
        break;
    }
    return;
}