//----------------------------------------------------
// Name: Matthew Wood
// ID: 1573807
// CMPUT 275, Winter 2020
//
// Final Project
//
// References: How to use dallas Temperature Library: https://gist.github.com/brooksware2000/3628594
//             How to implement PID: https://www.youtube.com/watch?v=JVqJ7uRGwoA
//                                   https://www.youtube.com/watch?v=JEpWlTl95Tw&list=WL&index=268&t=0s
//----------------------------------------------------
//testing3
//Install DallasTeamperature library for the following two libraries
#include <OneWire.h> // Library for waterproof temp sensors
#include <DallasTemperature.h> //interperets the oneWire signal
#include <Servo.h>

//Define parameters
#define numOfTempSensors 4
#define tempMeasurementInterval 1 //The number of seconds between each time we poll the temp sensors

//Define pins
#define ONE_WIRE_BUS 3 //sets waterproof temp data wire, in this case on D13
//#define DHTTYPE DHT11
#define ac 4
#define heater 5
#define fans 6
#define relay4 7
#define servoPin 13

//Tuning for when climate control kicks in
//#define heatIfMin 5
#define heatIfAvg 16
//#define coolIfMax 42
#define coolIfAvg 28
//#define maxCoolIfMax 54
#define maxCoolIfAvg 40

#define tempBuffer 2 //takes into account that temp readings are not always in phase with actual temp

//#define heatTime 45000
//#define coolTime 300000
//#define maxCoolTime 150000
#define waitingTime 30000

//Onewire temperature Processing Initialization
OneWire oneWire(ONE_WIRE_BUS); 
// create Onewire object
DallasTemperature sensors(&oneWire);


class RoboticVent {
  //On the outside of the projector enclosure there is a servo-actuated
  //exhaust vent/door that allows for insulation when closed, and cooling
  //when open. This class will help automate some of the door's functions.

  public:
    void open(int speed) {
      //This function opens the flap door given a set percentage
      //of speed (0-100%)

      //determine corresponding delaytime
      int waitTime = speedToDelay(speed);

      if (LastState != wasOpen) {
      	while (pos < start + duration) {
      		pos++;
      		flapServo.write(pos);
      		delay(waitTime);
      		//Serial.println("opening");
      	}
      }

      if ((LastState != wasOpen) && (pos == start + duration)) {
        LastState = wasOpen;
        Serial.println("Flap opened");
      }

    }

    void closed(int speed) {
      //This function closes the flap door given a set percentage
      //of speed (0-100%)

      //determine corresponding delaytime
      int waitTime = speedToDelay(speed);

      if (LastState != wasClosed) {
      	while (pos > start) {
      		pos--;
      		flapServo.write(pos);
      		delay(waitTime);
      		//Serial.println("closing");
      	}
      }

      if ((LastState != wasClosed) && (pos == start)) {
        LastState = wasClosed;
        Serial.println("Flap closed");
      }
    }

    void begin() {
      flapServo.attach(servoPin); //Initializes the servo object on the specified pin
      LastState = startup;
    }

  private:
    int start = 31; //the starting angle for the servo (closed position)
    int duration = 33; //The num of degrees the servo travels
    Servo flapServo;  // create servo object to control the servo in the vent door class
    int pos = start; 
    int interval;
    enum state{wasOpen, wasClosed, startup} LastState;


    int speedToDelay(int speed) {
      //This program takes in a percentage of speed (0-100%)
      //and returns a value for the delay in each servo movement
      int waitTime = map(speed, 0, 100, 10, 0);
      return waitTime;
    }
};

class ThermalControl {
  public:
      void maintainTemp() {
        //note that it is harmful (and annoying) to have components constantly
        //starting and stopping. To avoid this, once we turn on a device, we tell it
        //to stay on for a set time, regardless of the temp, unless of course this
        //extended period produces dangerous temps

        //Decide the next state based on the current state
        switch (state) {
          case heat:
            //heat until just shy of us needing to cool
            if (avgTemp() > (coolIfAvg - tempBuffer)) {
              //since we were just heating, we will naturally cool down (cuz it's cold
              //outside)
              lastInState = millis();
              lastState = heat;
              state = waiting;
            }

            break;

          case idle:
            //Criteria to heat
            if (avgTemp() < heatIfAvg) {
              state = heat;
            }

            //Criteria to cool
            if (avgTemp() > coolIfAvg) {
              state = cool;
            }

            //Criteria to max cool
            if (avgTemp() > maxCoolIfAvg) {
              state = maxCool;
            }

            //Note that if we do not satisfy any of these criteria we remain
            //on the idle state.

            break;

          case cool:
            //cool until just shy of needing to heat
            if (avgTemp() < (heatIfAvg + tempBuffer)) {
              //since we were just cooling, we will naturally heat up since the
              //projector is running and/or its hot outside
              lastInState = millis();
              lastState = cool;
              state = waiting;
            }

            //if it gets too hot while we're attemping to cool, activate maxcool
            if (avgTemp() > maxCoolIfAvg) {
              state = maxCool;
            }
            break;

          case maxCool: 
            //Cool until just shy of us needing to heat
            if (avgTemp() < (heatIfAvg + (tempBuffer/2))) {
              //since we were just cooling at maximum capacity, we will naturally 
              //heat up very fast. So hence the smaller temp buffer
              lastInState = millis();
              lastState = maxCool;
              state = waiting;
            }
            break;


          case waiting:
            //After cooling/heating has completed, we will stay in this idle state of
            //sorts to allow time for the min/max temp to catch up to the average temp
            //and acts as error correction in case we overshoot
            if (((millis()-lastInState) > waitingTime) || ((millis()-lastInState) < 0)) {
              state = idle;
            }


            //if the temps are hella wrong, address it?

            break;

        }



        //Apply the settings of the current state
        if (state == heat) {
          //turn on heater
          vent.closed(80); //close vent
          digitalWrite(ac, LOW);
          digitalWrite(heater, HIGH);
          digitalWrite(fans, LOW);
        }

        if (state == idle){
          //turn everything off
          digitalWrite(ac, LOW);
          digitalWrite(heater, LOW);
          digitalWrite(fans, LOW);
        }

        if (state == cool) {
          //turn on fans
          vent.open(80); // open vent
          digitalWrite(ac, LOW);
          digitalWrite(heater, LOW);
          digitalWrite(fans, HIGH);
        }

        if (state == maxCool){
          //turn on AC and fans
          vent.open(80); //open vent
          digitalWrite(ac, HIGH);
          digitalWrite(heater, LOW);
          digitalWrite(fans, HIGH);
        }

        if (state == waiting){
          //turn everything off and allow temp to average out throughout the case
          digitalWrite(ac, LOW);
          digitalWrite(heater, LOW);
          digitalWrite(fans, LOW);
        }
      }

      float maxTemp() {
          updateTemps(); //This will ensure that temps are updated every set interval

          float maxTemp = currentTemps[0];
          for (int i=1; i<n; i++) {
              if (currentTemps[i] > maxTemp) {
                maxTemp = currentTemps[i];
              }
          }
          return maxTemp;
      }

      float minTemp() {
          updateTemps(); //This will ensure that temps are updated every set interval

          float minTemp = currentTemps[0];
          for (int i=1; i<n; i++) {
              if (currentTemps[i] < minTemp) {
                minTemp = currentTemps[i];
              }
          }
          return minTemp;
      }

      float avgTemp() {
          updateTemps(); //This will ensure that temps are updated every set interval

          float runningSum = 0;
          for (int i=0; i<n; i++) {
              runningSum += currentTemps[i];
          }
          return runningSum / n;
      }

      void printTempData() {
          updateTemps(); //This will ensure that temps are updated every set interval

          for (int i=0; i<n; i++) {
              if (i == n-1) {
                Serial.println(currentTemps[i]);
              }
              else {
                Serial.print(currentTemps[i]);
                Serial.print(" ");
              }
          }

          Serial.print("Max Temp: ");
          Serial.print(maxTemp());
          Serial.print(", Min Temp: ");
          Serial.println(minTemp());
          Serial.println();
      }

      void printData(int interval) {
        //this function prints the current state and temps at an interval provided
        //as an argument (in seconds)

        if (((millis() - lastPrintData) > (interval*1000)) || ((millis() - lastPrintData) < 0)) {
          //Print the current state
          Serial.print("State: ");
          switch (state) {
            case heat:
              Serial.print("heating");
              break;
            case idle:
              Serial.print("idle");
              break;
            case cool:
              Serial.print("cooling");
              break;
            case maxCool:
              Serial.print("maxCooling");
              break;
            case waiting:
              Serial.print("waiting");
              break;
          }
          Serial.print(", ");

          //Print the current temp
          Serial.print("AvgTemp: ");
          Serial.print(avgTemp());
          Serial.print(", ");

          Serial.print("MaxTemp: ");
          Serial.print(maxTemp());
          Serial.print(", ");

          Serial.print("MinTemp: ");
          Serial.print(minTemp());
          Serial.print(", ");

          Serial.print("    Front top right: ");
          Serial.print(currentTemps[2]);
          Serial.print(",  ");

          Serial.print("Front top left: ");
          Serial.print(currentTemps[0]);
          Serial.print(",  ");

          Serial.print("Rear top: ");
          Serial.print(currentTemps[3]);
          Serial.print(",  ");

          Serial.print("Rear bottom: ");
          Serial.println(currentTemps[1]);

          lastPrintData = millis(); //keep track of when we last printed
        }
      }

      void plotData(int interval) {
        if (((millis() - lastPlotData) > (interval*1000)) || ((millis() - lastPlotData) < 0)) {
          //prints using serial plotter the current temp
          Serial.print("State: ");
          Serial.print(state);
          Serial.print(", ");

          Serial.print("lastState: ");
          Serial.print(lastState);
          Serial.print(", ");

          Serial.print("AvgTemp:");
          Serial.print(avgTemp());
          Serial.print(", ");

          //Print the target temp
          Serial.print("maxTemp: ");
          Serial.print(maxTemp());
          Serial.print(", ");

          //print the current error
          Serial.print("minTemp: ");
          Serial.println(minTemp());

          lastPlotData = millis();
        }
      }

      void getTimeData() {
        //This function is simply for testing the efficiencies of 
        //diffferent methods during development


        //perhaps we only execute this once in a while? that way we dont needlessly slow things down
        long timer1;
        long timer2;
        

        timer1 = millis();
        updateTemps();
        timer1 = millis() - timer1;
        float test;

        timer2 = millis();
        for (int i=0; i<n; i++) {
          test = currentTemps[i];
        }
        timer2 = millis() - timer2;

        Serial.print("Time to execute first command: ");
        Serial.print(timer1);
        Serial.println("ms");

        Serial.print("Time to execute second command: ");
        Serial.print(timer2);
        Serial.println("ms");
      }

      void testServo(int speed) {
        //This function is used for setup to make sure the bounds of the servo
        //flap is correct

        unsigned long timer = millis();

        while ((millis() - timer) < 2000 ) {
          vent.open(speed);
        }
        timer = millis();
        while ((millis() - timer) < 2000 ) {
          vent.closed(speed);
        }
      }

      void begin() {
          //This function is used to initialize parameters needed for the
          //temperature regulation

          vent.begin(); //initialize vent attributes

          //Cycle the vent for calibration:
          for (int i=0; i<3; i++) {
    			  vent.open(80);
    			  delay(500);
    			  vent.closed(80);
    			  delay(500);
          }





          sensors.requestTemperatures();  
          delay(50);
          for (int i=0; i<n; i++) {
            currentTemps[i] = sensors.getTempCByIndex(i);
            //Note that getTempCByIndex() is known to be slow,
            //There may be a faster way of accessing temps.
          }
          state = idle;
      }


  private:
      //inititate vent
      RoboticVent vent;

      //Variables for temp measurement
      int n = numOfTempSensors; //Number of temp probes connected to OneWire Bus
      float currentTemps[numOfTempSensors];
        //Note that currently we have the temp probes in the following locations:
        //currentTemps[0] on top level, left of the projector
        //currentTemps[1] below exhaust fan, on top of harmony
        //currentTemps[2] top right, in front of projector(by air intake)
        //currentTemps[3] top left rear of box, right by apple tv


      unsigned long lastTempUpdate = tempMeasurementInterval * 1000; //the millis() of the last time we updated temps
      unsigned long lastPrintData = 0;
      unsigned long lastPlotData = 0;

      enum heatingModes{heat, idle, cool, maxCool, waiting};
      heatingModes state;
      heatingModes lastState;
      unsigned long lastInState = 0; 
      unsigned long lastIdle;



      void updateTemps(){
          //This function is intened to update the array currentTemps[] with
          //the current value of temperatures. This function is needed
          //as we rarely need to update temps (which takes lots of time
          //about 290ms), yet we often need to quickly access the temps. 

          //if we are within time polling interval
          if (((millis() - lastTempUpdate) >= (tempMeasurementInterval * 1000)) || ((millis() - lastTempUpdate) < 0)) {
              sensors.requestTemperatures();

              //update current temps
              for (int i=0; i<n; i++) {
                currentTemps[i] = sensors.getTempCByIndex(i);
                //Note that getTempCByIndex() is known to be slow,
                //There may be a faster way of accessing temps.
              }

              lastTempUpdate = millis();
          }
      }
};


ThermalControl projector;


void setup() {
  
  //delay(2000); //Allow time to open the Serial Monitor to debug
  
  Serial.begin(9600);
  //delay(2000);

  //Initialize relays to be off (LOW is off)
  pinMode(ac, OUTPUT);
  digitalWrite(ac, LOW);

  pinMode(heater, OUTPUT);
  digitalWrite(heater, LOW);

  pinMode(fans, OUTPUT);
  digitalWrite(fans, LOW);

  pinMode(relay4, OUTPUT);
  digitalWrite(relay4, LOW);


  projector.begin();  // attaches the servo to specified pin
}

void loop() {
  //While it may seem unnecesary to only have a few lines here, temperature
  //is only one job that the arduino is performing. I have multiple other
  //ideas for other tasks, and those could be implemented here as well.

  projector.maintainTemp();
  projector.printData(5);



}
