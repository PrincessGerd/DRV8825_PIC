/* 
 * File:   timer.h
 * Author: Gard
 *
 * Created on January 20, 2026, 3:44 PM
 */

#ifndef TMR2H
#define	TMR2H

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "registers.h"

#define STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)

typedef enum {
    TMR2_MODE_FREE_RUNNING = 0b00,
    TMR2_MODE_ONE_SHOT     = 0b01,
    TMR2_MODE_MONOSTABLE   = 0b10
} timer_mode_e;

// For FREE_RUNNING mode:
typedef enum {
    TMR2_FR_SW_GATE       = 0b000,
    TMR2_FR_HW_GATE_HIGH  = 0b001,
    TMR2_FR_HW_GATE_LOW   = 0b010,
    TMR2_FR_EDGE_RESET_ANY     = 0b011,
    TMR2_FR_EDGE_RESET_RISING  = 0b100,
    TMR2_FR_EDGE_RESET_FALLING = 0b101,
    TMR2_FR_LEVEL_RESET_LOW    = 0b110,
    TMR2_FR_LEVEL_RESET_HIGH   = 0b111
} timer_free_running_submode_e;

// For ONE_SHOT mode:
typedef enum {
    TMR2_OS_SOFTWARE_START     = 0b000,
    TMR2_OS_RISING_EDGE_START  = 0b001,
    TMR2_OS_FALLING_EDGE_START = 0b010,
    TMR2_OS_ANY_EDGE_START     = 0b011,
    TMR2_OS_EDGE_START_RST_RISING  = 0b100,
    TMR2_OS_EDGE_START_RST_FALLING = 0b101,
    TMR2_OS_EDGE_START_LOW_RST     = 0b110,
    TMR2_OS_EDGE_START_HIGH_RST    = 0b111
} timer_one_shot_submode_e;

// For MONOSTABLE mode:
typedef enum {
    TMR2_MS_SOFTWARE_START     = 0b000,
    TMR2_MS_RISING_EDGE_START  = 0b001,
    TMR2_MS_FALLING_EDGE_START = 0b010,
    TMR2_MS_ANY_EDGE_START     = 0b011,
    // Only valid for monostable
    TMR2_MS_HIGH_START_LOW_RST = 0b110,
    TMR2_MS_LOW_START_HIGH_RST = 0b111
} timer_monostable_submode_e;

typedef enum {
    TMR2_CLK_TMR2OSC_CLC4_OUT     = 0xD,
    TMR2_CLK_CLC3_OUT     = 0xC,
    TMR2_CLK_CLC2_OUT     = 0xB,
    TMR2_CLK_CLC1_OUT     = 0xA,
    TMR2_CLK_CLKREF_OUT   = 0x9,
    TMR2_CLK_EXTOSC       = 0x8,
    TMR2_CLK_SOSC         = 0x7,
    TMR2_CLK_MFINTOSC_32  = 0x6,
    TMR2_CLK_MFINTOSC_500 = 0x5,
    TMR2_CLK_LFINTOSC     = 0x4,
    TMR2_CLK_HFINTOSC     = 0x3,
    TMR2_CLK_FOSC         = 0x2,
    TMR2_CLK_FOSC4        = 0x1,
    TMR2_CLK_PPS_SEL_OSC  = 0x00
}timer_clk_src_e;

typedef enum {
    TMR2_RST_U2TX_EDGE           = 0x10,
    TMR2_RST_U2RX_EDGE           = 0x0F,
    TMR2_RST_U1TX_EDGE           = 0x0E,
    TMR2_RST_U1RX_EDGE           = 0x0D,
    TMR2_RST_CLC4_OUT            = 0x0C,
    TMR2_RST_CLC3_OUT            = 0x0B,
    TMR2_RST_CLC2_OUT            = 0x0A,
    TMR2_RST_CLC1_OUT            = 0x09,
    TMR2_RST_PWM2S1P2_OUT        = 0x08,
    TMR2_RST_PWM2S1P1_OUT        = 0x07,
    TMR2_RST_PWM1S1P2_OUT        = 0x06,
    TMR2_RST_PWM1S1P1_OUT        = 0x05,
    TMR2_RST_CCP2_OUT            = 0x04,
    TMR2_RST_CCP1_OUT            = 0x03,
    TMR2_RST_TMR4_POSTSCALER_OUT = 0x02,
    TMR2_RST_TMR2_POSTSCALER_OUT = 0x01,
    TMR2_RST_PPS_SEL             = 0x00
} timer_reset_src_e;

typedef enum {
    TMR2_PRESCALER_128   = 0x7,
    TMR2_PRESCALER_64    = 0x6,
    TMR2_PRESCALER_32    = 0x5,
    TMR2_PRESCALER_16    = 0x4,
    TMR2_PRESCALER_8     = 0x3,
    TMR2_PRESCALER_4     = 0x2,
    TMR2_PRESCALER_2     = 0x1,
    TMR2_PRESCALER_1     = 0x0
} timer_prescaler_e;

typedef enum {
    TMR2_POSTSCALER_16 = 0xF,
    TMR2_POSTSCALER_15 = 0xE,
    TMR2_POSTSCALER_14 = 0xD,
    TMR2_POSTSCALER_13 = 0xC,
    TMR2_POSTSCALER_12 = 0xB,
    TMR2_POSTSCALER_11 = 0xA,
    TMR2_POSTSCALER_10 = 0x9,
    TMR2_POSTSCALER_9  = 0x8,
    TMR2_POSTSCALER_8  = 0x7,
    TMR2_POSTSCALER_7  = 0x6,
    TMR2_POSTSCALER_6  = 0x5,
    TMR2_POSTSCALER_5  = 0x4,
    TMR2_POSTSCALER_4  = 0x3,
    TMR2_POSTSCALER_3  = 0x2,
    TMR2_POSTSCALER_2  = 0x1,
    TMR2_POSTSCALER_1  = 0x0,           
} timer_postscaler_e;

typedef enum {
    TMR2_FALLING_EDGE = 0,
    TMR2_RISING_EDGE  = 1
} timer_polarity_e;

/*---------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------*/

#define TIMER_REG_ADDR(module_num, offset) \
    ((volatile uint8_t*)(TMR_BASE_ADDRESS + (module_num * TMR_OFFSET_NEXT_MODULE) + (offset)))

typedef struct {
    volatile uint8_t* TMR;
    volatile uint8_t* PR;
    volatile uint8_t* CON;
    volatile uint8_t* HLT;
    volatile uint8_t* CLKCON;
    volatile uint8_t* RST;
} timer_regs_t;

#define TMR2MODE(group, sub) (((uint8_t)group << 3) | (uint8_t)sub)
typedef struct {
    timer_mode_e mode;
    union { // One submode per timer config
        timer_free_running_submode_e    free_running;
        timer_one_shot_submode_e        one_shot;
        timer_monostable_submode_e      monostable;
        uint8_t raw;
    } submode;
} timer_mode_t;

typedef struct {
    timer_mode_t        mode;
    timer_clk_src_e     clk_src;
    timer_reset_src_e   reset_src;
    timer_polarity_e    polarity; 
    timer_prescaler_e   prescaler;
    timer_postscaler_e  postscaler;
    bool clk_sync;
    bool prescaler_sync;
} timer_config_t;

typedef void(*timer_callback_t)(void);
timer_callback_t timer_callbacks[TMR_NUM_MODULES];

/*---------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------*/

typedef struct {
    void(*init)(void* self, const timer_config_t* config);
    void(*enable)(void* self);
    void(*disable)(void* self);
    void(*set_count)(void* self, uint8_t value);
    void(*set_period)(void* self, uint8_t value);
    void(*set_callback)(void* self, timer_callback_t callback);
    timer_regs_t regs;
    timer_config_t config;
    uint8_t module_num;
} timer_instance_t;

#define TMR_MODULE_A 0 // tmr 2
#define TMR_MODULE_B 1 // tmr 4

#define DECLARE_TIMER(name, module) \
    bool is_initialized##module = false; \
    static timer_instance_t name = { \
        .init           = timer_init_impl, \
        .enable         = timer_enable_impl, \
        .disable        = timer_disable_impl, \
        .set_count      = timer_set_count_impl, \
        .set_period     = timer_set_period_impl, \
        .set_callback   = timer_set_callback_impl, \
        .module_num     = module, \
        .config = {}, \
        .regs = { \
            .TMR = TIMER_REG_ADDR(module, TMR_TMR_OFFSET), \
            .PR = TIMER_REG_ADDR(module, TMR_PR_OFFSET), \
            .CON = TIMER_REG_ADDR(module, TMR_CON_OFFSET), \
            .HLT = TIMER_REG_ADDR(module, TMR_HLT_OFFSET), \
            .CLKCON = TIMER_REG_ADDR(module, TMR_CLKCON_OFFSET), \
            .RST = TIMER_REG_ADDR(module, TMR_RST_OFFSET), \
        }\
    };

// timer 2
void timer_init_impl(void* self, const timer_config_t* config);
inline void timer_enable_impl(void* self);
inline void timer_disable_impl(void* self);
inline void timer_set_count_impl(void* self,uint8_t count);
inline void timer_set_period_impl(void* self,uint8_t count);
void timer_set_callback_impl(void* self, timer_callback_t callback);

inline void timer2_enable_interrupt();
inline void timer2_disable_interrupt();

void timer2_isr(void);
#endif	/* TMR2H */

