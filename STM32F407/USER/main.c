#include "sys.h"
#include "led.h"
#include "key.h"
#include "sram.h"
#include "malloc.h"
#include "sdio_sdcard.h"
#include "malloc.h"
#include "w25qxx.h"
#include "ff.h"
#include "exfuns.h"
#include "tftlcd.h"
#include "spi.h"
#include "timer.h"
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
#include "exti.h"


#define sampNumble 1600

u32 red_1;
u32 ir_1;
u32 red_2;
u32 ir_2;

u32 count;
uint8_t uch_dummy;//max30102参数
u8 tbuf[40];//缓存时间
u8 t = 0;  //

FIL fil;
FRESULT res,s,openfileres,createfileres;//s:内存卡是否挂载成功
DIR* dp;//文件夹指针
FILINFO* fno ;//文件信息结构体
char path1[10] = "0704";//更改常量path
const TCHAR* path =  path1;
char buffer[1024];//存储读入的数据 
char *bk=buffer;  //   

UINT bww;
u32 buf[4];
int i, j;//j:定时控制
u32 f = 0;//第一次画点标志位
u8 flag = 0; // 当前状态标志位 0 初始状态  1 保存中  2 保存完成
u8 k = 0;	//定时控制
int b = 0;    //均值滤波判断次数
char str[20];  //显示用
RTC_TimeTypeDef RTC_TimeStruct;
RTC_DateTypeDef RTC_DateStruct;
unsigned int FATFSNumSize;
char filename[80] = "07-05/06_09_50.txt";
u8 clock;
u16 index1 = 0,index2=0;//index1:测试运行次数

u32 ir11 = 0,ir12=0;//ir11：是否收到666 ir12:传递画图参数
u8 x=5; //画图x坐标
u32 sum1=0,sum2=0; //sum1:用于第一次进行均值滤波求和，并用作15个点判断标志位。sum2：非第一次均值滤波求总和
float a;         
u32 a_buf[60];  //均值滤波缓存数据，用于显示波形时和存储数据不显示波形时
u32 b_buf[60];   //均值滤波缓存数据，用于不显示波形时
u16 Yinit=100;  //画图数据参数
static u16 Ypos1 = 0,Ypos2 = 0,Ypre=0;//画图参数,Ypre三点中第一点，变化后Ypre未使用

u8 len;//串口接收数据长度
u8 str1[200]="666";//串口发送数据指令


static u8 maxx=0,maxx1=0;//最大值x,x1坐标,x1前一个点
static u8 minx=0,minx1=0;//最小值x,x1坐标，x1前一个点

u8 countmax=0;//最大值计数
u8 countmin=0;//最小值计数
u32 prepoint = 0;//三点中最先一点
u8 starttime = 0;//计算心率开始时间
u8 endtime = 0;//计算心率结束时间
u8 HR = 0 ;//心率

u8 command = 0;
void sensor_config_1(char *LedvalueRED, char *LedvalueIR, u16 SampleRate, u8 Average, u16 PluseWidth, u16 ADCRange);
void sensor_config_2(char *LedvalueRED, char *LedvalueIR, u16 SampleRate, u8 Average, u16 PluseWidth, u16 ADCRange);
void init_config(void); // 初始化配置
void loop(void); // 循环体
void showfile(void); // 保存文件
void TIM3_IRQHandler(void); // 定时器中断处理方法
void getSensorData(void); // 获取传感器数据
void printStatus(void); // 显示存储状态
void showtime(void); // 显示时间
void createfile(void);//创建文件
void showwave(void);
void Lcd_DrawNetwork(void);
void DrawOscillogram_Clear(u32 buff);
void clear_point(void);
void EXTI2_IRQHandler(void);
void makedir(void);
void readfile(void);
void reinit(void);
void DrawOscillogram_Clearfinger(u32 buff);
void flagcontrol(void);
void testwave(u32 buff);//存储数据时检测波形是否正确，不画波形
void averagefilter(void);//取得手掌处传感器数据，并进行判断
int main(void)
{	
	init_config();
	RTC_Set_WakeUp(RTC_WakeUpClock_CK_SPRE_16bits,0);		//配置WAKE UP中断,1秒钟中断一次
	delay_ms(100);
	// 给传感器初始值
	sensor_config_1("7f", "7f", 800, 2, 69, 16384);
	sensor_config_2("1f", "1f", 800, 2, 69, 16384);
	// 计时器5s 一次循环
	TIM3_Int_Init(50000 - 1, 8400 - 1);
	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq(&RCC_Clocks);
	clock = 0x11;
       clock = RCC_GetSYSCLKSource();
	Lcd_DrawNetwork();
	//Set_BackGround();
	while (1)
	{
		loop();
	}
}
/**
 * 获取传感器的值
 */
void getSensorData(void)
{
//	if (USART_RX_STA & 0x8000)
//	{
//		char *first = strtok((char *)USART_RX_BUF, ",");
//		char *LedvalueRED = strtok(NULL, ",");
//		char *LedvalueIR = strtok(NULL, ",");
//		char *third = strtok(NULL, ",");
//		char *four = strtok(NULL, ",");
//		char *five = strtok(NULL, ",");
//		char *six = strtok(NULL, ",");
//		int sensor = atoi(first);
//		int SampleRate = atoi(third);
//		int Average = atoi(four);
//		int PluseWidth = atoi(five);
//		int ADCRange = atoi(six);
//		if (strcmp(first, "1") == 0)
//		{
//			sensor_config_1(LedvalueRED, LedvalueIR, SampleRate, Average, PluseWidth, ADCRange);
//		}
//		if (strcmp(first, "2") == 0)
//		{
//			sensor_config_2(LedvalueRED, LedvalueIR, SampleRate, Average, PluseWidth, ADCRange);
//		}
//		USART_RX_STA = 0;
//	};

	max30102_read_fifo(&red_1, &ir_1);
	max30102_read_fifo_2(&red_2, &ir_2);
}

/**
 * 按键检测， 如果检测按下保存键：
 *   1. 定时器开始计时
 *   2. 开始保存文件
 *   3. 时间到了则退出  flag = 2
 * 
 */
void loop(void)
{
       //sprintf(filename, "20%02d_%02d_%02d_%02d_%02d", RTC_DateStruct.RTC_Month, RTC_DateStruct.RTC_Date, RTC_TimeStruct.RTC_Hours, RTC_TimeStruct.RTC_Minutes,RTC_TimeStruct.RTC_Seconds);
	//printf("%s \r\n",filename);
	showtime();
	// 判断是否按下 存储键
	openfileres = 999;
	createfileres = 999;	
	if (flag == 4 || flag == 5)
	{	
		while (1)
		{	 flagcontrol();	
			// 业务逻辑
			if(flag >=6 || flag == 0){
			  // sprintf(str,"flag11:%d",flag);
	                 //LCD_ShowString(0, 140, 240, 32, 32, str);
			  clear_point();	
	                 Lcd_DrawNetwork();
			   memset(USART_RX_BUF, 0, USART_REC_LEN);
			   break;
			}
                    		
			showwave();						
		}			
	}
	
	
	if(flag == 1){
		clear_point();
		Lcd_DrawNetwork();
	       j = 1;
		int start = j;
		k = 1;
		createfile();
		if(createfileres == FR_OK) openfileres = f_open(&fil, filename, FA_WRITE | FA_OPEN_ALWAYS);
	       while (1)
		{      
		    	getSensorData();
                    averagefilter();			
			showfile();
			showtime();
			//showwave();
			flagcontrol();
			// 到达10s 退出循环
			if (j - start > 6)
			{
				j = 0;
				flag = 2;
				//flag=0;
				break;
			}
			
			if(flag == 2){
			     //flag=0;
			     break;
			}
			
	       }
		if(openfileres == FR_OK)  f_close(&fil);	
	}
	
	
	
	if(flag >= 3){
	  reinit();	  
	  memset(USART_RX_BUF, 0, USART_REC_LEN);
	  flag = 0;
	  
	}
			
	   //LCD_ShowString(0, 180, 240, 32, 32, "USART");

		//printf("\r\nshujuwei:\r\n");
		if(ir11 == 1 && flag == 2){
	         openfileres = f_open(&fil, filename, FA_READ | FA_OPEN_ALWAYS);
		   while(1){			   
	           readfile();   
		    if(f_eof(&fil)==1  || fil.fptr - fil.fsize <128){
			 ir11=0;   
	               break;
	           }//读到末尾
//		    if ((index2 % 100) == 0){
//		         sprintf(str,"index2:%d",index2);
//		         LCD_ShowString(0, 90, 240, 32, 32, str);
//		    }
		   }
                 if(openfileres == FR_OK)  f_close(&fil);  		   
		}
	flagcontrol();
	
}
	

void sensor_config_1(char *LedvalueRED, char *LedvalueIR, u16 SampleRate, u8 Average, u16 PluseWidth, u16 ADCRange)
{
	setLEDValueRED(LedvalueRED);
	setLEDValueIR(LedvalueIR);
	setSampleRate(SampleRate);
	setFIFOAverage(Average);
	setPulseWidth(PluseWidth);
	setADCRange(ADCRange);
}
void sensor_config_2(char *LedvalueRED, char *LedvalueIR, u16 SampleRate, u8 Average, u16 PluseWidth, u16 ADCRange)
{
	setLEDValueRED_2(LedvalueRED);
	setLEDValueIR_2(LedvalueIR);
	setSampleRate_2(SampleRate);
	setFIFOAverage_2(Average);
	setPulseWidth_2(PluseWidth);
	setADCRange_2(ADCRange);
}
void init_config()
{
	delay_init(168);
	IIC_Init();
	IIC_Init_2();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	uart_init(9600);
	max30102_reset();
	max30102_reset_2();
	max30102_read_reg(0, &uch_dummy);
	max30102_read_reg_2(0, &uch_dummy);
	max30102_init();
	max30102_init_2();

       EXTIX_Init();
	KEY_Init();			
	W25QXX_Init();	
	my_mem_init(SRAMIN); 
	my_mem_init(SRAMCCM); 

	exfuns_init(); 
	s = f_mount(fs[0], "0:", 1);
	
	//printf("%d\r\n", s);
	LCD_Init(); 
	LCD_Clear(WHITE);
	//Display_ALIENTEK_LOGO(0, 0);
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	BACK_COLOR = RED;
	if (s == FR_NOT_READY)
	{
		LCD_ShowString(0, 60, 240, 32, 32, "Insert SDcard");
	}
	BACK_COLOR = WHITE;
       //showwave();
	//showxy();
	//LCD_ShowString(0, 100, 100, 32, 32, "HR:");
	//LCD_ShowString(0, 140, 100, 32, 32, "SPO2:");

	//sprintf(str,"init:%d",s);
	 //LCD_ShowString(0, 140, 240, 32, 32, str);
	My_RTC_Init();
}

void showfile(void)
{
	if (openfileres == FR_OK)
	{     
		res = f_lseek(&fil, fil.fsize);
		i = f_printf(&fil, "%u,%u,%u,%u\r\n", red_1, ir_1, red_2, ir_2);
		index1++;

	}	
}

void makedir(void){
	
	sprintf(path1, "%02d-%02d", RTC_DateStruct.RTC_Month, RTC_DateStruct.RTC_Date);

	//path = &str;
      //f_stat(path, fno);
	f_mkdir(path);
      //sprintf(str,"file: %s",path1);
      //LCD_ShowString(0,180, 240, 32, 32, str);
}

void printStatus(void)
{
	//sprintf(str, "j: %d", value);
	if (flag == 1)
	{       
		sprintf(str, "   Saving");
	}
	 if (flag == 2)
	{
		sprintf(str, "   Finish:%u",index1);
	}
	if(flag == 0)
	{       
		sprintf(str, "   Ready");
	}
	LCD_ShowString(0,30, 240, 32, 32, str);
}

void TIM3_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)
	{
		j++;
	}
	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
}

void showtime(void)
{
	t++;
	
	if ((t % 100) == 0)
	{
		printStatus();
		RTC_GetTime(RTC_Format_BIN, &RTC_TimeStruct);
		
		sprintf((char *)tbuf, "  20%02d-%02d-%02d  ", RTC_DateStruct.RTC_Year, RTC_DateStruct.RTC_Month, RTC_DateStruct.RTC_Date);

		
		LCD_ShowString(0, 215, 100, 16, 16, tbuf);

		RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);
              sprintf((char *)tbuf, "%02d:%02d:%02d", RTC_TimeStruct.RTC_Hours, RTC_TimeStruct.RTC_Minutes, RTC_TimeStruct.RTC_Seconds);
		
		LCD_ShowString(105, 215, 135, 16, 16, tbuf);

		
		//sprintf(str, " HR: %d", red_1);
		//LCD_ShowString(0, 100, 100, 32, 32, str);		
	     // sprintf(str, " SPO2: %d", red_2);
	      // LCD_ShowString(0, 140, 240, 32, 32, str);
	}
}


void createfile(void){
	makedir();
    if (k == 1)
	{
		RTC_GetTime(RTC_Format_BIN, &RTC_TimeStruct);
		RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);
		sprintf(filename, "%s/%02d_%02d_%02d.txt", path,RTC_TimeStruct.RTC_Hours, RTC_TimeStruct.RTC_Minutes,RTC_TimeStruct.RTC_Seconds);
		createfileres = f_open(&fil, filename, FA_CREATE_NEW | FA_OPEN_ALWAYS);
		f_close(&fil);
		LCD_ShowString(0, 180, 240, 24, 24, filename);
		k = 0;
	}
}


void showwave(void){ 
	             getSensorData();
	              b++;
	              if(flag == 4){
				ir12=red_1;//red_1手指上数据
			}
	              if(flag == 5){
				ir12=red_2;//red_2手掌上数据
			}
                     a_buf[b]=ir12; 	
			if(b == 60){
		
				for(k=0;k<60;k++){
				  sum1=sum1+a_buf[k];
				}
				ir12=sum1/60;
				x=x+2;
				if(flag == 4){
			           //sprintf(str,"finger:%d",Ypos2);
	                         LCD_ShowString(0, 0, 240, 24, 24, "finger");
				    
			          // DrawOscillogram_Clear(ir12);
			       }
				if(flag == 5){
				  LCD_ShowString(0, 0, 240, 24, 24, "plam");
			           //DrawOscillogram_Clear(ir12);
			       }
				//sum1=0;
				 DrawOscillogram_Clear(ir12);
				b=0;
                            //ir11 = ir12;				
		       }
			
			if(sum1!=0 && b==30){
			     sum1=0;
			     for(k=0;k<60;k++){
				  sum2=sum2+a_buf[k];
				}
			    
			     ir12=sum2/60;
			      x=x+2;

//				if(flag == 1){
			       DrawOscillogram_Clear(ir12);
//			       }
//				if(flag == 2){
//			           DrawOscillogram_Clearfinger(ir12);
//			       }
				 //ir11 = ir12;
				sum2=0;
			}
			
			if(x>=240){
			   x =1;
                     clear_point();
			   Lcd_DrawNetwork();
			
			if((countmax<=4 && countmin <= 4 )|| (countmax>23 && countmin>23  )){
			   clear_point();
			   Lcd_DrawNetwork();
			   if(flag == 4){
			   LCD_ShowString(0, 0, 240, 24, 24, "finger fail!!!!");
		          }
			   if(flag == 5){
			     LCD_ShowString(0, 0, 240, 24, 24, "plam fail!!!!");
			   } 
			   countmax=0;
			   countmin=0;
			}else{
			   clear_point();	
			    Lcd_DrawNetwork();
//			    if(flag == 1){
//			    LCD_ShowString(0, 0, 240, 24, 24, "finger!"); 
//		           }
			    if((endtime-starttime)>0){
			         HR=(60*(countmax-1))/(endtime-starttime);
			    }else{
			         HR=(60*(countmax-1))/(endtime+(60-starttime));
			    }
			    if(flag == 4){
			    sprintf(str,"finger!  HR:%d",HR);
			    LCD_ShowString(0, 0, 240, 24, 24, str);
			     if(HR <40 || HR>120){
			        LCD_ShowString(0, 0, 240, 24, 24, "finger fail!!!!");
			    }
			    }
			    if(flag == 5){
			    sprintf(str,"plam!  HR:%d",HR);
			    LCD_ShowString(0, 0, 240, 24, 24, str);
			    if(HR <40 || HR>120){
			        LCD_ShowString(0, 0, 240, 24, 24, "plam fail!!!!");
			    }
			    }			     
			   countmax=0;
			   countmin=0;
			    maxx=0;
			    minx=0;
			}				
			}
}


void Lcd_DrawNetwork(void)
{
	u16 index_y = 0;
	u16 index_x = 0;	
	
    //画列点	
	for(index_x = 50;index_x < 240;index_x += 50)
	{
		for(index_y = 0;index_y < 240;index_y += 4)
		{
			LCD_Draw_ColorPoint(index_x,index_y,0X534c);	
		}
	}
	//画行点
	for(index_y = 50;index_y < 240;index_y += 50)
	{
		for(index_x = 0;index_x < 240;index_x += 4)
		{
			LCD_Draw_ColorPoint(index_x,index_y,0X534c);	
		}
	}
   LCD_DrawLine_color (0 ,120 , 240 ,120,BROWN);//////十字叉
   LCD_DrawLine_color (120 ,000 , 120 ,240,BROWN);
}

void clear_point(void)//更新显示屏当前列
{
	LCD_Fill(0,0,240,240,WHITE);
}

void DrawOscillogram_Clear(u32 buff)//画波形图
{
	POINT_COLOR = BLACK;
	if(f == 0 ){ //f画点标志位
	    Ypos1=120;
		f=buff;
	}
       if(f != 0){
	   if(buff>=f){
             if(flag == 4){
	         Ypos2=Ypos2-(buff-f)/5;
	      }
             if(flag == 5){
	          Ypos2=Ypos2-(buff-f)*2;
	      }
                //Ypos2=Ypos2-(buff-f)*2;	      
               if(f<prepoint){//最小值               
                if(minx == 0){
		      minx = x;
		  }
                  //LCD_Fill(x-2,Ypos1-2,x+2,Ypos1+2,BLUE);
		     //取心跳范围30~150  每30个点做一次滤波   每次更新15个点
		    //条件1：(minx-minx1)>=22 && (minx-minx1) <106 ) 波峰或波谷间距离
		    //条件2：((minx-maxx)>11 && (minx-maxx)<53)) 波峰和波谷之间的距离
		    //条件3：(minx-maxx)<53)) || (minx-maxx)<0) 跨屏显示波峰和波谷间距离
		    //取心跳范围40~120  每60个点做一次滤波   每次更新30个点
		   // && ((minx-maxx)>5 && (minx-maxx)<26)
		  if((((minx-minx1)>12 && (minx-minx1) <=60 )) || (minx-maxx)<0) {
		       minx = x;  
		    
			       LCD_Fill(x-2,Ypos1-2,x+2,Ypos1+2,BLUE);
				  countmin++;
			  }
			 
		  }		  
			 //countmax++;
	      
		prepoint = f;
		f=buff;
	       minx1 = minx;
	     }else{
                if(flag == 4){
	         Ypos2=Ypos2+(f-buff)/5;
	      }
             if(flag == 5){
	          Ypos2=Ypos2+(f-buff)*2;
	      }		     	      
             //Ypos2=Ypos2+(f-buff)*2;
	      if(f>prepoint){//最大值
		   if(maxx == 0){
		      maxx = x;
		   }   	             
		       //LCD_Fill(x-2,Ypos1-2,x+2,Ypos1+2,RED);
		   // && ((maxx-minx)>5 && (maxx-minx)<26)
		   
		  if((((maxx-maxx1)>12 && (maxx-maxx1) <=60 )) || (maxx-minx)<0){
		     maxx = x;
                  
		     LCD_Fill(x-2,Ypos1-2,x+2,Ypos1+2,RED);
		     countmax++;
		     RTC_GetTime(RTC_Format_BIN, &RTC_TimeStruct);
		     if(countmax == 1){
		          starttime = RTC_TimeStruct.RTC_Seconds;
		     }
		     endtime = RTC_TimeStruct.RTC_Seconds;
		  }
		  //countmin++;
	      
      }
	      prepoint = f;
	      f=buff;	
             maxx1 = maxx;	      
	    }
		if(Ypos2 >235) Ypos2 =235; //超出范围不显示
	       if(Ypos2 <5) Ypos2 =5; //超出范围不显示
	       	                   	
  }
	
	//LCD_Draw_ColorPoint(x+2,Ypos2,RED);
       LCD_DrawLine(x,Ypos1,x+2,Ypos2);
	Ypre = Ypos1;
       Ypos1 = Ypos2;        	
}

/**
*   按键中断服务函数
*/
void EXTI2_IRQHandler(void){
	if(EXTI_GetITStatus(EXTI_Line2)==1){
        delay_ms(10);	//消抖
	 if(KEY_Scan(1) == 3){
	 flag++;
	 }
        
//	 sprintf(str,"flag:%d",flag);
//	 LCD_ShowString(0, 90, 240, 24, 24, str);
	}
	EXTI_ClearITPendingBit(EXTI_Line2);//清除LINE2上的中断标志位 
}


void readfile(void){
       res=f_read(&fil,bk,1024 ,&br); //一次读一个字节直到读完全部文件信息
       //f_gets(buffer,20,&fil);
	//bk++;
       if(res==FR_OK)
       {
           printf("%s",buffer);
        }
       else
        {
             printf("\r\n f_read()fail..\r\n");
        }
 }

void reinit(void){
      clear_point();
	Lcd_DrawNetwork();
       index1 = 0,index2=0;
	ir11 = 0,ir12=0;
	sum1=0,sum2=0;
	x=5;
	Ypos1 = 0,Ypos2 = 0;

}

/**
* 蓝牙控制flag
*/

void flagcontrol(void){
	if(USART_RX_STA&0x8000){					   
			len=USART_RX_STA&0x3fff;//得到此次接收到的数据长度
			//printf("\r\nshujuwei:\r\n");
			if(strcmp((char*)USART_RX_BUF,(char*)str1)==0 ){					
			    ir11=1;
				printf("\r\n\r\n shujuwei:");//插入换行
			        for(t=0;t<len;t++)
			{
				USART_SendData(USART1, USART_RX_BUF[t]);         //向串口1发送数据
				while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
			}
			printf("\r\n\r\n");//插入换行
			       memset(USART_RX_BUF, 0, USART_REC_LEN);
			    USART_RX_STA=0;
			    command =1;
			    LCD_ShowString(0, 180, 240, 32, 32, "rec:666");
			    printf("%s","rec:666");
			}

 		        if(strcmp((char*)USART_RX_BUF,(char*)"start")==0 ){
                         flag = 0;	
                          printf("\r\n\r\n shujuwei:");//插入换行
			        for(t=0;t<len;t++)
			{
				USART_SendData(USART1, USART_RX_BUF[t]);         //向串口1发送数据
				while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
			}
			printf("\r\n\r\n");//插入换行
                         memset(USART_RX_BUF, 0, USART_REC_LEN);			
                         USART_RX_STA=0;
			    command =1;
                         LCD_ShowString(0, 180, 240, 32, 32, "rec:start");
				printf("%s","rec:start:");
                         printf("%d\r\n",flag);				
			}

			   if(strcmp((char*)USART_RX_BUF,(char*)"+1")==0 ){				
			    flag++;
			    if(flag>=3){
			       flag = 0;
				reinit();
			    }
			    clear_point();
	                  Lcd_DrawNetwork();
			    printf("\r\n\r\n shujuwei:");//插入换行
			        for(t=0;t<len;t++)
			{
				USART_SendData(USART1, USART_RX_BUF[t]);         //向串口1发送数据
				while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
			}
			printf("\r\n\r\n");//插入换行
			    memset(USART_RX_BUF, 0, USART_REC_LEN);
			    USART_RX_STA=0;
			    command =1;
			LCD_ShowString(0, 180, 240, 32, 32, "rec:+1");
			printf("%s","rec:+1");
			     printf("%d\r\n",flag);		
			}
			  
			  if(strcmp((char*)USART_RX_BUF,(char*)"finger")==0 ){
                         flag = 4;	
                         printf("\r\n\r\n shujuwei:");//插入换行
			        for(t=0;t<len;t++)
			{
				USART_SendData(USART1, USART_RX_BUF[t]);         //向串口1发送数据
				while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
			}
			printf("\r\n\r\n");//插入换行				  
                         USART_RX_STA=0;
			     memset(USART_RX_BUF, 0, USART_REC_LEN);
			    command =1;
                         LCD_ShowString(0, 180, 240, 32, 32, "rec:finger");
                         printf("%s","rec:finger");
                          printf("%d\r\n",flag);					  
			  }
			  
			   if(strcmp((char*)USART_RX_BUF,(char*)"plam")==0 ){
                         flag = 5;	
                        printf("\r\n\r\n shujuwei:");//插入换行
			        for(t=0;t<len;t++)
			{
				USART_SendData(USART1, USART_RX_BUF[t]);         //向串口1发送数据
				while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
			}
			printf("\r\n\r\n");//插入换行	
                          memset(USART_RX_BUF, 0, USART_REC_LEN);			
                         USART_RX_STA=0;
				   command =1;
                          LCD_ShowString(0, 180, 240, 32, 32, "rec:plam");	
                          printf("%s","rec:plam"); 
                           printf("%d\r\n",flag);					   
			  }
			   
			     if(strcmp((char*)USART_RX_BUF,(char*)"exit")==0 ){
                         flag = 0;	
                        printf("\r\n\r\n shujuwei:");//插入换行
			        for(t=0;t<len;t++)
			{
				USART_SendData(USART1, USART_RX_BUF[t]);         //向串口1发送数据
				while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
			}
			printf("\r\n\r\n");//插入换行
                          memset(USART_RX_BUF, 0, USART_REC_LEN);			
                         USART_RX_STA=0;
				     command =1;
                          LCD_ShowString(0, 180, 240, 32, 32, "rec:exit");
				   printf("%s","rec:exit:"); 
                              printf("%d\r\n",flag);					     
			  }
			   if(command == 0){	
                          printf("\r\n\r\n shujuwei:");//插入换行
			        for(t=0;t<len;t++)
			{
				USART_SendData(USART1, USART_RX_BUF[t]);         //向串口1发送数据
				while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
			}
			printf("\r\n\r\n");//插入换行
                           memset(USART_RX_BUF, 0, USART_REC_LEN);			
                         USART_RX_STA=0;
                          LCD_ShowString(0, 180, 240, 32, 32, "rec:commamdwrong");
			     printf("%s","commandwrong:"); 
                          printf("%d\r\n",flag);
			   }
		  USART_RX_STA=0;
	         command = 0;
		}

}

/**
* 存储数据时检测波形是否正确
*/

void testwave(u32 buff){
   	if(f == 0 ){ //f确定是否初始化
		f=buff;
	}
       if(f != 0){
	   if(buff>=f){	      	      
               if(f<prepoint){//最小值               
                if(minx == 0){
		      minx = x;
		  }
		  if((((minx-minx1)>12 && (minx-minx1) <=60 )) || (minx-maxx)<0) {
		       minx = x;  
		        countmin++;
			  }
			 
		  }		  
			 //countmax++;
	      
		prepoint = f;
		f=buff;
	       minx1 = minx;
	     }else{
	      if(f>prepoint){//最大值
		   if(maxx == 0){
		      maxx = x;
		   }   	             
		  if((((maxx-maxx1)>12 && (maxx-maxx1) <=60 )) || (maxx-minx)<0){
		     maxx = x;                 
		     countmax++;
		     RTC_GetTime(RTC_Format_BIN, &RTC_TimeStruct);
		     if(countmax == 1){
		          starttime = RTC_TimeStruct.RTC_Seconds;
		     }
		     endtime = RTC_TimeStruct.RTC_Seconds;
		  }
		  //countmin++;
	      
      }
	      prepoint = f;
	      f=buff;	
             maxx1 = maxx;	      
	    }
	       	                   	
     }
}

/**
*   取得手掌处传感器数值并进行均值滤波,并判断数据是否有误，有误则显示，
*/
 void averagefilter(void){
	          b++;
	          ir12=red_2;//red_2手掌上数据
                     a_buf[b]=ir12;	
			if(b == 60){
		
				for(k=0;k<60;k++){
				  sum1=sum1+a_buf[k];
				}
				ir12=sum1/60;
				x=x+2;
				testwave(ir12);
				b=0;			
		       }
			if(sum1!=0 && b==30){
			     sum1=0;
			     for(k=0;k<60;k++){
				  sum2=sum2+a_buf[k];
				}
			    
			     ir12=sum2/60;
			      x=x+2;
			      testwave(ir12);
			      sum2=0;
			}
			
			//判断是否正确
			if(x>=240){
			   x =1;                			
			if((countmax<6 && countmin < 6 )|| (countmax>18 && countmin>18  )){			  
		        //波形错误
			   BACK_COLOR = RED;
                        LCD_ShowString(0, 60, 240, 32, 32, "plam fail!!!!");	
                        BACK_COLOR = WHITE;			   
			   countmax=0;
			   countmin=0;
			}else{
			    if((endtime-starttime)>0){
			         HR=(60*(countmax-1))/(endtime-starttime);
			    }else{
			         HR=(60*(countmax-1))/(endtime+(60-starttime));
			    }
			    if(HR <40 || HR>120){
			   BACK_COLOR = RED;
                        LCD_ShowString(0, 60, 240, 32, 32, "plam fail!!!!");	
		          f_printf(&fil, "%s \r\n", "plam fail!!!!");
                        BACK_COLOR = WHITE;	
			    }else{
			       showtime();
				 sprintf(str,"plam!  HR:%d",HR);
			    LCD_ShowString(0, 0, 240, 24, 24, str);
				    LCD_Fill(0,60,240,92,WHITE);
				
			    }
			    }
			   countmax=0;
			   countmin=0;
			    maxx=0;
			    minx=0;				
			}
}
