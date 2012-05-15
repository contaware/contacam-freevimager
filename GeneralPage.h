#if !defined(AFX_GENERALPAGE_H__0C566865_8717_497C_BFF4_68AC9FB23F18__INCLUDED_)
#define AFX_GENERALPAGE_H__0C566865_8717_497C_BFF4_68AC9FB23F18__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GeneralPage.h : header file
//

#ifdef VIDEODEVICEDOC

#include "resource.h"
#include "PeakMeterCtrl.h"
#include "StaticLink.h"
#include "NumSpinCtrl.h"
#include "NumEdit.h"

#define WM_PEAKMETER_UPDATE		WM_USER + 200

// Forward Declaration
class CVideoDeviceDoc;

/////////////////////////////////////////////////////////////////////////////
// CGeneralPage dialog

#define GENERALDLG_TIMER_MS					300
#define AUDIOCAPTURE_INCONSISTENCY_TIMEOUT	2	// In GENERALDLG_TIMER_MS units
#define FRAMERATE_CHANGE_TIMEOUT			4	// In GENERALDLG_TIMER_MS units

class CGeneralPage : public CPropertyPage
{
// Construction
public:
	CGeneralPage();
	void SetDoc(CVideoDeviceDoc* pDoc);
	~CGeneralPage();
	void ClearOnceScheduler();
	void SetCheckSchedulerOnce(BOOL bCheck);
	void SetCheckSchedulerDaily(BOOL bCheck);
	CStaticLink m_DirLabel;

// Dialog Data
	//{{AFX_DATA(CGeneralPage)
	enum { IDD = IDD_GENERAL };
	CSliderCtrl	m_VideoRecQuality;
	CComboBox	m_VideoCompressionChoose;
	CSliderCtrl	m_RecVolumeRight;
	CSliderCtrl	m_RecVolumeLeft;
	CNumEdit	m_FrameRate;
	CNumSpinCtrl m_SpinFrameRate;
	CPeakMeterCtrl	m_PeakMeter;
	CTime	m_SchedulerOnceDateStart;
	CTime	m_SchedulerOnceTimeStart;
	CTime	m_SchedulerOnceTimeStop;
	CTime	m_SchedulerOnceDateStop;
	CTime	m_SchedulerDailyTimeStart;
	CTime	m_SchedulerDailyTimeStop;
	int		m_nVideoRecDataRate;
	int		m_nVideoRecKeyframesRate;
	int		m_nVideoRecQualityBitrate;
	BOOL	m_bRecTimeSegmentation;
	BOOL	m_bAutorun;
	int		m_nDeleteRecordingsOlderThanDays;
	int		m_nTimeSegmentationIndex;
	BOOL	m_bRecAutoOpen;
	BOOL	m_bDeinterlace;
	BOOL	m_bRotate180;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGeneralPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void ShowHideCtrls();
	// Generated message map functions
	//{{AFX_MSG(CGeneralPage)
	afx_msg void OnVideoFormat();
	afx_msg void OnVideoSource();
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeFrameRate();
	afx_msg void OnAudioFormat();
	afx_msg void OnRecAudio();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnAudioInput();
	afx_msg void OnSelchangeVideoCompressionChoose();
	afx_msg void OnRecordSaveas();
	afx_msg void OnAudioMixer();
	afx_msg void OnVideoInput();
	afx_msg void OnVideoTuner();
	afx_msg void OnCheckSchedulerOnce();
	afx_msg void OnCheckSchedulerDaily();
	afx_msg void OnChangeEditKeyframesRate();
	afx_msg void OnChangeEditDatarate();
	afx_msg void OnRadioQuality();
	afx_msg void OnRadioBitrate();
	afx_msg void OnCheckTimeSegmentation();
	afx_msg void OnCheckAutorun();
	afx_msg void OnChangeEditDeleteRecordingsDays();
	afx_msg void OnSelchangeTimeSegmentation();
	afx_msg void OnCheckLiveDeinterlace();
	afx_msg void OnCheckLiveRotate180();
	afx_msg void OnCheckAutoopen();
	//}}AFX_MSG
	afx_msg LRESULT OnMixerCtrlChange(WPARAM wParam, LPARAM lParam);
	afx_msg LONG OnPeakMeterUpdate(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

	CVideoDeviceDoc* m_pDoc;
	CDWordArray m_VideoCompressionFcc;
	CDWordArray m_VideoCompressionKeyframesRateSupport;
	CDWordArray m_VideoCompressionDataRateSupport;
	CDWordArray m_VideoCompressionQualitySupport;
	BOOL m_bDoChangeFrameRate;
	int m_nFrameRateChangeTimeout;
	int m_nAudioCaptureInconsistencyTimeout;
	BOOL m_bDlgInitialized;
};

#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GENERALPAGE_H__0C566865_8717_497C_BFF4_68AC9FB23F18__INCLUDED_)
