/*
 * AudioController.cpp
 *
 *  Created on: 01-01-2012
 *      Author: radomir.mazon
 */

#include "AudioController.h"
#include <avr/interrupt.h>

AudioController AudioController::instance;

AudioController::AudioController() {
	sdReady = FR_NOT_READY;
	ch1State = 0;
	ch2State = 0;
	index[0] = BUFF_SIZE;
	index[1] = BUFF_SIZE;
	/**
	 * Odpowiedzialny za samplowanie
	 */
	initTimer0();
	/**
	 * Odpowiedzialny za generowanie PWM
	 */
	initTimer1();

	//montowanie karty sd;

	if (pff.pf_mount(&fatfs) == FR_OK && pff2.pf_mount(&fatfs2) == FR_OK) {
		sdReady = FR_OK;
	}
}


void AudioController::play(const char* fileName) {
	if (pff.pf_open(fileName) == FR_OK) {
		pff.pf_lseek(0x44);
		ch1State = 1;
	}
}

void AudioController::startLoop(const char* fileName) {
	if (pff2.pf_open(fileName) == FR_OK) {
		pff2.pf_lseek(0x44);
		ch2State = 1;
	}

}

void AudioController::stopLoop() {

}

void AudioController::initTimer0() {
	//prescaler 8
	TCCR0 = 1<< CS01;
	TIMSK |= _BV(TOIE0);

}

void AudioController::initTimer1() {
	//WGM - fast PWM 8-bit
	//COM - clean on compare match
	//Wyjscie to:
	TCCR1A = (1 << COM1A1) | (1 << WGM12) | (1 << WGM10);
	TCCR1B = 1 << CS10;
	OCR1A = 127;
}

void AudioController::loadSample(uint8_t i) {
	WORD resCount = BUFF_SIZE;
	WORD resCount2 = BUFF_SIZE;
	if (index[i] >= BUFF_SIZE)
	{

		if (ch1State) {
			pff.pf_read(buff[i], BUFF_SIZE, &resCount);
		}
		if (ch2State) {
			pff2.pf_read(buff[i], BUFF_SIZE, &resCount2, ch1State);
		}

		index[i] = 0;
	}
	if (resCount < BUFF_SIZE) {
		ch1State = 0;
	}
	if (resCount2 < BUFF_SIZE) {
		ch2State = 0;
	}

}

void AudioController::onSample() {
	if (index[buffSelector%2] == BUFF_SIZE) {
		buffSelector++;
	}
	uint8_t selector = buffSelector%2;

	if (index[selector] < BUFF_SIZE) {
		OCR1A = buff[selector][index[selector]];
		index[selector]++;
	}
}

void AudioController::onTick() {
		loadSample(0);
		loadSample(1);
}

SIGNAL(TIMER0_OVF_vect) {
	//częstotliwosc samplowania (22050Hz)
	//TCNT0 = 211;
	TCNT0 = 165;
	//on
	//PORTC = PORTC & ~0x05;
	AudioController::get()->onSample();
}
