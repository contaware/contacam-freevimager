#if !defined(AFX_SETTINGSDLG_H__0997CB57_6EF4_460E_8A48_98C75661EBB5__INCLUDED_)
#define AFX_SETTINGSDLG_H__0997CB57_6EF4_460E_8A48_98C75661EBB5__INCLUDED_

#pragma once

// SettingsDlg.h : header file
//

#ifndef VIDEODEVICEDOC

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
	BOOL	m_bSingleInstance;
	BOOL	m_bTrayIcon;
	BOOL	m_bAutostart;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSettingsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void FileAssociation(BOOL bDoAssociation);
	// Generated message map functions
	//{{AFX_MSG(CSettingsDlg)
	afx_msg void OnBnClickedCheckStartwithWindows();
	afx_msg void OnBnClickedCheckTrayicon();
	afx_msg void OnBnClickedCheckSingleinstance();
	afx_msg void OnBnClickedButtonAssociate();
	afx_msg void OnBnClickedButtonUnassociate();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETTINGSDLG_H__0997CB57_6EF4_460E_8A48_98C75661EBB5__INCLUDED_)
