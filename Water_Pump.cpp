#include <Arduino.h>

// Define pump pins
const int motorB_1A = 33;
const int motorB_2A = 15;

void pump(){

  digitalWrite(motorB_1A, HIGH);
  digitalWrite(motorB_2A, LOW);

  delay(5000);// delay 5 seconds

  digitalWrite(motorB_1A, LOW);  // turn off pump
  digitalWrite(motorB_2A, LOW);
}

void setup() {

  pinMode(motorB_1A, OUTPUT);  // set pump pin 1 as output
  pinMode(motorB_2A, OUTPUT);  // set pump pin 2 as output

  

  

}

void loop() {

}