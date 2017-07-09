#include <stm32f10x_conf.h>
#include "usart1.h"
namespace cdh{
    bool usart1_t::inited = false;
    driver_t *usart1_t::open() {
        position = 0;

        if(inited)
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
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
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
        USART_Cmd(USART1, ENABLE);
        inited = true;
        return this;
    }

    std::size_t usart1_t::read(void *ptr, std::size_t size, std::size_t count) {
        return 0;
    }

    std::size_t usart1_t::write(const unsigned char *ptr, std::size_t size, std::size_t count) {
        for(int i = 0;i<count;++i){
            while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
            USART_SendData(USART1,*(ptr+i*size));
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
}
