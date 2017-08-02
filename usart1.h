
#include <cstddef>
namespace cdh{
    
    class /*!!!*/usart1_t{
    public:
        static void init(int baud_rate = 115200);
        static std::size_t read(unsigned char *ptr, std::size_t count);
        static std::size_t write(const unsigned char *ptr, std::size_t count);
        static bool have_data_to_read();
        static void trim_buffer_head();
    };
    
}
