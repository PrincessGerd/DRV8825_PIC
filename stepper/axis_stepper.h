#ifndef DUAL_AXIS_STEPPER_H
#define DUAL_AXIS_STEPPER_H

#include <stdint.h>
#include <stdbool.h>
#include "../inc/math.h"
#include "../core/task_manager.h"
#include "../core/dma_descriptor.h"

#define MAX_AXES 4

#define AXIS_STEPPER_BUFFER_SIZE 32
#define NUM_DESCRIPTORS 2

typedef union __attribute__((packed, aligned(4))) uv{
    struct{
        uint16_t u;
        uint16_t v;
    } axis;
    uint16_t uv[2];
} uv_t;

struct axis_stepper;
uv_t* axis_stepper_get_fill_buffer(struct axis_stepper* self);
void axis_stepper_instance(struct axis_stepper** inst_out, task_t* const owner, uint8_t module_num);
void axis_stepper_init(
    struct axis_stepper* const self, 
    uint8_t axis_count);
void axis_stepper_start_move(
    struct axis_stepper* self,
    uint32_t steps // steps in dominant axis
);  
#endif