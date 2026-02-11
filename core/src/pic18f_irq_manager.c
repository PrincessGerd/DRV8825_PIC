#include "../irq_manager.h"
#include "../irq_device.h"
#include <stdint.h> 
#include <stddef.h>
#include "../types.h"
#include <xc.h>

// vic offset table. only used as a refrence for finding the pir, pie, and ipr 
typedef enum
{
    VIC_SOFT            = 0x00,
    VIC_INT0            = 0x01,
    VIC_INT1            = 0x02,
    VIC_INT2            = 0x03,

    VIC_DMA1SCNT        = 0x04,
    VIC_DMA1DCNT        = 0x05,
    VIC_DMA1OR          = 0x06,
    VIC_DMA1A           = 0x07,

    VIC_DMA2SCNT        = 0x08,
    VIC_DMA2DCNT        = 0x09,
    VIC_DMA2OR          = 0x0A,
    VIC_DMA2A           = 0x0B,

    VIC_DMA3SCNT        = 0x0C,
    VIC_DMA3DCNT        = 0x0D,
    VIC_DMA3OR          = 0x0E,
    VIC_DMA3A           = 0x0F,

    VIC_DMA4SCNT        = 0x10,
    VIC_DMA4DCNT        = 0x11,
    VIC_DMA4OR          = 0x12,
    VIC_DMA4A           = 0x13,

    VIC_NVM             = 0x14,
    VIC_CRC             = 0x15,
    VIC_SCAN            = 0x16,
    VIC_ACT             = 0x17,
    VIC_CSW             = 0x18,
    VIC_OSF             = 0x19,
    VIC_VDDIO2          = 0x1A,
    VIC_VDDIO3          = 0x1B,
    VIC_IOC             = 0x1C,

    VIC_TMR0            = 0x1D,
    VIC_TMR1            = 0x1E,
    VIC_TMR1G           = 0x1F,
    VIC_TMR2            = 0x20,
    VIC_TMR4            = 0x21,

    VIC_TU16A           = 0x22,
    VIC_TU16B           = 0x23,

    VIC_CCP1            = 0x24,
    VIC_CCP2            = 0x25,
    VIC_PWM1            = 0x26,

    VIC_PWM1INT         = 0x27,
    VIC_PWM2INT         = 0x28,
    VIC_PWM2GINT        = 0x29,

    VIC_CWG1            = 0x2A,
    VIC_CLC1            = 0x2B,
    VIC_CLC2            = 0x2C,
    VIC_CLC3            = 0x2D,
    VIC_CLC4            = 0x2E,

    VIC_IOCSR           = 0x2F,

    VIC_U1RX            = 0x30,
    VIC_U1TX            = 0x31,
    VIC_U1              = 0x32,
    VIC_U1E             = 0x33,

    VIC_U2RX            = 0x34,
    VIC_U2TX            = 0x35,
    VIC_U2              = 0x36,
    VIC_U2E             = 0x37,

    VIC_SPI1RX          = 0x38,
    VIC_SPI1TX          = 0x39,
    VIC_SPI1            = 0x3A,

    VIC_I2C1RX          = 0x3B,
    VIC_I2C1TX          = 0x3C,
    VIC_I2C1            = 0x3D,
    VIC_I2C1E           = 0x3E,

    VIC_I3C1RX          = 0x3F,
    VIC_I3C1TX          = 0x40,
    VIC_I3C1            = 0x41,
    VIC_I3C1E           = 0x42,
    VIC_I3C1R           = 0x43,

    VIC_I3C2RX          = 0x44,
    VIC_I3C2TX          = 0x45,
    VIC_I3C2            = 0x46,
    VIC_I3C2E           = 0x47,
    VIC_I3C2R           = 0x48,

    VIC_HLVD            = 0x4D,
    VIC_ADC             = 0x4E,
    VIC_ADT             = 0x4F,
    VIC_COUNT
} vic_offset_t;

#define NUM_INT_REGS 10
static volatile uint8_t* const pie_list[NUM_INT_REGS] = {&PIE0, &PIE1, &PIE2, &PIE3, &PIE4, &PIE5, &PIE6, &PIE7, &PIE8, &PIE9};
static volatile uint8_t* const pir_list[NUM_INT_REGS] = {&PIR0, &PIR1, &PIR2, &PIR3, &PIR4, &PIR5, &PIR6, &PIR7, &PIR8, &PIR9};
static volatile uint8_t* const ipr_list[NUM_INT_REGS] = {&IPR0, &IPR1, &IPR2, &IPR3, &IPR4, &IPR5, &IPR6, &IPR7, &IPR8, &IPR9};

#define MAX_IRQ_DEVICES 16
struct pic18f_irq{
    irq_manager_t dev;
    irq_device_t* irq_device_list[2][MAX_IRQ_DEVICES];   // high and low prio
    uint8_t dev_count[2];                                // high and low prio
};
static struct pic18f_irq instance;

static void pic18f_irq_register(irq_manager_t mngr, irq_device_t* _irq);
void pic18f_irq_disable(irq_manager_t mngr, uint8_t vic);
void pic18f_irq_write(irq_manager_t mngr, uint8_t vic, bool value);
void pic18f_irq_enable(irq_manager_t mngr, uint8_t vic);

static struct irq_manager_ops pic18f_irq_ops = {
    .register_dev = pic18f_irq_register,
    .enable   = pic18f_irq_enable,
    .disable   = pic18f_irq_disable,
    .write   = pic18f_irq_write
};

void irq_manager_instance(irq_manager_t* inst_out){
    if(!instance.dev){  // singleton instance
        instance.dev = &pic18f_irq_ops;
        instance.dev_count[0] = 0;
        instance.dev_count[1] = 0;
    }
    *inst_out = &instance.dev;
}

void pic18f_irq_enable(irq_manager_t mngr, uint8_t vic){
    struct pic18f_irq *self = container_of(mngr, struct pic18f_irq, dev);
    const uint8_t pix_idx = vic >> 3;   // since the pir and pie is proportional to the vic PIx[0-7] ~ VIC[0-7]
    const uint8_t bit = vic & 0x07;
    *pie_list[pix_idx] |= (1 << bit);
}

void pic18f_irq_disable(irq_manager_t mngr, uint8_t vic){
    struct pic18f_irq *self = container_of(mngr, struct pic18f_irq, dev);
    const uint8_t pix_idx = vic >> 3;
    const uint8_t bit = vic & 0x07;        
    *pie_list[pix_idx] &= ~(1 << bit);
}

void pic18f_irq_write(irq_manager_t mngr, uint8_t vic, bool value){
    struct pic18f_irq *self = container_of(mngr, struct pic18f_irq, dev);
    const uint8_t pix_idx = vic >> 3;
    const uint8_t bit = vic & 0x07;   
    if(value){
        *pir_list[pix_idx] |= (1 << bit);
    } else{
        *pir_list[pix_idx] &= ~(1 << bit);
    }
}

static void pic18f_irq_register(irq_manager_t mngr, irq_device_t* _irq){
    struct pic18f_irq *self = container_of(mngr, struct pic18f_irq, dev);
    const uint8_t pix_idx = _irq->vic_offset >> 3;
    const uint8_t bit = _irq->vic_offset & 0x07;
    if(self->dev_count[_irq->prio] < MAX_IRQ_DEVICES){
        self->irq_device_list[_irq->prio][self->dev_count[_irq->prio]] = _irq;
        self->dev_count[_irq->prio] += 1;
        if(_irq->prio){
            *ipr_list[pix_idx] |= (1<<bit);  // set high priority
        }else{
            *ipr_list[pix_idx] &= ~(1<<bit); // set low priority
        }
    }
}

//  high and low dispatcher, since i couldnt figure out how to swap ivt table at runtime
//  its most likely not intended to be posible since its staticaly linked.
//  but i thought it could be updated at runtime, since its posible to change the base
//  address of the ivt ;(
static void pic18f_irq_dispatcher(irq_priority_e priority){
    // workaround by having the manager as a singleton, 
    //  and using the instance for this private function
    //  this is very slow. but i dont want staticly linked isr's
    struct pic18f_irq *self = container_of(&instance.dev, struct pic18f_irq, dev);
    for(uint8_t i = 0; i < self->dev_count[priority]; i++){
        const uint8_t pix_idx = self->irq_device_list[priority][i]->vic_offset >> 3;
        const uint8_t bit     = self->irq_device_list[priority][i]->vic_offset & 0x07;
        if(*pir_list[pix_idx] & (1 << bit)){
            // pases the instance of the manager, and context stored in the irq_device
            self->irq_device_list[priority][i]->handle(&instance.dev, self->irq_device_list[priority][i]->context);
        }
    }
}

void __interrupt(high_priority) HighDispatcher(void) {
    pic18f_irq_dispatcher(IRQ_HIGH_PRIORITY);
}

void __interrupt(low_priority) LowDispatcher(void) {
    pic18f_irq_dispatcher(IRQ_LOW_PRIORITY);
}

//void __interrupt(irq(TMR2), base(0x8)) isr();