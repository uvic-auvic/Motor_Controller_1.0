/*
 * temp_sensor.c
 *
 *  Created on: Mar 19, 2017
 *      Author: Jacob.W.L
 *
 *	Code for the LM75B Digital Temperature Sensor
 *
 *
 */
#include "temp_sensor.h"
#include "Timer.h"
#include "stm32f0xx.h"
#include "stm32f0xx_misc.h"
#include "Timer.h"


int setup()
{
	// Create I2C bus
	int file;
	char *bus = "/dev/i2c-1";
	if((file = open(bus, O_RDWR)) < 0)
	{
		printf("Failed to open the bus. \n");
		exit(1);
	}
	// Get I2C device, LM75B I2C address is 0x49(53)
	ioctl(file, I2C_SLAVE, 0x49);

	// Select configuration register(0x01)
	// Continuous operation, normal operation(0x00)
	char config[2] = {0};
	config[0] = 0x01;
	config[1] = 0x00;
	write(file, config, 2);
	sleep(1);

	// Read 2 bytes of data from register(0x00)
	// temp msb, temp lsb
	char reg[1] = {0x00};
	write(file, reg, 1);
	char data[2] = {0};
	if(read(file, data, 2) != 2)
	{
		printf("Erorr : Input/output Erorr \n");
	}

	else
	{
		// Convert the data to 9-bits
		int temp = (data[0] * 256 + (data[1] & 0x80)) / 128;
		if(temp > 255)
		{
			temp -= 512;
		}
		float cTemp = temp * 0.5;
		float fTemp = cTemp * 1.8 + 32;

		// Output data to screen
		printf("Temperature in Celsisus : %.2f \n", cTemp);
		printf("Temperature in Fahrenheit : %.2f \n", fTemp);
	}
}

double tos = 30.0;                                            // 0.5℃ --> 80℃
double thyst = 28.0;                                          //0.5℃ -->75℃
signed int tos_data = (signed int)(tos / 0.5) << 7;
signed int thyst_data = (signed int)(thyst / 0.5) << 7;
int osPin = 2;                                                //LM75BiOS

void setup()
{
  pinMode(osPin,INPUT_PULLUP);
  attachInterrupt(0,temp_interrupt,CHANGE);
  Wire.begin();
  Serial.begin(9600);
  Wire.beginTransmission(LM75B_address);
  Wire.write(tos_reg);
  Wire.write(tos_data >> 8);
  Wire.write(tos_data);
  Wire.endTransmission();
  Wire.beginTransmission(LM75B_address);
  Wire.write(thyst_reg);
  Wire.write(thyst_data >> 8);                    //thyst
  Wire.write(thyst_data);
  Wire.endTransmission();
  Wire.beginTransmission(LM75B_address);
  Wire.write(temp_reg);
  Wire.endTransmission();

}

void loop()
{
  char str[20] = { 
    0    };
  signed int temp_data = 0;                    //LM75B
  double temp = 0.0;
  Wire.requestFrom(LM75B_address,2);
  while(Wire.available()){
    temp_data |= (Wire.read() << 8);         //8bit
    temp_data |= Wire.read();                //8bit
  }

  temp = (temp_data >> 5) * 0.125;

  Serial.println(temp);
  delay(100);
}

void temp_interrupt()
{
  if(digitalRead(osPin) == 0){
    Serial.print("TEMP : OVER ");
    Serial.print(tos);
    Serial.println(" deg");
  }
  else{
    Serial.print("TEMP : UNDER ");
    Serial.print(thyst);
    Serial.println(" deg");
  }
}
