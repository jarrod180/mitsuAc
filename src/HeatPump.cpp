/*
  HeatPump.cpp - Mitsubishi Heat Pump control library for Arduino
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
#include <HeatPump.h>

HeatPump::HeatPump() {
}

void HeatPump::connect(HardwareSerial *serial) {
  _HardSerial = serial;
  _HardSerial->begin(2400, SERIAL_8E1);
  delay(BEGIN_DELAY_TIME);
  
  byte buf[16] = {0};
  int len = ml.encodeConnectPacket (buf);

  for(int cnt = 0; cnt < CONNECT_TX_COUNT; cnt++) {
    for(int i = 0; i < len; i++) {
      _HardSerial->write((uint8_t)buf[i]);
    }
    delay(CONN_DELAY_TIME);
  }
}

void HeatPump::monitor() {
  while (_HardSerial->available() > 0){
    ml.feed(_HardSerial->read());
	if (ml.isDataAvailable()) {
		mitsiLib::msgData_t data = ml.getData();
		switch (data.kind){
			case mitsiLib::msgKind_t::settings :
				packetReceivedCallback(0,0);
				break;
			case mitsiLib::msgKind_t::roomTemp:
				roomTempChangedCallback(data.data.roomTemp.roomTemp);
				break;
			case mitsiLib::msgKind_t::status:
				settingsChangedCallback();
				break;
		}
	}
  }
}

void HeatPump::setSettingsChangedCallback(SETTINGS_CHANGED_CALLBACK_SIGNATURE) {
  this->settingsChangedCallback = settingsChangedCallback;
}

void HeatPump::setPacketReceivedCallback(PACKET_RECEIVED_CALLBACK_SIGNATURE) {
  this->packetReceivedCallback = packetReceivedCallback;
}

void HeatPump::setRoomTempChangedCallback(ROOM_TEMP_CHANGED_CALLBACK_SIGNATURE) {
  this->roomTempChangedCallback = roomTempChangedCallback;

}



