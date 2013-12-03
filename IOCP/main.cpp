#include "IocpServer.h"
#include <iostream>
using namespace std;

void main() 
{
	IocpServer iocp;
	iocp.Init();
	iocp.Listen();
}
