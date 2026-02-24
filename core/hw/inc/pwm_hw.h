#ifndef PWM_HW_H
#define PWM_HW_H
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    PWMx_CLK_CLC4_OUT     = 0xC,
    PWMx_CLK_CLC3_OUT     = 0xB,
    PWMx_CLK_CLC2_OUT     = 0xA,
    PWMx_CLK_CLC1_OUT     = 0x9,
    PWMx_CLK_CLKREF_OUT   = 0x8,
    PWMx_CLK_EXTOSC       = 0x7,
    PWMx_CLK_SOSC         = 0x6,
    PWMx_CLK_MFINTOSC_32  = 0x5,
    PWMx_CLK_MFINTOSC_500 = 0x4,
    PWMx_CLK_LFINTOSC     = 0x3,
    PWMx_CLK_HFINTOSC     = 0x2,
    PWMx_CLK_FOSC         = 0x1,
    PWMx_CLK_PPS1_SEL_OSC  = 0x1,
    PWMx_CLK_PPS0_SEL_OSC  = 0x0
}pwmx_clk_src_e;

typedef enum {
    PWMx_ATOLOAD_DISABLE = 0x0,
    PWMx_AUTOLOAD_PPS0 = 0x1,
    PWMx_AUTOLOAD_PPS1 = 0x2,
    PWMx_AUTOLOAD_CLC1_OUT = 0x3,
    PWMx_AUTOLOAD_CLC2_OUT = 0x4,
    PWMx_AUTOLOAD_CLC3_OUT = 0x5,
    PWMx_AUTOLOAD_CLC4_OUT = 0x6,
    PWMx_AUTOLOAD_DMA1 = 0x7,
    PWMx_AUTOLOAD_DMA2 = 0x8,
    PWMx_AUTOLOAD_DMA3 = 0x9,
    PWMx_AUTOLOAD_DMA4 = 0xA
}pwmx_autoload_e;

typedef enum {
    PWMx_MODE_COMPARE_TOGGLE    =0x5,
    PWMx_MODE_COMPARE_SET       =0x4,
    PWMx_MODE_VARIABLE_ALIGN    =0x3,
    PWMx_MODE_CENTER_ALIGN      =0x2,
    PWMx_MODE_RIGHT_ALIGN       =0x1,
    PWMx_MODE_LEFT_ALIGN        =0x0
} pwmx_mode_e;

typedef enum{
    PWMx_OUTPUT_P1 = 0,
    PWMx_OUTPUT_P2
} pwmx_outputs_e;

typedef struct {
    pwmx_clk_src_e  clk_src;
    pwmx_autoload_e autoload;
    pwmx_mode_e     mode;
    bool out1_polarity_low;
    bool out2_polarity_low;
    bool push_pull_mode;
} pwm_hw_config_t;


struct pwm_hw;
void pwm_hw_create(uint8_t module_num, const struct pwm_hw ** pwm_hw_inst_out);
void pwm_hw_init(const struct pwm_hw* self, pwm_hw_config_t* config);
void pwm_hw_clock_prescaler(const struct pwm_hw* self, uint8_t prescaler);
void pwm_hw_enable(const struct pwm_hw* self);
void pwm_hw_disable(const struct pwm_hw* self);
void pwm_hw_enable_reload(const struct pwm_hw* self);
void pwm_hw_disable_reload(const struct pwm_hw* self);
void pwm_set_period_common(const struct pwm_hw* self, uint16_t period);
void pwm_set_period_Px(const struct pwm_hw* self, pwmx_outputs_e px, uint16_t period);
#endif