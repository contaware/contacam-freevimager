// PicturePrintPreviewView.cpp 

#include "stdafx.h"
#include "uImager.h"
#include "MainFrm.h"
#include "resource.h"
#include "PictureView.h"
#include "PicturePrintPreviewView.h"
#include "PrintPreviewScaleEdit.h"
#include "NumSpinCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CPicturePrintPreviewView, CPreviewView)

CPicturePrintPreviewView::CPicturePrintPreviewView()
{
	// Replace the PAGE_INFO array with our one to make sure its large enough
	m_pPageInfo = m_pageInfoArray2;
	m_Across = 1;		// default number of pages across the screen
	m_Down = 1;			// default number of pages down the screen
	m_nPages = 1;
	m_bGotScrollLines = FALSE;
	m_uCachedScrollLines = 3;
	m_pScaleEdit = NULL;
	m_pScaleSpin = NULL;
	m_ptStartClickPosition = CPoint(0, 0);
	m_bMouseCaptured = FALSE;
	m_hGrabClosedCursor = NULL;
	m_hGrabNormalCursor = NULL;
	m_rcPrevMemDC = CRect(0, 0, 0, 0);
	m_bInchPaperFormat = FALSE;
	m_ptMouseMoveLastPoint = CPoint(0, 0);
}

CPicturePrintPreviewView::~CPicturePrintPreviewView()
{
	if (m_pScaleEdit)
		delete m_pScaleEdit;
	if (m_pScaleSpin)
		delete m_pScaleSpin;
}

BEGIN_MESSAGE_MAP(CPicturePrintPreviewView, CPreviewView)
	//{{AFX_MSG_MAP(CPicturePrintPreviewView)
	ON_WM_CREATE()
	ON_COMMAND(AFX_ID_PREVIEW_NUMPAGE, OnNumPageChange)
	ON_COMMAND(AFX_ID_PREVIEW_ZOOMIN, OnZoomIn)
	ON_COMMAND(AFX_ID_PREVIEW_ZOOMOUT, OnZoomOut)
	ON_UPDATE_COMMAND_UI(AFX_ID_PREVIEW_NUMPAGE, OnUpdateNumPageChange)
	ON_UPDATE_COMMAND_UI(AFX_ID_PREVIEW_ZOOMIN, OnUpdateZoomIn)
	ON_UPDATE_COMMAND_UI(AFX_ID_PREVIEW_ZOOMOUT, OnUpdateZoomOut)
	ON_WM_NCLBUTTONDOWN()
	ON_WM_NCLBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_SETCURSOR()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
	ON_WM_VSCROLL()
	ON_COMMAND(IDC_RADIO_ORIENTATION_PORTRAIT, OnPortrait)
	ON_COMMAND(IDC_RADIO_ORIENTATION_LANDSCAPE, OnLandscape)
	ON_COMMAND(IDC_CHECK_PRINTMARGIN, OnPrintMargin)
	ON_COMMAND(IDC_CHECK_SIZE_FIT, OnSizeFit)
	ON_COMMAND(ID_BUTTON_PRINT_SETUP, OnPrintSetup)
	//ON_COMMAND(ID_PREVIEW_PAGES, OnPreviewPages)
	ON_MESSAGE(WM_GESTURE, OnGesture)
END_MESSAGE_MAP()

BOOL CPicturePrintPreviewView::PreCreateWindow(CREATESTRUCT& cs)
{
	if (cs.lpszClass == NULL)
		cs.lpszClass = AfxRegisterWndClass(CS_DBLCLKS|CS_HREDRAW|CS_VREDRAW|CS_BYTEALIGNCLIENT);
	return CPreviewView::PreCreateWindow(cs);
}

#ifdef _DEBUG
void CPicturePrintPreviewView::AssertValid() const
{
	// Bug nearby Article ID: Q192853 
	//	CPreviewView::AssertValid();
}

void CPicturePrintPreviewView::Dump(CDumpContext& dc) const
{
	TRACE(_T("Dump\n"));
	CPreviewView::Dump(dc);
}

CPictureDoc* CPicturePrintPreviewView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CPictureDoc)));
	return (CPictureDoc*)m_pDocument;
}
#endif //_DEBUG

BOOL CPicturePrintPreviewView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CPoint point;
	::GetCursorPos(&point);
	ScreenToClient(&point); // Client coordinates of mouse position
	CRect rcClient;
	GetClientRect(&rcClient);
	
	UINT nPage;
	if (rcClient.PtInRect(point) && FindPageRect(point, nPage))	// FindPageRect() Changes point and nPage!
	{
		if (m_bMouseCaptured)
		{
			if (m_hGrabClosedCursor == NULL)
				m_hGrabClosedCursor = ::AfxGetApp()->LoadCursor(IDC_GRAB_CLOSED);
			::SetCursor(m_hGrabClosedCursor);
			return TRUE;
		}
		else
		{
			if (m_hGrabNormalCursor == NULL)
				m_hGrabNormalCursor = ::AfxGetApp()->LoadCursor(IDC_GRAB_NORMAL);
			::SetCursor(m_hGrabNormalCursor);
			return TRUE;
		}
	}
	else
		return CPreviewView::OnSetCursor(pWnd, nHitTest, message);
}

// Finds page pointed to and convert to 1:1 screen device units
BOOL CPicturePrintPreviewView::FindPageRect(CPoint& point, UINT& nPage)
{
	// Original
	//if (m_nZoomState != ZOOM_OUT)
	//	point += (CSize)GetDeviceScrollPosition();

	// New
	if (m_nZoomState != ZOOM_OUT)
	{
		CPoint ptOffset = -GetDeviceScrollPosition();
		if (m_bCenter)
		{
			CRect rect;
			GetClientRect(&rect);
			// if client area is larger than total device size,
			// override scroll positions to place origin such that
			// output is centered in the window
			if (m_totalDev.cx < rect.Width())
				ptOffset.x = (rect.Width() - m_totalDev.cx) / 2;
			if (m_totalDev.cy < rect.Height())
				ptOffset.y = (rect.Height() - m_totalDev.cy) / 2;
		}
		point -= ptOffset;
	}


	for (nPage = 0; nPage < m_nPages; nPage++)
	{
		if (m_pPageInfo[nPage].rectScreen.PtInRect(point))
		{
			// adjust point for page position
			point -= (CSize)m_pPageInfo[nPage].rectScreen.TopLeft();

			// convert to 1:1
			point.x = MulDiv(point.x, m_pPageInfo[nPage].sizeScaleRatio.cy,
									m_pPageInfo[nPage].sizeScaleRatio.cx);
			point.y = MulDiv(point.y, m_pPageInfo[nPage].sizeScaleRatio.cy,
									m_pPageInfo[nPage].sizeScaleRatio.cx);
			return TRUE;
		}
	}

	return FALSE;
}

void CPicturePrintPreviewView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// To Remove Focus From CDialogBar's Edit Box in Print Preview Mode
	SetFocus();

	// Point On Page?
	CPoint pt = point;
	UINT nPage;
	if (!FindPageRect(pt, nPage))	// This Function Changes pt and nPage!
	{
		return;
	}

	// Mouse Captured
	m_ptStartClickPosition = point;
	m_bMouseCaptured = TRUE;
	::SendMessage(	GetSafeHwnd(),
					WM_SETCURSOR,
					(WPARAM)GetSafeHwnd(),
					MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
	SetCapture();

	/* Old Code
	// Set new zoom state
	SetZoomState((m_nZoomState == ZOOM_IN_400) ? ZOOM_OUT : m_nZoomState + 1, nPage, point);
	if (m_nZoomState == ZOOM_OUT)
	{
		// make sure the current page does not cause blank pages to be shown
		if (m_nCurrentPage >= m_pPreviewInfo->GetMaxPage() - m_nPages)
			m_nCurrentPage = m_pPreviewInfo->GetMaxPage() - m_nPages + 1;
		if (m_nCurrentPage < 0)
			m_nCurrentPage = 0;
	}
	*/
}

void CPicturePrintPreviewView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_bMouseCaptured)
	{
		ReleaseCapture();
		m_bMouseCaptured = FALSE;
		GetDocument()->m_ptLastPrintOffset = GetDocument()->m_ptPrintOffset;
		::SendMessage(	GetSafeHwnd(),
						WM_SETCURSOR,
						(WPARAM)GetSafeHwnd(),
						MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
	}
}

void CPicturePrintPreviewView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// To Remove Focus From CDialogBar's Edit Box in Print Preview Mode
	SetFocus();

	/* Old Code
	OnLButtonDown(nFlags, point);
	*/ 
}

void CPicturePrintPreviewView::SetZoomState(UINT nNewState, UINT nPage, CPoint point)
{
	if (m_nZoomState != nNewState)
	{
		m_nZoomState = nNewState;
		DoZoom(nPage, point);
	}
}

// Actual zoom code
void CPicturePrintPreviewView::DoZoom(UINT nPage, CPoint point)
{
	if (m_nZoomState == ZOOM_OUT)
	{
		// Taking over scroll bars
		m_nPages = m_nZoomOutPages;
		
		BOOL bShowBar = m_pPreviewInfo->GetMaxPage() < 0x8000 &&
						m_pPreviewInfo->GetMaxPage() -
						m_pPreviewInfo->GetMinPage() <= 32767U;
		
		ShowScrollBar(SB_VERT, bShowBar);	// Show the vertical bar
		
		if (bShowBar)
		{
			SCROLLINFO info;
			info.fMask = SIF_PAGE | SIF_RANGE;
			info.nMin = m_pPreviewInfo->GetMinPage();
			info.nMax = m_pPreviewInfo->GetMaxPage();
			info.nPage = 1;
			if (!SetScrollInfo(SB_VERT, &info, FALSE))
				SetScrollRange(SB_VERT, info.nMin, info.nMax, FALSE);
		}
		
		SetCurrentPage(m_nCurrentPage, TRUE);
		SetupScrollbar();

		ShowScrollBar(SB_HORZ, FALSE);		// Hide the horizontal bar
	}
	else
	{
		m_nPages = 1;	// Only one page in zoomed states
		m_pPageInfo[0].sizeZoomOutRatio = m_pPageInfo[nPage].sizeZoomOutRatio;
		m_pPageInfo[0].sizeUnscaled = m_pPageInfo[nPage].sizeUnscaled;
		
		// Sets the printer page
		SetCurrentPage(m_nCurrentPage + nPage, FALSE);
		
		SetScaledSize(0);
		
		CSize* pRatio = &m_pPageInfo[nPage].sizeScaleRatio;
		
		// Convert Hit Point from screen 1:1
		point.x = MulDiv(point.x, pRatio->cx, pRatio->cy);
		point.y = MulDiv(point.y, pRatio->cx, pRatio->cy);
		
		// Adjust point for page position
		point += (CSize)m_pPageInfo[0].rectScreen.TopLeft();
		
		// Scroll to center
		CenterOnPoint(point);
	}
}

void CPicturePrintPreviewView::PositionPage(UINT nPage)
{
	CSize windowSize = CalcPageDisplaySize();
	
	VERIFY(m_dcPrint.Escape(GETPHYSPAGESIZE, 0, NULL, (LPVOID)&m_pPageInfo[nPage].sizeUnscaled));
	
	CSize* pSize = &m_pPageInfo[nPage].sizeUnscaled;
	
	// Convert page size to screen coordinates
	pSize->cx = MulDiv(pSize->cx, afxData.cxPixelsPerInch, m_sizePrinterPPI.cx);
	pSize->cy = MulDiv(pSize->cy, afxData.cyPixelsPerInch, m_sizePrinterPPI.cy);
	
	m_pPageInfo[nPage].sizeZoomOutRatio = CalcScaleRatio(windowSize, *pSize);
	
	SetScaledSize(nPage);
}

void CPicturePrintPreviewView::SetScaledSize(UINT nPage)
{
	CSize* pSize = &m_pPageInfo[nPage].sizeUnscaled;
	CSize* pRatio = &m_pPageInfo[nPage].sizeScaleRatio;
	CSize* pZoomOutRatio = &m_pPageInfo[nPage].sizeZoomOutRatio;
	CSize windowSize = CalcPageDisplaySize();
	BOOL bPaperLarger = pZoomOutRatio->cx < pZoomOutRatio->cy;
	// whether the paper is larger than the screen, or vice versa

	switch (m_nZoomState)
	{
		case ZOOM_OUT:
			*pRatio = *pZoomOutRatio;
			break;
			
		case ZOOM_MIDDLE:
			pRatio->cy = pZoomOutRatio->cy;
			pRatio->cx = Round(pZoomOutRatio->cx * 1.2);
			break;
			
		case ZOOM_IN:
			pRatio->cy = pZoomOutRatio->cy;
			pRatio->cx = Round(pZoomOutRatio->cx * 1.4);
			break;

		case ZOOM_IN_160:
			pRatio->cy = pZoomOutRatio->cy;
			pRatio->cx = Round(pZoomOutRatio->cx * 1.6);
			break;
			
		case ZOOM_IN_180:
		pRatio->cy = pZoomOutRatio->cy;
			pRatio->cx = Round(pZoomOutRatio->cx * 1.8);
			break;
			
		case ZOOM_IN_200:
			pRatio->cy = pZoomOutRatio->cy;
			pRatio->cx = pZoomOutRatio->cx * 2;
			break;

		/* Original Code
		case ZOOM_OUT:
			*pRatio = *pZoomOutRatio;
			break;
			
		case ZOOM_MIDDLE:
			// the middle zoom state is a ratio between cx/cy and
			// 1/1 (or cy/cy).  It is, therefore:
			//
			// (cx + cy)/2
			// -----------
			//     cy
			//
			// if the paper is larger than the screen, or
			//
			// (3*cx - cy)/2
			// -------------
			//      cy
			//
			// if the paper is smaller than the screen.
			if (bPaperLarger)
			{
				pRatio->cy = pZoomOutRatio->cy;
				pRatio->cx = (pZoomOutRatio->cx + pRatio->cy) / 2;
			}
			else
			{
				pRatio->cy = pZoomOutRatio->cy;
				pRatio->cx = (3*pZoomOutRatio->cx - pRatio->cy) / 2;
			}
			break;
			
		case ZOOM_IN:
			if (bPaperLarger)
			{
				pRatio->cx = pRatio->cy = 1;
			}
			else
			{
				// if the paper is smaller than the screen space we're displaying
				// it in, then using a ratio of 1/1 will result in a smaller image
				// on the screen, not a larger one. To get a larger image in this
				// case we double the zoom out ratio.
				pRatio->cy = pZoomOutRatio->cy;
				pRatio->cx = 2*pZoomOutRatio->cx - pZoomOutRatio->cy;
			}
			break;
		*/
		default:
			ASSERT(FALSE);
	}
	
	// Convert to scaled size
	CSize scaledSize;
	scaledSize.cx = MulDiv(pSize->cx, pRatio->cx, pRatio->cy);
	scaledSize.cy = MulDiv(pSize->cy, pRatio->cx, pRatio->cy);
	
	CRect* pRect = &m_pPageInfo[nPage].rectScreen;
	pRect->SetRect(	PREVIEW_MARGIN, PREVIEW_MARGIN,
					scaledSize.cx + PREVIEW_MARGIN + 3,
					scaledSize.cy + PREVIEW_MARGIN + 3);
	
	if (m_nZoomState == ZOOM_OUT)
	{
		pRect->OffsetRect(	(windowSize.cx - pRect->Size().cx) / 2 - 1,
							(windowSize.cy - pRect->Size().cy) / 2 - 1);

		// We need to offset the page multiple times
		int local = nPage % m_Across;
		while (local-- >= 1)
		{
			pRect->OffsetRect(m_PageOffset.x, 0);
		}
		local = nPage / m_Across;
		while (local-- > 0)
		{
			pRect->OffsetRect(0, m_PageOffset.y);
		}

		// Clear scroll size
		SetScrollSizes(MM_TEXT, CSize(0, 0));
	}
	else
	{
		// Set up scroll size
		SetScrollSizes(MM_TEXT, pRect->Size() +
								CSize(	PREVIEW_MARGIN * 2,
										PREVIEW_MARGIN * 2),
								windowSize);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CPicturePrintPreviewView

void CPicturePrintPreviewView::OnUpdateNumPageChange(CCmdUI* pCmdUI)
{
	// button has been removed from toolbar
	//UINT nPages = m_nZoomState == ZOOM_OUT ? m_nPages : m_nZoomOutPages;
	//pCmdUI->Enable(m_nZoomState == ZOOM_OUT && m_nMaxPages != 1 && (m_pPreviewInfo->GetMaxPage() > 1 || m_nPages > 1));
	//	CPreviewView::OnUpdateNumPageChange(pCmdUI);
}

void CPicturePrintPreviewView::OnUpdateZoomIn(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_nZoomState != ZOOM_IN_200);
}

void CPicturePrintPreviewView::OnUpdateZoomOut(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_nZoomState != ZOOM_OUT);
}

void CPicturePrintPreviewView::OnNumPageChange() 
{
	// doesn't do anything any more
	// button has been removed from toolbar
}

void CPicturePrintPreviewView::OnZoomIn() 
{
	if (m_nZoomState != ZOOM_IN_200)
		SetZoomState(m_nZoomState + 1, 0, CPoint(0, 0));
}

void CPicturePrintPreviewView::OnZoomOut() 
{
	if (m_nZoomState != ZOOM_OUT)
		SetZoomState(m_nZoomState - 1, 0, CPoint(0, 0));
}

int CPicturePrintPreviewView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CPreviewView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Subclass the DialogBar's Scale Edit And Scale Spin Controls
	m_pScaleEdit = new CPrintPreviewScaleEdit(GetDocument(), this);
	if (m_pScaleEdit)
	{
		m_pScaleEdit->SubclassDlgItem(IDC_EDIT_SIZE, m_pToolBar);
		m_pScaleEdit->SetMinNumberOfNumberAfterPoint(1);
		m_pScaleEdit->SetMaxNumberOfNumberAfterPoint(1);
	}
	m_pScaleSpin = new CNumSpinCtrl;
	if (m_pScaleSpin)
	{
		m_pScaleSpin->SubclassDlgItem(IDC_SPIN_SIZE, m_pToolBar);
		m_pScaleSpin->SetBuddy(m_pScaleEdit);
		m_pScaleSpin->SetRange(MIN_PRINT_SCALE, MAX_PRINT_SCALE);
		m_pScaleSpin->SetDelta(0.1);
	}
	if (m_pScaleEdit)
		m_pScaleEdit->SetPrintScale(GetDocument()->m_dPrintScale);

	// Subclass the zoom in & out buttons
	m_ZoomInButton.SubclassDlgItem(AFX_ID_PREVIEW_ZOOMIN, m_pToolBar);
	m_ZoomOutButton.SubclassDlgItem(AFX_ID_PREVIEW_ZOOMOUT, m_pToolBar);
	m_ZoomInButton.SetIcon(IDI_MAGNIFYPLUS, CXButtonXP::LEFT);
	m_ZoomOutButton.SetIcon(IDI_MAGNIFYMINUS, CXButtonXP::LEFT);

	// Draw Margin Check Box
	CButton* pCheckPrintMargin = (CButton*)m_pToolBar->GetDlgItem(IDC_CHECK_PRINTMARGIN);
	pCheckPrintMargin->SetCheck(GetDocument()->m_bPrintMargin);

	// Fit Size
	FitSize();
	
	// Init the Portrait Landscape Radio Buttons
	int orientation = GetPrintOrientation();
	CButton* pRadioPortrait = (CButton*)m_pToolBar->GetDlgItem(IDC_RADIO_ORIENTATION_PORTRAIT);
	CButton* pRadioLandscape = (CButton*)m_pToolBar->GetDlgItem(IDC_RADIO_ORIENTATION_LANDSCAPE);
	if (orientation == DMORIENT_PORTRAIT)
	{
		pRadioPortrait->SetCheck(1);
		pRadioLandscape->SetCheck(0);
	}
	else
	{
		pRadioPortrait->SetCheck(0);
		pRadioLandscape->SetCheck(1);
	}

	// Init The cm / inch format flag
	m_bInchPaperFormat = IsInchPaperFormat();

	// Enable ToolTips
	m_pToolBar->EnableToolTips(TRUE);

	// Gesture Config
	typedef BOOL (WINAPI * FPSETGESTURECONFIG)(HWND hwnd, DWORD dwReserved, UINT cIDs, PGESTURECONFIG pGestureConfig, UINT cbSize);
	HINSTANCE h = ::LoadLibrary(_T("user32.dll"));
	if (h)
	{
		FPSETGESTURECONFIG fpSetGestureConfig = (FPSETGESTURECONFIG)::GetProcAddress(h, "SetGestureConfig");
		if (fpSetGestureConfig)
		{
			DWORD dwPanWant  = GC_PAN | GC_PAN_WITH_INERTIA | GC_PAN_WITH_SINGLE_FINGER_VERTICALLY | GC_PAN_WITH_SINGLE_FINGER_HORIZONTALLY;
			DWORD dwPanBlock = GC_PAN_WITH_GUTTER;
			GESTURECONFIG gc[] =	{{GID_ZOOM, GC_ZOOM, 0},				// Handled in OnGesture()
									{GID_ROTATE, 0, GC_ROTATE},				// Disabled
									{GID_PAN, dwPanWant , dwPanBlock},		// Handled in OnGesture()
									{GID_TWOFINGERTAP, 0, GC_TWOFINGERTAP},	// Disabled
									{GID_PRESSANDTAP, 0, GC_PRESSANDTAP}};	// Disabled
			fpSetGestureConfig(GetSafeHwnd(), 0, 5, gc, sizeof(GESTURECONFIG));
		}
		::FreeLibrary(h);
	}

	return 0;
}

CSize CPicturePrintPreviewView::CalcPageDisplaySize()
	// calculate the current page size
	// MFC used to set 'm_nSecondPageOffset' to start of second page
	// as we have multiple pages we use m_PageOffset which holds the
	// diferences across and down the pages
	// return size of current page less margins
{
	// just checking...
	ASSERT(m_Down >= 1);
	ASSERT(m_Across >= 1);
	ASSERT(m_nPages >= 1);

	CSize windowSize, scrollSize;
	GetTrueClientSize(windowSize, scrollSize);

	// Subtract out vertical scrollbar if zoomed out and page range is known
	// and there is more than one page.
	if (m_nZoomState == ZOOM_OUT && (m_pPreviewInfo->GetMaxPage() != 0xffff) &&
		(m_pPreviewInfo->GetMaxPage() - m_pPreviewInfo->GetMinPage() != 0))
		windowSize.cx -= scrollSize.cx;
	m_PageOffset.y = 0;
	if (m_Down > 1)
	{
		// we need to make room for more pages under the first
		windowSize.cy = (windowSize.cy - (PREVIEW_MARGIN * (m_Down - 1))) / m_Down;
		m_PageOffset.y = windowSize.cy + PREVIEW_MARGIN;
	}
	else
	{
		// its a single page down, it uses all the area previously calculated
	}

	if (m_Across <= 2)
		m_PageOffset.x = (windowSize.cx - PREVIEW_MARGIN) / 2;
	else
		m_PageOffset.x = (windowSize.cx - PREVIEW_MARGIN) / m_Across;

	// Make sure all pages across fit in the screen area
	windowSize.cx = (windowSize.cx - ((m_Across + 1) * PREVIEW_MARGIN)) / m_Across;
	//windowSize.cx = (m_nPages == 2) ? (windowSize.cx - 3*PREVIEW_MARGIN) / 2 :
	//								windowSize.cx - 2*PREVIEW_MARGIN;

	windowSize.cy -= 2*PREVIEW_MARGIN;
	return windowSize;
}

/*
void CPicturePrintPreviewView::OnPreviewPages()
{
	CPoint point;
	CRect rect;

	CWnd *pWnd = m_pToolBar->GetDlgItem(ID_PREVIEW_PAGES);
	if (pWnd != NULL)
	{
		// Place the menu just below the button
		pWnd->GetWindowRect(&rect);
		point = CPoint(rect.left, rect.bottom);
	}
	else
		::GetCursorPos(&point); // failed to get window, use the mouse position
	CMenu menu;
	CMenu* pSub;

	// Popup a menu to get the number of pages to display
	VERIFY(menu.LoadMenu(IDR_PREVIEW_PAGES));
	pSub = menu.GetSubMenu(0);

	int command = command;//pSub->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD, point.x, point.y, ::AfxGetMainFrame());

	switch (command)
	{
		case ID_PAGES_1PAGE :
			m_Across = 1;
			m_Down = 1;
			m_nPages = 1;
			break;
		case ID_PAGES_2PAGES :
			m_Across = 2;
			m_Down = 1;
			m_nPages = 2;
			break;
		case ID_PAGES_3PAGES :
			m_Across = 3;
			m_Down = 1;
			m_nPages = 3;
			break;
		case ID_PAGES_4PAGES :
			m_Across = 2;
			m_Down = 2;
			m_nPages = 4;
			break;
		case ID_PAGES_6PAGES :
			m_Across = 3;
			m_Down = 2;
			m_nPages = 6;
			break;
		case ID_PAGES_9PAGES :
			m_Across = 3;
			m_Down = 3;
			m_nPages = 9;
			break;
		default :
			return;
	}
		
	AfxGetApp()->m_nNumPreviewPages = m_nPages;
	m_nZoomOutPages = m_nPages;
	m_nMaxPages = m_nPages;
	if (m_nZoomState == ZOOM_OUT)
	{
		// make sure the current page does not cause blank pages to be shown
		if (m_nCurrentPage >= m_pPreviewInfo->GetMaxPage() - m_nPages)
			m_nCurrentPage = m_pPreviewInfo->GetMaxPage() - m_nPages + 1;
		if (m_nCurrentPage < 0)
			m_nCurrentPage = 0;
	}
	// Just do this to set the status correctly and invalidate
	SetCurrentPage(m_nCurrentPage, TRUE);
	SetupScrollbar();
}
*/

void CPicturePrintPreviewView::SetupScrollbar()
{
	// This procedure makes sure that the scroll bar does not allow us to scroll the window
	// such that we end up displaying blank pages
	// correctly range the scroll bars
	if (m_pPreviewInfo->GetMaxPage() < 0x8000 && m_pPreviewInfo->GetMaxPage() - m_pPreviewInfo->GetMinPage() <= 32767U)
	{
		SCROLLINFO info;
		info.fMask = SIF_PAGE|SIF_RANGE;
		info.nMin = m_pPreviewInfo->GetMinPage();
		info.nMax = m_pPreviewInfo->GetMaxPage() - (m_nPages - 1);
		info.nPage = 1;
		if (!SetScrollInfo(SB_VERT, &info, FALSE))
			SetScrollRange(SB_VERT, info.nMin, info.nMax, FALSE);
	}
	else
		ShowScrollBar(SB_VERT, FALSE);      // if no range specified, or too
}

void CPicturePrintPreviewView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (m_nZoomState != ZOOM_OUT)
	{
		CScrollView::OnVScroll(nSBCode, nPos, pScrollBar);
		return;
	}
	SetupScrollbar();
	switch (nSBCode)
	{
		case SB_BOTTOM:
			SetCurrentPage(m_pPreviewInfo->GetMaxPage(), TRUE);
			break;

		case SB_TOP:
			SetCurrentPage(m_pPreviewInfo->GetMinPage(), TRUE);
			break;

		case SB_PAGEDOWN:
			SetCurrentPage(m_nCurrentPage +
				(m_pPreviewInfo->GetMaxPage() - m_pPreviewInfo->GetMinPage() + 9) / 10, TRUE);
			break;

		case SB_PAGEUP:
			SetCurrentPage(m_nCurrentPage -
				(m_pPreviewInfo->GetMaxPage() - m_pPreviewInfo->GetMinPage() + 9) / 10, TRUE);
			break;

		case SB_LINEDOWN:
			if (m_nCurrentPage <= m_pPreviewInfo->GetMaxPage() - m_nPages)
				SetCurrentPage(m_nCurrentPage + 1, TRUE);
			break;

		case SB_LINEUP:
			if (m_nCurrentPage > 0)
				SetCurrentPage(m_nCurrentPage - 1, TRUE);
			break;

		case SB_THUMBPOSITION:
			SetCurrentPage(nPos, TRUE);
			break;
	}
}

void CPicturePrintPreviewView::OnSizeFit()
{
	CButton* pCheck = (CButton*)m_pToolBar->GetDlgItem(IDC_CHECK_SIZE_FIT);
	GetDocument()->m_bPrintSizeFit = pCheck->GetCheck();
	if (!GetDocument()->m_bPrintSizeFit)
	{
		// Set it to 100%
		GetDocument()->m_dPrintScale = 1.0;
		m_pScaleEdit->SetPrintScale(GetDocument()->m_dPrintScale);
	}
	Invalidate(FALSE);
}

void CPicturePrintPreviewView::FitSize()
{
	CButton* pCheck = (CButton*)m_pToolBar->GetDlgItem(IDC_CHECK_SIZE_FIT);
	pCheck->SetCheck(1);
	GetDocument()->m_bPrintSizeFit = TRUE;
}

void CPicturePrintPreviewView::OnPrintMargin()
{
	CButton* pCheck = (CButton*)m_pToolBar->GetDlgItem(IDC_CHECK_PRINTMARGIN);
	GetDocument()->m_bPrintMargin = pCheck->GetCheck();
	if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
	{
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("PrintMargin"),
										GetDocument()->m_bPrintMargin);
	}
	Invalidate(FALSE);
}

void CPicturePrintPreviewView::OnPrintSetup()
{
	CPrintDialog pd(TRUE);
	if (((CUImagerApp*)::AfxGetApp())->DoPrintDialog(&pd) == IDOK)
	{
		((CPictureView*)m_pPrintView)->OnEndPrintingFromPrintPreview(m_pPreviewDC, m_pPreviewInfo);

		CPrintInfo *pInfo = m_pPreviewInfo;
		m_pPreviewInfo = NULL;
		m_dcPrint.Detach();
		delete pInfo;
		delete m_pPreviewDC;
		m_pPreviewDC = NULL;
		FitSize();
		SetPrintView(m_pPrintView);

		// Update the Portrait Landscape Radio Buttons
		int orientation = GetPrintOrientation();
		CButton* pRadioPortrait = (CButton*)m_pToolBar->GetDlgItem(IDC_RADIO_ORIENTATION_PORTRAIT);
		CButton* pRadioLandscape = (CButton*)m_pToolBar->GetDlgItem(IDC_RADIO_ORIENTATION_LANDSCAPE);
		if (orientation == DMORIENT_PORTRAIT)
		{
			pRadioPortrait->SetCheck(1);
			pRadioLandscape->SetCheck(0);
		}
		else
		{
			pRadioPortrait->SetCheck(0);
			pRadioLandscape->SetCheck(1);
		}

		// Update the Printer Name, the Paper Size Name and the DPI
		DisplayPrinterPaperDPI();

		// Update The cm / inch format flag
		m_bInchPaperFormat = IsInchPaperFormat();
	}
}

void CPicturePrintPreviewView::DisplayPrinterPaperDPI()
{
	CEdit* pEdit = (CEdit*)m_pToolBar->GetDlgItem(IDC_EDIT_PRINTER_PAPERSIZE_DPI);
	if (pEdit)
	{
		int index = ((CUImagerApp*)::AfxGetApp())->GetCurrentPrinterIndex();
		CString sPrinterName = ((CUImagerApp*)::AfxGetApp())->m_PrinterControl.GetPrinterName(index);
		CString sPaperSizeDpi;
		sPaperSizeDpi.Format(_T("%s , %dx%ddpi"), GetPaperSizeName(), m_sizePrinterPPI.cx, m_sizePrinterPPI.cy);
		CDC* pDC = pEdit->GetDC();
		CFont* pOldFont = (CFont*)pDC->SelectObject(pEdit->GetFont());
		CSize szTextExtentPrinterName = pDC->GetTextExtent(sPrinterName);
		CSize szTextExtentPaperSizeDpi = pDC->GetTextExtent(sPaperSizeDpi);
		pDC->SelectObject(pOldFont);
		pEdit->ReleaseDC(pDC);
		int nMaxTextExtent = MAX(szTextExtentPrinterName.cx, szTextExtentPaperSizeDpi.cx);
		CRect rcWnd;
		pEdit->GetWindowRect(&rcWnd);
		CRect rcParent;
		pEdit->GetParent()->GetWindowRect(&rcParent);
		rcWnd.right = rcWnd.left + nMaxTextExtent + 8;
		rcWnd.OffsetRect(-rcParent.TopLeft());
		pEdit->MoveWindow(rcWnd);
		pEdit->SetWindowText(sPrinterName + _T("\r\n") + sPaperSizeDpi);
	}
}

void CPicturePrintPreviewView::OnPortrait()
{
	if (GetPrintOrientation() != DMORIENT_PORTRAIT)
	{
		((CPictureView*)m_pPrintView)->OnEndPrintingFromPrintPreview(m_pPreviewDC, m_pPreviewInfo);

		// Get the current CPrintInfo object
		CPrintInfo *pInfo = m_pPreviewInfo;
		m_pPreviewInfo = NULL;

		// Switch the print mode
		SetPrintOrientation(DMORIENT_PORTRAIT);
		m_dcPrint.Detach();
		delete pInfo;
		delete m_pPreviewDC;
		m_pPreviewDC = NULL;
		FitSize();
		SetPrintView(m_pPrintView);
	}
}

void CPicturePrintPreviewView::OnLandscape()
{
	if (GetPrintOrientation() != DMORIENT_LANDSCAPE)
	{
		((CPictureView*)m_pPrintView)->OnEndPrintingFromPrintPreview(m_pPreviewDC, m_pPreviewInfo);

		// Get the current CPrintInfo object
		CPrintInfo *pInfo = m_pPreviewInfo;
		m_pPreviewInfo = NULL;
		
		// Switch the print mode
		SetPrintOrientation(DMORIENT_LANDSCAPE);
		m_dcPrint.Detach();
		delete pInfo;
		delete m_pPreviewDC;
		m_pPreviewDC = NULL;
		FitSize();
		SetPrintView(m_pPrintView);
	}
}

BOOL CPicturePrintPreviewView::SetPrintOrientation(int orientation) const
{
	PRINTDLG pd;
	pd.lStructSize = (DWORD)sizeof(PRINTDLG);
	BOOL bRet = ::AfxGetApp()->GetPrinterDeviceDefaults(&pd);

	if (bRet)
	{
		switch (orientation)
		{
	        case DMORIENT_PORTRAIT:
			{
				// Portrait mode
				LPDEVMODE pDevMode = (LPDEVMODE)::GlobalLock(pd.hDevMode);

				// Set orientation to portrait
				if (pDevMode)
					pDevMode->dmOrientation = DMORIENT_PORTRAIT;
				::GlobalUnlock(pd.hDevMode);
				break;
			}
			       
			case DMORIENT_LANDSCAPE :
			{
				// Landscape mode
				LPDEVMODE pDevMode = (LPDEVMODE)::GlobalLock(pd.hDevMode);

				// Set orientation to landscape
				if (pDevMode)
					pDevMode->dmOrientation = DMORIENT_LANDSCAPE;
				::GlobalUnlock(pd.hDevMode);
				break;
			}
			        
			default :	
				ASSERT(FALSE);	// invalid parameter
				return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}

int CPicturePrintPreviewView::GetPrintOrientation() const
{
	PRINTDLG pd;
	pd.lStructSize = (DWORD)sizeof(PRINTDLG);
	BOOL bRet = ::AfxGetApp()->GetPrinterDeviceDefaults(&pd);
	int orientation = DMORIENT_PORTRAIT;
	if (bRet)
	{
		// Protect memory handle with ::GlobalLock and ::GlobalUnlock
		LPDEVMODE pDevMode = (LPDEVMODE)::GlobalLock(pd.hDevMode);
		
		// Get orientation
		if (pDevMode)
			orientation = pDevMode->dmOrientation;
		::GlobalUnlock(pd.hDevMode);
	}
	return orientation;
}

int CPicturePrintPreviewView::GetPaperSize() const
{
	PRINTDLG pd;
	pd.lStructSize = (DWORD)sizeof(PRINTDLG);
	BOOL bRet = ::AfxGetApp()->GetPrinterDeviceDefaults(&pd);
	int papersize = DMPAPER_LETTER;
	if (bRet)
	{
		// Protect memory handle with ::GlobalLock and ::GlobalUnlock
		LPDEVMODE pDevMode = (LPDEVMODE)::GlobalLock(pd.hDevMode);
		
		// Get paper size
		if (pDevMode)
			papersize = pDevMode->dmPaperSize;
		::GlobalUnlock(pd.hDevMode);
	}
	return papersize;
}

// From MFC's UINT PASCAL _AfxGetMouseScrollLines()
UINT CPicturePrintPreviewView::GetMouseScrollLines()
{
	// if we've already got it and we're not refreshing,
	// return what we've already got
	if (m_bGotScrollLines)
		return m_uCachedScrollLines;

	m_bGotScrollLines = TRUE;
	m_uCachedScrollLines = 3; // reasonable default
	::SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &m_uCachedScrollLines, 0);

	return m_uCachedScrollLines;
}

BOOL CPicturePrintPreviewView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	// Scroll Horizontal or Vertical?
	BOOL bScrollHoriz = (MK_SHIFT & nFlags) ? TRUE : FALSE;

	DWORD dwStyle = GetStyle();
	CScrollBar* pBar = GetScrollBarCtrl(SB_VERT);
	BOOL bHasVertBar = ((pBar != NULL) && pBar->IsWindowEnabled()) || (dwStyle & WS_VSCROLL);

	pBar = GetScrollBarCtrl(SB_HORZ);
	BOOL bHasHorzBar = ((pBar != NULL) && pBar->IsWindowEnabled()) || (dwStyle & WS_HSCROLL);

	if (!bHasVertBar && !bHasHorzBar)
		return FALSE;

	BOOL bResult = FALSE;
	UINT uWheelScrollLines = GetMouseScrollLines();
	int nToScroll;
	int nDisplacement;

	if (bHasVertBar && !bScrollHoriz)
	{
		nToScroll = ::MulDiv(-zDelta, uWheelScrollLines, WHEEL_DELTA);
		if (nToScroll == -1 || uWheelScrollLines == WHEEL_PAGESCROLL)
		{
			nDisplacement = m_pageDev.cy;
			if (zDelta > 0)
				nDisplacement = -nDisplacement;
		}
		else
		{
			nDisplacement = nToScroll * m_lineDev.cy;
			nDisplacement = min(nDisplacement, m_pageDev.cy);
		}
		bResult = OnScrollBy(CSize(0, nDisplacement), TRUE);
	}
	
	if (bHasHorzBar && bScrollHoriz)
	{
		nToScroll = ::MulDiv(-zDelta, uWheelScrollLines, WHEEL_DELTA);
		if (nToScroll == -1 || uWheelScrollLines == WHEEL_PAGESCROLL)
		{
			nDisplacement = m_pageDev.cx;
			if (zDelta > 0)
				nDisplacement = -nDisplacement;
		}
		else
		{
			nDisplacement = nToScroll * m_lineDev.cx;
			nDisplacement = min(nDisplacement, m_pageDev.cx);
		}
		bResult = OnScrollBy(CSize(nDisplacement, 0), TRUE);
	}

	if (bResult)
		UpdateWindow();

	return bResult;
}

void CPicturePrintPreviewView::OnMouseMove(UINT nFlags, CPoint point) 
{	
	// Print Offset Calculation
	if (m_bMouseCaptured)
	{
		CPoint ptOffset = point - m_ptStartClickPosition;
		int nXOffset = MulDiv(ptOffset.x, m_pPageInfo[0].sizeScaleRatio.cy, m_pPageInfo[0].sizeScaleRatio.cx);
		int nYOffset = MulDiv(ptOffset.y, m_pPageInfo[0].sizeScaleRatio.cy, m_pPageInfo[0].sizeScaleRatio.cx);

		// Convert screen coordinates to printer coordinates 
		nXOffset = MulDiv(nXOffset, m_sizePrinterPPI.cx, afxData.cxPixelsPerInch);
		nYOffset = MulDiv(nYOffset, m_sizePrinterPPI.cy, afxData.cyPixelsPerInch);

		// Set Offset
		GetDocument()->m_ptPrintOffset.x = GetDocument()->m_ptLastPrintOffset.x + nXOffset;
		GetDocument()->m_ptPrintOffset.y = GetDocument()->m_ptLastPrintOffset.y + nYOffset;

		// Clear Print Size Fit Check Box
		CButton* pCheck = (CButton*)m_pToolBar->GetDlgItem(IDC_CHECK_SIZE_FIT);
		pCheck->SetCheck(0);
		GetDocument()->m_bPrintSizeFit = FALSE;

		Invalidate(FALSE);
	}

	// Update Last Point
	m_ptMouseMoveLastPoint = point;

	// Update Pane Text
	UpdatePaneText();

	CPreviewView::OnMouseMove(nFlags, point);
}

void CPicturePrintPreviewView::UpdatePaneText()
{
	CUImagerDoc* pDoc = (CUImagerDoc*)GetDocument();
	ASSERT_VALID(pDoc);

	CString sXCoord, sYCoord;
	CPoint ptCoord = m_ptMouseMoveLastPoint;

	if (m_nZoomState == ZOOM_OUT)
		ptCoord -= m_pPageInfo[0].rectScreen.TopLeft();
	else
	{
		CPoint ptOffset = -GetDeviceScrollPosition();
		if (m_bCenter)
		{
			CRect rect;
			GetClientRect(&rect);
			// if client area is larger than total device size,
			// override scroll positions to place origin such that
			// output is centered in the window
			if (m_totalDev.cx < rect.Width())
				ptOffset.x = (rect.Width() - m_totalDev.cx) / 2;
			if (m_totalDev.cy < rect.Height())
				ptOffset.y = (rect.Height() - m_totalDev.cy) / 2;
		}
		ptCoord -= ptOffset;
		ptCoord -= CPoint(PREVIEW_MARGIN, PREVIEW_MARGIN);
	}
	
	double dX = MulDiv(ptCoord.x, m_pPageInfo[0].sizeScaleRatio.cy, m_pPageInfo[0].sizeScaleRatio.cx);
	double dY = MulDiv(ptCoord.y, m_pPageInfo[0].sizeScaleRatio.cy, m_pPageInfo[0].sizeScaleRatio.cx);
	if (((CUImagerApp*)::AfxGetApp())->m_nCoordinateUnit == COORDINATES_PIX)
	{
		CalcPhysPix(dX, dY);
		sXCoord.Format(_T("X: %d px"), Round(dX));
		sYCoord.Format(_T("Y: %d px"), Round(dY));
	}
	else if (((CUImagerApp*)::AfxGetApp())->m_nCoordinateUnit == COORDINATES_INCH)
	{
		CalcPhysInch(dX, dY);
		sXCoord.Format(_T("X: %0.2f inch"), dX);
		sYCoord.Format(_T("Y: %0.2f inch"), dY);
	}
	else
	{
		CalcPhysCm(dX, dY);
		sXCoord.Format(_T("X: %0.2f cm"), dX);
		sYCoord.Format(_T("Y: %0.2f cm"), dY);
	}
	::AfxGetMainFrame()->GetStatusBar()->SetPaneText(2, sXCoord);
	::AfxGetMainFrame()->GetStatusBar()->SetPaneText(3, sYCoord);
}

void CPicturePrintPreviewView::OnDraw(CDC* pDC) 
{
	ASSERT_VALID(pDC);

	// Set the number of displayed pages to avoid MFC to display a blank page at the end...
	int nOldPages = m_nPages;
	m_nPages = min(m_nPages, max(1, m_pPreviewInfo->GetMaxPage()-m_nCurrentPage + 1));

	// The Modified CPreviewView::OnDraw() Function:

	// don't do anything if not fully initialized
	if (m_pPrintView == NULL || m_dcPrint.m_hDC == NULL)
		return;

	CPoint ViewportOrg = pDC->GetViewportOrg();
	CPen rectPen;
	rectPen.CreatePen(PS_SOLID, 2, GetSysColor(COLOR_WINDOWFRAME));
	CPen shadowPen;
	shadowPen.CreatePen(PS_SOLID, 3, GetSysColor(COLOR_BTNSHADOW));

	m_pPreviewInfo->m_bContinuePrinting = TRUE;     // do this once each paint

	for (UINT nPage = 0; nPage < m_nPages; nPage++)
	{
		int nSavedState = m_dcPrint.SaveDC();       // Save pristine state of DC

		CSize* pRatio = &m_pPageInfo[nPage].sizeScaleRatio;
		CRect* pRect = &m_pPageInfo[nPage].rectScreen;

		// If page position has not been determined
		if (pRatio->cx == 0)
		{
			PositionPage(nPage);    // compute page position
			if (m_nZoomState != ZOOM_OUT)
			{
				ViewportOrg = -GetDeviceScrollPosition();
				if (m_bCenter)
				{
					CRect rect;
					GetClientRect(&rect);
					// if client area is larger than total device size,
					// override scroll positions to place origin such that
					// output is centered in the window
					if (m_totalDev.cx < rect.Width())
						ViewportOrg.x = (rect.Width() - m_totalDev.cx) / 2;
					if (m_totalDev.cy < rect.Height())
						ViewportOrg.y = (rect.Height() - m_totalDev.cy) / 2;
				}
			}
		}

		// Create Mem DC And Erase Background
		CRect rcClient, rcMemDC, rcClip;
		GetClientRect(&rcClient);
		pDC->GetClipBox(&rcClip);
		if (m_nZoomState == ZOOM_OUT)
			rcMemDC = rcClient;
		else
		{
			CSize size = GetTotalSize();
			rcMemDC.left = 0;
			rcMemDC.top = 0;
			rcMemDC.right = MAX(size.cx, rcClient.Width());
			rcMemDC.bottom = MAX(size.cy, rcClient.Height());
		}
		CDC MemDC;
        MemDC.CreateCompatibleDC(pDC);
		if (m_rcPrevMemDC != rcMemDC)
		{
			m_MemDCBitmap.DeleteObject();
			m_MemDCBitmap.CreateCompatibleBitmap(pDC, rcMemDC.Width(), rcMemDC.Height());
		}
        CBitmap* pOldBitmap = MemDC.SelectObject(&m_MemDCBitmap);
		CBrush backBrush(GetSysColor(COLOR_APPWORKSPACE));
		CBrush* pOldBrush = MemDC.SelectObject(&backBrush);
		MemDC.PatBlt(rcMemDC.left, rcMemDC.top, rcMemDC.Width(), rcMemDC.Height(), PATCOPY);
		MemDC.SelectObject(pOldBrush);

		// Use paint DC for print preview output
		m_pPreviewDC->SetOutputDC(MemDC.GetSafeHdc());

		m_pPreviewInfo->m_nCurPage = m_nCurrentPage + nPage;

		// Only call PrepareDC if within page range, otherwise use default
		// rect to draw page rectangle
		if (m_nCurrentPage + nPage >= m_nCurrentPage && 
		    m_nCurrentPage + nPage >= nPage && 
		    m_nCurrentPage + nPage <= m_pPreviewInfo->GetMaxPage())
		{
			m_pPrintView->OnPrepareDC(m_pPreviewDC, m_pPreviewInfo);
		}

		// Set up drawing rect to entire page (in logical coordinates)
		m_pPreviewInfo->m_rectDraw.SetRect(0, 0,
			m_pPreviewDC->GetDeviceCaps(HORZRES),
			m_pPreviewDC->GetDeviceCaps(VERTRES));
		m_pPreviewDC->DPtoLP(&m_pPreviewInfo->m_rectDraw);

		// Draw empty page on screen

		MemDC.SaveDC();          // save the output dc state

		MemDC.SetMapMode(MM_TEXT);   // Page Rectangle is in screen device coords
		MemDC.SetViewportOrg(ViewportOrg);
		MemDC.SetWindowOrg(0, 0);

		MemDC.SelectStockObject(HOLLOW_BRUSH);
		MemDC.SelectObject(&rectPen);
		MemDC.Rectangle(pRect);

		MemDC.SelectObject(&shadowPen);

		MemDC.MoveTo(pRect->right + 1, pRect->top + 3);
		MemDC.LineTo(pRect->right + 1, pRect->bottom + 1);
		MemDC.MoveTo(pRect->left + 3, pRect->bottom + 1);
		MemDC.LineTo(pRect->right + 1, pRect->bottom + 1);

		// Erase background to white (most paper is white)
		CRect rectFill = *pRect;
		rectFill.left += 1;
		rectFill.top += 1;
		rectFill.right -= 2;
		rectFill.bottom -= 2;
		::FillRect(MemDC.m_hDC, rectFill, (HBRUSH)GetStockObject(WHITE_BRUSH));
		
		MemDC.RestoreDC(-1);     // restore to synchronized state


		if (!m_pPreviewInfo->m_bContinuePrinting ||
				m_nCurrentPage + nPage > m_pPreviewInfo->GetMaxPage())
		{
			m_pPreviewDC->ReleaseOutputDC();
			m_dcPrint.RestoreDC(nSavedState);   // restore to untouched state

			// if the first page is not displayable, back up one page
			// but never go below 1
			if (nPage == 0 && m_nCurrentPage > 1)
				SetCurrentPage(m_nCurrentPage - 1, TRUE);
			break;
		}

		// Set scale ratio for this page
		m_pPreviewDC->SetScaleRatio(pRatio->cx, pRatio->cy);

		// Print Offset
		CSize PrintOffset;
		VERIFY(m_pPreviewDC->Escape(GETPRINTINGOFFSET, 0, NULL, (LPVOID)&PrintOffset));
		m_pPreviewDC->PrinterDPtoScreenDP((LPPOINT)&PrintOffset);
		PrintOffset += (CSize)pRect->TopLeft();
		PrintOffset += CSize(1, 1);
		PrintOffset += (CSize)ViewportOrg;  // For Scrolling

		// Set Offset, Clip And Finally Print
		m_pPreviewDC->SetTopLeftOffset(PrintOffset);
		m_pPreviewDC->ClipToPage();
		// Call the Protected OnPrint() function:
		((CPictureView*)m_pPrintView)->OnPrintCall(m_pPreviewDC, m_pPreviewInfo);

		// Display page number (after OnPrintCall because m_rcDibPrint is neeeded!)
		OnDisplayPageNumber(m_nCurrentPage, nPage + 1);

		// Release DCs
		m_pPreviewDC->ReleaseOutputDC();
		m_dcPrint.RestoreDC(nSavedState);   // restore to untouched state

		// Copy Bits
		MemDC.SetWindowOrg(0, 0);
		MemDC.SetViewportOrg(0, 0);
		CPoint ptOldWindowOrg = pDC->GetWindowOrg();
		CPoint ptOldViewportOrg = pDC->GetViewportOrg();
		pDC->SetWindowOrg(0, 0);
		pDC->SetViewportOrg(0, 0);
		pDC->BitBlt(0, 0, rcMemDC.Width(), rcMemDC.Height(),
					&MemDC, 0, 0, SRCCOPY);      
		MemDC.SelectObject(pOldBitmap);
		m_rcPrevMemDC = rcMemDC;
		pDC->SetWindowOrg(ptOldWindowOrg);
		pDC->SetViewportOrg(ptOldViewportOrg);
	}

	rectPen.DeleteObject();
	shadowPen.DeleteObject();

	m_nPages = nOldPages;
}

void CPicturePrintPreviewView::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
	// To Remove Focus From CDialogBar's Edit Box in Print Preview Mode
	SetFocus();
	CPreviewView::OnNcLButtonDown(nHitTest, point);
}

void CPicturePrintPreviewView::OnNcLButtonDblClk(UINT nHitTest, CPoint point)
{
	// To Remove Focus From CDialogBar's Edit Box in Print Preview Mode
	SetFocus();
	CPreviewView::OnNcLButtonDblClk(nHitTest, point);
}

void CPicturePrintPreviewView::CalcPhysPix(double& dX, double& dY)
{
	dX = dX / afxData.cxPixelsPerInch * m_sizePrinterPPI.cx;
	dY = dY / afxData.cyPixelsPerInch * m_sizePrinterPPI.cy;
}

void CPicturePrintPreviewView::CalcPhysInch(double& dX, double& dY)
{
	dX /= afxData.cxPixelsPerInch;
	dY /= afxData.cyPixelsPerInch;
}

void CPicturePrintPreviewView::CalcPhysCm(double& dX, double& dY)
{
	dX = dX * 2.54 / afxData.cxPixelsPerInch;
	dY = dY * 2.54 / afxData.cyPixelsPerInch;
}

BOOL CPicturePrintPreviewView::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE;
}

void CPicturePrintPreviewView::UpdateStatusText()
{
	// Note:
	// m_sizePrinterPPI.cx is set to m_dcPrint.GetDeviceCaps(LOGPIXELSX);
	// m_sizePrinterPPI.cy is set to m_dcPrint.GetDeviceCaps(LOGPIXELSY);

	// Printer Page Size
	CSize PageSize;
	VERIFY(m_dcPrint.Escape(GETPHYSPAGESIZE, 0, NULL, (LPVOID)&PageSize));

	// Printer Offset
	CSize PrintOffset;
	VERIFY(m_dcPrint.Escape(GETPRINTINGOFFSET, 0, NULL, (LPVOID)&PrintOffset));
	
	// Printable Rect
	CRect rcPrintable = CRect(	0,
								0,
								m_dcPrint.GetDeviceCaps(HORZRES),
								m_dcPrint.GetDeviceCaps(VERTRES)); //=m_pPreviewInfo->m_rectDraw
	rcPrintable.OffsetRect(PrintOffset);

	// Format String
	CString s;
	CRect DibPrint = ((CPictureView*)m_pPrintView)->m_rcDibPrint;
	DibPrint.OffsetRect(PrintOffset);
	double dLeft = (double)DibPrint.left;
	double dTop = (double)DibPrint.top;
	double dRight = (double)DibPrint.right;
	double dBottom = (double)DibPrint.bottom;
	double dRightMargin = (double)(PageSize.cx - DibPrint.right);
	double dBottomMargin = (double)(PageSize.cy - DibPrint.bottom);
	if (((CUImagerApp*)::AfxGetApp())->m_nCoordinateUnit == COORDINATES_PIX)
	{
		s.Format(	_T("W=%dpx , H=%dpx (l=%d,t=%d,r=%d,b=%d)"),
					DibPrint.right - DibPrint.left,
					DibPrint.bottom - DibPrint.top,
					DibPrint.left,
					DibPrint.top,
					PageSize.cx - DibPrint.right,
					PageSize.cy - DibPrint.bottom);
	}
	else if (((CUImagerApp*)::AfxGetApp())->m_nCoordinateUnit == COORDINATES_INCH)
	{
		dLeft /= m_sizePrinterPPI.cx;
		dTop /= m_sizePrinterPPI.cy;
		dRight /= m_sizePrinterPPI.cx;
		dBottom /= m_sizePrinterPPI.cy;
		dRightMargin /= m_sizePrinterPPI.cx;
		dBottomMargin /= m_sizePrinterPPI.cy;
		s.Format(	_T("W=%0.2fin , H=%0.2fin (l=%0.2f,t=%0.2f,r=%0.2f,b=%0.2f)"),
					dRight - dLeft,
					dBottom - dTop,
					dLeft,
					dTop,
					dRightMargin,
					dBottomMargin);
	}
	else
	{
		dLeft = dLeft * 2.54 / m_sizePrinterPPI.cx;
		dTop = dTop * 2.54 / m_sizePrinterPPI.cy;
		dRight = dRight * 2.54 / m_sizePrinterPPI.cx;
		dBottom = dBottom * 2.54 / m_sizePrinterPPI.cy;
		dRightMargin = dRightMargin * 2.54 / m_sizePrinterPPI.cx;
		dBottomMargin = dBottomMargin * 2.54 / m_sizePrinterPPI.cy;
		s.Format(	_T("W=%0.2fcm , H=%0.2fcm (l=%0.2f,t=%0.2f,r=%0.2f,b=%0.2f)"),
					dRight - dLeft,
					dBottom - dTop,
					dLeft,
					dTop,
					dRightMargin,
					dBottomMargin);
	}
	
	// Show Status Text
	::AfxGetMainFrame()->StatusText(s);
}

void CPicturePrintPreviewView::OnDisplayPageNumber(UINT nPage, UINT nPagesDisplayed)
{
	UpdateStatusText();
}

BOOL CPicturePrintPreviewView::IsInchPaperFormat()
{
	switch (GetPaperSize())
	{
		case DMPAPER_LETTER :					/* Letter 8 1/2 x 11 in               */
			return TRUE;

		case DMPAPER_LETTERSMALL :				/* Letter Small 8 1/2 x 11 in         */
			return TRUE;

		case DMPAPER_TABLOID :					/* Tabloid 11 x 17 in                 */
			return TRUE;

		case DMPAPER_LEDGER :					/* Ledger 17 x 11 in                  */
			return TRUE;

		case DMPAPER_LEGAL :					/* Legal 8 1/2 x 14 in                */
			return TRUE;

		case DMPAPER_STATEMENT :				/* Statement 5 1/2 x 8 1/2 in         */
			return TRUE;

		case DMPAPER_EXECUTIVE :				/* Executive 7 1/4 x 10 1/2 in        */
			return TRUE;

		case DMPAPER_A3 :						/* A3 297 x 420 mm                    */
			return FALSE;

		case DMPAPER_A4 :						/* A4 210 x 297 mm                    */
			return FALSE;

		case DMPAPER_A4SMALL :					/* A4 Small 210 x 297 mm              */
			return FALSE;

		case DMPAPER_A5 :						/* A5 148 x 210 mm                    */
			return FALSE;

		case DMPAPER_B4 :						/* B4 (JIS) 250 x 354 mm              */
			return FALSE;

		case DMPAPER_B5 :						/* B5 (JIS) 182 x 257 mm              */
			return FALSE;

		case DMPAPER_FOLIO :					/* Folio 8 1/2 x 13 in                */
			return TRUE;

		case DMPAPER_QUARTO :					/* Quarto 215 x 275 mm                */
			return FALSE;

		case DMPAPER_10X14 :					/* 10x14 in                           */
			return TRUE;

		case DMPAPER_11X17 :					/* 11x17 in                           */
			return TRUE;

		case DMPAPER_NOTE :						/* Note 8 1/2 x 11 in                 */
			return TRUE;

		case DMPAPER_ENV_9 :					/* Envelope #9 3 7/8 x 8 7/8          */
			return TRUE;

		case DMPAPER_ENV_10 :					/* Envelope #10 4 1/8 x 9 1/2         */
			return TRUE;

		case DMPAPER_ENV_11 :					/* Envelope #11 4 1/2 x 10 3/8        */
			return TRUE;

		case DMPAPER_ENV_12 :					/* Envelope #12 4 \276 x 11           */
			return TRUE;

		case DMPAPER_ENV_14 :					/* Envelope #14 5 x 11 1/2            */
			return TRUE;

		case DMPAPER_CSHEET :					/* C size sheet                       */
			return TRUE;

		case DMPAPER_DSHEET :					/* D size sheet                       */
			return TRUE;

		case DMPAPER_ESHEET :					/* E size sheet                       */
			return TRUE;

		case DMPAPER_ENV_DL :					/* Envelope DL 110 x 220mm            */
			return FALSE;

		case DMPAPER_ENV_C5 :					/* Envelope C5 162 x 229 mm           */
			return FALSE;

		case DMPAPER_ENV_C3 :					/* Envelope C3  324 x 458 mm          */
			return FALSE;

		case DMPAPER_ENV_C4 :					/* Envelope C4  229 x 324 mm          */
			return FALSE;

		case DMPAPER_ENV_C6 :					/* Envelope C6  114 x 162 mm          */
			return FALSE;

		case DMPAPER_ENV_C65 :					/* Envelope C65 114 x 229 mm          */
			return FALSE;

		case DMPAPER_ENV_B4 :					/* Envelope B4  250 x 353 mm          */
			return FALSE;

		case DMPAPER_ENV_B5 :					/* Envelope B5  176 x 250 mm          */
			return FALSE;

		case DMPAPER_ENV_B6 :					/* Envelope B6  176 x 125 mm          */
			return FALSE;

		case DMPAPER_ENV_ITALY :				/* Envelope 110 x 230 mm              */
			return FALSE;

		case DMPAPER_ENV_MONARCH :				/* Envelope Monarch 3.875 x 7.5 in    */
			return TRUE;

		case DMPAPER_ENV_PERSONAL :				/* 6 3/4 Envelope 3 5/8 x 6 1/2 in    */
			return TRUE;

		case DMPAPER_FANFOLD_US :				/* US Std Fanfold 14 7/8 x 11 in      */
			return TRUE;

		case DMPAPER_FANFOLD_STD_GERMAN :		/* German Std Fanfold 8 1/2 x 12 in   */
			return TRUE;

		case DMPAPER_FANFOLD_LGL_GERMAN :		/* German Legal Fanfold 8 1/2 x 13 in */
			return TRUE;

		case DMPAPER_ISO_B4 :					/* B4 (ISO) 250 x 353 mm              */
			return FALSE;

		case DMPAPER_JAPANESE_POSTCARD :		/* Japanese Postcard 100 x 148 mm     */
			return FALSE;

		case DMPAPER_9X11 :						/* 9 x 11 in                          */
			return TRUE;

		case DMPAPER_10X11 :					/* 10 x 11 in                         */
			return TRUE;

		case DMPAPER_15X11 :					/* 15 x 11 in                         */
			return TRUE;

		case DMPAPER_ENV_INVITE :				/* Envelope Invite 220 x 220 mm       */
			return FALSE;

		case DMPAPER_RESERVED_48 :				/* RESERVED--DO NOT USE               */
			return FALSE;

		case DMPAPER_RESERVED_49 :				/* RESERVED--DO NOT USE               */
			return FALSE;

		case DMPAPER_LETTER_EXTRA :				/* Letter Extra 9 \275 x 12 in        */
			return TRUE;

		case DMPAPER_LEGAL_EXTRA :				/* Legal Extra 9 \275 x 15 in         */
			return TRUE;

		case DMPAPER_TABLOID_EXTRA :			/* Tabloid Extra 11.69 x 18 in        */
			return TRUE;

		case DMPAPER_A4_EXTRA :					/* A4 Extra 236 x 322 mm           */
			return FALSE;

		case DMPAPER_LETTER_TRANSVERSE :		/* Letter Transverse 8 \275 x 11 in   */
			return TRUE;

		case DMPAPER_A4_TRANSVERSE :			/* A4 Transverse 210 x 297 mm         */
			return FALSE;

		case DMPAPER_LETTER_EXTRA_TRANSVERSE :	/* Letter Extra Transverse 9\275 x 12 in */
			return TRUE;

		case DMPAPER_A_PLUS :					/* SuperA/SuperA/A4 227 x 356 mm      */
			return FALSE;

		case DMPAPER_B_PLUS :					/* SuperB/SuperB/A3 305 x 487 mm      */
			return FALSE;

		case DMPAPER_LETTER_PLUS :				/* Letter Plus 8.5 x 12.69 in         */
			return TRUE;

		case DMPAPER_A4_PLUS :					/* A4 Plus 210 x 330 mm               */
			return FALSE;

		case DMPAPER_A5_TRANSVERSE :			/* A5 Transverse 148 x 210 mm         */
			return FALSE;

		case DMPAPER_B5_TRANSVERSE :			/* B5 (JIS) Transverse 182 x 257 mm   */
			return FALSE;

		case DMPAPER_A3_EXTRA :					/* A3 Extra 322 x 445 mm              */
			return FALSE;

		case DMPAPER_A5_EXTRA :					/* A5 Extra 174 x 235 mm              */
			return FALSE;

		case DMPAPER_B5_EXTRA :					/* B5 (ISO) Extra 201 x 276 mm        */
			return FALSE;

		case DMPAPER_A2 :						/* A2 420 x 594 mm                    */
			return FALSE;

		case DMPAPER_A3_TRANSVERSE :			/* A3 Transverse 297 x 420 mm         */
			return FALSE;

		case DMPAPER_A3_EXTRA_TRANSVERSE :		/* A3 Extra Transverse 322 x 445 mm   */
			return FALSE;

		case DMPAPER_DBL_JAPANESE_POSTCARD :	/* Japanese Double Postcard 200 x 148 mm */
			return FALSE;

		case DMPAPER_A6 :						/* A6 105 x 148 mm                 */
			return FALSE;

		case DMPAPER_JENV_KAKU2 :				/* Japanese Envelope Kaku #2       */
			return FALSE;

		case DMPAPER_JENV_KAKU3 :				/* Japanese Envelope Kaku #3       */
			return FALSE;

		case DMPAPER_JENV_CHOU3 :				/* Japanese Envelope Chou #3       */
			return FALSE;

		case DMPAPER_JENV_CHOU4 :				/* Japanese Envelope Chou #4       */
			return FALSE;

		case DMPAPER_LETTER_ROTATED :			/* Letter Rotated 11 x 8 1/2 11 in */
			return TRUE;

		case DMPAPER_A3_ROTATED :				/* A3 Rotated 420 x 297 mm         */
			return FALSE;

		case DMPAPER_A4_ROTATED :				/* A4 Rotated 297 x 210 mm         */
			return FALSE;

		case DMPAPER_A5_ROTATED :				/* A5 Rotated 210 x 148 mm         */
			return FALSE;

		case DMPAPER_B4_JIS_ROTATED :			/* B4 (JIS) Rotated 364 x 257 mm   */
			return FALSE;

		case DMPAPER_B5_JIS_ROTATED :			/* B5 (JIS) Rotated 257 x 182 mm   */
			return FALSE;

		case DMPAPER_JAPANESE_POSTCARD_ROTATED :/* Japanese Postcard Rotated 148 x 100 mm */
			return FALSE;

		case DMPAPER_DBL_JAPANESE_POSTCARD_ROTATED :/* Double Japanese Postcard Rotated 148 x 200 mm */
			return FALSE;

		case DMPAPER_A6_ROTATED :				/* A6 Rotated 148 x 105 mm         */
			return FALSE;

		case DMPAPER_JENV_KAKU2_ROTATED :		/* Japanese Envelope Kaku #2 Rotated */
			return FALSE;

		case DMPAPER_JENV_KAKU3_ROTATED :		/* Japanese Envelope Kaku #3 Rotated */
			return FALSE;

		case DMPAPER_JENV_CHOU3_ROTATED :		/* Japanese Envelope Chou #3 Rotated */
			return FALSE;

		case DMPAPER_JENV_CHOU4_ROTATED :		/* Japanese Envelope Chou #4 Rotated */
			return FALSE;

		case DMPAPER_B6_JIS :					/* B6 (JIS) 128 x 182 mm           */
			return FALSE;

		case DMPAPER_B6_JIS_ROTATED :			/* B6 (JIS) Rotated 182 x 128 mm   */
			return FALSE;

		case DMPAPER_12X11 :					/* 12 x 11 in                      */
			return TRUE;

		case DMPAPER_JENV_YOU4 :				/* Japanese Envelope You #4        */
			return FALSE;

		case DMPAPER_JENV_YOU4_ROTATED :		/* Japanese Envelope You #4 Rotated*/
			return FALSE;

		case DMPAPER_P16K :						/* PRC 16K 146 x 215 mm            */
			return FALSE;

		case DMPAPER_P32K :						/* PRC 32K 97 x 151 mm             */
			return FALSE;

		case DMPAPER_P32KBIG :					/* PRC 32K(Big) 97 x 151 mm        */
			return FALSE;

		case DMPAPER_PENV_1 :					/* PRC Envelope #1 102 x 165 mm    */
			return FALSE;

		case DMPAPER_PENV_2 :					/* PRC Envelope #2 102 x 176 mm    */
			return FALSE;

		case DMPAPER_PENV_3 :					/* PRC Envelope #3 125 x 176 mm    */
			return FALSE;

		case DMPAPER_PENV_4 :					/* PRC Envelope #4 110 x 208 mm    */
			return FALSE;

		case DMPAPER_PENV_5 :					/* PRC Envelope #5 110 x 220 mm    */
			return FALSE;

		case DMPAPER_PENV_6 :					/* PRC Envelope #6 120 x 230 mm    */
			return FALSE;

		case DMPAPER_PENV_7 :					/* PRC Envelope #7 160 x 230 mm    */
			return FALSE;

		case DMPAPER_PENV_8 :					/* PRC Envelope #8 120 x 309 mm    */
			return FALSE;

		case DMPAPER_PENV_9 :					/* PRC Envelope #9 229 x 324 mm    */
			return FALSE;

		case DMPAPER_PENV_10 :					/* PRC Envelope #10 324 x 458 mm   */
			return FALSE;

		case DMPAPER_P16K_ROTATED :				/* PRC 16K Rotated                 */
			return FALSE;

		case DMPAPER_P32K_ROTATED :				/* PRC 32K Rotated                 */
			return FALSE;

		case DMPAPER_P32KBIG_ROTATED :			/* PRC 32K(Big) Rotated            */
			return FALSE;

		case DMPAPER_PENV_1_ROTATED :			/* PRC Envelope #1 Rotated 165 x 102 mm */
			return FALSE;

		case DMPAPER_PENV_2_ROTATED :			/* PRC Envelope #2 Rotated 176 x 102 mm */
			return FALSE;

		case DMPAPER_PENV_3_ROTATED :			/* PRC Envelope #3 Rotated 176 x 125 mm */
			return FALSE;

		case DMPAPER_PENV_4_ROTATED :			/* PRC Envelope #4 Rotated 208 x 110 mm */
			return FALSE;

		case DMPAPER_PENV_5_ROTATED :			/* PRC Envelope #5 Rotated 220 x 110 mm */
			return FALSE;

		case DMPAPER_PENV_6_ROTATED :			/* PRC Envelope #6 Rotated 230 x 120 mm */
			return FALSE;

		case DMPAPER_PENV_7_ROTATED :			/* PRC Envelope #7 Rotated 230 x 160 mm */
			return FALSE;

		case DMPAPER_PENV_8_ROTATED :			/* PRC Envelope #8 Rotated 309 x 120 mm */
			return FALSE;

		case DMPAPER_PENV_9_ROTATED :			/* PRC Envelope #9 Rotated 324 x 229 mm */
			return FALSE;

		case DMPAPER_PENV_10_ROTATED :			/* PRC Envelope #10 Rotated 458 x 324 mm */
			return FALSE;

		default :
			return FALSE;
	}
}

CString CPicturePrintPreviewView::PaperSizeToFormName(int nPaperSize) const
{
	// Get current printer name
	int index = ((CUImagerApp*)::AfxGetApp())->GetCurrentPrinterIndex();
	CString sPrinterName = ((CUImagerApp*)::AfxGetApp())->m_PrinterControl.GetPrinterName(index);

	/* DC_PAPERNAMES : retrieves a list of supported paper names (for example, Letter or Legal).
	The pOutput buffer receives an array of string buffers. Each string buffer is 64 characters
	long and contains the name of a paper form. The return value indicates the number of entries
	in the array. The name strings are null-terminated unless the name is 64 characters long.
	If pOutput is NULL, the return value is the number of paper forms. */
	int i;
	CStringArray PaperNames;
	TCHAR FormName[65];
	int nFormNameCount = ::DeviceCapabilities(sPrinterName, NULL, DC_PAPERNAMES, NULL, NULL);
	if (nFormNameCount <= 0)
		return _T("");
	TCHAR* pPaperNames = new TCHAR[64 * nFormNameCount];
	if (!pPaperNames)
		return _T("");
	memset(pPaperNames, 0, sizeof(TCHAR) * 64 * nFormNameCount);
	::DeviceCapabilities(sPrinterName, NULL, DC_PAPERNAMES, pPaperNames, NULL);
	for (i = 0 ; i < nFormNameCount ; i++)
	{
		_tcsncpy(FormName, (LPCTSTR)(pPaperNames + 64 * i), 64);
		FormName[64] = _T('\0');
		PaperNames.Add(FormName);
	}
	delete [] pPaperNames;

	/* DC_PAPERS : retrieves a list of supported paper sizes. The pOutput buffer receives an array
	of WORD values that indicate the available paper sizes for the printer. The return value
	indicates the number of entries in the array. For a list of the possible array values, see the
	description of the dmPaperSize member of the DEVMODE structure. If pOutput is NULL, the return
	value indicates the required number of entries in the array. */
	int nPaperSizeCount = ::DeviceCapabilities(sPrinterName, NULL, DC_PAPERS, NULL, NULL);
	if (nPaperSizeCount != nFormNameCount)
		return _T("");
	WORD* pPapers = new WORD[nPaperSizeCount];
	if (!pPapers)
		return _T("");
	::DeviceCapabilities(sPrinterName, NULL, DC_PAPERS, (LPTSTR)pPapers, NULL);
	for (i = 0 ; i < nPaperSizeCount ; i++)
	{
		if (pPapers[i] == nPaperSize)
			break;
	}
	delete [] pPapers;
	if (i < nPaperSizeCount)
		return PaperNames[i];
	else
		return _T("");
}

CString CPicturePrintPreviewView::GetPaperSizeName() const
{
	int papersize = GetPaperSize();
	switch (papersize)
	{
		case DMPAPER_LETTER :
			return CString(_T("Letter 8 1/2 x 11 in"));

		case DMPAPER_LETTERSMALL :
			return CString(_T("Letter Small 8 1/2 x 11 in"));

		case DMPAPER_TABLOID :
			return CString(_T("Tabloid 11 x 17 in"));

		case DMPAPER_LEDGER :
			return CString(_T("Ledger 17 x 11 in"));

		case DMPAPER_LEGAL :
			return CString(_T("Legal 8 1/2 x 14 in"));

		case DMPAPER_STATEMENT :
			return CString(_T("Statement 5 1/2 x 8 1/2 in"));

		case DMPAPER_EXECUTIVE :
			return CString(_T("Executive 7 1/4 x 10 1/2 in"));

		case DMPAPER_A3 :
			return CString(_T("A3 297 x 420 mm"));

		case DMPAPER_A4 :
			return CString(_T("A4 210 x 297 mm"));

		case DMPAPER_A4SMALL :
			return CString(_T("A4 Small 210 x 297 mm"));

		case DMPAPER_A5 :
			return CString(_T("A5 148 x 210 mm"));

		case DMPAPER_B4 :
			return CString(_T("B4 (JIS) 250 x 354 mm")); 

		case DMPAPER_B5 :
			return CString(_T("B5 (JIS) 182 x 257 mm"));

		case DMPAPER_FOLIO :
			return CString(_T("Folio 8 1/2 x 13 in"));

		case DMPAPER_QUARTO :
			return CString(_T("Quarto 215 x 275 mm"));

		case DMPAPER_10X14 :
			return CString(_T("10x14 in"));

		case DMPAPER_11X17 :
			return CString(_T("11x17 in"));

		case DMPAPER_NOTE :
			return CString(_T("Note 8 1/2 x 11 in"));  

		case DMPAPER_ENV_9 :
			return CString(_T("Envelope #9 3 7/8 x 8 7/8")); 

		case DMPAPER_ENV_10 :
			return CString(_T("Envelope #10 4 1/8 x 9 1/2"));

		case DMPAPER_ENV_11 :
			return CString(_T("Envelope #11 4 1/2 x 10 3/8"));

		case DMPAPER_ENV_12 :
			return CString(_T("Envelope #12 4 \276 x 11"));

		case DMPAPER_ENV_14 :
			return CString(_T("Envelope #14 5 x 11 1/2"));

		case DMPAPER_CSHEET :
			return CString(_T("C size sheet"));

		case DMPAPER_DSHEET :
			return CString(_T("D size sheet"));

		case DMPAPER_ESHEET :
			return CString(_T("E size sheet"));

		case DMPAPER_ENV_DL :
			return CString(_T("Envelope DL 110 x 220mm"));

		case DMPAPER_ENV_C5 :
			return CString(_T("Envelope C5 162 x 229 mm"));

		case DMPAPER_ENV_C3 :
			return CString(_T("Envelope C3  324 x 458 mm"));

		case DMPAPER_ENV_C4 :
			return CString(_T("Envelope C4  229 x 324 mm"));

		case DMPAPER_ENV_C6 :
			return CString(_T("Envelope C6  114 x 162 mm"));

		case DMPAPER_ENV_C65 :
			return CString(_T("Envelope C65 114 x 229 mm"));

		case DMPAPER_ENV_B4 :
			return CString(_T("Envelope B4  250 x 353 mm"));

		case DMPAPER_ENV_B5 :
			return CString(_T("Envelope B5  176 x 250 mm"));

		case DMPAPER_ENV_B6 :
			return CString(_T("Envelope B6  176 x 125 mm"));

		case DMPAPER_ENV_ITALY :
			return CString(_T("Envelope 110 x 230 mm"));

		case DMPAPER_ENV_MONARCH :
			return CString(_T("Envelope Monarch 3.875 x 7.5 in"));

		case DMPAPER_ENV_PERSONAL :
			return CString(_T("6 3/4 Envelope 3 5/8 x 6 1/2 in"));

		case DMPAPER_FANFOLD_US :
			return CString(_T("US Std Fanfold 14 7/8 x 11 in"));

		case DMPAPER_FANFOLD_STD_GERMAN :
			return CString(_T("German Std Fanfold 8 1/2 x 12 in"));

		case DMPAPER_FANFOLD_LGL_GERMAN :
			return CString(_T("German Legal Fanfold 8 1/2 x 13 in"));

		case DMPAPER_ISO_B4 :
			return CString(_T("B4 (ISO) 250 x 353 mm"));

		case DMPAPER_JAPANESE_POSTCARD :
			return CString(_T("Japanese Postcard 100 x 148 mm"));

		case DMPAPER_9X11 :
			return CString(_T("9 x 11 in"));

		case DMPAPER_10X11 :
			return CString(_T("10 x 11 in"));

		case DMPAPER_15X11 :
			return CString(_T("15 x 11 in"));

		case DMPAPER_ENV_INVITE :
			return CString(_T("Envelope Invite 220 x 220 mm"));

		case DMPAPER_RESERVED_48 :				/* RESERVED--DO NOT USE               */
			return CString(_T(""));

		case DMPAPER_RESERVED_49 :				/* RESERVED--DO NOT USE               */
			return CString(_T(""));

		case DMPAPER_LETTER_EXTRA :
			return CString(_T("Letter Extra 9 \275 x 12 in"));

		case DMPAPER_LEGAL_EXTRA :
			return CString(_T("Legal Extra 9 \275 x 15 in"));

		case DMPAPER_TABLOID_EXTRA :
			return CString(_T("Tabloid Extra 11.69 x 18 in"));

		case DMPAPER_A4_EXTRA :
			return CString(_T("A4 Extra 236 x 322 mm"));

		case DMPAPER_LETTER_TRANSVERSE :
			return CString(_T("Letter Transverse 8 \275 x 11 in"));

		case DMPAPER_A4_TRANSVERSE :
			return CString(_T("A4 Transverse 210 x 297 mm"));

		case DMPAPER_LETTER_EXTRA_TRANSVERSE :
			return CString(_T("Letter Extra Transverse 9\275 x 12 in"));

		case DMPAPER_A_PLUS :
			return CString(_T("SuperA/SuperA/A4 227 x 356 mm"));

		case DMPAPER_B_PLUS :
			return CString(_T("SuperB/SuperB/A3 305 x 487 mm"));

		case DMPAPER_LETTER_PLUS :
			return CString(_T("Letter Plus 8.5 x 12.69 in"));

		case DMPAPER_A4_PLUS :
			return CString(_T("A4 Plus 210 x 330 mm"));

		case DMPAPER_A5_TRANSVERSE :
			return CString(_T("A5 Transverse 148 x 210 mm"));

		case DMPAPER_B5_TRANSVERSE :
			return CString(_T("B5 (JIS) Transverse 182 x 257 mm"));

		case DMPAPER_A3_EXTRA :
			return CString(_T("A3 Extra 322 x 445 mm"));

		case DMPAPER_A5_EXTRA :
			return CString(_T("A5 Extra 174 x 235 mm"));

		case DMPAPER_B5_EXTRA :
			return CString(_T("B5 (ISO) Extra 201 x 276 mm"));

		case DMPAPER_A2 :
			return CString(_T("A2 420 x 594 mm"));

		case DMPAPER_A3_TRANSVERSE :
			return CString(_T("A3 Transverse 297 x 420 mm"));

		case DMPAPER_A3_EXTRA_TRANSVERSE :
			return CString(_T("A3 Extra Transverse 322 x 445 mm"));

		case DMPAPER_DBL_JAPANESE_POSTCARD :
			return CString(_T("Japanese Double Postcard 200 x 148 mm"));

		case DMPAPER_A6 :
			return CString(_T("A6 105 x 148 mm"));

		case DMPAPER_JENV_KAKU2 :
			return CString(_T("Japanese Envelope Kaku #2"));

		case DMPAPER_JENV_KAKU3 :
			return CString(_T("Japanese Envelope Kaku #3"));

		case DMPAPER_JENV_CHOU3 :
			return CString(_T("Japanese Envelope Chou #3"));

		case DMPAPER_JENV_CHOU4 :
			return CString(_T("Japanese Envelope Chou #4"));

		case DMPAPER_LETTER_ROTATED :
			return CString(_T("Letter Rotated 11 x 8 1/2 11 in"));

		case DMPAPER_A3_ROTATED :
			return CString(_T("A3 Rotated 420 x 297 mm"));

		case DMPAPER_A4_ROTATED :
			return CString(_T("A4 Rotated 297 x 210 mm"));

		case DMPAPER_A5_ROTATED :
			return CString(_T("A5 Rotated 210 x 148 mm"));

		case DMPAPER_B4_JIS_ROTATED :
			return CString(_T("B4 (JIS) Rotated 364 x 257 mm"));

		case DMPAPER_B5_JIS_ROTATED :
			return CString(_T("B5 (JIS) Rotated 257 x 182 mm"));

		case DMPAPER_JAPANESE_POSTCARD_ROTATED :
			return CString(_T("Japanese Postcard Rotated 148 x 100 mm"));

		case DMPAPER_DBL_JAPANESE_POSTCARD_ROTATED :
			return CString(_T("Double Japanese Postcard Rotated 148 x 200 mm"));

		case DMPAPER_A6_ROTATED :
			return CString(_T("A6 Rotated 148 x 105 mm"));

		case DMPAPER_JENV_KAKU2_ROTATED :
			return CString(_T("Japanese Envelope Kaku #2 Rotated"));

		case DMPAPER_JENV_KAKU3_ROTATED :
			return CString(_T("Japanese Envelope Kaku #3 Rotated"));

		case DMPAPER_JENV_CHOU3_ROTATED :
			return CString(_T("Japanese Envelope Chou #3 Rotated"));

		case DMPAPER_JENV_CHOU4_ROTATED :
			return CString(_T("Japanese Envelope Chou #4 Rotated"));

		case DMPAPER_B6_JIS :
			return CString(_T("B6 (JIS) 128 x 182 mm"));

		case DMPAPER_B6_JIS_ROTATED :
			return CString(_T("B6 (JIS) Rotated 182 x 128 mm"));

		case DMPAPER_12X11 :
			return CString(_T("12 x 11 in"));

		case DMPAPER_JENV_YOU4 :
			return CString(_T("Japanese Envelope You #4"));

		case DMPAPER_JENV_YOU4_ROTATED :
			return CString(_T("Japanese Envelope You #4 Rotated"));

		case DMPAPER_P16K :
			return CString(_T("PRC 16K 146 x 215 mm"));

		case DMPAPER_P32K :
			return CString(_T("PRC 32K 97 x 151 mm"));

		case DMPAPER_P32KBIG :
			return CString(_T("PRC 32K(Big) 97 x 151 mm"));

		case DMPAPER_PENV_1 :
			return CString(_T("PRC Envelope #1 102 x 165 mm"));

		case DMPAPER_PENV_2 :
			return CString(_T("PRC Envelope #2 102 x 176 mm"));

		case DMPAPER_PENV_3 :
			return CString(_T("PRC Envelope #3 125 x 176 mm"));

		case DMPAPER_PENV_4 :
			return CString(_T("PRC Envelope #4 110 x 208 mm"));

		case DMPAPER_PENV_5 :
			return CString(_T("PRC Envelope #5 110 x 220 mm"));

		case DMPAPER_PENV_6 :
			return CString(_T("PRC Envelope #6 120 x 230 mm"));

		case DMPAPER_PENV_7 :
			return CString(_T("PRC Envelope #7 160 x 230 mm"));

		case DMPAPER_PENV_8 :
			return CString(_T("PRC Envelope #8 120 x 309 mm"));

		case DMPAPER_PENV_9 :
			return CString(_T("PRC Envelope #9 229 x 324 mm"));

		case DMPAPER_PENV_10 :
			return CString(_T("PRC Envelope #10 324 x 458 mm"));

		case DMPAPER_P16K_ROTATED :
			return CString(_T("PRC 16K Rotated"));

		case DMPAPER_P32K_ROTATED :
			return CString(_T("PRC 32K Rotated"));

		case DMPAPER_P32KBIG_ROTATED :
			return CString(_T("PRC 32K(Big) Rotated"));

		case DMPAPER_PENV_1_ROTATED :
			return CString(_T("PRC Envelope #1 Rotated 165 x 102 mm"));

		case DMPAPER_PENV_2_ROTATED :
			return CString(_T("PRC Envelope #2 Rotated 176 x 102 mm"));

		case DMPAPER_PENV_3_ROTATED :
			return CString(_T("PRC Envelope #3 Rotated 176 x 125 mm"));

		case DMPAPER_PENV_4_ROTATED :
			return CString(_T("PRC Envelope #4 Rotated 208 x 110 mm"));

		case DMPAPER_PENV_5_ROTATED :
			return CString(_T("PRC Envelope #5 Rotated 220 x 110 mm"));

		case DMPAPER_PENV_6_ROTATED :
			return CString(_T("PRC Envelope #6 Rotated 230 x 120 mm"));

		case DMPAPER_PENV_7_ROTATED :
			return CString(_T("PRC Envelope #7 Rotated 230 x 160 mm"));

		case DMPAPER_PENV_8_ROTATED :
			return CString(_T("PRC Envelope #8 Rotated 309 x 120 mm"));

		case DMPAPER_PENV_9_ROTATED :
			return CString(_T("PRC Envelope #9 Rotated 324 x 229 mm"));

		case DMPAPER_PENV_10_ROTATED :
			return CString(_T("PRC Envelope #10 Rotated 458 x 324 mm"));

		default :
			// Get the current form name from the DeviceCapabilities function
			CString sFormName = PaperSizeToFormName(papersize);
			if (sFormName != _T(""))
				return sFormName;
			// If the above fails use dmFormName, that must be the last option
			// because many drivers do not support this field!
			else
			{
				PRINTDLG pd;
				pd.lStructSize = (DWORD)sizeof(PRINTDLG);
				BOOL bRet = ::AfxGetApp()->GetPrinterDeviceDefaults(&pd);
				if (bRet)
				{
					// Protect memory handle with ::GlobalLock and ::GlobalUnlock
					LPDEVMODE pDevMode = (LPDEVMODE)::GlobalLock(pd.hDevMode);
			
					// Get form name
					if (pDevMode)
						sFormName = pDevMode->dmFormName;
					::GlobalUnlock(pd.hDevMode);
				}
				return sFormName;
			}
	}
}

// Gesture handling for Windows 7 and higher
LRESULT CPicturePrintPreviewView::OnGesture(WPARAM /*wParam*/, LPARAM lParam)
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// Get gesture function pointers
	typedef	BOOL (WINAPI *FPGETGESTUREINFO)(HGESTUREINFO_COMPATIBLE, PGESTUREINFO_COMPATIBLE);
	typedef	BOOL (WINAPI *FPCLOSEGESTUREINFOHANDLE)(HGESTUREINFO_COMPATIBLE);
	HINSTANCE h = ::LoadLibrary(_T("user32.dll"));
	if (!h)
		return Default();
	FPGETGESTUREINFO fpGetGestureInfo = (FPGETGESTUREINFO)::GetProcAddress(h, "GetGestureInfo");
	FPCLOSEGESTUREINFOHANDLE fpCloseGestureInfoHandle = (FPCLOSEGESTUREINFOHANDLE)::GetProcAddress(h, "CloseGestureInfoHandle");
	if (fpGetGestureInfo == NULL || fpCloseGestureInfoHandle == NULL)
	{
		::FreeLibrary(h);
		return Default();
	}

	// Get current gesture info
	GESTUREINFO_COMPATIBLE CurrentGestureInfo;
	memset(&CurrentGestureInfo, 0, sizeof(GESTUREINFO_COMPATIBLE));
	CurrentGestureInfo.cbSize = sizeof(GESTUREINFO_COMPATIBLE);
	if (!fpGetGestureInfo((HGESTUREINFO_COMPATIBLE)lParam, &CurrentGestureInfo) || CurrentGestureInfo.hwndTarget != GetSafeHwnd())
	{
		::FreeLibrary(h);
		return Default();
	}

	// Process gesture
	BOOL bDefaultProcessing = TRUE;
	switch (CurrentGestureInfo.dwID)
	{
		case GID_ZOOM :
		{
			if (CurrentGestureInfo.dwFlags & GF_BEGIN)
			{
				// Init vars
				m_ullGestureZoomStart = CurrentGestureInfo.ullArguments;
				if (m_ullGestureZoomStart == 0)
					m_ullGestureZoomStart = 1;
				m_dGestureZoomStartPrintScale = pDoc->m_dPrintScale;
			}
			else
			{
				// Calc. zoom percent
				double dValue = 100.0 * m_dGestureZoomStartPrintScale * CurrentGestureInfo.ullArguments / m_ullGestureZoomStart;
				if (dValue >= MIN_PRINT_SCALE && dValue <= MAX_PRINT_SCALE)
				{
					// Set new print scale
					pDoc->m_dPrintScale = dValue / 100.0;
					m_pScaleEdit->SetPrintScale(pDoc->m_dPrintScale);

					// Clear Print Size Fit Check Box
					CButton* pCheck = (CButton*)m_pToolBar->GetDlgItem(IDC_CHECK_SIZE_FIT);
					pCheck->SetCheck(0);
					pDoc->m_bPrintSizeFit = FALSE;

					// Invalidate
					Invalidate(FALSE);
				}
			}

			bDefaultProcessing = FALSE;
			break;
		}

		case GID_PAN :
		{
			// Get current position in client coordinates
			CPoint pt(CurrentGestureInfo.ptsLocation.x, CurrentGestureInfo.ptsLocation.y);
			ScreenToClient(&pt);

			if (CurrentGestureInfo.dwFlags & GF_BEGIN)
			{
				// Init var
				m_ptGesturePanStart = pt;
			}
			else if (CurrentGestureInfo.dwFlags & GF_END)
			{
				// Init var
				pDoc->m_ptLastPrintOffset = pDoc->m_ptPrintOffset;
			}
			else
			{
				// Calc. delta
				int dx = pt.x - m_ptGesturePanStart.x;
				int dy = pt.y - m_ptGesturePanStart.y;
				
				// Print Offset Calculation
				int nXOffset = MulDiv(dx, m_pPageInfo[0].sizeScaleRatio.cy, m_pPageInfo[0].sizeScaleRatio.cx);
				int nYOffset = MulDiv(dy, m_pPageInfo[0].sizeScaleRatio.cy, m_pPageInfo[0].sizeScaleRatio.cx);

				// Convert screen coordinates to printer coordinates 
				nXOffset = MulDiv(nXOffset, m_sizePrinterPPI.cx, afxData.cxPixelsPerInch);
				nYOffset = MulDiv(nYOffset, m_sizePrinterPPI.cy, afxData.cyPixelsPerInch);

				// Set Offset
				pDoc->m_ptPrintOffset.x = pDoc->m_ptLastPrintOffset.x + nXOffset;
				pDoc->m_ptPrintOffset.y = pDoc->m_ptLastPrintOffset.y + nYOffset;

				// Clear Print Size Fit Check Box
				CButton* pCheck = (CButton*)m_pToolBar->GetDlgItem(IDC_CHECK_SIZE_FIT);
				pCheck->SetCheck(0);
				pDoc->m_bPrintSizeFit = FALSE;

				// Invalidate
				Invalidate(FALSE);
			}

			bDefaultProcessing = FALSE;
			break;
		}

		default :
			break; // do default processing
	}

	// Close info handle if processing stops here
	if (!bDefaultProcessing)
		fpCloseGestureInfoHandle((HGESTUREINFO_COMPATIBLE)lParam);

	// Free library
	::FreeLibrary(h);

	// return 0 if processing done here
	return bDefaultProcessing ? Default() : 0;
}