/*
 Demonstration sketch for DS18B20 temperature sensors using the DallasTemperature library.
 
 Reads temperature (Celsius) and temperature (Fahrenheit). Displays results to I2C LCD.
*/

#include <Wire.h>                 // I2C
#include <OneWire.h>              // One-wire devices
#include <DallasTemperature.h>    // DS18B20

#define    DS18B20_BUS     13     // Dallas DS18B20 temperature sensors

const char degree = 223;          // Degree symbol

/*-----------------------------------------------------------------------------------------------
 * Create object references
 * ----------------------------------------------------------------------------------------------*/


// Setup One-wire instance to communicate with DS18B20 sensors
OneWire  oneWire(DS18B20_BUS);

// Pass oneWire reference to DallasTemperature library
DallasTemperature  temperatureSensors(&oneWire);

// Create an array for DS18B20 sensor addresses
DeviceAddress  DS18B20[3]; 

/*-----------------------------------------------------------------------------------------------
 * Function: INIT_DS18B20
 * Description: Initialize DS18B20 Temperature sensor(s)
 * Ins: Integer value for sensor precision.  Can be 9, 10, 11 or 12 bits
 * Outs: none
 * ----------------------------------------------------------------------------------------------*/
 
void INIT_DS18B20(int precision)
{
  temperatureSensors.begin();

  int available = temperatureSensors.getDeviceCount();

  for(int x = 0; x!= available; x++)
  {
    if(temperatureSensors.getAddress(DS18B20[x], x))
    {
      temperatureSensors.setResolution(DS18B20[x], precision);
    }
  }
}

/*-----------------------------------------------------------------------------------------------
 * Function: DS18B20_CELSIUS
 * Description: Get celsius reading from DS18B20 Temperature sensor
 * Ins: Integer value for sensor address
 * Outs: Returns celsius reading
 * ----------------------------------------------------------------------------------------------*/
 
int DS18B20_CELSIUS(int address)
{
  if (temperatureSensors.getAddress(DS18B20[address], address))
  {
    temperatureSensors.requestTemperatures();
    return temperatureSensors.getTempC(DS18B20[address]);
  }
  else
    return 0;
}

/*-----------------------------------------------------------------------------------------------
 * Function: DS18B20_FAHRENHEIT
 * Description: Get fahrenheit reading from DS18B20 Temperature sensor
 * Ins: Integer value for sensor address
 * Outs: Returns fahrenheit reading
 * ----------------------------------------------------------------------------------------------*/
 
int DS18B20_FAHRENHEIT(int address)
{
  if (temperatureSensors.getAddress(DS18B20[address], address))
  {
    temperatureSensors.requestTemperatures();
    return DallasTemperature::toFahrenheit(temperatureSensors.getTempC(DS18B20[address]));
  }
  else
    return 0;
}

/*-----------------------------------------------------------------------------------------------
 * Main routines
 * ----------------------------------------------------------------------------------------------*/

void setup()
{
  // Initialize DS18B20 temperature sensors with precision set to 9
  INIT_DS18B20(9);
  Serial.begin(9600);
}

void loop()
{
  float temp_c;

  // Read values from the sensor at address 0
  for (int i=0; i<4; i++) {
    Serial.print(DS18B20_CELSIUS(0));
    Serial.print(" ");
  }
  Serial.println();
  

}