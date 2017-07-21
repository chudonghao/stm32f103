//
// Created by chudonghao on 17-7-22.
//

#ifndef CDH_COMMON_FUNC_H
#define CDH_COMMON_FUNC_H
#error "just code template."

#include <stm32f10x_conf.h>
namespace cdh {
    namespace common_func {
        static inline void init_gpio(){
#ifndef CDH_GPIOx
#define CDH_GPIOx(x) GPIO##x
#endif
#ifndef CDH_GPIO_Pin_x
#define CDH_GPIO_Pin_x(x) GPIO_Pin_##x
#endif
#ifndef CDH_GPIO_INITSTRUCTURE
#define CDH_GPIO_INITSTRUCTURE
            GPIO_InitTypeDef GPIO_InitStruct;
#endif
#define TEM_GPIO_GROUP A//A~G
#define TEM_GPIO_Pin 0//0~15
//            GPIO_InitStruct.GPIO_Pin = CDH_GPIO_Pin_x(TEM_GPIO_Pin);


        }


        static inline void init_tim();
    }
}

#endif //CDH_COMMON_FUNC_H
