/****************************************************************************\
Datei  : MDITabs.h
Projekt: MDITabs, a tabcontrol for switching between MDI-views
Inhalt : CMDITabs implementation
Datum  : 03.10.2001
Autor  : Christian Rodemeyer
Hinweis: © 2001 by Christian Rodemeyer
\****************************************************************************/

#include "stdafx.h"
#include "MDITabs.h"
#include <AFXPRIV.H>
#include <algorithm>
#include <vector>

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
#define CLOSE_PADDING			7

/////////////////////////////////////////////////////////////////////////////
// CMDITabs

CMDITabs::CMDITabs()
{
  m_mdiClient = NULL;
  m_minViews = 0;
  m_bImages = false;
  m_bTop    = false;
  m_bTracking = FALSE;
  m_nCloseHotTabIndex = -1;
}

BEGIN_MESSAGE_MAP(CMDITabs, CTabCtrl)
  ON_NOTIFY_REFLECT(TCN_SELCHANGE, OnSelChange)
  ON_WM_PAINT()
  ON_WM_NCPAINT()
  ON_WM_CONTEXTMENU()
  ON_WM_LBUTTONDOWN()
  ON_WM_MOUSEMOVE()
  ON_WM_MOUSELEAVE()
  ON_MESSAGE(WM_SIZEPARENT, OnSizeParent)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMDITabs message handlers

afx_msg LRESULT CMDITabs::OnSizeParent(WPARAM, LPARAM lParam)
{
  CFrameWnd* pFrame = GetParentFrame();
  if (pFrame) 
    pFrame = pFrame->GetActiveFrame();
  if (GetItemCount() < m_minViews || (pFrame && !pFrame->IsZoomed()))
  {
	  ShowWindow(SW_HIDE);
  }
  else 
  {  
    AFX_SIZEPARENTPARAMS* pParams = reinterpret_cast<AFX_SIZEPARENTPARAMS*>(lParam);
	const bool bLayoutQuery = (pParams->hDWP == NULL);

    const int height = ::SystemDPIScale(2 * TABS_PADDING_Y + TABS_HEIGHT) + (m_bImages ? 1 : 0);
    const int offset = 2;

    m_height = height + offset;
    m_width  = pParams->rect.right - pParams->rect.left;

    if (m_bTop)
    {
      pParams->rect.top += height;
	  if (!bLayoutQuery)
        MoveWindow(pParams->rect.left, pParams->rect.top - height, m_width, m_height, true);
    }
    else
    {
      pParams->rect.bottom -= height;
	  if (!bLayoutQuery)
        MoveWindow(pParams->rect.left, pParams->rect.bottom - offset, m_width, m_height, true);
    }
    ShowWindow(SW_NORMAL);
  }
  return 0;
}

void CMDITabs::OnSelChange(NMHDR* pNMHDR, LRESULT* pResult)
{
  TCITEM item;
  item.mask = TCIF_PARAM;
  GetItem(GetCurSel(), &item);
  ::BringWindowToTop(HWND(item.lParam));
  *pResult = 0;
}

void CMDITabs::Update()
{
  SetRedraw(false);

  HWND active = ::GetTopWindow(m_mdiClient); // get active view window (actually the frame of the view)

  typedef std::vector<HWND> TWndVec;
  typedef TWndVec::iterator TWndIter;

  TWndVec vChild; // put all child windows in a list (actually a vector)
  for (HWND child = active; child; child = ::GetNextWindow(child, GW_HWNDNEXT))
  {
    vChild.push_back(child);
  }

  TCITEM item;
  TCHAR text[256];
  item.pszText = text;

  int i;
  for (i = GetItemCount(); i--;)  // for each tab
  {
    item.mask = TCIF_PARAM;
    GetItem(i, &item);

    TWndIter it = std::find(vChild.begin(), vChild.end(), HWND(item.lParam));
    if (it == vChild.end()) // associatete view does no longer exist, so delete the tab
    {
      DeleteItem(i);
      if (m_bImages) RemoveImage(i);
    }
    else // update the tab's text, image and selection state
    {
      item.mask = TCIF_TEXT;
      ::GetWindowText(*it, text, 256);
      TCHAR* subtext = _tcsstr(text, _T(" , "));
      if (subtext)
        text[subtext - text] = _T('\0');

	  if (m_bImages) m_images.Replace(i, (HICON)::GetClassLongPtr(*it, GCLP_HICON));
      SetItem(i, &item);
      if (*it == active) SetCurSel(i); // associated view is active => make it the current selection
      vChild.erase(it);                // remove view from list
    }
  }

  // all remaining views in vChild have to be added as new tabs
  i = GetItemCount();
  for (TWndIter it = vChild.begin(), end = vChild.end(); it != end; ++it)
  {
    item.mask = TCIF_TEXT|TCIF_PARAM|TCIF_IMAGE;
    ::GetWindowText(*it, text, 256);
    TCHAR* subtext = _tcsstr(text, _T(" , "));
    if (subtext)
      text[subtext - text] = _T('\0');

	if (m_bImages) m_images.Add((HICON)::GetClassLongPtr(*it, GCLP_HICON));
    item.iImage = i;
    item.lParam = LPARAM(*it);
    InsertItem(i, &item);
    if (*it == active) SetCurSel(i);
    ++i;
  }

  // this removes the control when there are no tabs and shows it when there is at least one tab
  CWnd* wnd = FromHandlePermanent(active);
  bool bShow = (GetItemCount() >= m_minViews && wnd->IsZoomed());
  if ((!bShow && IsWindowVisible()) || (bShow && !IsWindowVisible())) 
  {
    static_cast<CMDIFrameWnd*>(FromHandlePermanent(::GetParent(m_mdiClient)))->RecalcLayout();
  }

  RedrawWindow(NULL, NULL, RDW_FRAME|RDW_INVALIDATE|RDW_ERASE);
  SetRedraw(true);
}

void CMDITabs::OnPaint()
{
	CPaintDC dc(this);

	if (GetItemCount() == 0) return; // do nothing

	// windows should draw the control as usual
	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	pThreadState->m_lastSentMsg.wParam = WPARAM(HDC(dc));
	Default();

	// Paint Close Cross
	int nPadding = ::SystemDPIScale(CLOSE_PADDING);
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

void CMDITabs::Create(CFrameWnd* pMainFrame, DWORD dwStyle)
{
  m_bTop = (dwStyle & MT_TOP);
  m_minViews = (dwStyle & MT_HIDEWLT2VIEWS) ? 2 : 1;

  CTabCtrl::Create(WS_CHILD|WS_VISIBLE|(m_bTop?0:TCS_BOTTOM)|TCS_SINGLELINE|TCS_FOCUSNEVER|TCS_FORCEICONLEFT|WS_CLIPSIBLINGS, CRect(0, 0, 0, 0), pMainFrame, 42);
  ModifyStyleEx(0, WS_EX_CLIENTEDGE);
  NONCLIENTMETRICS ncm = {sizeof(NONCLIENTMETRICS)};
  VERIFY(::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0));
  m_Font.CreateFontIndirect(&(ncm.lfMessageFont));
  SetFont(&m_Font, FALSE);

  HWND wnd;
  for (wnd = ::GetTopWindow(*pMainFrame); wnd; wnd = ::GetNextWindow(wnd, GW_HWNDNEXT))
  {
    TCHAR wndClass[32];
    ::GetClassName(wnd, wndClass, 32);
    if (_tcsncmp(wndClass, _T("MDIClient"), 32) == 0) break;
  }
  m_mdiClient = wnd;

  ASSERT(m_mdiClient); // Ooops, no MDIClient window?

  // manipulate Z-order so, that our tabctrl is above the mdi client, but below any status bar
  ::SetWindowPos(m_hWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
  ::SetWindowPos(m_mdiClient, m_hWnd, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
  m_bImages = (dwStyle & MT_IMAGES) != 0;
  if (m_bImages)
  {
    if (m_images.GetSafeHandle()) 
    {
      m_images.SetImageCount(0);
    }
    else    
    {
      m_images.Create(::SystemDPIScale(16), ::SystemDPIScale(16), ILC_COLORDDB | ILC_MASK, 1, 1);
    }
    SetImageList(&m_images);
  }

  SetPadding(CSize(::SystemDPIScale(TABS_PADDING_X), ::SystemDPIScale(TABS_PADDING_Y)));
}

CRect CMDITabs::GetCloseBkgRect(int nTabIndex)
{
	CRect rc(0,0,0,0);
	if (nTabIndex >= 0)
	{
		GetItemRect(nTabIndex, &rc);
		int nPadding = ::SystemDPIScale(CLOSE_PADDING);
		rc.left = rc.right - rc.Height();
		rc.DeflateRect(nPadding / 2, nPadding / 2);
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
		TCITEM item;
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
    TCITEM item;
    item.mask = TCIF_PARAM;
    GetItem(i, &item);

    HWND hWnd = HWND(item.lParam);
    SetCurSel(i);
    ::BringWindowToTop(hWnd);

    HMENU menu = HMENU(::SendMessage(::GetTopWindow(hWnd), WM_GETTABSYSMENU, 0, 0));
    if (menu == 0) menu = ::GetSystemMenu(hWnd, FALSE);
	UINT cmd = ::TrackPopupMenu(menu, TPM_RETURNCMD|TPM_RIGHTBUTTON|TPM_LEFTALIGN, point.x, point.y, 0, m_hWnd, NULL);
    ::SendMessage(hWnd, WM_SYSCOMMAND, cmd, 0);
  }
}
