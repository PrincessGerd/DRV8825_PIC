#include "utmr.h"
#include <stdint.h>
#include <assert.h>
#include "registers.h"

void utmr_init_impl(void* self, const utmr_config_t* config){
    utmr_regs_t* regs = &((utmr_instance_t*)self)->regs;
    // make sure the timer is off before configuring
    *regs->CON0 &= ~TU16_CON0_ON_MASK; 
    *regs->CON1 = 0U;
    
    *regs->HLT |= TU16_HLT_START_MASK & (config->start_cond << TU16_HLT_START_SHIFT);
    *regs->HLT |= TU16_HLT_RESET_MASK & (config->reset_cond << TU16_HLT_RESET_SHIFT);
    *regs->HLT |= TU16_HLT_STOP_MASK  & (config->stop_cond << TU16_HLT_STOP_SHIFT);
    *regs->HLT |= TU16_HLT_EPOL_MASK  & (config->invert_reset << TU16_HLT_EPOL_SHIFT);
    *regs->HLT |= TU16_HLT_CSYNC_MASK & (config->sync << TU16_HLT_CSYNC_SHIFT);
    
    *regs->CLK |= TU16_CLK_MASK & (config->clock_source <<TU16_CLK_SHIFT);
    *regs->ERS |= TU16_ERS_MASK  & (config->reset_source << TU16_ERS_SHIFT);
    *regs->PS  =  0U;

    *regs->CON0 |= TU16_CON0_CPOL_MASK  & (config->clk_polarity << TU16_CON0_CPOL_SHIFT);
    *regs->CON0 |= TU16_CON0_OPOL_MASK  & (config->invert_output << TU16_CON0_OPOL_SHIFT);
    *regs->CON0 |= TU16_CON0_OM_MASK    & (config->output_mode << TU16_CON0_OM_SHIFT);
    //*regs->CON0 |= TU16_CON0_RDSEL_MASK & (config->read_mode << TU16_CON0_RDSEL_SHIFT);
}

void utmr_enable_impl(void* self){
     utmr_regs_t* regs = &((utmr_instance_t*)self)->regs;
     *regs->CON0 |= (1U << TU16_CON0_ON_SHIFT);   
}

void utmr_disable_impl(void* self){
     utmr_regs_t* regs = &((utmr_instance_t*)self)->regs;
     *regs->CON0 &= ~(1U << TU16_CON0_ON_SHIFT);     
}

void utmr_set_prescaler_impl(void* self, uint8_t value){
    utmr_regs_t* regs = &((utmr_instance_t*)self)->regs;
    if((*regs->CON0 & TU16_CON0_ON_MASK) != 1){
        *regs->PS ^= *regs->PS; // clear register
        *regs->PS = value;
    }
}

void utmr_set_counter_impl(void* self, uint16_t value){
    utmr_regs_t* regs = &((utmr_instance_t*)self)->regs;
    // make sure the counter is off before accessing
    //*regs->TMR ^= *regs->TMR;            // clear register
    //*(regs->TMR + 1) ^= *(regs->TMR +1); // clear register
    if((*regs->CON0 & TU16_CON0_ON_MASK) != 1){
        *regs->CON0 |= (1U << TU16_CON0_RDSEL_SHIFT);
        *regs->TMR = (uint8_t)(value & 0xFF);               // set low byte
        *(regs->TMR + 1) = (uint8_t)((value >> 8) & 0xFF); // set high byte
        *regs->CON0 &= ~(1U << TU16_CON0_RDSEL_SHIFT);
    }
}
void utmr_set_period_impl(void* self, uint16_t value){
    utmr_regs_t* regs = &((utmr_instance_t*)self)->regs;
    // the period is buffered if the couter is on, so no need to check
    // if timer is enabled
    *regs->PR ^= *regs->PR;            // clear register
    *(regs->PR + 1) ^= *(regs->PR +1); // clear register
    *regs->PR = (uint8_t)(value & 0xFF);               // set low byte
    *(regs->PR + 1) = (uint8_t)((value >> 8) & 0xFF); // set high byte
}

void utmr_clear_impl(void* self){
    utmr_regs_t* regs = &((utmr_instance_t*)self)->regs;
    // clear counter and prescaler
    *regs->CON1 |= (1U << TU16_CON1_CLR_SHIFT);
    while((*regs->CON1 & (1U << TU16_CON1_CLR_SHIFT)) == 1U);
}

// dont know how i want this to work yet.
    // just sett mode manualy for now
void utmr_set_mode_impl(void* self, uint8_t mode){
    (void)mode;
}

void utmr_set_callback(void* self, utmr_callback_t callback){
    if(callback != 0){ 
        // probably a better way, but it works
        utmr_callbacks[((utmr_instance_t*)self)->module_num] = callback;
    }
}