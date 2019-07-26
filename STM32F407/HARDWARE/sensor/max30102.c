#include "myiic.h"
#include "max30102.h"
#include "usart.h"
#include <stdlib.h>

// MAX30102 控制指令
//寄存器地址
// FIFO Registers
static const uint8_t MAX30102_FIFOWRITEPTR =    0x04;
static const uint8_t MAX30102_FIFOREADPTR =     0x06;
static const uint8_t MAX30102_PARTICLECONFIG = 0x0A;//SpO2配置寄存器
static const uint8_t MAX30102_FIFOCONFIG = 0x08;//fifo配置寄存器
//采样平均处理  可选参数：1,2,4,8,16,32
static const uint8_t MAX30102_SAMPLEAVG_MASK =  0xE0;
static const uint8_t MAX30102_SAMPLEAVG_1 =     0x00;
static const uint8_t MAX30102_SAMPLEAVG_2 =     0x20;
static const uint8_t MAX30102_SAMPLEAVG_4 =     0x40;
static const uint8_t MAX30102_SAMPLEAVG_8 =     0x60;
static const uint8_t MAX30102_SAMPLEAVG_16 =    0x80;
static const uint8_t MAX30102_SAMPLEAVG_32 =    0xA0;
// AD采样范围控制    可选参数：2048/4096/8192/16384
static const uint8_t MAX30102_ADCRANGE_MASK =   0x9F;
static const uint8_t MAX30102_ADCRANGE_2048 =   0x00;
static const uint8_t MAX30102_ADCRANGE_4096 =   0x20;
static const uint8_t MAX30102_ADCRANGE_8192 =   0x40;
static const uint8_t MAX30102_ADCRANGE_16384 =  0x60;
// 采样频率控制    可选参数：50/100/200/400/800/1000/1600/3200
static const uint8_t MAX30102_SAMPLERATE_MASK = 0xE3;
static const uint8_t MAX30102_SAMPLERATE_50 =   0x00;
static const uint8_t MAX30102_SAMPLERATE_100 =  0x04;
static const uint8_t MAX30102_SAMPLERATE_200 =  0x08;
static const uint8_t MAX30102_SAMPLERATE_400 =  0x0C;
static const uint8_t MAX30102_SAMPLERATE_800 =  0x10;
static const uint8_t MAX30102_SAMPLERATE_1000 = 0x14;
static const uint8_t MAX30102_SAMPLERATE_1600 = 0x18;
static const uint8_t MAX30102_SAMPLERATE_3200 = 0x1C;
// LED宽度控制    可选参数：69/118/215/411
static const uint8_t MAX30102_PULSEWIDTH_MASK = 0xFC;
static const uint8_t MAX30102_PULSEWIDTH_69 =   0x00;
static const uint8_t MAX30102_PULSEWIDTH_118 =  0x01;
static const uint8_t MAX30102_PULSEWIDTH_215 =  0x02;
static const uint8_t MAX30102_PULSEWIDTH_411 =  0x03;

/*

* 第1个传感器 操作函数

*/
bool max30102_write_reg(uint8_t uch_addr, uint8_t uch_data)
/**
* \简述        发送一个值给MAX30102上的某个寄存器
*
* \输入参数    uch_addr    -  寄存器地址
* \输入参数    uch_data    -  发送的数据
*
* \返回值      发送成功 True
*             发送失败  False
*/
{
    // 启动第1路i2c
    IIC_Start();
    // 发送i2c写入地址
    IIC_Send_Byte(I2C_WRITE_ADDR);
    if (IIC_Wait_Ack() != 0)
    {
        goto cmd_fail;
    }
    // 发送寄存器地址
    IIC_Send_Byte(uch_addr);
    if (IIC_Wait_Ack() != 0)
    {
        goto cmd_fail;
    }
    // 发送数据
    IIC_Send_Byte(uch_data);
    if (IIC_Wait_Ack() != 0)
    {
        goto cmd_fail;
    }
    // 停止i2c
    IIC_Stop();
    return true;

cmd_fail:

    IIC_Stop();
    return false;
}
bool max30102_read_reg(uint8_t uch_addr, uint8_t *puch_data)
/**
* \简述        读取MAX30102上的某个寄存器
*
* \输入参数     uch_addr       -  寄存器地址
* \输出参数    *puch_data    -  发送的数据
*
* \返回值      发送成功 True
*             发送失败  False
*/
{
    // 启动i2c
    IIC_Start();
    // 发送i2c写入地址·
    IIC_Send_Byte(I2C_WRITE_ADDR);
    if (IIC_Wait_Ack() != 0)
    {
        goto cmd_fail;
    }
    // 发送寄存器地址·
    IIC_Send_Byte((uint8_t)uch_addr);
    if (IIC_Wait_Ack() != 0)
    {
        goto cmd_fail;
    }
    // 重启i2c
    IIC_Start();
    // 发送i2c读取地址
    IIC_Send_Byte(I2C_READ_ADDR);
    if (IIC_Wait_Ack() != 0)
    {
        goto cmd_fail;
    }
    *puch_data = IIC_Read_Byte();
    IIC_NAck();
    IIC_Stop();
    return true;
cmd_fail:
    IIC_Stop();
    return false;
}

bool max30102_init(void)
/**
* \简述        初始化MAX30102
*
* \输入参数     None
*
* \返回值      发送成功 True
*             发送失败  False
*/
{
    if(!max30102_write_reg(REG_INTR_ENABLE_1, 0xc0)) // INTR setting
        return false;
    if(!max30102_write_reg(REG_INTR_ENABLE_2, 0x00))
        return false;
    if(!max30102_write_reg(REG_FIFO_WR_PTR, 0x00)) //FIFO_WR_PTR[4:0]
        return false;
    if(!max30102_write_reg(REG_OVF_COUNTER, 0x00)) //OVF_COUNTER[4:0]
        return false;
    if(!max30102_write_reg(REG_FIFO_RD_PTR, 0x00)) //FIFO_RD_PTR[4:0]
        return false;
    if(!max30102_write_reg(REG_FIFO_CONFIG, 0x0f)) //sample avg = 1, fifo rollover=false, fifo almost full = 17
        return false;
    if(!max30102_write_reg(REG_MODE_CONFIG, 0x03))  //0x02 for Red only, 0x03 for SpO2 mode 0x07 multimode LED
        return false;
    if(!max30102_write_reg(REG_SPO2_CONFIG, 0x38)) // SPO2_ADC range = 4096nA, SPO2 sample rate (1600 Hz), LED pulseWidth (69uS)
        return false;

    if(!max30102_write_reg(REG_LED1_PA, 0x7f))  //Choose value for ~ 6.4mA for LED1
        return false;
    if(!max30102_write_reg(REG_LED2_PA, 0x8f))  // Choose value for ~ 6.4mA for LED2
        return false;
    if(!max30102_write_reg(REG_PILOT_PA, 0x7f))  // Choose value for ~ 25mA for Pilot LED
        return false;
    return true;
}

bool max30102_read_fifo(uint32_t *pun_red_led, uint32_t *pun_ir_led)

/**
* \简述        读取MAX30102上FIFO寄存器的采样值，保存到两个指针中
*
* \输出参数    *pun_red_led       -  红光数据
* \输出参数    *pun_ir_led        -  红外光数据
*
* \返回值      发送成功 True
*             发送失败  False
*/
{
    uint8_t samp;
    max30102_read_reg(0x0A,&samp);
    //printf("fifo status  %x \r\n",samp);
    uint32_t un_temp;
    uint32_t red;
    uint32_t ir;
    unsigned char uch_temp;
    *pun_ir_led = 0;
    *pun_red_led = 0;
    max30102_read_reg(REG_INTR_STATUS_1, &uch_temp);
    max30102_read_reg(REG_INTR_STATUS_2, &uch_temp);
    
    IIC_Start();
    IIC_Send_Byte(I2C_WRITE_ADDR);
    if (IIC_Wait_Ack() != 0)
    {
        //printf("read fifo failed");
        goto cmd_fail;
    }
    IIC_Send_Byte((uint8_t)REG_FIFO_DATA);
    if (IIC_Wait_Ack() != 0)
    {
		//printf("read fifo failed");
        goto cmd_fail;
    }
    IIC_Start();
    IIC_Send_Byte(I2C_READ_ADDR);
    if (IIC_Wait_Ack() != 0)
    {
		//printf("read fifo failed");
        goto cmd_fail;
    }
    un_temp = IIC_Read_Byte();
    IIC_Ack();
    un_temp <<= 16;
    red = un_temp;
    un_temp = IIC_Read_Byte();
    IIC_Ack();
    un_temp <<= 8;
    red += un_temp;
    un_temp = IIC_Read_Byte();
    IIC_Ack();

    red += un_temp;
    red &= 0x03FFFF;
    *pun_red_led = red;

    un_temp = IIC_Read_Byte();
    IIC_Ack();
    un_temp <<= 16;
    ir = un_temp;
    un_temp = IIC_Read_Byte();
    IIC_Ack();
    un_temp <<= 8;
    ir += un_temp;
    un_temp = IIC_Read_Byte();
    IIC_Ack();
    ir += un_temp;
    ir &= 0x03FFFF;
    *pun_ir_led = ir;
		
		
   IIC_Stop();
    return true;
cmd_fail:
    //printf("fail !!!");
   IIC_Stop();
    return false;
}

bool max30102_reset()
/**
* \简述        重启MAX30102
*
* \输出参数    None
* \返回值      发送成功 True
*             发送失败  False
*/
{
    if(!max30102_write_reg(REG_MODE_CONFIG, 0x40))
        return false;
    else
        return true;
}

void bitMask(uint8_t reg, uint8_t mask, uint8_t thing)
/**
* \简述        寄存器赋值函数
*
* \输出参数    reg：寄存器地址
              mask：操作位mask值
              thing：赋值位内容
* \返回值      无
*/
{
    //printf("bitMask   \r\n");
    uint8_t originalContents;
    max30102_read_reg(reg, &originalContents);
    originalContents = originalContents & mask;
    max30102_write_reg(reg, originalContents | thing);

}
void setLEDValueRED(char* ledValue)
	/**
* \简述        设置LED电流值
*
* \输出参数    LedValue：（0-255）

* \返回值      无
*/
{
	max30102_write_reg(REG_LED1_PA, StringToInt(ledValue));
	max30102_write_reg(REG_LED1_PA, StringToInt(ledValue));
}
void setLEDValueIR(char* ledValue)
	/**
* \简述        设置LED电流值
*
* \输出参数    LedValue：（0-255）

* \返回值      无
*/
{
	max30102_write_reg(REG_LED2_PA, StringToInt(ledValue));
}

void setSampleRate(uint16_t sampleRate)
/**
* \简述        设置采样频率
*
* \输出参数    sampleRate：采样值（50/100/200/400/800/1000/1600/3200）

* \返回值      无
*/
{
    uint8_t samp;
    max30102_read_reg(0x0A,&samp);
    //printf("start status  %x \r\n",samp);
    if (sampleRate < 100) bitMask(MAX30102_PARTICLECONFIG, MAX30102_SAMPLERATE_MASK,MAX30102_SAMPLERATE_50); //Take 50 samples per second
    else if (sampleRate < 200) bitMask(MAX30102_PARTICLECONFIG, MAX30102_SAMPLERATE_MASK,MAX30102_SAMPLERATE_100);
    else if (sampleRate < 400) bitMask(MAX30102_PARTICLECONFIG, MAX30102_SAMPLERATE_MASK,MAX30102_SAMPLERATE_200);
    else if (sampleRate < 800) bitMask(MAX30102_PARTICLECONFIG, MAX30102_SAMPLERATE_MASK,MAX30102_SAMPLERATE_400);
    else if (sampleRate < 1000) bitMask(MAX30102_PARTICLECONFIG, MAX30102_SAMPLERATE_MASK,MAX30102_SAMPLERATE_800);
    else if (sampleRate < 1600) bitMask(MAX30102_PARTICLECONFIG, MAX30102_SAMPLERATE_MASK,MAX30102_SAMPLERATE_1000);
    else if (sampleRate < 3200) bitMask(MAX30102_PARTICLECONFIG, MAX30102_SAMPLERATE_MASK,MAX30102_SAMPLERATE_1600);
    else if (sampleRate == 3200) bitMask(MAX30102_PARTICLECONFIG, MAX30102_SAMPLERATE_MASK,MAX30102_SAMPLERATE_3200);
    else bitMask(MAX30102_PARTICLECONFIG, MAX30102_SAMPLERATE_MASK,MAX30102_SAMPLERATE_50);
    //max30102_read_reg(0x0A,&samp);
    //printf("after status %x  \r\n", samp);
}
void setFIFOAverage(uint8_t sampleAverage)
/**
* \简述        设置采样频均值
*
* \输出参数    sampless：均值（1/2/4/8/16/32）

* \返回值      无
*/
{
  uint8_t samp;
  max30102_read_reg(MAX30102_FIFOCONFIG,&samp);
  if (sampleAverage == 1) bitMask(MAX30102_FIFOCONFIG, MAX30102_SAMPLEAVG_MASK,MAX30102_SAMPLEAVG_1);
  else if (sampleAverage == 2) bitMask(MAX30102_FIFOCONFIG, MAX30102_SAMPLEAVG_MASK,MAX30102_SAMPLEAVG_2);
  else if (sampleAverage == 4) bitMask(MAX30102_FIFOCONFIG, MAX30102_SAMPLEAVG_MASK,MAX30102_SAMPLEAVG_4);
  else if (sampleAverage == 8) bitMask(MAX30102_FIFOCONFIG, MAX30102_SAMPLEAVG_MASK,MAX30102_SAMPLEAVG_8);
  else if (sampleAverage == 16) bitMask(MAX30102_FIFOCONFIG, MAX30102_SAMPLEAVG_MASK,MAX30102_SAMPLEAVG_16);
  else if (sampleAverage == 32) bitMask(MAX30102_FIFOCONFIG, MAX30102_SAMPLEAVG_MASK,MAX30102_SAMPLEAVG_32);
  else bitMask(MAX30102_FIFOCONFIG, MAX30102_SAMPLEAVG_MASK,MAX30102_SAMPLEAVG_4);

}
void setADCRange(uint16_t adcRange)
/**
* \简述        设置ADC采样范围
*
* \输出参数    adcRange：2048/4096/8192/16384

* \返回值      无
*/
{
  uint8_t samp;
  max30102_read_reg(MAX30102_PARTICLECONFIG,&samp);
  if(adcRange < 4096) bitMask(MAX30102_PARTICLECONFIG, MAX30102_ADCRANGE_MASK,MAX30102_ADCRANGE_2048); 
  else if(adcRange < 8192) bitMask(MAX30102_PARTICLECONFIG, MAX30102_ADCRANGE_MASK,MAX30102_ADCRANGE_4096); 
  else if(adcRange < 16384) bitMask(MAX30102_PARTICLECONFIG, MAX30102_ADCRANGE_MASK,MAX30102_ADCRANGE_8192); 
  else if(adcRange == 16384) bitMask(MAX30102_PARTICLECONFIG, MAX30102_ADCRANGE_MASK,MAX30102_ADCRANGE_16384); 
  else bitMask(MAX30102_PARTICLECONFIG, MAX30102_ADCRANGE_MASK,MAX30102_ADCRANGE_2048);
}
void setPulseWidth(uint16_t pulseWidth)
/**
* \简述        设置脉冲宽度
*
* \输出参数    pulseWidth：69/118/215/411

* \返回值      无
*/
{
 uint8_t samp;
  max30102_read_reg(MAX30102_PARTICLECONFIG,&samp);
 if (pulseWidth < 118) bitMask(MAX30102_PARTICLECONFIG, MAX30102_PULSEWIDTH_MASK,MAX30102_PULSEWIDTH_69); //Gets us 15 bit resolution
  else if (pulseWidth < 215) bitMask(MAX30102_PARTICLECONFIG, MAX30102_PULSEWIDTH_MASK,MAX30102_PULSEWIDTH_118); //16 bit resolution
  else if (pulseWidth < 411) bitMask(MAX30102_PARTICLECONFIG, MAX30102_PULSEWIDTH_MASK,MAX30102_PULSEWIDTH_215); //17 bit resolution
  else if (pulseWidth == 411) bitMask(MAX30102_PARTICLECONFIG, MAX30102_PULSEWIDTH_MASK,MAX30102_PULSEWIDTH_411); //18 bit resolution
  else bitMask(MAX30102_PARTICLECONFIG, MAX30102_PULSEWIDTH_MASK,MAX30102_PULSEWIDTH_69);
}


uint8_t getWritePointer_1(void) 
/**
* \简述        获取FIFO寄存器写指针位置
*
* \输入参数    无 

* \返回值      写指针位置
*/
{
    uint8_t writeData;
    max30102_read_reg(MAX30102_FIFOWRITEPTR,&writeData);
    return writeData;
}

uint8_t getReadPointer_1(void) 
/**
* \简述        获取FIFO寄存器读指针位置
*
* \输入参数    无 

* \返回值      读指针位置
*/
{
    uint8_t readData;
    max30102_read_reg(MAX30102_FIFOREADPTR,&readData);
    return readData;
}
bool check_1(void)
{
	  u8 readPointer = getReadPointer_1();
		u8 writePointer = getWritePointer_1();
	if(readPointer != writePointer)
	{
		return true;
	}else{
	 return false;
	}
}
/**************************************************************

* 第2个传感器 操作函数

*********************************************************************/
bool max30102_write_reg_2(uint8_t uch_addr, uint8_t uch_data)
/**
* \简述        发送一个值给MAX30102上的某个寄存器
*
* \输入参数    uch_addr    -  寄存器地址
* \输入参数    uch_data    -  发送的数据
*
* \返回值      发送成功 True
*             发送失败  False
*/
{
    // 启动第2路i2c
    IIC_Start_2();
    // 发送i2c写入地址
    IIC_Send_Byte_2(I2C_WRITE_ADDR);
    if (IIC_Wait_Ack_2() != 0)
    {
        goto cmd_fail;
    }
    // 发送寄存器地址
    IIC_Send_Byte_2(uch_addr);
    if (IIC_Wait_Ack_2() != 0)
    {
        goto cmd_fail;
    }
    // 发送数据
    IIC_Send_Byte_2(uch_data);
    if (IIC_Wait_Ack_2() != 0)
    {
        goto cmd_fail;
    }
    // 停止i2c
    IIC_Stop_2();
    return true;

cmd_fail:

    IIC_Stop_2();
    return false;
}
bool max30102_read_reg_2(uint8_t uch_addr, uint8_t *puch_data)
/**
* \简述        读取MAX30102上的某个寄存器
*
* \输入参数     uch_addr       -  寄存器地址
* \输出入参数    *puch_data    -  发送的数据
*
* \返回值      发送成功 True
*             发送失败  False
*/
{
    // 启动i2c
    IIC_Start_2();
    // 发送i2c写入地址·
    IIC_Send_Byte_2(I2C_WRITE_ADDR);
    if (IIC_Wait_Ack_2() != 0)
    {
        goto cmd_fail;
    }
    // 发送寄存器地址·
    IIC_Send_Byte_2((uint8_t)uch_addr);
    if (IIC_Wait_Ack_2() != 0)
    {
        goto cmd_fail;
    }
    // 重启i2c
    IIC_Start_2();
    // 发送i2c读取地址
    IIC_Send_Byte_2(I2C_READ_ADDR);
    if (IIC_Wait_Ack_2() != 0)
    {
        goto cmd_fail;
    }
    *puch_data = IIC_Read_Byte_2();
    IIC_NAck_2();
    IIC_Stop_2();
    return true;
cmd_fail:
    IIC_Stop_2();
    return false;
}

bool max30102_init_2(void)
/**
* \简述        初始化MAX30102
*
* \输入参数     None
*
* \返回值      发送成功 True
*             发送失败  False
*/
{
    if(!max30102_write_reg_2(REG_INTR_ENABLE_1, 0xc0)) // INTR setting
        return false;
    if(!max30102_write_reg_2(REG_INTR_ENABLE_2, 0x00))
        return false;
    if(!max30102_write_reg_2(REG_FIFO_WR_PTR, 0x00)) //FIFO_WR_PTR[4:0]
        return false;
    if(!max30102_write_reg_2(REG_OVF_COUNTER, 0x00)) //OVF_COUNTER[4:0]
        return false;
    if(!max30102_write_reg_2(REG_FIFO_RD_PTR, 0x00)) //FIFO_RD_PTR[4:0]
        return false;

    /*
    设置采样均值
    */
    if(!max30102_write_reg_2(REG_FIFO_CONFIG, 0x0f))
        return false;
    if(!max30102_write_reg_2(REG_MODE_CONFIG, 0x03))  //0x02 for Red only, 0x03 for SpO2 mode 0x07 multimode LED
        return false;
    /*
    设置ADC范围、精度和采样率
    */
    if(!max30102_write_reg_2(REG_SPO2_CONFIG, 0x38))   // 00101001    200采样率
        return false;

    if(!max30102_write_reg_2(REG_LED1_PA, 0x5f))
        return false;
    if(!max30102_write_reg_2(REG_LED2_PA, 0x4f))
        return false;
    if(!max30102_write_reg_2(REG_PILOT_PA, 0x2f))  // Choose value for ~ 25mA for Pilot LED
        return false;
    return true;
}

bool max30102_read_fifo_2(uint32_t *pun_red_led, uint32_t *pun_ir_led)

/**
* \简述        读取MAX30102上FIFO寄存器的采样值，保存到两个指针中
*
* \输出参数    *pun_red_led       -  红光数据
* \输出参数    *pun_ir_led        -  红外光数据
*
* \返回值      发送成功 True
*             发送失败  False
*/
{

    uint32_t un_temp;
    uint32_t red;
    uint32_t ir;
    unsigned char uch_temp;
    *pun_ir_led = 0;
    *pun_red_led = 0;
    max30102_read_reg_2(REG_INTR_STATUS_1, &uch_temp);
    max30102_read_reg_2(REG_INTR_STATUS_2, &uch_temp);

    IIC_Start_2();
    IIC_Send_Byte_2(I2C_WRITE_ADDR);
    if (IIC_Wait_Ack_2() != 0)
    {
        //printf("read fifo failed");
        goto cmd_fail;
    }
    IIC_Send_Byte_2((uint8_t)REG_FIFO_DATA);
    if (IIC_Wait_Ack_2() != 0)
    {
        goto cmd_fail;
    }
    IIC_Start_2();
    IIC_Send_Byte_2(I2C_READ_ADDR);
    if (IIC_Wait_Ack_2() != 0)
    {
        goto cmd_fail;
    }
    un_temp = IIC_Read_Byte_2();
    IIC_Ack_2();
    un_temp <<= 16;
    red = un_temp;
    un_temp = IIC_Read_Byte_2();
    IIC_Ack_2();
    un_temp <<= 8;
    red += un_temp;
    un_temp = IIC_Read_Byte_2();
    IIC_Ack_2();

    red += un_temp;
    red &= 0x03FFFF;
    *pun_red_led = red;

    un_temp = IIC_Read_Byte_2();
    IIC_Ack_2();
    un_temp <<= 16;
    ir = un_temp;
    un_temp = IIC_Read_Byte_2();
    IIC_Ack_2();
    un_temp <<= 8;
    ir += un_temp;
    un_temp = IIC_Read_Byte_2();
    IIC_Ack_2();
    ir += un_temp;
    ir &= 0x03FFFF;
    *pun_ir_led = ir;
    IIC_Stop_2();
    return true;
cmd_fail:
    //printf("fail !!!");
    IIC_Stop_2();
    return false;
}

bool max30102_reset_2()
/**
* \简述        重启MAX30102
*
* \输出参数    None
* \返回值      发送成功 True
*             发送失败  False
*/
{
    if(!max30102_write_reg_2(REG_MODE_CONFIG, 0x40))
        return false;
    else
        return true;
}

void bitMask_2(uint8_t reg, uint8_t mask, uint8_t thing)
/**
* \简述        寄存器赋值函数
*
* \输出参数    reg：寄存器地址
              mask：操作位mask值
              thing：赋值位内容
* \返回值      无
*/
{
    uint8_t originalContents;
    max30102_read_reg_2(reg, &originalContents);
    originalContents = originalContents & mask;
    max30102_write_reg_2(reg, originalContents | thing);

}
void setSampleRate_2(uint16_t sampleRate)
/**
* \简述        设置采样频率
*
* \输出参数    sampleRate：采样值（50/100/200/400/800/1000/1600/3200）

* \返回值      无
*/
{
    if (sampleRate < 100) bitMask_2(MAX30102_PARTICLECONFIG, MAX30102_SAMPLERATE_MASK,MAX30102_SAMPLERATE_50);
    else if (sampleRate < 200) bitMask_2(MAX30102_PARTICLECONFIG, MAX30102_SAMPLERATE_MASK,MAX30102_SAMPLERATE_100);
    else if (sampleRate < 400) bitMask_2(MAX30102_PARTICLECONFIG, MAX30102_SAMPLERATE_MASK,MAX30102_SAMPLERATE_200);
    else if (sampleRate < 800) bitMask_2(MAX30102_PARTICLECONFIG, MAX30102_SAMPLERATE_MASK,MAX30102_SAMPLERATE_400);
    else if (sampleRate < 1000) bitMask_2(MAX30102_PARTICLECONFIG, MAX30102_SAMPLERATE_MASK,MAX30102_SAMPLERATE_800);
    else if (sampleRate < 1600) bitMask_2(MAX30102_PARTICLECONFIG, MAX30102_SAMPLERATE_MASK,MAX30102_SAMPLERATE_1000);
    else if (sampleRate < 3200) bitMask_2(MAX30102_PARTICLECONFIG, MAX30102_SAMPLERATE_MASK,MAX30102_SAMPLERATE_1600);
    else if (sampleRate == 3200) bitMask_2(MAX30102_PARTICLECONFIG, MAX30102_SAMPLERATE_MASK,MAX30102_SAMPLERATE_3200);
    else bitMask_2(MAX30102_PARTICLECONFIG, MAX30102_SAMPLERATE_MASK,MAX30102_SAMPLERATE_50);
}
void setFIFOAverage_2(uint8_t sampleAverage)
/**
* \简述        设置采样频均值
*
* \输出参数    sampless：均值（1/2/4/8/16/32）

* \返回值      无
*/
{
  uint8_t samp;
  max30102_read_reg_2(MAX30102_FIFOCONFIG,&samp);
  if (sampleAverage == 1) bitMask_2(MAX30102_FIFOCONFIG, MAX30102_SAMPLEAVG_MASK,MAX30102_SAMPLEAVG_1);
  else if (sampleAverage == 2) bitMask_2(MAX30102_FIFOCONFIG, MAX30102_SAMPLEAVG_MASK,MAX30102_SAMPLEAVG_2);
  else if (sampleAverage == 4) bitMask_2(MAX30102_FIFOCONFIG, MAX30102_SAMPLEAVG_MASK,MAX30102_SAMPLEAVG_4);
  else if (sampleAverage == 8) bitMask_2(MAX30102_FIFOCONFIG, MAX30102_SAMPLEAVG_MASK,MAX30102_SAMPLEAVG_8);
  else if (sampleAverage == 16) bitMask_2(MAX30102_FIFOCONFIG, MAX30102_SAMPLEAVG_MASK,MAX30102_SAMPLEAVG_16);
  else if (sampleAverage == 32) bitMask_2(MAX30102_FIFOCONFIG, MAX30102_SAMPLEAVG_MASK,MAX30102_SAMPLEAVG_32);
  else bitMask_2(MAX30102_FIFOCONFIG, MAX30102_SAMPLEAVG_MASK,MAX30102_SAMPLEAVG_4);

}
void setADCRange_2(uint16_t adcRange)
/**
* \简述        设置ADC采样范围
*
* \输出参数    adcRange：2048/4096/8192/16384

* \返回值      无
*/
{
  uint8_t samp;
  max30102_read_reg_2(MAX30102_PARTICLECONFIG,&samp);
  if(adcRange < 4096) bitMask_2(MAX30102_PARTICLECONFIG, MAX30102_ADCRANGE_MASK,MAX30102_ADCRANGE_2048); 
  else if(adcRange < 8192) bitMask_2(MAX30102_PARTICLECONFIG, MAX30102_ADCRANGE_MASK,MAX30102_ADCRANGE_4096); 
  else if(adcRange < 16384) bitMask_2(MAX30102_PARTICLECONFIG, MAX30102_ADCRANGE_MASK,MAX30102_ADCRANGE_8192); 
  else if(adcRange == 16384) bitMask_2(MAX30102_PARTICLECONFIG, MAX30102_ADCRANGE_MASK,MAX30102_ADCRANGE_16384); 
  else bitMask_2(MAX30102_PARTICLECONFIG, MAX30102_ADCRANGE_MASK,MAX30102_ADCRANGE_2048);
}
void setPulseWidth_2(uint16_t pulseWidth)
/**
* \简述        设置脉冲宽度
*
* \输出参数    pulseWidth：69/118/215/411

* \返回值      无
*/
{
 uint8_t samp;
  max30102_read_reg_2(MAX30102_PARTICLECONFIG,&samp);
 if (pulseWidth < 118) bitMask_2(MAX30102_PARTICLECONFIG, MAX30102_PULSEWIDTH_MASK,MAX30102_PULSEWIDTH_69); //Gets us 15 bit resolution
  else if (pulseWidth < 215) bitMask_2(MAX30102_PARTICLECONFIG, MAX30102_PULSEWIDTH_MASK,MAX30102_PULSEWIDTH_118); //16 bit resolution
  else if (pulseWidth < 411) bitMask_2(MAX30102_PARTICLECONFIG, MAX30102_PULSEWIDTH_MASK,MAX30102_PULSEWIDTH_215); //17 bit resolution
  else if (pulseWidth == 411) bitMask_2(MAX30102_PARTICLECONFIG, MAX30102_PULSEWIDTH_MASK,MAX30102_PULSEWIDTH_411); //18 bit resolution
  else bitMask_2(MAX30102_PARTICLECONFIG, MAX30102_PULSEWIDTH_MASK,MAX30102_PULSEWIDTH_69);
}


uint8_t getWritePointer_2(void) 
/**
* \简述        获取FIFO寄存器写指针位置
*
* \输入参数    无 

* \返回值      写指针位置
*/
{
    uint8_t writeData;
    max30102_read_reg_2(MAX30102_FIFOWRITEPTR,&writeData);
    return writeData;
}

uint8_t getReadPointer_2(void) 
/**
* \简述        获取FIFO寄存器读指针位置
*
* \输入参数    无 

* \返回值      读指针位置
*/
{
    uint8_t readData;
    if(max30102_read_reg_2(MAX30102_FIFOREADPTR,&readData)) {
        return readData;
    } else {
        return 0;
    }
}
bool check_2(void)
{
	  u8 readPointer_2 = getReadPointer_2();
		u8 writePointer_2 = getWritePointer_2();
	if(readPointer_2 != writePointer_2)
	{
		return true;
	}else{
	 return false;
	}
}
void setLEDValueRED_2(char* ledValue)
	/**
* \简述        设置LED电流值
*
* \输出参数    LedValue：（0-50）

* \返回值      无
*/
{
	max30102_write_reg_2(REG_LED1_PA, StringToInt(ledValue));
	max30102_write_reg_2(REG_LED1_PA, StringToInt(ledValue));
}


void setLEDValueIR_2(char* ledValue)
	/**
* \简述        设置LED电流值
*
* \输出参数    LedValue：（0-50）

* \返回值      无
*/
{
	max30102_write_reg_2(REG_LED2_PA, StringToInt(ledValue));
}


int StringToInt(char *hex)
{
    return CharToInt(hex[0]) * 16 + CharToInt(hex[1]);
}


int CharToInt(char hex) 
{
    if (hex>='0' && hex <='9')
        return hex - '0';
    if (hex>='A' && hex <= 'F')
        return hex-'A'+10;
    if(hex>='a' && hex <= 'f')
        return hex-'a'+10;
    return -1;
}
