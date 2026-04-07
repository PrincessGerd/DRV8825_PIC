#include "../inc/uart_hw.h"
#include "../core/interrupts.h"
#include "../inc/ringbuffer.h"  // move to core
#include <stdbool.h>
#include <stdint.h>
#include <xc.h>


typedef enum {
    UART_STATUS_RESET = 0x00,
    UART_STATUS_IDLE  = 0x00,
    UART_STATUS_BUSY  = 0x00
}uart_status_e;

struct uart_hw{
    uint16_t baud;
    uart_mode_e mode;
    bool high_speed;
    bool checksum;
    rb_state_t* rx_buffer;
    rb_state_t* tx_buffer;
    uart_status_e status;
};

static struct uart_hw inst = {0};

// check TXBE in FIFO register
static inline bool tx_empty(){
    return ((U1FIFO  & (1 << 5)) == 0); }
// check TXBF in FIFO register
static inline bool tx_full(){
    return ((U1FIFO  & (1 << 4)) == 0); }
// check RXBE in FIFO register
static inline bool rx_empty(){
    return ((U1FIFO  & (1 << 1)) == 0); }
// check RXBF in FIFO register
static inline bool rx_full(){
    return ((U1FIFO  & (1 << 0)) == 0); }

void create_uart_hw(struct uart_hw const **uart_hw_inst_out){
    //static struct uart_hw hw;
    *uart_hw_inst_out = &inst;
}
void uart_hw_init(struct uart_hw *self, const uart_hw_config_t* config){
    self->mode = config->mode;
    self->high_speed = config->high_speed;
    self->checksum   = config->use_checksum;
    U1CON1 = 0;
    U1CON0 ^= U1CON0;
    U1CON0 = (
        (config->high_speed << 7) | 
        (config->auto_baud << 6)  | 
        (config->mode << 0));
        
    U1CON2 ^- U1CON2;
    U1CON2 = (
        (config->run_during_overflow << 7) | 
        (config->invert_recive << 6)       | 
        (config->stop_bits << 4)           | 
        (config->use_checksum << 3)        | 
        (config->invert_transmit << 2)     | 
        (config->flow_ctrl << 0));  

    if(!config->auto_baud){
        uart_hw_set_baud(self, self->baud);
    }
}

void uart_hw_set_baud(struct uart_hw *self, uint16_t baud){
    uint8_t k = (self->high_speed == 1) ? 4 : 16;
    uint8_t baudh = (uint8_t)((_XTAL_FREQ / k / baud) - 1) / 256;
    uint8_t baudl = (uint8_t)((_XTAL_FREQ / k / baud) - 1) % 256;
    if(self->mode & (1<<4)){
        // clamp baud for LIN and DMX mode
        baud = (self->high_speed) 
            ? ((baud > 0x7FFE) ? 0x7FFE : baud)
            : ((baud > 0x1FFE) ? 0x1FFE : baud);
    }
    self->baud = baud;
    U1BRGL = baud & 0x00FF;
    U1BRGH = baud & 0xFF00;
}

void uart_hw_enable(struct uart_hw const *self){
    U1CON1 |= (1 << 7);
}

void uart_hw_TX_enable(struct uart_hw const *self){
    U1CON0 |= (1 << 5);
}
void uart_hw_RX_enable(struct uart_hw const *self){
    U1CON0 |= (1 << 4);   
}

void uart_hw_disable(struct uart_hw const *self){
    U1CON0 &= ~(1 << 5);
}
void uart_hw_TX_disable(struct uart_hw const *self){
    U1CON0 &= ~(1 << 5);
}

void uart_hw_RX_disable(struct uart_hw const *self){
    U1CON1 &= ~(1 << 7);
}

void uart_hw_write_byte(struct uart_hw const *self, uint8_t byte);
void uart_hw_read_byte(struct uart_hw const *self, uint8_t *byte);

void uart_hw_write(struct uart_hw const *self, uint8_t *bytes, uint16_t tx_len, uint16_t* bytes_writen){
    uint16_t count = 0;
    bool btx = false;
    for(int16_t i = 0; i < tx_len; i++){
        btx = ring_buffer_enqueue(self->tx_buffer,&bytes[i]);
        if(btx){
            count++;
        }else{ break; }
    }
    *bytes_writen = count;
}

void uart_hw_read(struct uart_hw const *self, uint8_t *bytes, uint16_t* bytes_recived){
    uint16_t count = 0;
    uint8_t tmp;
    while(ring_buffer_dequeue(self->rx_buffer, &tmp)){
        bytes[count] = tmp;
        count++;
    }
    *bytes_recived = count;
}


void __interrupt(irq(0x30)) uart_tx_isr(){
    uint8_t tmp = 0;
    interrupt_clear(0x30);
    while(ring_buffer_dequeue(inst.tx_buffer, &tmp)){
        U1TXB = tmp;
        while(!tx_empty()){/*wait*/};
    }
}

void __interrupt(irq(0x31)) uart_rx_isr(){
    uint8_t tmp = 0;
    interrupt_clear(0x31);
    do{
        tmp = U1RXB;
    }while(ring_buffer_enqueue(inst.rx_buffer,&tmp));
}