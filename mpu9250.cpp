//
// Created by leu19 on 2017/7/20.
//

#include <stm32f10x_conf.h>
#include <cstdio>
#include <cstring>
#include "mpu9250.h"
#include "usart1.h"

using namespace std;
namespace {
    typedef struct {
        //0x55 0x50 YY MM DD hh mm ss msL msH SUM
        unsigned char YY;
        unsigned char MM;
        unsigned char DD;
        unsigned char hh;
        unsigned char mm;
        unsigned char ss;
        unsigned short ms;
    } time_t;
    typedef struct {
        //0x55 0x51 AxL AxH AyL AyH AzL AzH TL TH SUM
        short x;
        short y;
        short z;
        short Tem;
    } acc_t;
    typedef struct {
        //0x55 0x52 wxL wxH wyL wyH wzL wzH TL TH SUM
        short x;
        short y;
        short z;
        short Tem;
    } gyro_t;
    typedef struct {
        //0x55 0x53 RollL RollH PitchL PitchH YawL YawH TL TH SUM
        short Roll;
        short Pitch;
        short Yaw;
        short Tem;
    } angle_t;
    typedef struct {
        time_t time;
        acc_t acc;
        gyro_t gyro;
        angle_t angle;
    } mpu9250_data_t;
    typedef struct {
        int x;
        int y;
        int z;
    } acc_int_t;
    acc_int_t acc_int;
    mpu9250_data_t mpu9250_data;
    int usart2_data_buffer_length = 0;
    unsigned char usart2_data_buffer[14];
    bool trans_completed = false;
}

extern "C" void USART2_IRQHandler(void) {
    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
        trans_completed = false;
        unsigned char data = USART_ReceiveData(USART2);
        usart2_data_buffer[usart2_data_buffer_length++] = data;
        if (usart2_data_buffer_length == 1) {
            if (data != 0x55) {
                usart2_data_buffer_length = 0;
            }
        } else if (usart2_data_buffer_length == 14) {
            switch (usart2_data_buffer[1]) {
                case 0x51:
                    memcpy(&acc_int, &usart2_data_buffer[2], 12);
                    trans_completed = true;
                    break;
            }
            usart2_data_buffer_length = 0;
        }
    }
}

namespace cdh {
    mpu9250_t *mpu9250_t::open() {
        GPIO_InitTypeDef GPIO_InitStructure;
        //*********usart2****************************************************************************/
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
        //USART1 Tx(PA.02)
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
        //USART1 Rx(PA.03)
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
        USART_InitTypeDef USART_InitStructure;
        USART_InitStructure.USART_BaudRate = 115200;
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;
        USART_InitStructure.USART_StopBits = USART_StopBits_1;
        USART_InitStructure.USART_Parity = USART_Parity_No;
        USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
        USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
        USART_Init(USART2, &USART_InitStructure);
        NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
        NVIC_InitTypeDef NVIC_InitStructure;
        NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
        USART_ClearFlag(USART2, USART_FLAG_TC);//防止第一个数据被覆盖
        USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启中断
        USART_Cmd(USART2, ENABLE);
        return 0;
    }

    void mpu9250_t::test() {
//        for (int i = 0; i < sizeof(mpu9250_data_t); ++i) {
//            unsigned char data = *(((unsigned char *) &mpu9250_data) + i);
//            usart1_t usart1;
//            usart1.write(&data, 1, 1);
//        }
//        int ss = mpu9250_data.time.ss;
//        int ms = mpu9250_data.time.ms;
//        float ax = (float) mpu9250_data.acc.x / 32768 * 16;
//        float ay = (float) mpu9250_data.acc.y / 32768 * 16;
//        float az = (float) mpu9250_data.acc.z / 32768 * 16;
//        float wx = (float) mpu9250_data.gyro.x / 32768 * 2000;
//        float wy = (float) mpu9250_data.gyro.y / 32768 * 2000;
//        float wz = (float) mpu9250_data.gyro.z / 32768 * 2000;
//        printf("ax:%.3f\tay:%.3f\taz:%.3f\tx:%.3f\ty:%.3f\tz:%.3f\r\n", ax, ay, az, wx, wy, wz);

//        printf("%c,%c,%c\r\n",usart2_data_buffer[0],usart2_data_buffer[1],usart2_data_buffer[2]);
//        while (trans_completed == false) {
//            printf("wait data:%d\r\n", usart2_data_buffer_length);
//        }
        while (!trans_completed) {
            printf("wait data.%d\r\n", usart2_data_buffer_length);
        }
        float ax = (float) acc_int.x * 0.002392578f;
        float ay = (float) acc_int.y * 0.002392578f;
        float az = (float) acc_int.z * 0.002392578f;
        printf("ax:%.3f\tay:%.3f\taz:%.3f\r\n", ax, ay, az);

//        //Êä³öÊ±¼ä
//        printf("Time:20%d-%d-%d %d:%d:%.3f\r\n",stcTime.ucYear,stcTime.ucMonth,stcTime.ucDay,stcTime.ucHour,stcTime.ucMinute,(float)stcTime.ucSecond+(float)stcTime.usMiliSecond/1000);
//        //Êä³ö¼ÓËÙ¶È
//        printf("Acc:%.3f %.3f %.3f\r\n",(float)stcAcc.a[0]/32768*16,(float)stcAcc.a[1]/32768*16,(float)stcAcc.a[2]/32768*16);
//        //Êä³ö½ÇËÙ¶È
//        printf("Gyro:%.3f %.3f %.3f\r\n",(float)stcGyro.w[0]/32768*2000,(float)stcGyro.w[1]/32768*2000,(float)stcGyro.w[2]/32768*2000);
//        //Êä³ö½Ç¶È
//        printf("Angle:%.3f %.3f %.3f\r\n",(float)stcAngle.Angle[0]/32768*180,(float)stcAngle.Angle[1]/32768*180,(float)stcAngle.Angle[2]/32768*180);
//        //Êä³ö´Å³¡
//        printf("Mag:%d %d %d\r\n",stcMag.h[0],stcMag.h[1],stcMag.h[2]);
//        //Êä³öÆøÑ¹¡¢¸ß¶È
//        printf("Pressure:%ld Height%.2f\r\n",stcPress.lPressure,(float)stcPress.lAltitude/100);
//
//        //Êä³ö¶Ë¿Ú×´Ì¬
//        printf("DStatus:%d %d %d %d\r\n",stcDStatus.sDStatus[0],stcDStatus.sDStatus[1],stcDStatus.sDStatus[2],stcDStatus.sDStatus[3]);
//
//        //Êä³ö¾­Î³¶È
//        printf("Longitude:%ldDeg%.5fm Lattitude:%ldDeg%.5fm\r\n",stcLonLat.lLon/10000000,(double)(stcLonLat.lLon % 10000000)/1e5,stcLonLat.lLat/10000000,(double)(stcLonLat.lLat % 10000000)/1e5);
//
//        //Êä³öµØËÙ
//        printf("GPSHeight:%.1fm GPSYaw:%.1fDeg GPSV:%.3fkm/h\r\n",(float)stcGPSV.sGPSHeight/10,(float)stcGPSV.sGPSYaw/10,(float)stcGPSV.lGPSVelocity/1000);
//
//        //Êä³öËÄÔªËØ
//        printf("Four elements:%.5f %.5f %.5f %.5f\r\n\r\n",(float)stcQ.q[0]/32768,(float)stcQ.q[1]/32768,(float)stcQ.q[2]/32768,(float)stcQ.q[3]/32768);
    }

    vec2<float> mpu9250_t::acc() {
        while (!trans_completed);//等待完整的数据
        trans_completed = false;//读取一次
        return vec2<float>((float) acc_int.x * 0.002392578f, (float) acc_int.y * 0.002392578f);
    }

}

