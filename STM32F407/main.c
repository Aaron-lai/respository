//#include "sys.h"
//#include "delay.h"
//#include "usart.h"
#include "led.h"
#include "key.h"  
#include "sram.h"   
#include "malloc.h" 
#include "usmart.h"  
#include "sdio_sdcard.h"    
#include "malloc.h" 
#include "w25qxx.h"    
#include "ff.h"  
#include "exfuns.h"    
#include "tftlcd.h"
#include "spi.h"
#include "timer.h"
////ALIENTEK ̽����STM32F407������ ʵ��39
////FATFS ʵ�� -�⺯���汾
////����֧�֣�www.openedv.com
////�Ա����̣�http://eboard.taobao.com
////������������ӿƼ����޹�˾    
////���ߣ�����ԭ�� @ALIENTEK 
// 




#include "stm32f4xx.h"
//#include "LED.h"
#include "delay.h"
#include "usart.h"
#include "myiic.h"
#include "max30102.h"
#include "dma.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "rtc.h"

//SD������ͷ�ļ�
//#include "sys.h"
//#include "delay.h"
//#include "usart.h"
//#include "led.h"
//#include "lcd.h"
//#include "key.h"  
//#include "sram.h"   
//#include "malloc.h" 
//#include "usmart.h"  
//#include "sdio_sdcard.h"    
//#include "malloc.h" 
//#include "w25qxx.h"    
//#include "ff.h"  
//#include "exfuns.h"  
#define sampNumble  1600



// ���[red]�ͺ����[ir] ������  ������=sampNumble
uint32_t red_1;
uint32_t ir_1;
uint32_t red_2;
uint32_t ir_2;

uint32_t count; //�������
uint8_t uch_dummy;
u8 tbuf[40];
u8 t=0;

//SD�������������
FIL fil;
FRESULT res,s;
UINT bww;
uint32_t buf[4];
int i,j;
u8 flag = 0;//�������±�־λ
u8 k = 0;//�ļ����ɱ�־λ
int b = 0;
char str[20];
RTC_TimeTypeDef RTC_TimeStruct;
RTC_DateTypeDef RTC_DateStruct;
unsigned int FATFSNumSize;//�ļ���С�洢����,Ϊ�˲鿴����ŵ������ⲿ
char filename[80]={0};

// ��������
void sensor_config_1(char* LedvalueRED, char* LedvalueIR, u16 SampleRate, u8 Average, u16 PluseWidth, u16 ADCRange);
void sensor_config_2(char* LedvalueRED, char* LedvalueIR, u16 SampleRate, u8 Average, u16 PluseWidth, u16 ADCRange);
void init_config(void);
void loop(void);
void showfile(void);
void TIM3_IRQHandler(void);

void keytest(int value);
//  ������
int main(void) {
	
  	
    usmart_dev.init(84); 	//��ʼ��USMART
    init_config();
    delay_ms(100);
	sensor_config_1("2f","2f",800,2,69,16384);
	sensor_config_2("4f","4f",800,2,69,16384);
	
    while(1) {
        loop();
    }
   
    
}
void loop(void)
// ѭ���庯��,���ݷ��͸�SendBuff
{
    // ����Ƿ������ݽ���
    //printf("%s",filename);
    if(USART_RX_STA & 0x8000)
    {
        char* first = strtok((char*)USART_RX_BUF, ",");
        char* LedvalueRED = strtok(NULL, ",");
        char* LedvalueIR = strtok(NULL, ",");
        char* third = strtok(NULL, ",");
        char* four = strtok(NULL, ",");
        char* five = strtok(NULL, ",");
        char* six = strtok(NULL, ",");
        int sensor = atoi(first);
        int SampleRate = atoi(third);
        int Average = atoi(four);
        int PluseWidth = atoi(five);
        int ADCRange = atoi(six);
        if(strcmp(first,"1") == 0) {
            sensor_config_1(LedvalueRED, LedvalueIR, SampleRate, Average, PluseWidth, ADCRange);	
        }
        if(strcmp(first,"2") == 0) {
            sensor_config_2(LedvalueRED, LedvalueIR, SampleRate, Average, PluseWidth, ADCRange);
        }
        USART_RX_STA = 0;
    };

    // ��������
    //delay_ms(10);
    max30102_read_fifo(&red_1, &ir_1);
    max30102_read_fifo_2(&red_2, &ir_2);
    
    
    
//    if (flag == 0){
//     j=KEY_Scan(1);   //ɨ�谴��    
//    }
    
//    if(KEY_Scan(1)== 3){
//       flag ++; //�����ѱ�����
//    }
    //printf("Flag:%u \r\n",flag);
    //delay_ms(1000);
//    if(flag == 1){        
//       showfile();
//    }else if(flag == 2){
//      flag = 0;
//      k = 0;	    
//    }
    

		    if (KEY_Scan(1) == 3){
		    if(flag == 1){
			flag = 2;
			    }else{
			    flag = 1;
		     TIM3_Int_Init(50000-1,8400-1);	//��ʱ��ʱ��84M����Ƶϵ��8400������84M/8400=10Khz�ļ���Ƶ�ʣ�����5000��Ϊ500ms     
			    }
		    while(1){
//			if (j == 1000){
//				j = 0;
//			if(flag == 1){
//			flag = 2;
//			    }else{
//			    flag = 1;
//			    }
//				break;
//			}
//			j++;   
			keytest(flag);
			TIM3_IRQHandler();
			if (j == 2){
			    break;
			}
//			if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET){
//				j = 1;
//				keytest(flag);
//	                     showfile();
//			}
//			TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  //����жϱ�־λ
//			if(j == 1){
//				break;

//		    }

		}    
    
    
       t++;
	if((t%20)==0)	//ÿ100ms����һ����ʾ����
		{                  

		    //keytest(flag);			
			RTC_GetTime(RTC_Format_BIN,&RTC_TimeStruct);
			
			sprintf((char*)tbuf,"Time:%02d:%02d:%02d",RTC_TimeStruct.RTC_Hours,RTC_TimeStruct.RTC_Minutes,RTC_TimeStruct.RTC_Seconds); 
			LCD_ShowString(0,215,100,16,16,tbuf);	
			//printf("%s\r\n",tbuf);
			RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);
			
			sprintf((char*)tbuf,"Date:20%02d-%02d-%02d",RTC_DateStruct.RTC_Year,RTC_DateStruct.RTC_Month,RTC_DateStruct.RTC_Date); 
			LCD_ShowString(105,215,135,16,16,tbuf);	
			//printf("%s\r\n",tbuf);
			//sprintf((char*)tbuf,"Week:%d",RTC_DateStruct.RTC_WeekDay); 
			//LCD_ShowString(30,180,210,16,16,tbuf);
			//printf("%s\r\n",tbuf);
		}

   
}
		    }

void sensor_config_1(char* LedvalueRED, char* LedvalueIR, u16 SampleRate, u8 Average, u16 PluseWidth, u16 ADCRange)
{
    setLEDValueRED(LedvalueRED);
    setLEDValueIR(LedvalueIR);
    setSampleRate(SampleRate);
    setFIFOAverage(Average);
    setPulseWidth(PluseWidth);
    setADCRange(ADCRange);
}
void sensor_config_2(char* LedvalueRED, char* LedvalueIR, u16 SampleRate, u8 Average, u16 PluseWidth, u16 ADCRange)
{
    setLEDValueRED_2(LedvalueRED);
    setLEDValueIR_2(LedvalueIR);
    setSampleRate_2(SampleRate);
    setFIFOAverage_2(Average);
    setPulseWidth_2(PluseWidth);
    setADCRange_2(ADCRange);
}
void init_config()
// ��ʼ������  ������ͨѶ\max30102\max30102_2
{
    delay_init(168);
    //LED_init();
    IIC_Init();
    IIC_Init_2();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    uart_init(115200);
    max30102_reset();
    max30102_reset_2();
    max30102_read_reg(0, &uch_dummy);
    max30102_read_reg_2(0, &uch_dummy);
    max30102_init();
    max30102_init_2();

	
    //��ʼ��SD������
	
    	//LED_Init();					//��ʼ��LED 
	usmart_dev.init(84);		//��ʼ��USMART
 	//LCD_Init();					//LCD��ʼ��  
 	KEY_Init();					//������ʼ�� 
	W25QXX_Init();				//��ʼ��W25Q128
	my_mem_init(SRAMIN);		//��ʼ���ڲ��ڴ�� 
	my_mem_init(SRAMCCM);		//��ʼ��CCM�ڴ��
	
	exfuns_init();							//Ϊfatfs��ر��������ڴ�				 
 	s = f_mount(fs[0],"0:",1);
	printf("%d\r\n",s);
	
	LCD_Init();					//LCD��ʼ��  
 	
	LCD_Clear(WHITE);
	Display_ALIENTEK_LOGO(0, 0);
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	//LCD_ShowString(0, 70, 240, 32, 32, "Begin Test");
	//LCD_ShowChar(0, 140,'A',16);
	//sprintf(str,"Memory: %d",res);
	BACK_COLOR = RED;
	if(s == FR_NOT_READY){
	   LCD_ShowString(0, 60, 240, 32, 32,"Insert SDcard");
	}
	BACK_COLOR = WHITE;
//	}else if(res == FR_DISK_ERR){
//	    LCD_ShowString(0, 60, 240, 32, 32,"FR_DISK_ERR");
//	}
	    //LCD_ShowString(0, 60, 100, 32, 32,str);
       //LCD_ShowString(0, 60, 100, 32, 32,res);
	//POINT_COLOR = WHITE;
	//BACK_COLOR = BLUE;
	//LCD_ShowString(0, 170, 240, 16, 16, "��");
	//LCD_ShowString(0, 160, 240, 32, 32, "Writing data to");
      // LCD_ShowString(0, 192, 240, 32, 32, "SDcard:");
      // LCD_ShowString(0, 60, 100, 32, 32,"Memory:");
//       for(s=FR_OK;s<FR_INVALID_PARAMETER;s++){
//	    
//	}
	LCD_ShowString(0, 100, 100, 32, 32,"HR:");
	LCD_ShowString(0, 140, 100, 32, 32,"SPO2:");
	//LCD_ShowString(0, 180, 100, 32, 32,"BP:");
	
	
	
       //��SD��д����ʾ��Ϣ����������
//      res=f_open (&fil,"0:/message.txt", FA_WRITE|FA_OPEN_ALWAYS);	
//      res = f_lseek(&fil,fil.fsize);
//      for(b=0;b<10;b++){
//	    i = f_printf(&fil,"\r\n"); 
//      }	      
//      i = f_printf(&fil,"%s\r\n","д����ʾ�ɼ���Ϣ");
//      b=0;
      
       My_RTC_Init();    
}

void showfile(void){
	if(k == 0){
	   RTC_GetTime(RTC_Format_BIN,&RTC_TimeStruct);
          RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);
          sprintf(filename,"%02d_%02d_%02d_%02d_%02d.txt",RTC_DateStruct.RTC_Month,RTC_DateStruct.RTC_Date,RTC_TimeStruct.RTC_Hours,RTC_TimeStruct.RTC_Minutes,RTC_TimeStruct.RTC_Seconds); 
          res=f_open (&fil,filename, FA_CREATE_NEW|FA_OPEN_ALWAYS);
          f_close(&fil);
          sprintf(str,"name: %s",filename);
	   LCD_ShowString(0, 180, 240, 24, 24,str);	
	   k++;
	}
         
    if(b == 4000){
	    sprintf(str,"HR: %d",red_1);
	    LCD_ShowString(0, 100, 240, 32, 32,str);
	    sprintf(str,"SPO2: %d",red_2);
	    LCD_ShowString(0, 140, 100, 32, 32,str);
	    //sprintf(str,"BP: %d",ir_1);
	    //LCD_ShowString(0, 180, 100, 32, 32,str);
	    b = 0;
	}
      if(s != FR_NOT_READY){
      res=f_open (&fil,filename, FA_WRITE|FA_OPEN_ALWAYS);	
      res = f_lseek(&fil,fil.fsize);		
      i = f_printf(&fil,"%u,%u,%u,%u \r\n", red_1, ir_1, red_2, ir_2); //�洢����
       f_close(&fil);
	if(res == FR_OK){
	   // sprintf(str,"Memory: ONON");
	 //  LCD_ShowString(0, 60, 240, 32, 32,str);
	}	
	if(i == -1){
		//LCD_ShowString(0, 60, 240, 32, 32,"Memory: Failed");
	}else{
		//LCD_ShowString(0, 60, 240, 32, 32,"Memory:ONON");		
	}
     }	
      
}

 void keytest(int value){
	 if ( value == 1){
		 sprintf(str,"Saving");	 
	 }else if (value == 2){
	 sprintf(str,"finish");
	 }else{
		sprintf(str,"ready"); 
		 
	 }
         LCD_ShowString(0, 60, 240, 32, 32,str);
 }


void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET) //����ж�
	{
		j = 2;
		
	}
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  //����жϱ�־λ
}









