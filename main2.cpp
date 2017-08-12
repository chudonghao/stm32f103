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
#include "led0.h"
#include "main2.h"
#include "main1.h"

using namespace cdh;
using namespace std;
using namespace glm;
namespace {
    enum aim_position_type_e {
        aim_position_type_stop_2_second_e,
        aim_position_type_enter_e,
        aim_position_type_pass_e,
        aim_position_type_none_e
    };
    const int BLOCK_SIZE = 20;
    flat_board_t flat_board;
    ball_t ball;
    vec2 ball_position_sampling;
    int ball_position_sampling_index;
    vec2 aim_position = vec2(0.f);
    aim_position_type_e aim_position_type;
    vec2 aim_position_stop_threshold = vec2(11.f);
    vec2 aim_position_enter_threshold = vec2(13.f);
    vec2 aim_position_pass_threshold = vec2(30.f);

//    float pid_kp_typical_vlaue = 1.f;
//    float pid_ki_typical_value = 0.01f;
//    float pid_kd_typical_value = 24.f;
    float pid_kp_typical_vlaue = 3.f;
    float pid_ki_typical_value = 0.003f;
    float pid_kd_typical_value = 60.f;
    float pid_ki_aim_close_typical_value = 0.05f;
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
        ball_func_a_b_c_d_e,
        ball_func_circle_e

    };
    ball_func_e ball_func;
    bool keep_running = false;
    bool force_keep_running = false;
    bool get_point_a_b_c_d = false;
    bool ball_func_follow_bright = false;
    int ball_func_a_b_c_d_state_index = 0;
    vec2 ball_func_a_b_c_d_state[7];
    int ball_func_circle_state = 0;
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

    bool common_stop_condition_satisfied() {
        if (abs(ball_position_sampling.x - aim_position.x) < aim_position_stop_threshold.x
            && abs(ball_position_sampling.y - aim_position.y) < aim_position_stop_threshold.y) {
            return true;
        }
        return false;
    }

    bool common_enter_condition_satisfied() {
        if (abs(ball_position_sampling.x - aim_position.x) < aim_position_enter_threshold.x
            && abs(ball_position_sampling.y - aim_position.y) < aim_position_enter_threshold.y) {
            return true;
        }
        return false;
    }

    bool common_pass_condition_satisfied() {
        if (abs(ball_position_sampling.x - aim_position.x) < aim_position_pass_threshold.x
            && abs(ball_position_sampling.y - aim_position.y) < aim_position_pass_threshold.y) {
            return true;
        }
        return false;
    }

    void refresh_pid_arg() {
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
        if (abs(diff_p.x) < 60.f) {
            arm_pid_instance1.Ki = pid_ki_aim_close_typical_value;
            arm_pid_instance1.Kd = pid_kd_aim_close_typical_value;
            arm_pid_init_f32(&arm_pid_instance1, 0);
        } else {
            arm_pid_instance1.Ki = pid_ki_typical_value;
            arm_pid_instance1.Kd = pid_kd_typical_value;
            arm_pid_init_f32(&arm_pid_instance1, 0);
        }
        if (abs(diff_p.y) < 60.f) {
            arm_pid_instance2.Ki = pid_ki_aim_close_typical_value;
            arm_pid_instance2.Kd = pid_kd_aim_close_typical_value;
            arm_pid_init_f32(&arm_pid_instance2, 0);
        } else {
            arm_pid_instance2.Ki = pid_ki_typical_value;
            arm_pid_instance2.Kd = pid_kd_typical_value;
            arm_pid_init_f32(&arm_pid_instance2, 0);
        }
    }

    void common_ball_move_func() {
        vec2 out_put = vec2(0.f, 0.f);
        //动态调节pid参数
        refresh_pid_arg();
        //进入pid调节
        {
            out_put.x = -arm_pid_f32(&arm_pid_instance1, aim_position.x - ball_position_sampling.x);
            out_put.y = arm_pid_f32(&arm_pid_instance2, aim_position.y - ball_position_sampling.y);
            //限幅
            float threshold = 90.f;
            if (abs(ball.v().x) < 20.f && abs(ball.v().y) < 20.f)
                threshold = 150.f;
            out_put.x = clamp(out_put.x, -threshold, threshold);
            out_put.y = clamp(out_put.y, -threshold, threshold);
        }
        //强制电机向指定步数运动
        step_motor_couple_t::set_next_steps(out_put, true);
        step_motor_couple_t::step();
        //printf("%.3f,%.3f;\r\n", ball_position_sampling.x, ball_position_sampling.y);
    }

    void ball_func_1_to_5_start() {
        ball_func = ball_func_1_5_e;
        aim_position = POINT_1;
        aim_position_type = aim_position_type_enter_e;
    }

    void ball_func_1_to_5_next_aim_position() {
        if (aim_position == POINT_1) {
            aim_position = POINT_5;
            aim_position_type = aim_position_type_stop_2_second_e;
        } else {
            aim_position_type = aim_position_type_none_e;
        }
    }

    void ball_func_to_2_start() {
        ball_func = ball_func_to_2_e;
        aim_position = POINT_2;
        aim_position_type = aim_position_type_stop_2_second_e;
    }

    void ball_func_to_2_next_aim_position() {
        aim_position_type = aim_position_type_none_e;
    }

    void ball_func_1_4_5_start() {
        ball_func = ball_func_1_4_5_e;
        aim_position = POINT_1;
        aim_position_type = aim_position_type_enter_e;
    }

    void ball_func_1_4_5_next_aim_position() {
        if (aim_position == POINT_1) {
            aim_position = POINT_4;
            aim_position_type = aim_position_type_stop_2_second_e;
        } else if (aim_position == POINT_4) {
            aim_position = POINT_5;
            aim_position_type = aim_position_type_stop_2_second_e;
        } else {
            aim_position_type = aim_position_type_none_e;
        }
    }

    void ball_func_1_9_start() {
        ball_func = ball_func_1_9_e;
        aim_position = POINT_1;
        aim_position_type = aim_position_type_enter_e;
    }

    void ball_func_1_9_next_aim_position() {
        if (aim_position == POINT_1) {
            aim_position = POINT_17;
            aim_position_type = aim_position_type_pass_e;
        } else if (aim_position == POINT_17) {
            aim_position = POINT_9;
            aim_position_type = aim_position_type_stop_2_second_e;
        } else {
            aim_position_type = aim_position_type_none_e;
        }
    }

    void ball_func_1_2_6_9_start() {
        ball_func = ball_func_1_2_6_9_e;
        aim_position = POINT_1;
        aim_position_type = aim_position_type_enter_e;
    }

    void ball_func_1_2_6_9_next_aim_position() {
        if (aim_position == POINT_1) {
            aim_position = POINT_2;
            aim_position_type = aim_position_type_enter_e;
        } else if (aim_position == POINT_2) {
            aim_position = POINT_6;
            aim_position_type = aim_position_type_enter_e;
        } else if (aim_position == POINT_6) {
            aim_position = POINT_9;
            aim_position_type = aim_position_type_stop_2_second_e;
        } else {
            aim_position_type = aim_position_type_none_e;
        }
    }

    vec2 two_point_interpolation(const vec2 &start, const vec2 &end) {
        vec2 vec_two_point = end - start;
        vec2 res = start + vec_two_point / 2.f;
        if (abs(vec_two_point.x) == 400.f && abs(vec_two_point.y) == 400) {//对角线
            if (vec_two_point.x > 0.f) {
                if (vec_two_point.y > 0.f) {
                    res += vec2(0, -100.f);
                } else {
                    res += vec2(-100.f, 0.f);
                }
            } else {
                if (vec_two_point.y > 0.f) {
                    res += vec2(100.f, 0.f);
                } else {
                    res += vec2(0.f, 100.f);
                }
            }
        } else if ((abs(vec_two_point.x) == 400.f && vec_two_point.y == 0.f)
                   || (abs(vec_two_point.y) == 400.f && vec_two_point.x == 0.f)) {//水平或垂直
            if (vec_two_point.x < 0.f) {
                if (start.y > 0.f)//最外侧
                    res += vec2(0.f, -100.f);
                else
                    res += vec2(0.f, 100.f);
            } else if (vec_two_point.x > 0.f) {
                if (start.y < 0.f)//最外侧
                    res += vec2(0.f, 100.f);
                else
                    res += vec2(0.f, -100.f);
            } else if (vec_two_point.y < 0.f) {
                if (start.x < 0.f)//最外侧
                    res += vec2(100.f, 0.f);
                else
                    res += vec2(-100.f, 0.f);
            } else {//y > 0.f
                if (start.x > 0.f)//最外侧
                    res += vec2(-100.f, 0.f);
                else
                    res += vec2(100.f, 0.f);
            }
        }
        return res;
    }

    void ball_func_a_b_c_d_start() {
        ball_func_a_b_c_d_state_index = 0;
        //todo 对abcd点插值
        ball_func_a_b_c_d_state[1] = two_point_interpolation(ball_func_a_b_c_d_state[0],
                                                             ball_func_a_b_c_d_state[2]);
        ball_func_a_b_c_d_state[3] = two_point_interpolation(ball_func_a_b_c_d_state[2],
                                                             ball_func_a_b_c_d_state[4]);
        ball_func_a_b_c_d_state[5] = two_point_interpolation(ball_func_a_b_c_d_state[4],
                                                             ball_func_a_b_c_d_state[6]);
        ball_func = ball_func_a_b_c_d_e;
        aim_position = ball_func_a_b_c_d_state[0];
        aim_position_type = aim_position_type_enter_e;
//        printf("ball_state:\r\n");
//        printf("%f,%f;\r\n", ball_func_a_b_c_d_state[0].x, ball_func_a_b_c_d_state[0].y);
//        printf("%f,%f;\r\n", ball_func_a_b_c_d_state[1].x, ball_func_a_b_c_d_state[1].y);
//        printf("%f,%f;\r\n", ball_func_a_b_c_d_state[2].x, ball_func_a_b_c_d_state[2].y);
//        printf("%f,%f;\r\n", ball_func_a_b_c_d_state[3].x, ball_func_a_b_c_d_state[3].y);
//        printf("%f,%f;\r\n", ball_func_a_b_c_d_state[4].x, ball_func_a_b_c_d_state[4].y);
//        printf("%f,%f;\r\n", ball_func_a_b_c_d_state[5].x, ball_func_a_b_c_d_state[5].y);
//        printf("%f,%f;\r\n", ball_func_a_b_c_d_state[6].x, ball_func_a_b_c_d_state[6].y);
    }

    void ball_func_a_b_c_d_next_aim_position() {
        if (ball_func_a_b_c_d_state_index == 6) {
            aim_position_type = aim_position_type_none_e;
        } else {
            ++ball_func_a_b_c_d_state_index;
            aim_position = ball_func_a_b_c_d_state[ball_func_a_b_c_d_state_index];
            if (ball_func_a_b_c_d_state_index == 2 || ball_func_a_b_c_d_state_index == 4) {
                aim_position_type = aim_position_type_enter_e;
            } else if (ball_func_a_b_c_d_state_index == 1 || ball_func_a_b_c_d_state_index == 3 ||
                       ball_func_a_b_c_d_state_index == 5) {
                aim_position_type = aim_position_type_pass_e;
            } else/* 6 */{
                aim_position_type = aim_position_type_stop_2_second_e;
            }
        }
    }

    void ball_func_circle_start() {
        ball_func = ball_func_circle_e;
        ball_func_circle_state = 0;
        aim_position = POINT_4;
        aim_position_type = aim_position_type_enter_e;
    }

    void ball_func_circle_next_aim_position() {
        if (aim_position == POINT_9) {//抵达终点
            aim_position_type = aim_position_type_none_e;
        } else if (aim_position == POINT_4) {//从起点出发
            arm_pid_reset_f32(&arm_pid_instance1);
            arm_pid_reset_f32(&arm_pid_instance2);
            aim_position = POINT_14;
            aim_position_type = aim_position_type_pass_e;
        } else if (aim_position == POINT_14) {//圆上第一个点
            arm_pid_reset_f32(&arm_pid_instance1);
            arm_pid_reset_f32(&arm_pid_instance2);
            if (ball_func_circle_state == 3) {//三圈完成
                ball_func_circle_state = 0;
                aim_position = POINT_9;
                aim_position_type = aim_position_type_stop_2_second_e;
            } else {//进入下一圈
                aim_position = POINT_20;
            }
        } else if (aim_position == POINT_20) {//圆上第二个点
            arm_pid_reset_f32(&arm_pid_instance1);
            arm_pid_reset_f32(&arm_pid_instance2);
            aim_position = POINT_11;
        } else if (aim_position == POINT_11) {//圆上第三个点
            arm_pid_reset_f32(&arm_pid_instance1);
            arm_pid_reset_f32(&arm_pid_instance2);
            aim_position = POINT_17;
        } else if (aim_position == POINT_17) {//圆上第四个点
            arm_pid_reset_f32(&arm_pid_instance1);
            arm_pid_reset_f32(&arm_pid_instance2);
            ++ball_func_circle_state;
            aim_position = POINT_14;
        }
    }

}


void pid_init_2() {}

void pid_loop_2() {
    static int last_condition_satisfied_time_ms = -1;
    static int last_ball_position_sampling_index = 0;
    if (last_ball_position_sampling_index == ball_position_sampling_index) {
        //todo
        bool condition_satisfied = false;
        if (aim_position_type == aim_position_type_stop_2_second_e) {
            if (common_stop_condition_satisfied()) {
                if (last_condition_satisfied_time_ms == -1) {
                    last_condition_satisfied_time_ms = osKernelSysTick();
                    printf("hint\r\n");
                } else {
                    if (osKernelSysTick() - last_condition_satisfied_time_ms > 1800) {
                        condition_satisfied = true;
                        flat_board.dip_angle(vec2(0.f));
                        flat_board.motor();
                        printf("stop\r\n");
                    }
                }
            } else {
                if (last_condition_satisfied_time_ms > 0) {
                    printf("stop\r\n");
                    last_condition_satisfied_time_ms = -1;
                }
            }
        } else if (aim_position_type == aim_position_type_enter_e) {
            if (common_enter_condition_satisfied()) {
                condition_satisfied = true;
            }
        } else if (aim_position_type == aim_position_type_pass_e) {
            if (common_pass_condition_satisfied()) {
                condition_satisfied = true;
            }
        }
        if (condition_satisfied) {
            switch (ball_func) {
                case ball_func_1_5_e:
                    ball_func_1_to_5_next_aim_position();
                    break;
                case ball_func_to_2_e:
                    ball_func_to_2_next_aim_position();
                    break;
                case ball_func_1_4_5_e:
                    ball_func_1_4_5_next_aim_position();
                    break;
                case ball_func_1_9_e:
                    ball_func_1_9_next_aim_position();
                    break;
                case ball_func_1_2_6_9_e:
                    ball_func_1_2_6_9_next_aim_position();
                    break;
                case ball_func_a_b_c_d_e:
                    ball_func_a_b_c_d_next_aim_position();
                    break;
                case ball_func_circle_e:
                    ball_func_circle_next_aim_position();
                    break;
            }
        }
    } else {
        refresh_ball_state();
        if (keep_running && aim_position_type != aim_position_type_none_e) {
            common_ball_move_func();
        } else if (keep_running && ball_func_follow_bright) {
            common_ball_move_func();
        } else if(force_keep_running){
            common_ball_move_func();
        }
        last_ball_position_sampling_index = ball_position_sampling_index;
    }
}

void key_board_init_2() {}

void key_board_loop_2() {
    static int old_key = -1;
    static int delay_count = 0;
    key_board_t::key_board_e key = key_board_t::scan();
    if (old_key == key && delay_count < 10) {
        ++delay_count;
        if (delay_count > 0) {
            osDelay(30);
            return;
        }
    } else if (old_key != key) {
        old_key = key;
        delay_count = 0;
    }
    if (get_point_a_b_c_d) {
        static vec2 cur_point = POINT_1;
        bool add_point = false;
        switch (key) {
            case key_board_t::key_board_0_e:
                get_point_a_b_c_d = false;
                break;
            case key_board_t::key_board_1_e:
                cur_point = POINT_1;
                add_point = true;
                break;
            case key_board_t::key_board_2_e:
                cur_point = POINT_2;
                add_point = true;
                break;
            case key_board_t::key_board_3_e:
                cur_point = POINT_3;
                add_point = true;
                break;
            case key_board_t::key_board_4_e:
                cur_point = POINT_4;
                add_point = true;
                break;
            case key_board_t::key_board_5_e:
                cur_point = POINT_5;
                add_point = true;
                break;
            case key_board_t::key_board_6_e:
                cur_point = POINT_6;
                add_point = true;
                break;
            case key_board_t::key_board_7_e:
                cur_point = POINT_7;
                add_point = true;
                break;
            case key_board_t::key_board_8_e:
                cur_point = POINT_8;
                add_point = true;
                break;
            case key_board_t::key_board_9_e:
                cur_point = POINT_9;
                add_point = true;
                break;
            case key_board_t::key_board_a_e:
                break;
            case key_board_t::key_board_b_e:
                break;
            case key_board_t::key_board_c_e:
                break;
            case key_board_t::key_board_d_e:
                ball_func_a_b_c_d_start();
                get_point_a_b_c_d = false;
                break;
            case key_board_t::key_board_e_e:
                break;
            case key_board_t::key_board_f_e:
                break;
            default:
                break;
        }
        if (add_point) {
            ball_func_a_b_c_d_state[0] = ball_func_a_b_c_d_state[2];
            ball_func_a_b_c_d_state[2] = ball_func_a_b_c_d_state[4];
            ball_func_a_b_c_d_state[4] = ball_func_a_b_c_d_state[6];
            ball_func_a_b_c_d_state[6] = cur_point;
        }
    } else
        switch (key) {
            case key_board_t::key_board_0_e:
                get_point_a_b_c_d = false;
                aim_position_type = aim_position_type_none_e;
                aim_position = ball_position_sampling;
                arm_pid_reset_f32(&arm_pid_instance1);
                arm_pid_reset_f32(&arm_pid_instance2);
                flat_board.set_base();
                break;
            case key_board_t::key_board_none_e:
                break;
            case key_board_t::key_board_1_e:
                ivec2 cur = step_motor_couple_t::current_steps();
                cur.y += 1;
                step_motor_couple_t::set_next_steps(cur);
                step_motor_couple_t::step();
                break;
            case key_board_t::key_board_2_e: {
                ivec2 cur = step_motor_couple_t::current_steps();
                cur.x += 1;
                step_motor_couple_t::set_next_steps(cur);
                step_motor_couple_t::step();
                break;
            }
            case key_board_t::key_board_3_e:
                if (code_type_1) {
                    main_init_2();
                    pid_init_2();
                    key_board_init_2();
                    code_type_1 = false;
                } else {
                    main_init_1();
                    pid_init_1();
                    key_board_init_1();
                    code_type_1 = true;
                }
                break;
            case key_board_t::key_board_4_e:
                keep_running = !keep_running;
                break;
            case key_board_t::key_board_5_e: {
                ivec2 cur = step_motor_couple_t::current_steps();
                cur.y -= 1;
                step_motor_couple_t::set_next_steps(cur);
                step_motor_couple_t::step();
                break;
            }
            case key_board_t::key_board_6_e: {
                ivec2 cur = step_motor_couple_t::current_steps();
                cur.x -= 1;
                step_motor_couple_t::set_next_steps(cur);
                step_motor_couple_t::step();
                break;
            }
            case key_board_t::key_board_7_e:
                force_keep_running = !force_keep_running;
                break;
            case key_board_t::key_board_8_e:
                ball_func_to_2_start();
                break;
            case key_board_t::key_board_9_e:
                ball_func_1_to_5_start();
                break;
            case key_board_t::key_board_a_e:
                ball_func_1_4_5_start();
                break;
            case key_board_t::key_board_b_e:
                ball_func_1_9_start();
                break;
            case key_board_t::key_board_c_e:
                ball_func_1_2_6_9_start();
                break;
            case key_board_t::key_board_d_e:
                if (get_point_a_b_c_d == false) {
                    get_point_a_b_c_d = true;
                }
                break;
            case key_board_t::key_board_e_e:
                ball_func_circle_start();
                break;
            case key_board_t::key_board_f_e:
                if (ball_func_follow_bright) {
                    ball_func_follow_bright = false;
                    aim_position_type = aim_position_type_none_e;
                } else {
                    ball_func_follow_bright = true;
                    aim_position_type = aim_position_type_enter_e;
                }
                break;
            default:
                break;
        }
    if (key != key_board_t::key_board_none_e) {
        led0_t::on();
        osDelay(10);
        led0_t::off();
    } else {
        osDelay(30);
    }
}

void main_init_2() {
    arm_pid_instance1.Kp = pid_kp_typical_vlaue;
    arm_pid_instance1.Ki = pid_ki_typical_value;
    arm_pid_instance1.Kd = pid_kd_typical_value;
    arm_pid_instance2.Kp = arm_pid_instance1.Kp;
    arm_pid_instance2.Ki = arm_pid_instance1.Ki;
    arm_pid_instance2.Kd = arm_pid_instance1.Kd;
    arm_pid_init_f32(&arm_pid_instance1, 1);
    arm_pid_init_f32(&arm_pid_instance2, 1);
    led0_t::off();
    osDelay(100);
    led0_t::on();
    osDelay(100);
    led0_t::off();
    osDelay(100);
    led0_t::on();
    osDelay(100);
    led0_t::off();
}

void main_loop_2() {
    static char ch[128];
    if (!uart1_have_data_to_read()) {
        osThreadYield();
    }
    scanf("%s", ch);
    if (strcmp(ch, "ball") == 0) {
        vec2 ball_position_sampling_tmp;
        int scanf_res = scanf("%f%f", &ball_position_sampling_tmp.x, &ball_position_sampling_tmp.y);
        if (scanf_res != 2) {
            return;
        }
        if (ball_position_sampling_tmp.x >= 150.f && ball_position_sampling_tmp.y < -150.f) {
            ball_position_sampling_tmp.x += 5.f;
            ball_position_sampling_tmp.y -= 5.f;
        }
        ball_position_sampling = ball_position_sampling_tmp;
        ++ball_position_sampling_index;
    } else if (strcmp(ch, "test") == 0) {
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
    } else if (strcmp(ch, "bright") == 0) {
        vec2 tmp;
        int scanf_res = scanf("%f%f", &tmp.x, &tmp.y);
        if (scanf_res == 2 && ball_func_follow_bright) {
            aim_position = tmp;
        }
        //printf("unknown func:%s\r\n", ch);
    }
}

