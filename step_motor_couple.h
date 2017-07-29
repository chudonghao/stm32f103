//
// Created by leu19 on 2017/7/28.
//

#ifndef CDH_STEP_MOTOR_COUPLE_H
#define CDH_STEP_MOTOR_COUPLE_H
#include <fix_glm.h>
#include <glm/glm.hpp>
namespace cdh{
    enum step_motor_couple_status_e{
        step_motor_couple_status_running_e = -1,
        step_motor_couple_status_stopped_e = 0
    };
    class step_motor_couple_t {
    public:
        static void test();
        static int set_next_steps(const glm::ivec2 &);
        static void step();
        static void set_current_steps(const glm::ivec2 &);
        static step_motor_couple_status_e status();
        static glm::ivec2 map_position_to_steps(const glm::vec2&);
        static glm::ivec2 current_steps();
    };
}



#endif //CDH_STEP_MOTOR_COUPLE_H
