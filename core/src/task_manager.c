#include "../types.h"
#include "../task_manager.h"
#include "../interrupts.h"
#include <stdint.h>
#include "../core/gpio.h"

static volatile uint8_t lock_count = 0;

#define LOCK() \
    do { \
        if(lock_count == 0) { \
            disable_global_interrupts(); \
        } \
        lock_count++; \
    } while(0)

#define UNLOCK() \
    do { \
        if(lock_count > 0) { \
            lock_count--; \
            if(lock_count == 0) { \
                enable_global_interrupts(); \
            } \
        } \
    } while(0)

#define mod(a,b) (a & (b-1u))
#define PTR_ADD(p,val)  ((event_t const*)(p) + (val))
#define PTR_SUB(p,val)  ((event_t const*)(p) - (val))

void task_create(
    task_t*  self, 
    event_handler_t init, 
    event_handler_t dispatch){
        self->init = init;
        self->dispatch = dispatch;
}

void task_start(
    task_t* const self,
    uint8_t capacity,
    tm_priority_e prio,
    event_t** queue,
    event_t * ie){
        self->queue = queue;
        self->capacity = capacity;
        self->head = 0;
        self->tail = 0;
        self->count = 0;
        self->prio = prio;
        self->init(self,ie);
}

void task_event_post(task_t* self, struct event* event){
    LOCK();
    if (self->count < self->capacity) {
        self->queue[self->tail] = event;
        self->tail = mod((self->tail + 1), self->capacity);
        self->count++;
    }
    UNLOCK();
}

void task_event_consume(task_t* self){
    const struct event* event = 0;
    LOCK();
    if (self->count > 0) {
        event = self->queue[self->head];
        self->head = mod((self->head + 1),self->capacity);
        self->count--;
    }
    UNLOCK();
    self->dispatch(self, event);
}