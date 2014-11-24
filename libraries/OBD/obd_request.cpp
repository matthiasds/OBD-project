/*
 * obd_request.ino
 * Description: Translates obd request of pid's in CAN messages and anlyses answers from the CAN bus
 *
 *  Created on: 16/11/2014
 *  Author: Matthias de Schepper
 */

/* ----------------------------------------------------------------------------------------------------------------------
include files
 ----------------------------------------------------------------------------------------------------------------------*/
#include "mcp_can.h"
#include "obd_request.h"
#include "PID.h"
#include "Arduino.h"
/* ----------------------------------------------------------------------------------------------------------------------
local definitions
 ----------------------------------------------------------------------------------------------------------------------*/
//Standard OBD requests and responses
#define DEBUGMODE 1

#define ID_REQUEST              0x7DF
#define ID_RESPONSE	            0x7E8

MCP_CAN CAN(53);                                            // Set CS to pin 53
volatile uint8_t messageReceived = 0;

/* ----------------------------------------------------------------------------------------------------------------------
private function prototypes
 ----------------------------------------------------------------------------------------------------------------------*/
static void mcp_can_ISR();
/* ----------------------------------------------------------------------------------------------------------------------
public functions
 ----------------------------------------------------------------------------------------------------------------------*/

/*=======================================================================================================================
 Name: init
 ------------------------------------------------------------------------------------------------------------------------
 Purpose: 	function to initialize the OBD communication
 Param:		baudrate	CAN BUS baudrate
 Returns:	n/a
=======================================================================================================================*/

void OBD::init(uint8_t baudrate)
{
	this->baudrate=baudrate;
	uint8_t canInitState = 1;
	while (canInitState != CAN_OK)
	{
		canInitState= CAN.begin(baudrate);
		if(canInitState == CAN_OK)                   // init can bus : baudrate = 500k
		{
			Serial.println("CAN BUS init ok!");
		}
		else
		{
			Serial.println("CAN BUS init fail");
			Serial.println("We will try again");
			delay(100);
		}
	}
    
    /*
     * set mask, set both the mask to 0x7ff
     * this 0x7FF means that all bits of the filter are used
     */
    CAN.init_Mask(0, 0, 0x7FF);                         // there are 2 mask in mcp2515, you need to set both of them
    CAN.init_Mask(1, 0, 0x7FF);
    
    
    /*
     * set filter, we can receive id from 0x04 ~ 0x09
     */
    CAN.init_Filt(0, 0, 0x00);                          // there are 6 filter in mcp2515
    CAN.init_Filt(1, 0, 0x00);                          // there are 6 filter in mcp2515
    
    CAN.init_Filt(2, 0, 0x00);                          // there are 6 filter in mcp2515
    CAN.init_Filt(3, 0, 0x00);                          // there are 6 filter in mcp2515
    CAN.init_Filt(4, 0, 0x00);                          // there are 6 filter in mcp2515
    CAN.init_Filt(5, 0, 0x00);                          // there are 6 filter in mcp2515*/
    
    attachInterrupt(2, mcp_can_ISR, FALLING); // start interrupt
    getReceivedMessage(&receiveMessage);      // read data,  len: data length, buf: data buf
    messageReceived = 0;
    
    //do one dummy read to clear potential interrupts
    getReceivedMessage(&receiveMessage);    
    messageReceived=0;
}


/*=======================================================================================================================
 Name: processing
 ------------------------------------------------------------------------------------------------------------------------
 Purpose: 	function to call from the main loop to process new data if an interrupt happened.
 Param:		n/a 
 Returns:	n/a
=======================================================================================================================*/
void OBD::processing() 
{
	switch (processingState) 
	{
		case STATE_CONFIG:
		case STATE_DISABLED:
		break;
		
		case STATE_OPEN:
		case STATE_LISTEN:
		if(messageReceived)						// check if get data
		{
			char type ='t'; // t = no rtr frame, r = rtr frame
			uint8_t idLength;
			uint16_t timestamp = (uint16_t) millis() & 0xFFFF;
			messageReceived = 0;  // clear flag
			getReceivedMessage(&receiveMessage);
			if (receiveMessage.id > 0x7FF) //extended frame?
			{
				type -= 'a' - 'A';
			}
	
			Serial.print(type);
			Serial.print(receiveMessage.id,HEX);
			Serial.print(receiveMessage.length,HEX);
			
			if (!receiveMessage.rtr) {
				for (uint8_t i = 0; i < receiveMessage.length; i++) {
					Serial.print(receiveMessage.data[i], HEX);
				}
			}

			if (timestamping) {
				Serial.print(timestamp, 4);
			}

			Serial.println("");

			
			if(obdDebug==1) {
			printMessage(&receiveMessage);
			}
		}
		break;
		
		case STATE_SCANNING:
		{
			if(obdDebug==1) {
			Serial.println("Scan started for standard ID's from 0x000 to 0x7FF");
			}
			uint32_t currentTime, previousTime;
			
			for (uint16_t currentId=0x00; currentId <= 0x7FF; currentId++)
			{
				CAN.init_Filt(0, 0, currentId);                          // there are 6 filter in mcp2515
				if(obdDebug==1) {
					Serial.print("Trying ID: "); Serial.println(currentId, HEX);
				}
				previousTime = currentTime;
				while(currentTime-previousTime<300) //test each id for 1 second
				{
					
					if(messageReceived)                  // check if get data
					{
						getReceivedMessage(&receiveMessage);
						messageReceived = 0;                // clear flag
						currentTime=previousTime+300; //force break
						if(obdDebug==1) {
						Serial.println("\r\n------------------------------------------------------------------");
						Serial.print("Get Data From id: ");
						Serial.print("0x");
						} else {
						Serial.print("i");
						}
						Serial.println(receiveMessage.id, HEX);
						
						if(obdDebug==1) {
						for(int i = 0; i<receiveMessage.length; i++)    // print the data
						{
							Serial.print("0x");
							Serial.print(receiveMessage.data[i], HEX);
							Serial.print("\t");
						}
						Serial.println();
						}
					}
					else
					{
						currentTime=millis();
					}
					
				}
			}
			init(baudrate); //done initialize again to reset filters
			processingState=STATE_LISTEN;
		}
		break;
		
		case STATE_QUERYING:
		{
			static uint32_t currentTime, previousTime;
			currentTime=millis();
			if(currentTime-previousTime>1000) //do the request 1's per second
			{
				previousTime=currentTime;
				obdRequest(pidToQuery);
			}
			if (receiveMessage.id==ID_RESPONSE) { //check for response all the time
				uint16_t data =  (uint16_t(receiveMessage.data[3]*256) + receiveMessage.data[4])/4;

				if(obdDebug==1) {
				printMessage(&receiveMessage);
				} else {
				Serial.print("Q");
				Serial.print(data, HEX);
				Serial.println("");
				}
				
			}
		}
		break;
	}
}

/*=======================================================================================================================
 Name: canControllerRead_Register
 ------------------------------------------------------------------------------------------------------------------------
 Purpose: 	function to read a specified register from the CAN controller itself
 Param:		address	address of register to read
 Returns:	n/a
=======================================================================================================================*/
uint8_t OBD::canControllerReadRegister(uint8_t address) 
{
	return CAN.mcp2515_readRegister(address);
}

/*=======================================================================================================================
 Name: canControllerWriteRegister
 ------------------------------------------------------------------------------------------------------------------------
 Purpose: 	function to write a specified register from the CAN controller itself
 Param:		address	address of register to write
			data	data to write to the register
 Returns:	n/a
=======================================================================================================================*/
void OBD::canControllerWriteRegister(uint8_t address, uint8_t data)
{
	CAN.mcp2515_setRegister(address, data);
}

/*=======================================================================================================================
 Name: canControllerSetNormalOperatingMode
 ------------------------------------------------------------------------------------------------------------------------
 Purpose: 	function to place the CAN controller in the normal mode
 Param:		n/a
 Returns:	n/a
=======================================================================================================================*/
void OBD::canControllerSetNormalOperatingMode()
{
	CAN.mcp2515_modifyRegister(MCP_CANCTRL, 0xE0, 0x00);
}

/*=======================================================================================================================
 Name: canControllerSetLoopbackMode
 ------------------------------------------------------------------------------------------------------------------------
 Purpose: 	function to place the CAN controller in loopback mode
 Param:		n/a
 Returns:	n/a
=======================================================================================================================*/
void OBD::canControllerSetLoopbackMode()
{
	CAN.mcp2515_modifyRegister(MCP_CANCTRL, 0xE0, 0x40);
}

/*=======================================================================================================================
 Name: canControllerSetListenOnlyMode
 ------------------------------------------------------------------------------------------------------------------------
 Purpose: 	function to place the CAN controller in listen only mode
 Param:		n/a
 Returns:	n/a
=======================================================================================================================*/
void OBD::canControllerSetListenOnlyMode()
{
	CAN.mcp2515_modifyRegister(MCP_CANCTRL, 0xE0, 0x60);
}

/*=======================================================================================================================
 Name: canControllerSetConfigurationMode
 ------------------------------------------------------------------------------------------------------------------------
 Purpose: 	function to place the CAN controller in configuration mode
 Param:		n/a
 Returns:	n/a
=======================================================================================================================*/
void OBD::canControllerSetConfigurationMode()
{
	CAN.mcp2515_modifyRegister(MCP_CANCTRL, 0xE0, 0x60);
}

/*=======================================================================================================================
 Name: setStandardMask
 ------------------------------------------------------------------------------------------------------------------------
 Purpose: 	function to set a maks for Standard CAN frames
 Param:		masks	array containing all the masks to be set
			maskNr	number of masks to be set
 Returns:	n/a
=======================================================================================================================*/
void OBD::setStandardMasks(uint32_t *masks, uint8_t maskNr)
{
	if (maskNr<2)
	{
		for (uint8_t i=0; i < maskNr; i++)
		{
			CAN.MCP_CAN::init_Mask(i, 0, masks[i]);
		}
	}
	
}

/*=======================================================================================================================
 Name: setStandardFilter
 ------------------------------------------------------------------------------------------------------------------------
 Purpose: 	function to set a filter for Standard CAN frames
 Param:		filters		array containing all the filters to be set
			filterNr	number of filters to be set
 Returns:	n/a
=======================================================================================================================*/
void OBD::setStandardFilter(uint32_t *filters, uint8_t filterNr)
{
	if (filterNr<6)
	{
		for (uint8_t i=0; i < filterNr; i++)
		{
			CAN.MCP_CAN::init_Filt(i, 0, filters[i]);
		}
	}
}

/*=======================================================================================================================
 Name: enableTimeStamping
 ------------------------------------------------------------------------------------------------------------------------
 Purpose: 	enable TimeStamping in CAN output prints
 Param:		n/a
 Returns:	n/a
=======================================================================================================================*/
void OBD::enableTimeStamping()
{
	timestamping=1;
}

/*=======================================================================================================================
 Name: disableTimeStamping
 ------------------------------------------------------------------------------------------------------------------------
 Purpose: 	disable TimeStamping in CAN output prints
 Param:		n/a
 Returns:	n/a
=======================================================================================================================*/
void OBD::disableTimeStamping()
{
	timestamping=0;
}

/*=======================================================================================================================
 Name: getObdProcessingtate
 ------------------------------------------------------------------------------------------------------------------------
 Purpose: 	get the state of the obd processing state machine
 Param:		n/a
 Returns:	processingState		current state of the state machine
=======================================================================================================================*/
obdProcessingtates OBD::getObdProcessingtate()
{
	return processingState;
}

/*=======================================================================================================================
Name: setQueryPid
------------------------------------------------------------------------------------------------------------------------
Purpose: set the PID used for the following query
Param:		pid
Returns:	n/a
=======================================================================================================================*/
void OBD::setQueryPid(uint16_t pid)
{
	pidToQuery=pid;
}


/*=======================================================================================================================
 Name: setObdProcessingtate
 ------------------------------------------------------------------------------------------------------------------------
 Purpose: 	set the state of the obd processing state machine
 Param:		processingState		current state of the state machine
 Returns:	n/a
=======================================================================================================================*/
void OBD::setObdProcessingtate(obdProcessingtates processingState)
{
	switch (processingState)
	{
		case STATE_DISABLED:
		case STATE_CONFIG:
			disableCANController();
			this->processingState=processingState;
		break;

		case STATE_LISTEN:
		case STATE_OPEN:
			init(baudrate);
			this->processingState=processingState;
		break;

		case STATE_SCANNING:
			initIdScanStandard();			
			this->processingState=processingState;
		break;

		case STATE_QUERYING:
			initContinuousQueryStandard();	
			this->processingState=processingState;	
		break;
	}
}
	



/* ----------------------------------------------------------------------------------------------------------------------
private functions
 ----------------------------------------------------------------------------------------------------------------------*/




/*=======================================================================================================================
 Name: disableCANController
 ------------------------------------------------------------------------------------------------------------------------
 Purpose: 	disable CAN controller
 Param:		n/a
 Returns:	n/a
=======================================================================================================================*/
void OBD::disableCANController()
{
	 detachInterrupt(2); //disable interrupt
	//make all standard ID's covered by the mask
	CAN.init_Mask(0, 0, 0x7FF);                         // there are 2 mask in mcp2515, you need to set both of them
    CAN.init_Mask(1, 0, 0x7FF);
    
    /*
     * set all filters to 00
     */
    CAN.init_Filt(0, 0, 0x00);                          // there are 6 filter in mcp2515
    CAN.init_Filt(1, 0, 0x00);                          // there are 6 filter in mcp2515
    
    CAN.init_Filt(2, 0, 0x00);                          // there are 6 filter in mcp2515
    CAN.init_Filt(3, 0, 0x00);                          // there are 6 filter in mcp2515
    CAN.init_Filt(4, 0, 0x00);                          // there are 6 filter in mcp2515
    CAN.init_Filt(5, 0, 0x00);                          // there are 6 filter in mcp2515
}
/*=======================================================================================================================
 Name: initIdScanStandard
 ------------------------------------------------------------------------------------------------------------------------
 Purpose: 	Do a Scan of all standard id's available on the bus 
 Param:		n/a
 Returns:	n/a
=======================================================================================================================*/
void OBD::initIdScanStandard()
{
	//make all standard ID's covered by the mask
	CAN.init_Mask(0, 0, 0x7FF);                         // there are 2 mask in mcp2515, you need to set both of them
    CAN.init_Mask(1, 0, 0x7FF);
    
    /*
     * set all filters to 00
     */
    CAN.init_Filt(0, 0, 0x00);                          // there are 6 filter in mcp2515
    CAN.init_Filt(1, 0, 0x00);                          // there are 6 filter in mcp2515
    
    CAN.init_Filt(2, 0, 0x00);                          // there are 6 filter in mcp2515
    CAN.init_Filt(3, 0, 0x00);                          // there are 6 filter in mcp2515
    CAN.init_Filt(4, 0, 0x00);                          // there are 6 filter in mcp2515
    CAN.init_Filt(5, 0, 0x00);                          // there are 6 filter in mcp2515
}

/*=======================================================================================================================
 Name: startContinuousQueryStandard
 ------------------------------------------------------------------------------------------------------------------------
 Purpose: 	Start a loop of querying a certain Standard PID 
 Param:		n/a
 Returns:	n/a
=======================================================================================================================*/
void OBD::initContinuousQueryStandard()
{
	//make all standard ID's covered by the mask
	CAN.init_Mask(0, 0, 0x7FF);                         // there are 2 mask in mcp2515, you need to set both of them
    CAN.init_Mask(1, 0, 0x7FF);
    
    /*
     * set all filters to 00
     */
    CAN.init_Filt(0, 0, pidToQuery);                    // there are 6 filter in mcp2515
    CAN.init_Filt(1, 0, 0x00);                          // there are 6 filter in mcp2515
    
    CAN.init_Filt(2, 0, 0x00);                          // there are 6 filter in mcp2515
    CAN.init_Filt(3, 0, 0x00);                          // there are 6 filter in mcp2515
    CAN.init_Filt(4, 0, 0x00);                          // there are 6 filter in mcp2515
    CAN.init_Filt(5, 0, 0x00);                          // there are 6 filter in mcp2515
}

/*=======================================================================================================================
 Name: obdRequest
 ------------------------------------------------------------------------------------------------------------------------
 Purpose: 	function that passes a PID request to a CAN message
 Param:		pid			OBD PID 
 Returns:	n/a
=======================================================================================================================*/


void OBD::obdRequest(uint8_t PID)
{
	transmitMessage.id = ID_REQUEST;
	transmitMessage.rtr = 0;
	transmitMessage.length = 8;
	transmitMessage.data[0]= 0x02;
	transmitMessage.data[1]= 0x01;
	transmitMessage.data[2]= PID;
	
	if(obdDebug==1) {
	Serial.print("request:");
	printMessage(&transmitMessage);
	}
	sendMessage(&transmitMessage);
}

/*=======================================================================================================================
 Name: printMessage
 ------------------------------------------------------------------------------------------------------------------------
 Purpose: 	function to print a CAN message to the serial port. For debugging purposes
 Param:		message			a can message
 Returns:	n/a
=======================================================================================================================*/


void OBD::printMessage(CanMessage *message)
{
	Serial.print("  id: ");
	Serial.print(message->id,HEX);
	Serial.print("  rtr: ");
	Serial.print(message->rtr,HEX);
	Serial.print(" => ");
	
	if (!message->rtr) {
		//Data
		Serial.print(" data:  ");
		Serial.print(message->data[0],HEX); Serial.print(",");
		Serial.print(message->data[1],HEX); Serial.print(",");
		Serial.print(message->data[2],HEX); Serial.print(",");
		Serial.print(message->data[3],HEX); Serial.print(",");
		Serial.print(message->data[4],HEX); Serial.print(",");
		Serial.print(message->data[5],HEX); Serial.print(",");
		Serial.print(message->data[6],HEX); Serial.print(",");
		Serial.println(message->data[7],HEX);
	}
}

/*=======================================================================================================================
 Name: sendMessage
 ------------------------------------------------------------------------------------------------------------------------
 Purpose: 	function to send a CAN message to the CAN bus through the lower layer library
 Param:		message			a can message
 Returns:	n/a
=======================================================================================================================*/
void OBD::sendMessage(CanMessage *message)
{
	CAN.sendMsgBuf(message->id, message->frameType, message->length, message->data);
}

/*=======================================================================================================================
 Name: getReceivedMessage
 ------------------------------------------------------------------------------------------------------------------------
 Purpose: 	function to get a received CAN message from the CAN bus controller
 Param:		message			a can message
 Returns:	n/a
=======================================================================================================================*/
void OBD::getReceivedMessage(CanMessage *message)
{
	CAN.readMsgBufID(&(message->id), &(message->length), message->data);
	message->rtr=0;
}







/* ----------------------------------------------------------------------------------------------------------------------
Interrupt functions
 ----------------------------------------------------------------------------------------------------------------------*/
 
 /*=======================================================================================================================
 Name: mcp_can_ISR
 ------------------------------------------------------------------------------------------------------------------------
 Purpose: 	interrupt function that handles interrupt of can transceiver
 Param:		n/a
 Returns:	n/a
=======================================================================================================================*/

static void mcp_can_ISR()
{
     messageReceived = 1;
}
 