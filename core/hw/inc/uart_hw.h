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
void uart_hw_init(const uart_hw_config_t* config);
void uart_hw_set_baud(uint16_t baud);

void uart_hw_enable(void);
void uart_hw_TX_enable(void);
void uart_hw_RX_enable(void);

void uart_hw_disable(void);
void uart_hw_TX_disable(void);
void uart_hw_RX_disable(void);

void uart_hw_write(uint8_t *bytes, unsigned int len);
void uart_hw_read(uint8_t* byte);
void uart_hw_write_polling(uint8_t *bytes, uint8_t tx_len, uint8_t* bytes_writen);
#endif