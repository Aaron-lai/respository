#include "myiic.h"
#include "delay.h"

/*
PB8，PB9代表第一路i2c接口的引脚
以下函数为第一路i2c函数
*/
void IIC_Init(void)
/*
初始化i2c第一路通道
返回值： 无
*/
{

    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    //GPIOB8,B9初始化
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    IIC_SCL=1;
    IIC_SDA=1;
}

void IIC_Start(void)
/*
启动i2c第一路通道
返回值： 无
*/
{
    SDA_OUT();     //sda输出模式
    IIC_SDA=1;
    IIC_SCL=1;
    delay_us(4);
    IIC_SDA=0;
    delay_us(4);
    IIC_SCL=0;
}
void IIC_Stop(void)
/*
停止i2c第一路通道
返回值： 无
*/
{
    SDA_OUT();
    IIC_SCL=0;
    IIC_SDA=0;
    delay_us(4);
    IIC_SCL=1;
    IIC_SDA=1;
    delay_us(4);
}

u8 IIC_Wait_Ack(void)
/*
IIC_Wait_Ack  i2c等待应答函数
返回值：  0  应答成功
		 1  应答失败
*/
{
    u8 ucErrTime=0;
    SDA_IN();
    IIC_SDA=1;
    delay_us(1);
    IIC_SCL=1;
    delay_us(1);
    while(READ_SDA)
    {
        ucErrTime++;
        if(ucErrTime>250)
        {
            IIC_Stop();
            return 1;
        }
    }
    IIC_SCL=0;
    return 0;
}
void IIC_Ack(void)
/*
IIC_Ack  i2c应答函数
返回值：  无
*/
{
    IIC_SCL=0;
    SDA_OUT();
    IIC_SDA=0;
    delay_us(2);
    IIC_SCL=1;
    delay_us(2);
    IIC_SCL=0;
}

void IIC_NAck(void)
/*
IIC_NAck  i2c不应答函数
返回值：  无
*/
{
    IIC_SCL=0;
    SDA_OUT();
    IIC_SDA=1;
    delay_us(2);
    IIC_SCL=1;
    delay_us(2);
    IIC_SCL=0;
}
void IIC_Send_Byte(u8 txd)
/*
IIC_Send_Byte  i2c发送数据函数

入口参数： u8 txd（发送一个字节数据）
返回值：   无
*/
{
    u8 t;
    SDA_OUT();
    IIC_SCL=0;
    for(t=0; t<8; t++)
    {
        IIC_SDA=(txd&0x80)>>7;
        txd<<=1;
        delay_us(2);
        IIC_SCL=1;
        delay_us(2);
        IIC_SCL=0;
        delay_us(2);
    }
}

u8 IIC_Read_Byte(void)
/*
IIC_Read_Byte  i2c读取数据函数

入口参数： 无
返回值：   unsigned char receive（发送一个字节数据）
*/
{
    unsigned char i,receive=0;
    SDA_IN();
    for(i=0; i<8; i++ )
    {
        IIC_SCL=0;
        delay_us(2);
        IIC_SCL=1;
        receive<<=1;
        if(READ_SDA)receive++;
        delay_us(1);
    }
    return receive;
}



/*
PB6，PB7代表第二路i2c接口的引脚
以下函数为第二路i2c函数
*/
void IIC_Init_2(void)
/*
初始化i2c第2路通道
返回值： 无
*/
{

    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    //GPIOB8,B9初始化
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    IIC_SCL_2=1;
    IIC_SDA_2=1;
}

void IIC_Start_2(void)
/*
启动i2c第2路通道
返回值： 无
*/
{
    SDA_OUT_2();     //sda输出模式
    IIC_SDA_2=1;
    IIC_SCL_2=1;
    delay_us(4);
    IIC_SDA_2=0;
    delay_us(4);
    IIC_SCL_2=0;
}
void IIC_Stop_2(void)
/*
停止i2c第2路通道
返回值： 无
*/
{
    SDA_OUT_2();
    IIC_SCL_2=0;
    IIC_SDA_2=0;
    delay_us(4);
    IIC_SCL_2=1;
    IIC_SDA_2=1;
    delay_us(4);
}

u8 IIC_Wait_Ack_2(void)
/*
IIC_Wait_Ack  i2c等待应答函数
返回值：  0  应答成功
		 1  应答失败
*/
{
    u8 ucErrTime=0;
    SDA_IN_2();
    IIC_SDA_2=1;
    delay_us(1);
    IIC_SCL_2=1;
    delay_us(1);
    while(READ_SDA_2)
    {
        ucErrTime++;
        if(ucErrTime>250)
        {
            IIC_Stop_2();
            return 1;
        }
    }
    IIC_SCL_2=0;
    return 0;
}
void IIC_Ack_2(void)
/*
IIC_Ack  i2c应答函数
返回值：  无
*/
{
    IIC_SCL_2=0;
    SDA_OUT_2();
    IIC_SDA_2=0;
    delay_us(2);
    IIC_SCL_2=1;
    delay_us(2);
    IIC_SCL_2=0;
}

void IIC_NAck_2(void)
/*
IIC_NAck  i2c不应答函数
返回值：  无
*/
{
    IIC_SCL_2=0;
    SDA_OUT_2();
    IIC_SDA_2=1;
    delay_us(2);
    IIC_SCL_2=1;
    delay_us(2);
    IIC_SCL_2=0;
}
void IIC_Send_Byte_2(u8 txd)
/*
IIC_Send_Byte  i2c发送数据函数

入口参数： u8 txd（发送一个字节数据）
返回值：   无
*/
{
    u8 t;
    SDA_OUT_2();
    IIC_SCL_2=0;
    for(t=0; t<8; t++)
    {
        IIC_SDA_2=(txd&0x80)>>7;
        txd<<=1;
        delay_us(2);
        IIC_SCL_2=1;
        delay_us(2);
        IIC_SCL_2=0;
        delay_us(2);
    }
}

u8 IIC_Read_Byte_2(void)
/*
IIC_Read_Byte  i2c读取数据函数

入口参数： 无
返回值：   unsigned char receive（发送一个字节数据）
*/
{
    unsigned char i,receive=0;
    SDA_IN_2();
    for(i=0; i<8; i++ )
    {
        IIC_SCL_2=0;
        delay_us(2);
        IIC_SCL_2=1;
        receive<<=1;
        if(READ_SDA_2)receive++;
        delay_us(1);
    }
    return receive;
}


























