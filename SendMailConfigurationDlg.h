#if !defined(AFX_SENDMAILCONFIGURATIONDLG_H__A45E1E9C_BFAD_41C4_BE6D_604B5D5F0628__INCLUDED_)
#define AFX_SENDMAILCONFIGURATIONDLG_H__A45E1E9C_BFAD_41C4_BE6D_604B5D5F0628__INCLUDED_

#pragma once

// SendMailConfigurationDlg.h : header file
//

#ifdef VIDEODEVICEDOC

// Email sending configuration structure
enum ConnectionType
{
	PlainText		= 0, 
	SSL_TLS			= 1,
	STARTTLS		= 2
};
typedef struct tagSendMailConfigurationStruct
{
	CString			m_sSubject;
	CString			m_sTo;
	CString			m_sFrom;
	CString			m_sHost;
	CString			m_sFromName;
	int				m_nPort;
	ConnectionType	m_ConnectionType;
	CString			m_sUsername;
	CString			m_sPassword;
} SendMailConfigurationStruct;

/////////////////////////////////////////////////////////////////////////////
// CSendMailConfigurationDlg dialog

class CSendMailConfigurationDlg : public CDialog
{
// Construction
public:
	CSendMailConfigurationDlg(const CString& sName);
	SendMailConfigurationStruct m_SendMailConfiguration;
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
	CString m_sName;
	void CopyToStruct();
	BOOL IsEmail(const CString& sEmail);
	BOOL ValidateEmailsAndHost();
	// Generated message map functions
	//{{AFX_MSG(CSendMailConfigurationDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEditSenderMail();
	virtual void OnOK();
	afx_msg void OnButtonTest();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SENDMAILCONFIGURATIONDLG_H__A45E1E9C_BFAD_41C4_BE6D_604B5D5F0628__INCLUDED_)
