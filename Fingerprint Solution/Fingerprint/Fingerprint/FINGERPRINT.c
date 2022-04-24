#include "FINGERPRINT.h"

#define USART_BAUDRATE 9600 
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)

#define uchar unsigned char
#define uint unsigned int

#define LCDPORTDIR DDRB
#define LCDPORT PORTB

#define KeyPORTDIR DDRA
#define KeyPORT PORTA
#define key PINA

#define LEDDIR DDRC
#define LEDPORT PORTC

#define rs 0
#define rw 1
#define en 2

#define RSLow (LCDPORT &= ~(1 << rs))
#define RSHigh (LCDPORT |= (1 << rs))
#define RWLow (LCDPORT &= ~(1 << rw))
#define ENLow (LCDPORT &= ~(1 << en))
#define ENHigh (LCDPORT |= (1 << en))

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

#define BUZ 2
#define LED 3

#define BUZHigh (LEDPORT |= (1 << BUZ))
#define BUZLow (LEDPORT &= ~(1 << BUZ))

#define LEDHigh (LEDPORT |= (1 << LED))
#define LEDLow (LEDPORT &= ~(1 << LED))


#define PASS 0
#define ERROR 1

#define HIGH 1
#define LOW 0

uchar buf[20];
uchar buf1[20];
volatile uint ind;
volatile uint flag;
uchar data[10];
uint id = 1;

const char f_detect[] = {0xEF, 0x1, 0xFF, 0xFF, 0xFF, 0xFF, 0x1, 0x0, 0x3, 0x1, 0x0, 0x5};
const char f_imz2ch1[] = {0xEF, 0x1, 0xFF, 0xFF, 0xFF, 0xFF, 0x1, 0x0, 0x4, 0x2, 0x1, 0x0, 0x8};
const char f_search[] = {0xEF, 0x1, 0xFF, 0xFF, 0xFF, 0xFF, 0x1, 0x0, 0x8, 0x1B, 0x1, 0x0, 0x0, 0x0, 0xA3, 0x0, 0xC8};
const char f_imz2ch2[] = {0xEF, 0x1, 0xFF, 0xFF, 0xFF, 0xFF, 0x1, 0x0, 0x4, 0x2, 0x2, 0x0, 0x9};
const char f_createModel[] = {0xEF, 0x1, 0xFF, 0xFF, 0xFF, 0xFF, 0x1, 0x0, 0x3, 0x5, 0x0, 0x9};
char f_storeModel[] = {0xEF, 0x1, 0xFF, 0xFF, 0xFF, 0xFF, 0x1, 0x0, 0x6, 0x6, 0x1, 0x0, 0x1, 0x0, 0xE};
char f_delete[] = {0xEF, 0x1, 0xFF, 0xFF, 0xFF, 0xFF, 0x1, 0x0, 0x7, 0xC, 0x0, 0x0, 0x0, 0x1, 0x0, 0x15};


int timeStamp[7], day;

enum
{
	CMD = 0,
	DATA,
};


void fingerprint_init(){
	LEDDIR = 0xFF;
	LEDPORT = 0x03; 
	KeyPORTDIR = 0xF0;
	KeyPORT = 0x0F; 
	LCDPORTDIR = 0xFF; 
	DDRD += 1 << 7;
	PORTD += 1 << 7;
}

void lcdwrite(char ch, char r) 
{
	LCDPORT = ch & 0xF0;
	RWLow; //0: write
	if (r == 1)
	RSHigh; //1: data register
	else
	RSLow; //0: command reg
	ENHigh;
	_delay_ms(5);
	ENLow;
	_delay_ms(10);

	LCDPORT = ch << 4 & 0xF0;
	RWLow; //0: write
	if (r == 1)
	RSHigh; //1: data register
	else
	RSLow; //0: command reg
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



void serialbegin()
{
	UCSRB = (1 << RXEN) | (1 << TXEN) | (1 << RXCIE); 
	UCSRC = (1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1); 
	UBRRL = BAUD_PRESCALE; 
	UBRRH = (BAUD_PRESCALE >> 8); 
	
	
	sei(); 
}


void serialwrite(char ch)
{
	while ((UCSRA & (1 << UDRE)) == 0); 
	UDR = ch;
}

void serialprint(char *str)
{
	while (*str)
	{
		serialwrite(*str++);
	}
}

void serialprintln(char *str)
{
	serialprint(str);
	serialwrite(0x0d);
	serialwrite(0x0a);
}

void serialFlush()
{
	for (int i = 0; i < sizeof(buf); i++)
	{
		buf[i] = 0;
	}
}

int bcdtochar(char num)
{
	return ((num / 16 * 10) + (num % 16));
}

int eeprom_write(unsigned int add, unsigned char data)
{
	while (EECR & (1 << EEWE));
	EEAR = add;
	EEDR = data;
	EECR |= (1 << EEMWE);
	EECR |= (1 << EEWE);
	return 0;
}

char eeprom_read(unsigned int add)
{
	while (EECR & (1 << EEWE));
	EEAR = add;
	EECR |= (1 << EERE);
	return EEDR;
}

void saveData(int id)
{
	uint cIndex = eeprom_read(id);
	if (cIndex == 0)
	cIndex = 1;
	uint cAddress = (cIndex * 6) + (id - 1) * 48;

	for (int i = 0; i < 6; i++)
	eeprom_write(cAddress + i, timeStamp[i]);
	eeprom_write(id, cIndex + 1);
}

int sendcmd2fp(char *pack, int len)
{
	int res = ERROR;
	serialFlush();
	ind = 0;
	_delay_ms(100);
	for (int i = 0; i < len; i++)
	{
		serialwrite(*(pack + i));
	}
	_delay_ms(1000);
	if (flag == 1)
	{
		if (buf[0] == 0xEF && buf[1] == 0x01)
		{
			if (buf[6] == 0x07)
			{
				if (buf[9] == 0)
				{
					uint data_len = buf[7];
					data_len <<= 8;
					data_len |= buf[8];
					for (int i = 0; i < data_len; i++)
					data[i] = 0;
					
					for (int i = 0; i < data_len - 2; i++)
					{
						data[i] = buf[10 + i];
					}
					res = PASS;
				}

				else
				{
					res = ERROR;
				}
			}
		}
		ind = 0;
		flag = 0;
		return res;
	}
	return res;
}

uint getId()
{
	uint id = 0;
	lcdwrite(1, CMD);
	while (1)
	{
		//check(id);
		if (up == LOW)
		{
			id++;
			buzzer(200);
		}
		else if (down == LOW)
		{
			id--;
			if (id == 0)
			id = 0;
			buzzer(200);
		}
		else if (ok == LOW)
		{
			buzzer(200);
			return id;
		}
		lcdwrite(0x80, CMD);
		(void)sprintf((char *)buf1, "Enter Id:%d ", id);
		lcdprint((char *)buf1);
		_delay_ms(200);
	}
}

void buzzer(uint t)
{
	BUZHigh;
	for (int i = 0; i < t; i++)
	_delay_ms(1);
	BUZLow;
}

void matchFinger()
{
	if (!sendcmd2fp((char *)&f_detect[0], sizeof(f_detect)))
	{
		if (!sendcmd2fp((char *)&f_imz2ch1[0], sizeof(f_imz2ch1)))
		{
			if (!sendcmd2fp((char *)&f_search[0], sizeof(f_search)))
			{
				LEDHigh;
				buzzer(200);
				uint id = data[0];
				id <<= 8;
				id += data[1];
				uint score = data[2];
				score <<= 8;
				score += data[3];
				(void)sprintf((char *)buf1, "Id: %d", (int)id);
				lcdwrite(1, CMD);
				lcdprint((char *)buf1);

				saveData(id);

				_delay_ms(1000);
				lcdwrite(1, CMD);
				lcdprint("Fingerprint entry is");
				lcdwrite(192, CMD);
				lcdprint("succeeded");
				_delay_ms(2000);
				LEDLow;
			}

			else
			{
				LEDHigh;
				lcdwrite(1, CMD);
				lcdprint("Not Found");
				buzzer(5000);
				LEDLow;
			}
		}
		else
		{
			LEDHigh;
			lcdwrite(1, CMD);
			lcdprint("Not Found");
			buzzer(2000);
			LEDLow;
		}
	}
}

void enrolFinger()
{
	lcdwrite(1, CMD);
	lcdprint("Enroll Finger");
	_delay_ms(2000);
	lcdwrite(1, CMD);
	lcdprint("Place Finger");
	lcdwrite(192, CMD);
	_delay_ms(1000);
	for (int i = 0; i < 3; i++)
	{
		if (!sendcmd2fp((char *)&f_detect[0], sizeof(f_detect)))
		{
			if (!sendcmd2fp((char *)&f_imz2ch1[0], sizeof(f_imz2ch1)))
			{
				lcdwrite(192, CMD);
				lcdprint("Finger Detected");
				_delay_ms(1000);
				lcdwrite(1, CMD);
				lcdprint("Place Finger");
				lcdwrite(192, CMD);
				lcdprint("    Again   ");
				_delay_ms(2000);
				if (!sendcmd2fp((char *)&f_detect[0], sizeof(f_detect)))
				{
					if (!sendcmd2fp((char *)&f_imz2ch2[0], sizeof(f_imz2ch2)))
					{
						lcdwrite(1, CMD);
						lcdprint("Finger Detected");
						_delay_ms(1000);
						if (!sendcmd2fp((char *)&f_createModel[0], sizeof(f_createModel)))
						{
							id = getId();
							f_storeModel[11] = (id >> 8) & 0xff;
							f_storeModel[12] = id & 0xff;
							f_storeModel[14] = 14 + id;
							if (!sendcmd2fp((char *)&f_storeModel[0], sizeof(f_storeModel)))
							{
								buzzer(200);
								lcdwrite(1, CMD);
								lcdprint("Finger Stored");
								(void)sprintf((char *)buf1, "Id : %d", (int)id);
								lcdwrite(192, CMD);
								lcdprint((char *)buf1);
								_delay_ms(1000);
							}

							else
							{
								lcdwrite(1, CMD);
								lcdprint("Finger Not Stored");
								buzzer(3000);
							}
						}
						else
						lcdprint("Error");
					}
					else
					lcdprint("Error");
				}
				else
				i = 2;
			}
			break;
		}
		if (i == 2)
		{
			lcdwrite(0xc0, CMD);
			lcdprint("No Finger");
		}
	}
	_delay_ms(2000);
}

void deleteFinger()
{
	id = getId();
	f_delete[10] = id >> 8 & 0xff;
	f_delete[11] = id & 0xff;
	f_delete[14] = (21 + id) >> 8 & 0xff;
	f_delete[15] = (21 + id) & 0xff;
	if (!sendcmd2fp(&f_delete[0], sizeof(f_delete)))
	{
		lcdwrite(1, CMD);
		sprintf((char *)buf1, "Finger ID %d ", id);
		lcdprint((char *)buf1);
		lcdwrite(192, CMD);
		lcdprint("Deleted Success");
	}
	else
	{
		lcdwrite(1, CMD);
		lcdprint("Error");
	}
	_delay_ms(2000);
}

void lcdinst()
{
	lcdwrite(0x80, CMD);
	lcdprint("1-Enroll Finger");
	lcdwrite(0xc0, CMD);
	lcdprint("2-delete Finger");
	_delay_ms(10);
}

ISR(USART_RXC_vect)
{
	char ch = UDR;
	buf[ind++] = ch;
	if (ind > 0)
	flag = 1;
}
