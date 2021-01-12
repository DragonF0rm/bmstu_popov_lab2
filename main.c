/*
 Управление портами ввода/вывода.
 Для правильного связывания модулей отметить
 Options -> Linker -> Use Memory Layout from Target Dialog
*/

#include <LPC23xx.H> /* Описание LPC23xx */

#define STB 26 //Port1.26
#define CLK 27 //Port1.27
#define DIO 28 //Port1.28

void delay(unsigned int count) {
	unsigned int i;
	for (i=0;i<count;i++){}
}

void tm1638_sendbyte(unsigned int x) {
	unsigned int i;
	IODIR1 |= (1<<DIO);//Устанавливаем пин DIO на вывод
	for(i = 0; i < 8; i++) {
		IOCLR1=(1<<CLK); //Сигнал CLK устанавливаем в 0
		delay(0xfff);
		
		if (x&1) {       //Устанавливаем значение на выходе DIO
			IOSET1=(1<<DIO);
		} else {
			IOCLR1=(1<<DIO);
		}
		
		delay(0xfff);
		IOSET1=(1<<CLK); //Сигнал CLK устанавливаем в 1
		
		x >>= 1;
		delay(0x1fff);
	}
}

unsigned int tm1638_receivebyte() {
	unsigned int i;
	unsigned int x=0;
	IODIR1 &= ~(1<<DIO);//Устанавливаем пин DIO на ввод
	for(i = 0; i < 32; i++) {
		IOCLR1=(1<<CLK);//Сигнал CLK устанавливаем в 0
		delay(0xfff);
		
		if (IOPIN1&(1<<DIO)) {
			x |= (1<<i);
		}
		
		delay(0xfff);
		IOSET1=(1<<CLK);//Сигнал CLK устанавливаем в 1
		
		delay(0x1fff);
	}
	return x;
}

void tm1638_sendcmd(unsigned int x) {
	//Устанавливаем пассивный высокий уровень сигнала STB
	IOSET1=(1<<STB);
	//Устанавливаем пины CLK,DIO,STB на вывод
	IODIR1 = (1<<CLK)|(1<<DIO)|(1<<STB);
	//Устанавливаем активный низкий уровень сигнала STB
	IOCLR1=(1<<STB);
	tm1638_sendbyte(x);
}

void tm1638_setadr(unsigned int adr) {
	//Установить адрес регистра LED инидикации
	tm1638_sendcmd(0xC0|adr);
}

void tm1638_init() {
	unsigned int i;
	tm1638_sendcmd(0x88);//Разрешить работу индикации
	//Установить режим адресации: автоинкремент
	//Установить адрес регистра LED инидикации
	tm1638_sendcmd(0x40);
	tm1638_setadr(0);//Сбросить адрес
	for (i=0;i<=0xf;i++)
		tm1638_sendbyte(0);//Установить режим адресации: фиксированный
	tm1638_sendcmd(0x44);
}

unsigned int tm1638_is_S1_pressed() {
	tm1638_sendcmd(0x46);
	return tm1638_receivebyte();
}

#define LED1_ADDR 1
#define LED2_ADDR 3
#define LED3_ADDR 5

#define LED_ON  1
#define LED_OFF 0

int main (void) {
	unsigned int S1;
	tm1638_init();
	
	while (1) {
		tm1638_sendcmd(0x46);
		S1 = tm1638_receivebyte()&1;
		
		if (S1) {
			// button S1 pressed
			tm1638_setadr(LED1_ADDR);
			tm1638_sendbyte(LED_ON);
			
			tm1638_setadr(LED2_ADDR);
			tm1638_sendbyte(LED_OFF);
			
			tm1638_setadr(LED3_ADDR);
			tm1638_sendbyte(LED_OFF);
		} else {
			// button S1 is not pressed
			tm1638_setadr(LED1_ADDR);
			tm1638_sendbyte(LED_OFF);
			
			tm1638_setadr(LED2_ADDR);
			tm1638_sendbyte(LED_ON);
			
			tm1638_setadr(LED3_ADDR);
			tm1638_sendbyte(LED_ON);
		}
		
		delay(0xffff);
	}
}
