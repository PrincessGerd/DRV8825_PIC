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

void __interrupt() isr(void){
    if(PIR0bits.DMA1AIF == 1){
        TRISCbits.TRISC7 = 0;
        PIR0bits.DMA1AIF = 0;
        task_event_consume(AO_drv8825);
    }
    
    //if(PWM1GIRbits.S1P1IF == 1){
    //    task_event_consume(AO_drv8825);
    //}
}

extern task_t* AO_drv8825;
int main(void) {
    //PWM1_16BIT_Initialize();
    INTCON0bits.GIEH    = 1;   /* Enables all high priority unmasked interrupts */
    INTCON0bits.GIEL    = 1; 
    INTCON0bits.GIE = 1;
    //PWM1_16BIT_Enable();
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
    PIR4bits.PWM1PIF = 1;
    PIR4bits.PWM1IF = 1;
    PIE4bits.PWM1PIE = 1;
    PIE4bits.PWM1IE = 1;
    enable_global_interrupts();
    task_event_consume(AO_drv8825);
    return task_run();
}