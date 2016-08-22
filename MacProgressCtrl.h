#if !defined(AFX_COLORPROGRESSCTRL_H__603BBF44_B19C_11D3_90FA_0020AFBC499D__INCLUDED_)
#define AFX_COLORPROGRESSCTRL_H__603BBF44_B19C_11D3_90FA_0020AFBC499D__INCLUDED_

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CColorProgressCtrl window

class CColorProgressCtrl : public CProgressCtrl
{
// Construction
public:
	CColorProgressCtrl();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColorProgressCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	COLORREF GetColor() const;
	void SetColor(COLORREF crColor);
	virtual ~CColorProgressCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CColorProgressCtrl)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
private:
	BOOL IsThemed();
	void DeletePenBorder();
	void CreatePenBorder();
	CPen m_PenBorder;
	void CalcColorBorder();
	COLORREF m_crColor;
	COLORREF m_crColorBorder;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLORPROGRESSCTRL_H__603BBF44_B19C_11D3_90FA_0020AFBC499D__INCLUDED_)
