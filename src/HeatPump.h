/*
  HeatPump.h - Mitsubishi Heat Pump control library for Arduino
  Copyright (c) 2017 Al Betschart.  All right reserved.

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
#include "Arduino.h"
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
#define SETTINGS_CHANGED_CALLBACK_SIGNATURE std::function<void()> settingsChangedCallback
#define PACKET_RECEIVED_CALLBACK_SIGNATURE std::function<void(byte* data, unsigned int length)> packetReceivedCallback
#define ROOM_TEMP_CHANGED_CALLBACK_SIGNATURE std::function<void(unsigned int newTemp)> roomTempChangedCallback
#else
#define SETTINGS_CHANGED_CALLBACK_SIGNATURE void (*settingsChangedCallback)();
#define PACKET_RECEIVED_CALLBACK_SIGNATURE void (*packetReceivedCallback)(byte* data, unsigned int length);
#define ROOM_TEMP_CHANGED_CALLBACK_SIGNATURE void (*roomTempChangedCallback)(unsigned int newTemp);
#endif

class HeatPump
{
  public:
    // Constructor
    HeatPump();

    // Library callback methods
    void setSettingsChangedCallback(SETTINGS_CHANGED_CALLBACK_SIGNATURE);
    void setPacketReceivedCallback(PACKET_RECEIVED_CALLBACK_SIGNATURE);
    void setRoomTempChangedCallback(ROOM_TEMP_CHANGED_CALLBACK_SIGNATURE);

    // Control methods
    void connect(HardwareSerial *serial);
    void monitor();


  private:
	mitsiLib ml;

    // Serial constants
    const static int PACKET_DELAY_TIME_MS = 1000;
    const static int BEGIN_DELAY_TIME = 2000;
    const static int CONN_DELAY_TIME = 1100;
	const static int CONNECT_TX_COUNT = 2;

    // Serial
    HardwareSerial * _HardSerial;

    // callbacks
    SETTINGS_CHANGED_CALLBACK_SIGNATURE;
    PACKET_RECEIVED_CALLBACK_SIGNATURE;
    ROOM_TEMP_CHANGED_CALLBACK_SIGNATURE;
};
#endif
