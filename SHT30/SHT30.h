/*
 * SHT30.h
 *
 *  Created on: Dec 18, 2020
 *      Author: Andrei
 *
 *  SHT30 I2C temperature and humidity sensor driver
 *
 *  Prerequisites:
 *  - forward declaration of the I2C peripheral instance in main.h
 *  - modify the Setup defines below for your setup
 *
 *  Usage:
 *  	After the HAL I2C driver is initialized call initAirSensor()
 *  	To get the values for the sensor you can use getTemperature() and getHumidity() - both return float values
 *
 *  For more information about the implementation of the library please consult the datasheet of the IC:
 *  https://datasheet.lcsc.com/szlcsc/1809281116_Sensirion-SHT30-ARP-B_C86346.pdf
 */

#ifndef INC_SHT30_H_
#define INC_SHT30_H_

/* Setup defines: change the values to whatever your setup is */
#define SHT30_PERIPHERIAL 			hi2c3			/* i2c HAL peripheral */
#define SHT30_ADDRESS 				(0x44 << 1)		/* address of the sensor shifted left by 1 */
/* END Setup defines */

#define SHT30_READ_COMMAND 			0x2C06
#define SHT30_READ_COMMAND_SIZE 	2
#define SHT30_READ_DATA_SIZE 		6
#define convertToC(x) 				(-45.0 + 175.0 * (float)(x) / 65535.0)
#define convertToRH(x) 				(100.0 * (float)(x) / 65535.0)
#define SHT30_ERROR 				-5000

#include "main.h"

typedef struct
{
	I2C_HandleTypeDef* i2cDev;
	uint16_t address;
	float temp;
	float rh;
}SHT30_Sensor;

SHT30_Sensor airSensor;

ErrorStatus initAirSensor();
ErrorStatus getAirData();
float getTemperature();
float getHumidity();

#endif /* INC_SHT30_H_ */
