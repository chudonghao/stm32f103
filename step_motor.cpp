//
// Created by leu19 on 2017/7/19.
//

#include "step_motor.h"
#include "vec2.h"
#include <stm32f10x_conf.h>

static int step = 0;
static int dir = 0;
static int next_step = 0;

extern "C" void TIM5_IRQHandler() {
    if (TIM_GetITStatus(TIM5, TIM_IT_CC1) == SET) {
        TIM_ClearITPendingBit(TIM5, TIM_IT_CC1);
        step += dir;
        if(step == next_step)
            TIM_Cmd(TIM5,DISABLE);
    }
}

namespace cdh {
    cdh::step_motor_t *cdh::step_motor_t::open() {
        //pc0 pa0
        GPIO_InitTypeDef GPIO_InitStructure;
        TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
        NVIC_InitTypeDef NVIC_InitStructure;
        TIM_OCInitTypeDef TIM_OCInitStructure;
        //config rcc
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
        //config gpio
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOC, &GPIO_InitStructure);

        //config gpio PA0-TIM5_CH1
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
        //config tim5
        TIM_TimeBaseStructure.TIM_Period = 720 - 1;
        TIM_TimeBaseStructure.TIM_Prescaler = 72 - 1;
        TIM_TimeBaseStructure.TIM_ClockDivision = 1 - 1;
        TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
        TIM_ARRPreloadConfig(TIM5, ENABLE);
        TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);
        //config tim5_it_cc1
        TIM_ITConfig(TIM5, TIM_IT_CC1, ENABLE);
        NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
        //config tim5 ch1
        TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
        TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
        TIM_OCInitStructure.TIM_Pulse = 10;
        TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
        TIM_OC1PreloadConfig(TIM5, TIM_OCPreload_Enable);
        TIM_OC1Init(TIM5, &TIM_OCInitStructure);

//        //enable tim5
//        TIM_Cmd(TIM5, ENABLE);
        return 0;
    }

    void step_motor_t::set_dir(int dir) {

    }

    void step_motor_t::set_next_step(int next_step,int p) {
        TIM_Cmd(TIM5, DISABLE);
        if(p>=720)
            TIM_SetAutoreload(TIM5,p);

        ::next_step = next_step;
        if (::next_step > ::step) {
            dir = 1;
            GPIO_WriteBit(GPIOC, GPIO_Pin_0, Bit_SET);
        } else if(::next_step < ::step){
            dir = -1;
            GPIO_WriteBit(GPIOC, GPIO_Pin_0, Bit_RESET);
        } else{
            return;
        }
        TIM_Cmd(TIM5, ENABLE);
    }

    int step_motor_t::map_angle_to_step(float angle) {
        return angle / 2 / M_PI * 3200;
    }
}

