#include "../inc/pwm_hw.h"
#include <stdint.h>
#include <stdbool.h>
#include "../registers.h"

struct pwm_hw {
    uintptr_t base;
    bool in_use;
};


static struct pwm_hw pwm_instances[2]; 

void pwm_hw_create(uint8_t module_num, const struct pwm_hw ** pwm_hw_inst_out) {
    if (module_num >= 2 || pwm_hw_inst_out == 0) return;
    pwm_instances[module_num].base =
        PWM_BASE_ADDR + (module_num * PWM_OFFSET_NEXT_MODULE);
    pwm_instances[module_num].in_use = true;
    *pwm_hw_inst_out = &pwm_instances[module_num];
}

void pwm_hw_init(const struct pwm_hw* self, pwm_hw_config_t* config){
   //*REG8(self->base, PWM_CON_OFFSET) &= ~(PWM_CON_EN_MASK);
   *REG8(self->base, PWM_CON_OFFSET) = 0x0;
   *REG8(self->base, PWM_CLK_OFFSET) = config->clk_src;
   *REG8(self->base, PWM_CFG_OFFSET) |= (config->mode << (uint8_t)PWM_CFG_MODE_SHIFT) |
                    (config->out1_polarity_low << PWM_CFG_POL1_SHIFT) |
                    (config->out2_polarity_low << PWM_CFG_POL2_SHIFT) |
                    (config->push_pull_mode << PWM_CFG_PPEN_SHIFT);
    *REG8(self->base, PWM_LDS_OFFSET) = 0x0;
    *REG8(self->base, PWM_ERS_OFFSET) = 0x0;
}

void pwm_hw_masked_enable(uint8_t mask){
    *((volatile uint8_t*)(PWM_BASE_ADDR + PWM_EN_OFFSET)) = mask;
}

void pwm_hw_set_lds(const struct pwm_hw* self, uint8_t lds){
    *REG8(self->base, PWM_LDS_OFFSET) = lds;
}

void pwm_hw_clock_prescaler(const struct pwm_hw* self, uint8_t prescaler){
    prescaler = prescaler < 0xFF ? prescaler : 0xFE;
    *REG8(self->base, PWM_CPRE_OFFSET) = prescaler;
}

void pwm_hw_enable(const struct pwm_hw* self){
   *REG8(self->base, PWM_CON_OFFSET) |= PWM_CON_EN_MASK;
}
void pwm_hw_disable(const struct pwm_hw* self){
   *REG8(self->base, PWM_CON_OFFSET) &= ~(PWM_CON_EN_MASK);
}

void pwm_hw_enable_buffered(const struct pwm_hw* self){
   *REG8(self->base, PWM_CON_OFFSET) |= PWM_CON_LD_MASK;
}

void pwm_hw_disable_buffered(const struct pwm_hw* self){
   *REG8(self->base, PWM_CON_OFFSET) &= ~(PWM_CON_LD_MASK);
}

void pwm_set_period_common(const struct pwm_hw* self, uint16_t period){
    *REG8(self->base, PWM_PRL_OFFSET) = (uint8_t)period;            // set low byte
    *REG8(self->base, PWM_PRH_OFFSET) = (uint8_t)(period >> 8); // set high byte
}

void pwm_set_period_Px(const struct pwm_hw* self, pwmx_outputs_e px, uint16_t period){
    if(px == PWMx_OUTPUT_P1){
        *REG8(self->base, PWM_SAP1_OFFSET) = (uint8_t)period;            // set low byte
        *REG8(self->base, PWM_SAP1_OFFSET + 1) = (uint8_t)(period >> 8); // set high byte
    }else{
        *REG8(self->base, PWM_SAP2_OFFSET) = (uint8_t)period;            // set low byte
        *REG8(self->base, PWM_SAP2_OFFSET + 1) = (uint8_t)(period >> 8); // set high byte
    }
}