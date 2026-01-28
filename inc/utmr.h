#ifndef UTMR_H
#define UTMR_H

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "registers.h"


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


#define UTMR_REG(module_num, offset) \
    (volatile uint8_t*)(TU16_BASE_ADDRESS + (module_num * TU16_OFFSET_NEXT_MODULE) + offset)

typedef struct{ 
    volatile uint8_t* CON0;
    volatile uint8_t* CON1;
    volatile uint8_t* HLT;
    volatile uint8_t* PS;
    volatile uint8_t* TMR;
    volatile uint8_t* CR;
    volatile uint8_t* PR;
    volatile uint8_t* CLK;
    volatile uint8_t* ERS;
} utmr_regs_t;

// wont change during opperation
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
utmr_callback_t utmr_callbacks[2] = {0,0};

/*---------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------*/

typedef struct { // add more functionality as needed. cant be bothered rn
    void(*init)(void* self, const utmr_config_t* config);
    void(*enable)(void* self);
    void(*disable)(void* self);
    void(*set_prescaler)(void* self, uint8_t value);
    void(*set_counter)(void* self, uint16_t value);
    void(*set_period)(void* self, uint16_t value);
    void(*set_mode)(void* self, uint8_t mode);
    void(*clear)(void* self);
#ifdef __DEBUG
    void(*start)(void* self);    // for emulating a start condition
    void(*stop)(void* self);     // for emulating a stop condition
#endif    
    void(*set_callback)(void* self, utmr_callback_t callback);
    utmr_regs_t regs;
    utmr_config_t config;
    uint8_t module_num;
}utmr_instance_t;

#define DECLARE_UTMR_INSTANCE(name,module) \
    bool is_initialized_utmr##module = false; \
    static utmr_instance_t name = { \
        .init           = utmr_init_impl, \
        .enable         = utmr_enable_impl, \
        .disable        = utmr_disable_impl, \
        .set_prescaler  = utmr_set_prescaler_impl, \
        .set_counter    = utmr_set_counter_impl, \
        .set_period     = utmr_set_period_impl, \
        .set_mode       = utmr_set_mode_impl, \
        .clear          = utmr_clear_impl, \
        .module_num     = module, \
        .config         = {}, \
        .regs = {\
            .CON0 = UTMR_REG(module,TU16_CON0_OFFSET),\
            .CON1 = UTMR_REG(module,TU16_CON1_OFFSET),\
            .HLT  = UTMR_REG(module,TU16_HLT_OFFSET),\
            .PS   = UTMR_REG(module,TU16_PS_OFFSET),\
            .TMR  = UTMR_REG(module,TU16_TMR_OFFSET),\
            .CR   = UTMR_REG(module,TU16_CR_OFFSET),\
            .PR   = UTMR_REG(module,TU16_PR_OFFSET),\
            .CLK  = UTMR_REG(module,TU16_CLK_OFFSET),\
            .ERS  = UTMR_REG(module,TU16_ERS_OFFSET),\
        } \
    };

void utmr_init_impl(void* self, const utmr_config_t* config);
void utmr_enable_impl(void* self);
void utmr_disable_impl(void* self);
void utmr_set_prescaler_impl(void* self, uint8_t value);
void utmr_set_counter_impl(void* self, uint16_t value);
void utmr_set_period_impl(void* self, uint16_t value);
void utmr_set_mode_impl(void* self, uint8_t mode);
void utmr_clear_impl(void* self);
void utmr_set_callback(void* self, utmr_callback_t callback);
#endif