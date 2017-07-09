cmake_minimum_required (VERSION 3.5)
project(keil)

set(Keil_v5 /home/chudonghao/Keil/Keil_v5)

include_directories(
        ${Keil_v5}/ARM/Pack/ARM/CMSIS/5.0.1/CMSIS/Include
        ${Keil_v5}/ARM/Pack/Keil/STM32F1xx_DFP/2.2.0/Device/StdPeriph_Driver/inc
        ${Keil_v5}/ARM/Pack/Keil/STM32F1xx_DFP/2.2.0/Device/Include
        ${PROJECT_SOURCE_DIR}/RTE/Device/STM32F103RC
        ${PROJECT_SOURCE_DIR}/RTE
        ${Keil_v5}/ARM/ARMCC/include)
add_executable(keil main.cpp usart1.h usart1.cpp driver.h led.cpp led.h)