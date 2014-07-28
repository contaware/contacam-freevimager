#ifndef __WORKERTHREAD_H__
#define __WORKERTHREAD_H__

class CWorkerThread : public CWinThread
{
public:
	CWorkerThread();
	virtual ~CWorkerThread();
	virtual bool Start(int nPriority = THREAD_PRIORITY_NORMAL);
	bool Pause();
	bool Kill(DWORD dwTimeout = INFINITE);						// this is Kill_NoBlocking() + return WaitDone_Blocking(dwTimeout)
	void Kill_NoBlocking();										// sets m_hKillEvent
	bool WaitDone_Blocking(DWORD dwTimeout = INFINITE);			// returns FALSE if after the timeout the thread was forced to terminate!
	void SetProcMsg(bool bProcMsg) {m_bProcMsg = bProcMsg;};	// to "abuse" the thread class with a message loop instead of running a thread!
	__forceinline bool DoExit() {return (m_bProcMsg ? ProcMsg() : ::WaitForSingleObject(m_hKillEvent, 0) == WAIT_OBJECT_0);};
	__forceinline HANDLE GetHandle() const {return m_hThread;};
	__forceinline DWORD GetId() const {return m_nThreadID;};
	__forceinline HANDLE GetKillEvent() const {return m_hKillEvent;};
	__forceinline HANDLE GetStartupEvent() const {return m_hStartupEvent;};
	__forceinline bool IsRunning() {return m_bRunning;};
	__forceinline bool IsAlive() {return m_bAlive;};

protected:
	virtual BOOL InitInstance(){return TRUE;};
	virtual BOOL OnInitThread(){return TRUE;};
	virtual void OnExitThread(int nExitCode){nExitCode;};
	virtual int Run();
	virtual int Work();
	__forceinline bool ProcMsg()
	{
		MSG Msg;
		while (::PeekMessage(&Msg, NULL, NULL, NULL, PM_NOREMOVE))
		{
			// Exit?
			if ((Msg.message == WM_COMMAND && Msg.wParam == ID_APP_EXIT)				||
				Msg.message == WM_QUERYENDSESSION										||
				(Msg.message == WM_LBUTTONUP && ::GetDlgCtrlID(Msg.hwnd) == IDCANCEL))
				return true;
			// Sent also if MainFrame disabled...remove them!
			else if (Msg.message == WM_MOUSEWHEEL	||
					Msg.message == WM_KEYDOWN		||
					Msg.message == WM_KEYUP			||
					Msg.message == WM_SYSKEYDOWN	||
					Msg.message == WM_SYSKEYUP)
				::GetMessage(&Msg, NULL, NULL, NULL);
			else
				::AfxGetThread()->PumpMessage();
		}
		return false;
	}

	HANDLE volatile m_hStartupEvent;
	HANDLE volatile m_hKillEvent;
	volatile bool m_bRunning;
	volatile bool m_bAlive;
	bool m_bProcMsg;
	CRITICAL_SECTION m_cs; // Critical section for m_bRunning and m_bAlive
};

#endif /* __WORKERTHREAD_H__ */