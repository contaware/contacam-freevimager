#if !defined(AFX_SETTINGSDLGVIDEODEVICEDOC_H__0997CB57_6EF4_460E_8A48_98C75661EBB5__INCLUDED_)
#define AFX_SETTINGSDLGVIDEODEVICEDOC_H__0997CB57_6EF4_460E_8A48_98C75661EBB5__INCLUDED_

#pragma once

#include "WorkerThread.h"

// SettingsDlgVideoDeviceDoc.h : header file
//

#ifdef VIDEODEVICEDOC

//  Merge Directory Thread
class CMergeDirThread : public CWorkerThread
{
	public:
		CMergeDirThread(){;};
		virtual ~CMergeDirThread(){Kill();};
		__forceinline int GetMergeDirFilesCount() const {return m_nMergedFilesCount;};
		__forceinline DWORD GetMergeDirLastError() const {return m_dwMergeDirLastError;};
		CString m_sFromDir;
		CString m_sToDir;

	protected:
		volatile int m_nMergedFilesCount;
		volatile DWORD m_dwMergeDirLastError;
		int Work()
		{
			m_nMergedFilesCount = 0;
			m_dwMergeDirLastError = 0;
			if (!::MergeDirContent(m_sFromDir, m_sToDir, TRUE, TRUE, (int*)&m_nMergedFilesCount)) // overwrite existing
				m_dwMergeDirLastError = ::GetLastError();
			else
				::DeleteDir(m_sFromDir); // no error message on failure
			return 0;
		};
};

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
	BOOL	m_bTrayIcon;
	BOOL	m_bAutostart;
	BOOL	m_bTopMost;
	BOOL	m_bStartMicroApache;
	int		m_nMicroApachePort;
	int		m_nMicroApachePortSSL;
	CString	m_sMicroApacheUsername;
	CString	m_sMicroApachePassword;
	CString	m_sMicroApacheCertFileSSL;
	CString	m_sMicroApacheKeyFileSSL;
	CString m_sMicroApacheDocRoot;
	BOOL	m_bBrowserAutostart;
	BOOL	m_bStartFromService;
	int		m_nFirstStartDelay;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSettingsDlgVideoDeviceDoc)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CString m_sMicroApacheDocRootOld;
	int m_nMicroApacheDocRootOldFilesCount;
	BOOL m_bDoApplySettings;
	CMergeDirThread m_MergeDirThread;
	void ApplySettingsInit();
	void ApplySettingsEnd();
	void EnableDisableAllCtrls(BOOL bEnable);
	// Generated message map functions
	//{{AFX_MSG(CSettingsDlgVideoDeviceDoc)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnButtonDocRoot();
	afx_msg void OnButtonCertSsl();
	afx_msg void OnButtonKeySsl();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETTINGSDLGVIDEODEVICEDOC_H__0997CB57_6EF4_460E_8A48_98C75661EBB5__INCLUDED_)
