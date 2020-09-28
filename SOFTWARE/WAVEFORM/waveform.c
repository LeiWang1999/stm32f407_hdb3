//STM32 使用DMA+DAC+TIMER输出正弦波//

/* 

          那么对于使用DMA+DAC+TIMER产生正弦波的原理或过程，我有这样一个简单的理解：
      先将一个可以生成正弦波的数据表保存在静态内存中，然后在DAC以及这块内存中间使
      用DMA建立一个通道，经过以上步骤之后，DAC模块就可以通过DMA通道拿取静态内存中
      可以生成正弦波的数据，拿取数据，然后经过数模准换，在引脚进行输出就可以得到正
      弦波了。那么当然，这个速度是非常快的，如果没有一定的延时，那么得到的估计就是
      一个变化很快的模拟量。所以这个时候就需要使用定时器TIMER了。DAC在初始化的时候，
      可以设置成使用定时器触发，这就意味着，当定时器溢满的时候，就会触发DAC工作。
      这样一来，就可以通过改变定时器的定时时间来改变正弦波的周期了。

                          电压大小的显示用DAC来处理
*/

//#include "dac.h"
#include "sys.h"
#include "math.h"
#include "waveform.h"
#include "key.h"
#include "delay.h"
#define PI 3.14159
#define DAC_DHR12R1_ADDRESS 0x40007408

u16 sinTable[tableSize];

u8 KEY_Scan(u8 mode);
u32 max_data;
u16 position = 200; //波形图中心轴

//将正弦波数据保存在静态内存中
void sin_Generation(void)
{
	u16 n;
	u16 temp = 1023;
	//temp=KEY_Scan(0);
	if (KEY_Scan(0) == 4)
	{
		temp = temp + 200;
	}
	for (n = 0; n < tableSize; n++)
	{
		sinTable[n] = (sin(2 * PI * n / tableSize) + 1) * (temp);
	}
}

//锯齿波
void sawtooth_Generation(void)
{
	u16 n;
	for (n = 0; n < tableSize; n++)
	{
		sinTable[n] = ((2 * n * 1000) / tableSize);
	}
}

//三角波
void triangle_Generation(void)
{
	u16 n;
	for (n = 0; n < tableSize / 2; n++)
	{
		sinTable[n] = ((2 * n * 1000) / tableSize);
	}
	for (; n < tableSize; n++)
	{
		sinTable[n] = (2 * 1000 - (2 * n * 1000) / tableSize);
	}
}

//矩形波
void rectangle_Generation(void)
{
	u16 n;
	for (n = 0; n < tableSize / 2; n++)
	{
		sinTable[n] = 0;
	}
	for (; n < (tableSize); n++)
	{
		sinTable[n] = 1000;
	}
}

//当定时器溢满的时候，就会触发DAC工作，这样一来，就可以通过改变定时器的定时时间来改变正弦波的周期了
void TIM6_Configuration(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

	TIM_PrescalerConfig(TIM6, 83, TIM_PSCReloadMode_Update);
	TIM_SetAutoreload(TIM6, 4);
	TIM_SelectOutputTrigger(TIM6, TIM_TRGOSource_Update);
	TIM_Cmd(TIM6, ENABLE);
}

void DacGPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure; //结构体

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); //使能GPIOA时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;		 //选定4号引脚,4,5都可以
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;	 //模拟输入
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; //不上拉，不下拉
	GPIO_Init(GPIOA, &GPIO_InitStructure);			 //确定为GPIOA的初始化
}

//后在DAC以及这块内存中间使用DMA建立一个通道
void DAC_DMA_Configuration(void)
{
	DAC_InitTypeDef DAC_InitStructure; //DAC结构体

	DMA_InitTypeDef DMA_InitStructure; //DMA结构体

	//使能DMA1时钟，则我们要找DMA1的请求映射来进行下面的配置
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE); //使能DAC时钟

	DAC_InitStructure.DAC_Trigger = DAC_Trigger_T6_TRGO;			//使用触发功能
	DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None; //不用STM32自带的波形
	DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Disable;  //一般设置不使用输出缓存
	//STM32的DAC有两个通道，这里我们使用的是DAC的通道1，所以对DAC进行初始化
	DAC_Init(DAC_Channel_1, &DAC_InitStructure);

	//DMA1请求映射，DMA的数据流5和通道7为DAC1功能
	DMA_DeInit(DMA1_Stream5);					   //设置数据流5
	DMA_InitStructure.DMA_Channel = DMA_Channel_7; //设置通道7
	//把正弦波的数据表保存在静态内存中
	DMA_InitStructure.DMA_PeripheralBaseAddr = DAC_DHR12R1_ADDRESS;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&sinTable; //定义DMA外设基地址
	DMA_InitStructure.DMA_BufferSize = tableSize;				 //设置DMA缓存大小
	//外设数据宽度
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	//设置为循环工作模式
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA1_Stream5, &DMA_InitStructure);

	DMA_Cmd(DMA1_Stream5, ENABLE); //使能DMA1数据流5

	DAC_Cmd(DAC_Channel_1, ENABLE); //使能DAC的通道1

	//将DAC通道和DMA通道接起来
	DAC_DMACmd(DAC_Channel_1, ENABLE); //使能DAC_DMA通道1
}


void MYDAC_Init(void)
{

	DacGPIO_Configuration();
	TIM6_Configuration();
	DAC_DMA_Configuration();

}

void clear_point(u16 num) //更新显示屏当前列
{
	u16 lie = 0;
	POINT_COLOR = DARKBLUE;
	for (lie = 1; lie < 400; lie++)
	{
		LCD_DrawPoint(num, lie);
	}
	if (!(num % 50)) //判断hang是否为50的倍数 画列点
	{
		for (lie = 10; lie < 400; lie += 10)
		{
			LCD_Fast_DrawPoint(num, lie, WHITE);
		}
	}
	if (!(num % 10)) //判断hang是否为10的倍数 画行点
	{
		for (lie = 50; lie < 400; lie += 50)
		{
			LCD_Fast_DrawPoint(num, lie, WHITE);
		}
	}
	POINT_COLOR = YELLOW;
}

void DrawOscillogram(u16 *buff, signed char *adc_binary_array, signed char *hdb3_binary_array, u16 len, signed int offset1, signed int offset2) //画波形图
{

	static u16 Ypos1_1 = 0, Ypos1_2 = 0, Ypos2_1 = 0, Ypos2_2 = 0, Ypos3_1 = 0, Ypos3_2 = 0;
	signed int adc_binary_array_buff[800];
	signed int hdb3_binary_array_buff[800];
	signed char temp;
	u16 i = 0;

	for (i = 1; i < 700; i++) //存储AD数值
	{
		buff[i] = Get_Adc(ADC_Channel_6);
		temp = adc_binary_array[(i * len) / 700];
		adc_binary_array_buff[i] = temp * 1024;
		temp = hdb3_binary_array[(i * len) / 700] + 1;
		hdb3_binary_array_buff[i] = temp * 1024;
	}
	for (i = 1; i < 700; i++)
	{
		clear_point(i);
		Ypos1_2 = position - (buff[i] * 165 / 4096); //转换坐标//4096
		Ypos1_2 = Ypos1_2 * 2;						 //纵坐标倍数
		if (Ypos1_2 > 400)
			Ypos1_2 = 400; //超出范围不显示
		POINT_COLOR = YELLOW;
		LCD_DrawLine(i, Ypos1_1, i + 1, Ypos1_2); //波形连接
		Ypos1_1 = Ypos1_2;
		Ypos2_2 = position - (adc_binary_array_buff[i] * 165 / 4096);
		Ypos2_2 -= offset1;
		if (Ypos2_2 > 400)
			Ypos2_2 = 400;
		POINT_COLOR = WHITE;
		LCD_DrawLine(i, Ypos2_1, i + 1, Ypos2_2); //波形连接
		Ypos2_1 = Ypos2_2;
		Ypos3_2 = position - (hdb3_binary_array_buff[i] * 165 / 4096);
		Ypos3_2 -= offset2;
		if (Ypos3_2 > 400)
			Ypos3_2 = 400;
		POINT_COLOR = RED;
		LCD_DrawLine(i, Ypos3_1, i + 1, Ypos3_2); //波形连接
		Ypos3_1 = Ypos3_2;
	}
	Ypos1_1 = 0;
	Ypos2_1 = 0;
	Ypos3_1 = 0;
}

void Set_BackGround(void)
{
	LCD_Clear(DARKBLUE);
	POINT_COLOR = WHITE;
	LCD_DrawRectangle(0, 0, 700, 400);
}

void Lcd_DrawNetwork(void)
{
	u16 index_y = 0;
	u16 index_x = 0;
	for (index_x = 50; index_x < 700; index_x += 50)
	{
		for (index_y = 10; index_y < 400; index_y += 10)
		{
			LCD_Fast_DrawPoint(index_x, index_y, WHITE);
		}
	}
	//画行点
	for (index_y = 50; index_y < 400; index_y += 50)
	{
		for (index_x = 10; index_x < 700; index_x += 10)
		{
			LCD_Fast_DrawPoint(index_x, index_y, WHITE);
		}
	}
}

float get_vpp(u16 *buf) //获取峰峰值
{

	u32 max_data = buf[0];
	u32 min_data = buf[0];
	u32 n = 0;
	float Vpp = 0;
	for (n = 1; n < 256; n++)
	{
		if (buf[n] > max_data)
		{
			max_data = buf[n];
		}
		else if (buf[n] < min_data)
			min_data = buf[n];
	}
	Vpp = (float)(max_data - min_data);
	Vpp = Vpp * (3.3 / 4096);
	max_data = max_data * (3.3 / 4096);
	min_data = min_data * (3.3 / 4096);
	return Vpp;
}
