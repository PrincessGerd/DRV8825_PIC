#ifndef MATH_H
#define MATH_H
#include <stdint.h>

///////////////////////////////////////////////
// FIXED POINT
///////////////////////////////////////////////
#define Q15_BITS 14u // 14 fractional bits, one sign
#define Q15_ONE (1u << Q15_BITS)
typedef uint16_t fp15_t;
static inline fp15_t fp_mul(fp15_t a, fp15_t b){
    return ((uint32_t)a * b) >> Q15_BITS;
}

static inline fp15_t fp_div(fp15_t a, fp15_t b){
    return (((uint32_t)a << Q15_BITS) / b);
}

///////////////////////////////////////////////
uint32_t isqrt32(uint32_t n);
///////////////////////////////////////////////
#endif