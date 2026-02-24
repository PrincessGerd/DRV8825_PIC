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
typedef void(*event_handler_t)(struct task* const self, struct event const* e);

typedef struct task {
    struct event** queue;
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
    task_t* self, 
    event_handler_t init, 
    event_handler_t dispatch);

void task_start(
    task_t* const self,
    uint8_t capacity,
    tm_priority_e prio,
    struct event** queue,
    struct event* ie);

// base event type, message
typedef uint8_t signal_t;
typedef struct event{
    signal_t signal;     // signal of what should be handled by the dispatch task
}event_t;

void task_event_post(task_t* self, struct event* event);
void task_event_consume(task_t* self);
#endif