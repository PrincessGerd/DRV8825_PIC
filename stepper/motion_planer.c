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
#include "../inc/cordic.h"
#include "../inc/ringbuffer.h"
#include <stdint.h>
#include <stdbool.h>
#include <xc.h>

#define MOVEMENT_BUFER_SIZE 32
#define CMD_BUFFER_LEN 16
//__attribute__((packed, aligned(9)))
struct move_cmd {
    gcode_cmd_t mode;
    int16_t X, Y;
    int16_t I, J;
};

struct move_state{
    int32_t e1[2], e2[2];   // basis vectors
    int32_t c[2];           // center point
    int32_t steps;          // remaining steps 
    int32_t theta;          // total angle between points
    int16_t radius;         // 
};

struct motion_planer{
    task_t super;
    event_t evt;
    uint32_t tick_frequency;
    struct axis_stepper* axes;
    uint8_t axis_count;
    uint8_t active;
    int16_t last_pos[2];
    struct move_cmd* curr_move;
    struct move_state* ms;
};

RING_BUFFER_DECLARE(move_queue,MOVEMENT_BUFER_SIZE,sizeof(struct move_cmd))
static void stepper_init(task_t* const super, event_t const* const ie);
static void stepper_dispatch(task_t* const super, event_t* const e);

void gcode_line(
    int16_t U[2], 
    int16_t X, int16_t Y) {
        
    int16_t x0 = U[0];
    int16_t y0 = U[1];
    int16_t dx = (X > x0) ? (X - x0) : (x0 - X);
    int16_t dy = (Y > y0) ? (Y - y0) : (y0 - Y);
    int16_t sx = (x0 < X) ? 1 : -1;
    int16_t sy = (y0 < Y) ? 1 : -1;

    int16_t err = dx - dy;
    int16_t point[2] = {x0,y0};
    while(true) {
        //printf("%d,%d\n", x0, y0);
        if (x0 == X && y0 == Y)
            break;
        int16_t e2 = (err << 1);
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}


void arc_move_init(
    int16_t* u[2],
    int16_t X, int16_t Y,
    int16_t I, int16_t J,
    bool clockwise,
    struct move_state* out_state) {
        int32_t v[2] = {X, Y};
        int32_t c[2] = {(*u[0])+I, (*u[1])+J};

        int32_t umc[2] = {((*u[0]) - c[0]), ((*u[1]) - c[1])};
        int32_t vmc[2] = {(v[0] - c[0]), (v[1] - c[1])};
    
        int16_t r;
        cordic_hypot(umc[0], umc[1], &r);
        r = r < 0 ? -r : r;

        int32_t sin_theta = (umc[0]*vmc[1] - umc[1]*vmc[0]); // cross
        int32_t cos_theta = (umc[0]*vmc[0] + umc[1]*vmc[1]); // dot
        fp15_t theta; // total angle between start and end positions
        cordic_atan2(sin_theta, cos_theta, &theta);
    
        if (clockwise)
            theta = -theta;
        if (theta < 0)
            theta += Q15_ONE;
    
        int32_t arc_len = mul_i32_q15(r,theta);
        int32_t steps = max(1, arc_len);
    
        // basis vectors
        // second bais is perpendicular to e1
        int16_t su[2] = {fp15_div(umc[0],r), fp15_div(umc[1],r)};
        int32_t e1[2] = {su[0], su[1]};
        int32_t e2[2] = {-e1[1], e1[0]};
        if (clockwise) {
            e2[0] =  e1[1];
            e2[1] = -e1[0];
        }
        out_state->e1[0] = e1[0];
        out_state->e1[0] = e1[0];
        out_state->e2[1] = e2[1];
        out_state->e2[1] = e2[1];
        out_state->theta = theta;
        out_state->steps = steps;
        out_state->radius = r;
    }

void arc_move(int16_t* u[2], struct move_state* ms, uint16_t* out_buffer){
    fp15_t dtheta = (ms->theta / ms->steps);
    fp15_t dcos=0, dsin=0;
    cordic_sincos(dtheta,&dcos,&dsin);
    int32_t x = -mul_i32_q15(ms->radius,dcos); // initial condition is [r,0];
    int32_t y = -mul_i32_q15(ms->radius,dsin); // [x,y] * [[cos(theta) - sin(theta)],[sin(theta) + cos(theta)]];
    for(int i = 0; i < (AXIS_STEPPER_BUFFER_SIZE * sizeof(int16_t)); i+=2){
        ms->theta += dtheta;
        // TODO: this is wrong the steps should be defined as [0,1], [-1, 0] ... [1,1] and so on.
        out_buffer[i] = (x * ms->e1[0] + y * ms->e2[0]) >> Q15_BITS;
        out_buffer[i+1] = (x * ms->e1[1] + y * ms->e2[1]) >> Q15_BITS;
        int32_t x_new = mul_i32_q15(x,dsin) + mul_i32_q15(y,dcos);
        int32_t y_new = mul_i32_q15(x,dcos) - mul_i32_q15(y,dsin);
        x = x_new;
        y = y_new;
        ms->steps--;
    }
}

void stepper_create(task_t** self){
    static struct motion_planer mp_inst;
    axis_stepper_instance(&mp_inst.axes, &mp_inst.super,0);
    task_create(&mp_inst.super,
        (event_handler_t)&stepper_init,
        (event_handler_t)&stepper_dispatch);
    *self = &mp_inst.super;
}

static void stepper_init(task_t* const super, event_t const* const ie){
    struct motion_planer* self = container_of(super, struct motion_planer, super); 
    struct stepper_initEvt* initial_event = (struct stepper_initEvt*)ie;
    self->axis_count = 2;
    self->tick_frequency = 64000000/32; // ie->tick_frequency;
    self->active = 0;
    axis_stepper_init(self->axes, self->axis_count);  
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
            struct move_cmd* next_move = PTR_ADD(&event->super, sizeof(event->super));
            ring_buffer_enqueue(move_queue, (uint8_t*)(next_move));
            if(!self->active){
                axis_stepper_start_move(self->axes,dominant);
                self->evt.signal = EV_BUFFER_FILL_SIG;   
                task_event_post(super,&self->evt);
            }  
        } break;
        ////////////////////////////////////////////////////////
        case EV_BUFFER_FILL_SIG:{
            int16_t* fill;
            fill = axis_stepper_get_fill_buffer(self->axes);
            switch (self->curr_move->mode) {
                case G_ARC_CCW | G_ARC_CW:
                    arc_move(
                        self->last_pos,self->ms, fill);
                    break;
                case G_LINE:
                    gcode_line(
                        self->last_pos,
                        self->curr_move->X, self->curr_move->Y);
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
            *self->curr_move = next_move;
            int16_t* fill;
            fill = axis_stepper_get_fill_buffer(self->axes);
            switch (self->curr_move->mode) {
                case G_ARC_CCW | G_ARC_CW:
                    arc_move_init(self->last_pos, 
                        self->curr_move->X, self->curr_move->Y,
                        self->curr_move->I, self->curr_move->J,
                        self->curr_move->mode, self->ms);
                    arc_move(
                        self->last_pos,self->ms, fill);
                    break;
                case G_LINE:
                    gcode_line(
                        self->last_pos,
                        self->curr_move->X, self->curr_move->Y);
                    break;         
                case G_DWELL:
                    break; // not sure how to handle this yet. dont want to waste compute time
                default:
                    break;
            }
            int16_t dx = self->curr_move->X - self->last_pos[0];   // steps in y axis
            int16_t dy = self->curr_move->Y - self->last_pos[1];   // steps in x axis
            int16_t dominant = (dx < dy) ? dy : dx;
            axis_stepper_start_move(self->axes,dominant);
            TRISCbits.TRISC7 = 1;
        }break;
        ////////////////////////////////////////////////////////
        case EV_IDLE_SIG: {
            // do nothing. just defined for traceability
        }break;
    default:
        break;
    }
    return;
}