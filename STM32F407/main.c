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
////ALIENTEK 探索者STM32F407开发板 实验39
////FATFS 实验 -库函数版本
////技术支持：www.openedv.com
////淘宝店铺：http://eboard.taobao.com
////广州市星翼电子科技有限公司    
////作者：正点原子 @ALIENTEK 
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

//SD卡所需头文件
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



// 红光[red]和红外光[ir] 缓存流  缓存数=sampNumble
uint32_t red_1;
uint32_t ir_1;
uint32_t red_2;
uint32_t ir_2;

uint32_t count; //缓存计数
uint8_t uch_dummy;
u8 tbuf[40];
u8 t=0;

//SD卡所需变量定义
FIL fil;
FRESULT res,s;
UINT bww;
uint32_t buf[4];
int i,j;
u8 flag = 0;//按键按下标志位
u8 k = 0;//文件生成标志位
int b = 0;
char str[20];
RTC_TimeTypeDef RTC_TimeStruct;
RTC_DateTypeDef RTC_DateStruct;
unsigned int FATFSNumSize;//文件大小存储变量,为了查看方便放到函数外部
char filename[80]={0};

// 声明函数
void sensor_config_1(char* LedvalueRED, char* LedvalueIR, u16 SampleRate, u8 Average, u16 PluseWidth, u16 ADCRange);
void sensor_config_2(char* LedvalueRED, char* LedvalueIR, u16 SampleRate, u8 Average, u16 PluseWidth, u16 ADCRange);
void init_config(void);
void loop(void);
void showfile(void);
void TIM3_IRQHandler(void);

void keytest(int value);
//  主函数
int main(void) {
	
  	
    usmart_dev.init(84); 	//初始化USMART
    init_config();
    delay_ms(100);
	sensor_config_1("2f","2f",800,2,69,16384);
	sensor_config_2("4f","4f",800,2,69,16384);
	
    while(1) {
        loop();
    }
   
    
}
void loop(void)
// 循环体函数,数据发送给SendBuff
{
    // 检测是否有数据接收
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

    // 发送数据
    //delay_ms(10);
    max30102_read_fifo(&red_1, &ir_1);
    max30102_read_fifo_2(&red_2, &ir_2);
    
    
    
//    if (flag == 0){
//     j=KEY_Scan(1);   //扫描按键    
//    }
    
//    if(KEY_Scan(1)== 3){
//       flag ++; //按键已被按下
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
		     TIM3_Int_Init(50000-1,8400-1);	//定时器时钟84M，分频系数8400，所以84M/8400=10Khz的计数频率，计数5000次为500ms     
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
//			TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  //清除中断标志位
//			if(j == 1){
//				break;

//		    }

		}    
    
    
       t++;
	if((t%20)==0)	//每100ms更新一次显示数据
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
// 初始化函数  ：串口通讯\max30102\max30102_2
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

	
    //初始化SD卡操作
	
    	//LED_Init();					//初始化LED 
	usmart_dev.init(84);		//初始化USMART
 	//LCD_Init();					//LCD初始化  
 	KEY_Init();					//按键初始化 
	W25QXX_Init();				//初始化W25Q128
	my_mem_init(SRAMIN);		//初始化内部内存池 
	my_mem_init(SRAMCCM);		//初始化CCM内存池
	
	exfuns_init();							//为fatfs相关变量申请内存				 
 	s = f_mount(fs[0],"0:",1);
	printf("%d\r\n",s);
	
	LCD_Init();					//LCD初始化  
 	
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
	//LCD_ShowString(0, 170, 240, 16, 16, "开");
	//LCD_ShowString(0, 160, 240, 32, 32, "Writing data to");
      // LCD_ShowString(0, 192, 240, 32, 32, "SDcard:");
      // LCD_ShowString(0, 60, 100, 32, 32,"Memory:");
//       for(s=FR_OK;s<FR_INVALID_PARAMETER;s++){
//	    
//	}
	LCD_ShowString(0, 100, 100, 32, 32,"HR:");
	LCD_ShowString(0, 140, 100, 32, 32,"SPO2:");
	//LCD_ShowString(0, 180, 100, 32, 32,"BP:");
	
	
	
       //往SD卡写入提示信息，方便区分
//      res=f_open (&fil,"0:/message.txt", FA_WRITE|FA_OPEN_ALWAYS);	
//      res = f_lseek(&fil,fil.fsize);
//      for(b=0;b<10;b++){
//	    i = f_printf(&fil,"\r\n"); 
//      }	      
//      i = f_printf(&fil,"%s\r\n","写入提示采集信息");
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
      i = f_printf(&fil,"%u,%u,%u,%u \r\n", red_1, ir_1, red_2, ir_2); //存储数据
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
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET) //溢出中断
	{
		j = 2;
		
	}
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  //清除中断标志位
}









