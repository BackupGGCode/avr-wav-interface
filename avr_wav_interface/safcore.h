/*
 * core.h
 *
 *  Created on: 2010-09-29
 *      Author: rmazon
 *      Ver: 1.2
 *      Diff: (1.0 -> 1.1)
 *      * Clock::setClock
 *		Diff: (1.1 -> 1.2)
 *		* RingBuffer template
 *		Diff: (1.2 -> 1.3)
 *		* Clock interval
 *		* Event object has int value
 *
 *	In this case (avr-wav-interface) file has been modified !
 */

#ifndef CORE_H_
#define CORE_H_

#include "Event.h"
#include <avr/io.h>



//config--------------------------------------------------------------------
#define MAX_OBJECT 4
#define BUFFOR_SIZE 32


//--------------------------------------------------------------------------


class RingBufferModel {
public:
	uint8_t code;
	int value;
	uint8_t empty;
	RingBufferModel() {
		empty = 0;
		code = 0;
		value = 0;
	}
};

class RingBuffer {
public:
	RingBuffer();
	void add(RingBufferModel c);
	RingBufferModel get();
	uint8_t available();
	void flush();
private:
	RingBufferModel buffer[BUFFOR_SIZE];
	uint8_t head;
	uint8_t tail;
};

class EventInterface {
public:
	virtual void onEvent(uint8_t code, int value)=0;
};

class  EventBus {
 private:
	EventInterface* tab[MAX_OBJECT];
	uint8_t index;
	RingBuffer buffer;
	static EventBus instance;
 protected:
	EventBus() {index =0;}
 public:
	static EventBus* get();
	void send(uint8_t code, int value);
	void send(uint8_t code);
	void add(EventInterface* er);
	EventBus& operator=(const EventBus&);
	void process();
};

class EventReceiver : public EventInterface {
 public:
	EventReceiver() {
		EventBus::get()->add(this);
	}
};

class ClockTick {
public:
	virtual void onTick()=0;
};


#endif /* CORE_H_ */
