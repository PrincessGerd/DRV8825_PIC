#include "../inc/cordic.h"

static const fp15_t cordic_atan[FP_SHIFT] = {
    0b0010000000000000,
    0b0001001011100100,
    0b0000100111111011,
    0b0000010100010001,
    0b0000001010001011,
    0b0000000101000110,
    0b0000000010100011,
    0b0000000001010001,
    0b0000000000101001,
    0b0000000000010100,
    0b0000000000001010,
    0b0000000000000101,
    0b0000000000000011,
    0b0000000000000001,
    0b0000000000000001
};
 

void cordic(fp15_t *x, fp15_t *y, fp15_t* z, cordic_mode_e mode){
  fp15_t dx = *x;
  fp15_t dy = *y;
  fp15_t dz = *z;
  fp15_t* d = mode == CORDIC_ROTATION ? &dz : &dy;
  for (int i = 0; i < FP_SHIFT; i++) {
      fp15_t x_shr = dx >> i;
      fp15_t y_shr = dy >> i;
      if(dz >= 0){
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