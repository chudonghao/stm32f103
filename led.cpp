//
// Created by chudonghao on 17-7-9.
//

#include <stm32f10x_conf.h>
#include "led.h"

namespace cdh {
    bool led1_t::inited = false;

    led1_t * led1_t::open() {
        if (inited)
            return this;
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
        GPIO_InitTypeDef GPIO_InitStructure;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
        GPIO_WriteBit(GPIOA, GPIO_Pin_8, Bit_RESET);
				
        inited = true;
        return this;
    }

    int led1_t::close() {
        return 0;
    }

    void led1_t::on() {
        GPIO_WriteBit(GPIOA, GPIO_Pin_8,Bit_RESET);
    }

    void led1_t::off() {
        GPIO_WriteBit(GPIOA, GPIO_Pin_8, Bit_SET);
    }

}

