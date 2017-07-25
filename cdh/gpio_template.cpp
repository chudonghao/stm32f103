//
// Created by leu19 on 2017/7/25.
//

#include <stm32f10x_conf.h>

namespace cdh{
    void init(){
        GPIO_InitTypeDef GPIO_InitStructure;
        RCC_APB2PeriphClockCmd(/*■■■*/RCC_APB2Periph_GPIOA, ENABLE);
        GPIO_InitStructure.GPIO_Pin = /*■■■*/GPIO_Pin_9;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = /*■■■*/GPIO_Mode_Out_PP;
        GPIO_Init(/*■■■*/GPIOA, &GPIO_InitStructure);
    }
    void write(){
        GPIO_WriteBit(/*■■■*/GPIOA,/*■■■*/GPIO_Pin_9,/*■■■*/1?Bit_SET:Bit_RESET);
    }
    void read(){
        u8 data = GPIO_ReadInputDataBit(/*■■■*/GPIOA,/*■■■*/GPIO_Pin_9);
    }
}
