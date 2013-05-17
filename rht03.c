/** \file rht03.c
 *  \brief Grove devices support library 
 */

/**
\addtogroup Grove devices
@{
*/

/* **************************************************************************																					
 *                                OpenPicus                 www.openpicus.com
 *                                                            italian concept
 * 
 *            openSource wireless Platform for sensors and Internet of Things	
 * **************************************************************************
 *  FileName:        rht03.c
 *  Module:          FlyPort WI-FI/EHT
 *  Compiler:        Microchip C30 v3.12 or higher
 *
 *  Author               Rev.    Date              Comment
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  Davide Vicca    1.0     11/17/2012		   First release  (core team)
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *  Software License Agreement
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  This is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License (version 2) as published by 
 *  the Free Software Foundation AND MODIFIED BY OpenPicus team.
 *  
 *  ***NOTE*** The exception to the GPL is included to allow you to distribute
 *  a combined work that includes OpenPicus code without being obliged to 
 *  provide the source code for proprietary components outside of the OpenPicus
 *  code. 
 *  OpenPicus software is distributed in the hope that it will be useful, but 
 *  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details. 
 * 
 * 
 * Warranty
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT
 * WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 * LIMITATION, ANY WARRANTY OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * WE ARE LIABLE FOR ANY INCIDENTAL, SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF
 * PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR SERVICES, ANY CLAIMS
 * BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE
 * THEREOF), ANY CLAIMS FOR INDEMNITY OR CONTRIBUTION, OR OTHER
 * SIMILAR COSTS, WHETHER ASSERTED ON THE BASIS OF CONTRACT, TORT
 * (INCLUDING NEGLIGENCE), BREACH OF WARRANTY, OR OTHERWISE.
 *
 **************************************************************************/
#include "taskFlyport.h"
#include "grovelib.h"
#include "input_capture.h"


extern unsigned char flag;
struct Interface *attachSensorToDigioBus(void *,int,int);



/**
 * struct RHT03 - The structure for RHT03 Hum&Temp grove sensor 
 */
struct RHT03
{
	const void *RHT03;
	struct Interface *inter;
	BYTE bitone_lenght;
	BYTE ic_module;
};


/**
 * static int checksum(unsigned long data1,unsigned long data2)-Perform the checksum on the outgoing rht03 data
 * \unsignend long data1- first rht03 data
 * \unsignend long data2- second rht03 data
  *\return - the status of operation
 	<LI><B>0 </B> the operation was successful</LI>	
 	<LI><B>-1 </B> the operation was unsuccessful (a timeout event occurred)</LI>	
*/
static int checksum(unsigned long data1,unsigned long data2)
{
	BYTE sum = 0;
	int i = 0;
	for (;i<25;i+=8)
		sum += data1>>i;
	if (sum == data2)
		return 0;
	else
		return -1;
}	
	
/**
 * static void *RHT03_ctor (void * _self, va_list *app) -RHT03 grove device Constructor  
 * \param *_self - pointer to the RHT03 grove device class.
 * \param *app - none
* \return - Pointer to the rht03 devices instantiated
*/
static void *RHT03_ctor (void * _self, va_list *app)
{
	struct RHT03 *self = _self;
	self->inter = NULL;
	return self;
}	


/**
 * static void RHT03_dtor (void * _sensor)- RHT03 grove device Destructor  
 * \param *_sensor - pointer to the RHT03 grove device class.
 * \return - None
*/
static void RHT03_dtor (void * _sensor)
{
	struct RHT03 *sensor = _sensor;
	if(sensor->inter)
	{
		free(sensor->inter->port);
		free(sensor->inter);
	}
}	

/**
 * static void *RHT03_attach (void * _board,void *_sensor,int n) - attach a RHT03 grove device to the GroveNest digital I/O port  
 * \param *_board - pointer to the GroveNest 
 * \param *_sensor - pointer to the RHT03 grove device class.
 * \param n - port number which RHT03 device is connected to
 * \return 
 <UL>
	<LI><Breturn = Pointer to the digital interface created:</B> the operation was successful.</LI> 
	<LI><B>return = NULL:</B> the operation was unsuccessful.</LI> 
 </UL>
 */
static void *RHT03_attach (void * _board,void *_sensor,int n)
{
	struct RHT03 *sensor = _sensor;
	sensor->inter = attachSensorToDigioBus(_board,n,IN);	
	return sensor->inter;

}	


/**
 * static int RHT03_configure (void * _self, va_list *app) -  Configure the RHT03 grove device
 * \param *_self - pointer to the device 
 * \param *app - the number of IC module is being used by the RHT03 device 
 * \return operation is always successful:
 	<LI><Breturn = 0:</B> </LI> 
 </UL>
 */
static int RHT03_configure (void * _self, va_list *app)
{
	struct RHT03 *self = _self;
	self->ic_module = va_arg(*app, const BYTE);
	IOInit_(self->inter->port->Pin1,self->ic_module+50);
	return 0;
}	

/**
 * static float RHT03_get(void * _self,va_list *app) -  read a float value from the rht03 device.
 * \param *_self - pointer to the device 
 * \param *app - HUMD / TEMP  
  <UL>
	<LI><B HUMD </B> Get humidity value.</LI> 
	<LI><B TEMP </B> Get temperature value.</LI> 
 * \return the Temperature or humidity values:
 */
static float RHT03_get(void * _self,va_list *app)
{
	struct RHT03 *self = _self;
	BYTE h_t= va_arg(*app, const BYTE);
	IOInit(self->inter->port->Pin1,IN);
	Delay10us(20);
	unsigned long data1,data2;
	unsigned int timeout = 65500;
	IOInit(self->inter->port->Pin1,OUT);
	IOPut(self->inter->port->Pin1,OFF);
	Delay10us(300);
	vTaskSuspendAll();
	/*****Start the trigger procedure********/
	IOInit(self->inter->port->Pin1,IN);
	while((!IOGet(self->inter->port->Pin1))&&timeout)
		timeout--;
	while((IOGet(self->inter->port->Pin1))&&timeout)
		timeout--;
	while((!IOGet(self->inter->port->Pin1))&&timeout)
		timeout--;
	while((IOGet(self->inter->port->Pin1))&&timeout)
		timeout--;
	int error = 0;
	if(!timeout)
	{
		xTaskResumeAll();
		flag = 1;
		return -1;
	}
	/*******Decode the incoming pulses from RHT03 device**************/
	error = OneWireDecode(self->ic_module,40,32,&data1); 
	if(!error)
		error = OneWireDecode(self->ic_module,40,8,&data2);
	ICOff(self->ic_module);
	xTaskResumeAll();
	if(error)
	{
		flag = -1; 
		return -1;
	}

	//check the data out
	if(checksum(data1,data2))
	{
		flag = 1;
		return -1;
	}
	else
	{
		if(!h_t)//Humidity 
		{
			float h = ((float)(data1>>16))/10.0;
			return h;
		}
		else//temperature
		{
			float temp;
			unsigned int temp_ = data1&0x7FFF;
			if(data1&0x8000)
				temp = -(float)temp_/10.0;//negative temperature value		
			else
				temp = (float)temp_/10.0;		
			return temp;
		}	
	}
}	

static const struct SensorClass _RHT03 =
{	
	sizeof(struct RHT03),
	RHT03_ctor,
	RHT03_dtor,
	RHT03_attach,
	RHT03_configure,
	0,
	RHT03_get,
};

const void *RHT03 = &_RHT03;





