#ifndef IRQ_MANAGER_H
#define IRQ_MANAGER_H
#include "irq_device.h"

struct irq_device;
struct irq_manager_ops;
typedef const struct irq_manager_ops **irq_manager_t;
struct irq_manager_ops {
    void (*enable)(irq_manager_t mngr, uint8_t vic);
    void (*disable)(irq_manager_t mngr, uint8_t vic);
    void (*write)(irq_manager_t mngr, uint8_t vic, bool value);
    void (*register_dev)(irq_manager_t self, struct irq_device *dev);
};

void irq_manager_instance(irq_manager_t* inst_out);
#define irq_manager_register(manager, irq_ptr)((*(manager))->register_dev(manager,irq_ptr))
#define enable_interrupt(manager, vic)((*(manager))->enable(manager,vic))
#define disable_interrupt(manager, vic)((*(manager))->disable(manager,vic))
#define set_interrupt(manager, vic)((*(manager))->write(manager,vic,true))
#define clear_interrupt(manager, vic)((*(manager))->write(manager,vic,false))
#endif