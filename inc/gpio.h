/* 
 * File:   gpio.h
 * Author: Gard
 *
 * Created on 21 January 2026, 23:46
 */

#ifndef GPIO_H
#define	GPIO_H

#include <stdint.h>

#define IO_PORT_COUNT (3u)

typedef enum {
    IO_01, IO_02, IO_03, IO_04, IO_05, IO_06, IO_07, IO_08, IO_09, IO_10,
    IO_11, IO_12, IO_13, IO_14, IO_15, IO_16, IO_17, IO_18, IO_19, IO_20
} io_generic_e;

// remapping pins to more uasable names
typedef enum {
    IO_RA0 = IO_01, 
    IO_RA1, IO_RA2, IO_RA3, IO_RA4, IO_RA5, IO_RA6, IO_RA7,
    IO_RB0, IO_RB1, IO_RB2, IO_RB3, IO_RB4, IO_RB5, IO_RB6, IO_RB7,
    IO_RC0, IO_RC1, IO_RC2, IO_RC3, IO_RC4, IO_RC5, IO_RC6, IO_RC7 = IO_20,
}io_e;

typedef enum{
    IO_DIR_INPUT,
    IO_DIR_OUTPUT,
} io_dir_e;

typedef enum{
    IO_MODE_DIGITAL,
    IO_MODE_ANALOG,
}io_mode_e;

typedef enum{
    IO_MODE_PERIPHERAL_OUTPUT,
    IO_MODE_PERIPHERAL_INPUT,
    IO_MODE_PERIPHERAL_BIDIRECTIONAL,
} io_pps_mode_e;

typedef enum{
    IO_PULLUP_DISABLED,
    IO_PULLUP_ENABLED,
}io_pullup_e;

typedef enum{
    IO_OUTPUT_HIGH,
    IO_OUTPUT_LOW,
} io_output_e;

typedef struct {
    io_dir_e dir;
    io_mode_e mode;
    io_pullup_e pullup;
    //uint8_t peripheral;
    //io_pps_mode_e pps;
} io_config;


void io_set_direction(io_e io, io_dir_e direction);
void io_set_mode(io_e io, io_mode_e mode);
void io_set_pullup(io_e io, io_pullup_e pullup);
void io_select_peripheral(io_e io, io_dir_e dir ,uint8_t pps_reg);
void io_set_output(io_e io, io_output_e output);
void io_toggle_output(io_e io);
void pps_lock(void);
void pps_unlock(void);
#endif	/* GPIO_H */

