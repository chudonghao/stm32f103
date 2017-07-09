#ifndef CDH_DRIVER_H
#define CDH_DRIVER_H

#include <cstddef>
#include <cstdio>
namespace cdh{
    class driver_t {
    protected:
        virtual ~driver_t() {}
    public:
        /// open device
        /// \return driver ptr on success
        /// \return NULL on failed
        virtual driver_t* open() = 0;
        /// read data from device
        /// \param ptr
        /// \param size
        /// \param count
        /// \return  number of data successfully read
        virtual std::size_t read(unsigned char *ptr, std::size_t size, std::size_t count) =0;
        /// write data to device
        /// \param ptr
        /// \param size
        /// \param count
        /// \return number of data successfully write
        virtual std::size_t write(const unsigned char *ptr, std::size_t size, std::size_t count) = 0;
        /// reset position
        /// \param offset
        /// \return 0 on success
        virtual int seek(long int offset) = 0;
        /// get position
        /// \return position
        virtual long int tell() = 0;
        /// close device
        /// \return 0 on success
        /// \return EOF on failed
        virtual int close() = 0;
    };
}


#endif //CDH_DRIVER_H
