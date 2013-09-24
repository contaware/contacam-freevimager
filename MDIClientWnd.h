#if !defined(AFX_MDICLIENTWND_H__9A6BEBCA_829C_4085_8A12_19A15A12F62C__INCLUDED_)
#define AFX_MDICLIENTWND_H__9A6BEBCA_829C_4085_8A12_19A15A12F62C__INCLUDED_

#pragma once

// Forward Declaration
class CMyMemDC;

class CMDIClientWnd : public CWnd
{
	DECLARE_DYNAMIC(CMDIClientWnd)

public:
	CMDIClientWnd();
	virtual ~CMDIClientWnd();
#ifdef VIDEODEVICEDOC
	void ViewWeb(CString sHost);
#endif

	// Generated message map functions
protected:
	BOOL m_bFontCreated;
	int m_nFontSize;
	CFont m_Font;
	CFont m_FontUnderline;
	CString m_sFontFace;
	COLORREF m_crFontColor;
	COLORREF m_crLinkColor;
	int m_nLeftMargin;
	int m_nTopMargin;
	CRect m_rcLinkComputer;
	CRect m_rcLinkLocalhost;
	void DrawT(CMyMemDC& memDC, CString s, CRect rcDraw);
	CRect DrawTAndCalcRect(CMyMemDC& memDC, CString s, CRect rcDraw);
	//{{AFX_MSG(CMDIClientWnd)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MDICLIENTWND_H__9A6BEBCA_829C_4085_8A12_19A15A12F62C__INCLUDED_)


