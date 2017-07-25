//
// Created by leu19 on 2017/7/25.
//
#include <stm32f10x_conf.h>
extern "C" void EXTI4_IRQHandler() {
    EXTI_ClearITPendingBit(/*■■■*/EXTI_Line4);

}
namespace cdh{
    void init(){
        GPIO_InitTypeDef GPIO_InitStructure;
        EXTI_InitTypeDef EXTI_InitStructure;
        NVIC_InitTypeDef NVIC_InitStructure;
        RCC_APB2PeriphClockCmd(/*■■■*/RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOC, ENABLE);

        GPIO_InitStructure.GPIO_Pin = /*■■■*/GPIO_Pin_4;
        GPIO_InitStructure.GPIO_Mode = /*■■■*/GPIO_Mode_IPU;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(/*■■■*/GPIOC, &GPIO_InitStructure);

        GPIO_EXTILineConfig(/*■■■*/GPIO_PortSourceGPIOC, /*■■■*/GPIO_PinSource4);
        EXTI_InitStructure.EXTI_Line = /*■■■*/EXTI_Line4;
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
        EXTI_InitStructure.EXTI_Trigger = /*■■■*/EXTI_Trigger_Rising_Falling;
        EXTI_InitStructure.EXTI_LineCmd = ENABLE;
        EXTI_Init(&EXTI_InitStructure);

        NVIC_InitStructure.NVIC_IRQChannel = /*■■■*/EXTI4_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = /*■■■*/0x0;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = /*■■■*/0x0;
        NVIC_Init(&NVIC_InitStructure);
    }


}
