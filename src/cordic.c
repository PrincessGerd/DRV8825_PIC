#include "../inc/cordic.h"
#include "../inc/math.h"

 
static const int16_t cordic_atan[Q15_BITS+1] = {
  25735,
  15192,
  8027,
  4074,
  2045,
  1023,
  512,
  256,
  128,
  64,
  32,
  16,
  8,
  4,
  2,
  1};

void cordic(int32_t *x, int32_t *y, int32_t* z, cordic_mode_e mode){
  int32_t dx = *x;
  int32_t dy = *y;
  int32_t dz = *z;
  int16_t d = 0;
  for (int i = 0; i < Q15_BITS+1; i++) {
      d = mode == CORDIC_ROTATION ? (dz >= 0 ? 1 : -1) : (dy >= 0 ? -1 : 1);
      int16_t x_shr = dx >> i;
      int16_t y_shr = dy >> i;
      dx -= d*y_shr;
      dy += d*x_shr;
      dz -= d*cordic_atan[i];
  }
  *x = dx;
  *y = dy;
  *z = dz;
}