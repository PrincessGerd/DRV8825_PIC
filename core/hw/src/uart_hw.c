#include "../inc/utmr_hw.h"
#include <stdbool.h>
#include <stdint.h>
#include <xc.h>

struct uart_hw{
    uint16_t baud;
    bool high_speed;
    uart_mode_e mode;
};

void create_uart_hw(struct uart_hw const **uart_hw_inst_out){
    static struct uart_hw hw;
    *uart_hw_inst_out = &hw;
}
void uart_hw_init(struct uart_hw const *self, uart_hw_config_t* config){
    self->move = config->mode;
    self->high_speed = config->high_speed;
    U1CON0 ^= U1CON0;
    U1CON0 = (
        (self->high_speed << 7) | 
        (self->auto_baud << 6)  | 
        (self->mode << 0));
}

void uart_hw_set_baud(struct uart_hw const *self, uint16_t baud){
    if(self->mode & (1<<4)){
        baud = (self->high_speed) 
            ? ((baud > 0x7FFE) ? 0x7FFE : baud)
            : ((baud > 0x1FFE) ? 0x1FFE : baud);
    }
    self->baud = baud;
    U1BRGL = baud & 0x00FF;
    u1BRGH = baud & 0xFF00;
}

void uart_hw_enable(struct uart_hw const *self);
void uart_hw_TX_enable(struct uart_hw const *self);
void uart_hw_RX_enable(struct uart_hw const *self);

void uart_hw_disable(struct uart_hw const *self);
void uart_hw_TX_disable(struct uart_hw const *self);
void uart_hw_RX_disable(struct uart_hw const *self);

void uart_hw_write_byte(struct uart_hw const *self, uint8_t byte);
void uart_hw_read_byte(struct uart_hw const *self, uint8_t *byte);
void uart_hw_write(struct uart_hw const *self, uint8_t *byte);
void uart_hw_read(struct uart_hw const *self, uint8_t *byte);
