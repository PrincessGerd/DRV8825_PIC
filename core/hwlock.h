#ifndef HWLOCK_H
#define HWLOCK_H
#include<stdint.h>
#include<stdbool.h>

#define HW_OWNER_NONE 0x0
#define HW_OWNER_INVALID 0xFF
typedef volatile uint8_t hw_owner_t;
typedef struct {
    hw_owner_t owner;
} hwlock_t;

bool hwlock_aquire(hwlock_t* lck, hw_owner_t owner);
bool hwlock_release(hwlock_t* lck, hw_owner_t owner);

#endif