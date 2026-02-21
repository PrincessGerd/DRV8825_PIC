#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H
#include <stdbool.h>
#include <stdint.h>

#define MESSAGE_MAX_LENGTH 8
typedef enum{
    TM_LOW_PRIORITY = 0,
    TM_HIGH_PRIORITY
} tm_priority_e;

struct task;
struct event;
typedef void(*event_handler_t)(struct task* const self, struct event* const e);
typedef struct task {
    struct event const** queue;
    uint8_t capacity;
    uint8_t count;
    uint8_t head;
    uint8_t tail;

    uint8_t irq_num;
    tm_priority_e prio;
    event_handler_t init;
    event_handler_t dispatch;
} task_t;

void task_create(
    task_t* const self, 
    event_handler_t init, 
    event_handler_t dispatch);

void task_start(
    task_t* const self,
    uint8_t capacity,
    tm_priority_e prio,
    struct event const** queue,
    struct event const * const ie);

// base event type, message
typedef uint8_t signal_t;
typedef struct event{
    task_t*  owner;      // pointer to the task that owns this event
    signal_t signal;     // signal of what should be handled by the dispatch task
}event_t;

void task_event_post(task_t* const self, struct event* const event);
void task_event_consume(task_t* const self);
void task_signal_post(task_t* const self,  signal_t signal);

void event_create(
    event_t* const self, // this event
    task_t* const owner, // the task this event belongs to
    signal_t signal);

// event for timing 
struct timed_event;
typedef struct timed_event{
    event_t super;
    uint16_t count;
    uint16_t interval;
    struct timed_event* next;
} timed_event_t;

// seperate global list for timed events
static timed_event_t const *tevent_head;
void timed_event_create(
    timed_event_t* const self, 
    task_t* const owner, 
    signal_t signal);

void timed_event_arm(
    timed_event_t* const self, 
    uint16_t count, 
    uint16_t interval);

void timed_event_disarm(
    timed_event_t* const self);

// fast tick events with imediate dispatch for motor controll
struct fast_tickEvt;
static struct fast_tickEvt *fast_tickEvt_head;
typedef struct fast_tickEvt{
    event_t super;
    uint32_t accumulator;
    uint32_t incrementor;
    struct fast_tickEvt* next;
} fast_tickEvt_t;

void fast_tick_event_create(
    fast_tickEvt_t* const self, 
    task_t* const owner,
    signal_t signal);

void fast_tick_event_arm(
    fast_tickEvt_t* const self, 
    uint32_t accumulator, 
    uint32_t incrementor);

void fast_tick_event_disarm(
    fast_tickEvt_t* const self);
#endif