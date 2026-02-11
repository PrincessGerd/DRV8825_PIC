#ifndef TIMER2_HW_H
#define TIMER2_HW_H
#include <stdint.h>
#include <stdbool.h>
#include "../../../registers.h"

typedef enum {
    TMR2_MODE_FREE_RUNNING = 0b00,
    TMR2_MODE_ONE_SHOT     = 0b01,
    TMR2_MODE_MONOSTABLE   = 0b10
} timer_mode_e;

// For FREE_RUNNING mode:
typedef enum {
    TMR2_FR_SW_GATE             = 0b000,
    TMR2_FR_HW_GATE_HIGH        = 0b001,
    TMR2_FR_HW_GATE_LOW         = 0b010,
    TMR2_FR_EDGE_RESET_ANY      = 0b011,
    TMR2_FR_EDGE_RESET_RISING   = 0b100,
    TMR2_FR_EDGE_RESET_FALLING  = 0b101,
    TMR2_FR_LEVEL_RESET_LOW     = 0b110,
    TMR2_FR_LEVEL_RESET_HIGH    = 0b111
} timer_free_running_submode_e;

// For ONE_SHOT mode:
typedef enum {
    TMR2_OS_SOFTWARE_START          = 0b000,
    TMR2_OS_RISING_EDGE_START       = 0b001,
    TMR2_OS_FALLING_EDGE_START      = 0b010,
    TMR2_OS_ANY_EDGE_START          = 0b011,
    TMR2_OS_EDGE_START_RST_RISING   = 0b100,
    TMR2_OS_EDGE_START_RST_FALLING  = 0b101,
    TMR2_OS_EDGE_START_LOW_RST      = 0b110,
    TMR2_OS_EDGE_START_HIGH_RST     = 0b111
} timer_one_shot_submode_e;

// For MONOSTABLE mode:
typedef enum {
    TMR2_MS_SOFTWARE_START          = 0b000,
    TMR2_MS_RISING_EDGE_START       = 0b001,
    TMR2_MS_FALLING_EDGE_START      = 0b010,
    TMR2_MS_ANY_EDGE_START          = 0b011,
    // Only valid for monostable
    TMR2_MS_HIGH_START_LOW_RST      = 0b110,
    TMR2_MS_LOW_START_HIGH_RST      = 0b111
} timer_monostable_submode_e;

typedef enum {
    TMR2_CLK_TMR2OSC_CLC4_OUT       = 0xD,
    TMR2_CLK_CLC3_OUT               = 0xC,
    TMR2_CLK_CLC2_OUT               = 0xB,
    TMR2_CLK_CLC1_OUT               = 0xA,
    TMR2_CLK_CLKREF_OUT             = 0x9,
    TMR2_CLK_EXTOSC                 = 0x8,
    TMR2_CLK_SOSC                   = 0x7,
    TMR2_CLK_MFINTOSC_32            = 0x6,
    TMR2_CLK_MFINTOSC_500           = 0x5,
    TMR2_CLK_LFINTOSC               = 0x4,
    TMR2_CLK_HFINTOSC               = 0x3,
    TMR2_CLK_FOSC                   = 0x2,
    TMR2_CLK_FOSC4                  = 0x1,
    TMR2_CLK_PPS_SEL_OSC            = 0x00
}timer_clk_src_e;

typedef enum {
    TMR2_RST_U2TX_EDGE           = 0x10,
    TMR2_RST_U2RX_EDGE           = 0x0F,
    TMR2_RST_U1TX_EDGE           = 0x0E,
    TMR2_RST_U1RX_EDGE           = 0x0D,
    TMR2_RST_CLC4_OUT            = 0x0C,
    TMR2_RST_CLC3_OUT            = 0x0B,
    TMR2_RST_CLC2_OUT            = 0x0A,
    TMR2_RST_CLC1_OUT            = 0x09,
    TMR2_RST_PWM2S1P2_OUT        = 0x08,
    TMR2_RST_PWM2S1P1_OUT        = 0x07,
    TMR2_RST_PWM1S1P2_OUT        = 0x06,
    TMR2_RST_PWM1S1P1_OUT        = 0x05,
    TMR2_RST_CCP2_OUT            = 0x04,
    TMR2_RST_CCP1_OUT            = 0x03,
    TMR2_RST_TMR4_POSTSCALER_OUT = 0x02,
    TMR2_RST_TMR2_POSTSCALER_OUT = 0x01,
    TMR2_RST_PPS_SEL             = 0x00
} timer_reset_src_e;

typedef enum {
    TMR2_PRESCALER_128   = 0x7,
    TMR2_PRESCALER_64    = 0x6,
    TMR2_PRESCALER_32    = 0x5,
    TMR2_PRESCALER_16    = 0x4,
    TMR2_PRESCALER_8     = 0x3,
    TMR2_PRESCALER_4     = 0x2,
    TMR2_PRESCALER_2     = 0x1,
    TMR2_PRESCALER_1     = 0x0
} timer_prescaler_e;

typedef enum {
    TMR2_POSTSCALER_16 = 0xF,
    TMR2_POSTSCALER_15 = 0xE,
    TMR2_POSTSCALER_14 = 0xD,
    TMR2_POSTSCALER_13 = 0xC,
    TMR2_POSTSCALER_12 = 0xB,
    TMR2_POSTSCALER_11 = 0xA,
    TMR2_POSTSCALER_10 = 0x9,
    TMR2_POSTSCALER_9  = 0x8,
    TMR2_POSTSCALER_8  = 0x7,
    TMR2_POSTSCALER_7  = 0x6,
    TMR2_POSTSCALER_6  = 0x5,
    TMR2_POSTSCALER_5  = 0x4,
    TMR2_POSTSCALER_4  = 0x3,
    TMR2_POSTSCALER_3  = 0x2,
    TMR2_POSTSCALER_2  = 0x1,
    TMR2_POSTSCALER_1  = 0x0,           
} timer_postscaler_e;

typedef enum {
    TMR2_FALLING_EDGE = 0,
    TMR2_RISING_EDGE  = 1
} timer_polarity_e;

/*---------------------------------------------------------------------------------------*/
/*TIMER 2&4 CONFIG                                                                       */
/*---------------------------------------------------------------------------------------*/
#define TMR2MODE(group, sub) (((uint8_t)group << 3) | (uint8_t)sub)
typedef struct {
    timer_mode_e mode;
    union { // One submode per timer config
        timer_free_running_submode_e    free_running;
        timer_one_shot_submode_e        one_shot;
        timer_monostable_submode_e      monostable;
        uint8_t raw;
    } submode;
} timer_mode_t;

typedef struct {
    timer_mode_t        mode;
    timer_clk_src_e     clk_src;
    timer_reset_src_e   reset_src;
    timer_polarity_e    polarity; 
    timer_prescaler_e   prescaler;
    timer_postscaler_e  postscaler;
    bool clk_sync;
    bool prescaler_sync;
} timer_config_t;

/*---------------------------------------------------------------------------------------*/
/*TIMER 2 and 4 DEFINITION                                                               */
/*---------------------------------------------------------------------------------------*/
struct timer2x_hw;
typedef void (*timer2_callback_t)(const struct timer2x_hw* self);

void create_timer2x(const struct timer2x_hw **timer2x_hw_inst_out, uint8_t module_num);
void timer2x_init(const struct timer2x_hw* self, timer_config_t* config);
void timer2x_enable(const struct timer2x_hw* self);
void timer2x_disable(const struct timer2x_hw* self);
void timer2x_clear(const struct timer2x_hw* self);
void timer2x_set_period(const struct timer2x_hw* self, uint8_t period);
void timer2x_read_counter(const struct timer2x_hw* self, uint8_t* count);

#endif