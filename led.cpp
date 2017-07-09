//
// Created by chudonghao on 17-7-9.
//

#include <stm32f10x_conf.h>
#include "led.h"

namespace cdh {
    bool led1_t::inited = false;

    driver_t *led1_t::open() {
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

    std::size_t led1_t::read(unsigned char *ptr, std::size_t size, std::size_t count) {
        return 1;
    }

    std::size_t led1_t::write(const unsigned char *ptr, std::size_t size, std::size_t count) {
        if (count) {
            GPIO_WriteBit(GPIOA, GPIO_Pin_8, (*ptr)?Bit_RESET:Bit_SET);
            return 1;
        } else
            return 0;
    }

    int led1_t::seek(long int offset) {
        return 0;
    }

    long led1_t::tell() {
        return 0;
    }

    int led1_t::close() {
        return 0;
    }

}

