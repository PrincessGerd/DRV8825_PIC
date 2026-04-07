#ifndef UART_HW_H
#define UART_HW_H
#include <stdint.h>
#include <stdbool.h>
#include "../core/system.h"

typedef enum {
    UART_LIN_MODE           = 0b1100,
    UART_DMX_MODE           = 0b1010,
    UART_ADDRESS_MODE       = 0b0100,
    UART_EVEN_PARITY_MODE   = 0b0011,
    UART_ODD_PARITY_MODE    = 0b0010,
    UART_7BIT_MODE          = 0b0001,
    UART_8BIT_MODE          = 0b0000,
} uart_mode_e;

typedef enum {
    UART_STOP_BIT_2_1 = 0x3,  // 2 stop bits, verify first
    UART_STOP_BIT_2_2 = 0x2,  // 2 stop bits, verify both
    UART_STOP_BIT_1_5 = 0x1,  // 1.5 stop bits, verify first
    UART_STOP_BIT_1_1 = 0x0  // first stop bit verify
} uart_hw_stop_bit_e;

typedef enum {
    UART_FCTRL_RTS_CTS = 0x2,
    UART_FCTRL_XON_XOF_SW = 0x1,
    UART_FCTRL_OFF  = 0x0
} uart_hw_flow_ctrl_e;


typedef struct{
    uart_mode_e mode;
    uart_hw_stop_bit_e stop_bits;
    uart_hw_flow_ctrl_e flow_ctrl;
    uint16_t baud;
    bool run_during_overflow;
    bool invert_recive;
    bool invert_transmit;
    bool use_checksum;  // checksum always enabled for LIN
    bool auto_baud;
    bool high_speed;
}uart_hw_config_t;

struct uart_hw;
void create_uart_hw(struct uart_hw const **uart_hw_inst_out);
void uart_hw_init(struct uart_hw *self,const uart_hw_config_t* config);
void uart_hw_set_baud(struct uart_hw *self, uint16_t baud);

void uart_hw_enable(struct uart_hw const *self);
void uart_hw_TX_enable(struct uart_hw const *self);
void uart_hw_RX_enable(struct uart_hw const *self);

void uart_hw_disable(struct uart_hw const *self);
void uart_hw_TX_disable(struct uart_hw const *self);
void uart_hw_RX_disable(struct uart_hw const *self);


extern void uart_hw_write_byte(struct uart_hw const *self, uint8_t byte);
extern void uart_hw_read_byte(struct uart_hw const *self, uint8_t *byte);
void uart_hw_write(struct uart_hw const *self, uint8_t *bytes, uint16_t tx_len, uint16_t* bytes_writen);
void uart_hw_read(struct uart_hw const *self, uint8_t *bytes, uint16_t* bytes_recived);

#endif