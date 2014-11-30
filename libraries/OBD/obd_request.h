/*
 * obd_request.h
 * Description: Translates obd request of pid's in CAN messages and anlyses answers from the CAN bus
 *
 *  Created on: 16/11/2014
 *  Author: Matthias de Schepper
 */

#ifndef OBD_REQUEST_H_
#define OBD_REQUEST_H_

typedef enum 
{
	STANDARD_FRAME = 0,
	EXTENDED_FRAME = 1
}FrameType;

typedef enum
{
	STATE_CONFIG,
	STATE_OPEN,
	STATE_LISTEN,
	STATE_SCANNING,
	STATE_QUERYING,
	STATE_DISABLED
} obdProcessingtates;

class OBD
{
	private:
	typedef struct{
		uint32_t id;
		FrameType frameType;
		uint8_t rtr;
		uint8_t length;
		uint8_t data[8];
	}CanMessage;
	
	CanMessage transmitMessage;
	CanMessage receiveMessage;
	obdProcessingtates processingState;
	uint8_t timestamping;
	uint8_t baudrate;
	uint16_t pidToQuery;
	
	
	void obdRequest(uint8_t pid);
	void disableCANController();
	void printMessage(CanMessage *message);
	void sendMessage(CanMessage *message);
	void getReceivedMessage(CanMessage *message);
	void initIdScanStandard();
	void initContinuousQueryStandard();
	
	public:
	void init(uint8_t baudrate);
	void processing();
	uint8_t canControllerReadRegister(uint8_t address);
	void canControllerWriteRegister(uint8_t address, uint8_t data);
	void canControllerSetNormalOperatingMode();
	void canControllerSetLoopbackMode();
	void canControllerSetListenOnlyMode();
	void canControllerSetConfigurationMode();
	void setStandardMasks(uint32_t *masks, uint8_t maskNr);
	void setExtendedMasks(uint32_t *masks, uint8_t maskNr);
	void setStandardFilter(uint32_t *filters, uint8_t filterNr);
	void setExtendedFilter(uint32_t *filters, uint8_t filterNr);
	void enableTimeStamping();
	void disableTimeStamping();
	obdProcessingtates getObdProcessingtate();
	void setObdProcessingtate(obdProcessingtates processingState);
	void setQueryPid(uint16_t pid);
	
	uint8_t obdDebug;
};



#endif /* OBD_REQUEST_H_ */