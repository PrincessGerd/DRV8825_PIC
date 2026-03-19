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
#include "../inc/vector.h"
#include "../inc/cordic.h"
#include <stdint.h>
#include <stdbool.h>
#include <xc.h>

#define MOVEMENT_BUFER_SIZE 512;
#define CMD_BUFFER_LEN 16;
struct motion_planer{
    task_t super;
    event_t evt;
    uint32_t tick_frequency;
    struct axis_stepper_t* axes;
    uint8_t axis_count;
    uint8_t active;

    uint8_t head;
    uint8_t tail;
};

typedef enum {
    G_ARC_CW = 0,
    G_ARC_CCW,
    G_LINE,
    G_DWELL
} gcode_cmd_t;

struct move_cmd{
    gcode_cmd_t cmd;
    int16_t[4] data;
};

static uv_t movement_buffer[MOVEMENT_BUFER_SIZE];
static struct move_cmd cmd_buffer[CMD_BUFFER_LEN];

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

    while (true) {
        printf("%d,%d\n", x0, y0);
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

void gcode_arc(int16_t u[2],
  int16_t X, int16_t Y,
  int16_t I, int16_t J,
  bool clockwise,
  uint16_t step = 1) {
    int32_t v[2] = {X, Y};
    int32_t c[2] = {u[0]+I, u[1]+J};
 
    int32_t umc[2] = {(u[0] - c[0]), (u[1] - c[1])};
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
        theta += FP_ONE;
 
    int32_t arc_len = mul_i32_q15(r,theta);
    int32_t steps = max(1, arc_len);
    int32_t segments = (steps % 31) + 1;
    fp15_t dtheta = (theta / (steps > 31 ? 31 : steps));
 
    // basis vectors
    // second bais is perpendicular to e1
    int16_t su[2] = {fp15_div(umc[0],r), fp15_div(umc[1],r)};
    int32_t e1[2] = {su[0], su[1]};
    int32_t e2[2] = {-e1[1], e1[0]};
    if (clockwise) {
        e2[0] =  e1[1];
        e2[1] = -e1[0];
    }
    //printf("r =%d | theta = %d | arc_len = %d\n",r, theta, arc_len);
    fp15_t dcos=0, dsin=0;
    cordic_sincos(dtheta,&dcos,&dsin);
    int32_t x = -mul_i32_q15(r,dcos); // initial condition is [r,0];
    int32_t y = -mul_i32_q15(r,dsin); // [x,y] * [[cos(theta) - sin(theta)],[sin(theta) + cos(theta)]];
    //printf("dtheta = %d,cos = %d | sin = %d\n",dtheta,dcos,dsin);
    // breakes down when recalculating, so removed. still breaks for large stepping segments
    // do to drift.
    // TODO : add recalculation
    for(int i = 0; i < steps; i++){
        int32_t px = (c[0] + x * e1[0] + y * e2[0]) >> FP_SHIFT;
        int32_t py = (c[1] + x * e1[1] + y * e2[1]) >> FP_SHIFT;
        int32_t x_new = mul_i32_q15(x,dsin) + mul_i32_q15(y,dcos);
        int32_t y_new = mul_i32_q15(x,dcos) - mul_i32_q15(y,dsin);
        x = x_new;
        y = y_new;
        printf("%d,%d,%d\n", px, py, 0);
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
    self->axis_count = initial_event.axes;
    self->tick_frequency = 64000000/32; // ie->tick_frequency;
    self->active = 0;
    axis_stepper_init(self->axes, self->axis_count);    
}

static void stepper_dispatch(task_t* const super, event_t* const e){
    struct motion_planer* self = container_of(super, struct motion_planer, super); 
    switch (e->signal) {
        ////////////////////////////////////////////////////////
        case EV_WORK_SIG:{
            struct stepper_workEvt* event = (struct stepper_workEvt*)e;
            uint32_t dx = event->steps_x;   // steps in y axis
            uint32_t dy = event->steps_y;   // steps in x axis
            
            axis_stepper_start_move(self->axes,mag);
            self->evt.signal = EV_BUFFER_FILL_SIG;   
            task_event_post(super,&self->evt);
        } break;
        ////////////////////////////////////////////////////////
        case EV_BUFFER_FILL_SIG:{
            uv_t* fill;
            fill = axis_stepper_get_fill_buffer(self->axes);
            for(int i = 0; i < MOVEMENT_BUFER_SIZE; i++){
                fill[i] = movement_buffer[i];
            }


        } break;
        ////////////////////////////////////////////////////////
        case EV_DONE_SIG:{
            TRISCbits.TRISC7 = 1;
        }break;
        ////////////////////////////////////////////////////////
        case EV_IDLE_SIG: {

        }break;
    default:
        break;
    }
    return;
}