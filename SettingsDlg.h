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
	virtual ~CSettingsDlg();

// Dialog Data
	//{{AFX_DATA(CSettingsDlg)
	enum { IDD = IDD_SETTINGS };
	BOOL	m_bCheckBmp;
	BOOL	m_bCheckJpeg;
	BOOL	m_bCheckPcx;
	BOOL	m_bCheckEmf;
	BOOL	m_bCheckPng;
	BOOL	m_bCheckTiff;
	BOOL	m_bCheckGif;
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
	BOOL m_bCleanupCOM;
	void SettingsPageAppsDefaults(const CString& sTarget = _T(""));
	void Apply();
	// Generated message map functions
	//{{AFX_MSG(CSettingsDlg)
	afx_msg void OnButtonClearall();
	afx_msg void OnButtonSetall();
	afx_msg void OnButtonAppsDefaults();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETTINGSDLG_H__0997CB57_6EF4_460E_8A48_98C75661EBB5__INCLUDED_)
