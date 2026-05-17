#include "../inc/uart_hw.h"
#include "../core/interrupts.h"
#include "../inc/ringbuffer.h"
#include "../core/serial_logger.h"
#include <stdbool.h>
#include <stdint.h>
#include <xc.h>

typedef enum{
    UART_ERROR_NONE = 0,
    UART_ERROR_OVERRUN,
    UART_ERROR_FRAMING,
    UART_ERROR_CHECKSUM
} uart_error_e;

#define UART_BUFFER_SIZE (24u)
typedef struct {
    uint8_t* data;
    unsigned int len;
    unsigned int offset;
    bool active;
} uart_tx_work_t;
static uart_tx_work_t tx_job;

RING_BUFFER_DECLARE(rx_buffer, UART_BUFFER_SIZE, uint8_t);
RING_BUFFER_DECLARE(tx_buffer, UART_BUFFER_SIZE, uint8_t);
uart_error_e fault_state;

#define UART_TX_ENABLE_BIT 5
#define MIN(a,b) (a < b ? a : b)


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

void uart_hw_enable(void){
    U1CON1 |= (1 << 7);
}

void uart_hw_TX_enable(void){
    U1CON0 |= (1 << 5);
}
void uart_hw_RX_enable(void){
    U1CON0 |= (1 << 4);   
}

void uart_hw_disable(void){
    U1CON0 &= ~(1 << 5);
}
void uart_hw_TX_disable(void){
    U1CON0 &= ~(1 << 5);
}

void uart_hw_RX_disable(void){
    U1CON1 &= ~(1 << 7);
}

void uart_hw_init(const uart_hw_config_t* config){
    fault_state = UART_ERROR_NONE;

    U1CON1 = 0;
    U1CON0 ^= U1CON0;
    U1CON2 ^- U1CON2;

    U1CON0 = (
        (config->high_speed << 7) | 
        (config->auto_baud << 6)  | 
        (config->mode << 0));
        
    U1CON2 = (
        (config->run_during_overflow << 7) | 
        (config->invert_recive << 6)       | 
        (config->stop_bits << 4)           | 
        (config->use_checksum << 3)        | 
        (config->invert_transmit << 2)     | 
        (config->flow_ctrl << 0));  

    if(!config->auto_baud){
        uart_hw_set_baud(config->baud);
    }
}

void uart_hw_set_baud(uint16_t baud){
    bool high_speed = U1CON0 & (1 << 7);
    uint8_t k = (high_speed == 1) ? 4 : 16;
    uint8_t baudh = (uint8_t)((_XTAL_FREQ / k / baud) - 1) / 256;
    uint8_t baudl = (uint8_t)((_XTAL_FREQ / k / baud) - 1) % 256;
    if(U1CON0 & (1<<4)){
        // clamp baud for LIN and DMX mode
        baud = (high_speed) 
            ? ((baud > 0x7FFE) ? 0x7FFE : baud)
            : ((baud > 0x1FFE) ? 0x1FFE : baud);
    }
    U1BRGL = baud & 0x00FF;
    U1BRGH = baud & 0xFF00;
}


void uart_hw_write_polling(uint8_t *bytes, uint8_t tx_len, uint8_t* bytes_writen){
    uint16_t count = 0;
    for(int16_t i = 0; i < tx_len; i++){
        U1TXB = bytes[i];
        while(!(U1FIFO & (1 << 5))){/*wait for TXBE*/}
        count++;
    }
    *bytes_writen = count;
}


static void uart_tx_callback(void){
    SLOG_ASSERT(tx_job.active && (tx_job.offset < tx_job.len))
    unsigned int free_space =
        UART_BUFFER_SIZE - tx_buffer->count;
    if(!free_space) { return; }    

    unsigned int remaining = tx_job.len - tx_job.offset;
    unsigned int chunk = MIN(remaining, free_space);

    ring_buffer_enqueue_chunk(
        tx_buffer, 
        &tx_job.data[tx_job.offset], 
        chunk);
    
    tx_job.offset += chunk;
}

void uart_hw_write(uint8_t *bytes, unsigned int len){
    if(bytes == NULL || len == 0){ return;}

    interrupt_disable(0x30);
    if(tx_job.active){
        interrupt_enable(0x30);
        return;
    }

    tx_job.data = bytes;
    tx_job.len  = len;
    tx_job.offset = 0;
    tx_job.active = true;

    uart_tx_callback();

    if (!(U1CON0 & (1 << UART_TX_ENABLE_BIT))) {
        U1CON0 |= (1 << UART_TX_ENABLE_BIT);
        uint8_t first = 0;
        ring_buffer_dequeue(tx_buffer, &first);
        U1TXB = first;
    }
    interrupt_enable(0x30);
}

void uart_hw_read(uint8_t* byte){
    SLOG_ASSERT(!ring_buffer_is_empty(rx_buffer));
    ring_buffer_dequeue(rx_buffer, byte);
}

void __interrupt(irq(0x30)) uart_tx_isr(){
    SLOG_ASSERT(!ring_buffer_is_empty(tx_buffer));
    interrupt_clear(0x30);
    
    uint8_t byte = 0;
    ring_buffer_dequeue(tx_buffer, &byte);
    U1TXB = byte;
    
    if(ring_buffer_is_empty(tx_buffer)){
        U1CON0 &= ~(1 << 5); // disable transmit
        tx_job.active = false;
    } else if(tx_job.active && tx_buffer->count < 2){
        uart_tx_callback();
    }
}

void __interrupt(irq(0x31)) uart_rx_isr(){
    SLOG_ASSERT(!ring_buffer_is_full(rx_buffer));
    interrupt_clear(0x31);
    uint8_t byte = U1RXB;
    ring_buffer_enqueue(rx_buffer, &byte);
}