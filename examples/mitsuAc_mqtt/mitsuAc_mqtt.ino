#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <MitsuAc.h>

WiFiClient wifi;
PubSubClient mqttClient(wifi);
MitsuAc ac(&Serial);

static const char* mqttServer="192.168.1.120";
static const char* mqttClientId ="aircon1";
static const char* mqttStateTopic="home/aircon1";
static const char* mqttSetTopic="home/aircon1/set";
static const char* mqttUser="mqtt";
static const char* mqttPass="mqtt";
static const char* ssid = "xxx";
static const char* password="xxx";


void debug(const char* msg){
  mqttClient.publish(mqttStateTopic, msg, false);
}

void mqttConnect() {
  char strOnline[64],strOffline[64];
  strcpy(strOnline,mqttClientId);
  strcat(strOnline," is online");
  strcpy(strOffline,mqttClientId);
  strcat(strOffline," is offline");

  while (!mqttClient.connected()) {
    if (mqttClient.connect(mqttClientId, mqttUser, mqttPass, mqttStateTopic, 0, true, strOffline)) {        
      // Connected, subscribe to the set topic and publish sensor state to online
      mqttClient.subscribe(mqttSetTopic);
      mqttClient.publish(mqttStateTopic, strOnline, true);
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
    char str[length];
    memcpy(str,payload,length);
    str[length] = '\0';
    ac.putSettingsJson(str,length);
};

void setup() {
  // Connect to a/c unit
  ac.connect();
  
  // Set up the MQTT client
  mqttClient.setServer(mqttServer, 1883);
  mqttClient.setCallback(mqttCallback);
  ac.setDebugCb(&debug);
}

char lastSettings[128];

void loop() {
  if (WiFi.status() != WL_CONNECTED){
    wifiConnect();
  }  
  if (!mqttClient.connected()) {
    mqttConnect();
  }
  mqttClient.loop();

  ac.monitor();

  char newSettings[128];
  size_t len=0;
  ac.getSettingsJson(newSettings,len);
  
  if (strcmp(lastSettings,newSettings) != 0){
    mqttClient.publish(mqttStateTopic, newSettings, true);
    strcpy(lastSettings, newSettings);
  }
  
  delay(100);
}
