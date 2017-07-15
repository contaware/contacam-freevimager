#include "stdafx.h"
#include "WorkerThread.h"
#include <process.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

UINT WorkerThreadProc(LPVOID pParam)
{
	CWorkerThread* pWorkerThread = (CWorkerThread*)pParam;
	if (pWorkerThread == NULL)
		return 1;

	int res = 0;
	try
	{
		res = pWorkerThread->Work();
		::EnterCriticalSection(&pWorkerThread->m_cs);
		pWorkerThread->m_bRunning = false;
		pWorkerThread->m_bAlive = false;
		::LeaveCriticalSection(&pWorkerThread->m_cs);
		return res;
	}
	catch (CException* e)
	{
		e->ReportError();
		e->Delete();
		::EnterCriticalSection(&pWorkerThread->m_cs);
		pWorkerThread->m_bRunning = false;
		pWorkerThread->m_bAlive = false;
		::LeaveCriticalSection(&pWorkerThread->m_cs);
		return 0;
	}
}

CWorkerThread::CWorkerThread()
{
	// m_hThread, m_nThreadID and m_pMainWnd are reset inside the base constructor
	m_pfnThreadProc =	WorkerThreadProc;
	m_pThreadParams =	(LPVOID)this;

	m_bRunning =		false;
	m_bAlive =			false;
	m_bAutoDelete =		FALSE;

	m_hKillEvent =		::CreateEvent(NULL, TRUE, FALSE, NULL);

	::InitializeCriticalSection(&m_cs);
}

CWorkerThread::~CWorkerThread()
{
	if (!m_bAutoDelete)
		Kill();
	
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
	// Set m_pMainWnd to the main window (necessary if showing modal dialogs
	// from this thread).
	//
	// In _AfxThreadEntry() of thrdcore.cpp if m_pMainWnd is NULL it gets init
	// to the main window on the stack. A problem arises if we want to re-use
	// the thread object for a second time, in that case m_pMainWnd still points
	// to the old CWnd on the stack.
	// The solution is to init it before _AfxThreadEntry() is called making sure
	// that all threads are done when deleting the Main Frame object on app exit.
	if (!m_pMainWnd)
		m_pMainWnd = ((CWinThread*)::AfxGetApp())->GetMainWnd();

	::EnterCriticalSection(&m_cs);
	if (!m_bAlive)
	{
		// Wait done
		if (m_hThread)
		{
			WaitDone_Blocking(1000); // Give 1 sec to completely exit
			::CloseHandle(m_hThread);
			m_hThread = NULL;
		}
		m_nThreadID = 0;
		::ResetEvent(m_hKillEvent);	// Be Sure To Reset This Event!

		// Start thread suspended
		if (!CreateThread(CREATE_SUSPENDED))
		{
			::LeaveCriticalSection(&m_cs);
			return false;
		}
		m_bAlive = true;
	}

	if (!m_bRunning)
	{
		SetThreadPriority(nPriority);
		if (ResumeThread() != 0xFFFFFFFF)
		{
			m_bRunning = true;
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

int CWorkerThread::Work()
{
	while (TRUE)
	{
		if (::WaitForSingleObject(m_hKillEvent, 1000) == WAIT_OBJECT_0)
			return 0;
		::LogLine(_T("Thread: %lu"), m_nThreadID);
	}
	return 0;
}

bool CWorkerThread::Kill(DWORD dwTimeout/*=INFINITE*/)
{
#ifdef _DEBUG
	if (m_bAutoDelete)
	{
		// Use Kill_NoBlocking() for auto delete threads!
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
	if (m_hThread && ::WaitForSingleObject(m_hThread, dwTimeout) != WAIT_OBJECT_0)
	{
		::LogLine(_T("Thread with ID = 0x%08X is not stopping after waiting for %u ms"), m_nThreadID, dwTimeout);
		return false;
	}
	return true;
}