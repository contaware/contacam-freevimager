#if !defined(AFX_AUTHENTICATIONDLG_H__43A61683_0E19_45B4_8256_84ABBF44A947__INCLUDED_)
#define AFX_AUTHENTICATIONDLG_H__43A61683_0E19_45B4_8256_84ABBF44A947__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AuthenticationDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAuthenticationDlg dialog

class CAuthenticationDlg : public CDialog
{
// Construction
public:
	CAuthenticationDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAuthenticationDlg)
	enum { IDD = IDD_HTTP_AUTHENTICATION };
	BOOL	m_bSaveAuthenticationData;
	CString	m_sPassword;
	CString	m_sUsername;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAuthenticationDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAuthenticationDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AUTHENTICATIONDLG_H__43A61683_0E19_45B4_8256_84ABBF44A947__INCLUDED_)
