#include "usart1.h"
#include "led.h"
#include "step_motor_couple.h"
#include "encoder.h"
#include "step_motor.h"
#include "mpu9250.h"
#include "third_party/lcd.h"

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

static int func = 0;
static vec2<float> acc_base;
static float f_base;

static void led1_func(void *) {
    char str[32];
    printf("func:circle coin laser.\r\n");
    printf("default func laser.\r\n");
    //***
    printf("laser.\r\n");
    f_base = encoder_t::angle();
    func = 3;
    //*//
    for (int i = 0;; ++i) {
        mpu9250_t::test();
        continue;
        while (!usart1.have_data_to_read())
            osThreadYield();
        scanf("%s", str);
        func = 0;
        f_base = encoder_t::angle();
        acc_base = mpu9250_t::acc();

        if (strcmp(str, "circle") == 0) {
            printf("circle.\r\n");

            func = 1;
        } else if (strcmp(str, "coin") == 0) {
            printf("coin.\r\n");
            func = 2;
        } else if (strcmp(str, "laser") == 0) {
            printf("laser.\r\n");

            func = 3;
        } else {
            printf("unknown func.\r\n");
        }
        usart1.trim_buffer_head();
    }
}

static void main_func(void *) {
    for (int i = 0;; ++i) {
        switch (func) {
            case 1:
                break;
            case 2:
                vec2<float> current_an = mpu9250_t::acc();
                printf("acc_base:%f,%f\tnew_acc:%f,%f\r\n", acc_base.x, acc_base.y, current_an.x, current_an.y);
                float s = ::cdh::angle(current_an, acc_base);
                float f = encoder_t::angle() - f_base;
                if (f < 0) {
                    s = -s;
                }
                step_motor_t::set_next_step(step_motor_t::map_angle_to_step(s));
                break;
            case 3: {
                const static float l1 = 102.f;
                const static float l2 = 150.f;
                float f = encoder_t::angle() - f_base;
                float l4 = l1 * sin(f);
                float l3 = l1 - l1 * cos(f);
                float l5 = l2 - l4;
                float tan_s = l3 / l5;
                float s = atan(tan_s);
                float a = f + s;
                step_motor_t::set_next_step(step_motor_t::map_angle_to_step(a));
                break;
            }
            default:
                break;
        }

        if (1) {//激光

        } else if (1) { // 保持水平
            float f = encoder_t::angle() - f_base;
            step_motor_t::set_next_step(step_motor_t::map_angle_to_step(f));
//            printf("%f\r\n",f);
        } else {
            //        vec2<float> acc = mpu9250_t::acc();
//        float s = angle(acc, acc_base);
            float f = encoder_t::angle() - f_base;
//        float a = s - f;
//        if (f < 0) {
//            s = -s;
//        }
//        printf("acc_base:%.3f,%.3f f_base:%.3f acc:%.3f,%.3f s:%.3f f:%.3f a:%.3f\r\n", acc_base.x, acc_base.y, f_base,
//               acc.x, acc.y, s, f, a);
            step_motor_t::set_next_step(step_motor_t::map_angle_to_step(f));
//        printf("f:%f,step:%d\r\n",f,encoder_t::value());
        }
    }
}

int main() {
    SystemCoreClockUpdate();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    usart1.open();
    printf("usart1 inited.\r\n");
    led1 = led1_t::open();
    encoder_t::open();
    mpu9250_t::open();
    step_motor_t::open();

//    acc_base = mpu9250_t::acc();
//    while (acc_base == vec2<float>()) {
//        printf("wait mpu9250\r\n");
//        acc_base = mpu9250_t::acc();
//    }
    printf("inited.\r\n");

    osKernelInitialize();
    const static osThreadAttr_t thread1_attr = {
            "", osThreadJoinable,
            0, 0,
            0, 1024,
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

