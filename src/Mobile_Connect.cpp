#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>






const char* ssid = "CaseRegistered";




// Create AsyncWebServer object on port 80
AsyncWebServer server(80);


// Create an Event Source on /events
AsyncEventSource events("/events");


// Define pump pins
const int motorB_1A = 33;
const int motorB_2A = 15;

const int green_led = 13;
const int blue_led = 12;
const int red_led = 27;

// Pin Configuration
const int soilPin = 34;  //A2

// Calibration value)
int dryValue = 2600;  // ESP32 ADC: 0–4095

//CONFIGURATION
const int SENSOR_PIN = 35;     // A3
const int THRESHOLD  = 20;      // "Add Water" alert triggers below 20%
const int MAX_VAL    = 2500;    // Calibration: Max analog value when cup is full
const int MIN_VAL    = 0;       // Calibration: Value when sensor is dry

void pump(){

  digitalWrite(motorB_1A, HIGH);
  digitalWrite(motorB_2A, LOW);

  delay(2000);// delay 2 seconds

  digitalWrite(motorB_1A, LOW);  // turn off pump
  digitalWrite(motorB_2A, LOW);
}





void ledON() {
    Serial.println("C++ ledON() called (toggle ON)");
    digitalWrite(green_led, HIGH);
    digitalWrite(blue_led, HIGH);
    digitalWrite(red_led, HIGH);
    
}

void ledOFF() {
    Serial.println("C++ ledOFF() called (toggle OFF)");
    digitalWrite(green_led, LOW);
    digitalWrite(blue_led, LOW);
    digitalWrite(red_led, LOW);
}

void pumpDuration() {
    Serial.println("C++ pumpDuration() start");

    pump();

    Serial.println("C++ pumpDuration() end");
}

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
}


const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP Plant Monitor</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    p { font-size: 1.2rem;}
    body {  margin: 0;}
    .topnav { overflow: hidden; background-color: #50B8B4; color: white; font-size: 1rem; }
    .content { padding: 20px; }
    .card { background-color: white; box-shadow: 2px 2px 12px 1px rgba(205, 59, 59, 0.81); padding: 20px; }
    .reading { font-size: 1.4rem; }
    .button { padding: 12px 20px; font-size: 1rem; border: none; border-radius: 8px; color: purple; cursor: pointer; }
    .toggle-on { background-color: green; }
    .toggle-off { background-color: red; }
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
</html>)rawliteral";


void setup() {
    Serial.begin(9600);


    initWiFi();


    // Handle Web Server
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", index_html);
    });

    server.on("/actionA", HTTP_GET, [](AsyncWebServerRequest *request){
        ledON();
        request->send(200, "text/plain", "actionA triggered");
    });

    server.on("/actionB", HTTP_GET, [](AsyncWebServerRequest *request){
        ledOFF();
        request->send(200, "text/plain", "actionB triggered");
    });

    server.on("/action2s", HTTP_GET, [](AsyncWebServerRequest *request){
        pumpDuration();
        request->send(200, "text/plain", "pumpDuration triggered");
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

    pinMode(motorB_1A, OUTPUT);  // set pump pin 1 as output
    pinMode(motorB_2A, OUTPUT);  // set pump pin 2 as output

    pinMode(green_led, OUTPUT);
    pinMode(blue_led, OUTPUT);
    pinMode(red_led, OUTPUT);

}




void loop() {
    // 1. Read soil moisture sensor
    int moistureValue = analogRead(soilPin);
    String soilMessage = String(moistureValue);
    events.send(soilMessage.c_str(), "soil", millis());

    // // 2. Read reservoir level sensor
    // int rawValue = analogRead(SENSOR_PIN);
    // Serial.println("Raw reservoir sensor value: " + String(rawValue));
    // int levelPercent = map(rawValue, MIN_VAL, MAX_VAL, 0, 100);
    // levelPercent = constrain(levelPercent, 0, 100);
    // String reservoirMessage = String(levelPercent) + "%";
    // events.send(reservoirMessage.c_str(), "reservoir", millis());

    String reservoirMessage = "100%";
    events.send(reservoirMessage.c_str(), "reservoir", millis());

    // 3. Determine and send status message
    // String statusMessage;
    // if (levelPercent < THRESHOLD) {
    //     statusMessage = "Add Water";
    //     Serial.print("ALERT: Low Water (");
    // } else {
    //     statusMessage = "Good";
    //     Serial.print("Level OK (");
    // }
    // Serial.print(levelPercent);
    // Serial.println("%)");
    // events.send(statusMessage.c_str(), "status", millis());
    String statusMessage = "Good";
    events.send(statusMessage.c_str(), "status", millis());

    delay(500);  // Send message every 2 seconds
}
