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
#endif
/* END DEBUG */

MitsuAc::MitsuAc() {
}

void MitsuAc::requestInfo(MitsuProtocol::info_t kind){
    byte buf[32] = {0};
    int len = ml.getTxInfoPacket (buf, kind);
    write (buf, len);
}

void MitsuAc::connect(HardwareSerial *serial) {
  _HardSerial = serial;
  _HardSerial->begin(2400, SERIAL_8E1);
  delay(1000);
  
  byte buf[16] = {0};
  int len = ml.getTxConnectPacket (buf);

  for(int cnt = 0; cnt < 3; cnt++) {
    write(buf, len);
    delay(250);
  }
}

const char* MitsuAc::getRoomTempJson() {
   String json = String(String("{'roomtemp':") + lastRoomTemp.roomTemp);
   json += String("}");
   return json.c_str();
}

const char* MitsuAc::getSettingsJson(){
   String json = String(String("{'power':'") + ml.power_tToString(lastSettings.power));
   json += String(String("','mode':'") + ml.mode_tToString(lastSettings.mode));
   json += String(String("','fan':'") + ml.fan_tToString(lastSettings.fan));
   json += String(String("','vane':'") + ml.vane_tToString(lastSettings.vane));
   json += String(String("','widevane':'") + ml.wideVane_tToString(lastSettings.wideVane));
   json += String(String("','temp':") + lastSettings.tempDegC);
   json += String("}");
   return json.c_str();
}

int MitsuAc::putSettings(const char* jsonSettings){
    StaticJsonBuffer<100> jsonBuffer;
	JsonObject& root = jsonBuffer.parseObject(jsonSettings);
    MitsuProtocol::settings_t newSettings = lastSettings;
	bool success=false;
	bool jsonOk = true;
	
	if (root.containsKey("power") && root["power"].is<const char*>()){
	  MitsuProtocol::power_tFromString(root["power"],newSettings.power,success);
	  jsonOk = (jsonOk & success);
	}else{
	  jsonOk = false;
	}
	if (root.containsKey("mode") && root["mode"].is<const char*>()){
	  MitsuProtocol::mode_tFromString(root["mode"],newSettings.mode,success);
	  jsonOk = (jsonOk & success);
	}else{
	  jsonOk = false;
	}
	if (root.containsKey("fan") && root["fan"].is<const char*>()){
	  MitsuProtocol::fan_tFromString(root["fan"],newSettings.fan,success);
	  jsonOk = (jsonOk & success);
	}else{
	  jsonOk = false;
	}	
	if (root.containsKey("vane") && root["vane"].is<const char*>()){
	  MitsuProtocol::vane_tFromString(root["vane"],newSettings.vane,success);
	  jsonOk = (jsonOk & success);
	}else{
	  jsonOk = false;
	}	
	if (root.containsKey("widevane") && root["widevane"].is<const char*>()){
	  MitsuProtocol::wideVane_tFromString(root["widevane"],newSettings.wideVane,success);
	  jsonOk = (jsonOk & success);
	}else{
	  jsonOk = false;
	}	
	if (root.containsKey("temp") && root["temp"].is<int>()){
	  newSettings.tempDegC = root["temp"];
	}else{
	  jsonOk = false;
	}
	return 0;
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

