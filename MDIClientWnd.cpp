// MDIClientWnd.cpp : implementation file
//

#include "stdafx.h"
#include "uImager.h"
#include "MDIClientWnd.h"
#include "MyMemDC.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CMDIClientWnd::CMDIClientWnd()
{
	m_bFontCreated = FALSE;
	m_nFontSize = 10;
	m_sFontFace = _T("Verdana");
	m_crFontColor = RGB(255,255,255);
	m_crLinkColor = RGB(0x99,0xdd,0xff);
	m_nLeftMargin = 2;
	m_nTopMargin = 0;
	m_rcLinkComputer = CRect(0,0,0,0);
	m_rcLinkLocalhost = CRect(0,0,0,0);
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
	memDC.FillSolidRect(&rcClient, ::GetSysColor(COLOR_APPWORKSPACE));

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
							OUT_CHARACTER_PRECIS,
							CLIP_CHARACTER_PRECIS,
							ANTIALIASED_QUALITY,
							DEFAULT_PITCH | FF_DONTCARE,
							m_sFontFace);
		m_FontUnderline.CreateFont(	-nFontHeight,
							0, 0, 0,
							FW_NORMAL,
							FALSE,		// Italic
							TRUE,		// Underline
							FALSE,		// Strikethrough
							DEFAULT_CHARSET,
							OUT_CHARACTER_PRECIS,
							CLIP_CHARACTER_PRECIS,
							ANTIALIASED_QUALITY,
							DEFAULT_PITCH | FF_DONTCARE,
							m_sFontFace);
		m_bFontCreated = TRUE;
	}

	// Select color & font
	COLORREF crOldTextColor = memDC.SetTextColor(m_crFontColor);
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
#ifdef TRACELOGFILE
	s.Format(_T("%s %s - ONLY FOR DEBUG (see %s with Browse Config/Log Files under Tools menu)"), APPNAME_NOEXT, APPVERSION, TRACENAME_EXT);
#else
	s.Format(_T("%s %s"), APPNAME_NOEXT, APPVERSION);
#endif
	DrawT(memDC, s, rcDraw);

#ifdef VIDEODEVICEDOC
	// Draw point 1.
	rcDraw.top += 2*nLineHeight;
	DrawT(memDC, ML_STRING(1751, "1. Make sure your video capture device is plugged-in and drivers are installed"), rcDraw);
	rcDraw.top += nLineHeight;
	DrawT(memDC, _T("    ") + ML_STRING(1752, "(network and DV cameras do not need specific drivers)"), rcDraw);

	// Draw point 2.
	rcDraw.top += nLineHeight;
	DrawT(memDC, ML_STRING(1753, "2. From the Capture menu select the device you want to use"), rcDraw);
	rcDraw.top += nLineHeight;
	DrawT(memDC, _T("    ") + ML_STRING(1754, "(for network cameras supply host name or ip address, port and camera type)"), rcDraw);

	// Draw point 3.
	rcDraw.top += nLineHeight;
	DrawT(memDC, ML_STRING(1755, "3. Follow the Camera Basic Settings dialog"), rcDraw);
	rcDraw.top += nLineHeight;
	DrawT(memDC, _T("    ") + ML_STRING(1756, "(if it's not popping-up select Camera Basic Settings from the Capture menu)"), rcDraw);

	// Draw point 4.
	rcDraw.top += nLineHeight;	
	DrawT(memDC, ML_STRING(1757, "4. Choose Browse from the View menu") +  _T(" ") + ML_STRING(1867, "or type in Web Browser's address bar") + _T(":"), rcDraw);

	// Draw indent
	rcDraw.top += nLineHeight;
	CRect rcLastText = DrawTAndCalcRect(memDC, _T("    "), rcDraw);

	// Draw computer link
	rcDraw.left = rcLastText.right;
	CFont* pPrevFont = memDC.SelectObject(&m_FontUnderline);
	COLORREF crPrevTextColor = memDC.SetTextColor(m_crLinkColor);
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
		::GetCursorPos(&pt);
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

void CMDIClientWnd::OnLButtonUp(UINT nFlags, CPoint point) 
{
#ifdef VIDEODEVICEDOC
	CString sHost;
	if (m_rcLinkComputer.PtInRect(point))
		sHost = ::GetComputerName();
	else if (m_rcLinkLocalhost.PtInRect(point))
		sHost =  _T("localhost");
	ViewWeb(sHost);
#endif
	CWnd::OnLButtonUp(nFlags, point);
}
