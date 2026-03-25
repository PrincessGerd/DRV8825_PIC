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

void gpio_set(uint8_t io_pin);
void gpio_clear(uint8_t io_pin);
void gpio_write(uint8_t io_pin, bool value);
void gpio_read(uint8_t io_pin, bool* value);
void gpio_toggle(uint8_t io_pin);
void gpio_set_mode(uint8_t io_pin, gpio_mode_e mode);
void gpio_set_direction(uint8_t io_pin, gpio_dir_e direction);
#endif	/* GPIO_H */

