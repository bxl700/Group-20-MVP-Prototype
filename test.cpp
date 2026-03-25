#include <Arduino.h>
#include <Wire.h>

// This code is for testing the I2C communication by scanning for devices on the I2C bus.
void setup() {
  Serial.begin(115200);
  Wire.begin();   // Feather default I2C pins
  delay(1000);

  Serial.println("\nI2C Scanner");
  for (uint8_t i = 1; i < 127; i++) {
    Wire.beginTransmission(i);
    if (Wire.endTransmission() == 0) {
      Serial.print("Found device at 0x");
      Serial.println(i, HEX);
    }
  }
}

void loop() {}