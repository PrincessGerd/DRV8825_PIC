//#include "../core/gpio.h"
//#include "../stepper/axis_stepper.h" 
//#include "../stepper/motion_planer.h"
//#include "../core/task_manager.h"
//#include "../core/system.h"
//#include "../core/interrupts.h"
#include "../core/serial_logger.h"
#include <xc.h>

int task_run(void) { // to prevent reset
    //system_init();
    //systick_config(10000, SYS_TICK_TIMER);
    //systick_config(2000000, FAST_TICK_TIMER);
    while(1) {
        __nop();
    }
}

//extern task_t* AO_drv8825;
int main(void) {
    
    //PIR0bits.DMA1SCNTIF = 0;                 // Clear interrupt flag
    //PIE0bits.DMA1SCNTIE = 1;                 // Enable source count done interrupt
    //gpio_set_direction(RC_4, IO_DIR_OUTPUT);
    //gpio_set_mode(RC_4, IO_MODE_DIGITAL);
    //gpio_set_direction(RC_5, IO_DIR_OUTPUT);
    //gpio_set_mode(RC_5, IO_MODE_DIGITAL);
    //gpio_set_direction(RC_1, IO_DIR_OUTPUT);
    //gpio_set_mode(RC_1, IO_MODE_DIGITAL);
    //gpio_set_direction(RC_0, IO_DIR_OUTPUT);
    //gpio_set_mode(RC_0, IO_MODE_DIGITAL);
    //INTCON0bits.GIE = 1; //Restore interrupts
//
    //stepper_create(&AO_drv8825);
    //static event_t* StpQueue[4];
    //task_event_post(
    //    AO_drv8825,
    //    &drv8825_initEvt->super);
    //task_start(
    //    AO_drv8825,
    //    4u,
    //    7,
    //    StpQueue,
    //    &(drv8825_initEvt->super));
    //AO_drv8825->dispatch(AO_drv8825,&drv8825_workEvt->super);
    //task_event_post(AO_drv8825, &drv8825_workEvt->super);
    //enable_global_interrupts();
    //task_event_consume(AO_drv8825);
    init_loggger();
    SLOG_TRACE("message with some numbers %d %d" , 10, 20);
    
    return task_run();
}