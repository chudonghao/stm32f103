#ifndef CDH_USART1_H
#define CDH_USART1_H

#include "driver.h"

namespace cdh{
    class usart1_t :public driver_t{
    private:
        static bool inited;
    private:
        long int position;
    public:
        virtual ~usart1_t() {}

        virtual driver_t *open();

        virtual std::size_t read(void *ptr, std::size_t size, std::size_t count);

        virtual std::size_t write(const unsigned char *ptr, std::size_t size, std::size_t count);

        virtual int seek(long int offset);

        virtual long tell();

        virtual int close();
    };
}


#endif //CDH_DRIVER_H
