/*
 * TestLogic.h
 *
 *  Created on: 01-04-2012
 *      Author: radomir.mazon
 */

#ifndef TESTLOGIC_H_
#define TESTLOGIC_H_
#include "safcore.h"

class TestLogic : public EventReceiver {
	void onEvent(uint8_t code, int value);
};

#endif /* TESTLOGIC_H_ */
