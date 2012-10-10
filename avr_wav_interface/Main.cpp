#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>


int main()
{
	DDRD = 0x00;
	DDRC = 0xff;
	DDRB = 0xff;


	while(1) {

	}
}
