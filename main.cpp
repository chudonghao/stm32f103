
#include <cstring>
#include <FreeRTOS.h>
#include <task.h>
#include <stm32f10x_conf.h>
#include "fix_glm.h"
#include <glm/glm.hpp>

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

#include "step_motor_couple.h"

using namespace cdh;
using namespace glm;
using namespace std;
namespace {
    const int camera_w = 240;
    const int camera_h = 320;

    class point_on_canvas_t {
    public:
        point_on_canvas_t() : top(-1), left(-1), right(-1), bottom(-1) {}

        int top;
        int left;
        int right;
        int bottom;

        ivec2 get_position() {
            int resx,resy;
            if(right - left >15)
                resx = -2;
            else
                resx = (left + right) / 2;
            if(top - bottom >15)
                resy = -2;
            else
                resy = (top + bottom) / 2;
            return ivec2(resx,resy);
        }

        void add_point(int x, int y) {
            if (left == -1) {
                left = x;
            }
            if (right == -1 || right < x) {
                right = x;
            }
            if (top == -1) {
                top = y;
            }
            if (bottom == -1 || bottom > y) {
                bottom = y;
            }
        }
    };

    point_on_canvas_t top_point_on_canvas;
    point_on_canvas_t left_point_on_canvas;
    point_on_canvas_t right_point_on_canvas;
    point_on_canvas_t bottom_point_on_canvas;
    point_on_canvas_t laser_point_on_canvas;
}


extern "C" u8 ov_sta;    //��exit.c�� �涨��


//����LCD��ʾ
void camera_refresh(void) {
    u32 j;
    u16 color;
    if (ov_sta)//��֡�жϸ��£�
    {
        LCD_Scan_Dir(U2D_L2R);        //���ϵ���,������
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
        top_point_on_canvas = point_on_canvas_t();
        bottom_point_on_canvas = point_on_canvas_t();
        left_point_on_canvas = point_on_canvas_t();
        right_point_on_canvas = point_on_canvas_t();
        laser_point_on_canvas = point_on_canvas_t();

        for (j = 0; j < camera_w * camera_h; j++) {
            OV7670_RCK_L;
            color = GPIOC->IDR & 0XFF;    //������
            OV7670_RCK_H;
            color <<= 8;
            OV7670_RCK_L;
            color |= GPIOC->IDR & 0XFF;    //������
            OV7670_RCK_H;

//            if(j%camera_h == 0)
//                printf("line:");
//            printf("%d,",tmp);

            LCD->LCD_RAM = color;
        }

//        printf("\r\n");
        //rgbͼ����
        for (int x = 0; x < camera_h; ++x) {
            for (int y = 0; y < camera_w; ++y) {
                color = LCD_ReadPoint(x, y);
                u16 tmp;
                //���
                tmp = color & ((u16) 0x1f << 11);
                tmp >>= 11;
                if (tmp > 28) {
                    POINT_COLOR = RED;
                } else if (tmp < 10) {
                    POINT_COLOR = BLACK;
                } else {
                    POINT_COLOR = WHITE;
                }
                LCD_DrawPoint(x, y + 240);

                tmp = color & ((u16) 0x3f << 5);
                tmp >>= 5;
                if (tmp > 50) {
                    POINT_COLOR = GREEN;
                } else if (tmp < 20) {
                    POINT_COLOR = BLACK;
                } else {
                    POINT_COLOR = WHITE;
                }
                LCD_DrawPoint(x + 320, y);

                tmp = color & ((u16) 0x1f);
                if (tmp > 28) {
                    POINT_COLOR = BLUE;
                } else if (tmp < 10) {
                    POINT_COLOR = BLACK;
                } else {
                    POINT_COLOR = WHITE;
                }
                LCD_DrawPoint(x + 320, y + 240);

            }
        }
        return;
        //yuvͼ����
        for (int x = 0; x < camera_w; ++x) {
            for (int y = 0; y < camera_h; ++y) {
                int fix_y = 319 - y;
                int point_type = 0;
                color = LCD_ReadPoint(y, x);
                u16 tmp;
                //���
                tmp = color & ((u16) 0xff << 8);
                if (tmp > ((u16) 200 << 8)) {
                    point_type = 1;
                    POINT_COLOR = WHITE;
                } else if (tmp < ((u16) 100 << 8)) {
                    point_type = -1;
                    POINT_COLOR = BLACK;
                } else {
                    POINT_COLOR = GRAY;
                }
                LCD_DrawPoint(y, x + 240);

                if (point_type == -1) {
                    if (x < 80 && fix_y >= 120 && fix_y < 200) {//left point
                        left_point_on_canvas.add_point(x, fix_y);
                    } else if (x >= 160 && fix_y >= 120 && fix_y < 200) {//right point
                        right_point_on_canvas.add_point(x, fix_y);
                    } else if (x >= 80 && x < 160 && fix_y >= 200 && fix_y < 290) {//top point
                        top_point_on_canvas.add_point(x, fix_y);
                    } else if (x >= 80 && x < 160 && fix_y >= 30 && fix_y < 120) {//bottom point
                        bottom_point_on_canvas.add_point(x, fix_y);
                    }
                } else if (point_type == 1) {
                    if (fix_y >= 30 && fix_y < 290)
                        laser_point_on_canvas.add_point(x, fix_y);
                }
            }
        }
        ov_sta = 0;                    //����֡�жϱ��
        LCD_Scan_Dir(DFT_SCAN_DIR);    //�ָ�Ĭ��ɨ�跽��
        static u8 ch1[128];
        sprintf((char*)ch1,"top=(%d,%d) bottom=(%d,%d) left=(%d,%d) right=(%d,%d) laser=(%d,%d)\r\n",
                top_point_on_canvas.get_position().x, top_point_on_canvas.get_position().y,
                bottom_point_on_canvas.get_position().x, bottom_point_on_canvas.get_position().y,
                left_point_on_canvas.get_position().x, left_point_on_canvas.get_position().y,
                right_point_on_canvas.get_position().x, right_point_on_canvas.get_position().y,
                laser_point_on_canvas.get_position().x, laser_point_on_canvas.get_position().y
        );
        LCD_ShowString(0,320,800,16,16,ch1);
    }
}

void main_task(void *) {
    for (;;) {
        camera_refresh();//������ʾ
    }
}

int main(void) {
    SystemCoreClockUpdate();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
    delay_init();                                   //��ʱ������ʼ��
    LCD_Init();                                     //��ʼ��LCD
//    LCD_Display_Dir(1);
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

//    //����ΪQVGA YUV
//    //�ο� https://www.amobbs.com/thread-5490146-1-1.html
//    //{ 0x12, 0x10 }
//    //{ 0x3a, 0x14 }ʹ�ù̶�UV���
//    //{ 0x3d, 0x80 }ʹ�ù̶�UV���
//    //{ 0x67, 0x11 }�̶�Uֵ��0x11���������
//    //{ 0x68, 0xFF }�̶�Vֵ��0xFF���������
//    //{ 0x40, 0xC0 }
//    SCCB_WR_Reg(0x12, 0x10);
//    //SCCB_WR_Reg(0x3a, 0x14);
//    //SCCB_WR_Reg(0x3d, 0x80);
//    //SCCB_WR_Reg(0x67, 0x11);
//    //SCCB_WR_Reg(0x68, 0xFF);
//    SCCB_WR_Reg(0x40, 0xC0);

    OV7670_Window_Set(12, 176, camera_w, camera_h);              //���ô���
    OV7670_CS = 0;
    LCD_Clear(BLACK);

    xTaskCreate(main_task, 0, 100, 0, 1, 0);
    vTaskStartScheduler();
}
