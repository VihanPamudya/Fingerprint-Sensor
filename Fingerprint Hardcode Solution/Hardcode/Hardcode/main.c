

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#define uchar unsigned char
#define uint unsigned int

#define LCDPORTDIR DDRB
#define LCDPORT PORTB
#define rs 0
#define rw 1
#define en 2

#define RSLow (LCDPORT &= ~(1 << rs))
#define RSHigh (LCDPORT |= (1 << rs))
#define RWLow (LCDPORT &= ~(1 << rw))
#define ENLow (LCDPORT &= ~(1 << en))
#define ENHigh (LCDPORT |= (1 << en))

enum
{
	CMD = 0,
	DATA,
};


void lcdwrite(char ch, char r)
{
	LCDPORT = ch & 0xF0;
	RWLow;
	if (r == 1)
	RSHigh;
	else
	RSLow;
	ENHigh;
	_delay_ms(5);
	ENLow;
	_delay_ms(10);

	LCDPORT = ch << 4 & 0xF0;
	RWLow;
	if (r == 1)
	RSHigh;
	else
	RSLow;
	ENHigh;
	_delay_ms(5);
	ENLow;
	_delay_ms(10);
}

void lcdprint(char *str)
{
	while (*str)
	{
		lcdwrite(*str++, DATA);
		_delay_ms(5);
	}
}

void lcdbegin()
{
	uchar lcdcmd[5] = {0x02, 0x28, 0x0E, 0x06, 0x01};
	uint i = 0;
	for (i = 0; i < 5; i++)
	lcdwrite(lcdcmd[i], CMD);
}

void buzzer(uint t)
{
	PORTD |= (1<<PD3);
	for (int i = 0; i < t; i++)
	_delay_ms(1);
	PORTD &= ~(1<<PD3);
}

void fingerprintright(){
	int input[8] = {1,1,1,1,0,0,0,0};
	int status = 0;

	int storedArray[5][8] = {{1,1,1,0,0,0,0,0},
	{1,0,1,0,0,0,0,0},
	{0,0,1,0,0,0,0,0},
	{1,1,1,0,0,0,1,0},
	{1,1,1,1,0,0,0,0}};


	for(int i=0;i<5;i++){

		for(int j=0;j<8;j++){

			if(input[j]==storedArray[i][j]){
				status++;
			}
		}

		if(status==8){
			break;
			}else{
			status=0;
		}
	}


	if(status==8){
		buzzer(1000);
		PORTD |= (1<<PD0);
		_delay_ms(2000);
		PORTD &= ~(1<<PD0);
		lcdwrite(1, CMD);
		lcdprint("  FP entry is");
		lcdwrite(192, CMD);
		lcdprint("   succeeded");
		_delay_ms(2500);
		lcdwrite(1, CMD);
		lcdprint(" --Safe Locker--");
		lcdwrite(192, CMD);
		lcdprint(" Using Atmega32");
		_delay_ms(2000);
		
		lcdwrite(1, CMD);
		lcdprint("    FP Found");
		_delay_ms(10);
		lcdwrite(192, CMD);
		lcdprint("  Place Finger");
	}

}

void fingerprintwrong(){
	int input[8] = {0,0,0,0,1,1,1,1};
	int status = 0;

	int storedArray[5][8] = {{1,1,1,0,0,0,0,0},
	{1,0,1,0,0,0,0,0},
	{0,0,1,0,0,0,0,0},
	{1,1,1,0,0,0,1,0},
	{1,1,1,1,0,0,0,0}};


	for(int i=0;i<5;i++){

		for(int j=0;j<8;j++){

			if(input[j]==storedArray[i][j]){
				status++;
			}
		}

		if(status==8){
			break;
			}else{
			status=0;
		}
	}


	if(status==0){
		buzzer(1000);
		PORTA |= (1<<PA1);
		_delay_ms(2000);
		PORTA &= ~(1<<PA1);
		lcdwrite(1, CMD);
		lcdprint("  Fingerprint");
		lcdwrite(192, CMD);
		lcdprint("   Not Found");
		_delay_ms(2500);
		lcdwrite(1, CMD);
		lcdprint(" --Safe Locker--");
		lcdwrite(192, CMD);
		lcdprint(" Using Atmega32");
		_delay_ms(2000);
		
		lcdwrite(1, CMD);
		lcdprint("    FP Found");
		_delay_ms(10);
		lcdwrite(192, CMD);
		lcdprint("  Place Finger");
	}
}


int main(){
	LCDPORTDIR |=0b11111111;
	DDRC |=0b11111110;
	DDRA |=0b11111110;
	DDRD |=0b11111111;
	
	buzzer(2000);
	
	lcdbegin();
	lcdprint(" --Safe Locker--");
	lcdwrite(192, CMD);
	lcdprint(" Using Atmega32");
	_delay_ms(2000);
	
	lcdwrite(1, CMD);
	lcdprint("    FP Found");
	_delay_ms(10);
	lcdwrite(192, CMD);
	lcdprint("  Place Finger");
	_delay_ms(2000);
	
	while(1){
		
		if((PINC & (1<<PC0)) == 1){
			
			fingerprintright();
		}
		if((PINA & (1<<PA0)) == 1){
			
			fingerprintwrong();
		}
		
	}
	
}

