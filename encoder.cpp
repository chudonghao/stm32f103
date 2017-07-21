//
// Created by leu19 on 2017/7/19.
//

#include "encoder.h"
#include <stm32f10x_conf.h>
#include "vec2.h"

using namespace cdh;
using namespace std;
namespace {
    static int dir = 0;
    static int step = 0;
//    typedef struct {
//        time_t time;
//        int angle;
//    } time_angle_t;
//    static time_angle_t time_angle[400];
}

extern "C" void EXTI4_IRQHandler() {
    EXTI_ClearITPendingBit(EXTI_Line4);
    TIM_SetCounter(TIM4, 0x0);
    if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4) == Bit_SET) {
        dir = 1;
    } else {
        dir = -1;
    }
}

extern "C" void EXTI9_5_IRQHandler() {
    EXTI_ClearITPendingBit(EXTI_Line5);
    step += dir;
//    time_angle[angle+200].time = TIM_GetCounter(TIM4);
//    time_angle[angle+200].angle = angle;
}

namespace cdh {
    encoder_t *encoder_t::encoder = 0;

    encoder_t *encoder_t::open() {
        if (encoder)return encoder;
        //pc4 pa5
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOC| RCC_APB2Periph_GPIOA, ENABLE);

        GPIO_InitTypeDef GPIO_InitStructure;
        EXTI_InitTypeDef EXTI_InitStructure;
        NVIC_InitTypeDef NVIC_InitStructure;

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOC, &GPIO_InitStructure);
			  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOA, &GPIO_InitStructure);

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

        GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource5);
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

        //tim4
        TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
        //config rcc
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
        //config tim4
        TIM_TimeBaseStructure.TIM_Period = 60000 - 1;
        TIM_TimeBaseStructure.TIM_Prescaler = 2400 - 1;
        TIM_TimeBaseStructure.TIM_ClockDivision = 1 - 1;
        TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
        TIM_ARRPreloadConfig(TIM4, ENABLE);
        TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
        TIM_Cmd(TIM4, ENABLE);

        return encoder;
    }

    int encoder_t::value() {
        return step;
    }

    void encoder_t::test() {
//        static int old_dir = dir;
//        if (old_dir != dir) {
//            for (int i = 0; i < 400; ++i) {
//                printf("%u,", time_angle[i].time);
//            }
//            printf("\r\n");
//            for (int i = 0; i < 400; ++i) {
//                printf("%d,", time_angle[i].angle);
//            }
//            printf("\r\n");
//            old_dir = dir;
//            memset(time_angle,0, sizeof(time_angle));
//        }
    }

    float encoder_t::angle() {
        return (float) step / 1024 * 2 * M_PI * 22 / 32;
    }
}
