//
// Created by leu19 on 2017/7/19.
//

#ifndef CDH_ENCODER_H
#define CDH_ENCODER_H
namespace cdh{

    class encoder_t {
        static encoder_t*encoder;
    public:
        static encoder_t*open();
        static int value();
        static int dir();
        static float angle();
        static void test();
        static void enable_soft_sampling();
    };

}


#endif //CDH_ENCODER_H
