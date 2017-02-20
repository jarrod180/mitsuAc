/*
  MitsuProtocol.cpp - Mitsubishi Air Conditioner/Heat Pump protocol library
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

/* DEBUG */
#define DEBUG 1
#if DEBUG
void MitsuProtocol::log (const char* msg){
	if (debugCb){
		debugCb (msg);
	}
}
#endif
/* END DEBUG */


MitsuProtocol::MitsuProtocol() {
}

void MitsuProtocol::setDebugCb(DEBUG_CB){
	this->debugCb = debugCb;
}

int MitsuProtocol::getTxSettingsPacket (settings_t settings, byte* buffer){
	#if (DEBUG > 0)
	log("MitsuProtocol.getTxSettingsPacket()");
	#endif	
	
	// Zeroize the packet
	for (int i = 0; i < DATA_PACKET_LEN; i++){
		buffer[i] = 0;
	}
	
	buffer[HEADER_1_POS]      = HEADER_1;
	buffer[MSG_TYPE_POS]      = static_cast<byte>(msgKind_t::txSettings);
	buffer[HEADER_3_POS]      = HEADER_3;
	buffer[HEADER_4_POS]      = HEADER_4;
	buffer[LENGTH_POS]        = DATA_PACKET_LEN - CHECKSUM_LEN - HEADER_LEN;
	buffer[DATA_KIND_POS]     = static_cast<byte>(info_t::settings);
	buffer[DATA_POWER_POS]    = settings.powerValid?static_cast<byte>(settings.power):0;
	buffer[DATA_MODE_POS]     = settings.powerValid?static_cast<byte>(settings.mode):0;	
	buffer[DATA_TEMP_POS]     = tempToByte(settings.tempDegC);
	buffer[DATA_FAN_POS]      = settings.powerValid?static_cast<byte>(settings.fan):0;
	buffer[DATA_VANE_POS]     = settings.powerValid?static_cast<byte>(settings.vane):0;
	buffer[DATA_WIDEVANE_POS] = settings.powerValid?static_cast<byte>(settings.wideVane):0;
	
	// Checksum
	buffer[DATA_CHECKSUM_POS] = MitsuProtocol::calculateChecksum(buffer, DATA_PACKET_LEN - 1);

	return DATA_PACKET_LEN;
}

int MitsuProtocol::getTxConnectPacket (byte* buffer){
	#if (DEBUG > 0)
	log("MitsuProtocol.getTxConnectPacket()");
	#endif	
	
	// Zeroize the packet
	for (int i = 0; i < CONNECT_PACKET_LEN; i++){
		buffer[i] = 0;
	}	
	
	buffer[HEADER_1_POS]  = HEADER_1;
	buffer[MSG_TYPE_POS]  = static_cast<byte>(msgKind_t::txConnect);
	buffer[HEADER_3_POS]  = HEADER_3;
	buffer[HEADER_4_POS]  = HEADER_4;
	buffer[LENGTH_POS]    = CONNECT_PACKET_LEN - CHECKSUM_LEN - HEADER_LEN;
	buffer[CONNECT_1_POS] = CONNECT_1;
	buffer[CONNECT_2_POS] = CONNECT_2;
	
	// Checksum
	buffer[CONNECT_CHECKSUM_POS] = MitsuProtocol::calculateChecksum(buffer, CONNECT_PACKET_LEN - 1);

	return CONNECT_PACKET_LEN;
}


int MitsuProtocol::getTxInfoPacket (byte* buffer, info_t kind){
	#if (DEBUG > 0)
	log("MitsuProtocol.getTxInfoPacket()");
	#endif	
	
	// Zeroize the packet
	for (int i = 0; i < INFO_PACKET_LEN; i++){
		buffer[i] = 0;
	}	
	
	buffer[HEADER_1_POS]  = HEADER_1;
	buffer[MSG_TYPE_POS]  = static_cast<byte>(msgKind_t::txInfoRequest);
	buffer[HEADER_3_POS]  = HEADER_3;
	buffer[HEADER_4_POS]  = HEADER_4;
    buffer[LENGTH_POS]    = INFO_PACKET_LEN - CHECKSUM_LEN - HEADER_LEN;	
	buffer[INFO_KIND]     = static_cast<byte>(kind);
	// Checksum
	buffer[INFO_CHECKSUM_POS] = MitsuProtocol::calculateChecksum(buffer, INFO_PACKET_LEN - 1);

	return INFO_PACKET_LEN;
}


byte MitsuProtocol::calculateChecksum(byte* data, int len) {
	byte sum = 0;
	for (int i = 0; i < len; i++) {
		sum += data[i];
	}
	return (0xfc - sum) & 0xff;
}

const char* MitsuProtocol::power_tToString (power_t power){
    switch (power){
	    case power_t::powerOn:return "on";
		case power_t::powerOff:return "off";
	}
	return "undefined";
}
void power_tFromString (const char* powerStr, const power_t power, bool success){
	success=true;
    switch (power){
	    case power_t::"on":power=powerOn;break;
		case power_t::"off":power=powerOff;break;
		default:power=powerOff;success=false;
	}
{

const char* MitsuProtocol::mode_tToString (mode_t mode){
    switch (mode){
	    case mode_t::modeHeat:return "heat";
		case mode_t::modeDry:return "dry";
		case mode_t::modeFan:return "fan";
		case mode_t::modeCool:return "cool";
		case mode_t::modeAuto:return "auto";
	}
	return "undefined";
}
const mode_t MitsuProtocol::mode_tFromString (const char* mode, bool success){
    switch (mode){
	    case : "heat" return mode_t::modeHeat;
		case : "dry" return mode_t::modeDry;
		case : "fan" return mode_t::modeFan;
		case : "cool" return mode_t::modeCool;
		case : "auto" return mode_t::modeAuto;
	}
	return mode_t::modeFan;
}

const char* MitsuProtocol::fan_tToString (fan_t fan){
    switch (fan){
	    case fan_t::fanAuto:return "auto";
		case fan_t::fanQuiet:return "quiet";
		case fan_t::fan1:return "1";
		case fan_t::fan2:return "2";
		case fan_t::fan3:return "3";
		case fan_t::fan4:return "4";
	}
	return "undefined";
}
const fan_t MitsuProtocol::fan_tFromString (const char* fan){
    switch (fan){
	    case "auto": return fan_t::fanAuto;
		case "quiet": return fan_t::fanQuiet;
		case "1": return fan_t::fan1;
		case "2": return fan_t::fan2;
		case "3": return  fan_t::fan3;
		case "4": return fan_t::fan4;
	}
	return fan_t::fan1;
}

const char* MitsuProtocol::vane_tToString (vane_t vane){
    switch (vane){
	    case vane_t::vaneAuto:return "auto";
		case vane_t::vane1:return "1";
		case vane_t::vane2:return "2";
		case vane_t::vane3:return "3";
		case vane_t::vane4:return "4";
		case vane_t::vane5:return "5";
		case vane_t::vaneSwing:return "swing";
	}
	return "undefined";
}

const vane_t vane_tFromString (const char* vane){
    switch (vane){
	    case "auto":return vane_t::vaneAuto;
		case "1":return vane_t::vane1;
		case "2":return vane_t::vane2;
		case "3":return vane_t::vane3;
		case "4":return vane_t::vane4;
		case "5":return vane_t::vane5;
		case "swing":return vane_t::vane3;
	}
	return vane_t::vane3;
}

const char* MitsuProtocol::wideVane_tToString (wideVane_t wideVane){
    switch (wideVane){
	    case wideVane_t::wideVaneFullLeft:return "full_left";
	    case wideVane_t::wideVaneHalfLeft:return "half_left";
	    case wideVane_t::wideVaneCenter:return "center";
	    case wideVane_t::wideVaneHalfRight:return "half_right";
	    case wideVane_t::wideVaneFullRight:return "full_right";
	    case wideVane_t::wideVaneLeftAndRight:return "left_and_right";
	    case wideVane_t::wideVaneSwing:return "swing";
	}
	return "undefined";
}
const wideVane_t wideVane_tFromString (const char* wideVane){
    switch (wideVane){
	    case "full_left": return wideVane_t::wideVaneFullLeft;
	    case "half_left": return wideVane_t::wideVaneHalfLeft;
	    case "center": return wideVane_t::wideVaneCenter;
	    case "half_right": return wideVane_t::wideVaneHalfRight;
	    case "full_right": return wideVane_t::wideVaneFullRight;
	    case "left_and_right": return wideVane_t::wideVaneLeftAndRight;
	    case "swing": return wideVane_t::wideVaneSwing;
	}
	return wideVane_t::wideVaneCenter;
}

/* packetBuilder */

MitsuProtocol::packetBuilder::packetBuilder(MitsuProtocol* parent) {
	this->parent = parent;
	cursor = 0;
	for (int i = 0; i < MAX_SIZE; i++){
		buffer[i] = 0;
	}
};	

int MitsuProtocol::packetBuilder::addByte(byte b){
	#if (DEBUG > 1)
	parent->log (String(String("Rx: 0x") + String(b,HEX)).c_str());
	#endif
	  
	// Too many bytes, reset
	if (cursor >= MAX_SIZE){
		#if (DEBUG > 0)
        parent->log("packetBuilder.addByte: cursor reset");
		#endif
		reset();
	}

	if ((cursor == 0 && b == HEADER_1) || cursor > 0){
		#if (DEBUG > 1)
        if (cursor==0){
			parent->log("packetBuilder.addByte: packet start");
		}
		#endif
		
		buffer[cursor] = b;
		cursor++;
		return 0; // OK, Byte accepted
	}else{
		return 1; // OK, but ignored - waiting for packet start
	}
}

bool MitsuProtocol::packetBuilder::complete(){
	int expectedLength = MitsuProtocol::HEADER_LEN + buffer[MitsuProtocol::LENGTH_POS] + MitsuProtocol::CHECKSUM_LEN;
	return (cursor >= expectedLength - 1);
}

bool MitsuProtocol::packetBuilder::valid(){
	if (!complete()){
		return false;
	}
	bool headerValid = (buffer[0] == MitsuProtocol::HEADER_1 &&
						buffer[2] == MitsuProtocol::HEADER_3 &&
						buffer[3] == MitsuProtocol::HEADER_4);
						
	int checksumPos = MitsuProtocol::HEADER_LEN + buffer[MitsuProtocol::LENGTH_POS];
	int numBytesInMsg = MitsuProtocol::HEADER_LEN + buffer[MitsuProtocol::LENGTH_POS];
	bool checksumValid = (buffer[checksumPos] == calculateChecksum (buffer,numBytesInMsg));

	return (headerValid && checksumValid);
}

MitsuProtocol::msg_t MitsuProtocol::packetBuilder::getData(){
	#if (DEBUG > 0)
	parent->log("packetBuilder.getData()");
	#endif
		
	MitsuProtocol::msg_t msg;
	
	msg.msgKindValid = false;
	
	if (!valid()) {
		#if (DEBUG > 0)
        parent->log("packetBuilder.getData: invalid");
		#endif
		return msg;
	} //TBD
	
	switch(static_cast<msgKind_t>(buffer[MSG_TYPE_POS])){
		case msgKind_t::txConnect:
		case msgKind_t::txSettings:
		case msgKind_t::txInfoRequest:
			break; // We don't care about these right now
			
		case msgKind_t::rxCurrentSettings:
			#if (DEBUG > 0)
			parent->log("packetBuilder.getData: rxCurrentSettings");
			parent->log(String(buffer[MSG_TYPE_POS],HEX).c_str());
			#endif			
			msg.kind = MitsuProtocol::msgKind_t::rxCurrentSettings;
			msg.msgKindValid = true;
			msg.data.rxCurrentSettingsData.kind = static_cast<info_t>(buffer[DATA_KIND_POS]);
			
			switch (msg.data.rxCurrentSettingsData.kind){
				case settings:
					msg.data.rxCurrentSettingsData.data.settings.power = static_cast<power_t>(buffer[DATA_POWER_POS]);
					msg.data.rxCurrentSettingsData.data.settings.mode = static_cast<mode_t>(buffer[DATA_MODE_POS]);
					msg.data.rxCurrentSettingsData.data.settings.tempDegC = byteToTemp(buffer[DATA_TEMP_POS]);	
					msg.data.rxCurrentSettingsData.data.settings.fan = static_cast<fan_t>(buffer[DATA_FAN_POS]);
					msg.data.rxCurrentSettingsData.data.settings.vane = static_cast<vane_t>(buffer[DATA_VANE_POS]);
					msg.data.rxCurrentSettingsData.data.settings.wideVane = static_cast<wideVane_t>(buffer[DATA_WIDEVANE_POS]);
					break;
				case roomTemp:
					msg.data.rxCurrentSettingsData.data.roomTemp.roomTemp = byteToRoomTemp(buffer[DATA_ROOM_TEMP_POS]);
					break;
				default:
					#if (DEBUG > 0)
					parent->log(String(String("packetBuilder.getData: unrecognised rx settings kind:") + String(buffer[MSG_TYPE_POS],HEX)).c_str());
					#endif	
					break;
			}
			break;
		case msgKind_t::rxStatusOk:
			#if (DEBUG > 0)
			parent->log("packetBuilder.getData: rxStatusOk");
			parent->log(String(buffer[MSG_TYPE_POS],HEX).c_str());
			#endif		
			msg.kind = MitsuProtocol::msgKind_t::rxStatusOk;
			msg.msgKindValid = true;
			break;
		case msgKind_t::rxStatusNok:
			#if (DEBUG > 0)
			parent->log("packetBuilder.getData: rxStatusNok");
			parent->log(String(buffer[MSG_TYPE_POS],HEX).c_str());
			#endif		
			msg.kind = MitsuProtocol::msgKind_t::rxStatusNok;
			msg.msgKindValid = true;
			break;
		default:
			#if (DEBUG > 0)
			parent->log("packetBuilder.getData: unrecognised message type:");
			parent->log(String(buffer[MSG_TYPE_POS],HEX).c_str());
			#endif		
			break; // Unrecognised message
	}
	return msg;
}

void MitsuProtocol::packetBuilder::reset(){
	#if (DEBUG > 2)
	parent->log("packetBuilder.getData: reset()");
	#endif
	cursor = 0;
	for (int i; i < MAX_SIZE; i++){
		buffer[i] = 0;
	}
}