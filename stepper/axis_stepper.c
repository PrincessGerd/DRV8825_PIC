#include "axis_stepper.h"
#include "motion_planer.h"
#include "../core/hw/inc/dma_hw.h"
#include "../core/hw/inc/timer1_hw.h"
#include "../core/hw/inc/clcx_hw.h"
#include "../core/gpio.h"
#include "../core/interrupts.h"
#include "../inc/math.h"
#include "../core/task_manager.h"
#include "../core/dma_descriptor.h"
#include <xc.h>

// (64Mhz/4/8) = 2 Mhz tick frequency
static const tmr1_config_t tick_timer_cfg = {
    .clk_src = TMR1_CLK_FOSC4,
    .prescaler = TMR1_PRESCALER_1,
    .gate = {0},
    .gate_enable = false,
    .clk_sync = true
};

// Buffer stream must be in form [[u_step, u_dir, ...], [us,ud, vs,vd, ws,wd, es,ed]]
typedef struct axis_stepper {
    task_t* owner;
    event_t evt;    
    struct dma_hw* dma;         // DMA for this stepper
    volatile uint8_t* port;
    volatile uint8_t  port_mask;
    int32_t steps_remaining;   // steps remaining of dominant axis
    dma_descriptor_handle_t dma_disp_handle;
    dma_descriptor_t descriptors[NUM_DESCRIPTORS]; // linked list of dma descriptors for buffer swap
    volatile uint8_t bufffer[NUM_DESCRIPTORS][AXIS_STEPPER_BUFFER_SIZE];   // equal number of buffers
    uint8_t axis_count;  // number of axes for this stepper
} axis_stepper_t;


static axis_stepper_t    instance;
static struct timer1_hw* tick_timer;

void axis_stepper_instance(axis_stepper_t** inst_out, task_t* const owner, uint8_t module_num){
    if (module_num >= 4 || inst_out == 0) return;
    if(owner == 0) return;
    dma_hw_create(module_num,&instance.dma);
    timer1_create(&tick_timer);
    instance.owner = owner;
    *inst_out = &instance;
}

void axis_stepper_init(
    axis_stepper_t* const self, 
    uint8_t axis_count,
    volatile uint8_t* port,
    uint8_t  mask){
        self->axis_count = axis_count;
        self->port = port;
        self->port_mask = mask;

        timer1_init(tick_timer, &tick_timer_cfg);
        timer1_set_counter(tick_timer, 0xEFFF); // int on reload. can varry timing later if needed
        dma_hw_configure(
            self->dma,                  //
            DMA_MEM_SFR_GPR_SEL,        // use ram
            DMA_ACCESS_MODE_INCREMENT,  // increment src ptr
            DMA_ACCESS_MODE_UNCHAINGED, // static dst ptr
            true,                       // stop on end of buffer
            false);                     // no change on dst count
        dma_hw_set_arbiter_prio(self->dma, 1);   // this is reqired for dma to start
        dma_descriptor_setup(
            &self->dma_disp_handle,   // shared handle to descriptor linked-list
            self->dma,               // shared dma
            0x1E);                   // trigger irq set to TMR1

        //init and link buffers to descriptors
        for (int i = 0; i < NUM_DESCRIPTORS; i++) {
            self->descriptors[i].src     = (uint24_t)&self->bufffer[i][0];
            self->descriptors[i].srcSize = AXIS_STEPPER_BUFFER_SIZE;
            self->descriptors[i].dst     = (uint24_t)self->port;
            self->descriptors[i].dstSize = sizeof(uint8_t);   // single port 8-bit register
            self->descriptors[i].next    = 0;
            dma_descriptor_enqueue(&self->dma_disp_handle, &self->descriptors[i]);
        }

}

void axis_stepper_start_move(
    axis_stepper_t* self,
    int32_t steps){
        *self->port = 0; // clear port
        self->steps_remaining = steps;
        dma_descriptor_start(&self->dma_disp_handle);     // push first dma descriptor
        dma_descriptor_dispatch(&self->dma_disp_handle); // arm next 
        dma_hw_enable(self->dma);
        timer1_enable(tick_timer);
}

void axis_stepper_get_fill_buffer(struct axis_stepper* self, uint8_t **out){
    *out = ((uint8_t*)self->dma_disp_handle.fill->src);
}


// TODO : FIX PROBLEM WITH TIMER1 TRIGGER for DMA
void __interrupt(irq(0x4)) dma_axis1_isr(void){
    interrupt_clear(0x4);
    axis_stepper_t* stepper = &instance;
    dma_descriptor_dispatch(&stepper->dma_disp_handle); // swap and re-amr dma
    //dma_descriptor_dispatch(&stepper->dma_disp_handle);
    if(stepper->steps_remaining > 0){
        stepper->evt.signal = EV_BUFFER_FILL_SIG;  // inform motion planer of empty buffer
        task_event_post(stepper->owner,&stepper->evt);
        stepper->steps_remaining -= AXIS_STEPPER_BUFFER_SIZE;
    }else{
        timer1_disable(tick_timer);
        stepper->evt.signal = EV_MOVE_DONE_SIG;  // inform motion planer of move done
        task_event_post(stepper->owner,&stepper->evt);
        *stepper->port &= ~stepper->port_mask;
    }
    task_event_consume(stepper->owner);
}