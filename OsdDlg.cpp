// OsdDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "MainFrm.h"
#include "OsdDlg.h"
#include "MyMemDC.h"
#include "Dib.h"
#include "PictureDoc.h"
#include "PictureView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COsdDlg dialog

COsdDlg::COsdDlg(CPictureDoc* pDoc)
	: CDialog(COsdDlg::IDD, NULL)
{
	//{{AFX_DATA_INIT(COsdDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	// Init Vars
	m_pDoc = pDoc;
	ASSERT_VALID(m_pDoc);
	m_bLayered = FALSE;
	m_uiTimerID = 0;
	m_bDataToDisplay = FALSE;
	if (pDoc->m_pSetLayeredWindowAttributes)
	{
		m_nOpacityInc = 5;
		m_nOpacityDec = 2;
	}
	else
	{
		m_nOpacityInc = 25;
		m_nOpacityDec = 25;
	}
	m_nMinOpacity = MIN_OPACITY;
	m_nOpacity = MIN_OPACITY;
	m_nOldMouseX = 0;
	m_nOldMouseY = 0;
	m_bDrag = FALSE;
	m_nCountdown = 0;
	m_nForceShowCount = 0;
	m_bFontCreated = FALSE;
	m_sHeadline = _T("");
	m_sDescription = _T("");
	m_bAutoSizeNow = FALSE;
	Defaults();

	// Load Settings
	LoadSettings();

	// Create Dlg
	CDialog::Create(COsdDlg::IDD, NULL);
}

void COsdDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COsdDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COsdDlg, CDialog)
	//{{AFX_MSG_MAP(COsdDlg)
	ON_WM_ERASEBKGND()
	ON_WM_SIZING()
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_WM_NCPAINT()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_CREATE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MBUTTONDBLCLK()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_WM_SIZE()
	ON_WM_NCACTIVATE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_ENTERSIZEMOVE, OnEnterSizeMove)
	ON_MESSAGE(WM_EXITSIZEMOVE, OnExitSizeMove)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COsdDlg message handlers

int COsdDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	// Create
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Set Top Most Style
	::SetWindowLong(this->GetSafeHwnd(),
					GWL_EXSTYLE,
					::GetWindowLong(this->GetSafeHwnd(),
									GWL_EXSTYLE) |
									WS_EX_TOPMOST);

	// Set Opacity
	SetOpacity(m_nOpacity);

	// Get Monitor Rectangle
	m_rcMonitor = ::AfxGetMainFrame()->GetMonitorFullRect();

	// Get Dialog Size
	CRect rc;
	GetWindowRect(&rc);
	m_nMinDlgSizeX = rc.Width();
	m_nMinDlgSizeY = rc.Height();

	// Load Placement Settings
	if (!LoadPlacementSettings())
	{
		WINDOWPLACEMENT	wpl;
		GetWindowPlacement(&wpl);
		wpl.showCmd = SW_HIDE;
		wpl.rcNormalPosition.left	=	m_rcMonitor.left;
		wpl.rcNormalPosition.right	=	m_rcMonitor.right;
		wpl.rcNormalPosition.top	=	m_rcMonitor.top;
		wpl.rcNormalPosition.bottom	=	m_rcMonitor.top + DEFAULT_DLG_HEIGHT;
		SetWindowPos(	NULL,
						wpl.rcNormalPosition.left,
						wpl.rcNormalPosition.top,
						wpl.rcNormalPosition.right - wpl.rcNormalPosition.left,
						wpl.rcNormalPosition.bottom - wpl.rcNormalPosition.top,
						SWP_NOACTIVATE | SWP_NOZORDER);

		// Note:
		// SetWindowPlacement is not placing windows above the hidden taskbar -> use SetWindowPos
		//SetWindowPlacement(&wpl);
	}
		
	return 0;
}

BOOL COsdDlg::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE;
}

BOOL COsdDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// Init Timeout
	if (m_nTimeout == TIMEOUT_INFINITE)
		ForceShow();
	else
	{
		if (m_pDoc->m_pSetLayeredWindowAttributes)
			m_nCountdown = m_nTimeout / TRANSPARENCY_TIMER_MS;
		else
			m_nCountdown = m_nTimeout / NOTRANSPARENCY_TIMER_MS;
	}

	// Init Timer
	if (m_pDoc->m_pSetLayeredWindowAttributes)
	{
		// Start Timer
		m_uiTimerID = SetTimer(	ID_TIMER_TRANSPARENCY,
								TRANSPARENCY_TIMER_MS,
								NULL);
	}
	else
	{
		// Start Timer
		m_uiTimerID = SetTimer(	ID_TIMER_TRANSPARENCY,
								NOTRANSPARENCY_TIMER_MS,
								NULL);
	}

	// Set Focus to Main Frame
	::SetActiveWindow(::AfxGetMainFrame()->GetSafeHwnd());
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COsdDlg::Close()
{
	OnClose();
}

void COsdDlg::OnClose() 
{
	// Kill Timer
	if (m_uiTimerID > 0)
	{
		KillTimer(m_uiTimerID);
		m_uiTimerID = 0;
	}

	// Save Settings
	SaveSettings();

	// Destroy Window
	DestroyWindow();
}

void COsdDlg::PostNcDestroy() 
{
	m_pDoc->m_pOsdDlg = NULL;
	delete this;
	CDialog::PostNcDestroy();
}

BOOL COsdDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if (HIWORD (wParam) == BN_CLICKED)
	{
		switch (LOWORD (wParam))
		{
			case IDOK:
				return TRUE;
			case IDCANCEL:
				return TRUE;
			default:
				return CDialog::OnCommand(wParam, lParam);
		}
	}
	return CDialog::OnCommand(wParam, lParam);
}

void COsdDlg::SetTimeout(int nTimeout)
{
	if (nTimeout == TIMEOUT_INFINITE)
	{
		if (m_nTimeout != TIMEOUT_INFINITE)
			ForceShow();
	}
	else
	{
		if (m_nTimeout == TIMEOUT_INFINITE)
			ForceShow(FALSE);
	}	
	m_nTimeout = nTimeout;
	RestartTimeout();
}

void COsdDlg::RestartTimeout()
{
	if (m_pDoc->m_pSetLayeredWindowAttributes)
		m_nCountdown = m_nTimeout / TRANSPARENCY_TIMER_MS;
	else
		m_nCountdown = m_nTimeout / NOTRANSPARENCY_TIMER_MS;
}

void COsdDlg::ForceShow(BOOL bForce/*=TRUE*/)
{
	if (bForce)
		m_nForceShowCount++;
	else
		m_nForceShowCount--;
}

void COsdDlg::OnTimer(UINT nIDEvent) 
{
	POINT ptCursor;
	CRect rcDlg;
	::GetSafeCursorPos(&ptCursor);
	GetWindowRect(&rcDlg);
	
	if (::IsWindow(m_pDoc->GetView()->GetSafeHwnd()))
	{
		if (m_bDataToDisplay)
		{
			if (rcDlg.PtInRect(ptCursor)	||
				m_nCountdown > 0			||
				m_nForceShowCount > 0)
			{
				if (m_nCountdown > 0)
					--m_nCountdown;	
				if (m_nOpacity < m_nMaxOpacity)
				{
					m_nOpacity += m_nOpacityInc;
					m_nOpacity = MIN(m_nMaxOpacity, m_nOpacity);
					SetOpacity(m_nOpacity);
				}
				else if (m_nOpacity > m_nMaxOpacity)
				{
					m_nOpacity -= m_nOpacityInc;
					m_nOpacity = MAX(m_nMaxOpacity, m_nOpacity);
					SetOpacity(m_nOpacity);
				}
			}
			else
			{
				if (m_nOpacity > m_nMinOpacity)
				{
					m_nOpacity -= m_nOpacityDec;
					m_nOpacity = MAX(m_nMinOpacity, m_nOpacity);
					SetOpacity(m_nOpacity);
				}
			}
		}
	}

	CDialog::OnTimer(nIDEvent);
}

BOOL COsdDlg::OnNcActivate(BOOL bActive)
{
	// To avoid painting the gray border
	// when activated or disactivated
	return TRUE;
}

void COsdDlg::OnNcPaint()
{
	// Get window DC that is clipped to the non-client area
	CWindowDC dc(this);
	CRect rectClient;
	GetClientRect(rectClient);
	CRect rectWindow;
	GetWindowRect(rectWindow);
	ScreenToClient(rectWindow);
	rectClient.OffsetRect(-rectWindow.left, -rectWindow.top);
	dc.ExcludeClipRect(rectClient);

	// Fill with Brush
	rectWindow.OffsetRect(-rectWindow.left, -rectWindow.top);
	CBrush br;
	br.CreateSolidBrush(m_crBackgroundColor);
	dc.FillRect(&rectWindow, &br);
	br.DeleteObject();

	// Erase parts not drawn
	dc.IntersectClipRect(rectWindow);
}

void COsdDlg::OnSize(UINT nType, int cx, int cy) 
{
	Invalidate();
	CDialog::OnSize(nType, cx, cy);
}

void COsdDlg::SetFontSize(int nFontSize)
{
	if (m_nFontSize != nFontSize)
	{
		m_bFontCreated = FALSE;
		m_nFontSize = nFontSize;
		m_bAutoSizeNow = m_bAutoSize;
		Invalidate();
	}
}

void COsdDlg::SetFontColor(COLORREF crFontColor)
{
	if (m_crFontColor != crFontColor)
	{
		m_crFontColor = crFontColor;
		Invalidate();
	}
}

void COsdDlg::SetBackgroundColor(COLORREF crBackgroundColor)
{
	if (m_crBackgroundColor != crBackgroundColor)
	{
		m_crBackgroundColor = crBackgroundColor;
		RedrawWindow(	NULL,
						NULL,
						RDW_INVALIDATE |
						RDW_FRAME);	// Redraw also Frame!
	}
}

void COsdDlg::OnPaint() 
{
	CString s;

	// Device context for painting
	CPaintDC dc(this);
	CMyMemDC memDC(&dc);

	// Get Client Rectangle
	CRect rcClient;
	GetClientRect(&rcClient);
	
	// Erase Background
	CBrush br;
	br.CreateSolidBrush(m_crBackgroundColor);
	memDC.FillRect(&rcClient, &br);
	br.DeleteObject();

	// Font Sizes
	int nFontHeight = ::MulDiv(m_nFontSize, ::GetDeviceCaps(dc.GetSafeHdc(), LOGPIXELSY), 72);
	int nFontHeightDesc = ::MulDiv(	m_nFontSize - (m_nFontSize == COsdDlg::FONT_BIG ? 4 : 2),
									::GetDeviceCaps(dc.GetSafeHdc(), LOGPIXELSY), 72);

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
							OUT_DEFAULT_PRECIS,
							CLIP_DEFAULT_PRECIS,
							PROOF_QUALITY,
							DEFAULT_PITCH | FF_DONTCARE,
							g_szDefaultFontFace);

		m_FontDesc.DeleteObject();
		m_FontDesc.CreateFont(	-nFontHeightDesc,
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
	TEXTMETRIC TextMetricsDesc;
	CFont* pOF = memDC.SelectObject(&m_FontDesc);
	memDC.GetTextMetrics(&TextMetricsDesc);
	memDC.SelectObject(pOF);
	int nLineHeight = 6 * TextMetrics.tmHeight / 5;
	int nLineHeightDesc = 6 * TextMetricsDesc.tmHeight / 5;
	
	// Draw
	const int nLeftRightMargin = 2;
	const int nTopMargin = 0;
	const int nBottomMargin = 2;
	CRect rcDraw(	nLeftRightMargin,
					nTopMargin,
					rcClient.right - nLeftRightMargin,
					rcClient.bottom - nBottomMargin);
	CRect rcDrawPrev = rcDraw;
	rcDraw.top += TextMetrics.tmAscent;

	// Draw Top Data
	if (m_TopArray.GetSize() > 0)
		PaintArray(&memDC, m_TopArray, rcDraw, rcDrawPrev, nLineHeight);

	// Draw Headline
	if (m_sHeadline != _T(""))
	{
		// Description fits on same line as Headline?
		s = m_sHeadline + _T(":");
		BOOL bDescFitsSameLine = FALSE;
		CRect rcDesc(rcDraw);
		CSize szHeadline = GetOneLineTextSize(&memDC, s);
		rcDesc.left += szHeadline.cx + szHeadline.cy / 3;
		if (m_sDescription != _T(""))
		{
			pOF = memDC.SelectObject(&m_FontDesc);
			if (FitsOneLine(&memDC, m_sDescription, rcDesc))
				bDescFitsSameLine = TRUE;
			memDC.SelectObject(pOF);
		}

		// Draw Headline
		if (bDescFitsSameLine)
			s = m_sHeadline + _T(":");
		else
			s = m_sHeadline;
		::DrawTextEx(	memDC.GetSafeHdc(),
						s.GetBuffer(s.GetLength() + 1),
						s.GetLength(),
						&rcDraw,
						DT_END_ELLIPSIS | DT_WORD_ELLIPSIS | DT_LEFT | DT_NOCLIP | DT_NOPREFIX,
						NULL);
		s.ReleaseBuffer();
		rcDrawPrev = rcDraw;
		if (m_sDescription != _T(""))
		{
			if (bDescFitsSameLine)
				rcDraw = rcDesc;
			else
				rcDraw.top += nLineHeightDesc;
		}
		else
			rcDraw.top += nLineHeight;
	}

	// Draw Description
	if (m_sDescription != _T(""))
	{
		pOF = memDC.SelectObject(&m_FontDesc);
		s = m_sDescription;
		int nTextHeight = ::DrawTextEx(	memDC.GetSafeHdc(),
										s.GetBuffer(s.GetLength() + 1),
										s.GetLength(),
										&rcDraw,
										DT_WORDBREAK | DT_LEFT | DT_NOCLIP | DT_NOPREFIX,
										NULL);
		s.ReleaseBuffer();
		if (nTextHeight == TextMetricsDesc.tmHeight)
		{
			rcDrawPrev = rcDraw;
			rcDraw.top += nLineHeight;
		}
		else
		{
			rcDrawPrev.top = rcDraw.top + nTextHeight - TextMetricsDesc.tmHeight;
			rcDraw.top += nTextHeight + 3 * (TextMetrics.tmHeight - TextMetricsDesc.tmHeight);
		}
		rcDraw.left = nLeftRightMargin;
		memDC.SelectObject(pOF);
	}

	// Draw Exif Camera Data
	if (m_ExifArray.GetSize() > 0)
		PaintArray(&memDC, m_ExifArray, rcDraw, rcDrawPrev, nLineHeight);

	// Clean-Up
	memDC.SetTextAlign(uiOldTextAlign);
	memDC.SelectObject(pOldFont);
	memDC.SetBkMode(nOldBkMode);
	memDC.SetTextColor(crOldTextColor);

	// Auto-Size
	if (m_bAutoSizeNow)
	{
		m_bAutoSizeNow = FALSE;
		CRect rcWnd;
		GetWindowRect(&rcWnd);
		SetDlgHeight(rcDrawPrev.top + nBottomMargin		+
					TextMetrics.tmDescent				+
					(rcWnd.Height() - rcClient.Height()));
	}

	// Do not call CDialog::OnPaint() for painting messages
}

void COsdDlg::PaintArray(CDC* pDC, CStringArray& a, CRect& rcDraw, CRect& rcDrawPrev, int nLineHeight)
{
	CString s(_T(""));
	CString sPrev(_T(""));

	for (int i = 0 ; i < a.GetSize() ; i++)
	{
		if (s != _T(""))
			s += _T(", ");
		s += a[i];
		if (!FitsOneLine(pDC, s, rcDraw))
		{
			if (sPrev != _T(""))
			{
				::DrawTextEx(	pDC->GetSafeHdc(),
								sPrev.GetBuffer(sPrev.GetLength() + 1),
								sPrev.GetLength(),
								&rcDraw,
								DT_END_ELLIPSIS | DT_WORD_ELLIPSIS | DT_LEFT | DT_NOCLIP | DT_NOPREFIX,
								NULL);
				sPrev.ReleaseBuffer();
				s = a[i];
			}
			else
			{
				::DrawTextEx(	pDC->GetSafeHdc(),
								s.GetBuffer(s.GetLength() + 1),
								s.GetLength(),
								&rcDraw,
								DT_END_ELLIPSIS | DT_WORD_ELLIPSIS | DT_LEFT | DT_NOCLIP | DT_NOPREFIX,
								NULL);
				s.ReleaseBuffer();
				s = _T("");
			}
			rcDrawPrev = rcDraw;
			rcDraw.top += nLineHeight;
		}
		sPrev = s;
	}
	if (s != _T(""))
	{
		::DrawTextEx(	pDC->GetSafeHdc(),
						s.GetBuffer(s.GetLength() + 1),
						s.GetLength(),
						&rcDraw,
						DT_END_ELLIPSIS | DT_WORD_ELLIPSIS | DT_LEFT | DT_NOCLIP | DT_NOPREFIX,
						NULL);
		s.ReleaseBuffer();
		rcDrawPrev = rcDraw;
		rcDraw.top += nLineHeight;
	}
}

CSize COsdDlg::GetOneLineTextSize(CDC* pDC, CString s)
{
	CRect rcNecessary;
	::DrawTextEx(	pDC->GetSafeHdc(),
					s.GetBuffer(s.GetLength() + 1),
					s.GetLength(),
					&rcNecessary,
					DT_LEFT | DT_CALCRECT | DT_NOCLIP | DT_NOPREFIX,
					NULL);
	s.ReleaseBuffer();
	return CSize(rcNecessary.Width(), rcNecessary.Height());
}

BOOL COsdDlg::FitsOneLine(CDC* pDC, CString s, const CRect& rc)
{
	CRect rcNecessary(rc);
	::DrawTextEx(	pDC->GetSafeHdc(),
					s.GetBuffer(s.GetLength() + 1),
					s.GetLength(),
					&rcNecessary,
					DT_LEFT | DT_CALCRECT | DT_NOCLIP | DT_NOPREFIX,
					NULL);
	s.ReleaseBuffer();
	return (rcNecessary.left >= rc.left &&
			rcNecessary.right <= rc.right);
}

void COsdDlg::SetDlgHeight(int nHeight)
{
	CRect rcDlg;
	GetWindowRect(&rcDlg);

	// Check Min Dlg Height
	nHeight = MAX(nHeight, m_nMinDlgSizeY);

	// Calc. Middle Points
	int nDlgMiddle = rcDlg.top + rcDlg.Height() / 2;
	int nMonitorMiddle = m_rcMonitor.top + m_rcMonitor.Height() / 2;
	
	// Resize Bottom Edge
	if (nDlgMiddle <= nMonitorMiddle)
		rcDlg.bottom = rcDlg.top + nHeight;
	// Resize Top Edge
	else
		rcDlg.top = rcDlg.bottom - nHeight;

	// Clip to Monitor
	::AfxGetMainFrame()->ClipToFullRect(rcDlg);

	// Set New Position
	if (IsWindowVisible())
	{
		SetWindowPos(	NULL,
						rcDlg.left,
						rcDlg.top,
						rcDlg.Width(),
						rcDlg.Height(),
						SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
	}
	else
	{
		SetWindowPos(	NULL,
						rcDlg.left,
						rcDlg.top,
						rcDlg.Width(),
						rcDlg.Height(),
						SWP_HIDEWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
	}
}

void COsdDlg::OnSizing(UINT fwSide, LPRECT pRect) 
{
	CDialog::OnSizing(fwSide, pRect);

	// Min. Size Check
	if (pRect->right - pRect->left < m_nMinDlgSizeX)
	{
		if ((fwSide == WMSZ_BOTTOMLEFT) || (fwSide == WMSZ_LEFT) || (fwSide == WMSZ_TOPLEFT))
			pRect->left = pRect->right - m_nMinDlgSizeX;
		else 
			pRect->right = pRect->left + m_nMinDlgSizeX;
	}
	
	if (pRect->bottom - pRect->top < m_nMinDlgSizeY)
	{
		if((fwSide == WMSZ_TOP) || (fwSide == WMSZ_TOPLEFT) || (fwSide == WMSZ_TOPRIGHT))
			pRect->top = pRect->bottom - m_nMinDlgSizeY;
		else
			pRect->bottom = pRect->top + m_nMinDlgSizeY;
	}

	// Out Off Screen Check
	if (pRect->left < m_rcMonitor.left)
		pRect->left = m_rcMonitor.left;
	if (pRect->top < m_rcMonitor.top)
		pRect->top = m_rcMonitor.top;
	if (pRect->right > m_rcMonitor.right)
		pRect->right = m_rcMonitor.right;
	if (pRect->bottom > m_rcMonitor.bottom)
		pRect->bottom = m_rcMonitor.bottom;
}

void COsdDlg::SetOpacity(int nPercent)
{
	// Check
	if (nPercent < 0)
		nPercent = 0;
	else if (nPercent > 100)
		nPercent = 100;

	// Set
	if (m_pDoc->m_pSetLayeredWindowAttributes)
	{
		m_pDoc->m_pSetLayeredWindowAttributes(	this->GetSafeHwnd(),
												0,
												(255 * nPercent) / 100,
												LWA_ALPHA);
	}
	else
	{
		if (nPercent == 0)
		{
			if (IsWindowVisible())
			{
				// Hide
				ShowWindow(SW_HIDE);

				// Set Focus to Main Frame
				::SetActiveWindow(::AfxGetMainFrame()->GetSafeHwnd());
			}
		}
		else
		{
			if (!IsWindowVisible() && m_bDataToDisplay)
			{
				// Show no Active
				ShowWindow(SW_SHOWNA);
				SetLayered();

				// Set Focus to Main Frame
				::SetActiveWindow(::AfxGetMainFrame()->GetSafeHwnd());
			}
		}
	}
}

// Set Layered Extended Style
void COsdDlg::SetLayered()
{
	if (m_pDoc->m_pSetLayeredWindowAttributes && !m_bLayered)
	{
		// Set WS_EX_LAYERED to this window styles
		::SetWindowLong(this->GetSafeHwnd(),
						GWL_EXSTYLE,
						::GetWindowLong(this->GetSafeHwnd(),
										GWL_EXSTYLE) |
										WS_EX_LAYERED);
		m_bLayered = TRUE;
	}
}

// Remove Layered Extended Style
void COsdDlg::RemoveLayered()
{
	if (m_pDoc->m_pSetLayeredWindowAttributes && m_bLayered)
	{
		// Remove WS_EX_LAYERED from this window styles
		::SetWindowLong(this->GetSafeHwnd(), 
						GWL_EXSTYLE,
						::GetWindowLong(this->GetSafeHwnd(),
										GWL_EXSTYLE) &
										~WS_EX_LAYERED);
		m_bLayered = FALSE;
	}
}

LRESULT COsdDlg::OnEnterSizeMove(WPARAM, LPARAM)
{
	// Set Force Focus
	m_pDoc->GetView()->ForceCursor();

	// Set Force Show
	ForceShow();
	
	return 0;
}

LRESULT COsdDlg::OnExitSizeMove(WPARAM, LPARAM)
{
	// Reset Force Show
	ForceShow(FALSE);

	// Reset Force Focus
	m_pDoc->GetView()->ForceCursor(FALSE);

	// Set Focus to Main Frame
	::SetActiveWindow(::AfxGetMainFrame()->GetSafeHwnd());

	return 0;
}

void COsdDlg::MonitorSwitch()
{
	WINDOWPLACEMENT placement;
	if (GetWindowPlacement(&placement))
	{
		CRect rcOldMonitor = m_rcMonitor;
		m_rcMonitor = ::AfxGetMainFrame()->GetMonitorFullRect();
		SetPlacement(placement, rcOldMonitor);
	}
}

void COsdDlg::ActivateDrag()
{
	// Set Force Focus
	m_pDoc->GetView()->ForceCursor();

	// Set Force Show
	ForceShow();

	// Get Monitor Rectangle
	m_rcMonitor = ::AfxGetMainFrame()->GetMonitorFullRect();
	
	// Get Current Cursor Position
	CPoint ptGlobPos;
	::GetSafeCursorPos(&ptGlobPos);
	m_nOldMouseX = ptGlobPos.x;
	m_nOldMouseY = ptGlobPos.y;

	// Set Capture
	SetCapture();

	// Set Flag
	m_bDrag = TRUE;
}

void COsdDlg::DeactivateDrag()
{
	// Reset Flag
	m_bDrag = FALSE;

	// Reset Force Show
	ForceShow(FALSE);

	// Reset Force Focus
	m_pDoc->GetView()->ForceCursor(FALSE);
	
	// Release Capture
	ReleaseCapture();

	// Set Focus to Main Frame
	::SetActiveWindow(::AfxGetMainFrame()->GetSafeHwnd());
}

void COsdDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	CDialog::OnMouseMove(nFlags, point);

	if (m_bDrag)
	{
		// Get the placement of the dialog
		WINDOWPLACEMENT placement;
		if (!GetWindowPlacement(&placement))
			return;
		int nTop = placement.rcNormalPosition.top;
		int nBottom = placement.rcNormalPosition.bottom;
		int nLeft = placement.rcNormalPosition.left;
		int nRight = placement.rcNormalPosition.right;

		// Get coordinates of the mouse
		CPoint ptGlobPos;
		if (!::GetSafeCursorPos(&ptGlobPos))
			return;
			
		// Calculate the difference between old 
		// and new X pos of the mouse
		int nDiffX = ptGlobPos.x - m_nOldMouseX;
		
		// Calculate the difference between old 
		// and new Y pos of the mouse
		int nDiffY = ptGlobPos.y - m_nOldMouseY;

		// Calculate the new windowposition
		nLeft = nLeft + nDiffX;
		nTop = nTop + nDiffY;
		nRight = nRight + nDiffX;
		nBottom = nBottom + nDiffY;

		// Calculate the width and height of the dialog
		int nWidth	= nRight-nLeft;
		int nHeight	= nBottom-nTop;
		
		// If dialog should stay in screen the coordinates are
		// corrected if necessary
		if (nLeft < m_rcMonitor.left)
			nLeft = m_rcMonitor.left;
		if (nRight > m_rcMonitor.right)
			nLeft = m_rcMonitor.right - nWidth;
		if (nTop < m_rcMonitor.top)
			nTop = m_rcMonitor.top;
		if (nBottom > m_rcMonitor.bottom)
			nTop = m_rcMonitor.bottom - nHeight;
		
		// Save actual Mousepos for the next call of OnMouseMove()
		m_nOldMouseX = ptGlobPos.x;
		m_nOldMouseY = ptGlobPos.y;
		
		// Move the window to new coordinates
		MoveWindow(nLeft, nTop, nWidth, nHeight, TRUE);
	}

	// Post to parent so that moving the cursor will
	// show the hidden cursor in full-screen mode
	ClientToScreen(&point);
	m_pDoc->GetView()->ScreenToClient(&point);
	m_pDoc->GetView()->PostMessage(	WM_MOUSEMOVE,
									(WPARAM)nFlags,
									(LPARAM)MAKELONG(point.x, point.y));
}

void COsdDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CDialog::OnLButtonDown(nFlags, point);

	if (!m_bDrag)
		ActivateDrag();
}

void COsdDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CDialog::OnLButtonUp(nFlags, point);

	if (m_bDrag)
		DeactivateDrag();
}

void COsdDlg::OnLButtonDblClk(UINT nFlags, CPoint point) 
{	
	CDialog::OnLButtonDblClk(nFlags, point);

	// Post to parent so that it can handle the double-click
	ClientToScreen(&point);
	m_pDoc->GetView()->ScreenToClient(&point);
	m_pDoc->GetView()->PostMessage(	WM_LBUTTONDBLCLK,
									(WPARAM)nFlags,
									(LPARAM)MAKELONG(point.x, point.y));

	// Set Focus to Main Frame
	::SetActiveWindow(::AfxGetMainFrame()->GetSafeHwnd());
}

void COsdDlg::OnMButtonUp(UINT nFlags, CPoint point) 
{
	CDialog::OnMButtonUp(nFlags, point);

	// Set Focus to Main Frame
	::SetActiveWindow(::AfxGetMainFrame()->GetSafeHwnd());
}

void COsdDlg::OnMButtonDblClk(UINT nFlags, CPoint point) 
{	
	CDialog::OnMButtonDblClk(nFlags, point);

	// Set Focus to Main Frame
	::SetActiveWindow(::AfxGetMainFrame()->GetSafeHwnd());
}

void COsdDlg::OnRButtonDown(UINT nFlags, CPoint point) 
{
	CDialog::OnRButtonDown(nFlags, point);

	// Post to parent so that it can handle the menu pop-up
	ClientToScreen(&point);
	m_pDoc->GetView()->ScreenToClient(&point);
	m_pDoc->GetView()->PostMessage(	WM_RBUTTONDOWN,
									(WPARAM)nFlags,
									(LPARAM)MAKELONG(point.x, point.y));

	// Focus will be set to the Main Frame
	// in CPictureDoc when the pop-up menu closes!
}

void COsdDlg::OnRButtonUp(UINT nFlags, CPoint point) 
{
	CDialog::OnRButtonUp(nFlags, point);

	// Post to parent
	ClientToScreen(&point);
	m_pDoc->GetView()->ScreenToClient(&point);
	m_pDoc->GetView()->PostMessage(	WM_RBUTTONUP,
									(WPARAM)nFlags,
									(LPARAM)MAKELONG(point.x, point.y));

	// Focus will be set to the Main Frame
	// in CPictureDoc when the pop-up menu closes!
}

void COsdDlg::OnRButtonDblClk(UINT nFlags, CPoint point) 
{	
	CDialog::OnRButtonDblClk(nFlags, point);

	// Post to parent
	ClientToScreen(&point);
	m_pDoc->GetView()->ScreenToClient(&point);
	m_pDoc->GetView()->PostMessage(	WM_RBUTTONDBLCLK,
									(WPARAM)nFlags,
									(LPARAM)MAKELONG(point.x, point.y));

	// Set Focus to Main Frame
	::SetActiveWindow(::AfxGetMainFrame()->GetSafeHwnd());
}

BOOL COsdDlg::PreTranslateMessage(MSG* pMsg) 
{
	switch (pMsg->message)
	{
		// Disable ALT+F4 Close
		case WM_SYSKEYDOWN:
			if (pMsg->wParam == VK_F4)
				return TRUE;
			break;

		default:
			break;
	}
	
	return CDialog::PreTranslateMessage(pMsg);
}
	
BOOL COsdDlg::LoadPlacementSettings()
{
	CWinApp* pApp = ::AfxGetApp();
	CString sSection(_T("OsdDlg"));
	
	WINDOWPLACEMENT	wpl;
	GetWindowPlacement(&wpl);
	wpl.showCmd = SW_HIDE;
	CRect rcOrigMonitor(0,0,0,0);
	wpl.rcNormalPosition.left	=	pApp->GetProfileInt(sSection,	_T("Left"),			0);
	wpl.rcNormalPosition.right	=	pApp->GetProfileInt(sSection,	_T("Right"),		0);
	wpl.rcNormalPosition.top	=	pApp->GetProfileInt(sSection,	_T("Top"),			0);
	wpl.rcNormalPosition.bottom	=	pApp->GetProfileInt(sSection,	_T("Bottom"),		0);
	rcOrigMonitor.left			=	pApp->GetProfileInt(sSection,	_T("MonitorLeft"),	0);
	rcOrigMonitor.right			=	pApp->GetProfileInt(sSection,	_T("MonitorRight"),	0);
	rcOrigMonitor.top			=	pApp->GetProfileInt(sSection,	_T("MonitorTop"),	0);
	rcOrigMonitor.bottom		=	pApp->GetProfileInt(sSection,	_T("MonitorBottom"),0);
	return SetPlacement(wpl, rcOrigMonitor);
}

void COsdDlg::ClipToMonitorRect(RECT& rc)
{
	int w = rc.right - rc.left;
	int h = rc.bottom - rc.top;

	// Clip
	rc.left = MAX(m_rcMonitor.left, MIN(m_rcMonitor.right - w, rc.left));
	rc.top = MAX(m_rcMonitor.top, MIN(m_rcMonitor.bottom - h, rc.top));
	rc.right = rc.left + w;
	rc.bottom = rc.top + h;
}

BOOL COsdDlg::SetPlacement(WINDOWPLACEMENT&	wpl, const CRect& rcOrigMonitor)
{
	// First time running application?
	if (wpl.rcNormalPosition.left == 0		&&
		wpl.rcNormalPosition.right == 0		&&
		wpl.rcNormalPosition.top == 0		&&
		wpl.rcNormalPosition.bottom == 0	&&
		rcOrigMonitor.left == 0				&&
		rcOrigMonitor.right == 0			&&
		rcOrigMonitor.top == 0				&&
		rcOrigMonitor.bottom == 0)
		return FALSE;
	else
	{
		int nWidth = wpl.rcNormalPosition.right - wpl.rcNormalPosition.left;
		int nHeight = wpl.rcNormalPosition.bottom - wpl.rcNormalPosition.top;
		BOOL bFixedX = FALSE;
		BOOL bFixedY = FALSE;

		// Docking left and right?
		if (rcOrigMonitor.left == wpl.rcNormalPosition.left &&
			rcOrigMonitor.right == wpl.rcNormalPosition.right)
		{
			bFixedX = TRUE;
			wpl.rcNormalPosition.left = m_rcMonitor.left;
			wpl.rcNormalPosition.right = m_rcMonitor.right;
		}
		// Docking left?
		else if (rcOrigMonitor.left == wpl.rcNormalPosition.left)
		{
			bFixedX = TRUE;
			wpl.rcNormalPosition.left = m_rcMonitor.left;
			wpl.rcNormalPosition.right = wpl.rcNormalPosition.left + nWidth;
		}
		// Or docking right?
		else if (rcOrigMonitor.right == wpl.rcNormalPosition.right)
		{
			bFixedX = TRUE;
			wpl.rcNormalPosition.right = m_rcMonitor.right;
			wpl.rcNormalPosition.left = wpl.rcNormalPosition.right - nWidth;
		}

		// Docking top and bottom?
		if (rcOrigMonitor.top == wpl.rcNormalPosition.top &&
			rcOrigMonitor.bottom == wpl.rcNormalPosition.bottom)
		{
			bFixedY = TRUE;
			wpl.rcNormalPosition.top = m_rcMonitor.top;
			wpl.rcNormalPosition.bottom = m_rcMonitor.bottom;
		}
		// Docking top?
		else if (rcOrigMonitor.top == wpl.rcNormalPosition.top)
		{
			bFixedY = TRUE;
			wpl.rcNormalPosition.top = m_rcMonitor.top;
			wpl.rcNormalPosition.bottom = wpl.rcNormalPosition.top + nHeight;
		}
		// Or docking bottom?
		else if (rcOrigMonitor.bottom == wpl.rcNormalPosition.bottom)
		{
			bFixedY = TRUE;
			wpl.rcNormalPosition.bottom = m_rcMonitor.bottom;
			wpl.rcNormalPosition.top = wpl.rcNormalPosition.bottom - nHeight;
		}

		// Floating in X direction?
		if (!bFixedX)
		{
			int nOrigMonitorWidth = rcOrigMonitor.right - rcOrigMonitor.left;
			double dWidthScale = (double)m_rcMonitor.Width() / (double)nOrigMonitorWidth;
			wpl.rcNormalPosition.left = Round((double)(wpl.rcNormalPosition.left) * dWidthScale);
			wpl.rcNormalPosition.right = wpl.rcNormalPosition.left + nWidth;
		}

		// Floating in Y direction?
		if (!bFixedY)
		{
			int nOrigMonitorHeight = rcOrigMonitor.bottom - rcOrigMonitor.top;
			double dHeightScale = (double)m_rcMonitor.Height() / (double)nOrigMonitorHeight;
			wpl.rcNormalPosition.top = Round((double)(wpl.rcNormalPosition.top) * dHeightScale);
			wpl.rcNormalPosition.bottom = wpl.rcNormalPosition.top + nHeight;
		}

		// Min. Sizes
		if (wpl.rcNormalPosition.right - wpl.rcNormalPosition.left < m_nMinDlgSizeX)
			wpl.rcNormalPosition.right = wpl.rcNormalPosition.left + m_nMinDlgSizeX;
		if (wpl.rcNormalPosition.bottom - wpl.rcNormalPosition.top < m_nMinDlgSizeY)
			wpl.rcNormalPosition.bottom = wpl.rcNormalPosition.top + m_nMinDlgSizeY;

		// Max. Sizes
		if (wpl.rcNormalPosition.right - wpl.rcNormalPosition.left > m_rcMonitor.Width())
			wpl.rcNormalPosition.right = wpl.rcNormalPosition.left + m_rcMonitor.Width();
		if (wpl.rcNormalPosition.bottom - wpl.rcNormalPosition.top > m_rcMonitor.Height())
			wpl.rcNormalPosition.bottom = wpl.rcNormalPosition.top + m_rcMonitor.Height();

		// Clip to Current Monitor
		ClipToMonitorRect(wpl.rcNormalPosition);

		// Set Placement
		return SetWindowPos(NULL,
							wpl.rcNormalPosition.left,
							wpl.rcNormalPosition.top,
							wpl.rcNormalPosition.right - wpl.rcNormalPosition.left,
							wpl.rcNormalPosition.bottom - wpl.rcNormalPosition.top,
							SWP_NOACTIVATE | SWP_NOZORDER);

		// Note:
		// SetWindowPlacement is not placing windows above the hidden taskbar -> use SetWindowPos
		//return SetWindowPlacement(&wpl);
	}
}

void COsdDlg::Defaults()
{
	m_bAutoSize = TRUE;
	m_nFontSize = FONT_MEDIUM;
	m_nTimeout = TIMEOUT_4SEC;
	m_crFontColor = RGB(192,192,192);
	m_crBackgroundColor = RGB(0,0,0);
	m_bUsePictureDocBackgroundColor = FALSE;
	m_nMaxOpacity = DEFAULT_MAX_OPACITY;
	m_dwDisplayState =	DISPLAY_METADATADATE	|
						DISPLAY_LOCATION		|
						DISPLAY_HEADLINEDESCRIPTION;
}

void COsdDlg::LoadSettings()
{
	CWinApp* pApp = ::AfxGetApp();
	CString sSection(_T("OsdDlg"));

	m_bAutoSize = (BOOL)pApp->GetProfileInt(sSection,	_T("AutoSize"), TRUE);
	m_nFontSize = pApp->GetProfileInt(sSection,	_T("FontSize"), FONT_MEDIUM);
	m_nTimeout = pApp->GetProfileInt(sSection,	_T("TimeoutMs"), TIMEOUT_4SEC);
	m_crFontColor = (COLORREF)pApp->GetProfileInt(sSection,	_T("FontColor"), RGB(192,192,192));
	m_crBackgroundColor = (COLORREF)pApp->GetProfileInt(sSection,	_T("BackgroundColor"), RGB(0,0,0));
	m_bUsePictureDocBackgroundColor = (BOOL)pApp->GetProfileInt(sSection,	_T("UsePictureDocBackgroundColor"), FALSE);
	m_nMaxOpacity = pApp->GetProfileInt(sSection,	_T("MaxOpacity"), DEFAULT_MAX_OPACITY);
	m_dwDisplayState = (DWORD)pApp->GetProfileInt(sSection, _T("DisplayState"),	DISPLAY_METADATADATE	|
																				DISPLAY_LOCATION		|
																				DISPLAY_HEADLINEDESCRIPTION);
}

void COsdDlg::SaveSettings()
{
	WINDOWPLACEMENT	wpl;
	CWinApp* pApp = ::AfxGetApp();
	CString sSection(_T("OsdDlg"));

	GetWindowPlacement(&wpl);
	pApp->WriteProfileInt(sSection,	_T("Left"),			wpl.rcNormalPosition.left);
	pApp->WriteProfileInt(sSection,	_T("Right"),		wpl.rcNormalPosition.right);
	pApp->WriteProfileInt(sSection,	_T("Top"),			wpl.rcNormalPosition.top);
	pApp->WriteProfileInt(sSection,	_T("Bottom"),		wpl.rcNormalPosition.bottom);
	pApp->WriteProfileInt(sSection,	_T("MonitorLeft"),	m_rcMonitor.left);
	pApp->WriteProfileInt(sSection,	_T("MonitorRight"),	m_rcMonitor.right);
	pApp->WriteProfileInt(sSection,	_T("MonitorTop"),	m_rcMonitor.top);
	pApp->WriteProfileInt(sSection,	_T("MonitorBottom"),m_rcMonitor.bottom);

	pApp->WriteProfileInt(sSection,	_T("AutoSize"), m_bAutoSize);
	pApp->WriteProfileInt(sSection, _T("FontSize"), m_nFontSize);
	pApp->WriteProfileInt(sSection,	_T("TimeoutMs"), m_nTimeout);
	pApp->WriteProfileInt(sSection,	_T("FontColor"), m_crFontColor);
	pApp->WriteProfileInt(sSection,	_T("BackgroundColor"), m_crBackgroundColor);
	pApp->WriteProfileInt(sSection,	_T("UsePictureDocBackgroundColor"), m_bUsePictureDocBackgroundColor);
	pApp->WriteProfileInt(sSection,	_T("MaxOpacity"), m_nMaxOpacity);
	pApp->WriteProfileInt(sSection, _T("DisplayState"),	m_dwDisplayState);
}

void COsdDlg::UpdateDisplay()
{
	CString t, m;

	// Update Background Color
	if (DoUsePictureDocBackgroundColor())
	{
		if (m_pDoc->m_bImageBackgroundColor)
			SetBackgroundColor(m_pDoc->m_crImageBackgroundColor);
		else
			SetBackgroundColor(m_pDoc->m_crBackgroundColor);
	}

	if (m_pDoc->m_sFileName == _T(""))
	{
		// Top Array
		m_TopArray.RemoveAll();

		// Size
		if (DoDisplayState(COsdDlg::DISPLAY_SIZESCOMPRESSION))
		{
			// Image Dimension
			t.Format(_T("%d x %d px"), m_pDoc->m_pDib->GetWidth(), m_pDoc->m_pDib->GetHeight());
			if (t != _T(""))
				m_TopArray.Add(t);
		}

		// Clear the Other
		m_sHeadline = _T("");
		m_sDescription = _T("");
		m_ExifArray.RemoveAll();
	}
	else
	{
		// Top Array
		m_TopArray.RemoveAll();
		
		// File Name
		if (DoDisplayState(COsdDlg::DISPLAY_FILENAME))
		{
			t = ::GetShortFileName(m_pDoc->m_sFileName);
			if (t != _T(""))
				m_TopArray.Add(t);
		}

		// Sizes and Compression
		if (DoDisplayState(COsdDlg::DISPLAY_SIZESCOMPRESSION))
		{
			// Image Dimension
			t.Format(_T("%d x %d px"), m_pDoc->m_pDib->GetWidth(), m_pDoc->m_pDib->GetHeight());
			if (t != _T(""))
				m_TopArray.Add(t);

			// Dpi
			int nXDpi = m_pDoc->m_pDib->GetXDpi();
			int nYDpi = m_pDoc->m_pDib->GetYDpi();
			BOOL bDpi = TRUE;
			if (nXDpi == 0 || nYDpi == 0)
				bDpi = FALSE;
			if (bDpi)
			{
				t.Format(_T("%d x %d dpi"), nXDpi, nYDpi);
				if (t != _T(""))
					m_TopArray.Add(t);
			}

			// Depth & Compression
			t = m_pDoc->m_pDib->m_FileInfo.GetDepthName();
			if (t != _T(""))
				m_TopArray.Add(t);
		}

		// File Date
		if (DoDisplayState(COsdDlg::DISPLAY_DATE))
		{
			t = _T("");

			// Get File Time
			CFileStatus FileStatus;
			BOOL bFileTimesOk = CFile::GetStatus(m_pDoc->m_sFileName, FileStatus);
			if (bFileTimesOk)
			{
				t =	::MakeDateLocalFormat(FileStatus.m_ctime, TRUE) +
					_T(", ") +
					::MakeTimeLocalFormat(FileStatus.m_ctime, TRUE);
			}
			t.Replace(_T("\r\n"), _T(" "));
			t.Replace(_T("\r"), _T(" "));
			t.Replace(_T("\n"), _T(" "));
			t.Replace(_T("\t"), _T(" "));
			t.TrimLeft();
			t.TrimRight();
			if (t != _T(""))
				m_TopArray.Add(t);
		}

		// Metadata Date
		if (DoDisplayState(COsdDlg::DISPLAY_METADATADATE))
		{
			t = _T("");

			// Xmp Date
			if (m_pDoc->m_pDib->GetIptcFromXmpInfo()->DateCreated != _T(""))
			{
				CTime Time = CMetadata::GetDateFromIptcLegacyString(m_pDoc->m_pDib->GetIptcFromXmpInfo()->DateCreated);
				t = ::MakeDateLocalFormat(Time, TRUE);
			}
			// Iptc Legacy Date
			else if (m_pDoc->m_pDib->GetIptcLegacyInfo()->DateCreated != _T(""))
			{
				CTime Time = CMetadata::GetDateFromIptcLegacyString(m_pDoc->m_pDib->GetIptcLegacyInfo()->DateCreated);
				t = ::MakeDateLocalFormat(Time, TRUE);
			}
			// Exif Date Time
			else if (m_pDoc->m_pDib->GetExifInfo()->DateTime[0])
			{
				CTime Time = CMetadata::GetDateTimeFromExifString(CString(m_pDoc->m_pDib->GetExifInfo()->DateTime));
				t = ::MakeDateLocalFormat(Time, TRUE) + _T(", ") + ::MakeTimeLocalFormat(Time, TRUE);
			}
			t.Replace(_T("\r\n"), _T(" "));
			t.Replace(_T("\r"), _T(" "));
			t.Replace(_T("\n"), _T(" "));
			t.Replace(_T("\t"), _T(" "));
			t.TrimLeft();
			t.TrimRight();
			if (t != _T(""))
				m_TopArray.Add(t);
		}

		// Location
		if (DoDisplayState(COsdDlg::DISPLAY_LOCATION))
		{
			t = _T("");
			if (m_pDoc->m_pDib->GetXmpInfo()->Location != _T(""))
				t = m_pDoc->m_pDib->GetXmpInfo()->Location;
			if (m_pDoc->m_pDib->GetIptcFromXmpInfo()->City != _T(""))
			{
				if (t != _T(""))
					t += _T(", ");
				t += m_pDoc->m_pDib->GetIptcFromXmpInfo()->City;
			}
			else if (m_pDoc->m_pDib->GetIptcLegacyInfo()->City != _T(""))
			{
				if (t != _T(""))
					t += _T(", ");
				t += m_pDoc->m_pDib->GetIptcLegacyInfo()->City;
			}
			if (m_pDoc->m_pDib->GetIptcFromXmpInfo()->ProvinceState != _T(""))
			{
				if (t != _T(""))
					t += _T(", ");
				t += m_pDoc->m_pDib->GetIptcFromXmpInfo()->ProvinceState;
			}
			else if (m_pDoc->m_pDib->GetIptcLegacyInfo()->ProvinceState != _T(""))
			{
				if (t != _T(""))
					t += _T(", ");
				t += m_pDoc->m_pDib->GetIptcLegacyInfo()->ProvinceState;
			}
			if (m_pDoc->m_pDib->GetIptcFromXmpInfo()->Country != _T(""))
			{
				if (t != _T(""))
					t += _T(", ");
				t += m_pDoc->m_pDib->GetIptcFromXmpInfo()->Country;
			}
			else if (m_pDoc->m_pDib->GetIptcLegacyInfo()->Country != _T(""))
			{
				if (t != _T(""))
					t += _T(", ");
				t += m_pDoc->m_pDib->GetIptcLegacyInfo()->Country;
			}
			t.Replace(_T("\r\n"), _T(" "));
			t.Replace(_T("\r"), _T(" "));
			t.Replace(_T("\n"), _T(" "));
			t.Replace(_T("\t"), _T(" "));
			t.TrimLeft();
			t.TrimRight();
			if (t != _T(""))
				m_TopArray.Add(t);
		}

		// Headline & Description
		if (DoDisplayState(COsdDlg::DISPLAY_HEADLINEDESCRIPTION))
		{
			t = _T("");
			if (m_pDoc->m_pDib->GetIptcFromXmpInfo()->Headline != _T(""))
			{
				t += m_pDoc->m_pDib->GetIptcFromXmpInfo()->Headline;
			}
			else if (m_pDoc->m_pDib->GetIptcLegacyInfo()->Headline != _T(""))
			{
				t += m_pDoc->m_pDib->GetIptcLegacyInfo()->Headline;
			}
			t.Replace(_T("\r\n"), _T(" "));
			t.Replace(_T("\r"), _T(" "));
			t.Replace(_T("\n"), _T(" "));
			t.Replace(_T("\t"), _T(" "));
			t.TrimLeft();
			t.TrimRight();
			m_sHeadline = t;

			t = _T("");
			if (m_pDoc->m_pDib->GetIptcFromXmpInfo()->Caption != _T(""))
			{
				t += m_pDoc->m_pDib->GetIptcFromXmpInfo()->Caption;
			}
			else if (m_pDoc->m_pDib->GetIptcLegacyInfo()->Caption != _T(""))
			{
				t += m_pDoc->m_pDib->GetIptcLegacyInfo()->Caption;
			}
			t.Replace(_T("\r\n"), _T(" "));
			t.Replace(_T("\r"), _T(" "));
			t.Replace(_T("\n"), _T(" "));
			t.Replace(_T("\t"), _T(" "));
			t.TrimLeft();
			t.TrimRight();
			m_sDescription = t;
		}
		else
		{
			m_sHeadline = _T("");
			m_sDescription = _T("");
		}

		// Exif Camera Data
		m_ExifArray.RemoveAll();
		if (DoDisplayState(COsdDlg::DISPLAY_FLASH)		&&
			m_pDoc->m_pDib->GetExifInfo()->Flash >= 0	&&
			(m_pDoc->m_pDib->GetExifInfo()->Flash & 1))
		{
			m_ExifArray.Add(_T("\u21af"));
		}
		if (DoDisplayState(COsdDlg::DISPLAY_EXPOSURETIME) &&
			m_pDoc->m_pDib->GetExifInfo()->ExposureTime)
		{
			if (m_pDoc->m_pDib->GetExifInfo()->ExposureTime < 0.010f)
				m.Format(_T("%.4f s"), (double)m_pDoc->m_pDib->GetExifInfo()->ExposureTime);
			else
				m.Format(_T("%.3f s"), (double)m_pDoc->m_pDib->GetExifInfo()->ExposureTime);
			t = m;
			if (m_pDoc->m_pDib->GetExifInfo()->ExposureTime <= 0.5f)
			{
				m.Format(_T(" (1/%d)"), Round(1.0 / m_pDoc->m_pDib->GetExifInfo()->ExposureTime));
				t += m;
			}
			if (t != _T(""))
				m_ExifArray.Add(t);
		}
		if (DoDisplayState(COsdDlg::DISPLAY_APERTURE) &&
			m_pDoc->m_pDib->GetExifInfo()->ApertureFNumber)
		{
			m.Format(_T("f/%.1f"), (double)m_pDoc->m_pDib->GetExifInfo()->ApertureFNumber);
			if (m != _T(""))
				m_ExifArray.Add(m);
		}	
	}

	// Empty?
	if (m_TopArray.GetSize() == 0	&&
		m_sHeadline == _T("")		&&
		m_sDescription == _T("")	&&
		m_ExifArray.GetSize() == 0)
	{
		m_bDataToDisplay = FALSE;
		m_nCountdown = 0;
		m_nOpacity = m_nMinOpacity;
		SetOpacity(m_nOpacity);
	}
	else
	{
		m_bDataToDisplay = TRUE;
		if (!IsWindowVisible())
		{
			ShowWindow(SW_SHOWNA);
			SetLayered();
		}
		RestartTimeout();
	}

	// Set Focus to Main Frame
	::SetActiveWindow(::AfxGetMainFrame()->GetSafeHwnd());

	// Auto-Size with next paint
	m_bAutoSizeNow = m_bAutoSize;

	// Invalidate
	Invalidate();
}
