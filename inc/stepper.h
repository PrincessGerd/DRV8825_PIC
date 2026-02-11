/* 
* File:   stepper.h
* Author: Gard
*
* Created on January 20, 2026, 2:16 PM
*/

#ifndef STEPPER_H
#define	STEPPER_H

#include <stdint.h>
#include <stdbool.h>


typedef enum {
    STEPPER_DIR_CW = 0,
    STEPPER_DIR_CCW
} stepper_dir_e;

typedef struct stepper_pin_config{
    uint8_t step_pin;
    uint8_t dir_pin;
    uint8_t enable_pin;
    uint8_t mode0_pin;
    uint8_t mode1_pin;
    uint8_t mode2_pin;
} stepper_pin_config_t;

struct stepper_ops;
typedef const struct stepper_ops** stepper_device_t;
typedef void(*stepper_device_callback)(stepper_device_t dev);
struct stepper_ops{
    void(*init)(stepper_device_t dev, stepper_pin_config_t* cfg);
    void(*enable)(stepper_device_t dev);
    void(*disable)(stepper_device_t dev);
    void(*single_step)(stepper_device_t dev);
    void(*multi_step)(stepper_device_t dev,uint16_t steps);
    void(*set_speed)(stepper_device_t dev,uint16_t frequency);
    void(*set_direction)(stepper_device_t dev,stepper_dir_e dir);
    void(*set_microsteps)(stepper_device_t dev,uint8_t ms);
    bool(*bussy)(stepper_device_t dev);
    //void(*set_callback)(stepper_device_t dev, stepper_device_callback callback);
};

#define stepper_init(stepper, config)((*(stepper))->init(stepper,config))
#define stepper_enable(stepper) ((*(stepper))->enable(stepper))
#define stepper_disable(stepper)((*(stepper))->disable(stepper))
#define stepper_single_step(stepper)((*(stepper))->single_step(stepper))
#define stepper_multi_step(stepper,steps)((*(stepper))->multi_step(stepper, steps))
#define stepper_set_speed(stepper,frequency)((*(stepper))->set_speed(stepper, frequency))
#define stepper_set_direction(stepper,dir)((*(stepper))->set_direction(stepper, dir))
#define stepper_set_microsteps(stepper,ms)((*(stepper))->set_microsteps(stepper,ms))
#define stepper_bussy(stepper)((*(stepper))->bussy(stepper))
//#define stepper_set_callback(stepper,callback)((*(stepper))->set_callback(stepper,callback))

void create_stepper_device(stepper_device_t* dev);
#endif	/* STEPPER_H*/


