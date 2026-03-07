#include "../dma_descriptor.h"

void dma_descriptor_setup(
    struct dma_descriptor_handle* handle, 
    struct dma_hw* dma, 
    dma_descriptor_t* const desc,
    uint8_t trigger) {
        handle->dma = dma;
        handle->trigger = trigger;
        handle->active = desc;
        dma_hw_arm(
            handle->dma,
            handle->trigger,
            desc->src,
            desc->srcSize,
            desc->dst,
            desc->dstSize
        ); 
}

void dma_descriptor_enqueue(
    struct dma_descriptor_handle* handle,
    dma_descriptor_t* const desc) {
        dma_descriptor_t* head = handle->active;
        dma_descriptor_t* tail = head;

        while(tail->next != head){
            tail = tail->next;
        }
        desc->next = head;
        tail->next = desc;
}

void dma_descriptor_dispatch(struct dma_descriptor_handle* handle){
    dma_descriptor_t* tmp = handle->active;
    handle->active = handle->active->next;
    handle->fill   = tmp;
    dma_hw_arm(
        handle->dma,
        handle->trigger,
        handle->active->src,
        handle->active->srcSize,
        handle->active->dst,
        handle->active->dstSize
    ); 
}