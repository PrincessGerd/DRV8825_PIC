#pragma once
#include <stdint.h>

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