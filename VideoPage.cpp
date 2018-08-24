// VideoPage.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "MainFrm.h"
#include "VideoPage.h"
#include "VideoDeviceDoc.h"
#include "VideoDeviceView.h"
#include "DxCapture.h"
#include "DxVideoInputDlg.h"
#include "ResizingDlg.h"
#include "FTPUploadConfigurationDlg.h"
#include "BrowseDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef VIDEODEVICEDOC

/////////////////////////////////////////////////////////////////////////////
// CVideoPage property page

IMPLEMENT_DYNCREATE(CVideoPage, CPropertyPage)

CVideoPage::CVideoPage()
	: CPropertyPage(CVideoPage::IDD)
{
	m_pDoc = NULL;
	m_bInOnTimer = FALSE;
	m_bDoChangeFrameRate = FALSE;
	m_nFrameRateChangeTimeout = FRAMERATE_CHANGE_TIMEOUT;
}

void CVideoPage::SetDoc(CVideoDeviceDoc* pDoc)
{
	ASSERT(pDoc);
	m_pDoc = pDoc;
}

CVideoPage::~CVideoPage()
{
}

void CVideoPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_SECONDS_BEFORE_MOVEMENT_BEGIN, m_nSecondsBeforeMovementBegin);
	DDV_MinMaxInt(pDX, m_nSecondsBeforeMovementBegin, 1, 99);
	DDX_Text(pDX, IDC_SECONDS_AFTER_MOVEMENT_END, m_nSecondsAfterMovementEnd);
	DDV_MinMaxInt(pDX, m_nSecondsAfterMovementEnd, 1, 99);
	DDX_Text(pDX, IDC_EDIT_DETECTION_MIN_LENGTH, m_nDetectionMinLengthSeconds);
	DDV_MinMaxInt(pDX, m_nDetectionMinLengthSeconds, 0, 99);
	DDX_Text(pDX, IDC_EDIT_DETECTION_MAX_FRAMES, m_nDetectionMaxFrames);
	DDV_MinMaxInt(pDX, m_nDetectionMaxFrames, 1, MOVDET_MAX_MAX_FRAMES_IN_LIST);
	DDX_Check(pDX, IDC_CHECK_LIVE_ROTATE180, m_bRotate180);
	DDX_DateTimeCtrl(pDX, IDC_TIME_DAILY_START, m_DetectionStartTime);
	DDX_DateTimeCtrl(pDX, IDC_TIME_DAILY_STOP, m_DetectionStopTime);
	DDX_Check(pDX, IDC_CHECK_AUDIO_LISTEN, m_bAudioListen);
	DDX_Control(pDX, IDC_VIDEO_COMPRESSION_QUALITY, m_VideoRecQuality);
}

BEGIN_MESSAGE_MAP(CVideoPage, CPropertyPage)
	ON_WM_DESTROY()
	ON_EN_CHANGE(IDC_FRAMERATE, OnChangeFrameRate)
	ON_WM_TIMER()
	ON_EN_CHANGE(IDC_SECONDS_BEFORE_MOVEMENT_BEGIN, OnChangeSecondsBeforeMovementBegin)
	ON_EN_CHANGE(IDC_SECONDS_AFTER_MOVEMENT_END, OnChangeSecondsAfterMovementEnd)
	ON_EN_CHANGE(IDC_EDIT_DETECTION_MIN_LENGTH, OnChangeEditDetectionMinLength)
	ON_EN_CHANGE(IDC_EDIT_DETECTION_MAX_FRAMES, OnChangeEditDetectionMaxFrames)
	ON_BN_CLICKED(IDC_CHECK_LIVE_ROTATE180, OnCheckLiveRotate180)
	ON_BN_CLICKED(IDC_VIDEO_FORMAT, OnVideoFormat)
	ON_BN_CLICKED(IDC_VIDEO_SOURCE, OnVideoSource)
	ON_BN_CLICKED(IDC_VIDEO_INPUT, OnVideoInput)
	ON_BN_CLICKED(IDC_VIDEO_TUNER, OnVideoTuner)
	ON_BN_CLICKED(IDC_REC_AUDIO, OnRecAudio)
	ON_BN_CLICKED(IDC_CHECK_AUDIO_LISTEN, OnCheckAudioListen)
	ON_BN_CLICKED(IDC_REC_AUDIO_FROM_STREAM, OnRecAudioFromStream)
	ON_BN_CLICKED(IDC_REC_AUDIO_FROM_SOURCE, OnRecAudioFromSource)
	ON_BN_CLICKED(IDC_AUDIO_INPUT, OnAudioInput)
	ON_BN_CLICKED(IDC_AUDIO_MIXER, OnAudioMixer)
	ON_CBN_SELCHANGE(IDC_COMBOBOX_DETECTION_SCHEDULER, OnCbnSelchangeComboboxDetectionScheduler)
	ON_BN_CLICKED(IDC_CHECK_SCHEDULER_SUNDAY, OnCheckSchedulerSunday)
	ON_BN_CLICKED(IDC_CHECK_SCHEDULER_MONDAY, OnCheckSchedulerMonday)
	ON_BN_CLICKED(IDC_CHECK_SCHEDULER_TUESDAY, OnCheckSchedulerTuesday)
	ON_BN_CLICKED(IDC_CHECK_SCHEDULER_WEDNESDAY, OnCheckSchedulerWednesday)
	ON_BN_CLICKED(IDC_CHECK_SCHEDULER_THURSDAY, OnCheckSchedulerThursday)
	ON_BN_CLICKED(IDC_CHECK_SCHEDULER_FRIDAY, OnCheckSchedulerFriday)
	ON_BN_CLICKED(IDC_CHECK_SCHEDULER_SATURDAY, OnCheckSchedulerSaturday)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_TIME_DAILY_START, OnDatetimechangeTimeDailyStart)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_TIME_DAILY_STOP, OnDatetimechangeTimeDailyStop)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_SAVE_VIDEO_MOVEMENT_DETECTION, OnSaveVideoMovementDetection)
	ON_BN_CLICKED(IDC_SAVE_ANIMATEDGIF_MOVEMENT_DETECTION, OnSaveAnimGifMovementDetection)
	ON_BN_CLICKED(IDC_ANIMATEDGIF_SIZE, OnAnimatedgifSize)
	ON_BN_CLICKED(IDC_FTP_MOVEMENT_DETECTION, OnFtpMovementDetection)
	ON_BN_CLICKED(IDC_FTP_CONFIGURE, OnFtpConfigure)
	ON_BN_CLICKED(IDC_EXEC_MOVEMENT_DETECTION, OnExecMovementDetection)
	ON_CBN_SELCHANGE(IDC_EXECMODE_MOVEMENT_DETECTION, OnSelchangeExecmodeMovementDetection)
	ON_EN_CHANGE(IDC_EDIT_EXE, OnChangeEditExe)
	ON_EN_CHANGE(IDC_EDIT_PARAMS, OnChangeEditParams)
	ON_BN_CLICKED(IDC_CHECK_HIDE_EXEC_COMMAND, OnCheckHideExecCommand)
	ON_BN_CLICKED(IDC_CHECK_WAIT_EXEC_COMMAND, OnCheckWaitExecCommand)
END_MESSAGE_MAP()

BOOL CVideoPage::OnInitDialog() 
{
	// Init vars
	m_nSecondsBeforeMovementBegin = m_pDoc->m_nMilliSecondsRecBeforeMovementBegin / 1000;
	m_nSecondsAfterMovementEnd = m_pDoc->m_nMilliSecondsRecAfterMovementEnd / 1000;
	m_nDetectionMinLengthSeconds = m_pDoc->m_nDetectionMinLengthMilliSeconds / 1000;
	m_nDetectionMaxFrames = m_pDoc->m_nDetectionMaxFrames;
	m_bRotate180 = m_pDoc->m_bRotate180;
	m_DetectionStartTime = m_pDoc->m_DetectionStartTime;
	m_DetectionStopTime = m_pDoc->m_DetectionStopTime;
	m_bAudioListen = m_pDoc->m_bAudioListen;

	// Init Combo Boxes
	CComboBox* pComboBoxDetectionScheduler = (CComboBox*)GetDlgItem(IDC_COMBOBOX_DETECTION_SCHEDULER);
	pComboBoxDetectionScheduler->AddString(ML_STRING(1874, "Always enabled (scheduler is off)"));
	pComboBoxDetectionScheduler->AddString(ML_STRING(1875, "Enabled:"));
	pComboBoxDetectionScheduler->AddString(ML_STRING(1876, "Disabled:"));
	CComboBox* pComboBoxExexMode = (CComboBox*)GetDlgItem(IDC_EXECMODE_MOVEMENT_DETECTION);
	pComboBoxExexMode->AddString(ML_STRING(1842, "Recording starts"));
	pComboBoxExexMode->AddString(ML_STRING(1843, "Saving done"));

	// This calls UpdateData(FALSE)
	CPropertyPage::OnInitDialog();

	// Frame Rate
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_FRAMERATE);
	if (m_pDoc->m_pDxCapture)
	{
		if (m_pDoc->m_pDxCapture->GetFrameRate() <= 0.0) // cannot be set
			pEdit->EnableWindow(FALSE);
	}
	else if (m_pDoc->m_pVideoNetCom)
	{
		// Disable all HTTP devices which do not support setting the framerate. 
		// HTTP jpeg snapshots devices can only be detected as such when running,
		// so enable them later on in OnTimer()
		if (m_pDoc->m_nNetworkDeviceTypeMode != CVideoDeviceDoc::AXIS_SP	&&
			m_pDoc->m_nNetworkDeviceTypeMode != CVideoDeviceDoc::EDIMAX_SP	&&
			m_pDoc->m_nNetworkDeviceTypeMode != CVideoDeviceDoc::PIXORD_SP	&&
			m_pDoc->m_nNetworkDeviceTypeMode != CVideoDeviceDoc::FOSCAM_SP)
			pEdit->EnableWindow(FALSE);
	}
	else
		pEdit->EnableWindow(FALSE);
	CString sFrameRate;
	sFrameRate.Format(_T("%0.1f"), m_pDoc->m_dFrameRate);
	if (pEdit->IsWindowEnabled())
		pEdit->SetWindowText(sFrameRate);
	else
		pEdit->SetWindowText(_T(""));

	// Enable Format Button?
	CButton* pButton = (CButton*)GetDlgItem(IDC_VIDEO_FORMAT);
	if ((m_pDoc->m_pDxCapture && m_pDoc->m_pDxCapture->HasFormats()) ||
		(m_pDoc->m_pDxCapture && m_pDoc->m_pDxCapture->IsDV() && m_pDoc->m_pDxCapture->HasDVFormatDlg()) ||
		!m_pDoc->m_pDxCapture)
		pButton->EnableWindow(TRUE);
	else
		pButton->EnableWindow(FALSE);

	// Enable Source Button?
	pButton = (CButton*)GetDlgItem(IDC_VIDEO_SOURCE);
	if (m_pDoc->m_pDxCapture && !m_pDoc->m_pDxCapture->IsOpenWithMediaSubType())
	{
		if (m_pDoc->m_pDxCapture->HasVideoCaptureFilterDlg())
			pButton->EnableWindow(TRUE);
		else
			pButton->EnableWindow(FALSE);
	}
	else
		pButton->EnableWindow(FALSE);

	// Enable TV-Tuner Button?
	pButton = (CButton*)GetDlgItem(IDC_VIDEO_TUNER);
	if (m_pDoc->m_pDxCapture)
	{
		if (m_pDoc->m_pDxCapture->HasVideoTVTunerDlg())
			pButton->EnableWindow(TRUE);
		else
			pButton->EnableWindow(FALSE);
	}
	else
		pButton->EnableWindow(FALSE);

	// Enable Input Button?
	// (It's for selecting video inputs like S-Video, TV-Tuner,...)
	pButton = (CButton*)GetDlgItem(IDC_VIDEO_INPUT);
	if (m_pDoc->m_pDxCapture)
	{
		if (m_pDoc->m_pDxCapture->GetInputsCount() > 0)
			pButton->EnableWindow(TRUE);
		else
			pButton->EnableWindow(FALSE);
	}
	else
		pButton->EnableWindow(FALSE);

	// Capture Audio Check Box
	CButton* pCheckAudio = (CButton*)GetDlgItem(IDC_REC_AUDIO);
	if (m_pDoc->m_bCaptureAudio)
		pCheckAudio->SetCheck(1);
	else
		pCheckAudio->SetCheck(0);

	// Capture Audio from Stream or from Source Check Boxes
	CButton* pCheckAudioFromStream = (CButton*)GetDlgItem(IDC_REC_AUDIO_FROM_STREAM);
	CButton* pCheckAudioFromSource = (CButton*)GetDlgItem(IDC_REC_AUDIO_FROM_SOURCE);
	if (m_pDoc->m_bCaptureAudioFromStream)
	{
		pCheckAudioFromStream->SetCheck(1);
		pCheckAudioFromSource->SetCheck(0);
	}
	else
	{
		pCheckAudioFromStream->SetCheck(0);
		pCheckAudioFromSource->SetCheck(1);
	}

	// Scheduler
	pComboBoxDetectionScheduler->SetCurSel(m_pDoc->m_nDetectionStartStop);
	CButton* pCheckScheduler = (CButton*)GetDlgItem(IDC_CHECK_SCHEDULER_SUNDAY);
	pCheckScheduler->SetCheck(m_pDoc->m_bDetectionSunday);
	pCheckScheduler = (CButton*)GetDlgItem(IDC_CHECK_SCHEDULER_MONDAY);
	pCheckScheduler->SetCheck(m_pDoc->m_bDetectionMonday);
	pCheckScheduler = (CButton*)GetDlgItem(IDC_CHECK_SCHEDULER_TUESDAY);
	pCheckScheduler->SetCheck(m_pDoc->m_bDetectionTuesday);
	pCheckScheduler = (CButton*)GetDlgItem(IDC_CHECK_SCHEDULER_WEDNESDAY);
	pCheckScheduler->SetCheck(m_pDoc->m_bDetectionWednesday);
	pCheckScheduler = (CButton*)GetDlgItem(IDC_CHECK_SCHEDULER_THURSDAY);
	pCheckScheduler->SetCheck(m_pDoc->m_bDetectionThursday);
	pCheckScheduler = (CButton*)GetDlgItem(IDC_CHECK_SCHEDULER_FRIDAY);
	pCheckScheduler->SetCheck(m_pDoc->m_bDetectionFriday);
	pCheckScheduler = (CButton*)GetDlgItem(IDC_CHECK_SCHEDULER_SATURDAY);
	pCheckScheduler->SetCheck(m_pDoc->m_bDetectionSaturday);

	// Save Video Movement Detection Check Box
	CButton* pCheckVideoSaveMovementDetection = (CButton*)GetDlgItem(IDC_SAVE_VIDEO_MOVEMENT_DETECTION);
	if (m_pDoc->m_bSaveVideoMovementDetection)
		pCheckVideoSaveMovementDetection->SetCheck(1);
	else
		pCheckVideoSaveMovementDetection->SetCheck(0);

	// Video Compressor Quality
	m_VideoRecQuality.SetRange((int)VIDEO_QUALITY_BEST, (int)VIDEO_QUALITY_LOW);
	m_VideoRecQuality.SetPageSize(1);
	m_VideoRecQuality.SetLineSize(1);
	m_pDoc->m_fVideoRecQuality = CAVRec::ClipVideoQuality(m_pDoc->m_fVideoRecQuality);
	::SetRevertedPos(&m_VideoRecQuality, (int)m_pDoc->m_fVideoRecQuality);
	UpdateVideoQualityInfo();

	// Save Animated GIF Movement Detection Check Box
	CButton* pCheckAnimGIFSaveMovementDetection = (CButton*)GetDlgItem(IDC_SAVE_ANIMATEDGIF_MOVEMENT_DETECTION);
	if (m_pDoc->m_bSaveAnimGIFMovementDetection)
		pCheckAnimGIFSaveMovementDetection->SetCheck(1);
	else
		pCheckAnimGIFSaveMovementDetection->SetCheck(0);

	// Animated GIF Button
	CString sSize;
	sSize.Format(ML_STRING(1769, "Thumbnail Size %i x %i"),	m_pDoc->m_dwAnimatedGifWidth,
															m_pDoc->m_dwAnimatedGifHeight);
	CButton* pButtonAnimGIFSize = (CButton*)GetDlgItem(IDC_ANIMATEDGIF_SIZE);
	pButtonAnimGIFSize->SetWindowText(sSize);

	// FTP Upload Movement Detection Check Box
	CButton* pCheckFTPUploadMovementDetection = (CButton*)GetDlgItem(IDC_FTP_MOVEMENT_DETECTION);
	if (m_pDoc->m_bFTPUploadMovementDetection)
		pCheckFTPUploadMovementDetection->SetCheck(1);
	else
		pCheckFTPUploadMovementDetection->SetCheck(0);

	// Execute Command Movement Detection
	CButton* pCheckExecCommandMovementDetection = (CButton*)GetDlgItem(IDC_EXEC_MOVEMENT_DETECTION);
	if (m_pDoc->m_bExecCommandMovementDetection)
		pCheckExecCommandMovementDetection->SetCheck(1);
	else
		pCheckExecCommandMovementDetection->SetCheck(0);
	pComboBoxExexMode->SetCurSel(m_pDoc->m_nExecModeMovementDetection);
	CEdit* pEditExecCommandMovementDetection = (CEdit*)GetDlgItem(IDC_EDIT_EXE);
	pEditExecCommandMovementDetection->SetWindowText(m_pDoc->m_sExecCommandMovementDetection);
	CEdit* pEditExecParamsMovementDetection = (CEdit*)GetDlgItem(IDC_EDIT_PARAMS);
	pEditExecParamsMovementDetection->SetWindowText(m_pDoc->m_sExecParamsMovementDetection);
	CButton* pCheckHideExecCommandMovementDetection = (CButton*)GetDlgItem(IDC_CHECK_HIDE_EXEC_COMMAND);
	if (m_pDoc->m_bHideExecCommandMovementDetection)
		pCheckHideExecCommandMovementDetection->SetCheck(1);
	else
		pCheckHideExecCommandMovementDetection->SetCheck(0);
	CButton* pCheckWaitExecCommandMovementDetection = (CButton*)GetDlgItem(IDC_CHECK_WAIT_EXEC_COMMAND);
	if (m_pDoc->m_bWaitExecCommandMovementDetection)
		pCheckWaitExecCommandMovementDetection->SetCheck(1);
	else
		pCheckWaitExecCommandMovementDetection->SetCheck(0);

	// Set Page Pointer to this
	m_pDoc->m_pVideoPage = this;
	
	// Set Timer
	SetTimer(ID_TIMER_MOVDETPAGE, MOVDETPAGE_TIMER_MS, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CVideoPage::OnDestroy() 
{
	// Kill timer
	KillTimer(ID_TIMER_MOVDETPAGE);

	// Base class
	CPropertyPage::OnDestroy();

	// Set Page Pointer to NULL
	m_pDoc->m_pVideoPage = NULL;
}

void CVideoPage::OnChangeFrameRate()
{
	CString sFrameRate;
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_FRAMERATE);
	pEdit->GetWindowText(sFrameRate);
	double dFrameRate = _tcstod(sFrameRate, NULL); // here we need the full precision, will be rounded in OnTimer()
	if (sFrameRate != _T("") && dFrameRate != m_pDoc->m_dFrameRate)
	{
		m_nFrameRateChangeTimeout = FRAMERATE_CHANGE_TIMEOUT;
		m_pDoc->StopProcessFrame(PROCESSFRAME_CHANGEFRAMERATE);
		m_bDoChangeFrameRate = TRUE; // done in OnTimer()
	}
}

void CVideoPage::OnTimer(UINT nIDEvent)
{
	// m_bInOnTimer avoids that a message pumping routine (like a modal dialog)
	// inside this OnTimer() calls OnTimer() again!
	if (!m_bInOnTimer)
	{
		// Set flag
		m_bInOnTimer = TRUE;

		if (!m_pDoc->m_bClosing)
		{
			// Enable Frame Rate Edit Control for HTTP jpeg snapshots devices
			CString sFrameRate;
			CEdit* pEdit = (CEdit*)GetDlgItem(IDC_FRAMERATE);
			if (m_pDoc->m_pVideoNetCom &&
				m_pDoc->m_pHttpVideoParseProcess->m_FormatType == CVideoDeviceDoc::CHttpParseProcess::FORMATVIDEO_JPEG &&
				!pEdit->IsWindowEnabled())
			{
				pEdit->EnableWindow(TRUE);
				sFrameRate.Format(_T("%0.1f"), m_pDoc->m_dFrameRate);
				pEdit->SetWindowText(sFrameRate); // this triggers OnChangeFrameRate()
			}

			// Change The Frame Rate if Necessary
			if (m_bDoChangeFrameRate && --m_nFrameRateChangeTimeout <= 0 && m_pDoc->IsProcessFrameStopped(PROCESSFRAME_CHANGEFRAMERATE))
			{
				pEdit->GetWindowText(sFrameRate);
				double dFrameRate = round(_tcstod(sFrameRate, NULL) * 10.0) / 10.0;
				if (sFrameRate != _T(""))
				{
					if (dFrameRate >= MIN_FRAMERATE && dFrameRate <= MAX_FRAMERATE)
					{
						// Correct the display format
						m_pDoc->m_dFrameRate = dFrameRate;
						sFrameRate.Format(_T("%0.1f"), m_pDoc->m_dFrameRate);
						pEdit->SetWindowText(sFrameRate); // this triggers OnChangeFrameRate()

														  // Set to device
						if (m_pDoc->m_pDxCapture)
						{
							m_pDoc->m_pDxCapture->Stop();
							m_pDoc->m_pDxCapture->SetFrameRate(m_pDoc->m_dFrameRate);
							m_pDoc->m_pDxCapture->Run();
						}
						else if (m_pDoc->m_pVideoNetCom)
						{
							if (m_pDoc->m_pHttpVideoParseProcess->m_FormatType == CVideoDeviceDoc::CHttpParseProcess::FORMATVIDEO_MJPEG)
							{
								if (m_pDoc->m_nNetworkDeviceTypeMode == CVideoDeviceDoc::EDIMAX_SP)
									m_pDoc->m_pHttpVideoParseProcess->m_bSetVideoFramerate = TRUE;
								m_pDoc->m_HttpThread.SetEventVideoConnect();
							}
						}
					}
					else
					{
						// Restore
						sFrameRate.Format(_T("%0.1f"), m_pDoc->m_dFrameRate);
						pEdit->SetWindowText(sFrameRate); // this triggers OnChangeFrameRate()
						pEdit->SetFocus();
						pEdit->SetSel(0xFFFF0000);
					}
				}

				// Restart process frame
				m_pDoc->StartProcessFrame(PROCESSFRAME_CHANGEFRAMERATE);

				// Clear flag here at the end
				m_bDoChangeFrameRate = FALSE;
			}
		}

		// Clear flag
		m_bInOnTimer = FALSE;
	}
	CPropertyPage::OnTimer(nIDEvent);
}

void CVideoPage::OnChangeSecondsBeforeMovementBegin()
{
	if (!IsEmpty(IDC_SECONDS_BEFORE_MOVEMENT_BEGIN) && UpdateData(TRUE))
		m_pDoc->m_nMilliSecondsRecBeforeMovementBegin = m_nSecondsBeforeMovementBegin * 1000;
}

void CVideoPage::OnChangeSecondsAfterMovementEnd()
{
	if (!IsEmpty(IDC_SECONDS_AFTER_MOVEMENT_END) && UpdateData(TRUE))
		m_pDoc->m_nMilliSecondsRecAfterMovementEnd = m_nSecondsAfterMovementEnd * 1000;
}

void CVideoPage::OnChangeEditDetectionMinLength()
{
	if (!IsEmpty(IDC_EDIT_DETECTION_MIN_LENGTH) && UpdateData(TRUE))
		m_pDoc->m_nDetectionMinLengthMilliSeconds = m_nDetectionMinLengthSeconds * 1000;
}

void CVideoPage::OnChangeEditDetectionMaxFrames()
{
	if (!IsEmpty(IDC_EDIT_DETECTION_MAX_FRAMES) && UpdateData(TRUE))
		m_pDoc->m_nDetectionMaxFrames = m_nDetectionMaxFrames;
}

void CVideoPage::OnCheckLiveRotate180()
{
	if (UpdateData(TRUE))
		m_pDoc->m_bRotate180 = m_bRotate180;
}

void CVideoPage::OnVideoFormat()
{
	// Open the video format dialog
	m_pDoc->VideoFormatDialog();

	// Read Frame-Rate which may have been changed
	if (m_pDoc->m_pDxCapture && m_pDoc->m_pDxCapture->GetFrameRate() > 0.0)
	{
		m_pDoc->m_dFrameRate = m_pDoc->m_pDxCapture->GetFrameRate();
		CString sFrameRate;
		sFrameRate.Format(_T("%0.1f"), m_pDoc->m_dFrameRate);
		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_FRAMERATE);
		pEdit->SetWindowText(sFrameRate);
	}
}

void CVideoPage::OnVideoSource()
{
	if (m_pDoc->m_pDxCapture)
	{
		// Open the video source dialog
		m_pDoc->m_pDxCapture->ShowVideoCaptureFilterDlg();

		// Read Frame-Rate which may have been changed
		if (m_pDoc->m_pDxCapture->GetFrameRate() > 0.0)
		{
			m_pDoc->m_dFrameRate = m_pDoc->m_pDxCapture->GetFrameRate();
			CString sFrameRate;
			sFrameRate.Format(_T("%0.1f"), m_pDoc->m_dFrameRate);
			CEdit* pEdit = (CEdit*)GetDlgItem(IDC_FRAMERATE);
			pEdit->SetWindowText(sFrameRate);
		}
	}
}

void CVideoPage::OnVideoInput()
{
	// Open the video input dialog
	if (m_pDoc->m_pDxCapture)
	{
		CDxVideoInputDlg dlg(m_pDoc);
		dlg.DoModal();
	}
}

void CVideoPage::OnVideoTuner()
{
	// Open the tv tuner dialog
	if (m_pDoc->m_pDxCapture)
		m_pDoc->m_pDxCapture->ShowVideoTVTunerDlg();
}

void CVideoPage::OnRecAudio()
{
	// Stop Save Frame List Thread
	m_pDoc->m_SaveFrameListThread.Kill();

	// Start/Stop Capture Audio Thread
	CButton* pCheck = (CButton*)GetDlgItem(IDC_REC_AUDIO);
	BOOL bDoCaptureAudio = (pCheck->GetCheck() == 1);
	if (bDoCaptureAudio)
	{
		m_pDoc->m_bCaptureAudio = TRUE;
		if (!m_pDoc->m_bCaptureAudioFromStream)
			m_pDoc->m_CaptureAudioThread.Start();
	}
	else
	{
		m_pDoc->m_bCaptureAudio = FALSE;
		if (!m_pDoc->m_bCaptureAudioFromStream)
			m_pDoc->m_CaptureAudioThread.Kill();
	}

	// Restart Save Frame List Thread
	m_pDoc->m_SaveFrameListThread.Start();
}

void CVideoPage::OnCheckAudioListen()
{
	if (UpdateData(TRUE))
		m_pDoc->m_bAudioListen = m_bAudioListen;
}

void CVideoPage::OnRecAudioFromStream()
{
	// Stop Save Frame List Thread
	m_pDoc->m_SaveFrameListThread.Kill();

	// Stop the Capture Audio Thread (if audio enabled)
	// and set the Capture Audio from Stream flag
	if (m_pDoc->m_bCaptureAudio)
		m_pDoc->m_CaptureAudioThread.Kill();
	m_pDoc->m_bCaptureAudioFromStream = TRUE;

	// Restart Save Frame List Thread
	m_pDoc->m_SaveFrameListThread.Start();
}

void CVideoPage::OnRecAudioFromSource()
{
	// Stop Save Frame List Thread
	m_pDoc->m_SaveFrameListThread.Kill();

	// Clear the Capture Audio from Stream flag
	// and start the Capture Audio Thread (if audio enabled)
	m_pDoc->m_bCaptureAudioFromStream = FALSE;
	if (m_pDoc->m_bCaptureAudio)
		m_pDoc->m_CaptureAudioThread.Start();

	// Restart Save Frame List Thread
	m_pDoc->m_SaveFrameListThread.Start();
}

void CVideoPage::OnAudioInput()
{
	m_pDoc->m_CaptureAudioThread.AudioInSourceDialog();
}

/*
Win95 and higher sndvol32.exe switches:
- /p Play
- /r Record
- /t Tray
- /s Small
- /d DeviceID starting from 0

Vista uses sndvol.exe:
- Master Volume Left: SndVol.exe -f 0
- Master Volume Right: SndVol.exe -f 49825268
- Volume Mixer Left: SndVol.exe -r 0
- Volume Mixer Right: SndVol.exe -r 49490633
- Playback Devices: control.exe mmsys.cpl,,0
- Recording Devices: control.exe mmsys.cpl,,1
- Sounds: control.exe mmsys.cpl,,2
Note: sndvol.exe currently displays volume controls for
audio-rendering endpoint devices only.
It does not display volume controls for
audio-capture devices.
*/
void CVideoPage::OnAudioMixer()
{
	::ShellExecute(NULL, NULL,
		_T("control.exe"), _T("mmsys.cpl,,1"), NULL, SW_SHOWNORMAL);
}

void CVideoPage::UpdateDetectionStartStopTimes()
{
	if (m_pDoc->m_nDetectionStartStop > 0)
	{
		// Start Time
		m_pDoc->m_DetectionStartTime = CTime(2000,
			1,
			1,
			m_DetectionStartTime.GetHour(),
			m_DetectionStartTime.GetMinute(),
			m_DetectionStartTime.GetSecond());

		// Stop Time
		m_pDoc->m_DetectionStopTime = CTime(2000,
			1,
			1,
			m_DetectionStopTime.GetHour(),
			m_DetectionStopTime.GetMinute(),
			m_DetectionStopTime.GetSecond());
	}
}

void CVideoPage::OnCbnSelchangeComboboxDetectionScheduler()
{
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBOBOX_DETECTION_SCHEDULER);
	m_pDoc->m_nDetectionStartStop = pComboBox->GetCurSel();
	UpdateDetectionStartStopTimes();
}

void CVideoPage::OnCheckSchedulerSunday()
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_SCHEDULER_SUNDAY);
	pCheck->GetCheck() == 0 ? m_pDoc->m_bDetectionSunday = FALSE : m_pDoc->m_bDetectionSunday = TRUE;
}

void CVideoPage::OnCheckSchedulerMonday()
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_SCHEDULER_MONDAY);
	pCheck->GetCheck() == 0 ? m_pDoc->m_bDetectionMonday = FALSE : m_pDoc->m_bDetectionMonday = TRUE;
}

void CVideoPage::OnCheckSchedulerTuesday()
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_SCHEDULER_TUESDAY);
	pCheck->GetCheck() == 0 ? m_pDoc->m_bDetectionTuesday = FALSE : m_pDoc->m_bDetectionTuesday = TRUE;
}

void CVideoPage::OnCheckSchedulerWednesday()
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_SCHEDULER_WEDNESDAY);
	pCheck->GetCheck() == 0 ? m_pDoc->m_bDetectionWednesday = FALSE : m_pDoc->m_bDetectionWednesday = TRUE;
}

void CVideoPage::OnCheckSchedulerThursday()
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_SCHEDULER_THURSDAY);
	pCheck->GetCheck() == 0 ? m_pDoc->m_bDetectionThursday = FALSE : m_pDoc->m_bDetectionThursday = TRUE;
}

void CVideoPage::OnCheckSchedulerFriday()
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_SCHEDULER_FRIDAY);
	pCheck->GetCheck() == 0 ? m_pDoc->m_bDetectionFriday = FALSE : m_pDoc->m_bDetectionFriday = TRUE;
}

void CVideoPage::OnCheckSchedulerSaturday()
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_SCHEDULER_SATURDAY);
	pCheck->GetCheck() == 0 ? m_pDoc->m_bDetectionSaturday = FALSE : m_pDoc->m_bDetectionSaturday = TRUE;
}

void CVideoPage::OnDatetimechangeTimeDailyStart(NMHDR* pNMHDR, LRESULT* pResult)
{
	if (UpdateData(TRUE))
		UpdateDetectionStartStopTimes();
	*pResult = 0;
}

void CVideoPage::OnDatetimechangeTimeDailyStop(NMHDR* pNMHDR, LRESULT* pResult)
{
	if (UpdateData(TRUE))
		UpdateDetectionStartStopTimes();
	*pResult = 0;
}

BOOL CVideoPage::IsEmpty(int nIDC)
{
	CString s;
	CEdit* pEdit = (CEdit*)GetDlgItem(nIDC);
	if (pEdit)
	{
		pEdit->GetWindowText(s);
		s.Trim();
	}
	return s.IsEmpty();
}

void CVideoPage::UpdateVideoQualityInfo()
{
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_VIDEO_COMPRESSION_QUALITY_INFO);
	CString sQuality;
	switch (::GetRevertedPos(&m_VideoRecQuality))
	{
		case 2: sQuality = ML_STRING(1545, "Best (lossless)"); break;
		case 3: sQuality = ML_STRING(1544, "Very Good"); break;
		case 4: sQuality = ML_STRING(1543, "Good"); break;
		case 5: sQuality = ML_STRING(1542, "Medium"); break;
		default: sQuality = ML_STRING(1541, "Low"); break;
	}
	pEdit->SetWindowText(sQuality);
}

void CVideoPage::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar)
	{
		CSliderCtrl* pSlider = (CSliderCtrl*)pScrollBar;

		if ((SB_THUMBTRACK == nSBCode) ||	// Dragging Slider
			(SB_THUMBPOSITION == nSBCode) ||// Wheel On Mouse And End Of Dragging Slider
			(SB_LINEUP == nSBCode) ||		// Keyboard Arrow
			(SB_LINEDOWN == nSBCode) ||	// Keyboard Arrow
			(SB_PAGEUP == nSBCode) ||		// Mouse Press Above Slider
			(SB_PAGEDOWN == nSBCode) ||		// Mouse Press Below Slider
			(SB_LEFT == nSBCode) ||		// Home Button
			(SB_RIGHT == nSBCode))			// End Button  
		{
			if (pSlider->GetDlgCtrlID() == IDC_VIDEO_COMPRESSION_QUALITY)
			{
				m_pDoc->m_fVideoRecQuality = (float)::GetRevertedPos(&m_VideoRecQuality);
				UpdateVideoQualityInfo();
			}
		}
	}
	CPropertyPage::OnHScroll(nSBCode, nPos, (CScrollBar*)pScrollBar);
}

void CVideoPage::OnSaveVideoMovementDetection()
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_SAVE_VIDEO_MOVEMENT_DETECTION);
	m_pDoc->m_bSaveVideoMovementDetection = pCheck->GetCheck() > 0;
}

void CVideoPage::OnSaveAnimGifMovementDetection()
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_SAVE_ANIMATEDGIF_MOVEMENT_DETECTION);
	m_pDoc->m_bSaveAnimGIFMovementDetection = pCheck->GetCheck() > 0;
}

void CVideoPage::OnAnimatedgifSize()
{
	// Stop Save Frame List Thread
	m_pDoc->m_SaveFrameListThread.Kill();

	CResizingDlg dlg(m_pDoc->m_DocRect.Width(), m_pDoc->m_DocRect.Height(),
					(int)m_pDoc->m_dwAnimatedGifWidth, (int)m_pDoc->m_dwAnimatedGifHeight,
					this);
	if (dlg.DoModal() == IDOK)
	{
		// Must be a multiple of 4 for stretch
		m_pDoc->m_dwAnimatedGifWidth = CVideoDeviceDoc::MakeSizeMultipleOf4(dlg.m_nPixelsWidth);
		m_pDoc->m_dwAnimatedGifHeight = CVideoDeviceDoc::MakeSizeMultipleOf4(dlg.m_nPixelsHeight);

		CString sSize;
		sSize.Format(ML_STRING(1769, "Thumbnail Size %i x %i"),
					m_pDoc->m_dwAnimatedGifWidth,
					m_pDoc->m_dwAnimatedGifHeight);
		CButton* pButton = (CButton*)GetDlgItem(IDC_ANIMATEDGIF_SIZE);
		pButton->SetWindowText(sSize);
	}

	// Restart Save Frame List Thread
	m_pDoc->m_SaveFrameListThread.Start();
}

void CVideoPage::OnFtpMovementDetection()
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_FTP_MOVEMENT_DETECTION);
	m_pDoc->m_bFTPUploadMovementDetection = pCheck->GetCheck() > 0;
}

void CVideoPage::OnFtpConfigure()
{
	// Stop Save Frame List Thread
	m_pDoc->m_SaveFrameListThread.Kill();

	// FTP Config Dialog
	CFTPUploadConfigurationDlg dlg;
	dlg.m_FTPUploadConfiguration = m_pDoc->m_MovDetFTPUploadConfiguration;
	if (dlg.DoModal() == IDOK)
		m_pDoc->m_MovDetFTPUploadConfiguration = dlg.m_FTPUploadConfiguration;

	// Restart Save Frame List Thread
	m_pDoc->m_SaveFrameListThread.Start();
}

void CVideoPage::OnExecMovementDetection()
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_EXEC_MOVEMENT_DETECTION);
	m_pDoc->m_bExecCommandMovementDetection = pCheck->GetCheck() > 0;
}

void CVideoPage::OnSelchangeExecmodeMovementDetection()
{
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_EXECMODE_MOVEMENT_DETECTION);
	m_pDoc->m_nExecModeMovementDetection = pComboBox->GetCurSel();
}

void CVideoPage::OnChangeEditExe()
{
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_EXE);
	::EnterCriticalSection(&m_pDoc->m_csExecCommandMovementDetection);
	pEdit->GetWindowText(m_pDoc->m_sExecCommandMovementDetection);
	::LeaveCriticalSection(&m_pDoc->m_csExecCommandMovementDetection);
}

void CVideoPage::OnChangeEditParams()
{
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PARAMS);
	::EnterCriticalSection(&m_pDoc->m_csExecCommandMovementDetection);
	pEdit->GetWindowText(m_pDoc->m_sExecParamsMovementDetection);
	::LeaveCriticalSection(&m_pDoc->m_csExecCommandMovementDetection);
}

void CVideoPage::OnCheckHideExecCommand()
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_HIDE_EXEC_COMMAND);
	::EnterCriticalSection(&m_pDoc->m_csExecCommandMovementDetection);
	m_pDoc->m_bHideExecCommandMovementDetection = pCheck->GetCheck() > 0;
	::LeaveCriticalSection(&m_pDoc->m_csExecCommandMovementDetection);
}

void CVideoPage::OnCheckWaitExecCommand()
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_WAIT_EXEC_COMMAND);
	::EnterCriticalSection(&m_pDoc->m_csExecCommandMovementDetection);
	m_pDoc->m_bWaitExecCommandMovementDetection = pCheck->GetCheck() > 0;
	::LeaveCriticalSection(&m_pDoc->m_csExecCommandMovementDetection);
}

#endif

