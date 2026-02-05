#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>
#include <stdint.h>

#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))


#endif