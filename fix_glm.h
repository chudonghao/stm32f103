//
// Created by leu19 on 2017/7/27.
//

#ifndef CDH_FIX_GLM_H
#define CDH_FIX_GLM_H

#include <cmath>

#undef isnan
#undef isinf
namespace std{
    static inline bool isnan(float x){ return __ARM_isnanf(x);}
    static inline bool isinf(float x){ return __ARM_isinff(x);}
}

#endif //CDH_FIX_GLM_H
