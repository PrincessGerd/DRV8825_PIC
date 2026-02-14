#include <xc.h>
#include "../core/gpio.h"
#include "../inc/stepper.h" 
#include "../core/hw/inc/timer0_hw.h"
#include "../core/hw/inc/timer2x_hw.h"
#include "../core/hw/inc/clcx_hw.h"
#include "../core/irq_manager.h"

//extern enum vic_offset_t VIC_TMR0;
int main(void) {
    //INTCON0bits.GIEH     = 1;  /* Set GIE. Enables all high priority unmasked interrupts */
    //INTCON0bits.GIEL    = 1;  /* Set GIEL. Enables all low priority unmasked interrupts */
    //PIE3bits.TMR0IE = 1;
    stepper_pin_config_t pin_cfg = {
        .step_pin   = RC_4,      //rc4
        .dir_pin    = RC_5,      //rc5
        .enable_pin = RC_6,      //rc6
        .mode0_pin  = RC_1,      //rc1
        .mode1_pin  = RC_0,      //rc0
        .mode2_pin  = RC_3       //rc3
    };
    irq_manager_t int_manager;
    irq_manager_instance(&int_manager);
    
    stepper_device_t drv8825_dev;
    create_stepper_device(&drv8825_dev);
    stepper_init(drv8825_dev,&pin_cfg);
    stepper_set_direction(drv8825_dev,STEPPER_DIR_CW);
    stepper_set_microsteps(drv8825_dev,0x7);
    stepper_set_speed(drv8825_dev, 20000);
    stepper_enable(drv8825_dev);
    clear_interrupt(int_manager,0x1D);
    enable_interrupt(int_manager,0x1D);
    stepper_multi_step(drv8825_dev, 0xF);

    /*for(int n = 0; n < 8; n++){
        __delay_ms(500);
        stepper_set_microsteps(drv8825_dev,n);
        __delay_ms(500);
    }*/
    int i = 0;
    while(1) {
        if(!stepper_bussy(drv8825_dev)){
            if(i < 8){
                stepper_set_microsteps(drv8825_dev, i);
                stepper_multi_step(drv8825_dev, 0xFFFF);              
                //TRISCbits.TRISC7 ^= 1;
                i++;
            }
           
        }
    }
}