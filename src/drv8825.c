#include "../inc/stepper_device.h"
// core
#include "../core/types.h"
#include "../core/gpio.h"

#define DRV8825_WAKEUP_TIME_US 1
#define DRV8825_PULSE_DURATION_US 2

struct drv8825{
    stepper_device_t dev;
    uint8_t     dir_pin;
    uint8_t     enable_pin;
    uint8_t     mode0_pin;
    uint8_t     mode1_pin;
    uint8_t     mode2_pin;
};

typedef struct{
    uint8_t step_pin;
    uint8_t dir_pin;
    uint8_t enable_pin;
    uint8_t mode0_pin;
    uint8_t mode1_pin;
    uint8_t mode2_pin;
} drv8825_pin_config_t;

static void drv8825_init(stepper_device_t dev, void* pin_cfg);
static void drv8825_enable(stepper_device_t dev);
static void drv8825_disable(stepper_device_t dev);
static void drv8825_set_direction(stepper_device_t dev, stepper_dir_e dir);
static void drv8825_set_microsteps(stepper_device_t dev, uint8_t ms);

static struct stepper_ops drv8825_ops = {
    .init           = drv8825_init,
    .enable         = drv8825_enable,
    .disable        = drv8825_disable,
    .set_direction  = drv8825_set_direction,
    .set_microsteps = drv8825_set_microsteps,
};

void create_stepper_device(stepper_device_t* out_dev) {
    static struct drv8825 self;
    static const struct stepper_ops *ops = &drv8825_ops;
    self.dev = ops;
    *out_dev = &self.dev;
}

static void drv8825_init(stepper_device_t dev, void* pin_cfg){
    struct drv8825* self = container_of(dev, struct drv8825, dev);
    drv8825_pin_config_t* cfg = (drv8825_pin_config_t*)pin_cfg;
    self->dir_pin = cfg->dir_pin;
    self->enable_pin = cfg->enable_pin;
    self->mode0_pin = cfg->mode0_pin;
    self->mode1_pin = cfg->mode1_pin;
    self->mode2_pin = cfg->mode2_pin;
    gpio_set_mode(self->dir_pin,IO_MODE_DIGITAL);
    gpio_set_mode(self->enable_pin,IO_MODE_DIGITAL);
    gpio_set_mode(self->mode0_pin,IO_MODE_DIGITAL);
    gpio_set_mode(self->mode1_pin,IO_MODE_DIGITAL);
    gpio_set_mode(self->mode2_pin,IO_MODE_DIGITAL);
    gpio_set_direction(self->dir_pin,IO_DIR_OUTPUT);
    gpio_set_direction(self->enable_pin,IO_DIR_OUTPUT);
    gpio_set_direction(self->mode0_pin,IO_DIR_OUTPUT);
    gpio_set_direction(self->mode1_pin,IO_DIR_OUTPUT);
    gpio_set_direction(self->mode2_pin,IO_DIR_OUTPUT);
}

static void drv8825_enable(stepper_device_t dev){
    struct drv8825* self = container_of(dev, struct drv8825, dev);
    gpio_set(self->enable_pin);
}

static void drv8825_disable(stepper_device_t dev){
    struct drv8825* self = container_of(dev, struct drv8825, dev);
    gpio_clear(self->enable_pin);
}

static void drv8825_set_direction(stepper_device_t dev, stepper_dir_e dir){
    struct drv8825* self = container_of(dev, struct drv8825, dev);
    gpio_write(self->dir_pin, dir == STEPPER_DIR_CCW ? 0 : 1);
}

static void drv8825_set_microsteps(stepper_device_t dev, uint8_t ms){
    struct drv8825* self = container_of(dev, struct drv8825, dev);
    ms = ms > 0x7 ? 0x7 : ms;
    gpio_write(self->mode0_pin, ((ms & (1 << 0))));
    gpio_write(self->mode1_pin, ((ms & (1 << 1))));
    gpio_write(self->mode2_pin, ((ms & (1 << 2))));
}