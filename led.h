//
// Created by chudonghao on 17-7-9.
//

#ifndef CDH_LED_H
#define CDH_LED_H

namespace cdh{
    class led1_t{
        static bool inited;
    public:
        led1_t * open();
        void on();
        void off();
        int close();
    };
}


#endif //CDH_LED_H
