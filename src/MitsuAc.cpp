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
#include "MitsuAc.h"
#include <ArduinoJson.h>


#ifdef DEBUG_ON
void MitsuAc::log (const char* msg){
    if (debugCb){
        debugCb (msg);
    }
}
void MitsuAc::setDebugCb(DEBUG_CB){
    this->debugCb = debugCb;
    ml.setDebugCb(debugCb);
}
void MitsuAc::sendPkt(uint8_t* data, size_t len){
    uint8_t buf[64] =  {0};
    memcpy(buf,data,len);
    
    // Put a checksum on the end
    uint8_t sum = 0;
    for (int i = 0; i < len; i++) {
        sum += data[i];
    }
    buf[len] = (0xfc - sum) & 0xff;    

    sendData (buf,len+1);
    
    
}
#endif
/* END DEBUG */

MitsuAc::MitsuAc(HardwareSerial *serial) {
  _HardSerial = serial;
}

void MitsuAc::initialize(){
  _HardSerial->begin(2400, SERIAL_8E1);
  delay(1000);
  firstRxSettingsReceived = false;
  sendInit();
}

void MitsuAc::sendRequestInfo(MitsuProtocol::info_t kind){
    uint8_t buf[32] = {0};
    int len = ml.getTxInfoPacket (buf, kind);
    sendData (buf, len);
    lastTxInfoRequestTime = millis();
}

void MitsuAc::sendInit() {
  uint8_t buf[16] = {0};
  int len = ml.getTxConnectPacket (buf);
  sendData(buf, len);
  lastTxInitTime = millis();
}

void MitsuAc::getSettingsJson(char* jsonSettings){
   char buf[16];
   strcpy(jsonSettings, "{\"pwr\":\""); 
   strcat(jsonSettings, ml.power_tToString(lastSettings.power));
   strcat(jsonSettings, "\",\"mode\":\"");
   strcat(jsonSettings, ml.mode_tToString(lastSettings.mode));
   strcat(jsonSettings, "\",\"fan\":\"");
   strcat(jsonSettings, ml.fan_tToString(lastSettings.fan));
   strcat(jsonSettings, "\",\"vane\":\"");
   strcat(jsonSettings, ml.vane_tToString(lastSettings.vane));
   strcat(jsonSettings, "\",\"wdvane\":\"");
   strcat(jsonSettings, ml.wideVane_tToString(lastSettings.wideVane));
   strcat(jsonSettings, "\",\"stemp\":");
   itoa(lastSettings.tempDegC,buf,10);
   strcat(jsonSettings, buf);
   strcat(jsonSettings, ",\"rtemp\":");
   itoa(lastRoomTemp.roomTemp,buf,10);
   strcat(jsonSettings, buf);
   strcat(jsonSettings, ",\"rtemp1\":");     
   dtostrf(lastRoomTemp.tempSens1Raw, 4, 1, buf);  
   strcat(jsonSettings, buf);
   strcat(jsonSettings, ",\"rtemp2\":");
   dtostrf(lastRoomTemp.tempSens2Raw, 4, 1, buf);   
   strcat(jsonSettings, buf);
   strcat(jsonSettings, "}");
}

int MitsuAc::putSettingsJson(const char* jsonSettings){
    StaticJsonBuffer<256> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(jsonSettings);
    //MitsuProtocol::settings_t targetSettings = ml.emptySettings;
    bool success=false;
    bool msgOk = true;
    
    if (root.containsKey("pwr") && root["pwr"].is<const char*>()){
      ml.power_tFromString(root["pwr"],&targetSettings.power,success);
      targetSettings.powerValid = true;
      msgOk = (msgOk & success);
    }else{
      targetSettings.powerValid = false;
      msgOk = false;
    }
    if (root.containsKey("mode") && root["mode"].is<const char*>()){
      ml.mode_tFromString(root["mode"],&targetSettings.mode,success);
      msgOk = (msgOk & success);
      targetSettings.modeValid = true;
    }else{
      msgOk = false;
      targetSettings.modeValid = false;
    }
    if (root.containsKey("fan")){
      ml.fan_tFromString(root["fan"],&targetSettings.fan,success);
      msgOk = (msgOk & success);
      targetSettings.fanValid = true;
    }else{
      msgOk = false;
      targetSettings.fanValid = false;
    }    
    if (root.containsKey("vane") && root["vane"].is<const char*>()){
      ml.vane_tFromString(root["vane"],&targetSettings.vane,success);
      msgOk = (msgOk & success);
      targetSettings.vaneValid = true;
    }else{
      msgOk = false;
      targetSettings.vaneValid = false;
    }    
    if (root.containsKey("wdvane") && root["wdvane"].is<const char*>()){
      ml.wideVane_tFromString(root["wdvane"],&targetSettings.wideVane,success);
      msgOk = (msgOk & success);
      targetSettings.wideVaneValid = true;
    }else{
      msgOk = false;
      targetSettings.wideVaneValid = false;
    }    
    if (root.containsKey("stemp") && root["stemp"].is<int>()){
      targetSettings.tempDegC = root["stemp"];
      targetSettings.tempDegCValid = true;
    }else{
      msgOk = false;
      targetSettings.tempDegCValid = false;
    }
    
    targetSettingsAchieved = false;

    uint8_t buf[32];
    int len = ml.getTxSettingsPacket(buf, targetSettings);
    sendData (buf,len);
    lastTxSettingsTime = millis();

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
  
  switch (currentState){

      case (INFO_REQ):
         // If no infos are being received, trigger an init packet
         if (((millis() - lastRxSettingsTime) > (MIN_INFO_REQ_WAIT_TIME * 10)) && 
             ((millis() - lastRxRoomTempTime) > (MIN_INFO_REQ_WAIT_TIME * 10)) &&
             ((millis() - lastTxInitTime) > MIN_CONNECTION_WAIT_TIME) &&
             ((millis() - lastTxTime) > MIN_TX_DELAY_WAIT_TIME)) {
            firstRxSettingsReceived = false;
            sendInit();
         }
         else if (((millis() - lastTxInfoRequestTime) > MIN_INFO_REQ_WAIT_TIME) &&
                  ((millis() - lastTxTime) > MIN_TX_DELAY_WAIT_TIME)){
             MitsuProtocol::info_t thisInfo = lastInfo==MitsuProtocol::settings ? MitsuProtocol::roomTemp : MitsuProtocol::settings;
             sendRequestInfo(thisInfo);
             lastInfo = thisInfo;
         }   
         currentState = SETTINGS;
         break;
      
      case (SETTINGS):
      
         // Check the target settings against the latest settings
         if (firstRxSettingsReceived &&
             !targetSettingsAchieved &&
             (!ml.equals(targetSettings, lastSettings)) && 
             ((millis() - lastTxSettingsTime) > MIN_SETTINGS_WAIT_TIME) &&
             ((millis() - lastTxTime) > MIN_TX_DELAY_WAIT_TIME)){
             uint8_t buf[32];
             int len = ml.getTxSettingsPacket(buf, targetSettings);
             sendData (buf,len);
             lastTxSettingsTime = millis();
         }
         currentState = INFO_REQ;
         break;
    }
}

// Private Methods
void MitsuAc::sendData(uint8_t* buf, int len){
    #ifdef DEBUG_CALLS
    log ("MitsuAc::sendData()");
    #endif
    #ifdef DEBUG_PACKETS
    char dmsg[256];
    strcpy (dmsg,"Tx Pkt: [");
    for(int i = 0; i < len; i++) {
        strcat(dmsg,"0x");
        char dbuf[8];
        if (buf[i]<=0x0f){strcat(dmsg,"0");}
        strcat(dmsg, itoa(buf[i],dbuf,16));
        if (i==4){strcat(dmsg,"]");};
        strcat(dmsg," ");
    }
    log(dmsg);
    #endif

    if (_HardSerial){
        for(int i = 0; i < len; i++) {
          #ifdef DEBUG_BYTES
          char dmsg[16];
          strcpy (dmsg,"Tx: 0x");
          char dbuf[8];
          strcat(dmsg, itoa(buf[i],dbuf,16));
          log(dmsg);    
          #endif
          _HardSerial->write((uint8_t)buf[i]);
        }
    }
    lastTxTime = millis();
}

void MitsuAc::storeRxSettings(MitsuProtocol::rxSettings_t settings){
    switch (settings.kind){
        case MitsuProtocol::info_t::settings:
            lastSettings = settings.data.settings;
            lastRxSettingsTime = millis();
            
            if(!firstRxSettingsReceived){
                firstRxSettingsReceived = true;
                targetSettings = settings.data.settings;
            }
            
            if (ml.equals(targetSettings, lastSettings)){
                // Target settings achieved, stop monitoring it
                targetSettingsAchieved = true;
            }
			   
            break;
        case MitsuProtocol::info_t::roomTemp :
            lastRoomTemp = settings.data.roomTemp;
			   lastRxRoomTempTime = millis();
            break;
    }
}

