#include "my_lcd.h"
#include "lcd.h"
#include "stdlib.h"
#include "usart.h"	 
#include "delay.h"


int lastball_x;
int lastball_y;

//��ʾ����,��λΪ0,����ʾ
//x,y :�������	 
//len :���ֵ�λ��
//size:�����С
//color:��ɫ 
//num:��ֵ(0~4294967295);	
void LCD_ShowIntNum(u16 x,u16 y,int num,u8 len,u8 size)
{         	
	u8 t,temp;
	u8 enshow=0;	
if(num>=0){	
	for(t=0;t<len;t++)
	{
		temp=(num/LCD_Pow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				LCD_ShowChar(x+(size/2)*t,y,' ',size,0);
				continue;
			}else enshow=1; 
		 	 
		}
	 	LCD_ShowChar(x+(size/2)*t,y,temp+'0',size,0); 
	}
}
else
{
	LCD_ShowChar(x,y,'-',size,0); 
	num=-num;
		for(t=0;t<len;t++)
	{
		temp=(num/LCD_Pow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				LCD_ShowChar(x+size+(size/2)*t,y,' ',size,0);
				continue;
			}else enshow=1; 
		 	 
		}
	 	LCD_ShowChar(x+size+(size/2)*t,y,temp+'0',size,0); 
	}
}
} 


void LCD_ClearCircle(int x0,int y0,int x1,int y1,int r0,int r1)
{
	int x,y,r=r1;
	POINT_COLOR=WHITE;
	for(y=y1 - r;y<y1 +r;y++)
	{
		for(x=x1 - r;x<x1+r;x++)
		{
			if(((x-x1)*(x-x1)+(y-y1)*(y-y1)) <= r1*r1)
			{
				if(((x-x0)*(x-x0)+(y-y0)*(y-y0))>=(r0*r0))
				{LCD_DrawPoint(x,y);}
				
			}
		}
	}

}

void LCD_DrawFullCircle(uint16_t Xpos,uint16_t Ypos,uint16_t Radius)
{
uint16_t x,y,r=Radius;
for(y=Ypos - r;y<Ypos +r;y++)
{
for(x=Xpos - r;x<Xpos+r;x++)
{
if(((x-Xpos)*(x-Xpos)+(y-Ypos)*(y-Ypos)) <= r*r)
{
LCD_DrawPoint(x,y);
}
}
}
}

//��ָ��λ�ÿ�ʼ��ʾһ���ַ���	    
//֧���Զ�����
//(x,y):��ʼ����
//width,height:����
//str  :�ַ���
//size :�����С
//mode:0,�ǵ��ӷ�ʽ;1,���ӷ�ʽ    	   		   
void Show_Str(u16 x,u16 y,u16 width,u16 height,u8*str,u8 size,u8 mode)
{					
	u16 x0=x;
	u16 y0=y;							  	  
    u8 bHz=0;     //�ַ���������  	    				    				  	  
    while(*str!=0)//����δ����
    { 
        if(!bHz)
        {
	        if(*str>0x80)bHz=1;//���� 
	        else              //�ַ�
	        {      
                if(x>(x0+width-size/2))//����
				{				   
					y+=size;
					x=x0;	   
				}							    
		        if(y>(y0+height-size))break;//Խ�緵��      
		        if(*str==13)//���з���
		        {         
		            y+=size;
					x=x0;
		            str++; 
		        }  
		        else LCD_ShowChar(x,y,*str,size,mode);//��Ч����д�� 
				str++; 
		        x+=size/2; //�ַ�,Ϊȫ�ֵ�һ�� 
	        }
        }else;				 
    }   
}  			 		 

void screen_base_init(void)
{
	LCD_Fill(M_position,POLE_height,N_position,POLE_height+4,BLACK);
	Show_Str(150,100,72,16,"position:",16,0);
}

void show_ball_position(float x_position)
{
	float x=M_position+x_position;
	float y=POLE_height-15;
	LCD_Fill(225,100,225+8*3,100+16,WHITE);
	POINT_COLOR=RED;
	LCD_DrawFullCircle(x,y,16);
	LCD_ShowIntNum(225,100,(int)x_position,3,16);
	POINT_COLOR=WHITE;
	LCD_ClearCircle(x,y,lastball_x,lastball_y,16,16);
	lastball_x=x;
	lastball_y=y;
}