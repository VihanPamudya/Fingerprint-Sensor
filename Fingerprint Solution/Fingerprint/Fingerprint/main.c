// define header files
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "FINGERPRINT.h"

#define uchar unsigned char
#define uint unsigned int

#define KeyPORTDIR DDRA
#define KeyPORT PORTA
#define key PINA

#define UP 0
#define DOWN 1
#define MATCH 1
#define ENROL 2
#define OK 3
#define DEL 3

#define enrol (key & (1 << ENROL)) // key 1
#define match (key & (1 << MATCH)) // key 4
#define delet (key & (1 << DEL))   // key 2
#define up (key & (1 << UP))       // key 3
#define down (key & (1 << DOWN))   // key 4
#define ok (key & (1 << OK))       // key 2

#define HIGH 1
#define LOW 0

volatile uint ind; 
volatile uint flag;
const char passPack[] = {0xEF, 0x1, 0xFF, 0xFF, 0xFF, 0xFF, 0x1, 0x0, 0x7, 0x13, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1B};

enum
{
	CMD = 0,
	DATA,
};

void buzzer(uint); 

int main()
{
	fingerprint_init();
	serialbegin();
	buzzer(2000);

	lcdbegin();
	lcdprint(" --Safe Locker--");
	lcdwrite(192, CMD);
	lcdprint("Using Atmega32");
	_delay_ms(2000);

	
	ind = 0;
	while (sendcmd2fp((char *)&passPack[0], sizeof(passPack)))
	{
		lcdwrite(1, CMD);
		lcdprint("FP Not Found");
		_delay_ms(2000);
		ind = 0;
	}
	lcdwrite(1, CMD);
	lcdprint("FP Found");
	_delay_ms(1000);
	lcdinst();
	_delay_ms(2000);
	lcdwrite(1, CMD);
	
	while (1)
	{
		matchFinger();
		

		if (enrol == LOW)
		{
			buzzer(200);
			enrolFinger();
			_delay_ms(2000);
			
		}

		else if (delet == LOW)
		{
			buzzer(200);
			getId();
			deleteFinger();
			_delay_ms(1000);
		}
	}
	return 0;
}


