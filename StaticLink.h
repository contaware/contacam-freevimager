#ifndef STATICLINK_H
#define STATICLINK_H

class CStaticLink : public CStatic
{
	public:
		CStaticLink();
		virtual ~CStaticLink();
		void SetLink(const CString& sLink) {m_sLink = sLink;};
		void SetUnvisitedColor(COLORREF crUnvisited) {m_crUnvisited = crUnvisited;};
		void SetVisitedColor(COLORREF crVisited) {m_crVisited = crVisited;};

	protected:
		DECLARE_DYNAMIC(CStaticLink)
		CFont m_Font;
		CString	 m_sLink;
		COLORREF m_crUnvisited;
		COLORREF m_crVisited; 
		BOOL	 m_bVisited;
		
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CStaticLink)
		//}}AFX_VIRTUAL

	protected:
		// Generated message map functions
		//{{AFX_MSG(CStaticLink)
		afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
		afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
		//}}AFX_MSG
		afx_msg void OnClicked();

		DECLARE_MESSAGE_MAP()
};


#endif // !defined(STATICLINK_H)
