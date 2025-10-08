// Escriba un programa que haga parpadear un color en el LED RGB integrado en la placa (enciende con 0)

#include "LPC17xx.h"
#include <stdint.h>
#include <cr_section_macros.h>

#define RED (1 << 22) // P0.22
#define GREEN (1 << 25) // P3.25
#define BLUE (1 << 26) //P3.26
#define DELAY 10000000

void configPorts(void);
void delay(int delayTime);

int main(){
	configPorts();
	while(1){
		LPC_GPIO0->FIOCLR = RED;
		delay(DELAY);
		LPC_GPIO0->FIOSET = RED;
		delay(DELAY);
		LPC_GPIO3->FIOCLR = GREEN;
		delay(DELAY);
		LPC_GPIO3->FIOSET = GREEN;
		delay(DELAY);
		LPC_GPIO3->FIOCLR = BLUE;
		delay(DELAY);
		LPC_GPIO3->FIOSET = BLUE;
		delay(DELAY);
	}
}

void configPorts(){
	LPC_PINCON->PINSEL1 &= ~(3 << 12);
	LPC_PINCON->PINSEL7 &= ~((3 << 18) | (3 << 20));
	LPC_GPIO0->FIODIR |= (1<<22);
	LPC_GPIO3->FIODIR |= (3<<25);
}

void delay(int delayTime){
	for(int i=0; i<delayTime; i++){}
}
