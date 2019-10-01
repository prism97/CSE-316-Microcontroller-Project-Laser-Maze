#define F_CPU 8000000UL

#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <math.h>  //include libm
#include "USART_RS232_H_file.h"

#include "mpu6050.h"

#define UART_BAUD_RATE 4800
#include "uart.h"
#define PI 3.14159


//*****useful if you want to use the gyro's timer*****//

volatile uint32_t tot_overflow;

void timer1_init() {
    //TCNT1, TCCR1A, TCCR1B, TIMSK, TIFR, ICR1, TIMER1_OVF_vect
    
    TCCR1B = 1 << CS10;
    TIMSK = 1 << TOIE1;
    TCNT1 = 0;
    tot_overflow = 0;
}

ISR(TIMER1_OVF_vect)
{
    // keep a track of number of overflows
    tot_overflow++;
}

uint32_t getElapsedTime() {
    //return tot_overflow * (255 - TCNT0);
    return (tot_overflow * 65536 + (uint32_t) TCNT1) / 1000  ; //time returned in micro seconds / 1000 = millis
    //return TCNT0;
}

//*****useful if you want to use the gyro's timer*****//


int main(void) {
    
    DDRA = 0xFF;
    DDRB = 0xFF;
    
    PORTA = 0xAA;
    PORTB = 0xFF;


	#if MPU6050_GETATTITUDE == 0
		int16_t ax = 0;
		int16_t ay = 0;
		int16_t az = 0;
		int16_t gx = 0;
		int16_t gy = 0;
		int16_t gz = 0;
		double axg = 0;
		double ayg = 0;
		double azg = 0;
		double gxds = 0;
		double gyds = 0;
		double gzds = 0;
	#endif

	//loop variable to check how much time the object has been still for
	int timeCheck = 0;

    //initialize UART
	uart_init(UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU));

	sei();	//enable global interrupt

	//initialize mpu6050
	mpu6050_init();
	_delay_ms(50);
    
    timer1_init();
	int still = 0;	//0 if the object is moving, 1 if still
	
	
	USART_Init(9600);	// initialize USART with 9600 baud rate 
	

	while(1) {
        
		#if MPU6050_GETATTITUDE == 0
		
		mpu6050_getRawData(&ax, &ay, &az, &gx, &gy, &gz);
		mpu6050_getConvData(&axg, &ayg, &azg, &gxds, &gyds, &gzds);
        
		if(gxds < 10 && gyds < 10 && gzds < 10)		//too little movement
		{
			if(still == 1)		//object is already in still state
			{
				timeCheck++;
				if(timeCheck >= 10)		//object has been still for too long
				{
					USART_TxChar('1');	
					timeCheck = 0;
				}
			}
			else 
			{
				still = 1;
				timeCheck = 0;
				USART_TxChar('0');
			}
		}
		else
		{
			still = 0;
			USART_TxChar('0');	
		}
		
		
		_delay_ms(500);
		#endif

	}

}
