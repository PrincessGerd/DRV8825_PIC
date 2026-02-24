#ifndef DMA_HW_H
#define DMA_HW_H
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    DMA_ACCESS_MODE_DECREMENT  = 0x2,
    DMA_ACCESS_MODE_INCREMENT  = 0x1,
    DMA_ACCESS_MODE_UNCHAINGED = 0x0,
} dma_acces_mode_e;

typedef enum {
    DMA_MEM_EEPROM_SEL   = 0x10,
    DMA_MEM_PG_FLASH_SEL = 0x1,
    DMA_MEM_SFR_GPR_SEL  = 0x0
} dma_memory_region_e;

typedef struct {
    dma_acces_mode_e dstAccess_mode;
    dma_acces_mode_e srcAccess_mode;
    dma_memory_region_e mem_region_sel;
    bool hw_int_trigger_start;
    bool hw_int_trigger_abort;
    bool dstIntClear_on_reset;
    bool srcIntClear_on_reset;
}dma_hw_config_t;

struct dma_hw;
void dma_hw_create(uint8_t module_num, const struct dma_hw ** dma_hw_inst_out);
void dma_hw_init(const struct dma_hw* self, const dma_hw_config_t* config);
void dma_hw_set_startirq(const struct dma_hw* self, uint8_t irq_num);
void dma_hw_set_abortirq(const struct dma_hw* self, uint8_t irq_num);
void dma_hw_set_src(const struct dma_hw* self, uintptr_t srcAddr);
void dma_hw_set_dst(const struct dma_hw* self, uintptr_t dstAddr);
void dma_hw_set_hwint(const struct dma_hw* self, bool start_trigg, bool abort_trigg);
void dma_hw_enable(const struct dma_hw* self);
void dma_hw_disable(const struct dma_hw* self);
#endif