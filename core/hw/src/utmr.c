#include "../inc/utmr_hw.h"
#include "../registers.h"
#include <stdint.h>
#include <assert.h>

struct utmr_hw {
    volatile uint8_t* CON0;
    volatile uint8_t* CON1;
    volatile uint8_t* HLT;
    volatile uint8_t* PS;
    volatile uint8_t* TMR;
    volatile uint8_t* CR;
    volatile uint8_t* PR;
    volatile uint8_t* CLK;
    volatile uint8_t* ERS;
};


void utmr_create(const struct utmr_hw* utmr_hw_inst_out, uint8_t module_num){
    const uintptr_t base =
        TU16_BASE_ADDRESS + (module_num * TU16_OFFSET_NEXT_MODULE);
    
    struct utmr_hw hw = {
            .CON0 = (volatile uint8_t*)(base + TU16_CON0_OFFSET),
            .CON1 = (volatile uint8_t*)(base + TU16_CON1_OFFSET),
            .HLT  = (volatile uint8_t*)(base + TU16_HLT_OFFSET),
            .PS   = (volatile uint8_t*)(base + TU16_PS_OFFSET),
            .TMR  = (volatile uint8_t*)(base + TU16_TMR_OFFSET),
            .CR   = (volatile uint8_t*)(base + TU16_CR_OFFSET),
            .PR   = (volatile uint8_t*)(base + TU16_PR_OFFSET),
            .CLK  = (volatile uint8_t*)(base + TU16_CLK_OFFSET),
            .ERS  = (volatile uint8_t*)(base + TU16_ERS_OFFSET),
    };
    utmr_hw_inst_out = &hw;
}

void utmr_init(const struct utmr_hw* self, const utmr_config_t* config){
    *self->CON0 &= ~TU16_CON0_ON_MASK; 
    *self->CON1 = 0U;
    *self->CON0 = 0U;
    *self->HLT  = 0U;
    
    *self->HLT |= TU16_HLT_START_MASK & (config->start_cond << TU16_HLT_START_SHIFT);
    *self->HLT |= TU16_HLT_RESET_MASK & (config->reset_cond << TU16_HLT_RESET_SHIFT);
    *self->HLT |= TU16_HLT_STOP_MASK  & (config->stop_cond << TU16_HLT_STOP_SHIFT);
    *self->HLT |= (config->invert_reset << TU16_HLT_EPOL_SHIFT) | 
                    (config->sync << TU16_HLT_CSYNC_SHIFT);
    

    *self->CLK |= TU16_CLK_MASK & (config->clock_source <<TU16_CLK_SHIFT);
    *self->ERS |= TU16_ERS_MASK  & (config->reset_source << TU16_ERS_SHIFT);
    *self->PS  =  0U;
    
    *self->CON0 |= (config->clk_polarity << TU16_CON0_CPOL_SHIFT) | 
        (config->invert_output << TU16_CON0_OPOL_SHIFT) |
        (config->output_mode << TU16_CON0_OM_SHIFT);
}

void utmr_enable(const struct utmr_hw* self){
     *self->CON0 |= (1U << TU16_CON0_ON_SHIFT);   
}

void utmr_disable(const struct utmr_hw* self){
     *self->CON0 &= ~(1U << TU16_CON0_ON_SHIFT);     
}

void utmr_set_prescaler(const struct utmr_hw* self, uint8_t value){
    if((*self->CON0 & TU16_CON0_ON_MASK) != 1){
        *self->PS ^= *self->PS; // clear register
        *self->PS = value;
    }
}

void utmr_set_counter(const struct utmr_hw* self, uint16_t value){
    // make sure the counter is off before accessing
    //*self->TMR ^= *self->TMR;            // clear register
    //*(self->TMR + 1) ^= *(self->TMR +1); // clear register
    if((*self->CON0 & TU16_CON0_ON_MASK) != 1){
        *self->CON0 |= (1U << TU16_CON0_RDSEL_SHIFT);
        *self->TMR = (uint8_t)(value & 0xFF);               // set low byte
        *(self->TMR + 1) = (uint8_t)((value >> 8) & 0xFF); // set high byte
        *self->CON0 &= ~(1U << TU16_CON0_RDSEL_SHIFT);
    }
}
void utmr_set_period(const struct utmr_hw* self, uint16_t value){
    // the period is buffered if the couter is on, so no need to check
    // if timer is enabled
    //*self->PR ^= *self->PR;            // clear register
    //*(self->PR + 1) ^= *(self->PR +1); // clear register
    *self->PR = (uint8_t)(value & 0xFF);               // set low byte
    *(self->PR + 1) = (uint8_t)((value >> 8) & 0xFF); // set high byte
}

void utmr_clear(const struct utmr_hw* self){
    // clear counter and prescaler
    *self->CON1 |= (1U << TU16_CON1_CLR_SHIFT);
    while((*self->CON1 & (1U << TU16_CON1_CLR_SHIFT)) == 1U);
}