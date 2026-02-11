#include "../pinmux.h"
#include <xc.h>

typedef enum {
    PPS_IN_CLCIN0, PPS_IN_CLCIN1, PPS_IN_CLCIN2, PPS_IN_CLCIN3, PPS_IN_CLCIN0R,
    PPS_IN_U1RX, PPS_IN_U1CTS, PPS_IN_U2RX, PPS_IN_U2CTS,
    PPS_IN_SPI_SCK, PPS_IN_SPI_SDI, PPS_IN_SPI_SS,
    PPS_IN_I2C1_SCL, PPS_IN_I2C1_SDA,
    PPS_IN_INT0, PPS_IN_INT1, PPS_IN_INT2,
    PPS_IN_T0CKI, PPS_IN_T1CKI, PPS_IN_T1G, PPS_IN_T2IN, PPS_IN_T4IN,
    PPS_IN_TUIN1, PPS_IN_TUIN2,
    PPS_IN_CCP1, PPS_IN_CCP2,
    PPS_IN_PWMIN0, PPS_IN_PWMIN1, PPS_IN_PWMxERS, PPS_IN_PWM2ERS,
    PPS_IN_CWG1, PPS_IN_ADACT,
    PPS_IN_COUNT
} pps_input_t;

typedef enum{
    PPS_RA0, PPS_RA1, PPS_RA2, PPS_RA3, PPS_RA4, PPS_RA5, PPS_RA6, PPS_RA7,
    PPS_RB0, PPS_RB1, PPS_RB2, PPS_RB3, PPS_RB4, PPS_RB5, PPS_RB6, PPS_RB7, 
    PPS_RC0, PPS_RC1, PPS_RC2, PPS_RC3, PPS_RC4, PPS_RC5, PPS_RC6, PPS_RC7,
    PPS_OUT_COUNT
}pps_output_t;

static volatile uint8_t* const PPS_output[PPS_OUT_COUNT] = {
    [PPS_RA0] = &RA0PPS,
    [PPS_RA1] = &RA1PPS,
    [PPS_RA2] = &RA2PPS,
    [PPS_RA3] = 0,
    [PPS_RA4] = &RA4PPS,
    [PPS_RA5] = &RA5PPS,
    [PPS_RA6] = 0,
    [PPS_RA7] = 0,

    [PPS_RB0] = 0,
    [PPS_RB1] = 0,
    [PPS_RB2] = 0,
    [PPS_RB3] = 0,
    [PPS_RB4] = 0,
    [PPS_RB5] = &RB5PPS,
    [PPS_RB6] = &RB6PPS,
    [PPS_RB7] = &RB7PPS,

    [PPS_RC0] = &RC0PPS,
    [PPS_RC1] = &RC1PPS,
    [PPS_RC2] = 0,
    [PPS_RC3] = &RC3PPS,
    [PPS_RC4] = &RC4PPS,
    [PPS_RC5] = &RC5PPS,
    [PPS_RC6] = &RC6PPS,
    [PPS_RC7] = &RC7PPS,
};

static volatile uint8_t* const PPS_input[PPS_IN_COUNT] = {
    [PPS_IN_CLCIN0]  = &CLCIN0PPS,
    [PPS_IN_CLCIN1]  = &CLCIN1PPS,
    [PPS_IN_CLCIN2]  = &CLCIN2PPS,
    [PPS_IN_CLCIN3]  = &CLCIN3PPS,

    [PPS_IN_U1RX]  = &U1RXPPS,
    [PPS_IN_U1CTS] = &U1CTSPPS,
    [PPS_IN_U2RX]  = &U2RXPPS,
    [PPS_IN_U2CTS] = &U2CTSPPS,


    [PPS_IN_I2C1_SCL] = &I2C1SCLPPS,
    [PPS_IN_I2C1_SDA] = &I2C1SDAPPS,

    [PPS_IN_INT0] = &INT0PPS,
    [PPS_IN_INT1] = &INT1PPS,
    [PPS_IN_INT2] = &INT2PPS,

    [PPS_IN_T0CKI] = &T0CKIPPS,
    [PPS_IN_T1CKI] = &T1CKIPPS,
    [PPS_IN_T1G]   = &T1GPPS,
    [PPS_IN_T2IN]  = &T2INPPS,
    [PPS_IN_T4IN]  = &T4INPPS,

    [PPS_IN_TUIN1] = &TUIN1PPS,

    [PPS_IN_CCP1] = &CCP1PPS,
    [PPS_IN_CCP2] = &CCP2PPS,

    [PPS_IN_PWMIN0] = &PWMIN0PPS,
    [PPS_IN_PWMIN1] = &PWMIN1PPS,
    [PPS_IN_PWM2ERS] = &PWM2ERSPPS,

    [PPS_IN_CWG1] = &CWG1PPS,
    [PPS_IN_ADACT] = &ADACTPPS,
};

#define PORT_COUNT (3u)
#define IO_PORT_OFFSET (3u)
#define IO_PORT_MASK (0x3u << IO_PORT_OFFSET)
#define IO_PIN_MASK (0x7u)

static volatile uint8_t* const pic18f_wpu[PORT_COUNT] = {&WPUA, &WPUB, &WPUC};
static volatile uint8_t* const pic18f_odc[PORT_COUNT] = {&ODCONA, &ODCONB, &ODCONC};

static inline uint8_t io_port(uint8_t pin){
    return (uint8_t)(pin & IO_PORT_MASK) >> IO_PORT_OFFSET; }

static inline uint8_t io_pin_bit(uint8_t pin){
    return (uint8_t)(1 << (pin & IO_PIN_MASK)); }

static inline void pps_unlock(void) {
    INTCON0bits.GIE = 0;    // Suspend interrupts
    PPSLOCK = 0x55;         // Required sequences
    PPSLOCK = 0xAA;
    PPSLOCKbits.PPSLOCKED = 0;
    INTCON0bits.GIE = 1;    // Restore interrupts
}

static inline void pps_lock(void) {
    INTCON0bits.GIE = 0;    // Suspend interrupts
    PPSLOCK = 0x55;         // Required sequences
    PPSLOCK = 0xAA;
    PPSLOCKbits.PPSLOCKED = 1;
    INTCON0bits.GIE = 1;    // Restore interrupts
}

void pinmux_set_output(struct pinmux* self, uint8_t pin, uint8_t alt_func){
    pps_unlock();
    *PPS_output[pin] = alt_func;
    pps_lock();
}

void pinmux_set_input(struct pinmux* self, uint8_t pin, uint8_t alt_func){
    pps_unlock();
    *PPS_input[alt_func] = (uint8_t)pin;
    pps_lock();
}

void pinmux_reset(struct pinmux* self, uint8_t pin, uint8_t alt_func){
    pps_unlock();
    *PPS_output[pin]  = 0x00;
    *PPS_input[alt_func] = 0x00;
    pps_lock();
}

void pinmux_resistor(struct pinmux* self, uint8_t pin, pinmux_resistor_e resistor){
    const uint8_t port = io_port(pin);
    const uint8_t bit =  io_pin_bit(pin);

    switch (resistor)
    {
    case PM_RESISTOR_PULLUP:
        *pic18f_wpu[port] |= bit;
        *pic18f_odc[port] &= ~bit;
        break;
    case PM_RESISTOR_PULLDOWN:
        *pic18f_odc[port] |= bit;
        *pic18f_wpu[port] &= ~bit;
        break;
    case PM_RESISTOR_DISABLE:
        *pic18f_wpu[port] &= ~bit;
        *pic18f_odc[port] &= ~bit;
        break;
    default:
        break;
    }
}