#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Arduino.h>


//CONFIGURATION
const int SENSOR_PIN = A0;      // Signal wire connected to Analog Pin A0
const int THRESHOLD  = 20;      // "Add Water" alert triggers below 20%
const int MAX_VAL    = 2500;    // Calibration: Max analog value when cup is full
const int MIN_VAL    = 0;       // Calibration: Value when sensor is dry

// Initialize LCD (Default SunFounder I2C address is 0x27)
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  // Start Serial for debugging
  Serial.begin(115200);

  // Initialize LCD
  lcd.init();
  lcd.backlight();
  
  // Quick Startup Message
  lcd.setCursor(0, 0);
  lcd.print("System Loading");
  for(int i = 0; i < 3; i++) {
    lcd.print(".");
    delay(300);
  }
  lcd.clear();
}

void loop() {
  // 1. Read the sensor
  int rawValue = analogRead(SENSOR_PIN);

  // 2. Convert to Percentage (0 to 100)
  // ESP32 ADC is 12-bit (0-4095), but these sensors usually peak lower
  int levelPercent = map(rawValue, MIN_VAL, MAX_VAL, 0, 100);
  levelPercent = constrain(levelPercent, 0, 100);

  // 3. Display Static Header
  lcd.setCursor(0, 0);
  lcd.print("Level: ");
  lcd.print(levelPercent);
  lcd.print("%    "); // Spaces clear trailing digits

  // 4. Threshold Logic & Status Message
  lcd.setCursor(0, 1); // Move to second line
  
  if (levelPercent < THRESHOLD) {
    // Alert State
    lcd.print(">> ADD WATER! <<");
    
    // Debugging info to Serial
    Serial.print("ALERT: Low Water (");
    Serial.print(levelPercent);
    Serial.println("%)");
  } 
  else {
    // Safe State
    lcd.print("Status: GOOD    ");
    
    Serial.print("Level OK (");
    Serial.print(levelPercent);
    Serial.println("%)");
  }

  // 5. Small delay for stability
  delay(500);
}