#include "../inc/pwm_hw.h"
#include <stdint.h>
#include <stdbool.h>
#include "../registers.h"
#include <xc.h>


struct pwm_hw {
    uint8_t module_num;   
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
    if (module_num >= 4 || pwm_hw_inst_out == 0) return;
    pwm_instances[module_num].module_num = module_num;
    const uintptr_t base =
        PWM_BASE_ADDR + (module_num * PWM_OFFSET_NEXT_MODULE);
    pwm_instances[module_num].CLK   = (volatile uint8_t*)(base + PWM_CLK_OFFSET);
    pwm_instances[module_num].LDS   = (volatile uint8_t*)(base + PWM_LDS_OFFSET);
    pwm_instances[module_num].CON   = (volatile uint8_t*)(base + PWM_CON_OFFSET);
    pwm_instances[module_num].CPRE  = (volatile uint8_t*)(base + PWM_CPRE_OFFSET);
    pwm_instances[module_num].PIPOS = (volatile uint8_t*)(base + PWM_PIPOS_OFFSET);
    pwm_instances[module_num].CFG   = (volatile uint8_t*)(base + PWM_CFG_OFFSET);
    pwm_instances[module_num].PR        = (volatile uint8_t*)(base + PWM_PR_OFFSET);
    pwm_instances[module_num].PWMSAP1   = (volatile uint8_t*)(base + PWM_SAP1_OFFSET);
    pwm_instances[module_num].PWMSAP2   = (volatile uint8_t*)(base + PWM_SAP2_OFFSET);
    *pwm_hw_inst_out = &pwm_instances[module_num];
}

void pwm_hw_init(const struct pwm_hw* self, pwm_hw_config_t* config){
   *self->CON &= ~(PWM_CON_EN_MASK);
   *self->CFG |= (config->mode << (uint8_t)PWM_CFG_MODE_SHIFT) |
                    (config->out1_polarity_low << PWM_CFG_POL1_SHIFT) |
                    (config->out1_polarity_low << PWM_CFG_POL2_SHIFT) |
                    (config->push_pull_mode << PWM_CFG_PPEN_SHIFT);
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

void pwm_set_period_common(const struct pwm_hw* self, uint16_t period){
    *self->PR = (uint8_t)(period & 0xFF);            // set low byte
    *(self->PR+1) = (uint8_t)((period >> 8) & 0xFF); // set high byte
}

void pwm_set_period_Px(const struct pwm_hw* self, pwmx_outputs_e px, uint16_t period){
    if(px == PWMx_OUTPUT_P1){
        *self->PWMSAP1 = (uint8_t)(period & 0xFF);            // set low byte
        *(self->PWMSAP1+1) = (uint8_t)((period >> 8) & 0xFF); // set high byte
    }else{
        *self->PWMSAP2 = (uint8_t)(period & 0xFF);            // set low byte
        *(self->PWMSAP2+1) = (uint8_t)((period >> 8) & 0xFF); // set high byte
    }
}