#include "../inc/timer0_hw.h"
#include "../registers.h"
#include <assert.h>

typedef struct timer0_hw{
    volatile uint8_t* TMR0L;
    volatile uint8_t* TMR0H;
    volatile uint8_t* TMR0_CON0;
    volatile uint8_t* TMR0_CON1;
    uint8_t mode_16bit;
} timer0_hw_t;

void create_timer0(const struct timer0_hw** timer0_hw_inst_out, bool mode16bit){
    static timer0_hw_t hw;
    assert(timer0_hw_inst_out != 0 );
    if(!timer0_hw_inst_out){return;}
    const uintptr_t base = TMR0_BASE_ADDRESS;
    hw.TMR0L     = (volatile uint8_t*)(base + TMR0_TMRL_OFFSET);
    hw.TMR0H     = (volatile uint8_t*)(base + TMR0_TMRH_OFFSET);
    hw.TMR0_CON0 = (volatile uint8_t*)(base + TMR0_CON0_OFFSET);
    hw.TMR0_CON1 = (volatile uint8_t*)(base + TMR0_CON1_OFFSET);
    hw.mode_16bit = mode16bit;
    *timer0_hw_inst_out = &hw;
}

void timer0_init(const struct timer0_hw* self, timer0_config_t* config){
    *self->TMR0_CON0 &= ~(TMR0_CON0_EN_MASK);
    *self->TMR0_CON0 |= (self->mode_16bit << TMR0_CON0_MD16_SHIFT) |
                    (config->prescaler  << TMR0_CON0_OUTPS_SHIFT);

    *self->TMR0_CON1 |= (config->clk_src << TMR0_CON1_CS_SHIFT)    |
                    (config->async << TMR0_CON1_ASYNC_SHIFT) |
                    (config->prescaler << TMR0_CON1_CKPS_SHIFT);
}

void timer0_enable(const struct timer0_hw* self){
    *self->TMR0_CON0 |= TMR0_CON0_EN_MASK;
}
void timer0_disable(const struct timer0_hw* self){
    *self->TMR0_CON0 &= ~(TMR0_CON0_EN_MASK);
}

void timer0_read_counter(const struct timer0_hw* self, uint16_t* count){
    if(!self->mode_16bit){
        uint16_t tmp = *self->TMR0L & 0xFF;     //get low byte
        tmp |= ((*self->TMR0H << 8) & 0xFF00);  //set high byte of count
        *count = *self->TMR0L;      
    }else {
        *count = (*self->TMR0H >> 8) & 0xFF;  // set high byte as low byte of count
    }
}

void timer0_set_counter(const struct timer0_hw* self, uint16_t value){
    if((*(self->TMR0_CON0) & TMR0_CON0_EN_MASK) != 1){
        if(!self->mode_16bit){
            *self->TMR0L = (uint8_t)(value & 0xFF);        // set low byte
            *self->TMR0H = (uint8_t)((value >> 8) & 0xFF); // set high byte
        }else{
            *self->TMR0H = (uint8_t)value;  // only use low byte
        }
    }
}