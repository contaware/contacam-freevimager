#if !defined(AFX_MOVEMENTDETECTIONPAGE_H__AAD75600_3201_49D1_A5D4_3AE518917430__INCLUDED_)
#define AFX_MOVEMENTDETECTIONPAGE_H__AAD75600_3201_49D1_A5D4_3AE518917430__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MovementDetectionPage.h : header file
//

#ifdef VIDEODEVICEDOC

#include "StaticLink.h"

// Forward Declaration
class CVideoDeviceDoc;

/////////////////////////////////////////////////////////////////////////////
// CMovementDetectionPage dialog

class CMovementDetectionPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CMovementDetectionPage)

// Construction
public:
	CMovementDetectionPage();
	void SetDoc(CVideoDeviceDoc* pDoc);
	~CMovementDetectionPage();
	CStaticLink m_DirLabel;

// Dialog Data
	//{{AFX_DATA(CMovementDetectionPage)
	enum { IDD = IDD_MOVDET };
	int	m_nSecondsAfterMovementEnd;
	int	m_nSecondsBeforeMovementBegin;
	CSpinButtonCtrl	m_SpinSecondsBeforeMovementBegin;
	CSpinButtonCtrl	m_SpinSecondsAfterMovementEnd;
	CSliderCtrl	m_DetectionLevel;
	CTime	m_DetectionStartTime;
	CTime	m_DetectionStopTime;
	int		m_nDeleteDetectionsOlderThanDays;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CMovementDetectionPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CVideoDeviceDoc* m_pDoc;
	void UpdateDetectionStartStopTimes();

	// Generated message map functions
	//{{AFX_MSG(CMovementDetectionPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnDetectionSaveas();
	afx_msg void OnCheckVideoDetectionMovement();
	afx_msg void OnCheckDetPreview();
	afx_msg void OnChangeSecondsAfterMovementEnd();
	afx_msg void OnChangeSecondsBeforeMovementBegin();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnReleasedcaptureDetectionLevel(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSaveAviMovementDetection();
	afx_msg void OnAnimatedgifSize();
	afx_msg void OnSaveAnimGifMovementDetection();
	afx_msg void OnSendmailConfigure();
	afx_msg void OnSendmailMovementDetection();
	afx_msg void OnFtpMovementDetection();
	afx_msg void OnFtpConfigure();
	afx_msg void OnAviConfigure();
	afx_msg void OnCheckSchedulerDaily();
	afx_msg void OnDatetimechangeTimeDailyStart(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDatetimechangeTimeDailyStop(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSaveSwfMovementDetection();
	afx_msg void OnCheckAdjacentZonesDet();
	afx_msg void OnChangeEditDeleteDetectionsDays();
	afx_msg void OnSwfConfigure();
	afx_msg void OnCheckFalseDet();
	afx_msg void OnChangeEditFalseDetBlue();
	afx_msg void OnChangeEditFalseDetNoneblue();
	afx_msg void OnSelendokComboFalseDetAndor();
	afx_msg void OnExecMovementDetection();
	afx_msg void OnChangeEditExe();
	afx_msg void OnChangeEditParams();
	afx_msg void OnCheckHideExecCommand();
	afx_msg void OnCheckWaitExecCommand();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MOVEMENTDETECTIONPAGE_H__AAD75600_3201_49D1_A5D4_3AE518917430__INCLUDED_)
