#if !defined(AFX_REGISTRATIONDLG_H__C374DC3B_6818_43D8_BDC5_608CD4DE4658__INCLUDED_)
#define AFX_REGISTRATIONDLG_H__C374DC3B_6818_43D8_BDC5_608CD4DE4658__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RegistrationDlg.h : header file
//

#ifdef VIDEODEVICEDOC

/////////////////////////////////////////////////////////////////////////////
// CRegistrationDlg dialog

class CRegistrationDlg : public CDialog
{
// Construction
public:
	CRegistrationDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CRegistrationDlg)
	enum { IDD = IDD_REGISTRATION };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRegistrationDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void UpdateRegCtrls();
	// Generated message map functions
	//{{AFX_MSG(CRegistrationDlg)
	afx_msg void OnButtonBuyLicense();
	afx_msg void OnButtonOpenKeyfile();
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REGISTRATIONDLG_H__C374DC3B_6818_43D8_BDC5_608CD4DE4658__INCLUDED_)
