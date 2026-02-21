#include "../core/gpio.h"
#include "../inc/stepper.h" 
#include "../core/task_manager.h"
#include "../core/system.h"

int main(void) {
    system_init();
    systick_config(1000, SYS_TICK_TIMER);
    systick_config(1000000, FAST_TICK_TIMER);
    return 0;

    static task_t const* AO_drv8825;
    stepper_create(AO_drv8825);
    static event_t const* StpQueue[4];
    task_start(
        AO_drv8825,
        4u,
        7,
        StpQueue,
        (event_t const*)0);
}