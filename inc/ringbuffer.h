#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include <stdbool.h>

#define mod(a,b) (a & (b-1u))
#define PTR_ADD(p,val)  ((uint8_t*)(p) + (val))
#define PTR_SUB(p,val)  ((uint8_t*)(p) - (val))

#define STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#define MAX_BUFFER_LEN 256

typedef struct {
    uint16_t capacity;
    uint16_t block_len;
    uint16_t count;
    uint16_t head;
    uint16_t tail;
    uint8_t* data;
} rb_state_t;

// Static implementation
#define RING_BUFFER_DECLARE(name, capacity_, block_len_) \
    STATIC_ASSERT(capacity_ > 0 && capacity_ < MAX_BUFFER_LEN, "Invalid Size for ring buffer"); \
    static uint32_t name##_memory[sizeof(rb_state_t) + (capacity_ * block_len_)] __attribute__((space(prog))) = {0}; \
    static rb_state_t* name = (rb_state_t*)name##_memory; \
    void name##_init(void){ \
        name->capacity = capacity_; \
        name->block_len = block_len_; \
        name->count = 0; \
        name->head = 0; \
        name->tail = 0; \
        name->data = PTR_ADD(name, sizeof(rb_state_t)); \
    }

static inline bool ring_buffer_is_full(const rb_state_t* buffer);
static inline bool ring_buffer_is_empty(const rb_state_t* buffer);
void ring_buffer_enqueue(rb_state_t* buffer, uint8_t* data);
void ring_buffer_dequeue(rb_state_t* buffer, uint8_t* out_data);

/*
    RING_BUFFER_DECLARE(midi, 48, sizeof(uint32_t));

    int main() {
        rb_state_t* midi_buffer = midi_buffer_get();
        for(int i=0; i < 48; i++){
            ring_buffer_enqueue(midi_buffer, i);
            
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