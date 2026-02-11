/* 
 * File:   gpio.h
 * Author: Gard
 *k
 * Created on 21 January 2026, 23:46
 */

#ifndef GPIO_H
#define	GPIO_H

#include <stdint.h>
#include <stdbool.h>

typedef enum{
    IO_DIR_INPUT,
    IO_DIR_OUTPUT
} gpio_dir_e;

typedef enum{
    IO_MODE_DIGITAL,
    IO_MODE_ANALOG,
    IO_MODE_ALT
}gpio_mode_e;

typedef enum{
    IO_RESISTOR_PULLUP,
    IO_RESISTOR_PULLDOWN,
    IO_RESISTOR_DISABLE
}gpio_resistor_e;

typedef struct {
    gpio_dir_e      dir;
    gpio_mode_e     mode;
}gpio_config;

typedef enum {
    IO_0, IO_1,  IO_2,  IO_3,  IO_4,  IO_5,  IO_6,  IO_7, // PORTA
    IO_8, IO_9, IO_10, IO_11, IO_12, IO_13, IO_14, IO_15, // PORTB
    IO_16,IO_17,IO_18, IO_19, IO_20, IO_21, IO_22, IO_23, // PORTC
    IO_24,IO_25,IO_26, IO_27, IO_28, IO_29, IO_30, IO_31, // PORTD
    IO_32,IO_33,IO_34, IO_35, IO_36, IO_37, IO_38, IO_39, // PORTE
    IO_40,IO_41,IO_42, IO_43, IO_44, IO_45, IO_46, IO_47, // PORTF
} gpio_pin;

// left as a interface since it might be used as a interface for a gpio expander
struct gpio_ops;
typedef const struct gpio_ops ** gpio_t;
struct gpio_ops{
    void (*write_pin)(gpio_t dev, uint8_t pin, bool value);
    void (*read_pin)(gpio_t dev, uint8_t pin, bool* value);
    void (*toggle_pin)(gpio_t dev, uint8_t pin);
    void (*set_mode)(gpio_t dev, uint8_t pin, gpio_mode_e mode);
    void (*set_direction)(gpio_t dev, uint8_t pin, gpio_dir_e direction);
};

void gpio_instance(gpio_t* inst_out);
#define gpio_set(gpio,pin)((*(gpio))->write_pin(gpio,pin,true))
#define gpio_clear(gpio,pin)((*(gpio))->write_pin(gpio,pin,false))
#define gpio_write(gpio,pin,val)((*(gpio))->write_pin(gpio,pin,val))
#define gpio_toggle(gpio,pin)((*(gpio))->toggle_pin(gpio,pin))
#define gpio_set_mode(gpio,pin,mode)((*(gpio))->set_mode(gpio,pin,mode))
#define gpio_set_direction(gpio,pin,direction)((*(gpio))->set_direction(gpio,pin,direction))

// should be seperate in a pinmux implementation
void pps_lock(void);
void pps_unlock(void);
#endif	/* GPIO_H */

