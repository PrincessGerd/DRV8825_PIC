#include "../inc/pwm_hw.h"
#include <stdint.h>
#include <stdbool.h>
#include "../registers.h"
#include <xc.h>


struct pwm_hw {
    uint8_t module_num;   
    volatile uint8_t* ERS;
    volatile uint8_t* CLK;
    volatile uint8_t* LDS;
    volatile uint8_t* CON;
    volatile uint8_t* CPRE;
    volatile uint8_t* PIPOS;
    volatile uint8_t* CFG;
    volatile uint8_t* PR;
    volatile uint8_t* PWMSAP1;
    volatile uint8_t* PWMSAP2;
};


static struct pwm_hw pwm_instances[2];  
void pwm_hw_create(uint8_t module_num, const struct pwm_hw ** pwm_hw_inst_out) {
    if (module_num >= 2 || pwm_hw_inst_out == 0) return;
    pwm_instances[module_num].module_num = module_num;
    const uintptr_t base =
        PWM_BASE_ADDR + (module_num * PWM_OFFSET_NEXT_MODULE);
    pwm_instances[module_num].ERS   = (volatile uint8_t*)(base + PWM_ERS_OFFSET);
    pwm_instances[module_num].CLK   = (volatile uint8_t*)(base + PWM_CLK_OFFSET);
    pwm_instances[module_num].LDS   = (volatile uint8_t*)(base + PWM_LDS_OFFSET);
    pwm_instances[module_num].CON   = (volatile uint8_t*)(base + PWM_CON_OFFSET);
    pwm_instances[module_num].CPRE  = (volatile uint8_t*)(base + PWM_CPRE_OFFSET);
    pwm_instances[module_num].PIPOS = (volatile uint8_t*)(base + PWM_PIPOS_OFFSET);
    pwm_instances[module_num].CFG   = (volatile uint8_t*)(base + PWM_CFG_OFFSET);
    pwm_instances[module_num].PR        = (volatile uint8_t*)(base + PWM_PRL_OFFSET);
    pwm_instances[module_num].PWMSAP1   = (volatile uint8_t*)(base + PWM_SAP1_OFFSET);
    pwm_instances[module_num].PWMSAP2   = (volatile uint8_t*)(base + PWM_SAP2_OFFSET);
    *pwm_hw_inst_out = &pwm_instances[module_num];
}

void pwm_hw_init(const struct pwm_hw* self, pwm_hw_config_t* config){
   //*self->CON &= ~(PWM_CON_EN_MASK);
   *self->CON = 0x0;
   *self->CLK = config->clk_src;
   *self->CFG |= (config->mode << (uint8_t)PWM_CFG_MODE_SHIFT) |
                    (config->out1_polarity_low << PWM_CFG_POL1_SHIFT) |
                    (config->out2_polarity_low << PWM_CFG_POL2_SHIFT) |
                    (config->push_pull_mode << PWM_CFG_PPEN_SHIFT);
    *self->LDS = 0x0;
    PWM1ERS = 0x0;
}

void pwm_hw_clock_prescaler(const struct pwm_hw* self, uint8_t prescaler){
    prescaler = prescaler < 0xFF ? prescaler : 0xFE;
    *self->CPRE = prescaler;
}

void pwm_hw_enable(const struct pwm_hw* self){
   *self->CON |= PWM_CON_EN_MASK;
}
void pwm_hw_disable(const struct pwm_hw* self){
   *self->CON &= ~(PWM_CON_EN_MASK);
}

void pwm_hw_enable_reload(const struct pwm_hw* self){
   *self->CON |= PWM_CON_LD_MASK;
}

void pwm_hw_disable_reload(const struct pwm_hw* self){
   *self->CON &= ~(PWM_CON_LD_MASK);
}

uint16_t pwm_hw_calculate_duty(uint16_t period, uint8_t percent) {
    uint32_t temp = (uint32_t)(period + 1) * percent;
    temp /= 100;
    if (temp > period)
        temp = period;
    return (uint16_t)temp;
}

void pwm_set_period_common(const struct pwm_hw* self, uint16_t period){
    *self->PR = (uint8_t)period;            // set low byte
    *(self->PR+1) = (uint8_t)(period >> 8); // set high byte
}

void pwm_set_period_Px(const struct pwm_hw* self, pwmx_outputs_e px, uint16_t period){
    if(px == PWMx_OUTPUT_P1){
        *self->PWMSAP1 = (uint8_t)period;            // set low byte
        *(self->PWMSAP1+1) = (uint8_t)(period >> 8); // set high byte
    }else{
        *self->PWMSAP2 = (uint8_t)period;            // set low byte
        *(self->PWMSAP2+1) = (uint8_t)(period >> 8); // set high byte
    }
}