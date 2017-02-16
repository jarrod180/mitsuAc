
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <HeatPump.h>

// Include user configuration settings
#include "config.h"

// pinouts
const int redLedPin  = 0; // Onboard LED = digital pin 0 (red LED on adafruit ESP8266 huzzah)
const int blueLedPin = 2; // Onboard LED = digital pin 0 (blue LED on adafruit ESP8266 huzzah)

// wifi, mqtt and heatpump client instances
WiFiClient espClient;
PubSubClient mqtt_client(espClient);
HeatPump hp;

void setup() {
  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    // wait 500ms, flashing the blue LED to indicate WiFi connecting...
    digitalWrite(blueLedPin, LOW);
    delay(250);
    digitalWrite(blueLedPin, HIGH);
    delay(250);
  }

  // startup mqtt connection
  mqtt_client.setServer(mqtt_server, 1883);
  mqtt_client.setCallback(mqttCallback);

  // connect to the heatpump
  hp.connect(&Serial);

  hp.setSettingsChangedCallback(hpSettingsChanged);
  hp.setPacketReceivedCallback(hpPacketReceived);
  hp.setRoomTempChangedCallback(hpRoomTempChanged);
}

void hpSettingsChanged() {
    mqtt_client.publish(hp_debug_topic, "settings changed");
}

void hpRoomTempChanged(int newTemp) {
    mqtt_client.publish(hp_debug_topic, "temp changed");
}

void hpPacketReceived(byte* packet, unsigned int length) {
    mqtt_client.publish(hp_debug_topic, "packet rx");
  
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {

}

void reconnect() {
  // Loop until we're reconnected
  while (!mqtt_client.connected()) {
    // Attempt to connect
    if (mqtt_client.connect(client_id, mqtt_username, mqtt_password)) {
      mqtt_client.subscribe(hp_set_topic);
      mqtt_client.subscribe(hp_debug_set_topic);
    } else {
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

int lastSend = 0;

void loop() {

  if (!mqtt_client.connected()) {
    reconnect();
  }
  
  if (millis() > lastSend + 1000){
    char blah[64] = {0};
    strcat (blah, client_id);
    strcat (blah, " online");
    mqtt_client.publish(hp_debug_topic, blah, false);
    lastSend = millis();
  }

  hp.monitor();

  mqtt_client.loop();
}



