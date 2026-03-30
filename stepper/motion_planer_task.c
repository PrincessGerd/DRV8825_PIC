#include "motion_planer.h"
#include "../core/interrupts.h"
#include "../core/task_manager.h"
#include "../core/gpio.h"
#include <xc.h>


static const struct stepper_workEvt _drv8825_workEvt = {
    .mode = G_ARC_CW,
    .X = 0,
    .Y = 200,
    .I = 0,
    .J = -200,
    .super.signal = EV_WORK_SIG
};

static const struct stepper_initEvt _drv8825_initEvt = {
    .super.signal = EV_WORK_SIG,
    .tick_frequency = 64000000/32
};

const struct stepper_initEvt* drv8825_workEvt = &_drv8825_workEvt;
const struct stepper_initEvt* drv8825_initEvt = &_drv8825_initEvt;

