#include "axis_stepper.h"
#include "motion_planer.h"
#include "../core/hw/inc/dma_hw.h"
#include "../core/hw/inc/pwm_hw.h"
#include "../core/hw/inc/clcx_hw.h"
#include "../core/interrupts.h"
#include "../inc/math.h"
#include "../core/task_manager.h"
#include "../core/dma_descriptor.h"
#include <xc.h>

static const pwm_hw_config_t pwm_config = {
    .out1_polarity_low = false,
    .out2_polarity_low = false,
    .push_pull_mode = false,
    .mode = PWMx_MODE_RIGHT_ALIGN,
    .clk_src = PWMx_CLK_FOSC
};

// Buffer stream must be in form [[u, v], [u, v]]
// destination pointer is [pwm1S1, pwm1S2]
#define AXES_PER_MODULE 2

typedef union __attribute__((packed, aligned(4))) uv{
    struct{
        uint16_t u;
        uint16_t v;
    } axis;
    uint16_t uv[2];
} uv_t;

typedef struct axis_stepper {
    struct dma_hw* dma;     // DMA for this stepper
    struct pwm_hw* pwm;     // PWM for this steper (2 slices)
    uint32_t steps_remaining;   // steps remaining of dominant stepper
    dma_descriptor_handle_t dma_disp_handle;
    dma_descriptor_t descriptors[NUM_DESCRIPTORS]; // linked list of dma descriptors for buffer swap
    uv_t bufffer[NUM_DESCRIPTORS][BUFFER_SIZE];   // equal number of buffers
    uint8_t axis_count;  // number of axes for this stepper
} axis_stepper_t;

static axis_stepper_t instance;
static volatile uint16_t* const period_regs[2] = {&PWM1S1P1L, &PWM2S1P1L}; // base period regs for each pwm module

void axis_stepper_instance(axis_stepper_t** inst_out, task_t* const owner, uint8_t module_num){
    if (module_num >= 4 || inst_out == 0) return;
    dma_hw_create(module_num,&instance.dma);
    *inst_out = &instance;
}

void axis_stepper_init(
    axis_stepper_t* const self, 
    uint8_t axis_count){
        self->axis_count = axis_count;
        dma_hw_configure(
            self->dma,                  //
            DMA_MEM_SFR_GPR_SEL,        // use ram
            DMA_ACCESS_MODE_INCREMENT,  // increment src ptr
            DMA_ACCESS_MODE_INCREMENT,  // increment dst ptr
            true,                       // stop on end of buffer
            false);                     // no change on dst count
        dma_hw_set_arbiter_prio(self->dma, 1);   // this is reqired for dma to start
        dma_descriptor_setup(
            &self->dma_disp_handle,   // shared handle to descriptor linked-list
            self->dma,               // shared dma
            0x26);                   // trigger irq set to PWM1

        //init and link buffers to descriptors
        for (int i = 0; i < NUM_DESCRIPTORS; i++) {
            self->descriptors[i].src     = (uint24_t)&self->bufffer[i][0];
            self->descriptors[i].srcSize = BUFFER_SIZE * sizeof(uv_t);
            self->descriptors[i].dst     = (uint24_t)period_regs[0];
            self->descriptors[i].dstSize = sizeof(uv_t);   // two 16-bit registers
            self->descriptors[i].next    = 
                            &self->descriptors[(i + 1) % NUM_DESCRIPTORS];
            dma_descriptor_enqueue(&self->dma_disp_handle, &self->descriptors[i]);
        }
        // initialise the required pwm modules
        pwm_hw_create(0,&self->pwm);
        pwm_hw_init(self->pwm, &pwm_config);     // same config for both
        pwm_hw_clock_prescaler(self->pwm,32);    // tick period of 1/2Mhz
        pwm_hw_enable_buffered(self->pwm);
        pwm_hw_set_lds(self->pwm, 0x7);
}

void axis_stepper_start_move(
    axis_stepper_t* self,
    uint32_t steps){
        int idx = 0;
        while(idx < BUFFER_SIZE){
            self->bufffer[0][idx].axis.u = (0xFFFE - idx*256);
            self->bufffer[1][idx].axis.u = ((uint16_t)self->bufffer[1][idx].axis.u - idx*256);
            self->bufffer[0][idx].axis.v = (0xFFFE - idx*256);
            self->bufffer[1][idx].axis.v = ((uint16_t)self->bufffer[1][idx].axis.v - idx*256);
            idx+=1;
        }
        PWM1S1P1L = self->bufffer[0][0].axis.u;
        PWM1S1P2L = self->bufffer[0][0].axis.v;
        self->steps_remaining = steps;
        pwm_set_period_common(self->pwm, 0xFFFF);
        dma_descriptor_start(&self->dma_disp_handle); // push first dma descriptor
        dma_descriptor_dispatch(&self->dma_disp_handle);
        pwm_hw_enable(self->pwm);
}

uint16_t* axis_stepper_get_fill_buffer(struct axis_stepper* self){
    return (uint16_t*)self->dma_disp_handle.fill->src;
}

void __interrupt(irq(0x4)) dma_axis1_isr(void){
    interrupt_clear(0x4);
    TRISCbits.TRISC7 ^= 1;
    axis_stepper_t* stepper = &instance;
    dma_descriptor_dispatch(&stepper->dma_disp_handle); // swap and reamr active buffer
    dma_descriptor_dispatch(&stepper->dma_disp_handle); // swap and reamr active buffer
    if(stepper->steps_remaining > 0){
        //stepper->evt.signal = STEPPER_UPDATE_SIG;  // inform motion planer of empty buffer
        //task_event_post(stepper->owner,&stepper->evt);
        stepper->steps_remaining -= BUFFER_SIZE;
    }else{
        pwm_hw_disable(stepper->pwm);
        //stepper->evt.signal = STEPPER_DONE_SIG;  // inform motion planer of empty buffer
        //task_event_post(stepper->owner,&stepper->evt);
    }
}