//
// Created by leu19 on 2017/7/28.
//

#include <cstdio>
#include <usart.h>

#ifdef __GNUC__/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE extern "C" int __io_putchar(int ch)
#define GETCHAR_PROTOTYPE extern "C" int __io_getchar(FILE *f)
#else
#define PUTCHAR_PROTOTYPE int std::fputc(int ch, std::FILE *f)
#define GETCHAR_PROTOTYPE int std::fgetc(std::FILE *f)
#endif /* __GNUC__ */

    PUTCHAR_PROTOTYPE {
        /* Place your implementation of fputc here */
        /* e.g. write a character to the USART1 and Loop until the end of transmission */
        HAL_UART_Transmit(&huart1, (unsigned char * ) & ch, 1, 0xFFFF);

        return ch;
    }

    GETCHAR_PROTOTYPE {
        unsigned char ch = 0;
        HAL_UART_Receive(&huart1, &ch, 1, 0xffff);
        return ch;
    }
