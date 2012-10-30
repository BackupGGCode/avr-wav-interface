#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "pff.h"
#include "AudioController.h"
#include <stdlib.h>
#include "safcore.h"
#include "InputButtons.h"
#include "TestLogic.h"

int main()
{
	DDRD = 0x00;
	DDRC = 0xff;
	DDRB = 0xff;

	TestLogic test;
	InputButtons inputButtons;

	sei();
	AudioController::get()->play("b.wav");

	uint8_t count =0;
	while(1) {
		count++;
		AudioController::get()->onTick();
		if (count == 0) {
			inputButtons.onTick();
		}
		EventBus::get()->process();
	}
}
