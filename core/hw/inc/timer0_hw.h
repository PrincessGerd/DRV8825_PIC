#ifndef TIMER0_H
#define TIMER0_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    TMR0_CLK_PPS            = 0x0,
    TMR0_CLK_PPS_INV        = 0x1,
    TMR0_CLK_FOSC4          = 0x2,
    TMR0_CLK_HFINTOSC       = 0x3,
    TMR0_CLK_LFINTOSC       = 0x4,
    TMR0_CLK_MFINTOSC_500   = 0x5,
    TMR0_CLK_SOSC           = 0x6,
    TMR0_CLK_CLC1_OUT       = 0x7
} tmr0_clk_src_e;

typedef struct {
    tmr0_clk_src_e clk_src;
    uint8_t prescaler;  // 1 -> 32768, exponential
    uint8_t postscaler; // 1 -> 16,    linear
    bool async;
} timer0_config_t;

struct timer0_hw;
void timer0_create(struct timer0_hw const** timer0_hw_inst_out, bool mode16bit);
void timer0_init(struct timer0_hw* const self, timer0_config_t* config);
void timer0_enable(struct timer0_hw* const self);
void timer0_disable(struct timer0_hw* const self);
void timer0_set_counter(struct timer0_hw* const self, uint16_t value);
void timer0_read_counter(struct timer0_hw* const self, uint16_t* count);
void timer0_set_prescaler(struct timer0_hw* const self, uint8_t prescaler);
#endif