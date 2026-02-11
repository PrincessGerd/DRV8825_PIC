#include "../inc/timer1_hw.h"
#include "../registers.h"
struct timer1_hw {
    volatile uint8_t* TMR1_TMRL;
    volatile uint8_t* TMR1_TMRH;
    volatile uint8_t* TMR1_CON;
    volatile uint8_t* TMR1_GCON;
    volatile uint8_t* TMR1_GATE;
    volatile uint8_t* TMR1_CLK;
    bool mode_16bit;
};

void timer1_init(const struct timer1_hw* self, tmr1_config_t* config){
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


void timer1_enable(const struct timer1_hw* self){
    *self->TMR1_CON |=  TMR1_CON_ON_MASK;
}
void timer1_disable(const struct timer1_hw* self){
    *self->TMR1_CON &=  ~(TMR1_CON_ON_MASK);
}

void timer1_read_counter(const struct timer1_hw* self, uint16_t* count){
    if(self->mode_16bit){
        *count = *self->TMR1_TMRL;   
    }else{
        uint16_t tmp = *self->TMR1_TMRL & 0xFF;
        tmp |= ((*self->TMR1_TMRH << 8) & 0xFF00);
        *count = tmp;
    }
}
void timer1_set_counter(const struct timer1_hw* self, uint16_t value){
    if(self->mode_16bit){
        *self->TMR1_TMRL = value;
    } else{
        *self->TMR1_TMRL = (uint8_t)(value & 0xFF);
        *self->TMR1_TMRH = (uint8_t)(value >> 8);
    }
}
