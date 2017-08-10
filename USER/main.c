#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "lcd.h"
#include "usmart.h"  
#include "usart2.h"  
#include "timer.h" 
#include "ov2640.h"
#include "dcmi.h"
#include "ov2640cfg.h"
#include <string.h>
//ALIENTEK 探索者STM32F407开发板 实验35
//摄像头 实验 -库函数版本
//技术支持：www.openedv.com
//淘宝店铺：http://eboard.taobao.com  
//广州市星翼电子科技有限公司  
//作者：正点原子 @ALIENTEK

__align(4) u16 rgb_buf[200*200 + 16];
u16 * rgb_buffer = &rgb_buf[16];
u8 frame_state = 0;
void on_frame_got(void);
										//0,数据没有采集完;
										//1,数据采集完了,但是还没处理;
										//2,数据已经处理完成了,可以开始下一帧接收

const u8*EFFECTS_TBL[7]={"Normal","Negative","B&W","Redish","Greenish","Bluish","Antique"};	//7种特效 
const u8*JPEG_SIZE_TBL[9]={"QCIF","QQVGA","CIF","QVGA","VGA","SVGA","XGA","SXGA","UXGA"};	//JPEG图片 9种尺寸 


//处理JPEG数据
//当采集完一帧JPEG数据后,调用此函数,切换JPEG BUF.开始下一帧采集.
void jpeg_data_process(void)
{
    if(frame_state==0)
    {	
        DMA_Cmd(DMA2_Stream1, DISABLE);//停止当前传输 
        while (DMA_GetCmdStatus(DMA2_Stream1) != DISABLE){}//等待DMA2_Stream1可配置  
        frame_state=1; 				//标记JPEG数据采集完按成,等待其他函数处理
    }
    if(frame_state==2)	//上一次的jpeg数据已经被处理了
    {
        DMA2_Stream1->NDTR=100*200;	
        DMA_SetCurrDataCounter(DMA2_Stream1,100*200);//传输长度为jpeg_buf_size*4字节
        DMA_Cmd(DMA2_Stream1, ENABLE);//重新传输
        frame_state=0;						//标记数据未采集
    }
} 
#define write_SCCB SCCB_WR_Reg
#define read_SCCB SCCB_RD_Reg
//RGB565测试
//RGB数据直接显示在LCD上面
void rgb565_test(void)
{ 
	u8 key,temp;
	u8 effect=0,saturation=2,contrast=2;
	u8 scale=1;		//默认是全尺寸缩放
	u8 msgbuf[15];	//消息缓存区 

	//OV2640_RGB565_Mode();	//RGB565模式
    //设置为yuyv
    write_SCCB(0xff, 0x00);
    write_SCCB(0xda, 0x00);
    temp = read_SCCB(0xc2);
    temp &= 0xef;
    write_SCCB(0xc2, temp);


    
 	OV2640_ImageWin_Set(200,100,400,400);
    OV2640_OutSize_Set(200,200);
    
	My_DCMI_Init();			//DCMI配置
	DCMI_DMA_Init((u32)&rgb_buf,200 * 200,DMA_MemoryDataSize_HalfWord,DMA_MemoryInc_Enable);//DCMI DMA配置  
	DCMI_Start(); 		//启动传输
    
	while(1)
	{ 
		key=KEY_Scan(0); 
		if(key)
		{ 
			DCMI_Stop(); //停止显示
			switch(key)
			{				    
				case KEY0_PRES:	//对比度设置
					contrast++;
					if(contrast>4)contrast=0;
					OV2640_Contrast(contrast);
					sprintf((char*)msgbuf,"Contrast:%d",(signed char)contrast-2);
					break;
				case KEY1_PRES:	//饱和度Saturation
					saturation++;
					if(saturation>4)saturation=0;
					OV2640_Color_Saturation(saturation);
					sprintf((char*)msgbuf,"Saturation:%d",(signed char)saturation-2);
					break;
				case KEY2_PRES:	//特效设置				 
					effect++;
					if(effect>6)effect=0;
					OV2640_Special_Effects(effect);//设置特效
					sprintf((char*)msgbuf,"%s",EFFECTS_TBL[effect]);
					break;
				case WKUP_PRES:	//1:1尺寸(显示真实尺寸)/缩放	    
					
					break;
			}
			LCD_ShowString(30,50,210,16,16,msgbuf);//显示提示内容
			delay_ms(800);            
			DCMI_Start();//重新开始传输
		} 
		
        if(frame_state == 1){
            LCD_Set_Window(0,0,200,200);
            LCD_WriteRAM_Prepare();		//开始写入GRAM
            memcpy(&rgb_buffer[200*200 - 16],&rgb_buf[0],16*2);
            for(int i = 0;i<200*200;++i){
                LCD->LCD_RAM = (rgb_buffer)[i];
            }
            LCD_Set_Window(200,0,200,200);
            LCD_WriteRAM_Prepare();		//开始写入GRAM
            on_frame_got();
            sprintf((char*)msgbuf,"%u,%u,%u,%u",(u32)(rgb_buffer[100*200+100]&0xff),(u32)(rgb_buffer[100*200+100]>>8),(u32)(rgb_buffer[100*200+101])&0xff,(u32)(rgb_buffer[100*200+101]>>8));
            LCD_ShowString(0,200,200,200,16,msgbuf);
            frame_state = 2;
        }
        //delay_ms(10);
	}    
}

int main(void)
{ 
	u8 key;
	u8 t;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(168);  //初始化延时函数
	uart_init(115200);		//初始化串口波特率为115200
	usart2_init(42,115200);		//初始化串口2波特率为115200
	LED_Init();					//初始化LED 
 	LCD_Init();					//LCD初始化
    //LCD_Display_Dir(1);
 	KEY_Init();					//按键初始化 
	//TIM3_Int_Init(10000-1,8400-1);//10Khz计数,1秒钟中断一次
	
 	//usmart_dev.init(84);		//初始化USMART
 	POINT_COLOR=RED;//设置字体为红色 
	LCD_ShowString(30,50,200,16,16,"Explorer STM32F4");	
	LCD_ShowString(30,70,200,16,16,"OV2640 TEST");	
	LCD_ShowString(30,90,200,16,16,"ATOM@ALIENTEK");
	LCD_ShowString(30,110,200,16,16,"2014/5/14");  	 
	while(OV2640_Init())//初始化OV2640
	{
		LCD_ShowString(30,130,240,16,16,"OV2640 ERR");
		delay_ms(200);
	    LCD_Fill(30,130,239,170,WHITE);
		delay_ms(200);
	}
    //OV2640,采用SVGA分辨率(800*600)  
    for(int i=0;i<sizeof(ov2640_svga_init_reg_tbl)/2;i++)
    {
        SCCB_WR_Reg(ov2640_svga_init_reg_tbl[i][0],ov2640_svga_init_reg_tbl[i][1]);
    }
	LCD_ShowString(30,130,200,16,16,"OV2640 OK");  	  
    delay_ms(1000);
    LCD_Clear(BLACK);
    rgb565_test(); 
}
