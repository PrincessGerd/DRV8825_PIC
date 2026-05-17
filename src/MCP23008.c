#include "../inc/MCP23008.h"
#include <stdint.h>
#include <stdbool.h>

#define MCP23008_REG_IODIR      0x00
#define MCP23008_REG_IPOL       0x01
#define MCP23008_REG_GPINTEN    0x02
#define MCP23008_REG_DEFVAL     0x03
#define MCP23008_REG_INTCON     0x04
#define MCP23008_REG_IOCON      0x05
#define MCP23008_REG_GPPU       0x06
#define MCP23008_REG_INTF       0x07
#define MCP23008_REG_INTCAP     0x08
#define MCP23008_REG_GPIO       0x09
#define MCP23008_REG_OLAT       0x0A

#define MCP23008_I2C_ADDR_BASE  0x64

typedef struct mcp23008{
    struct i2c_hw* i2c;
    uint8_t address;
} mcp23008_t;


bool mcp23008_write_reg(mcp23008_t* dev, uint8_t reg, uint8_t value){
    uint8_t tx[2] = {0};
    tx[0] = reg;
    tx[1] = value;
    return i2c_hw_write(dev->i2c, dev->address, tx, 2);
}

bool mcp23008_read_reg(mcp23008_t* dev, uint8_t reg, uint8_t* value){
    if(!i2c_hw_write(dev->i2c, dev->address, &reg, 1)){ return false; }
    return i2c_hw_read(dev->i2c, dev->address, value, 1);
}

bool mcp23008_init(mcp23008_t* dev, struct i2c_hw* i2c_inst, uint8_t address){
    dev->i2c = i2c_inst;
    dev->address = address;
    // set all pins to output, and disable pullup and interrupts
    if(!mcp23008_write_reg(dev, MCP23008_REG_IODIR, 0x00)){ return false;}
    if(!mcp23008_write_reg(dev, MCP23008_REG_GPPU, 0x00)){ return false;}
    if(!mcp23008_write_reg(dev, MCP23008_REG_GPINTEN, 0x00)){ return false;}
    return true;
}

// TODO: CHEK in ds if its correct
bool mcp23008_write_pin(mcp23008_t* dev, uint8_t pin, bool value){
    uint8_t tx[2] = {0};
    tx[0] = MCP23008_REG_OLAT;
    tx[1] = (value << pin);
    return i2c_hw_write(dev->i2c, dev->address, tx, 2);
}

bool mcp23008_read_pin(mcp23008_t* dev, uint8_t pin, bool* value){
    uint8_t reg = MCP23008_REG_GPIO;
    if(!i2c_hw_write(dev->i2c, dev->address, &reg, 1)){ return false; }
    uint8_t _val = 0;
    bool res = i2c_hw_read(dev->i2c, dev->address, &_val, 1);
    *value = (_val & (1 << pin));
}
