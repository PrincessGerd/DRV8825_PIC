#include "../inc/timer2x_hw.h"
#include "../registers.h"
#include <stdint.h>
#include "../core/serial_logger.h"

struct timer2x_hw{
    uintptr_t base;
    bool in_use;
};

static struct timer2x_hw hw[2] = {0};

void create_timer2x(const struct timer2x_hw **timer2x_hw_inst_out, uint8_t module_num){
    hw[module_num].base = (TMR_BASE_ADDRESS + (module_num * TMR_NUM_MODULES));
    hw[module_num].in_use = true;
    *timer2x_hw_inst_out = &hw;
}

void timer2x_init(const struct timer2x_hw *self, timer_config_t* config){
    *REG8(self->base, TMR_CON_OFFSET)      &= ~(1 << TMR_CON_ON_SHIFT);
    *REG8(self->base, TMR_CLKCON_OFFSET)   |= TMR_CLKCON_CS_MASK & (config->clk_src << TMR_CLKCON_CS_SHIFT);
    *REG8(self->base, TMR_CON_OFFSET)      |= TMR_CON_CKPS_MASK  & (config->prescaler << TMR_CON_CKPS_SHIFT);
    *REG8(self->base, TMR_CON_OFFSET)      |= TMR_CON_OUTPS_MASK & (config->postscaler << TMR_CON_OUTPS_SHIFT);
    *REG8(self->base, TMR_HLT_OFFSET)      |= TMR_HLT_MODE_MASK  & (TMR2MODE(config->mode.mode,config->mode.submode.raw) << TMR_HLT_MODE_SHIFT);
    *REG8(self->base, TMR_HLT_OFFSET)      |= TMR_HLT_CSYNC_MASK & (config->clk_sync << TMR_HLT_CSYNC_SHIFT);
    *REG8(self->base, TMR_HLT_OFFSET)      |= TMR_HLT_PSYNC_MASK & (config->prescaler_sync << TMR_HLT_PSYNC_SHIFT);
    *REG8(self->base, TMR_RST_OFFSET)     |= TMR_RST_RSEL_MASK  & (config->reset_src << TMR_RST_RSEL_SHIFT);
}

void timer2x_enable(const struct timer2x_hw* self){
    *REG8(self->base, TMR_CON_OFFSET) |= (1 << TMR_CON_ON_SHIFT);
}

void timer2x_disable(const struct timer2x_hw* self) {
    *REG8(self->base, TMR_CON_OFFSET) &= ~(1 << TMR_CON_ON_SHIFT);
}

void timer2x_set_period(const struct timer2x_hw* self, uint8_t period){
    if((*REG8(self->base, TMR_CON_OFFSET) & TMR_CON_ON_MASK) == 0){
        *REG8(self->base, TMR_PR_OFFSET) = (uint8_t)period;
    }
}

void timer2x_clear(const struct timer2x_hw* self){
    if((*REG8(self->base, TMR_CON_OFFSET) & TMR_CON_ON_MASK) == 0){
        *REG8(self->base, TMR_TMR_OFFSET) ^= *REG8(self->base, TMR_TMR_OFFSET);
    }    
}

void timer2x_read_counter(const struct timer2x_hw* self, uint8_t* count){
    if((*REG8(self->base, TMR_CON_OFFSET) & TMR_CON_ON_MASK) == 0){
        *count = *REG8(self->base, TMR_TMR_OFFSET);
    }
}

void timer2x_set_prescaler(const struct timer2x_hw* self, timer_prescaler_e prescaler){
    *REG8(self->base, TMR_CON_OFFSET)  |= TMR_CON_CKPS_MASK  & (prescaler << TMR_CON_CKPS_SHIFT);
}