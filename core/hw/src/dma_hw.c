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

void dma_hw_configure(
    const struct dma_hw* self,
    dma_memory_region_e  mem_region,
    dma_acces_mode_e     src_acces_mode,
    dma_acces_mode_e     dest_acces_mode,
    bool src_rst_on_done,
    bool dest_rst_on_done){
        DMASELECT = self->module_num;
        DMAnCON1bits.SMR   = mem_region;        // set source memory region
        DMAnCON1bits.SMODE = src_acces_mode;    // set source memory access mode
        DMAnCON1bits.DMODE = dest_acces_mode;   // set destination memory access mode
        
        DMAnCON1bits.SSTP  = src_rst_on_done;   // set source trigger reset on count done
        DMAnCON1bits.DSTP  = dest_rst_on_done;  // set destination trigger reset on count done 
}

void dma_hw_arm(
    const struct dma_hw* self, 
    uint8_t trigger, 
    uint24_t src_addr,
    uint16_t src_len,
    uint24_t dest_addr,
    uint16_t dest_len)
{
    DMASELECT = self->module_num;
    DMAnSSA = src_addr;
    DMAnDSA = dest_addr;
    DMAnSSZ = src_len;
    DMAnDSZ = dest_len;
    DMAnSIRQ  = trigger;

    DMAnCON0bits.SIRQEN = 1;
    DMAnCON0bits.EN     = 1;
    DMAnCON0bits.DGO    = 1;
}

void dma_hw_set_arbiter_prio(const struct dma_hw* self, uint8_t prio){
    // prio between 0 and 7
    if(prio > 7){ // should be a static assert
        prio = 1;
    }
    DMA1PR = prio;           // Change the priority only if needed
    PRLOCK = 0x55;           // This sequence
    PRLOCK = 0xAA;           // is mandatory
    PRLOCKbits.PRLOCKED = 1; // for DMA operation
}

void dma_hw_set_abortirq(const struct dma_hw* self, uint8_t irq_num){
    DMASELECT = self->module_num;
    DMAnAIRQ  = irq_num;
}

void dma_hw_enable(const struct dma_hw* self){
    DMASELECT = self->module_num;
    DMAnCON0bits.EN = 1; 
}

void dma_hw_disable(const struct dma_hw* self){
    DMASELECT = self->module_num;
    DMAnCON0bits.EN = 0;  
}

/*
void DMA1_Init(struct stepper* self) { // refer to figure 16-5. DMA Operation with Hardware Trigger
    DMASELECT = 0;   // Select DMA1

    DMAnSSA  = (uint16_t)&dma_buffer[0];     // Source = RAM buffer
    DMAnDSA  = (uint16_t)&PWM1PRL;         // Dest = PWM period register
    DMAnSSZ  = BUFFER_SIZE * 2;             // 2 bytes per trigger
    DMAnDSZ  = 2;

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
*/