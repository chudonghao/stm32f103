//
// Created by leu19 on 2017/7/13.
//

#include <stm32f10x_conf.h>
#include <cstdio>
#include <misc.h>
#include "infra_red.h"

static unsigned char usart2_data_buffer = 0;
extern "C" {
void USART2_IRQHandler(void) {
    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
        usart2_data_buffer = USART_ReceiveData(USART2);
    }
}
}
using namespace std;
namespace cdh {
    infra_red_t *infra_red_t::infra_red = 0;

    infra_red_t *infra_red_t::open() {
        if (infra_red)
            return infra_red;
        infra_red = new infra_red_t();
        GPIO_InitTypeDef GPIO_InitStructure;
        ///*********usart2****************************************************************************/
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
        //USART1 Tx(PA.02)
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
        //USART1 Rx(PA.03)
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
        USART_InitTypeDef USART_InitStructure;
        USART_InitStructure.USART_BaudRate = 9600;
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;
        USART_InitStructure.USART_StopBits = USART_StopBits_1;
        USART_InitStructure.USART_Parity = USART_Parity_No;
        USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
        USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
        USART_Init(USART2, &USART_InitStructure);
        NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
        NVIC_InitTypeDef NVIC_InitStructure;
        NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
        USART_ClearFlag(USART2, USART_FLAG_TC);//防止第一个数据被覆盖
        USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启中断
        USART_Cmd(USART2, ENABLE);

        return infra_red;
    }

    void infra_red_t::test() {
        u16 data_ = usart2_data_buffer;
        printf("data:");
        for (int i = 0; i < 8; ++i) {
            if (data_ & 0x80) {
                printf("1");
            } else {
                printf("0");
            }
            data_ <<= 1;
        }
        printf("\r\n");
    }

    infra_red_t::infra_red_t() {}

    unsigned char infra_red_t::data() {
//        u16 data = GPIO_ReadInputData(GPIOB);
//        data >>= 5;
        return usart2_data_buffer;
    }
}
