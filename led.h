//
// Created by chudonghao on 17-7-9.
//

#ifndef CDH_LED_H
#define CDH_LED_H

namespace cdh{
    class led1_t{
        static led1_t*led1;
        bool m_on;
        led1_t():m_on(false){}
    public:
        static led1_t * open();
        void on();
        void off();
        bool is_on();
        int close();
    };
}


#endif //CDH_LED_H
