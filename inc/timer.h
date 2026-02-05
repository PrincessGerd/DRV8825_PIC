/* 
 * File:   timer.h
 * Author: Gard
 *
 * Created on January 20, 2026, 3:44 PM
 */

#ifndef TMR2H
#define	TMR2H

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "registers.h"

struct timer_device_ops;
struct timer_device { 
    const struct timer_device_ops *ops;
    void (*init) (const struct timer_device *dev);
};

typedef const struct timer_device* timer_device_t;
typedef void (*timer_callback_t)(timer_device_t dev);

struct timer_device_ops{
    void (*enable)(const timer_device_t dev);
    void (*disable)(const timer_device_t dev);
    void (*set_period)(const timer_device_t dev, uint16_t period);
    void (*set_callback)(const timer_device_t dev, timer_callback_t callback);
};
void timer_device_init(timer_device_t dev, const struct timer_device_ops *ops);

#define TIMER_REG_ADDR(module_num, offset) \
    ((volatile uint8_t*)(TMR_BASE_ADDRESS + (module_num * TMR_OFFSET_NEXT_MODULE) + (offset)))

extern struct timer_device_ops tmr2_ops_impl;
void timer2_device_init_impl(timer_device_t dev);

#define TMR2_DECLARE_DEVICE(name, module, cfg_ptr)  \
    static timer_regs_t name##_regs = { \
        .TMR = TIMER_REG_ADDR((module), TMR_TMR_OFFSET), \
        .PR = TIMER_REG_ADDR((module), TMR_PR_OFFSET), \
        .CON = TIMER_REG_ADDR((module), TMR_CON_OFFSET), \
        .HLT = TIMER_REG_ADDR((module), TMR_HLT_OFFSET), \
        .CLKCON = TIMER_REG_ADDR((module), TMR_CLKCON_OFFSET), \
        .RST = TIMER_REG_ADDR((module), TMR_RST_OFFSET) \
    }; \
    static struct _pic18f_tmr2x_hw name##_inst = { \
        .dev = { \
            .ops = &tmr2_ops_impl, \
            .init = timer2_device_init_impl \
        }, \
        .regs   = &name##_regs, \
        .config = (cfg_ptr), \
        .is_initialised = false \
    }; \
    static timer_device_t name = &name##_inst.dev;

#endif	/* TMR2H */

