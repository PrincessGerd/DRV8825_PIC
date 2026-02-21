#include "../types.h"
#include "../task_manager.h"
#include "../interrupts.h"
#include <stdint.h>

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

#define mod(a,b) (a & (b-1))
#define PTR_ADD(p,val)  ((event_t* const)(p) + (val))
#define PTR_SUB(p,val)  ((event_t* const)(p) - (val))

void task_create(
    task_t* const self, 
    event_handler_t init, 
    event_handler_t dispatch){
        self->init = init;
        self->dispatch = dispatch;
}

void task_start(
    task_t* const self,
    uint8_t capacity,
    tm_priority_e prio,
    event_t const** queue,
    event_t const * const ie){
        self->queue = queue;
        self->capacity = capacity;
        self->head = 0;
        self->tail = 0;
        self->count = 0;
        self->prio = prio;
        interrupt_set_priority(self->irq_num, self->prio == TM_HIGH_PRIORITY);
        interrupt_enable(self->irq_num);
}

void task_event_post(task_t* const self, struct event* const event){
    LOCK();
    if (self->count < self->capacity) {
        self->queue[mod((self->tail + 1), self->capacity)] = event;
        self->tail = mod((self->tail + 1), self->capacity);
        self->count++;
    }
    UNLOCK();
}

void task_signal_post(task_t* const self,  signal_t signal){
    event_t event = {0};
    event.signal = signal;
    task_event_post(self, &event);
}

void task_event_consume(task_t* const self){
    LOCK();
    event_t* event;
    if (self->count > 0) {
        event = self->queue[mod((self->head + 1), self->capacity)];
        self->head = mod((self->head + 1), self->capacity);
        self->count--;
    }
    UNLOCK();
    self->dispatch(self, event);
}

void event_create(
    event_t* const self, 
    task_t* const owner, 
    signal_t signal){
        LOCK();
        self->owner = owner;
        self->signal = signal;
        UNLOCK();
}

void timed_event_create(
    timed_event_t* const self, 
    task_t* const owner, 
    signal_t signal){
        LOCK();
        self->super.owner = owner;
        self->super.signal = signal;
        self->next  = tevent_head;
        tevent_head = self;
        UNLOCK();
}

void timed_event_arm(timed_event_t* const self, uint16_t count, uint16_t interval){
    LOCK();
    self->count = count;
    self->interval = interval;
    UNLOCK();
}
void timed_event_disarm(timed_event_t* const self){
    LOCK();
    self->count = 0;
    self->interval = 0;
    UNLOCK();
}

void fast_tick_event_create(
    fast_tickEvt_t* const self, 
    task_t* const owner,
    signal_t signal){
        LOCK();
        self->super.owner    = owner;
        self->super.signal   = signal;
        UNLOCK();
    }

void fast_tick_event_arm(
    fast_tickEvt_t* const self, 
    uint32_t accumulator, 
    uint32_t incrementor){
        LOCK();
        self->accumulator = accumulator;
        self->incrementor = incrementor;
        self->next = fast_tickEvt_head;
        fast_tickEvt_head = self;
        UNLOCK();
    }

void fast_tick_event_disarm(
    fast_tickEvt_t* const self){
    // remove fast tick event from list
    fast_tickEvt_t* prev = fast_tickEvt_head;
    for(fast_tickEvt_t* ft = fast_tickEvt_head;
        ft != (fast_tickEvt_t*)0;
        ft = ft->next){
        LOCK();
        if(ft == self){
            prev->next = ft->next;
            UNLOCK();
            break;
        }
        prev = ft;
    }
    UNLOCK();
}

void fast_tick(void){
    for(fast_tickEvt_t* ft = fast_tickEvt_head;
        ft != (fast_tickEvt_t*)0;
        ft = ft->next){
        LOCK();;
            ft->accumulator += ft->incrementor;
            if(ft->accumulator >= (1UL << 16)){ // uses q16 fp
                ft->accumulator -= (1UL << 16);
                UNLOCK();
                ft->super.owner->dispatch(ft->super.owner,&ft->super);
            }
        }
    UNLOCK();
}