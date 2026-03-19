#include "../inc/vector.h"

 #define FP_BITS 15U
#define FP_ONE (1U << FP_BITS)
typedef int16_t fp15_t; 

typedef struct{
    fp15_t x, y, z;
} vec_t;

vec_t vec_add(vec_t u, vec_t v){
    vec_t res = {u.x + v.x, u.y + v.y, u.z + v.z};
    return res;
}

vec_t vec_sub(vec_t u, vec_t v){
    vec_t res = {u.x - v.x, u.y - v.y, u.z - v.z};
    return res;
}

vec_t vec_mul(vec_t u, fp15_t s){
    vec_t res = {        
        fp_mul(u.x, s),
        fp_mul(u.y, s),
        fp_mul(u.z, s)
    };
    return res;
}

vec_t vec_div(vec_t u, fp15_t s){
    vec_t res = {
        fp_div(u.x, s),
        fp_div(u.y, s),
        fp_div(u.z, s)
    };
    return res;
}

vec_t vec_cross(vec_t u, vec_t v){
    vec_t res = {
        fp_mul(u.y, v.z) - fp_mul(u.z, u.y),
        fp_mul(u.z, v.x) - fp_mul(u.z, u.x),
        fp_mul(u.x, v.y) - fp_mul(u.y, u.x)
    };
    return res;
}

fp15_t vec_dot(vec_t u, vec_t v) {
    return fp_mul(u.x,v.x) + fp_mul(u.y, v.y) + fp_mul(u.z, v.z);
}

fp15_t vec_norm(vec_t u){
    uint32_t tmp = vec_dot(u,u);
    fp15_t res = (fp15_t)isqrt32(tmp);
    return res;
}

vec_t vec_normalise(vec_t u){
    fp15_t norm = vec_norm(u);
    if(norm == 0) return u;
    return vec_div(u,norm);
}
