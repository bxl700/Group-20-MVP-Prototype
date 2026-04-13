#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// WiFi Configuration
const char* ssid = "CaseRegistered";

// Web Server Configuration
AsyncWebServer server(80);
AsyncEventSource events("/events");

// Pin Definitions
const int motorB_1A = 33;
const int motorB_2A = 15;
const int greenLed = 13;
const int blueLed = 12;
const int redLed = 27;
const int soilMoisturePin = 34;  // A2
const int waterLevelPin = 36;    // A4

// Sensor Thresholds and Calibration
const int dryValue = 2500;  // ESP32 ADC: 0–4095
const int waterThreshold = 20;      // "Add Water" alert triggers below 20%
const int waterMaxVal = 2500;       // Calibration: Max analog value when cup is full
const int waterMinVal = 0;          // Calibration: Value when sensor is dry

// Default pump duration
const int defaultPumpTime = 2000;  // milliseconds

// function to turn pump on for specified time
void pump(int time) {
  digitalWrite(motorB_1A, HIGH);
  digitalWrite(motorB_2A, LOW);
  delay(time); // delay for the specified time
  digitalWrite(motorB_1A, LOW);  // turn off pump
  digitalWrite(motorB_2A, LOW);
}

// function to toggle LEDs on
void ledON() {
  Serial.println("C++ ledON() called (toggle ON)");
  digitalWrite(greenLed, HIGH);
  digitalWrite(blueLed, HIGH);
  digitalWrite(redLed, HIGH);
}

// function to toggle LEDs off
void ledOFF() {
  Serial.println("C++ ledOFF() called (toggle OFF)");
  digitalWrite(greenLed, LOW);
  digitalWrite(blueLed, LOW);
  digitalWrite(redLed, LOW);
}

// function to call pump()
void pumpDuration(int time = defaultPumpTime) {
  Serial.println("C++ pumpDuration() start");
  pump(time); // pump on for the specified time
  Serial.println("C++ pumpDuration() end");
}

// Initialize WiFi and connect to ssid, print local ip and MAC address to serial
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid);
  Serial.println("MAC Address: " + WiFi.macAddress());
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  
  int attempts = 0;
  const int maxAttempts = 20;  // Timeout after ~20 seconds
  while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
    Serial.print('.');
    delay(1000);
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.println("IP Address: " + WiFi.localIP().toString());
  } else {
    Serial.println("\nWiFi connection failed!");
  }
}

// HTML & CSS content for the web page
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>ESP Plant Monitor</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
    html {
      font-family: Arial;
      display: inline-block;
      text-align: center;
    }
    p {
      font-size: 1.2rem;
    }
    body {
      margin: 0;
      font-family: 'Segoe UI', Tahoma, sans-serif;
      background: linear-gradient(135deg, #dff6f5, #f7fbff);
    }
    .topnav {
      background: linear-gradient(135deg, #50B8B4, #2f8f8b);
      padding: 15px;
      box-shadow: 0 2px 10px rgba(0,0,0,0.1);
    }
    .topnav h1 {
      margin: 0;
      font-size: 1.5rem;
      color: white;
    }
    .content {
      padding: 20px;
      display: flex;
      justify-content: center;   /* horizontal center */
      align-items: center;       /* vertical center */
      min-height: calc(100vh - 70px); /* full height minus header */
    }
    .card {
      background-color: white;
      border-radius: 16px;
      box-shadow: 0 8px 25px rgba(0,0,0,0.1);
      padding: 30px;
      width: 320px;
      transition: transform 0.2s ease;
    }
    .reading {
      font-size: 1.8rem;
      font-weight: bold;
      color: #333;
    }
    .card p {
      margin: 15px 0;
    }
    .button {
      padding: 12px 20px;
      font-size: 1rem;
      border: none;
      border-radius: 10px;
      color: white;
      cursor: pointer;
      transition: all 0.2s ease;
    }
    .button:hover {
      transform: scale(1.05);
      opacity: 0.9;
    }
    .toggle-on {
      background: linear-gradient(135deg, #4CAF50, #2e7d32);
    }
    .toggle-off {
      background: linear-gradient(135deg, #f44336, #c62828);
    }
    #pumpDuration {
      background: linear-gradient(135deg, #2196F3, #1565c0);
    }
  </style>
</head>
<body>
  <div class="topnav">
    <h1>Plant Health Monitor</h1>
  </div>
  <div class="content">
    <div class="card">
      <p>Soil Moisture: <span class="reading" id="soil-moisture">--</span></p>
      <p>Reservoir Level: <span class="reading" id="reservoir-level">--</span></p>
      <p>Status: <span class="reading" id="status-msg">--</span></p>
      <p>
        <button id="ledToggle" class="button toggle-off" onclick="toggleButtonClicked()">LEDs OFF</button>
      </p>
      <p>
        <button id="pumpDuration" class="button" onclick="twoSecondButtonClicked()">Pump on for 2 seconds</button>
      </p>
    </div>
  </div>
  <script>
    if (!!window.EventSource) {
      var source = new EventSource('/events');

      source.addEventListener('open', function(e) {
        console.log("Events Connected");
      }, false);

      source.addEventListener('error', function(e) {
        if (e.target.readyState != EventSource.OPEN) {
          console.log("Events Disconnected");
        }
      }, false);

      source.addEventListener('soil', function(e) {
        console.log("soil message", e.data);
        document.getElementById("soil-moisture").innerHTML = e.data;
      }, false);

      source.addEventListener('reservoir', function(e) {
        console.log("reservoir message", e.data);
        document.getElementById("reservoir-level").innerHTML = e.data;
      }, false);

      source.addEventListener('status', function(e) {
        console.log("status message", e.data);
        document.getElementById("status-msg").innerHTML = e.data;
      }, false);
    }

    // Toggle button state
    let toggleOn = false;
    function updateToggleUI() {
      const btn = document.getElementById('ledToggle');
      if (toggleOn) {
        btn.classList.remove('toggle-off');
        btn.classList.add('toggle-on');
        btn.innerText = 'LEDs ON';
      } else {
        btn.classList.remove('toggle-on');
        btn.classList.add('toggle-off');
        btn.innerText = 'LEDs OFF';
      }
    }

    function toggleButtonClicked() {
      toggleOn = !toggleOn;
      updateToggleUI();
      if (toggleOn) {
        toggleOnFunc();
      } else {
        toggleOffFunc();
      }
    }

    function toggleOnFunc() {
      console.log('toggleOnFunc called (toggle ON)');
      fetch('/actionA').catch(err => console.error('actionA request failed', err));
    }

    function toggleOffFunc() {
      console.log('toggleOffFunc called (toggle OFF)');
      fetch('/actionB').catch(err => console.error('actionB request failed', err));
    }

    function pumpDuration() {
      console.log('pumpDuration called');
      fetch('/action2s').catch(err => console.error('action2s request failed', err));
    }

    function twoSecondButtonClicked() {
      console.log('2s function start');
      pumpDuration();
      setTimeout(function() {
        pumpDurationEnd();
        console.log('2s function end');
      }, 2000);
    }

    function pumpDurationEnd() {
      console.log('Pump duration ended');
    }
  </script>
</body>
</html>
)rawliteral";


void setup() {
  Serial.begin(9600);
  delay(1000);  // Allow serial to initialize

  // Initialize WiFi
  initWiFi();

  // Configure web server routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", index_html);
  });

  server.on("/actionA", HTTP_GET, [](AsyncWebServerRequest *request) {
    ledON();
    request->send(200, "text/plain", "LEDs turned ON");
  });

  server.on("/actionB", HTTP_GET, [](AsyncWebServerRequest *request) {
    ledOFF();
    request->send(200, "text/plain", "LEDs turned OFF");
  });

  server.on("/action2s", HTTP_GET, [](AsyncWebServerRequest *request) {
    pumpDuration();
    request->send(200, "text/plain", "Pump activated for 2 seconds");
  });

  // Configure EventSource
  events.onConnect([](AsyncEventSourceClient *client) {
    if (client->lastId()) {
      Serial.printf("Client reconnected! Last message ID: %u\n", client->lastId());
    }
    client->send("Connected!", NULL, millis(), 1000);
  });
  server.addHandler(&events);
  server.begin();
  Serial.println("Web server started!");

  // Configure pin modes
  pinMode(motorB_1A, OUTPUT);
  pinMode(motorB_2A, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(blueLed, OUTPUT);
  pinMode(redLed, OUTPUT);


  // NEW CODE
  pinMode(soilMoisturePin, INPUT);
  pinMode(waterLevelPin, INPUT);
  analogSetAttenuation(ADC_11db);
 

}

void loop() {
  // Read sensor values
  int soilValue = analogRead(soilMoisturePin);
  int waterValue = analogRead(waterLevelPin);
  
  Serial.print("Raw soil moisture sensor value: ");
  Serial.println(soilValue);
  Serial.print("Raw reservoir sensor value: ");
  Serial.println(waterValue);

  // Send soil moisture value via EventSource
  String soilMessage = String(soilValue);
  events.send(soilMessage.c_str(), "soil", millis());

  // Calculate and send water level percentage
  int waterLevelPercent = map(waterValue, waterMinVal, waterMaxVal, 0, 100);
  waterLevelPercent = constrain(waterLevelPercent, 0, 100);
  String reservoirMessage = String(waterLevelPercent) + "%";
  events.send(reservoirMessage.c_str(), "reservoir", millis());

  // Determine and send status message
  String waterStatusMsg;
  if (waterLevelPercent < waterThreshold) {
    // waterStatusMsg = "Add Water";
    Serial.print("ALERT: Low Water (");
  } else {
    // waterStatusMsg = "Good";
    Serial.print("Level OK (");
  }
  Serial.print(waterLevelPercent);
  Serial.println("%)");
  // events.send(waterStatusMsg.c_str(), "status", millis());

  String soilStatusMsg;
  if (soilValue > dryValue) {
    soilStatusMsg = "Soil needs water!";
    Serial.println("ALERT: Soil is Dry");
  } else {
    soilStatusMsg = "Moist";
    Serial.println("Soil moisture is good");
  }
  events.send(soilStatusMsg.c_str(), "status", millis());

  // dummy test message for reservoir level and status
  // String reservoirMessage = "100%";
  // events.send(reservoirMessage.c_str(), "reservoir", millis());
  // String waterStatusMsg = "Good";
  // events.send(waterStatusMsg.c_str(), "status", millis());

  delay(500);  // update sensors every half second
}
