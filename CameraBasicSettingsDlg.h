#if !defined(AFX_CAMERABASICSETTINGSDLG_H__6E88B0A4_B936_40BD_B351_397CEA44BF51__INCLUDED_)
#define AFX_CAMERABASICSETTINGSDLG_H__6E88B0A4_B936_40BD_B351_397CEA44BF51__INCLUDED_

#pragma once

// CameraBasicSettingsDlg.h : header file
//

#include "SendMailConfigurationDlg.h"

#ifdef VIDEODEVICEDOC

// Forward Declaration
class CVideoDeviceDoc;

/////////////////////////////////////////////////////////////////////////////
// CCameraBasicSettingsDlg dialog

class CCameraBasicSettingsDlg : public CDialog
{
// Construction
public:
	CCameraBasicSettingsDlg(CVideoDeviceDoc* pDoc, CWnd* pParent = NULL);
	~CCameraBasicSettingsDlg();

// Dialog Data
	//{{AFX_DATA(CCameraBasicSettingsDlg)
	enum { IDD = IDD_CAMERABASICSETTINGS };
	int		m_nComboKeepFor;
	CString	m_sName;
	CString m_sMaxCameraFolderSizeGB;
	CString m_sMinDiskFreePercent;
	int		m_nUsage;
	int		m_nComboSnapshotRate;
	int		m_nComboSnapshotHistoryRate;
	int		m_nComboSnapshotHistorySize;
	BOOL	m_bCheckFullStretch;
	BOOL	m_bCheckTrashCommand;
	BOOL	m_bCheckCameraCommands;
	BOOL	m_bCheckSendMailMalfunction;
	BOOL	m_bCheckSendMailACLineMalfunction;
	BOOL	m_bCheckSendMailDeviceOK;
	BOOL	m_bCheckSendMailMovementDetection;
	int		m_nComboSendMailMovementDetectionAttachment;
	int		m_nSendMailSecBetweenMsg;
	CSpinButtonCtrl	m_SpinSendMailSecBetweenMsg;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CCameraBasicSettingsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CVideoDeviceDoc* m_pDoc;
	SendMailConfigurationStruct m_CurrentSendMailConfiguration;
	BOOL m_bDoApplySettings;
	int m_nRetryTimeMs;
	void EnableDisableCtrls();
	void EnableDisableAllCtrls(BOOL bEnable);
	void Rename();
	void ApplySettingsSnapshot(int nThumbWidth, int nThumbHeight, double dSnapshotRate);
	void ApplySettings();
	// Generated message map functions
	//{{AFX_MSG(CCameraBasicSettingsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnRadioMovdet();
	afx_msg void OnRadioSnapshothistory();
	afx_msg void OnRadioSnapshot();
	afx_msg void OnRadioManual();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnSendmailConfigure();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CAMERABASICSETTINGSDLG_H__6E88B0A4_B936_40BD_B351_397CEA44BF51__INCLUDED_)
