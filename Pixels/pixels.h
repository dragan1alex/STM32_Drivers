/*
 * pixels.h
 *
 *  Created on: Dec 26, 2020
 *      Author: Andrei
 *
 *  Pixel library for driving WS2812 and other variants of individually addressable LEDs
 *  Prerequisites:
 *  - set up the timer used for data transfer to run at ~800KHz
 *  - enable the interrupts on the channel used for communicating with the first pixel
 *  - enable DMA transfer from memory to peripherial in circular mode,
 *  	memory address increment,
 *  	data width for peripherial: half word, for memory: byte
 *  - forward declaration of the TIM handle in main.h (copy the line containing htimX from main.c to main.h)
 *
 *  (*) if other interrupts are enabled, you may want to increase the interrupt priority for this timer to a higher value
 *
 *
 *
 */

#ifndef INC_PIXELS_H_
#define INC_PIXELS_H_

/* Setup for the pixel library */
#define PIXEL_DATA_TIMER 			htim3
#define PIXEL_DATA_CHANNEL			TIM_CHANNEL_1
#define MAX_BRIGHTNESS				100
#define COUNTER_PERIOD				80

#define NUMBER_OF_PIXELS 			34
#define PIXEL_RESET_COUNTER_MAX		2

#include "main.h"
#include "stdlib.h"

typedef enum
{
	DMA_HALF_COMPLETE,
	DMA_COMPLETE
}DMA_INT_STATUS;

typedef enum
{
	LED_DATA,
	RESET_DATA
}DMA_DATA_TYPE;

typedef struct
{
	uint8_t r,g,b;
	uint8_t brightness;
}Pixel;

typedef struct
{
	uint8_t buffer[48];
	DMA_INT_STATUS dmaIntStatus;
	DMA_DATA_TYPE dataType;
	uint16_t currentOutputLed;
	uint8_t resetDataCounter;
}PixelData;

Pixel pixel[NUMBER_OF_PIXELS];

void initPixels();
void setPixel(uint16_t number, Pixel p);
Pixel getPixel(uint16_t number);
void setPixelBrightness(uint16_t number, uint8_t brightness);
void setPixelColor(uint16_t number, uint8_t red, uint8_t green, uint8_t blue);

void initDmaTransfer();
void HAL_TIM_IC_CaptureHalfCpltCallback(TIM_HandleTypeDef *htim);
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim);
void copyLedToBuffer(uint16_t number, uint8_t* buffer);
void transferDataToDma(DMA_INT_STATUS intStatus);


#endif /* INC_PIXELS_H_ */
