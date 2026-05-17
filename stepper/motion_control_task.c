#include "motion_control.h"
#include "../core/interrupts.h"
#include "../core/task_manager.h"
#include "../core/gpio.h"

static const struct motion_control_workEvt _motion_control_workEvt = {
    .mode = G_ARC_CW,
    .X = 0,
    .Y = 200,
    .I = 0,
    .J = -200,
    .super.signal = EV_WORK_SIG
};

static const struct motion_control_initEvt _motion_control_initEvt = {
    .super.signal = EV_WORK_SIG,
    .tick_frequency = 64000000/32
};

const struct stepper_initEvt* motion_control_workEvt = &_motion_control_workEvt;
const struct stepper_initEvt* motion_control_initEvt = &_motion_control_initEvt;

