#include <WinSock2.h>
#pragma comment(lib, "WS2_32")

class InitSock
{
public:
	InitSock(BYTE miniVer = 2, BYTE majorVer = 2)
	{
		WSADATA wsaData;
		WORD sockVersion = MAKEWORD(miniVer, majorVer);
		if (WSAStartup(sockVersion, &wsaData) != 0)
		{
			exit(0);
		}
	}
	
	~InitSock() 
	{
		WSACleanup();
	}
};