#include "../timer.h"

#include <stddef.h>

#define TMR_BASE_ADDRESS 0x0119
#define TMR_OFFSET_NEXT_MODULE 0x6
#define TMR_NUM_MODULES 0x2

#define TMR_TMR_OFFSET        0x0
#define TMR_PR_OFFSET         0x1
#define TMR_CON_OFFSET        0x2
#define TMR_HLT_OFFSET        0x3
#define TMR_CLKCON_OFFSET     0x4
#define TMR_RST_OFFSET        0x5

#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))


#define TIMER_REG_ADDR(module_num, offset) \
    ((volatile uint8_t*)(TMR_BASE_ADDRESS + (module_num * TMR_OFFSET_NEXT_MODULE) + (offset)))

typedef struct {
    volatile uint8_t* TMR;
    volatile uint8_t* PR;
    volatile uint8_t* CON;
    volatile uint8_t* HLT;
    volatile uint8_t* CLKCON;
    volatile uint8_t* RST;
} timer_regs_t;

// should be converted to zephyr-like style with data and config. but it works so leave it
typedef struct _pic18f_tmr2x_hw {
    const struct timer_device*  dev;
    timer_regs_t*         regs;
    timer_config_t*      config;
    void (*callback)(const struct timer_device *dev);
} pic18f_tmr2x_hw_t;


/*
    INTERFACE FUNCTIONS
*/
void timer_enable(const timer_device_t dev)  {
    dev->ops->enable(dev); 
}

void timer_disable(const timer_device_t dev) {
    dev->ops->disable(dev);
}

void set_period(const timer_device_t dev, uint16_t period){
    dev->ops->set_period(dev,period);
}

void timer_set_callback(const timer_device_t dev, timer_callback_t callback){
    if(callback != 0){
        dev->ops->set_callback(dev,callback);
    }
}

/*
    TIMER2 and 4 specific functions
*/
void timer2_device_init_impl(device_t _dev){
    timer_device_t dev = (timer_device_t)_dev;
    pic18f_tmr2x_hw_t* self = container_of(dev, struct _pic18f_tmr2x_hw, dev);
    self->regs   = (timer_regs_t*)dev->config;
    self->config = (timer_config_t*)dev->data;

    const timer_regs_t* regs        = self->regs;
    const timer_config_t* config    = self->config;
    *regs->CON      &= ~(1 << TMR_CON_ON_SHIFT);
    *regs->CLKCON   |= TMR_CLKCON_CS_MASK & (config->clk_src << TMR_CLKCON_CS_SHIFT);
    *regs->CON      |= TMR_CON_CKPS_MASK  & (config->prescaler << TMR_CON_CKPS_SHIFT);
    *regs->CON      |= TMR_CON_OUTPS_MASK & (config->postscaler << TMR_CON_OUTPS_SHIFT);
    *regs->HLT      |= TMR_HLT_MODE_MASK  & (TMR2MODE(config->mode.mode,config->mode.submode.raw) << TMR_HLT_MODE_SHIFT);
    *regs->HLT      |= TMR_HLT_CSYNC_MASK & (config->clk_sync << TMR_HLT_CSYNC_SHIFT);
    *regs->HLT      |= TMR_HLT_PSYNC_MASK & (config->prescaler_sync << TMR_HLT_PSYNC_SHIFT);
    *regs->RST      |= TMR_RST_RSEL_MASK  & (config->reset_src << TMR_RST_RSEL_SHIFT);
    //self->is_initialised = true;
}

void timer2_enable_impl(const timer_device_t dev){
    pic18f_tmr2x_hw_t* self = container_of(dev, struct _pic18f_tmr2x_hw, dev);
    const timer_regs_t* regs = self->regs;
    *regs->CON |= (1 << TMR_CON_ON_SHIFT);
}

void timer2_disable_impl(const timer_device_t dev) {
    pic18f_tmr2x_hw_t* self = container_of(dev, struct _pic18f_tmr2x_hw, dev);
    const timer_regs_t* regs = self->regs;
    *regs->CON &= ~(1 << TMR_CON_ON_SHIFT);
}

void timer2_set_period_impl(const timer_device_t dev, uint16_t count){
    pic18f_tmr2x_hw_t* self = container_of(dev, struct _pic18f_tmr2x_hw, dev);
    const timer_regs_t* regs = self->regs;
    if((*regs->CON & TMR_CON_ON_MASK ) == 0){
        *regs->PR = (uint8_t)count;
    }
}

void timer2_set_callback_impl(const timer_device_t dev, timer_callback_t callback){
    pic18f_tmr2x_hw_t* self = container_of(dev, struct _pic18f_tmr2x_hw, dev);
    self->callback = callback;
}

static const timer_regs_t timer2_regs = {
    .TMR    = TIMER_REG_ADDR(0, TMR_TMR_OFFSET),
    .PR     = TIMER_REG_ADDR(0, TMR_PR_OFFSET),
    .CON    = TIMER_REG_ADDR(0, TMR_CON_OFFSET),
    .HLT    = TIMER_REG_ADDR(0, TMR_HLT_OFFSET),
    .CLKCON = TIMER_REG_ADDR(0, TMR_CLKCON_OFFSET),
    .RST    = TIMER_REG_ADDR(0, TMR_RST_OFFSET)
};

static const timer_regs_t timer4_regs = {
    .TMR    = TIMER_REG_ADDR(1, TMR_TMR_OFFSET),
    .PR     = TIMER_REG_ADDR(1, TMR_PR_OFFSET),
    .CON    = TIMER_REG_ADDR(1, TMR_CON_OFFSET),
    .HLT    = TIMER_REG_ADDR(1, TMR_HLT_OFFSET),
    .CLKCON = TIMER_REG_ADDR(1, TMR_CLKCON_OFFSET),
    .RST    = TIMER_REG_ADDR(1, TMR_RST_OFFSET)
};

timer_config_t timer2_config = {
    .mode = { 
        .mode = TMR2_MODE_FREE_RUNNING, 
        .submode.free_running = TMR2_FR_SW_GATE
    },
    .clk_src = TMR2_CLK_MFINTOSC_500,
    //.reset_src = TMR2_RST_CCP2_OUT,
    .polarity = TMR2_RISING_EDGE, 
    .prescaler = TMR2_PRESCALER_2, 
    .postscaler = TMR2_POSTSCALER_1,
    .clk_sync = true, 
    .prescaler_sync = true
};
timer_config_t timer4_config = {0};

struct timer_device_ops tmr2_ops_impl = {
    .enable = &timer2_enable_impl,
    .disable = &timer2_disable_impl,
    .set_period = &timer2_set_period_impl,
    .set_callback = &timer2_set_callback_impl
};

struct timer_device TIMER_2 = {
    .ops    = &tmr2_ops_impl,
    .init   = timer2_device_init_impl,
    .config = &timer2_regs,
    .data   = &timer2_config
};
struct timer_device TIMER_4 = {
    .ops    = &tmr2_ops_impl,
    .init   = timer2_device_init_impl,
    .config = &timer4_regs,
    .data   = &timer4_config
};