#ifndef MCP23008_H
#define MCP23008_H
#include <stdint.h>
#include <stdbool.h>
#include "../core/hw/inc/i2c_hw.h"

    struct mcp23008;
    bool mcp23008_init(struct mcp23008* dev, struct i2c_hw* i2c_inst, uint8_t address);
    bool mcp23008_write_reg(struct mcp23008* dev, uint8_t reg, uint8_t value);
    bool mcp23008_read_reg(struct mcp23008* dev, uint8_t reg, uint8_t* value);

    bool mcp23008_write_pin(struct mcp23008* dev, uint8_t pin, bool value);
    bool mcp23008_read_pin(struct mcp23008* dev, uint8_t pin, bool* value);

    #define mcp23008_write_gpio(dev, value) (mcp23008_write_reg(dev, MCP23008_REG_OLAT, value))
    #define mcp23008_write_gpio(dev, val_ptr) (mcp23008_write_reg(dev, MCP23008_REG_GPIO, (val_ptr)))
#endif