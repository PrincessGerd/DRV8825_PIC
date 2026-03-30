#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_BUFFER_LEN 256
#define PTR_ADD(p,val)  ((void*)(p) + (val))
#define PTR_SUB(p,val)  ((void*)(p) - (val))
#define mod(a,b) (a & (b-1u))
#define RB_INDEX(rb, i) (mod(i, (rb)->capacity))

#define STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)

typedef struct {
    uint16_t capacity;
    uint16_t elem_size;
    uint16_t count;
    uint16_t head;
    uint16_t tail;
    uint8_t* data;
} rb_state_t;

#define RING_BUFFER_DECLARE(name, capacity_, type_) \
    static uint8_t name##_memory[sizeof(rb_state_t) + (capacity_ * sizeof(type_))] = {0}; \
    static rb_state_t* name = (rb_state_t*)name##_memory; \
    void name##_init(void) { \
        name->capacity = (capacity_); \
        name->elem_size = sizeof(type_); \
        name->count = 0; \
        name->head = 0; \
        name->tail = 0; \
        name->data = PTR_ADD(name, sizeof(rb_state_t)); \
    }


static inline bool ring_buffer_is_full(const rb_state_t* buffer);
static inline bool ring_buffer_is_empty(const rb_state_t* buffer);
bool ring_buffer_enqueue(rb_state_t* rb, const void* item);
bool ring_buffer_dequeue(rb_state_t* rb, void* out_item);

/*
    RING_BUFFER_DECLARE(midi, 48, uint32_t);

    int main() {
        midi_buffer_init();
        for(int i=0; i < 48; i++){
            ring_buffer_enqueue(midi_buffer, &i);
        }

        uint32_t data_out;
        for(int i=0; i < 48; i++){
            ring_buffer_dequeue(midi_buffer, &data_out);
            printf("%u\n", data_out);
        }
        ring_buffer_dequeue(midi_buffer, &data_out);

        return 0;
    }

*/

#endif