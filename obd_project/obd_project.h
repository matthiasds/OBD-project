/*
 * obd_project.h
 * Description: Main include file for the OBD project
 *
 *  Created on: 16/11/2014
 *  Author: Matthias de Schepper
 */


#ifndef OBD_PROJECT_H_
#define OBD_PROJECT_H_

#include "Arduino.h"


#define SERIAL_BUF_LEN           128     // Maximum length for any serial string

#define VERSION_HARDWARE_MAJOR 1
#define VERSION_HARDWARE_MINOR 0
#define VERSION_FIRMWARE_MAJOR 1
#define VERSION_FIRMWARE_MINOR 4

/**************** List of Supported Commands *************************/
/* SLCAN compatible commands: see list  http://www.fischl.de/usbtin/ */
/*********************************************************************/
#define SET_BAUD_RATE								'S'		//followed by 0-8 representing choice of "List of standard baud rates"
#define SET_CUSTOM_BIT_RATE							's'		//followed by 3 hexadecimal bytes
#define READ_MPC2515_REGISTER						'G'		//followed by 1 hexadecimal byte representing the address to read, returns 1 hexadecimal byte representing the read value
#define GET_HARDWARE_VERSION						'V'		//prints 2 hexadecimal bytes representing MAJOR and MINOR number
#define GET_FIRMWARE_VERSION						'v'		//prints 2 hexadecimal bytes representing MAJOR and MINOR number
#define GET_SERIAL_NUMBER							'N'
#define OPEN_CAN_CHANNEL							'O'		//no parameters or returns
#define OPEN_DEVICE_IN_LOOPBACK_MODE				'l'		//no parameters or returns
#define OPEN_CAN_CHANNEL_IN_LISTEN_ONLY_MODE		'L'		//no parameters or returns
#define CLOSE_CAN_CHANNEL							'C'		//no parameters or returns
#define TRANSMIT_STANDARD_FRAME						't'		
#define TRANSMIT_EXTENDED_FRAME						'T'
#define TRANSMIT_STANDARD_RTR_FRAME					'r'
#define TRANSMIT_EXTENDED_RTR_FRAME					'R'
#define READ_CAN_CONTROLLER_STATUS_ERROR			'F'		//prints 1 hexadecimal byte with error code
															/************ error bits: ***********/
															/* bit0 not used					*/
															/* bit1 not used					*/
															/* bit2 error warning (EWARN)		*/
															/* bit3 data overrun (RX1OVR/RX0OVR)*/
															/* bit4 not used					*/
															/* bit5 error passive (TXEP/RXEP)	*/
															/* bit6 not used					*/
															/* bit7 Bus error (TXBO)			*/
															/************************************/

#define SET_TIMESTAMPING							'Z'		//folowed by 0 for off or 1 for on

/* extended commands */
#define SET_ACCEPTANCE_MASK							'm'		//followed by 2*2 hexadecimal bytes representing 2 acceptance masks
#define SET_ACCEPTANCE_FILTER						'M'		//followed by 4*2 hexadecimal bytes representing 4 acceptance filters
#define WRITE_MPC2515_REGISTER						'W'		//followed by 2 hexadecimal bytes representing the address to write, and the data to write
#define ENABLE_INTERFACE							'I'		
#define DISABLE_INTERFACE							'i'
	
#define GPS_INTERFACE	'P'
#define GSM_INTERFACE	'G'
#define CAN_INTERFACE	'C'

#define SCAN_STANDARD_BUS_IDS						'b'
#define SCAN_EXTENDED_BUS_IDS						'B'
#define QUERY_PID_VALUE								'Q'
#define DISABLE_DEBUG_OUTPUT						'd'
#define ENABLE_DEBUG_OUTPUT							'D'



/********** List of standard baud rates *************/
#define BAUD_10KB	'0'
#define BAUD_20KB	'1'
#define BAUD_50KB	'2'
#define BAUD_100KB	'3'
#define BAUD_125KB	'4'
#define BAUD_250KB	'5'
#define BAUD_500KB	'6'
#define BAUD_800KB	'7'
#define BAUD_1MB	'8'





/**
 * Global variables
 */
char serialCmd[SERIAL_BUF_LEN];          // Serial buffer
int length = 0;
int mainDebug=0;



#endif /* OBD_PROJECT_H_ */
