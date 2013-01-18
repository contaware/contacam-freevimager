#if !defined(AFX_ASSISTANTDLG_H__6E88B0A4_B936_40BD_B351_397CEA44BF51__INCLUDED_)
#define AFX_ASSISTANTDLG_H__6E88B0A4_B936_40BD_B351_397CEA44BF51__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AssistantDlg.h : header file
//

#ifdef VIDEODEVICEDOC

// Forward Declaration
class CVideoDeviceDoc;

/////////////////////////////////////////////////////////////////////////////
// CAssistantDlg dialog

#define ASSISTANTDLG_TIMER_MS		500

class CAssistantDlg : public CDialog
{
// Construction
public:
	CAssistantDlg(CVideoDeviceDoc* pDoc, CWnd* pParent = NULL);
	~CAssistantDlg();

// Dialog Data
	//{{AFX_DATA(CAssistantDlg)
	enum { IDD = IDD_ASSISTANT };
	BOOL	m_bCheck24hRec;
	int		m_nComboKeepFor;
	CString	m_sName;
	int		m_nUsage;
	int		m_nComboSnapshotRate;
	int		m_nComboSnapshotHistoryRate;
	int		m_nComboSnapshotHistorySize;
	BOOL	m_bCheckFullStretch;
	BOOL	m_bCheckPrintCommand;
	BOOL	m_bCheckSaveCommand;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CAssistantDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CVideoDeviceDoc* m_pDoc;
	CString	m_sParentDir;
	BOOL m_bDoApplySettings;
	int m_nRetryTimeMs;
	void EnableDisableCtrls();
	void EnableDisableAllCtrls(BOOL bEnable);
	void Rename();
	void EnableDisable24hRec(BOOL bEnable);
	BOOL Is24hRec();
	void ApplySettingsUpdate(int nThumbWidth, int nThumbHeight, const CString& sSnapShotRate);
	void ApplySettings();
	// Generated message map functions
	//{{AFX_MSG(CAssistantDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnRadioMovdet();
	afx_msg void OnRadioSnapshothistory();
	afx_msg void OnRadioSnapshot();
	afx_msg void OnRadioManual();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnRadioNochange();
	virtual void OnOK();
	afx_msg void OnButtonParentDir();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ASSISTANTDLG_H__6E88B0A4_B936_40BD_B351_397CEA44BF51__INCLUDED_)
