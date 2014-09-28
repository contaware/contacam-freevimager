#include "stdafx.h"
#include "uImager.h"
#include "MainFrm.h"
#include "uImagerDoc.h"
#include "uImagerView.h"
#include "ToolBarChildFrm.h"
#include "LicenseDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CUImagerView

IMPLEMENT_DYNCREATE(CUImagerView, CScrollView)

BEGIN_MESSAGE_MAP(CUImagerView, CScrollView)
	//{{AFX_MSG_MAP(CUImagerView)
	ON_WM_DESTROY()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MBUTTONDBLCLK()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_TIMER()
	ON_WM_MOUSEMOVE()
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_APP_LICENSE, OnAppLicense)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_THREADSAFE_UPDATEWINDOWSIZES, OnThreadSafeUpdateWindowSizes)
	ON_MESSAGE(WM_THREADSAFE_SETDOCUMENTTITLE, OnThreadSafeSetDocumentTitle)
	ON_MESSAGE(WM_FORCE_CURSOR, OnForceCursor)
	ON_MESSAGE(WM_ENABLE_CURSOR, OnEnableCursor)
	ON_MESSAGE(WM_PROGRESS, OnProgress)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUImagerView construction/destruction

CUImagerView::CUImagerView()
{
	m_ZoomRect = CRect(0,0,0,0);
	m_UserZoomRect = CRect(0,0,0,0);
	m_bCursor = TRUE;
	m_bForceCursor = FALSE;
	m_nForceCursorCount = 0;
	m_nMouseMoveCount = 0;
	m_ptMouseMoveLastPoint = CPoint(0,0);
	m_nMouseHideTimerCount = 0;
	m_bFullScreenMode = false;
}

CUImagerView::~CUImagerView()
{
	
}

void CUImagerView::OnInitialUpdate()
{
	SetScrollSizes(MM_TEXT, CSize(0, 0));
	CScrollView::OnInitialUpdate();
}

void CUImagerView::OnDestroy()
{
	CUImagerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// Kill Timer
	KillTimer(ID_TIMER_FULLSCREEN);

	// Make Sure Cursor is Enabled!
	OnEnableCursor((WPARAM)TRUE, (LPARAM)NULL);

	CScrollView::OnDestroy();
}

// The wparam may be a combination of:
// UPDATEWINDOWSIZES_INVALIDATE
// UPDATEWINDOWSIZES_ERASEBKG
// UPDATEWINDOWSIZES_SIZETODOC
LONG CUImagerView::OnThreadSafeUpdateWindowSizes(WPARAM wparam, LPARAM lparam)
{
	DWORD dwFlag = wparam;
	
	CUImagerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	
	if (pDoc && !pDoc->m_bClosing)
	{
		POSITION posView = pDoc->GetFirstViewPosition();
		while (posView != NULL)
		{
			CUImagerView* pView = (CUImagerView*)pDoc->GetNextView(posView);
			ASSERT_VALID(pView);

			// This check is necessary because if in Print Preview
			// the Print Preview View may be returned!
			if (pView && pView->IsKindOf(RUNTIME_CLASS(CUImagerView)))
			{
				pView->UpdateWindowSizes(	(dwFlag & UPDATEWINDOWSIZES_INVALIDATE)	== UPDATEWINDOWSIZES_INVALIDATE,
											(dwFlag & UPDATEWINDOWSIZES_ERASEBKG)	== UPDATEWINDOWSIZES_ERASEBKG,
											(dwFlag & UPDATEWINDOWSIZES_SIZETODOC)	== UPDATEWINDOWSIZES_SIZETODOC);
			}
		}
		return 1;
	}
	else
		return 0;
}

LONG CUImagerView::OnThreadSafeSetDocumentTitle(WPARAM wparam, LPARAM lparam)
{
	CUImagerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	
	if (pDoc &&
		!pDoc->m_bClosing)
	{
		pDoc->SetDocumentTitle();
		return 1;
	}
	else
		return 0;
}

BOOL CUImagerView::IsXAndYScroll()
{
	CSize sizeClient;
	CSize sizeSb;
	if (!GetTrueClientSize(sizeClient, sizeSb))
		return FALSE;

	CSize sizeRange;
	CPoint ptMove;
	CSize needSb;
	GetScrollBarState(sizeClient, needSb, sizeRange, ptMove, TRUE);
	if (needSb.cx && needSb.cy)
		return TRUE;
	else
		return FALSE;
}

BOOL CUImagerView::IsXOrYScroll()
{
	CSize sizeClient;
	CSize sizeSb;
	if (!GetTrueClientSize(sizeClient, sizeSb))
		return FALSE;

	CSize sizeRange;
	CPoint ptMove;
	CSize needSb;
	GetScrollBarState(sizeClient, needSb, sizeRange, ptMove, TRUE);
	if (needSb.cx || needSb.cy)
		return TRUE;
	else
		return FALSE;
}

BOOL CUImagerView::IsXScroll()
{
	CSize sizeClient;
	CSize sizeSb;
	if (!GetTrueClientSize(sizeClient, sizeSb))
		return FALSE;

	CSize sizeRange;
	CPoint ptMove;
	CSize needSb;
	GetScrollBarState(sizeClient, needSb, sizeRange, ptMove, TRUE);
	if (needSb.cx)
		return TRUE;
	else
		return FALSE;
}

BOOL CUImagerView::IsYScroll()
{
	CSize sizeClient;
	CSize sizeSb;
	if (!GetTrueClientSize(sizeClient, sizeSb))
		return FALSE;

	CSize sizeRange;
	CPoint ptMove;
	CSize needSb;
	GetScrollBarState(sizeClient, needSb, sizeRange, ptMove, TRUE);
	if (needSb.cy)
		return TRUE;
	else
		return FALSE;
}

void CUImagerView::UpdateCursor()
{
	CPoint pos;
	::GetCursorPos(&pos);
	if (CWnd::WindowFromPoint(pos) == this)
	{
		ScreenToClient(&pos);
		CRect rcClient;
		GetClientRect(&rcClient);
		if (rcClient.PtInRect(pos))
		{
			PostMessage(WM_SETCURSOR,
						(WPARAM)GetSafeHwnd(),
						MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
		}
	}
}

void CUImagerView::UpdateScrollSize()
{
	// No Scroll!
	SetScrollSizes(MM_TEXT, CSize(0, 0));
}

void CUImagerView::UpdateZoomRect()
{
	CUImagerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	
	// New Zoom Rect
	if (m_bFullScreenMode)
	{
		CRect ClientRect;
		GetClientRect(&ClientRect);
		
		// Fit Zoom Factor
		FitZoomFactor();

		// New Zoom Size
		CSize ZoomedSize(	Round(pDoc->m_DocRect.Width() * pDoc->m_dZoomFactor),
							Round(pDoc->m_DocRect.Height() * pDoc->m_dZoomFactor));

		// Set it
		m_ZoomRect.left = (ClientRect.Width() - ZoomedSize.cx) / 2;
		m_ZoomRect.right = m_ZoomRect.left + ZoomedSize.cx;
		m_ZoomRect.top = (ClientRect.Height() - ZoomedSize.cy) / 2;
		m_ZoomRect.bottom = m_ZoomRect.top + ZoomedSize.cy;
	}
	else
		GetClientRect(&m_ZoomRect);
}

BOOL CUImagerView::UpdateWindowSizes(BOOL bInvalidate,
									 BOOL bEraseBkg,
									 BOOL bSizeToDoc)
{
	CUImagerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// Make sure that the ToolBar is already initialized
	if (((CToolBarChildFrame*)GetParentFrame())->GetToolBar())
	{
		// Get Client Frame Rect
		CRect rcc_frame;
		GetParentFrame()->GetClientRect(&rcc_frame);

		// Size to Doc
		if (bSizeToDoc &&
			pDoc->m_DocRect.Width() != 0	&&
			pDoc->m_DocRect.Height() != 0	&&
			!m_bFullScreenMode				&&
			!GetParentFrame()->IsZoomed())
		{
			// Remove Scrolls to calculate the right fitting
			SetScrollSizes(MM_TEXT, CSize(0, 0));

			// Get View Rect
			CRect rcw;
			GetWindowRect(&rcw);

			// Get View Client Rect
			CRect rcc;
			GetClientRect(&rcc);

			// Get View Borders
			CSize szViewBorder;
			szViewBorder.cx = rcw.Width() - rcc.Width();
			szViewBorder.cy = rcw.Height() - rcc.Height();

			// Doc Size
			CSize szDoc;
			szDoc.cx = pDoc->m_DocRect.right;
			szDoc.cy = pDoc->m_DocRect.bottom;

			// Document Borders
			if (!pDoc->m_bNoBorders)
			{
				szDoc.cx += 2 * MIN_BORDER;
				szDoc.cy += 2 * MIN_BORDER;
			}

			// Set Max ToolBar Width
			((CToolBarChildFrame*)GetParentFrame())->GetToolBar()->m_nMaxToolbarWidth = szDoc.cx + szViewBorder.cx;

			// Make sure the Min ToolBar Width is respected
			if (((CToolBarChildFrame*)GetParentFrame())->GetToolBar()->m_nMaxToolbarWidth <
				((CToolBarChildFrame*)GetParentFrame())->GetToolBar()->m_nMinToolbarWidth)
			{
				int nBorderWidth =	((CToolBarChildFrame*)GetParentFrame())->GetToolBar()->m_nMinToolbarWidth -
									((CToolBarChildFrame*)GetParentFrame())->GetToolBar()->m_nMaxToolbarWidth;

				((CToolBarChildFrame*)GetParentFrame())->GetToolBar()->m_nMaxToolbarWidth = 
				((CToolBarChildFrame*)GetParentFrame())->GetToolBar()->m_nMinToolbarWidth;
				
				double dRatio = (double)pDoc->m_DocRect.Height() / (double)pDoc->m_DocRect.Width();
				szDoc.cx += nBorderWidth;
				szDoc.cy += Round(nBorderWidth * dRatio);
			}

			// Update ToolBar and get its size
			CRect rcw_toolbar;
			(((CToolBarChildFrame*)GetParentFrame())->GetToolBar())->UpdateControls();
			((CToolBarChildFrame*)GetParentFrame())->GetToolBar()->GetWindowRect(&rcw_toolbar);	

			// Get Frame Borders
			CRect rcw_frame;
			GetParentFrame()->GetWindowRect(&rcw_frame);
			CSize szFrameBorder;
			szFrameBorder.cx = rcw_frame.Width() - rcc_frame.Width();
			szFrameBorder.cy = rcw_frame.Height() - rcc_frame.Height();

			// Clip to MDI Rect
			WINDOWPLACEMENT wndpl;
			memset(&wndpl, 0, sizeof(wndpl));
			wndpl.length = sizeof(wndpl);
			GetParentFrame()->GetWindowPlacement(&wndpl);
			wndpl.rcNormalPosition.right = wndpl.rcNormalPosition.left + szDoc.cx + szViewBorder.cx + szFrameBorder.cx;
			wndpl.rcNormalPosition.bottom = wndpl.rcNormalPosition.top + szDoc.cy + szViewBorder.cy + szFrameBorder.cy + rcw_toolbar.Height();
			::AfxGetMainFrame()->ClipToMDIRect(&wndpl.rcNormalPosition);

			// Frame Width and Height
			int nFrameWidth = wndpl.rcNormalPosition.right - wndpl.rcNormalPosition.left;
			int nFrameHeight = wndpl.rcNormalPosition.bottom - wndpl.rcNormalPosition.top;

			// Set the new Frame Position
			GetParentFrame()->SetWindowPos(	NULL,
											wndpl.rcNormalPosition.left,
											wndpl.rcNormalPosition.top,
											nFrameWidth,
											nFrameHeight,
											SWP_NOZORDER);

			// Set Max ToolBar Width, Update ToolBar and get its size
			((CToolBarChildFrame*)GetParentFrame())->GetToolBar()->m_nMaxToolbarWidth = nFrameWidth - szFrameBorder.cx;
			(((CToolBarChildFrame*)GetParentFrame())->GetToolBar())->UpdateControls();
			((CToolBarChildFrame*)GetParentFrame())->GetToolBar()->GetWindowRect(&rcw_toolbar);

			// Set the ToolBar Size
			((CToolBarChildFrame*)GetParentFrame())->GetToolBar()->SetWindowPos(NULL,
																				0,
																				0,
																				((CToolBarChildFrame*)GetParentFrame())->GetToolBar()->m_nMaxToolbarWidth,
																				rcw_toolbar.Height(),
																				SWP_NOMOVE | SWP_NOZORDER);
		}
		else
		{
			((CToolBarChildFrame*)GetParentFrame())->GetToolBar()->m_nMaxToolbarWidth = rcc_frame.Width();
			(((CToolBarChildFrame*)GetParentFrame())->GetToolBar())->UpdateControls();
		}
	}

	// Set the Scroll Sizes
	UpdateScrollSize();

	// Update Zoom Rectangle
	UpdateZoomRect();

	// Invalidate
	if (bInvalidate)
	{
		if (bEraseBkg)
			Invalidate(TRUE);
		else
		{
			CRect rcClient;
			GetClientRect(&rcClient);
			InvalidateRect(rcClient, FALSE);
		}
	}
	else
	{
		if (bEraseBkg)
			InvalidateRect(CRect(0,0,0,0), TRUE);
	}

	return TRUE;
}

// Called From inside CToolBarChildFrame::OnSize(),
// and from all functions that change the doc size!
void CUImagerView::FitZoomFactor()
{
	CUImagerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// Remove Scrolls to calculate the right fitting!
	SetScrollSizes(MM_TEXT, CSize(0, 0));

	double dZoomFactorX, dZoomFactorY, dZoomFactor;
	CRect rcClient;
	GetClientRect(rcClient);
	if ((pDoc->m_DocRect.Width() == 0) || (pDoc->m_DocRect.Height() == 0))
	{
		dZoomFactorX = 1.0;
		dZoomFactorY = 1.0;
	}
	else
	{
		dZoomFactorX = (double)(rcClient.Width()) / (double)(pDoc->m_DocRect.Width());
		dZoomFactorY = (double)(rcClient.Height()) / (double)(pDoc->m_DocRect.Height());
	}

	dZoomFactor = MIN(dZoomFactorX, dZoomFactorY);

	// 5% Less to have some Borders
	if (!m_bFullScreenMode &&
		!pDoc->m_bNoBorders)
		dZoomFactor *= FIT_ZOOMFACTOR_MARGIN;

	pDoc->m_dZoomFactor = dZoomFactor;
}

// Called From inside CToolBarChildFrame::OnSize(),
// and from all functions that change the doc size!
void CUImagerView::FitBigZoomFactor()
{
	CUImagerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// Remove Scrolls to calculate the right fitting!
	SetScrollSizes(MM_TEXT, CSize(0, 0));

	double dZoomFactorX, dZoomFactorY, dZoomFactor;
	CRect rcClient;
	GetClientRect(rcClient);
	if ((pDoc->m_DocRect.Width() == 0) || (pDoc->m_DocRect.Height() == 0))
	{
		dZoomFactorX = 1.0;
		dZoomFactorY = 1.0;
	}
	else
	{
		dZoomFactorX = (double)(rcClient.Width()) / (double)(pDoc->m_DocRect.Width());
		dZoomFactorY = (double)(rcClient.Height()) / (double)(pDoc->m_DocRect.Height());
	}

	dZoomFactor = MIN(dZoomFactorX, dZoomFactorY);

	// 5% Less to have some Margin
	if (!m_bFullScreenMode &&
		!pDoc->m_bNoBorders)
		dZoomFactor *= FIT_ZOOMFACTOR_MARGIN;

	// Max Zoom Factor is 1.0
	if (dZoomFactor > 1.0)
		dZoomFactor = 1.0;
	
	pDoc->m_dZoomFactor = dZoomFactor;
}

void CUImagerView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	EnableCursor();
	::AfxGetMainFrame()->EnterExitFullscreen();
	CScrollView::OnLButtonDblClk(nFlags, point);
}

void CUImagerView::OnMButtonDblClk(UINT nFlags, CPoint point) 
{
	EnableCursor();	
	CScrollView::OnMButtonDblClk(nFlags, point);
}

void CUImagerView::OnRButtonDblClk(UINT nFlags, CPoint point) 
{
	EnableCursor();	
	CScrollView::OnRButtonDblClk(nFlags, point);
}

void CUImagerView::OnTimer(UINT nIDEvent) 
{
	CUImagerDoc* pDoc = (CUImagerDoc*)GetDocument();
	ASSERT_VALID(pDoc);

	switch (nIDEvent)
	{
		case ID_TIMER_FULLSCREEN :
		{
			m_nMouseMoveCount = 0;

			if (m_bFullScreenMode	&&
				!m_bForceCursor		&&
				m_bCursor)
				m_nMouseHideTimerCount++;
			else
				m_nMouseHideTimerCount = 0;

			if (m_nMouseHideTimerCount > MOUSE_HIDE_AFTER_SEC)
			{
				// Hide Cursor
				EnableCursor(FALSE);
				m_nMouseHideTimerCount = 0;
			}

			break;
		}
		default:
			break;
	}
	
	CScrollView::OnTimer(nIDEvent);
}

LONG CUImagerView::OnProgress(WPARAM wparam, LPARAM lparam)
{
	lparam;
	if (::AfxGetMainFrame()->MDIGetActive() &&
		::AfxGetMainFrame()->MDIGetActive()->GetActiveView()->GetSafeHwnd() == GetSafeHwnd())
	{
		::AfxGetMainFrame()->Progress((int)wparam);
		return 1;
	}
	else
		return 0;
}

void CUImagerView::ForceCursor(BOOL bForce/*=TRUE*/)
{
	PostMessage(WM_FORCE_CURSOR, bForce, 0);
}

void CUImagerView::EnableCursor(BOOL bEnable/*=TRUE*/)
{
	PostMessage(WM_ENABLE_CURSOR, bEnable, 0);
}

LONG CUImagerView::OnForceCursor(WPARAM wparam, LPARAM lparam)
{
	BOOL bForce = (BOOL)wparam;

	if (bForce)
	{
		m_nForceCursorCount++;
		if (m_nForceCursorCount > 0)
		{
			EnableCursor();
			m_bForceCursor = TRUE;
		}
	}
	else
	{
		m_nForceCursorCount--;
		if (m_nForceCursorCount <= 0)
		{
			m_nMouseHideTimerCount = 0;
			m_bForceCursor = FALSE;
		}
	}

	return 0;
}

LONG CUImagerView::OnEnableCursor(WPARAM wparam, LPARAM lparam)
{
	BOOL bEnable = (BOOL)wparam;

	if (bEnable)
	{
		if (!m_bCursor)
		{
			::ShowCursor(TRUE);
			m_bCursor = TRUE;
		}
	}
	else
	{
		if (m_bCursor)
		{
			::ShowCursor(FALSE);
			m_bCursor = FALSE;
		}
	}

	return 0;
}

void CUImagerView::OnMouseMove(UINT nFlags, CPoint point) 
{
	CUImagerDoc* pDoc = (CUImagerDoc*)GetDocument();
	ASSERT_VALID(pDoc);

	// Full-Screen Cursor Enable / Disable
	if (m_bFullScreenMode)
	{
		if (m_ptMouseMoveLastPoint != point)
			m_nMouseMoveCount++;
	}
	else
		m_nMouseMoveCount = 0;
	if (m_nMouseMoveCount >= MOUSE_HIDE_MOVECOUNT_THRESHOLD)
	{
		// Show Cursor
		if (!m_bCursor)
			EnableCursor();
		m_nMouseMoveCount = 0;
		m_nMouseHideTimerCount = 0;
	}

	// Update Last Point
	m_ptMouseMoveLastPoint = point;

	// Update Pane Text
	UpdatePaneText();

	CScrollView::OnMouseMove(nFlags, point);
}

void CUImagerView::UpdatePaneText()
{
	CUImagerDoc* pDoc = (CUImagerDoc*)GetDocument();
	ASSERT_VALID(pDoc);

	// Show X, Y Coordinates
	CString sXCoord, sYCoord;
	CPoint ptCoord = m_ptMouseMoveLastPoint;
	ptCoord += GetScrollPosition();
	ptCoord -= m_ZoomRect.TopLeft();
	ptCoord.x = (int)floor((double)ptCoord.x / pDoc->m_dZoomFactor);
	ptCoord.y = (int)floor((double)ptCoord.y / pDoc->m_dZoomFactor);
	if (((CUImagerApp*)::AfxGetApp())->m_nCoordinateUnit == COORDINATES_PIX)
	{
		sXCoord.Format(_T("X: %dpx"), ptCoord.x);
		sYCoord.Format(_T("Y: %dpx"), ptCoord.y);
	}
	else if (((CUImagerApp*)::AfxGetApp())->m_nCoordinateUnit == COORDINATES_INCH)
	{
		int nXDpi = 0;
		if (pDoc->m_pDib)
			nXDpi = pDoc->m_pDib->GetXDpi();
		if (nXDpi <= 0)
			nXDpi = DEFAULT_DPI;
		sXCoord.Format(_T("X: %0.2fin"), (double)ptCoord.x / (double)nXDpi);
		int nYDpi = 0;
		if (pDoc->m_pDib)
			nYDpi = pDoc->m_pDib->GetYDpi();
		if (nYDpi <= 0)
			nYDpi = DEFAULT_DPI;
		sYCoord.Format(_T("Y: %0.2fin"), (double)ptCoord.y / (double)nYDpi);
	}
	else
	{
		int nXDpi = 0;
		if (pDoc->m_pDib)
			nXDpi = pDoc->m_pDib->GetXDpi();
		if (nXDpi <= 0)
			nXDpi = DEFAULT_DPI;
		sXCoord.Format(_T("X: %0.2fcm"), (double)ptCoord.x * 2.54 / (double)nXDpi);
		int nYDpi = 0;
		if (pDoc->m_pDib)
			nYDpi = pDoc->m_pDib->GetYDpi();
		if (nYDpi <= 0)
			nYDpi = DEFAULT_DPI;
		sYCoord.Format(_T("Y: %0.2fcm"), (double)ptCoord.y * 2.54 / (double)nYDpi);
	}
	::AfxGetMainFrame()->GetStatusBar()->SetPaneText(2, sXCoord);
	::AfxGetMainFrame()->GetStatusBar()->SetPaneText(3, sYCoord);
}

int CUImagerView::DoRealizePalette(BOOL bForceBackground)
{
	CUImagerDoc* pDoc = (CUImagerDoc*)GetDocument();
	ASSERT_VALID(pDoc);

	if (!pDoc->m_pDib)
		return 0;

	CPalette* pPalette = pDoc->m_pDib->GetPreviewDib() ?
			pDoc->m_pDib->GetPreviewDib()->GetPalette() : pDoc->m_pDib->GetPalette();
	if (pPalette == NULL)
		return 0; // the DIB has no palette

	CMainFrame* pFrame = (CMainFrame*)::AfxGetMainFrame();
	ASSERT_VALID(pFrame);

	CClientDC appDC(pFrame);
	CPalette* oldPalette = appDC.SelectPalette(pPalette, bForceBackground);
	int nColorsChanged = 0;
	if (oldPalette != NULL)
	{
		nColorsChanged = appDC.RealizePalette();
		if (nColorsChanged > 0)
		{
			pDoc->UpdateAllViews(NULL);
		}
		appDC.SelectPalette(oldPalette, TRUE);
	}
	else
	{
		TRACE(_T("\tSelectPalette failed in CUImagerView::DoRealizePalette\n"));
	}
	
	return nColorsChanged;
}

void CUImagerView::OnDraw(CDC* pDC) 
{
	// TODO: Add your specialized code here and/or call the base class
}

void CUImagerView::OnAppAbout() 
{
	ForceCursor();
	
	// Clickable Links only in Normal Screen Mode!
	CAboutDlg aboutDlg(!m_bFullScreenMode);
	aboutDlg.DoModal();

	ForceCursor(FALSE);
}

void CUImagerView::OnAppLicense() 
{
	ForceCursor();

	CLicenseDlg licenseDlg;
	licenseDlg.DoModal();

	ForceCursor(FALSE);
}

void CUImagerView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{
	CUImagerDoc* pDoc = (CUImagerDoc*)GetDocument();
	ASSERT_VALID(pDoc);

	if (!bActivate)
	{
		// Done (=100%)
		::AfxGetMainFrame()->Progress((int)100);
	}

	CScrollView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

// Enable Tablet Press-and-Hold Gesture, see:
// http://support.microsoft.com/kb/2846829
// Virtual function in CWnd is implemented like this:
// ULONG CWnd::GetGestureStatus(CPoint /*ptTouch*/)
// {
//     return TABLET_DISABLE_PRESSANDHOLD;
// }
ULONG CUImagerView::GetGestureStatus(CPoint /*ptTouch*/)
{
	return 0;
}


/////////////////////////////////////////////////////////////////////////////
// CUImagerView diagnostics

#ifdef _DEBUG
void CUImagerView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CUImagerView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CUImagerDoc* CUImagerView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CUImagerDoc)));
	return (CUImagerDoc*)m_pDocument;
}
#endif //_DEBUG
