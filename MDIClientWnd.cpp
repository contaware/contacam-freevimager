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

	// Font Sizes
	int nFontHeight = ::MulDiv(m_nFontSize, ::GetDeviceCaps(dc.GetSafeHdc(), LOGPIXELSY), 72);

	// Create Font
	if (!m_bFontCreated)
	{
		m_Font.DeleteObject();
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
	
	// Draw
	const int nLeftRightMargin = 2;
	const int nTopMargin = 0;
	const int nBottomMargin = 2;
	CRect rcDraw(	nLeftRightMargin,
					nTopMargin,
					rcClient.right - nLeftRightMargin,
					rcClient.bottom - nBottomMargin);
	rcDraw.top += TextMetrics.tmAscent;

	CString s;

#ifdef VIDEODEVICEDOC
	s.Format(_T("%s %s"), APPNAME_NOEXT, APPVERSION);
	::DrawTextEx(memDC.GetSafeHdc(),
				s.GetBuffer(s.GetLength() + 1),
				s.GetLength(),
				&rcDraw,
				DT_END_ELLIPSIS | DT_WORD_ELLIPSIS | DT_LEFT | DT_NOCLIP | DT_NOPREFIX,
				NULL);
	s.ReleaseBuffer();

	rcDraw.top += 2*nLineHeight;
	s = ML_STRING(1751, "1. Make sure your video capture device is plugged-in and drivers are installed");
	::DrawTextEx(	memDC.GetSafeHdc(),
					s.GetBuffer(s.GetLength() + 1),
					s.GetLength(),
					&rcDraw,
					DT_END_ELLIPSIS | DT_WORD_ELLIPSIS | DT_LEFT | DT_NOCLIP | DT_NOPREFIX,
					NULL);
	s.ReleaseBuffer();

	rcDraw.top += nLineHeight;
	s = _T("    ") + ML_STRING(1752, "(network and DV cameras do not need specific drivers)");
	::DrawTextEx(	memDC.GetSafeHdc(),
					s.GetBuffer(s.GetLength() + 1),
					s.GetLength(),
					&rcDraw,
					DT_END_ELLIPSIS | DT_WORD_ELLIPSIS | DT_LEFT | DT_NOCLIP | DT_NOPREFIX,
					NULL);
	s.ReleaseBuffer();

	rcDraw.top += nLineHeight;
	s = ML_STRING(1753, "2. From the Capture menu select the device you want to use");
	::DrawTextEx(	memDC.GetSafeHdc(),
					s.GetBuffer(s.GetLength() + 1),
					s.GetLength(),
					&rcDraw,
					DT_END_ELLIPSIS | DT_WORD_ELLIPSIS | DT_LEFT | DT_NOCLIP | DT_NOPREFIX,
					NULL);
	s.ReleaseBuffer();

	rcDraw.top += nLineHeight;
	s = _T("    ") + ML_STRING(1754, "(for network cameras supply host name or ip address, port and device type)");
	::DrawTextEx(	memDC.GetSafeHdc(),
					s.GetBuffer(s.GetLength() + 1),
					s.GetLength(),
					&rcDraw,
					DT_END_ELLIPSIS | DT_WORD_ELLIPSIS | DT_LEFT | DT_NOCLIP | DT_NOPREFIX,
					NULL);
	s.ReleaseBuffer();

	rcDraw.top += nLineHeight;
	s = ML_STRING(1755, "3. Follow the Device Assistant");
	::DrawTextEx(	memDC.GetSafeHdc(),
					s.GetBuffer(s.GetLength() + 1),
					s.GetLength(),
					&rcDraw,
					DT_END_ELLIPSIS | DT_WORD_ELLIPSIS | DT_LEFT | DT_NOCLIP | DT_NOPREFIX,
					NULL);
	s.ReleaseBuffer();

	rcDraw.top += nLineHeight;
	s = _T("    ") + ML_STRING(1756, "(if it is not popping-up select Device Assistant from the Capture menu)");
	::DrawTextEx(	memDC.GetSafeHdc(),
					s.GetBuffer(s.GetLength() + 1),
					s.GetLength(),
					&rcDraw,
					DT_END_ELLIPSIS | DT_WORD_ELLIPSIS | DT_LEFT | DT_NOCLIP | DT_NOPREFIX,
					NULL);
	s.ReleaseBuffer();

	rcDraw.top += nLineHeight;
	s = ML_STRING(1757, "4. Choose Browse from the View menu");
	::DrawTextEx(	memDC.GetSafeHdc(),
					s.GetBuffer(s.GetLength() + 1),
					s.GetLength(),
					&rcDraw,
					DT_END_ELLIPSIS | DT_WORD_ELLIPSIS | DT_LEFT | DT_NOCLIP | DT_NOPREFIX,
					NULL);
	s.ReleaseBuffer();

	rcDraw.top += nLineHeight;
	if (((CUImagerApp*)::AfxGetApp())->m_nMicroApachePort == 80)
		s.Format(_T("    (") + ML_STRING(1867, "or type in Web Browser's address bar") + _T(" http://%s)"), ::GetComputerName());
	else
		s.Format(_T("    (") + ML_STRING(1867, "or type in Web Browser's address bar") + _T(" http://%s:%d)"), ::GetComputerName(), ((CUImagerApp*)::AfxGetApp())->m_nMicroApachePort);
	::DrawTextEx(	memDC.GetSafeHdc(),
					s.GetBuffer(s.GetLength() + 1),
					s.GetLength(),
					&rcDraw,
					DT_END_ELLIPSIS | DT_WORD_ELLIPSIS | DT_LEFT | DT_NOCLIP | DT_NOPREFIX,
					NULL);
	s.ReleaseBuffer();
#else
	s.Format(_T("%s %s"), APPNAME_NOEXT, APPVERSION);
	::DrawTextEx(	memDC.GetSafeHdc(),
					s.GetBuffer(s.GetLength() + 1),
					s.GetLength(),
					&rcDraw,
					DT_END_ELLIPSIS | DT_WORD_ELLIPSIS | DT_LEFT | DT_NOCLIP | DT_NOPREFIX,
					NULL);
	s.ReleaseBuffer();
#endif

	// Clean-Up
	memDC.SetTextAlign(uiOldTextAlign);
	memDC.SelectObject(pOldFont);
	memDC.SetBkMode(nOldBkMode);
	memDC.SetTextColor(crOldTextColor);
}

void CMDIClientWnd::OnSize(UINT nType, int cx, int cy) 
{
	Invalidate();
	CWnd::OnSize(nType, cx, cy);
}
