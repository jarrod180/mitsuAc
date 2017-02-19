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

/* DEBUG */
#define DEBUG 1
#if DEBUG
void HeatPump::log (const char* msg){
	if (debugCb){
		debugCb (msg);
	}
}
#endif
/* END DEBUG */

HeatPump::HeatPump() {
}

void HeatPump::setDebugCb(DEBUG_CB){
	this->debugCb = debugCb;
	ml.setDebugCb(debugCb);
}

void HeatPump::requestInfo(mitsiLib::info_t kind){
    byte buf[32] = {0};
    int len = ml.getTxInfoPacket (buf, kind);
    for(int i = 0; i < len; i++) {
	  #if (DEBUG > 1)
	  log (String(String("Tx: 0x") + String(buf[i],HEX)).c_str());
	  #endif		
      _HardSerial->write((uint8_t)buf[i]);
    }
}

void HeatPump::connect(HardwareSerial *serial) {
  _HardSerial = serial;
  _HardSerial->begin(2400, SERIAL_8E1);
  delay(BEGIN_DELAY_TIME);
  
  byte buf[16] = {0};
  int len = ml.getTxConnectPacket (buf);

  for(int cnt = 0; cnt < CONNECT_TX_COUNT; cnt++) {
    for(int i = 0; i < len; i++) {
	  #if (DEBUG > 1)
	  log (String(String("Tx: 0x") + String(buf[i],HEX)).c_str());
	  #endif
      _HardSerial->write((uint8_t)buf[i]);
    }
    delay(CONN_DELAY_TIME);
  }
}

void HeatPump::monitor() {
  while (_HardSerial->available() > 0){
    pb.addByte(_HardSerial->read());
	if (pb.complete() && pb.valid()) {
		mitsiLib::msg_t msg = pb.getData();
		if (msg.msgKindValid){
			switch (msg.kind){
				case mitsiLib::msgKind_t::rxCurrentSettings:
					rxSettingsCb(&msg.data.rxCurrentSettingsData);
					break;
			}
		}
		pb.reset();
	}
  }
}

void HeatPump::setRxSettingsCb(RX_SETTINGS_CB){
  this->rxSettingsCb = rxSettingsCb;
}
