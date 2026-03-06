#ifndef MATH_H
#define MATH_H
#include <stdint.h>

///////////////////////////////////////////////
// FIXED POINT
///////////////////////////////////////////////
#define Q15_BITS 15
#define Q15_ONE (1 << Q15_BITS)
typedef uint16_t fp15_t;
static inline fp15_t fp_mul(fp15_t a, fp15_t b){
    return ((uint32_t)a * b) >> Q15_BITS;
}

static inline fp15_t fp_div(fp15_t a, fp15_t b){
    return (((uint32_t)a << Q15_BITS) / b);
}

///////////////////////////////////////////////
uint16_t isqrt16(uint16_t n);
///////////////////////////////////////////////
#endif