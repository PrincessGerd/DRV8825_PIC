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
    .srcAccess_mode = DMA_ACCESS_MODE_INCREMENT,
    .dstAccess_mode = DMA_ACCESS_MODE_INCREMENT,
    .dstIntClear_on_reset = false,
    .srcIntClear_on_reset = false,
    .hw_int_trigger_abort = false,
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

#define BUFFER_SIZE 32

struct stepper{
    task_t              super;
    event_t             evt;
    stepper_device_t    device;
    
    struct pwm_hw*      pwm_module;
    struct dma_hw*      dma_period;
    
    uint16_t c_n;
    uint16_t step_count;
    uint16_t accel_steps;

    motion_states_e   motion_state;

    volatile uint16_t steps_remaining;
    volatile uint16_t buffer_index;
};

volatile uint16_t dma_buffer[BUFFER_SIZE];
static void stepper_init(task_t* const super, event_t const* const ie);
static void stepper_dispatch(task_t* const super, event_t* const e);

typedef uint16_t(*next_period_func_t)(uint16_t* c_n, uint16_t* step_num);
void fill_dma_buffer(struct stepper* self, next_period_func_t next_period) {
    for(uint16_t i = 0; i < BUFFER_SIZE && self->steps_remaining > 0; i++) {
        dma_buffer[i] = self->c_n;                       // write period for DMA
        self->c_n = self->c_n +128;//next_period(&self->c_n, &self->step_count); // compute next period
        self->step_count++;
        self->steps_remaining--;
    }
}

void stepper_create(task_t** self){
    static struct stepper stepper_inst;
    create_stepper_device(&stepper_inst.device);
    pwm_hw_create(0,&stepper_inst.pwm_module);
    //dma_hw_create(0,&stepper_inst.dma_period);
    task_create(&stepper_inst.super,
        (event_handler_t)&stepper_init,
        (event_handler_t)&stepper_dispatch);
    *self = &stepper_inst.super;
}

void DMA1_Init(struct stepper* self) { // refer to figure 16-5. DMA Operation with Hardware Trigger
    DMASELECT = 0;   // Select DMA1

    DMAnSSA  = (uint16_t)&dma_buffer[0];     // Source = RAM buffer
    DMAnDSA  = (uint16_t)&PWM1PRL;         // Dest = PWM period register

    DMAnSSZ  = BUFFER_SIZE * 2;             // 2 bytes per trigger
    DMAnDSZ  = 2;

    //DMAnSCNT = BUFFER_SIZE * 2;            // Total bytes allowed

    DMAnCON1bits.SMR   = 0b00;               // Source = GPR (RAM)
    DMAnCON1bits.SMODE = 0b01;               // Increment source
    DMAnCON1bits.DMODE = 0b01;               // Fixed destination

    DMAnCON1bits.SSTP  = 1;                  // Stop at end of buffer
    DMAnCON1bits.DSTP  = 0;

    DMAnSIRQ = 0x26;                         // PWM1 reload trigger (verify in datasheet)

    DMAnCON0bits.SIRQEN = 1;                 // Enable trigger
    DMAnCON0bits.EN     = 1;                 // Enable DMA
    DMAnCON0bits.DGO    = 1;
    PIR0bits.DMA1SCNTIF = 0;                 // Clear interrupt flag
    PIE0bits.DMA1SCNTIE = 1;                 // Enable source count done interrupt
    // Use default priority level
    // Lock priority to grant memory access
    asm ("BANKSEL PRLOCK");
    asm ("MOVLW 0x55");
    asm ("MOVWF PRLOCK");
    asm ("MOVLW 0xAA");
    asm ("MOVWF PRLOCK");
    asm ("BSF PRLOCK, 0");
}

static void stepper_init(task_t* const super, event_t const* const ie){
    struct stepper* self = container_of(super, struct stepper, super); 
    struct stepper_initEvt* initial_event = (struct stepper_initEvt*)ie;
    (*(self->device))->init(self->device, &initial_event->pins);
    pwm_hw_init(self->pwm_module,&pwm_config);
    pwm_hw_clock_prescaler(self->pwm_module, 255);
    //dma_hw_init(self->dma_period,&dma_config);
    //dma_hw_configure(
    //    self->dma_period,
    //    (uint16_t)dma_buffer,       // source ptr
    //    2*BUFFER_SIZE,                      // source message size
    //    (uint16_t)PWM1S1P1L,              // destination ptr
    //    2                                 // destination msg size
    //);
    //dma_hw_set_startirq(self->dma_period, 0x5E); // PWM1S1P1
    DMA1_Init(self);
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

uint16_t next_period_accel(uint16_t* c_n, uint16_t* step_num){
    // c_{n+1} = c_n - (2c_n)/(4n+1)
    // AVR446 example 
    uint32_t cn = *c_n;
    uint32_t n  = *step_num;
    uint32_t delta = (cn << 1) / (4*n + 1);
    cn -= delta;
    *c_n = (uint16_t)cn;
    return *c_n;
}

uint16_t next_period_const(uint16_t* c_n, uint16_t* step_num){
    (void)step_num;
    return *c_n;
}

uint16_t next_period_deccel(uint16_t* c_n, uint16_t* step_num){
    uint32_t cn = *c_n;
    uint32_t n  = *step_num;
    uint32_t delta = (cn << 1) / (4*n + 1);
    cn += delta;
    *c_n = (uint16_t)cn;
    return *c_n;
}
static void stepper_dispatch(task_t* const super, event_t* const e){
    struct stepper* self = container_of(super, struct stepper, super); 
    switch (e->signal) {
        ////////////////////////////////////////////////////////
        case STEPPER_WORK_SIG:{
            struct stepper_workEvt* event = (struct stepper_workEvt*)e;
            self->steps_remaining = 0xFFFE;//event->steps;
            self->c_n = 0xFFFE;
            self->step_count = 0;
            self->accel_steps = 0xFFF;
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
            //dma_hw_enable(self->dma_period);
        } break;
        ////////////////////////////////////////////////////////
        case STEPPER_UPDATE_SIG:{
            switch (self->motion_state) {
                case STEPPER_STATE_ACCELERATE:
                    if(self->step_count < self->accel_steps){
                        fill_dma_buffer(self,&next_period_accel);
                        self->buffer_index = 0;
                    }else{
                        self->motion_state = STEPPER_STATE_CONSTANT_SPEED;
                        self->evt.signal = STEPPER_UPDATE_SIG;   
                    }
                    task_event_post(super,&self->evt);
                    break;
                case STEPPER_STATE_CONSTANT_SPEED:
                    if(self->steps_remaining > self->accel_steps){
                        fill_dma_buffer(self, &next_period_const);
                        self->buffer_index = 0;
                    }else{
                        self->motion_state = STEPPER_STATE_DEACCELERATE;
                        self->evt.signal = STEPPER_UPDATE_SIG;   
                    }
                    task_event_post(super,& self->evt);
                break;
                case STEPPER_STATE_DEACCELERATE:
                    if(self->steps_remaining < 1){
                        fill_dma_buffer(self, &next_period_deccel);
                        self->buffer_index = 0;
                    } else{
                        self->motion_state = STEPPER_STATE_IDLE;
                        self->evt.signal = STEPPER_DONE_SIG;   
                    }
                    task_event_post(super,&self->evt);
                break;
                default:    // default case
                    TRISCbits.TRISC7 = 0;
                    break;
            }
            TRISCbits.TRISC7 ^= 1;
            DMASELECT = 0x0;
            DMAnSSA  = (uint16_t)&dma_buffer[0];     // Source = RAM buffer
            DMAnSSZ  = BUFFER_SIZE * 2;              // 2 bytes per trigger
            DMAnDSZ  = 2;
            DMAnCON0bits.SIRQEN = 1;                 // Enable trigger
            DMAnCON0bits.EN     = 1;                 // Enable DMA
            DMAnCON0bits.DGO    = 1;
        } break;
        ////////////////////////////////////////////////////////
        case DRIVER_FAULT_SIG:{

        }break;
        ////////////////////////////////////////////////////////
        case STEPPER_IDLE_SIG:{

        }break;
        ////////////////////////////////////////////////////////
        case STEPPER_DONE_SIG:{
            TRISCbits.TRISC7 = 0;
            pwm_hw_disable(self->pwm_module);
            dma_hw_disable(self->dma_period);
            pwm_set_period_Px(self->pwm_module,PWMx_OUTPUT_P1,0xFFFF);
            self->steps_remaining = 0;
            // post .. to notify toplayer of done
        }break;

    default:
        break;
    }
    return;
}