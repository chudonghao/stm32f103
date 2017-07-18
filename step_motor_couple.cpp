//
// Created by leu19 on 2017/7/11.
//

#include "step_motor_couple.h"
#include "point_array.h"
#include "box.h"
#include <stm32f10x_conf.h>
#include <cstdio>
#include <cmath>
#include <rtx_os.h>


#define CDH_DISTANCE_BETWEEN_CONTROL_POINT_AND_PEN 8
using namespace std;

namespace cdh {
    step_motor_couple_t *step_motor_couple_t::step_motor_couple = NULL;
    extern "C" {
    void TIM3_IRQHandler() {
        if (TIM_GetITStatus(TIM3, TIM_IT_CC3) == SET) {
            TIM_ClearITPendingBit(TIM3, TIM_IT_CC3);
            if (step_motor_couple_t::step_motor_couple->steps.x <
                step_motor_couple_t::step_motor_couple->next_steps.x) {
                ++step_motor_couple_t::step_motor_couple->steps.x;
            } else if (step_motor_couple_t::step_motor_couple->steps.x >
                       step_motor_couple_t::step_motor_couple->next_steps.x) {
                --step_motor_couple_t::step_motor_couple->steps.x;
            }
            if (step_motor_couple_t::step_motor_couple->steps.x ==
                step_motor_couple_t::step_motor_couple->next_steps.x) {
                TIM_Cmd(TIM3, DISABLE);
            }
        }
    }

    void TIM4_IRQHandler() {
        if (TIM_GetITStatus(TIM4, TIM_IT_CC1) == SET) {
            TIM_ClearITPendingBit(TIM4, TIM_IT_CC1);
            if (step_motor_couple_t::step_motor_couple->steps.y <
                step_motor_couple_t::step_motor_couple->next_steps.y) {
                ++step_motor_couple_t::step_motor_couple->steps.y;
            } else if (step_motor_couple_t::step_motor_couple->steps.y >
                       step_motor_couple_t::step_motor_couple->next_steps.y) {
                --step_motor_couple_t::step_motor_couple->steps.y;
            }
            if (step_motor_couple_t::step_motor_couple->steps.y ==
                step_motor_couple_t::step_motor_couple->next_steps.y) {
                TIM_Cmd(TIM4, DISABLE);
            }
        }
    }
    }

    step_motor_couple_t::step_motor_couple_t()
            : box(box_t::instance()) {
        double_int_t p1_x_y;
        p1_x_y.int1 = 400;
        p1_x_y.int2 = 900;
        double_int_t p2_x_y;
        p2_x_y.int1 = 400;
        p2_x_y.int2 = 200;
        double_int_t e_l;
        e_l.int1 = 598;
        e_l.int2 = 1094;
        double_int_t f_l;
        f_l.int1 = 601;
        f_l.int2 = 1098;
        fix_e(p1_x_y, p2_x_y, e_l);
        fix_f(p1_x_y, p2_x_y, f_l);
        reset_current_position(0, 0);
        steps_mutex = osMutexNew(NULL);
    }

    step_motor_couple_t *step_motor_couple_t::open() {
        if (step_motor_couple)
            return step_motor_couple;
        step_motor_couple = new step_motor_couple_t();
        GPIO_InitTypeDef GPIO_InitStructure;
        TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
        NVIC_InitTypeDef NVIC_InitStructure;
        TIM_OCInitTypeDef TIM_OCInitStructure;
        //config rcc
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
        //config gpio
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOC, &GPIO_InitStructure);
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_3;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOC, &GPIO_InitStructure);
        GPIO_WriteBit(GPIOC, GPIO_Pin_0, Bit_SET);
        GPIO_WriteBit(GPIOC, GPIO_Pin_1, Bit_SET);
        GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);
        GPIO_WriteBit(GPIOC, GPIO_Pin_3, Bit_RESET);
        //config gpio PB0-TIM3_CH3 PB6-TIM4_CH1
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_6;
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
        //config tim4
        TIM_TimeBaseStructure.TIM_Period = 720 - 1;
        TIM_TimeBaseStructure.TIM_Prescaler = 72 - 1;
        TIM_TimeBaseStructure.TIM_ClockDivision = 1 - 1;
        TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
        TIM_ARRPreloadConfig(TIM4, ENABLE);
        TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
        //config tim4_it_cc1
        TIM_ITConfig(TIM4, TIM_IT_CC1, ENABLE);
        NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
        //config tim4 ch1
        TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
        TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
        TIM_OCInitStructure.TIM_Pulse = 360;
        TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
        TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);
        TIM_OC1Init(TIM4, &TIM_OCInitStructure);
//        //enable tim3
//        TIM_Cmd(TIM3, ENABLE);
//        //enable tim4
//        TIM_Cmd(TIM4, ENABLE);

        return step_motor_couple;
    }

    void step_motor_couple_t::step(int motor1_steps, int motor2_steps) {
        while (set_next_steps(steps + vec2_t(motor1_steps, motor2_steps)))
            osThreadYield();
    }

    void step_motor_couple_t::step() {
        vec2_t steps_dir = steps - next_steps;
        if (steps_dir.x < 0) {
            steps_dir.x = -steps_dir.x;
        }
        if (steps_dir.y < 0) {
            steps_dir.y = -steps_dir.y;
        }
        if (steps_dir.length2() != 0)
            if (steps_dir.x == 0) {
                TIM_SetAutoreload(TIM4, 600 - 1);
            } else if (steps_dir.y == 0) {
                TIM_SetAutoreload(TIM3, 600 - 1);
            } else {
                if (steps_dir.x >= steps_dir.y) {
                    int k = steps_dir.x / steps_dir.y;
                    if (k > 65536 / 600) {
                        TIM_SetAutoreload(TIM3, 600 - 1);
                        TIM_SetAutoreload(TIM4, 65536 - 1);
                    } else {
                        TIM_SetAutoreload(TIM3, 600 - 1);
                        TIM_SetAutoreload(TIM4, 600 * k - 1);
                    }
                } else {
                    int k = steps_dir.y / steps_dir.x;
                    if (k > 65536 / 600) {
                        TIM_SetAutoreload(TIM4, 600 - 1);
                        TIM_SetAutoreload(TIM3, 65536 - 1);
                    } else {
                        TIM_SetAutoreload(TIM4, 600 - 1);
                        TIM_SetAutoreload(TIM3, 600 * k - 1);
                    }
                }
            }
    }

    vec2_t step_motor_couple_t::pre_map_position_to_steps(int x_mm, int y_mm) {
//        printf("pre_map_position_to_steps [%d,%d] ", x_mm, y_mm);
        box->position.x = x_mm;
        box->position.y = y_mm;

        y_mm += CDH_DISTANCE_BETWEEN_CONTROL_POINT_AND_PEN;
        vec2_t steps_of_motors;
        double l1 = sqrt(
                double((x_mm - point_e.x) * (x_mm - point_e.x)) + (y_mm - point_e.y) * (y_mm - point_e.y));
        double l2 = sqrt(
                double((x_mm - point_f.x) * (x_mm - point_f.x)) + (y_mm - point_f.y) * (y_mm - point_f.y));
//        steps_of_motors.x = l1 * 24.6822823f;
//        steps_of_motors.y = l2 * 24.6822823f;
        steps_of_motors.x = l1 * 24.9578823f;
        steps_of_motors.y = l2 * 24.9672823f;
//        steps_of_motors.x = l1 * 25.1045407351f;
//        steps_of_motors.y = l2 * 25.1045407351f;
//        steps_of_motors.x = l1 * 24.6161420351f;
//        steps_of_motors.y = l2 * 24.6161420351f;
//        printf("to l[%lf,%lf] step[%d,%d]\r\n", l1, l2, steps_of_motors.x, steps_of_motors.y);
        return steps_of_motors;
    }

    void step_motor_couple_t::reset_current_position(int x_mm, int y_mm) {
        TIM_Cmd(TIM3, DISABLE);
        TIM_Cmd(TIM4, DISABLE);
        vec2_t steps_of_motors = pre_map_position_to_steps(x_mm, y_mm);
        steps = steps_of_motors;
        next_steps = steps_of_motors;
        box->position.x = x_mm;
        box->position.y = y_mm;
        //printf("reset_current_position steps=[%d,%d]\r\n", steps.int1, steps.int2);
    }

    void step_motor_couple_t::fix_e(const double_int_t &p1_x_y, const double_int_t &p2_x_y, const double_int_t &l) {
        float p1_x = p1_x_y.int1;
        float p1_y = p1_x_y.int2;
        float p2_x = p2_x_y.int1;
        float p2_y = p2_x_y.int2;
        float length_p1_e = l.int1;
        float length_p2_e = l.int2;

        float length_p1_p2 = sqrt((p1_x - p2_x) * (p1_x - p2_x) + (p1_y - p2_y) * (p1_y - p2_y));

        float angle_p2_cos = (length_p2_e * length_p2_e + length_p1_p2 * length_p1_p2 - length_p1_e * length_p1_e) /
                             (2 * length_p2_e * length_p1_p2);
        float angle_p1_p2_x_tan;
        float angle_p1_p2_x;
        if (p1_x == p2_x) {
            angle_p1_p2_x = 3.14159265358979f / 2;
        } else {
            angle_p1_p2_x_tan = (p1_y - p2_y) / (p1_x - p2_x);
            angle_p1_p2_x = atan(angle_p1_p2_x_tan);
        }
        float angle_p2 = acos(angle_p2_cos);
        float angle_e_p2_mx = 3.14159265358979f - angle_p1_p2_x - angle_p2;
        float dx = length_p2_e * cos(angle_e_p2_mx);
        float dy = length_p2_e * sin(angle_e_p2_mx);
        point_e.x = p2_x - dx;
        point_e.y = p2_y + dy;
        printf("E:%f,%f\r\n", p2_x - dx, p2_y + dy);
        printf("E:%d,%d\r\n", point_e.x, point_e.y);
    }

    void step_motor_couple_t::fix_f(const double_int_t &p1_x_y, const double_int_t &p2_x_y, const double_int_t &l) {
        float p1_x = p1_x_y.int1;
        float p1_y = p1_x_y.int2;
        float p2_x = p2_x_y.int1;
        float p2_y = p2_x_y.int2;
        float length_p1_f = l.int1;
        float length_p2_f = l.int2;

        float length_p1_p2 = sqrt((p1_x - p2_x) * (p1_x - p2_x) + (p1_y - p2_y) * (p1_y - p2_y));

        float angle_p2_cos = (length_p2_f * length_p2_f + length_p1_p2 * length_p1_p2 - length_p1_f * length_p1_f) /
                             (2 * length_p2_f * length_p1_p2);
        float angle_p1_p2_x_tan;
        float angle_p1_p2_x;
        if (p1_x == p2_x) {
            angle_p1_p2_x = 3.14159265358979f / 2;
        } else {
            angle_p1_p2_x_tan = (p1_y - p2_y) / (p1_x - p2_x);
            angle_p1_p2_x = atan(angle_p1_p2_x_tan);
        }
        float angle_p2 = acos(angle_p2_cos);
        float angle_f_p2_mx = angle_p1_p2_x - angle_p2;
        float dx = length_p2_f * cos(angle_f_p2_mx);
        float dy = length_p2_f * sin(angle_f_p2_mx);
        point_f.x = p2_x + dx;

        point_f.y = p2_y + dy;
        printf("F:%f,%f\r\n", p2_x + dx, p2_y + dy);
        printf("F:%d,%d\r\n", point_f.x, point_f.y);
    }

    int step_motor_couple_t::set_next_steps(const vec2_t &next_steps) {
        if (this->next_steps == steps) {
            this->next_steps = next_steps;
            //prepare to move box
            vec2_t steps_dir = steps - next_steps;
            BitAction motor1_dir;
            BitAction motor2_dir;
            if (steps_dir.x >= 0) {
                motor1_dir = Bit_RESET;
            } else {
                motor1_dir = Bit_SET;
            }
            if (steps_dir.y >= 0) {
                motor2_dir = Bit_RESET;
            } else {
                motor2_dir = Bit_SET;
            }
            GPIO_WriteBit(GPIOC, GPIO_Pin_13, motor1_dir);
            GPIO_WriteBit(GPIOC, GPIO_Pin_3, motor2_dir);
            TIM_SetAutoreload(TIM3, 300);
            TIM_SetAutoreload(TIM4, 300);
            TIM_Cmd(TIM3, ENABLE);
            TIM_Cmd(TIM4, ENABLE);
            return 0;
        }
        return -1;
    }

}
