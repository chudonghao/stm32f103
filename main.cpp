#include <stm32f10x_conf.h>
#include <cstdio>
#include "usart1.h"

using namespace std;
using namespace cdh;

usart1_t usart1;

int std::fputc(int c, FILE *f) {
    usart1.write((unsigned char*)&c,4,1);
    return c;
}

int main() {
    usart1.open();

    printf("printf\n");

    for (;;) {
        for (int i = 0; i < 1000000; ++i);
        GPIO_WriteBit(GPIOA, GPIO_Pin_8, Bit_SET);
        for (int i = 0; i < 1000000; ++i);
        GPIO_WriteBit(GPIOA, GPIO_Pin_8, Bit_RESET);
    }
}

