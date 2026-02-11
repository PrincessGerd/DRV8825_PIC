/* 
 * File:   tmr1.h
 * Author: Gard
 *
 * Created on 22 January 2026, 17:24
 */

#ifndef TIMER1_HW_H
#define	TIMER1_HW_H
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    TMR1_PRESCALER_8     = 0x3,
    TMR1_PRESCALER_4     = 0x2,
    TMR1_PRESCALER_2     = 0x1,
    TMR1_PRESCALER_1     = 0x0
} tmr1_prescaler_e;

typedef enum {
    TMR1_CLK_CLC4_OUT     = 0xF,
    TMR1_CLK_CLC3_OUT     = 0xE,
    TMR1_CLK_CLC2_OUT     = 0xD,
    TMR1_CLK_CLC1_OUT     = 0xC,
    TMR1_CLK_TMR0_OUT     = 0xB,
    TMR1_CLK_CLKREF_OUT   = 0xA,
    TMR1_CLK_EXTOSC       = 0x9,
    TMR1_CLK_SOSC         = 0x8,
    TMR1_CLK_MFINTOSC_32  = 0x7,
    TMR1_CLK_MFINTOSC_500 = 0x6,
    TMR1_CLK_SFINTOSC     = 0x5,
    TMR1_CLK_LFINTOSC     = 0x4,
    TMR1_CLK_HFINTOSC     = 0x3,
    TMR1_CLK_FOSC         = 0x2,
    TMR1_CLK_FOSC4        = 0x1,
    TMR1_CLK_PPS_SEL_OSC  = 0x0
}tmr1_clk_src_e;

typedef enum {
    TMR1_GATE_CLC4_OUT            = 0xD,
    TMR1_GATE_CLC3_OUT            = 0xC,
    TMR1_GATE_CLC2_OUT            = 0xB,
    TMR1_GATE_CLC1_OUT            = 0xA,
    TMR1_GATE_PWM2S1P2_OUT        = 0x9,
    TMR1_GATE_PWM2S1P1_OUT        = 0x8,
    TMR1_GATE_PWM1S1P2_OUT        = 0x7,
    TMR1_GATE_PWM1S1P1_OUT        = 0x6,
    TMR1_GATE_CCP2_OUT            = 0x5,
    TMR1_GATE_CCP1_OUT            = 0x4,
    TMR1_GATE_TMR4_Postscaler_OUT = 0x3,
    TMR1_GATE_TMR2_Postscaler_OUT = 0x2,
    TMR1_GATE_TMR0_OUT            = 0x1,
    TMR1_GATE_T1GPPS             = 0x0
}tmr1_gate_src_e;

typedef enum {
    TMR1_FALLING_EDGE = 0,
    TMR1_RISING_EDGE  = 1
} tmr1_polarity_e;

typedef struct {
    tmr1_clk_src_e clk_src;
    tmr1_prescaler_e prescaler;
    struct {    // if not set then timer is continiously counting
        tmr1_gate_src_e gate_source;
        tmr1_polarity_e gate_polarity;
        bool toggle_mode_enable;
        bool pulse_mode_enable;
    }gate;
    bool gate_enable;
    bool clk_sync;
} tmr1_config_t;


struct timer1_hw;
void timer1_init(const struct timer1_hw* self, tmr1_config_t* config);
void timer1_enable(const struct timer1_hw* self);
void timer1_disable(const struct timer1_hw* self);
void timer1_read_counter(const struct timer1_hw* self, uint16_t* count);
void timer1_set_counter(const struct timer1_hw* self, uint16_t value);

#endif	/* TIMER16BIT_H */

