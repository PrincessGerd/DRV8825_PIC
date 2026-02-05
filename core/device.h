#ifndef DEVICE_H
#define	DEVICE_H
#include <stdint.h>

struct device;
struct device_api;
typedef const struct device* device_t;

struct device{
    const struct device_api *api;
    const void* config;
    void* data;
    void (*init) (const struct device *dev);
};

extern struct timer_device TIMER_2;
extern struct timer_device TIMER_4;
#define MAX_INIT_ENTRIES 255
static const device_t* init_table[MAX_INIT_ENTRIES] = {
    &TIMER_2,
    &TIMER_4,
    0
};
static uint8_t init_count;

#define DECLARE_INIT_DEVICE(name,dev_ptr) \
    static void __attribute((constructor)) name##_register(void){ \
        init_table[init_count++] = dev_ptr; \
    }

void system_init(void);
#endif	/* DEVICE_H */

