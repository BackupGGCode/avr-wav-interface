/*
 * InputButtons.cpp
 *
 *  Created on: 23-09-2011
 *      Author: radomir.mazon
 */

#include "InputButtons.h"
#include "safcore.h"

InputButtons::InputButtons() {
	sfr[0] = _D;
	bit[0] = 0;
	sfr[1] = _D;
	bit[1] = 1;
	sfr[2] = _D;
	bit[2] = 2;
	sfr[3] = _D;
	bit[3] = 3;

	setup();
}

void InputButtons::setup() {
	for (uint8_t i=0; i<INPUT_SIZE; i++) {
		//DDR
		_SFR_IO8(sfr[i] + _DDR) &= ~(1<<bit[i]);
		_SFR_IO8(0x0A) &= ~(1<<0);
		//PULL-UP
		_SFR_IO8(sfr[i] + _PORT) |= 1<<bit[i];
		//state
		state[i] = bit_is_clear(_SFR_IO8(sfr[i]), bit[i]);
	}
}

void InputButtons::onTick() {
	for (uint8_t i=0; i<INPUT_SIZE; i++) {
		uint8_t curState = bit_is_clear(_SFR_IO8(sfr[i]), bit[i]);
		if (state[i] != curState)
		{
			state[i] = curState;
			if (curState == 1) {
				EventBus::get()->send(EVENT_BUTTON_DOWN, i);
			} else {
				EventBus::get()->send(EVENT_BUTTON_UP, i);
			}
		}
	}
}
