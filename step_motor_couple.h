//
// Created by leu19 on 2017/7/28.
//

#ifndef CDH_STEP_MOTOR_COUPLE_H
#define CDH_STEP_MOTOR_COUPLE_H
#include <fix_glm.h>
#include <glm/glm.hpp>
namespace cdh{
    class step_motor_couple_t {
    public:
        static void test();
        static int set_next_steps(glm::ivec2 next_steps);
        static void step();
        static void set_current_steps(glm::ivec2 current_steps);
        static int status();
        static glm::ivec2 map_position_to_steps(const glm::vec2);
    };
}



#endif //CDH_STEP_MOTOR_COUPLE_H
