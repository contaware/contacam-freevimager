#if !defined(AFX_PALETTEWND_H__71DAEF19_5ED9_4537_9008_92F1F70D45CE__INCLUDED_)
#define AFX_PALETTEWND_H__71DAEF19_5ED9_4537_9008_92F1F70D45CE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PaletteWnd.h : header file
//

#define		WM_COLOR_PICKED			WM_USER + 700
#define		WM_COLOR_PICKER_CLOSED	WM_USER + 701

/////////////////////////////////////////////////////////////////////////////
// CPaletteWnd window

class CPaletteWnd : public CWnd
{
// Construction
public:
	CPaletteWnd();
	CPaletteWnd(CPoint pt, CWnd* pParentWnd, CPalette* pPalette);

// Attributes
public:

// Operations
public:


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPaletteWnd)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
public:
	BOOL Create(CPoint pt, CWnd* pParentWnd, CPalette* pPalette);
	void Close();
	void SetCurrentColor(COLORREF crCurrent);

protected:
	bool			m_bDrag;
	int				m_nIndex;
	CWnd*			m_pParentWnd;
	BOOL			SelectColor(CPoint& pt);
	CRect			m_rc;
	CToolTipCtrl	m_wndToolTip;
	void			CreateToolTips();
	int				m_nColorMapWidth;
	int				m_nColorMapHeight;

	// Generated message map functions
protected:
	//{{AFX_MSG(CPaletteWnd)
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(AFX_PALETTEWND_H__71DAEF19_5ED9_4537_9008_92F1F70D45CE__INCLUDED_)
