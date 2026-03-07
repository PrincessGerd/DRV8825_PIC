#include "axis_stepper.h"
#include "motion_planer.h"
#include "../core/hw/inc/dma_hw.h"
#include "../core/hw/inc/pwm_hw.h"
#include "../core/interrupts.h"
#include "../inc/math.h"
#include "../core/task_manager.h"
#include "../core/dma_descriptor.h"
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
    struct dma_hw* dma;
    struct pwm_hw* pwm;
    dma_descriptor_handle_t* dma_disp_handle;
    uint16_t bufffer[2][BUFFER_SIZE];
    uint8_t module_num;
} axis_stepper_t;

static dma_descriptor_t descriptors[2][2];
static axis_stepper_t axes[2];
static const volatile uint16_t* period_regs[2] = {&PWM1PRL, &PWM2PRL};

void axis_stepper_init_descriptors(axis_stepper_t* self) {
    // Descriptor for first buffer
    descriptors[self->module_num][0].src     = (uint24_t)self->bufffer[0];
    descriptors[self->module_num][0].srcSize = BUFFER_SIZE * sizeof(uint16_t);
    descriptors[self->module_num][0].dst     = (uint24_t)period_regs[self->module_num];
    descriptors[self->module_num][0].dstSize = sizeof(uint16_t);
    descriptors[self->module_num][0].next    = &descriptors[self->module_num][1];

    // Descriptor for second buffer
    descriptors[self->module_num][1].src     = (uint24_t)self->bufffer[1];
    descriptors[self->module_num][1].srcSize = BUFFER_SIZE * sizeof(uint16_t);
    descriptors[self->module_num][1].dst     = (uint24_t)period_regs[self->module_num];
    descriptors[self->module_num][1].dstSize = sizeof(uint16_t);
    descriptors[self->module_num][1].next    = &descriptors[self->module_num][0];

    dma_descriptor_enqueue(self->dma_disp_handle,&descriptors[self->module_num][0]);
    dma_descriptor_enqueue(self->dma_disp_handle,&descriptors[self->module_num][1]);
}

void axis_stepper_instance(axis_stepper_t** inst_out, task_t* const owner, uint8_t module_num){
    if (module_num >= 2 || inst_out == 0) return;
    static struct dma_descriptor_handle handles[2];
    axis_stepper_t* inst = &axes[module_num];
    axes[module_num].module_num = module_num;
    axes[module_num].owner = owner;
    axes[module_num].dma_disp_handle = &handles[module_num];
    dma_hw_create(module_num,&(axes[module_num].dma));
    pwm_hw_create(module_num,&(axes[module_num].pwm));
    *inst_out = &axes[module_num];
}

void axis_stepper_init(axis_stepper_t* const self){
    pwm_hw_init(self->pwm, &pwm_config);     // same config for both
    pwm_hw_clock_prescaler(self->pwm,32);    // tick period of 1/2Mhz
    // configure dma for this axis
    pwm_hw_set_lds(self->pwm, 0x7 + self->module_num); // dma1 or dma2 as trigger
    dma_hw_configure(
        self->dma,                  //
        DMA_MEM_SFR_GPR_SEL,        // use ram
        DMA_ACCESS_MODE_INCREMENT,  // increment src ptr
        DMA_ACCESS_MODE_INCREMENT,  // increment dst ptr
        true,                       // stop on end of buffer
        false);                     // no change on dst count
    dma_hw_set_arbiter_prio(self->dma, 1);   // this is reqired for dma to start
    axis_stepper_init_descriptors(self);
    for(uint8_t i=0; i < BUFFER_SIZE; i++){
        self->bufffer[0][i] = 0xFFFF - 128*i;
        self->bufffer[1][i] = self->bufffer[0][i] - 128*i;
    }
    dma_descriptor_setup(
        self->dma_disp_handle,   // this axis handle to descriptor linked-list
        self->dma,              // this axis dma
        &descriptors[self->module_num][0],  // initial descriptor
        0x26+ (2*self->module_num));       // trigger irq
}

void axis_stepper_start_move(
    axis_stepper_t* self,
    uint32_t steps){
        self->steps_remaining = steps;
        pwm_set_period_common(self->pwm, self->bufffer[0][0]);
        pwm_hw_enable(self->pwm);
}

uint16_t* axis_stepper_get_fill_buffer(struct axis_stepper* self){
    return (uint16_t*)self->dma_disp_handle->fill->src;
}

void __interrupt(irq(0x4)) dma_axis1_isr(void){
    interrupt_clear(0x4);
    TRISCbits.TRISC7 ^= 1;
    axis_stepper_t* axis = &axis[0];
    if(axis->steps_remaining > 0){
        dma_descriptor_dispatch(axis->dma_disp_handle); // swap and reamr active buffer
        axis->evt.signal = STEPPER_UPDATE_SIG;  // inform motion planer of empty buffer
        task_event_post(axis->owner,&axis->evt);
        axis->steps_remaining -= BUFFER_SIZE;
        axis->step_count += BUFFER_SIZE;
    }else{
        pwm_hw_disable(axis->pwm);
        axis->evt.signal = STEPPER_DONE_SIG;  // inform motion planer of empty buffer
        task_event_post(axis->owner,&axis->evt);
    }
    task_event_consume(axis->owner);
}

//void __interrupt(irq(0x8)) dma_axis2_isr(void){
//    interrupt_clear(0x8);
//    axis_stepper_t* axis = &axis[1];
//    if(axis->steps_remaining > 0){
//        dma_descriptor_dispatch(axis->dma_disp_handle); // swap and reamr active buffer
//        axis->evt.signal = STEPPER_UPDATE_SIG;  // inform motion planer of empty buffer
//        task_event_post(axis->owner,&axis->evt);
//        axis->steps_remaining -= BUFFER_SIZE;
//        axis->step_count += BUFFER_SIZE;
//    }else{
//        pwm_hw_disable(axis->pwm);
//        axis->evt.signal = STEPPER_DONE_SIG;  // inform motion planer of empty buffer
//        task_event_post(axis->owner,&axis->evt);
//    }
//}
