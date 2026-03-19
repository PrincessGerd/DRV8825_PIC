#include "../inc/math.h"
#include <stdint.h>

#define Q15_BITS 15u
#define Q15_ONE (1u << Q15_BITS)
typedef int16_t fp15_t;

#define HALF_PI Q15_ONE/4
#define PI      Q15_ONE/2

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
 
    return ((int32_t)a_hi * b << 1) +
           (((int32_t)a_lo * b) >> 15);
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