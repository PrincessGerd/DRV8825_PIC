#include "../inc/timer1_hw.h"
#include "../registers.h"
#include <assert.h>

struct timer1_hw {
    uintptr_t base;
    bool in_use;
};

void timer1_create(struct timer1_hw const** timer1_hw_inst_out){
    static struct timer1_hw hw;
    assert(timer1_hw_inst_out != 0 );
    if(!timer1_hw_inst_out){return;}
    hw.base = TMR1_BASE_ADDRESS;
    hw.in_use = true;
    *timer1_hw_inst_out = &hw;
}

void timer1_init(struct timer1_hw* const self, tmr1_config_t* config){
    *REG8(self->base, TMR1_CON_OFFSET) &=  ~(TMR1_CON_ON_MASK);
    *REG8(self->base, TMR1_CLK_OFFSET) = config->clk_src;
    *REG8(self->base, TMR1_CON_OFFSET) |= (config->prescaler << TMR1_CON_CKPS_SHIFT) |
                    (config->mode_16bit << TMR1_CON_RD16_SHIFT) |
                    (~(config->clk_sync << TMR1_CON_SYNC_MASK));
    if(config->gate_enable){
        *REG8(self->base, TMR1_GCON_OFFSET) |= (1 << TMR1_GCON_GE_SHIFT) |
                        (config->gate.gate_polarity << TMR1_GCON_GPOL_SHIFT) |
                        (config->gate.toggle_mode_enable << TMR1_GCON_GTM_SHIFT) |
                        (config->gate.pulse_mode_enable << TMR1_GCON_GSPM_SHIFT);
        *REG8(self->base, TMR1_GATE_OFFSET)  = config->gate.gate_source;
    }
}

void timer1_set_prescaler(struct timer1_hw* const self, uint8_t prescaler){
    prescaler = prescaler < 4 ? prescaler : 4;
    *REG8(self->base, TMR1_CON_OFFSET) &= ~(TMR1_CON_CKPS_MASK);
    *REG8(self->base, TMR1_CON_OFFSET) |= (prescaler << TMR1_CON_CKPS_SHIFT);
}

void timer1_enable(struct timer1_hw* const self){
    *REG8(self->base, TMR1_CON_OFFSET) |=  TMR1_CON_ON_MASK;
}
void timer1_disable(struct timer1_hw* const self){
    *REG8(self->base, TMR1_CON_OFFSET) &=  ~(TMR1_CON_ON_MASK);
}

void timer1_read_counter(struct timer1_hw* const self, uint16_t* count){
    uint16_t tmp = *REG8(self->base, TMR1_TMRL_OFFSET) & 0xFF;
    tmp |= ((*REG8(self->base, TMR1_TMRH_OFFSET) << 8U) & 0xFF00);
    *count = tmp;
}
void timer1_set_counter(struct timer1_hw* const self, uint16_t value){
    *REG8(self->base, TMR1_TMRL_OFFSET) = (uint8_t)(value & 0xFF);
    *REG8(self->base, TMR1_TMRH_OFFSET) = (uint8_t)(value >> 8U);
}
