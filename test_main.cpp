
#include <FreeRTOS.h>
#include <task.h>

#include <stm32f10x_conf.h>
#include <cstddef>
#ifndef CDH_USART_DATA_BUFFER_LENGTH
#define CDH_USART_DATA_BUFFER_LENGTH 128
#endif
static char usart_data_buffer[CDH_USART_DATA_BUFFER_LENGTH] = {0};
static unsigned char usart_buffer_first = 0;
static unsigned char usart_buffer_end = 0;

extern "C" {
void /*■■■*/USART1_IRQHandler(void) {
    if (USART_GetITStatus(/*■■■*/USART1, USART_IT_RXNE) != RESET) {
        if (usart_buffer_end == CDH_USART_DATA_BUFFER_LENGTH)
            usart_buffer_end = 0;
        usart_data_buffer[usart_buffer_end++] = USART_ReceiveData(/*■■■*/USART1);
    }
}
}
namespace cdh {

    void init() {
        GPIO_InitTypeDef GPIO_InitStructure;
        RCC_APB2PeriphClockCmd(/*■■■*/RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);
        //USART Tx
        GPIO_InitStructure.GPIO_Pin = /*■■■*/GPIO_Pin_9;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_Init(/*■■■*/GPIOA, &GPIO_InitStructure);
        //USART Rx
        GPIO_InitStructure.GPIO_Pin = /*■■■*/GPIO_Pin_10;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(/*■■■*/GPIOA, &GPIO_InitStructure);

        USART_InitTypeDef USART_InitStructure;
        USART_InitStructure.USART_BaudRate = /*■■■*/9600;
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;
        USART_InitStructure.USART_StopBits = USART_StopBits_1;
        USART_InitStructure.USART_Parity = USART_Parity_No;
        USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
        USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
        USART_Init(/*■■■*/USART1, &USART_InitStructure);

        NVIC_InitTypeDef NVIC_InitStructure;
        NVIC_InitStructure.NVIC_IRQChannel = /*■■■*/USART1_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = /*■■■*/3;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = /*■■■*/3;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);

        USART_ClearFlag(/*■■■*/USART1, USART_FLAG_TC);//防止第一个数据被覆盖
        USART_ITConfig(/*■■■*/USART1, USART_IT_RXNE, ENABLE);//开启中断
        USART_Cmd(/*■■■*/USART1, ENABLE);
    }

    std::size_t read(unsigned char *ptr, std::size_t count) {
        if (!count)
            return 0;
        int i = 0;
        for (; i < count && usart_buffer_first != usart_buffer_end; ++i) {
            if (usart_buffer_first == CDH_USART_DATA_BUFFER_LENGTH)
                usart_buffer_first = 0;
            ptr[i] = usart_data_buffer[usart_buffer_first++];
        }
        return i;
    }

    std::size_t write(const unsigned char *ptr, std::size_t count) {
        for (int i = 0; i < count; ++i) {
            while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
            USART_SendData(USART1, *(ptr + i));
        }
        return count;
    }


    bool have_data_to_read() {
        if (usart_buffer_first != usart_buffer_end)
            return true;
        return false;
    }

    void trim_buffer_head() {
        while(usart_buffer_first!=usart_buffer_end){
            if(usart_buffer_first==CDH_USART_DATA_BUFFER_LENGTH)
                usart_buffer_first =0;
            unsigned char ch = usart_data_buffer[usart_buffer_first];
            if(ch == '\n'||ch == ' ' || ch == '\r')
                ++usart_buffer_first;
            else break;
        }
    }
}


void task_1(void*){
    for(;;){
        unsigned char a = 'a';
        cdh::write(&a,1);
        vTaskDelay(1000);
    }
}
void task_2(void*){
    for(;;){
        unsigned char b = 'b';
        cdh::write(&b,1);
        vTaskDelay(2000);
    }
}
int main(){
    SystemCoreClockUpdate();//设置系统时钟
    NVIC_SetPriorityGrouping(NVIC_PriorityGroup_2);
    cdh::init();
    xTaskCreate(task_1,"task_1",50,0,1,0);
    xTaskCreate(task_2,"task_2",50,0,1,0);
    vTaskStartScheduler();
    return 0;
}
