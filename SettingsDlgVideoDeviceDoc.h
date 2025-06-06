#if !defined(AFX_SETTINGSDLGVIDEODEVICEDOC_H__0997CB57_6EF4_460E_8A48_98C75661EBB5__INCLUDED_)
#define AFX_SETTINGSDLGVIDEODEVICEDOC_H__0997CB57_6EF4_460E_8A48_98C75661EBB5__INCLUDED_

#pragma once

#include "WorkerThread.h"
#include "GetDirContentSize.h"

// SettingsDlgVideoDeviceDoc.h : header file
//

#ifdef VIDEODEVICEDOC

//  Move Directory Thread
class CMoveDirThread : public CWorkerThread
{
	public:
		CMoveDirThread(){;};
		virtual ~CMoveDirThread(){Kill();};
		__forceinline int GetMoveDirFilesCount() const {return m_nMovedFilesCount;};
		__forceinline DWORD GetMoveDirLastError() const {return m_dwMoveDirLastError;};
		CString m_sFromDir;
		CString m_sToDir;

	protected:
		volatile int m_nMovedFilesCount;
		volatile DWORD m_dwMoveDirLastError;
		int Work()
		{
			m_nMovedFilesCount = 0;
			m_dwMoveDirLastError = 0;
			if (!::MergeDirContent(m_sFromDir, m_sToDir, TRUE, TRUE, (int*)&m_nMovedFilesCount)) // overwrite existing
				m_dwMoveDirLastError = ::GetLastError();
			else
			{
				// MergeDirContent() leaves empty folders but should not leave files
				int nFilesCount = 0;
				::GetDirContentSize(m_sFromDir, &nFilesCount);
				if (nFilesCount == 0)
					::DeleteDir(m_sFromDir);
			}
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
	BOOL	m_bAutostart;
	BOOL	m_bTrayIcon;
	BOOL	m_bStartFromService;
	BOOL	m_bTopMost;
	int		m_nFirstStartDelay;
	CString m_sMicroApacheDocRoot;
	BOOL	m_bStartMicroApache;
	int		m_nMicroApachePort;
	int		m_nMicroApachePortSSL;
	CString	m_sMicroApacheUsername;
	CString	m_sMicroApachePassword;
	CString	m_sMicroApacheCertFileSSL;
	CString	m_sMicroApacheKeyFileSSL;
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
	CMoveDirThread m_MoveDirThread;
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
	afx_msg void OnSyslinkHttpdConfEditable(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETTINGSDLGVIDEODEVICEDOC_H__0997CB57_6EF4_460E_8A48_98C75661EBB5__INCLUDED_)
