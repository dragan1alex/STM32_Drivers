/*
 * SPI Flash HAL driver for interfacing with SPI flash devices (designed for Winbond flash ICs like W25X40CL)
 *
 * Prerequisites:
 *  - main.h must contain the forward declaration of the hspi instance used for communicating with the flash
 *
 * Usage:
 *  - read data from the flash: 	readBytesFlash
 *  - write data to the flash: 		writeBytesFlash
 *  	- before a write operation you may want to erase that specific sector or block of memory first with
 *  	sectorErase4KFlash, blockErase32KFlash, blockErase64KFlash or chipEraseFlash for the whole memory
 */

#ifndef SPI_FLASH_H
#define SPI_FLASH_H

#include "main.h"

/* Setup defines, modify with your own hspi instance and CS GPIO port and pin */
#define SPIFLASH_CS_PORT			GPIOB
#define SPIFLASH_CS_PIN				GPIO_PIN_12
#define SPIFLASH_SPI_DEVICE			hspi2
/* End of setup variables */

#define SPIFLASH_WRITEENABLE      	0x06        // write enable
#define SPIFLASH_WRITEDISABLE     	0x04        // write disable

#define SPIFLASH_BLOCKERASE_4K    	0x20        // erase one 4K block of flash memory
#define SPIFLASH_BLOCKERASE_32K   	0x52        // erase one 32K block of flash memory
#define SPIFLASH_BLOCKERASE_64K   	0xD8        // erase one 64K block of flash memory
#define SPIFLASH_CHIPERASE        	0x60        // chip erase (may take several seconds depending on size)
/* but no actual need to wait for completion (instead need to check the status register BUSY bit)*/
#define SPIFLASH_STATUSREAD       	0x05        // read status register
#define SPIFLASH_STATUSWRITE      	0x01        // write status register
#define SPIFLASH_ARRAYREAD        	0x0B        // read array (fast, need to add 1 dummy byte after 3 address bytes)
#define SPIFLASH_ARRAYREADLOWFREQ 	0x03        // read array (low frequency)

#define SPIFLASH_SLEEP            	0xB9        // deep power down
#define SPIFLASH_WAKE             	0xAB        // deep power wake up
#define SPIFLASH_BYTEPAGEPROGRAM  	0x02        // write (1 to 256bytes)
#define SPIFLASH_IDREAD           	0x9f        // read JEDEC manufacturer and device ID (2 bytes, specific bytes for each manufacturer and device)
/* Example for Atmel-Adesto 4Mbit AT25DF041A: 0x1F44 (page 27: http://www.adestotech.com/sites/default/files/datasheets/doc3668.pdf)*/
/* Example for Winbond 4Mbit W25X40CL: 0xEF30 (page 14: http://www.winbond.com/NR/rdonlyres/6E25084C-0BFE-4B25-903D-AE10221A0929/0/W25X40CL.pdf)*/
#define SPIFLASH_MACREAD          	0x4B        // read unique ID number (MAC)

#include "main.h"

void selectFlash();

uint8_t receiveFlash();

uint8_t transferFlash(uint8_t data);

void unselectFlash();

uint8_t readFlashStatus();

uint8_t isFlashBusy();

void commandFlash(uint8_t cmd, uint8_t write);

uint32_t readFlashID();

uint8_t readByteFlash(uint32_t address);

void readBytesFlash(uint32_t address, uint8_t* ret, int len);

void writeByteFlash(uint32_t addr, uint8_t byt);

void writeBytesFlash(uint32_t addr, void* buf, uint16_t len);

void chipEraseFlash();

/* erase a 4Kbyte sector in a block*/
void sectorErase4KFlash(uint32_t addr);

/* erase a 32Kbyte block*/
void blockErase32KFlash(uint32_t addr);

/* erase a 64Kbyte block*/
void blockErase64KFlash(uint32_t addr);

void sleepFlash();

void wakeUpFlash();

#endif
