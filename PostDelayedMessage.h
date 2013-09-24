#if !defined(AFX_POSTDELAYEDMESSAGE_H__FFD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_)
#define AFX_POSTDELAYEDMESSAGE_H__FFD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_

#pragma once

#include "WorkerThread.h"
#include "MainFrm.h"

// Each 30 milliseconds the Exit() function
// checks whether all threads have exited
#define EXIT_CHECK_INTERVAL		30

// Post Delayed Message Thread Class
class CPostDelayedMessageThread : CWorkerThread
{
	public:
		static void Init();
		static void Exit();
		static BOOL PostDelayedMessage(	HWND hWnd,
										UINT uiMsg,
										DWORD dwMilliseconds,
										WPARAM wParam,
										LPARAM lParam);

	protected:
		CPostDelayedMessageThread(	HWND hWnd,
									UINT uiMsg,
									DWORD dwMilliseconds,
									WPARAM wParam,
									LPARAM lParam) :
									m_hWnd(hWnd),
									m_uiMsg(uiMsg),
									m_dwMilliseconds(dwMilliseconds),
									m_wParam(wParam),
									m_lParam(lParam)
									{m_bAutoDelete = TRUE;};
		virtual ~CPostDelayedMessageThread(){;};
		int Work();

		static HANDLE volatile m_hExitEvent;
		static volatile LONG m_lCount;
		static volatile LONG m_lExit;

		HWND m_hWnd;
		UINT m_uiMsg;
		DWORD m_dwMilliseconds;
		WPARAM m_wParam;
		LPARAM m_lParam;
};


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_POSTDELAYEDMESSAGE_H__FFD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_)