cmake_minimum_required (VERSION 3.5)
project(cdh)

set(Keil_v5 D:/MDK5)
#set(Keil_v5  /home/chudonghao/Keil/Keil_v5)
add_definitions(USE_STDPERIPH_DRIVER ARM_MATH_CM3 STM32F10X_HD)
include_directories(
#        ${Keil_v5}/ARM/Pack/ARM/CMSIS/4.2.0/CMSIS/Include
        ${Keil_v5}/ARM/Pack/ARM/CMSIS/5.0.1/CMSIS/Include
        ${Keil_v5}/ARM/Pack/Keil/STM32F1xx_DFP/1.0.5/Device/StdPeriph_Driver/inc
#        ${Keil_v5}/ARM/Pack/Keil/STM32F1xx_DFP/2.2.0/Device/StdPeriph_Driver/inc
        ${Keil_v5}/ARM/Pack/Keil/STM32F1xx_DFP/1.0.5/Device/Include
#        ${Keil_v5}/ARM/Pack/Keil/STM32F1xx_DFP/2.2.0/Device/Include
        ${Keil_v5}/ARM/Pack/ARM/CMSIS/5.0.1/CMSIS/RTOS2/Include
        ${Keil_v5}/ARM/Pack/ARM/CMSIS/5.0.1/CMSIS/RTOS2/RTX/Include
        ${PROJECT_SOURCE_DIR}/RTE/Device/STM32F103RC
        ${PROJECT_SOURCE_DIR}/RTE
        ${Keil_v5}/ARM/ARMCC/include
)
add_executable(cdh
        main.cpp
        usart1.h
        usart1.cpp
        driver.h
        led.cpp
        led.h
        pwm1.cpp
        pwm1.h
        step_motor_couple.cpp
        step_motor_couple.h
        box.cpp
        box.h
        point_array.cpp
        point_array.h
        infra_red.cpp
        RTE/CMSIS/RTX_Config.c
        RTE/CMSIS/RTX_Config.h
        infra_red.h encoder.cpp encoder.h step_motor.cpp step_motor.h)