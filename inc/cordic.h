#ifndef MATH_H
#define  MATH_H
 
#include <stdint.h>
 
#define FP_SHIFT 15
#define FP_ONE   (1U << FP_SHIFT)
typedef int16_t fp15_t;
 
#define HALF_PI FP_ONE/4
#define PI      FP_ONE/2
#define CORDIC_K 0x00009b75
 
typedef enum {
  CORDIC_ROTATION = 0,
  CORDIC_VECTOR
} cordic_mode_e;
 
void cordic(fp15_t *x, fp15_t *y, fp15_t* z, cordic_mode_e mode);

#define cordic_sincos(angle, cos_ptr, sin_ptr) \
do { \
    fp15_t _z = (angle); \
    fp15_t _x = CORDIC_K; \
    fp15_t _y = 0; \
    cordic(&_x, &_y, &_z, CORDIC_ROTATION); \
    *(cos_ptr) = _x; \
    *(sin_ptr) = _y; \
} while(0)
 
#define cordic_sin(angle, sin_ptr) \
do { \
    fp15_t _x = CORDIC_K; \
    fp15_t _y = 0; \
    fp15_t _z = (angle); \
    cordic(&_x, &_y, &_z, CORDIC_ROTATION); \
    *(sin_ptr) = _y; \
} while(0)
 
#define cordic_cos(angle, cos_ptr) \
do { \
    fp15_t _x = CORDIC_K; \
    fp15_t _y = 0; \
    fp15_t _z = (angle); \
    cordic(&_x, &_y, &_z, CORDIC_ROTATION); \
    *(cos_ptr) = _x; \
} while(0)
 
#define cordic_atan(y, angle_ptr) \
do { \
    fp15_t _x = 1; \
    fp15_t _y = (y); \
    fp15_t _z = 0; \
    cordic(&_x, &_y, &_z, CORDIC_VECTOR); \
    *(angle_ptr) = _z; \
} while(0)
 
#define cordic_atan2(y, x, angle_ptr) \
do { \
    fp15_t _x = (x); \
    fp15_t _y = (y); \
    fp15_t _z = 0; \
    cordic(&_x, &_y, &_z, CORDIC_VECTOR); \
    *(angle_ptr) = _z; \
} while(0)
 
#define cordic_hypot(x, y, mag_ptr) \
do { \
    fp15_t _x = (x); \
    fp15_t _y = (y); \
    fp15_t _z = 0; \
    cordic(&_x, &_y, &_z, CORDIC_VECTOR); \
    *(mag_ptr) = _x; \
} while(0)
#endif