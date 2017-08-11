//
// Created by leu19 on 2017/7/28.
//

#include "key_board.h"
#include <gpio.h>
#include <FreeRTOS.h>
#include <task.h>

namespace {

}
namespace cdh{

key_board_t::key_board_e cdh::key_board_t::scan() {
    HAL_GPIO_WritePin(key_board_out1_GPIO_Port, key_board_out2_Pin | key_board_out3_Pin | key_board_out4_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(key_board_out1_GPIO_Port, key_board_out1_Pin,GPIO_PIN_RESET);
    if(HAL_GPIO_ReadPin(key_board_in1_GPIO_Port,key_board_in1_Pin) == GPIO_PIN_RESET){
        return key_board_c_e;
    }else if(HAL_GPIO_ReadPin(key_board_in1_GPIO_Port,key_board_in2_Pin) == GPIO_PIN_RESET){
        return key_board_8_e;
    }else if(HAL_GPIO_ReadPin(key_board_in1_GPIO_Port,key_board_in3_Pin) == GPIO_PIN_RESET){
        return key_board_4_e;
    }else if(HAL_GPIO_ReadPin(key_board_in1_GPIO_Port,key_board_in4_Pin) == GPIO_PIN_RESET){
        return key_board_0_e;
    }
    HAL_GPIO_WritePin(key_board_out1_GPIO_Port, key_board_out1_Pin | key_board_out3_Pin | key_board_out4_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(key_board_out1_GPIO_Port, key_board_out2_Pin,GPIO_PIN_RESET);
    if(HAL_GPIO_ReadPin(key_board_in1_GPIO_Port,key_board_in1_Pin) == GPIO_PIN_RESET){
        return key_board_d_e;
    }else if(HAL_GPIO_ReadPin(key_board_in1_GPIO_Port,key_board_in2_Pin) == GPIO_PIN_RESET){
        return key_board_9_e;
    }else if(HAL_GPIO_ReadPin(key_board_in1_GPIO_Port,key_board_in3_Pin) == GPIO_PIN_RESET){
        return key_board_5_e;
    }else if(HAL_GPIO_ReadPin(key_board_in1_GPIO_Port,key_board_in4_Pin) == GPIO_PIN_RESET){
        return key_board_1_e;
    }
    HAL_GPIO_WritePin(key_board_out1_GPIO_Port, key_board_out2_Pin | key_board_out1_Pin | key_board_out4_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(key_board_out1_GPIO_Port, key_board_out3_Pin,GPIO_PIN_RESET);
    if(HAL_GPIO_ReadPin(key_board_in1_GPIO_Port,key_board_in1_Pin) == GPIO_PIN_RESET){
        return key_board_e_e;
    }else if(HAL_GPIO_ReadPin(key_board_in1_GPIO_Port,key_board_in2_Pin) == GPIO_PIN_RESET){
        return key_board_a_e;
    }else if(HAL_GPIO_ReadPin(key_board_in1_GPIO_Port,key_board_in3_Pin) == GPIO_PIN_RESET){
        return key_board_6_e;
    }else if(HAL_GPIO_ReadPin(key_board_in1_GPIO_Port,key_board_in4_Pin) == GPIO_PIN_RESET){
        return key_board_2_e;
    }
    HAL_GPIO_WritePin(key_board_out1_GPIO_Port, key_board_out2_Pin | key_board_out3_Pin | key_board_out1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(key_board_out1_GPIO_Port, key_board_out4_Pin,GPIO_PIN_RESET);
    if(HAL_GPIO_ReadPin(key_board_in1_GPIO_Port,key_board_in1_Pin) == GPIO_PIN_RESET){
        return key_board_f_e;
    }else if(HAL_GPIO_ReadPin(key_board_in1_GPIO_Port,key_board_in2_Pin) == GPIO_PIN_RESET){
        return key_board_b_e;
    }else if(HAL_GPIO_ReadPin(key_board_in1_GPIO_Port,key_board_in3_Pin) == GPIO_PIN_RESET){
        return key_board_7_e;
    }else if(HAL_GPIO_ReadPin(key_board_in1_GPIO_Port,key_board_in4_Pin) == GPIO_PIN_RESET){
        return key_board_3_e;
    }

    return key_board_none_e;
}
}
