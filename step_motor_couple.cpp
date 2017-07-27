//
// Created by leu19 on 2017/7/11.
//

#include "step_motor_couple.h"
#include <stm32f10x_conf.h>
#include <cstdio>
#include <cmath>

using namespace std;
using namespace glm;
namespace {
    glm::ivec2 steps;
    glm::ivec2 next_steps;
}
namespace cdh {
    step_motor_couple_t *step_motor_couple_t::step_motor_couple = NULL;
    extern "C" {
    void /*■■■*/TIM2_IRQHandler() {
        if (TIM_GetITStatus(/*■■■*/TIM2, /*■■■*/TIM_IT_CC2) != RESET) {
            TIM_ClearITPendingBit(/*■■■*/TIM2, /*■■■*/TIM_IT_CC2);
            if (steps.x < next_steps.x) {
                ++steps.x;
            } else if (steps.x > next_steps.x) {
                --steps.x;
            }
            if (steps.x == next_steps.x) {
                TIM_Cmd(TIM2, DISABLE);
            }
        }
    }

    void /*■■■*/TIM5_IRQHandler() {
        if (TIM_GetITStatus(/*■■■*/TIM5, /*■■■*/TIM_IT_CC1) != RESET) {
            TIM_ClearITPendingBit(/*■■■*/TIM5, /*■■■*/TIM_IT_CC1);
            if (steps.y < next_steps.y) {
                ++steps.y;
            } else if (steps.y > next_steps.y) {
                --steps.y;
            }
            if (steps.y == next_steps.y) {
                TIM_Cmd(TIM5, DISABLE);
            }
        }
    }
    }

    step_motor_couple_t::step_motor_couple_t() {
        reset_current_position(0, 0);
    }

    step_motor_couple_t *step_motor_couple_t::init() {
        if (step_motor_couple)
            return step_motor_couple;
        step_motor_couple = new step_motor_couple_t();
        //PA0 TIM5_CH1
        //PA1 TIM2_CH2
        //PA2 dir_1
        //PA3 dir_2
        GPIO_InitTypeDef GPIO_InitStructure;
        TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
        NVIC_InitTypeDef NVIC_InitStructure;
        TIM_OCInitTypeDef TIM_OCInitStructure;
        //config rcc
        RCC_APB2PeriphClockCmd(/*■■■*/RCC_APB2Periph_GPIOA, ENABLE);
        RCC_APB1PeriphClockCmd(/*■■■*/RCC_APB1Periph_TIM5 | RCC_APB1Periph_TIM2, ENABLE);
        //config gpio
        GPIO_InitStructure.GPIO_Pin = /*■■■*/GPIO_Pin_0 | GPIO_Pin_1;
        GPIO_InitStructure.GPIO_Mode = /*■■■*/GPIO_Mode_AF_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(/*■■■*/GPIOA, &GPIO_InitStructure);
        GPIO_InitStructure.GPIO_Pin = /*■■■*/GPIO_Pin_2 | GPIO_Pin_3;
        GPIO_InitStructure.GPIO_Mode = /*■■■*/GPIO_Mode_Out_OD;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(/*■■■*/GPIOA, &GPIO_InitStructure);
        //config tim
        TIM_TimeBaseStructure.TIM_Period = /*■■■*/720 - 1;
        TIM_TimeBaseStructure.TIM_Prescaler = /*■■■*/72 - 1;
        TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
        TIM_TimeBaseStructure.TIM_CounterMode = /*■■■*/TIM_CounterMode_Up;
        TIM_ARRPreloadConfig(/*■■■*/TIM5, ENABLE);
        TIM_TimeBaseInit(/*■■■*/TIM5, &TIM_TimeBaseStructure);
        TIM_TimeBaseStructure.TIM_Period = /*■■■*/720 - 1;
        TIM_TimeBaseStructure.TIM_Prescaler = /*■■■*/72 - 1;
        TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
        TIM_TimeBaseStructure.TIM_CounterMode = /*■■■*/TIM_CounterMode_Up;
        TIM_ARRPreloadConfig(/*■■■*/TIM2, ENABLE);
        TIM_TimeBaseInit(/*■■■*/TIM2, &TIM_TimeBaseStructure);
        //config tim ch
        TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
        TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
        TIM_OCInitStructure.TIM_Pulse = /*■■■*/100;
        TIM_OCInitStructure.TIM_OCPolarity = /*■■■*/TIM_OCPolarity_High;
        /*■■■*/TIM_OC1PreloadConfig(/*■■■*/TIM5, TIM_OCPreload_Enable);
        /*■■■*/TIM_OC1Init(/*■■■*/TIM5, &TIM_OCInitStructure);
        TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
        TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
        TIM_OCInitStructure.TIM_Pulse = /*■■■*/100;
        TIM_OCInitStructure.TIM_OCPolarity = /*■■■*/TIM_OCPolarity_High;
        /*■■■*/TIM_OC2PreloadConfig(/*■■■*/TIM2, TIM_OCPreload_Enable);
        /*■■■*/TIM_OC2Init(/*■■■*/TIM2, &TIM_OCInitStructure);
        //config tim_it_cc
        TIM_ITConfig(/*■■■*/TIM5, /*■■■*/TIM_IT_CC1, ENABLE);
        NVIC_InitStructure.NVIC_IRQChannel = /*■■■*/TIM5_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = /*■■■*/0;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = /*■■■*/0;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
        TIM_ITConfig(/*■■■*/TIM2, /*■■■*/TIM_IT_CC2, ENABLE);
        NVIC_InitStructure.NVIC_IRQChannel = /*■■■*/TIM2_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = /*■■■*/0;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = /*■■■*/0;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
        return step_motor_couple;
    }

    void step_motor_couple_t::step(int motor1_steps, int motor2_steps) {
        while (set_next_steps(steps + ivec2(motor1_steps, motor2_steps)));
    }

    void step_motor_couple_t::step() {
        ivec2 steps_dir = steps - next_steps;
        if (steps_dir.x < 0) {
            steps_dir.x = -steps_dir.x;
        }
        if (steps_dir.y < 0) {
            steps_dir.y = -steps_dir.y;
        }
        if (length(vec2(steps_dir)) != 0)
            if (steps_dir.x == 0) {
                TIM_SetAutoreload(/*■■■*/TIM5, 600 - 1);
            } else if (steps_dir.y == 0) {
                TIM_SetAutoreload(/*■■■*/TIM2, 600 - 1);
            } else {
                if (steps_dir.x >= steps_dir.y) {
                    int k = steps_dir.x / steps_dir.y;
                    if (k > 65536 / 600) {
                        TIM_SetAutoreload(/*■■■*/TIM2, 600 - 1);
                        TIM_SetAutoreload(/*■■■*/TIM5, 65536 - 1);
                    } else {
                        TIM_SetAutoreload(/*■■■*/TIM2, 600 - 1);
                        TIM_SetAutoreload(/*■■■*/TIM5, 600 * k - 1);
                    }
                } else {
                    int k = steps_dir.y / steps_dir.x;
                    if (k > 65536 / 600) {
                        TIM_SetAutoreload(/*■■■*/TIM5, 600 - 1);
                        TIM_SetAutoreload(/*■■■*/TIM2, 65536 - 1);
                    } else {
                        TIM_SetAutoreload(/*■■■*/TIM5, 600 - 1);
                        TIM_SetAutoreload(/*■■■*/TIM2, 600 * k - 1);
                    }
                }
            }
    }

    ivec2 step_motor_couple_t::pre_map_position_to_steps(int x_mm, int y_mm) {
        ivec2 steps_of_motors = ivec2(x_mm, y_mm);
        return steps_of_motors;
    }

    void step_motor_couple_t::reset_current_position(int x_mm, int y_mm) {
        TIM_Cmd(/*■■■*/TIM2, DISABLE);
        TIM_Cmd(/*■■■*/TIM5, DISABLE);
        ivec2 steps_of_motors = pre_map_position_to_steps(x_mm, y_mm);
        steps = steps_of_motors;
        next_steps = steps_of_motors;
        //printf("reset_current_position steps=[%d,%d]\r\n", steps.int1, steps.int2);
    }

    int step_motor_couple_t::set_next_steps(const ivec2 &next_steps) {
        if (::next_steps == steps) {
            ::next_steps = next_steps;
            //prepare to move box
            ivec2 steps_dir = steps - next_steps;
            BitAction motor1_dir;
            BitAction motor2_dir;
            if (steps_dir.x > 0) {
                motor1_dir = Bit_RESET;
            } else if (steps_dir.x < 0) {
                motor1_dir = Bit_SET;
            }
            if (steps_dir.y > 0) {
                motor2_dir = Bit_RESET;
            } else if (steps_dir.y < 0) {
                motor2_dir = Bit_SET;
            }
            TIM_SetAutoreload(TIM2,0);
            TIM_SetAutoreload(TIM5,0);
            TIM_Cmd(/*■■■*/TIM2, ENABLE);
            TIM_Cmd(/*■■■*/TIM5, ENABLE);
            GPIO_WriteBit(/*■■■*/GPIOA,/*■■■*/GPIO_Pin_2, motor1_dir);
            GPIO_WriteBit(/*■■■*/GPIOA,/*■■■*/GPIO_Pin_3, motor1_dir);
            return 0;
        }
        return -1;
    }

    void step_motor_couple_t::test() {
        TIM_Cmd(/*■■■*/TIM2, ENABLE);
        TIM_Cmd(/*■■■*/TIM5, ENABLE);
    }

}
