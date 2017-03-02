/*
  MitsuAc.h - Mitsubishi Air Conditioner/Heat Pump protocol library
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
//#ifndef __MitsuAc_H__
#define __MitsuAc_H__
#include <HardwareSerial.h>
#include "Arduino.h"
#include "MitsuProtocol.h"

class MitsuAc
{
  public:
    // Constructor
    MitsuAc(HardwareSerial *serial);
       
    // Start the serial and trigger an init packet to the unit
    void initialize();
    
    // Monitor the unit, call this regularly in the main loop
    void monitor();
    
    // Get current settings, json encoded
    void getSettingsJson(char* jsonSettings);
    
    // Put immediately the requested settings
    int putSettingsJson(const char* jsonSettings);

    #ifdef DEBUG_ON
    void setDebugCb(DEBUG_CB);
    void sendPkt(uint8_t data[], size_t len);
    #endif

  private:
    #ifdef DEBUG_ON
    void log (const char* msg);
    DEBUG_CB;
    #endif
	 
	 // Constants
	 const int MIN_INFO_REQ_WAIT_TIME   = 400;  //ms 
	 const int MIN_CONNECTION_WAIT_TIME = 5000; //ms
	 const int MIN_SETTINGS_WAIT_TIME   = 400;  //ms 
	 const int MIN_TX_DELAY_WAIT_TIME   = 100;   //ms - must be less than the above
    
    // Protocol objects
    MitsuProtocol ml = MitsuProtocol();
    MitsuProtocol::packetBuilder pb = MitsuProtocol::packetBuilder(&ml);
    
    // Private Methods
	 void sendInit();
    void sendRequestInfo(MitsuProtocol::info_t kind);
    void storeRxSettings(MitsuProtocol::rxSettings_t settings);
    
    // Internal states
    MitsuProtocol::settings_t lastSettings = ml.emptySettings;
    MitsuProtocol::settings_t targetSettings = ml.emptySettings;
    MitsuProtocol::roomTemp_t lastRoomTemp = {0,false,0.0,false,0.0,false};
    MitsuProtocol::info_t lastInfo = MitsuProtocol::roomTemp;
    
    unsigned long lastRxSettingsTime = 0;
    unsigned long lastRxRoomTempTime = 0;    
    unsigned long lastTxInfoRequestTime = 0;
    unsigned long lastTxSettingsTime = 0;    
    unsigned long lastTxInitTime = 0;
    unsigned long lastTxTime = 0;

    // Serial object and methods
    void sendData(uint8_t* buf, int len);
    HardwareSerial * _HardSerial;
};
//endif
