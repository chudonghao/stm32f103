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
using namespace cdh;
using namespace std;
using namespace glm;
namespace {
    track_t track;
    ball_t ball;
    bool start = false;
    float aim_position = 275.f;
    arm_pid_instance_f32 arm_pid_instance;
}
extern "C" void key_board_task(const void *){
    static int old_key = -1;
    static int delay_count = 0;
    for (;;) {
        int key = key_board_t::scan();
        if(old_key == key && delay_count < 10){
            ++delay_count;
            if(delay_count > 0){
                osDelay(30);
                continue;
            }
        }else if(old_key != key){
            old_key = key;
            delay_count = 0;
        }
        if(key == 0){
            arm_pid_reset_f32(&arm_pid_instance);
            track.set_base();
            start = !start;
        }else if(key == 2){
            ivec2 cur = step_motor_couple_t::current_steps();
            cur.y+=1;
            step_motor_couple_t::set_next_steps(cur);
        }else if(key == 5){
            ivec2 cur = step_motor_couple_t::current_steps();
            cur.x-=1;
            step_motor_couple_t::set_next_steps(cur);
        }else if(key == 1){
            ivec2 cur = step_motor_couple_t::current_steps();
            cur.x+=1;
            step_motor_couple_t::set_next_steps(cur);
        }else if(key == 6){
            ivec2 cur = step_motor_couple_t::current_steps();
            cur.y-=1;
            step_motor_couple_t::set_next_steps(cur);
        }
        osDelay(10);
    }
}

extern "C" void pid_task(const void*){
    arm_pid_instance.Kp = 0.03f / 50.f;
    arm_pid_instance.Ki = 0.f;
    arm_pid_instance.Kd = 0.f;
    arm_pid_init_f32(&arm_pid_instance,1);
    for(;;){
        if(start){
            float diff_position = aim_position - ball.position();
            float aim_v;
            float aim_angle;
            if(diff_position >= 50.f){
                 aim_v = 50.f;
            }else if(diff_position < -50.f){
                 aim_v = -50.f;
            }
            float input = aim_v - ball.v();
            aim_angle = - arm_pid_f32(&arm_pid_instance,input);
            printf("aim_v=%f,ball.p=%f,aim_angle=%f\r\n",aim_v,ball.position(),aim_angle);
            
            if(diff_position >= -15 && diff_position <=15 && aim_angle < 0.02f && aim_angle >=-0.02f){
                track.dip_angle(0.f);
            }else{
                track.dip_angle(aim_angle);
            }
            track.motor_dip_angle();
            osDelay(100);
        }
    }
}
extern "C" void main_task(const void*) {
    printf("inited.\r\n");
    for (;;) {
        static char ch[128];
        scanf("%s", ch);
        if (strcmp(ch, "data") == 0) {
            static float last_ms = 0,last_ball_position = 0,last_v = 0;
            float ms, y_left,y_right,ball_position;
            scanf("%f%f%f%f",&ms, &y_left,&y_right,&ball_position);
            printf("get data:%f %f %f %f\r\n", ms, y_left,y_right,ball_position);
            if(last_ms != 0 && ms != last_ms){
                float s = (ms - last_ms) / 1000.f;
                float v = (ball_position - last_ball_position) / s;
                float a = (v - last_v) / s;//TODO 粗略用ms-last_ms代替加速时间
                ball.position(ball_position);
                ball.v(v);
                ball.a(a);
                last_v = v;
            }
            //printf("ball=(.position=%f,.v=%f,.a=%f)\r\n", ball.position(),ball.v(),ball.a());

            last_ms = ms;
            last_ball_position = ball_position;
        }else if(strcmp(ch, "test") == 0){
            float dip_angle,height;
            scanf("%f%f",&dip_angle, &height);
            printf("test:%f %f\r\n", dip_angle,height);
            track.dip_angle(dip_angle);
            track.height(height);
            while(track.motor_height() == -1){
                step_motor_couple_t::step();
            }
            while(track.motor_dip_angle() == -1){
                step_motor_couple_t::step();
            }
        }else if(strcmp(ch, "test2") == 0){
            float height_base;
            scanf("%f",&height_base);
            printf("test2:%f\r\n", height_base);
            track.height_base(height_base);
        }else{
            printf("unknown func:%s\r\n",ch);
        }
    }
}

