#ifndef DUAL_AXIS_STEPPER_H
#define DUAL_AXIS_STEPPER_H

#include <stdint.h>
#include <stdbool.h>
#include "../inc/math.h"
#include "../core/task_manager.h"

#define BUFFER_SIZE 32
struct axis_stepper;
void axis_stepper_get_inactive_buffer(struct axis_stepper* const self, uint16_t* buffer);
void axis_stepper_get_active_buffer(struct axis_stepper* const self, uint16_t* buffer);
void axis_stepper_fill_buffer(struct axis_stepper* const self, uint16_t* period);
void axis_stepper_instance(struct axis_stepper** inst_out, task_t* const owner, uint8_t module_num);
void axis_stepper_init(const struct axis_stepper* self);
void axis_stepper_start_move(
    struct axis_stepper* self,
    uint32_t steps,  // steps in current axis
    uint16_t n,       // normalised scalar in q1.15 for period
    uint16_t* period // common period 
);  
#endif