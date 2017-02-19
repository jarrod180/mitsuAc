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

#ifdef ESP8266
#include <functional>
#define DEBUG_CB std::function<void(const char* msg)> debugCb
#else
#define DEBUG_CB void (*debugCb)();
#endif

#define DEBUG 1

class mitsiLib
{
public:
	#if DEBUG
    void setDebugCb(DEBUG_CB);
	#endif
	
    enum class power_t : byte
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

    struct settings_t {
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

    struct roomTemp_t {
       int roomTemp;
       bool roomTempValid;
    };	
	
	enum class dataKind_t {
		settingsRequest = 0x01,
		currentSettings = 0x02,
		currentRoomTemp = 0x03
	};

	enum info_t {
		settings = 0x02,
		roomTemp = 0x03
	};

	struct rxSettings_t
    {
        info_t kind;
        union
        {
          settings_t settings;
          roomTemp_t roomTemp;
        } data;
    };	
	
	struct status_t 
	{
	};
	
	struct connect_t 
	{
	};
	
    enum class msgKind_t {
       txConnect         = 0x5a,
	   txSettings        = 0x41,
       txInfoRequest     = 0x42,
	   rxCurrentSettings = 0x62,
	   rxStatusOk        = 0x61,
	   rxStatusNok       = 0x7a // confirm this is correct!
    };

    struct msg_t
    {
        msgKind_t kind;
        bool msgKindValid;
        union
        {
          rxSettings_t rxCurrentSettingsData;
		  settings_t txSettingsData;
		  info_t txInfoRequestData;
		  connect_t txConnectData;
		  status_t rxStatusOkData;
		  status_t rxStatusNokData;
        } data;
    };
	
	    
	// Constructor
    mitsiLib();
	
	/* 
	packetBuilder Class -
	Add one byte at a time and discover when a valid
	packet is detected and then get the data.
	*/
	class packetBuilder
	{
		friend class mitsiLib;
		
		public:
			packetBuilder(mitsiLib* parent);
		    int addByte(byte b);
			bool complete();
			bool valid();
			mitsiLib::msg_t getData();
			void reset();
			
		private:
			mitsiLib* parent;
			static const int MAX_SIZE=32;
			byte buffer[MAX_SIZE];
			int cursor;
	};	
	
	// Return different kinds of tx packet
    int getTxSettingsPacket (settings_t settings, byte* buffer);
	int getTxConnectPacket (byte* buffer);
	int getTxInfoPacket (byte* buffer, info_t kind);

private:
	#if DEBUG
	DEBUG_CB;
	void log (const char* msg);
	#endif
	
	/* 
	The temperatures byte values are a constant offset from the
	degrees Celcius value. These routines check the range and
	convert from encoded byte to degrees Celcius.
	*/
	static inline byte roomTempToByte(int roomTemp) {
		return (roomTemp >= 10 && roomTemp <= 41)?byte(roomTemp - 10):0;
	}
	static inline int byteToRoomTemp(byte b) {
		return (b >= 0x00 && b <= 0x1f)?int(b + 10):0;
	}
	static inline byte tempToByte(int temp) {
		return (temp >= 31 && temp <= 16)?byte(temp - 31):0;
	}
	static inline int byteToTemp(byte b) {
		return (b >= 0x00 && b <= 0x0f)?int(b + 31):0;
	}
	
	// Calculate the checksum for given bytes.
	static byte calculateChecksum(byte* data, int len);
	
    /* Constants */
	
	// All Packets
	static const int HEADER_LEN   = 5;
	static const int CHECKSUM_LEN = 1;
	
	static const int HEADER_1_POS = 0;
	static const int MSG_TYPE_POS = 1;
	static const int HEADER_3_POS = 2;
	static const int HEADER_4_POS = 3;
	static const int LENGTH_POS   = 4;
	
	// Data Packet
	static const int DATA_PACKET_LEN    = 21;
	static const int DATA_KIND_POS      = 5;
	static const int DATA_6             = 6;
	static const int DATA_7             = 7;
    static const int DATA_POWER_POS     = 8;
	static const int DATA_ROOM_TEMP_POS = 8;
    static const int DATA_MODE_POS      = 9;
    static const int DATA_TEMP_POS      = 10;
    static const int DATA_FAN_POS       = 11;
    static const int DATA_VANE_POS      = 12;
	static const int DATA_13            = 13;
	static const int DATA_14            = 14;
    static const int DATA_WIDEVANE_POS  = 15;
	static const int DATA_16            = 16;
	static const int DATA_17            = 17;
	static const int DATA_18            = 18;
	static const int DATA_19            = 19;
    static const int DATA_CHECKSUM_POS  = 20;
	
	// Connect Packet 
	static const int CONNECT_PACKET_LEN   = 8;
    static const int CONNECT_1_POS        = 5;
    static const int CONNECT_2_POS        = 6;
    static const int CONNECT_CHECKSUM_POS = 7;
   
    // Info Packet
	static const int INFO_PACKET_LEN   = 21;
    static const int INFO_KIND         = 5;
    static const int INFO_CHECKSUM_POS = 20;
	
	
	// Data values
	static const int HEADER_1 = 0xfc;
	static const int HEADER_3 = 0x01;
	static const int HEADER_4 = 0x30;
	
	static const int CONNECT_1 = 0xca; // seems to be constant
	static const int CONNECT_2 = 0x01; // seems to be constant
	
};