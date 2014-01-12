#if !defined(AFX_SETTINGSDLGVIDEODEVICEDOC_H__0997CB57_6EF4_460E_8A48_98C75661EBB5__INCLUDED_)
#define AFX_SETTINGSDLGVIDEODEVICEDOC_H__0997CB57_6EF4_460E_8A48_98C75661EBB5__INCLUDED_

#pragma once

// SettingsDlgVideoDeviceDoc.h : header file
//

#ifdef VIDEODEVICEDOC

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
	BOOL	m_bStartFullScreenMode;
	BOOL	m_bEscExit;
	BOOL	m_bTrayIcon;
	BOOL	m_bAutostart;
	BOOL	m_bDisableExtProg;
	BOOL	m_bTopMost;
	BOOL	m_bStartMicroApache;
	int		m_nMicroApachePort;
	CString	m_sMicroApacheAreaname;
	CString	m_sMicroApacheUsername;
	CString	m_sMicroApachePassword;
	BOOL	m_bFullscreenBrowser;
	BOOL	m_bBrowserAutostart;
	CString	m_sFullscreenBrowserExitString;
	BOOL	m_bStartFromService;
	BOOL	m_bMicroApacheDigestAuth;
	BOOL	m_bIPv6;
	int		m_nAutostartDelay;
	BOOL	m_bUseCustomTempFolder;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSettingsDlgVideoDeviceDoc)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CString m_sLastValidApacheUsername;
	BOOL m_bRejectingApacheUsernameChange;
	// Generated message map functions
	//{{AFX_MSG(CSettingsDlgVideoDeviceDoc)
	afx_msg void OnButtonClearall();
	afx_msg void OnButtonSetall();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnCheckFullscreenbrowser();
	afx_msg void OnUpdateAuthUsername();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETTINGSDLGVIDEODEVICEDOC_H__0997CB57_6EF4_460E_8A48_98C75661EBB5__INCLUDED_)
