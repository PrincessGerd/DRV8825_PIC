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

static const dma_hw_config_t dma_config = {
    .dstAccess_mode = DMA_ACCESS_MODE_UNCHAINGED,
    .dstIntClear_on_reset = true,
    .srcIntClear_on_reset = true,
    .hw_int_trigger_abort = true,
    .hw_int_trigger_start = true,
    .mem_region_sel = DMA_MEM_SFR_GPR_SEL
};

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

#define RB_SIZE 64

struct stepper{
    task_t              super;
    event_t             evt;
    motion_states_e     motion_state;
    stepper_device_t    device;
    
    uint32_t stepps_remaining;
    uint32_t accel;
    uint16_t accel_delta;
    uint16_t period;

    struct pwm_hw*      pwm_module;
    struct dma_hw*      dma_module;
    
    uint16_t period_buffer[RB_SIZE];
    volatile uint8_t head;
    volatile uint8_t tail;
    volatile uint8_t count;
};

static uint32_t julery_isqrt(uint32_t val) {
    uint32_t  temp, g=0, b = 0x8000, bshft = 15;
    do {
        if (val >= (temp = (((g << 1) + b)<<bshft--))) {
           g += b;
           val -= temp;
        }
    } while (b >>= 1);
    return g;
}

static void update_buffer(struct stepper* self){
    self->accel_delta = self->accel_delta > 0 ? self->accel_delta - 1 : 1;
    while (self->count < RB_SIZE) {
        uint16_t period = pwm_hw_calculate_duty(self->period,self->accel_delta);

        self->period_buffer[self->tail] = period;
        self->tail = mod((self->tail + 1), RB_SIZE);
        self->count++;
        if(self->stepps_remaining == 0){
            break;
        }
    }
};

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
    dma_hw_init(self->dma_module,&dma_config);
    dma_hw_set_src(self->dma_module, (uint16_t)&self->period_buffer[0]);
    dma_hw_set_dst(self->dma_module, (uint16_t)&PWM1PR);
    dma_hw_set_startirq(self->dma_module, 0x5E); // pwm1sp1
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

    self->head =0;
    self->tail =0;
    self->count = 0;
    self->accel_delta = 0;
}

static void stepper_dispatch(task_t* const super, event_t* const e){
    struct stepper* self = container_of(super, struct stepper, super); 
    switch (e->signal) {
        ////////////////////////////////////////////////////////
        case STEPPER_WORK_SIG:{
            struct stepper_workEvt* event = (struct stepper_workEvt*)e;
            self->accel = event->accel;
            self->stepps_remaining = event->steps;
            update_buffer(self);
            pwm_hw_disable(self->pwm_module);
            uint16_t test_per = pwm_hw_calculate_duty(1000,50);
            pwm_set_period_common(self->pwm_module,test_per );
            pwm_set_period_Px(self->pwm_module, PWMx_OUTPUT_P1, pwm_hw_calculate_duty(test_per,50));
            interrupt_enable_priority();          
            PIR4bits.PWM1PIF = 0;
            PIR4bits.PWM1IF = 0;
            PWM1GIRbits.S1P1IF = 0;
            PWM1GIRbits.S1P2IF = 0;
            PIE4bits.PWM1IE = 1;
            PIE4bits.PWM1PIE = 1;
            IPR4bits.PWM1IP = 1;
            enable_global_interrupts();
            //dma_hw_enable(self->dma_module);
            pwm_hw_enable(self->pwm_module);
            self->evt.signal = STEPPER_UPDATE_SIG;
            task_event_post(super,& self->evt);
        } break;
        ////////////////////////////////////////////////////////
        case STEPPER_UPDATE_SIG:{
            switch (self->motion_state) {
                case STEPPER_STATE_ACCELERATE:
                    update_buffer(self);
                break;
                case STEPPER_STATE_CONSTANT_SPEED:
                break;
                case STEPPER_STATE_DEACCELERATE:
                    update_buffer(self);
                break;
                default:    // default case
                    break;
            }
        } break;
        ////////////////////////////////////////////////////////
        case DRIVER_FAULT_SIG:{

        }break;
        ////////////////////////////////////////////////////////
        case STEPPER_IDLE_SIG:{

        }break;
        ////////////////////////////////////////////////////////
        case STEPPER_DONE_SIG:{
            pwm_hw_disable(self->pwm_module);
            dma_hw_disable(self->dma_module);
            self->stepps_remaining = 0;
            self->accel = 0;
            // post .. to notify toplayer of done
        }break;

    default:
        break;
    }
    return;
}