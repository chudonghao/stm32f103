cmake_minimum_required(VERSION 3.5)
project(cdh)

include_directories(./Inc
        ./Drivers/CMSIS/Device/ST/STM32F1xx/Include
        ./Drivers/STM32F1xx_HAL_Driver/Inc
        ./Drivers/CMSIS/Include
        ./Middlewares/Third_Party/FreeRTOS/Source/include
        ./Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS/
        ./third_party/glm
        )
add_definitions(cdh USE_HAL_DRIVER STM32F103xE)
add_executable(cdh step_motor_couple.cpp step_motor_couple.h uart_int.c ./third_party/glm main.cpp key_board.cpp key_board.h laser.h usart3.cpp usart3.h undecided.cpp undecided.h led0.cpp led0.h main2.cpp main.h main2.h)
