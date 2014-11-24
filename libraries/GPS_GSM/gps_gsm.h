/*
 * gps_gsm.h
 *
 * Created: 17/11/2014 16:37:45
 *  Author: matthias
 */ 

#include <SoftwareSerial.h>
#include <SIM900.h>
#include <TinyGPS++.h>	


#ifndef GPS_GSM_H_
#define GPS_GSM_H_

void gsm_gps_setup();
void gsm_gps_loop();
void connectSerialGsm();
void connectInternet();
int attachGPRS();
void getData();
void SendToGPS (String GPScommand);
int checkSum (String Command);

#endif /* GPS_GSM_H_ */