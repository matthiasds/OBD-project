#include "SIM900.h"
#include <SoftwareSerial.h>
#include <TinyGPS++.h>	

TinyGPSPlus gps;
String ReceivedData;
 
int numdata;
boolean started=false;
boolean connected;
int num_char;


/* ----------------------------------------------------------------------------------------------------------------------
public functions
 ----------------------------------------------------------------------------------------------------------------------*/
 
void gsm_setup()
{
//GSM settings
	//Serial.begin(9600); 									//USB  serial communication
	Serial.println("******START******");
	connectSerialGsm();										//start GSM communication
	Serial.println("******GSM CONNECTED******");
	connectInternet();										//connect internet
	Serial.println("******INTERNET CONNECTED******");
	delay(500);
	gsm.SimpleWriteln("AT+HTTPINIT");						//HTTP init
	Serial.println(gsm.WaitResp(10000, 200, "OK"));			// wait for response
	delay(500);
	gsm.SimpleWriteln("AT+HTTPPARA=\"CID\",\"1\"");			// http parameter
	Serial.println(gsm.WaitResp(10000, 200, "OK"));			//// wait for response
	delay(500);
//end gsm settings
}
 void gps_setup()
 {
 //GPS settings
	Serial2.begin(9600);									//start GPS communication
	SendToGPS("PSRF103,00,00,00,01"); 						// Stop GGA (fixe rate=0=off)
    SendToGPS("PSRF103,01,00,00,01"); 						// Stop GLL (fixe rate=0=off) 
    SendToGPS("PSRF103,02,00,00,01"); 						// Stop GSA (fixe rate=0=off)
    SendToGPS("PSRF103,03,00,00,01"); 						// Stop GSV (fixe rate=0=off)
    SendToGPS("PSRF103,04,00,00,01"); 						// Stop RMC (fixe rate=0=off)
    SendToGPS("PSRF103,05,00,00,01"); 						// Stop VTG (fixe rate=0=off)
//end gps settings	
}

TinyGPSPlus getGPS_data()
{
  //get GPS DATA
		//delay(5000);
		Serial2.flush();
		SendToGPS("PSRF103,00,01,00,01"); // activate GGA rate=1
		while(!Serial2.available()); // Wait for data
		getData();
		
		SendToGPS("PSRF103,00,00,00,01");
return gps;
//end get GPS DATA
  }
  
  void sendGSM_data(String data)
  {
    //send data
    unsigned int len = 100;
    char msg[60],buf[100];
    data.toCharArray(buf, len);
  //gsm.SimpleWriteln("AT+HTTPPARA=\"URL\",\"namking.be/project/index.php?name=test\"");
        gsm.SimpleWriteln(buf);
	Serial.println(gsm.WaitResp(10000, 200, "OK"));
	gsm.SimpleWriteln("AT+HTTPACTION=0");
	Serial.println(gsm.WaitResp(10000, 200, "OK")); 
	Serial.println(gsm.WaitResp(10000, 200, "ERROR"));
//end send
    }
	
 
 void connectSerialGsm()
 {
	if (gsm.begin(2400))
		{
		  started=true;  
		}
	else 
		Serial.println("STATUS=IDLE");
 }
 
 void connectInternet()
 {
 if(started)
  {
    if (attachGPRS()) 
	Serial.println("status=ATTACHED");
    else 
	Serial.println("status=ERROR");
  }
 }
 
  int attachGPRS()
{
  
	 gsm.SimpleWriteln("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
        Serial.println(gsm.WaitResp(10000, 200, "OK"));
        delay(500);
        gsm.SimpleWriteln("AT+SAPBR=3,1,\"APN\",\"web.be\"");
        Serial.println(gsm.WaitResp(10000, 200, "OK"));
        delay(500);
        gsm.SimpleWriteln("AT+SAPBR=3,1,\"USER\",\"web\"");
        Serial.println(gsm.WaitResp(10000, 200, "OK"));
        delay(500);
        gsm.SimpleWriteln("AT+SAPBR=3,1,\"PWD\",\"web\"");
        Serial.println(gsm.WaitResp(10000, 200, "OK"));
        delay(500);
        gsm.SimpleWriteln("AT+SAPBR=1,1");
        Serial.println(gsm.WaitResp(10000, 200, "OK"));
        delay(1000);
return 1;
}

void getData()
	{
		while(Serial2.available())
		{
			int c = Serial2.read();
			ReceivedData=ReceivedData+char(c);
			boolean test=gps.encode(c);
			delay(1);
			
			if (test==true)
			{
				ReceivedData=""; //Full data string is decoded. Start looking for a new one.
			}
			
		}
	}

void SendToGPS (String GPScommand) 
	{
	  String checksumCOMMAND=String (checkSum(GPScommand),HEX);
	  GPScommand="$"+GPScommand+"*"+checksumCOMMAND+char(0xD)+char(0xA); 
	  Serial2.flush(); // EMPTY BUFFER
	  Serial2.print(GPScommand); // SEND COMMAND
	}

int checkSum (String Command) 
	{ 
		int test=0; 
		for (int i=0; i<Command.length(); i++)
		{ 
			char c=Command.charAt(i);
			if (test == 0) 
			{
				test = byte(c);
			}
			else 
			{
				test = test ^ byte(c);
			}
		}
		return test; 
	}
