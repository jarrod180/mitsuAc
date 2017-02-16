/*
  mitsiLib.h - Mitsubishi Heat Pump protocol library
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
#include "Arduino.h"

class mitsiLib
{
public:
	
    enum power_t : byte
    {
        powerOff = 0x00,
        powerOn  = 0x01
    };
    enum class mode_t : byte
    {
        modeHeat = 0x01,
        modeDry  = 0x02,
        modeFan  = 0x03,
        modeCool = 0x07,
        modeAuto = 0x08
    };
    enum class fan_t : byte 
	{
        fanAuto  = 0x00,
        fanQuiet = 0x01,
        fan1     = 0x02,
        fan2     = 0x03,
        fan3     = 0x05,
        fan4     = 0x06
    };
    enum class vane_t : byte
	{
        vaneAuto  = 0x00,
        vane1     = 0x01,
        vane2     = 0x02,
        vane3     = 0x03,
        vane4     = 0x04,
        vane5     = 0x05,
        vaneSwing = 0x07
    };
    enum class wideVane_t : byte 
	{
        wideVaneHardLeft     = 0x01,
        wideVaneLeft         = 0x02,
        wideVaneMiddle       = 0x03,
        wideVaneRight        = 0x04,
        wideVaneHardRight    = 0x05,
        wideVaneLeftAndRight = 0x08,
        wideVaneSwing        = 0x0c
    };

    struct mitsiSettings_t {
       power_t power;
       bool powerValid;
       mode_t mode;
       bool modeValid;
       fan_t fan;
       bool fanValid;
       vane_t vane;
       bool vaneValid;
       wideVane_t wideVane;
       bool wideVaneValid;
	   int tempDegC;
	   bool tempDegCValid;
    };

    struct mitsiRoomTemp_t {
       int roomTemp;
       bool roomTempValid;
    };

    enum class msgKind_t {
      settings, roomTemp, status
    };

    struct msgData_t
    {
        msgKind_t kind;
        bool msgKindValid;
        union
        {
          mitsiSettings_t settings;
          mitsiRoomTemp_t roomTemp;
        } data;
    };
    
	// Constructor
    mitsiLib();

	// Reset the protocol library
    void reset();
	
	// Feed a byte to the protocol
    void feed (byte b);
	
	// Check if a message has been decoded yet
    bool isDataAvailable();
	
	// Get the decoded data
    msgData_t getData();
	
	// Encode a TX message to the supplied buffer
    int encodeControlPacket (mitsiSettings_t settings, byte* buffer);
	int encodeConnectPacket (byte* buffer);

private:
	
	/* 
	The temperatures byte values are a constant offset from the
	degrees Celcius value. If this is ever proven to not work	
	in all cases, the below routines will need to be modified.
	*/
	// Room Temperature
	static inline byte roomTempToByte(int roomTemp) {
		return (roomTemp >= 10 && roomTemp <= 41)?byte(roomTemp - 10):0;
	}
	static inline int byteToRoomTemp(byte b) {
		return (b >= 0x00 && b <= 0x1f)?int(b + 10):0;
	}
	// Control Temperature
	static inline byte tempToByte(int temp) {
		return (temp >= 31 && temp <= 16)?byte(temp - 31):0;
	}
	static inline int byteToTemp(byte b) {
		return (b >= 0x00 && b <= 0x0f)?int(b + 31):0;
	}
	
	static byte calculateChecksum(byte* data, int len);

	
    class rxPacket
	{
		friend class mitsiLib;
		public:

			rxPacket();	

			int addByte(byte b);

			bool complete();

			bool valid();

			mitsiLib::msgData_t getData();

			void reset();
			
		private:
			static const int MAX_SIZE=32;
			byte buffer[MAX_SIZE];
			int cursor;
	};
    
	rxPacket thePacket = rxPacket();
	
    /* Constants */
    static const byte PACKET_START_VALUE = 0xfc;

    /* RX packet data */
    static const int RX_HEADER_LEN = 5;
    static const int RX_CHECKSUM_LEN = 1;

    static const int RX_HEADER_START_POS = 0;
    static const int RX_KIND_POS = 1;
    static const int RX_HEADER_END_POS = 4;
    static const int RX_DATA_LEN_POS = 4;
    static const int RX_DATA_START_POS = 5;
    static const int RX_TYPE_POS = 5;

    static const int RX_OK = 0x61;
    static const int RX_DATA_PACKET = 0x62;

    static const int RX_SETTINGS = 0x02;
    static const int RX_ROOMTEMP = 0x03;

    static const byte RX_HEADER_0 = 0xfc;
    static const byte RX_HEADER_2 = 0x01;
    static const byte RX_HEADER_3 = 0x30;
	
	static const int RX_RT_ROOM_TEMP_POS = 8;
	
    static const int RX_SET_POWER_POS = 8;
    static const int RX_SET_MODE_POS  = 9;
    static const int RX_SET_TEMP_POS  = 10;
    static const int RX_SET_FAN_POS   = 11;
    static const int RX_SET_VANE_POS  = 12;
    // 13 .. 14 not used ?
    static const int RX_SET_WIDEVANE_POS = 15;
    // 16 .. 20 not used ?
    static const int RX_SET_CHECKSUM_POS = 21;

    /*
    Connect Packet
    */
    static const int CONNECT_PACKET_LEN = 8;
    const byte CONNECT[CONNECT_PACKET_LEN] = {0xfc, 0x5a, 0x01, 0x30, 0x02, 0xca, 0x01, 0xa8};

    /*
    Infomode Packet
    */
    static const int INFO_PACKET_LEN = 21;
    static const int INFO_HEADER_LEN = 5;

    static const int HEADER_START = 0;
    static const int HEADER_END = 4;
    static const int INFO_POS = 5;
    // 6 .. 20 unused?
    static const int INFO_CHECKSUM_POS = 21;

	static const int INFOMODE_LEN = 2;
    const byte INFO_HEADER[INFO_HEADER_LEN]  = {0xfc, 0x42, 0x01, 0x30, 0x10};
    const byte INFOMODE[INFOMODE_LEN] = {0x02, 0x03};

    /*
    Control Packet
    */
    static const int CTRL_PACKET_LEN   = 21;
    static const int CTRL_HEADER_LEN  = 8;

    static const int CTRL_HEADER_START = 0;
    static const int CTRL_HEADER_END   = 7;
    static const int CTRL_POWER_POS    = 8;
    static const int CTRL_MODE_POS     = 9;
    static const int CTRL_TEMP_POS     = 10;
    static const int CTRL_FAN_POS      = 11;
    static const int CTRL_VANE_POS     = 12;
    // 13 .. 14 not used ?
    static const int CTRL_WIDEVANE_POS = 15;
    // 16 .. 20 not used ?
    static const int CTRL_CHECKSUM_POS = 21;

    const byte CTRL_HEADER[CTRL_HEADER_LEN]   = {0xfc, 0x41, 0x01, 0x30, 0x10, 0x01, 0x9f, 0x00};

    // Value mapping
    static const int POWER_LEN = 2;
    static const int MODE_LEN = 5;
    static const int TEMP_LEN = 16;
    static const int FAN_LEN = 6;
    static const int VANE_LEN = 7;
    static const int WIDEVANE_LEN = 7;
    static const int ROOMTEMP_LEN = 32;


};



