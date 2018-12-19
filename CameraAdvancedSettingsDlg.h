#if !defined(AFX_CAMERAADVANCEDSETTINGSDLG_H__AAD75600_3201_49D1_A5D4_3AE518917430__INCLUDED_)
#define AFX_CAMERAADVANCEDSETTINGSDLG_H__AAD75600_3201_49D1_A5D4_3AE518917430__INCLUDED_

#pragma once

// CameraAdvancedSettingsDlg.h : header file
//

#ifdef VIDEODEVICEDOC

// Forward Declaration
class CVideoDeviceDoc;

#define FRAMERATE_CHANGE_TIMEOUT	6	// In CAMERAADVANCEDSETTINGSDLG_TIMER_MS units

class CCameraAdvancedSettingsDlg : public CDialog
{
public:
	// Construction
	CCameraAdvancedSettingsDlg(CWnd* pParent);
	~CCameraAdvancedSettingsDlg();

	void Show();
	void Hide(BOOL bSaveSettingsOnHiding);
	void DestroyOnAppExit();
	void UpdateTitle();
	void ChangeThumbSize(int nNewWidth, int nNewHeight);

	// Dialog data
	enum { IDD = IDD_CAMERAADVANCEDSETTINGS };
	int	m_nSecondsBeforeMovementBegin;
	int	m_nSecondsAfterMovementEnd;
	int	m_nDetectionMinLengthSeconds;
	int m_nDetectionMaxFrames;
	BOOL m_bFlipH;
	BOOL m_bFlipV;
	CTime m_SchedulerStartTime;
	CTime m_SchedulerStopTime;
	BOOL m_bAudioListen;
	CSliderCtrl	m_VideoRecQuality;

protected:
	// Helper vars
	CVideoDeviceDoc* m_pDoc;
	BOOL m_bInOnTimer;
	BOOL m_bDoChangeFrameRate;
	int m_nFrameRateChangeTimeout;

	// Helper functions
	static int GetRevertedPos(CSliderCtrl* pSliderCtrl);
	static void SetRevertedPos(CSliderCtrl* pSliderCtrl, int nPos);
	BOOL IsEmpty(int nIDC);
	void UpdateVideoQualityInfo();
	void UpdateStartStopTimes();

	// Dialog functions
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void PostNcDestroy();
	afx_msg void OnDestroy();
	afx_msg void OnClose();
	afx_msg void OnChangeFrameRate();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnChangeSecondsBeforeMovementBegin();
	afx_msg void OnChangeSecondsAfterMovementEnd();
	afx_msg void OnChangeEditDetectionMinLength();
	afx_msg void OnChangeEditDetectionMaxFrames();
	afx_msg void OnCheckFlipH();
	afx_msg void OnCheckFlipV();
	afx_msg void OnVideoFormat();
	afx_msg void OnVideoSource();
	afx_msg void OnVideoInput();
	afx_msg void OnVideoTuner();
	afx_msg void OnRecAudio();
	afx_msg void OnCheckAudioListen();
	afx_msg void OnRecAudioFromStream();
	afx_msg void OnRecAudioFromSource();
	afx_msg void OnAudioInput();
	afx_msg void OnAudioMixer();
	afx_msg void OnCbnSelchangeComboboxScheduler();
	afx_msg void OnCheckSchedulerSunday();
	afx_msg void OnCheckSchedulerMonday();
	afx_msg void OnCheckSchedulerTuesday();
	afx_msg void OnCheckSchedulerWednesday();
	afx_msg void OnCheckSchedulerThursday();
	afx_msg void OnCheckSchedulerFriday();
	afx_msg void OnCheckSchedulerSaturday();
	afx_msg void OnDatetimechangeTimeDailyStart(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDatetimechangeTimeDailyStop(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSaveVideo();
	afx_msg void OnSaveAnimGif();
	afx_msg void OnAnimatedgifSize();
	afx_msg void OnChangeEditSnapshotRate();
	afx_msg void OnButtonThumbSize();
	afx_msg void OnExecCommand();
	afx_msg void OnNMClickSyslinkParamsHelp(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeExecCommandMode();
	afx_msg void OnChangeEditExe();
	afx_msg void OnChangeEditParams();
	afx_msg void OnCheckHideExecCommand();
	afx_msg void OnCheckWaitExecCommand();
	DECLARE_MESSAGE_MAP()
};

#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CAMERAADVANCEDSETTINGSDLG_H__AAD75600_3201_49D1_A5D4_3AE518917430__INCLUDED_)
