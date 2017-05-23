#include "stm32f0xx.h"
#include "temp_sensor.h"
#include "FreeRTOS.h"
#include "task.h"
#include "I2C.h"

#define number_of_bytes 			2
#define temp_sensor_ADDRESS         (0x4F)

uint16_t temp = 0;	//received data
uint16_t actual_temperature = 0;


void tempUpdates(void *dummy){


	while(1){

		I2C2_send_message_no_cb(0, temp_sensor_ADDRESS);


		ulTaskNotifyTake( pdTRUE, portMAX_DELAY );
		vTaskDelay(100);

		I2C2_recv_message_with_cb(temp_sensor_ADDRESS, 2, (&temp));
		ulTaskNotifyTake( pdTRUE, portMAX_DELAY );

		//uint16_t thing = 0;
		temp = ((temp & 0xFF00) >> 8) | ((temp & 0x00FF) << 8);

		actual_temperature = (temp>>5)/(8);

		vTaskDelay(100);
	}
}

