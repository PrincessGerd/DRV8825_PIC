#ifndef SYSTEM_H
#define SYSTEM_H
#include <stdint.h>

#define SYSTEM_CLOCK_HZ (64000000/4)
typedef enum {
    FAST_TICK_TIMER,
    SYS_TICK_TIMER
} system_timer_e;

void system_init(void);
void systick_config(uint32_t freq, system_timer_e timer);
#endif