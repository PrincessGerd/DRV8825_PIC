#include "../inc/ringbuffer.h"
#include <string.h>

static inline bool ring_buffer_is_full(const rb_state_t* rb) {
    if (!rb) return false;
    return rb->count == rb->capacity;
}

static inline bool ring_buffer_is_empty(const rb_state_t* rb) {
    if (!rb) return true;
    return rb->count == 0;
}

bool ring_buffer_enqueue(rb_state_t* rb, const void* item) {
    if (!rb || !item) return false;
    if (ring_buffer_is_full(rb)) return false;

    uint8_t* dest = rb->data + (rb->tail * rb->elem_size);
    memcpy(dest, item, rb->elem_size);

    rb->tail = RB_INDEX(rb, rb->tail + 1);
    rb->count++;
    return true;
}

bool ring_buffer_dequeue(rb_state_t* rb, void* out_item) {
    if (!rb || !out_item) return false;
    if (ring_buffer_is_empty(rb)) return false;

    uint8_t* src = rb->data + (rb->head * rb->elem_size);
    memcpy(out_item, src, rb->elem_size);

    rb->head = RB_INDEX(rb, rb->head + 1);
    rb->count--;
    return true;
}