cmake_minimum_required (VERSION 3.5)
project(cdh)

set(Keil_v5_dir1 D:/MDK5)
set(Keil_v5_dir2 /home/chudonghao/Keil/Keil_v5)
set(FreeRTOS_dir ${PROJECT_SOURCE_DIR}/third_party/FreeRTOSv9.0.0)

add_definitions(USE_STDPERIPH_DRIVER ARM_MATH_CM3 STM32F10X_HD)
include_directories(
        ${PROJECT_SOURCE_DIR}/RTE/Device/STM32F103RC
        ${PROJECT_SOURCE_DIR}/RTE
        ${PROJECT_SOURCE_DIR}

        ${Keil_v5_dir1}/ARM/Pack/ARM/CMSIS/4.2.0/CMSIS/Include
        ${Keil_v5_dir1}/ARM/Pack/ARM/CMSIS/5.0.1/CMSIS/Include
        ${Keil_v5_dir1}/ARM/Pack/Keil/STM32F1xx_DFP/1.0.5/Device/StdPeriph_Driver/inc
        ${Keil_v5_dir1}/ARM/Pack/Keil/STM32F1xx_DFP/2.2.0/Device/StdPeriph_Driver/inc
        ${Keil_v5_dir1}/ARM/Pack/Keil/STM32F1xx_DFP/1.0.5/Device/Include
        ${Keil_v5_dir1}/ARM/Pack/Keil/STM32F1xx_DFP/2.2.0/Device/Include
        ${Keil_v5_dir1}/ARM/Pack/ARM/CMSIS/5.0.1/CMSIS/RTOS2/Include
        ${Keil_v5_dir1}/ARM/Pack/ARM/CMSIS/5.0.1/CMSIS/RTOS2/RTX/Include
        ${Keil_v5_dir1}/ARM/ARMCC/include

        ${Keil_v5_dir2}/ARM/Pack/ARM/CMSIS/4.2.0/CMSIS/Include
        ${Keil_v5_dir2}/ARM/Pack/ARM/CMSIS/5.0.1/CMSIS/Include
        ${Keil_v5_dir2}/ARM/Pack/Keil/STM32F1xx_DFP/1.0.5/Device/StdPeriph_Driver/inc
        ${Keil_v5_dir2}/ARM/Pack/Keil/STM32F1xx_DFP/2.2.0/Device/StdPeriph_Driver/inc
        ${Keil_v5_dir2}/ARM/Pack/Keil/STM32F1xx_DFP/1.0.5/Device/Include
        ${Keil_v5_dir2}/ARM/Pack/Keil/STM32F1xx_DFP/2.2.0/Device/Include
        ${Keil_v5_dir2}/ARM/Pack/ARM/CMSIS/5.0.1/CMSIS/RTOS2/Include
        ${Keil_v5_dir2}/ARM/Pack/ARM/CMSIS/5.0.1/CMSIS/RTOS2/RTX/Include
        ${Keil_v5_dir2}/ARM/ARMCC/include

        ${FreeRTOS_dir}/FreeRTOS/Source/include
        ${PROJECT_SOURCE_DIR}/third_party/stm32f1_warship_v3/camera_test/HARDWARE/LCD
        ${PROJECT_SOURCE_DIR}/third_party/stm32f1_warship_v3/camera_test/HARDWARE/OV7670

)
add_executable(main.cpp usart1.cpp)
