//
// Created by leu19 on 2017/7/25.
//
#include <stm32f10x_conf.h>
namespace cdh{
    void init(){
        GPIO_InitTypeDef GPIO_InitStructure;
        TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
        NVIC_InitTypeDef NVIC_InitStructure;
        TIM_OCInitTypeDef TIM_OCInitStructure;
        //config rcc
        RCC_APB2PeriphClockCmd(/*■■■*/RCC_APB2Periph_GPIOC, ENABLE);
        RCC_APB2PeriphClockCmd(/*■■■*/RCC_APB2Periph_GPIOA, ENABLE);
        RCC_APB1PeriphClockCmd(/*■■■*/RCC_APB1Periph_TIM5, ENABLE);
        //config gpio

        //config gpio PA0-TIM5_CH1
        GPIO_InitStructure.GPIO_Pin = /*■■■*/GPIO_Pin_0;
        GPIO_InitStructure.GPIO_Mode = /*■■■*/GPIO_Mode_AF_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(/*■■■*/GPIOA, &GPIO_InitStructure);
        //config tim5
        TIM_TimeBaseStructure.TIM_Period = /*■■■*/720 - 1;
        TIM_TimeBaseStructure.TIM_Prescaler = /*■■■*/72 - 1;
        TIM_TimeBaseStructure.TIM_ClockDivision = 1 - 1;
        TIM_TimeBaseStructure.TIM_CounterMode = /*■■■*/TIM_CounterMode_Up;
        TIM_ARRPreloadConfig(/*■■■*/TIM5, ENABLE);
        TIM_TimeBaseInit(/*■■■*/TIM5, &TIM_TimeBaseStructure);
        //config tim5 ch1
        TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
        TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
        TIM_OCInitStructure.TIM_Pulse = /*■■■*/10;
        TIM_OCInitStructure.TIM_OCPolarity = /*■■■*/TIM_OCPolarity_High;
        /*■■■*/TIM_OC1PreloadConfig(/*■■■*/TIM5, TIM_OCPreload_Enable);
        /*■■■*/TIM_OC1Init(/*■■■*/TIM5, &TIM_OCInitStructure);

        //config tim5_it_cc1
        TIM_ITConfig(/*■■■*/TIM5, /*■■■*/TIM_IT_CC1, ENABLE);
        NVIC_InitStructure.NVIC_IRQChannel = /*■■■*/TIM5_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = /*■■■*/0;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = /*■■■*/0;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);

//        //enable tim5
//        TIM_Cmd(TIM5, ENABLE);


    }


}
