#include "../inc/dma_hw.h"
#include <xc.h>

struct dma_hw{
    uint8_t module_num;
};

static struct dma_hw dma_instances[4];  
void dma_hw_create(uint8_t module_num, const struct dma_hw ** dma_hw_inst_out) {
    if (module_num >= 4 || dma_hw_inst_out == 0) return;
    dma_instances[module_num].module_num = module_num;
    *dma_hw_inst_out = &dma_instances[module_num];
}

void dma_hw_init(const struct dma_hw* self, const dma_hw_config_t* config){
    DMASELECT = self->module_num;
    DMAnCON0bits.EN = 0;
    DMAnCON0bits.AIRQEN = config->hw_int_trigger_abort;
    DMAnCON0bits.SIRQEN = config->hw_int_trigger_start;
    DMAnCON1bits.DSTP = config->dstIntClear_on_reset;
    DMAnCON1bits.SSTP = config->srcIntClear_on_reset;
    DMAnCON1bits.DMODE = config->dstAccess_mode;
    DMAnCON1bits.SMODE = config->srcAccess_mode;
    DMAnCON1bits.SMR = config->mem_region_sel;
}

void dma_hw_configure(
    const struct dma_hw* self,
    volatile uint16_t src,
    uint16_t src_msg_size,
    volatile uint16_t dest,
    uint16_t dst_msg_size){
    DMASELECT = self->module_num;
    DMAnSSA = src;
    DMAnSSZ = src_msg_size;
    DMAnDSA = dest;
    DMAnDSZ = dst_msg_size;
}

void dma_hw_set_startirq(const struct dma_hw* self, uint8_t irq_num){
    DMASELECT = self->module_num;
    DMAnSIRQ  = irq_num;
}
void dma_hw_set_abortirq(const struct dma_hw* self, uint8_t irq_num){
    DMASELECT = self->module_num;
    DMAnAIRQ  = irq_num;
}


void dma_hw_set_hwint(const struct dma_hw* self, bool start_trigg, bool abort_trigg){
    DMASELECT = self->module_num;
    DMAnCON0bits.AIRQEN = start_trigg;
    DMAnCON0bits.SIRQEN = abort_trigg;
}

void dma_hw_enable(const struct dma_hw* self){
    DMASELECT = self->module_num;
    DMAnCON0bits.EN = 1; 
}

void dma_hw_disable(const struct dma_hw* self){
    DMASELECT = self->module_num;
    DMAnCON0bits.EN = 0;  
}