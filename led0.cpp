//
// Created by leu19 on 2017/8/12.
//
#include "main.h"
#include "gpio.h"
#include "led0.h"


namespace cdh{

    void led0_t::on() {
        HAL_GPIO_WritePin(led0_GPIO_Port,led0_Pin,GPIO_PIN_RESET);
    }

    void led0_t::off() {
           HAL_GPIO_WritePin(led0_GPIO_Port,led0_Pin,GPIO_PIN_SET);
    }
}
