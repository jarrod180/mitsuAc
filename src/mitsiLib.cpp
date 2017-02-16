/*
  mitsiLib.cpp - Mitsubishi Heat Pump protocol library
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
#include <mitsiLib.h>

	mitsiLib::rxPacket::rxPacket() {
		cursor = 0;
		for (int i = 0; i < MAX_SIZE; i++){
			buffer[i] = 0;
		}
	};	

	int mitsiLib::rxPacket::addByte(byte b){
		// Too many bytes, reset
		if (cursor >= MAX_SIZE){
			reset();
		}

		if ((cursor == 0 && b == PACKET_START_VALUE) || cursor > 0){
			buffer[cursor] = b;
			cursor++;
			return 0; // OK, Byte accepted
		}else{
			return 1; // OK, but ignored - waiting for packet start
		}
	}

	bool mitsiLib::rxPacket::complete(){
		int expectedLength = RX_HEADER_LEN + buffer[RX_DATA_LEN_POS] + RX_CHECKSUM_LEN;
		return (cursor >= expectedLength - 1);
	}

	bool mitsiLib::rxPacket::valid(){
		if (!complete()){
			return false;
		}

		bool headerValid = (buffer[0] == RX_HEADER_0 &&
							buffer[2] == RX_HEADER_2 &&
							buffer[3] == RX_HEADER_3);
		bool checksumValid = calculateChecksum (buffer,mitsiLib::RX_HEADER_LEN + buffer[mitsiLib::RX_DATA_LEN_POS]);

		return (headerValid && checksumValid);
	}

	mitsiLib::msgData_t mitsiLib::rxPacket::getData(){
		mitsiLib::msgData_t msgData;
		
		if (!valid()) {return msgData;} //TBD


		if(buffer[RX_KIND_POS] == RX_DATA_PACKET &&
			buffer[RX_TYPE_POS] == RX_SETTINGS)  // Settings message
		{
			msgData.kind = mitsiLib::msgKind_t::settings;
			msgData.data.settings.power = static_cast<power_t>(buffer[RX_SET_POWER_POS]);
			msgData.data.settings.mode = static_cast<mode_t>(buffer[RX_SET_MODE_POS]);
			msgData.data.settings.fan = static_cast<fan_t>(buffer[RX_SET_FAN_POS]);
			msgData.data.settings.vane = static_cast<vane_t>(buffer[RX_SET_VANE_POS]);
			msgData.data.settings.wideVane = static_cast<wideVane_t>(buffer[RX_SET_WIDEVANE_POS]);
			msgData.data.settings.tempDegC = byteToTemp(buffer[RX_SET_TEMP_POS]);
		}
		else if(buffer[RX_KIND_POS] == RX_DATA_PACKET &&
			buffer[RX_TYPE_POS] == RX_ROOMTEMP) // Room Temp message
		{
			msgData.kind = mitsiLib::msgKind_t::roomTemp;
			msgData.data.roomTemp.roomTemp = byteToRoomTemp(buffer[RX_RT_ROOM_TEMP_POS]);
		}
		else if(buffer[RX_KIND_POS] == RX_OK) // RX OK message
		{
			msgData.kind =  mitsiLib::msgKind_t::status;
			// TBD - is there a payload for this message?
		}
		
		reset();
		
		return msgData;

	}

	void mitsiLib::rxPacket::reset(){
		cursor = 0;
		for (int i; i < MAX_SIZE; i++){
			buffer[i] = 0;
		}
	}
	
	static const int MAX_SIZE=32;
	byte buffer[MAX_SIZE];
	int cursor;
	
	


mitsiLib::mitsiLib() {
	
}

void mitsiLib::reset() {
	thePacket.reset();
}

void mitsiLib::feed (byte b){
	thePacket.addByte(b);
}

bool mitsiLib::isDataAvailable(){
	return (thePacket.complete() && thePacket.valid());
}

mitsiLib::msgData_t mitsiLib::getData(){
	return thePacket.getData();
}

int mitsiLib::encodeControlPacket (mitsiSettings_t settings, byte* buffer){
	
	// Init the packet
	for (int i = 0; i < CTRL_PACKET_LEN; i++){
		buffer[i] = 0;
	}

	// Populate header data
	for (int i = 0; i < CTRL_PACKET_LEN; i++) {
		buffer[i] = CTRL_HEADER[i];
	}

	// Populate packet data
	buffer[CTRL_POWER_POS] = settings.powerValid?static_cast<byte>(settings.power):0;
	buffer[CTRL_MODE_POS] = settings.powerValid?static_cast<byte>(settings.mode):0;		
	buffer[CTRL_FAN_POS] = settings.powerValid?static_cast<byte>(settings.fan):0;
	buffer[CTRL_VANE_POS] = settings.powerValid?static_cast<byte>(settings.vane):0;
	buffer[CTRL_WIDEVANE_POS] = settings.powerValid?static_cast<byte>(settings.wideVane):0;
	buffer[CTRL_TEMP_POS] = tempToByte(settings.tempDegC);

	// Checksum
	buffer[CTRL_CHECKSUM_POS] = mitsiLib::calculateChecksum(buffer, CTRL_PACKET_LEN - 1);
	
	// Only one size at this time
	return CTRL_PACKET_LEN;
}

int mitsiLib::encodeConnectPacket (byte* buffer){
	// Populate connect data
	for (int i = 0; i < CONNECT_PACKET_LEN; i++) {
		buffer[i] = CONNECT[i];
	}

	// Only one size at this time
	return CONNECT_PACKET_LEN;
}


byte mitsiLib::calculateChecksum(byte* data, int len) {
	byte sum = 0;
	for (int i = 0; i < len; i++) {
		sum += data[i];
	}
	return (0xfc - sum) & 0xff;
}