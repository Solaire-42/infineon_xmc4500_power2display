/*
 * main.c
 *
 *  Created on: 2021 Dec 11 18:25:18
 *  Author: fluky
 *
 *  AC POWER2DISPLAY
 *  This code reads the DAC voltage output of another XMC4500.
 *  The measured voltage is then converted into power.
 *  Finally, the power is shown on the display.
 *
 *  THIS CODE WAS SPECIALLY DEVEOLOPED FOR CHECKING THE DAC-OUTPUT OF "infineon_xmc4500_ac-current-meter"
 *
 *  CONNECTIONS (for XMC4500):
 *  - Display
 *    GND - GND
 *    VCC - 3.3V
 *    SCL - P0.11
 *    SDA - P0.5
 *  - Analog input
 *    P14.0
 */

#include "DAVE.h"
#include "SSD1306/ssd1306.h"
#include <stdio.h>
#include <math.h>
#include <string.h>

//ADC MEASUREMENT
uint16_t result;
uint16_t Adc_Measurement_Handler()
{
	//Get result from ADC-Converter
	result = ADC_MEASUREMENT_GetResult(&ADC_MEASUREMENT_Channel_A);			//results for 12bit: 0...4096 => 0...3.3V
	ADC_MEASUREMENT_StartConversion(&ADC_MEASUREMENT_0);

	return result;
}

//DISPLAY
uint32_t SysTickCnt = 0;
void SysTick_Handler() {
	SysTickCnt++;
}

void delay_ms(uint32_t ms) {
	uint32_t now = SysTickCnt;
	while((now + ms) > SysTickCnt) __NOP();
}

//WRITE STRING ON DISPLAY
void ssd1306_text(char *data1){
	ssd1306_Fill(Black);
	ssd1306_SetCursor(2, 1);
	ssd1306_WriteString(data1, Font_6x8, White);
	ssd1306_UpdateScreen();
}

void float2string(float value, char *variable, char *unit, char *str){
	//uint16_t to string: from stackoverflow.com/questions/905928/using-floats-with-sprintf-in-embedded-c
	char *tmpSign = (value < 0) ? "-" : "";
	float tmpVal = (value < 0) ? -value : value;

	int tmpInt1 = tmpVal;                  	// Get the integer
	float tmpFrac = tmpVal - tmpInt1;      	// Get fraction
	int tmpInt2 = trunc(tmpFrac * 10);  	// Turn into integer - For changing decimal place: change 10 to 100 or 1000 and in sprintf

	sprintf(str, "%s %s%d.%01d %s", variable, tmpSign, tmpInt1, tmpInt2, unit);
}


int main(void)
{
	//DAVE
	DAVE_Init();

	//ADC VARIABLES
	float adc;
	float voltage;
	float current;
	float power;

	//OPEN COMMUNICATION WITH I2C
	SysTick_Config(SystemCoreClock / 1000);
	ssd1306_Init(I2C_MASTER_0.channel, &delay_ms);

	while(1)
	{
		//GET RESULT FROM ADC
		adc = result;

		//CALCULATE
		voltage = adc * (3.3/4096.0);
		current = voltage * 10;
		power = current * 230;
		//Smoothing the measurement and compensating environmental influences
		if(voltage <= 0.03){
			current = 0;
			power = 0;
		}

		//POWER STRING
		char power_str [100];
		float2string(power, "Power:", "W", power_str);

		//DISPLAY THE STRING'S
		ssd1306_text(power_str);
	}
}
