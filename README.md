lib_grove_RHT03
===============

OpenPicus Flyport library for RHT03 humidity and temperature sensor. The library can be used to read both humidity and temperature from the RHT03 sensor.More infos on wiki.openpicus.com.
<br>1) import files inside Flyport IDE using the external libs button.<br>
2) add following code example in FlyportTask.c:

<pre>
#include "taskFlyport.h"
#include "grovelib.h"
#include "rht03.h"
 
void FlyportTask()
{  
	float data = 0;
	char msg[100];
 
	vTaskDelay(50);
	UARTWrite(1,"Welcome to GROVE NEST example!\r\n");

	//	Board and sensor objects instances
	void *board = new(GroveNest);
	void *rht03 = new(RHT03);
 
	//	Sensor is attached to board and configured
    attachToBoard(board, rht03, DIG1);
    configure(rht03, 3);
	
	while(1)
	{
		//	Humidity reading
		data = get(rht03,HUMD);
		if(!readError())
		{
			sprintf(msg,"humidity %.1f\r\n",(double)data);
			UARTWrite(1,msg);
		}
		vTaskDelay(100);
		//	Temperature reading
		data = get(rht03,TEMP);
		if(!readError())
		{
			sprintf(msg,"temperature %.1f\r\n",(double)data);
			UARTWrite(1,msg);
		}
	}	
}

</pre>
