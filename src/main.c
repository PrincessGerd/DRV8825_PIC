#include "../core/gpio.h"
#include "../stepper/stepper.h" 
#include "../core/task_manager.h"
#include "../core/system.h"
#include "../core/interrupts.h"
#include "stepper.h"
#include <xc.h>

int task_run(void) { // to prevent reset
    system_init();
    systick_config(10000, SYS_TICK_TIMER);
    systick_config(2000000, FAST_TICK_TIMER);
    while(1) {
        __nop();
    }
}

void __interrupt(high_priority) isr(void){
    if(PIR0bits.DMA1DCNTIF == 1){
    PIR0bits.DMA1DCNTIF = 0;
    }
    if(PIR0bits.DMA1SCNTIF == 1){
        PIR0bits.DMA1SCNTIF = 0;
        //TRISCbits.TRISC7 = 0;
        task_event_consume(AO_drv8825);
    }
    if(PIR4bits.PWM1PIF == 1 || PIR4bits.PWM1IF == 1){
        PIR4bits.PWM1PIF = 0;
        PIR4bits.PWM1PIF = 0;
        //TRISCbits.TRISC7 = 0;
    }
}

extern task_t* AO_drv8825;
int main(void) {
    stepper_create(&AO_drv8825);
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
    task_event_post(AO_drv8825, &drv8825_workEvt->super);
    enable_global_interrupts();
    task_event_consume(AO_drv8825);
    return task_run();
}