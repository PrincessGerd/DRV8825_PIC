#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>
#include <stdbool.h>

typedef enum{
    LOW_PRIORITY  = 0,
    HIGH_PRIORITY = 1
} int_prio_e;

void enable_global_interrupts(void);
void disable_global_interrupts(void);
inline void interrupt_enable_priority();
void interrupt_enable(uint8_t irq_num);
void interrupt_disable(uint8_t irq_num);
bool interrupt_flag(uint8_t irq_num);
void interrupt_clear(uint8_t irq_num);
void interrupt_set_priority(uint8_t irq_num, bool high_prio);

#endif