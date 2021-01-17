/*
 * SHT30.c
 *
 *  Created on: Dec 18, 2020
 *      Author: Andrei
 */

#include "SHT30.h"

SHT30_Sensor airSensor;

/*
 * Initialization of the sensor structure
 */
ErrorStatus initAirSensor()
{
	airSensor.address = SHT30_ADDRESS;
	airSensor.i2cDev = &SHT30_PERIPHERIAL;
	airSensor.temp = 0;
	airSensor.rh = 0;
	return SUCCESS;
}

/*
 * Gets the data from the sensor and puts the values in the sensor structure
 *
 * @return	SUCCESS if the data acquisition was successful
 * 			ERROR if something went wrong
 */
ErrorStatus getAirData()
{
	uint8_t sensorData[6] = {0};
	uint16_t readCommand = SHT30_READ_COMMAND;
	if(HAL_I2C_IsDeviceReady(airSensor.i2cDev, airSensor.address, 3, 100) != HAL_OK)
		return ERROR;
	if(HAL_I2C_Mem_Read(airSensor.i2cDev,
			airSensor.address,
			readCommand,
			SHT30_READ_COMMAND_SIZE,
			sensorData,
			SHT30_READ_DATA_SIZE,
			100) != HAL_OK)
		return ERROR;
	airSensor.temp  = convertToC((uint16_t)(sensorData[0] << 8) + sensorData[1]);
	airSensor.rh = convertToRH((uint16_t)(sensorData[3] << 8) + sensorData[4]);
	return SUCCESS;
}

/*
 * Get the sensor temperature in degrees C
 *
 * @return	temperature of the sensor in degrees C
 * 			SHT30_ERROR in case of an error
 */
float getTemperature()
{
	if (getAirData() == ERROR)
		return SHT30_ERROR;
	return airSensor.temp;
}

/*
 * Get the percent value of the relative humidity measured by the sensor
 *
 * @return	RH measured by the sensor in percent
 * 			SHT30_ERROR in case of an error
 */
float getHumidity()
{
	if(getAirData() == ERROR)
		return SHT30_ERROR;
	return airSensor.rh;
}

