#if !defined(AFX_SETTINGSDLGVIDEODEVICEDOC_H__0997CB57_6EF4_460E_8A48_98C75661EBB5__INCLUDED_)
#define AFX_SETTINGSDLGVIDEODEVICEDOC_H__0997CB57_6EF4_460E_8A48_98C75661EBB5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SettingsDlgVideoDeviceDoc.h : header file
//

#ifdef VIDEODEVICEDOC

#include "StaticLink.h"

/////////////////////////////////////////////////////////////////////////////
// CSettingsDlgVideoDeviceDoc dialog

class CSettingsDlgVideoDeviceDoc : public CDialog
{
// Construction
public:
	CSettingsDlgVideoDeviceDoc(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSettingsDlgVideoDeviceDoc)
	enum { IDD = IDD_SETTINGS_VIDEODEVICEDOC };
	BOOL	m_bCheckAvi;
	BOOL	m_bCheckBmp;
	BOOL	m_bCheckJpeg;
	BOOL	m_bCheckPcx;
	BOOL	m_bCheckEmf;
	BOOL	m_bCheckPng;
	BOOL	m_bCheckTiff;
	BOOL	m_bCheckGif;
	BOOL	m_bCheckZip;
	BOOL	m_bCheckAif;
	BOOL	m_bCheckAu;
	BOOL	m_bCheckMidi;
	BOOL	m_bCheckMp3;
	BOOL	m_bCheckWav;
	BOOL	m_bCheckWma;
	BOOL	m_bCheckCda;
	BOOL	m_bEscExit;
	BOOL	m_bTrayIcon;
	BOOL	m_bAutostart;
	BOOL	m_bDisableExtProg;
	BOOL	m_bTopMost;
	BOOL	m_bStartMicroApache;
	CString	m_sMicroApacheDocRoot;
	int		m_nMicroApachePort;
	CString	m_sMicroApachePassword;
	CString	m_sMicroApacheUsername;
	BOOL	m_bFullscreenBrowser;
	BOOL	m_bBrowserAutostart;
	CString	m_sFullscreenBrowserExitString;
	//}}AFX_DATA

	CStaticLink m_MicroApacheDocRootLabel;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSettingsDlgVideoDeviceDoc)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CSettingsDlgVideoDeviceDoc)
	afx_msg void OnButtonClearall();
	afx_msg void OnButtonSetall();
	virtual void OnOK();
	afx_msg void OnWebserverRootdir();
	virtual BOOL OnInitDialog();
	afx_msg void OnCheckFullscreenbrowser();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETTINGSDLGVIDEODEVICEDOC_H__0997CB57_6EF4_460E_8A48_98C75661EBB5__INCLUDED_)
