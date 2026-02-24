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
    STEPPER_IDLE_SIG,
    STEPPER_WORK_SIG,
    STEPPER_DONE_SIG,
    STEPPER_UPDATE_SIG,
    DRIVER_FAULT_SIG
};
typedef struct stepper_workEvt{
    event_t     super;
    uint32_t    steps;
    uint16_t    speed;
    uint16_t    accel; 
}stepper_workEvt_t;

typedef struct stepper_initEvt{
    event_t   super;
    uint32_t  tick_frequency;
    stepper_pins_t  pins;
}stepper_initEvt_t;

struct stepper;
void stepper_create(task_t** self);
#endif	/* STEPPER_H*/


