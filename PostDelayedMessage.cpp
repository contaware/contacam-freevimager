#include "stdafx.h"
#include "PostDelayedMessage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

HANDLE volatile CPostDelayedMessageThread::m_hExitEvent = NULL;
std::atomic<int> CPostDelayedMessageThread::m_nCount(0);
std::atomic<BOOL> CPostDelayedMessageThread::m_bExit(FALSE);

void CPostDelayedMessageThread::Init()
{
	if (m_hExitEvent == NULL)
	{
		m_hExitEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
		m_nCount = 0;
		m_bExit = FALSE;
	}
}
	
void CPostDelayedMessageThread::Exit()
{
	if (m_hExitEvent)
	{
		m_bExit = TRUE;
		::SetEvent(m_hExitEvent);

		while (m_nCount > 0)
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
	++m_nCount;

	if (m_bExit || m_hExitEvent == NULL)
	{
		--m_nCount;
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
		--m_nCount;
		return FALSE;
	}
	else if (!p->Start())
	{
		delete p;
		--m_nCount;
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

	--m_nCount;

	return 0;
}