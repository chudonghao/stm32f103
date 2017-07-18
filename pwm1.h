//
// Created by leu19 on 2017/7/10.
//

#ifndef CDH_PWM1_H
#define CDH_PWM1_H

namespace cdh{
    class pwm1_t {
        static bool inited;
    public:
        pwm1_t*open();
        void start();
        void stop();
    };

}



#endif //CDH_PWM1_H
