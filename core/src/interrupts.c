#include "../interrupts.h"
#include <xc.h>

#define NUM_INT_REGS 10
static volatile uint8_t* const pie_list[NUM_INT_REGS] = {&PIE0, &PIE1, &PIE2, &PIE3, &PIE4, &PIE5, &PIE6, &PIE7, &PIE8, &PIE9};
static volatile uint8_t* const pir_list[NUM_INT_REGS] = {&PIR0, &PIR1, &PIR2, &PIR3, &PIR4, &PIR5, &PIR6, &PIR7, &PIR8, &PIR9};
static volatile uint8_t* const ipr_list[NUM_INT_REGS] = {&IPR0, &IPR1, &IPR2, &IPR3, &IPR4, &IPR5, &IPR6, &IPR7, &IPR8, &IPR9};

inline void enable_global_interrupts(){
    INTCON0bits.GIE = 1;
}
inline void disable_global_interrupts(){
    INTCON0bits.GIE = 0;
}

void interrupt_enable(uint8_t irq_num){
    // interrupt status registers are mapped 1:1 with vic irq numbers PIx[0-7] ~ VIC[0-7]
    const uint8_t pix_idx = irq_num >> 3;
    const uint8_t bit = irq_num & 0x07;
    *pie_list[pix_idx] |= (1 << bit);
}

void interrupt_disable(uint8_t irq_num){
    const uint8_t pix_idx = irq_num >> 3;
    const uint8_t bit = irq_num & 0x07;        
    *pie_list[pix_idx] &= ~(1 << bit);
}

bool interrupt_flag(uint8_t irq_num){
    const uint8_t pix_idx = irq_num >> 3;
    const uint8_t bit = irq_num & 0x07;
    return (*pir_list[pix_idx] & (1 <<bit));    
}

void interrupt_clear(uint8_t irq_num){
    const uint8_t pix_idx = irq_num >> 3;
    const uint8_t bit = irq_num & 0x07;        
    *pir_list[pix_idx] &= ~(1 << bit);
}

inline void interrupt_enable_priority(){
    INTCON0bits.GIEH    = 1;   /* Enables all high priority unmasked interrupts */
    INTCON0bits.GIEL    = 1;   /* Enables all low priority unmasked interrupts */
}

void interrupt_set_priority(uint8_t irq_num, bool high_prio){
    const uint8_t pix_idx = irq_num >> 3;
    const uint8_t bit = irq_num & 0x07;
    *ipr_list[pix_idx] &= ~(1<<bit);
    if(high_prio){
        *ipr_list[pix_idx] |= (1<<bit);
    }
}