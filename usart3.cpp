//
// Created by leu19 on 2017/7/29.
//

#include <math.h>
#include "usart3.h"
#include <usart.h>
#include <cstring>
using namespace std;

extern "C" bool uart3_have_sentence;
extern "C" unsigned char uart3_rx_buffer[128];
extern "C" unsigned char uart3_rx_buffer_start;
extern "C" unsigned char uart3_rx_buffer_end;
namespace cdh{

    bool usart3_t::have_sentence() {
        HAL_UART_Receive_IT(&huart3,&uart3_rx_buffer[uart3_rx_buffer_end],1);
        return uart3_have_sentence;
    }

    char *usart3_t::c_str() {
        uart3_have_sentence = 0;
        char* res = (char*)&uart3_rx_buffer[uart3_rx_buffer_start];
        uart3_rx_buffer_start = uart3_rx_buffer_end;
        return res;
    }

    void usart3_t::print(char *str) {
        HAL_UART_Transmit(&huart3,(uint8_t*)str,strlen(str),0xffff);
    }
}
