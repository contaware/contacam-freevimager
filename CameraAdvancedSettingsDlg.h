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
	void Hide();
	void DestroyOnAppExit();
	void UpdateTitleAndDir();
	void ChangeThumbSize(int nNewWidth, int nNewHeight);

	// Dialog data
	enum { IDD = IDD_CAMERAADVANCEDSETTINGS };
	int m_nSaveFreqDiv;
	int	m_nSecondsBeforeMovementBegin;
	int	m_nSecondsAfterMovementEnd;
	int	m_nDetectionMinLengthSeconds;
	int m_nDetectionMaxFrames;
	BOOL m_bFlipH;
	BOOL m_bFlipV;
	BOOL m_bAudioListen;
	CSliderCtrl	m_VideoRecQuality;
	int m_nExecCommandProfile;
	BOOL m_bExecCommand;
	int m_nExecCommandMode;
	CString m_sExecCommand;
	CString m_sExecParams;
	BOOL m_bHideExecCommand;
	BOOL m_bWaitExecCommand;

protected:
	// Helper vars
	CVideoDeviceDoc* m_pDoc;
	BOOL m_bInOnTimer;
	BOOL m_bDoChangeFrameRate;
	int m_nFrameRateChangeTimeout;
	CString m_sHostPortDlgDeviceTypeMode;

	// Helper functions
	static int GetRevertedPos(CSliderCtrl* pSliderCtrl);
	static void SetRevertedPos(CSliderCtrl* pSliderCtrl, int nPos);
	BOOL IsEmpty(int nIDC);
	void UpdateVideoQualityInfo();
	CString GetCurlPath();

	// Dialog functions
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void PostNcDestroy();
	afx_msg void OnDestroy();
	afx_msg void OnClose();
	afx_msg void OnChangeFrameRate();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnChangeSaveFreqDiv();
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
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSaveVideo();
	afx_msg void OnVideoRecFast();
	afx_msg void OnSaveAnimGif();
	afx_msg void OnAnimatedgifSize();
	afx_msg void OnSaveStartPicture();
	afx_msg void OnSelchangeSnapshotRate();
	afx_msg void OnButtonThumbSize();
	afx_msg void OnSyslinkControlHelp(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeExecCommandProfile();
	afx_msg void OnCheckExecCommand();
	afx_msg void OnSelchangeExecCommandMode();
	afx_msg void OnChangeEditExe();
	afx_msg void OnSyslinkParamsHelp(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeEditParams();
	afx_msg void OnCheckHideExecCommand();
	afx_msg void OnCheckWaitExecCommand();
	afx_msg void OnButtonPlaySound();
	afx_msg void OnButtonFtpUpload();
	afx_msg void OnButtonPlateRecognizer();
	afx_msg void OnButtonBackupFiles();
	DECLARE_MESSAGE_MAP()
};

#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CAMERAADVANCEDSETTINGSDLG_H__AAD75600_3201_49D1_A5D4_3AE518917430__INCLUDED_)
