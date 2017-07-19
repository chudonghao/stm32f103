#include "usart1.h"
#include "led.h"
#include "step_motor_couple.h"
#include "encoder.h"
#include "step_motor.h"

#include <stm32f10x_conf.h>
#include <cmsis_os2.h>
#include <rtx_os.h>
#include <cstdio>
#include <cstring>
#include <rtx_evr.h>

using namespace std;
using namespace cdh;
usart1_t usart1;
led1_t *led1;

int std::fputc(int c, FILE *f) {
    usart1.write((unsigned char *) &c, 1, 1);
    return c;
}

int std::fgetc(FILE *f) {
    unsigned char ch;
    for (;;) {
        if (usart1.read(&ch, 1, 1)) {
            return ch;
        }
    }
}

static void led1_func(void *) {
    for (int i = 0;; ++i) {
        osDelay(50);
        step_motor_t::set_next_step(encoder_t::value());
        printf("%d\r\n", encoder_t::value());
    }
}

static void main_func(void *) {
    for (int i = 0;; ++i) {
        char ch[31] = {0};
        int j = 0, k = 0, l = 0, m = 0;
        scanf("%s", ch);

        usart1.trim_buffer_head();
    }
}

int main() {
    SystemCoreClockUpdate();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    usart1.open();
    led1 = led1_t::open();
    encoder_t::open();
    step_motor_t::open();

    printf("inited\r\n");

    osKernelInitialize();
    const static osThreadAttr_t thread1_attr = {
            "", osThreadJoinable,
            0, 0,
            0, 512,
            osPriorityAboveNormal, 0
    };
    osThreadNew(led1_func, NULL, &thread1_attr);
    const static osThreadAttr_t main_attr = {
            "", osThreadJoinable,
            0, 0,
            0, 2048,
            osPriorityAboveNormal, 0
    };
    osThreadNew(main_func, NULL, &main_attr);
    osKernelStart();
}

