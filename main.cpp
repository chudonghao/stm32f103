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
    vec2 aim_position_threshold = vec2(10.f);
    float pid_ki_typical_value = 0.01f;
    float pid_kd_typical_value = 60.f;
    float pid_ki_aim_close_typical_value = 0.1f;
    float pid_kd_aim_close_typical_value = 60.f;

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
        ball_func_to_2_e = 1,
        ball_func_1_5_e,
        ball_func_1_4_5_e,
        ball_func_1_9_e,
        ball_func_1_2_6_9_e,
    };
    ball_func_e ball_func;
    bool is_running = false;
    const static vec2 POINT_1 = vec2(-200.f, 200.f);
    const static vec2 POINT_2 = vec2(0.f, 200.f);
    const static vec2 POINT_3 = vec2(200.f, 200.f);
    const static vec2 POINT_4 = vec2(-200.f, 0.f);
    const static vec2 POINT_5 = vec2(0.f, 0.f);
    const static vec2 POINT_6 = vec2(200.f, 0.f);
    const static vec2 POINT_7 = vec2(-200.f, -200.f);
    const static vec2 POINT_8 = vec2(0.f, -200.f);
    const static vec2 POINT_9 = vec2(200.f, -200.f);
    const static vec2 POINT_10 = vec2(-200.f, 100.f);
    const static vec2 POINT_11 = vec2(0.f, 100.f);
    const static vec2 POINT_12 = vec2(200.f, 100.f);
    const static vec2 POINT_13 = vec2(-200.f, -100.f);
    const static vec2 POINT_14 = vec2(0.f, -100.f);
    const static vec2 POINT_15 = vec2(200.f, -100.f);
    const static vec2 POINT_16 = vec2(-100.f, 200.f);
    const static vec2 POINT_17 = vec2(-100.f, 0.f);
    const static vec2 POINT_18 = vec2(-100.f, -200.f);
    const static vec2 POINT_19 = vec2(100.f, 200.f);
    const static vec2 POINT_20 = vec2(100.f, 0.f);
    const static vec2 POINT_21 = vec2(100.f, -200.f);
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
            ball_func = ball_func_to_2_e;
            aim_position = POINT_2;
            aim_position_threshold = vec2(10.f);
            is_running = true;
        } else if (key == 9) {
            ball_func = ball_func_1_5_e;
            aim_position = POINT_5;
            aim_position_threshold = vec2(10.f);
            is_running = true;
        } else if (key == 10) {
            ball_func = ball_func_1_4_5_e;
            aim_position = POINT_4;
            aim_position_threshold = vec2(10.f);
            is_running = true;
        } else if(key == 11){
            ball_func = ball_func_1_9_e;
            aim_position = POINT_17;
            aim_position_threshold = vec2(30.f);
            is_running = true;
        } else if(key == 12){
            ball_func = ball_func_1_2_6_9_e;
            aim_position = POINT_2;
            aim_position_threshold = vec2(10.f);
            is_running = true;
        }
        osDelay(10);
    }
}

void refresh_ball_state() {
    static float last_ms = 0.f;
    static float ms;
    ms = osKernelSysTick();
    if (ms != last_ms) {
        vec2 v;
        v.x = (ball_position_sampling.x - ball.position().x) * 1000.f / (ms - last_ms);
        v.y = (ball_position_sampling.y - ball.position().y) * 1000.f / (ms - last_ms);
        ball.position(ball_position_sampling);
        ball.v(v);
    }
    last_ms = ms;
}

void refresh_pid_arg(){
//    //根据距离的Kd
//    vec2 dynamic_Kd = /*diff_p / 6.f*/(aim_position - ball_position_sampling) / 200.f * pid_kd_typical_value;
//    //abs
//    if (dynamic_Kd.x < 0.f)
//        dynamic_Kd.x = -dynamic_Kd.x;
//    if (dynamic_Kd.y < 0.f)
//        dynamic_Kd.y = -dynamic_Kd.y;
//    //Kd不能太小
//    //使pid参数修改生效
//    if (dynamic_Kd.x >= pid_kd_typical_value) {
//        arm_pid_instance1.Kd = dynamic_Kd.x;
//        arm_pid_init_f32(&arm_pid_instance1, 0);
//    }
//    if (dynamic_Kd.y >= pid_kd_typical_value) {
//        arm_pid_instance2.Kd = dynamic_Kd.y;
//        arm_pid_init_f32(&arm_pid_instance2, 0);
//    }
    vec2 diff_p = aim_position - ball.position();
    if(abs(diff_p.x) < 70.f){
        arm_pid_instance1.Ki = pid_ki_aim_close_typical_value;
        arm_pid_instance1.Kd = pid_kd_aim_close_typical_value;
        arm_pid_init_f32(&arm_pid_instance1,0);
    }else{
        arm_pid_instance1.Ki = pid_ki_typical_value;
        arm_pid_instance1.Kd = pid_kd_typical_value;
        arm_pid_init_f32(&arm_pid_instance1,0);
    }
    if(abs(diff_p.y) < 70.f){
        arm_pid_instance2.Ki = pid_ki_aim_close_typical_value;
        arm_pid_instance2.Kd = pid_kd_aim_close_typical_value;
        arm_pid_init_f32(&arm_pid_instance2,0);
    }else{
        arm_pid_instance2.Ki = pid_ki_typical_value;
        arm_pid_instance2.Kd = pid_kd_typical_value;
        arm_pid_init_f32(&arm_pid_instance2,0);
    }
}

void common_ball_move_func() {
    refresh_pid_arg();
    //进入pid调节
    vec2 out_put = vec2(0.f, 0.f);
    out_put.x = -arm_pid_f32(&arm_pid_instance1, aim_position.x - ball_position_sampling.x);
    out_put.y = arm_pid_f32(&arm_pid_instance2, aim_position.y - ball_position_sampling.y);
    //限幅
    out_put.x = clamp(out_put.x, -90.f, 90.f);
    out_put.y = clamp(out_put.y, -90.f, 90.f);
    //强制电机向指定步数运动
    step_motor_couple_t::set_next_steps(out_put, true);
    step_motor_couple_t::step();
    printf("%.3f,%.3f;\r\n", ball_position_sampling.x, ball_position_sampling.y);
}

void ball_func_1_to_5() {
    if (abs(ball_position_sampling.x - aim_position.x) < aim_position_threshold.x
        && abs(ball_position_sampling.y - aim_position.y) < aim_position_threshold.y) {
        if (abs(ball.v().x) < 20.f && abs(ball.v().y) < 20.f) {
            flat_board.dip_angle(vec2(0.f));
            flat_board.motor();
            is_running = false;
            return;
        }
    }
    common_ball_move_func();
}

void ball_func_to_2() {
    if (abs(ball_position_sampling.x - aim_position.x) < aim_position_threshold.x
        && abs(ball_position_sampling.y - aim_position.y) < aim_position_threshold.y) {
        if (abs(ball.v().x) < 20.f && abs(ball.v().y) < 20.f) {
            flat_board.dip_angle(vec2(0.f));
            flat_board.motor();
            is_running = false;
            return;
        }
    }
    common_ball_move_func();
}

void ball_func_1_4_5() {
    if (abs(ball_position_sampling.x - aim_position.x) < aim_position_threshold.x
        && abs(ball_position_sampling.y - aim_position.y) < aim_position_threshold.y) {
        if (abs(ball.v().x) < 20.f && abs(ball.v().y) < 20.f) {
            flat_board.dip_angle(vec2(0.f));
            flat_board.motor();
            if (aim_position == POINT_4) {
                aim_position = POINT_5;
                osDelay(1800);
            } else {
                is_running = false;
                return;
            }
        }
    }
    common_ball_move_func();
}

void ball_func_1_9(){
    if (abs(ball_position_sampling.x - aim_position.x) < aim_position_threshold.x
        && abs(ball_position_sampling.y - aim_position.y) < aim_position_threshold.y) {
        if(aim_position == POINT_17){//抵达中间点
            aim_position = POINT_9;
            aim_position_threshold = vec2(10.f);
        }else if (abs(ball.v().x) < 20.f && abs(ball.v().y) < 20.f) {//抵达9
            flat_board.dip_angle(vec2(0.f));
            flat_board.motor();
            is_running = false;
            return;
        }
    }
    common_ball_move_func();
}

void ball_func_1_2_6_9(){
    if (abs(ball_position_sampling.x - aim_position.x) < aim_position_threshold.x
        && abs(ball_position_sampling.y - aim_position.y) < aim_position_threshold.y) {
        if(aim_position == POINT_2){//抵达中间点
            aim_position = POINT_6;
            aim_position_threshold = vec2(10.f);
        }else if(aim_position == POINT_6){
            aim_position = POINT_9;
            aim_position_threshold = vec2(10.f);
        }else if (abs(ball.v().x) < 20.f && abs(ball.v().y) < 20.f) {//抵达9
            flat_board.dip_angle(vec2(0.f));
            flat_board.motor();
            is_running = false;
            return;
        }
    }
    common_ball_move_func();
}

extern "C" void pid_task(const void *) {
    osThreadTerminate(osThreadGetId());
}

extern "C" unsigned char uart1_have_data_to_read();
extern "C" void main_task(const void *) {
    arm_pid_instance1.Kp = 3.f;
    arm_pid_instance1.Ki = pid_ki_typical_value;
    arm_pid_instance1.Kd = pid_kd_typical_value;
    arm_pid_instance2.Kp = arm_pid_instance1.Kp;
    arm_pid_instance2.Ki = arm_pid_instance1.Ki;
    arm_pid_instance2.Kd = arm_pid_instance1.Kd;
    arm_pid_init_f32(&arm_pid_instance1, 1);
    arm_pid_init_f32(&arm_pid_instance2, 1);
    printf("inited.\r\n");
    for (;;) {
        static char ch[128];
        if (!uart1_have_data_to_read()) {
            osThreadYield();
        }
        scanf("%s", ch);
        if (strcmp(ch, "ball") == 0) {
            int scanf_res = scanf("%f%f", &ball_position_sampling.x, &ball_position_sampling.y);
            if (scanf_res != 2) {
                continue;
            }
            refresh_ball_state();
            if (is_running) {
                switch (ball_func) {
                    case ball_func_1_5_e:
                        ball_func_1_to_5();
                        break;
                    case ball_func_to_2_e:
                        ball_func_to_2();
                        break;
                    case ball_func_1_4_5_e:
                        ball_func_1_4_5();
                        break;
                    case ball_func_1_9_e:
                        ball_func_1_9();
                        break;
                    case ball_func_1_2_6_9_e:
                        ball_func_1_2_6_9();
                        break;
                }
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

