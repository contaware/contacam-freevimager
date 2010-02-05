#include "stdafx.h"
#include "PostDelayedMessage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

HANDLE volatile CPostDelayedMessageThread::m_hExitEvent = NULL;
volatile LONG CPostDelayedMessageThread::m_lCount = 0;
volatile LONG CPostDelayedMessageThread::m_lExit = 0;

void CPostDelayedMessageThread::Init()
{
	if (m_hExitEvent == NULL)
	{
		m_hExitEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
		m_lCount = 0;
		m_lExit = 0;
	}
}
	
void CPostDelayedMessageThread::Exit()
{
	if (m_hExitEvent)
	{
		::InterlockedExchange(&m_lExit, 1);
		::SetEvent(m_hExitEvent);

		while (m_lCount > 0)
			::Sleep(EXIT_CHECK_INTERVAL);

		::CloseHandle(m_hExitEvent);
		m_hExitEvent = NULL;
	}
}

BOOL CPostDelayedMessageThread::PostDelayedMessage(	HWND hWnd,
													UINT uiMsg,
													DWORD dwMilliseconds,
													WPARAM wParam,
													LPARAM lParam)
{
	::InterlockedIncrement(&m_lCount);

	if (m_lExit || m_hExitEvent == NULL)
	{
		::InterlockedDecrement(&m_lCount);
		return FALSE;
	}

	CPostDelayedMessageThread* p =
		new CPostDelayedMessageThread(	hWnd,
										uiMsg,
										dwMilliseconds,
										wParam,
										lParam);
	if (!p)
	{
		::InterlockedDecrement(&m_lCount);
		return FALSE;
	}
	else if (!p->Start())
	{
		delete p;
		::InterlockedDecrement(&m_lCount);
		return FALSE;
	}
	else
		return TRUE;
}

int CPostDelayedMessageThread::Work()	
{
	DWORD Event = ::WaitForSingleObject(m_hExitEvent, m_dwMilliseconds);
	switch (Event)
	{
		// Exit
		case WAIT_OBJECT_0 :		break;

		// Timeout
		case WAIT_TIMEOUT :			if (::IsWindow(m_hWnd))
									{
										::PostMessage(	m_hWnd,
														m_uiMsg,
														m_wParam,
														m_lParam);
									}
									break;

		default:					break;
									
	};

	::InterlockedDecrement(&m_lCount);

	return 0;
}