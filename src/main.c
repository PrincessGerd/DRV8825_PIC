#include "../core/gpio.h"
#include "../stepper/stepper.h" 
#include "../core/task_manager.h"
#include "../core/system.h"
#include "drv8825_task.h"
#include <xc.h>

extern void stepper_dispatch(task_t* const super, event_t* const e);

int main(void) {
    system_init();
    systick_config(1000, SYS_TICK_TIMER);
    systick_config(1000000, FAST_TICK_TIMER);

    static task_t* AO_drv8825;
    stepper_create(&AO_drv8825);
    static event_t const* StpQueue[4];
    fast_tick_event_create(&drv8825_initEvt->super,AO_drv8825,STEPPER_WORK_SIG);
    task_start(
        AO_drv8825,
        4u,
        7,
        StpQueue,
        &(drv8825_initEvt->super));
        stepper_workEvt_t work = {
            .accel = 1,
            .speed = 128,
            .steps = 0xFFFF-1,
            .super.super.signal = STEPPER_WORK_SIG
        };
    stepper_dispatch(AO_drv8825, &work.super.super);
    task_event_post(AO_drv8825,&work.super.super);
}