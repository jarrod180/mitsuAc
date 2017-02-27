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
    void getSettingsJson(char* settings);
    
    // Put immediately the requested settings
    int putSettingsJson(const char* jsonSettings);

    #ifdef DEBUG_ON
    void setDebugCb(DEBUG_CB);
    #endif

  private:
    #ifdef DEBUG_ON
    void log (const char* msg);
    DEBUG_CB;
    #endif
	
    const int INFO_REQ_INTERVAL = 500; //ms 
    const int TX_MIN_WAIT_INTERVAL = 5000; //ms
    
    // Protocol objects
    MitsuProtocol ml = MitsuProtocol();
    MitsuProtocol::packetBuilder pb = MitsuProtocol::packetBuilder(&ml);
    
    // Private Methods
	void sendInit();
    void sendRequestInfo(MitsuProtocol::info_t kind);
    void storeRxSettings(MitsuProtocol::rxSettings_t settings);
    
    // Internal states
    MitsuProtocol::settings_t lastSettings = {MitsuProtocol::power_t::powerOff,false,
                                              MitsuProtocol::mode_t::modeFan,false,
                                              MitsuProtocol::fan_t::fan1,false,
                                              MitsuProtocol::vane_t::vane1,false,
                                              MitsuProtocol::wideVane_t::wideVaneCenter,false,
                                              0,false};
    MitsuProtocol::roomTemp_t lastRoomTemp = {0,false,0.0,false,0.0,false};
    MitsuProtocol::info_t lastInfo = MitsuProtocol::roomTemp;
    
    unsigned long lastSettingsTime = 0;
    unsigned long lastInfoRequestTime = 0;
    unsigned long lastRoomTempTime = 0;
    unsigned long lastTxInitTime = 0;

    // Serial object and methods
    void sendData(uint8_t* buf, int len);
    HardwareSerial * _HardSerial;
};
//endif
