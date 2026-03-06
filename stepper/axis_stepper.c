#include "axis_stepper.h"
#include "motion_planer.h"
#include "../core/hw/inc/dma_hw.h"
#include "../core/hw/inc/pwm_hw.h"
#include "../core/interrupts.h"
#include "../inc/math.h"
#include "../core/task_manager.h"
#include <xc.h>

static const pwm_hw_config_t pwm_config = {
    .out1_polarity_low = false,
    .out2_polarity_low = false,
    .push_pull_mode = false,
    .mode = PWMx_MODE_COMPARE_TOGGLE,
    .clk_src = PWMx_CLK_FOSC
};

typedef struct axis_stepper{
    task_t* owner;
    event_t evt;
    uint32_t steps_remaining;
    uint32_t step_count;
    fp15_t   n;  // normalised scaler

    struct dma_hw* dma;
    struct pwm_hw* pwm;

    uint16_t buffer[2][BUFFER_SIZE];
    uint8_t active_buffer;
    uint8_t module_num;
} axis_stepper_t;

static axis_stepper_t axes[2];
static const volatile uint8_t* period_regs[2] = {&PWM1PRL, &PWM2PRL};
void axis_stepper_fill_buffer(axis_stepper_t* const self, uint16_t* period){
    uint8_t inactive = self->active_buffer ^ 1;
    for(int i = 0; i < BUFFER_SIZE; i++){
        self->buffer[inactive][i] = ((uint32_t)period[i] * self->n) >> Q15_BITS;
    }
}

void axis_stepper_get_inactive_buffer(axis_stepper_t* const self, uint16_t* buffer){
    uint8_t inactive = self->active_buffer ^ 1;
    buffer = self->buffer[inactive];
}

void axis_stepper_get_active_buffer(axis_stepper_t* const self, uint16_t* buffer){
    buffer = self->buffer[self->active_buffer];
}

static void arm_dma(const axis_stepper_t* self){
    dma_hw_arm(
        self->dma,
        0x26 + (2*self->module_num),// irq trigger PWM1
        &self->buffer[self->active_buffer][0], // source ptr
        BUFFER_SIZE*2,          // source message size
        &PWM1PRL,    // destination ptr
        2                     // destination msg size
    );
}

void axis_stepper_instance(axis_stepper_t** inst_out, task_t* const owner, uint8_t module_num){
    if(module_num > 2){*inst_out = 0; return;}
    axis_stepper_t* inst = &axes[module_num];
    axes[module_num].module_num = module_num;
    axes[module_num].owner = owner;
    dma_hw_create(module_num,&(axes[module_num].dma));
    pwm_hw_create(module_num,&(axes[module_num].pwm));
    *inst_out = &axes[module_num];
}

void axis_stepper_init(const axis_stepper_t* self){
    pwm_hw_init(self->pwm, &pwm_config);     // same config for both
    pwm_hw_clock_prescaler(self->pwm,32);    // tick period of 1/2Mhz
    pwm_set_period_common(self->pwm,0xFFFE); // 
    dma_hw_configure(
        self->dma,                  //
        DMA_MEM_SFR_GPR_SEL,        // use ram
        DMA_ACCESS_MODE_INCREMENT,  // increment src ptr
        DMA_ACCESS_MODE_INCREMENT,  // increment dst ptr
        true,                       // stop on end of buffer
        false);                     // no change on dst count
    dma_hw_set_arbiter_prio(self->dma, 1);   // this is reqired for dma to start
}

void axis_stepper_start_move(
    axis_stepper_t* self,
    uint32_t steps,
    uint16_t n,
    uint16_t* period){
        self->n = n;   // normalised scaler for axis
        self->steps_remaining = steps;
        for(int i = 0; i < BUFFER_SIZE; i++){
            self->buffer[0][i] = (period[i] * self->n) >> Q15_BITS;
            self->buffer[1][i] = (128*i * self->n) >> Q15_BITS;
        }
        arm_dma(self);  // arm dma with first buffer
        pwm_set_period_common(self->pwm, self->buffer[self->active_buffer][0]);
        pwm_hw_enable_buffered(self->pwm);
        pwm_hw_enable(self->pwm);    // start pwm
}

void __interrupt(irq(0x4)) dma_axis1_isr(void){
    interrupt_clear(0x4);
    TRISCbits.TRISC7 ^= 1;
    if(axes[0].steps_remaining > 0){
        axes[0].active_buffer ^= 1; // swap active buffer
        arm_dma(&axes[0]); // reamr dma with swapped buffer
        axes[0].evt.signal = STEPPER_UPDATE_SIG;  // inform motion planer of empty buffer
        task_event_post(axes[0].owner,&axes[0].evt);
        axes[0].steps_remaining -= BUFFER_SIZE;
        axes[0].step_count += BUFFER_SIZE;
    }else{
        pwm_hw_disable(axes[0].pwm);
        axes[0].evt.signal = STEPPER_DONE_SIG;  // inform motion planer of empty buffer
        task_event_post(axes[0].owner,&axes[0].evt);
    }
}

void __interrupt(irq(0x8)) dma_axis2_isr(void){
    interrupt_clear(0x8);
    TRISCbits.TRISC7 ^= 1;
    if(axes[1].steps_remaining > 0){
        axes[1].active_buffer ^= 1; // swap active buffer
        arm_dma(&axes[1]); // reamr dma with swapped buffer
        axes[1].evt.signal = STEPPER_UPDATE_SIG;  // inform motion planer of empty buffer
        task_event_post(axes[1].owner,&axes[1].evt);
        axes[1].steps_remaining -= BUFFER_SIZE;
        axes[1].step_count += BUFFER_SIZE;
    }else{
        pwm_hw_disable(axes[1].pwm);
        axes[1].evt.signal = STEPPER_DONE_SIG;  // inform motion planer of empty buffer
        task_event_post(axes[1].owner,&axes[1].evt);
    }
}
