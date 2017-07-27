//
// Created by leu19 on 2017/7/28.
//
#include <cstdio>
#include <cstring>
#include "step_motor_couple.h"

using namespace cdh;
using namespace std;
using namespace glm;
extern "C" int main_task() {
    printf("inited.\r\n");
    printf("test step couple.\r\n");
    for (;;) {
        char ch[128];
        scanf("%s", ch);
        float x, y;
        if (strcmp(ch, "scs") == 0 || strcmp(ch, "set_current_steps") == 0) {
            scanf("%f%f", &x, &y);
            printf("set current steps:(x,y)=(%.3f,%.3f)\r\n", x, y);
            step_motor_couple_t::set_current_steps(ivec2(x,y));
        }else if(strcmp(ch, "sns") == 0 || strcmp(ch, "set_next_steps") == 0){
            scanf("%f%f", &x, &y);
            printf("set current steps:(x,y)=(%.3f,%.3f)\r\n", x, y);
            step_motor_couple_t::set_next_steps(ivec2(x,y));
            step_motor_couple_t::step();
        }
    }
}

