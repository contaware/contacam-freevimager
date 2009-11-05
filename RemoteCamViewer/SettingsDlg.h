#if !defined(AFX_SETTINGSDLG_H__2E2C22E0_0CAC_4B58_9E35_8E0043A0B663__INCLUDED_)
#define AFX_SETTINGSDLG_H__2E2C22E0_0CAC_4B58_9E35_8E0043A0B663__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SettingsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSettingsDlg dialog

class CSettingsDlg : public CDialog
{
// Construction
public:
	CSettingsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSettingsDlg)
	enum { IDD = IDD_SETTINGS };
	CString	m_sTitle;
	CString	m_sLabel0;
	CString	m_sLabel1;
	CString	m_sLabel2;
	CString	m_sLabel3;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSettingsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSettingsDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETTINGSDLG_H__2E2C22E0_0CAC_4B58_9E35_8E0043A0B663__INCLUDED_)
