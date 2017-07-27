
#include "cstring"
#include <FreeRTOS.h>
#include <task.h>
#include <stm32f10x_conf.h>

#define SYSTEM_SUPPORT_OS 1
extern "C" {
#include "delay.h"
#include "sys.h"
#include "lcd.h"
#include "ov7670.h"
#include "timer.h"
#include "exti.h"
#include "usart.h"
}

extern u8 ov_sta;    //��exit.c�� �涨��
const static int camera_w = 240;
const static int camera_h = 320;

//����LCD��ʾ
void camera_refresh(void) {
    u32 j;
    u16 color;
    if (ov_sta)//��֡�жϸ��£�
    {

        LCD_Scan_Dir(D2U_R2L);        //���ϵ���,������
        LCD_Set_Window(0, 0, camera_h, camera_w);
//        if (lcddev.id == 0X1963)
//            LCD_Set_Window((lcddev.width - camera_w) / 2, (lcddev.height - camera_h) / 2, camera_w, camera_h);//����ʾ�������õ���Ļ����
//        else if (lcddev.id == 0X5510 || lcddev.id == 0X5310)
//            LCD_Set_Window((lcddev.width - camera_h) / 2, (lcddev.height - camera_w) / 2, camera_h, camera_w);//����ʾ�������õ���Ļ����
        LCD_WriteRAM_Prepare();     //��ʼд��GRAM
        OV7670_RRST = 0;                //��ʼ��λ��ָ��
        OV7670_RCK_L;
        OV7670_RCK_H;
        OV7670_RCK_L;
        OV7670_RRST = 1;                //��λ��ָ�����
        OV7670_RCK_H;
//        printf("red:");
        for (j = 0; j < camera_w * camera_h; j++) {
            OV7670_RCK_L;
            color = GPIOC->IDR & 0XFF;    //������
            OV7670_RCK_H;
            color <<= 8;
            OV7670_RCK_L;
            color |= GPIOC->IDR & 0XFF;    //������
            OV7670_RCK_H;

//            u16 tmp = color & ((u16)0x1f << 11);
//            tmp >>= 11;
//            if(j%camera_h == 0)
//                printf("line:");
//            printf("%d,",tmp);

//            if(tmp > 28)
//                color = 0xf800;

//            tmp = color & ((u16)0x3f << 5);
//            if(tmp < ((u16)50 << 5))
//                result = 0x0;

//            tmp = color & ((u16)0x1f);
//            if(tmp < ((u16)30))
//                result = 0x0;

            LCD->LCD_RAM = color;
        }

//        printf("\r\n");
//        //rgbͼ����
//        for (int x = 0; x < camera_h; ++x) {
//            for (int y = 0; y < camera_w; ++y) {
//                color = LCD_ReadPoint(x, y);
//                u16 tmp;
//                //���
//                tmp = color & ((u16) 0x1f << 11);
//                tmp >>= 11;
//                if (tmp > 28) {
//                    POINT_COLOR = RED;
//                } else if (tmp < 10) {
//                    POINT_COLOR = BLACK;
//                } else {
//                    POINT_COLOR = WHITE;
//                }
//                LCD_DrawPoint(x, y + 240);
//
//                tmp = color & ((u16) 0x3f << 5);
//                tmp >>= 5;
//                if (tmp > 50) {
//                    POINT_COLOR = GREEN;
//                } else if (tmp < 20) {
//                    POINT_COLOR = BLACK;
//                } else {
//                    POINT_COLOR = WHITE;
//                }
//                LCD_DrawPoint(x + 320, y);
//
//                tmp = color & ((u16) 0x1f);
//                if (tmp > 28) {
//                    POINT_COLOR = BLUE;
//                } else if (tmp < 10) {
//                    POINT_COLOR = BLACK;
//                } else {
//                    POINT_COLOR = WHITE;
//                }
//                LCD_DrawPoint(x + 320, y + 240);
//
//            }
//        }

        ov_sta = 0;                    //����֡�жϱ��
        LCD_Scan_Dir(DFT_SCAN_DIR);    //�ָ�Ĭ��ɨ�跽��
    }
}

void main_task(void *) {
    for (;;) {
        vTaskSuspendAll();
        camera_refresh();//������ʾ
        xTaskResumeAll();
    }
}

int main(void) {
    SystemCoreClockUpdate();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
    delay_init();                                   //��ʱ������ʼ��
    LCD_Init();                                     //��ʼ��LCD
    uart_init(115200);

    POINT_COLOR = RED;                                //��������Ϊ��ɫ
    while (OV7670_Init())                            //��ʼ��OV7670
    {
        u8 ch[] = "OV7670 Error!!";
        LCD_ShowString(30, 230, 200, 16, 16, ch);
        delay_ms(200);
        LCD_Fill(30, 230, 239, 246, WHITE);
        delay_ms(200);
    }
    u8 ch[] = "OV7670 Init OK";
    LCD_ShowString(30, 230, 200, 16, 16, ch);
    OV7670_Light_Mode(0);
    OV7670_Color_Saturation(2);
    OV7670_Brightness(2);
    OV7670_Contrast(2);
    OV7670_Special_Effects(0);
    EXTI8_Init();                                   //ʹ�ܶ�ʱ������
//    OV7670_Window_Set(12, 176, 240, 320);              //���ô���
    //SCCB_WR_Reg(0x12,0x24);

    //����ΪQVGA YUV
    //�ο� https://www.amobbs.com/thread-5490146-1-1.html
    //{ 0x12, 0x10 }
    //{ 0x3a, 0x14 }ʹ�ù̶�UV���
    //{ 0x3d, 0x80 }ʹ�ù̶�UV���
    //{ 0x67, 0x11 }�̶�Uֵ��0x11���������
    //{ 0x68, 0xFF }�̶�Vֵ��0xFF���������
    //{ 0x40, 0xC0 }
    SCCB_WR_Reg(0x12, 0x10);
    //SCCB_WR_Reg(0x3a, 0x14);
    //SCCB_WR_Reg(0x3d, 0x80);
    //SCCB_WR_Reg(0x67, 0x11);
    //SCCB_WR_Reg(0x68, 0xFF);
    SCCB_WR_Reg(0x40, 0xC0);

    OV7670_Window_Set(12, 176, camera_w, camera_h);              //���ô���
    OV7670_CS = 0;
    LCD_Clear(WHITE);
    delay_ms(100);
    LCD_Clear(BLUE);
    delay_ms(100);
    LCD_Clear(GREEN);
    delay_ms(100);
    LCD_Clear(RED);
    delay_ms(100);
    LCD_Clear(BLACK);
    xTaskCreate(main_task, 0, 100, 0, 1, 0);
    xPortStartScheduler();
}
