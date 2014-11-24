#ifndef _INETGSM_H_
#define _INETGSM_H_

#define BUFFERSIZE 1

#include "SIM900.h"

class InetGSM
{
  private:
    char _buffer[BUFFERSIZE];
    
  public:
    int httpPOST(const char* server, int port, const char* path, const char* parameters, char* result, int resultlength);
    //int httpPOSTagain(const char* server, int port, const char* path, const char* parameters, char* result, int resultlength);
    int attachGPRS(char* domain, char* dom1, char* dom2);
    int dettachGPRS();
    int connectTCP(const char* server, int port);
    int disconnectTCP();
	int tcpWrite(const char* server, int port, char* data, char* result, int resultlength);
	


};

#endif
