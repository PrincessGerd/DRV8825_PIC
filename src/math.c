#include "../inc/math.h"
#include <stdint.h>

uint16_t isqrt16(uint16_t n) {
    uint32_t x = n;
    uint32_t y = (x + 1) >> 1;
    while (y < x) {
        x = y;
        y = (x + n / x) >> 1;
    }
    return (uint16_t)x;
}