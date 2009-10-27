#if !defined(AFX_MDICLIENTWND_H__9A6BEBCA_829C_4085_8A12_19A15A12F62C__INCLUDED_)
#define AFX_MDICLIENTWND_H__9A6BEBCA_829C_4085_8A12_19A15A12F62C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMDIClientWnd : public CWnd
{
	DECLARE_DYNAMIC(CMDIClientWnd)

public:
	CMDIClientWnd();
	virtual ~CMDIClientWnd();

	// Generated message map functions
protected:
	BOOL m_bFontCreated;
	int m_nFontSize;
	CFont m_Font;
	CFont m_FontDesc;
	CString m_sFontFace;
	COLORREF m_crFontColor;
	//{{AFX_MSG(CMDIClientWnd)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MDICLIENTWND_H__9A6BEBCA_829C_4085_8A12_19A15A12F62C__INCLUDED_)


