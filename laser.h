//
// Created by leu19 on 2017/7/28.
//

#ifndef CDH_LASER_H
#define CDH_LASER_H

#include <gpio.h>
namespace cdh{
    class laser_t {
    public:
        static inline void on() {
            HAL_GPIO_WritePin(laser_GPIO_Port,laser_Pin,GPIO_PIN_RESET);
        }

        static inline void off() {
            HAL_GPIO_WritePin(laser_GPIO_Port,laser_Pin,GPIO_PIN_SET);
        }
    };
}



#endif //CDH_LASER_H
