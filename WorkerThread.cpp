#include "stdafx.h"
#include "WorkerThread.h"
#include <process.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// TODO: remove that if dropping Windows XP support
#if _MSC_VER > 1600
UINT WorkerThreadProc(LPVOID pParam);
extern "C" void __cdecl __acrt_freeptd(); // defined in per_thread_data.cpp
static void WorkerThreadCleanAndEnd(UINT nExitCode)
{
	// From _AfxThreadEntry in thrdcore.cpp

	// remove current CWinThread object from memory
	AFX_MODULE_THREAD_STATE* pState = AfxGetModuleThreadState();
	CWinThread* pThread = pState->m_pCurrentWinThread;
	if (pThread != NULL)
	{
		ASSERT_VALID(pThread);
		ASSERT(pThread != AfxGetApp());

		// cleanup OLE if required
		if (pThread->m_lpfnOleTermOrFreeLib != NULL)
			(*pThread->m_lpfnOleTermOrFreeLib)(TRUE, FALSE);

		pThread->Delete();
		pState->m_pCurrentWinThread = NULL;
	}

	// allow cleanup of any thread local objects
	AfxTermThread();

	// allow C-runtime to cleanup, and exit the thread

	// With newer compilers (VS2010 is not affected) the crt thread code has been rewritten and
	// needs FlsAlloc() which allocates fiber local storage and permits the registration of a
	// function called when the fiber/thread terminates. Under Windows XP the FlsAlloc(callback)
	// API is missing and that causes a heap memory leak for each terminated thread.

	// In per_thread_data.cpp __acrt_initialize_ptd() calls __acrt_FlsAlloc() with destroy_fls()
	// as callback so that on thread termination the heap memory is freed.
	// The __acrt_FlsAlloc() wrapper in winapi_thunks.cpp calls TlsAlloc() if FlsAlloc() is not
	// available, but TlsAlloc() cannot register a callback!!!

	// Best would be if Microsoft called destroy_fls() before exiting the thread in
	// common_end_thread() (see thread.cpp) for the case that FlsAlloc() is missing.

	// We cannot force Microsoft to fix their code for an unsupported OS, but we can reproduce
	// a corrected common_end_thread() here:

	// 1. - sets the FLS slot to NULL so that destroy_fls() is not called anymore (destroy_fls()
	//      has been registered by FlsAlloc() for non-XP systems)
	//    - frees thread heap memory by calling destroy_fls()
	__acrt_freeptd();

	// 2. _initialized_apartment is not set as we are not a packaged app,
	//    (we do not have to call __acrt_RoUninitialize())

	// 3. We do not have to close the thread handle because we use _beginthreadex(), 
	//    (if we were using _beginthread() then we had to close the handle here)

	// 4. We have to decrement the module reference count (see create_thread_parameter() in thread.cpp)
	HMODULE hModule = NULL; // pseudo handle storing the fixed address of the module
	GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, reinterpret_cast<LPCWSTR>(WorkerThreadProc), &hModule);
	FreeLibrary(hModule); // free library to balance the above GetModuleHandleExW()
	FreeLibraryAndExitThread(hModule, nExitCode); // free library to balance create_thread_parameter()

	// 5. _endthreadex() which calls common_end_thread() is never executed because we already
	//    exited our thread at point 4. common_end_thread() must not be called because it
	//    executes __acrt_getptd_noexit() that allocates a new ptd on the heap!!!
}
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
#if _MSC_VER > 1600
		WorkerThreadCleanAndEnd(res);
#endif
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
#if _MSC_VER > 1600
		WorkerThreadCleanAndEnd(0);
#endif
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