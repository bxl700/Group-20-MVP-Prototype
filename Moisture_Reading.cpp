#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "html.h"


// LCD I2C address and dimensions
LiquidCrystal_I2C lcd(0x27, 16, 2); // Pin 0x27 Tested

// Pin Configuration
const int soilPin = A2;

// WiFi Network Name
const char* ssid = "CaseRegistered";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
// Create an Event Source on /events
AsyncEventSource events("/events");

// Calibration value)
int dryValue = 2600;  // ESP32 ADC: 0–4095

// Initialize WiFi
void initWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid);
    Serial.println(WiFi.macAddress());
    Serial.print("Connecting to WiFi ..");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print('.');
        delay(1000);
    }
    Serial.println(WiFi.localIP());
    Serial.println(WiFi.macAddress());
}


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

    // Handle Web Server
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", index_html);
    });

    // Handle Web Server Events
    events.onConnect([](AsyncEventSourceClient *client){
        if(client->lastId()){
            Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
        }
        client->send("Connected!", NULL, millis(), 1000);
    });
    server.addHandler(&events);
    server.begin();

    Serial.println("Web server started!");
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

    String testMessage = "Soil: " + String(moistureValue);
    events.send(testMessage.c_str(), "test", millis());
    // Printing the status of the soil moisture on the second line of the LCD
    if (moistureValue > dryValue) {
        lcd.print("WATER!!");
    } else {
        lcd.print("Good^^");
    }

    delay(1000);
}