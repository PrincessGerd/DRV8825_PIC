/* 
 * File:   ccp.h
 * Author: Gard
 *
 * Created on January 23, 2026, 12:32 AM
 */

#ifndef CCP_H
#define	CCP_H
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>
#include "registers.h"
#define STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)


#define CCP_REG(module_num, offset) \
    (volatile uint8_t*)(CCP_BASE_ADDRESS + (module_num * CCP_OFFSET_NEXT_CCP) + offset)

#define CCPXCON(timer_num)   CCP_REG(timer_num, CCPCON_OFFSET)
#define CCPXCAP(timer_num)   CCP_REG(timer_num, CCPCAP_OFFSET)
#define CCPRXL(timer_num)    CCP_REG(timer_num, CCPCCPRL_OFFSET)
#define CCPRXH(timer_num)    CCP_REG(timer_num, CCPCCPRH_OFFSET)

#define SET_MODE(mode,module)       (SET_REG_FIELD(CCPXCON(module),CCP_MODE_SHIFT,CCP_MODE_MASK,mode))
#define SET_ALIGNMENT(align,module) (SET_REG_FIELD(CCPXCON(module),CCP_FMT_SHIFT,CCP_FMT_MASK,align))
#define SET_TRIGGER(trig,module)    (SET_REG_FIELD(CCPXCON(module),CCP_CTS_SHIFT,CCP_CTS_MASK,trig))

// TODO: found a cleaner way to do this. refactor
/*
    INSTANCE_DEFINE(xxx,0);
    config_t cfg = {
    ...}
    init(&xxx) and so on;
*/

typedef enum {
    CCP_TRIG_UNUSED = 0x0,
    CCP_TRIG_PPSPIN = 0x0,
    CCP_TRIG_IOCINT = 0x1,
    CCP_TRIG_IOCSR = 0x2,
    CCP_TRIG_CLC1 = 0x3,
    CCP_TRIG_CLC2 = 0x4,
    CCP_TRIG_CLC3 = 0x5,
    CCP_TRIG_CLC4 = 0x6,
}ccp_trigger_src_e;

typedef enum {
    CCP_ALIGN_UNUSED = 0x0,
    CCP_RIGHT_ALIGNED = 0x0,
    CCP_LEFT_ALIGNED = 0x1,
}ccp_value_alignment_e;

typedef enum {
    CPP_MODE_DISABLED = 0x00,
    CCP_MODE_COMPARE_TOGGLE_CLEAR = 0x01,
    CCP_MODE_COMPARE_TOGGLE = 0x2,
    CCP_MODE_CAPTURE_FALLING_EDGE= 0x04,
    CCP_MODE_CAPTURE_RISING_EDGE = 0x05,
    CCP_MODE_COMPARE_PULSE_OUT = 0x0A,
    CCP_MODE_PWM = 0x0F
}ccp_mode_e;

typedef struct {
    ccp_mode_e              mode;
    ccp_trigger_src_e       trigger;
    ccp_value_alignment_e   align;
}ccp_config_t;

typedef void (*ccp_callback_t)(void);
ccp_callback_t ccp_callbacks[CCP_NUM_MODULES] = {0,0};

typedef struct {
    void(*init)(void* self);
    void(*enable)(void* self);
    void(*disable)(void* self);
    void(*set_register)(void* self, uint16_t value);
    void(*set_callback)(void* self, ccp_callback_t callback_func);
    ccp_config_t config;
    uint8_t module_num;
} ccp_instance_t;

void ccp_init(void* self);
void ccp_enable(void* self);
void ccp_disable(void* self);
void ccp_set_register(void* self, uint16_t value);
void ccp_set_callback(void* self, ccp_callback_t callback_func);

void ccp_enable_interrupt(void* self);
void ccp_disable_interrupt(void* self);



#define CCP_MODULE_1 0
#define CCP_MODULE_2 1

#define CCP_DECLARE(name, _module, _mode, _trigger, _align) \
    STATIC_ASSERT((_module <= CCP_NUM_MODULES-1) && (_module >= 0), "Invalid module num selected"); \
    bool is_initialized_ccp_##_module = false; \
    static ccp_instance_t name = { \
        .init = ccp_init, \
        .enable = ccp_enable, \
        .disable = ccp_disable, \
        .set_register = ccp_set_register, \
        .set_callback = ccp_set_callback, \
        .config = { \
            .mode = _mode, \
            .trigger = _trigger, \
            .align = _align \
        }, \
        .module_num = _module \
    };
//void init_ccp(ccp_config_t *config);
//inline void enable_ccp();
//inline void disable_ccp();
//void set_ccp_register(uint16_t value);
//
//void(*ccp_callback)(void);
//void set_ccp_callback(void(*func)(void));
//inline void enable_ccp_interrupt();
//inline void disable_ccp_interrupt();
//void ccp_isr(void);
#endif	/* CCP_H */

