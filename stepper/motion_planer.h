#ifndef MOTION_PLANER_H
#define MOTION_PLANER_H
#include <stdint.h>
#include <stdbool.h>
#include "../core/task_manager.h"
#include "stepper_device.h"

extern const struct stepper_initEvt* drv8825_workEvt;
extern const struct stepper_initEvt* drv8825_initEvt;

typedef enum {
    G_ARC_CW = 0,
    G_ARC_CCW,
    G_LINE,
    G_DWELL
} gcode_cmd_t;

enum motion_planer_signals{
    EV_IDLE_SIG,
    EV_WORK_SIG,
    EV_DONE_SIG,
    EV_BUFFER_FILL_SIG
};
typedef struct stepper_workEvt{
    event_t     super;
    gcode_cmd_t mode;
    int16_t X, Y;
    int16_t I, J;
}stepper_workEvt_t;

typedef struct stepper_initEvt{
    event_t   super;
    uint32_t  tick_frequency;
}stepper_initEvt_t;

struct stepper;
static task_t* AO_drv8825;

void stepper_create(task_t** self);

#endif