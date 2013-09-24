#if !defined(AFX_IMAPI2DOWNLOADDLG_H__5FDB6180_6EF4_4533_B2B2_D77511ADDEA1__INCLUDED_)
#define AFX_IMAPI2DOWNLOADDLG_H__5FDB6180_6EF4_4533_B2B2_D77511ADDEA1__INCLUDED_

#pragma once

// IMAPI2DownloadDlg.h : header file
//

#include "Staticlink.h"

/////////////////////////////////////////////////////////////////////////////
// CIMAPI2DownloadDlg dialog

class CIMAPI2DownloadDlg : public CDialog
{
// Construction
public:
	CIMAPI2DownloadDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CIMAPI2DownloadDlg)
	enum { IDD = IDD_IMAPI2_DOWNLOAD };
	CString	m_sTextRow1;
	CString	m_sTextRow2;
	//}}AFX_DATA
	CString	m_sTextLink;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIMAPI2DownloadDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CStaticLink m_TextLink;
	// Generated message map functions
	//{{AFX_MSG(CIMAPI2DownloadDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMAPI2DOWNLOADDLG_H__5FDB6180_6EF4_4533_B2B2_D77511ADDEA1__INCLUDED_)
