#include "../inc/i2c_hw.h"
#include "../registers.h"
#include "../core/interrupts.h"
#include "../core/serial_logger.h"
#include <stdint.h>
#include <stdbool.h>

//36.4.2.7.1. Host Transmission (7-Bit Addressing Mode)

typedef enum {
    I2C_ERROR_NONE,
    I2C_ERROR_NACK,
    I2C_ERROR_BUS_COLISION,
}i2c_host_error_e;

typedef enum{
    I2C_EVENT_IDLE = 0,
    I2c_EVENT_TX,
    I2c_EVENT_RX,
    I2C_EVENT_DONE,
    I2C_EVENT_ERROR
} i2c_host_event_e;

typedef struct i2c_hw{
    uintptr_t base;
    i2c_host_event_e state;
    i2c_host_error_e errno;
    uint8_t* write_ptr;
    uint8_t* read_ptr;
    unsigned int write_len;
    unsigned int read_len;
    bool busy;
    bool in_use;
} i2c_hw_t;

static i2c_hw_t inst;

void i2c_hw_instance(i2c_hw_t* i2c_out){
    if(!inst.in_use && i2c_out != 0){
        inst.base = I2C1_BASE_ADDR;
        inst.in_use = true;
    }
    i2c_out = &inst;
}

void i2c_hw_initialise(i2c_hw_t* self, i2c_hw_config_t config){

    uint8_t* CON0 = REG8(self->base, I2C_CON0_OFFSET);
    CLEAR_BIT(CON0, I2C_CON0_EN_SHIFT);

    interrupt_enable(0x3B);
    interrupt_clear(0x3B);
    interrupt_enable(0x3C);
    interrupt_clear(0x3C);
    interrupt_enable(0x3D);
    interrupt_clear(0x3D);
    interrupt_enable(0x3E);
    interrupt_clear(0x3E);

    uint8_t* ERR = REG8(self->base, I2C_ERR_OFFSET);
    *ERR ^= *ERR;
    *ERR = I2C_ERR_BTOIE_MASK | I2C_ERR_BCLIE_MASK | I2C_ERR_NACKIE_MASK;
    
    uint8_t* PIE =  REG8(self->base, I2C_PIE_OFFSET);
    *PIE ^= *PIE;
    *PIE = I2C_PIE_ACKTIE_MASK | I2C_PIE_CNTIE_MASK | I2C_PIE_PCIE_MASK;
}

bool i2c_hw_busy(i2c_hw_t* self){
    return self->busy | GET_BIT(REG8(self->base,I2C_STAT0_OFFSET), I2C_STAT0_MMA_SHIFT);
}

bool i2c_hw_write(i2c_hw_t* self, uint16_t address, uint8_t *data, unsigned int len){
    SLOG_ASSERT(self->in_use);

    if(i2c_hw_busy(self) || len == 0){
        return false;
    }
    self->busy    = true;
    self->state   = I2c_EVENT_TX;
    self->errno   = I2C_ERROR_NONE;

    *REG8(self->base, I2C_ADB1_OFFSET) = ((address << 1) & ~0x01);
    *REG8(self->base, I2C_CNTL_OFFSET) = (uint8_t)(self->write_len & 0xFF);
    *REG8(self->base, I2C_CNTH_OFFSET) = (uint8_t)(self->write_len >> 8U);
    *REG8(self->base, I2C_TXB_OFFSET)  = (*self->write_ptr++);
    SET_BIT(REG8(self->base,I2C_CON0_OFFSET), I2C_CON0_S_SHIFT);
    return true;
}

bool i2c_hw_read (i2c_hw_t* self, uint16_t address, uint8_t *data, unsigned int len){
    SLOG_ASSERT(self->in_use);

    if(i2c_hw_busy(self) || len == 0){
        return false;
    }
    self->busy    = true;
    self->state   = I2c_EVENT_RX;
    self->errno   = I2C_ERROR_NONE;
    
    self->read_ptr = data;
    self->read_len = len;
    *REG8(self->base, I2C_ADB1_OFFSET) = ((address << 1) & ~0x01);
    *REG8(self->base, I2C_CNTL_OFFSET) = (uint8_t)(self->write_len & 0xFF);
    *REG8(self->base, I2C_CNTH_OFFSET) = (uint8_t)(self->write_len >> 8U);
    SET_BIT(REG8(self->base,I2C_CON0_OFFSET), I2C_CON0_S_SHIFT);
    return true;
}

void __interrupt(irq(0x3B)) i2c_rx_isr(){
    interrupt_clear(0x3B);
    (*inst.read_ptr++) =  *REG8(inst.base, I2C_RXB_OFFSET); 
}

void __interrupt(irq(0x3C)) i2c_tx_isr(){
    interrupt_clear(0x3C);
    *REG8(inst.base, I2C_TXB_OFFSET)  = (*inst.write_ptr++);
}

void __interrupt(irq(0x3D)) i2c_base_isr(){
    interrupt_clear(0x3D);
    uint8_t* PIR = REG8(inst.base,I2C_PIR_OFFSET);
    if(GET_BIT(PIR, I2C_PIR_CNTIF_SHIFT)){
        CLEAR_BIT(PIR,I2C_PIR_CNTIF_SHIFT);
        inst.state - I2C_EVENT_DONE;
    }

    if(GET_BIT(PIR, I2C_PIR_PCIF_SHIFT)){
        CLEAR_BIT(PIR, I2C_PIR_PCIF_SHIFT);
        inst.busy = false;
        if(inst.errno == I2C_ERROR_NONE){
            inst.state = I2C_EVENT_IDLE;
        }else{
            inst.state = I2C_EVENT_ERROR;
        }
    }

    if(GET_BIT(PIR, I2C_PIR_ACKTIF_SHIFT)){
        CLEAR_BIT(PIR, I2C_PIR_ACKTIF_SHIFT);
        inst.errno = I2C_ERROR_NACK;
        inst.state = I2C_EVENT_ERROR;
    }

    if(GET_BIT(PIR, I2C_PIR_SCIF_SHIFT)){
        CLEAR_BIT(PIR, I2C_PIR_SCIF_SHIFT);
    }
}

void __interrupt(irq(0x3E)) i2c_error_isr(){
    interrupt_clear(0x3E);
    inst.errno = I2C_ERROR_BUS_COLISION;

    inst.busy = false;
    inst.state = I2C_EVENT_ERROR;
    // add error handling
}