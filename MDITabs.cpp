// Credits: inspired by Christian Rodemeyer's CMDITabs

/* 
	Open BUG

	In rare cases the tabs are not drawn correctly, only my custom drawn X is shown.
	Two users reported about that and one time I could experience the problem while
	running in the debugger: I saw that the title strings were taken correctly from 
	the documents, I then tried to redraw, maximize, full-screen, nothing fixed it. 
	Only adding a new document fixed the tabs (the difference is that an InsertItem() 
	gets called).

	The Win32 documentation for the TCM_GETITEM message under the Remarks section 
	writes about a curious behaviour:
	"If the TCIF_TEXT flag is set in the mask member of the TCITEM structure, the 
	control may change the pszText member of the structure to point to the new text 
	instead of filling the buffer with the requested text. The control may set the 
	pszText member to NULL to indicate that no text is associated with the item."
	https://learn.microsoft.com/windows/win32/controls/tcm-getitem
	->	could it be that the control's own OnPaint() handler sends a TCM_GETITEM 
		message to read the text, but it does not take into account that the pointer 
		it passes can be replaced? Probably not, because when the bug appears, not 
		only the tab text are missing, but also all the separators are not drawn.
*/

#include "stdafx.h"
#include "MainFrm.h"
#include "uImagerDoc.h"
#include "MDITabs.h"
#include <afxpriv.h> // for WM_SIZEPARENT

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define TABS_HEIGHT				19
#define TABS_PADDING_X			26
#define TABS_PADDING_Y			5
#define CLOSE_THICKNESS			1
#define CLOSE_COLOR				RGB(0x30,0x30,0x30)
#define CLOSE_HOT_COLOR			RGB(0xff,0xff,0xff)
#define CLOSE_HOT_BKGCOLOR		RGB(0xe8,0x11,0x23)
#define CLOSE_BKG_PADDING		4
#define CLOSE_CROSS_PADDING		7

CMDITabs::CMDITabs()
{
	m_nMinViews = 0;
	m_bImages = FALSE;
	m_bTop    = FALSE;
	m_bTracking = FALSE;
	m_nCloseHotTabIndex = -1;
	m_bInUpdate = FALSE;
}

BEGIN_MESSAGE_MAP(CMDITabs, CTabCtrl)
	ON_MESSAGE(WM_SIZEPARENT, OnSizeParent)
	ON_NOTIFY_REFLECT(TCN_SELCHANGE, OnSelChange)
	ON_WM_PAINT()
	ON_WM_NCPAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

afx_msg LRESULT CMDITabs::OnSizeParent(WPARAM, LPARAM lParam)
{
	CFrameWnd* pFrame = GetParentFrame();
	if (pFrame)
		pFrame = pFrame->GetActiveFrame();
	if (GetItemCount() < m_nMinViews || (pFrame && !pFrame->IsZoomed()))
	{
		ShowWindow(SW_HIDE);
	}
	else 
	{  
		AFX_SIZEPARENTPARAMS* pParams = reinterpret_cast<AFX_SIZEPARENTPARAMS*>(lParam);
		const BOOL bLayoutQuery = (pParams->hDWP == NULL);

		const int height = ::SystemDPIScale(2 * TABS_PADDING_Y + TABS_HEIGHT) + (m_bImages ? 1 : 0);
		const int offset = 2;

		m_height = height + offset;
		m_width  = pParams->rect.right - pParams->rect.left;

		if (m_bTop)
		{
			pParams->rect.top += height;
			if (!bLayoutQuery)
				MoveWindow(pParams->rect.left, pParams->rect.top - height, m_width, m_height, TRUE);
		}
		else
		{
			pParams->rect.bottom -= height;
			if (!bLayoutQuery)
				MoveWindow(pParams->rect.left, pParams->rect.bottom - offset, m_width, m_height, TRUE);
		}
		ShowWindow(SW_NORMAL);
	}
	return 0;
}

void CMDITabs::OnSelChange(NMHDR* pNMHDR, LRESULT* pResult)
{
	TCITEM item = {};
	item.mask = TCIF_PARAM;
	GetItem(GetCurSel(), &item);
	::BringWindowToTop(HWND(item.lParam));
	*pResult = 0;
}

void CMDITabs::Update()
{
	// Do not call us another time (for example RecalcLayout() calls us another time)
	if (m_bInUpdate)
		return;

	// Set flag indicating that we are in Update()
	m_bInUpdate = TRUE;

	// Updating a control or window with large amounts of data may cause 
	// flicker. In such cases it may be better to turn off drawing to
	// prevent changes from being redrawn
	SetRedraw(FALSE);

	// Get active child frame window
	HWND hActiveChildFrameWnd = ::AfxGetMainFrame()->MDIGetActive()->GetSafeHwnd();

	// Put all child frame windows in an array
	CArray<HWND, HWND> ChildFrameWndArray;
	POSITION posTemplate = ::AfxGetApp()->GetFirstDocTemplatePosition();
	while (posTemplate)
	{
		CMultiDocTemplate* curTemplate = (CMultiDocTemplate*)(::AfxGetApp()->GetNextDocTemplate(posTemplate));
		POSITION posDoc = curTemplate->GetFirstDocPosition();
		while (posDoc)
		{
			CDocument* pDoc = (CDocument*)(curTemplate->GetNextDoc(posDoc));
			if (pDoc)
			{
				POSITION posView = pDoc->GetFirstViewPosition();
				if (posView != NULL)
				{
					CView* pView = pDoc->GetNextView(posView);
					if (pView && pView->GetParentFrame())
					{
						HWND hChildFrameWnd = pView->GetParentFrame()->GetSafeHwnd();
						if (::IsWindow(hChildFrameWnd))
							ChildFrameWndArray.Add(hChildFrameWnd);
					}
				}
			}
		}
	}

	// Vars used in the following two loops
	TCITEM item;
	TCHAR szTitle[MAX_PATH];
	int i;

	// Loop through all tabs
	for (i = GetItemCount(); i--;)
	{
		// Get the tab and search it in the child frame windows array
		memset(&item, 0, sizeof(TCITEM));
		item.mask = TCIF_PARAM;
		GetItem(i, &item);
		int nTabInChildFrameWndArrayPos = -1;
		for (int n = 0; n < ChildFrameWndArray.GetCount(); n++)
		{
			if (ChildFrameWndArray[n] == (HWND)(item.lParam))
			{
				nTabInChildFrameWndArrayPos = n;
				break;
			}
		}

		// Delete the tab (associated child frame window does no longer exist) 
		if (nTabInChildFrameWndArrayPos < 0)
		{
			DeleteItem(i);
			if (m_bImages)
				RemoveImage(i);
		}
		// Update the tab
		else
		{
			// Get tab title from document
			_tcsncpy(szTitle, ML_STRING(1384, "Loading..."), MAX_PATH);
			szTitle[MAX_PATH - 1] = _T('\0');
			CMDIChildWnd* pChildFrameWnd = (CMDIChildWnd*)FromHandlePermanent(ChildFrameWndArray[nTabInChildFrameWndArrayPos]);
			if (pChildFrameWnd != NULL)
			{
				CUImagerDoc* pDoc = (CUImagerDoc*)pChildFrameWnd->GetActiveDocument();
				if (pDoc != NULL)
				{
					_tcsncpy(szTitle, pDoc->m_sTabTitle, MAX_PATH);
					szTitle[MAX_PATH - 1] = _T('\0');
				}
			}

			// Update image
			if (m_bImages)
				m_images.Replace(i, (HICON)::GetClassLongPtr(ChildFrameWndArray[nTabInChildFrameWndArrayPos], GCLP_HICON));

			// Update tab item
			memset(&item, 0, sizeof(TCITEM));
			item.mask = TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM;
			item.pszText = szTitle;
			if (m_bImages)
				item.iImage = i;
			else
				item.iImage = -1;
			item.lParam = LPARAM(ChildFrameWndArray[nTabInChildFrameWndArrayPos]);
			SetItem(i, &item);

			// If associated view is active make it the current selection
			if (ChildFrameWndArray[nTabInChildFrameWndArrayPos] == hActiveChildFrameWnd)
				SetCurSel(i);

			// Remove child frame window from list
			ChildFrameWndArray.RemoveAt(nTabInChildFrameWndArrayPos);
		}
	}

	// All remaining child frame windows have to be added as new tabs
	i = GetItemCount(); // insertion index
	for (int n = 0; n < ChildFrameWndArray.GetCount(); n++)
	{
		// Get tab title from document
		_tcsncpy(szTitle, ML_STRING(1384, "Loading..."), MAX_PATH);
		szTitle[MAX_PATH - 1] = _T('\0');
		CMDIChildWnd* pChildFrameWnd = (CMDIChildWnd*)FromHandlePermanent(ChildFrameWndArray[n]);
		if (pChildFrameWnd != NULL)
		{
			CUImagerDoc* pDoc = (CUImagerDoc*)pChildFrameWnd->GetActiveDocument();
			if (pDoc != NULL)
			{
				_tcsncpy(szTitle, pDoc->m_sTabTitle, MAX_PATH);
				szTitle[MAX_PATH - 1] = _T('\0');
			}
		}

		// Add image
		if (m_bImages)
			m_images.Add((HICON)::GetClassLongPtr(ChildFrameWndArray[n], GCLP_HICON));

		// Add tab item
		memset(&item, 0, sizeof(TCITEM));
		item.mask = TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM;
		item.pszText = szTitle;
		if (m_bImages)
			item.iImage = i;
		else
			item.iImage = -1;
		item.lParam = LPARAM(ChildFrameWndArray[n]);
		InsertItem(i, &item);

		// If associated view is active make it the current selection
		if (ChildFrameWndArray[n] == hActiveChildFrameWnd)
			SetCurSel(i);

		// Inc. total items count
		++i;
	}

	// This removes the control when there are no tabs and shows it when there is at least one
	CWnd* pWnd = FromHandlePermanent(hActiveChildFrameWnd);
	BOOL bShow = (GetItemCount() >= m_nMinViews && pWnd && pWnd->IsZoomed());
	if ((!bShow && IsWindowVisible()) || (bShow && !IsWindowVisible()))
		::AfxGetMainFrame()->RecalcLayout();

	// Resume drawing
	SetRedraw(TRUE);
	RedrawWindow(NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_ERASE);

	// Clear flag indicating that we are in Update()
	m_bInUpdate = FALSE;
}

void CMDITabs::OnPaint()
{
	CPaintDC dc(this);

	// Do nothing if no items
	if (GetItemCount() == 0)
		return;

	// Draw the control as usual
	_AFX_THREAD_STATE* pThreadState = ::AfxGetThreadState();
	pThreadState->m_lastSentMsg.wParam = WPARAM(HDC(dc));
	Default();

	// And paint also the Close Cross
	int nPadding = ::SystemDPIScale(CLOSE_CROSS_PADDING);
	int nPenThickness = ::SystemDPIScale(CLOSE_THICKNESS);
	for (int i = 0; i < GetItemCount(); i++)
	{
		CRect rcItem;
		GetItemRect(i, &rcItem);
		CPen Pen;
		if (i == m_nCloseHotTabIndex)
		{
			dc.FillSolidRect(GetCloseBkgRect(i), CLOSE_HOT_BKGCOLOR);
			Pen.CreatePen(PS_SOLID, nPenThickness, CLOSE_HOT_COLOR);
		}
		else
			Pen.CreatePen(PS_SOLID, nPenThickness, CLOSE_COLOR);
		CPen* pOldPen = dc.SelectObject(&Pen);
		int left = rcItem.right - rcItem.Height() + nPadding;
		int top = rcItem.top + nPadding;
		int right = rcItem.right - nPadding;
		int bottom = rcItem.bottom - nPadding;
		dc.MoveTo(left, top);
		dc.LineTo(right + 1, bottom + 1);	/* draws: \ */
		dc.MoveTo(right, top);
		dc.LineTo(left - 1, bottom + 1);	/* draws: / */
		dc.SelectObject(pOldPen);
	}
}

void CMDITabs::OnNcPaint()
{
	HDC hdc = ::GetWindowDC(m_hWnd);

	CRect rect;
	rect.left = 0;
	rect.top = m_bTop ? 0 : -2;
	rect.right = m_width;
	rect.bottom = m_height;

	HPEN pen = ::CreatePen(PS_SOLID, 0, ::GetSysColor(COLOR_3DFACE));
	HGDIOBJ old = ::SelectObject(hdc, pen);
	if (m_bTop)
	{
		DrawEdge(hdc, rect, EDGE_SUNKEN, BF_LEFT|BF_RIGHT|BF_TOP);
		::MoveToEx(hdc, 2, m_height - 1, NULL);
		::LineTo(hdc, m_width - 2, m_height - 1);
		::MoveToEx(hdc, 2, m_height - 2, NULL);
		::LineTo(hdc, m_width - 2, m_height - 2);
	}
	else
	{
		DrawEdge(hdc, rect, EDGE_SUNKEN, BF_LEFT|BF_RIGHT|BF_BOTTOM);
		::MoveToEx(hdc, 2, 0, NULL);
		::LineTo(hdc, m_width - 2, 0);
		::MoveToEx(hdc, 2, 1, NULL);
		::LineTo(hdc, m_width - 2, 1);
	}
	::SelectObject(hdc, old);
	::DeleteObject(pen);
	::ReleaseDC(m_hWnd, hdc);
}

void CMDITabs::Create(CMainFrame* pMainFrame, DWORD dwStyle)
{
	ASSERT_VALID(pMainFrame);

	// Init vars
	m_bTop = (dwStyle & MT_TOP);
	m_nMinViews = (dwStyle & MT_HIDEWLT2VIEWS) ? 2 : 1;

	// Create tab
	CTabCtrl::Create(WS_CHILD | WS_VISIBLE | (m_bTop ? 0 : TCS_BOTTOM) | TCS_SINGLELINE | TCS_FOCUSNEVER | TCS_FORCEICONLEFT | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
					CRect(0, 0, 0, 0), pMainFrame, 42);
	ModifyStyleEx(0, WS_EX_CLIENTEDGE);
	
	// Set font
	NONCLIENTMETRICS ncm = {sizeof(NONCLIENTMETRICS)};
	VERIFY(::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0));
	if ((HFONT)m_Font)
		m_Font.DeleteObject();
	m_Font.CreateFontIndirect(&(ncm.lfMessageFont));
	SetFont(&m_Font, FALSE);

	// Manipulate Z-order so, that our tabctrl is above the mdi client, but below any status bar
	::SetWindowPos(m_hWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	::SetWindowPos(pMainFrame->m_hWndMDIClient, m_hWnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	// Init images
	m_bImages = (dwStyle & MT_IMAGES) != 0;
	if (m_bImages)
	{
		if (m_images.GetSafeHandle())
			m_images.SetImageCount(0);
		else    
			m_images.Create(::SystemDPIScale(16), ::SystemDPIScale(16), ILC_COLORDDB | ILC_MASK, 1, 1);
		SetImageList(&m_images);
	}

	// Set padding
	SetPadding(CSize(::SystemDPIScale(TABS_PADDING_X), ::SystemDPIScale(TABS_PADDING_Y)));
}

CRect CMDITabs::GetCloseBkgRect(int nTabIndex)
{
	CRect rc(0,0,0,0);
	if (nTabIndex >= 0)
	{
		GetItemRect(nTabIndex, &rc);
		int nPadding = ::SystemDPIScale(CLOSE_BKG_PADDING);
		rc.left = rc.right - rc.Height();
		rc.DeflateRect(nPadding, nPadding);
		rc.right += 1;
		rc.bottom += 1;
	}
	return rc;
}

void CMDITabs::OnLButtonDown(UINT nFlags, CPoint point)
{
	TCHITTESTINFO tcHit;
	tcHit.pt = point;
	int nTabIndex = HitTest(&tcHit);
	if (GetCloseBkgRect(nTabIndex).PtInRect(point))
	{
		TCITEM item = {};
		item.mask = TCIF_PARAM;
		GetItem(nTabIndex, &item);
		HWND hWnd = HWND(item.lParam);
		::SendMessage(hWnd, WM_SYSCOMMAND, SC_CLOSE, 0);
		return; // return without calling base class handler
	}
	CTabCtrl::OnLButtonDown(nFlags, point); // activate tab
}

void CMDITabs::OnMouseMove(UINT nFlags, CPoint point)
{
	// Do the tab hover
	CTabCtrl::OnMouseMove(nFlags, point);
		
	// Update m_nCloseHotTabIndex and invalidate for painting 
	TCHITTESTINFO tcHit;
	tcHit.pt = point;
	int nTabIndex = HitTest(&tcHit);
	int nCloseHotTabIndex = -1;
	if (GetCloseBkgRect(nTabIndex).PtInRect(point))
		nCloseHotTabIndex = nTabIndex;
	if (nCloseHotTabIndex != m_nCloseHotTabIndex)
	{
		m_nCloseHotTabIndex = nCloseHotTabIndex;
		Invalidate(FALSE);
	}

	// Get a message when leaving the client area
	// Note: newer tab control versions have tab hover display. In their
	// CTabCtrl::OnMouseMove(), the TrackMouseEvent() function is already
	// called, so that the following TrackMouseEvent() would not be 
	// necessary. But to remain compatible with older control versions, 
	// it does not harm to call TrackMouseEvent() multiple time (only
	// one OnMouseLeave() gets fired).
	if (!m_bTracking)
	{
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(tme);
		tme.hwndTrack = m_hWnd;
		tme.dwFlags = TME_LEAVE;
		tme.dwHoverTime = 0;
		m_bTracking = ::TrackMouseEvent(&tme);
	}
}

void CMDITabs::OnMouseLeave()
{	
	// Remove the tab hover
	CTabCtrl::OnMouseLeave();

	m_bTracking = FALSE;
	m_nCloseHotTabIndex = -1;
	Invalidate(FALSE);
}

void CMDITabs::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	TCHITTESTINFO hit;
	hit.pt = point;
	ScreenToClient(&hit.pt);
	int i = HitTest(&hit);
	if (i >= 0) 
	{
		TCITEM item = {};
		item.mask = TCIF_PARAM;
		GetItem(i, &item);
		HWND hWnd = HWND(item.lParam);
		SetCurSel(i);
		::BringWindowToTop(hWnd);

		HMENU menu = HMENU(::SendMessage(::GetTopWindow(hWnd), WM_GETTABSYSMENU, 0, 0));
		if (menu == 0)
			menu = ::GetSystemMenu(hWnd, FALSE);
		UINT cmd = ::TrackPopupMenu(menu, TPM_RETURNCMD|TPM_RIGHTBUTTON|TPM_LEFTALIGN, point.x, point.y, 0, m_hWnd, NULL);
		::SendMessage(hWnd, WM_SYSCOMMAND, cmd, 0);
	}
}
