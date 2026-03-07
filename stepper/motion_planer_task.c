#include "motion_planer.h"
#include "../core/interrupts.h"
#include "../core/task_manager.h"
#include "../core/gpio.h"
#include <xc.h>


static const struct stepper_workEvt _drv8825_workEvt = {
    .desired_speed = 4000,
    .steps_x = 15000,
    .steps_y = 12000,
    .super.signal = STEPPER_WORK_SIG
};
const struct stepper_initEvt* drv8825_workEvt = &_drv8825_workEvt;

static const struct stepper_initEvt _drv8825_initEvt = {
    .super.signal = STEPPER_WORK_SIG,
    .tick_frequency = 64000000/32
};
/*
    .pins = {
        .dir_pin    = RC_5,
        .enable_pin = RC_6,
        .mode0_pin  = RC_0,
        .mode0_pin  = RC_1,
        .mode0_pin  = RC_2
    }
*/
const struct stepper_initEvt* drv8825_initEvt = &_drv8825_initEvt;

