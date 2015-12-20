#if !defined(AFX_SENDMAILCONFIGURATIONDLG_H__A45E1E9C_BFAD_41C4_BE6D_604B5D5F0628__INCLUDED_)
#define AFX_SENDMAILCONFIGURATIONDLG_H__A45E1E9C_BFAD_41C4_BE6D_604B5D5F0628__INCLUDED_

#pragma once

// SendMailConfigurationDlg.h : header file
//

#ifdef VIDEODEVICEDOC

class CVideoDeviceDoc;

/////////////////////////////////////////////////////////////////////////////
// CSendMailConfigurationDlg dialog

class CSendMailConfigurationDlg : public CDialog
{
// Construction
public:
	CSendMailConfigurationDlg(CVideoDeviceDoc* pDoc);
	CVideoDeviceDoc::SendMailConfigurationStruct m_SendMailConfiguration;
// Dialog Data
	//{{AFX_DATA(CSendMailConfigurationDlg)
	enum { IDD = IDD_SENDMAIL_CONFIGURATION };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSendMailConfigurationDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CVideoDeviceDoc* m_pDoc;
	HANDLE m_hMailer;
	CString m_sLogFileName;
	int m_nRetryTimeMs;
	void CopyToStruct();
	// Generated message map functions
	//{{AFX_MSG(CSendMailConfigurationDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnButtonTest();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
};

#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SENDMAILCONFIGURATIONDLG_H__A45E1E9C_BFAD_41C4_BE6D_604B5D5F0628__INCLUDED_)
