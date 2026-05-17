#include "../inc/ringbuffer.h"
#include "../core/serial_logger.h"
#include <string.h>

inline bool ring_buffer_is_full(const rb_state_t* rb) {
    if (!rb) return false;
    return rb->count == rb->capacity;
}

inline bool ring_buffer_is_empty(const rb_state_t* rb) {
    if (!rb) return true;
    return rb->count == 0;
}

void ring_buffer_create(rb_state_t* rb_out, void* memory, unsigned int size, unsigned int elem_size){
    SLOG_ASSERT(rb_out != 0 || memory != 0);
    SLOG_ASSERT(size > sizeof(rb_state_t) + (elem_size << 1)); // absolute minimum for the ringbuffer to function 
    rb_state_t* lstate = (rb_state_t*)memory;
    lstate->capacity = mod(size, elem_size);
    lstate->elem_size = elem_size;
    lstate->count = 0;
    lstate->head = 0;
    lstate->tail = 0;
    lstate->data = PTR_ADD(memory, sizeof(rb_state_t));
    rb_out = lstate;
}

void ring_buffer_peek_tail(const rb_state_t* rb, void* data){
    SLOG_ASSERT(rb != 0 || data != 0);
    SLOG_ASSERT(!ring_buffer_is_empty(rb));
    uint8_t* src = rb->data + (rb->tail * rb->elem_size);
    memcpy(data, src, rb->elem_size);
}

void ring_buffer_enqueue(rb_state_t* rb, const void* item) {
    SLOG_ASSERT(rb != 0 || item != 0);
    SLOG_ASSERT(!ring_buffer_is_full(rb));

    uint8_t* dest = rb->data + (rb->tail * rb->elem_size);
    memcpy(dest, item, rb->elem_size);

    rb->tail = RB_INDEX(rb, rb->tail + 1);
    rb->count++;
}

void ring_buffer_dequeue(rb_state_t* rb, void* out_item) {
    SLOG_ASSERT(rb != 0 || out_item != 0);
    SLOG_ASSERT(!ring_buffer_is_empty(rb));

    uint8_t* src = rb->data + (rb->head * rb->elem_size);
    memcpy(out_item, src, rb->elem_size);

    rb->head = RB_INDEX(rb, rb->head + 1);
    rb->count--;
}

void ring_buffer_enqueue_chunk(rb_state_t* rb, const void* items, unsigned int len){
    SLOG_ASSERT(rb != 0 || items != 0);
    SLOG_ASSERT((rb->capacity - rb->count) > len);

    uint8_t* dest = rb->data + (rb->tail * rb->elem_size);
    memcpy(dest, items, rb->elem_size * len);

    rb->tail = RB_INDEX(rb, rb->tail + len);
    rb->count += len;
}

void ring_buffer_dequeue_chunk(rb_state_t* rb, const void* out_items, unsigned int len){
    SLOG_ASSERT(rb != 0 || out_items != 0);
    SLOG_ASSERT((rb->capacity - rb->count) > len);

    uint8_t* src = rb->data + (rb->head * rb->elem_size);
    memcpy(out_items, src, rb->elem_size * len);

    rb->head = RB_INDEX(rb, rb->head + len);
    rb->count -= len;
}