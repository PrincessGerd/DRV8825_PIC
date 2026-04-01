#ifndef MATH_H
#define MATH_H
#include <stdint.h>


#define max(a,b) ((a < b) ? b : a)
#define min(a,b) ((a < b) ? a : b)

///////////////////////////////////////////////
// FIXED POINT
///////////////////////////////////////////////
#define Q15_BITS 15u // 14 fractional bits, one sign
#define Q15_ONE (1u << Q15_BITS)
typedef int16_t fp15_t;
fp15_t i32_to_fp15(int32_t n);
fp15_t fp15_div(fp15_t a, fp15_t b);
fp15_t fp15_mul(fp15_t a, fp15_t b);
int32_t mul_i32_q15(int32_t a, fp15_t b);
fp15_t fpsqrt(fp15_t n);
int32_t __mul_i16__(int16_t a, int16_t b);
///////////////////////////////////////////////
uint32_t isqrt32(uint32_t n);
///////////////////////////////////////////////
#endif