#include "../inc/timer0_hw.h"
#include "../registers.h"
#include <assert.h>

struct timer0_hw{
    uintptr_t base;
    uint8_t mode_16bit;
    bool in_use;
};

void timer0_create(struct timer0_hw const** timer0_hw_inst_out, bool mode16bit){
    static struct timer0_hw hw;
    assert(timer0_hw_inst_out != 0 );
    if(!timer0_hw_inst_out){return;}
    hw.base = TMR0_BASE_ADDRESS;
    hw.mode_16bit = mode16bit;
    hw.in_use = true;
    *timer0_hw_inst_out = &hw;
}

void timer0_init(struct timer0_hw* const self, timer0_config_t* config){
    *REG8(self->base, TMR0_CON0_OFFSET) &= ~(TMR0_CON0_EN_MASK);
    *REG8(self->base, TMR0_CON0_OFFSET) |= (self->mode_16bit << TMR0_CON0_MD16_SHIFT) |
                    (config->prescaler  << TMR0_CON0_OUTPS_SHIFT);

    *REG8(self->base, TMR0_CON1_OFFSET) |= (config->clk_src << TMR0_CON1_CS_SHIFT)    |
                    (config->async << TMR0_CON1_ASYNC_SHIFT) |
                    (config->prescaler << TMR0_CON1_CKPS_SHIFT);
}

void timer0_enable(struct timer0_hw* const self){
    *REG8(self->base, TMR0_CON0_OFFSET) |= TMR0_CON0_EN_MASK;
}
void timer0_disable(struct timer0_hw* const self){
    *REG8(self->base, TMR0_CON0_OFFSET) &= ~(TMR0_CON0_EN_MASK);
}

void timer0_read_counter(struct timer0_hw* const self, uint16_t* count){
    if(self->mode_16bit){
        uint16_t tmp = *REG8(self->base, TMR0_TMRL_OFFSET) & 0xFF;     //get low byte
        tmp |= ((*REG8(self->base, TMR0_TMRH_OFFSET) << 8u) & 0xFF00);  //set high byte of count
        *count = *REG8(self->base, TMR0_TMRL_OFFSET);      
    }else {
        *count = (*REG8(self->base, TMR0_TMRH_OFFSET) >> 8u) & 0xFF;  // set high byte as low byte of count
    }
}

void timer0_set_counter(struct timer0_hw* const self, uint16_t value){
    if((*(REG8(self->base, TMR0_CON0_OFFSET)) & TMR0_CON0_EN_MASK) != 1){
        if(self->mode_16bit){
            *REG8(self->base, TMR0_TMRL_OFFSET) = (uint8_t)(value & 0xFF);        // set low byte
            *REG8(self->base, TMR0_TMRH_OFFSET) = (uint8_t)((value >> 8) & 0xFF); // set high byte
        }else{
            *REG8(self->base, TMR0_TMRH_OFFSET) = (uint8_t)value;  // only use low byte
        }
    }
}

void timer0_set_prescaler(struct timer0_hw* const self, uint8_t prescaler){
    prescaler = prescaler < 0xF ? prescaler : 0xF;
    *REG8(self->base, TMR0_CON1_OFFSET) &= ~(TMR0_CON1_CKSPS_MASK);
    *REG8(self->base, TMR0_CON1_OFFSET) |= (prescaler << TMR0_CON1_CKPS_SHIFT);
}