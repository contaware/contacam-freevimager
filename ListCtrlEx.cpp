// ListCtrlEx.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "ListCtrlEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CListCtrlEx

CListCtrlEx::CListCtrlEx()
{
}

CListCtrlEx::~CListCtrlEx()
{
}

int CListCtrlEx::GetSelectedItem()
{
	UINT nListItem;
	if (GetSelectedCount() < 1)
		return -1;
	UINT nNoOfItems = GetItemCount();
	for (nListItem = 0 ; nListItem < nNoOfItems ; nListItem++)
	{
		if (GetItemState(nListItem, LVIS_SELECTED))
			break;
	}
	if (nListItem < nNoOfItems)
		return nListItem;
	else
		return -1;
}

BEGIN_MESSAGE_MAP(CListCtrlEx, CListCtrl)
	//{{AFX_MSG_MAP(CListCtrlEx)
	ON_WM_KEYDOWN()
	ON_NOTIFY_REFLECT_EX(LVN_BEGINLABELEDIT, OnBeginLabelEditList)
	ON_NOTIFY_REFLECT_EX(LVN_ENDLABELEDIT, OnEndLabelEditList)
	ON_NOTIFY_REFLECT_EX(NM_DBLCLK, OnDblClickEx)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CListCtrlEx message handlers

void CListCtrlEx::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	switch (nChar)
	{
		case VK_F2 :
		{
			ASSERT(GetStyle() & LVS_EDITLABELS);

			// Don't do an Edit Label when multiple items are selected
			if (GetSelectedCount( ) == 1)
			{
				UINT nListSelectedItem = GetSelectedItem();
				VERIFY(EditLabel(nListSelectedItem) != NULL);
			}
			else
				CListCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
			break;
		}
		default:
			CListCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
			break;
	}
}

BOOL CListCtrlEx::OnDblClickEx(NMHDR* pNMHDR, LRESULT* pResult)
{
	ASSERT(GetStyle() & LVS_EDITLABELS);

	// Don't do an Edit Label when multiple items are selected
	if (GetSelectedCount( ) == 1)
	{
		UINT nListSelectedItem = GetSelectedItem();
		VERIFY(EditLabel(nListSelectedItem) != NULL);
	}

	return FALSE;
}

BOOL CListCtrlEx::OnBeginLabelEditList(NMHDR* pNMHDR, LRESULT* pResult)
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;

	// This Limits the size of the Tag Name
	GetEditControl()->LimitText(MAX_LABEL_SIZE);

	// Set to TRUE if you don't want to allow the user to change the label
	*pResult = FALSE;
	return FALSE;
}

BOOL CListCtrlEx::OnEndLabelEditList(NMHDR* pNMHDR, LRESULT *  pResult)
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	
	// If label editing wasn't canceled
	if ((pDispInfo->item.pszText != NULL) && (pDispInfo->item.iItem > -1))
	{
		*pResult = TRUE;
		return TRUE;
	}
	else
	{
		*pResult = FALSE;
		return FALSE;
	}
}
