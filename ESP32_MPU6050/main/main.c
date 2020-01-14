
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include "esp_log.h"
#include "driver/i2c.h"

#include "I2Cdev.h"
#include "MPU6050_lite.h"
#include "sdkconfig.h"




void app_main(void)
{
	//example
  MPU6050_initialize();
  MPU6050_calibrate();
     int16_t ax, ay, az;
     int16_t gx, gy, gz;
     while(1){

    	 MPU6050_getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    	 printf ("temp = %.2f  ", (MPU6050_getTemperature()/340.0+36.53));
    	 printf ("x = %d   ", ax);
    	 printf ("y = %d  ", ay);
    	 printf ("z = %d\n\n", az);
    	 vTaskDelay(500/portTICK_PERIOD_MS);
     }
}
