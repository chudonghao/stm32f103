//
// Created by leu19 on 2017/7/28.
//
#include <cstdio>
#include <cstring>
#include <main.h>
#include <gpio.h>
#include <cmsis_os.h>
#include <arm_math.h>

#include "step_motor_couple.h"
#include "key_board.h"
#include "usart3.h"
#include "undecided.h"
#include "fdacoefs.h"

using namespace cdh;
using namespace std;
using namespace glm;
namespace {
    const int BLOCK_SIZE = 20;
    track_t track;
    ball_t ball;
    float ball_position_sampling;
    bool start = false;
    float aim_position = 425.f;
    arm_pid_instance_f32 arm_pid_instance;
    arm_fir_instance_f32 arm_fir_instance1;
    float arm_fir_state1[TAP_SIZE + BLOCK_SIZE - 1];
    arm_fir_instance_f32 arm_fir_instance2;
    float arm_fir_state2[TAP_SIZE + BLOCK_SIZE - 1];
//    arm_fir_instance_f32 arm_fir_instance3;
//    float arm_fir_state3[TAP_SIZE+BLOCK_SIZE-1];
}
extern "C" void key_board_task(const void *) {
    static int old_key = -1;
    static int delay_count = 0;
    for (;;) {
        int key = key_board_t::scan();
        if (old_key == key && delay_count < 10) {
            ++delay_count;
            if (delay_count > 0) {
                osDelay(30);
                continue;
            }
        } else if (old_key != key) {
            old_key = key;
            delay_count = 0;
        }
        if (key == 0) {
            arm_pid_reset_f32(&arm_pid_instance);
            track.set_base();
            start = !start;
        } else if (key == 2) {
            ivec2 cur = step_motor_couple_t::current_steps();
            cur.y += 1;
            step_motor_couple_t::set_next_steps(cur);
        } else if (key == 5) {
            ivec2 cur = step_motor_couple_t::current_steps();
            cur.x -= 1;
            step_motor_couple_t::set_next_steps(cur);
        } else if (key == 1) {
            ivec2 cur = step_motor_couple_t::current_steps();
            cur.x += 1;
            step_motor_couple_t::set_next_steps(cur);
        } else if (key == 6) {
            ivec2 cur = step_motor_couple_t::current_steps();
            cur.y -= 1;
            step_motor_couple_t::set_next_steps(cur);
        }
        osDelay(10);
    }
}

static inline void test() {

}

static inline float pid_input(float except_position) {
    float diff_p = except_position - ball.position();
    float diff_v;
    float aim_v;
    float aim_a;
    float pid_input;
    if (diff_p >= 70.f) {
        aim_v = 70.f;
    } else if (diff_p < -70.f) {
        aim_v = -70.f;
    } else if (diff_p >= -15.f && diff_p < 15.f) {
        aim_v = 0.f;
    } else if (diff_p < 0) {
        aim_v = diff_p;
    } else {
        aim_v = diff_p;
    }
    diff_v = aim_v - ball.v();
    if (diff_v >= 70.f) {
        aim_a = 100.f;
    } else if (diff_v < -70.f) {
        aim_a = -100.f;
    } else {
        aim_a = diff_v * (100.f / 70.f);
    }
    pid_input = aim_a - ball.a();
    if (diff_p < 25.f && diff_p >= -25.f) {
        pid_input *= 0.2;
    } else if (pid_input >= 50.f) {
        pid_input = 50.f;
    } else if (pid_input < -50.f) {
        pid_input = -50.f;
    }
    return pid_input;
}

extern "C" void pid_task(const void *) {
    arm_pid_instance.Kp = 0.03f / 100.f;
    arm_pid_instance.Ki = 0.00001f;
    arm_pid_instance.Kd = 0.00006f;
    arm_pid_init_f32(&arm_pid_instance, 1);
    arm_fir_init_f32(&arm_fir_instance1, TAP_SIZE, TAP, arm_fir_state1, BLOCK_SIZE);
    arm_fir_init_f32(&arm_fir_instance2, TAP_SIZE, TAP, arm_fir_state2, BLOCK_SIZE);
    for (;;) {
        static float last_ms;
        float ms = xTaskGetTickCount();
        float new_ball_p = 0.f;
        float new_ball_v = 0.f;
        float ball_a_sampling = 0.f;
        float new_ball_a = 0.f;
        arm_fir_f32(&arm_fir_instance1, &ball_position_sampling, &new_ball_p, 1);
        if (last_ms != ms) {
            new_ball_v = (new_ball_p - ball.position()) * 1000.f / (ms - last_ms);
            ball_a_sampling = (new_ball_v - ball.v()) * 1000.f / (ms - last_ms);
        }
        arm_fir_f32(&arm_fir_instance2, &ball_a_sampling, &new_ball_a, 1);

        last_ms = ms;
        ball.position(new_ball_p);
        ball.v(new_ball_v);
        ball.a(new_ball_a);
//        printf("%f,%f,%f;\r\n", ball.position(), ball.v(), ball.a());
//            static int last_ms ;
//            static float last_v;
//            int ms = xTaskGetTickCount();
//            last_ms = ms;
//            last_v = ball_v;
//            {
//                static float last_ms = 0,last_ball_position = 0,last_v = 0;
//                float ms = xTaskGetTickCount();
//                float v;
//                if(ms != last_ms){
//                    v = (ball_position - last_ball_position) * 1000.f/ (ms - last_ms);
//                }
//                last_ms = ms;
//                last_ball_position = ball_position;
//                last_v = v;
//
//            }

        float aim_angle = -arm_pid_f32(&arm_pid_instance, pid_input(aim_position));
        //printf("aim_v=%f,ball.p=%f,aim_angle=%f\r\n",aim_v,ball.position(),aim_angle);

        if (start) {
            if (ball.position() - aim_position >= -15 && ball.position() - aim_position <= 15
                && ball.v() < 20.f && ball.v() >= -20.f) {
                track.dip_angle(0.f);
                
                start = false;
                printf("hint\r\n");
            } else {
                track.dip_angle(aim_angle);
            }
            track.height_base(ball.position() + 10.f);
            track.motor();
        }
        osDelay(30);
    }
}
extern "C" void main_task(const void *) {
    printf("inited.\r\n");
    for (;;) {
        static char ch[128];
        scanf("%s", ch);
        if (strcmp(ch, "ball") == 0) {
            float ball_position;
            scanf("%f", &ball_position);
            //printf("get data:%f %f %f %f\r\n", ms, y_left,y_right,ball_position);
            ball_position_sampling = ball_position;
        } else if (strcmp(ch, "red") == 0) {
            float red_point_position;
            scanf("%f", &red_point_position);
            aim_position = red_point_position;
        } else if (strcmp(ch, "test") == 0) {
            float dip_angle, height;
            scanf("%f%f", &dip_angle, &height);
            printf("test:%f %f\r\n", dip_angle, height);
            track.dip_angle(dip_angle);
            track.height(height);
            while (track.motor() == -1) {
                step_motor_couple_t::step();
            }
            while (track.motor() == -1) {
                step_motor_couple_t::step();
            }
        } else if (strcmp(ch, "test2") == 0) {
            float height_base;
            scanf("%f", &height_base);
            printf("test2:%f\r\n", height_base);
            track.height_base(height_base);
        } else {
            //printf("unknown func:%s\r\n", ch);
        }
    }
}

