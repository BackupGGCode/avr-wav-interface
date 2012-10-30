/*
 * TestLogic.cpp
 *
 *  Created on: 01-04-2012
 *      Author: radomir.mazon
 */

#include "TestLogic.h"
#include "Event.h"
#include "AudioController.h"

void TestLogic::onEvent(uint8_t code, int value) {

	if (code == EVENT_BUTTON_UP) {
		if (value == 0) {
			AudioController::get()->play("ldo.wav");
			PORTC = PORTC & ~(1<<PIN5);
		}
		if (value == 1) {
			AudioController::get()->play("rdo.wav");
		}
	}

	if (code == EVENT_BUTTON_DOWN) {
		if (value == 0) {
			AudioController::get()->play("ldc.wav");
			PORTC = PORTC | (1<<PIN5);
		}
		if (value == 1) {
			AudioController::get()->play("rdc.wav");
		}
		if (value == 2) {
			AudioController::get()->play("rsd.wav");
		}
		if (value == 3) {
			AudioController::get()->play("lsd.wav");
		}
	}
}
