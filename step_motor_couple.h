//
// Created by leu19 on 2017/7/11.
//

#ifndef CDH_STEP_MOTOR_COUPLE_H
#define CDH_STEP_MOTOR_COUPLE_H

#include "box.h"
#include "point_array.h"
#include <cmath>
#include <rtx_evr.h>

namespace cdh {
    typedef struct {
        int int1;
        int int2;
    } double_int_t;

    class step_motor_couple_t {
    public:
        static step_motor_couple_t *step_motor_couple;
        box_t *box;
        vec2_t steps;
        vec2_t point_e;
        vec2_t point_f;
        vec2_t next_steps;
        osMutexId_t steps_mutex;
        step_motor_couple_t();

        static step_motor_couple_t *open();

        vec2_t pre_map_position_to_steps(int x_mm, int y_mm);
        //step func
        void step();
        //pre step
        void step(int motor1_steps, int motor2_steps);
        int set_next_steps(const vec2_t&next_steps);
        //reset position
        void reset_current_position(int x_mm, int y_mm);

        void fix_e(const double_int_t &p1_x_y, const double_int_t &p2_x_y, const double_int_t &l);

        void fix_f(const double_int_t &p1_x_y, const double_int_t &p2_x_y, const double_int_t &l);
    };
}


#endif //CDH_STEP_MOTOR_COUPLE_H
