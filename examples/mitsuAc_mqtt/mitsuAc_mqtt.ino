#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <MitsuAc.h>

WiFiClient wifi;
PubSubClient mqttClient(wifi);
MitsuAc ac(&Serial);

static const char* mqttServer="192.168.1.120";
static const char* mqttClientId="aircon1";
static const char* mqttStateTopic="home/aircon1";
static const char* mqttSetTopic="home/aircon1/set";
static const char* mqttUser="xxx";
static const char* mqttPass="xxx";
static const char* ssid = "xxx";
static const char* password="xxx";

void mqttConnect() {
  // Offline is set as the MQTT will in case of ungraceful disconnect
  String strOffline = String (String(mqttClientId) + " is offline");
  String strOnline = String (String(mqttClientId) + " is online");

  while (!mqttClient.connected()) {
    if (mqttClient.connect(mqttClientId, mqttUser, mqttPass, mqttStateTopic, 0, true, strOffline.c_str())) {        
      // Connected, subscribe to the set topic and publish sensor state to online
      mqttClient.subscribe(mqttSetTopic);
      mqttClient.publish(mqttStateTopic, strOnline.c_str(), true);
    } else {
      delay(4000);
    }
  }
}

void wifiConnect(){
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay (100);
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length){
    char pl[256] = {0};
    for (int i; i<length; i++){
      pl[i] = payload[i];
    }
    mqttClient.publish(mqttStateTopic, pl, true);
    ac.putSettingsJson(pl);
};

void setup() {
  // Connect to a/c unit
  ac.connect();
  
  // Set up the MQTT client
  mqttClient.setServer(mqttServer, 1883);
  mqttClient.setCallback(mqttCallback);
}

String lastSettings = String("");

void loop() {
  if (WiFi.status() != WL_CONNECTED){
    wifiConnect();
  }  
  if (!mqttClient.connected()) {
    mqttConnect();
  }
  
  ac.monitor();

  String newSettings;
  ac.getSettingsJson(newSettings);
  
  if (!lastSettings.equals(newSettings)){
    mqttClient.publish(mqttStateTopic, newSettings.c_str(), true);
    lastSettings = String (newSettings.c_str());
  }
  
  mqttClient.loop();
}
