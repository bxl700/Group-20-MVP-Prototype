#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>






const char* ssid = "CaseRegistered";




// Create AsyncWebServer object on port 80
AsyncWebServer server(80);


// Create an Event Source on /events
AsyncEventSource events("/events");




// Initialize WiFi
void actionA() {
    Serial.println("C++ actionA() called (toggle ON)");
    // TODO: add the real C++ behavior for function A (example: turn on pump/led, etc.)
}

void actionB() {
    Serial.println("C++ actionB() called (toggle OFF)");
    // TODO: add the real C++ behavior for function B (example: turn off pump/led, etc.)
}

void actionTwoSeconds() {
    Serial.println("C++ actionTwoSeconds() start");
    // TODO: add the real C++ 2-second behavior, e.g., hold pin HIGH for 2 seconds
    // Example placeholder:
    // digitalWrite(GPIO_NUM_2, HIGH);
    delay(2000);
    // digitalWrite(GPIO_NUM_2, LOW);
    Serial.println("C++ actionTwoSeconds() end");
}

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
    .card { background-color: white; box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5); padding: 20px; }
    .reading { font-size: 1.4rem; }
    .button { padding: 12px 20px; font-size: 1rem; border: none; border-radius: 8px; color: white; cursor: pointer; }
    .toggle-on { background-color: red; }
    .toggle-off { background-color: green; }
  </style>
</head>
<body>
  <div class="topnav">
    <h1>Plant Health Monitor</h1>
  </div>
  <div class="content">
    <div class="card">
      <p>Status: <span class="reading" id="msg">Waiting...</span></p>
      <p>
        <button id="toggleBtn" class="button toggle-off" onclick="toggleButtonClicked()">Toggle OFF</button>
      </p>
      <p>
        <button id="twoSecondBtn" class="button" onclick="twoSecondButtonClicked()">Run 2s Function</button>
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
 
 source.addEventListener('test', function(e) {
  console.log("test message", e.data);
  document.getElementById("msg").innerHTML = e.data;
 }, false);
}

// Toggle button state
let toggleOn = false;
function updateToggleUI() {
  const btn = document.getElementById('toggleBtn');
  if (toggleOn) {
    btn.classList.remove('toggle-off');
    btn.classList.add('toggle-on');
    btn.innerText = 'Toggle ON';
  } else {
    btn.classList.remove('toggle-on');
    btn.classList.add('toggle-off');
    btn.innerText = 'Toggle OFF';
  }
}

function toggleButtonClicked() {
  toggleOn = !toggleOn;
  updateToggleUI();
  if (toggleOn) {
    functionA();
  } else {
    functionB();
  }
}

function functionA() {
  console.log('functionA called (toggle ON)');
  fetch('/actionA').catch(err => console.error('actionA request failed', err));

}

function functionB() {
  console.log('functionB called (toggle OFF)');
  fetch('/actionB').catch(err => console.error('actionB request failed', err));

}

function functionTwoSeconds() {
  console.log('functionTwoSeconds called');
  fetch('/action2s').catch(err => console.error('action2s request failed', err));

}

function twoSecondButtonClicked() {
  console.log('2s function start');
  functionTwoSeconds();
  setTimeout(function() {
    functionTwoSecondsEnd();
    console.log('2s function end');
  }, 2000);
}

function functionTwoSeconds() {

  console.log('Running 2-second action');
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
        actionA();
        request->send(200, "text/plain", "actionA triggered");
    });

    server.on("/actionB", HTTP_GET, [](AsyncWebServerRequest *request){
        actionB();
        request->send(200, "text/plain", "actionB triggered");
    });

    server.on("/action2s", HTTP_GET, [](AsyncWebServerRequest *request){
        actionTwoSeconds();
        request->send(200, "text/plain", "actionTwoSeconds triggered");
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




   
    String testMessage = "Soil: " + String("aaah");
    Serial.println(testMessage);
   
    // Send the test message to all connected clients
    events.send(testMessage.c_str(), "test", millis());
   
    delay(2000);  // Send message every 2 seconds
}
