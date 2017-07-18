//
// Created by leu19 on 2017/7/13.
//

#ifndef CDH_INFRA_RED_H
#define CDH_INFRA_RED_H

namespace cdh{
    class infra_red_t {
        static infra_red_t* infra_red;
        infra_red_t();
    public:
        static infra_red_t* open();
        unsigned char data();
        void test();
    };
}



#endif //CDH_INFRA_RED_H
