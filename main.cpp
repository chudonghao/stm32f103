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
    bool is_running = false;
    bool get_point_a_b_c_d = false;
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

    void ball_func_1_9() {
        if (abs(ball_position_sampling.x - aim_position.x) < aim_position_threshold.x
            && abs(ball_position_sampling.y - aim_position.y) < aim_position_threshold.y) {
            if (aim_position == POINT_17) {//抵达中间点
                aim_position = POINT_9;
                aim_position_threshold = vec2(10.f);
            } else if (abs(ball.v().x) < 20.f && abs(ball.v().y) < 20.f) {//抵达9
                flat_board.dip_angle(vec2(0.f));
                flat_board.motor();
                is_running = false;
                return;
            }
        }
        common_ball_move_func();
    }

    void ball_func_1_2_6_9() {
        if (abs(ball_position_sampling.x - aim_position.x) < aim_position_threshold.x
            && abs(ball_position_sampling.y - aim_position.y) < aim_position_threshold.y) {
            if (aim_position == POINT_2) {//抵达中间点
                aim_position = POINT_6;
                aim_position_threshold = vec2(10.f);
            } else if (aim_position == POINT_6) {
                aim_position = POINT_9;
                aim_position_threshold = vec2(10.f);
            } else if (abs(ball.v().x) < 20.f && abs(ball.v().y) < 20.f) {//抵达9
                flat_board.dip_angle(vec2(0.f));
                flat_board.motor();
                is_running = false;
                return;
            }
        }
        common_ball_move_func();
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

    void refresh_ball_func_a_b_c_d_state() {
        ball_func_a_b_c_d_state_index = 0;
        //todo 对abcd点插值
        ball_func_a_b_c_d_state[1] = ball_func_a_b_c_d_state[2];
        ball_func_a_b_c_d_state[3] = ball_func_a_b_c_d_state[4];
        ball_func_a_b_c_d_state[5] = ball_func_a_b_c_d_state[6];
        ball_func_a_b_c_d_state[1] = two_point_interpolation(ball_func_a_b_c_d_state[0], ball_func_a_b_c_d_state[1]);
        ball_func_a_b_c_d_state[3] = two_point_interpolation(ball_func_a_b_c_d_state[2], ball_func_a_b_c_d_state[4]);
        ball_func_a_b_c_d_state[5] = two_point_interpolation(ball_func_a_b_c_d_state[4], ball_func_a_b_c_d_state[6]);

        printf("ball_state:\r\n");
        printf("%f,%f;\r\n", ball_func_a_b_c_d_state[0].x, ball_func_a_b_c_d_state[0].y);
        printf("%f,%f;\r\n", ball_func_a_b_c_d_state[1].x, ball_func_a_b_c_d_state[1].y);
        printf("%f,%f;\r\n", ball_func_a_b_c_d_state[2].x, ball_func_a_b_c_d_state[2].y);
        printf("%f,%f;\r\n", ball_func_a_b_c_d_state[3].x, ball_func_a_b_c_d_state[3].y);
        printf("%f,%f;\r\n", ball_func_a_b_c_d_state[4].x, ball_func_a_b_c_d_state[4].y);
        printf("%f,%f;\r\n", ball_func_a_b_c_d_state[5].x, ball_func_a_b_c_d_state[5].y);
        printf("%f,%f;\r\n", ball_func_a_b_c_d_state[6].x, ball_func_a_b_c_d_state[6].y);
    }

    void ball_func_a_b_c_d() {
        if (abs(ball_position_sampling.x - aim_position.x) < aim_position_threshold.x
            && abs(ball_position_sampling.y - aim_position.y) < aim_position_threshold.y) {
            if (ball_func_a_b_c_d_state_index == 6) {
                if (abs(ball.v().x) < 20.f && abs(ball.v().y) < 20.f) {//抵达终点
                    flat_board.dip_angle(vec2(0.f));
                    flat_board.motor();
                    is_running = false;
                    return;
                }
            } else {
                arm_pid_reset_f32(&arm_pid_instance1);
                arm_pid_reset_f32(&arm_pid_instance2);
                if (ball_func_a_b_c_d_state_index == 0
                    || ball_func_a_b_c_d_state_index == 2
                    || ball_func_a_b_c_d_state_index == 4
                        ) {
                    aim_position_threshold = vec2(30.f);
                } else {
                    aim_position_threshold = vec2(10.f);
                }
                ++ball_func_a_b_c_d_state_index;
                aim_position = ball_func_a_b_c_d_state[ball_func_a_b_c_d_state_index];
            }
        }
        common_ball_move_func();
    }

    void ball_func_circle() {
        if (abs(ball_position_sampling.x - aim_position.x) < aim_position_threshold.x
            && abs(ball_position_sampling.y - aim_position.y) < aim_position_threshold.y) {
            if (aim_position == POINT_9) {//抵达终点
                if (abs(ball.v().x) < 20.f && abs(ball.v().y) < 20.f) {
                    flat_board.dip_angle(vec2(0.f));
                    flat_board.motor();
                    is_running = false;
                    return;
                }
            } else if (aim_position == POINT_4) {//从起点出发
                aim_position = POINT_14;
                aim_position_threshold = vec2(30.f);
            } else if (aim_position == POINT_14) {//圆上第一个点
                aim_position = POINT_20;
            } else if (aim_position == POINT_20) {//圆上第二个点
                aim_position = POINT_11;
            } else if (aim_position == POINT_11) {//圆上第三个点
                aim_position = POINT_17;
            } else if (aim_position == POINT_17) {//圆上第四个点
                ++ball_func_circle_state;
                if (ball_func_circle_state == 3) {//三圈完成
                    ball_func_circle_state = 0;
                    aim_position = POINT_9;
                    aim_position_threshold = vec2(10.f);
                } else {
                    aim_position = POINT_14;//进入下一圈
                }
            }
        }
        common_ball_move_func();
    }

}
extern "C" void key_board_task(const void *) {
    static int old_key = -1;
    static int delay_count = 0;
    for (;;) {
        key_board_t::key_board_e key = key_board_t::scan();
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
                    ball_func = ball_func_a_b_c_d_e;
                    refresh_ball_func_a_b_c_d_state();
                    aim_position = ball_func_a_b_c_d_state[0];
                    aim_position_threshold = vec2(10.f);
                    arm_pid_reset_f32(&arm_pid_instance1);
                    arm_pid_reset_f32(&arm_pid_instance2);
                    is_running = true;
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
                    is_running = false;
                    get_point_a_b_c_d = false;
                    aim_position_threshold = vec2(10.f);
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
                    break;
                case key_board_t::key_board_4_e:
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
                    break;
                case key_board_t::key_board_8_e:
                    ball_func = ball_func_to_2_e;
                    aim_position = POINT_2;
                    aim_position_threshold = vec2(10.f);
                    is_running = true;
                    break;
                case key_board_t::key_board_9_e:
                    ball_func = ball_func_1_5_e;
                    aim_position = POINT_5;
                    aim_position_threshold = vec2(10.f);
                    is_running = true;
                    break;
                case key_board_t::key_board_a_e:
                    ball_func = ball_func_1_4_5_e;
                    aim_position = POINT_4;
                    aim_position_threshold = vec2(10.f);
                    is_running = true;
                    break;
                case key_board_t::key_board_b_e:
                    ball_func = ball_func_1_9_e;
                    aim_position = POINT_17;
                    aim_position_threshold = vec2(30.f);
                    is_running = true;
                    break;
                case key_board_t::key_board_c_e:
                    ball_func = ball_func_1_2_6_9_e;
                    aim_position = POINT_2;
                    aim_position_threshold = vec2(10.f);
                    is_running = true;
                    break;
                case key_board_t::key_board_d_e:
                    if (get_point_a_b_c_d == false) {
                        get_point_a_b_c_d = true;
                    }
                    break;
                case key_board_t::key_board_e_e:
                    ball_func = ball_func_circle_e;
                    ball_func_circle_state = 0;
                    aim_position = POINT_4;
                    aim_position_threshold = vec2(10.f);
                    is_running = true;
                    break;
                case key_board_t::key_board_f_e:
                    break;
                default:
                    break;
            }
        osDelay(10);
    }
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
                    case ball_func_a_b_c_d_e:
                        ball_func_a_b_c_d();
                        break;
                    case ball_func_circle_e:
                        ball_func_circle();
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

