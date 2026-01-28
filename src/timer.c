#include "timer.h"
#include <xc.h>

void timer_init_impl(void* self, const timer_config_t* config){
    timer_regs_t* regs = &((timer_instance_t*)self)->regs; 
    *regs->CON &= ~(1 << TMR_CON_ON_SHIFT);
    *regs->CLKCON |= TMR_CLKCON_CS_MASK & (config->clk_src << TMR_CLKCON_CS_SHIFT);
    *regs->CON |= TMR_CON_CKPS_MASK  & (config->prescaler << TMR_CON_CKPS_SHIFT);
    *regs->CON |= TMR_CON_OUTPS_MASK & (config->postscaler << TMR_CON_OUTPS_SHIFT);
    *regs->HLT |= TMR_HLT_MODE_MASK  & (TMR2MODE(config->mode.mode,config->mode.submode.raw) << TMR_HLT_MODE_SHIFT);
    *regs->HLT |= TMR_HLT_CSYNC_MASK & (config->clk_sync << TMR_HLT_CSYNC_SHIFT);
    *regs->HLT |= TMR_HLT_PSYNC_MASK & (config->prescaler_sync << TMR_HLT_PSYNC_SHIFT);
    *regs->RST |= TMR_RST_RSEL_MASK  & (config->reset_src << TMR_RST_RSEL_SHIFT);
    //T2CONbits.ON = 0;
    //T2CLKCONbits.CS     = config.clk_src;             // set timer clock source       
    //T2CONbits.CKPS      = config.prescaler;           // set prescaler  1/xx
    //T2CONbits.OUTPS     = config.postscaler;          // set postscaler 1/xx
    //T2HLTbits.MODE      = TMR2MODE(config.mode.mode, 
    //                        config.mode.submode.raw); // set opertation mode
    //T2HLTbits.CKSYNC    = config.clk_sync;            // enable/disable clock sync
    //T2HLTbits.PSYNC     = config.prescaler_sync;      // enable/disable prescaler sync
}

inline void timer_enable_impl(void* self)  {
    timer_regs_t* regs = &((timer_instance_t*)self)->regs; 
    *regs->CON |= (1 << TMR_CON_ON_SHIFT);
}

inline void timer_disable_impl(void* self) {
    timer_regs_t* regs = &((timer_instance_t*)self)->regs; 
    *regs->CON &= ~(1 << TMR_CON_ON_SHIFT);
}

inline void timer_set_period_impl(void* self,uint8_t count){
    timer_regs_t* regs = &((timer_instance_t*)self)->regs; 
    if(*regs->CON & (1 << TMR_CON_ON_SHIFT)){
        *regs->PR = count;
    }
}

inline void timer_set_count_impl(void* self,uint8_t count){
    timer_regs_t* regs = &((timer_instance_t*)self)->regs; 
    if(*regs->CON & (1 << TMR_CON_ON_SHIFT)){
        *regs->TMR = count;
    }
}

void timer_set_callback_impl(void* self, timer_callback_t callback){
    if(callback != 0){
        timer_callbacks[((timer_instance_t*)self)->module_num] = callback;
    }
}

inline void timer2_enable_interrupt() {PIE4bits.TMR2IE = 1;}
inline void timer2_disable_interrupt() {PIE4bits.TMR2IE = 0;}
//void __interrupt() timer2_isr(void){
//    if(PIR4bits.TMR2IF == 1){
//        PIR4bits.TMR2IF = 0;
//    }
//    if(timer2_callback != 0){
//        timer2_callback();
//    }
//}