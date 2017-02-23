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
#define DEBUG 0
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
    sendBytes (buf, len);
}

void MitsuAc::connect() {
  _HardSerial->begin(2400, SERIAL_8E1);
  delay(1000);
  byte buf[16] = {0};
  int len = ml.getTxConnectPacket (buf);
  sendBytes(buf, len);
}

void MitsuAc::getSettingsJson(char* settings){
   char buf[3];
   strcpy(settings, "{\"power\":\""); 
   strcat(settings, ml.power_tToString(lastSettings.power));
   strcat(settings, "\",\"mode\":\"");
   strcat(settings, ml.mode_tToString(lastSettings.mode));
   strcat(settings, "\",\"fan\":\"");
   strcat(settings, ml.fan_tToString(lastSettings.fan));
   strcat(settings, "\",\"vane\":\"");
   strcat(settings, ml.vane_tToString(lastSettings.vane));
   strcat(settings, "\",\"widevane\":\"");
   strcat(settings, ml.wideVane_tToString(lastSettings.wideVane));
   strcat(settings, "\",\"temp\":");
   itoa(lastSettings.tempDegC,&buf[0],10);
   strcat(settings, buf);
   strcat(settings, ",\"roomtemp\":");
   itoa(lastRoomTemp.roomTemp,&buf[0],10);
   strcat(settings, buf);
   strcat(settings, "}");
}

int MitsuAc::putSettingsJson(const char* jsonSettings){
    StaticJsonBuffer<256> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(jsonSettings);
    MitsuProtocol::settings_t newSettings = ml.emptySettings;
    bool success=false;
    bool jsonOk = true;
    
    if (root.containsKey("power")){
	  if (root["power"].is<const char*>()){
        ml.power_tFromString(root["power"],&newSettings.power,success);
        newSettings.powerValid = true;
        jsonOk = (msgOk & success);
	  }else{
	    jsonOk = false;
	  }
    }else{
      newSettings.powerValid = false;
      jsonOk = false;
    }
	
	
	
    if (root.containsKey("temp") && root["temp"].is<int>()){
      newSettings.tempDegC = root["temp"];
      newSettings.tempDegCValid = true;
    }else{
      jsonOk = false;
      newSettings.tempDegCValid = false;
    }

    byte buf[128];
    int len = ml.getTxSettingsPacket(buf, newSettings);
    sendBytes (buf,len);

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
void MitsuAc::sendBytes(byte* buf, int len){
    #if DEBUG > 1
    log ("MitsuAc::sendBytes(), len:");
    log (String(len).c_str());
    String msg("Tx Pkt: ");
    for(int i = 0; i < len; i++) {
        msg = msg + " 0x";
        msg = msg + String(buf[i],HEX);
    }
    log(msg.c_str());
    #endif

    if (_HardSerial){
        for(int i = 0; i < len; i++) {
          #if DEBUG > 2
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

