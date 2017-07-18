#include <stm32f10x_conf.h>
#include "usart1.h"
#include <cstddef>
#ifndef CDH_USART1_DATA_BUFFER_LENGTH
#define CDH_USART1_DATA_BUFFER_LENGTH 128
#endif
static char usart1_data_buffer[CDH_USART1_DATA_BUFFER_LENGTH] = {0};
static unsigned char usart1_buffer_first = 0;
static unsigned char usart1_buffer_end = 0;

extern "C" {
void USART1_IRQHandler(void) {
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        if (usart1_buffer_end == CDH_USART1_DATA_BUFFER_LENGTH)
            usart1_buffer_end = 0;
        usart1_data_buffer[usart1_buffer_end++] = USART_ReceiveData(USART1);
    }
}

}
namespace cdh {
    bool usart1_t::inited = false;

    usart1_t *usart1_t::open() {
        position = 0;

        if (inited)
            return this;

        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

        GPIO_InitTypeDef GPIO_InitStructure;
        //USART1 Tx(PA.09)
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
        //USART1 Rx(PA.10)
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOA, &GPIO_InitStructure);

        USART_InitTypeDef USART_InitStructure;
        USART_InitStructure.USART_BaudRate = 9600;
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;
        USART_InitStructure.USART_StopBits = USART_StopBits_1;
        USART_InitStructure.USART_Parity = USART_Parity_No;
        USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
        USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
        USART_Init(USART1, &USART_InitStructure);

        NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
        NVIC_InitTypeDef NVIC_InitStructure;
        NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);

        USART_ClearFlag(USART1, USART_FLAG_TC);//防止第一个数据被覆盖
        USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启中断
        USART_Cmd(USART1, ENABLE);
        inited = true;
        return this;
    }

    std::size_t usart1_t::read(unsigned char *ptr, std::size_t size, std::size_t count) {
        if (!count)
            return 0;
        long int max_length = size * count;
        int i = 0;
        for (; i < max_length && usart1_buffer_first != usart1_buffer_end; ++i) {
            if (usart1_buffer_first == CDH_USART1_DATA_BUFFER_LENGTH)
                usart1_buffer_first = 0;
            ptr[i] = usart1_data_buffer[usart1_buffer_first++];
        }
        return i;
    }

    std::size_t usart1_t::write(const unsigned char *ptr, std::size_t size, std::size_t count) {
        for (int i = 0; i < count; ++i) {
            while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
            USART_SendData(USART1, *(ptr + i * size));
        }
        return count;
    }

    int usart1_t::seek(long int offset) {
        return 0;
    }

    long usart1_t::tell() {
        return 0;
    }

    int usart1_t::close() {
        return 0;
    }

    bool usart1_t::have_data_to_read() {
        if (usart1_buffer_first != usart1_buffer_end)
            return true;
        return false;
    }

    void usart1_t::trim_buffer_head() {
        while(usart1_buffer_first!=usart1_buffer_end){
            if(usart1_buffer_first==CDH_USART1_DATA_BUFFER_LENGTH)
                usart1_buffer_first =0;
            unsigned char ch = usart1_data_buffer[usart1_buffer_first];
            if(ch == '\n'||ch == ' ' || ch == '\r')
                ++usart1_buffer_first;
            else break;
        }
    }
}
