#include "../core/gpio.h"
#include "../stepper/axis_stepper.h" 
#include "../stepper/motion_planer.h"
#include "../core/task_manager.h"
#include "../core/system.h"
#include "../core/interrupts.h"
#include <xc.h>

int task_run(void) { // to prevent reset
    system_init();
    systick_config(10000, SYS_TICK_TIMER);
    systick_config(2000000, FAST_TICK_TIMER);
    while(1) {
        __nop();
    }
}

extern task_t* AO_drv8825;
int main(void) {
    PWM1GIRbits.S1P1IF = 0;
    PWM1GIEbits.S1P1IE = 1;
    PWM2GIRbits.S1P1IF = 0;
    PWM2GIEbits.S1P1IE = 1;

    PIR0bits.DMA1SCNTIF = 0;                 // Clear interrupt flag
    PIE0bits.DMA1SCNTIE = 1;                 // Enable source count done interrupt
    //PIR5bits.PWM2IF = 0;
    //PIR5bits.PWM2PIF = 0;
    //PIE5bits.PWM2IE = 1;
    //PIE5bits.PWM2PIE = 1;
    //PIR1bits.DMA2SCNTIF = 0;                 // Clear interrupt flag
    //PIE1bits.DMA2SCNTIE = 1;                 // Enable source count done interrupt
    gpio_set_direction(RC_4, IO_DIR_OUTPUT);
    gpio_set_mode(RC_4, IO_MODE_DIGITAL);
    gpio_set_direction(RC_5, IO_DIR_OUTPUT);
    gpio_set_mode(RC_5, IO_MODE_DIGITAL);
    INTCON0bits.GIE = 0; //Suspend interrupts
    PPSLOCK = 0x55; //Required sequence
    PPSLOCK = 0xAA; //Required sequence
    PPSLOCKbits.PPSLOCKED = 0; //Set PPSLOCKED bit
    INTCON0bits.GIE = 1; //Restore interrupts
    RC4PPS = 0x07;
    RC5PPS = 0x0A;
    //RC3PPS = 0x08;
    INTCON0bits.GIE = 0; //Suspend interrupts
    PPSLOCK = 0x55; //Required sequence
    PPSLOCK = 0xAA; //Required sequence
    PPSLOCKbits.PPSLOCKED = 1; //Set PPSLOCKED bit
    INTCON0bits.GIE = 1; //Restore interrupts
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