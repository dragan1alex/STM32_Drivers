/*
 * pixels.h
 *
 *  Created on: Dec 26, 2020
 *      Author: Andrei
 */

#ifndef INC_PIXELS_H_
#define INC_PIXELS_H_

#define PIXEL_DATA_TIMER 			htim3
#define PIXEL_DATA_CHANNEL			TIM_CHANNEL_1
#define MAX_BRIGHTNESS				100

#define NUMBER_OF_PIXELS 			34
#define PIXEL_RESET_COUNTER_MAX		4

#include "globalVariables.h"
#include "main.h"
#include "settings.h"
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
	uint8_t canFade;
	uint16_t fadeOutDelay;
	uint32_t nextFadeTime;
}Pixel;

typedef struct
{
	uint8_t r,g,b; 			//desired color
	uint8_t ir,ig,ib; 		//initial color
	uint32_t startTime;		//cpu tick count when fade started
	uint32_t fadeDuration;	//in milliseconds
	uint8_t fadeFinished;	//flag to offload CPU when no fade is needed
}PixelFade;

typedef struct
{
	uint8_t buffer[48];
	DMA_INT_STATUS dmaIntStatus;
	DMA_DATA_TYPE dataType;
	uint16_t currentOutputLed;
	uint8_t resetDataCounter;
}PixelData;

Pixel pixel[NUMBER_OF_PIXELS];
PixelFade pixelFade[NUMBER_OF_PIXELS];
PixelData pixelData;

void initPixels();
void setPixel(uint16_t number, Pixel p);
Pixel getPixel(uint16_t number);
void normalizePixel(Pixel* p);
void decreasePixelBrightness(uint16_t number, uint64_t currentTime);
void setPixelBrightness(uint16_t number, uint8_t brightness);
void randomizePixelColor(uint16_t number, uint8_t brightness);
void setPixelColorNormalized(uint16_t number, uint8_t red, uint8_t green, uint8_t blue, uint8_t brightness);
void setPixelColor(uint16_t number, uint8_t red, uint8_t green, uint8_t blue);
void enablePixelsFade();
void disablePixelsFade();
void pixelCanFade(uint16_t number, uint8_t status);
void setNextPixelFadeColor(uint16_t number, uint8_t r, uint8_t g, uint8_t b, uint32_t duration);
void fadePixelColor(uint16_t number,uint32_t currentTime);

void initDmaTransfer();
void HAL_TIM_IC_CaptureHalfCpltCallback(TIM_HandleTypeDef *htim);
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim);
void copyLedToBuffer(uint16_t number, uint8_t* buffer);
void transferDataToDma(DMA_INT_STATUS intStatus);


#endif /* INC_PIXELS_H_ */
