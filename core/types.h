#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define container_of(ptr, type, member) \
    ((type *)((uint8_t *)(ptr) - offsetof(type, member)))   

#define event_of(e, type, sig) \
    ((e)->signal == (sig) ? \
     container_of(e, type, super) : NULL)

#endif