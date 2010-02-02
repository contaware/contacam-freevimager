#include "stdafx.h"
#include "resource.h"
#include "uImager.h"
#include "MainFrm.h"
#include "trayicon.h"
#include <afxpriv.h> // for AfxLoadString

IMPLEMENT_DYNAMIC(CTrayIcon, CCmdTarget)

HWND CTrayIcon::m_hWndInvisible = NULL;
static WNDPROC g_pOldDefWndProc = NULL;

CTrayIcon::CTrayIcon(UINT uID)
{
	m_bMinimizedToTray = FALSE;

	// Initialize NOTIFYICONDATA
	memset(&m_nid, 0 , sizeof(m_nid));
	m_nid.cbSize = sizeof(m_nid);
	m_nid.uID = uID;	// never changes after construction

	// Use resource string as tip if there is one
	::AfxLoadString(uID, m_nid.szTip, sizeof(m_nid.szTip) / sizeof(TCHAR));
}

CTrayIcon::~CTrayIcon()
{
	SetIcon(0); // remove icon from system tray
	if (::IsWindow(m_nid.hWnd))
		::SetParent(m_nid.hWnd, NULL);
	if (::IsWindow(m_hWndInvisible))
	{
		::SetWindowLongPtr(	m_hWndInvisible,
							GWLP_WNDPROC, 
							(LONG_PTR)g_pOldDefWndProc);
		::DestroyWindow(m_hWndInvisible);
	}
}

void CTrayIcon::SetWnd(CWnd* pWnd, UINT uCbMsg)
{
	// If the following assert fails, you're probably
	// calling me before you created your window. Oops.
	ASSERT(pWnd==NULL || ::IsWindow(pWnd->GetSafeHwnd()));
	m_nid.hWnd = pWnd->GetSafeHwnd();

	ASSERT(uCbMsg==0 || uCbMsg>=WM_USER);
	m_nid.uCallbackMessage = uCbMsg;
}

BOOL CTrayIcon::GetWndPlacement(WINDOWPLACEMENT* pWndPlacement)
{
	if (pWndPlacement)
	{
		memcpy(pWndPlacement, &m_WndPlacement, sizeof(WINDOWPLACEMENT));
		return TRUE;
	}
	else
		return FALSE;
}

void CTrayIcon::SetWndPlacement(WINDOWPLACEMENT* pWndPlacement)
{
	if (pWndPlacement)
		memcpy(&m_WndPlacement, pWndPlacement, sizeof(WINDOWPLACEMENT));
}

BOOL CTrayIcon::SetIcon(UINT uID)
{ 
	HICON hicon = NULL;
	if (uID)
	{
		::AfxLoadString(uID, m_nid.szTip, sizeof(m_nid.szTip) / sizeof(TCHAR));
		hicon = ::AfxGetApp()->LoadIcon(uID);
	}
	return SetIcon(hicon, NULL);
}

BOOL CTrayIcon::SetIcon(HICON hicon, LPCTSTR lpTip) 
{
	UINT msg;
	m_nid.uFlags = 0;

	// Set the icon
	if (hicon)
	{
		// Add or replace icon in system tray
		msg = m_nid.hIcon ? NIM_MODIFY : NIM_ADD;
		m_nid.hIcon = hicon;
		m_nid.uFlags |= NIF_ICON;
	}
	// Remove icon from tray
	else
	{
		if (m_nid.hIcon==NULL)
			return TRUE;		// already deleted
		msg = NIM_DELETE;
	}

	// Use the tip, if any
	if (lpTip)
	{
		_tcsncpy(m_nid.szTip, lpTip, sizeof(m_nid.szTip) / sizeof(TCHAR));
		m_nid.szTip[sizeof(m_nid.szTip) / sizeof(TCHAR) - 1] = _T('\0');
	}
	if (m_nid.szTip[0])
		m_nid.uFlags |= NIF_TIP;

	// Use callback if any
	if (m_nid.uCallbackMessage && m_nid.hWnd)
		m_nid.uFlags |= NIF_MESSAGE;

	// Do it
	BOOL bRet = ::Shell_NotifyIcon(msg, &m_nid);
	if (msg == NIM_DELETE || !bRet)
		m_nid.hIcon = NULL;	// failed
	return bRet;
}

// Default event handler handles right-menu and doubleclick.
LRESULT CTrayIcon::OnTrayNotification(WPARAM wID, LPARAM lEvent)
{
	if (wID!=m_nid.uID ||
		(lEvent!=WM_RBUTTONUP && lEvent!=WM_LBUTTONDBLCLK))
		return 0;

    return 1; // handled
}

LRESULT CALLBACK NewWndProc(HWND hwnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	// Send to the mainframe because if minimized to tray
	// mainframe has a parent window and will not get the
	// shutdown messages!
	if (uiMsg == WM_QUERYENDSESSION ||
		uiMsg == WM_ENDSESSION)
		return ::SendMessage(::AfxGetMainFrame()->GetSafeHwnd(), uiMsg, wParam, lParam);
	else
		return ::CallWindowProc(g_pOldDefWndProc, hwnd, uiMsg, wParam, lParam);
}

BOOL CTrayIcon::RemoveTaskbarIcon(HWND hWnd)
{
    // Create static invisible window
    if (!::IsWindow(m_hWndInvisible))
    {
		m_hWndInvisible = ::CreateWindowEx(0, _T("Static"), _T(""), WS_POPUP,
				CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
				NULL, 0, NULL, 0);
		if (!m_hWndInvisible)
			return FALSE;
    }
	g_pOldDefWndProc = (WNDPROC)::SetWindowLongPtr(	m_hWndInvisible,
													GWLP_WNDPROC, 
													(LONG_PTR)NewWndProc);
    ::SetParent(hWnd, m_hWndInvisible);

    return TRUE;
}

void CTrayIcon::MinimizeToTray()
{
	memset(&m_WndPlacement, 0, sizeof(m_WndPlacement));
	m_WndPlacement.length = sizeof(m_WndPlacement);
	::GetWindowPlacement(m_nid.hWnd, &m_WndPlacement);
	if (m_WndPlacement.showCmd == SW_SHOWMINIMIZED)
		m_WndPlacement.showCmd = SW_SHOWNORMAL;
    RemoveTaskbarIcon(m_nid.hWnd);
	::SetWindowLong(m_nid.hWnd, GWL_STYLE, ::GetWindowLong(m_nid.hWnd, GWL_STYLE) & ~WS_VISIBLE);
	m_bMinimizedToTray = TRUE;
}

void CTrayIcon::MaximizeFromTray()
{
	m_bMinimizedToTray = FALSE;
	if (::IsWindow(m_hWndInvisible))
	{
		::SetWindowLongPtr(	m_hWndInvisible,
							GWLP_WNDPROC, 
							(LONG_PTR)g_pOldDefWndProc);
	}
	::SetParent(m_nid.hWnd, NULL);
	::SetWindowLong(m_nid.hWnd, GWL_STYLE, ::GetWindowLong(m_nid.hWnd, GWL_STYLE) | WS_VISIBLE);
	::SetWindowPlacement(m_nid.hWnd, &m_WndPlacement);
    ::RedrawWindow(	m_nid.hWnd,
					NULL, NULL,
					RDW_UPDATENOW | RDW_ALLCHILDREN |
					RDW_FRAME | RDW_INVALIDATE | RDW_ERASE);

    // Move focus away and back again to ensure taskbar icon is recreated
	if (::IsWindow(m_hWndInvisible))
		::SetActiveWindow(m_hWndInvisible);
    ::SetActiveWindow(m_nid.hWnd);
	::SetForegroundWindow(m_nid.hWnd);
}
