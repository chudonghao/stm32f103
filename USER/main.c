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
//ALIENTEK ̽����STM32F407������ ʵ��35
//����ͷ ʵ�� -�⺯���汾
//����֧�֣�www.openedv.com
//�Ա����̣�http://eboard.taobao.com  
//������������ӿƼ����޹�˾  
//���ߣ�����ԭ�� @ALIENTEK

__align(4) u16 rgb_buf[200*200 + 16];
u16 * rgb_buffer = &rgb_buf[16];
u8 frame_state = 0;
void on_frame_got(void);
										//0,����û�вɼ���;
										//1,���ݲɼ�����,���ǻ�û����;
										//2,�����Ѿ����������,���Կ�ʼ��һ֡����

const u8*EFFECTS_TBL[7]={"Normal","Negative","B&W","Redish","Greenish","Bluish","Antique"};	//7����Ч 
const u8*JPEG_SIZE_TBL[9]={"QCIF","QQVGA","CIF","QVGA","VGA","SVGA","XGA","SXGA","UXGA"};	//JPEGͼƬ 9�ֳߴ� 


//����JPEG����
//���ɼ���һ֡JPEG���ݺ�,���ô˺���,�л�JPEG BUF.��ʼ��һ֡�ɼ�.
void jpeg_data_process(void)
{
    if(frame_state==0)
    {	
        DMA_Cmd(DMA2_Stream1, DISABLE);//ֹͣ��ǰ���� 
        while (DMA_GetCmdStatus(DMA2_Stream1) != DISABLE){}//�ȴ�DMA2_Stream1������  
        frame_state=1; 				//���JPEG���ݲɼ��갴��,�ȴ�������������
    }
    if(frame_state==2)	//��һ�ε�jpeg�����Ѿ���������
    {
        DMA2_Stream1->NDTR=100*200;	
        DMA_SetCurrDataCounter(DMA2_Stream1,100*200);//���䳤��Ϊjpeg_buf_size*4�ֽ�
        DMA_Cmd(DMA2_Stream1, ENABLE);//���´���
        frame_state=0;						//�������δ�ɼ�
    }
} 
#define write_SCCB SCCB_WR_Reg
#define read_SCCB SCCB_RD_Reg
//RGB565����
//RGB����ֱ����ʾ��LCD����
void rgb565_test(void)
{ 
	u8 key,temp;
	u8 effect=0,saturation=2,contrast=2;
	u8 scale=1;		//Ĭ����ȫ�ߴ�����
	u8 msgbuf[15];	//��Ϣ������ 

	//OV2640_RGB565_Mode();	//RGB565ģʽ
    //����Ϊyuyv
    write_SCCB(0xff, 0x00);
    write_SCCB(0xda, 0x00);
    temp = read_SCCB(0xc2);
    temp &= 0xef;
    write_SCCB(0xc2, temp);


    
 	OV2640_ImageWin_Set(200,100,400,400);
    OV2640_OutSize_Set(200,200);
    
	My_DCMI_Init();			//DCMI����
	DCMI_DMA_Init((u32)&rgb_buf,200 * 200,DMA_MemoryDataSize_HalfWord,DMA_MemoryInc_Enable);//DCMI DMA����  
	DCMI_Start(); 		//��������
    
	while(1)
	{ 
		key=KEY_Scan(0); 
		if(key)
		{ 
			DCMI_Stop(); //ֹͣ��ʾ
			switch(key)
			{				    
				case KEY0_PRES:	//�Աȶ�����
					contrast++;
					if(contrast>4)contrast=0;
					OV2640_Contrast(contrast);
					sprintf((char*)msgbuf,"Contrast:%d",(signed char)contrast-2);
					break;
				case KEY1_PRES:	//���Ͷ�Saturation
					saturation++;
					if(saturation>4)saturation=0;
					OV2640_Color_Saturation(saturation);
					sprintf((char*)msgbuf,"Saturation:%d",(signed char)saturation-2);
					break;
				case KEY2_PRES:	//��Ч����				 
					effect++;
					if(effect>6)effect=0;
					OV2640_Special_Effects(effect);//������Ч
					sprintf((char*)msgbuf,"%s",EFFECTS_TBL[effect]);
					break;
				case WKUP_PRES:	//1:1�ߴ�(��ʾ��ʵ�ߴ�)/����	    
					
					break;
			}
			LCD_ShowString(30,50,210,16,16,msgbuf);//��ʾ��ʾ����
			delay_ms(800);            
			DCMI_Start();//���¿�ʼ����
		} 
		
        if(frame_state == 1){
            LCD_Set_Window(0,0,200,200);
            LCD_WriteRAM_Prepare();		//��ʼд��GRAM
            memcpy(&rgb_buffer[200*200 - 16],&rgb_buf[0],16*2);
            for(int i = 0;i<200*200;++i){
                LCD->LCD_RAM = (rgb_buffer)[i];
            }
            LCD_Set_Window(200,0,200,200);
            LCD_WriteRAM_Prepare();		//��ʼд��GRAM
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
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����ϵͳ�ж����ȼ�����2
	delay_init(168);  //��ʼ����ʱ����
	uart_init(115200);		//��ʼ�����ڲ�����Ϊ115200
	usart2_init(42,115200);		//��ʼ������2������Ϊ115200
	LED_Init();					//��ʼ��LED 
 	LCD_Init();					//LCD��ʼ��
    //LCD_Display_Dir(1);
 	KEY_Init();					//������ʼ�� 
	//TIM3_Int_Init(10000-1,8400-1);//10Khz����,1�����ж�һ��
	
 	//usmart_dev.init(84);		//��ʼ��USMART
 	POINT_COLOR=RED;//��������Ϊ��ɫ 
	LCD_ShowString(30,50,200,16,16,"Explorer STM32F4");	
	LCD_ShowString(30,70,200,16,16,"OV2640 TEST");	
	LCD_ShowString(30,90,200,16,16,"ATOM@ALIENTEK");
	LCD_ShowString(30,110,200,16,16,"2014/5/14");  	 
	while(OV2640_Init())//��ʼ��OV2640
	{
		LCD_ShowString(30,130,240,16,16,"OV2640 ERR");
		delay_ms(200);
	    LCD_Fill(30,130,239,170,WHITE);
		delay_ms(200);
	}
    //OV2640,����SVGA�ֱ���(800*600)  
    for(int i=0;i<sizeof(ov2640_svga_init_reg_tbl)/2;i++)
    {
        SCCB_WR_Reg(ov2640_svga_init_reg_tbl[i][0],ov2640_svga_init_reg_tbl[i][1]);
    }
	LCD_ShowString(30,130,200,16,16,"OV2640 OK");  	  
    delay_ms(1000);
    LCD_Clear(BLACK);
    rgb565_test(); 
}
