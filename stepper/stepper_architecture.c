#include "stepper.h"
#include "stepper_device.h"
// core
#include "../core/types.h"
#include "../core/task_manager.h"
#include "../core/gpio.h"
#include "../core/system.h"
#include "../core/interrupts.h"
// hw
#include "../core/hw/inc/dma_hw.h"
#include "../core/hw/inc/pwm_hw.h"
//
#include <stdint.h>
#include <stdbool.h>
#include <xc.h>

#define mod(a,b) (a & (b-1u))
static const pwm_hw_config_t pwm_config = {
    .out1_polarity_low = false,
    .out2_polarity_low = false,
    .push_pull_mode = false,
    .mode = PWMx_MODE_COMPARE_TOGGLE,
    .clk_src = PWMx_CLK_FOSC
};

typedef enum{
    STEPPER_STATE_ACCELERATE,
    STEPPER_STATE_CONSTANT_SPEED,
    STEPPER_STATE_DEACCELERATE,
    STEPPER_STATE_IDLE
}motion_states_e;

#define BUFFER_SIZE 32

struct stepper{
    task_t              super;
    event_t             evt;
    stepper_device_t    device;
    
    struct pwm_hw*      pwm_module;
    struct dma_hw*      dma_module;
        
    uint32_t tick_frequency;
    uint16_t step_count;
    uint16_t accel_steps;
    uint16_t max_step_freq;
    uint16_t initial_step_freq;
    uint16_t steps_remaining;
    motion_states_e motion_state;
};

volatile uint16_t dma_buffer[BUFFER_SIZE];
static void stepper_init(task_t* const super, event_t const* const ie);
static void stepper_dispatch(task_t* const super, event_t* const e);

void stepper_create(task_t** self){
    static struct stepper stepper_inst;
    create_stepper_device(&stepper_inst.device);
    pwm_hw_create(0,&stepper_inst.pwm_module);
    dma_hw_create(0,&stepper_inst.dma_module);
    task_create(&stepper_inst.super,
        (event_handler_t)&stepper_init,
        (event_handler_t)&stepper_dispatch);
    *self = &stepper_inst.super;
}

static void stepper_init(task_t* const super, event_t const* const ie){
    struct stepper* self = container_of(super, struct stepper, super); 
    struct stepper_initEvt* initial_event = (struct stepper_initEvt*)ie;
    (*(self->device))->init(self->device, &initial_event->pins);
    pwm_hw_init(self->pwm_module,&pwm_config);
    pwm_hw_clock_prescaler(self->pwm_module, 32); // period of 1/2Mhz
    self->tick_frequency = 64000000/32;
    dma_hw_configure(
        self->dma_module,           //
        DMA_MEM_SFR_GPR_SEL,        // ram
        DMA_ACCESS_MODE_INCREMENT,  // ptr+ 0 -> ptr+ BUFFER_SIZE
        DMA_ACCESS_MODE_INCREMENT,  // static register
        true,                       // stop on end of buffer
        false);                     // no change

    dma_hw_arm(
        self->dma_module,
        0x26,
        dma_buffer,     // source ptr
        BUFFER_SIZE*2,  // source message size
        &PWM1PRL,       // destination ptr
        2               // destination msg size
    );
    dma_hw_set_arbiter_prio(self->dma_module, 1);   // this is reqired for dma to start
    PIR0bits.DMA1SCNTIF = 0;                 // Clear interrupt flag
    PIE0bits.DMA1SCNTIE = 1;                 // Enable source count done interrupt

    gpio_set_direction(RC_4, IO_DIR_OUTPUT);
    gpio_set_mode(RC_4, IO_MODE_DIGITAL);
    INTCON0bits.GIE = 0; //Suspend interrupts
    PPSLOCK = 0x55; //Required sequence
    PPSLOCK = 0xAA; //Required sequence
    PPSLOCKbits.PPSLOCKED = 0; //Set PPSLOCKED bit
    INTCON0bits.GIE = 1; //Restore interrupts

    RC4PPS = 0x07;

    INTCON0bits.GIE = 0; //Suspend interrupts
    PPSLOCK = 0x55; //Required sequence
    PPSLOCK = 0xAA; //Required sequence
    PPSLOCKbits.PPSLOCKED = 1; //Set PPSLOCKED bit
    INTCON0bits.GIE = 1; //Restore interrupts
}

// helper functions for fixedpoint 
#define Q15_BITS 15
#define Q15_ONE (1 << Q15_BITS)
typedef uint16_t fp15_t;

static inline fp15_t fp_mul(fp15_t a, fp15_t b){
    return ((uint32_t)a * b) >> Q15_BITS;
}

static inline fp15_t fp_div(fp15_t a, fp15_t b){
    return (((uint32_t)a << Q15_BITS) / b);
}
// -2s^3 + 3s^2
static inline fp15_t smoothstep_cubic(fp15_t s) {
    fp15_t s2 = fp_mul(s,s);
    fp15_t c1 = 3 * Q15_ONE;
    fp15_t c2 = fp_mul((2u *Q15_ONE), s);
    return fp_mul(s2, (c1 - c2));
}

static uint16_t next_period_accel(struct stepper* self) {
    fp15_t s = ((uint32_t)self->step_count << Q15_BITS) / self->accel_steps;
    if (s > Q15_ONE) s = Q15_ONE;
    fp15_t scale = smoothstep_cubic(s);
    uint32_t delta_f = self->max_step_freq - self->initial_step_freq;
    uint32_t freq  = self->initial_step_freq + ((delta_f * scale) >> 16);
    // Convert frequency to timer period
    uint32_t period = (self->tick_frequency / freq);
    return (uint16_t)period;
}

static uint16_t next_period_const(struct stepper* self){
    uint32_t period = self->tick_frequency / self->max_step_freq; 
    return (uint16_t)period;
}

static uint16_t next_period_deccel(struct stepper* self){
    fp15_t s = ((uint32_t)self->steps_remaining << Q15_BITS) / self->accel_steps;
    if (s > Q15_ONE) s = Q15_ONE;
    fp15_t scale = smoothstep_cubic(s);
    uint32_t delta_f = self->max_step_freq - self->initial_step_freq;
    uint32_t freq  = self->initial_step_freq + ((delta_f * scale)  >> 16);
    // Convert frequency to timer period
    uint32_t period = (self->tick_frequency / freq);
    return (uint16_t)period;
}

typedef uint16_t(*next_period_func_t)(struct stepper* self);
static void fill_dma_buffer(struct stepper* self, next_period_func_t next_period) {
    uint16_t p = 0;
    for(uint16_t i = 0; i < BUFFER_SIZE && self->steps_remaining > 0; i++) {
        p = next_period(self);
        dma_buffer[i] = p;  // compute next step period
        self->step_count++;
        self->steps_remaining--;
    }
}

static void stepper_dispatch(task_t* const super, event_t* const e){
    struct stepper* self = container_of(super, struct stepper, super); 
    switch (e->signal) {
        ////////////////////////////////////////////////////////
        case STEPPER_WORK_SIG:{
            struct stepper_workEvt* event = (struct stepper_workEvt*)e;
            self->steps_remaining = 20000;//0xFFFE; //event->steps;
            self->accel_steps = (self->steps_remaining >> 2); // steps required for acceleration
            self->initial_step_freq = 200;                  // start speed   (steps/tick)
            self->max_step_freq = 4000;                     // desired speed (steps/tick)
            self->step_count = 0;                           // 
            fill_dma_buffer(self,&next_period_accel);
            pwm_set_period_common(self->pwm_module, dma_buffer[0]);
            pwm_hw_enable_buffered(self->pwm_module);
            self->motion_state = STEPPER_STATE_ACCELERATE;
            self->evt.signal = STEPPER_UPDATE_SIG;
            task_event_post(super, &self->evt);

            PWM1GIRbits.S1P1IF = 0;
            PWM1GIEbits.S1P1IE = 1;
            PIR4bits.PWM1IF = 0;
            PIR4bits.PWM1PIF = 0;
            PIE4bits.PWM1IE = 1;
            PIE4bits.PWM1PIE = 1;
            pwm_hw_enable(self->pwm_module);
            //dma_hw_enable(self->dma_module);
        } break;
        ////////////////////////////////////////////////////////
        case STEPPER_UPDATE_SIG:{
            switch (self->motion_state) {
                case STEPPER_STATE_ACCELERATE:
                    if(self->step_count < self->accel_steps){
                        fill_dma_buffer(self,&next_period_accel);
                    }else{
                        self->motion_state = STEPPER_STATE_CONSTANT_SPEED;
                        self->evt.signal = STEPPER_UPDATE_SIG;   
                    }
                    task_event_post(super,&self->evt);
                    break;
                case STEPPER_STATE_CONSTANT_SPEED:
                    if(self->steps_remaining > self->accel_steps){
                        fill_dma_buffer(self, &next_period_const);
                    }else{
                        self->motion_state = STEPPER_STATE_DEACCELERATE;
                        self->evt.signal = STEPPER_UPDATE_SIG;   
                    }
                    task_event_post(super,& self->evt);
                break;
                case STEPPER_STATE_DEACCELERATE:
                    if(self->steps_remaining > 0){
                        fill_dma_buffer(self, &next_period_deccel);
                    } else{
                        self->motion_state = STEPPER_STATE_IDLE;
                        self->evt.signal = STEPPER_DONE_SIG;   
                    }
                    task_event_post(super,&self->evt);
                break;
                default:    // default case
                    TRISCbits.TRISC7 = 0;
                    return;
            }
            // re-arm the dma with new data
            dma_hw_arm(
                self->dma_module,
                0x26,
                dma_buffer,     // source ptr
                BUFFER_SIZE*2,  // source message size
                &PWM1PRL,       // destination ptr
                2               // destination msg size
            );
        } break;
        ////////////////////////////////////////////////////////
        case DRIVER_FAULT_SIG:{
            // TODO: do fault handling
        }break;
        ////////////////////////////////////////////////////////
        case STEPPER_IDLE_SIG:{
            // TODO: consider removing this. seems unessecary
        }break;
        ////////////////////////////////////////////////////////
        case STEPPER_DONE_SIG:{
            TRISCbits.TRISC7 = 0;
            pwm_hw_disable(self->pwm_module);
            dma_hw_disable(self->dma_module);
            pwm_set_period_Px(self->pwm_module,PWMx_OUTPUT_P1,0xFFFF);
            self->steps_remaining = 0;
            // post .. to notify toplayer of done
        }break;

    default:
        break;
    }
    return;
}