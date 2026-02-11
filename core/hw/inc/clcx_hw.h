#ifndef CLCX_HW_H
#define CLCX_HW_H
#include <stdint.h>
#include <stdbool.h>

typedef enum{
    CLC_HW_MODE_AND_OR = 0,
    CLC_HW_MODE_OR_XOR,
    CLC_HW_MODE_4IN_AND,
    CLC_HW_MODE_SR_LATCH,
    CLC_HW_MODE_1IN_DFF_SR,
    CLC_HW_MODE_2IN_DFF_R,
    CLC_HW_MODE_JKFF_R,
    CLC_HW_MODE_TRANS_LATCH_SR,
}clc_hw_mode_e;

typedef enum {
    CLC_HW_INT_NONE = 0x0,
    CLC_HW_INT_POS_EDGE = 0x1,
    CLC_HW_INT_NEG_EDGE = 0x2
} clc_hw_intpol_e;

typedef enum {
    CLC_HW_GATE_1 = 0,
    CLC_HW_GATE_2 = 1,
    CLC_HW_GATE_3 = 2,
    CLC_HW_GATE_4 = 3,
}clc_hw_gates_e;

typedef union {
    struct{
        unsigned gate_1_pos : 1;
        unsigned gate_1_neg : 1;
        unsigned gate_2_pos : 1;
        unsigned gate_2_neg : 1;
        unsigned gate_3_pos : 1;
        unsigned gate_3_neg : 1;
        unsigned gate_4_pos : 1;
        unsigned gate_4_neg : 1;
    };
    uint8_t data;
} gate_selection_t;

struct data_selection{
    uint8_t input_src;
    volatile gate_selection_t gate_sel;
    bool invert_output;
};

typedef struct {
    clc_hw_mode_e mode;
    struct data_selection data_input1;
    struct data_selection data_input2;
    struct data_selection data_input3;
    struct data_selection data_input4;
    clc_hw_intpol_e int_polarity;
    bool invert_output;
} clc_hw_config_t;

struct clcx_hw;
typedef void (*clc_callback_t)(const struct clcx_hw* self);

void clcx_hw_create(uint8_t module_num, clc_callback_t callback, const struct clcx_hw ** clc_hw_inst_out);
void clcx_hw_init(const struct clcx_hw* self, clc_hw_config_t* config);
void clcx_hw_enable(const struct clcx_hw* self);
void clcx_hw_disable(const struct clcx_hw* self);
void clcx_hw_change_gate_source(const struct clcx_hw* self, clc_hw_gates_e gate, uint8_t gate_src);
void clcx_hw_flip_output_polarity(const struct clcx_hw* self);
#endif