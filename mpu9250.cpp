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
    mpu9250_data_t mpu9250_data;
    int usart2_data_buffer_length = 0;
    unsigned char usart2_data_buffer[11];
    bool trans_completed = true;
//    struct STime {
//        unsigned char ucYear;
//        unsigned char ucMonth;
//        unsigned char ucDay;
//        unsigned char ucHour;
//        unsigned char ucMinute;
//        unsigned char ucSecond;
//        unsigned short usMiliSecond;
//    };
//    struct SAcc {
//        short a[3];
//        short T;
//    };
//    struct SGyro {
//        short w[3];
//        short T;
//    };
//    struct SAngle {
//        short Angle[3];
//        short T;
//    };
//    struct SMag {
//        short h[3];
//        short T;
//    };
//
//    struct SDStatus {
//        short sDStatus[4];
//    };
//
//    struct SPress {
//        long lPressure;
//        long lAltitude;
//    };
//
//    struct SLonLat {
//        long lLon;
//        long lLat;
//    };
//
//    struct SGPSV {
//        short sGPSHeight;
//        short sGPSYaw;
//        long lGPSVelocity;
//    };
//    struct SQ {
//        short q[4];
//    };
//    struct STime stcTime;
//    struct SAcc stcAcc;
//    struct SGyro stcGyro;
//    struct SAngle stcAngle;
//    struct SMag stcMag;
//    struct SDStatus stcDStatus;
//    struct SPress stcPress;
//    struct SLonLat stcLonLat;
//    struct SGPSV stcGPSV;
//    struct SQ stcQ;
//
//    static unsigned char ucRxBuffer[11];
//    static unsigned char ucRxCnt = 0;
}

extern "C" void USART2_IRQHandler(void) {
    USART_ClearITPendingBit(USART2, USART_IT_ORE);
    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
        trans_completed = false;
        unsigned char data = USART_ReceiveData(USART2);
        usart2_data_buffer[usart2_data_buffer_length++] = data;
        if (usart2_data_buffer_length == 1) {
            if (data != 0x55) {
                usart2_data_buffer_length = 0;
            }
        } else if (usart2_data_buffer_length < 11) {
        } else {
            switch (usart2_data_buffer[1]) {
                case 0x50:
                    memcpy(&mpu9250_data.time, &usart2_data_buffer[2], 8);
                    break;
                case 0x51:
                    memcpy(&mpu9250_data.acc, &usart2_data_buffer[2], 8);
                    break;
                case 0x52:
                    memcpy(&mpu9250_data.gyro, &usart2_data_buffer[2], 8);
                    break;
                case 0x53:
                    memcpy(&mpu9250_data.angle, &usart2_data_buffer[2], 8);
                    trans_completed = true;
                    break;
            }
            usart2_data_buffer_length = 0;
        }
//        ucRxBuffer[ucRxCnt++]=data;
//        if (ucRxBuffer[0]!=0x55) //Êý¾ÝÍ·²»¶Ô£¬ÔòÖØÐÂ¿ªÊ¼Ñ°ÕÒ0x55Êý¾ÝÍ·
//        {
//            ucRxCnt=0;
//            return;
//        }
//        if (ucRxCnt<11) {return;}//Êý¾Ý²»Âú11¸ö£¬Ôò·µ»Ø
//        else
//        {
//            switch(ucRxBuffer[1])//ÅÐ¶ÏÊý¾ÝÊÇÄÄÖÖÊý¾Ý£¬È»ºó½«Æä¿½±´µ½¶ÔÓ¦µÄ½á¹¹ÌåÖÐ£¬ÓÐÐ©Êý¾Ý°üÐèÒªÍ¨¹ýÉÏÎ»»ú´ò¿ª¶ÔÓ¦µÄÊä³öºó£¬²ÅÄÜ½ÓÊÕµ½Õâ¸öÊý¾Ý°üµÄÊý¾Ý
//            {
//                case 0x50:	memcpy(&stcTime,&ucRxBuffer[2],8);break;//memcpyÎª±àÒëÆ÷×Ô´øµÄÄÚ´æ¿½±´º¯Êý£¬ÐèÒýÓÃ"string.h"£¬½«½ÓÊÕ»º³åÇøµÄ×Ö·û¿½±´µ½Êý¾Ý½á¹¹ÌåÀïÃæ£¬´Ó¶øÊµÏÖÊý¾ÝµÄ½âÎö¡£
//                case 0x51:	memcpy(&stcAcc,&ucRxBuffer[2],8);break;
//                case 0x52:	memcpy(&stcGyro,&ucRxBuffer[2],8);break;
//                case 0x53:	memcpy(&stcAngle,&ucRxBuffer[2],8);break;
//                case 0x54:	memcpy(&stcMag,&ucRxBuffer[2],8);break;
//                case 0x55:	memcpy(&stcDStatus,&ucRxBuffer[2],8);break;
//                case 0x56:	memcpy(&stcPress,&ucRxBuffer[2],8);break;
//                case 0x57:	memcpy(&stcLonLat,&ucRxBuffer[2],8);break;
//                case 0x58:	memcpy(&stcGPSV,&ucRxBuffer[2],8);break;
//                case 0x59:	memcpy(&stcQ,&ucRxBuffer[2],8);break;
//            }
//            ucRxCnt=0;//Çå¿Õ»º´æÇø
//        }
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
        USART_InitStructure.USART_BaudRate = 19200;
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;
        USART_InitStructure.USART_StopBits = USART_StopBits_1;
        USART_InitStructure.USART_Parity = USART_Parity_No;
        USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
        USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
        USART_Init(USART2, &USART_InitStructure);
        NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
        NVIC_InitTypeDef NVIC_InitStructure;
        NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
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
        int ss = mpu9250_data.time.ss;
        int ms = mpu9250_data.time.ms;
        float ax = (float) mpu9250_data.acc.x / 32768 * 16;
        float ay = (float) mpu9250_data.acc.y / 32768 * 16;
        float az = (float) mpu9250_data.acc.z / 32768 * 16;
        float wx = (float) mpu9250_data.gyro.x / 32768 * 2000;
        float wy = (float) mpu9250_data.gyro.y / 32768 * 2000;
        float wz = (float) mpu9250_data.gyro.z / 32768 * 2000;
        printf("ax:%.3f\tay:%.3f\taz:%.3f\tx:%.3f\ty:%.3f\tz:%.3f\r\n", ax, ay, az, wx, wy, wz);
        //Êä³öÊ±¼ä
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
        USART_Cmd(USART2, DISABLE);
        trans_completed = false;//读取一次

        short x = mpu9250_data.acc.x;
        short y = mpu9250_data.acc.y;
        USART_Cmd(USART2, ENABLE);
        float ax = (float) x / 32768 * 16 * 9.8;
        float ay = (float) y / 32768 * 16 * 9.8;
        return vec2<float>(ax, ay);
    }

}

