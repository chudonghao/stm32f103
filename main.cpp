
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
        point_on_canvas_t() {}

        ivec2 sum;
        int count;
        int last_record_column;
        int last_record_row;

        void restart_record(int column = 0, int row = 0) {
            sum = ivec2(0, 0);
            count = 0;
            last_record_column = column;
            last_record_row = row;
        }

        const ivec2 get_position(int fix = 5) {
            if (count) {
                if (count > fix) {
                    return sum / count;
                } else {
                    return ivec2(-2, -2);
                }
            } else {
                return ivec2(-1, -1);
            }
        }

        void record(int x, int y) {
            if (x - last_record_column > 1) {//列间隔
                if (count > 10)
                    return;
                restart_record(x, y);
            } else {
                if (x - last_record_column == 0) {
                    if (y - last_record_row == -1) {
                        sum += ivec2(x, y);
                        ++count;
                    }
                }
            }
            last_record_column = x;
            last_record_row = y;
        }
    };

    point_on_canvas_t top_point_on_canvas;
    point_on_canvas_t left_point_on_canvas;
    point_on_canvas_t right_point_on_canvas;
    point_on_canvas_t bottom_point_on_canvas;
    point_on_canvas_t laser_point_on_canvas;
    point_on_canvas_t green_laser_point_on_canvas;

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
        OV7670_Contrast(4);//对比度设置 0:-2 1:-1 2:0 3:1 4:2
        OV7670_Special_Effects(0);//特效设置 0:普通模式 1,负片 2,黑白 3,偏红色 4,偏绿色 5,偏蓝色 6,复古
        EXTI8_Init();
        OV7670_Window_Set(12, 176, camera_w, camera_h);              //设置窗口
        OV7670_CS = 0;
    }

    enum color_type_e {
        color_type_common_e = 0,
        color_type_red_e,
        color_type_green_e,
        color_type_black_e = -1
    };

    color_type_e color_type_rgb(u16 color) {
        u8 red = color >> 11;
        u8 green = (color >> 5) & 0x3f;
        u8 blue = (color) & 0x1f;
        if (red >= 29 && green < 60) {
            return color_type_red_e;
        } else if (green >= 60 && red < 29) {
            return color_type_green_e;
        } else if (red < 14 && green < 28 && blue < 14) {
            return color_type_black_e;
        }
        return color_type_common_e;
    }
}


extern "C" u8 ov_sta;    //在exit.c里 面定义

//更新LCD显示
void camera_refresh(void) {
    u32 j;
    u16 color;
    if (ov_sta)//有帧中断更新？
    {
        LCD_Scan_Dir(U2D_L2R);        //从上到下,从左到右
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
        top_point_on_canvas.restart_record();
        bottom_point_on_canvas.restart_record();
        left_point_on_canvas.restart_record();
        right_point_on_canvas.restart_record();
        laser_point_on_canvas.restart_record();
        green_laser_point_on_canvas.restart_record();

        for (int x = 0; x < camera_w; ++x) {
            for (int y = 0; y < camera_h; ++y) {
                color = 0;
                OV7670_RCK_L;
                color = GPIOC->IDR & 0XFF;    //读数据
                OV7670_RCK_H;
                color <<= 8;
                OV7670_RCK_L;
                color |= GPIOC->IDR & 0XFF;    //读数据
                OV7670_RCK_H;

//            if(j%camera_h == 0)
//                printf("line:");
//            printf("%d,",tmp);
//                LCD->LCD_RAM = color;
                int fix_y = 319 - y;
                if (fix_y < 30 || fix_y >= 290) {
                    LCD->LCD_RAM = BLACK;
                    continue;
                }
                switch (color_type_rgb(color)) {
                    case color_type_red_e:
                        LCD->LCD_RAM = RED;
                        laser_point_on_canvas.record(x, fix_y);
                        break;
                    case color_type_black_e:
                        LCD->LCD_RAM = BLACK;
                        if (x < 80 && fix_y >= 120 && fix_y < 200) {//left point
                            left_point_on_canvas.record(x, fix_y);
                        } else if (x >= 160 && fix_y >= 120 && fix_y < 200) {//right point
                            right_point_on_canvas.record(x, fix_y);
                        } else if (x >= 80 && x < 160 && fix_y >= 200 && fix_y < 290) {//top point
                            top_point_on_canvas.record(x, fix_y);
                        } else if (x >= 80 && x < 160 && fix_y >= 30 && fix_y < 120) {//bottom point
                            bottom_point_on_canvas.record(x, fix_y);
                        }
                        break;
                    case color_type_green_e:
                        LCD->LCD_RAM = GREEN;
                        green_laser_point_on_canvas.record(x, fix_y);
                        break;
                    default:
                        LCD->LCD_RAM = GRAY;
                }
            }
        }

        ov_sta = 0;                    //清零帧中断标记
        LCD_Scan_Dir(DFT_SCAN_DIR);    //恢复默认扫描方向
//        //rgb图像处理 结果显示
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
        //计算实际点位
        static char ch1[128];
        sprintf(ch1, "top=(%d,%d) bottom=(%d,%d) left=(%d,%d) right=(%d,%d) laser=(%d,%d) green_laser=(%d,%d)\r\n",
                top_point_on_canvas.get_position().x, top_point_on_canvas.get_position().y,
                bottom_point_on_canvas.get_position().x, bottom_point_on_canvas.get_position().y,
                left_point_on_canvas.get_position().x, left_point_on_canvas.get_position().y,
                right_point_on_canvas.get_position().x, right_point_on_canvas.get_position().y,
                laser_point_on_canvas.get_position().x, laser_point_on_canvas.get_position().y,
                green_laser_point_on_canvas.get_position().x, green_laser_point_on_canvas.get_position().y
        );
        LCD_ShowString(0, 320, 800, 16, 16, (u8 *) ch1);
        vec2 top = top_point_on_canvas.get_position();
        vec2 bottom = bottom_point_on_canvas.get_position();
        vec2 left = left_point_on_canvas.get_position();
        vec2 right = right_point_on_canvas.get_position();
        vec2 laser = laser_point_on_canvas.get_position();
        vec2 green_laser = green_laser_point_on_canvas.get_position();
        if (top.x > 0 && top.y > 0 && bottom.x > 0 && bottom.y > 0
            && left.x > 0 && left.y > 0 && right.x > 0 && right.y > 0) {
            if (laser.x > 0 && laser.y > 0) {
                vec2 vec_top_bottom = top - bottom;
                vec2 vec_right_left = right - left;
                float top_bottom_pixel = length(vec_top_bottom);
                float right_left_pixel = length(vec_right_left);

                vec2 vec_laser_bottom = laser - bottom;
                vec2 vec_laser_left = laser - left;
                float vec_cross_z;
                float laser_x_pixel;
                float laser_y_pixel;
                float laser_x;
                float laser_y;
                vec_cross_z = vec_laser_bottom.x * vec_top_bottom.y - vec_laser_bottom.y * vec_top_bottom.x;
                laser_x_pixel = vec_cross_z / top_bottom_pixel;
                vec_cross_z = vec_right_left.x * vec_laser_left.y - vec_right_left.y * vec_laser_left.x;
                laser_y_pixel = vec_cross_z / right_left_pixel;

                laser_x = 615.0f / right_left_pixel * laser_x_pixel;
                laser_y = 615.0f / top_bottom_pixel * laser_y_pixel;
                static int count = 0;
                sprintf(ch1, "count=%d,laser=(%.3f,%.3f)\r\n", count, laser_x, laser_y);
                LCD_ShowString(0, 320 + 16, 800, 16, 16, (u8 *) ch1);
                printf("scp %.3f %.3f\r\n", laser_x, laser_y, sizeof(color_type_e));
                ++count;
                if (green_laser.x > 0 && green_laser.y > 0) {
                    vec_laser_bottom = green_laser - bottom;
                    vec_laser_left = green_laser - left;
                    vec_cross_z = vec_laser_bottom.x * vec_top_bottom.y - vec_laser_bottom.y * vec_top_bottom.x;
                    laser_x_pixel = vec_cross_z / top_bottom_pixel;
                    vec_cross_z = vec_right_left.x * vec_laser_left.y - vec_right_left.y * vec_laser_left.x;
                    laser_y_pixel = vec_cross_z / right_left_pixel;
                    laser_x = 615.0f / right_left_pixel * laser_x_pixel;
                    laser_y = 615.0f / top_bottom_pixel * laser_y_pixel;
                    static int count = 0;
                    sprintf(ch1, "count=%d,green_laser=(%.3f,%.3f)\r\n", count, laser_x, laser_y);
                    LCD_ShowString(0, 320 + 32, 800, 16, 16, (u8 *) ch1);
                    printf("snp %.3f %.3f\r\n", laser_x, laser_y, sizeof(color_type_e));
                    ++count;
                }else{
                    sprintf(ch1, "**************************\r\n");
                    LCD_ShowString(0, 320 + 32, 800, 16, 16, (u8 *) ch1);
                }
            } else {
                sprintf(ch1, "**************************\r\n");
                LCD_ShowString(0, 320 + 16, 800, 16, 16, (u8 *) ch1);
            }
        }
    }
}

void main_task(void *) {
    for (;;) {
        camera_refresh();//更新显示
    }
}

int main(void) {
    SystemCoreClockUpdate();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
    delay_init();                                   //延时函数初始化
    uart_init(115200);
    lcd_init();

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

//    OV7670_Window_Set(12, 176, 240, 320);              //设置窗口
    //SCCB_WR_Reg(0x12,0x24);

//    //设置为QVGA YUV
//    //参考 https://www.amobbs.com/thread-5490146-1-1.html
//    //{ 0x12, 0x10 }
//    //{ 0x3a, 0x14 }使用固定UV输出
//    //{ 0x3d, 0x80 }使用固定UV输出
//    //{ 0x67, 0x11 }固定U值，0x11，方便测试
//    //{ 0x68, 0xFF }固定V值，0xFF，方便测试
//    //{ 0x40, 0xC0 }
//    SCCB_WR_Reg(0x12, 0x10);
//    //SCCB_WR_Reg(0x3a, 0x14);
//    //SCCB_WR_Reg(0x3d, 0x80);
//    //SCCB_WR_Reg(0x67, 0x11);
//    //SCCB_WR_Reg(0x68, 0xFF);
//    SCCB_WR_Reg(0x40, 0xC0);

    LCD_Clear(BLACK);

    xTaskCreate(main_task, 0, 200, 0, 1, 0);
    vTaskStartScheduler();
}
