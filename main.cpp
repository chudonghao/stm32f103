
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
}


/************************************************
 ALIENTEKս��STM32������ʵ��35
 ����ͷOV7670 ʵ��
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com 
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/

extern u8 ov_sta;    //��exit.c�� �涨��
//����LCD��ʾ
void camera_refresh(void) {
    u32 j;
    u16 color;
    if (ov_sta)//��֡�жϸ��£�
    {
        LCD_Scan_Dir(U2D_L2R);        //���ϵ���,������
        if (lcddev.id == 0X1963)
            LCD_Set_Window((lcddev.width - 240) / 2, (lcddev.height - 320) / 2, 240, 320);//����ʾ�������õ���Ļ����
        else if (lcddev.id == 0X5510 || lcddev.id == 0X5310)
            LCD_Set_Window((lcddev.width - 320) / 2, (lcddev.height - 240) / 2, 320, 240);//����ʾ�������õ���Ļ����
        LCD_WriteRAM_Prepare();     //��ʼд��GRAM
        OV7670_RRST = 0;                //��ʼ��λ��ָ��
        OV7670_RCK_L;
        OV7670_RCK_H;
        OV7670_RCK_L;
        OV7670_RRST = 1;                //��λ��ָ�����
        OV7670_RCK_H;
        for (j = 0; j < 76800; j++) {
            OV7670_RCK_L;
            color = GPIOC->IDR & 0XFF;    //������
            OV7670_RCK_H;
            color <<= 8;
            OV7670_RCK_L;
            color |= GPIOC->IDR & 0XFF;    //������
            OV7670_RCK_H;
            LCD->LCD_RAM = color & 0x001f;
        }
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
    OV7670_Window_Set(12, 176, 240, 320);              //���ô���
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
    xTaskCreate(main_task,0,100,0,1,0);
    xPortStartScheduler();
}
 