
/*
D0   = 16
D1   = 5
D2   = 4
D3   = 0
D4   = 2
D5   = 14
D6   = 12
D7   = 13
D8   = 15
D9   = 3
D10  = 1

Heater = D3 = 0
AC = D2 = 4
*/

//Defining Global Variables
int ac = 4;
int heater = 0;
int timer;

//Onewire temperature Processing Initialization
#include <OneWire.h> // Library for waterproof temp sensors
#include <DallasTemperature.h> // Not entirely sure what this is for, beleive it processes the onwire signal
#include <ESP8266WiFi.h> // Library for ESP wifi login
#define ONE_WIRE_BUS 5 //sets waterproof temp data wire, in this case on D1
#define DHTTYPE DHT11
OneWire oneWire(ONE_WIRE_BUS); 
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

//DHT Initialization Stuff
#include "DHT.h" // Library for humidity/temp sensor
const int DHTPin = 15; //sets DHT sensor data wire
DHT dht(DHTPin, DHTTYPE);
// Temporary variables
static char celsiusTemp[7];
static char fahrenheitTemp[7];
static char humidityTemp[7];

//Cayenne Initialization Stuff
#define CAYENNE_PRINT Serial
#include <CayenneMQTTESP8266.h>
// WiFi network info.
char ssid[] = "Smart Home";
char wifiPassword[] = "safehouse";
// Cayenne authentication info. This should be obtained from the Cayenne Dashboard.
char username[] = "6057abe0-a24d-11e8-9127-e3914d9843e1";
char password[] = "ef738c1dee864cb2260bd449c14c4706e5e5bbec";
char clientID[] = "5c8fecf0-a4a7-11e8-83a8-5719c4d6e74d";
unsigned long lastMillis = 0;



void setup() {
  delay(5000); //Allow time to open the Serial Monitor to debug
	Serial.begin(9600);
  Cayenne.begin(username, password, clientID, ssid, wifiPassword);
  pinMode(ac, OUTPUT);
  pinMode(heater, OUTPUT);
  timer = 0;
  // Low means activated, as the relay is triggered with a ground. 
  delay(10);
  dht.begin();

  //Initialize the relay to be off, as it will take 30 seconds to reassess if it should be off
  digitalWrite(ac, HIGH);
  digitalWrite(heater, HIGH);
}

void loop() {
  Cayenne.loop();
     
  sensors.requestTemperatures(); // Send the command to get temperature readings 
  float temp1 = sensors.getTempCByIndex(0);
  float temp2 = sensors.getTempCByIndex(1);
  float temp3 = sensors.getTempCByIndex(2);
  float temp4 = sensors.getTempCByIndex(3);
  float avg_wtr_temp = ((temp1 + temp2 + temp3 + temp4)/4);
  float maximum = maxtemp(temp1, temp2, temp3, temp4);
  float minimum = mintemp(temp1, temp2, temp3, temp4);

  if (maximum >= 45)
    {digitalWrite(ac, LOW);
     digitalWrite(heater, HIGH);
     timer = millis();
     }
     //This chunk of code sets a maximum temp for any one sensor. If intense hotspots occur, this will cool it
     // Look into user defined functions, it would be cool to make a max/min temp function

  else if (minimum <= 2)
    {digitalWrite(ac, HIGH);
     digitalWrite(heater, LOW);
     timer = millis();}
     //Sets a minimum temp for any sensor. Heats the box if it occurs.
  
  else if ((avg_wtr_temp) >= 28)
    {digitalWrite(ac, LOW);
     digitalWrite(heater, HIGH);
     timer = millis();
      }
     //Heats if the average temp is above 28
    
  else if ((avg_wtr_temp) <= 10)
    {digitalWrite(ac, HIGH);
     digitalWrite(heater, LOW);
     timer = millis();}
     //cools if the average temp is below 10
   
  if (((millis() - timer) > 30000) || ((millis() - timer) < 0)) // This won't activate until 30 seconds after the last case was completed
  // the less than 0 case is in case millis rolls over while the timer is counting. This case will be selected and the timer will be reset
    {digitalWrite(ac, HIGH);
     digitalWrite(heater, HIGH);
     timer = 0;} //Reset the timer, this isn't actually necessary lol
   //default where neither the ac nor heating is on

}

//Define the function maxtemp() which returns the max temp of 4 inputs
float maxtemp(float temp1, float temp2, float temp3, float temp4){
  float result;
  if ((temp1 >= temp2) && (temp1 >= temp3) && (temp1 >= temp4))
    {result = temp1;}
  else if ((temp2 > temp1) && (temp2 > temp3) && (temp2 > temp4))
    {result = temp2;}
  else if ((temp3 > temp1) && (temp3 > temp2) && (temp3 > temp4))
    {result = temp3;}
  else if ((temp4 > temp1) && (temp4 > temp2) && (temp4 > temp3))
    {result = temp4;}
  return result;
}

float mintemp(float temp1, float temp2, float temp3, float temp4){
  float result;
  if ((temp1 <= temp2) && (temp1 <= temp3) && (temp1 <= temp4))
    {result = temp1;}
  else if ((temp2 <= temp1) && (temp2 <= temp3) && (temp2 <= temp4))
    {result = temp2;}
  else if ((temp3 <= temp1) && (temp3 <= temp2) && (temp3 <= temp4))
    {result = temp3;}
  else if ((temp4 <= temp1) && (temp4 <= temp2) && (temp4 <= temp3))
    {result = temp4;}
  return result;
}

// Default function for sending sensor data at intervals to Cayenne.
// You can also use functions for specific channels, e.g CAYENNE_OUT(1) for sending channel 1 data.
CAYENNE_OUT_DEFAULT()
{
  float temp1 = sensors.getTempCByIndex(0);
  float temp2 = sensors.getTempCByIndex(1);
  float temp3 = sensors.getTempCByIndex(2);
  float temp4 = sensors.getTempCByIndex(3);
  float avg_wtr_temp1 = ((temp1 + temp2 + temp3 + temp4)/4);
  float maximum = maxtemp(temp1, temp2, temp3, temp4);
  float minimum = mintemp(temp1, temp2, temp3, temp4);
  // Write data to Cayenne here. This example just sends the current uptime in milliseconds on virtual channel 0.
  Cayenne.celsiusWrite(0, temp1);
  Cayenne.virtualWrite(1, dht.readTemperature());
  Cayenne.virtualWrite(2, dht.readHumidity());
  Cayenne.celsiusWrite(3, temp2);
  Cayenne.celsiusWrite(4, temp3);
  Cayenne.celsiusWrite(5, temp4);
  Cayenne.celsiusWrite(6, avg_wtr_temp1);
  Cayenne.virtualWrite(7, digitalRead(4), "Air Conditioner", "0 = on");
  Cayenne.virtualWrite(8, digitalRead(0), "Heater", "0 = on");
  Cayenne.celsiusWrite(9, minimum);
  Cayenne.celsiusWrite(10, maximum);
  //do another for time the heater/ac has been on
  
  // Some examples of other functions you can use to send data.
  //Cayenne.luxWrite(2, 700);
  //Cayenne.virtualWrite(3, 50, TYPE_PROXIMITY, UNIT_CENTIMETER);
}

// Default function for processing actuator commands from the Cayenne Dashboard.
// You can also use functions for specific channels, e.g CAYENNE_IN(1) for channel 1 commands.
CAYENNE_IN_DEFAULT()
{
  CAYENNE_LOG("Channel %u, value %s", request.channel, getValue.asString());
  //Process message here. If there is an error set an error message using getValue.setError(), e.g getValue.setError("Error message");
}






