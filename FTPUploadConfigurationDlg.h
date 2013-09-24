#if !defined(AFX_FTPUPLOADCONFIGURATIONDLG_H__05943574_4165_406F_BF7E_8CFA7C4A9F9E__INCLUDED_)
#define AFX_FTPUPLOADCONFIGURATIONDLG_H__05943574_4165_406F_BF7E_8CFA7C4A9F9E__INCLUDED_

#pragma once

// FTPUploadConfigurationDlg.h : header file
//

#ifdef VIDEODEVICEDOC

class CVideoDeviceDoc;

/////////////////////////////////////////////////////////////////////////////
// CFTPUploadConfigurationDlg dialog

class CFTPUploadConfigurationDlg : public CDialog
{
// Construction
public:
	CFTPUploadConfigurationDlg(UINT idd);
	CVideoDeviceDoc::FTPUploadConfigurationStruct m_FTPUploadConfiguration;
// Dialog Data
	//{{AFX_DATA(CFTPUploadConfigurationDlg)
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFTPUploadConfigurationDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void CopyToStruct();
	// Generated message map functions
	//{{AFX_MSG(CFTPUploadConfigurationDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnButtonTest();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FTPUPLOADCONFIGURATIONDLG_H__05943574_4165_406F_BF7E_8CFA7C4A9F9E__INCLUDED_)
