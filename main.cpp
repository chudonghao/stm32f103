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
static bool on_hand = true;

static void led1_func(void *) {
//    encoder_t::enable_soft_sampling();
    char str[32];
    printf(">> func:circle coin laser.\r\n");
//    printf("mpu9250 test:\r\n");
//    mpu9250_t::test();
    f_base = encoder_t::angle();
    printf(">> f_base:%f\r\n", f_base);

    for (int i = 0;; ++i) {
//        encoder_t::test();
//        continue;
        while (!usart1.have_data_to_read())
            osThreadYield();
        scanf("%s", str);
        func = 0;
//        acc_base = mpu9250_t::acc();
//        printf("acc_base:%f,%f f_base:%f\r\n", acc_base.x, acc_base.y, f_base);

        if (strcmp(str, "circle") == 0) {
            printf("circle.\r\n");
            printf("!!set step motor pulse to 800.\r\n");
            step_motor_t::set_pulse(800);
            on_hand = true;
            func = 1;
        } else if (strcmp(str, "coin") == 0) {
            printf("coin.\r\n");
            printf("!!set step motor pulse to 800.\r\n");
            step_motor_t::set_pulse(800);
            on_hand = true;
            func = 2;
        } else if (strcmp(str, "laser") == 0) {
            printf("laser.\r\n");
            printf("!!set step motor pulse to 3200.\r\n");
            step_motor_t::set_pulse(3200);
            func = 3;
            for (int i = 0; i < 3; ++i) {
                led1->off();
                osDelay(200);
                led1->on();
                osDelay(200);
            }
        } else if (strcmp(str, "up") == 0) {
            printf("up step motor.\r\n");
            step_motor_t::fix_step(1);
        } else if (strcmp(str, "down") == 0) {
            printf("down step motor.\r\n");
            step_motor_t::fix_step(-1);
        } else {
            printf("unknown func.\r\n");
        }
        usart1.trim_buffer_head();
    }
}

static void main_func(void *) {
    for (int i = 0;; ++i) {
        switch (func) {
            case 1: {
                static float start_angle_of_pendulum = 0.f;
                static float w = 0.f;
                static float w_fix = 0.f;
                static int old_dir = 0;
                if (on_hand) {
                    start_angle_of_pendulum = 0.f;
                    w = 0.f;
                    w_fix = 0.f;
                    float current_angle = encoder_t::angle() - f_base;
                    if (abs(current_angle) < M_PI * 10 / 180) { // 角度太小
                        old_dir = encoder_t::dir();
                    } else if (old_dir == encoder_t::dir()) {//手在施力
                    } else { //释放
                        on_hand = false;
                        start_angle_of_pendulum = encoder_t::angle() - f_base;
                        osDelay(10);
                        old_dir = encoder_t::dir();
                    }
                } else {//手已经释放
                    if (old_dir == encoder_t::dir()) {
                        float current_angle = encoder_t::angle() - f_base;
                        w = abs(current_angle - start_angle_of_pendulum) / abs(start_angle_of_pendulum) / 2 * M_PI;
                    } else {
                        w = 0.f;
                        old_dir = encoder_t::dir();
                        start_angle_of_pendulum = encoder_t::angle() - f_base;
                        w_fix += M_PI;
                    }
                    step_motor_t::set_next_step(step_motor_t::map_angle_to_step(w_fix + w), 360);
                }

                break;
            }
            case 2: {
                static int old_dir = 0;
                if (on_hand) {
                    float current_angle = encoder_t::angle() - f_base;
                    if (abs(current_angle) < M_PI * 10 / 180) {//角度太小
                        old_dir = encoder_t::dir();
                        step_motor_t::set_next_step(step_motor_t::map_angle_to_step(current_angle), 360);
                    } else if (old_dir == encoder_t::dir()) {//手在施力
                        step_motor_t::set_next_step(step_motor_t::map_angle_to_step(current_angle), 360);
                    } else {//释放
                        on_hand = false;
                    }
                } else {//手已经释放
                    step_motor_t::set_next_step(step_motor_t::map_angle_to_step(0), 900);
                }
                break;
            }
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
                int step_fix;
                if (encoder_t::dir() > 0)
                    step_fix = 5;
                else
                    step_fix = -5;
                step_motor_t::set_next_step(step_motor_t::map_angle_to_step(a) + step_fix,360);
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
//    encoder_t::enable_soft_sampling();
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

