#include <xc.h>
#include "../core/gpio.h"
#include "../inc/stepper.h" 
#include "../core/task_manager.h"
#include "../core/system.h"

int main(void) {
    system_init();
    systick_config(1000, SYS_TICK_TIMER);
    systick_config(1000000, FAST_TICK_TIMER);
    return 0;
}