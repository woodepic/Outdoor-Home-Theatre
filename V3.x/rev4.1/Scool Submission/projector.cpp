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
//testing2
#include <OneWire.h> // Library for waterproof temp sensors
#include <DallasTemperature.h> //interperets the oneWire signal
#include <Servo.h>

//Define parameters
#define numOfTempSensors 4
#define tempMeasurementInterval 1 //The number of seconds between each time we poll the temp sensors
#define acTime 180000
#define heaterTime 30000
#define fansTime 700000

//Define pins
#define ONE_WIRE_BUS 13 //sets waterproof temp data wire, in this case on D13
//#define DHTTYPE DHT11
#define ac 12
#define heater 10
#define fans 8
#define relay4 6
#define servoPin 4


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
      		Serial.println("opening");
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
      		Serial.println("closing");
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
      void setTemp(float inputedTemp) {
        //sets the desired temperature in degrees C
        targetTemp = inputedTemp;
      }

      void setTolerance(float inputedTolerance) {
        //sets the allowed deviation from the desired temperature in degrees C.
        //Too low of a tolerance will lead to racing in the system, and frequent
        //devices starting up & shutting down.

        tolerance = inputedTolerance;
      }

      void processPID() {


        error = targetTemp - avgTemp();

        //Proportional componenet
        output = pGain * error;

        //integral component
        float iComp = iGain * sumOfPastError;

        //Limit exteme integrator values
        if (iComp > 20) {
          //limit the maximum output to avoid temp overshooting
          iComp = 20;
        }

        else if (iComp < -20) {
          //limnit the minimum output to avoid temp undershooting
          iComp = -20;
        }
        output += iComp;

        //derivative component
        output += dGain * lastError;




        //Since we can't adjust the level of the fans/ac/heater, and we can only
        //specify on/off, we need to set targets for each output level and what
        //to do when the output hits that level

        //Also notethat it is harmful (and annoying) to have components constantly
        //starting and stopping. To avoid this, once we turn on a device, we tell it
        //to stay on for a set time, regardless of the temp

        //Determine what the next case will be
        //Note that we are "officially in" the state if State == LastState (this is used to keep track
        //of times and the last mode we were in)
        switch (State) {
          case startup:

            //goto heating
            if (output > (3*tolerance)) {
              //we need to heat
              State = heating;
            }

            //goto coolLow
            else if ((output < (-2 * tolerance)) && (output > (-4 * tolerance))) {
              //we need to turn on fans
              State = coolLow;
            }

            //goto coolHigh
            else if (output <= (-4 * tolerance)) {
              //This is max cooling mode. We need to turn on both fans and AC
              State = coolHigh;
            }

            //goto idle
            else {
              //this is when we are happy with the temperatures as they are.
              State = idle;
            }

            LastState = startup;
            stateChangeTime = millis();
            break;

          case heating:
            switch (LastState) {
                case startup:
                //immediately jump into this case
                LastState = heating;
                break;
              case idle:
                //immediately jump into this case
                LastState = heating;
                break;
              case heating:
                //determine next state
                //stay on heating
                if (output > (3*tolerance)) {
                  //we need to heat
                  State = heating;
                }
                //goto idle
                else {
                  //this is when we are happy with the temperatures as they are.
                  State = idle;
                }

                LastState = heating;
                stateChangeTime = millis();
                break;
            }
            break;

          case idle:
            switch (LastState) {
              case startup:
                //immediately confirm into this case
                LastState = idle;
                break;
              case heating:
                //wait for time defined in heaterTime before officially entering idle
                if (((millis()-stateChangeTime) > heaterTime) || ((millis()-stateChangeTime) < 0)) {
                  LastState = idle;
                }
                break;
              case coolLow:
                if (((millis()-stateChangeTime) > fansTime) || ((millis()-stateChangeTime) < 0)) {
                	if (output > (-2 * tolerance)) {
                		//after waiting for the fan to turn off, only stay on idle if the temp is right
                		LastState = idle;
                	}
                }

                //If during the time we are waiting for the fan to turn off it becomes too hot, this will give more cooling
                if (output < (-2 * tolerance)) {
                	State = coolLow;
                }
                break;
              case idle:
                //decide what happens next
                if (output > (3* tolerance)) {
                  State = heating;
                }
                else if (output < (-2 * tolerance)) {
                  State = coolLow;
                }
                else {
                  State = idle;
                }

                LastState = idle;
                stateChangeTime = millis();
                break;
            }
            break;

          case coolLow:
            switch (LastState) {
              case startup:
                //instantly jump into this case officially
                LastState = coolLow;
                break;
              case idle:
                //instantly jump into this case officially
                LastState = coolLow;
                break;
              case coolHigh:
                //wait for time defined in acTime before officially being in this state
                if (((millis() - stateChangeTime) > acTime) || ((millis()-stateChangeTime) < 0)) {
                  LastState = coolLow;
                }
                break;

              case coolLow:
                //asess what the next state should be

                //goto idle
                if (output > (-2 * tolerance)){
                  State = idle;
                }

                //stay on coolLow
                if ((output < (-2 * tolerance)) && (output > (-4 * tolerance))) {
                  State = coolLow;
                }

                //goto coolHigh
                if (output <= (-4 * tolerance)) {
                  State = coolHigh;
                }

                LastState = coolLow;
                stateChangeTime = millis();
                break;       
            }
            break;

          case coolHigh:
            switch (LastState) {
              case startup:
                //instantly jump into this case officially
                LastState = coolHigh;
                break;
              case coolLow:
                //instantly jump into this case officially
                LastState = coolHigh;
                break;
              case coolHigh:
                //assess what the next state should be
                if (output <= (-4 * tolerance)) {
                  //This is max cooling mode. We need to turn on both fans and AC
                  State = coolHigh;
                }

                else{
                  State = coolLow;
                }

                LastState = coolHigh;
                stateChangeTime = millis();
                break;
            }
            break;
        }



        //if the state and the last states match, then activate the relays accordingly
        if ((State == heating) && (LastState == heating)) {
          //turn on heater
          vent.closed(80); //close vent
          digitalWrite(ac, HIGH);
          digitalWrite(heater, LOW);
          digitalWrite(fans, HIGH);
        }

        if ((State == idle) && LastState == idle) {
          //turn everything off
          digitalWrite(ac, HIGH);
          digitalWrite(heater, HIGH);
          digitalWrite(fans, HIGH);
        }

        if ((State == coolLow) && (LastState == coolLow)) {
          //turn on fans
          vent.open(80); // open vent
          digitalWrite(ac, HIGH);
          digitalWrite(heater, HIGH);
          digitalWrite(fans, LOW);
        }

        if ((State == coolHigh) && (LastState == coolHigh)) {
          //turn on AC and fans
          vent.open(80); //open vent
          digitalWrite(ac, LOW);
          digitalWrite(heater, HIGH);
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

      void printStateData() {
        Serial.print("State: ");
        Serial.print(State);
        Serial.print(", ");

        Serial.print("LastState: ");
        Serial.print(LastState);
        Serial.print(", ");

        Serial.print("Target: ");
        Serial.print(targetTemp);
        Serial.print(", ");

        Serial.print("Temp: ");
        Serial.print(avgTemp());
        Serial.print(", ");

        Serial.print("Output: ");
        Serial.println(output);
      }

      void plotTempData() {

        //prints using serial plotter the current temp
        Serial.print("AvgTemp:");
        Serial.print(avgTemp());
        Serial.print(", ");

        //Print the target temp
        Serial.print("Target:");
        Serial.print(targetTemp);
        Serial.print(", ");

        //print the current error
        Serial.print("Error:");
        Serial.print(error);
        Serial.print(", ");

        Serial.print("Output:");
        Serial.println(output);
        //Serial.print(",");
        //Serial.println("uT");

        // for (int i=0; i<n; i++) {
        //   Serial.println(currentTemps[i]);
        //   Serial.println(",");
        // }
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

          vent.begin(); //initialize vent attributes\

          //Cycle the vent for calibration:

          for (int i=0; i<5; i++) {
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
          State = startup;
      }


  private:
      //Variables for temp measurement
      int n = numOfTempSensors; //Number of temp probes connected to OneWire Bus
      float currentTemps[numOfTempSensors];
      unsigned long lastUpdate = tempMeasurementInterval * 1000; //the millis() of the last time we updated temps
      bool labelsPrinted = false;
      enum state{coolHigh, coolLow, heating, idle, startup};
      state State;
      state LastState;
      unsigned long stateChangeTime = 0;

      //PID variables
      int pGain = 1; //the gain for the proportional component
      int iGain = 0; //the gain for the integral component
      int dGain = 0; //the gain for the derivative component

      float targetTemp;
      float tolerance;
      float output;
      float error = 0; //The difference between desired and actual temp
      float sumOfPastError = 0; //used to calculate the integral component of the PID
      float lastError; //used to calculate the derivative component for PID
      RoboticVent vent;


      void updateTemps(){
          //This function is intened to update the array currentTemps[] with
          //the current value of temperatures. This function is needed
          //as we rarely need to update temps (which takes lots of time
          //about 290ms), yet we often need to quickly access the temps. 

          //if we are within time polling interval
          if ((millis() - lastUpdate) >= (tempMeasurementInterval * 1000)) {
              sensors.requestTemperatures();

              //update current temps
              for (int i=0; i<n; i++) {
                currentTemps[i] = sensors.getTempCByIndex(i);
                //Note that getTempCByIndex() is known to be slow,
                //There may be a faster way of accessing temps.
              }

              sumOfPastError += error;
              lastError = error;

              lastUpdate = millis();
          }
      }
};


ThermalControl projector;


void setup() {
  
  //delay(2000); //Allow time to open the Serial Monitor to debug
  
  Serial.begin(9600);
  //delay(2000);
  pinMode(ac, OUTPUT);
  pinMode(heater, OUTPUT);
  pinMode(fans, OUTPUT);
  pinMode(relay4, OUTPUT); //unused for now
  
  // Low means activated, as the relay is triggered with a ground. 
  //delay(10);
  //dht.begin();

  //Initialize the relay to be off
  digitalWrite(ac, HIGH);
  digitalWrite(heater, HIGH);
  digitalWrite(fans, HIGH);
  digitalWrite(relay4, HIGH);

  projector.begin();  // attaches the servo to specified pin
  projector.setTemp(22);
  projector.setTolerance(1); //the degrees C of allowed variation
  
}

void loop() {
  //While it may seem unnecesary to only have a few lines here, temperature
  //is only one job that the arduino is performing. I have multiple other
  //ideas for other tasks, and those could be implemented here as well.

   projector.processPID();
   projector.plotTempData();
   delay(500);
//     projector.testServo(50);


}
