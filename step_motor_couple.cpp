//
// Created by leu19 on 2017/7/28.
//
#include <fix_glm.h>
#include <glm/glm.hpp>
#include "step_motor_couple.h"
#include <main.h>
#include <tim.h>
#include <gpio.h>
#include <cmsis_os.h>
#include <stm32f1xx_hal_tim.h>

using namespace glm;
namespace {
    const static int step_per_loop = 800;
    const static float r = 10.f;
    const static float pi = 3.141592653f;
    ivec2 current_steps;
    ivec2 next_steps;
}


extern "C" void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2) {
        if (next_steps.x > current_steps.x) {
            ++current_steps.x;
        } else if (next_steps.x < current_steps.x) {
            --current_steps.x;
        }

        if (next_steps.x == current_steps.x) {
            HAL_TIM_PWM_Stop_IT(&htim2, TIM_CHANNEL_2);
        } else if (::current_steps.x < next_steps.x) {
            HAL_GPIO_WritePin(step_motor1_dir_GPIO_Port, step_motor1_dir_Pin, GPIO_PIN_SET);
        } else if (::current_steps.x > next_steps.x) {
            HAL_GPIO_WritePin(step_motor1_dir_GPIO_Port, step_motor1_dir_Pin, GPIO_PIN_RESET);
        }
    } else if (htim3.Instance == TIM3) {
        if (next_steps.y > current_steps.y) {
            ++current_steps.y;
        } else if (next_steps.y < current_steps.y) {
            --current_steps.y;
        }
        if (next_steps.y == current_steps.y) {
            HAL_TIM_PWM_Stop_IT(&htim3, TIM_CHANNEL_1);
        } else if (::current_steps.y < ::next_steps.y) {
            HAL_GPIO_WritePin(step_motor2_dir_GPIO_Port, step_motor2_dir_Pin, GPIO_PIN_SET);
        } else if (::current_steps.y > ::next_steps.y) {
            HAL_GPIO_WritePin(step_motor2_dir_GPIO_Port, step_motor2_dir_Pin, GPIO_PIN_RESET);
        }
    }
}

namespace cdh {

    void step_motor_couple_t::test() {
        static float i = 0.01;
        static float x_base = 0;
        static vec2 cur;
        cur = vec2(10 * x_base, 10 * sin(x_base));

        while (set_next_steps(cur) == -1) {}
        step();
        x_base += i;
        while (status() == -1) {
            osDelay(100);
        }

        if (x_base >= 6.28)
            i = -0.01;
        if (x_base <= 0)
            i = 0.01;
    }

    int step_motor_couple_t::set_next_steps(const glm::ivec2 &next_steps, bool force) {
        if (::next_steps == next_steps) {
            return 0;
        } else if (force || ::next_steps == ::current_steps) {
            if (force) {
                HAL_TIM_PWM_Stop_IT(&htim2, TIM_CHANNEL_2);
                HAL_TIM_PWM_Stop_IT(&htim3, TIM_CHANNEL_1);
            }
            ::next_steps = next_steps;
            if (::current_steps.x != next_steps.x) {
                if (::current_steps.x < next_steps.x) {
                    HAL_GPIO_WritePin(step_motor1_dir_GPIO_Port, step_motor1_dir_Pin, GPIO_PIN_SET);
                } else if (::current_steps.x > next_steps.x) {
                    HAL_GPIO_WritePin(step_motor1_dir_GPIO_Port, step_motor1_dir_Pin, GPIO_PIN_RESET);
                }
                __HAL_TIM_SET_AUTORELOAD(&htim2, 20);
                HAL_TIM_PWM_Start_IT(&htim2, TIM_CHANNEL_2);
            }
            if (::current_steps.y != ::next_steps.y) {
                if (::current_steps.y < ::next_steps.y) {
                    HAL_GPIO_WritePin(step_motor2_dir_GPIO_Port, step_motor2_dir_Pin, GPIO_PIN_SET);
                } else if (::current_steps.y > ::next_steps.y) {
                    HAL_GPIO_WritePin(step_motor2_dir_GPIO_Port, step_motor2_dir_Pin, GPIO_PIN_RESET);
                }
                __HAL_TIM_SET_AUTORELOAD(&htim3, 20);
                HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_1);
            }
            return 0;
        } else {
            return -1;
        }
    }

    void step_motor_couple_t::step() {
        ivec2 steps_dir = ::current_steps - ::next_steps;
        if (steps_dir.x < 0) {
            steps_dir.x = -steps_dir.x;
        }
        if (steps_dir.y < 0) {
            steps_dir.y = -steps_dir.y;
        }
        if (steps_dir.x != 0 || steps_dir.y != 0)
            if (steps_dir.x == 0) {
                __HAL_TIM_SET_AUTORELOAD(&htim3, 900 - 1);
            } else if (steps_dir.y == 0) {
                __HAL_TIM_SET_AUTORELOAD(&htim2, 900 - 1);
            } else {
                __HAL_TIM_SET_AUTORELOAD(&htim3, 900 - 1);
                __HAL_TIM_SET_AUTORELOAD(&htim2, 900 - 1);
//                if (steps_dir.x >= steps_dir.y) {
//                    float k = steps_dir.x / steps_dir.y;
//                    if (k > (float) 65536 / 1000) {
//                        __HAL_TIM_SET_AUTORELOAD(&htim2, 1000 - 1);
//                        __HAL_TIM_SET_AUTORELOAD(&htim3, 65536 - 1);
//                    } else {
//                        __HAL_TIM_SET_AUTORELOAD(&htim2, 1000 - 1);
//                        __HAL_TIM_SET_AUTORELOAD(&htim3, 1000 * k - 1);
//                    }
//                } else {
//                    float k = steps_dir.y / steps_dir.x;
//                    if (k > (float) 65536 / 1500) {
//                        __HAL_TIM_SET_AUTORELOAD(&htim3, 1000 - 1);
//                        __HAL_TIM_SET_AUTORELOAD(&htim2, 65536 - 1);
//                    } else {
//                        __HAL_TIM_SET_AUTORELOAD(&htim3, 1000 - 1);
//                        __HAL_TIM_SET_AUTORELOAD(&htim2, 1000 * k - 1);
//                    }
//                }
            }
    }

    void step_motor_couple_t::set_current_steps(const glm::ivec2 &current) {
        HAL_TIM_PWM_Stop_IT(&htim2, TIM_CHANNEL_2);
        HAL_TIM_PWM_Stop_IT(&htim3, TIM_CHANNEL_1);
        ::current_steps = current;
        next_steps = current;
    }

    step_motor_couple_status_e step_motor_couple_t::status() {
        if (::next_steps != ::current_steps)
            return step_motor_couple_status_running_e;
        return step_motor_couple_status_stopped_e;
    }

    glm::ivec2 step_motor_couple_t::current_steps() {
        return ::current_steps;
    }

    glm::ivec2 step_motor_couple_t::map_angle_to_steps(const glm::vec2 &angle) {
        vec2 steps = angle / 2.f / pi * (float) step_per_loop;
        return steps;
    }

    glm::ivec2 step_motor_couple_t::map_length_to_steps(const glm::vec2 &length) {
        vec2 steps = length / r / 2.f / pi * (float) step_per_loop;
        return steps;
    }

    glm::vec2 step_motor_couple_t::current_angle() {
        vec2 angle = vec2(::current_steps) / (float) step_per_loop * 2.f * pi;
        return angle;
    }

    glm::vec2 step_motor_couple_t::current_length() {
        vec2 length = vec2(::current_steps) / (float) step_per_loop * 2.f * pi * r;
        return length;
    }

}

