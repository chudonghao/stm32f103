
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

extern u8 ov_sta;    //在exit.c里 面定义
const static int camera_w = 240;
const static int camera_h = 320;

//更新LCD显示
void camera_refresh(void) {
    u32 j;
    u16 color;
    if (ov_sta)//有帧中断更新？
    {

        LCD_Scan_Dir(D2U_R2L);        //从上到下,从左到右
        LCD_Set_Window(0, 0, camera_h, camera_w);
//        if (lcddev.id == 0X1963)
//            LCD_Set_Window((lcddev.width - camera_w) / 2, (lcddev.height - camera_h) / 2, camera_w, camera_h);//将显示区域设置到屏幕中央
//        else if (lcddev.id == 0X5510 || lcddev.id == 0X5310)
//            LCD_Set_Window((lcddev.width - camera_h) / 2, (lcddev.height - camera_w) / 2, camera_h, camera_w);//将显示区域设置到屏幕中央
        LCD_WriteRAM_Prepare();     //开始写入GRAM
        OV7670_RRST = 0;                //开始复位读指针
        OV7670_RCK_L;
        OV7670_RCK_H;
        OV7670_RCK_L;
        OV7670_RRST = 1;                //复位读指针结束
        OV7670_RCK_H;
//        printf("red:");
        for (j = 0; j < camera_w * camera_h; j++) {
            OV7670_RCK_L;
            color = GPIOC->IDR & 0XFF;    //读数据
            OV7670_RCK_H;
            color <<= 8;
            OV7670_RCK_L;
            color |= GPIOC->IDR & 0XFF;    //读数据
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
//        //rgb图像处理
//        for (int x = 0; x < camera_h; ++x) {
//            for (int y = 0; y < camera_w; ++y) {
//                color = LCD_ReadPoint(x, y);
//                u16 tmp;
//                //红点
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
    uart_init(115200);

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
//    OV7670_Window_Set(12, 176, 240, 320);              //设置窗口
    //SCCB_WR_Reg(0x12,0x24);

    //设置为QVGA YUV
    //参考 https://www.amobbs.com/thread-5490146-1-1.html
    //{ 0x12, 0x10 }
    //{ 0x3a, 0x14 }使用固定UV输出
    //{ 0x3d, 0x80 }使用固定UV输出
    //{ 0x67, 0x11 }固定U值，0x11，方便测试
    //{ 0x68, 0xFF }固定V值，0xFF，方便测试
    //{ 0x40, 0xC0 }
    SCCB_WR_Reg(0x12, 0x10);
    //SCCB_WR_Reg(0x3a, 0x14);
    //SCCB_WR_Reg(0x3d, 0x80);
    //SCCB_WR_Reg(0x67, 0x11);
    //SCCB_WR_Reg(0x68, 0xFF);
    SCCB_WR_Reg(0x40, 0xC0);

    OV7670_Window_Set(12, 176, camera_w, camera_h);              //设置窗口
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
