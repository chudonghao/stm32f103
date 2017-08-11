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
    flat_board_t flat_board;
    ball_t ball;
    vec2 ball_position_sampling;
    vec2 aim_position = vec2(0.f);
    vec2 aim_position_threshold = vec2(13.f);
    arm_pid_instance_f32 arm_pid_instance1;
    arm_pid_instance_f32 arm_pid_instance2;
    arm_fir_instance_f32 arm_fir_instance1;
    arm_fir_instance_f32 arm_fir_instance2;
    arm_fir_instance_f32 arm_fir_instance3;
    arm_fir_instance_f32 arm_fir_instance4;
    float arm_fir_state1[TAP20_FS60_FC1_SIZE + BLOCK_SIZE - 1];
    float arm_fir_state2[TAP20_FS60_FC1_SIZE + BLOCK_SIZE - 1];
    float arm_fir_state3[TAP20_FS60_FC1_SIZE + BLOCK_SIZE - 1];
    float arm_fir_state4[TAP20_FS60_FC1_SIZE + BLOCK_SIZE - 1];
//    arm_fir_instance_f32 arm_fir_instance3;
//    float arm_fir_state3[TAP_SIZE+BLOCK_SIZE-1];
    enum ball_func_e {
        ball_func_to_zero_e = 1,
    };
    ball_func_e ball_func;
    bool is_running = false;
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
            is_running = false;
            arm_pid_reset_f32(&arm_pid_instance1);
            arm_pid_reset_f32(&arm_pid_instance2);
            flat_board.set_base();
        } else if (key == 2) {
            ivec2 cur = step_motor_couple_t::current_steps();
            cur.y += 1;
            step_motor_couple_t::set_next_steps(cur);
            step_motor_couple_t::step();
        } else if (key == 5) {
            ivec2 cur = step_motor_couple_t::current_steps();
            cur.x -= 1;
            step_motor_couple_t::set_next_steps(cur);
            step_motor_couple_t::step();
        } else if (key == 1) {
            ivec2 cur = step_motor_couple_t::current_steps();
            cur.x += 1;
            step_motor_couple_t::set_next_steps(cur);
            step_motor_couple_t::step();
        } else if (key == 6) {
            ivec2 cur = step_motor_couple_t::current_steps();
            cur.y -= 1;
            step_motor_couple_t::set_next_steps(cur);
            step_motor_couple_t::step();
        } else if (key == 8) {
            ball_func = ball_func_to_zero_e;
            is_running = true;
        }
        osDelay(10);
    }
}

static inline vec2 compute_aim_v(vec2 diff_p) {
    int sign_diff_p;
    float abs_diff_p;
    vec2 res = vec2(0.f);
    if (diff_p.x < 0) {
        abs_diff_p = -diff_p.x;
        sign_diff_p = -1;
    } else {
        abs_diff_p = diff_p.x;
        sign_diff_p = 1;
    }
    if (abs_diff_p< aim_position_threshold.x) {
        res.x = 0.f;
    }else{
        res.x = (abs_diff_p + 10.f) * 0.5f * sign_diff_p;
    }
    if (diff_p.y < 0) {
        abs_diff_p = -diff_p.y;
        sign_diff_p = -1;
    } else {
        abs_diff_p = diff_p.y;
        sign_diff_p = 1;
    }
    if (abs_diff_p< aim_position_threshold.y) {
        res.y = 0.f;
    }else{
        res.y = (abs_diff_p + 10.f) * 0.5f * sign_diff_p;
    }
    return res;
}
const static float max_a = 200.f;
static inline vec2 compute_aim_a(vec2 diff_v){
    vec2 aim_a = vec2(0.f);
    if (diff_v.x >= 70.f) {
        aim_a.x = max_a;
    } else if (diff_v.x < -70.f) {
        aim_a.x = -max_a;
    } else {
        aim_a.x = (diff_v.x) * max_a / 70.f;
    }
    if (diff_v.y >= 70.f) {
        aim_a.y = max_a;
    } else if (diff_v.y < -70.f) {
        aim_a.y = -max_a;
    } else {
        aim_a.y = (diff_v.y) * max_a / 70.f;
    }
    return aim_a;
}
static inline ivec2 pid_input(vec2 except_position) {
    vec2 diff_p = except_position - ball.position();
    vec2 diff_v;
    vec2 aim_a;
    vec2 pid_input;
    diff_v = compute_aim_v(diff_p) - ball.v();
    aim_a = compute_aim_a(diff_v) - ball.a();
    pid_input = aim_a - ball.a();
    return pid_input;
}

extern "C" void pid_task(const void *) {
    osThreadTerminate(osThreadGetId());
//    arm_pid_instance1.Kp = 1.f / 9800.f;
//    arm_pid_instance1.Ki = 0.f;
//    arm_pid_instance1.Kd = 0.f;
//    arm_pid_instance2.Kp = arm_pid_instance1.Kp;
//    arm_pid_instance2.Ki = arm_pid_instance1.Ki;
//    arm_pid_instance2.Kd = arm_pid_instance1.Kd;
//    arm_pid_init_f32(&arm_pid_instance1, 1);
//    arm_pid_init_f32(&arm_pid_instance2, 1);
//
//    arm_fir_init_f32(&arm_fir_instance1, TAP20_FS60_FC1_SIZE, TAP20_FS60_FC1, arm_fir_state1, BLOCK_SIZE);
//    arm_fir_init_f32(&arm_fir_instance2, TAP20_FS60_FC1_SIZE, TAP20_FS60_FC1, arm_fir_state2, BLOCK_SIZE);
//    arm_fir_init_f32(&arm_fir_instance3, TAP20_FS60_FC1_SIZE, TAP20_FS60_FC1, arm_fir_state3, BLOCK_SIZE);
//    arm_fir_init_f32(&arm_fir_instance4, TAP20_FS60_FC1_SIZE, TAP20_FS60_FC1, arm_fir_state4, BLOCK_SIZE);
//    for (;;) {
//        static float last_ms;
//        static float ms;
//        static vec2 new_ball_p = vec2(0.f);
//        static vec2 new_ball_v = vec2(0.f);
//        static vec2 ball_a_sampling = vec2(0.f);
//        static vec2 new_ball_a = vec2(0.f);
//        ms = xTaskGetTickCount();
//        new_ball_p = vec2(0.f);
//        new_ball_v = vec2(0.f);
//        ball_a_sampling = vec2(0.f);
//        new_ball_a = vec2(0.f);
//
//        arm_fir_f32(&arm_fir_instance1, &ball_position_sampling.x, &new_ball_p.x, 1);
//        arm_fir_f32(&arm_fir_instance2, &ball_position_sampling.y, &new_ball_p.y, 1);
//        if (last_ms != ms) {
//            new_ball_v = (new_ball_p - ball.position()) * 1000.f / (ms - last_ms);
//            ball_a_sampling = (new_ball_v - ball.v()) * 1000.f / (ms - last_ms);
//            //ball_a_sampling = - flat_board.dip_angle() * 9800.f;
//        }
//        arm_fir_f32(&arm_fir_instance3, &ball_a_sampling.x, &new_ball_a.x, 1);
//        arm_fir_f32(&arm_fir_instance4, &ball_a_sampling.y, &new_ball_a.y, 1);
//
//        last_ms = ms;
//        ball.position(new_ball_p);
//        ball.v(new_ball_v);
//        ball.a(new_ball_a);
//        printf("%f,%f,%f,%f,%f;\r\n",ms, ball_position_sampling.x, ball.position().x
//        , ball.v().x,ball.a().x);
//
//        if (is_running) {
//            vec2 pid_input = ::pid_input(aim_position);
//            vec2 aim_angle;
//            aim_angle.x= -arm_pid_f32(&arm_pid_instance1, pid_input.x);
//            aim_angle.y= -arm_pid_f32(&arm_pid_instance2, pid_input.y);
//            //printf("aim_v=%f,ball.p=%f,aim_angle=%f\r\n",aim_v,ball.position(),aim_angle);
//            switch (ball_func) {
//                case ball_func_to_zero_e:
//                    aim_angle.x = 0.f;//TODO 测试单向运动
//                    flat_board.dip_angle(aim_angle);
//                    flat_board.motor();
//                    break;
//            }
//        }
//
//        osDelay(16);//达到60Hz
//    }
}
extern "C" unsigned char uart1_have_data_to_read();
extern "C" void main_task(const void *) {
    arm_pid_instance1.Kp = 1.f;
    arm_pid_instance1.Ki = 0.01f;
    arm_pid_instance1.Kd = 6.f;
    arm_pid_instance2.Kp = arm_pid_instance1.Kp;
    arm_pid_instance2.Ki = arm_pid_instance1.Ki;
    arm_pid_instance2.Kd = arm_pid_instance1.Kd;
    arm_pid_init_f32(&arm_pid_instance1,1);
    arm_pid_init_f32(&arm_pid_instance2,1);
    printf("inited.\r\n");
    for (;;) {
        static char ch[128];
        if(!uart1_have_data_to_read()){
            osThreadYield();
        }
        scanf("%s", ch);
        if (strcmp(ch, "ball") == 0) {
            scanf("%f%f", &ball_position_sampling.x,&ball_position_sampling.y);
            if(is_running) {
                vec2 out_put = vec2(0.f,0.f);
                //TODO 先实现一个轴向
                out_put.x = - arm_pid_f32(&arm_pid_instance1, 200.f - ball_position_sampling.x);
                out_put.y = arm_pid_f32(&arm_pid_instance2,200.f - ball_position_sampling.y);
                out_put.x = clamp(out_put.x,-70.f,70.f);
                out_put.y = clamp(out_put.y,-70.f,70.f);
                step_motor_couple_t::set_next_steps(out_put,true);
                step_motor_couple_t::step();
            }
//            printf("get data:%f %f\r\n", ball_position_sampling.x,ball_position_sampling.y);
        }
//        else if (strcmp(ch, "red") == 0) {
//            float red_point_position;
//            scanf("%f", &red_point_position);
//            if (ball_func == ball_func_follow_red_e || ball_func == ball_func_follow_red_2_e)
//                aim_position = red_point_position;
//        }
        else if (strcmp(ch, "test") == 0) {
            float dip_angle, height;
            scanf("%f%f", &dip_angle, &height);
            printf("test:%f %f\r\n", dip_angle, height);
//            track.dip_angle(dip_angle);
//            track.height(height);
//            while (track.motor() == -1) {
//                step_motor_couple_t::step();
//            }
//            while (track.motor() == -1) {
//                step_motor_couple_t::step();
//            }
        } else {
            //printf("unknown func:%s\r\n", ch);
        }
    }
}

