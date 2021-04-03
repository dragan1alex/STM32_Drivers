/*
 * pixels.c
 *
 *  Created on: Dec 26, 2020
 *      Author: Andrei
 */

#include "pixels.h"

Pixel pixel[NUMBER_OF_PIXELS];
PixelFade pixelFade[NUMBER_OF_PIXELS];
PixelData pixelData;

void initPixels()
{
	uint16_t i;
	for(i=0;i<NUMBER_OF_PIXELS;i++)
	{
		//randomizePixelColor(i, MAX_BRIGHTNESS);
	}
	disablePixelsFade();
	initDmaTransfer();
}

void setPixel(uint16_t number, Pixel p)
{
	if(number < NUMBER_OF_PIXELS)
		pixel[number] = p;
}

Pixel getPixel(uint16_t number)
{
	Pixel error;
	error.brightness = 0;
	if(number >= NUMBER_OF_PIXELS)
		return error;
	return pixel[number];
}

/*
 * Sets the red, green and blue channels to the maximum brightness, keeping the ratio between them
 * Also sets the fade out delay for the pixel
 */
void normalizePixel(Pixel* p)
{
	uint8_t max = 0;
	float ratio;
	if(p->r > max)
	{
		max = p->r;
	}
	if(p->g > max)
	{
		max = p->g;
	}
	if(p->b > max)
	{
		max = p->b;
	}
	if(max == 0)
	{
		p->r = 255;
		p->g = 255;
		p->b = 255;
		max = 255;
	}
	ratio = ((float)p->brightness * 255.0 / 100.0) / (float)max;
	p->r = ratio * p->r;
	p->g = ratio * p->g;
	p->b = ratio * p->b;
	p->fadeOutDelay = settings.fadeOutTime / p->brightness;
}

void decreasePixelBrightness(uint16_t number, uint64_t currentTime)
{
	if(number >= NUMBER_OF_PIXELS)
		return;
	if(pixel[number].canFade == FALSE)
		return;
	if(pixel[number].brightness == 0)
		return;
	if(currentTime >= pixel[number].nextFadeTime)
	{
		pixel[number].brightness--;
		pixel[number].nextFadeTime = currentTime + pixel[number].fadeOutDelay;
	}
}

void setPixelBrightness(uint16_t number, uint8_t brightness)
{
	if(number >= NUMBER_OF_PIXELS || brightness > MAX_BRIGHTNESS)
		return;
	pixel[number].brightness = brightness;
	if(brightness == 0)
		return;
	pixel[number].fadeOutDelay = settings.fadeOutTime / pixel[number].brightness;
}

void randomizePixelColor(uint16_t number, uint8_t brightness)
{
	if(number >= NUMBER_OF_PIXELS)
		return;
	Pixel* p = &pixel[number];
	p->r = rand() % 256;
	p->g = rand() % 256;
	p->b = rand() % 256;
	p->brightness = brightness;
	normalizePixel(p);
}

void setPixelColorNormalized(uint16_t number, uint8_t red, uint8_t green, uint8_t blue, uint8_t brightness)
{
	if(number >= NUMBER_OF_PIXELS || brightness > 100)
		return;
	Pixel* p = &pixel[number];
	p->r = red;
	p->g = green;
	p->b = blue;
	p->brightness = brightness;
	normalizePixel(p);
}

void setPixelColor(uint16_t number, uint8_t red, uint8_t green, uint8_t blue)
{
	if(number >= NUMBER_OF_PIXELS)
		return;
	Pixel *p = &pixel[number];
	p->r = red;
	p->g = green;
	p->b = blue;
}

void enablePixelsFade()
{
	uint16_t i;
	for(i=0;i<NUMBER_OF_PIXELS;i++)
	{
		pixel[i].canFade = TRUE;
	}
}

void disablePixelsFade()
{
	uint16_t i;
	for(i=0;i<NUMBER_OF_PIXELS;i++)
	{
		pixel[i].canFade = FALSE;
		pixelFade[i].fadeFinished = TRUE;
	}
}

void pixelCanFade(uint16_t number, uint8_t status)
{
	if(number >= NUMBER_OF_PIXELS)
		return;
	if(status)
		pixel[number].canFade = TRUE;
	else
		pixel[number].canFade = FALSE;
}

void setNextPixelFadeColor(uint16_t number, uint8_t r, uint8_t g, uint8_t b, uint32_t duration)
{
	if(number >= NUMBER_OF_PIXELS)
		return;
	pixelFade[number].ir = pixel[number].r;
	pixelFade[number].ig = pixel[number].g;
	pixelFade[number].ib = pixel[number].b;
	pixelFade[number].r = r;
	pixelFade[number].g = g;
	pixelFade[number].b = b;
	pixelFade[number].startTime = osKernelGetTickCount();
	pixelFade[number].fadeDuration = duration;
	pixelFade[number].fadeFinished = FALSE;
}

void fadePixelColor(uint16_t number, uint32_t currentTime)
{
	if(number >= NUMBER_OF_PIXELS)
		return;
	if(pixelFade[number].fadeFinished)
		return;
	uint8_t r,g,b;
	float divider = 0;
	divider = (float)(currentTime - pixelFade[number].startTime) / (float)pixelFade[number].fadeDuration;
	r = pixelFade[number].ir + ((int16_t)pixelFade[number].r - (int16_t)pixelFade[number].ir) * divider;
	g = pixelFade[number].ig + ((int16_t)pixelFade[number].g - (int16_t)pixelFade[number].ig) * divider;
	b = pixelFade[number].ib + ((int16_t)pixelFade[number].b - (int16_t)pixelFade[number].ib) * divider;
	setPixelColor(number, r, g, b);
	setPixelBrightness(number, settings.maxBrightness);
	if(divider >= 1)
		pixelFade[number].fadeFinished = TRUE;
}

/*
 * Below are the functions responsible for transferring the Pixel data to the pixels
 * You can modify the parameters required for data transfer in pixels.h
 * If the pixels light up fine don't mess around with the below functions
 */

/*
 * Needs to be called after MCU boot to start the asynchronous pixel data transfer
 */
void initDmaTransfer()
{
	pixelData.currentOutputLed = 2;
	pixelData.dataType = LED_DATA;
	pixelData.dmaIntStatus = DMA_COMPLETE;
	copyLedToBuffer(0, &pixelData.buffer[0]);
	copyLedToBuffer(1, &pixelData.buffer[24]);
	HAL_TIM_PWM_Start_DMA(&PIXEL_DATA_TIMER, PIXEL_DATA_CHANNEL, (uint32_t*)pixelData.buffer, 48);
}

void HAL_TIM_PWM_PulseFinishedHalfCpltCallback(TIM_HandleTypeDef *htim)
{
	transferDataToDma(DMA_HALF_COMPLETE);
}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
	transferDataToDma(DMA_COMPLETE);
}

/*
 * Function responsible for translating 8-bit red, green, blue channels to pwm values for the LEDs to understand
 * For more info about the protocol go to https://datasheet.lcsc.com/szlcsc/2009231607_TCWIN-TC1010RGB-3CSA-TX1812-1010A-1010_C784559.pdf
 * or any neopixel documentation.
 */
void copyLedToBuffer(uint16_t number, uint8_t* buffer)
{
	int8_t i;
	uint8_t currentValue;
	if(number >= NUMBER_OF_PIXELS || buffer == NULL)
		return;
	/* Colors are stored in the 24-bit data buffer in this order: red, green, blue */
	/* Copy red color */
	currentValue = pixel[number].r * pixel[number].brightness / 100;
	for(i=7;i>=0;i--)
	{
		if(currentValue & (1<<i))
			*buffer = 60;
		else
			*buffer = 20;
		buffer++;
	}
	/* Copy green color */
	currentValue = pixel[number].g * pixel[number].brightness / 100;
	for(i=7;i>=0;i--)
	{
		if(currentValue & (1<<i))
			*buffer = 60;
		else
			*buffer = 20;
		buffer++;
	}
	/* Copy blue color */
	currentValue = pixel[number].b * pixel[number].brightness / 100;
	for(i=7;i>=0;i--)
	{
		if(currentValue & (1<<i))
			*buffer = 60;
		else
			*buffer = 20;
		buffer++;
	}
}

/*
 * State machine for deciding what kind of data to put in the DMA buffer
 */
void transferDataToDma(DMA_INT_STATUS intStatus)
{
	/* Sanity check, intStatus should always be alternating between DMA_COMPLETE and DMA_HALF_COMPLETE */
	if(intStatus == pixelData.dmaIntStatus)
		return;
	pixelData.dmaIntStatus = intStatus;
	/* If led data needs to be transferred */
	if(pixelData.dataType == LED_DATA)
	{
		if(intStatus == DMA_COMPLETE)
		{
			copyLedToBuffer(pixelData.currentOutputLed, &pixelData.buffer[24]);
		}
		if(intStatus == DMA_HALF_COMPLETE)
		{
			copyLedToBuffer(pixelData.currentOutputLed, &pixelData.buffer[0]);
		}
		pixelData.currentOutputLed++;
		/* Reached the last LED, next time sent the reset code for the LEDs to display the transmitted color */
		if(pixelData.currentOutputLed == NUMBER_OF_PIXELS)
		{
			pixelData.dataType = RESET_DATA;
			pixelData.resetDataCounter = 0;
		}
	}
	/* Reset sequence needs to be sent */
	else
	{
		/* First 2 calls need to fill the DMA data buffer with 0 (reset PWM value) */
		if(pixelData.resetDataCounter < 2)
		{
			uint8_t i;
			if(intStatus == DMA_HALF_COMPLETE)
			{
				for(i=0;i<24;i++)
					pixelData.buffer[i] = 0;
			}
			else
			{
				for(i=0;i<24;i++)
					pixelData.buffer[24 + i] = 0;
			}
		}
		pixelData.resetDataCounter++;
		/* Reset code transmission complete, start sending pixel colors again */
		if(pixelData.resetDataCounter >= PIXEL_RESET_COUNTER_MAX)
		{
			pixelData.currentOutputLed = 0;
			pixelData.dataType = LED_DATA;
		}
	}
}

