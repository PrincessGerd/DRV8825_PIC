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
#include "stepper_device.h"

enum signals{
    FAST_TICK_TIMEOUT_SIG,
    STEPPER_WORK_SIG,
    STEPPER_STOP_SIG,
    STEPPER_DONE_SIG
};

typedef struct stepper_workEvt{
    event_t     super;
    uint16_t    steps;
    uint16_t    speed;
    uint16_t    accel; 
}stepper_workEvt_t;

typedef struct stepper_initEvt{
    event_t         super;
    stepper_pins_t  pins;
}stepper_initEvt_t;

struct stepper;
void stepper_create(task_t** self);
#endif	/* STEPPER_H*/


