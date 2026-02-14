#include "../inc/timer2x_hw.h"
#include "../registers.h"
#include <stdint.h>

typedef struct timer2x_hw{
    volatile uint8_t* TMR;
    volatile uint8_t* PR;
    volatile uint8_t* CON;
    volatile uint8_t* HLT;
    volatile uint8_t* CLKCON;
    volatile uint8_t* RST;
} timer2x_hw_t;


void create_timer2x(const struct timer2x_hw **timer2x_hw_inst_out, uint8_t module_num){
    const uintptr_t base = (TMR_BASE_ADDRESS + (module_num * TMR_NUM_MODULES));
    static struct timer2x_hw hw;
    hw.TMR    = (volatile uint8_t*)(base + TMR_TMR_OFFSET);
    hw.PR     = (volatile uint8_t*)(base + TMR_PR_OFFSET);
    hw.CON    = (volatile uint8_t*)(base + TMR_CON_OFFSET);
    hw.HLT    = (volatile uint8_t*)(base + TMR_HLT_OFFSET);
    hw.CLKCON = (volatile uint8_t*)(base + TMR_CLKCON_OFFSET);
    hw.RST    = (volatile uint8_t*)(base + TMR_RST_OFFSET);
    *timer2x_hw_inst_out = &hw;
}

void timer2x_init(const struct timer2x_hw *self, timer_config_t* config){
    *self->CON      &= ~(1 << TMR_CON_ON_SHIFT);
    *self->CLKCON   |= TMR_CLKCON_CS_MASK & (config->clk_src << TMR_CLKCON_CS_SHIFT);
    *self->CON      |= TMR_CON_CKPS_MASK  & (config->prescaler << TMR_CON_CKPS_SHIFT);
    *self->CON      |= TMR_CON_OUTPS_MASK & (config->postscaler << TMR_CON_OUTPS_SHIFT);
    *self->HLT      |= TMR_HLT_MODE_MASK  & (TMR2MODE(config->mode.mode,config->mode.submode.raw) << TMR_HLT_MODE_SHIFT);
    *self->HLT      |= TMR_HLT_CSYNC_MASK & (config->clk_sync << TMR_HLT_CSYNC_SHIFT);
    *self->HLT      |= TMR_HLT_PSYNC_MASK & (config->prescaler_sync << TMR_HLT_PSYNC_SHIFT);
    *self->RST      |= TMR_RST_RSEL_MASK  & (config->reset_src << TMR_RST_RSEL_SHIFT);
}

void timer2x_enable(const struct timer2x_hw* self){
    *self->CON |= (1 << TMR_CON_ON_SHIFT);
}

void timer2x_disable(const struct timer2x_hw* self) {
    *self->CON &= ~(1 << TMR_CON_ON_SHIFT);
}

void timer2x_set_period(const struct timer2x_hw* self, uint8_t period){
    if((*self->CON & TMR_CON_ON_MASK) == 0){
        *self->PR = (uint8_t)period;
    }
}

void timer2x_clear(const struct timer2x_hw* self){
    if((*self->CON & TMR_CON_ON_MASK) == 0){
        *self->TMR ^= *self->TMR;
    }    
}

void timer2x_read_counter(const struct timer2x_hw* self, uint8_t* count){
    if((*self->CON & TMR_CON_ON_MASK) == 0){
        *count = *self->TMR;
    }
}

