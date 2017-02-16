/*
Configuration settings for ESP8266 Mitsubishi HeatPump controller
*/

// wifi settings
const char* ssid     = "<SSID>";
const char* password = "<PASSWORD>";

// mqtt server settings
const char* mqtt_server   = "<MQTT SERVER>";
const char* mqtt_username = "<MQTT USER>";
const char* mqtt_password = "<MQTT PASS>";

// mqtt client settings
const char* client_id          = "hp-1"; // Must be unique on the MQTT network
const char* hp_topic           = "hp/";
const char* hp_set_topic       = "hp/set";
const char* hp_roomtemp_topic  = "hp/roomtemp";
const char* hp_debug_topic     = "hp/debug/output";
const char* hp_debug_set_topic = "hp/debug/set";
