#include <Arduino.h>
// Circuit Setup:
// Cathode - GND
//VCC - 3.3 V
//GND - GND
// SIG - A3


const int green_led = 13;
const int blue_led = 12;
const int red_led = 27;

const int buttonPin = A3;

bool ledState = false;
bool lastButtonState = LOW;

void setup() {

  Serial.begin(115200);
  pinMode(green_led, OUTPUT);
  pinMode(blue_led, OUTPUT);
  pinMode(red_led, OUTPUT);
  pinMode(buttonPin, INPUT);
}

void loop() {

    bool buttonState = digitalRead(buttonPin);

    // Button press
    if (buttonState == HIGH && lastButtonState == LOW) {
        ledState = !ledState;  
        digitalWrite(green_led, ledState);
        digitalWrite(blue_led, ledState);
        digitalWrite(red_led, ledState);
        delay(200);
    }

    lastButtonState = buttonState;
}