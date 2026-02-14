#include "../inc/fixed_point.h"
#include <stdint.h>


fp8 fpdiv_impl(fp8 lhs, fp8 rhs){
    uint16_t overflow = (uint16_t)(lhs);
    overflow <<= FIXED_POINT_BITS;
    return (fp8)(overflow/rhs);
}

fp8 fpmul_impl(fp8 lhs, fp8 rhs){
    uint16_t overflow = (uint16_t)(lhs) * (uint16_t)(rhs);
    return (fp8)(overflow >> FIXED_POINT_BITS);
}

fp16 fpdiv16_impl(fp16 lhs, fp16 rhs){
    uint32_t overflow = (uint32_t)(lhs);
    overflow <<= FIXED_POINT_BITS;
    return (fp8)(overflow/rhs);
}

fp16 fpmul16_impl(fp16 lhs, fp16 rhs){
    const uint8_t frac_mask = ~(1U << FIXED_POINT_BITS);
    const uint8_t int_mask  = ~frac_mask;
    uint8_t ah = ((lhs & (int_mask)) >> FIXED_POINT_BITS);
    uint8_t bh = ((lhs & (int_mask)) >> FIXED_POINT_BITS);
    uint8_t al = (lhs & (frac_mask));
    uint8_t bl = (lhs & (frac_mask));
    return (fp16)(((ah*bh) << FIXED_POINT_BITS) + (ah*bl + bh*al) + ((al*bl) >> FIXED_POINT_BITS));
}