/*
   Bluetooth_Interface with ATmega16 to Control LED via smartphone
   http://www.electronicwings.com
 
 */ 

#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include "USART_RS232_H_file.h"		//include USART library 


int main(void)
{
	char Data_in;
    DDRB = 0xFF;		//make PORT as output port 
	PORTB = 0x00;
	USART_Init(9600);	//initialize USART with 9600 baud rate 
	
	while(1)
	{
		PORTB = 0x00;
		Data_in = USART_RxChar();	//receive data from BlueTooth device
		if(Data_in =='1')	//object still for too long
		{
			PORTB = 0xFF;	//send signal for interrupt
		}
		else if(Data_in =='0')
		{
			PORTB = 0x00;
		}
		_delay_ms(50);
	}		
	return 0;
}