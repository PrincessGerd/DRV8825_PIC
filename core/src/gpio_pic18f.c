#include "../gpio.h"
#include <xc.h>
// singleton implementation of gpio

void _gpio_write_pin(gpio_t dev, uint8_t io_pin, bool value);
void _gpio_read_pin(gpio_t dev, uint8_t io_pin, bool* value);
void _gpio_toggle_pin(gpio_t dev, uint8_t io_pin);
void _gpio_set_mode(gpio_t dev, uint8_t io_pin, gpio_mode_e mode);
void _gpio_set_direction(gpio_t dev, uint8_t io_pin, gpio_dir_e direction);

struct pic18f16{gpio_t dev;};
static struct pic18f16 instance;
static struct gpio_ops pic18f_gpio_ops = {
    .write_pin = _gpio_write_pin,
    .read_pin = _gpio_read_pin,
    .toggle_pin = _gpio_toggle_pin,
    .set_mode = _gpio_set_mode,
    .set_direction = _gpio_set_direction
};


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


void gpio_instance(gpio_t* inst_out){
    if(!instance.dev){
        instance.dev = &pic18f_gpio_ops;
    }
    *inst_out = &instance.dev;
}

void _gpio_write_pin(gpio_t dev, uint8_t io_pin, bool value) {
    (void)dev;
    const uint8_t port = io_port(io_pin);
    const uint8_t pin = io_pin_bit(pin);
    if(value){
        *io_latch_register[port] |= pin;
    }else{
        *io_latch_register[port] &= ~pin;
    }
}

void _gpio_read_pin(gpio_t dev, uint8_t io_pin, bool* value){
    (void)dev;
    const uint8_t port = io_port(io_pin);
    const uint8_t pin = io_pin_bit(io_pin);
    *value = pin & (*io_port_register[port]);
}

void _gpio_toggle_pin(gpio_t dev, uint8_t io_pin){
    (void)dev;
    const uint8_t port = io_port(io_pin);
    const uint8_t pin = io_pin_bit(io_pin);
    *io_latch_register[port] ^= pin;
}

void _gpio_set_mode(gpio_t dev, uint8_t io_pin, gpio_mode_e mode){
    (void)dev;
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
void _gpio_set_direction(gpio_t dev, uint8_t io_pin, gpio_dir_e direction){
    (void)dev;
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
/*----------------------------------------------------------------*/
void pps_unlock(void) {
    INTCON0bits.GIE = 0;    // Suspend interrupts
    PPSLOCK = 0x55;         // Required sequences
    PPSLOCK = 0xAA;
    PPSLOCKbits.PPSLOCKED = 0;
    INTCON0bits.GIE = 1;    // Restore interrupts
}

void pps_lock(void) {
    INTCON0bits.GIE = 0;    // Suspend interrupts
    PPSLOCK = 0x55;         // Required sequences
    PPSLOCK = 0xAA;
    PPSLOCKbits.PPSLOCKED = 1;
    INTCON0bits.GIE = 1;    // Restore interrupts
}