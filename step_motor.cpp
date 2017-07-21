//
// Created by leu19 on 2017/7/19.
//

#include "step_motor.h"
#include "vec2.h"
#include <stm32f10x_conf.h>

static int step = 0;
static int dir = 0;
static int next_step = 0;

extern "C" void TIM3_IRQHandler() {
    if (TIM_GetITStatus(TIM3, TIM_IT_CC3) == SET) {
        TIM_ClearITPendingBit(TIM3, TIM_IT_CC3);
        step += dir;
        if(step == next_step)
            TIM_Cmd(TIM3,DISABLE);
    }
}

namespace cdh {
    cdh::step_motor_t *cdh::step_motor_t::open() {
        //pc0 pb0
        GPIO_InitTypeDef GPIO_InitStructure;
        TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
        NVIC_InitTypeDef NVIC_InitStructure;
        TIM_OCInitTypeDef TIM_OCInitStructure;
        //config rcc
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
        //config gpio
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOC, &GPIO_InitStructure);

        //config gpio PB0-TIM3_CH3
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOB, &GPIO_InitStructure);
        //config tim3
        TIM_TimeBaseStructure.TIM_Period = 720 - 1;
        TIM_TimeBaseStructure.TIM_Prescaler = 72 - 1;
        TIM_TimeBaseStructure.TIM_ClockDivision = 1 - 1;
        TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
        TIM_ARRPreloadConfig(TIM3, ENABLE);
        TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
        //config tim3_it_cc3
        TIM_ITConfig(TIM3, TIM_IT_CC3, ENABLE);
        NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
        //config tim3 ch3
        TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
        TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
        TIM_OCInitStructure.TIM_Pulse = 360;
        TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
        TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);
        TIM_OC3Init(TIM3, &TIM_OCInitStructure);

//        //enable tim3
//        TIM_Cmd(TIM3, ENABLE);
        return 0;
    }

    void step_motor_t::set_dir(int dir) {

    }

    void step_motor_t::set_next_step(int next_step) {
        TIM_Cmd(TIM3, DISABLE);
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
        TIM_Cmd(TIM3, ENABLE);
    }

    int step_motor_t::map_angle_to_step(float angle) {
        return angle / 2 / M_PI * 1600;
    }
}

