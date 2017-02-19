/*
  mitsiLib.h - Mitsubishi Heat Pump protocol library
  Copyright (c) 2017 Jarrod Lamb.  All right reserved.

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
#ifndef __HeatPump_H__
#define __HeatPump_H__
#include <stdint.h>
#include <WString.h>
#include <math.h>
#include <HardwareSerial.h>
#if defined(ARDUINO) && ARDUINO >= 100
#else
#include "WProgram.h"
#endif
#include "mitsiLib.h"

/*
 * Callback function definitions. Code differs for the ESP8266 platform, which requires the functional library.
 * Based on callback implementation in the Arduino Client for MQTT library (https://github.com/knolleary/pubsubclient)
 */
#ifdef ESP8266
#include <functional>
#define RX_SETTINGS_CB std::function<void(mitsiLib::rxSettings_t* msg)> rxSettingsCb
#else
#define RX_SETTINGS_CB void (*rxSettingsCb)();


#endif


#ifdef ESP8266
#include <functional>
#define DEBUG_CB std::function<void(const char* msg)> debugCb
#else
#define DEBUG_CB void (*debugCb)();
#endif

#define DEBUG 1

class HeatPump
{
  public:
    // Constructor
    HeatPump();
	
	#if DEBUG
	void setDebugCb(DEBUG_CB);
	#endif

    // Library callback methods
    void setRxSettingsCb(RX_SETTINGS_CB);
   
    // Control methods
    void connect(HardwareSerial *serial);
	void requestInfo(mitsiLib::info_t kind);
    void monitor();


  private:
	#if DEBUG
	void log (const char* msg);
    DEBUG_CB;
	#endif
	
    mitsiLib ml = mitsiLib();
	mitsiLib::packetBuilder pb = mitsiLib::packetBuilder(&ml);
	
    // Serial constants
    const static int PACKET_DELAY_TIME_MS = 1000;
    const static int BEGIN_DELAY_TIME = 2000;
    const static int CONN_DELAY_TIME = 1100;
	const static int CONNECT_TX_COUNT = 2;

    // Serial
    HardwareSerial * _HardSerial;

    // Callbacks
	RX_SETTINGS_CB;
};
#endif