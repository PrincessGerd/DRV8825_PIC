#ifndef DMA_DESCRIPTOR_H
#define DMA_DESCRIPTOR_H

#include <stdint.h>
#include "../core/hw/inc/dma_hw.h"
typedef struct dma_descriptor{
    uint24_t src;
    uint16_t srcSize;
    uint24_t dst;
    uint16_t dstSize;
    struct dma_descriptor* next;
} dma_descriptor_t;

typedef struct dma_descriptor_handle {
    dma_descriptor_t* active;
    dma_descriptor_t* fill;
    struct dma_hw* dma;
    uint8_t trigger;
}dma_descriptor_handle_t; 

void dma_descriptor_setup(
    struct dma_descriptor_handle* handle, 
    struct dma_hw* dma, 
    uint8_t trigger);
void dma_descriptor_start(struct dma_descriptor_handle* handle);
void dma_descriptor_enqueue(
    struct dma_descriptor_handle* handle,
    dma_descriptor_t* desc);

void dma_descriptor_dispatch(struct dma_descriptor_handle* handle);

#endif
