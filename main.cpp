
#include <cstring>
#include <cstdio>
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
#include "led.h"
#include "beep.h"
#include "my_lcd.h"
}

#include "usart1.h"
#include "step_motor_couple.h"

using namespace cdh;
using namespace glm;
using namespace std;
namespace {
    const int camera_w = 240;
    const int camera_h = 320;

    class point_on_canvas_t {
    public:
        point_on_canvas_t() {}

        ivec2 sum;
        int count;
        ivec2 last_record;

        void restart_record(const ivec2 &last_record = ivec2(0, 0)) {
            sum = ivec2(0, 0);
            count = 0;
            this->last_record = last_record;
        }

        vec2 get_position(int fix = 5) {
            if (count) {
                if (count > fix) {
                    return vec2(sum) / (float) count;
                } else {
                    return vec2(-2, -2);
                }
            } else {
                return vec2(-1, -1);
            }
        }

        void record(const ivec2 &record) {
            sum += record;
            ++count;
//            if (last_record.y - record.y > 1) {//列间隔
//                if (count > 10)
//                    return;
//                restart_record(record);
//            } else {
//                if (last_record.y - record.y == 0) {
//                    if (last_record.x - record.x == 1) {
//                        sum += record;
//                        ++count;
//                    }
//                }
//            }
//            last_record = record;
        }
    };

    point_on_canvas_t left_point_on_canvas;
    point_on_canvas_t right_point_on_canvas;
    point_on_canvas_t red_point_on_canvas;
    point_on_canvas_t ball_point_on_canvas;

    void lcd_init() {
        LCD_Init();                                     //初始化LCD
        LCD_Display_Dir(1);
    }

    bool camera_init() {
        return OV7670_Init();
    };

    void camera_config() {
        OV7670_Light_Mode(0);//白平衡设置  0:自动 1:太阳sunny 2,阴天cloudy 3,办公室office 4,家里home
        OV7670_Color_Saturation(2);
        OV7670_Brightness(2);
        OV7670_Contrast(2);//对比度设置 0:-2 1:-1 2:0 3:1 4:2
        OV7670_Special_Effects(0);//特效设置 0:普通模式 1,负片 2,黑白 3,偏红色 4,偏绿色 5,偏蓝色 6,复古
        EXTI8_Init();
        OV7670_Window_Set(12, 176, camera_w, camera_h);              //设置窗口
        OV7670_CS = 0;
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
    }

    enum color_type_e {
        color_type_common_e = 0,
        color_type_red_e,
        color_type_green_e,
        color_type_blue_e,
        color_type_bright_e,
        color_type_black_e = -1
    };
    int red_threshold = 25;
    int green_threshold = 60;
    int blue_threshold = 25;

    color_type_e color_type_rgb(u16 color) {
        u8 red = color >> 11;
        u8 green = ((color >> 5) & 0x3f) >> 1;
        u8 blue = (color) & 0x1f;

//        if(red >= red_threshold && green < green_threshold && blue < blue_threshold )
//            return color_type_red_e;
//        if(green >= green_threshold && red < red_threshold && blue < blue_threshold )
//            return color_type_green_e;
//        if(blue >= blue_threshold && green < green_threshold && red < red_threshold )
//            return color_type_blue_e;

        if (red >= 14 && green < 10 && blue < 12)
            return color_type_red_e;
        if (green >= green_threshold && red < red_threshold && blue < blue_threshold)
            return color_type_green_e;
        if (blue >= blue_threshold && green < green_threshold && red < red_threshold)
            return color_type_blue_e;
        if (blue < 10 && green < 20 && red < 10)
            return color_type_black_e;
        else return color_type_common_e;

    }

    color_type_e color_type_yuv(unsigned char y, unsigned char u, unsigned char v) {
        if (u >= 150) {
            return color_type_red_e;
        }
        if (v >= 150) {
            return color_type_blue_e;
        }
        if (y < 70) {
            return color_type_black_e;
        }
        if (y > 195) {
            return color_type_bright_e;
        }
        return color_type_common_e;
    }

    bool lose_valid_frame = true;
    bool show_image_process = false;
    bool need_clear_error_frame = false;
    float x_ball_zero_sampling = 0.f;
}


extern "C" u8 ov_sta;    //在exit.c里 面定义

//更新LCD显示
void camera_refresh(void) {
    //show_image_process = true;
    u16 color;
    if (ov_sta)//有帧中断更新？
    {
        //写扫描方向
        LCD_Scan_Dir(L2R_D2U);
        LCD_Set_Window(SSD_HOR_RESOLUTION - camera_h, 0, camera_h, camera_w);
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

        left_point_on_canvas.restart_record();
        right_point_on_canvas.restart_record();
        ball_point_on_canvas.restart_record();
        red_point_on_canvas.restart_record();
        for (int y = camera_w - 1; y >= 0; --y) {
            for (int x = camera_h - 2; x >= 0; x = x - 2) {
                static unsigned char y0 = 0;
                static unsigned char y1 = 0;
                static unsigned char v = 0;
                static unsigned char u = 0;

                OV7670_RCK_L;
                y0 = GPIOC->IDR & 0XFF;    //???
                OV7670_RCK_H;
                OV7670_RCK_L;
                u = GPIOC->IDR & 0XFF;    //???
                OV7670_RCK_H;
                OV7670_RCK_L;
                y1 = GPIOC->IDR & 0XFF;    //???
                OV7670_RCK_H;
                OV7670_RCK_L;
                v = GPIOC->IDR & 0XFF;    //???
                OV7670_RCK_H;

//                //B = 1.164(Y - 16) + 2.018(U - 128)
//                //G = 1.164(Y - 16) - 0.813(V - 128) - 0.391(U - 128)
//                //R = 1.164(Y - 16) + 1.596(V - 128)
//                unsigned char r0 = 1.164 * (y0 - 16) + 2.018 * (u0 - 128);
//                unsigned char g0 = 1.164 * (y0 - 16) - 0.813 * (v0 - 128) - 0.391 * (u0 - 128);
//                unsigned char b0 = 1.164 * (y0 - 16) + 1.596 * (v0 - 128);
//                unsigned char r1 = 1.164 * (y1 - 16) + 2.018 * (u1 - 128);
//                unsigned char g1 = 1.164 * (y1 - 16) - 0.813 * (v1 - 128) - 0.391 * (u1 - 128);
//                unsigned char b1 = 1.164 * (y1 - 16) + 1.596 * (v1 - 128);

//                //R = Y + 1.402 (Cr-128)
//                //G = Y - 0.34414 (Cb-128) - 0.71414 (Cr-128)
//                //B = Y + 1.772 (Cb-128)
//                unsigned char r0 = y0 + 1.772 * (u0 - 128);
//                unsigned char g0 = y0 - 0.34414 * (u0 - 128) - 0.71414 * (v0 - 128);
//                unsigned char b0 = y0 + 1.402 * (v0 - 128);
//                unsigned char r1 = y1 + 1.772 * (u1 - 128);
//                unsigned char g1 = y1 - 0.34414 * (u1 - 128) - 0.71414 * (v1 - 128);
//                unsigned char b1 = y1 + 1.402 * (v1 - 128);
//                color = (r0 >> 3) << 11 ;
//                color |= (g0 >> 2) << 5;
//                color |= (b0 >> 3);
//                LCD->LCD_RAM = color;
//                color = (r1 >> 3) << 11 ;
//                color |= (g1 >> 2) << 5;
//                color |= (b1 >> 3);
//                LCD->LCD_RAM = color;
//                if(y0 >= red_threshold && u0 < green_threshold && v0 < blue_threshold )
//                    LCD->LCD_RAM = RED;
//                else LCD->LCD_RAM = GRAY;
//                if(y1 >= red_threshold && u1 < green_threshold && v1 < blue_threshold )
//                    LCD->LCD_RAM = RED;
//                else LCD->LCD_RAM = GRAY;
                if (y >= 100 && y < 180) {
                    switch (color_type_yuv(y0, u, v)) {
                        case color_type_red_e:
                            if (x >= 15 && x < 305) {
                                red_point_on_canvas.record(ivec2(x + 1, y));
                            }
                            break;
                        case color_type_black_e:
                            if (x < 30) {
                                left_point_on_canvas.record(ivec2(x + 1, y));
                            } else if (x >= 290) {
                                right_point_on_canvas.record(ivec2(x + 1, y));
                            }
                            break;
                        case color_type_green_e:
                            break;
                        case color_type_blue_e:
                            break;
                        case color_type_bright_e:
                            if (x >= 15 && x < 305) {
                                ball_point_on_canvas.record(ivec2(x + 1, y));
                            }
                            break;
                    }
                    switch (color_type_yuv(y1, u, v)) {
                        case color_type_red_e:
                            if (x >= 15 && x < 305) {
                                red_point_on_canvas.record(ivec2(x, y));
                            }
                            break;
                        case color_type_black_e:
                            if (x < 30) {
                                left_point_on_canvas.record(ivec2(x, y));
                            } else if (x >= 290) {
                                right_point_on_canvas.record(ivec2(x, y));
                            }
                            break;
                        case color_type_green_e:
                            break;
                        case color_type_blue_e:
                            break;
                        case color_type_bright_e:
                            if (x >= 15 && x < 305) {
                                ball_point_on_canvas.record(ivec2(x, y));
                            }
                            break;
                    }
                    if (show_image_process) {
                        switch (color_type_yuv(y0, u, v)) {
                            case color_type_red_e:
                                LCD->LCD_RAM = RED;
                                break;
                            case color_type_black_e:
                                LCD->LCD_RAM = BLACK;
                                break;
                            case color_type_green_e:
                                LCD->LCD_RAM = GREEN;
                                break;
                            case color_type_blue_e:
                                LCD->LCD_RAM = BLUE;
                                break;
                            case color_type_bright_e:
                                LCD->LCD_RAM = WHITE;
                                break;
                            default:
                                LCD->LCD_RAM = GRAY;
                        }
                        switch (color_type_yuv(y1, u, v)) {
                            case color_type_red_e:
                                LCD->LCD_RAM = RED;
                                break;
                            case color_type_black_e:
                                LCD->LCD_RAM = BLACK;
                                break;
                            case color_type_green_e:
                                LCD->LCD_RAM = GREEN;
                                break;
                            case color_type_blue_e:
                                LCD->LCD_RAM = BLUE;
                                break;
                            case color_type_bright_e:
                                LCD->LCD_RAM = WHITE;
                                break;
                            default:
                                LCD->LCD_RAM = GRAY;
                        }
                    }
                } else {
                    if (show_image_process) {
                        LCD->LCD_RAM = (y0 << 8) | u;
                        LCD->LCD_RAM = (y1 << 8) | v;
                    }
                }
            }
        }

        ov_sta = 0;                    //等待下一个帧中断
//        //rgb???? ????
//        for (int x = 0; x < camera_w; ++x) {
//            for (int y = 0; y < camera_h; ++y) {
//                if (y < 30 || y >= 290) {
//                    continue;
//                }
//                color = LCD_ReadPoint(x, y);
//                switch (color_type_rgb(color)) {
//                    case color_type_red_e:
//                        POINT_COLOR = RED;
//                        break;
//                    case color_type_black_e:
//                        POINT_COLOR = BLACK;
//                        break;
//                    case color_type_green_e:
//                        POINT_COLOR = GREEN;
//                    default:
//                        POINT_COLOR = GRAY;
//                }
//                LCD_DrawPoint(x + 240, y);
//            }
//        }
        //视觉正方向
        LCD_Scan_Dir(R2L_D2U);
        //获得点位坐标

        vec2 left = left_point_on_canvas.get_position();
        vec2 right = right_point_on_canvas.get_position();
        vec2 red = red_point_on_canvas.get_position();
        vec2 ball = ball_point_on_canvas.get_position(0);
        //LCD_DrawLine(laser.x,0,laser.x,320);
        //LCD_DrawLine(0,320 - laser.y,240,320 - laser.y);
        POINT_COLOR = RED;
        if (left.x > 0 && left.y > 0 && right.x > 0 && right.y > 0 && ball.x > 0 && ball.y > 0) {
            vec2 vec_left_right = right - left;
            vec2 vec_left_ball = ball - left;
            float pixel_left_right = length(vec_left_right);
            float pixel_ball_left = dot(vec_left_right, vec_left_ball) / pixel_left_right;
            float length_per_pixel = 570.f / pixel_left_right;
            float y_left_bottom = (left.y - 120.f) * length_per_pixel;
            float y_right_bottom = (right.y - 120.f) * length_per_pixel;
            float x_ball_zero = pixel_ball_left * length_per_pixel - 10.f;

            if (red.x > 0 && red.y > 0) {
                vec2 vec_left_red = red - left;
                float pixel_red_left = dot(vec_left_right, vec_left_red) / pixel_left_right;
                float x_red_left = pixel_red_left * length_per_pixel - 10.f;
                printf("red %.3f\r\n", x_red_left);
            }
            printf("ball %.3f\r\n", x_ball_zero);

            if (show_image_process) {
                POINT_COLOR = BRED;
                static char ch1[128];
                sprintf(ch1, "left=(%.3f,%.3f) right=(%.3f,%.3f) ball=(%.3f,%.3f)",
                        left_point_on_canvas.get_position().x, left_point_on_canvas.get_position().y,
                        right_point_on_canvas.get_position().x, right_point_on_canvas.get_position().y,
                        ball_point_on_canvas.get_position().x, ball_point_on_canvas.get_position().y
                );
                LCD_ShowString(0, 240, 800, 16, 16, (u8 *) ch1);
                sprintf(ch1, "plr=%f,pbl=%f,ylb=%f,yrb=%f,ybl=%f",
                        pixel_left_right, pixel_ball_left, y_left_bottom, y_right_bottom, x_ball_zero
                );
                LCD_ShowString(0, 240 + 16, 800, 16, 16, (u8 *) ch1);
            }
            lose_valid_frame = false;
            show_image_process = false;
        } else {
            lose_valid_frame = true;
            show_image_process = true;
        }
        if (show_image_process) {
            LCD_DrawLine(0, 120, 319, 120);
            LCD_DrawLine(0, 60, 319, 60);
            if(lose_valid_frame == false){
                need_clear_error_frame = true;
            }
        }
    }
}

void main_task(void *) {
    for (;;) {
        camera_refresh();//更新显示
    }
}
void lcd_task(void*){
    for(;;){
        static float old_x_ball_position_sampling = 0.f;
        if(lose_valid_frame == false && old_x_ball_position_sampling != x_ball_zero_sampling){
            show_ball_position(x_ball_zero_sampling);
        }else if (need_clear_error_frame == true){
            screen_base_init();
            need_clear_error_frame = false;
        }else{
            taskYIELD();
        }
    }
}
void control_task(void *) {
    for (;;) {
        static char ch[128];
        usart1_t::trim_buffer_head();
        while (!usart1_t::have_data_to_read()) {
            taskYIELD();
        }
        scanf("%s", ch);
        if (strcmp(ch, "st") == 0) {
            int count = scanf("%d%d%d", &red_threshold, &green_threshold, &blue_threshold);
            if (count == 3) {
                printf("st %d %d %d\r\n", red_threshold, green_threshold, blue_threshold);
            } else {
                printf("st error input\r\n");
            }
        } else if (strcmp(ch, "sip") == 0) {
            show_image_process = !show_image_process;
        } else if (strcmp(ch, "hint") == 0) {
            BEEP = 1;
            for (int i = 0; i < 30; ++i) {
                LED0 = !LED0;
                vTaskDelay(100);
            }
            LED0 = 0;
            BEEP = 0;
        } else {
            printf("unknow func\r\n");
        }
    }
}

int main(void) {
    SystemCoreClockUpdate();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
    delay_init();                                   //延时函数初始化
    lcd_init();
    LED_Init();
    BEEP_Init();
    usart1_t::init();
    printf("reset.\r\n");
    POINT_COLOR = RED;                                //设置字体为红色
    while (camera_init())                            //初始化OV7670
    {
        u8 ch[] = "OV7670 Error!!";
        LCD_ShowString(30, 230, 200, 16, 16, ch);
        delay_ms(200);
        LCD_Fill(30, 230, 239, 246, WHITE);
        delay_ms(200);
    }
    camera_config();
    u8 ch[] = "OV7670 Init OK";
    LCD_ShowString(30, 230, 200, 16, 16, ch);

    LCD_Clear(BLACK);

    xTaskCreate(main_task, 0, 200, 0, 1, 0);
    xTaskCreate(control_task, 0, 200, 0, 1, 0);
    xTaskCreate(lcd_task,0,200,0,1,0);
    vTaskStartScheduler();
}
