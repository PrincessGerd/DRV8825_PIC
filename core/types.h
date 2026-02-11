#ifndef TPYES_H
#define TYPES_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define container_of(ptr, type, member) \
    ((type *)((uint8_t *)(ptr) - offsetof(type, member)))   

#endif