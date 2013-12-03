
#include "IocpServer.h"
#include <iostream>
using namespace std;

IocpServer::IocpServer()
{
	env = new InitSock();
}

IocpServer::~IocpServer()
{
	delete env;
	for (int i = 0; i < tasks.GetSize(); i++ ) {
		TaskBase* pTask = tasks[i];
		ATLASSERT( NULL != pTask );
		delete pTask;
	}
	pool.Shutdown();
}

int IocpServer::Init() {
    SYSTEM_INFO siSys;
    
	struct sockaddr_in addrLocal;

    hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (hCompletionPort == INVALID_HANDLE_VALUE)
    {
        cout<<"创建IO完成端口失败"<<endl;
        return 2;
    }

    GetSystemInfo(&siSys);
	int NumberOfProcessors = siSys.dwNumberOfProcessors * 2 + 2;
	

	HRESULT hr = pool.Initialize(hCompletionPort, NumberOfProcessors);

	if( SUCCEEDED( hr ) ) {
		for (int i = 0; i < NumberOfProcessors; i++ ) {
			TaskBase* pTask = new SocketTask;
			tasks.Add(pTask);
			ATLASSERT( NULL != pTask );
			pool.QueueRequest((SocketWorker::RequestType)pTask);
		}	
	}
    sockListen = socket(AF_INET,SOCK_STREAM,0);
    if (sockListen == SOCKET_ERROR)
    {
        cout<<"socket错误"<<endl;
        return 3;
    }

    addrLocal.sin_family = AF_INET;
    addrLocal.sin_addr.s_addr = htonl(INADDR_ANY);
    addrLocal.sin_port = htons(9090);
    if (bind(sockListen, (struct sockaddr *)&addrLocal, sizeof(sockaddr_in)) != 0)
    {
        cout<<"bind错误"<<endl;
        int n = WSAGetLastError();
        return 5;
    }
  
    if(listen(sockListen, 5)!=0)
    {
        cout<<"listen错误"<<endl;
        return 6;
    }
	return 0;
}

int IocpServer::Listen()
{
	DWORD dwRecvBytes;
	LPPER_HANDLE_DATA perHandleData;
    LPPER_IO_OPERATION_DATA ioperdata;
    int nRet = 0;
   
    SOCKET sockAccept;
    DWORD dwFlags = 0;
   
	while(true)
    {
		SOCKADDR_IN addr;
		int addrlen = sizeof(addr);
        sockAccept = accept(sockListen, (sockaddr*)&addr, &addrlen);
        perHandleData = (LPPER_HANDLE_DATA)malloc(sizeof(PER_HANDLE_DATA));
		
        if(perHandleData == NULL)
            continue;
        cout<<"socket number "<<sockAccept<<"接入"<<endl;
        perHandleData->sock = sockAccept;

        ioperdata = (LPPER_IO_OPERATION_DATA)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(PER_IO_OPERATION_DATA));
        (ioperdata->DataBuff).len = 24;
        (ioperdata->DataBuff).buf = ioperdata->Buff;
        ioperdata->OperationType = RECV_POSTED;
        if( ioperdata == NULL)
        {
            free(perHandleData);
            continue;
        }
       
		if(CreateIoCompletionPort((HANDLE)perHandleData->sock, hCompletionPort, (DWORD)perHandleData->sock, 0) == NULL)
        {
            cout<<sockAccept<<"createiocompletionport错误"<<endl;
            free(perHandleData);
			HeapFree(GetProcessHeap(), 0, ioperdata);
            continue;
        }
       
        WSARecv(perHandleData->sock, &ioperdata->DataBuff, 1, &dwRecvBytes,&dwFlags, &(ioperdata->Overlapped), NULL);
    }
}

void SocketTask::DoTask(void *pvParam, OVERLAPPED *pOverlapped)
{
	HANDLE ComPort = (HANDLE)pvParam;
    DWORD BytesTransferred;
    LPPER_HANDLE_DATA PerHandleData = (LPPER_HANDLE_DATA)malloc(sizeof(PER_HANDLE_DATA));
    LPPER_IO_OPERATION_DATA PerIoData;
    DWORD SendBytes,RecvBytes;
    DWORD Flags;
    BOOL bT;
	int ret;

    while(TRUE)
    {
        bT = GetQueuedCompletionStatus(ComPort,
            &BytesTransferred, (PULONG_PTR)&(PerHandleData->sock),
            (LPOVERLAPPED*)&PerIoData,INFINITE);
		
		if (BytesTransferred == 0xFFFFFFFF)
		{
			cout<<"SOCKET down"<<endl;
			return;
		}
        if(BytesTransferred == 0 &&
            (PerIoData->OperationType == RECV_POSTED ||
            PerIoData->OperationType == SEND_POSTED))
        {
            //关闭SOCKET
            cout<<PerHandleData->sock<<"SOCKET关闭"<<endl;
            closesocket(PerHandleData->sock);
			HeapFree(GetProcessHeap(), 0, PerIoData);
            continue;
        }
		
        if(PerIoData->OperationType == RECV_POSTED)
        {
            //处理
            cout<<"接收处理"<<endl;
			PerIoData->Buff[BytesTransferred] = 0;
            //cout<<PerHandleData->sock<<"SOCKET :"  << endl;//<< PerIoData->Buff << endl;
			cout << "FROM " << *(SOCKET*)PerHandleData << ':' << PerIoData->Buff << endl;
            //回应客户端
            ZeroMemory(PerIoData->Buff,24);
            strcpy_s(PerIoData->Buff, "OK");
            Flags = 0;
            ZeroMemory((LPVOID)&(PerIoData->Overlapped),sizeof(OVERLAPPED));
            PerIoData->DataBuff.len = 2;
            PerIoData->DataBuff.buf = PerIoData->Buff;
            PerIoData->OperationType = SEND_POSTED;
			SendBytes = 2;
            ret = WSASend(PerHandleData->sock, &PerIoData->DataBuff,
                1,&SendBytes,0,&(PerIoData->Overlapped),NULL);
        }
        else //if(PerIoData->OperationType == SEND_POSTED)
        {
            //发送时的处理
            cout<<"发送处理"<<endl;
            Flags = 0;
            ZeroMemory((LPVOID)&(PerIoData->Overlapped), sizeof(OVERLAPPED));
            ZeroMemory(PerIoData->Buff, 24);
            PerIoData->DataBuff.len = 24;
            PerIoData->DataBuff.buf = PerIoData->Buff;
            PerIoData->OperationType = RECV_POSTED;
			ret = WSARecv(PerHandleData->sock, &PerIoData->DataBuff,
                1, &RecvBytes, &Flags, &(PerIoData->Overlapped), NULL);
        }
		if (ret == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				cout << "WSA send or recv ERROR" << endl;
			}
		}
    }
	free(PerHandleData);
	return;
}
