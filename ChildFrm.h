// ChildFrm.h : interface of the CChildFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHILDFRM_H__E852AF35_48F6_4D85_A507_3A77724D2AE5__INCLUDED_)
#define AFX_CHILDFRM_H__E852AF35_48F6_4D85_A507_3A77724D2AE5__INCLUDED_

#pragma once

class CChildFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CChildFrame)
public:
	CChildFrame();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChildFrame)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CChildFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	BOOL m_bFirstCloseAttempt;
	DWORD m_dwShutdownStartUpTime;

	//{{AFX_MSG(CChildFrame)
	//}}AFX_MSG
	LRESULT OnSetMessageString(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHILDFRM_H__E852AF35_48F6_4D85_A507_3A77724D2AE5__INCLUDED_)
