#include <ESP8266WiFi.h>
#include <PubSubClient.h> //Warning!!! increase MQTT_MAX_PACKET_SIZE in PubSubClient.h from 128 to 256!
#include <MitsuAc.h>

WiFiClient wifi;
PubSubClient mqttClient(wifi);
MitsuAc ac(&Serial);

static const char* mqttServer="192.168.1.120";
static const char* mqttClientId ="bed3ac";
static const char* mqttStateTopic="home/bed3ac";
static const char* mqttSetTopic="home/bed3ac/set";
static const char* mqttUser="mqtt";
static const char* mqttPass="mqtt";
static const char* ssid = "xxx";
static const char* password="xxx";

//DEBUG
static const char* mqttDebugTopic="home/bed3ac/debug";
static const char* mqttDebugPacketTopic="home/bed3ac/debug/packet";
void debug(const char* msg){
  mqttClient.publish(mqttDebugTopic, msg, false);
}
//END DEBUG


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
/*
    //DEBUG   
    if (strcmp(topic,mqttDebugPacketTopic)==0){
        uint8_t buf[64] = {0};
        int ptr = 0;
        char bstr[3] = {'\0'};
      
        for (int i = 0; i < length; i=i+2){
            bstr[0] = payload[i];
            bstr[1] = payload[i+1];
            buf[ptr] = static_cast<uint8_t>(strtol(bstr, NULL, 16));
            ptr++;
        }
        ac.sendPkt(buf,ptr);
        return;
    }
    // END DEBUG
*/ 
    char str[length];
    memcpy(str, payload, length);
    str[length] = '\0';
    ac.putSettingsJson(str);
};

void setup() {
  // Connect to a/c unit
  ac.initialize();
  
  // Set up the MQTT client
  mqttClient.setServer(mqttServer, 1883);
  mqttClient.setCallback(mqttCallback);
  ac.setDebugCb(&debug); //DEBUG
}

char lastSettings[256];

void loop() {
  if (WiFi.status() != WL_CONNECTED){
    wifiConnect();
  }  
  if (!mqttClient.connected()) {
    mqttConnect();
  }
  mqttClient.loop();

  ac.monitor();

  char newSettings[256] = {0};
  ac.getSettingsJson(newSettings);
  
  if (strcmp(lastSettings,newSettings) != 0){
    mqttClient.publish(mqttStateTopic, newSettings, true);
    strcpy(lastSettings, newSettings);
  }
  
  delay(20);
}
