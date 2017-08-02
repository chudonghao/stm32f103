//
// Created by leu19 on 2017/7/28.
//

#include <cstdio>
#include <usart.h>
#include <stm32f1xx_hal_conf.h>

//#ifdef __GNUC__/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
//   set to 'Yes') calls __io_putchar() */
//#define PUTCHAR_PROTOTYPE extern "C" int __io_putchar(int ch)
//#define GETCHAR_PROTOTYPE extern "C" int __io_getchar(FILE *f)
//#else
#define PUTCHAR_PROTOTYPE
#define GETCHAR_PROTOTYPE
//#endif /* __GNUC__ */
int fputc(int ch, FILE *f){
    /* Place your implementation of fputc here */
    /* e.g. write a character to the USART1 and Loop until the end of transmission */
    HAL_UART_Transmit(&huart1, (unsigned char * ) & ch, 1, 0xFFFF);
    return ch;
}
static unsigned char uart1_rx_buffer[128];
static unsigned char uart1_rx_buffer_start;
static unsigned char uart1_rx_buffer_end;

unsigned char uart3_rx_buffer[128];
unsigned char uart3_rx_buffer_start;
unsigned char uart3_rx_buffer_end;
int uart3_have_sentence = 0;
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
    if(huart->Instance == USART1){
        //HAL_UART_Transmit(&huart1, &uart1_rx_buffer[uart1_rx_buffer_end], 1, 0xFFFF);
        ++uart1_rx_buffer_end;
        if(uart1_rx_buffer_end == 128){
            uart1_rx_buffer_end = 0;
        }
        HAL_UART_Receive_IT(&huart1,&uart1_rx_buffer[uart1_rx_buffer_end],1);
    }
//    else if(huart->Instance == USART3){
//        if(uart3_rx_buffer[uart3_rx_buffer_end] == '\n'){
//            uart3_have_sentence = 1;
//        }
//        //HAL_UART_Transmit(&huart1, &uart1_rx_buffer[uart1_rx_buffer_end], 1, 0xFFFF);
//        ++uart3_rx_buffer_end;
//        if(uart3_rx_buffer_end == 128){
//            uart3_rx_buffer_end = 0;
//        }
//        HAL_UART_Receive_IT(&huart3,&uart3_rx_buffer[uart3_rx_buffer_end],1);
//    }
}

int fgetc(FILE *f){
    unsigned char res = 0;
    while(uart1_rx_buffer_start == uart1_rx_buffer_end){
        HAL_UART_Receive_IT(&huart1,&uart1_rx_buffer[uart1_rx_buffer_end],1);
    }
    res = uart1_rx_buffer[uart1_rx_buffer_start++];
    if(uart1_rx_buffer_start == 128){
        uart1_rx_buffer_start = 0;
    }
    return res;
}
