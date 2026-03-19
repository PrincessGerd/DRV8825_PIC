#ifndef VECTOR_H
#define VECTOR_H

#include <stdint.h>
#include "math.h"

#define FP_BITS 15U
#define FP_ONE (1U << FP_BITS)
typedef uint16_t fp15_t; 

typedef struct{
    uint16_t x, y, z;
} vec_t;

vec_t vec_add(vec_t u, vec_t v);
vec_t vec_sub(vec_t u, vec_t v);
vec_t vec_mul(vec_t u, uint16_t s);
vec_t vec_div(vec_t u, uint16_t s);
vec_t vec_cross(vec_t u, vec_t v);
uint16_t vec_dot(vec_t u, vec_t v);
uint16_t vec_norm(vec_t u);
vec_t vec_normalise(vec_t u);

#endif