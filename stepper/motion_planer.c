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

union stepper_port {
    struct{
        uint8_t x_step;
        uint8_t x_dir;
        uint8_t y_step;
        uint8_t y_dir;
    };
    uint8_t mask;
};
#define DIR_MASK(port) ((uint8_t)(port->x_dir | port->y_dir))
#define PORT_MASK(port) ((uint8_t)(port->x_step | port->y_step))

struct line_state {
    int16_t dx, dy;
    int16_t countx, county;
    int8_t sx, sy;
    int16_t err;
};

struct move_state{
    int16_t e1[2], e2[2];       // basis vectors
    int32_t c[2];               // center point
    int32_t steps_remaining;    //
    int32_t x;
    int32_t y; 
    int16_t dcos;               //
    int16_t dsin;               //
    struct line_state line;
    bool line_active;
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
    union stepper_port port;
    struct move_cmd* curr_move;
    struct move_state ms;
};

RING_BUFFER_DECLARE(move_queue,MOVEMENT_BUFER_SIZE,sizeof(struct move_cmd))
static void stepper_init(task_t* const super, event_t const* const ie);
static void stepper_dispatch(task_t* const super, event_t* const e);

void gcode_line_init(int16_t dx, int16_t dy, struct line_state* ls){
    ls->sx = (dx > 0) ? 1 : -1;
    ls->sy = (dy > 0) ? 1 : -1;

    ls->dx = (dx >= 0) ? dx : -dx;
    ls->dy = (dy >= 0) ? dy : -dy;

    ls->countx = 0;
    ls->county = 0;

    ls->err = dx - dy;
}

int gcode_line_move(
    struct line_state* ls,
    union stepper_port* port,
    uint8_t* out_buffer, 
    int16_t offset)
{
    int16_t i = offset;
    while (i < AXIS_STEPPER_BUFFER_SIZE-2) {
        if (ls->countx == ls->dx && ls->county == ls->dy)
            break;
            
        int16_t e2 = (ls->err << 1);
        uint8_t point = 0;

        if (e2 > -ls->dy) {
            ls->err -= ls->dy;
            ls->countx++;
            point |= port->x_step | (ls->sx < 0 ? port->x_dir : 0);
        }

        if (e2 < ls->dx) {
            ls->err += ls->dx;
            ls->county++;
            point |= port->y_step | (ls->sy < 0 ? port->y_dir : 0);
        }
        // TODO: find a way to make sure the transistion between descriptors doesnt end on high
        out_buffer[i++] = point;
        out_buffer[i++] = (point & DIR_MASK(port));
    }
    return (i - offset);
}

#define MAX_DTHETA_SHIFT 10
#define MAX_DTHETA_Q15 (1 << MAX_DTHETA_SHIFT)
void arc_move_init(
    int16_t u[2],
    int16_t X, int16_t Y,
    int16_t I, int16_t J,
    bool clockwise,
    struct move_state* out_state) {
        int32_t v[2] = {X, Y};
        int32_t c[2] = {u[0]+I, u[1]+J};

        int32_t umc[2] = {(u[0] - c[0]), (u[1] - c[1])};
        int32_t vmc[2] = {(v[0] - c[0]), (v[1] - c[1])};
    
        int32_t r;
        cordic_hypot(umc[0], umc[1], &r);
        r = abs(r);

        int32_t sin_theta = (((int32_t)umc[0]*vmc[1] - (int32_t)umc[1]*vmc[0])); // cross
        int32_t cos_theta = (((int32_t)umc[0]*vmc[0] + (int32_t)umc[1]*vmc[1])); // dot
        int32_t theta; // total angle between start and end positions
        cordic_atan2(sin_theta, cos_theta, &theta);
        if (clockwise)
            theta = -theta;
        
        int32_t steps = (abs(theta) + (MAX_DTHETA_Q15) - 1) >> MAX_DTHETA_SHIFT;
        steps = max(1, steps);
        int16_t dtheta = (theta / steps);

        // basis vectors
        // second bais is perpendicular to e1
        int16_t su[2] = {fp15_div(umc[0],r), fp15_div(umc[1],r)};
        int16_t e1[2] = {su[0], su[1]};
        int16_t e2[2] = {-e1[1], e1[0]};
        if (clockwise) {
            e2[0] =  e1[1];
            e2[1] = -e1[0];
        }

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
        out_state->x = ((int32_t)r << Q15_BITS);
        out_state->y = 0;
        out_state->c[0] = c[0];
        out_state->c[1] = c[1];
        out_state->line_active = false;
    }

void arc_move(struct move_state* ms, union stepper_port* port, uint8_t* out_buffer){
    int N = (AXIS_STEPPER_BUFFER_SIZE < ms->steps_remaining) ? AXIS_STEPPER_BUFFER_SIZE : ms->steps_remaining;
    int16_t offset = 0;
    int16_t dx = 0, dy = 0;
    while(offset < N-2){
        if(!ms->line_active){
            if(ms->steps_remaining == 0) {break;}
            int32_t x_new = mul_i32_q15(ms->x,ms->dcos) - mul_i32_q15(ms->y,ms->dsin);
            int32_t y_new = mul_i32_q15(ms->x,ms->dsin) + mul_i32_q15(ms->y,ms->dcos);
            dx = (int16_t)((mul_i32_q15((x_new - ms->x), ms->e1[0]) + mul_i32_q15((y_new - ms->y),ms->e2[0])) >> Q15_BITS);
            dy = (int16_t)((mul_i32_q15((x_new - ms->x), ms->e1[1]) + mul_i32_q15((y_new - ms->y),ms->e2[1])) >> Q15_BITS);
            if((dx == 1 || dx == -1) && (dy == 1 || dy == -1)){
                out_buffer[offset] |= port->x_step | (dx < 0 ? port->x_dir : 0);
                out_buffer[offset] |= port->y_step | (dy < 0 ? port->x_dir : 0);
                offset++;
            }else{
                gcode_line_init(dx,dy,&ms->line);
                ms->line_active = true;
            }
            ms->x = x_new;
            ms->y = y_new;
            ms->steps_remaining--;
        }
        offset +=  gcode_line_move(&ms->line, port, out_buffer, offset);
        if(ms->line.countx == ms->line.dx && ms->line.county == ms->line.dy){
            //memset(&ms->line, 0, sizeof(struct line_state));
            ms->line_active = false;
        }
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
    memset(&mp_inst.ms, 0, sizeof(struct move_state));
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
    union stepper_port port = {
        .x_step = (1 << 4),
        .x_dir  = (1 << 5),
        .y_step = (1 << 1),
        .y_dir  = (1 << 0)
    };
    self->port = port;
    axis_stepper_init(self->axes, self->axis_count, &LATC, port.mask);  
    move_queue_init();  
}

static void stepper_dispatch(task_t* const super, event_t* const e){
    struct motion_planer* self = container_of(super, struct motion_planer, super); 
    switch (e->signal) {
        ////////////////////////////////////////////////////////
        case EV_WORK_SIG:{
            struct stepper_workEvt* event = (struct stepper_workEvt*)e;
            struct move_cmd* next_move = PTR_ADD(&event->super, sizeof(event_t));
            ring_buffer_enqueue(move_queue, next_move);
            if(self->active == 0){
                self->curr_move = next_move;
                self->evt.signal = EV_MOVE_DONE_SIG;   
                task_event_post(super,&self->evt);
            }  
        } break;
        ////////////////////////////////////////////////////////
        case EV_BUFFER_FILL_SIG:{
            uint8_t* fill;
            axis_stepper_get_fill_buffer(self->axes, &fill);
            memset(fill, 0, AXIS_STEPPER_BUFFER_SIZE);
            switch (self->curr_move->mode) {
                case G_ARC_CW:{
                    arc_move(&self->ms, &self->port, fill);
                } break;
                case G_LINE:{
                    gcode_line_move(
                        &self->ms.line, &self->port, fill, 0);
                } break;         
                case G_DWELL:
                    break; // not sure how to handle this yet. dont want to waste compute time
                default:
                    break;
            }
            // THIS IS WRONG. 
            self->last_pos[0] = self->curr_move->X;
            self->last_pos[1] = self->curr_move->Y;
        } break;
        ////////////////////////////////////////////////////////
        case EV_MOVE_DONE_SIG:{
            struct move_cmd next_move;
            bool bdone = ring_buffer_dequeue(move_queue, (uint8_t*)(&next_move));  
            if(bdone == false){
                self->evt.signal = EV_IDLE_SIG;   
                task_event_post(super,&self->evt);
                return;
            }
            uint8_t* fill;
            axis_stepper_get_fill_buffer(self->axes, &fill);
            memset(fill, 0, AXIS_STEPPER_BUFFER_SIZE);
            self->curr_move = &next_move;
            int16_t steps = 0;
            switch (self->curr_move->mode) {
                case G_ARC_CW:{
                    arc_move_init(self->last_pos, 
                        self->curr_move->X, self->curr_move->Y,
                        self->curr_move->I, self->curr_move->J,
                        self->curr_move->mode, &self->ms);
                    arc_move(&self->ms, &self->port, fill);
                        steps = self->ms.steps_remaining;
                } break;
                case G_LINE:{
                    int16_t dx = self->curr_move->X - self->last_pos[0];   // steps in y axis
                    int16_t dy = self->curr_move->Y - self->last_pos[1];   // steps in x axis
                    steps = (dx < dy) ? abs(dy) : abs(dx);
                    gcode_line_init(dx,dy,&self->ms.line);
                    gcode_line_move(&self->ms.line, &self->port,fill,0);
                } break;
                case G_DWELL:
                    break; // not sure how to handle this yet. dont want to waste compute time
                default:
                    break;
            }
            axis_stepper_start_move(self->axes,steps);
            self->evt.signal = EV_BUFFER_FILL_SIG;   
            task_event_post(super,&self->evt);
            self->last_pos[0] = self->curr_move->X;
            self->last_pos[1] = self->curr_move->Y;
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