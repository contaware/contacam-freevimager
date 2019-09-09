#include "stdafx.h"
#include "MainFrm.h"
#include "resource.h"
#include "NotificationWnd.h"

#ifndef WP_CLOSEBUTTON
#define WP_CLOSEBUTTON 18
#endif

#ifndef CBS_PUSHED
#define CBS_PUSHED 3
#endif

#ifndef CBS_HOT
#define CBS_HOT 2
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const UINT NOTIFICATIONWND_TIMER_ID = 1;

BEGIN_MESSAGE_MAP(CNotificationWnd, CFrameWnd)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_SETCURSOR()
	ON_WM_CLOSE()
	ON_WM_CREATE()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

CNotificationWnd::CNotificationWnd(const CString& sTitle, const CString& sText, int nWidth, int nHeight, DWORD dwWaitTimeMs/*=0U*/)
{
	// Title
	m_sTitle = sTitle;
	m_colorTitle = ::GetSysColor(COLOR_WINDOWTEXT);
	m_TitleIcon = static_cast<HICON>(::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR));

	// Text
	m_sText = sText;
	m_bTextHot = IsBodyTextClickable(m_sText);
	if (m_bTextHot)
		m_colorText = ::GetSysColor(COLOR_HOTLIGHT); // color for a hyperlink, the associated background color is COLOR_WINDOW
	else
		m_colorText = ::GetSysColor(COLOR_WINDOWTEXT);
	m_colorBackground = ::GetSysColor(COLOR_WINDOW);
	m_dwTextStyle = DT_NOPREFIX | DT_CENTER | DT_WORDBREAK | DT_EDITCONTROL;

	// Size
	m_nWidth = ::SystemDPIScale(nWidth);
	m_nHeight = ::SystemDPIScale(nHeight);

	// Close timeout, 0 means show infinitely
	m_dwWaitTimeMs = dwWaitTimeMs;

	m_nTimerID = 0;
	m_bClosePressed = FALSE;
	m_bCloseHot = FALSE;
	m_bBodyTextPressed = FALSE;
	m_bSafeToClose = FALSE;

	// Dynamically pull in the uxtheme functions
	m_hUXTheme = LoadLibraryFromSystem32(_T("UxTheme.dll"));
	if (m_hUXTheme != NULL)
	{
		m_lpfnOpenThemeData = reinterpret_cast<LPOPENTHEMEDATA>(GetProcAddress(m_hUXTheme, "OpenThemeData"));
		m_lpfnDrawThemeBackground = reinterpret_cast<LPDRAWTHEMEBACKGROUND>(GetProcAddress(m_hUXTheme, "DrawThemeBackground"));
		m_lpfnCloseThemeData = reinterpret_cast<LPCLOSETHEMEDATA>(GetProcAddress(m_hUXTheme, "CloseThemeData"));

		if ((m_lpfnOpenThemeData == NULL) || (m_lpfnDrawThemeBackground == NULL) || (m_lpfnCloseThemeData == NULL))
		{
			m_lpfnOpenThemeData = NULL;
			m_lpfnDrawThemeBackground = NULL;
			m_lpfnCloseThemeData = NULL;

			FreeLibrary(m_hUXTheme);
			m_hUXTheme = NULL;
		}
	}
	else
	{
		m_lpfnOpenThemeData = NULL;
		m_lpfnDrawThemeBackground = NULL;
		m_lpfnCloseThemeData = NULL;
	}

	// Load up the hand cursor
	m_cursorHand = LoadCursor(NULL, IDC_HAND);
}

CNotificationWnd::~CNotificationWnd()
{
	if (m_hUXTheme != NULL)
	{
		FreeLibrary(m_hUXTheme);
		m_hUXTheme = NULL;
	}
	if (m_cursorHand != NULL)
	{
		DestroyCursor(m_cursorHand);
		m_cursorHand = NULL;
	}
	if (m_TitleIcon != NULL)
	{
		DestroyIcon(m_TitleIcon);
		m_TitleIcon = NULL;
	}
}

HMODULE CNotificationWnd::LoadLibraryFromSystem32(LPCTSTR lpFileName)
{
	// Get the Windows System32 directory
	TCHAR szFullPath[_MAX_PATH];
	szFullPath[0] = _T('\0');
	if (GetSystemDirectory(szFullPath, _countof(szFullPath)) == 0)
		return NULL;

	// Setup the full path and delegate to LoadLibrary
	#pragma warning(suppress: 6102) //There is a bug with the SAL annotation of GetSystemDirectory in the Windows 8.1 SDK
	_tcscat_s(szFullPath, _countof(szFullPath), _T("\\"));
	_tcscat_s(szFullPath, _countof(szFullPath), lpFileName);
	return LoadLibrary(szFullPath);
}

CRect CNotificationWnd::CalculatePopupPosition()
{
	// Get the working area for the current monitor
	// Note: displaying the popup window in another monitor with a different DPI doesn't work correctly
	//       (size and position would be wrong, tested in Windows 10). Adding WS_CAPTION fixes the problem
	//       but displays a bad looking inactive title bar. The solution is to always popup to the same
	//       monitor instead of the primary monitor
	MONITORINFO monInfo;
	monInfo.cbSize = sizeof(monInfo);
	HMONITOR hMonitor = ::MonitorFromWindow(::AfxGetMainFrame()->GetSafeHwnd(), MONITOR_DEFAULTTONEAREST);
	if (::GetMonitorInfo(hMonitor, &monInfo))
	{
		return CRect(	monInfo.rcWork.right - m_nWidth,
						monInfo.rcWork.bottom - m_nHeight,
						monInfo.rcWork.right,
						monInfo.rcWork.bottom);
	}
	else
		return CRect(0, 0, 0, 0);
}

void CNotificationWnd::CreateFonts()
{
	// Get default UI font
	CFont defaultGUIFont;
	NONCLIENTMETRICS ncm = { sizeof(NONCLIENTMETRICS) };
	VERIFY(::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0));
	defaultGUIFont.CreateFontIndirect(&(ncm.lfMessageFont));

	// Title
	LOGFONT lf;
	defaultGUIFont.GetLogFont(&lf);
	lf.lfWeight = FW_BOLD;
	m_fontTitle.CreateFontIndirect(&lf);

	// Message
	defaultGUIFont.GetLogFont(&lf);
	if (m_bTextHot)
		lf.lfUnderline = TRUE;
	m_fontText.CreateFontIndirect(&lf);
}

BOOL CNotificationWnd::Show()
{
	// Validate
	ASSERT(m_sText.GetLength());

	// Position
	CRect rcPopupPos(CalculatePopupPosition());

	// Create the window without setting a parent window so that we can decide
	// when to close it, but we have to remember to call Close() when the app exits!
	if (!__super::Create(NULL, m_sTitle, WS_POPUP, rcPopupPos, NULL/*No Parent Window*/, NULL, WS_EX_TOPMOST | WS_EX_TOOLWINDOW))
		return FALSE;

	// Create the fonts
	CreateFonts();

	// Calculate the position of the close button
	CRect rectClient;
	GetClientRect(&rectClient);
	m_rectClose = CRect(rectClient.right - ::SystemDPIScale(26), ::SystemDPIScale(5), rectClient.right - ::SystemDPIScale(13), ::SystemDPIScale(18));

	// Calculate the position of the icon
	if (m_TitleIcon)
		m_rectIcon = CRect(rectClient.left + 8, 5, GetSystemMetrics(SM_CXSMICON) + rectClient.left + 8, GetSystemMetrics(SM_CYSMICON) + 5);
	else
		m_rectIcon = CRect(-1, -1, -1, -1);

	// Calculate the position of the title text
	int nOffset;
	if (m_sTitle.GetLength())
	{
		CDC* pDC = GetDC();
		CFont* pOldFont = pDC->SelectObject(&m_fontTitle);

		CSize sizeTitle = pDC->GetTextExtent(m_sTitle);
		m_rectTitle = CRect(max(5, m_rectIcon.right + 5), 5, m_rectClose.left - 5, 5 + sizeTitle.cy);
		nOffset = max(max(m_rectTitle.bottom + 5, m_rectClose.bottom + 5), m_rectIcon.bottom + 5);

		if (pOldFont)
			pDC->SelectObject(pOldFont);

		ReleaseDC(pDC);
	}
	else
	{
		nOffset = m_rectClose.bottom + 2;
		m_rectTitle = CRect(-1, -1, -1, -1);
	}

	// Calculate the position of the body text
	int nBodyTextRight;
	nBodyTextRight = rectClient.right - 7;
	m_rectBodyText = CRect(7, nOffset, nBodyTextRight, nOffset);
	CDC* pDC = GetDC();
	CFont* pOldFont = pDC->SelectObject(&m_fontText);
	pDC->DrawText(m_sText, &m_rectBodyText, m_dwTextStyle | DT_CALCRECT);
	m_rectBodyText.right = nBodyTextRight;
	int nBodyHeight = m_rectBodyText.Height();
	int nNewTop = (m_nHeight - nBodyHeight - nOffset - 5) / 2 + nOffset;
	m_rectBodyText = CRect(m_rectBodyText.left, nNewTop, m_rectBodyText.right, nNewTop + nBodyHeight);
	if (m_rectBodyText.top < nOffset)
		m_rectBodyText.top = nOffset;
	if (m_rectBodyText.bottom > (m_nHeight - 7))
		m_rectBodyText.bottom = (m_nHeight - 7);
	if (pOldFont)
		pDC->SelectObject(pOldFont);
	ReleaseDC(pDC);

	// Set the window position
	SetWindowPos(NULL, rcPopupPos.left, rcPopupPos.top, rcPopupPos.Width(), rcPopupPos.Height(), SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);

	// Create the auto-closing timer
	if (m_dwWaitTimeMs > 0)
		m_nTimerID = SetTimer(NOTIFICATIONWND_TIMER_ID, m_dwWaitTimeMs, NULL);

	// Show the window (without activation)
	ShowWindow(SW_SHOWNOACTIVATE);

	return TRUE;
}

void CNotificationWnd::Close()
{
	m_bSafeToClose = TRUE;
	DestroyWindow();
}

void CNotificationWnd::OnDestroy() 
{
	// Kill the timer if active
	if (m_nTimerID)
	{
		KillTimer(m_nTimerID);
		m_nTimerID = 0;
	}
  
	// Let the base class do its thing
	__super::OnDestroy();
}

void CNotificationWnd::PostNcDestroy()
{
	// Let CFrameWnd::PostNcDestroy() do the "delete this;"
	__super::PostNcDestroy();
}

void CNotificationWnd::OnTimer(UINT_PTR nIDEvent) 
{
	if (nIDEvent == m_nTimerID)
	{
		CRect rectWindow;
		GetWindowRect(&rectWindow);
		CPoint cursorPos;
		::GetSafeCursorPos(&cursorPos);
		if (!rectWindow.PtInRect(cursorPos))
			HandleClosing();
	}
	else
	{
		// Let the base class do its thing
		__super::OnTimer(nIDEvent);
	}
}

void CNotificationWnd::OnPaint() 
{
	// Create the device context for painting
	CPaintDC dc(this);

	// Get the client area
	CRect rectClient;
	GetClientRect(&rectClient);
	int nClientWidth = rectClient.Width();
	int nClientHeight = rectClient.Height();

	// Create a memory device context to implement our double buffered painting
	CDC memDC;
	memDC.CreateCompatibleDC(&dc);
	CBitmap bmp;
	bmp.CreateCompatibleBitmap(&dc, nClientWidth, nClientHeight);
	CBitmap* pOldBitmap = memDC.SelectObject(&bmp);

	// Background
	memDC.FillSolidRect(&rectClient, m_colorBackground);

	// Draw the icon
	if (m_TitleIcon != NULL)
		DrawIconEx(memDC.GetSafeHdc(), m_rectIcon.left, m_rectIcon.top, m_TitleIcon, m_rectIcon.Width(), m_rectIcon.Height(), 0, NULL, DI_NORMAL); 
  
	// Draw the close button
	DrawCloseButton(&memDC); 

	// Draw text transparently
	int nOldBkMode = memDC.SetBkMode(TRANSPARENT);

	// Draw the title text	
	if (m_sTitle.GetLength())
	{ 
		CFont* pOldFont = memDC.SelectObject(&m_fontTitle);
		COLORREF nOldColor = memDC.SetTextColor(m_colorTitle);
		memDC.DrawText(m_sTitle, m_rectTitle, DT_NOPREFIX | DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS);
		memDC.SetTextColor(nOldColor);
		if (pOldFont)
			memDC.SelectObject(pOldFont);
	}

	// Draw the body text
	CFont* pOldFont = memDC.SelectObject(&m_fontText);
	COLORREF nOldColor = memDC.SetTextColor(m_colorText);
	memDC.DrawText(m_sText, m_rectBodyText, m_dwTextStyle);
	memDC.SetTextColor(nOldColor);
	if (pOldFont)
		memDC.SelectObject(pOldFont);

	// Restore background mode
	memDC.SetBkMode(nOldBkMode);

	// Transfer the memory DC to the screen  
	dc.BitBlt(0, 0, nClientWidth, nClientHeight, &memDC, 0, 0, SRCCOPY);
  
	// Restore the DC	
	memDC.SelectObject(pOldBitmap);
}

BOOL CNotificationWnd::OnEraseBkgnd(CDC* /*pDC*/) 
{
	// Do not do any background drawing since all our drawing is done in OnPaint
	return TRUE;
}

void CNotificationWnd::DrawCloseButton(CDC* pDC)
{
	if (IsAppThemed())
		DrawThemeCloseButton(pDC);
	else
		DrawLegacyCloseButton(pDC);
}

void CNotificationWnd::DrawThemeCloseButton(CDC* pDC)
{
	// Validate our parameters
	AFXASSUME(m_lpfnOpenThemeData != NULL);

	HTHEME hTheme = m_lpfnOpenThemeData(GetSafeHwnd(), L"Window");
	if (hTheme == NULL)
		DrawLegacyCloseButton(pDC);
	else
	{
		AFXASSUME(m_lpfnCloseThemeData != NULL);
		AFXASSUME(m_lpfnDrawThemeBackground != NULL);

		m_lpfnDrawThemeBackground(hTheme, pDC->GetSafeHdc(), WP_CLOSEBUTTON, m_bClosePressed ? CBS_PUSHED : (m_bCloseHot ? CBS_HOT : 0), m_rectClose, NULL);

		m_lpfnCloseThemeData(hTheme);
	}
}

void CNotificationWnd::DrawLegacyCloseButton(CDC* pDC)
{
	if (m_bClosePressed)
		pDC->DrawFrameControl(&m_rectClose, DFC_CAPTION, DFCS_CAPTIONCLOSE | DFCS_PUSHED);
	else
		pDC->DrawFrameControl(&m_rectClose, DFC_CAPTION, DFCS_CAPTIONCLOSE);
}

void CNotificationWnd::OnLButtonDown(UINT /*nFlags*/, CPoint point) 
{
	if (m_rectClose.PtInRect(point))
	{
		m_bClosePressed = TRUE;
		InvalidateRect(&m_rectClose);
	}
	else if (m_bTextHot && m_rectBodyText.PtInRect(point))
	{
		m_bBodyTextPressed = TRUE;
	}
}

void CNotificationWnd::HandleClosing()
{
	// Kill the timer if running
	if (m_nTimerID)
	{
		KillTimer(m_nTimerID);
		m_nTimerID = 0;
	}

	// Just hide ourselves
	ShowWindow(SW_HIDE);
}

BOOL CNotificationWnd::IsBodyTextClickable(const CString& sText)
{
	return (::IsExistingFile(sText)			||
			::IsExistingDir(sText)			||
			sText.Find(_T("http://")) == 0	||
			sText.Find(_T("https://")) == 0	||
			sText.Find(ML_STRING(1570, "In Power Options disable: ")) == 0);
}

void CNotificationWnd::OnLButtonUp(UINT /*nFlags*/, CPoint point) 
{
	if (m_bClosePressed)
	{
		m_bClosePressed = FALSE;
		InvalidateRect(&m_rectClose);
		if (m_rectClose.PtInRect(point))
			HandleClosing();
	}
	else if (m_bBodyTextPressed)
	{
		m_bBodyTextPressed = FALSE;
		if (m_rectBodyText.PtInRect(point))
		{
			if (IsBodyTextClickable(m_sText))
			{
				if (m_sText.Find(ML_STRING(1570, "In Power Options disable: ")) == 0)
					::ShellExecute(NULL, NULL, _T("control.exe"), _T("/name Microsoft.PowerOptions /page pagePlanSettings"), NULL, SW_SHOWNORMAL);
				else
					::ShellExecute(NULL, _T("open"), m_sText, NULL, NULL, SW_SHOWNORMAL);

				::AfxGetMainFrame()->CloseNotificationWnd();
			}
		}
	}
}

BOOL CNotificationWnd::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	const MSG* pMsg = GetCurrentMessage();
	CPoint pt(pMsg->pt);
	ScreenToClient(&pt);

	if (m_rectClose.PtInRect(pt) || (m_bTextHot && m_rectBodyText.PtInRect(pt)))
	{
		SetCursor(m_cursorHand);
		return TRUE;
	}
	else
		return __super::OnSetCursor(pWnd, nHitTest, message);
}

void CNotificationWnd::OnClose() 
{
	// Only really close the window if the m_bSafeToClose flag
	// is set. Doing this ensures that client code will always
	// has a valid pointer to us, which they can then use to 
	// synchronously close this instance via the Close() method
	if (m_bSafeToClose)
		__super::OnClose();
	else
		HandleClosing(); // like pressing ALT+F4
}

int CNotificationWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	// Let the base class do its thing
	if (__super::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

void CNotificationWnd::OnMouseMove(UINT nFlags, CPoint point) 
{
	// Let the base class do its thing
	__super::OnMouseMove(nFlags, point);

	// See if we are over the close button and if so make it "hot"
	// and force a redraw of it
	CPoint ptCursor;
	::GetSafeCursorPos(&ptCursor);
	ScreenToClient(&ptCursor);
	BOOL bNewCloseHot = m_rectClose.PtInRect(ptCursor);
	if (bNewCloseHot != m_bCloseHot)
	{
		m_bCloseHot = bNewCloseHot;
		InvalidateRect(m_rectClose);
	} 
}
