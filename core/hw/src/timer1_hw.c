#include "../inc/timer1_hw.h"
#include "../registers.h"
#include <assert.h>

struct timer1_hw {
    volatile uint8_t* TMR1_TMRL;
    volatile uint8_t* TMR1_TMRH;
    volatile uint8_t* TMR1_CON;
    volatile uint8_t* TMR1_GCON;
    volatile uint8_t* TMR1_GATE;
    volatile uint8_t* TMR1_CLK;
    bool mode_16bit;
};

void timer1_create(struct timer1_hw const** timer1_hw_inst_out){
    static struct timer1_hw hw;
    assert(timer1_hw_inst_out != 0 );
    if(!timer1_hw_inst_out){return;}
    const uintptr_t base = TMR1_BASE_ADDRESS;
    hw.TMR1_TMRL     = (volatile uint8_t*)(base + TMR1_TMRL_OFFSET);
    hw.TMR1_TMRH     = (volatile uint8_t*)(base + TMR1_TMRH_OFFSET);
    hw.TMR1_CON      = (volatile uint8_t*)(base + TMR1_CON_OFFSET);
    hw.TMR1_GCON     = (volatile uint8_t*)(base + TMR1_CON_OFFSET);
    hw.TMR1_GATE     = (volatile uint8_t*)(base + TMR1_GATE_OFFSET);
    hw.TMR1_CLK      = (volatile uint8_t*)(base + TMR1_CLK_OFFSET);
    *timer1_hw_inst_out = &hw;
}

void timer1_init(struct timer1_hw* const self, tmr1_config_t* config){
    *self->TMR1_CON &=  ~(TMR1_CON_ON_MASK);
    *self->TMR1_CLK = config->clk_src;
    *self->TMR1_CON |= (config->prescaler << TMR1_CON_CKPS_SHIFT) |
                    (self->mode_16bit << TMR1_CON_RD16_SHIFT) |
                    (~(config->clk_sync << TMR1_CON_SYNC_MASK));
    if(config->gate_enable){
        *self->TMR1_GCON |= (1 << TMR1_GCON_GE_SHIFT) |
                        (config->gate.gate_polarity << TMR1_GCON_GPOL_SHIFT) |
                        (config->gate.toggle_mode_enable << TMR1_GCON_GTM_SHIFT) |
                        (config->gate.pulse_mode_enable << TMR1_GCON_GSPM_SHIFT);
        *self->TMR1_GATE  = config->gate.gate_source;
    }
}

void timer1_set_prescaler(struct timer1_hw* const self, uint8_t prescaler){
    prescaler = prescaler < 4 ? prescaler : 4;
    *self->TMR1_CON &= ~(TMR1_CON_CKPS_MASK);
    *self->TMR1_CON |= (prescaler << TMR1_CON_CKPS_SHIFT);
}

void timer1_enable(struct timer1_hw* const self){
    *self->TMR1_CON |=  TMR1_CON_ON_MASK;
}
void timer1_disable(struct timer1_hw* const self){
    *self->TMR1_CON &=  ~(TMR1_CON_ON_MASK);
}

void timer1_read_counter(struct timer1_hw* const self, uint16_t* count){
    if(self->mode_16bit){
        *count = (*((uint16_t*)self->TMR1_TMRL));   
    }else{
        uint16_t tmp = *self->TMR1_TMRL & 0xFF;
        tmp |= ((*self->TMR1_TMRH << 8U) & 0xFF00);
        *count = tmp;
    }
}
void timer1_set_counter(struct timer1_hw* const self, uint16_t value){
    if(self->mode_16bit){
        (*((uint16_t*)self->TMR1_TMRL)) = value;
    } else{
        *self->TMR1_TMRL = (uint8_t)(value & 0xFF);
        *self->TMR1_TMRH = (uint8_t)(value >> 8U);
    }
}
