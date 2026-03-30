#ifndef CORDIC_H
#define CORDIC_H
 
#include <stdint.h>
#include "math.h"
 
#define HALF_PI  (FP_ONE/4)
#define PI       (FP_ONE/2)
#define CORDIC_K (19897)
 
typedef enum {
  CORDIC_ROTATION = 0,
  CORDIC_VECTOR
} cordic_mode_e;
 
void cordic(int32_t *x, int32_t *y, int32_t* z, cordic_mode_e mode);

#define cordic_sincos(angle, cos_ptr, sin_ptr) \
do { \
    int32_t _z = (angle); \
    int32_t _x = CORDIC_K; \
    int32_t _y = 0; \
    cordic(&_x, &_y, &_z, CORDIC_ROTATION); \
    *(cos_ptr) = _x; \
    *(sin_ptr) = _y; \
} while(0)
 
#define cordic_sin(angle, sin_ptr) \
do { \
    int32_t _x = CORDIC_K; \
    int32_t _y = 0; \
    int32_t _z = (angle); \
    cordic(&_x, &_y, &_z, CORDIC_ROTATION); \
    *(sin_ptr) = _y; \
} while(0)
 
#define cordic_cos(angle, cos_ptr) \
do { \
    int32_t _x = CORDIC_K; \
    int32_t _y = 0; \
    int32_t _z = (angle); \
    cordic(&_x, &_y, &_z, CORDIC_ROTATION); \
    *(cos_ptr) = _x; \
} while(0)
 
#define cordic_atan(y, angle_ptr) \
do { \
    int32_t _x = 1; \
    int32_t _y = (y); \
    int32_t _z = 0; \
    cordic(&_x, &_y, &_z, CORDIC_VECTOR); \
    *(angle_ptr) = _z; \
} while(0)
 
#define cordic_atan2(y, x, angle_ptr) \
do { \
    int32_t _x = (x); \
    int32_t _y = (y); \
    int32_t _z = 0; \
    cordic(&_x, &_y, &_z, CORDIC_VECTOR); \
    *(angle_ptr) = _z; \
} while(0)
 
#define cordic_hypot(x, y, mag_ptr) \
do { \
    int32_t _x = (x); \
    int32_t _y = (y); \
    int32_t _z = 0; \
    cordic(&_x, &_y, &_z, CORDIC_VECTOR); \
    *(mag_ptr) = _x; \
} while(0)
#endif