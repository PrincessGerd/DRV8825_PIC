#include <xc.h>
#include "../inc/timer.h"
#include "../inc/gpio.h"
#include "../inc/ccp.h"

CCP_DECLARE(pwm,
        CCP_MODULE_2,
        CCP_MODE_PWM,
        CCP_TRIG_UNUSED,
        CCP_RIGHT_ALIGNED);

DECLARE_TIMER(timer, TMR_MODULE_A);
 timer_config_t timer_cfg = {
        .mode = { 
            .mode = TMR2_MODE_FREE_RUNNING, 
            .submode.free_running = TMR2_FR_LEVEL_RESET_HIGH// TMR2_FR_LEVEL_RESET_HIGH
        },
        .clk_src = TMR2_CLK_FOSC4,    // default osc for stepper
        .reset_src = TMR2_RST_CCP1_OUT,
        .polarity = TMR2_RISING_EDGE,    // risingedge 
        .prescaler = TMR2_PRESCALER_16,   // no prescaling 
        .postscaler = TMR2_POSTSCALER_1, // no post scaling
        .clk_sync = true,           // sync with system clock
        .prescaler_sync = true
    };

 uint8_t count = 0;
 void __interrupt() isr(){
     if(PIR4bits.CCP2IF == 1){
        if(count < 255){
            count++;
        }else{
            TRISCbits.TRISC4 = 1;
            timer.disable(&timer);
            TRISCbits.TRISC7 = 0;
        }
        PIR4bits.CCP2IF = 0;
     }
 }
int main(void)
{
    SYSTEM_Initialize();
    INTERRUPT_GlobalInterruptEnable(); 
    
    ANSELCbits.ANSELC4 = 0;
    TRISCbits.TRISC4 = 1;
    //ODCONCbits.ODCC4 = 1; // enable for produc use with ext res
    
    pps_unlock();
    RC4PPS = 0x6;
    pps_lock();
    
    timer.init(&timer,&timer_cfg);
    timer.enable(&timer); 
    timer.set_period(&timer,249);
    
    pwm.init(&pwm);
   // pwm.set_register(&pwm,0xFF);
    uint16_t duty = 512;
    CCPR1H = duty >> 2;
    CCPR1L = (duty & 0x03) << 6;
    
    PIE4bits.CCP2IE = 1;
    
    while(!PIR4bits.TMR2IF);
    pwm.enable(&pwm);
    TRISCbits.TRISC4 = 0;
    
    while(1) {
    }
}