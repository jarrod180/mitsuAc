
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <HeatPump.h>

// Include user configuration settings
#include "config.h"

const int redLedPin  = 0; // Onboard LED = digital pin 0 (red LED on adafruit ESP8266 huzzah)
const int blueLedPin = 2; // Onboard LED = digital pin 0 (blue LED on adafruit ESP8266 huzzah)

// Variables
WiFiClient espClient;
PubSubClient mqttClient(espClient);
HeatPump hp = HeatPump();
long nextInfoTime = 0;
mitsiLib::info_t nextInfo = mitsiLib::settings;

// Methods
void mqttConnect() {
  char strWill[64] = {0};
  strcat (strWill, mqttClientId);
  strcat (strWill, ": Offline");
  
  while (!mqttClient.connected()) {
    if (mqttClient.connect(mqttClientId, mqttUser, mqttPass, mqttStateTopic, 0, true, strWill)) {        
      mqttClient.subscribe(mqttSetTopic);
      mqttClient.subscribe(mqttSetDebugTopic);

      char strStatus[64] = {0};
      strcat (strStatus, mqttClientId);
      strcat (strStatus, ": Online");
      mqttClient.publish(mqttStateTopic, strStatus, true);
        
      digitalWrite(redLedPin, LOW);
    } else {
      delay(4000);
    }
  }
}

void wifiConnect(){
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(blueLedPin, LOW);
  }
}

void debug(const char* msg){
    mqttClient.publish("debug", msg);  
}

void rxSettings(mitsiLib::rxSettings_t* msg) {
  String info;
  switch (msg->kind){
    case mitsiLib::settings:
      info = String("Rx Settings: ");
      mqttClient.publish(mqttDebugTopic, info.c_str());
      break;
    case mitsiLib::roomTemp:
      info = String("Rx Room Temp: " + String(msg->data.roomTemp.roomTemp));
      mqttClient.publish(mqttDebugTopic, info.c_str());
      break;
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length){};

/* Setup */
void setup() {
  mqttClient.setServer(mqttServer, 1883);
  mqttClient.setCallback(mqttCallback);

  hp.setDebugCb(debug);
  hp.setRxSettingsCb(rxSettings);
  hp.connect(&Serial);
}


/* Main Loop */
void loop() {
  if (!mqttClient.connected()) {
	digitalWrite(redLedPin, HIGH);
    mqttConnect();
  }
  if (WiFi.status() != WL_CONNECTED){
	digitalWrite(blueLedPin, HIGH);
    wifiConnect();
  }

  if (millis() >= nextInfoTime){
	  hp.requestInfo(nextInfo);
	  nextInfoTime = millis() + 5000;
	  nextInfo = nextInfo==mitsiLib::settings?mitsiLib::roomTemp:mitsiLib::settings;
  }
  
  hp.monitor();
  
  mqttClient.loop();
}
