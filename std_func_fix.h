//
// Created by chudonghao on 17-7-24.
//

#ifndef CDH_STD_FUNC_FIX_H
#define CDH_STD_FUNC_FIX_H

#ifdef __cplusplus

#include <cmath>

namespace std {
#undef isnan
    static bool isnan(float x){ return ((sizeof(x) == sizeof(float))? __ARM_isnanf(x) : __ARM_isnan(x));}
#undef isinf
    static bool isinf(float x){ return ((sizeof(x) == sizeof(float))? __ARM_isinff(x) : __ARM_isinf(x));}
}

#else
#include <math.h>
#endif

#endif //CDH_STD_FUNC_FIX_H
