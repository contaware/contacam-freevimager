#if !defined(AFX_DELETEPAGEDLG_H__9D401485_F03A_4890_BDD7_20A7FFE8AFD9__INCLUDED_)
#define AFX_DELETEPAGEDLG_H__9D401485_F03A_4890_BDD7_20A7FFE8AFD9__INCLUDED_

#pragma once

// DeletePageDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDeletePageDlg dialog

class CDeletePageDlg : public CDialog
{
// Construction
public:
	CDeletePageDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDeletePageDlg)
	enum { IDD = IDD_DELETEPAGE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDeletePageDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDeletePageDlg)
	afx_msg void OnNo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DELETEPAGEDLG_H__9D401485_F03A_4890_BDD7_20A7FFE8AFD9__INCLUDED_)
