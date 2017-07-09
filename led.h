//
// Created by chudonghao on 17-7-9.
//

#ifndef CDH_LED_H
#define CDH_LED_H

#include "driver.h"

namespace cdh{
    class led1_t :public driver_t{
        static bool inited;
    public:
        virtual ~led1_t(){}

        virtual driver_t *open();

        virtual std::size_t read(unsigned char *ptr, std::size_t size, std::size_t count);

        virtual std::size_t write(const unsigned char *ptr, std::size_t size, std::size_t count);

        virtual int seek(long int offset);

        virtual long tell();

        virtual int close();
    };
}


#endif //CDH_LED_H
