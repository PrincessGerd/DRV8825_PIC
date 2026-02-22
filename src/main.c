#include "../core/gpio.h"
#include "../stepper/stepper.h" 
#include "../core/task_manager.h"
#include "../core/system.h"
#include "drv8825_task.h"
#include <xc.h>

int task_run(void) { // to prevent reset
    system_init();
    systick_config(10000, SYS_TICK_TIMER);
    systick_config(200000, FAST_TICK_TIMER);
    while(1) {
        __nop();
    }
}

int main(void) {
    static task_t* AO_drv8825;
    stepper_create(&AO_drv8825);
    gpio_set_mode(RC_4,IO_MODE_DIGITAL);
    gpio_set_direction(RC_4,IO_DIR_OUTPUT);
    static event_t* StpQueue[4];
    task_event_post(
        AO_drv8825,
        &drv8825_initEvt->super);
    task_start(
        AO_drv8825,
        4u,
        7,
        StpQueue,
        &(drv8825_initEvt->super));
    AO_drv8825->dispatch(AO_drv8825,&drv8825_workEvt->super);
    //task_event_post(AO_drv8825, &drv8825_workEvt->super);
    return task_run();
}