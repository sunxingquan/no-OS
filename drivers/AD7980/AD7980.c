/***************************************************************************//**
 *   @file   AD7980.c
 *   @brief  Implementation of 7980 Driver.
 *   @author Bancisor Mihai(Bancisor.Mihai@analog.com)
********************************************************************************
 * Copyright 2012(c) Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  - Neither the name of Analog Devices, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *  - The use of this software may or may not infringe the patent rights
 *    of one or more patent holders.  This license does not release you
 *    from the requirement that you obtain separate licenses from these
 *    patent holders to use this software.
 *  - Use of the software either in source or binary form, must be run
 *    on or directly connected to an Analog Devices Inc. component.
 *
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
********************************************************************************
 *   SVN Revision: $WCREV$
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <stdint.h>
#include <stdlib.h>
#include "platform_drivers.h"
#include "AD7980.h"           // AD7980 definitions.

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/***************************************************************************//**
 * @brief Initializes the communication peripheral.
 *
 * @param device     - The device structure.
 * @param init_param - The structure that contains the device initial
 * 		       parameters.
 *
 * @return status - Initialization status.
 *                  Example: -1 - Initialization failed;
 *                            0 - Initialization succeeded.
*******************************************************************************/
char AD7980_Init(ad7980_dev **device,
		 ad7980_init_param init_param)
{
	ad7980_dev *dev;
	unsigned char status;

	dev = (ad7980_dev *)malloc(sizeof(*dev));
	if (!dev)
		return -1;

	/* SPI */
	status = spi_init(&dev->spi_desc, init_param.spi_init);
	/* GPIO */
	status |= gpio_get(&dev->gpio_cs, init_param.gpio_cs);

	if (dev->gpio_cs)
		status |= gpio_direction_output(dev->gpio_cs,
						GPIO_HIGH);

	*device = dev;

    return status;
}

/***************************************************************************//**
 * @brief Free the resources allocated by AD7980_Init().
 *
 * @param dev - The device structure.
 *
 * @return SUCCESS in case of success, negative error code otherwise.
*******************************************************************************/
int32_t ad7980_remove(ad7980_dev *dev)
{
	int32_t ret;

	ret = spi_remove(dev->spi_desc);

	ret |= gpio_remove(dev->gpio_cs);

	free(dev);

	return ret;
}

/***************************************************************************//**
 * @brief Initiates conversion and reads data.
 *
 * @param dev           - The device structure.
 *
 * @return receivedData - Data read from the ADC.
*******************************************************************************/
unsigned short AD7980_Conversion(ad7980_dev *dev)
{
	unsigned short receivedData = 0;
	unsigned char  txData[1] = {0};
	unsigned char  rxData[2] = {0, 0};

    txData[0] = 0x7F;
	spi_write_and_read(dev->spi_desc,
			   txData,
			   1);
	AD7980_CS_LOW;
    rxData[0] = 0xFF;
    rxData[1] = 0xFF;
	spi_write_and_read(dev->spi_desc,
			   rxData,
			   2);
	AD7980_CS_HIGH;
	receivedData = (rxData[0] << 8) + rxData[1];

	return(receivedData);
}

/***************************************************************************//**
 * @brief Converts a 16-bit raw sample to volts.
 *
 * @param rawSample - 16-bit data sample.
 * @param vRef      - The value of the voltage reference used by the device.
 *
 * @return voltage  - The result of the conversion expressed as volts.
*******************************************************************************/
float AD7980_ConvertToVolts(unsigned short rawSample, float vRef)
{
    float voltage = 0;

    voltage = vRef * (float)rawSample / (1ul << 16);

    return voltage;
}
