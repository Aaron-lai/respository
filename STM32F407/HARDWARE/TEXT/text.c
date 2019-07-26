//#include "text.h"
//#include "tftlcd.h"
////////////////////////////////////////////////////////////////////////////////////	 
////本程序只供学习使用，未经作者许可，不得用于其它任何用途
////ALIENTEK Mini STM32开发板
////OLED汉字显示驱动代码
////驱动方式:8080并口/4线串口
////正点原子@ALIENTEK
////技术论坛:www.openedv.com
////修改日期:2014/03/25
////版本：V1.0
////版权所有，盗版必究。
////Copyright(C) 广州市星翼电子科技有限公司 2009-2019
////All rights reserved	   
//////////////////////////////////////////////////////////////////////////////////// 	


////24*24的OLED汉字点阵，包括三个汉字：开、发、板。这三个汉字的点阵库，是利用PCtoLCD2002生产的，
////软件设置的取模方式同OLED实验ASCII的取模方式一模一样，字体采用24*24大小，一个汉字点阵占用72字节。
//const u8 OLED_HZK_TBL[3][72]=
//{
//{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x08,0x08,0x02,0x08,0x08,0x04,0x08,0x08,0x08,0x08,0x08,0x10,0x08,0x08,0xE0,
//0x0F,0xFF,0x80,0x08,0x08,0x00,0x08,0x08,0x00,0x08,0x08,0x00,0x08,0x08,0x00,0x08,0x08,0x00,0x08,0x08,0x00,0x0F,0xFF,0xFE,
//0x0F,0xFF,0xFE,0x08,0x08,0x00,0x08,0x08,0x00,0x10,0x08,0x00,0x10,0x10,0x00,0x00,0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x00},/*"开",0*/
//	
//{0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x04,0x01,0x80,0x08,0x0F,0x00,0x10,0x1C,0x80,0x60,0x00,0x81,0xC2,0x00,0x87,0x02,
//0x00,0xFC,0x02,0x1F,0xF0,0x04,0x3C,0x9C,0x08,0x20,0x92,0x08,0x00,0x91,0x90,0x00,0x90,0x70,0x00,0x90,0x60,0x20,0x91,0x90,
//0x10,0x93,0x18,0x1C,0x9C,0x08,0x0C,0x98,0x0C,0x00,0x80,0x04,0x01,0x00,0x06,0x01,0x00,0x04,0x00,0x00,0x04,0x00,0x00,0x00},/*"发",1*/
//	
//{0x00,0x00,0x00,0x00,0x00,0xC0,0x01,0x01,0x00,0x01,0x0E,0x00,0x01,0xF8,0x00,0x3F,0xFF,0xFE,0x21,0x20,0x00,0x01,0x10,0x02,
//0x02,0x0C,0x0E,0x00,0x00,0x12,0x00,0x07,0xE4,0x0F,0xFE,0x04,0x08,0x40,0x08,0x08,0x78,0x10,0x08,0x47,0x30,0x08,0x41,0xE0,
//0x10,0x41,0xE0,0x10,0x47,0x30,0x30,0x7C,0x18,0x30,0xE0,0x0C,0x10,0x00,0x0C,0x00,0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x00},/*"板",2*/
//};	
//	 
////在指定位置，显示一个24*24大小的汉字
////x,y :汉字的坐标
////fnum:汉字编号（在字库数组里面的编号） 
//void OLED_Show_Font(u16 x,u16 y,u8 fnum)
//{
//	u8 temp,t,t1;
//	u16 y0=y;
//	u8 *dzk;   
//	u8 csize=72;					//一个24*24的汉字72字节
//	dzk=(u8*)OLED_HZK_TBL[fnum];	//得到汉字编号对应的点阵库 
//	for(t=0;t<csize;t++)
//	{   												   
//		temp=dzk[t];				//得到点阵数据                          
//		for(t1=0;t1<8;t1++)
//		{
//			if(temp&0x80)OLED_DrawPoint(x,y,1);
//			else OLED_DrawPoint(x,y,0); 
//			temp<<=1;
//			y++;
//			if((y-y0)==24)
//			{
//				y=y0;
//				x++;
//				break;
//			}
//		}  	 
//	}  
//}