//
// Created by leu19 on 2017/7/28.
//

#include "key_board.h"
#include <gpio.h>
#include <FreeRTOS.h>
#include <task.h>

namespace {
    int current_row = 0;
    unsigned char key_board_rx_buffer[128];
    unsigned char key_board_rx_start = 0;
    unsigned char key_board_rx_end = 0;
    unsigned int old_time = 0;
}

int cdh::key_board_t::scan() {
    HAL_GPIO_WritePin(key_board_out1_GPIO_Port, key_board_out2_Pin | key_board_out3_Pin | key_board_out4_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(key_board_out1_GPIO_Port, key_board_out1_Pin,GPIO_PIN_RESET);
    if(HAL_GPIO_ReadPin(key_board_in1_GPIO_Port,key_board_in1_Pin) == GPIO_PIN_RESET){
        return 0;
    }else if(HAL_GPIO_ReadPin(key_board_in1_GPIO_Port,key_board_in2_Pin) == GPIO_PIN_RESET){
        return 1;
    }else if(HAL_GPIO_ReadPin(key_board_in1_GPIO_Port,key_board_in3_Pin) == GPIO_PIN_RESET){
        return 2;
    }else if(HAL_GPIO_ReadPin(key_board_in1_GPIO_Port,key_board_in4_Pin) == GPIO_PIN_RESET){
        return 3;
    }
    HAL_GPIO_WritePin(key_board_out1_GPIO_Port, key_board_out1_Pin | key_board_out3_Pin | key_board_out4_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(key_board_out1_GPIO_Port, key_board_out2_Pin,GPIO_PIN_RESET);
    if(HAL_GPIO_ReadPin(key_board_in1_GPIO_Port,key_board_in1_Pin) == GPIO_PIN_RESET){
        return 4;
    }else if(HAL_GPIO_ReadPin(key_board_in1_GPIO_Port,key_board_in2_Pin) == GPIO_PIN_RESET){
        return 5;
    }else if(HAL_GPIO_ReadPin(key_board_in1_GPIO_Port,key_board_in3_Pin) == GPIO_PIN_RESET){
        return 6;
    }else if(HAL_GPIO_ReadPin(key_board_in1_GPIO_Port,key_board_in4_Pin) == GPIO_PIN_RESET){
        return 7;
    }
    HAL_GPIO_WritePin(key_board_out1_GPIO_Port, key_board_out2_Pin | key_board_out1_Pin | key_board_out4_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(key_board_out1_GPIO_Port, key_board_out3_Pin,GPIO_PIN_RESET);
    if(HAL_GPIO_ReadPin(key_board_in1_GPIO_Port,key_board_in1_Pin) == GPIO_PIN_RESET){
        return 8;
    }else if(HAL_GPIO_ReadPin(key_board_in1_GPIO_Port,key_board_in2_Pin) == GPIO_PIN_RESET){
        return 9;
    }else if(HAL_GPIO_ReadPin(key_board_in1_GPIO_Port,key_board_in3_Pin) == GPIO_PIN_RESET){
        return 10;
    }else if(HAL_GPIO_ReadPin(key_board_in1_GPIO_Port,key_board_in4_Pin) == GPIO_PIN_RESET){
        return 11;
    }
    HAL_GPIO_WritePin(key_board_out1_GPIO_Port, key_board_out2_Pin | key_board_out3_Pin | key_board_out1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(key_board_out1_GPIO_Port, key_board_out4_Pin,GPIO_PIN_RESET);
    if(HAL_GPIO_ReadPin(key_board_in1_GPIO_Port,key_board_in1_Pin) == GPIO_PIN_RESET){
        return 12;
    }else if(HAL_GPIO_ReadPin(key_board_in1_GPIO_Port,key_board_in2_Pin) == GPIO_PIN_RESET){
        return 13;
    }else if(HAL_GPIO_ReadPin(key_board_in1_GPIO_Port,key_board_in3_Pin) == GPIO_PIN_RESET){
        return 14;
    }else if(HAL_GPIO_ReadPin(key_board_in1_GPIO_Port,key_board_in4_Pin) == GPIO_PIN_RESET){
        return 15;
    }

    return -1;
}
