/*
 * obd_project.cpp
 * Description: Main obd program
 *
 *  Created on: 16/11/2014
 *  Author: Matthias de Schepper & Kwinten Schram
 */

/* ----------------------------------------------------------------------------------------------------------------------
include files
 ----------------------------------------------------------------------------------------------------------------------*/
#include "OBD_project.h"
#include "Arduino.h"
#include "OBD_request.h"
#include "mcp_can.h"
#include <SPI.h>
#include "PID.h"
#include <SoftwareSerial.h>
#include <SIM900.h>
#include <TinyGPS++.h>	
#include "gps_gsm.h"
/* ----------------------------------------------------------------------------------------------------------------------
local definitions
 ----------------------------------------------------------------------------------------------------------------------*/
//Standard OBD requests and responses

#define ID_REQUEST              0x7DF
#define ID_RESPONSE				0x7E8

OBD OBD; //construct OBD
#define DEBUGMODE 1
/* ----------------------------------------------------------------------------------------------------------------------
private function prototypes
 ----------------------------------------------------------------------------------------------------------------------*/
uint8_t parseHex(char * line, uint8_t line_length, uint32_t * value);
/* ----------------------------------------------------------------------------------------------------------------------
public functions
 ----------------------------------------------------------------------------------------------------------------------*/

/*=======================================================================================================================
 Name: setup
 ------------------------------------------------------------------------------------------------------------------------
 Purpose: 	function that initializes all Arduino hardware
 Param:		n/a 
 Returns:	n/a
=======================================================================================================================*/


void setup()
{
	 Serial.begin(115200);
	 Serial.flush();
	 Serial.println("");

	 // Reset serial buffer
	 memset(serialCmd, 0, sizeof(serialCmd));
	 
	 // Default mode is COMMAND
	 Serial.println("OBD interface ready!");

	 //initialise CAN_BUS default on 500kBAD
	 OBD.init(CAN_500KBPS);
	 gsm_setup();
	 int gsm_enabled=1;
	 gps_setup();
	 int gps_enabled=1;
	 
	
}

/*=======================================================================================================================
 Name: loop
 ------------------------------------------------------------------------------------------------------------------------
 Purpose: 	function that calls all loop functions of all Arduino hardware
 Param:		n/a 
 Returns:	n/a
=======================================================================================================================*/


void loop()
{
	OBD.processing();
	//TODO Kwinten, voeg hier je GPS en GPS loop toe maar voorzie eerst enabale en disable state. Zie andere todo's verderop
	if (gps_enabled==1)
	{
	TinyGPSPlus getGPS_data();
	}
	
	if (gsm_enabled==1)
	{
	sendGSM_data("test");
	}
	
	// Read serial command
	if (Serial.available() > 0)
	{
		// Disable wireless reception interrupt
		//disableINT0irq();
		
		char ch = Serial.read();

		if (length >= SERIAL_BUF_LEN-1)
		{
			memset(serialCmd, 0, sizeof(serialCmd));
			length = 0;
		}
		else if (ch == 0x0D) //character = Carriage Return?
		{
			serialCmd[length] = 0;
			handleSerialCmd(serialCmd);
			memset(serialCmd, 0, sizeof(serialCmd));
			length = 0;
		}
		else
		{
			serialCmd[length] = ch; //add new character to command string
			length++;
		}
	}
}



/*=======================================================================================================================
 Name: processSerialCmd
 ------------------------------------------------------------------------------------------------------------------------
 Purpose: 	function that processes AT lik commands sent on the serial port
 Param:		n/a 
 Returns:	n/a
=======================================================================================================================*/
void handleSerialCmd(char* command)
{
	uint8_t length;
	
	switch (serialCmd[0]) //determine command based on first ASCII character
	{
		case SET_BAUD_RATE:
		{
			if (OBD.getObdProcessingtate() == STATE_CONFIG)
			{
				switch (serialCmd[1])
				{
					case BAUD_10KB	:	OBD.init(CAN_10KBPS); break;
					case BAUD_20KB	:	OBD.init(CAN_20KBPS); break;
					case BAUD_50KB	:	OBD.init(CAN_50KBPS); break;
					case BAUD_100KB	:	OBD.init(CAN_100KBPS); break;
					case BAUD_125KB	:	OBD.init(CAN_125KBPS); break;
					case BAUD_250KB	:	OBD.init(CAN_250KBPS); break;
					case BAUD_500KB	:	OBD.init(CAN_500KBPS); break;
					case BAUD_1MB	:	OBD.init(CAN_1000KBPS); break;
					default: Serial.println("Baudrate not supported");
				}
			}
		}
		break;
		case SET_CUSTOM_BIT_RATE:
			Serial.println("Custom bitrate not supported yet");
		break;
		
		case READ_MPC2515_REGISTER:
		{
			uint32_t address;
			if (parseHex(&serialCmd[1], 2, &address)) 
			{
				uint8_t read_value = OBD.canControllerReadRegister(address);
				Serial.println(read_value,HEX);
			}
		}
		break;
		
		case WRITE_MPC2515_REGISTER:
		{
			uint32_t address, data;
			if (parseHex(&serialCmd[1], 2, &address))
			{
				if (parseHex(&serialCmd[3], 2, &data))
				{
					OBD.canControllerWriteRegister(address, data);
					Serial.println("");
				}
			}
		}
		break;
		
		case GET_HARDWARE_VERSION:
		{
			Serial.print("V");
			Serial.print(VERSION_HARDWARE_MAJOR,HEX);
			Serial.println(VERSION_HARDWARE_MINOR,HEX);
		}
		break;
		
		case GET_FIRMWARE_VERSION:
		{
			Serial.print("v");
			Serial.print(VERSION_FIRMWARE_MAJOR,HEX);
			Serial.println(VERSION_FIRMWARE_MINOR,HEX);
		}
		break;
		
		case GET_SERIAL_NUMBER:
			Serial.println("Serial number not supported");
		break;
		
		case OPEN_CAN_CHANNEL:
		{
			if (OBD.getObdProcessingtate() == STATE_CONFIG)
			{
				void canControllerSetNormalOperatingMode(); 
				//clock_reset();
				OBD.setObdProcessingtate(STATE_OPEN);
				if(mainDebug==1) {
				Serial.print("0 = CAN channel opened");
				}
				Serial.println("");
			}
		}
		break;
		
		case OPEN_DEVICE_IN_LOOPBACK_MODE:
		{
			if (OBD.getObdProcessingtate() == STATE_CONFIG)
			{
				void canControllerSetLoopbackMode();
				OBD.setObdProcessingtate(STATE_OPEN);
				if(mainDebug==1) {
				Serial.print("l = CAN controller set in loopback mode");
				}
				Serial.println("");
			}
		}
		break;
		
		case OPEN_CAN_CHANNEL_IN_LISTEN_ONLY_MODE:
		{
			if (OBD.getObdProcessingtate() == STATE_CONFIG)
			{
				void canControllerSetListenOnlyMode();
				OBD.setObdProcessingtate(STATE_LISTEN);
				if(mainDebug==1) {
				Serial.print("L = CAN controller in Listen mode");
				}
				Serial.println("");
			}
		}
		break;
		
		case CLOSE_CAN_CHANNEL:
		{
			if (OBD.getObdProcessingtate() != STATE_CONFIG)
			{
				void canControllerSetConfigurationMode();
				OBD.setObdProcessingtate(STATE_CONFIG);
				if(mainDebug==1) {
				Serial.print("C = CAN channel  closed");
				}
				Serial.println("");
			}
		}
		break;
		
		case TRANSMIT_STANDARD_FRAME:
		case TRANSMIT_EXTENDED_FRAME:
		case TRANSMIT_STANDARD_RTR_FRAME:
		case TRANSMIT_EXTENDED_RTR_FRAME:
		{
			if (OBD.getObdProcessingtate() == STATE_OPEN)
			{
				Serial.println("RTR not supported");
			}
		}
		break;
		
		case READ_CAN_CONTROLLER_STATUS_ERROR:
		{
			uint8_t statusFlags = OBD.canControllerReadRegister(0x2d);
			uint8_t status = 0;
			if (statusFlags & 0x01) status |= 0x04; // error warning
			if (statusFlags & 0xC0) status |= 0x08; // data overrun
			if (statusFlags & 0x18) status |= 0x20; // passive error
			if (statusFlags & 0x20) status |= 0x80; // bus error
			Serial.print("F");
			Serial.println(status,HEX);
		}
		break;
		
		case SET_TIMESTAMPING:
			serialCmd[0] == '1' ? OBD.enableTimeStamping() : OBD.disableTimeStamping();
			if(mainDebug==1) {
			Serial.print("Z = timestamp set");
			}
			Serial.println("");
		break;
		
		case ENABLE_INTERFACE:
			switch (serialCmd[1])
			{
				case GPS_INTERFACE:
				gps_setup();
					//TODO Kwinten voeg hier de code toe om de gps te initialiseren en zijn loop code te activeren
                                        Serial.println("GPS enabled");
                                        gps_enabled=1;
				break;
				case GSM_INTERFACE:
				gsm_setup()
					//TODO Kwinten voeg hier de code toe om de gsm te initialiseren en zijn loop code te activeren
                                        Serial.println("GSM enabled");
                                        gsm_enabled=1;
				break;
				case CAN_INTERFACE:
					OBD.setObdProcessingtate(STATE_CONFIG);
                                        Serial.println("CAN enabled");
				break;
			}
		break;
		case DISABLE_INTERFACE:
			switch (serialCmd[1])
			{
				case GPS_INTERFACE:
				Serial2.end();
				gps_enabled=0;
				      //TODO Kwinten voeg hier de code toe om de gps en zijn loop code te deactiveren
                                       Serial.println("GPS disabled");
				break;
				case GSM_INTERFACE:
				//gsm.end(); //ben nog niet zeker of dit werkt
				gsm_enabled=0;
					//TODO Kwinten voeg hier de code toe om de gsm  en zijn loop code te deactiveren
                                        Serial.println("GSM disabled");
				break;
				case CAN_INTERFACE:
					OBD.setObdProcessingtate(STATE_DISABLED);
                                        Serial.println("CAN disabled");
				break;
			}
		break;
		
		case SET_ACCEPTANCE_MASK:
			uint32_t mask;
			if (parseHex(&serialCmd[1], 8, &mask))
			{
				OBD.setStandardMasks(&mask, 1);
				if(mainDebug==1) {
				Serial.print("m = set Mask: ");Serial.print(mask, HEX);
				}
				Serial.println("");
			}
		break;
			
		case SET_ACCEPTANCE_FILTER:
		uint32_t filter;
		if (parseHex(&serialCmd[1], 8, &filter)) 
		{
			OBD.setStandardFilter(&filter, 1);
			if(mainDebug==1) {
			Serial.print("M = set Filter: "); Serial.print(filter, HEX);
			}
			Serial.println("");
		}
		break;
		case SCAN_STANDARD_BUS_IDS:
			OBD.setObdProcessingtate(STATE_SCANNING);
			if(mainDebug==1) {
				Serial.print("Start Scanning bus for Standard ID's");
			}
			Serial.println("");
		break;
		
		case SCAN_EXTENDED_BUS_IDS:
			Serial.println("Modules not supported yet");
		break;

		case QUERY_PID_VALUE:
			uint32_t pid;
			if (parseHex(&serialCmd[1], 8 , &pid))
			{
				OBD.setQueryPid(pid);
				OBD.setObdProcessingtate(STATE_QUERYING);
			}
			if(mainDebug==1) {
				Serial.print("Start query for "); Serial.print(pid, HEX);
			}
			Serial.println("");
		break;

		case DISABLE_DEBUG_OUTPUT:
			mainDebug=0;
			OBD.obdDebug=0;
            Serial.println("Debugging disabled");
		break;

		case ENABLE_DEBUG_OUTPUT:
			mainDebug=1;
			OBD.obdDebug=1;
            Serial.println("Debugging enabled");
		break;
		
		
	}
}



/*=======================================================================================================================
 Name: parseHex
 ------------------------------------------------------------------------------------------------------------------------
 Purpose: 	Parse hex value of given string
 Param:		line Input string,
			length Count of characters to interpret, not more then 8 characters supported (result= 4 bytes)
			value Pointer to variable for the resulting decoded value
 Returns:	0 on error, 1 on success
=======================================================================================================================*/
uint8_t parseHex(char * line, uint8_t line_length, uint32_t * value) 
{
    *value = 0;
	if (line_length > 8) // not more then 8 characters allowed
	{
		 return 0;
	}
    while (line_length--) 
	{
        if (*line == 0) return 0;
        *value <<= 4;
        if ((*line >= '0') && (*line <= '9')) 
		{
           *value += *line - '0';
        } 
		else if ((*line >= 'A') && (*line <= 'F')) 
		{
           *value += *line - 'A' + 10;
        } 
		else if ((*line >= 'a') && (*line <= 'f')) 
		{
           *value += *line - 'a' + 10;
        } 
		else return 0;
        line++;
    }
    return 1;
}
