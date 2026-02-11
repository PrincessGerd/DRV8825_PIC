#ifndef UTMR_H
#define UTMR_H

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>


typedef enum {
    TU16_POLARITY_RISING_EDGE = 0x1, 
    TU16_POLARITY_FALLING_EDGE = 0x0
} utmr_polarity_e;

typedef enum {
    TU16_MODE_LEVEL_OUT = 0x1,
    TU16_MODE_PULSE_OUT = 0x0
}utmr_output_mode_e;

typedef enum {
    TU16_RW_MODE_COUNTER = 0x1,
    TU16_R_MODE_CAPTURE = 0x0
} read_mode_sel_e;

typedef enum {
    TU16_START_ERS_HIGH = 0x3,
    TU16_START_ERS_RISING_EDGE = 0x2,
    TU16_START_ERS_ANY_EDGE = 0x1,
    TU16_START_SW = 0x0,
} utmr_start_cond_e;

typedef enum {
    TU16_RESET_ON_MATCH = 0x3,
    TU16_RESET_START = 0x2,
    TU16_RESET_ERS_LOW = 0x1,
    TU16_RESET_DISABLED = 0x0,
} utmr_reset_cond_e;

typedef enum {
    TU16_STOP_ON_MATCH = 0x3,
    TU16_STOP_RISING_EDGE = 0x2,
    TU16_STOP_ANY_EDGE = 0x1,
    TU16_STOP_SW = 0x0,
} utmr_stop_cond_e;

/*---------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------*/

typedef struct{
    uint8_t clock_source;
    uint8_t reset_source;
    utmr_polarity_e clk_polarity;
    utmr_output_mode_e output_mode;
    utmr_start_cond_e start_cond;
    utmr_stop_cond_e  stop_cond;
    utmr_reset_cond_e reset_cond;
    bool sync;
    bool invert_output;
    bool invert_reset;
}utmr_config_t;

typedef void (*utmr_callback_t)(void);

/*---------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------*/

struct utmr_hw;
void utmr_init(const struct utmr_hw* self, const utmr_config_t* config);
void utmr_enable(const struct utmr_hw* self);
void utmr_disable(const struct utmr_hw* self);
void utmr_clear(const struct utmr_hw* self);
void utmr_set_prescaler(const struct utmr_hw* self, uint8_t value);
void utmr_set_counter(const struct utmr_hw* self, uint16_t value);
void utmr_set_period(const struct utmr_hw* self, uint16_t value);
void utmr_set_callback(const struct utmr_hw* self, utmr_callback_t callback);
#endif