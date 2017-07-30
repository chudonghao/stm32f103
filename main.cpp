//
// Created by leu19 on 2017/7/28.
//
#include <cstdio>
#include <cstring>
#include "step_motor_couple.h"
#include "key_board.h"
#include "laser.h"
#include "usart3.h"
#include <main.h>
#include <gpio.h>
#include <cmsis_os.h>

using namespace cdh;
using namespace std;
using namespace glm;
static bool follow_green_laser_point = false;
extern "C" void key_board_task(const void *){
    static int old_key = -1;
    static int delay_count = 0;
    for (;;) {
        if(usart3_t::have_sentence()){
            static vec2 position;
            int count = sscanf(usart3_t::c_str(),"P%f,%f",&position.x,&position.y);
            if(count == 2){
                while(step_motor_couple_t::set_next_steps(step_motor_couple_t::map_position_to_steps(position)) == -1){}
            }
        }
        int key = key_board_t::scan();
        if(old_key == key && delay_count < 10){
            osDelay(30);
            ++delay_count;
            continue;
        }else if(old_key != key){
            old_key = key;
            delay_count = 0;
        }
        if(key == 0){
            step_motor_couple_t::set_current_steps(ivec2(0,-20));
        }else if(key == 1){
            ivec2 cur = step_motor_couple_t::current_steps();
            cur.y+=1;
            step_motor_couple_t::set_next_steps(cur);
        }else if(key == 2){
            follow_green_laser_point = !follow_green_laser_point;
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
    }
}
static float current_x = 0,current_y = 0;
static int new_position = 0;
static bool new_shoot = false;
extern "C" void laser_task(const void*){
    for(;;){
        if(step_motor_couple_t::status() == -1){
            laser_t::off();
            new_shoot = true;
            new_position = 0;
        }else{
            laser_t::on();
            if(new_shoot == true && new_position > 2){
                new_shoot = false;
                new_position = 0;
                static char ch[128];
                sprintf(ch,"P%.3f,%.3f\r\n", current_x, current_y);
                usart3_t::print(ch);
            }
        }
        step_motor_couple_t::step();
    }
}
extern "C" void main_task() {
    printf("inited.\r\n");
    printf("test step couple.\r\n");
    for (;;) {
        static char ch[128];
        scanf("%s", ch);
        float x, y;
        if (strcmp(ch, "scs") == 0 || strcmp(ch, "set_current_steps") == 0) {
            int res_number = scanf("%f%f", &x, &y);
            if(res_number != 2){
                printf("scanf error.");
            }
            if(step_motor_couple_t::status() == step_motor_couple_status_running_e){
                continue;
            }
            printf("set current steps:(x,y)=(%.3f,%.3f)\r\n", x, y);
            step_motor_couple_t::set_current_steps(ivec2(x,y));
        }else if(strcmp(ch, "sns") == 0 || strcmp(ch, "set_next_steps") == 0){
            int res_number = scanf("%f%f", &x, &y);
            if(res_number != 2){
                printf("scanf error.");
            }
            printf("set next steps:(x,y)=(%.3f,%.3f)\r\n", x, y);
            step_motor_couple_t::set_next_steps(ivec2(x,y));
        }else if(strcmp(ch, "scp") == 0 || strcmp(ch, "set_current_position") == 0){
            //TODO scp 已经不是原有的意思
            //这里不再修正坐标，修正暂时使用 scs
            int res_number = scanf("%f%f", &x, &y);
            if(res_number != 2){
                printf("scanf error.");
            }
            if(step_motor_couple_t::status() == step_motor_couple_status_running_e){
                continue;
            }
            ++new_position;
            current_x = x;
            current_y = y;
        }else if(strcmp(ch, "snp") == 0 || strcmp(ch, "set_next_position") == 0){
            //TODO snp 已经不是原有的意思
            //暂不使用此功能
            int res_number = scanf("%f%f", &x, &y);
            if(res_number != 2){
                printf("scanf error.");
            }
            if(follow_green_laser_point){
                ivec2 steps = step_motor_couple_t::current_steps();
                printf("set next position:(x,y)=(%.3f,%.3f),steps=(%d,%d)", x, y);
                steps = step_motor_couple_t::map_position_to_steps(vec2(x,y));
                printf(",next_steps=(%d,%d)\r\n",steps.x,steps.y);
                step_motor_couple_t::set_next_steps(steps);
            }
        }else{
            printf("%s %f %f\r\n",ch,x,y);
        }
    }
}

