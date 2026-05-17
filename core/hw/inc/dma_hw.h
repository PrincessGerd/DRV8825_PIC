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
        dma_memory_region_e mem_region;
        dma_acces_mode_e    src_acces_mode;
        dma_acces_mode_e    dest_acces_mode;
        bool                src_rst_on_done;
        bool                dest_rst_on_done;
    }dma_hw_config_t;

    struct dma_hw;
    void dma_hw_create(uint8_t module_num, struct dma_hw ** dma_hw_inst_out);

    //void dma_hw_init(const struct dma_hw* self, dma_hw_config_t config)
    void dma_hw_configure(
        const struct dma_hw* self,
        dma_memory_region_e mem_region,
        dma_acces_mode_e    src_acces_mode,
        dma_acces_mode_e    dest_acces_mode,
        bool src_rst_on_done,
        bool dest_rst_on_done);

    void dma_hw_arm(
        const struct dma_hw* self, 
        uint8_t trigger, 
        uint24_t src_addr,
        uint16_t src_len,
        uint24_t dest_addr,
        uint16_t dest_len);

    void dma_hw_set_arbiter_prio(
        const struct dma_hw* self, 
        uint8_t prio);


    void dma_hw_set_abortirq(const struct dma_hw* self, uint8_t irq_num);

    void dma_hw_enable(const struct dma_hw* self);
    void dma_hw_disable(const struct dma_hw* self);
#endif