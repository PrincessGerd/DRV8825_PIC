#include "../inc/clcx_hw.h"
#include "registers.h"
#include "../registers.h"

typedef struct clcx_hw{
    volatile uint8_t* SEL;
    volatile uint8_t* CON;
    volatile uint8_t* POL;
    volatile uint8_t* SEL0;
    volatile uint8_t* SEL1;
    volatile uint8_t* SEL2;
    volatile uint8_t* SEL3;
    volatile uint8_t* GLS0;
    volatile uint8_t* GLS1;
    volatile uint8_t* GLS2;
    volatile uint8_t* GLS3;
    uint8_t module_num;
} clcx_hw_t;

 void clcx_hw_create(
        uint8_t module_num, 
        clc_callback_t callback, 
        const struct clcx_hw ** clc_hw_inst_out){
            
    // attempted fix for clc not working
    static clcx_hw_t hw[CLC_NUM_MODULES];

    const uintptr_t base =
        CLC_BASE_ADDRESS + (module_num * CLC_OFFSET_NEXT_MODULE);
        
    hw[module_num].SEL  = (volatile uint8_t*)(base + CLC_SELECT_OFFSET),
    hw[module_num].CON  = (volatile uint8_t*)(base + CLC_CON_OFFSET),
    hw[module_num].POL  = (volatile uint8_t*)(base + CLC_POL_OFFSET),

    hw[module_num].SEL0 = (volatile uint8_t*)(base + CLC_SEL0_OFFSET),
    hw[module_num].SEL1 = (volatile uint8_t*)(base + CLC_SEL1_OFFSET),
    hw[module_num].SEL2 = (volatile uint8_t*)(base + CLC_SEL2_OFFSET),
    hw[module_num].SEL3 = (volatile uint8_t*)(base + CLC_SEL3_OFFSET),

    hw[module_num].GLS0 = (volatile uint8_t*)(base + CLC_GLS0_OFFSET),
    hw[module_num].GLS1 = (volatile uint8_t*)(base + CLC_GLS1_OFFSET),
    hw[module_num].GLS2 = (volatile uint8_t*)(base + CLC_GLS2_OFFSET),
    hw[module_num].GLS3 = (volatile uint8_t*)(base + CLC_GLS3_OFFSET),
    hw[module_num].module_num = module_num,
    *clc_hw_inst_out = &hw[module_num];
}

 void clcx_hw_init(const struct clcx_hw* self, clc_hw_config_t* config){
    *self->CON &= ~CLC_CON_EN_MASK;
    *self->SEL = self->module_num;

    *self->CON =
            (CLC_CON_MODE_MASK & config->mode) |
            ((CLC_CON_INTP_MASK | CLC_CON_INTN_MASK) & (config->int_polarity << CLC_CON_INTN_SHIFT));

    *self->POL =
        (config->invert_output               << CLC_POL_POL_SHIFT)   |
        (config->data_input1.invert_output   << CLC_POL_G1POL_SHIFT) |
        (config->data_input2.invert_output   << CLC_POL_G2POL_SHIFT) |
        (config->data_input3.invert_output   << CLC_POL_G3POL_SHIFT) |
        (config->data_input4.invert_output   << CLC_POL_G4POL_SHIFT);

    *self->SEL0 = config->data_input1.input_src;
    *self->SEL1 = config->data_input2.input_src;
    *self->SEL2 = config->data_input3.input_src;
    *self->SEL3 = config->data_input4.input_src;

    *self->GLS0 = config->data_input1.gate_sel.data;
    *self->GLS1 = config->data_input2.gate_sel.data;
    *self->GLS2 = config->data_input3.gate_sel.data;
    *self->GLS3 = config->data_input4.gate_sel.data;
}


void clcx_hw_enable(const struct clcx_hw* self){
    *self->SEL = self->module_num;
    *self->CON |= CLC_CON_EN_MASK;
}

void clcx_hw_disable(const struct clcx_hw* self){
    *self->SEL = self->module_num;
    *self->CON &= ~CLC_CON_EN_MASK;
}

 void clcx_hw_flip_output_polarity(const struct clcx_hw* self){
    bool enable_state = *self->CON & CLC_CON_EN_MASK;
    *self->SEL = self->module_num;
    *self->CON &= ~CLC_CON_EN_MASK;
    *self->POL ^= ~CLC_POL_POL_MASK;
    *self->CON |= (enable_state << CLC_CON_EN_SHIFT);
}

 void clcx_hw_change_gate_source(const struct clcx_hw* self, clc_hw_gates_e gate, uint8_t gate_src){
    bool enable_state = *self->CON & CLC_CON_EN_MASK;
    *self->SEL = self->module_num;
    *self->CON &= ~CLC_CON_EN_MASK;
    switch (gate) {
        case CLC_HW_GATE_1:
            *self->SEL0 = gate_src;
            break;
        case CLC_HW_GATE_2:
            *self->SEL1 = gate_src;
            break;
        case CLC_HW_GATE_3:
            *self->SEL2 = gate_src;
            break;
        case CLC_HW_GATE_4:
            *self->SEL3 = gate_src;
            break;
        default:
            break;
    }
    if(enable_state){
        *self->CON |= (1 << CLC_CON_EN_SHIFT);
    }
}