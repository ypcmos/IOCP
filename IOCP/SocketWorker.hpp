#pragma once


///////////////////////////////////////////////////////////////////////////////
class TaskBase
{
public:
	virtual void DoTask(void *pvParam, OVERLAPPED *pOverlapped) = 0;
};

typedef CSimpleArray<TaskBase*> TaskArray;

///////////////////////////////////////////////////////////////////////////////
class SocketTask : public TaskBase
{
public:
	virtual void DoTask(void *pvParam, OVERLAPPED *pOverlapped);
};

///////////////////////////////////////////////////////////////////////////////
class SocketWorker
{
public:
    typedef DWORD_PTR RequestType;

	SocketWorker()
	{
	}

    virtual BOOL Initialize(void *pvParam)
    {
		return TRUE;
    }

    virtual void Terminate(void* /*pvParam*/)
	{
	}

	void Execute(RequestType dw, void *pvParam, OVERLAPPED* pOverlapped) throw()
    {
        ATLASSERT(pvParam != NULL);
		TaskBase* pTask = (TaskBase*)(DWORD_PTR)dw;
		pTask->DoTask(pvParam, pOverlapped);
	}

    virtual BOOL GetWorkerData(DWORD /*dwParam*/, void ** /*ppvData*/)
    {
        return FALSE;
    }

}; 

