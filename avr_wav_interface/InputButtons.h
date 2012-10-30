/*
 * InputButtons.h
 *
 *  Created on: 23-09-2011
 *      Author: radomir.mazon
 */

#ifndef INPUTBUTTONS_H_
#define INPUTBUTTONS_H_

#include "safcore.h"
#include <avr/io.h>

#define INPUT_SIZE 4

#define _DDR 1
#define _PORT 2

//ATMEGA8
#define _B    0x16
#define _C    0x13
#define _D    0x10

//ATMEGA168
//#define _B    0x03
//#define _C    0x06
//#define _D    0x09

class InputButtons : public ClockTick {
public:
	InputButtons();
	void onTick();

private:
	uint8_t state[INPUT_SIZE];
	uint8_t bit[INPUT_SIZE];
	uint8_t sfr[INPUT_SIZE];
	void setup();
};

#endif /* INPUTBUTTONS_H_ */
