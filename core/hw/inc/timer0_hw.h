#ifndef TIMER0_H
#define TIMER0_H

#include "../../../registers.h"
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
    uint8_t prescaler; // TODO: make a enum for this
    uint8_t postscaler; // TODO: make a enum for this
    bool async;
} timer0_config_t;

struct timer0_hw;
void create_timer0(const struct timer0_hw** timer0_hw_inst_out, bool mode16bit);
void timer0_init(const struct timer0_hw*  self, timer0_config_t* config);
void timer0_enable(const struct timer0_hw* self);
void timer0_disable(const struct timer0_hw* self);
void timer0_set_counter(const struct timer0_hw* self, uint16_t value);
void timer0_read_counter(const struct timer0_hw* self, uint16_t* count);
#endif