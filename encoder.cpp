//
// Created by leu19 on 2017/7/19.
//

#include "encoder.h"
#include <stm32f10x_conf.h>
#include <cstdio>
#include <arm_math.h>
#include <cmsis_os2.h>
#include "vec2.h"

using namespace cdh;
using namespace std;

static int dir = 0;
static int step = 0;

static arm_fir_instance_f32 arm_fir_instance;
static float lasted_angle;

//extern "C" void TIM4_IRQHandler() {
//    if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET) {
//        TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
//        float angle_ = (float) step / 1024 * 2 * M_PI * 22 / 32;
//        arm_fir_f32(&arm_fir_instance, &angle_, &lasted_angle, 1);
//        //TODO 不能在中断中 fir滤波
//        printf("test\r\n");
//    }
//};

extern "C" void EXTI4_IRQHandler() {
    EXTI_ClearITPendingBit(EXTI_Line4);
    if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4) != Bit_RESET) {
        dir = 1;
    } else {
        dir = -1;
    }
}

extern "C" void EXTI9_5_IRQHandler() {
    EXTI_ClearITPendingBit(EXTI_Line5);
    step += dir;
}

namespace cdh {
    encoder_t *encoder_t::encoder = 0;

    encoder_t *encoder_t::open() {
        if (encoder)return encoder;
        //pc4 pa5
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOA, ENABLE);

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
            ::dir = -1;
        } else {
            ::dir = 1;
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

//        //tim4
//        TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
//        //config rcc
//        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
//        //config tim4
//        TIM_TimeBaseStructure.TIM_Period = 36000 - 1;
//        TIM_TimeBaseStructure.TIM_Prescaler = 2 - 1;
//        TIM_TimeBaseStructure.TIM_ClockDivision = 0;
//        TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
//        TIM_ARRPreloadConfig(TIM4, ENABLE);
//        TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
//
//        TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
//        NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
//        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x3;
//        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0;
//        NVIC_Init(&NVIC_InitStructure);
//
//#define BLOCK_SIZE 50
//#define NUM_TAPS 30
//        static float coeffs[NUM_TAPS] = {
//                0.0005415165797, 0.001727426774, 0.003113061655, 0.003883380443, 0.002264012117,
//                -0.003352924949, -0.01260423567, -0.0218139533, -0.02428940125, -0.01267496031,
//                0.01740843058, 0.06416342407, 0.11868231, 0.1671814919, 0.1957704276,
//                0.1957704276, 0.1671814919, 0.11868231, 0.06416342407, 0.01740843058,
//                -0.01267496031, -0.02428940125, -0.0218139533, -0.01260423567, -0.003352924949,
//                0.002264012117, 0.003883380443, 0.003113061655, 0.001727426774, 0.0005415165797
//        };
//        static float state[BLOCK_SIZE + NUM_TAPS - 1];
//        arm_fir_init_f32(&arm_fir_instance, NUM_TAPS, coeffs, state, BLOCK_SIZE);
//        float tmp[50];
//        arm_fir_f32(&arm_fir_instance,tmp,tmp,50);
//        TIM_Cmd(TIM4, ENABLE);

        return encoder;
    }

    int encoder_t::value() {
        return step;
    }

    void encoder_t::test() {
        printf("step:%d,angle_origin:%f:,angle_fir:%f\r\n", step, (float) step / 1024 * 2 * M_PI * 22 / 32,
               lasted_angle);
    }

    float encoder_t::angle() {
//        static float input_data[50];
//        static float output_data[50];
//        int input_data_index = 0;
//        int time_now = TIM_GetCounter(TIM4);
//        time_now >>= 1;
//        time_now += second * 1000;
//
//        time_step_t *last_time_step_ = current_time_step;
//        int time_last;
//        float angle_last;
//        for (int i = 0; i < 20 && input_data_index < 50; ++i) {
//            time_last = last_time_step_->second * 1000 + last_time_step_->ms;
//            angle_last = (float) last_time_step_->step / 1024 * 2 * M_PI * 22 / 32;
//
//            for (; input_data_index < 50;) {
//                if (input_data_index <= time_now - time_last) {
//                    input_data[50 - input_data_index - 1] = angle_last;
//                    ++input_data_index;
//                } else {
//                    break;
//                }
//            }
//            if (last_time_step_ == &time_step[0]) {
//                last_time_step_ = &time_step[19];
//            } else {
//                --last_time_step_;
//            }
//        }
//        arm_fir_f32(&arm_fir_instance,input_data,output_data,BLOCK_SIZE);
//        return output_data[49];
        return (float) step / 1024 * 2 * M_PI * 22 / 32;
    }

    int encoder_t::dir() {
        return ::dir;
    }
}
