/*
 * AudioController.h
 *
 *  Created on: 01-01-2012
 *      Author: radomir.mazon
 */

#ifndef AUDIOCONTROLLER_H_
#define AUDIOCONTROLLER_H_

#include "pff.h"
#include "integer.h"
#include <avr/io.h>

#define BUFF_SIZE 200

class AudioController {
public:
	AudioController();
	static AudioController* get() {return &instance;}
	void onSample();
	void onTick();

	void play(const char*);
	void startLoop(const char*);
	void stopLoop();

private:
	uint16_t temp;
	static AudioController instance;
	Pff pff;
	Pff pff2;
	FATFS fatfs;				/* Petit-FatFs work area */
	FATFS fatfs2;				/* Petit-FatFs work area */
	BYTE buff[2][BUFF_SIZE];	/* Page data buffer */
	uint8_t buffSelector;
	uint8_t index[2];
	FRESULT sdReady;

	void initTimer0();
	void initTimer1();
	void loadSample(uint8_t i);

	uint8_t ch1State;
	uint8_t ch2State;
};

#endif /* AUDIOCONTROLLER_H_ */
