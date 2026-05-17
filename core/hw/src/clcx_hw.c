#include "../inc/clcx_hw.h"
#include "../registers.h"
#include "../registers.h"

typedef struct clcx_hw{
    uint8_t module_num; // registers are hidden so the module num is all thet is needed
} clcx_hw_t;

#define CLCSEL  (volatile uint8_t*)(CLC_BASE_ADDRESS + CLC_SELECT_OFFSET)
#define CLCCON  (volatile uint8_t*)(CLC_BASE_ADDRESS + CLC_CON_OFFSET)
#define CLCPOL  (volatile uint8_t*)(CLC_BASE_ADDRESS + CLC_POL_OFFSET)
#define CLCSEL0 (volatile uint8_t*)(CLC_BASE_ADDRESS + CLC_SEL0_OFFSET)
#define CLCSEL1 (volatile uint8_t*)(CLC_BASE_ADDRESS + CLC_SEL1_OFFSET)
#define CLCSEL2 (volatile uint8_t*)(CLC_BASE_ADDRESS + CLC_SEL2_OFFSET)
#define CLCSEL3 (volatile uint8_t*)(CLC_BASE_ADDRESS + CLC_SEL3_OFFSET)
#define CLCGLS0 (volatile uint8_t*)(CLC_BASE_ADDRESS + CLC_GLS0_OFFSET)
#define CLCGLS1 (volatile uint8_t*)(CLC_BASE_ADDRESS + CLC_GLS1_OFFSET)
#define CLCGLS2 (volatile uint8_t*)(CLC_BASE_ADDRESS + CLC_GLS2_OFFSET)
#define CLCGLS3 (volatile uint8_t*)(CLC_BASE_ADDRESS + CLC_GLS3_OFFSET)

 void clcx_hw_create(
        uint8_t module_num, 
        clc_callback_t callback, 
        const struct clcx_hw ** clc_hw_inst_out){
    static clcx_hw_t hw[CLC_NUM_MODULES];
    hw[module_num].module_num = module_num,
    *clc_hw_inst_out = &hw[module_num];
}

 void clcx_hw_init(const struct clcx_hw* self, clc_hw_config_t* config){
    *CLCCON &= ~CLC_CON_EN_MASK;
    *CLCSEL = self->module_num;

    *CLCCON =
            (CLC_CON_MODE_MASK & config->mode) |
            ((CLC_CON_INTP_MASK | CLC_CON_INTN_MASK) & (config->int_polarity << CLC_CON_INTN_SHIFT));

    *CLCPOL =
        (config->invert_output               << CLC_POL_POL_SHIFT)   |
        (config->data_input1.invert_output   << CLC_POL_G1POL_SHIFT) |
        (config->data_input2.invert_output   << CLC_POL_G2POL_SHIFT) |
        (config->data_input3.invert_output   << CLC_POL_G3POL_SHIFT) |
        (config->data_input4.invert_output   << CLC_POL_G4POL_SHIFT);

    *CLCSEL0 = config->data_input1.input_src;
    *CLCSEL1 = config->data_input2.input_src;
    *CLCSEL2 = config->data_input3.input_src;
    *CLCSEL3 = config->data_input4.input_src;

    *CLCGLS0 = config->data_input1.gate_sel.data;
    *CLCGLS1 = config->data_input2.gate_sel.data;
    *CLCGLS2 = config->data_input3.gate_sel.data;
    *CLCGLS3 = config->data_input4.gate_sel.data;
}


void clcx_hw_enable(const struct clcx_hw* self){
    *CLCSEL = self->module_num;
    *CLCCON |= CLC_CON_EN_MASK;
}

void clcx_hw_disable(const struct clcx_hw* self){
    *CLCSEL = self->module_num;
    *CLCCON &= ~CLC_CON_EN_MASK;
}

 void clcx_hw_flip_output_polarity(const struct clcx_hw* self){
    bool enable_state = *CLCCON & CLC_CON_EN_MASK;
    *CLCSEL = self->module_num;
    *CLCCON &= ~CLC_CON_EN_MASK;
    *CLCPOL ^= ~CLC_POL_POL_MASK;
    *CLCCON |= (enable_state << CLC_CON_EN_SHIFT);
}

 void clcx_hw_change_gate_source(const struct clcx_hw* self, clc_hw_gates_e gate, uint8_t gate_src){
    bool enable_state = *CLCCON & CLC_CON_EN_MASK;
    *CLCSEL = self->module_num;
    *CLCCON &= ~CLC_CON_EN_MASK;
    switch (gate) {
        case CLC_HW_GATE_1:
            *CLCSEL0 = gate_src;
            break;
        case CLC_HW_GATE_2:
            *CLCSEL1 = gate_src;
            break;
        case CLC_HW_GATE_3:
            *CLCSEL2 = gate_src;
            break;
        case CLC_HW_GATE_4:
            *CLCSEL3 = gate_src;
            break;
        default:
            break;
    }
    if(enable_state){
        *CLCCON |= (1 << CLC_CON_EN_SHIFT);
    }
}