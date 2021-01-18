/*
 * spi_flash.c
 *
 *  Created on: May 12, 2020
 *      Author: Andrei
 */

#include "spi_flash.h"
#include "main.h"

/*
 * Pull CS pin low to select the flash IC
 */
void selectFlash()
{
	HAL_GPIO_WritePin(SPIFLASH_CS_PORT, SPIFLASH_CS_PIN, GPIO_PIN_RESET);
}

/*
 * Pull CS pin high to release the flash IC
 */
void unselectFlash()
{
	HAL_GPIO_WritePin(SPIFLASH_CS_PORT, SPIFLASH_CS_PIN, GPIO_PIN_SET);
}

/*
 * Receive a byte from the flash IC
 *
 * return: 	byte from flash
 */
uint8_t receiveFlash()
{
	uint8_t byte;
	HAL_SPI_Receive(&SPIFLASH_SPI_DEVICE, &byte, (uint16_t)1, 10);
	return byte;
}

/*
 * Send a byte of data to the flash IC
 *
 * return:	always 1, can be ignored
 */
uint8_t transferFlash(uint8_t data)
{
	HAL_SPI_Transmit(&SPIFLASH_SPI_DEVICE,&data,1,10);
	return 1;
}

/*
 * Read the flash status register
 *
 * return:	status register value
 */
uint8_t readFlashStatus()
{
	selectFlash();
	transferFlash(SPIFLASH_STATUSREAD);
	uint8_t status = receiveFlash();
	unselectFlash();
	return status;
}

/*
 * Check if the flash IC is busy
 *
 * return:	1 if busy, 0 if available
 */
uint8_t isFlashBusy()
{
	return readFlashStatus() & 1;
}

/*
 * Send a command to the flash IC
 *
 * params:	cmd - command to send to the flash
 * 			write - 1 if the command is a write command, 0 otherwise
 */
void commandFlash(uint8_t cmd, uint8_t write)
{
	if(write)
	{
		commandFlash(SPIFLASH_WRITEENABLE, 0);
		unselectFlash();
	}
	if (cmd != SPIFLASH_WAKE)
		while(isFlashBusy());
	selectFlash();
	HAL_SPI_Transmit(&SPIFLASH_SPI_DEVICE, &cmd, 1, 10);
}

/*
 * Read the UDID of the flash IC
 *
 * return:	32bit value of the UDID
 */
uint32_t readFlashID()
{
	selectFlash();
	commandFlash(SPIFLASH_IDREAD, 0);
	uint32_t id = 0;
	transferFlash(0);
	id |= receiveFlash()<<8;
	id |= receiveFlash();
	unselectFlash();
	return id;
}

/*
 * Read a single byte to the flash
 *
 * params:	address - 32bit address of the byte to be read
 *
 * return:	byte from the specified address
 */
uint8_t readByteFlash(uint32_t address)
{
	selectFlash();
	commandFlash(SPIFLASH_ARRAYREAD, 0);
	transferFlash(address >> 16);
	transferFlash(address >> 8);
	transferFlash(address);
	transferFlash(0);
	uint8_t result = receiveFlash(1);
	unselectFlash();
	return result;
}

/*
 * Read multiple bytes from flash
 *
 * params:	address - 32bit start address of the memory to be read
 * 			*ret	- pointer to the structure to receive the data into
 * 			len		- number of bytes to read from flash
 */
void readBytesFlash(uint32_t address, uint8_t* ret, int len)
{
	selectFlash();
	commandFlash(SPIFLASH_ARRAYREAD, 0);
	transferFlash(address >> 16);
	transferFlash(address >> 8);
	transferFlash(address);
	transferFlash(0);
	HAL_SPI_Receive(&SPIFLASH_SPI_DEVICE, ret, len, 100);
	unselectFlash();
	return;
}

/*
 * Write a single byte to flash
 *
 * params:	addr 	- 32bit address at which to write the byte
 * 			byt		- byte to write
 */
void writeByteFlash(uint32_t addr, uint8_t byt) {
	commandFlash(SPIFLASH_BYTEPAGEPROGRAM, 1);
	transferFlash(addr >> 16);
	transferFlash(addr >> 8);
	transferFlash(addr);
	transferFlash(byt);
	unselectFlash();
}

/*
 * Write multiple bytes to flash
 *
 * params:	addr	- 32bit address at which to write the data
 * 			*buf	- data to be stored in flash
 * 			len		- number of bytes to be stored
 */
void writeBytesFlash(uint32_t addr, void* buf, uint16_t len)
{
	uint32_t addressOffset = 0;
	uint32_t currentAddress = addr;
	uint16_t currentTransferSize;
	do
	{
		currentAddress = addr + addressOffset;
		currentTransferSize = (len - addressOffset > 256) ? 256 : len-addressOffset;
		commandFlash(SPIFLASH_BYTEPAGEPROGRAM, 1);
		HAL_SPI_Transmit(&SPIFLASH_SPI_DEVICE, (uint8_t*)&currentAddress, 3, 10);
		HAL_SPI_Transmit(&SPIFLASH_SPI_DEVICE, buf + addressOffset, currentTransferSize, 100);
		unselectFlash();
		addressOffset += 256;
	}while(addressOffset < len);
}

/*
 * Erase the whole flash IC
 */
void chipEraseFlash()
{
	commandFlash(SPIFLASH_CHIPERASE, 1);
	unselectFlash();
}

/*
 * Erase a 4Kb sector of memory from flash
 *
 * params:	addr	- start address of the block
 */
void sectorErase4KFlash(uint32_t addr)
{
	commandFlash(SPIFLASH_BLOCKERASE_4K, 1);
	transferFlash(addr >> 16);
	transferFlash(addr >> 8);
	transferFlash(addr);
	unselectFlash();
}

/*
 * Erase a 32Kb block of memory from flash
 *
 * params:	addr	- start address of the block
 */
void blockErase32KFlash(uint32_t addr)
{
	commandFlash(SPIFLASH_BLOCKERASE_32K, 1);
	transferFlash(addr >> 16);
	transferFlash(addr >> 8);
	transferFlash(addr);
	unselectFlash();
}

/*
 * Erase a 64Kb block of memory from flash
 *
 * params:	addr	- start address of the block
 */
void blockErase64KFlash(uint32_t addr)
{
	commandFlash(SPIFLASH_BLOCKERASE_64K, 1);
	transferFlash(addr >> 16);
	transferFlash(addr >> 8);
	transferFlash(addr);
	unselectFlash();
}

/*
 * Put the flash IC in sleep mode
 */
void sleepFlash()
{
	commandFlash(SPIFLASH_SLEEP, 0);
	unselectFlash();
}

/*
 * Wake up the flash IC from sleep mode
 */
void wakeUpFlash()
{
	commandFlash(SPIFLASH_WAKE, 0);
	unselectFlash();
}
