#include "../inc/drv8825_task.h"
#include "../core/gpio.h"

static stepper_initEvt_t const _drv8825_initEvt ={
    .super.signal = STEPPER_WORK_SIG,
    .pins = {
        .step_pin   = RC_4,      //rc4
        .dir_pin    = RC_5,      //rc5
        .enable_pin = RC_6,      //rc6
        .mode0_pin  = RC_1,      //rc1
        .mode1_pin  = RC_0,      //rc0
        .mode2_pin  = RC_3       //rc3
    }
};


static stepper_workEvt_t const _drv8825_workEvt = {
    .super.signal = STEPPER_WORK_SIG,
    .accel = 0xFF,
    .steps = 0xFFFE,
    .speed = 0xFFFF
};

stepper_initEvt_t const* drv8825_initEvt = &_drv8825_initEvt;
stepper_workEvt_t const* drv8825_workEvt = &_drv8825_workEvt;