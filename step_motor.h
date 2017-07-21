//
// Created by leu19 on 2017/7/19.
//

#ifndef CDH_STEP_MOTOR_H
#define CDH_STEP_MOTOR_H

namespace cdh{
    class step_motor_t {
    public:
        static step_motor_t*open();
        static void set_dir(int dir);
        static void set_next_step(int next_step);
        static int map_angle_to_step(float angle);
    };
}



#endif //CDH_STEP_MOTOR_H
