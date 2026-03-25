#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>


// LCD I2C address and dimensions
LiquidCrystal_I2C lcd(0x27, 16, 2); // Pin 0x27 Tested

// Pin Configuration
const int soilPin = A2;

// Calibration value)
int dryValue = 2600;  // ESP32 ADC: 0–4095


void setup() {
    Serial.begin(115200);

    Wire.begin();  // Feather ESP32 default I2C pins

    // Starting Appearance for LCD
    lcd.init();
    lcd.backlight();

    // Startup Message
    lcd.setCursor(0,0);
    lcd.print("Plant Monitor");
    delay(1500);
    lcd.clear();
}

void loop() {

    int moistureValue = analogRead(soilPin);

    Serial.print("Soil Moisture Level: ");
    Serial.println(moistureValue);

    // Printing Soil Moisture Value on LCD as the first line
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Soil:");
    lcd.setCursor(6,0);
    lcd.print(moistureValue);

    lcd.setCursor(0,1);

    // Printing the status of the soil moisture on the second line of the LCD
    if (moistureValue > dryValue) {
        lcd.print("WATER!!");
    } else {
        lcd.print("Good^^");
    }

    delay(1000);
}