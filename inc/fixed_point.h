/* 
 * File:   fixed_point.h
 * Author: Gard
 *
 * Created on January 20, 2026, 1:38 PM
 */

#ifndef FIXED_POINT_H
#define	FIXED_POINT_H
#include <stdint.h>

// must be less than 8 
#define FIXED_POINT_BITS 6
typedef uint8_t fp8; 
typedef uint16_t fp16; 

fp8 fpdiv_impl(fp8 lhs, fp8 rhs);
fp8 fpmul_impl(fp8 lhs, fp8 rhs);
fp16 fpdiv16_impl(fp16 lhs, fp16 rhs);
fp16 fpmul16_impl(fp16 lhs, fp16 rhs);
#endif	/* FIXED_POINT_H */

