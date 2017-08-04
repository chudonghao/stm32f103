#ifndef __MY_LCD_H
#define __MY_LCD_H		
#include "sys.h"	 
#include "stdlib.h"


#define M_position 150
#define N_position 650
#define POLE_height 400



void LCD_ShowIntNum(u16 x,u16 y,int num,u8 len,u8 size);
void LCD_ClearCircle(int x0,int y0,int x1,int y1,int r0,int r1);
void LCD_DrawFullCircle(uint16_t Xpos,uint16_t Ypos,uint16_t Radius);
void Show_Str(u16 x,u16 y,u16 width,u16 height,u8*str,u8 size,u8 mode);
void screen_base_init(void);
void show_ball_position(float x_position);

#endif
