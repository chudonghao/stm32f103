//
// Created by leu19 on 2017/7/19.
//

#include "encoder.h"
#include <stm32f10x_conf.h>
#include "led.h"

using namespace cdh;

static int dir = 0;
static int angle = 0;
extern "C" {

void EXTI4_IRQHandler() {
    EXTI_ClearITPendingBit(EXTI_Line4);
    if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4) == Bit_SET) {
        dir = 1;
    } else {
        dir = -1;
    }
}

void EXTI9_5_IRQHandler() {
    EXTI_ClearITPendingBit(EXTI_Line5);
    angle += dir;
}

}
namespace cdh {
    encoder_t *encoder_t::encoder = 0;

    encoder_t *encoder_t::open() {
        if (encoder)return encoder;
        //pc4 pc5
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOC, ENABLE);

        GPIO_InitTypeDef GPIO_InitStructure;
        EXTI_InitTypeDef EXTI_InitStructure;
        NVIC_InitTypeDef NVIC_InitStructure;

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_4;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOC, &GPIO_InitStructure);

        if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4) == Bit_SET) {
            dir = -1;
        } else {
            dir = 1;
        }

        GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource4);
        EXTI_InitStructure.EXTI_Line = EXTI_Line4;
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
        EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
        EXTI_InitStructure.EXTI_LineCmd = ENABLE;
        EXTI_Init(&EXTI_InitStructure);

        NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0;
        NVIC_Init(&NVIC_InitStructure);

        GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource5);
        EXTI_InitStructure.EXTI_Line = EXTI_Line5;
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
        EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
        EXTI_InitStructure.EXTI_LineCmd = ENABLE;
        EXTI_Init(&EXTI_InitStructure);

        NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x1;
        NVIC_Init(&NVIC_InitStructure);

        return encoder;
    }

    int encoder_t::value() {
        return angle;
    }
}
