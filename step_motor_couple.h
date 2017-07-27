//
// Created by leu19 on 2017/7/11.
//

#ifndef CDH_STEP_MOTOR_COUPLE_H
#define CDH_STEP_MOTOR_COUPLE_H

#include "fix_glm.h"
#include <glm/glm.hpp>
#include <cmath>

namespace cdh {
    typedef struct {
        int int1;
        int int2;
    } double_int_t;

    class step_motor_couple_t {
        step_motor_couple_t();
    public:
        static step_motor_couple_t *step_motor_couple;
        static step_motor_couple_t *init();

        static glm::ivec2 pre_map_position_to_steps(int x_mm, int y_mm);
        //step func
        static void step();
        //pre step
        static void step(int motor1_steps, int motor2_steps);
        static int set_next_steps(const glm::ivec2&next_steps);
        //reset position
        static void reset_current_position(int x_mm, int y_mm);
        static void test();
    };
}


#endif //CDH_STEP_MOTOR_COUPLE_H
