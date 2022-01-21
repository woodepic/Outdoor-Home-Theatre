/* Sweep
 by BARRAGAN <http://barraganstudio.com>
 This example code is in the public domain.

 modified 8 Nov 2013
 by Scott Fitzgerald
 http://www.arduino.cc/en/Tutorial/Sweep
*/

#include <Servo.h>
#define start 21
#define duration 35
#define rest 2000
#define speedDelay 50

Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards

int pos = 0;    // variable to store the servo position

void setup() {
  myservo.attach(9);  // attaches the servo on pin 9 to the servo object
  Serial.begin(9600);
}

void loop() {

  
   for (pos = start + 0; pos <= start + duration; pos++) { // goes from 180 degrees to 0 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(speedDelay);                      // waits 15ms for the servo to reach the position
    Serial.println(pos-start); 
   }
  delay(rest);
  
   for (pos = start + duration; pos >= start + 0; pos--) { // goes from 180 degrees to 0 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(speedDelay);                      // waits 15ms for the servo to reach the position
    Serial.println(pos-start); 
   }
  delay(rest);
  

  myservo.write(25);
}
