#include "InitSock.h"
#include "stdafx.h"
#include "SocketWorker.hpp"

typedef struct _PER_HANDLE_DATA
{
    SOCKET sock;
}PER_HANDLE_DATA,* LPPER_HANDLE_DATA;

typedef struct _PER_IO_OPERATION_DATA
{
    OVERLAPPED Overlapped;
    WSABUF DataBuff;
    char Buff[24];
    BOOL OperationType;
}PER_IO_OPERATION_DATA,*LPPER_IO_OPERATION_DATA;

#define RECV_POSTED 1001
#define SEND_POSTED 1002

class IocpServer {
public:
	IocpServer();
	~IocpServer();
	int Init();
	int Listen();

private:
	HANDLE hCompletionPort;
	InitSock *env;
	SOCKET sockListen; 
	CThreadPool<SocketWorker> pool;
	TaskArray	tasks;
};