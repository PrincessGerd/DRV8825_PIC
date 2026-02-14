#include "stepper.h"
// hardware
#include "../core/hw/inc/timer0_hw.h"
#include "../core/hw/inc/timer2x_hw.h"
#include "../core/hw/inc/clcx_hw.h"
// core
#include "../core/irq_device.h"
#include "../core/irq_manager.h"
#include "../core/types.h"
#include "../core/pinmux.h"
#include "../core/gpio.h"
// utils
#include "../inc/fixed_point.h"
//system
#include <xc.h>
#define STEPPER_OSC_FREQUENCY 250000    // 500khz
// simple delay function
// using the avrage clock cycles per instruction of 4 as base
#define INSTR_FREQ (_XTAL_FREQ/4)
void delay_us(uint16_t us){
    uint32_t cycles = (INSTR_FREQ / 1000000);
    while(cycles--){
        NOP();
    }
}

#define DRV8825_WAKEUP_TIME_US 1
#define DRV8825_PULSE_DURATION_US 2

#define clamp(val, max) ((val > max) ? max : val)
#define mod(a,b) (a & (b-1))

static timer_config_t period_counter_config = {
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
 
static timer0_config_t step_counter_config = {
    .clk_src = TMR0_CLK_CLC1_OUT,
    .prescaler = 0,
    .postscaler = 0,
    .async = true,
};


// CLC1: JK flip-flop
// J = 1, K = 1
// CLK = TMR2
// R = TMR0
static clc_hw_config_t logic_config = {
    .mode = CLC_HW_MODE_JKFF_R,
    .data_input1 = {   // CLK
        .gate_sel.data = 0x2,
        .invert_output = false, 
        .input_src = 0x12    // TMR2
    },
    .data_input2 = {    // J inv
        .gate_sel.data = 0x8,
        .invert_output = true,
        .input_src  = 0x10  // TMR0
    },
    .data_input3 = {    // R
        .gate_sel.data = 0x20,
        .invert_output = false,
        .input_src = 0x10   // TMR0
    },
    .data_input4 = {    // K inv
        .gate_sel.data = 0x80,
        .invert_output = true,
        .input_src = 0x10   // TMR0
    }
 };
// CLC2: AND gate
// Output = CLC1_OUT AND !TMR0
static clc_hw_config_t logic2_config = {
    .mode = CLC_HW_MODE_AND_OR,
    .data_input1 = {    // AND IN1
        .gate_sel.data = 0x2,   // select gate input one
        .invert_output = false, 
        .input_src = 0x1E       // CLC1_OUT
    },
    .data_input2 = {    // AND IN2
        .gate_sel.data = 0x08,
        .invert_output = true,
        .input_src  = 0x10      // TMR0
    },
    .data_input3 = 0x0,   // unused
    .data_input4 = 0x0,
 };

struct drv8825{
    stepper_device_t         dev;
    gpio_t                   gpio;
    irq_manager_t            int_manager;
    irq_device_t             irq_device_;
    const struct pinmux*     mux;
    const struct timer2x_hw* period_counter;
    const struct timer0_hw*  step_counter;
    const struct clcx_hw*    clc_latch;
    const struct clcx_hw*    clc_and;
    stepper_device_callback  callback;
    uint16_t    speed;
    uint8_t     step_pin;
    uint8_t     enable_pin;
    uint8_t     dir_pin;
    uint8_t     mode0_pin;
    uint8_t     mode1_pin;
    uint8_t     mode2_pin;
    bool bussy;
};

static void drv8825_init(stepper_device_t dev, stepper_pin_config_t* pin_cfg);
static void drv8825_enable(stepper_device_t  dev);
static void drv8825_disable(stepper_device_t  dev);
static void drv8825_set_speed(stepper_device_t  dev, uint16_t frequency);
static void drv8825_set_direction(stepper_device_t  dev, stepper_dir_e dir);
static void drv8825_set_microsteps(stepper_device_t  dev, uint8_t ms);
static void drv8825_single_step(stepper_device_t  dev);
static void drv8825_multi_step(stepper_device_t dev, uint16_t steps);
static void drv8825_set_callback(stepper_device_t dev, stepper_device_callback callback);
static bool drv8825_bussy(stepper_device_t dev);

static struct stepper_ops drv8825_ops = {
    .init           = drv8825_init,
    .enable         = drv8825_enable,
    .disable        = drv8825_disable,
    .single_step    = drv8825_single_step,
    .multi_step     = drv8825_multi_step,
    .set_speed      = drv8825_set_speed,
    .set_direction  = drv8825_set_direction,
    .set_microsteps = drv8825_set_microsteps,
    //.set_callback   = drv8825_set_callback,
    .bussy = drv8825_bussy
};

//static void drv8825_set_callback(stepper_device_t dev, stepper_device_callback callback){
//    struct drv8825* self = container_of(dev, struct drv8825, dev);
//    if(callback != 0){
//        self->callback = callback;
//    }
//}

static void drv8825_enable(stepper_device_t  dev){
    const struct drv8825* self = container_of(dev, struct drv8825, dev);
    gpio_set(self->gpio, self->enable_pin);
}

static void drv8825_disable(stepper_device_t  dev){
    const struct drv8825* self = container_of(dev, struct drv8825, dev);
    gpio_clear(self->gpio, self->enable_pin);
    timer2x_disable(self->period_counter);
    timer0_disable(self->step_counter);
}

static void drv8825_set_speed(stepper_device_t  dev, uint16_t frequency){
    const struct drv8825* self = container_of(dev, struct drv8825, dev);
    fp16 div = fpdiv16_impl(STEPPER_OSC_FREQUENCY,frequency);
    uint8_t period = clamp(div, 0xFE);
    timer2x_set_period(self->period_counter, period);
}

static void drv8825_set_direction(stepper_device_t  dev, stepper_dir_e dir){
    const struct drv8825* self = container_of(dev, struct drv8825, dev);
    gpio_write(self->gpio,self->dir_pin, dir == STEPPER_DIR_CCW ? 0 : 1);
}

static void drv8825_set_microsteps(stepper_device_t  dev, uint8_t ms){
    const struct drv8825* self = container_of(dev, struct drv8825, dev);
    gpio_write(self->gpio, self->mode0_pin, ((ms & (1 << 0))));
    gpio_write(self->gpio, self->mode1_pin, ((ms & (1 << 1))));
    gpio_write(self->gpio, self->mode2_pin, ((ms & (1 << 2))));
}


// blocking single step
static void drv8825_single_step(stepper_device_t  dev){
    const struct drv8825* self = container_of(dev, struct drv8825, dev);
    delay_us(DRV8825_WAKEUP_TIME_US);
    //delay_us(self->wakeup_time_us);
    gpio_set(self->gpio, self->step_pin);
    delay_us(DRV8825_PULSE_DURATION_US);
    gpio_clear(self->gpio, self->step_pin);
    //delay_us(self->pulse_duration_us);
}


// non blocking method for stepping  1 < n < 2^16-1 times
static void drv8825_multi_step(stepper_device_t dev, uint16_t steps){
    struct drv8825* self = container_of(dev, struct drv8825, dev);
    timer0_set_counter(self->step_counter, 0xFFFF - (steps-1));   // preload timer
    timer2x_clear(self->period_counter);        // make sure the period counter is cleared
    clear_interrupt(self->int_manager, 0x1D);   // clear TMR0 int
    gpio_set_direction(self->gpio, self->step_pin, IO_DIR_OUTPUT);
    timer0_enable(self->step_counter);
    timer2x_enable(self->period_counter);   // enable last
    self->bussy = true;
}

static void drv8825_isr(irq_manager_t mngr, void* context){
    stepper_device_t dev = (stepper_device_t)context;
    struct drv8825* self = container_of(dev, struct drv8825, dev);
    gpio_set_direction(self->gpio, self->step_pin, IO_DIR_INPUT);
    clear_interrupt(self->int_manager, 0x1D);   // clear TMR0 int
    timer2x_disable(self->period_counter);
    timer0_disable(self->step_counter);
    timer0_set_counter(self->step_counter, 0);
    self->bussy = false;
}

static bool drv8825_bussy(stepper_device_t dev){
    struct drv8825* self = container_of(dev, struct drv8825, dev);
    return self->bussy;
}

static void drv8825_init(stepper_device_t dev, stepper_pin_config_t* pin_cfg){
    struct drv8825* self = container_of(dev, struct drv8825, dev);
    self->dir_pin = pin_cfg->dir_pin;
    self->step_pin = pin_cfg->step_pin;
    self->enable_pin = pin_cfg->enable_pin;
    self->mode0_pin = pin_cfg->mode0_pin;
    self->mode1_pin = pin_cfg->mode1_pin;
    self->mode2_pin = pin_cfg->mode2_pin;
    gpio_set_mode(self->gpio,self->dir_pin,IO_MODE_DIGITAL);
    gpio_set_mode(self->gpio,self->step_pin,IO_MODE_DIGITAL);
    gpio_set_mode(self->gpio,self->enable_pin,IO_MODE_DIGITAL);
    gpio_set_mode(self->gpio,self->mode0_pin,IO_MODE_DIGITAL);
    gpio_set_mode(self->gpio,self->mode1_pin,IO_MODE_DIGITAL);
    gpio_set_mode(self->gpio,self->mode2_pin,IO_MODE_DIGITAL);
    gpio_set_direction(self->gpio,self->dir_pin,IO_DIR_OUTPUT);
    gpio_set_direction(self->gpio,self->step_pin,IO_DIR_INPUT);
    gpio_set_direction(self->gpio,self->enable_pin,IO_DIR_OUTPUT);
    gpio_set_direction(self->gpio,self->mode0_pin,IO_DIR_OUTPUT);
    gpio_set_direction(self->gpio,self->mode1_pin,IO_DIR_OUTPUT);
    gpio_set_direction(self->gpio,self->mode2_pin,IO_DIR_OUTPUT);
    pinmux_set_output(self->mux, self->step_pin, 0x0F);
    pinmux_set_output(self->mux, self->mode0_pin, 0x0);
    pinmux_set_output(self->mux, self->mode1_pin, 0x0);
    pinmux_set_output(self->mux, self->mode2_pin, 0x0);
    timer2x_init(self->period_counter, &period_counter_config);
    timer0_init(self->step_counter, &step_counter_config);
    clcx_hw_init(self->clc_latch, &logic_config);
    clcx_hw_enable(self->clc_latch);
    //clcx_hw_init(self->clc_and, &logic2_config);
    irq_manager_instance(&self->int_manager);
    irq_manager_register(self->int_manager, &self->irq_device_);
    self->bussy = false;
}

void create_stepper_device(stepper_device_t* out_dev) {
    static struct drv8825 self;
    static const struct stepper_ops *ops = &drv8825_ops;
    self.dev = ops;
    *out_dev = &self.dev;
    gpio_instance(&self.gpio);
    create_timer0(&self.step_counter, false);
    create_timer2x(&self.period_counter, 0);
    clcx_hw_create(0,0,&self.clc_latch);
    // something is wrong with my clc implementation. so only clc1 works for some reason
    //clcx_hw_create(1,0,&self.clc_and); 
    self.irq_device_.handle     = drv8825_isr;
    self.irq_device_.prio       = IRQ_HIGH_PRIORITY;
    self.irq_device_.vic_offset = 0x1D;
    self.irq_device_.context    = &self.dev;
}