#ifndef IRQ_DEVICE_H
#define IRQ_DEVICE_H

#include <stdbool.h>
#include <stdint.h>
#include "irq_manager.h"

typedef enum{
    IRQ_LOW_PRIORITY = 0,
    IRQ_HIGH_PRIORITY
} irq_priority_e;

struct irq_device;
struct irq_manager_ops;
typedef void(*irq_handler_t)(const struct irq_manager_ops** mngr, void* context); // ISR
typedef struct irq_device {
    irq_handler_t           handle;       // pointer to the isr
    void*                   context;
    uint8_t                 vic_offset;   // irq
    irq_priority_e          prio;
} irq_device_t;
#endif