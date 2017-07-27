
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
 ALIENTEK战舰STM32开发板实验35
 摄像头OV7670 实验
 技术支持：www.openedv.com
 淘宝店铺：http://eboard.taobao.com 
 关注微信公众平台微信号："正点原子"，免费获取STM32资料。
 广州市星翼电子科技有限公司  
 作者：正点原子 @ALIENTEK
************************************************/

extern u8 ov_sta;    //在exit.c里 面定义
//更新LCD显示
void camera_refresh(void) {
    u32 j;
    u16 color;
    if (ov_sta)//有帧中断更新？
    {
        LCD_Scan_Dir(U2D_L2R);        //从上到下,从左到右
        if (lcddev.id == 0X1963)
            LCD_Set_Window((lcddev.width - 240) / 2, (lcddev.height - 320) / 2, 240, 320);//将显示区域设置到屏幕中央
        else if (lcddev.id == 0X5510 || lcddev.id == 0X5310)
            LCD_Set_Window((lcddev.width - 320) / 2, (lcddev.height - 240) / 2, 320, 240);//将显示区域设置到屏幕中央
        LCD_WriteRAM_Prepare();     //开始写入GRAM
        OV7670_RRST = 0;                //开始复位读指针
        OV7670_RCK_L;
        OV7670_RCK_H;
        OV7670_RCK_L;
        OV7670_RRST = 1;                //复位读指针结束
        OV7670_RCK_H;
        for (j = 0; j < 76800; j++) {
            OV7670_RCK_L;
            color = GPIOC->IDR & 0XFF;    //读数据
            OV7670_RCK_H;
            color <<= 8;
            OV7670_RCK_L;
            color |= GPIOC->IDR & 0XFF;    //读数据
            OV7670_RCK_H;
            LCD->LCD_RAM = color & 0x001f;
        }
        ov_sta = 0;                    //清零帧中断标记
        LCD_Scan_Dir(DFT_SCAN_DIR);    //恢复默认扫描方向
    }
}

void main_task(void *) {
    for (;;) {
        vTaskSuspendAll();
        camera_refresh();//更新显示
        xTaskResumeAll();
    }
}

int main(void) {
    SystemCoreClockUpdate();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
    delay_init();                                   //延时函数初始化
    LCD_Init();                                     //初始化LCD
    POINT_COLOR = RED;                                //设置字体为红色
    while (OV7670_Init())                            //初始化OV7670
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
    EXTI8_Init();                                   //使能定时器捕获
    OV7670_Window_Set(12, 176, 240, 320);              //设置窗口
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
 