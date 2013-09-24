#if !defined(AFX_LICENSEDLG_H__6F2B2BF7_7B72_4FC7_99F4_938AFC9A956E__INCLUDED_)
#define AFX_LICENSEDLG_H__6F2B2BF7_7B72_4FC7_99F4_938AFC9A956E__INCLUDED_

#pragma once

// LicenseDlg.h : header file
//

#include "BkgColEdit.h"

/////////////////////////////////////////////////////////////////////////////
// CLicenseDlg dialog

class CLicenseDlg : public CDialog
{
// Construction
public:
	CLicenseDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CLicenseDlg)
	enum { IDD = IDD_LICENSE };
	CBkgColEdit	m_License;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLicenseDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	CFont m_Font;
	// Generated message map functions
	//{{AFX_MSG(CLicenseDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LICENSEDLG_H__6F2B2BF7_7B72_4FC7_99F4_938AFC9A956E__INCLUDED_)
