// MDIClientWnd.cpp : implementation file
//

#include "stdafx.h"
#include "uImager.h"
#include "MDIClientWnd.h"
#include "LicenseHelper.h"
#include "MyMemDC.h"
#include "Dib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CMDIClientWnd::CMDIClientWnd()
{
	m_bFontCreated = FALSE;
	m_nFontSize = 11;
	m_crBackgroundColor = ::GetSysColor(COLOR_APPWORKSPACE);
	BYTE BackgroundColorGray = CDib::RGBToGray(	GetRValue(m_crBackgroundColor),
												GetGValue(m_crBackgroundColor),
												GetBValue(m_crBackgroundColor));
	if (BackgroundColorGray > 0xbbu)
		m_crTextColor = RGB(0,0,0);
	else
		m_crTextColor = RGB(0xff,0xff,0xff);
	m_crLinkColor = ::GetSysColor(COLOR_HOTLIGHT);
	m_crVerboseLoggingColor = RGB(0xee,0xdd,0x20);
	m_crDebugLoggingColor = RGB(0xff,0xbb,0x00);
	m_nLeftMargin = 2;
	m_nTopMargin = 0;
	m_rcLinkComputer = CRect(0,0,0,0);
	m_rcLinkLocalhost = CRect(0,0,0,0);
	m_TopLeftCornerClickTime = 0;
	m_TopRightCornerClickTime = 0;
	m_BottomRightCornerClickTime = 0;
}

CMDIClientWnd::~CMDIClientWnd()
{
}

IMPLEMENT_DYNAMIC(CMDIClientWnd, CWnd)

BEGIN_MESSAGE_MAP(CMDIClientWnd, CWnd)
	//{{AFX_MSG_MAP(CMDIClientWnd)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CMDIClientWnd::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void CMDIClientWnd::OnPaint() 
{
	// Device context for painting
	CPaintDC dc(this);
	CMyMemDC memDC(&dc);

	// Get Client Rectangle
	CRect rcClient;
	GetClientRect(&rcClient);
	
	// Erase Background
	memDC.FillSolidRect(&rcClient, m_crBackgroundColor);

	// Create Font
	if (!m_bFontCreated)
	{
		int nFontHeight = ::MulDiv(m_nFontSize, ::GetDeviceCaps(dc.GetSafeHdc(), LOGPIXELSY), 72);
		m_Font.DeleteObject();
		m_FontUnderline.DeleteObject();
		m_Font.CreateFont(	-nFontHeight,
							0, 0, 0,
							FW_NORMAL,
							FALSE,		// Italic
							FALSE,		// Underline
							FALSE,		// Strikethrough
							DEFAULT_CHARSET,
							OUT_DEFAULT_PRECIS,
							CLIP_DEFAULT_PRECIS,
							PROOF_QUALITY,
							DEFAULT_PITCH | FF_DONTCARE,
							g_szDefaultFontFace);
		m_FontUnderline.CreateFont(	-nFontHeight,
									0, 0, 0,
									FW_NORMAL,
									FALSE,		// Italic
									TRUE,		// Underline
									FALSE,		// Strikethrough
									DEFAULT_CHARSET,
									OUT_DEFAULT_PRECIS,
									CLIP_DEFAULT_PRECIS,
									PROOF_QUALITY,
									DEFAULT_PITCH | FF_DONTCARE,
									g_szDefaultFontFace);
		m_bFontCreated = TRUE;
	}

	// Select color & font
	COLORREF crOldTextColor = memDC.SetTextColor(m_crTextColor);
	int nOldBkMode = memDC.SetBkMode(TRANSPARENT);
	CFont* pOldFont = memDC.SelectObject(&m_Font);
	UINT uiOldTextAlign = memDC.SetTextAlign(VTA_BASELINE);

	// Get Text Metrics
	TEXTMETRIC TextMetrics;
	memDC.GetTextMetrics(&TextMetrics);
	int nLineHeight = TextMetrics.tmHeight;
	
	// Draw app name and version
	CRect rcDraw(	m_nLeftMargin,
					m_nTopMargin,
					rcClient.right,
					rcClient.bottom);
	rcDraw.top += TextMetrics.tmAscent;
	CString s;
	if (g_DonorEmailValidateThread.m_bNoDonation)
	{
		CString sNoDonation(ML_STRING(1734, "NO DONATION: see Help menu"));
		if (sNoDonation.GetLength() < 10)
			sNoDonation = _T("NO DONATION: see Help menu");
		if (g_nLogLevel <= 0)
			s.Format(_T("%s %s (%s)"), APPNAME_NOEXT, APPVERSION, sNoDonation);
		else
			s.Format(_T("%s %s (%s) - "), APPNAME_NOEXT, APPVERSION, sNoDonation);
	}
	else
	{
		if (g_nLogLevel <= 0)
			s.Format(_T("%s %s"), APPNAME_NOEXT, APPVERSION);
		else
			s.Format(_T("%s %s - "), APPNAME_NOEXT, APPVERSION);
	}
	CRect rcLastText = DrawTAndCalcRect(memDC, s, rcDraw);
	rcDraw.left = rcLastText.right;
	COLORREF crPrevTextColor;
	if (g_nLogLevel == 1)
	{
		crPrevTextColor = memDC.SetTextColor(m_crVerboseLoggingColor);
		s = ML_STRING(1810, "VERBOSE LOGGING");
		DrawT(memDC, s, rcDraw);
		memDC.SetTextColor(crPrevTextColor);
	}
	else if (g_nLogLevel > 1)
	{
		crPrevTextColor = memDC.SetTextColor(m_crDebugLoggingColor);
		s = ML_STRING(1811, "DEBUG LOGGING");
		DrawT(memDC, s, rcDraw);
		memDC.SetTextColor(crPrevTextColor);
	}

#ifdef VIDEODEVICEDOC
	// Draw Add Cam
	rcDraw.left = m_nLeftMargin;
	rcDraw.top += 2*nLineHeight;
	DrawT(memDC,  ML_STRING(1750, "ADD CAMERA"), rcDraw);

	// Draw point 1.
	rcDraw.top += nLineHeight;
	DrawT(memDC, ML_STRING(1751, "1. Make sure your video capture device is plugged-in and drivers are installed"), rcDraw);
	rcDraw.top += nLineHeight;
	DrawT(memDC, _T("    ") + ML_STRING(1752, "(Network cameras do not need drivers)"), rcDraw);

	// Draw point 2.
	rcDraw.top += nLineHeight;
	DrawT(memDC, ML_STRING(1753, "2. From the Camera menu select the device you want to use"), rcDraw);
	rcDraw.top += nLineHeight;
	DrawT(memDC, _T("    ") + ML_STRING(1754, "(for network cameras supply host name or ip address, port and camera type)"), rcDraw);

	// Draw point 3.
	rcDraw.top += nLineHeight;
	DrawT(memDC, ML_STRING(1755, "3. Follow the Camera Basic Settings dialog"), rcDraw);
	rcDraw.top += nLineHeight;
	DrawT(memDC, _T("    ") + ML_STRING(1756, "(if it's not popping-up select Camera Basic Settings from the Settings menu)"), rcDraw);

	// Draw point 4.
	rcDraw.top += nLineHeight;	
	DrawT(memDC, ML_STRING(1757, "4. To watch live stream / recordings enter in Web Browser's address bar") + _T(":"), rcDraw);

	// Draw indent
	rcDraw.top += nLineHeight;
	rcLastText = DrawTAndCalcRect(memDC, _T("    "), rcDraw);

	// Draw computer link
	rcDraw.left = rcLastText.right;
	CFont* pPrevFont = memDC.SelectObject(&m_FontUnderline);
	crPrevTextColor = memDC.SetTextColor(m_crLinkColor);
	if (((CUImagerApp*)::AfxGetApp())->m_nMicroApachePort == 80)
		s.Format(_T("http://%s"), ::GetComputerName());
	else
		s.Format(_T("http://%s:%d"), ::GetComputerName(), ((CUImagerApp*)::AfxGetApp())->m_nMicroApachePort);
	rcLastText = DrawTAndCalcRect(memDC, s, rcDraw);
	memDC.SelectObject(pPrevFont);
	memDC.SetTextColor(crPrevTextColor);
	m_rcLinkComputer = rcLastText;
	m_rcLinkComputer.top -= TextMetrics.tmAscent;
	m_rcLinkComputer.bottom -= TextMetrics.tmAscent;

	// Draw " or "
	rcDraw.left = rcLastText.right;
	rcLastText = DrawTAndCalcRect(memDC, _T(" ") + ML_STRING(1868, "or") + _T(" "), rcDraw);

	// Draw localhost link
	rcDraw.left = rcLastText.right;		
	pPrevFont = memDC.SelectObject(&m_FontUnderline);
	crPrevTextColor = memDC.SetTextColor(m_crLinkColor);
	if (((CUImagerApp*)::AfxGetApp())->m_nMicroApachePort == 80)
		s = _T("http://localhost");
	else
		s.Format(_T("http://localhost:%d"), ((CUImagerApp*)::AfxGetApp())->m_nMicroApachePort);
	rcLastText = DrawTAndCalcRect(memDC, s, rcDraw);
	memDC.SelectObject(pPrevFont);
	memDC.SetTextColor(crPrevTextColor);
	m_rcLinkLocalhost = rcLastText;
	m_rcLinkLocalhost.top -= TextMetrics.tmAscent;
	m_rcLinkLocalhost.bottom -= TextMetrics.tmAscent;

	// Draw Remove Cams
	rcDraw.left = m_nLeftMargin;
	rcDraw.top += 2*nLineHeight;
	DrawT(memDC, ML_STRING(1801, "REMOVE CAMERAS"), rcDraw);

	// Draw point 1.
	rcDraw.top += nLineHeight;
	DrawT(memDC, ML_STRING(1803, "1. Choose Remove Cameras from the Camera menu"), rcDraw);
#endif

	// Clean-Up
	memDC.SetTextAlign(uiOldTextAlign);
	memDC.SelectObject(pOldFont);
	memDC.SetBkMode(nOldBkMode);
	memDC.SetTextColor(crOldTextColor);
}

void CMDIClientWnd::DrawT(CMyMemDC& memDC, CString s, CRect rcDraw)
{
	::DrawTextEx(	memDC.GetSafeHdc(),
					s.GetBuffer(s.GetLength() + 1),
					s.GetLength(),
					&rcDraw,
					DT_LEFT | DT_NOCLIP | DT_NOPREFIX,
					NULL);
	s.ReleaseBuffer();
}

CRect CMDIClientWnd::DrawTAndCalcRect(CMyMemDC& memDC, CString s, CRect rcDraw)
{
	CRect rcLastText(rcDraw);
	::DrawTextEx(	memDC.GetSafeHdc(),
					s.GetBuffer(s.GetLength() + 1),
					s.GetLength(),
					&rcLastText,
					DT_CALCRECT | DT_LEFT | DT_NOCLIP | DT_NOPREFIX,
					NULL);
	s.ReleaseBuffer();
	DrawT(memDC, s, rcDraw);
	return rcLastText;
}

void CMDIClientWnd::OnSize(UINT nType, int cx, int cy) 
{
	Invalidate();
	CWnd::OnSize(nType, cx, cy);
}

BOOL CMDIClientWnd::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	// If Wait Cursor leave it!
	if (((CUImagerApp*)::AfxGetApp())->IsWaitCursor())
	{
		RestoreWaitCursor();
		return TRUE;
	}
#ifdef VIDEODEVICEDOC
	else
	{
		CPoint pt;
		::GetSafeCursorPos(&pt);
		ScreenToClient(&pt); // Client coordinates of mouse position
		if (nHitTest == HTCLIENT && pWnd->GetSafeHwnd() == GetSafeHwnd() &&
			(m_rcLinkComputer.PtInRect(pt) || m_rcLinkLocalhost.PtInRect(pt)))
		{
			// IDC_HAND_CURSOR has to be in Resource
			HCURSOR hCursor = ::AfxGetApp()->LoadCursor(IDC_HAND_CURSOR);	
			ASSERT(hCursor);
			::SetCursor(hCursor);		
			return TRUE;
		}
		else
			return CWnd::OnSetCursor(pWnd, nHitTest, message);
	}
#else
	else
		return CWnd::OnSetCursor(pWnd, nHitTest, message);
#endif	
}

#ifdef VIDEODEVICEDOC
void CMDIClientWnd::ViewWeb(CString sHost)
{
	if (!sHost.IsEmpty())
	{
		CString sUrl;
		if (((CUImagerApp*)::AfxGetApp())->m_nMicroApachePort == 80)
			sUrl.Format(_T("http://%s"), sHost);
		else
			sUrl.Format(_T("http://%s:%d"), sHost, ((CUImagerApp*)::AfxGetApp())->m_nMicroApachePort);
		sUrl = ::UrlEncode(sUrl, FALSE);
		BeginWaitCursor();
		::ShellExecute(	NULL,
						_T("open"),
						sUrl,
						NULL,
						NULL,
						SW_SHOWNORMAL);
		EndWaitCursor();
	}
}
#endif

// Disable warning C4723: potential divide by 0
#pragma warning(disable:4723)
void CMDIClientWnd::Crashme()
{
	// Division by zero to test procdump.exe
	int a = 0;
	int b = 5 / a;
	CString s;
	s.Format(_T("Divide by 0 is %d"), b);
	::AfxMessageBox(s);
	#pragma warning(default:4723)
}

void CMDIClientWnd::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// Links click
#ifdef VIDEODEVICEDOC
	if (m_rcLinkComputer.PtInRect(point))
		ViewWeb(::GetComputerName());
	else if (m_rcLinkLocalhost.PtInRect(point))
		ViewWeb(_T("localhost"));
#endif

	// Crash test
	// (click the four corners in sequence)
	CRect rcClient;
	GetClientRect(&rcClient);
	const LONGLONG nClickSpeedSec = 3;
	const int nEdge = 30;
	const int nMinWndEdge = 120;
	if (rcClient.Width() > nMinWndEdge && rcClient.Height() > nMinWndEdge)
	{
		// 1. Top-left corner
		if (CRect(0, 0, nEdge, nEdge).PtInRect(point))
		{
			m_TopLeftCornerClickTime = CTime::GetCurrentTime();
		}
		// 2. Top-right corner
		else if (CRect(rcClient.Width()-nEdge, 0, rcClient.Width(), nEdge).PtInRect(point))
		{
			CTime CurrentTime = CTime::GetCurrentTime();
			LONGLONG nDiffSec = (CurrentTime - m_TopLeftCornerClickTime).GetTotalSeconds();
			if (nDiffSec >= 0 && nDiffSec <= nClickSpeedSec)
				m_TopRightCornerClickTime = CurrentTime;
		}
		// 3. Bottom-right corner
		else if (CRect(rcClient.Width()-nEdge, rcClient.Height()-nEdge, rcClient.Width(), rcClient.Height()).PtInRect(point))
		{
			CTime CurrentTime = CTime::GetCurrentTime();
			LONGLONG nDiffSec = (CurrentTime - m_TopRightCornerClickTime).GetTotalSeconds();
			if (nDiffSec >= 0 && nDiffSec <= nClickSpeedSec)
				m_BottomRightCornerClickTime = CurrentTime;
		}
		// 4. Bottom-left corner
		else if (CRect(0, rcClient.Height()-nEdge, nEdge, rcClient.Height()).PtInRect(point))
		{
			CTime CurrentTime = CTime::GetCurrentTime();
			LONGLONG nDiffSec = (CurrentTime - m_BottomRightCornerClickTime).GetTotalSeconds();
			if (nDiffSec >= 0 && nDiffSec <= nClickSpeedSec)
				Crashme();
		}
	}

	CWnd::OnLButtonUp(nFlags, point);
}
