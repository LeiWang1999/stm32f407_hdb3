#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "lcd.h"
#include "beep.h"
#include "adc.h"
#include "dac.h"
#include "timer.h"
#include "stm32f4xx_it.h"
#include "key.h"
#include "hdb3.h"
#include "stm32f4xx.h" // Device header
#include "waveform.h"

int main(void)
{
	u16 buff[800];
	u16 adcx = 0;
	u16 dacx = 0;
	u16 len = 16;
	signed char adc_binary_array[16];
	signed char hdb3_binary_array[16];
	signed char dac_binary_array[16];
	float Adresult = 0;
	u8 Vpp_buff[20] = {0};
	u8 key = 0;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置系统中断优先级分组2
	delay_init(168);								//初始化延时函数
	uart_init(115200);								//初始化串口波特率为115200
	LED_Init();										//初始化LED
	BEEP_Init();									//初始化蜂鸣器端口
	LCD_Init();										//初始化LCD FSMC接口
	Adc_Init();										//初始化ADC
	Set_BackGround();								//显示背景
	Lcd_DrawNetwork();
	KEY_Init();
	LED0 = 0;
	MYDAC_Init();
	Dac2_Init();
	BACK_COLOR = DARKBLUE;
	while (1)
	{
		adcx = Get_Adc(ADC_Channel_6);
		Dec2Bin(adcx, adc_binary_array, len);
		HDB3_Encoding(hdb3_binary_array, adc_binary_array, len);
		HDB3_Decoding(dac_binary_array, hdb3_binary_array, len);
		
		printf("ADC_Value: %d \r\n", adcx);
		printf("ADC_Binary:");
		print_binary_string(adc_binary_array,len);
		printf("HDB3_Encode:");
		print_binary_string(hdb3_binary_array,len);
		printf("HDB3_Decode:");
		print_binary_string(dac_binary_array,len);
		
		Bin2Dec(dac_binary_array, dacx);
		Dac2_Set_Vol(dacx);
		DrawOscillogram(buff, adc_binary_array, hdb3_binary_array, 16, -20, 80); //画波形
		Adresult = get_vpp(buff); //峰峰值mv
		sprintf((char *)Vpp_buff, "Vpp = %0.3fV", Adresult);
		
		// 右侧提示
		POINT_COLOR = RED;
		LCD_ShowString(720, 50, 80, 24, 24, "HDB3");
		POINT_COLOR = WHITE;
		LCD_ShowString(720, 200, 80, 24, 24, "ADCX");
		// 下方提示
		POINT_COLOR = WHITE;
		LCD_ShowString(320, 425, 210, 24, 24, Vpp_buff);
		POINT_COLOR = WHITE;
		LCD_ShowString(600, 425, 288, 29, 24, "STOP");
		POINT_COLOR = YELLOW;
		LCD_ShowString(665, 425, 319, 29, 24, "RUN");
		
		LED0 = !LED0; 
		key = KEY_Scan(0); //得到键值
		if (key)
		{
			switch (key)
			{
			case WKUP_PRES:
				sin_Generation();
				BEEP = 1;
				delay_ms(100);
				BEEP = 0;
				break;
			case KEY1_PRES:
				triangle_Generation();
				BEEP = 1;
				delay_ms(100);
				BEEP = 0;
				break;
			case KEY2_PRES:
				sawtooth_Generation();
				BEEP = 1;
				delay_ms(100);
				BEEP = 0;
				break;
			}
		}
		else if (key == KEY0_PRES)
		{
			BEEP = 1;
			delay_ms(100);
			BEEP = 0;
			do
			{
				POINT_COLOR = YELLOW;
				LCD_ShowString(600, 425, 288, 29, 24, "STOP");
				POINT_COLOR = WHITE;
				LCD_ShowString(665, 425, 319, 29, 24, "RUN");
				key = KEY_Scan(0);
				if (key == 0 || key == 2 || key == 3 || key == 4)
				{
					BEEP = 1;
					delay_ms(100);
					BEEP = 0;
					switch (key)
					{
					case WKUP_PRES:
						sin_Generation();
						break;
					case KEY1_PRES:
						triangle_Generation();
						break;
					case KEY2_PRES:
						sawtooth_Generation();
						break;
					}
					break;
				}
			} while (1);
		}
	}
}
