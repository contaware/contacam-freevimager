#ifndef __WORKERTHREAD_H__
#define __WORKERTHREAD_H__

class CWorkerThread : public CWinThread
{
public:
	friend UINT WorkerThreadProc(LPVOID pParam);
	CWorkerThread();
	virtual ~CWorkerThread();
	virtual bool Start(int nPriority = THREAD_PRIORITY_NORMAL);
	bool Pause();
	bool Kill(DWORD dwTimeout = INFINITE);						// this is Kill_NoBlocking() + return WaitDone_Blocking(dwTimeout)
	void Kill_NoBlocking();										// sets m_hKillEvent
	bool WaitDone_Blocking(DWORD dwTimeout = INFINITE);			// returns false if after the given timeout the thread is not stopping
	__forceinline bool DoExit() {return (::WaitForSingleObject(m_hKillEvent, 0) == WAIT_OBJECT_0);};
	__forceinline HANDLE GetHandle() const {return m_hThread;};
	__forceinline DWORD GetId() const {return m_nThreadID;};
	__forceinline HANDLE GetKillEvent() const {return m_hKillEvent;};
	__forceinline bool IsRunning() {return m_bRunning;};
	__forceinline bool IsAlive() {return m_bAlive;};
	
protected:
	virtual int Work();
	HANDLE volatile m_hKillEvent;
	volatile bool m_bRunning;
	volatile bool m_bAlive;
	CRITICAL_SECTION m_cs; // Critical section for m_bRunning and m_bAlive
};

#endif /* __WORKERTHREAD_H__ */