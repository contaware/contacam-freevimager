#if !defined(AFX_WINSOCK2MISSINGDLG_H__1FF4D5CF_BE89_43DB_8CF6_3F8D2725DD4B__INCLUDED_)
#define AFX_WINSOCK2MISSINGDLG_H__1FF4D5CF_BE89_43DB_8CF6_3F8D2725DD4B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Winsock2MissingDlg.h : header file
//

#include "Staticlink.h"

/////////////////////////////////////////////////////////////////////////////
// CWinsock2MissingDlg dialog

class CWinsock2MissingDlg : public CDialog
{
// Construction
public:
	CWinsock2MissingDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CWinsock2MissingDlg)
	enum { IDD = IDD_WINSOCK2_MISSING };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWinsock2MissingDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CStaticLink m_TextLink;
	// Generated message map functions
	//{{AFX_MSG(CWinsock2MissingDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WINSOCK2MISSINGDLG_H__1FF4D5CF_BE89_43DB_8CF6_3F8D2725DD4B__INCLUDED_)
