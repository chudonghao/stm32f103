//
// Created by leu19 on 2017/7/20.
//

#ifndef CDH_MPU9250_H
#define CDH_MPU9250_H

#include "vec2.h"

namespace cdh{
    class mpu9250_t {
        mpu9250_t(){}
    public:
        static mpu9250_t*open();
        static void test();
        static vec2<float> acc();
    };
}




#endif //CDH_MPU9250_H
