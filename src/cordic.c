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

void cordic(fp15_t *x, fp15_t *y, fp15_t* z, cordic_mode_e mode){
  fp15_t dx = *x;
  fp15_t dy = *y;
  fp15_t dz = *z;
  for (int i = 0; i < Q15_BITS+1; i++) {
    int8_t d = mode == CORDIC_ROTATION ? (dz >= 0 ? 0 : -1) : (dy >= 0 ? 0 : -1);
      fp15_t x_shr = dx >> i;
      fp15_t y_shr = dy >> i;
      if(d >= 0){
        dx -= y_shr;
        dy += x_shr;
        dz -= cordic_atan[i];
      }else {
        dx += y_shr;
        dy -= x_shr;
        dz += cordic_atan[i];
      }
  }
  *x = dx;
  *y = dy;
  *z = dz;
}