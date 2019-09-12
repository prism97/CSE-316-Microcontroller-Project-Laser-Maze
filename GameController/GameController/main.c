/*
* GameController.c
*
* Created: 9/4/2019 7:22:14 PM
* Author : User
*/


#ifndef F_CPU
#define F_CPU 8000000UL
#endif


#define D0 eS_PORTD0
#define D1 eS_PORTD1
#define D2 eS_PORTD2
#define D3 eS_PORTD3
#define D4 eS_PORTD4
#define D5 eS_PORTD5
#define D6 eS_PORTD6
#define D7 eS_PORTD7
#define RS eS_PORTC6
#define EN eS_PORTC7


#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <string.h>
#include "lcd.h"
#include "rtc.h"


int life = 9;
int score = 1000;

uint16_t adc_output = 0;
float actual_voltage = 0;
int currentOff;
int wait;
int start = 0;
int analog_ir_sensor_value;


int output[5] = {
	0b01111000,
	0b10111000,
	0b11011000,
	0b11101000,
	0b11110000
};

int admux[5] = {
	0b01100111,
	0b01100110,
	0b01100101,
	0b01100100,
	0b01100011
};

rtc_t rtc;

void adc_init();
void RTC_values();
int read_adc_channel(unsigned char channel);


ISR(INT2_vect)
{
	score -= 100;
	_delay_ms(50);
}



int main(void)
{
	DDRB = 0b11111000;
	PORTB = 0b11111000;
	
	RTC_values();
	
	LCD_SetUp(PC_6,P_NC,PC_7,P_NC,P_NC,P_NC,P_NC,PD_4,PD_5,PD_6,PD_7);
	LCD_Init(2,16);
	RTC_Init();
	
	
	GICR = (1 << INT2);
	MCUCSR |= (1 << ISC2);
	
	sei();
	
	//ADMUX = 0b01100000;
	//ADCSRA = 0b10000111;
	currentOff = 4;
	wait = 0;
	char arr[9];
	strcpy(arr, "Life : ");
	arr[7] = life + '0';
	arr[8] = '\0';
	
	
	/* Replace with your application code */
	while (1)
	{
		if(start == 0)
		{
			adc_init();
			/*Reading analog ir sensor value*/
			analog_ir_sensor_value=read_adc_channel(0);
			analog_ir_sensor_value = analog_ir_sensor_value/256;
			
			if(analog_ir_sensor_value < 2)
			{
				//object detected
				start = 1;
				RTC_SetDateTime(&rtc);
				//ADCSRA = 0b10000111;
			}
			LCD_Clear();
			LCD_GoToLine(0);
			LCD_Printf("Start");
		}
		else if(start == 1)
		{
			adc_init();
			/*Reading analog ir sensor value*/
			analog_ir_sensor_value=read_adc_channel(2);
			analog_ir_sensor_value = analog_ir_sensor_value/256;
			
			if(analog_ir_sensor_value < 2)
			{
				//object detected at end point
				RTC_GetDateTime(&rtc);
				LCD_Clear();
				LCD_GoToLine(0);
				LCD_Printf("Game Over");
				_delay_ms(500);
				LCD_Clear();
				LCD_GoToLine(0);
				LCD_Printf("score: %d", score);
				LCD_GoToLine(1);
				LCD_Printf("time: %2x:%2x",(uint16_t)rtc.min,(uint16_t)rtc.sec);
				break;
			}
			
			ADCSRA = 0x00;
			ADMUX = 0x00;
			
			if(wait == 10)
			{
				wait = 0;
				currentOff = (currentOff + 1) % 5;
				PORTB = output[currentOff];
				_delay_ms(25);
			}
			
			for(int i = 0; i < 5; i++)
			{
				ADCSRA = 0b10000111;
				ADMUX = admux[i];
				
				ADCSRA |= (1 << ADSC);
				while (ADCSRA & (1 << ADSC));
				adc_output = (ADC >> 6);
				actual_voltage = 1.0 * adc_output / 256;
				
				if((actual_voltage > 2) && (i != currentOff))
				{
					//object crossed laser
					life--;
					score -= 50;
					if(life == 0) 
					{
						break;
					}
					arr[7] = life + '0';
				}
				_delay_ms(25);
			}
			
			if(life == 0)
			{
				RTC_GetDateTime(&rtc);
				LCD_Clear();
				LCD_GoToLine(0);
				LCD_Printf("Game Over");
				_delay_ms(500);
				LCD_Clear();
				LCD_GoToLine(0);
				LCD_Printf("score: %d", score);
				LCD_GoToLine(1);
				LCD_Printf("time: %2x:%2x",(uint16_t)rtc.min,(uint16_t)rtc.sec);
				break;
			}
			
			wait++;
			
			LCD_Clear();
			LCD_GoToLine(0);
			//LCD_Printf(arr);
			LCD_Printf("life: %d", life);
			LCD_GoToLine(1);
			LCD_Printf("score: %d", score);
			
		}
		_delay_ms(100);
		
	}
	return 0;
}

void RTC_values()
{
	rtc.hour = 0x00; //  10:40:20 am
	rtc.min =  0x00;
	rtc.sec =  0x00;

	rtc.date = 0x01; //1st Jan 2016
	rtc.month = 0x01;
	rtc.year = 0x16;
	rtc.weekDay = 5; // Friday: 5th day of week considering monday as first day.
}

/*ADC Function Definitions*/
void adc_init(void)
{
	ADCSRA=(1<<ADEN)|(1<<ADSC)|(1<<ADATE)|(1<<ADPS2);
	SFIOR=0x00;
}
int read_adc_channel(unsigned char channel)
{
	int adc_value;
	unsigned char temp;
	ADMUX=(1<<REFS0)|channel;
	_delay_ms(1);
	temp=ADCL;
	adc_value=ADCH;
	adc_value=(adc_value<<8)|temp;
	return adc_value;
}
