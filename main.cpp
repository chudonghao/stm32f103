#include <stm32f10x_conf.h>
#include <cstdio>
#include "usart1.h"
#include "led.h"

using namespace std;
using namespace cdh;

usart1_t usart1;
led1_t led1;
int std::fputc(int c, FILE *f) {
    if(c == EOF)
        return EOF;
    usart1.write((unsigned char*)&c,1,1);
    return c;
}
int std::fgetc(FILE *f){
    unsigned char ch;
		for (;;) {
        if(usart1.read(&ch,1,1)){
					return ch;
				}
    }
}

int main() {
    usart1.open();
    led1.open();
    printf("printf\n");
    for (;;) {
        unsigned char ch[30] = {0};
        scanf("%s",ch);
				printf((char*)ch);
				printf("\r\n");
    }
}

