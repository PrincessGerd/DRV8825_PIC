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
    RA_0, RA_1,  RA_2,  RA_3,  RA_4,  RA_5,  RA_6,  RA_7, // PORTA
    RB_0, RB_1,  RB_2,  RB_3,  RB_4,  RB_5,  RB_6,  RB_7, // PORTB
    RC_0, RC_1,  RC_2,  RC_3,  RC_4,  RC_5,  RC_6,  RC_7, // PORTC
    RD_0, RD_1,  RD_2,  RD_3,  RD_4,  RD_5,  RD_6,  RD_7, // PORTD
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

