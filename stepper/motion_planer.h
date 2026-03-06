#ifndef MOTION_PLANER_H
#define MOTION_PLANER_H
#include <stdint.h>
#include <stdbool.h>
#include "../core/task_manager.h"
#include "stepper_device.h"

extern const struct stepper_initEvt* drv8825_workEvt;
extern const struct stepper_initEvt* drv8825_initEvt;

enum motion_planer_signals{
    STEPPER_IDLE_SIG = 0,
    STEPPER_WORK_SIG = 1,
    STEPPER_DONE_SIG = 2,
    STEPPER_UPDATE_SIG = 3,
    DRIVER_FAULT_SIG = 4
};
typedef struct stepper_workEvt{
    event_t     super;
    uint32_t    steps_x;
    uint32_t    steps_y;
    uint16_t    desired_speed; 
}stepper_workEvt_t;

typedef struct stepper_initEvt{
    event_t   super;
    uint32_t  tick_frequency;
}stepper_initEvt_t;

struct stepper;
static task_t* AO_drv8825;

void stepper_create(task_t** self);

#endif