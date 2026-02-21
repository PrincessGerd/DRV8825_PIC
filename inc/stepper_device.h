#ifndef STEPPER_DEVICE_H
#define STEPPER_DEVICE_H
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    STEPPER_DIR_CW = 0,
    STEPPER_DIR_CCW
} stepper_dir_e;

struct stepper_ops;
typedef const struct stepper_ops** stepper_device_t;
struct stepper_ops{
    void(*init)(stepper_device_t dev, void* cfg);
    void(*enable)(stepper_device_t dev);
    void(*disable)(stepper_device_t dev);
    void(*set_direction)(stepper_device_t dev,stepper_dir_e dir);
    void(*set_microsteps)(stepper_device_t dev,uint8_t ms);
};

#define stepper_enable(stepper) ((*(stepper))->enable(stepper))
#define stepper_disable(stepper)((*(stepper))->disable(stepper))
#define stepper_set_direction(stepper,dir)((*(stepper))->set_direction(stepper, dir))
#define stepper_set_microsteps(stepper,ms)((*(stepper))->set_microsteps(stepper,ms))
void create_stepper_device(stepper_device_t* dev);

#endif