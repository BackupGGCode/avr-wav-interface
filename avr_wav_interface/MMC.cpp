/*
 * MMC.cpp
 *
 *  Created on: 19-11-2011
 *      Author: radomir.mazon
 */


/* ***********************************************************************
**
**  Copyright (C) 2006  Jesper Hansen <jesper@redegg.net>
**
**
**  Interface functions for MMC/SD cards
**
**  File mmc_if.h
**
**  Hacked by Michael Spiceland at http://tinkerish.com to support
**  writing as well.
**
*************************************************************************
**
**  This program is free software; you can redistribute it and/or
**  modify it under the terms of the GNU General Public License
**  as published by the Free Software Foundation; either version 2
**  of the License, or (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software Foundation,
**  Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
**
*************************************************************************/

/** \file mmc_if.c
	Simple MMC/SD-card functionality
*/

#include <avr/io.h>
#include <inttypes.h>
#include <stdio.h>
#include "MMC.h"

MMC::MMC() {
	init();
}

/** Hardware SPI I/O.
	\param byte Data to send over SPI bus
	\return Received data from SPI bus
*/
uint8_t MMC::spi_byte(uint8_t byte)
{
	SPDR = byte;
	while(!(SPSR & (1<<SPIF)))
	{}
	return SPDR;
}



/** Send a command to the MMC/SD card.
	\param command	Command to send
	\param px	Command parameter 1
	\param py	Command parameter 2
*/
void MMC::send_command(uint8_t command, uint16_t px, uint16_t py)
{
	register union u16convert r;

	MMC_CS_PORT &= ~(1 << MMC_CS);	// enable CS

	spi_byte(0xff);			// dummy byte

	spi_byte(command | 0x40);

	r.value = px;
	spi_byte(r.bytes.high);	// high byte of param x
	spi_byte(r.bytes.low);	// low byte of param x

	r.value = py;
	spi_byte(r.bytes.high);	// high byte of param y
	spi_byte(r.bytes.low);	// low byte of param y

	spi_byte(0x95);			// correct CRC for first command in SPI
							// after that CRC is ignored, so no problem with
							// always sending 0x95
	spi_byte(0xff);			// ignore return byte
}


/** Get Token.
	Wait for and return a non-ff token from the MMC/SD card
	\return The received token or 0xFF if timeout
*/
uint8_t MMC::get(void)
{
	uint16_t i = 0xffff;
	uint8_t b = 0xff;

	while ((b == 0xff) && (--i))
	{
		b = spi_byte(0xff);
	}
	return b;

}

/** Get Datatoken.
	Wait for and return a data token from the MMC/SD card
	\return The received token or 0xFF if timeout
*/
uint8_t MMC::datatoken(void)
{
	uint16_t i = 0xffff;
	uint8_t b = 0xff;

	while ((b != 0xfe) && (--i))
	{
		b = spi_byte(0xff);
	}
	return b;
}


/** Finish Clocking and Release card.
	Send 10 clocks to the MMC/SD card
 	and release the CS line
*/
void MMC::clock_and_release(void)
{
	uint8_t i;

	// SD cards require at least 8 final clocks
	for(i=0;i<10;i++)
		spi_byte(0xff);

    MMC_CS_PORT |= (1 << MMC_CS);	// release CS
}



/** Read MMC/SD sector.
 	Read a single 512 byte sector from the MMC/SD card
	\param lba	Logical sectornumber to read
	\param buffer	Pointer to buffer for received data
	\return 0 on success, -1 on error
*/
int MMC::readsector(uint32_t lba, uint8_t *buffer)
{
	uint16_t i;

	// send read command and logical sector address
	send_command(17,(lba>>7) & 0xffff, (lba<<9) & 0xffff);

	if (datatoken() != 0xfe)	// if no valid token
	{
		clock_and_release();	// cleanup and
		//for (i=0;i<512;i++)				// we don't want a false impression that everything was fine.
    		//	*buffer++ = 0x00;
   		return -1;					// return error code
	}

	for (i=0;i<512;i++)				// read sector data
    	*buffer++ = spi_byte(0xff);

	spi_byte(0xff);					// ignore dummy checksum
	spi_byte(0xff);					// ignore dummy checksum

	clock_and_release();		// cleanup

	return 0;						// return success
}

/************************** MMC get response **************************************/
/**** Repeatedly reads the MMC until we get the response we want or timeout ****/
/* this function taken from the PIC CCS example */
int MMC::response(unsigned char response)
{
        unsigned long count = 0xFFFF;           // 16bit repeat, it may be possible to shrink this to 8 bit but there is not much point

        while(spi_byte(0xFF) != response && --count > 0);

        if(count==0) return 1;                  // loop was exited due to timeout
        else return 0;                          // loop was exited before timeout
}


/** Write MMC/SD sector.
 	Write a single 512 byte sector from the MMC/SD card
	\param lba	Logical sectornumber to write
	\param buffer	Pointer to buffer to write from
	\return 0 on success, -1 on error
*/
int MMC::writesector(uint32_t lba, uint8_t *buffer)
{
	uint16_t i;

	// send read command and logical sector address
	send_command(24,(lba>>7) & 0xffff, (lba<<9) & 0xffff);

	// need wait for resonse here
	if((response(0x00))==1)
	{
		clock_and_release();	// cleanup and
		return -1;
	}

	// need to send token here
	spi_byte(0xfe);					// send data token /* based on PIC code example */

	for (i=0;i<512;i++)				// write sector data
    		spi_byte(*buffer++);

	spi_byte(0xff);					// ignore dummy checksum
	spi_byte(0xff);					// ignore dummy checksum

	// do we check the status here?
	if((spi_byte(0xFF)&0x0F)!=0x05) return -1;

	i = 0xffff;						// max timeout
	while(!spi_byte(0xFF) && (--i)){;} // wait until we are not busy

	clock_and_release();		// cleanup

	return 0;						// return success
}


/** Init MMC/SD card.
	Initialize I/O ports for the MMC/SD interface and
	send init commands to the MMC/SD card
	\return 0 on success, other values on error
*/
uint8_t MMC::init(void)
{
	int i;


	// setup I/O ports

	SPI_PORT &= ~((1 << MMC_SCK) | (1 << MMC_MOSI));	// low bits
	SPI_PORT |= (1 << MMC_MISO);						// high bits
	SPI_DDR  |= (1<<MMC_SCK) | (1<<MMC_MOSI);			// direction


	MMC_CS_PORT |= (1 << MMC_CS);	// Initial level is high
	MMC_CS_DIR  |= (1 << MMC_CS);	// Direction is output


	// also need to set SS as output
#if defined(__AVR_ATmega8__)
	// is already set as CS, but we set it again to accomodate for other boards
	SPI_DDR |= (1<<2);
#else
	SPI_DDR |= (1<<0);			// assume it's bit0 (mega128, portB and others)
#endif

	SPCR = (1<<MSTR)|(1<<SPE);	// enable SPI interface
	SPSR = 1;					// set double speed

	for(i=0;i<10;i++)			// send 80 clocks while card power stabilizes
		spi_byte(0xff);

	send_command(0,0,0);	// send CMD0 - reset card

	if (get() != 1)			// if no valid response code
	{
	   clock_and_release();
	   return 1;  				// card cannot be detected
	}

	//
	// send CMD1 until we get a 0 back, indicating card is done initializing
	//
	i = 0xffff;						// max timeout
	while ((spi_byte(0xff) != 0) && (--i))	// wait for it
	{
	     send_command(1,0,0);	// send CMD1 - activate card init
	}

    clock_and_release();		// clean up

	if (i == 0) {						// if we timed out above
	   return 2;					// return failure code
	}

	return 0;
}
