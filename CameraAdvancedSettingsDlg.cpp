// CameraAdvancedSettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "MainFrm.h"
#include "CameraAdvancedSettingsDlg.h"
#include "VideoDeviceDoc.h"
#include "VideoDeviceView.h"
#include "DxCapture.h"
#include "DxVideoInputDlg.h"
#include "HostPortDlg.h"
#include "ResizingDlg.h"
#include "BrowseDlg.h"
#include "PlateRecognizerDlg.h"
#include "FTPUploadDlg.h"
#include "NoVistaFileDlg.h"
#include "BrowseDlg.h"
#include <shlwapi.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef VIDEODEVICEDOC

CCameraAdvancedSettingsDlg::CCameraAdvancedSettingsDlg(CWnd* pParent)
	: CDialog(CCameraAdvancedSettingsDlg::IDD, pParent)
{
	CVideoDeviceView* pView = (CVideoDeviceView*)m_pParentWnd;
	ASSERT_VALID(pView);
	m_pDoc = (CVideoDeviceDoc*)pView->GetDocument();
	ASSERT_VALID(m_pDoc);
	m_bInOnTimer = FALSE;
	m_bDoChangeFrameRate = FALSE;
	m_nFrameRateChangeTimeout = FRAMERATE_CHANGE_TIMEOUT;
	CDialog::Create(CCameraAdvancedSettingsDlg::IDD, pParent);
}

CCameraAdvancedSettingsDlg::~CCameraAdvancedSettingsDlg()
{
}

void CCameraAdvancedSettingsDlg::Show()
{
	if (!IsWindowVisible())
	{
		m_pDoc->GetView()->ForceCursor();
		ShowWindow(SW_SHOW);
	}
}

void CCameraAdvancedSettingsDlg::Hide()
{
	if (IsWindowVisible())
	{
		m_pDoc->SaveSettings();
		ShowWindow(SW_HIDE);
		m_pDoc->GetView()->ForceCursor(FALSE);
	}
}

void CCameraAdvancedSettingsDlg::DestroyOnAppExit()
{
	if (IsWindowVisible())
		m_pDoc->GetView()->ForceCursor(FALSE);
	DestroyWindow(); // this calls OnDestroy() and PostNcDestroy()
}

void CCameraAdvancedSettingsDlg::OnClose()
{
	Hide();
}

BOOL CCameraAdvancedSettingsDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (HIWORD(wParam) == BN_CLICKED)
	{
		switch (LOWORD(wParam))
		{
		case IDOK:
			return TRUE;
		case IDCANCEL:
			Hide();
			return TRUE;
		default:
			return CDialog::OnCommand(wParam, lParam);
		}
	}
	return CDialog::OnCommand(wParam, lParam);
}

BOOL CCameraAdvancedSettingsDlg::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
		// Select All for all edit controls (single-line, multi-line or read-only)
		case WM_KEYDOWN:
			if (pMsg->wParam == 'A' && ::GetKeyState(VK_CONTROL) < 0)
			{
				CWnd* pWnd = GetFocus();
				if (pWnd != NULL)
				{
					TCHAR szClassName[6]; // one extra char to make sure it is exactly "Edit"
					::GetClassName(pWnd->GetSafeHwnd(), szClassName, 6);
					szClassName[5] = _T('\0');
					if (_tcsicmp(szClassName, _T("Edit")) == 0)
					{
						pWnd->SendMessage(EM_SETSEL, 0, -1);
						return TRUE;
					}
				}
			}
			break;

		default:
			break;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CCameraAdvancedSettingsDlg::OnDestroy()
{
	// Kill timer
	KillTimer(ID_TIMER_CAMERAADVANCEDSETTINGSDLG);

	// Base class
	CDialog::OnDestroy();
}

void CCameraAdvancedSettingsDlg::PostNcDestroy()
{
	m_pDoc->m_pCameraAdvancedSettingsDlg = NULL;
	delete this;
	CDialog::PostNcDestroy();
}

void CCameraAdvancedSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_SAVE_FREQ_DIV, m_nSaveFreqDiv);
	DDV_MinMaxInt(pDX, m_nSaveFreqDiv, 1, 9);
	DDX_Text(pDX, IDC_SECONDS_BEFORE_MOVEMENT_BEGIN, m_nSecondsBeforeMovementBegin);
	DDV_MinMaxInt(pDX, m_nSecondsBeforeMovementBegin, 1, 99);
	DDX_Text(pDX, IDC_SECONDS_AFTER_MOVEMENT_END, m_nSecondsAfterMovementEnd);
	DDV_MinMaxInt(pDX, m_nSecondsAfterMovementEnd, 1, 99);
	DDX_Text(pDX, IDC_EDIT_DETECTION_MIN_LENGTH, m_nDetectionMinLengthSeconds);
	DDV_MinMaxInt(pDX, m_nDetectionMinLengthSeconds, 0, 99);
	DDX_Text(pDX, IDC_EDIT_DETECTION_MAX_FRAMES, m_nDetectionMaxFrames);
	DDV_MinMaxInt(pDX, m_nDetectionMaxFrames, 1, MOVDET_MAX_MAX_FRAMES_IN_LIST);
	DDX_Check(pDX, IDC_CHECK_FLIP_H, m_bFlipH);
	DDX_Check(pDX, IDC_CHECK_FLIP_V, m_bFlipV);
	DDX_Check(pDX, IDC_CHECK_AUDIO_LISTEN, m_bAudioListen);
	DDX_Control(pDX, IDC_VIDEO_COMPRESSION_QUALITY, m_VideoRecQuality);
	DDX_Check(pDX, IDC_EXEC_COMMAND, m_bExecCommand);
	DDX_CBIndex(pDX, IDC_EXEC_COMMAND_MODE, m_nExecCommandMode);
	DDX_Text(pDX, IDC_EDIT_EXE, m_sExecCommand);
	DDX_Text(pDX, IDC_EDIT_PARAMS, m_sExecParams);
	DDX_Check(pDX, IDC_CHECK_HIDE_EXEC_COMMAND, m_bHideExecCommand);
	DDX_Check(pDX, IDC_CHECK_WAIT_EXEC_COMMAND, m_bWaitExecCommand);
}

BEGIN_MESSAGE_MAP(CCameraAdvancedSettingsDlg, CDialog)
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_EN_CHANGE(IDC_FRAMERATE, OnChangeFrameRate)
	ON_WM_TIMER()
	ON_EN_CHANGE(IDC_EDIT_SAVE_FREQ_DIV, OnChangeSaveFreqDiv)
	ON_EN_CHANGE(IDC_SECONDS_BEFORE_MOVEMENT_BEGIN, OnChangeSecondsBeforeMovementBegin)
	ON_EN_CHANGE(IDC_SECONDS_AFTER_MOVEMENT_END, OnChangeSecondsAfterMovementEnd)
	ON_EN_CHANGE(IDC_EDIT_DETECTION_MIN_LENGTH, OnChangeEditDetectionMinLength)
	ON_EN_CHANGE(IDC_EDIT_DETECTION_MAX_FRAMES, OnChangeEditDetectionMaxFrames)
	ON_BN_CLICKED(IDC_CHECK_FLIP_H, OnCheckFlipH)
	ON_BN_CLICKED(IDC_CHECK_FLIP_V, OnCheckFlipV)
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
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_SAVE_VIDEO, OnSaveVideo)
	ON_BN_CLICKED(IDC_SAVE_ANIMATEDGIF, OnSaveAnimGif)
	ON_BN_CLICKED(IDC_SAVE_START_PICTURE, OnSaveStartPicture)
	ON_BN_CLICKED(IDC_ANIMATEDGIF_SIZE, OnAnimatedgifSize)
	ON_CBN_SELCHANGE(IDC_COMBO_SNAPSHOT_RATE, OnSelchangeSnapshotRate)
	ON_BN_CLICKED(IDC_BUTTON_THUMB_SIZE, OnButtonThumbSize)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_CONTROL_HELP, OnSyslinkControlHelp)
	ON_NOTIFY(NM_RETURN, IDC_SYSLINK_CONTROL_HELP, OnSyslinkControlHelp)
	ON_BN_CLICKED(IDC_EXEC_COMMAND, OnCheckExecCommand)
	ON_CBN_SELCHANGE(IDC_EXEC_COMMAND_MODE, OnSelchangeExecCommandMode)
	ON_EN_CHANGE(IDC_EDIT_EXE, OnChangeEditExe)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_PARAMS_HELP, OnSyslinkParamsHelp)
	ON_NOTIFY(NM_RETURN, IDC_SYSLINK_PARAMS_HELP, OnSyslinkParamsHelp)
	ON_EN_CHANGE(IDC_EDIT_PARAMS, OnChangeEditParams)
	ON_BN_CLICKED(IDC_CHECK_HIDE_EXEC_COMMAND, OnCheckHideExecCommand)
	ON_BN_CLICKED(IDC_CHECK_WAIT_EXEC_COMMAND, OnCheckWaitExecCommand)
	ON_BN_CLICKED(IDC_BUTTON_PLAY_SOUND, OnButtonPlaySound)
	ON_BN_CLICKED(IDC_BUTTON_FTP_UPLOAD, OnButtonFtpUpload)
	ON_BN_CLICKED(IDC_BUTTON_PLATERECOGNIZER, OnButtonPlateRecognizer)
	ON_BN_CLICKED(IDC_BUTTON_BACKUP_FILES, OnButtonBackupFiles)
END_MESSAGE_MAP()

void CCameraAdvancedSettingsDlg::UpdateTitleAndDir()
{
	// Title
	CString sTitle;
	if (m_pDoc->m_pDxCapture)
	{
		if (m_pDoc->GetDeviceName() != m_pDoc->GetAssignedDeviceName())
			sTitle = m_pDoc->GetAssignedDeviceName() + _T(" , ") + m_pDoc->GetDeviceName();
		else
			sTitle = m_pDoc->GetDeviceName();
	}
	else if (m_pDoc->m_pVideoNetCom)
	{
		// URL?
		if ((m_pDoc->m_nNetworkDeviceTypeMode == CVideoDeviceDoc::GENERIC_CP	||
			m_pDoc->m_nNetworkDeviceTypeMode == CVideoDeviceDoc::GENERIC_SP)	&&
			m_pDoc->m_HttpGetFrameLocations[0] != _T("/"))
		{
			CString sOutHost;
			int nOutPort;
			int nOutDeviceTypeMode;
			CHostPortDlg::MakeUrl(	m_pDoc->m_sGetFrameVideoHost,
									m_pDoc->m_nGetFrameVideoPort,
									m_pDoc->m_HttpGetFrameLocations[0],
									m_pDoc->m_nNetworkDeviceTypeMode,
									sOutHost,
									nOutPort,
									nOutDeviceTypeMode);
			sTitle = m_pDoc->GetAssignedDeviceName() + _T(" , ") + sOutHost;
		}
		else
			sTitle = m_pDoc->GetAssignedDeviceName() + _T(" , ") + m_pDoc->GetDeviceName() + _T(" , ") + m_sHostPortDlgDeviceTypeMode;
	}
	else
	{
		// URL?
		if (m_pDoc->m_nNetworkDeviceTypeMode == CVideoDeviceDoc::URL_RTSP)
		{
			CString sOutHost;
			int nOutPort;
			int nOutDeviceTypeMode;
			CHostPortDlg::MakeUrl(	m_pDoc->m_sGetFrameVideoHost,
									m_pDoc->m_nGetFrameVideoPort,
									m_pDoc->m_HttpGetFrameLocations[0],
									m_pDoc->m_nNetworkDeviceTypeMode,
									sOutHost,
									nOutPort,
									nOutDeviceTypeMode);
			sTitle = m_pDoc->GetAssignedDeviceName() + _T(" , ") + sOutHost;
		}
		else
			sTitle = m_pDoc->GetAssignedDeviceName() + _T(" , ") + m_pDoc->GetDeviceName() + _T(" , ") + m_sHostPortDlgDeviceTypeMode;
	}
	SetWindowText(sTitle);

	// Dirs
	CString sRecordAutoSaveDir = m_pDoc->m_sRecordAutoSaveDir;
	sRecordAutoSaveDir.TrimRight(_T('\\'));
	CEdit* pEditBatch = (CEdit*)GetDlgItem(IDC_EDIT_BATCH);
	if (pEditBatch)
	{
		CString sCurrentBatch;
		pEditBatch->GetWindowText(sCurrentBatch);
		CString sNewBatch(_T("\"") + sRecordAutoSaveDir + _T("\\") + CAMERA_BATCH_FILENAME + _T("\""));
		if (sCurrentBatch != sNewBatch)
			pEditBatch->SetWindowText(sNewBatch);
	}
	pEditBatch = (CEdit*)GetDlgItem(IDC_EDIT_BATCH_SENSITIVITY);
	if (pEditBatch)
	{
		CString sCurrentBatch;
		pEditBatch->GetWindowText(sCurrentBatch);
		CString sNewBatch(_T("\"") + sRecordAutoSaveDir + _T("\\") + CAMERA_REC_SENSITIVITY_BATCH_FILENAME + _T("\""));
		if (sCurrentBatch != sNewBatch)
			pEditBatch->SetWindowText(sNewBatch);
	}
}

BOOL CCameraAdvancedSettingsDlg::OnInitDialog() 
{
	// Init vars
	m_nSaveFreqDiv = m_pDoc->m_nSaveFreqDiv;
	m_nSecondsBeforeMovementBegin = m_pDoc->m_nMilliSecondsRecBeforeMovementBegin / 1000;
	m_nSecondsAfterMovementEnd = m_pDoc->m_nMilliSecondsRecAfterMovementEnd / 1000;
	m_nDetectionMinLengthSeconds = m_pDoc->m_nDetectionMinLengthMilliSeconds / 1000;
	m_nDetectionMaxFrames = m_pDoc->m_nDetectionMaxFrames;
	m_bFlipH = m_pDoc->m_bFlipH;
	m_bFlipV = m_pDoc->m_bFlipV;
	m_bAudioListen = m_pDoc->m_bAudioListen;
	m_bExecCommand = m_pDoc->m_bExecCommand;
	m_nExecCommandMode = m_pDoc->m_nExecCommandMode;
	m_sExecCommand = m_pDoc->m_sExecCommand;
	m_sExecParams = m_pDoc->m_sExecParams;
	m_bHideExecCommand = m_pDoc->m_bHideExecCommand;
	m_bWaitExecCommand = m_pDoc->m_bWaitExecCommand;

	// Init m_sHostPortDlgDeviceTypeMode
	if (!m_pDoc->m_pDxCapture)
	{
		// When m_pDoc->m_nNetworkDeviceTypeMode is CVideoDeviceDoc::URL_RTSP
		// then there is no pair and m_sHostPortDlgDeviceTypeMode will be empty.
		// This behaviour is not a problem, see the UpdateTitleAndDir() function
		CArray<CDeviceTypeModePair> DeviceTypeModeArrayForCB;
		CHostPortDlg::InitDeviceTypeModeArrayForCB(DeviceTypeModeArrayForCB);
		for (int nModeIndex = 0; nModeIndex < DeviceTypeModeArrayForCB.GetSize(); nModeIndex++)
		{
			CDeviceTypeModePair& pair = DeviceTypeModeArrayForCB.ElementAt(nModeIndex);
			if (pair.m_dwMode == (DWORD)m_pDoc->m_nNetworkDeviceTypeMode)
			{
				m_sHostPortDlgDeviceTypeMode = pair.m_sMode;
				break;
			}
		}
	}

	// Init Combo Boxes
	CComboBox* pComboBoxSnapshotRate = (CComboBox*)GetDlgItem(IDC_COMBO_SNAPSHOT_RATE);
	pComboBoxSnapshotRate->SetItemData(pComboBoxSnapshotRate->AddString(ML_STRING(1888, "Fast")), 0);
	pComboBoxSnapshotRate->SetItemData(pComboBoxSnapshotRate->AddString(ML_STRING(1889, "1 second")), 1);
	pComboBoxSnapshotRate->SetItemData(pComboBoxSnapshotRate->AddString(ML_STRING(1890, "2 seconds")), 2);
	pComboBoxSnapshotRate->SetItemData(pComboBoxSnapshotRate->AddString(ML_STRING(1891, "3 seconds")), 3);
	pComboBoxSnapshotRate->SetItemData(pComboBoxSnapshotRate->AddString(ML_STRING(1892, "4 seconds")), 4);
	pComboBoxSnapshotRate->SetItemData(pComboBoxSnapshotRate->AddString(ML_STRING(1893, "5 seconds")), 5);
	pComboBoxSnapshotRate->SetItemData(pComboBoxSnapshotRate->AddString(ML_STRING(1894, "10 seconds")), 10);
	pComboBoxSnapshotRate->SetItemData(pComboBoxSnapshotRate->AddString(ML_STRING(1895, "15 seconds")), 15);
	pComboBoxSnapshotRate->SetItemData(pComboBoxSnapshotRate->AddString(ML_STRING(1896, "30 seconds")), 30);
	pComboBoxSnapshotRate->SetItemData(pComboBoxSnapshotRate->AddString(ML_STRING(1897, "1 minute")), 60);
	pComboBoxSnapshotRate->SetItemData(pComboBoxSnapshotRate->AddString(ML_STRING(1898, "2 minutes")), 120);
	pComboBoxSnapshotRate->SetItemData(pComboBoxSnapshotRate->AddString(ML_STRING(1899, "3 minutes")), 180);
	pComboBoxSnapshotRate->SetItemData(pComboBoxSnapshotRate->AddString(ML_STRING(1900, "4 minutes")), 240);
	pComboBoxSnapshotRate->SetItemData(pComboBoxSnapshotRate->AddString(ML_STRING(1901, "5 minutes")), 300);
	pComboBoxSnapshotRate->SetItemData(pComboBoxSnapshotRate->AddString(ML_STRING(1902, "10 minutes")), 600);
	pComboBoxSnapshotRate->SetItemData(pComboBoxSnapshotRate->AddString(ML_STRING(1903, "15 minutes")), 900);
	pComboBoxSnapshotRate->SetItemData(pComboBoxSnapshotRate->AddString(ML_STRING(1904, "30 minutes")), 1800);
	pComboBoxSnapshotRate->SetItemData(pComboBoxSnapshotRate->AddString(ML_STRING(1905, "1 hour")), 3600);
	CComboBox* pComboBoxExecCommandMode = (CComboBox*)GetDlgItem(IDC_EXEC_COMMAND_MODE);
	pComboBoxExecCommandMode->AddString(ML_STRING(1842, "Recording start"));
	pComboBoxExecCommandMode->AddString(ML_STRING(1843, "Recording saving done"));
	pComboBoxExecCommandMode->AddString(ML_STRING(1844, "Live snapshot"));
	pComboBoxExecCommandMode->AddString(ML_STRING(1845, "Daily summary"));

	// This calls UpdateData(FALSE)
	CDialog::OnInitDialog();

	// Frame Rate
	CEdit* pEditFrameRate = (CEdit*)GetDlgItem(IDC_FRAMERATE);
	if (m_pDoc->m_pDxCapture)
	{
		if (m_pDoc->m_pDxCapture->GetFrameRate() <= 0.0) // cannot be set
			pEditFrameRate->EnableWindow(FALSE);
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
			pEditFrameRate->EnableWindow(FALSE);
	}
	else
		pEditFrameRate->EnableWindow(FALSE);
	CString sFrameRate;
	sFrameRate.Format(_T("%0.1f"), m_pDoc->m_dFrameRate);
	if (pEditFrameRate->IsWindowEnabled())
		pEditFrameRate->SetWindowText(sFrameRate);
	else
		pEditFrameRate->SetWindowText(_T(""));

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

	// Save Video Check Box
	CButton* pCheckVideoSave = (CButton*)GetDlgItem(IDC_SAVE_VIDEO);
	if (m_pDoc->m_bSaveVideo)
		pCheckVideoSave->SetCheck(1);
	else
		pCheckVideoSave->SetCheck(0);

	// Video Compressor Quality
	m_VideoRecQuality.SetRange((int)VIDEO_QUALITY_BEST, (int)VIDEO_QUALITY_LOW);
	m_VideoRecQuality.SetPageSize(1);
	m_VideoRecQuality.SetLineSize(1);
	m_pDoc->m_fVideoRecQuality = CAVRec::ClipVideoQuality(m_pDoc->m_fVideoRecQuality);
	SetRevertedPos(&m_VideoRecQuality, (int)m_pDoc->m_fVideoRecQuality);
	UpdateVideoQualityInfo();

	// Save Animated GIF Check Box
	CButton* pCheckAnimGIFSave = (CButton*)GetDlgItem(IDC_SAVE_ANIMATEDGIF);
	if (m_pDoc->m_bSaveAnimGIF)
		pCheckAnimGIFSave->SetCheck(1);
	else
		pCheckAnimGIFSave->SetCheck(0);

	// Animated GIF Button
	CString sSize;
	sSize.Format(ML_STRING(1769, "Size %i x %i"),	m_pDoc->m_dwAnimatedGifWidth,
													m_pDoc->m_dwAnimatedGifHeight);
	CButton* pButtonAnimGIFSize = (CButton*)GetDlgItem(IDC_ANIMATEDGIF_SIZE);
	pButtonAnimGIFSize->SetWindowText(sSize);

	// Save Start Picture Check Box
	CButton* pCheckStartPictureSave = (CButton*)GetDlgItem(IDC_SAVE_START_PICTURE);
	if (m_pDoc->m_bSaveStartPicture)
		pCheckStartPictureSave->SetCheck(1);
	else
		pCheckStartPictureSave->SetCheck(0);

	// Snapshot rate
	for (int i = 0; i < pComboBoxSnapshotRate->GetCount(); i++)
	{
		if (m_pDoc->m_nSnapshotRate == pComboBoxSnapshotRate->GetItemData(i))
		{
			pComboBoxSnapshotRate->SetCurSel(i);
			break;
		}
	}

	// Thumbnail Size Button
	sSize.Format(ML_STRING(1769, "Size %i x %i"),	m_pDoc->m_nSnapshotThumbWidth,
													m_pDoc->m_nSnapshotThumbHeight);
	CButton* pButtonThumbnailSize = (CButton*)GetDlgItem(IDC_BUTTON_THUMB_SIZE);
	pButtonThumbnailSize->SetWindowText(sSize);

	// Set Pointer to this
	m_pDoc->m_pCameraAdvancedSettingsDlg = this;
	
	// Set title and recording dir
	UpdateTitleAndDir();

	// Set Timer
	SetTimer(ID_TIMER_CAMERAADVANCEDSETTINGSDLG, CAMERAADVANCEDSETTINGSDLG_TIMER_MS, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCameraAdvancedSettingsDlg::OnChangeFrameRate()
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

void CCameraAdvancedSettingsDlg::OnTimer(UINT nIDEvent)
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
	CDialog::OnTimer(nIDEvent);
}

void CCameraAdvancedSettingsDlg::OnChangeSaveFreqDiv()
{
	if (!IsEmpty(IDC_EDIT_SAVE_FREQ_DIV) && UpdateData(TRUE))
		m_pDoc->m_nSaveFreqDiv = m_nSaveFreqDiv;
}

void CCameraAdvancedSettingsDlg::OnChangeSecondsBeforeMovementBegin()
{
	if (!IsEmpty(IDC_SECONDS_BEFORE_MOVEMENT_BEGIN) && UpdateData(TRUE))
		m_pDoc->m_nMilliSecondsRecBeforeMovementBegin = m_nSecondsBeforeMovementBegin * 1000;
}

void CCameraAdvancedSettingsDlg::OnChangeSecondsAfterMovementEnd()
{
	if (!IsEmpty(IDC_SECONDS_AFTER_MOVEMENT_END) && UpdateData(TRUE))
		m_pDoc->m_nMilliSecondsRecAfterMovementEnd = m_nSecondsAfterMovementEnd * 1000;
}

void CCameraAdvancedSettingsDlg::OnChangeEditDetectionMinLength()
{
	if (!IsEmpty(IDC_EDIT_DETECTION_MIN_LENGTH) && UpdateData(TRUE))
		m_pDoc->m_nDetectionMinLengthMilliSeconds = m_nDetectionMinLengthSeconds * 1000;
}

void CCameraAdvancedSettingsDlg::OnChangeEditDetectionMaxFrames()
{
	if (!IsEmpty(IDC_EDIT_DETECTION_MAX_FRAMES) && UpdateData(TRUE))
		m_pDoc->m_nDetectionMaxFrames = m_nDetectionMaxFrames;
}

void CCameraAdvancedSettingsDlg::OnCheckFlipH()
{
	if (UpdateData(TRUE))
		m_pDoc->m_bFlipH = m_bFlipH;
}

void CCameraAdvancedSettingsDlg::OnCheckFlipV()
{
	if (UpdateData(TRUE))
		m_pDoc->m_bFlipV = m_bFlipV;
}

void CCameraAdvancedSettingsDlg::OnVideoFormat()
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

void CCameraAdvancedSettingsDlg::OnVideoSource()
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

void CCameraAdvancedSettingsDlg::OnVideoInput()
{
	// Open the video input dialog
	if (m_pDoc->m_pDxCapture)
	{
		CDxVideoInputDlg dlg(m_pDoc);
		dlg.DoModal();
	}
}

void CCameraAdvancedSettingsDlg::OnVideoTuner()
{
	// Open the tv tuner dialog
	if (m_pDoc->m_pDxCapture)
		m_pDoc->m_pDxCapture->ShowVideoTVTunerDlg();
}

void CCameraAdvancedSettingsDlg::OnRecAudio()
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

void CCameraAdvancedSettingsDlg::OnCheckAudioListen()
{
	if (UpdateData(TRUE))
		m_pDoc->m_bAudioListen = m_bAudioListen;
}

void CCameraAdvancedSettingsDlg::OnRecAudioFromStream()
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

void CCameraAdvancedSettingsDlg::OnRecAudioFromSource()
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

void CCameraAdvancedSettingsDlg::OnAudioInput()
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
void CCameraAdvancedSettingsDlg::OnAudioMixer()
{
	::ShellExecute(NULL, NULL, _T("control.exe"), _T("mmsys.cpl,,1"), NULL, SW_SHOWNORMAL);
}

int CCameraAdvancedSettingsDlg::GetRevertedPos(CSliderCtrl* pSliderCtrl)
{
	if (pSliderCtrl)
		return pSliderCtrl->GetRangeMin() + (pSliderCtrl->GetRangeMax() - pSliderCtrl->GetPos());
	else
		return 0;
}

void CCameraAdvancedSettingsDlg::SetRevertedPos(CSliderCtrl* pSliderCtrl, int nPos)
{
	if (pSliderCtrl)
		pSliderCtrl->SetPos(pSliderCtrl->GetRangeMin() + (pSliderCtrl->GetRangeMax() - nPos));
}

BOOL CCameraAdvancedSettingsDlg::IsEmpty(int nIDC)
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

void CCameraAdvancedSettingsDlg::UpdateVideoQualityInfo()
{
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_VIDEO_COMPRESSION_QUALITY_INFO);
	CString sQuality;
	switch (GetRevertedPos(&m_VideoRecQuality))
	{
		case 2: sQuality = ML_STRING(1545, "Best"); break;
		case 3: sQuality = ML_STRING(1544, "Very Good"); break;
		case 4: sQuality = ML_STRING(1543, "Good"); break;
		case 5: sQuality = ML_STRING(1542, "Medium"); break;
		default: sQuality = ML_STRING(1541, "Low"); break;
	}
	pEdit->SetWindowText(sQuality);
}

void CCameraAdvancedSettingsDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
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
				m_pDoc->m_fVideoRecQuality = (float)GetRevertedPos(&m_VideoRecQuality);
				UpdateVideoQualityInfo();
			}
		}
	}
	CDialog::OnHScroll(nSBCode, nPos, (CScrollBar*)pScrollBar);
}

void CCameraAdvancedSettingsDlg::OnSaveVideo()
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_SAVE_VIDEO);
	m_pDoc->m_bSaveVideo = pCheck->GetCheck() > 0;
}

void CCameraAdvancedSettingsDlg::OnSaveAnimGif()
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_SAVE_ANIMATEDGIF);
	m_pDoc->m_bSaveAnimGIF = pCheck->GetCheck() > 0;
}

void CCameraAdvancedSettingsDlg::OnAnimatedgifSize()
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
		sSize.Format(ML_STRING(1769, "Size %i x %i"),
					m_pDoc->m_dwAnimatedGifWidth,
					m_pDoc->m_dwAnimatedGifHeight);
		CButton* pButton = (CButton*)GetDlgItem(IDC_ANIMATEDGIF_SIZE);
		pButton->SetWindowText(sSize);
	}

	// Restart Save Frame List Thread
	m_pDoc->m_SaveFrameListThread.Start();
}

void CCameraAdvancedSettingsDlg::OnSaveStartPicture()
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_SAVE_START_PICTURE);
	m_pDoc->m_bSaveStartPicture = pCheck->GetCheck() > 0;
}

void CCameraAdvancedSettingsDlg::OnSelchangeSnapshotRate()
{
	// Get the set rate and update the variable
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_SNAPSHOT_RATE);
	m_pDoc->m_nSnapshotRate = pComboBox->GetItemData(pComboBox->GetCurSel());

	// Update the web interface seconds rate
	CString sText;
	sText.Format(_T("%d"), m_pDoc->m_nSnapshotRate);
	m_pDoc->PhpConfigFileSetParam(PHPCONFIG_SNAPSHOTREFRESHSEC, sText);

	// Update the web interface sub-seconds rate
	//
	// In case that SNAPSHOTREFRESHSEC is >= 1
	// - push.php polls the snapshots file modification time each
	//   SERVERPUSH_POLLRATE_MS and sends the frames not faster than each
	//   second because the modification time has a granularity of one second
	// - snapshot.php and snapshotfull.php load snapshots each SNAPSHOTREFRESHSEC
	//
	// In case that SNAPSHOTREFRESHSEC is 0
	// - push.php sends frames with a rate of SERVERPUSH_POLLRATE_MS
	// - snapshot.php and snapshotfull.php load snapshots each SERVERPUSH_POLLRATE_MS
	sText.Format(_T("%d"), DEFAULT_SERVERPUSH_POLLRATE_MS);
	m_pDoc->PhpConfigFileSetParam(PHPCONFIG_SERVERPUSH_POLLRATE_MS, sText);
}

void CCameraAdvancedSettingsDlg::ChangeThumbSize(int nNewWidth, int nNewHeight)
{
	// Init thumb vars: must be a multiple of 4 for some video codecs,
	// most efficient would be a multiple of 16 to fit the macro blocks
	m_pDoc->m_nSnapshotThumbWidth = CVideoDeviceDoc::MakeSizeMultipleOf4(nNewWidth);
	m_pDoc->m_nSnapshotThumbHeight = CVideoDeviceDoc::MakeSizeMultipleOf4(nNewHeight);
	CString sSize;
	sSize.Format(ML_STRING(1769, "Size %i x %i"),
				m_pDoc->m_nSnapshotThumbWidth,
				m_pDoc->m_nSnapshotThumbHeight);
	CButton* pButton = (CButton*)GetDlgItem(IDC_BUTTON_THUMB_SIZE);
	pButton->SetWindowText(sSize);
}

void CCameraAdvancedSettingsDlg::OnButtonThumbSize()
{
	CResizingDlg dlg(m_pDoc->m_DocRect.Width(), m_pDoc->m_DocRect.Height(),
					m_pDoc->m_nSnapshotThumbWidth, m_pDoc->m_nSnapshotThumbHeight,
					this);
	if (dlg.DoModal() == IDOK)
	{
		ChangeThumbSize(dlg.m_nPixelsWidth, dlg.m_nPixelsHeight);
		CString sWidth, sHeight;
		sWidth.Format(_T("%d"), m_pDoc->m_nSnapshotThumbWidth);
		sHeight.Format(_T("%d"), m_pDoc->m_nSnapshotThumbHeight);
		m_pDoc->PhpConfigFileSetParam(PHPCONFIG_THUMBWIDTH, sWidth);
		m_pDoc->PhpConfigFileSetParam(PHPCONFIG_THUMBHEIGHT, sHeight);
	}
}

void CCameraAdvancedSettingsDlg::OnSyslinkControlHelp(NMHDR* pNMHDR, LRESULT* pResult)
{
	::ShellExecute(NULL, _T("open"), EXAMPLE_CONTROL_ONLINE_PAGE, NULL, NULL, SW_SHOWNORMAL);
	*pResult = 0;
}

void CCameraAdvancedSettingsDlg::OnCheckExecCommand()
{
	if (UpdateData(TRUE))
		m_pDoc->m_bExecCommand = m_bExecCommand;
}

void CCameraAdvancedSettingsDlg::OnSelchangeExecCommandMode()
{
	if (UpdateData(TRUE))
		m_pDoc->m_nExecCommandMode = m_nExecCommandMode;
}

void CCameraAdvancedSettingsDlg::OnChangeEditExe()
{
	if (UpdateData(TRUE))
	{
		::EnterCriticalSection(&m_pDoc->m_csExecCommand);
		m_pDoc->m_sExecCommand = m_sExecCommand;
		::LeaveCriticalSection(&m_pDoc->m_csExecCommand);
	}
}

void CCameraAdvancedSettingsDlg::OnSyslinkParamsHelp(NMHDR* pNMHDR, LRESULT* pResult)
{
	::ShellExecute(NULL, _T("open"), EXAMPLE_COMMANDS_ONLINE_PAGE, NULL, NULL, SW_SHOWNORMAL);
	*pResult = 0;
}

void CCameraAdvancedSettingsDlg::OnChangeEditParams()
{
	if (UpdateData(TRUE))
	{
		::EnterCriticalSection(&m_pDoc->m_csExecCommand);
		m_pDoc->m_sExecParams = m_sExecParams;
		::LeaveCriticalSection(&m_pDoc->m_csExecCommand);
	}
}

void CCameraAdvancedSettingsDlg::OnCheckHideExecCommand()
{
	if (UpdateData(TRUE))
		m_pDoc->m_bHideExecCommand = m_bHideExecCommand;
}

void CCameraAdvancedSettingsDlg::OnCheckWaitExecCommand()
{
	if (UpdateData(TRUE))
		m_pDoc->m_bWaitExecCommand = m_bWaitExecCommand;
}

void CCameraAdvancedSettingsDlg::OnButtonPlaySound()
{
	// Check whether VLC is installed
	CString sVlcDir = ::GetRegistryStringValue(HKEY_LOCAL_MACHINE, _T("Software\\VideoLAN\\VLC"), _T("InstallDir")); // 32 bit vlc
	if (sVlcDir.IsEmpty())
		sVlcDir = ::GetRegistryStringValue(HKEY_LOCAL_MACHINE, _T("Software\\VideoLAN\\VLC"), _T("InstallDir"), KEY_WOW64_64KEY); // 64 bit vlc
	if (sVlcDir.IsEmpty())
	{
		CTaskDialog dlg(_T("<a href=\"https://www.videolan.org/\">www.videolan.org</a>"),
						ML_STRING(1911, "Please install VLC media player from:"),
						ML_STRING(1910, "VLC missing"),
						TDCBF_OK_BUTTON,
						TDF_ENABLE_HYPERLINKS | TDF_USE_COMMAND_LINKS | TDF_SIZE_TO_CONTENT);
		dlg.SetMainIcon(TD_ERROR_ICON);
		dlg.DoModal(::AfxGetMainFrame()->GetSafeHwnd());
	}
	else
	{
		// This transfers data from the dialog window to
		// the member variables validating it
		if (!UpdateData(TRUE))
			return;

		// Init audio file
		CString sExecParams(m_sExecParams);
		CString s, sFileName;
		while (!(s = ::ParseNextParam(sExecParams)).IsEmpty())
		{
			if (::IsExistingFile(s))
			{
				sFileName = s;
				break;
			}
		}

		// Prompt for audio file
		CNoVistaFileDlg fd(	TRUE,
							_T(""),
							sFileName,
							OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, // file must exist and hide the read-only check box
							_T("All Files (*.*)|*.*||"));
		CString sDlgTitle(ML_STRING(1848, "Choose Audio File"));
		fd.m_ofn.lpstrTitle = sDlgTitle;
		if (fd.DoModal() == IDOK)
		{
			// Fill executable & params
			sVlcDir.TrimRight(_T('\\'));
			m_sExecCommand = sVlcDir + _T("\\vlc.exe");
			m_sExecParams.Format(_T("-I dummy --dummy-quiet --play-and-exit --no-loop --no-repeat \"%s\""), fd.GetPathName());
			::EnterCriticalSection(&m_pDoc->m_csExecCommand);
			m_pDoc->m_sExecCommand = m_sExecCommand;
			m_pDoc->m_sExecParams = m_sExecParams;
			::LeaveCriticalSection(&m_pDoc->m_csExecCommand);

			// Fill flags and mode
			m_pDoc->m_bHideExecCommand = m_bHideExecCommand = TRUE;
			m_pDoc->m_bWaitExecCommand = m_bWaitExecCommand = TRUE;
			m_pDoc->m_nExecCommandMode = m_nExecCommandMode = 0;
			m_pDoc->m_bExecCommand = m_bExecCommand = TRUE;

			// Update data from vars to view
			UpdateData(FALSE);
		}
	}
}

CString CCameraAdvancedSettingsDlg::GetCurlPath()
{
	TCHAR szCurlPath[MAX_PATH];
	_tcscpy(szCurlPath, _T("curl.exe"));
	if (!::PathFindOnPath(szCurlPath, NULL))
	{
		/*
		The %windir%\System32 directory is reserved for 64-bit applications on 64-bit Windows.
		Most DLL file names were not changed when 64-bit versions of the DLLs were created,
		so 32-bit versions of the DLLs are stored in a different directory. WOW64 (Windows 32-bit
		on Windows 64-bit) hides this difference by using a file system redirector.

		Whenever a 32-bit application attempts to access %windir%\System32 the access is redirected to
		%windir%\SysWOW64.

		32-bit applications can access the native system directory by substituting %windir%\Sysnative
		for %windir%\System32. WOW64 recognizes Sysnative as a special alias used to indicate that
		the file system should not redirect the access. This mechanism is flexible and easy to use,
		therefore, it is the recommended mechanism to bypass file system redirection. Note that 64-bit
		applications cannot use the Sysnative alias as it is a virtual directory not a real one.

		https://docs.microsoft.com/en-us/windows/win32/winprog64/file-system-redirector

		On a 64-bit Windows it often happens that there is only a 64-bit curl.exe version in
		%windir%\System32 and nothing under %windir%\SysWOW64.
		We are a 32-bit application and our PathFindOnPath() calls do not find curl.exe under
		the redirected %windir%\System32 (note that only %windir%\System32 is in the PATH).

		The 64-bit Windows 10 since 1803 has both 32-bit and 64-bit curl.exe correctly installed.
		*/
		TCHAR szSysNative[MAX_PATH];
		::GetWindowsDirectory(szSysNative, MAX_PATH);
		TCHAR LastChar = _T('\0');
		int nLength = _tcslen(szSysNative);
		if (nLength > 0)
			LastChar = szSysNative[nLength - 1];
		if (LastChar != _T('\\'))
			_tcscat(szSysNative, (LPCTSTR)_T("\\"));
		_tcscat(szSysNative, (LPCTSTR)_T("Sysnative"));
		const TCHAR* OtherDirs[] = { szSysNative, NULL };
		if (!::PathFindOnPath(szCurlPath, OtherDirs))
		{
			TCHAR szSystem32Path[MAX_PATH];
			::GetSystemDirectory(szSystem32Path, MAX_PATH);
			CString sCurlMsg;
			sCurlMsg.Format(_T("<a href=\"https://curl.haxx.se/\">curl.haxx.se</a>\n") +
							ML_STRING(1915, "1. Download binary and unzip") + _T("\n") + 
							ML_STRING(1916, "2. From bin copy curl.exe and curl-ca-bundle.crt to %s"), szSystem32Path);
			CTaskDialog dlg(sCurlMsg,
							ML_STRING(1914, "Upgrade to Windows 10 (1803) or get curl from:"),
							ML_STRING(1913, "Curl missing"),
							TDCBF_OK_BUTTON,
							TDF_ENABLE_HYPERLINKS | TDF_USE_COMMAND_LINKS | TDF_SIZE_TO_CONTENT);
			dlg.SetMainIcon(TD_ERROR_ICON);
			dlg.DoModal(::AfxGetMainFrame()->GetSafeHwnd());
			return _T("");
		}
	}
	return szCurlPath;
}

void CCameraAdvancedSettingsDlg::OnButtonFtpUpload()
{
	CString sCurlPath = GetCurlPath();
	if (!sCurlPath.IsEmpty())
	{
		// This transfers data from the dialog window to
		// the member variables validating it
		if (!UpdateData(TRUE))
			return;

		// Init dialog vars
		CFTPUploadDlg dlg;
		CString sExecParams(m_sExecParams);
		CString s;
		CStringArray Params;
		while (!(s = ::ParseNextParam(sExecParams)).IsEmpty())
			Params.Add(s);
		int i = 0;
		while (i < Params.GetSize())
		{
			int n;
			if (Params[i].Compare(_T("--user")) == 0 || Params[i].Compare(_T("-u")) == 0)
			{
				if (++i >= Params.GetSize())
					break;
				if ((n = Params[i].Find(_T(":"))) >= 0)
				{
					dlg.m_sUsername = Params[i].Left(n);
					dlg.m_sPassword = Params[i].Mid(n + 1);
				}
			}
			else if (Params[i].Find(_T("ftp://")) == 0)
			{
				if ((n = Params[i].Find(_T("/"), 6)) > 6)
					dlg.m_sHost = Params[i].Mid(6, n - 6);
			}

			// Next
			i++;
		}

		// Show dialog
		if (dlg.DoModal() == IDOK)
		{
			// Fill executable & params
			m_sExecCommand = sCurlPath;
			m_sExecParams.Format(_T("--ftp-create-dirs --insecure --ssl --user \"%s:%s\" --upload-file \"%%full%%\" \"ftp://%s/%%year%%/%%month%%/%%day%%/\" --upload-file \"%%small%%\" \"ftp://%s/%%year%%/%%month%%/%%day%%/\""), dlg.m_sUsername, dlg.m_sPassword, dlg.m_sHost, dlg.m_sHost);
			::EnterCriticalSection(&m_pDoc->m_csExecCommand);
			m_pDoc->m_sExecCommand = m_sExecCommand;
			m_pDoc->m_sExecParams = m_sExecParams;
			::LeaveCriticalSection(&m_pDoc->m_csExecCommand);

			// Fill flags and mode
			m_pDoc->m_bHideExecCommand = m_bHideExecCommand = TRUE;
			m_pDoc->m_bWaitExecCommand = m_bWaitExecCommand = FALSE;
			m_pDoc->m_nExecCommandMode = m_nExecCommandMode = 1;
			m_pDoc->m_bExecCommand = m_bExecCommand = TRUE;

			// Update data from vars to view
			UpdateData(FALSE);
		}
	}
}

void CCameraAdvancedSettingsDlg::OnButtonPlateRecognizer()
{
	CString sCurlPath = GetCurlPath();
	if (!sCurlPath.IsEmpty())
	{
		// This transfers data from the dialog window to
		// the member variables validating it
		if (!UpdateData(TRUE))
			return;

		// Init dialog vars
		CPlateRecognizerDlg dlg;
		CString sExecParams(m_sExecParams);
		CString s, sUrl;
		CStringArray Params;
		while (!(s = ::ParseNextParam(sExecParams)).IsEmpty())
			Params.Add(s);
		int i = 0;
		while (i < Params.GetSize())
		{
			int n;
			if (Params[i].Compare(_T("--header")) == 0 || Params[i].Compare(_T("-H")) == 0)
			{
				if (++i >= Params.GetSize())
					break;
				if ((n = Params[i].Find(_T("Authorization:"))) >= 0)
				{
					if ((n = Params[i].Find(_T("Token"), n + 14)) >= 0)
					{
						dlg.m_sToken = Params[i].Mid(n + 5);
						dlg.m_sToken.Trim();
						dlg.m_nMode = 0;
					}
				}
			}
			else if (Params[i].Compare(_T("--form")) == 0  || Params[i].Compare(_T("-F")) == 0)
			{
				if (++i >= Params.GetSize())
					break;
				if ((n = Params[i].Find(_T("regions="))) >= 0)
				{
					if (!dlg.m_sRegions.IsEmpty())
						dlg.m_sRegions += _T(" ");
					dlg.m_sRegions += Params[i].Mid(n + 8);
				}
			}
			else if (Params[i].Find(_T("http")) == 0)
				sUrl = Params[i];
			
			// Next
			i++;
		}
		if (dlg.m_sToken.IsEmpty() && !sUrl.IsEmpty())
		{
			dlg.m_nMode = 1;
			dlg.m_sUrl = sUrl;
		}

		// Show dialog
		if (dlg.DoModal() == IDOK)
		{
			// Set executable
			m_sExecCommand = sCurlPath;

			// Parse regions
			CString sRegions = dlg.m_sRegions;
			CStringArray Regions;
			int nPos;
			while ((nPos = sRegions.Find(_T(' '))) >= 0)
			{
				CString sSingleRegion = sRegions.Left(nPos);
				if (!sSingleRegion.IsEmpty())
					Regions.Add(sSingleRegion);
				sRegions = sRegions.Mid(nPos + 1);
			}
			if (!sRegions.IsEmpty())
				Regions.Add(sRegions);

			// Fill params
			m_sExecParams = _T("-F \"upload=@%full%\"");
			for (int n = 0; n < Regions.GetCount(); n++)
				m_sExecParams += _T(" -F regions=") + Regions[n];
			if (dlg.m_nMode == 0)
			{
				m_sExecParams += _T(" -F \"camera_id=%name%\"");
				m_sExecParams += _T(" -H \"Authorization: Token ") + dlg.m_sToken + _T("\" https://api.platerecognizer.com/v1/plate-reader");
			}
			else
			{
				// Note: as per doc the SDK API does not support fields: camera_id and timestamp
				m_sExecParams += _T(" ") + dlg.m_sUrl;
			}

			// Update executable & params
			::EnterCriticalSection(&m_pDoc->m_csExecCommand);
			m_pDoc->m_sExecCommand = m_sExecCommand;
			m_pDoc->m_sExecParams = m_sExecParams;
			::LeaveCriticalSection(&m_pDoc->m_csExecCommand);

			// Fill flags and mode
			m_pDoc->m_bHideExecCommand = m_bHideExecCommand = TRUE;
			m_pDoc->m_bWaitExecCommand = m_bWaitExecCommand = FALSE;
			m_pDoc->m_nExecCommandMode = m_nExecCommandMode = 0;
			m_pDoc->m_bExecCommand = m_bExecCommand = TRUE;

			// Update data from vars to view
			UpdateData(FALSE);
		}
	}
}

void CCameraAdvancedSettingsDlg::OnButtonBackupFiles()
{
	/*
	cmd.exe
	
	Attention: for quote characters we means the double quote ", not the single quote '

	If /C or /K is specified, then the remainder of the command line after
	the switch is processed as a command line with no need to quote it.
	
	Quote characters are processed as follows:

	1.  If all of the following conditions are met, then quote characters
		on the command line after /C or /K are preserved:

		- no /S switch
		- exactly two quote characters in total!
		- no special characters between the two quote characters,
			where special is one of: &<>()@^|
		- there are one or more whitespace characters between the
			two quote characters
		- the string between the two quote characters is the name
			of an executable file

	2.  Otherwise, old behavior is to see if the first character is
		a quote character and if so, strip it and remove the last
		found quote character on the command line, preserving
		any text after this last quote character
	*/
	TCHAR szCmdPath[MAX_PATH];
	_tcscpy(szCmdPath, _T("cmd.exe"));
	if (::PathFindOnPath(szCmdPath, NULL))
	{
		// This transfers data from the dialog window to
		// the member variables validating it
		if (!UpdateData(TRUE))
			return;

		// Init destination folder
		CString sExecParams(m_sExecParams);
		CString s, sDst;
		while (!(s = ::ParseNextParam(sExecParams)).IsEmpty())
		{
			int n;
			if ((n = s.Find(_T("\\%year%\\%month%\\%day%"))) >= 0)
			{
				s = s.Left(n);
				if (::IsExistingDir(s))
				{
					sDst = s;
					break;
				}
			}
		}

		// Prompt for destination folder
		CBrowseDlg dlg(::AfxGetMainFrame(), &sDst, ML_STRING(1359, "Select the Destination Directory"), TRUE);
		if (dlg.DoModal() == IDOK)
		{
			sDst.TrimRight(_T('\\'));
			if (!sDst.IsEmpty())
			{
				// Fill executable & params
				m_sExecCommand = szCmdPath;
				m_sExecParams.Format(_T("/C mkdir \"%s\\%%year%%\\%%month%%\\%%day%%\" & copy \"%%full%%\" \"%s\\%%year%%\\%%month%%\\%%day%%\" & copy \"%%small%%\" \"%s\\%%year%%\\%%month%%\\%%day%%\""), sDst, sDst, sDst);
				::EnterCriticalSection(&m_pDoc->m_csExecCommand);
				m_pDoc->m_sExecCommand = m_sExecCommand;
				m_pDoc->m_sExecParams = m_sExecParams;
				::LeaveCriticalSection(&m_pDoc->m_csExecCommand);

				// Fill flags and mode
				m_pDoc->m_bHideExecCommand = m_bHideExecCommand = TRUE;
				m_pDoc->m_bWaitExecCommand = m_bWaitExecCommand = FALSE;
				m_pDoc->m_nExecCommandMode = m_nExecCommandMode = 1;
				m_pDoc->m_bExecCommand = m_bExecCommand = TRUE;

				// Update data from vars to view
				UpdateData(FALSE);
			}
		}
	}
}

#endif

