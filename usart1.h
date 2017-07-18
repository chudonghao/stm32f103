#ifndef CDH_USART1_H
#define CDH_USART1_H
#include <cstddef>
namespace cdh {
    class usart1_t {
    private:
        static bool inited;
    private:
        long int position;
    public:

        usart1_t *open();

        std::size_t read(unsigned char *ptr, std::size_t size, std::size_t count);

        std::size_t write(const unsigned char *ptr, std::size_t size, std::size_t count);

        int seek(long int offset);

        long tell();

        int close();

        bool have_data_to_read();
        void trim_buffer_head();
    };
}


#endif //CDH_DRIVER_H
