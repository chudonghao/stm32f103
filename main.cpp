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
    float aim_position = 425.f;
    float aim_position_threshold = 20.f;
    arm_pid_instance_f32 arm_pid_instance;
    arm_fir_instance_f32 arm_fir_instance1;
    float arm_fir_state1[TAP_SIZE + BLOCK_SIZE - 1];
    arm_fir_instance_f32 arm_fir_instance2;
    float arm_fir_state2[TAP_SIZE + BLOCK_SIZE - 1];
//    arm_fir_instance_f32 arm_fir_instance3;
//    float arm_fir_state3[TAP_SIZE+BLOCK_SIZE-1];
    enum ball_func_e {
        ball_func_ma_cd_ab_dn_e = 1,
        ball_func_follow_red_e,
        ball_func_follow_red_2_e,
        ball_func_up_down_e,
        ball_func_ab_cd_e,
        ball_func_cd_ab_e,
    };
    ball_func_e ball_func;
    bool is_running = false;
    bool ball_func_ab_cd_ab = false;
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
            ball_func_ab_cd_ab = false;
            is_running = false;
            arm_pid_reset_f32(&arm_pid_instance);
            track.set_base();
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
        } else if (key == 8) {
            ball_func = ball_func_ma_cd_ab_dn_e;
            is_running = true;
            aim_position = 50.f;
        } else if (key == 9) {
            ball_func = ball_func_follow_red_e;
            is_running = true;
        } else if (key == 10) {
            ball_func = ball_func_follow_red_2_e;
            is_running = true;
        } else if (key == 12) {
            ball_func = ball_func_up_down_e;
            is_running = true;
            aim_position = 275.f;
        } else if (key == 13) {
            ball_func = ball_func_ab_cd_e;
            is_running = true;
            aim_position = 425.f;
        } else if (key == 14) {
            ball_func = ball_func_cd_ab_e;
            is_running = true;
            aim_position = 125.f;
        } else if (key == 15) {
            ball_func = ball_func_ab_cd_e;
            is_running = true;
            ball_func_ab_cd_ab = true;
        }
        osDelay(10);
    }
}

static inline float compute_aim_v(float diff_p) {
    int sign_diff_p;
    float abs_diff_p;
    float res = 0.f;
    if (diff_p < 0) {
        abs_diff_p = -diff_p;
        sign_diff_p = -1;
    } else {
        abs_diff_p = diff_p;
        sign_diff_p = 1;
    }

    if (abs_diff_p < aim_position_threshold) {
        return 0.f;
    }

//    if(abs_diff_p < 10.f){
//        return 0.f;
//    }
//    if(abs_diff_p < 40.f) {
//        res = 30.f +(abs_diff_p - 10.f) * 2.f;
//        return res * sign_diff_p;
//    }
//
//    if(abs_diff_p < 70.f){
//        res = 90.f + (abs_diff_p - 40.f) * 1.5f;
//        return res * sign_diff_p;
//    }
//    res = 135.f + (abs_diff_p - 70.f) * 1.f;
    return (abs_diff_p + 40.f) * 0.8f * sign_diff_p;
}

static inline float pid_input(float except_position) {
    float diff_p = except_position - ball.position();
    float diff_v;
    float aim_a;
    float pid_input;
    diff_v = compute_aim_v(diff_p) - ball.v();
    if (diff_v >= 70.f) {
        aim_a = 240.f;
    } else if (diff_v < -70.f) {
        aim_a = -240.f;
    } else {
        aim_a = (diff_v) * 240.f / 70.f;
    }
    pid_input = aim_a - ball.a();
    static float  fix_aim_a =0.f;
    if (ball.v() < 30.f && ball.v() >= -30.f) {
        if (ball.position() - aim_position < -aim_position_threshold ||
            ball.position() - aim_position > aim_position_threshold) {
                if(diff_p > 0.f){
                    pid_input += 200.f;
                }else{
                    pid_input -= 200.f;
                }
//                fix_aim_a += 40.f;
        }
//            else{
//            fix_aim_a = 0.f;
//        }
    }
//    else{
//        if(fix_aim_a >=40.f)
//            fix_aim_a -= 40.f;
//    }
    
//    if(diff_p < 0.f){
//        pid_input = - fix_aim_a + (aim_a - ball.a());
//    }else{
//        pid_input = fix_aim_a + (aim_a -ball.a());
//    }
//    if (diff_p < 25.f && diff_p >= -25.f) {
//        pid_input *= 1.0f;
//    } else if (pid_input >= 50.f) {
//        pid_input = 50.f;
//    } else if (pid_input < -50.f) {
//        pid_input = -50.f;
//    }
    return pid_input;
}

extern "C" void pid_task(const void *) {
    arm_pid_instance.Kp = 1.f / 9800.f;
    arm_pid_instance.Ki = arm_pid_instance.Kp * 0.15f;
    arm_pid_instance.Kd = arm_pid_instance.Kp * 0.1f;
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
            ball_a_sampling = -track.dip_angle() * 9800.f;
        }
        arm_fir_f32(&arm_fir_instance2, &ball_a_sampling, &new_ball_a, 1);

        last_ms = ms;
        ball.position(new_ball_p);
        ball.v(new_ball_v);
        ball.a(new_ball_a);
//        printf("%f,%f,%f,%f;\r\n", ball.position(), ball.v(), ball.a(),track.dip_angle());
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


        if (is_running) {
            float aim_angle = -arm_pid_f32(&arm_pid_instance, pid_input(aim_position));
            //printf("aim_v=%f,ball.p=%f,aim_angle=%f\r\n",aim_v,ball.position(),aim_angle);
            switch (ball_func) {
                case ball_func_ma_cd_ab_dn_e:
                    if (ball.position() - aim_position >= -aim_position_threshold &&
                        ball.position() - aim_position <= aim_position_threshold
                        && ball.v() < 30.f && ball.v() >= -30.f && ball.a() < 80.f && ball.a() >= -80.f) {
                        track.dip_angle(0.f);
                        track.motor();
                        printf("hint\r\n");
                        if (aim_position == 50.f) {
                            aim_position_threshold = 20.f;
                            aim_position = 425.f;
                        } else if (aim_position == 425.f) {
                            aim_position_threshold = 20.f;
                            aim_position = 125.f;
                        } else if (aim_position == 125.f) {
                            aim_position_threshold = 45.f;
                            aim_position = 500.f;
                        } else {
                            aim_position_threshold = 20.f;
                            is_running = false;
                        }
                        osDelay(2500);
                    } else {
                        track.dip_angle(aim_angle);
                        track.motor();
                    }

                    break;
                case ball_func_follow_red_e:
                    if (ball.position() - aim_position >= -aim_position_threshold &&
                        ball.position() - aim_position <= aim_position_threshold
                        && ball.v() < 30.f && ball.v() >= -30.f && ball.a() < 80.f && ball.a() >= -80.f) {
                        track.dip_angle(0.f);
                        track.motor();
                        printf("hint\r\n");
                        is_running = false;
                    } else {
                        track.dip_angle(aim_angle);
                        track.motor();
                    }
                    break;
                case ball_func_follow_red_2_e:
                    if (ball.position() - aim_position >= -aim_position_threshold &&
                        ball.position() - aim_position <= aim_position_threshold
                        && ball.v() < 30.f && ball.v() >= -30.f && ball.a() < 80.f && ball.a() >= -80.f) {
                        track.dip_angle(0.f);
                        track.motor();
                        printf("hint\r\n");
                    } else {
                        track.dip_angle(aim_angle);
                        track.motor();
                    }
                    break;
                case ball_func_up_down_e: {
                    static bool up = true;
                    if (ball.position() - aim_position >= -aim_position_threshold &&
                        ball.position() - aim_position <= aim_position_threshold
                        && ball.v() < 30.f && ball.v() >= -30.f && ball.a() < 80.f && ball.a() >= -80.f) {
                        if (up) {
                            track.height(track.height() + 1.f);
                            if (track.height() >= 80) {
                                up = false;
                            }
                        } else {
                            track.height(track.height() - 1.f);
                        }
                        track.dip_angle(0.f);
                        track.motor();
                        if (up == false && track.height() <= 0) {
                            is_running = false;
                            up = true;
                            printf("hint\r\n");
                        }
                    } else {
                        track.dip_angle(aim_angle);
                        track.motor();
                    }
                    break;
                }
                case ball_func_ab_cd_e:
                case ball_func_cd_ab_e: {
                    if (ball.position() - aim_position >= -aim_position_threshold &&
                        ball.position() - aim_position <= aim_position_threshold
                        && ball.v() < 30.f && ball.v() >= -30.f && ball.a() < 80.f && ball.a() >= -80.f) {
                        track.dip_angle(0.f);
                        track.motor();
                        is_running = false;
                        printf("hint\r\n");
                        osDelay(2700);
                        if (ball_func_ab_cd_ab) {
                        is_running = true;
                        if (ball_func == ball_func_ab_cd_e) {
                            aim_position = 425.f;
                            ball_func = ball_func_cd_ab_e;
                        } else {
                            aim_position = 125.f;
                            ball_func = ball_func_ab_cd_e;
                        }
                    }
                    } else {
                        track.height((ball.position() - 125.f) / 300.f * 20.f);
                        track.dip_angle(aim_angle);
                        track.motor();
                    }
                    
                    break;
                }
                default:
                    break;
            }
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
            if (ball_func == ball_func_follow_red_e || ball_func == ball_func_follow_red_2_e)
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

