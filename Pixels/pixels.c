/*
 * pixels.c
 *
 *  Created on: Dec 26, 2020
 *      Author: Andrei
 */

#include "pixels.h"

Pixel pixel[NUMBER_OF_PIXELS];
PixelData pixelData;

/*
 * Initialization function for the pixels
 * - Sets up all the pixel values to white and half brightness
 * - Starts the timer in PWM DMA mode for data transfer
 */
void initPixels()
{
	uint16_t i;
	for(i=0;i<NUMBER_OF_PIXELS;i++)
	{
		setPixelColor(i, 255, 255, 255);
		setPixelBrightness(i, MAX_BRIGHTNESS / 2);
	}
	initDmaTransfer();
}

/*
 * Set up a pixel
 *
 * @params	number: pixel number from 0 to NUMBER_OF_PIXELS-1
 * 			p: Pixel type value to be set
 *
 * @return	nothing
 */
void setPixel(uint16_t number, Pixel p)
{
	if(number < NUMBER_OF_PIXELS)
		pixel[number] = p;
}

/*
 * Get a pixel
 *
 * @params 	number: pixel number from 0 to NUMBER_OF_PIXELS-1
 *
 * @return	Pixel type value
 */
Pixel getPixel(uint16_t number)
{
	Pixel error;
	error.brightness = 0;
	if(number >= NUMBER_OF_PIXELS)
		return error;
	return pixel[number];
}

/*
 * Set the brightness of a specified pixel
 *
 * @params	number: pixel number from 0 to NUMBER_OF_PIXELS-1
 * 			brightness: the brightness to set the pixel to from 0 to MAX_BRIGHTNESS
 *
 * @return	nothing
 */
void setPixelBrightness(uint16_t number, uint8_t brightness)
{
	if(number >= NUMBER_OF_PIXELS || brightness > MAX_BRIGHTNESS)
		return;
	pixel[number].brightness = brightness;
}

/*
 * Set a pixel to a specific color in RGB
 *
 * @params	number: pixel number from 0 to NUMBER_OF_PIXELS-1
 * 			red: red value from 0-255
 * 			green: green value from 0-255
 * 			blue: blue value from 0-255
 *
 * @return 	nothing
 */
void setPixelColor(uint16_t number, uint8_t red, uint8_t green, uint8_t blue)
{
	if(number >= NUMBER_OF_PIXELS)
		return;
	Pixel *p = &pixel[number];
	p->r = red;
	p->g = green;
	p->b = blue;
}

/*
 * Below are the functions responsible for transferring the Pixel data to the pixels
 * You can modify the parameters required for data transfer in pixels.h
 * If the pixels light up fine don't mess around with the below functions
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
 *
 * If the color seems messed up change the order of the color in this function :)
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
			*buffer = COUNTER_PERIOD * 3 / 4;
		else
			*buffer = COUNTER_PERIOD * 1 / 4;
		buffer++;
	}
	/* Copy green color */
	currentValue = pixel[number].g * pixel[number].brightness / 100;
	for(i=7;i>=0;i--)
	{
		if(currentValue & (1<<i))
			*buffer = COUNTER_PERIOD * 3 / 4;
		else
			*buffer = COUNTER_PERIOD * 1 / 4;
		buffer++;
	}
	/* Copy blue color */
	currentValue = pixel[number].b * pixel[number].brightness / 100;
	for(i=7;i>=0;i--)
	{
		if(currentValue & (1<<i))
			*buffer = COUNTER_PERIOD * 3 / 4;
		else
			*buffer = COUNTER_PERIOD * 1 / 4;
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

