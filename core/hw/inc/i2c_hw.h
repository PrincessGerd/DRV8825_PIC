#ifndef I2C_HW_H
#define I2C_HW_H
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    I2C_BAUD_100KHZ = 0,
    I2C_BAUD_400KHZ,
    I2C_BAUD_1MHZ
} i2c_baud_e;

typedef enum {

    I2C_BITMODE_7BIT = 1,
} i2c_bitmode_e;

typedef struct {
    i2c_bitmode_e bitmode;
} i2c_hw_config_t;

struct i2c_hw;
void i2c_hw_instance(struct i2c_hw* i2c_out);
void i2c_hw_initialise(struct i2c_hw* self, i2c_hw_config_t config);
bool i2c_hw_busy(struct i2c_hw* self);
bool i2c_hw_write(struct i2c_hw* self, uint16_t address, uint8_t *data, unsigned int len);
bool i2c_hw_read (struct i2c_hw* self, uint16_t address, uint8_t *data, unsigned int len);

#endif