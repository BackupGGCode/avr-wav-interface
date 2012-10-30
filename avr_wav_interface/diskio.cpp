/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for Petit FatFs (C)ChaN, 2009      */
/*-----------------------------------------------------------------------*/

#include <avr/io.h>
#include <inttypes.h>
#include <stdio.h>
#include "diskio.h"
#include "mix.h"



/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (void)
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
	   return STA_NODISK;  	// card cannot be detected
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
	   return STA_NOINIT;					// return failure code
	}

	return 0;
}



/*-----------------------------------------------------------------------*/
/* Read Partial Sector                                                   */
/*-----------------------------------------------------------------------*/
DRESULT disk_readp (
	BYTE* dest,			/* Pointer to the destination object */
	DWORD sector,		/* Sector number (LBA) */
	WORD sofs,			/* Offset in the sector */
	WORD count			/* Byte count (bit15:destination) */
) {
	return disk_readp(dest, sector, sofs, count, 0);
}


DRESULT disk_readp (
	BYTE* dest,			/* Pointer to the destination object */
	DWORD sector,		/* Sector number (LBA) */
	WORD sofs,			/* Offset in the sector */
	WORD count,			/* Byte count (bit15:destination) */
	BYTE mix			/* boolean - okresla czy miksowac dane z danymi w dest*/
)
{
	uint16_t i;
	// send read command and logical sector address
	send_command(17,(sector>>7) & 0xffff, (sector<<9) & 0xffff);

	if (datatoken() != 0xfe)	// if no valid token
	{
		clock_and_release();	// cleanup and
		//for (i=0;i<512;i++)				// we don't want a false impression that everything was fine.
    		//	*buffer++ = 0x00;
   		return RES_ERROR;					// return error code
	}

	for (i=0;i<512;i++)	{			// read sector data
		BYTE d = spi_byte(0xff);
		if (sofs<=i && (sofs+count)>i) {
			if (mix==0) {
				*dest++ = d;
			} else {
				*dest++ = mix_audio8bit(*dest, d);
			}
		}
	}

	spi_byte(0xff);					// ignore dummy checksum
	spi_byte(0xff);					// ignore dummy checksum

	clock_and_release();		// cleanup

	return RES_OK;						// return success
}



/*-----------------------------------------------------------------------*/
/* Write Partial Sector                                                  */
/*-----------------------------------------------------------------------*/

DRESULT disk_writep (
	BYTE* buff,		/* Pointer to the data to be written, NULL:Initiate/Finalize write operation */
	DWORD sc		/* Sector number (LBA) or Number of bytes to send */
)
{
	DRESULT res;


	if (!buff) {
		if (sc) {

			// Initiate write process

		} else {

			// Finalize write process

		}
	} else {

		// Send data to the disk

	}

	return res;
}



/** Hardware SPI I/O.
	\param byte Data to send over SPI bus
	\return Received data from SPI bus
*/
uint8_t spi_byte(uint8_t byte)
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
void send_command(uint8_t command, uint16_t px, uint16_t py)
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
uint8_t get(void)
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
uint8_t datatoken(void)
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

void clock_and_release(void)
{
	uint8_t i;

	// SD cards require at least 8 final clocks
	for(i=0;i<10;i++)
		spi_byte(0xff);

    MMC_CS_PORT |= (1 << MMC_CS);	// release CS
}

