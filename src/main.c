#include "../core/gpio.h"
#include "../stepper/stepper.h" 
#include "../core/task_manager.h"
#include "../core/system.h"
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
void PWM1_16BIT_Initialize(void)
{
    //PWMERS External Reset Disabled; 
    PWM1ERS = 0x0;

    //PWMCLK FOSC; 
    PWM1CLK = 0x2;

    //PWMLDS Autoload disabled; 
    PWM1LDS = 0x0;

    //PWMPRL 127; 
    PWM1PRL = 0x7F;

    //PWMPRH 2; 
    PWM1PRH = 0x2;

    //PWMCPRE No prescale; 
    PWM1CPRE = 0x0;

    //PWMPIPOS No postscale; 
    PWM1PIPOS = 0x0;

    //PWMS1P1IF PWM1 output match did not occur; PWMS1P2IF PWM2 output match did not occur; 
    PWM1GIR = 0x0;

    //PWMS1P1IE disabled; PWMS1P2IE disabled; 
    PWM1GIE = 0x0;

    //PWMPOL1 disabled; PWMPOL2 enabled; PWMPPEN disabled; PWMMODE Left aligned mode; 
    PWM1S1CFG = 0x80;

    //PWMS1P1L 160; 
    PWM1S1P1L = 0xA0;

    //PWMS1P1H 0; 
    PWM1S1P1H = 0x0;

    //PWMS1P2L 160; 
    PWM1S1P2L = 0xA0;

    //PWMS1P2H 0; 
    PWM1S1P2H = 0x0;
    
    //Clear PWM1_16BIT period interrupt flag
    PIR4bits.PWM1PIF = 0;
    
    //Clear PWM1_16BIT interrupt flag
    PIR4bits.PWM1IF = 0;
    
    //Clear PWM1_16BIT slice 1, output 1 interrupt flag
    PWM1GIRbits.S1P1IF = 0;
    
    //Clear PWM1_16BIT slice 1, output 2 interrupt flag
    PWM1GIRbits.S1P2IF = 0;
    
    //PWM1_16BIT interrupt enable bit
    PIE4bits.PWM1IE = 1;
    
    //PWM1_16BIT period interrupt enable bit
    PIE4bits.PWM1PIE = 1;
    
    //PWMEN enabled; PWMLD disabled; PWMERSPOL disabled; PWMERSNOW disabled; 
    PWM1CON = 0x80;
}

void PWM1_16BIT_Enable()
{
    PWM1CON |= _PWM1CON_EN_MASK;
}

void PWM1_16BIT_Disable()
{
    PWM1CON &= (~_PWM1CON_EN_MASK);
}

void __interrupt(high_priority) isr(void){
    if(PIR4bits.PWM1PIF == 1){
        TRISCbits.TRISC7 = 0;
        PIR4bits.PWM1IF = 0;
        task_event_consume(AO_drv8825);
    }
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
    //task_event_consume(AO_drv8825);
    return task_run();
    
}