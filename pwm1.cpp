//
// Created by leu19 on 2017/7/10.
//

#include <stm32f10x_conf.h>
#include "pwm1.h"

extern "C" void (){

}

namespace cdh {

    bool pwm1_t::inited = false;

    pwm1_t *pwm1_t::open() {
        //tim2_ch3 pa2
        if (inited)
            return this;
        //使能计数器2通道3的引脚pa.2的时钟
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
        //使能计数器2的时钟
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
        //配置pa.2
        GPIO_InitTypeDef GPIO_InitStructure;
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
        //模式为复用推挽输出
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOA, &GPIO_InitStructure);

        //配置计数器2
        TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
        //计数器最大值（计数器重装载寄存器-16bit）
        TIM_TimeBaseStructure.TIM_Period = 59999;
        //预分频系数（预分频寄存器）
        TIM_TimeBaseStructure.TIM_Prescaler = 2;
        //时钟分频系数（未理解）
        TIM_TimeBaseStructure.TIM_ClockDivision = 0;
        // 向上计数溢出模式
        TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
        //使能计时器2的预装载（未理解）
        TIM_ARRPreloadConfig(TIM2, ENABLE);
        //已知系统时钟为72MHz 预分频系数为2
        //得计数器加（已知向上计数溢出模式）一频率=72/(2+1)=24MHz
        //已知计数器最大值为59999
        //得计数器频率=24/(59999+1)=0.4MHz
        TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

        //配置计时器2的通道3 O-输出C-比较
        TIM_OCInitTypeDef TIM_OCInitStructure;
        //模式（未理解）
        TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
        //输出使能
        TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
        //计数器脉搏值（比较值，比较寄存器），当计数器计数到这个值时，电平发生跳变 Pluse-脉搏
        TIM_OCInitStructure.TIM_Pulse = 3600;
        //比较成立极性，当定时器计数值小于比较值时为高电平
        TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
        //使能通道3预装载（未理解）输出比较预装载
        TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);
        TIM_OC3Init(TIM2, &TIM_OCInitStructure);
            
        //使能计时器2
        TIM_Cmd(TIM2, ENABLE);
        return this;
    }

    void pwm1_t::start() {

    }

    void pwm1_t::stop() {

    }
}
