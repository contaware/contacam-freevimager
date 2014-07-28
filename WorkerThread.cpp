#include "stdafx.h"
#include "WorkerThread.h"
#include <process.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CWorkerThread::CWorkerThread()
{
	// m_hThread and m_nThreadID are reset inside the base constructor

	m_bRunning =		false;
	m_bAlive =			false;
	m_bProcMsg =		false;
	m_bAutoDelete =		FALSE;

	m_hStartupEvent =	::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hKillEvent =		::CreateEvent(NULL, TRUE, FALSE, NULL);

	::InitializeCriticalSection(&m_cs);
}

CWorkerThread::~CWorkerThread()
{
	if (!m_bAutoDelete)
		Kill();

	if (m_hStartupEvent)
		::CloseHandle(m_hStartupEvent);
	
	if (m_hKillEvent)
		::CloseHandle(m_hKillEvent);

	::DeleteCriticalSection(&m_cs);

	// CloseHandle(m_hThread) is called inside the base destructor
}

/*
THREAD_PRIORITY_TIME_CRITICAL
THREAD_PRIORITY_HIGHEST
THREAD_PRIORITY_ABOVE_NORMAL
THREAD_PRIORITY_NORMAL
THREAD_PRIORITY_BELOW_NORMAL
THREAD_PRIORITY_LOWEST
THREAD_PRIORITY_IDLE 
*/
bool CWorkerThread::Start(int nPriority/*=THREAD_PRIORITY_NORMAL*/)
{
	::EnterCriticalSection(&m_cs);
	bool bStartup;
	if (!m_bAlive)
	{
		if (m_hThread)
		{
			WaitDone_Blocking(1000); // Give 1 sec to completely exit
			::CloseHandle(m_hThread);
			m_hThread = NULL;
		}
		m_nThreadID = 0;
		::ResetEvent(m_hKillEvent);	// Be Sure To Reset This Event!
		if (!CreateThread(CREATE_SUSPENDED))
		{
			::LeaveCriticalSection(&m_cs);
			return false;
		}
		m_bAlive = true;
		bStartup = true;
	}
	else
		bStartup = false;

	if (!m_bRunning)
	{
		if (ResumeThread() != 0xFFFFFFFF)
		{
			m_bRunning = true;
			SetThreadPriority(nPriority);
			if (bStartup)
				::SetEvent(m_hStartupEvent);
			::LeaveCriticalSection(&m_cs);
			return true;
		}
		else
		{
			::LeaveCriticalSection(&m_cs);
			return false;
		}
	}
	else
	{
		SetThreadPriority(nPriority);
		::LeaveCriticalSection(&m_cs);
		return true;
	}
}

bool CWorkerThread::Pause()
{
	::EnterCriticalSection(&m_cs);
	if (m_bRunning == true)
	{
		if (SuspendThread() != 0xFFFFFFFF)
		{
			m_bRunning = false;
			::LeaveCriticalSection(&m_cs);
			return true;
		}
		else
		{
			::LeaveCriticalSection(&m_cs);
			return false;
		}
	}
	else
	{
		::LeaveCriticalSection(&m_cs);
		return true;
	}
}

int CWorkerThread::Run()
{
	int res = 0;
	try
	{
		// Set m_pMainWnd to the main window, this is necessary if showing
		// modal dialogs from this thread.
		//
		// See code of _AfxThreadEntry(void* pParam) in Thrdcore.cpp:
		// In that function m_pMainWnd is init to the main frame, but with a
		// CWnd object (not CFrameWnd) on the stack. A problem arises if 
		// if we want to re-use the thread object for a second time,
		// in that case m_pMainWnd still points to the old CWnd on the stack.
		// We could set m_pMainWnd to NULL before starting the thread, this
		// would force _AfxThreadEntry(void* pParam) to re-init m_pMainWnd.
		// Still remains the problem that we prefer to have a CFrameWnd
		// object and not a CWnd object. The following method solves both problems,
		// the only catch of it is that we have to make absolutely sure that all
		// threads are done before the Main Frame object is deleted!!!
		CWinThread* pUIThread = (CWinThread*)::AfxGetApp();
		if (pUIThread)
			m_pMainWnd = pUIThread->GetMainWnd();
		else
			m_pMainWnd = NULL;
		if (OnInitThread())
		{
			res = Work();
			OnExitThread(res);
			::ResetEvent(m_hStartupEvent); // Be Sure To Reset This Event!
		}
		::EnterCriticalSection(&m_cs);
		m_bRunning = false;
		m_bAlive = false;
		m_pMainWnd = NULL;
		::LeaveCriticalSection(&m_cs);
		return res;
	}
	catch (CException* e)
	{
		e->ReportError(); // ReportError() needs m_pMainWnd -> reset m_pMainWnd at the end!
		e->Delete();
		::ResetEvent(m_hStartupEvent); // Be Sure To Reset This Event!
		::EnterCriticalSection(&m_cs);
		m_bRunning = false;
		m_bAlive = false;
		m_pMainWnd = NULL;
		::LeaveCriticalSection(&m_cs);
		return 0;
	}
}

int CWorkerThread::Work()
{
	while (TRUE)
	{
		if (::WaitForSingleObject(m_hKillEvent, 1000) == WAIT_OBJECT_0)
		{
			return 0;
		}
		TRACE(_T("Thread: %lu\n"), m_nThreadID);
	}
	return 0;
}

bool CWorkerThread::Kill(DWORD dwTimeout/*=INFINITE*/)
{
#ifdef _DEBUG
	if (m_bAutoDelete)
	{
		TRACE(_T("Use Kill_NoBlocking() for auto delete threads!\n"));
		ASSERT(FALSE);
	}
#endif
	::EnterCriticalSection(&m_cs);
	if (m_bAlive)
	{
		// Be Sure the Thread is Running
		if (m_bRunning == false)
		{
			if (ResumeThread() != 0xFFFFFFFF)
				m_bRunning = true;
		}

		::LeaveCriticalSection(&m_cs);

		// Send the Thread Kill Event
		::SetEvent(m_hKillEvent);

		// Wait until thread exits
		return WaitDone_Blocking(dwTimeout);
	}
	else
	{
		::LeaveCriticalSection(&m_cs);

		// Give 1 sec to completely exit
		return WaitDone_Blocking(1000);
	}
}

void CWorkerThread::Kill_NoBlocking()
{
	::EnterCriticalSection(&m_cs);
	if (m_bAlive)
	{
		// Be Sure the Thread is Running
		if (m_bRunning == false)
		{
			if (ResumeThread() != 0xFFFFFFFF)
				m_bRunning = true;
		}

		::LeaveCriticalSection(&m_cs);

		// Send the Thread Kill Event
		::SetEvent(m_hKillEvent);
	}
	else
		::LeaveCriticalSection(&m_cs);
}

bool CWorkerThread::WaitDone_Blocking(DWORD dwTimeout/*=INFINITE*/)
{
	// Wait until thread exits
	if (m_hThread && ::WaitForSingleObject(m_hThread, dwTimeout) != WAIT_OBJECT_0)
	{
		// If it doesn't want to exit force the termination!
		if (m_hThread)
		{
			::TerminateThread(m_hThread, 0);
			::ResetEvent(m_hStartupEvent); // Be Sure To Reset This Event!
			::EnterCriticalSection(&m_cs);
			m_bRunning = false;
			m_bAlive = false;
			m_pMainWnd = NULL;
			::LeaveCriticalSection(&m_cs);
			TRACE(_T("Thread with ID = 0x%08X has been forced to terminate!\n"), m_nThreadID);
			return false;
		}
	}
	return true;
}