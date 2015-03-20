/////////////////////////////////////////////////////////////////////////////
// DragDropListCtrl.cpp : implementation file
//
// Adrian Stanley 13/02/2000
// Totally free source code - use however you like.
// adrian@adrians.demon.co.uk.
//
// This class illustrates how to implement drag and drop for a MFC 
// list control.
//
// It has the following features:
//	Supports dragging of single and multiple selections.
//	Potential drag targets are highlighted (selected) as the mouse moves
//	over them.
//	The list box will scroll when you try to drag out of the top or bottom.
//	Horizontal mouse movement is ignored; if the mouse is to the left or right
//	of the list control, dragging still occurs as though the mouse is over
//	the control.
//	Works with LVS_EX_FULLROWSELECT style on or off.
//	Preserves checked state of dragged items.
//	All code encapulated in the control - no changes required to parent 
//	class.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "DragDropListCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const int SCROLL_TIMER_ID =		50;
static const int SCROLL_TIME_SLOW =		200;
static const int SCROLL_TIME_MEDIUM =	100;
static const int SCROLL_TIME_FAST =		30;
static const int SCROLL_DISTANCE =		25;
static const int HITTEST_X_OFFSET =		8;
static const int HITTEST_Y_OFFSET =		6;

/////////////////////////////////////////////////////////////////////////////
// CDragDropListCtrl

CDragDropListCtrl::CDragDropListCtrl() :
	m_nDropIndex(-1),
	m_pDragImage(NULL),
	m_nPrevDropIndex(-1),
	m_uPrevDropState(NULL),
	m_uiScrollTimer(0),
	m_uiScrollTime(SCROLL_TIME_MEDIUM),
	m_ScrollDirection(scrollNull),
	m_bUseResDragImage(FALSE),
	m_uiSingleSelectImageRes(0),
	m_uiMultiSelectImageRes(0),
	m_dwStyle(NULL)
{
}

CDragDropListCtrl::~CDragDropListCtrl()
{
	// Fail safe clean up
	if (m_pDragImage)
	{
		delete m_pDragImage;
		m_pDragImage = NULL;
	}

	KillScrollTimer();
}


BEGIN_MESSAGE_MAP(CDragDropListCtrl, CListCtrl)
	//{{AFX_MSG_MAP(CDragDropListCtrl)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_NOTIFY_REFLECT(LVN_BEGINDRAG, OnBeginDrag)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDragDropListCtrl message handlers

void CDragDropListCtrl::OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if (pNMListView)
	{
		m_nPrevDropIndex	= -1;
		m_uPrevDropState	= NULL;
		
		// Items being dragged - can be one or more.
		m_anDragIndexes.RemoveAll();
		POSITION pos = GetFirstSelectedItemPosition();
		while (pos)
		{
			m_anDragIndexes.Add(GetNextSelectedItem(pos));
		}

		DWORD dwStyle = GetStyle();
		if ((dwStyle & LVS_SINGLESEL) == LVS_SINGLESEL)
		{
			// List control is single select; we need it to be multi-select so
			// we can show both the drag item and potential drag target as selected.
			m_dwStyle = dwStyle;
			ModifyStyle(LVS_SINGLESEL, NULL);
		}

		if (m_anDragIndexes.GetSize() > 0)
		{
			// Create a drag image from the centre point of the item image.
			// Clean up any existing drag image first.
			if (m_pDragImage)
				delete m_pDragImage;
			CPoint ptDragItem;
			if (m_bUseResDragImage)
				m_pDragImage = CreateResDragImageEx(&ptDragItem);
			else
				m_pDragImage = CreateDragImageEx(&ptDragItem);
			if (m_pDragImage)
			{
				m_pDragImage->BeginDrag(0, ptDragItem);
				m_pDragImage->DragEnter(CWnd::GetDesktopWindow(), pNMListView->ptAction);
			
				// Capture all mouse messages in case the user drags outside the control.
				SetCapture();
			}
		}
	}
	
	*pResult = 0;
}

CImageList* CDragDropListCtrl::CreateResDragImageEx(LPPOINT lpPoint)
{
	// Load Bitmaps
	CBitmap Bitmap;
	if (GetSelectedCount() > 1)
		Bitmap.LoadBitmap(m_uiMultiSelectImageRes);
	else
		Bitmap.LoadBitmap(m_uiSingleSelectImageRes);

	// Get bitmap information
    BITMAP bm;
    if (!Bitmap.GetObject(sizeof(bm), (LPVOID)&bm))
		return NULL;

	// Create Image List
	CImageList* pDragImage = new CImageList;
	if (!pDragImage)
		return NULL;
	if (!pDragImage->Create(bm.bmWidth,
							bm.bmHeight,
							ILC_COLORDDB | ILC_MASK,
							1, 1))
	{
		delete pDragImage;
		return NULL;
	}
	if (pDragImage->Add(&Bitmap, &Bitmap) >= 0)
	{
		lpPoint->x = -16;
		lpPoint->y = -2;
		return pDragImage;
	}
	else
	{
		delete pDragImage;
		return NULL;
	}
}

// Based on code by Frank Kobs.
CImageList* CDragDropListCtrl::CreateDragImageEx(LPPOINT lpPoint)
{
	CRect rectSingle;	
	CRect rectComplete(0, 0, 0, 0);
	int	nIndex	= -1;
	BOOL bFirst	= TRUE;

	// Determine the size of the drag image.
	POSITION pos = GetFirstSelectedItemPosition();
	while (pos)
	{
		nIndex = GetNextSelectedItem(pos);
		GetItemRect(nIndex, rectSingle, LVIR_BOUNDS);
		if (bFirst)
		{
			// Initialize the CompleteRect
			GetItemRect(nIndex, rectComplete, LVIR_BOUNDS);
			bFirst = FALSE;
		}
		rectComplete.UnionRect(rectComplete, rectSingle);
	}

	// Create bitmap in memory DC
	CClientDC dcClient(this);	
	CDC dcMem;	
	CBitmap Bitmap;

	if (!dcMem.CreateCompatibleDC(&dcClient))
	{
		return NULL;
	}

	if (!Bitmap.CreateCompatibleBitmap(&dcClient, rectComplete.Width(), rectComplete.Height()))
	{
		return NULL;
	}
	
	CBitmap* pOldMemDCBitmap = dcMem.SelectObject(&Bitmap);
	// Here green is used as mask color.
	dcMem.FillSolidRect(0, 0, rectComplete.Width(), rectComplete.Height(), RGB(0, 255, 0)); 

	// Paint each DragImage in the DC.
	CImageList* pSingleImageList = NULL;
	CPoint pt;

	pos = GetFirstSelectedItemPosition();
	while (pos)
	{
		nIndex = GetNextSelectedItem(pos);
		GetItemRect(nIndex, rectSingle, LVIR_BOUNDS);

		pSingleImageList = CreateDragImage(nIndex, &pt);
		if (pSingleImageList)
		{
			// Make sure width takes into account not using LVS_EX_FULLROWSELECT style.
			IMAGEINFO ImageInfo;
			pSingleImageList->GetImageInfo(0, &ImageInfo);
			rectSingle.right = rectSingle.left + (ImageInfo.rcImage.right - ImageInfo.rcImage.left);

			pSingleImageList->DrawIndirect(
				&dcMem, 
				0, 
				CPoint(rectSingle.left - rectComplete.left, 
				rectSingle.top - rectComplete.top),
				rectSingle.Size(), 
				CPoint(0,0));

			delete pSingleImageList;
		}
	}

	dcMem.SelectObject(pOldMemDCBitmap);

	// Create the imagelist	with the merged drag images.
	CImageList* pCompleteImageList = new CImageList;
	
	pCompleteImageList->Create(rectComplete.Width(), rectComplete.Height(), ILC_COLOR | ILC_MASK, 0, 1);
	// Here green is used as mask color.
	pCompleteImageList->Add(&Bitmap, RGB(0, 255, 0)); 

	Bitmap.DeleteObject();
	dcMem.DeleteDC();

	// As an optional service:
	// Find the offset of the current mouse cursor to the imagelist
	// this we can use in BeginDrag().
	if (lpPoint)
	{
		CPoint ptCursor;
		::GetSafeCursorPos(&ptCursor);
		ScreenToClient(&ptCursor);
		lpPoint->x = ptCursor.x - rectComplete.left;
		lpPoint->y = ptCursor.y - rectComplete.top;
	}

	return pCompleteImageList;
}

void CDragDropListCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	// Dragging if there is a drag image
	if (m_pDragImage)
	{
		int nDropIndex;

		// Move the drag image.
		CPoint ptDragImage(point);
		ClientToScreen(&ptDragImage);
		m_pDragImage->DragMove(ptDragImage);
		
		// Leave dragging so we can update potential drop target selection
		if (!g_bWinVistaOrHigher)
			m_pDragImage->DragLeave(CWnd::GetDesktopWindow());

		// Client Rect & Coordinates
		CRect rectClient;
		GetClientRect(rectClient);
		CPoint ptClientDragImage(ptDragImage);
		ScreenToClient(&ptClientDragImage);

		// Client rect includes header height, so ignore it, i.e.,
		// moving the mouse over the header (and higher) will result in a scroll up
		CHeaderCtrl* pHeader = (CHeaderCtrl*)GetHeaderCtrl();
		if (pHeader)
		{
			CRect rectHeader;
			pHeader->GetClientRect(rectHeader);
			rectClient.top += rectHeader.Height();
		}

		// Ensures x coordinate is always valid
		point.x = HITTEST_X_OFFSET;

		// Scroll-Up
		if (ptClientDragImage.y < rectClient.top)
		{
			point.y	= rectClient.top + HITTEST_Y_OFFSET;
			if ((nDropIndex = HitTest(point)) == 0)
				UpdateSelection(-1);
			else
				UpdateSelection(nDropIndex);

			if (ptClientDragImage.y < (rectClient.top - 2*SCROLL_DISTANCE))
				SetScrollTimer(scrollUp, SCROLL_TIME_FAST);
			else if (ptClientDragImage.y < (rectClient.top - SCROLL_DISTANCE))
				SetScrollTimer(scrollUp, SCROLL_TIME_MEDIUM);
			else
				SetScrollTimer(scrollUp, SCROLL_TIME_SLOW);
		}
		// Scroll-Down
		else if (ptClientDragImage.y >= rectClient.bottom)
		{
			point.y	= rectClient.bottom - HITTEST_Y_OFFSET;
			if ((nDropIndex = HitTest(point)) == -1)
				nDropIndex = GetItemCount() - 1;
			UpdateSelection(nDropIndex);

			if (ptClientDragImage.y >= (rectClient.bottom + 2*SCROLL_DISTANCE))
				SetScrollTimer(scrollDown, SCROLL_TIME_FAST);
			else if (ptClientDragImage.y >= (rectClient.bottom + SCROLL_DISTANCE))
				SetScrollTimer(scrollDown, SCROLL_TIME_MEDIUM);
			else
				SetScrollTimer(scrollDown, SCROLL_TIME_SLOW);
		}
		// No Scroll
		else
		{
			// Kill Timer
			KillScrollTimer();

			// Still in list control so select item under mouse as potential drop target
			UINT uiFlag = 0;
			nDropIndex = HitTest(point, &uiFlag);
			if (uiFlag & LVHT_NOWHERE)
				UpdateSelection(GetItemCount() - 1);
			else
				UpdateSelection(nDropIndex);
		}

		// Resume dragging
		if (!g_bWinVistaOrHigher)
			m_pDragImage->DragEnter(CWnd::GetDesktopWindow(), ptDragImage);
	}
	else
		KillScrollTimer();

	CListCtrl::OnMouseMove(nFlags, point);
}

void CDragDropListCtrl::UpdateSelection(int nDropIndex)
{
	if (nDropIndex <= -1)
	{
		// Drop index is valid and has changed since last mouse movement.

		RestorePrevDropItemState();

		// Save information about current potential drop target for restoring next time round.
		m_nPrevDropIndex = -1;
		m_uPrevDropState = 0;
		m_nDropIndex = -1;		// Used by DropItem().

		UpdateWindow();
	}
	else if (nDropIndex > -1 && nDropIndex < GetItemCount())
	{
		// Drop index is valid and has changed since last mouse movement.

		RestorePrevDropItemState();

		// Save information about current potential drop target for restoring next time round.
		m_nPrevDropIndex = nDropIndex;
		m_uPrevDropState = GetItemState(nDropIndex, LVIS_SELECTED);

		// Select current potential drop target.
		SetItemState(nDropIndex, LVIS_SELECTED, LVIS_SELECTED);
		m_nDropIndex = nDropIndex;		// Used by DropItem().

		UpdateWindow();
	}
}

void CDragDropListCtrl::RestorePrevDropItemState()
{
	if (m_nPrevDropIndex > -1 && m_nPrevDropIndex < GetItemCount())
	{
		// Restore state of previous potential drop target.
		SetItemState(m_nPrevDropIndex, m_uPrevDropState, LVIS_SELECTED);
	}
}

void CDragDropListCtrl::SetScrollTimer(	EScrollDirection ScrollDirection,
										UINT uiScrollTime)
{
	if (m_uiScrollTimer == 0)
		m_uiScrollTimer = SetTimer(SCROLL_TIMER_ID, uiScrollTime, NULL);
	else if (uiScrollTime != m_uiScrollTime)
	{
		KillTimer(SCROLL_TIMER_ID);
		m_uiScrollTimer = SetTimer(SCROLL_TIMER_ID, uiScrollTime, NULL);
	}
	m_uiScrollTime = uiScrollTime;
	m_ScrollDirection = ScrollDirection;
}

void CDragDropListCtrl::KillScrollTimer()
{
	if (m_uiScrollTimer != 0)
	{
		KillTimer(SCROLL_TIMER_ID);
		m_uiScrollTimer		= 0;
		m_ScrollDirection	= scrollNull;
	}
}

void CDragDropListCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	SetFocus();
	CListCtrl::OnLButtonDown(nFlags, point);
}

void CDragDropListCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_pDragImage)
	{
		KillScrollTimer();

		// Release the mouse capture and end the dragging.
		::ReleaseCapture();
		m_pDragImage->DragLeave(CWnd::GetDesktopWindow());
		m_pDragImage->EndDrag();

		delete m_pDragImage;
		m_pDragImage = NULL;

		// Drop the item on the list.
		DropItem();
	}
	
	CListCtrl::OnLButtonUp(nFlags, point);
}

void CDragDropListCtrl::DropItem()
{
	RestorePrevDropItemState();

	// Drop after currently selected item.
	m_nDropIndex++;
	if (m_nDropIndex < 0 || m_nDropIndex > GetItemCount() - 1)
	{
		// Fail safe - invalid drop index, so drop at end of list.
		m_nDropIndex = GetItemCount();
	}

	int nColumns = 1;
	CHeaderCtrl* pHeader = (CHeaderCtrl*)GetHeaderCtrl();
	if (pHeader)
	{
		nColumns = pHeader->GetItemCount();
	}

	// Move all dragged items to their new positions.
	for (int nDragItem = 0; nDragItem < m_anDragIndexes.GetSize(); nDragItem++)
	{
		int nDragIndex = m_anDragIndexes[nDragItem];

		if (nDragIndex > -1 && nDragIndex < GetItemCount())
		{
			// Get information about this drag item.
			TCHAR szText[256];
			LV_ITEM lvItem;
			ZeroMemory(&lvItem, sizeof(LV_ITEM));
			lvItem.iItem		= nDragIndex;
			lvItem.mask			= LVIF_TEXT | LVIF_IMAGE | LVIF_STATE | LVIF_PARAM;
			lvItem.stateMask	= LVIS_DROPHILITED | LVIS_FOCUSED | LVIS_SELECTED | LVIS_STATEIMAGEMASK;
			lvItem.pszText		= szText;
			lvItem.cchTextMax	= sizeof(szText) - 1;
			GetItem(&lvItem);
			BOOL bChecked = GetCheck(nDragIndex);
			
			// Before moving drag item, make sure it is deselected in its original location,
			// otherwise GetSelectedCount() will return 1 too many.
			SetItemState(nDragIndex, static_cast<UINT>(~LVIS_SELECTED), LVIS_SELECTED);

			// Insert the dragged item at drop index.
			lvItem.iItem		= m_nDropIndex;
			InsertItem(&lvItem);
			if (bChecked)
			{
				SetCheck(m_nDropIndex);
			}
			
			// Index of dragged item will change if item has been dropped above itself.
			if (nDragIndex >= m_nDropIndex)
			{
				nDragIndex++;
			}

			// Fill in all the columns for the dragged item.
			lvItem.mask		= LVIF_TEXT;
			lvItem.iItem	= m_nDropIndex;

			for (int nColumn = 1; nColumn < nColumns; nColumn++)
			{
				_tcscpy(lvItem.pszText, (LPCTSTR)(GetItemText(nDragIndex, nColumn)));
				lvItem.iSubItem = nColumn;
				SetItem(&lvItem);
			}
			
			// Delete the original item.
			DeleteItem(nDragIndex);

			// Need to adjust indexes of remaining drag items.
			for (int nNewDragItem = nDragItem; nNewDragItem < m_anDragIndexes.GetSize(); nNewDragItem++)
			{
				int nNewDragIndex = m_anDragIndexes[nNewDragItem];

				if (nDragIndex < nNewDragIndex && nNewDragIndex < m_nDropIndex)
				{
					// Item has been removed from above this item, and inserted after it,
					// so this item moves up the list.
					m_anDragIndexes[nNewDragItem] = max(nNewDragIndex - 1, 0);
				}
				else if (nDragIndex > nNewDragIndex && nNewDragIndex > m_nDropIndex)
				{
					// Item has been removed from below this item, and inserted before it,
					// so this item moves down the list.
					m_anDragIndexes[nNewDragItem] = nNewDragIndex + 1;
				}
			}
			if (nDragIndex > m_nDropIndex)
			{
				// Item has been added before the drop target, so drop target moves down the list.
				m_nDropIndex++;
			}
		}
	}

	if (m_dwStyle != NULL)
	{
		// Style was modified, so return it back to original style.
		ModifyStyle(NULL, m_dwStyle);
		m_dwStyle = NULL;
	}
}

void CDragDropListCtrl::OnTimer(UINT nIDEvent) 
{
	if (nIDEvent == SCROLL_TIMER_ID && m_pDragImage)
	{
		CPoint point;
		point.x = HITTEST_X_OFFSET;
		CRect rcClient;
		GetClientRect(rcClient);
		CHeaderCtrl* pHeader = (CHeaderCtrl*)GetHeaderCtrl();
		if (pHeader)
		{
			CRect rcHeader;
			pHeader->GetClientRect(rcHeader);
			rcClient.top += rcHeader.Height();
		}

		WPARAM wParam = NULL;
		int nDropIndex;
		if (m_ScrollDirection == scrollUp)
		{
			point.y		= rcClient.top + HITTEST_Y_OFFSET;
			wParam		= MAKEWPARAM(SB_LINEUP, 0);
			nDropIndex	= HitTest(point) - 1;
		}
		else if (m_ScrollDirection == scrollDown)
		{
			point.y		= rcClient.bottom - HITTEST_Y_OFFSET;
			wParam		= MAKEWPARAM(SB_LINEDOWN, 0);
			if ((nDropIndex = HitTest(point)) == -1)
				nDropIndex = GetItemCount() - 1;
			else
				nDropIndex++;
		}
		else
			return;

		m_pDragImage->DragShowNolock(FALSE);
		SendMessage(WM_VSCROLL, wParam, NULL);
		UpdateSelection(nDropIndex);
		m_pDragImage->DragShowNolock(TRUE);
	}
	else
		CListCtrl::OnTimer(nIDEvent);
}
