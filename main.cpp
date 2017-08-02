//
// Created by leu19 on 2017/7/28.
//
#include <cstdio>
#include <cstring>
#include "step_motor_couple.h"
#include "key_board.h"
#include "usart3.h"
#include "undecided.h"
#include <main.h>
#include <gpio.h>
#include <cmsis_os.h>

using namespace cdh;
using namespace std;
using namespace glm;
namespace {
    track_t track;
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
            track.set_base();
        }else if(key == 1){
            ivec2 cur = step_motor_couple_t::current_steps();
            cur.y+=1;
            step_motor_couple_t::set_next_steps(cur);
        }else if(key == 4){
            ivec2 cur = step_motor_couple_t::current_steps();
            cur.x-=1;
            step_motor_couple_t::set_next_steps(cur);
        }else if(key == 6){
            ivec2 cur = step_motor_couple_t::current_steps();
            cur.x+=1;
            step_motor_couple_t::set_next_steps(cur);
        }else if(key == 5){
            ivec2 cur = step_motor_couple_t::current_steps();
            cur.y-=1;
            step_motor_couple_t::set_next_steps(cur);
        }
        osDelay(30);
    }
}

extern "C" void main_task() {

    printf("inited.\r\n");
    for (;;) {
        static char ch[128];
        scanf("%s", ch);
        if (strcmp(ch, "data") == 0) {
            float ms, y_left,y_right,ball_position;
            scanf("%f%f%f%f",&ms, &y_left,&y_right,&ball_position);
            printf("get data:%f %f %f %f\r\n", ms, y_left,y_right,ball_position);
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

