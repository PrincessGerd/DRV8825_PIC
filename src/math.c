#include "../inc/math.h"
#include <stdint.h>
#include <xc.h>

#define Q15_BITS 15u
#define Q15_ONE (1u << Q15_BITS)
typedef int16_t fp15_t;

#define HALF_PI Q15_ONE/4
#define PI      Q15_ONE/2


//extern uint32_t __mul_u16__(uint16_t a, uint16_t b);
//extern int32_t __mul_i16__(int16_t a, int16_t b);

uint32_t __umul32__(uint32_t x, uint32_t y) {
    uint16_t xH = (uint16_t)(x >> 16);
    uint16_t xL = (uint16_t)(x & 0xFFFF);
    uint16_t yH = (uint16_t)(y >> 16);
    uint16_t yL = (uint16_t)(y & 0xFFFF);

    uint32_t result = (uint32_t)xL * yL;
    result += ((uint32_t)xL * yH) << 16;
    result += ((uint32_t)xH * yL) << 16;

    return result;
}

uint32_t isqrt32(uint32_t n) {
    uint32_t x = n;
    uint32_t y = (x + 1) >> 1;
    while (y < x) {
        x = y;
        y = (x + n / x) >> 1;
    }
    return x;
}

fp15_t i32_to_fp15(int32_t n){
    return (int16_t)(n << Q15_BITS);
}
 
fp15_t fp15_div(fp15_t a, fp15_t b){
  return (((int32_t)a << Q15_BITS) / b);
}
 
fp15_t fp15_mul(fp15_t a, fp15_t b){
  return (((int32_t)a * b) >> Q15_BITS);
}
 
int32_t mul_i32_q15(int32_t a, fp15_t b) {
    int16_t a_hi = a >> 16;
    uint16_t a_lo = a & 0xFFFF;
 
    return (((int32_t)(a_hi * b)) << 1) +
           (((int32_t)(a_lo * b)) >> 15);
}

fp15_t fpsqrt(fp15_t n){
  uint32_t x = n;
  uint32_t y = (x + 1) >> 1;
  while(y < x){
    x = y;
    y = fp15_div((x + n), x) >> 1;
  }
  return (fp15_t)x;
}