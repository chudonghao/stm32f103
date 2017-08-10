
#include "fix_glm.h"
#include <cstdio>
#include <glm/glm.hpp>
extern "C" {
#include "lcd.h"
}
using namespace glm;
using namespace std;
namespace {
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
    point_on_canvas_t top_point_on_canvas;
    point_on_canvas_t bottom_point_on_canvas;
    point_on_canvas_t ball_point_on_canvas;

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
        if (green >= 38 && red < 13 && blue < 23)
            return color_type_green_e;
        if (blue >= blue_threshold && green < green_threshold && red < red_threshold)
            return color_type_blue_e;
        if (blue < 10 && green < 20 && red < 10)
            return color_type_black_e;
        else return color_type_common_e;

    }

    color_type_e color_type_yuv(unsigned char y, unsigned char u, unsigned char v) {
        if(u < 120 && v<100){
            return color_type_green_e;
        }
        if (v >= 150) {
            return color_type_red_e;
        }
        if (u >= 150) {
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
    float x_ball_zero_sampling = 0.f;
}
extern "C" u16 * rgb_buffer;
extern "C" u8 draw_frame_on_lcd;
extern "C" void on_frame_got(void) {
    ball_point_on_canvas.restart_record();
    for (int y = 0; y < 200; ++y) {
        for (int x = 0; x <200; x+=2) {
//          u16 color;
            u8 y0 = rgb_buffer[y*200+x]&0xff
            ,y1 = rgb_buffer[y*200+x+1]&0xff
            ,u = rgb_buffer[y*200+x]>>8
            ,v = rgb_buffer[y*200+x + 1]>>8;
//                //B = 1.164(Y - 16) + 2.018(U - 128)
//                //G = 1.164(Y - 16) - 0.813(V - 128) - 0.391(U - 128)
//                //R = 1.164(Y - 16) + 1.596(V - 128)
//                unsigned char b0 = 1.164 * (y0 - 16) + 2.018 * (u - 128);
//                unsigned char g0 = 1.164 * (y0 - 16) - 0.813 * (v - 128) - 0.391 * (u - 128);
//                unsigned char r0 = 1.164 * (y0 - 16) + 1.596 * (v - 128);
//                unsigned char b1 = 1.164 * (y1 - 16) + 2.018 * (u - 128);
//                unsigned char g1 = 1.164 * (y1 - 16) - 0.813 * (v - 128) - 0.391 * (u - 128);
//                unsigned char r1 = 1.164 * (y1 - 16) + 1.596 * (v - 128);

////                //R = Y + 1.402 (Cr-128)
////                //G = Y - 0.34414 (Cb-128) - 0.71414 (Cr-128)
////                //B = Y + 1.772 (Cb-128)
////                unsigned char r0 = y0 + 1.772 * (u - 128);
////                unsigned char g0 = y0 - 0.34414 * (u - 128) - 0.71414 * (v - 128);
////                unsigned char b0 = y0 + 1.402 * (v - 128);
////                unsigned char r1 = y1 + 1.772 * (u - 128);
////                unsigned char g1 = y1 - 0.34414 * (u - 128) - 0.71414 * (v - 128);
////                unsigned char b1 = y1 + 1.402 * (v - 128);
//                color = (r0 >> 3) << 11 ;
//                color |= (g0 >> 2) << 5;
//                color |= (b0 >> 3);
//                LCD->LCD_RAM = color;
//                color = (r1 >> 3) << 11 ;
//                color |= (g1 >> 2) << 5;
//                color |= (b1 >> 3);
//                LCD->LCD_RAM = color;

            switch (color_type_yuv(y0, u, v)) {
                case color_type_red_e:
                    ball_point_on_canvas.record(ivec2(x,y));
                    break;
                case color_type_black_e:
                    break;
                case color_type_green_e:
                    break;
                case color_type_blue_e:
                    if(x < 50 && y >=75 && y < 125){
                        top_point_on_canvas.record(ivec2(x,y));
                    }else if(x >=75 && x < 125 && y <50){
                        right_point_on_canvas.record(ivec2(x,y));
                    }else if(x >=75 && x < 125 && y >= 150){
                        left_point_on_canvas.record(ivec2(x,y));
                    }else if(x >= 150 && y >= 75 && y < 125){
                        bottom_point_on_canvas.record(ivec2(x,y));
                    }
                    break;
                case color_type_bright_e:
                    break;
                default:
            }
            switch (color_type_yuv(y1, u, v)) {
                case color_type_red_e:
                    ball_point_on_canvas.record(ivec2(x+1,y));
                    break;
                case color_type_black_e:
                    break;
                case color_type_green_e:
                    break;
                case color_type_blue_e:
                    if(x + 1 < 50 && y >=75 && y < 125){
                        top_point_on_canvas.record(ivec2(x+1,y));
                    }else if(x + 1 >=75 && x + 1 < 125 && y <50){
                        right_point_on_canvas.record(ivec2(x+1,y));
                    }else if(x + 1 >=75 && x + 1 < 125 && y >= 150){
                        left_point_on_canvas.record(ivec2(x+1,y));
                    }else if(x + 1 >= 150 && y >= 75 && y < 125){
                        bottom_point_on_canvas.record(ivec2(x+1,y));
                    }
                    break;
                case color_type_bright_e:
                    break;
                default:
            }
        }
    }

    vec2 ball = ball_point_on_canvas.get_position(0);
    vec2 top = top_point_on_canvas.get_position();
    vec2 bottom = bottom_point_on_canvas.get_position();
    vec2 left = left_point_on_canvas.get_position();
    vec2 right = right_point_on_canvas.get_position();
    if(top.x > 0 && top.y > 0 && bottom.x > 0 && bottom.y > 0
            && left.x > 0 && left.y > 0 && right.x > 0 && right.y > 0){
        if(ball.x>0&&ball.y>0){
            vec2 vec_top_bottom = top - bottom;
            vec2 vec_right_left = right - left;
            float top_bottom_pixel = length(vec_top_bottom);
            float right_left_pixel = length(vec_right_left);
            vec2 vec_ball_bottom = ball - bottom;
            vec2 vec_ball_left = ball - left;
            float vec_cross_z;
            float ball_x_pixel;
            float ball_y_pixel;
            float ball_x;
            float ball_y;
            vec_cross_z = vec_ball_bottom.x * vec_top_bottom.y - vec_ball_bottom.y * vec_top_bottom.x;
            ball_x_pixel = vec_cross_z / top_bottom_pixel;
            vec_cross_z = vec_right_left.x * vec_ball_left.y - vec_right_left.y * vec_ball_left.x;
            ball_y_pixel = vec_cross_z / right_left_pixel;
            ball_x = 620.0f / right_left_pixel * ball_x_pixel;
            ball_y = 620.0f / top_bottom_pixel * ball_y_pixel;

            printf("ball %.3f %.3f\r\n",ball_x,ball_y);
        }
    }

    static char msg[30];
    if(draw_frame_on_lcd){
        sprintf(msg,"ball=(%f,%f)",ball.x,ball.y);
        LCD_ShowString(0,216,200,300,16,(u8*)msg);
        LCD_Set_Window(200,0,200,200);
        LCD_WriteRAM_Prepare();		//开始写入GRAM
        for (int y = 0; y < 200; ++y) {
            for (int x = 0; x <200; x+=2) {
                u8 y0 = rgb_buffer[y*200+x]&0xff
                ,y1 = rgb_buffer[y*200+x+1]&0xff
                ,u = rgb_buffer[y*200+x]>>8
                ,v = rgb_buffer[y*200+x + 1]>>8;

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
        }
    }
}
