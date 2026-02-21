#include "../gpio.h"
#include <xc.h>

// [7:5] zeros, [4:3] port, [2:0] pin
#define PORT_COUNT (3u)
#define IO_PORT_OFFSET (3u)
#define IO_PORT_MASK (0x3u << IO_PORT_OFFSET)
#define IO_PIN_MASK (0x7u)
static volatile uint8_t* const io_port_register[PORT_COUNT]              = {&PORTA,  &PORTB,  &PORTC};
static volatile uint8_t* const io_latch_register[PORT_COUNT]             = {&LATA,   &LATB,   &LATC};
static volatile uint8_t* const io_direction_register[PORT_COUNT]         = {&TRISA,  &TRISB,  &TRISC};
static volatile uint8_t* const io_analouge_select_register[PORT_COUNT]   = {&ANSELA, &ANSELB, &ANSELC};

static inline uint8_t io_port(uint8_t pin){
    return (uint8_t)(pin & IO_PORT_MASK) >> IO_PORT_OFFSET; }

static inline uint8_t io_pin_bit(uint8_t pin){
    return (uint8_t)(1 << (pin & IO_PIN_MASK)); }


void gpio_write_pin(uint8_t io_pin, bool value) { 
    const uint8_t port = io_port(io_pin);
    const uint8_t pin = io_pin_bit(io_pin);
    if(value){
        *io_latch_register[port] |= pin;
    }else{
        *io_latch_register[port] &= ~pin;
    }
}

void gpio_set(uint8_t io_pin){gpio_write_pin(io_pin,true);}
void gpio_clear(uint8_t io_pin){gpio_write_pin(io_pin,false);}

void gpio_read_pin(uint8_t io_pin, bool* value){   
    const uint8_t port = io_port(io_pin);
    const uint8_t pin = io_pin_bit(io_pin);
    *value = pin & (*io_port_register[port]);
}

void gpio_toggle_pin(uint8_t io_pin){
    const uint8_t port = io_port(io_pin);
    const uint8_t pin = io_pin_bit(io_pin);
    *io_latch_register[port] ^= pin;
}

void gpio_set_mode(uint8_t io_pin, gpio_mode_e mode){
    const uint8_t port = io_port(io_pin);
    const uint8_t pin = io_pin_bit(io_pin);
    switch (mode) {
        case IO_MODE_DIGITAL | IO_MODE_ALT:
            *io_analouge_select_register[port] &= ~pin;
            break;
        case IO_MODE_ANALOG:
            *io_analouge_select_register[port] |= pin;
    default:
        break;
    }
}
void gpio_set_direction(uint8_t io_pin, gpio_dir_e direction){
    const uint8_t port = io_port(io_pin);
    const uint8_t pin = io_pin_bit(io_pin);
    switch (direction) {
        case IO_DIR_INPUT:
            *io_direction_register[port] |= pin;
            break;
        case IO_DIR_OUTPUT:
            *io_direction_register[port] &= ~pin;
            break;
    default:
        break;
    }
}