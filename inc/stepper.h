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
    TIMEOUT_SIG,
    STEPPER_WORK_SIG,
    STEPPER_
};

typedef struct stepper_workEvt{
    event_t   super;
    uint32_t  steps;
    uint32_t  speed;
    uint32_t  accel; 
}stepper_workEvt_t;

void stepper_init(void);
extern task_t* const drv8825;

#endif	/* STEPPER_H*/


