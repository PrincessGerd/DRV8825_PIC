#ifndef MOTION_CONTROL_H
#define MOTION_CONTROL_H

#include <stdbool.h>
#include <stdint.h>
#include "../core/task_manager.h"

extern const struct motion_control_workEvt* drv8825_workEvt;
extern const struct motion_control_initEvt* drv8825_initEvt;

typedef enum {
    G_ARC_CW = 0,
    G_ARC_CCW,
    G_LINE,
    G_DWELL
} gcode_cmd_t;

typedef struct move_cmd {
    gcode_cmd_t mode;
    int16_t X, Y;
    int16_t I, J;
} move_cmd_t;

enum motion_control_signals{
    EV_IDLE_SIG = 0,
    EV_WORK_SIG,
    EV_MOVE_DONE_SIG,
    EV_BUFFER_FILL_SIG
};

typedef struct motion_control_workEvt{
    event_t     super;
    gcode_cmd_t mode;
    int16_t X, Y;
    int16_t I, J;
}motion_control_workEvt_t;

typedef struct motion_control_initEvt{
    event_t   super;
    uint32_t  tick_frequency;
}motion_control_initEvt_t;

struct motion_control;
static task_t* AO_drv8825;

void stepper_create(task_t** self);


#endif