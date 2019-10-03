#if !defined(AFX_FTPUPLOADDLG_H__05943574_4165_406F_BF7E_8CFA7C4A9F9E__INCLUDED_)
#define AFX_FTPUPLOADDLG_H__05943574_4165_406F_BF7E_8CFA7C4A9F9E__INCLUDED_

#pragma once

// FTPUploadDlg.h : header file
//

#ifdef VIDEODEVICEDOC

/////////////////////////////////////////////////////////////////////////////
// CFTPUploadDlg dialog

class CFTPUploadDlg : public CDialog
{
public:
	CFTPUploadDlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~CFTPUploadDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FTP_UPLOAD };
#endif

	CString m_sHost;
	CString m_sUsername;
	CString m_sPassword;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()	
};

#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FTPUPLOADDLG_H__05943574_4165_406F_BF7E_8CFA7C4A9F9E__INCLUDED_)
