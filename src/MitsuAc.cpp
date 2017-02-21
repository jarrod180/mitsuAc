/*
  MitsuAc.cpp - Mitsubishi Air Conditioner/Heat Pump protocol library
  Copyright (c) 2017 Jarrod Lamb.  All right reserved.
  Originally reverse engineered by Hadley Rich (http://nice.net.nz)

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include <MitsuAc.h>
#include <ArduinoJson.h>

/* DEBUG */
#define DEBUG 1
#if DEBUG
void MitsuAc::log (const char* msg){
    if (debugCb){
        debugCb (msg);
    }
}
void MitsuAc::setDebugCb(DEBUG_CB){
    this->debugCb = debugCb;
    ml.setDebugCb(debugCb);
}
#endif
/* END DEBUG */

MitsuAc::MitsuAc(HardwareSerial *serial) {
  _HardSerial = serial;
}

void MitsuAc::requestInfo(MitsuProtocol::info_t kind){
    byte buf[32] = {0};
    int len = ml.getTxInfoPacket (buf, kind);
    write (buf, len);
}

void MitsuAc::connect() {
  _HardSerial->begin(2400, SERIAL_8E1);
  delay(1000);
  byte buf[16] = {0};
  int len = ml.getTxConnectPacket (buf);
  write(buf, len);
}

void MitsuAc::getSettingsJson(char* settings, size_t len){
   char buf[sizeof(int)+1];
   strcpy(settings, "{'power':'"); 
   strcat(settings, ml.power_tToString(lastSettings.power));
   strcat(settings, "','mode':'");
   strcat(settings, ml.mode_tToString(lastSettings.mode));
   strcat(settings, "','fan':'");
   strcat(settings, ml.fan_tToString(lastSettings.fan));
   strcat(settings, "','vane':'");
   strcat(settings, ml.vane_tToString(lastSettings.vane));
   strcat(settings, "','widevane':'");
   strcat(settings, ml.wideVane_tToString(lastSettings.wideVane));
   strcat(settings, "','temp':");
   itoa(lastSettings.tempDegC,buf,10);
   strcat(settings, buf);
   itoa(lastRoomTemp.roomTemp,buf,10);
   strcat(settings, buf);
   strcat(settings, "}");
   len = strlen(settings);
}

int MitsuAc::putSettingsJson(const char* jsonSettings){
    StaticJsonBuffer<128> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(jsonSettings);
    MitsuProtocol::settings_t newSettings = lastSettings;
    bool success=false;
    bool msgOk = true;
    
    if (root.containsKey("power") && root["power"].is<const char*>()){
      ml.power_tFromString(root["power"],newSettings.power,success);
      msgOk = (msgOk & success);
    }else{
      msgOk = false;
    }
    if (root.containsKey("mode") && root["mode"].is<const char*>()){
      ml.mode_tFromString(root["mode"],newSettings.mode,success);
      msgOk = (msgOk & success);
    }else{
      msgOk = false;
    }
    if (root.containsKey("fan") && root["fan"].is<const char*>()){
      ml.fan_tFromString(root["fan"],newSettings.fan,success);
      msgOk = (msgOk & success);
    }else{
      msgOk = false;
    }    
    if (root.containsKey("vane") && root["vane"].is<const char*>()){
      ml.vane_tFromString(root["vane"],newSettings.vane,success);
      msgOk = (msgOk & success);
    }else{
      msgOk = false;
    }    
    if (root.containsKey("widevane") && root["widevane"].is<const char*>()){
      ml.wideVane_tFromString(root["widevane"],newSettings.wideVane,success);
      msgOk = (msgOk & success);
    }else{
      msgOk = false;
    }    
    if (root.containsKey("temp") && root["temp"].is<int>()){
      newSettings.tempDegC = root["temp"];
    }else{
      msgOk = false;
    }

    byte buf[128];
    int len = ml.getTxSettingsPacket(buf, newSettings);
    write (buf,len);

    return msgOk?0:-1;
}

void MitsuAc::monitor() {
  // Service the serial port
  while (_HardSerial->available() > 0){
    pb.addByte(_HardSerial->read());
    if (pb.complete() && pb.valid()) {
        MitsuProtocol::msg_t msg = pb.getData();
        if (msg.msgKindValid){
            switch (msg.kind){
                case MitsuProtocol::msgKind_t::rxCurrentSettings:
                    storeRxSettings(msg.data.rxCurrentSettingsData);
                    break;
            }
        }
        pb.reset();
    }
  }
  // Request an Info if required
  if (millis() > lastInfoRequestTime + 500){
     MitsuProtocol::info_t thisInfo = lastInfo==MitsuProtocol::settings ? MitsuProtocol::roomTemp : MitsuProtocol::settings;
     requestInfo(thisInfo);
     lastInfoRequestTime = millis();
     lastInfo = thisInfo;
  }
}

// Private Methods
void MitsuAc::write(byte* buf, int len){
    if (_HardSerial){
        for(int i = 0; i < len; i++) {
          #if (DEBUG > 1)
          log (String(String("Tx: 0x") + String(buf[i],HEX)).c_str());
          #endif
          _HardSerial->write((uint8_t)buf[i]);
        }
    }
}

void MitsuAc::storeRxSettings(MitsuProtocol::rxSettings_t settings){
    switch (settings.kind){
        case MitsuProtocol::info_t::settings:
            lastSettings = settings.data.settings;
            break;
        case MitsuProtocol::info_t::roomTemp :
            lastRoomTemp = settings.data.roomTemp;
            break;
    }
}

