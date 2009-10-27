#if !defined(AFX_YESNOALLCANCELDLG_H__B2EAD44B_F666_4D5E_BD55_FBCC831A1F9F__INCLUDED_)
#define AFX_YESNOALLCANCELDLG_H__B2EAD44B_F666_4D5E_BD55_FBCC831A1F9F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// YesNoAllCancelDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CYesNoAllCancelDlg dialog


class CYesNoAllCancelDlg : public CDialog
{
public:
	enum {NOT_DEFINED, REPLACE_FILE, REPLACEALL_FILES, RENAME_FILE, RENAMEALL_FILES};
	CYesNoAllCancelDlg(CWnd* pParent = NULL);   // standard constructor
	static CString MakeRenamedFileName(CString sFileName);

public:
	int m_nOperation;
	CString m_sFileName;

// Dialog Data
	//{{AFX_DATA(CYesNoAllCancelDlg)
	enum { IDD = IDD_YESNOALLCANCEL };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CYesNoAllCancelDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CYesNoAllCancelDlg)
	afx_msg void OnButtonRename();
	afx_msg void OnButtonRenameall();
	afx_msg void OnButtonReplaceall();
	virtual void OnCancel();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_YESNOALLCANCELDLG_H__B2EAD44B_F666_4D5E_BD55_FBCC831A1F9F__INCLUDED_)
