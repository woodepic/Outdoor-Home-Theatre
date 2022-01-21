#include <OneWire.h> // Library for waterproof temp sensors
#include <DallasTemperature.h> //interperets the oneWire signal
#define ONE_WIRE_BUS 13 //sets waterproof temp data wire, in this case on D13
//#define DHTTYPE DHT11
#define ac 12
#define heater 10
#define fans 8
#define relay4 6



//forward declaring Globals
long timer;
long timer2;

//DHT Initialization Stuff
//#include "DHT.h" // Library for humidity/temp sensor
//const int DHTPin = 15; //sets DHT sensor data wire
//DHT dht(DHTPin, DHTTYPE);
// Temporary variables
//static char celsiusTemp[7];
//static char fahrenheitTemp[7];
//static char humidityTemp[7];

//Onewire temperature Processing Initialization
OneWire oneWire(ONE_WIRE_BUS); 
// create Onewire object
DallasTemperature sensors(&oneWire);

void setup() {
  
  delay(5000); //Allow time to open the Serial Monitor to debug
  
	Serial.begin(9600);
  pinMode(ac, OUTPUT);
  pinMode(heater, OUTPUT);
  pinMode(fans, OUTPUT);
  pinMode(relay4, OUTPUT);
  timer = 0;
  timer2 = 0;
  // Low means activated, as the relay is triggered with a ground. 
  delay(10);
  //dht.begin();

  //Initialize the relay to be off, as it will take 30 seconds to reassess if it should be off
  digitalWrite(ac, HIGH);
  digitalWrite(heater, HIGH);
  digitalWrite(fans, HIGH);
  digitalWrite(relay4, HIGH);
}

void loop() {
  sensors.requestTemperatures(); // Send the command to get temperature readings 
  float temp1 = sensors.getTempCByIndex(0);
  float temp2 = sensors.getTempCByIndex(1);
  float temp3 = sensors.getTempCByIndex(2);
  float temp4 = sensors.getTempCByIndex(3);
  float avg_temp = ((temp1 + temp2 + temp3 + temp4)/4);
  float maximum = maxtemp(temp1, temp2, temp3, temp4);
  float minimum = mintemp(temp1, temp2, temp3, temp4);

  //Print all temps
  Serial.print(temp1); Serial.print(" "); Serial.print(temp2); Serial.print(" "); Serial.print(temp3); Serial.print(" "); Serial.println(temp4);

//  Serial.println(avg_temp);
//  Serial.println((millis() - timer));
//  Serial.println((millis() - timer2));
//  Serial.println(" ");
  

  if (maximum >= 45)
    {digitalWrite(ac, LOW); //turn on ac
     digitalWrite(heater, HIGH); //turn off heater
     timer = millis();
     }
     //This chunk of code sets a maximum temp for any one sensor. If intense hotspots occur, this will cool it

  if (minimum <= 34)
    {digitalWrite(ac, HIGH);
     digitalWrite(heater, LOW);
     timer = millis();}
     //Heats box if min temp is too low
  
  if ((avg_temp) >= 32)
    {digitalWrite(ac, LOW);
     digitalWrite(heater, HIGH);
     (timer = millis());
      }
     //Heats if the average temp is above 28
    
  if ((avg_temp) <= 10)
    {digitalWrite(ac, HIGH);
     digitalWrite(heater, LOW);
     timer = millis();}
     //cools if the average temp is below 10

  if ((avg_temp) >= 26)
    {digitalWrite(fans, LOW);
     timer2 = millis();
      }

  if ((avg_temp < 26) && (((millis() - timer2) > 30000) || ((millis() - timer2) < 0)))
  // This won't activate until 30 seconds after the last case was completed, and only if temps are good
  // the less than 0 case is in case millis rolls over while the timer is counting. This case will be selected and the timer will be reset
  //default where neither the ac nor heating is on
    {digitalWrite(fans, HIGH);
     timer2 = 0;} //Reset the timer


  if (((avg_temp > 10) && (avg_temp < 32) && (maximum < 45) && (minimum > 2)) && (((millis() - timer) > 30000) || ((millis() - timer) < 0)))
  // This won't activate until 30 seconds after the last case was completed, and only if temps are good
  // the less than 0 case is in case millis rolls over while the timer is counting. This case will be selected and the timer will be reset
  //default where neither the ac nor heating is on
    {digitalWrite(ac, HIGH);
     digitalWrite(heater, HIGH);
     timer = 0;} //Reset the timer
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
