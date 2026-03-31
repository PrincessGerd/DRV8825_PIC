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
 
int32_t __mul_i16__(int16_t a, int16_t b){
  uint8_t a_hi = a << 8;
  uint8_t a_lo = (uint8_t)(a & 0x00FF);
  uint8_t b_hi = b << 8;
  uint8_t b_lo = (uint8_t)(b & 0x00FF);

  uint8_t r0, r1, r2, r3;

  r0 = a_lo * b_lo;
  r1 = a_hi * b_lo;
  r2 = a_lo * b_hi;
  r3 = b_hi * a_hi;

return  ( (int32_t) (r3 << 24) |
    (int32_t) (r2 << 16) |
    (int32_t) (r1 << 8)  |
    (int32_t) (r0));
}

int32_t mul_i32_q15(int32_t a, fp15_t b) {
    int16_t a_hi = a >> 16;
    uint16_t a_lo = a & 0xFFFF;
 
    return (((int32_t)a_hi * b) << 1) +
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