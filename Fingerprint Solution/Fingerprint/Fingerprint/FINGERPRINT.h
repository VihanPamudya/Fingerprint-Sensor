

#ifndef FINGERPRINT_FILE_H_
#define FINGERPRINT_FILE_H_
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>

void fingerprint_init();
void lcdwrite(char ch, char r);
void lcdprint(char *str);
void lcdbegin();
void serialbegin();
void serialwrite(char ch);
void serialprint(char *str);
void serialprintln(char *str);
void serialFlush();
int bcdtochar(char num);
int eeprom_write(unsigned int add, unsigned char data);
char eeprom_read(unsigned int add);
void saveData(int id);
int sendcmd2fp(char *pack, int len);
unsigned int getId();
void buzzer(unsigned int t);
void matchFinger();
void enrolFinger();
void deleteFinger();
void lcdinst();

#endif /* FINGERPRINT_FILE_H_ */