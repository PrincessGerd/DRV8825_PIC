#ifndef PINMUX_H
#define PINMUX_H
#include <stdint.h>
typedef enum{
    PM_RESISTOR_PULLDOWN = 0x0,
    PM_RESISTOR_PULLUP = 0x1,
    PM_RESISTOR_DISABLE = 0x2
} pinmux_resistor_e;

struct pinmux;
void pinmux_set_output(struct pinmux* self, uint8_t pin, uint8_t alt_func);
void pinmux_set_input(struct pinmux* self, uint8_t pin, uint8_t alt_func);
void pinmux_reset(struct pinmux* self, uint8_t pin, uint8_t alt_func);
void pinmux_resistor(struct pinmux* self, uint8_t pin, pinmux_resistor_e resistor);

#endif