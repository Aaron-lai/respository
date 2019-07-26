#ifndef __MYIIC_H
#define __MYIIC_H
#include "sys.h"


//IO初始化配置
#define SDA_IN()  {GPIOB->MODER&=~(3<<(9*2));GPIOB->MODER|=0<<9*2;}	//PB9输入模式
#define SDA_OUT() {GPIOB->MODER&=~(3<<(9*2));GPIOB->MODER|=1<<9*2;} //PB9输出模式
#define SDA_IN_2()  {GPIOB->MODER&=~(3<<(11*2));GPIOB->MODER|=0<<11*2;}	//PB7输入模式
#define SDA_OUT_2() {GPIOB->MODER&=~(3<<(11*2));GPIOB->MODER|=1<<11*2;} //PB7输出模式
//IO引脚
#define IIC_SCL    PBout(8) //SCL
#define IIC_SDA    PBout(9) //SDA	 
#define READ_SDA   PBin(9)  //读模式
#define IIC_SCL_2    PBout(10) //SCL
#define IIC_SDA_2    PBout(11) //SDA	 
#define READ_SDA_2   PBin(11)  //读模式

//IIC函数  PB8 PB9 所代表的第一路i2c通道
void IIC_Init(void);                //初始化函数，配置i0口
void IIC_Start(void);				//发送 启动i2c信号
void IIC_Stop(void);	  			//发送 停止i2c信号
void IIC_Send_Byte(u8 txd);			//IIC发送数据信号
u8 IIC_Read_Byte(void);//IIC读入数据信号
u8 IIC_Wait_Ack(void); 				//IIC等待应答
void IIC_Ack(void);					//IIC·发送应答
void IIC_NAck(void);				//IIC·发送不应答

//IIC函数  PB6 PB7 所代表的第二路i2c通道
void IIC_Init_2(void);                //初始化函数，配置i0口
void IIC_Start_2(void);				//发送 启动i2c信号
void IIC_Stop_2(void);	  			//发送 停止i2c信号
void IIC_Send_Byte_2(u8 txd);			//IIC发送数据信号
u8 IIC_Read_Byte_2(void);//IIC读入数据信号
u8 IIC_Wait_Ack_2(void); 				//IIC等待应答
void IIC_Ack_2(void);					//IIC·发送应答
void IIC_NAck_2(void);				//IIC·发送不应答



void IIC_Write_One_Byte(u8 daddr,u8 addr,u8 data);
u8 IIC_Read_One_Byte(u8 daddr,u8 addr);
#endif
















