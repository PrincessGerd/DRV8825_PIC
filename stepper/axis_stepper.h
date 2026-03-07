#ifndef DUAL_AXIS_STEPPER_H
#define DUAL_AXIS_STEPPER_H

#include <stdint.h>
#include <stdbool.h>
#include "../inc/math.h"
#include "../core/task_manager.h"
#include "../core/dma_descriptor.h"

#define BUFFER_SIZE 32
struct axis_stepper;
uint16_t* axis_stepper_get_fill_buffer(struct axis_stepper* self);
void axis_stepper_instance(struct axis_stepper** inst_out, task_t* const owner, uint8_t module_num);
void axis_stepper_init(struct axis_stepper* const self);
void axis_stepper_start_move(
    struct axis_stepper* self,
    uint32_t steps  // steps in current axis
);  
#endif