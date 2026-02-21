/* 
* File:   stepper.h
* Author: Gard
*
* Created on January 20, 2026, 2:16 PM
*/

#ifndef STEPPER_H
#define	STEPPER_H

#include <stdint.h>
#include <stdbool.h>
#include "../core/task_manager.h"

enum signals{
    FAST_TICK_TIMEOUT_SIG,
    STEPPER_WORK_SIG,
    STEPPER_STOP_SIG,
    STEPPER_DONE_SIG
};

typedef struct stepper_workEvt{
    event_t   super;
    uint32_t  steps;
    uint32_t  speed;
    uint32_t  accel; 
}stepper_workEvt_t;

void stepper_create(task_t const* self);
#endif	/* STEPPER_H*/


