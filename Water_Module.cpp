#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Arduino.h>

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

const int sensorPin = A0; // Analog pin for Water Level Sensor
int sensorValue = 0;      // Variable to store sensor data

void setup() {
  Serial.begin(115200);
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  
  lcd.setCursor(0, 0);
  lcd.print("Water Level:");
}

void loop() {
  // Read the analog value (0 - 4095 for ESP32)
  sensorValue = analogRead(sensorPin);

  // Map the value to a percentage for easier reading
  // Note: You may need to calibrate 0 and 1500 based on your specific sensor
  int levelPercentage = map(sensorValue, 0, 2500, 0, 100);
  levelPercentage = constrain(levelPercentage, 0, 100);

  // Print to Serial Monitor for debugging
  Serial.print("Sensor Value: ");
  Serial.println(sensorValue);

  // Update LCD
  lcd.setCursor(0, 1);
  lcd.print("Level: ");
  lcd.print(levelPercentage);
  lcd.print("%   "); // Extra spaces clear old digits

  delay(500); // Wait half a second between readings
}
