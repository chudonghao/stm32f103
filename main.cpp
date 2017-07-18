#include "usart1.h"
#include "led.h"
#include "step_motor_couple.h"

#include <stm32f10x_conf.h>
#include <cmsis_os2.h>
#include <rtx_os.h>
#include <cstdio>
#include <cstring>
#include <rtx_evr.h>

using namespace std;
using namespace cdh;
usart1_t usart1;
led1_t led1;
step_motor_couple_t *step_motor_couple;
infra_red_t *infra_red;

int std::fputc(int c, FILE *f) {
    usart1.write((unsigned char *) &c, 1, 1);
    return c;
}

int std::fgetc(FILE *f) {
    unsigned char ch;
    for (;;) {
        if (usart1.read(&ch, 1, 1)) {
            return ch;
        }
    }
}

static void led1_func(void *) {
    for (int i = 0;; ++i) {
        if (i % 40 == 0)
            led1.on();
        if (i % 40 == 1)
            led1.off();
        if (i % 5 == 0)
            printf("P%d,%d\r\n", box_t::instance()->position.x, box_t::instance()->position.y);
        osDelay(100);
    }
}

static void step_motor_couple_func(void *) {
    for (;;) {
        step_motor_couple->step();
    }
};

static void main_func(void *) {
    for (int i = 0;; ++i) {
        char ch[31] = {0};
        int j = 0, k = 0, l = 0, m = 0;
        scanf("%s", ch);
        if (strcmp(ch, "move_box_to") == 0) {
            printf("move_box_to ");
            scanf("%d", &j);
            scanf("%d", &k);
            printf("%d %d\r\n", j, k);
            line_t line = line_t(box_t::instance()->position,vec2_t(j,k));
            vec2_t *next_point;
            while (next_point = line.next_point()){
                vec2_t next_steps = step_motor_couple->pre_map_position_to_steps(next_point->x, next_point->y);
                while (step_motor_couple->set_next_steps(next_steps) == -1) {
                    osThreadYield();
                }
            }
        } else if (strcmp(ch, "step") == 0) {
            printf("step ");
            scanf("%d", &j);
            scanf("%d", &k);
            printf("%d %d\r\n", j, k);
            step_motor_couple->step(j, k);
        } else if (strcmp(ch, "set_current_position") == 0) {
            printf("set_current_position ");
            scanf("%d", &j);
            scanf("%d", &k);
            printf("%d %d\r\n", j, k);
            step_motor_couple->reset_current_position(j, k);
        }
//        else if (strcmp(ch, "draw_line") == 0) {
//            printf("draw_line ");
//            vec2_t start;
//            vec2_t end;
//            scanf("%d", &j);
//            scanf("%d", &k);
//            scanf("%d", &l);
//            scanf("%d", &m);
//            printf("%d %d %d %d\r\n", j, k, l, m);
//            start.x = j;
//            start.y = k;
//            end.x = l, end.y = m;
//            line_t line(start, end);
//            step_motor_couple->move_box_by_point_array(&line);
//        }
        else if (strcmp(ch, "free_move") == 0) {
            heart_line_t heart_line = heart_line_t();
            vec2_t *next_point;
            while (next_point = heart_line.next_point()){
                vec2_t next_steps = step_motor_couple->pre_map_position_to_steps(next_point->x, next_point->y);
                while (step_motor_couple->set_next_steps(next_steps) == -1) {
                    osThreadYield();
                }
            }
        } else if (strcmp(ch, "draw_circle") == 0) {
            printf("draw_circle ");
            vec2_t center;
            scanf("%d", &j);
            scanf("%d", &k);
            scanf("%d", &l);
            printf("%d %d %d\r\n", j, k, l);
            center.x = j;
            center.y = k;
            circle_t circle(center, l);
            vec2_t *next_point = NULL;
            while (next_point = circle.next_point()) {
                vec2_t next_steps = step_motor_couple->pre_map_position_to_steps(next_point->x, next_point->y);
                while (step_motor_couple->set_next_steps(next_steps) == -1) {
                    osThreadYield();
                }
            }
//            step_motor_couple->move_box_by_point_array(&circle);

        } else if (strcmp(ch, "tracking") == 0) {
            printf("tracking \r\n");
            tracking_t tracking(vec2_t(box_t::instance()->position.x, box_t::instance()->position.y));
            vec2_t *next_point = NULL;
            while (next_point = tracking.next_point()) {
                vec2_t next_steps = step_motor_couple->pre_map_position_to_steps(next_point->x, next_point->y);
                while (step_motor_couple->set_next_steps(next_steps) == -1) {
                    osThreadYield();
                }
            }
        } else {
            printf("unknow func:%s\r\n", ch);
        }
        usart1.trim_buffer_head();
    }
}

int main() {
    SystemCoreClockUpdate();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    usart1.open();
    led1.open();
    led1.off();
    step_motor_couple = step_motor_couple_t::open();
    infra_red = infra_red_t::open();
    printf("inited\r\n");

    osKernelInitialize();
    const static osThreadAttr_t thread1_attr = {
            "", osThreadJoinable,
            0, 0,
            0, 512,
            osPriorityAboveNormal, 0
    };
    osThreadNew(led1_func, NULL, &thread1_attr);
    const static osThreadAttr_t main_attr = {
            "", osThreadJoinable,
            0, 0,
            0, 2048,
            osPriorityAboveNormal, 0
    };
    osThreadNew(main_func, NULL, &main_attr);
    const static osThreadAttr_t step_attr = {
            "", osThreadJoinable,
            0, 0,
            0, 512,
            osPriorityAboveNormal, 0
    };
    osThreadNew(step_motor_couple_func, NULL, &step_attr);
    osKernelStart();
}

