#include "gpio.h"
#include <xc.h>

static volatile uint8_t* const io_port_register[IO_PORT_COUNT]              = {&PORTA,  &PORTB,  &PORTC};
static volatile uint8_t* const io_latch_register[IO_PORT_COUNT]             = {&LATA,   &LATB,   &LATC};
static volatile uint8_t* const io_direction_register[IO_PORT_COUNT]         = {&TRISA,  &TRISB,  &TRISC};
static volatile uint8_t* const io_analouge_select_register[IO_PORT_COUNT]   = {&ANSELA, &ANSELB, &ANSELC};
static volatile uint8_t* const io_pullup_register[IO_PORT_COUNT]            = {&WPUA,   &WPUB,   &WPUC};

//static volatile uint8_t* const io_int_regs_pos[IO_PORT_COUNT]               = {&IOCAP,&IOCBP,&IOCCP};
//static volatile uint8_t* const io_int_regs_neg[IO_PORT_COUNT]               = {&IOCAN,&IOCBN,&IOCCN};
//static volatile uint8_t* const io_int_regs_flag[IO_PORT_COUNT]              = {&IOCAF,&IOCBF,&IOCCF};

// [7:5] zeros, [4:3] port, [2:0] pin
#define IO_PORT_OFFSET (3u)
#define IO_PORT_MASK (0x3u << IO_PORT_OFFSET)
#define IO_PIN_MASK (0x7u)

static inline uint8_t io_port(io_e io){
    return (uint8_t)(io & IO_PORT_MASK) >> IO_PORT_OFFSET;
}

static inline uint8_t io_pin_bit(io_e io){
    return (uint8_t)(1 << (io & IO_PIN_MASK));
}

void io_set_direction(io_e io, io_dir_e direction){
    const uint8_t port = io_port(io);
    const uint8_t pin = io_pin_bit(io);
    switch (direction) {
        case IO_DIR_INPUT:
            *io_direction_register[port] &= ~pin;
            break;
        case IO_DIR_OUTPUT:
            *io_direction_register[port] |= pin;
            break;
    }
}

void io_set_mode(io_e io, io_mode_e mode){
    const uint8_t port = io_port(io);
    const uint8_t pin_bit = io_pin_bit(io);
    switch (mode) {
        case IO_MODE_DIGITAL:
            *io_analouge_select_register[port] &= ~pin_bit;
            break;
        case IO_MODE_ANALOG:
            *io_analouge_select_register[port] |= pin_bit;
            break;
    }
}

void io_set_pullup(io_e io, io_pullup_e pullup) {
    const uint8_t port = io_port(io);
    const uint8_t pin = io_pin_bit(io);
    switch (pullup) {
        case IO_PULLUP_DISABLED:
            *io_pullup_register[port] &= ~pin;
            break;
        case IO_PULLUP_ENABLED:
            //INTCON2bits.nRBPU = 0;
            *io_pullup_register[port] |= pin;
            break;
    }
}

void io_set_output(io_e io, io_output_e output){
    const uint8_t port = io_port(io);
    const uint8_t pin = io_pin_bit(io);
    switch (output) {
        case IO_OUTPUT_HIGH:
            *io_latch_register[port] |= pin;
            break;
        case IO_OUTPUT_LOW:
            *io_latch_register[port] &= ~pin;
            break;
    }
}

void io_toggle_output(io_e io) {
    const uint8_t port = io_port(io);
    const uint8_t pin = io_pin_bit(io);
    *io_latch_register[port] ^= pin;
}

// must call pps_lock after
void pps_unlock(void) {
    INTCON0bits.GIE = 0;    // Suspend interrupts
    PPSLOCK = 0x55;         // Required sequences
    PPSLOCK = 0xAA;
    PPSLOCKbits.PPSLOCKED = 0;
    INTCON0bits.GIE = 1;    // Restore interrupts
}

void pps_lock(void) {
    INTCON0bits.GIE = 0;    // Suspend interrupts
    PPSLOCK = 0x55;         // Required sequences
    PPSLOCK = 0xAA;
    PPSLOCKbits.PPSLOCKED = 1;
    INTCON0bits.GIE = 1;    // Restore interrupts
}

void io_pps_clear(io_e io, uint8_t* pps_reg){
    pps_unlock();
    *((volatile uint8_t*)pps_reg) = 0x00;
    pps_lock();
}

void io_pps_set_input(io_e io, uint8_t* pps_reg){
    pps_unlock();
    *((volatile uint8_t*)pps_reg) = (uint8_t)io;
    pps_lock();
}

void io_pps_set_output(io_e io, uint8_t* pps_reg){
    pps_unlock();
    (*(volatile uint8_t*)io) = (volatile uint8_t)*pps_reg;
    pps_lock();
}

void io_set_pps(io_e io, io_pps_mode_e mode, uint8_t peripheral){
    switch(mode){
        case IO_MODE_PERIPHERAL_OUTPUT:
            io_pps_set_output(io, &peripheral);
            break;
        case IO_MODE_PERIPHERAL_INPUT:
            io_pps_set_input(io, &peripheral);
            break;
        case IO_MODE_PERIPHERAL_BIDIRECTIONAL:
            io_pps_clear(io,&peripheral);
            io_pps_set_output(io, &peripheral);
            io_pps_set_input(io, &peripheral);
            break;
    }
}


void io_configure(io_e io, io_config* config){
    //io_set_pps(io, config->pps, config->peripheral);
    io_set_direction(io, config->dir);
    io_set_pullup(io, config->pullup);
    io_set_mode(io, config->mode);
}
