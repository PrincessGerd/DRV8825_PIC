#include "../types.h"
#include "../task_manager.h"
#include "../interrupts.h"
#include <stdint.h>
#include "../core/gpio.h"
#include <xc.h>

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
        TRISCbits.TRISC7 = 0;
        self->init(self,ie);
        //interrupt_set_priority(self->irq_num, self->prio == TM_HIGH_PRIORITY);
        //interrupt_enable(self->irq_num);
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

static timed_event_t *fast_tick_EvtHead = 0;
void fast_tick_event_create(
    timed_event_t* self, 
    task_t* owner,
    signal_t signal){
        self->owner = owner;
        self->super.signal = signal;
        self->accumulator = 0;
        self->incrementor = 0;
        self->armed = false;
        self->next = 0;
    }

void fast_tick_event_arm(
    timed_event_t* self, 
    uint16_t accumulator, 
    uint16_t incrementor){
        self->accumulator = accumulator;
        self->incrementor = incrementor;
        self->armed = true;
        self->next = fast_tick_EvtHead;
        fast_tick_EvtHead = self;
    }

void fast_tick_event_disarm(
    timed_event_t* self){
        LOCK();
        self->armed = false;
        timed_event_t** te = &fast_tick_EvtHead;
        while(*te != 0){
            if((*te) == self){
                (*te) = self->next;
                break;
            }
            *te = (*te)->next;
        }
        UNLOCK();
    }

void fast_tick_handler(void){
    disable_global_interrupts();
    timed_event_t* te = fast_tick_EvtHead;
    while(te != 0){
        task_t* owner = te->owner;
        if (!te->armed) {
            te = te->next;
            continue;
        }
        if (te->accumulator <= te->incrementor) {
            te->accumulator = te->incrementor;
            owner->dispatch(owner,&te->super);
            //gpio_toggle(RC_4);
        }else {
            te->accumulator -= te->incrementor;
        }
        te = te->next;
    }
    enable_global_interrupts();
}