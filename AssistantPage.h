#if !defined(AFX_ASSISTANTPAGE_H__6E88B0A4_B936_40BD_B351_397CEA44BF51__INCLUDED_)
#define AFX_ASSISTANTPAGE_H__6E88B0A4_B936_40BD_B351_397CEA44BF51__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AssistantPage.h : header file
//

#ifdef VIDEODEVICEDOC

// Forward Declaration
class CVideoDeviceDoc;

#include "StaticLink.h"
#include "XButtonXP.h"

/////////////////////////////////////////////////////////////////////////////
// CAssistantPage dialog

#define ASSISTANTDLG_TIMER_MS		500

class CAssistantPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CAssistantPage)

// Construction
public:
	CAssistantPage();
	void SetDoc(CVideoDeviceDoc* pDoc);
	~CAssistantPage();

// Dialog Data
	//{{AFX_DATA(CAssistantPage)
	enum { IDD = IDD_ASSISTANT };
	CXButtonXP m_ButtonApplySettings;
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
	//{{AFX_VIRTUAL(CAssistantPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CVideoDeviceDoc* m_pDoc;
	BOOL m_bDoApplySettings;
	void EnableDisableCtrls();
	void EnableDisableAllCtrls(BOOL bEnable);
	void Rename();
	void EnableDisable24hRec(BOOL bEnable);
	BOOL Is24hRec();
	void ApplySettingsUpdate(int nThumbWidth, int nThumbHeight, const CString& sSnapShotRate);
	void ApplySettings();
	// Generated message map functions
	//{{AFX_MSG(CAssistantPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnButtonApplySettings();
	afx_msg void OnRadioMovdet();
	afx_msg void OnRadioSnapshothistory();
	afx_msg void OnRadioSnapshot();
	afx_msg void OnRadioManual();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnRadioNochange();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ASSISTANTPAGE_H__6E88B0A4_B936_40BD_B351_397CEA44BF51__INCLUDED_)
