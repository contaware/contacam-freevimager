#if !defined(AFX_DRAGDROPLISTCTRL_H__17F6D56D_591C_4319_B0EB_9D6D9B45BC79__INCLUDED_)
#define AFX_DRAGDROPLISTCTRL_H__17F6D56D_591C_4319_B0EB_9D6D9B45BC79__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DragDropListCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDragDropListCtrl window

class CDragDropListCtrl : public CListCtrl
{
protected:
	CDWordArray			m_anDragIndexes;
	int					m_nDropIndex;
	CImageList*			m_pDragImage;
	int					m_nPrevDropIndex;
	UINT				m_uPrevDropState;
	DWORD				m_dwStyle;
	BOOL				m_bUseResDragImage;
	UINT				m_uiSingleSelectImageRes;
	UINT				m_uiMultiSelectImageRes;

	enum EScrollDirection
	{
		scrollNull,
		scrollUp,
		scrollDown
	};
	EScrollDirection	m_ScrollDirection;
	UINT				m_uiScrollTimer;
	UINT				m_uiScrollTime;

// Construction
public:
	CDragDropListCtrl();
	virtual ~CDragDropListCtrl();
	BOOL IsDragging() const {return m_pDragImage != NULL;};
	int GetDropIndex() const {return m_nDropIndex;};
	void SetUseResDragImage(BOOL bUseResDragImage)
				{m_bUseResDragImage = bUseResDragImage;};
	void SetSingleSelectImageRes(UINT uiSingleSelectImageRes)
				{m_uiSingleSelectImageRes = uiSingleSelectImageRes;};
	void SetMultiSelectImageRes(UINT uiMultiSelectImageRes)
				{m_uiMultiSelectImageRes = uiMultiSelectImageRes;};

// Operations
public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDragDropListCtrl)
	//}}AFX_VIRTUAL

protected:
	void DropItem();
	void UpdateSelection(int nDropIndex);
	void RestorePrevDropItemState();
	void SetScrollTimer(EScrollDirection ScrollDirection,
						UINT uiScrollTime);
	void KillScrollTimer();
	CImageList* CreateDragImageEx(LPPOINT lpPoint);
	CImageList* CreateResDragImageEx(LPPOINT lpPoint);
	
// Generated message map functions
protected:
	//{{AFX_MSG(CDragDropListCtrl)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DRAGDROPLISTCTRL_H__17F6D56D_591C_4319_B0EB_9D6D9B45BC79__INCLUDED_)
