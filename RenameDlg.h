#if !defined(AFX_RENAMEDLG_H__7B107DC0_4568_4CAC_B9F2_6F7E790C9CD3__INCLUDED_)
#define AFX_RENAMEDLG_H__7B107DC0_4568_4CAC_B9F2_6F7E790C9CD3__INCLUDED_

#pragma once

// RenameDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRenameDlg dialog

class CRenameDlg : public CDialog
{
// Construction
public:
	CRenameDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CRenameDlg)
	enum { IDD = IDD_RENAME };
	CString	m_sFileName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRenameDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRenameDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RENAMEDLG_H__7B107DC0_4568_4CAC_B9F2_6F7E790C9CD3__INCLUDED_)
