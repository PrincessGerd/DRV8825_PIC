#ifndef MOTION_PLANER_H
#define MOTION_PLANER_H
#include <stdint.h>
#include <stdbool.h>
#include "../core/task_manager.h"

extern const struct motion_planer_initEvt* drv8825_workEvt;
extern const struct motion_planer_initEvt* drv8825_initEvt;


enum motion_planer_signals{
    EV_IDLE_SIG = 0,
    EV_WORK_SIG,
    EV_MOVE_DONE_SIG,
};

typedef struct motion_planer_workEvt{
    event_t     super;
    int16_t dx, dy, dz;
    int16_t feed_rate;
}motion_planer_workEvt_t;

typedef struct motion_planer_initEvt{
    event_t   super;
    uint32_t  tick_frequency;
}motion_planer_initEvt_t;

struct motion_planer;
static task_t* AO_motion_planer;

void stepper_create(task_t** self);

#endif