// MovementDetectionPage.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "MainFrm.h"
#include "MovementDetectionPage.h"
#include "VideoDeviceDoc.h"
#include "VideoDeviceView.h"
#include "MovDetAnimGifConfigurationDlg.h"
#include "VideoFormatDlg.h"
#include "SendMailConfigurationDlg.h"
#include "FTPUploadConfigurationDlg.h"
#include "BrowseDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef VIDEODEVICEDOC

/////////////////////////////////////////////////////////////////////////////
// CMovementDetectionPage property page

IMPLEMENT_DYNCREATE(CMovementDetectionPage, CPropertyPage)

CMovementDetectionPage::CMovementDetectionPage()
	: CPropertyPage(CMovementDetectionPage::IDD)
{
	// OnInitDialog() is called when first pressing the tab
	// OnInitDialog() inits the property page pointer in the doc
	// -> Move all inits to OnInitDialog() because the assistant
	// may change the doc vars between construction and OnInitDialog() call
	//{{AFX_DATA_INIT(CMovementDetectionPage)
	//}}AFX_DATA_INIT
	m_pDoc = NULL;
}

void CMovementDetectionPage::SetDoc(CVideoDeviceDoc* pDoc)
{
	ASSERT(pDoc);
	m_pDoc = pDoc;
}

CMovementDetectionPage::~CMovementDetectionPage()
{
}

void CMovementDetectionPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMovementDetectionPage)
	DDX_Text(pDX, IDC_SECONDS_AFTER_MOVEMENT_END, m_nSecondsAfterMovementEnd);
	DDV_MinMaxInt(pDX, m_nSecondsAfterMovementEnd, 1, 99);
	DDX_Text(pDX, IDC_SECONDS_BEFORE_MOVEMENT_BEGIN, m_nSecondsBeforeMovementBegin);
	DDV_MinMaxInt(pDX, m_nSecondsBeforeMovementBegin, 1, 9);
	DDX_Control(pDX, IDC_SPIN_SECONDS_BEFORE_MOVEMENT_BEGIN, m_SpinSecondsBeforeMovementBegin);
	DDX_Control(pDX, IDC_SPIN_SECONDS_AFTER_MOVEMENT_END, m_SpinSecondsAfterMovementEnd);
	DDX_Control(pDX, IDC_DETECTION_LEVEL, m_DetectionLevel);
	DDX_DateTimeCtrl(pDX, IDC_TIME_DAILY_START, m_DetectionStartTime);
	DDX_DateTimeCtrl(pDX, IDC_TIME_DAILY_STOP, m_DetectionStopTime);
	DDX_Text(pDX, IDC_EDIT_DELETE_DETECTIONS_DAYS, m_nDeleteDetectionsOlderThanDays);
	DDV_MinMaxInt(pDX, m_nDeleteDetectionsOlderThanDays, 0, 4000000);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMovementDetectionPage, CPropertyPage)
	//{{AFX_MSG_MAP(CMovementDetectionPage)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_DETECTION_SAVEAS, OnDetectionSaveas)
	ON_BN_CLICKED(IDC_CHECK_VIDEO_DETECTION_MOVEMENT, OnCheckVideoDetectionMovement)
	ON_BN_CLICKED(IDC_CHECK_DET_PREVIEW, OnCheckDetPreview)
	ON_EN_CHANGE(IDC_SECONDS_AFTER_MOVEMENT_END, OnChangeSecondsAfterMovementEnd)
	ON_EN_CHANGE(IDC_SECONDS_BEFORE_MOVEMENT_BEGIN, OnChangeSecondsBeforeMovementBegin)
	ON_WM_HSCROLL()
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_DETECTION_LEVEL, OnReleasedcaptureDetectionLevel)
	ON_BN_CLICKED(IDC_SAVE_AVI_MOVEMENT_DETECTION, OnSaveAviMovementDetection)
	ON_BN_CLICKED(IDC_ANIMATEDGIF_SIZE, OnAnimatedgifSize)
	ON_BN_CLICKED(IDC_SAVE_ANIMATEDGIF_MOVEMENT_DETECTION, OnSaveAnimGifMovementDetection)
	ON_BN_CLICKED(IDC_SENDMAIL_CONFIGURE, OnSendmailConfigure)
	ON_BN_CLICKED(IDC_SENDMAIL_MOVEMENT_DETECTION, OnSendmailMovementDetection)
	ON_BN_CLICKED(IDC_FTP_MOVEMENT_DETECTION, OnFtpMovementDetection)
	ON_BN_CLICKED(IDC_FTP_CONFIGURE, OnFtpConfigure)
	ON_BN_CLICKED(IDC_AVI_CONFIGURE, OnAviConfigure)
	ON_BN_CLICKED(IDC_CHECK_SCHEDULER_DAILY, OnCheckSchedulerDaily)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_TIME_DAILY_START, OnDatetimechangeTimeDailyStart)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_TIME_DAILY_STOP, OnDatetimechangeTimeDailyStop)
	ON_BN_CLICKED(IDC_SAVE_SWF_MOVEMENT_DETECTION, OnSaveSwfMovementDetection)
	ON_BN_CLICKED(IDC_CHECK_ADJACENT_ZONES_DET, OnCheckAdjacentZonesDet)
	ON_EN_CHANGE(IDC_EDIT_DELETE_DETECTIONS_DAYS, OnChangeEditDeleteDetectionsDays)
	ON_BN_CLICKED(IDC_SWF_CONFIGURE, OnSwfConfigure)
	ON_BN_CLICKED(IDC_CHECK_FALSE_DET, OnCheckFalseDet)
	ON_EN_CHANGE(IDC_EDIT_FALSE_DET_BLUE, OnChangeEditFalseDetBlue)
	ON_EN_CHANGE(IDC_EDIT_FALSE_DET_NONEBLUE, OnChangeEditFalseDetNoneblue)
	ON_CBN_SELENDOK(IDC_COMBO_FALSE_DET_ANDOR, OnSelendokComboFalseDetAndor)
	ON_BN_CLICKED(IDC_EXEC_MOVEMENT_DETECTION, OnExecMovementDetection)
	ON_EN_CHANGE(IDC_EDIT_EXE, OnChangeEditExe)
	ON_EN_CHANGE(IDC_EDIT_PARAMS, OnChangeEditParams)
	ON_BN_CLICKED(IDC_CHECK_HIDE_EXEC_COMMAND, OnCheckHideExecCommand)
	ON_BN_CLICKED(IDC_CHECK_WAIT_EXEC_COMMAND, OnCheckWaitExecCommand)
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMovementDetectionPage message handlers

BOOL CMovementDetectionPage::OnInitDialog() 
{
	// Init vars
	m_nSecondsBeforeMovementBegin = m_pDoc->m_nMilliSecondsRecBeforeMovementBegin / 1000;
	m_nSecondsAfterMovementEnd = m_pDoc->m_nMilliSecondsRecAfterMovementEnd / 1000;
	m_DetectionStartTime = m_pDoc->m_DetectionStartTime;
	m_DetectionStopTime = m_pDoc->m_DetectionStopTime;
	m_nDeleteDetectionsOlderThanDays = m_pDoc->m_nDeleteDetectionsOlderThanDays;

	// Init Combo Box
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_FALSE_DET_ANDOR);
	if (pComboBox)
	{
		pComboBox->AddString(ML_STRING(1556, "and"));
		pComboBox->AddString(ML_STRING(1557, "or"));
	}

	// This calls UpdateData(FALSE)
	CPropertyPage::OnInitDialog();

	// Movement Detector Check Box
	CButton* pCheckMovement = (CButton*)GetDlgItem(IDC_CHECK_VIDEO_DETECTION_MOVEMENT);
	if (m_pDoc->m_VideoProcessorMode & MOVEMENT_DETECTOR)
		pCheckMovement->SetCheck(1);
	else
		pCheckMovement->SetCheck(0);

	// Detection Dir
	m_DirLabel.SubclassDlgItem(IDC_TEXT_VIDEO_DET, this);
	m_DirLabel.SetVisitedColor(RGB(0, 0, 255));
	m_DirLabel.SetLink(m_pDoc->m_sDetectionAutoSaveDir);
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_DETECTION_SAVEAS_PATH);
	pEdit->SetWindowText(m_pDoc->m_sDetectionAutoSaveDir);
	
	// Movement Detection Save Seconds Before & After Detection Spin Controls
	m_SpinSecondsBeforeMovementBegin.SetRange(1, 9);
	m_SpinSecondsAfterMovementEnd.SetRange(1, 99);

	// Movement Detector Mix Preview Check Box
	CButton* pCheckPreview = (CButton*)GetDlgItem(IDC_CHECK_DET_PREVIEW);
	pCheckPreview->SetCheck(m_pDoc->m_bMovementDetectorPreview);
#ifndef _DEBUG
	pCheckPreview->ShowWindow(SW_HIDE);
#endif

	// Detection Level Slider & Edit Controls
	m_DetectionLevel.SetRange(1, 100, TRUE);
	m_DetectionLevel.SetPos(m_pDoc->m_nDetectionLevel);
	pEdit = (CEdit*)GetDlgItem(IDC_DETECTION_LEVEL_NUM);
	CString sLevel;
	sLevel.Format(_T("%i"), m_pDoc->m_nDetectionLevel);
	pEdit->SetWindowText(sLevel);

	// Detection Scheduler Check Box
	CButton* pCheckScheduler = (CButton*)GetDlgItem(IDC_CHECK_SCHEDULER_DAILY);
	pCheckScheduler->SetCheck(m_pDoc->m_bDetectionStartStop);

	// Adjacent Zones Detection
	CButton* pCheckAdjacentZonesDetection = (CButton*)GetDlgItem(IDC_CHECK_ADJACENT_ZONES_DET);
	pCheckAdjacentZonesDetection->SetCheck(m_pDoc->m_bDoAdjacentZonesDetection ? 1 : 0);
	
	// False Detection Check
	CButton* pCheckFalseDetection = (CButton*)GetDlgItem(IDC_CHECK_FALSE_DET);
	pCheckFalseDetection->SetCheck(m_pDoc->m_bDoFalseDetectionCheck ? 1 : 0);

	// False detection AND / OR
	if (pComboBox)
	{
		if (m_pDoc->m_bDoFalseDetectionAnd)
			pComboBox->SetCurSel(0);
		else
			pComboBox->SetCurSel(1);
	}

	// False Detection Thresholds
	CEdit* pEditFalseDetBlueThreshold = (CEdit*)GetDlgItem(IDC_EDIT_FALSE_DET_BLUE);
	CString sFalseDetBlueThreshold;
	sFalseDetBlueThreshold.Format(_T("%d"), m_pDoc->m_nFalseDetectionBlueThreshold);
	pEditFalseDetBlueThreshold->SetWindowText(sFalseDetBlueThreshold);
	CEdit* pEditFalseDetNoneBlueThreshold = (CEdit*)GetDlgItem(IDC_EDIT_FALSE_DET_NONEBLUE);
	CString sFalseDetNoneBlueThreshold;
	sFalseDetNoneBlueThreshold.Format(_T("%d"), m_pDoc->m_nFalseDetectionNoneBlueThreshold);
	pEditFalseDetNoneBlueThreshold->SetWindowText(sFalseDetNoneBlueThreshold);

	// Save SWF Movement Detection Check Box
	CButton* pCheckSWFSaveMovementDetection = (CButton*)GetDlgItem(IDC_SAVE_SWF_MOVEMENT_DETECTION);
	if (m_pDoc->m_bSaveSWFMovementDetection)
		pCheckSWFSaveMovementDetection->SetCheck(1);
	else
		pCheckSWFSaveMovementDetection->SetCheck(0);

	// Save AVI Movement Detection Check Box
	CButton* pCheckAVISaveMovementDetection = (CButton*)GetDlgItem(IDC_SAVE_AVI_MOVEMENT_DETECTION);
	if (m_pDoc->m_bSaveAVIMovementDetection)
		pCheckAVISaveMovementDetection->SetCheck(1);
	else
		pCheckAVISaveMovementDetection->SetCheck(0);

	// Save Animated GIF Movement Detection Check Box
	CButton* pCheckAnimGIFSaveMovementDetection = (CButton*)GetDlgItem(IDC_SAVE_ANIMATEDGIF_MOVEMENT_DETECTION);
	if (m_pDoc->m_bSaveAnimGIFMovementDetection)
		pCheckAnimGIFSaveMovementDetection->SetCheck(1);
	else
		pCheckAnimGIFSaveMovementDetection->SetCheck(0);

	// Send Email Movement Detection Check Box
	CButton* pCheckSendMailMovementDetection = (CButton*)GetDlgItem(IDC_SENDMAIL_MOVEMENT_DETECTION);
	if (m_pDoc->m_bSendMailMovementDetection)
		pCheckSendMailMovementDetection->SetCheck(1);
	else
		pCheckSendMailMovementDetection->SetCheck(0);

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

	// On detection execution example
	CStatic* pStatic = (CStatic*)GetDlgItem(IDC_STATIC2301);
	CString s;
	s.Format(ML_STRING(1719, "Example to play a sound on detection\nCmd\tmplay32.exe (or path to %s)\nParams\t/play /close \"audio file path\""), APPNAME_EXT);
	pStatic->SetWindowText(s);

	// Warning
	CEdit* pEditWarning = (CEdit*)GetDlgItem(IDC_WARNING);
	if ((m_pDoc->m_VideoProcessorMode & MOVEMENT_DETECTOR) &&
		m_pDoc->m_bUnsupportedVideoSizeForMovDet)
		pEditWarning->ShowWindow(TRUE);
	else
		pEditWarning->ShowWindow(FALSE);

	// Set Page Pointer to this
	m_pDoc->m_pMovementDetectionPage = this;
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMovementDetectionPage::OnDestroy() 
{
	CPropertyPage::OnDestroy();
	// Set Page Pointer to NULL
	m_pDoc->m_pMovementDetectionPage = NULL;
}

void CMovementDetectionPage::OnDetectionSaveas() 
{
	// Stop Save Frame List Thread
	m_pDoc->m_SaveFrameListThread.Kill();

	// Detection Dir Dialog
	CBrowseDlg dlg(	::AfxGetMainFrame(),
					&m_pDoc->m_sDetectionAutoSaveDir,
					ML_STRING(1399, "Select Folder For Detection Saving"),
					TRUE);
	dlg.DoModal();
	m_DirLabel.SetLink(m_pDoc->m_sDetectionAutoSaveDir);
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_DETECTION_SAVEAS_PATH);
	pEdit->SetWindowText(m_pDoc->m_sDetectionAutoSaveDir);

	// Restart Save Frame List Thread
	m_pDoc->m_SaveFrameListThread.Start();
}

void CMovementDetectionPage::OnCheckVideoDetectionMovement() 
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_VIDEO_DETECTION_MOVEMENT);
	if (pCheck->GetCheck())
		m_pDoc->m_VideoProcessorMode |= MOVEMENT_DETECTOR;
	else
		m_pDoc->m_VideoProcessorMode &= ~MOVEMENT_DETECTOR;
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_WARNING);
	if ((m_pDoc->m_VideoProcessorMode & MOVEMENT_DETECTOR) &&
		m_pDoc->m_bUnsupportedVideoSizeForMovDet)
		pEdit->ShowWindow(TRUE);
	else
		pEdit->ShowWindow(FALSE);
}

HBRUSH CMovementDetectionPage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr; 

	switch (nCtlColor) 
	{ 
		// For Read/Write Edit Controls
		case CTLCOLOR_EDIT:
		case CTLCOLOR_MSGBOX:
			hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
			break;

		// Process Static text, READONLY controls, DISABLED * controls.
		//   NOTE: Disabled controls can NOT have their text color
		//         changed.
		//         Suggest you change all your DISABLED controls to
		//         READONLY.
		case CTLCOLOR_STATIC:
			switch (pWnd->GetDlgCtrlID())
			{     
				case IDC_WARNING :
					pDC->SetBkMode(TRANSPARENT);
					pDC->SetTextColor(RGB(0xFF,0,0));
					hbr = (HBRUSH)::GetStockObject(NULL_BRUSH);
					break;
		
				default:
					hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
					break;
			}
			break;

		// Otherwise, do default handling of OnCtlColor
		default:
			hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
			break;
	}

	return hbr; // return brush
}

void CMovementDetectionPage::OnCheckDetPreview() 
{	
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_DET_PREVIEW);
	if (pCheck->GetCheck())
		m_pDoc->m_bMovementDetectorPreview = TRUE;
	else
		m_pDoc->m_bMovementDetectorPreview = FALSE;
}

void CMovementDetectionPage::OnChangeSecondsAfterMovementEnd() 
{
	if (::IsWindow(m_SpinSecondsAfterMovementEnd.GetSafeHwnd()))
	{
		if (UpdateData(TRUE))
		{
			m_pDoc->m_nMilliSecondsRecAfterMovementEnd = m_nSecondsAfterMovementEnd * 1000;
			m_pDoc->ResetMovementDetector();
		}
	}
}

void CMovementDetectionPage::OnChangeSecondsBeforeMovementBegin() 
{
	if (::IsWindow(m_SpinSecondsBeforeMovementBegin.GetSafeHwnd()))
	{
		if (UpdateData(TRUE))
		{
			m_pDoc->m_nMilliSecondsRecBeforeMovementBegin = m_nSecondsBeforeMovementBegin * 1000;
			m_pDoc->ResetMovementDetector();
		}
	}
}

void CMovementDetectionPage::OnChangeEditDeleteDetectionsDays() 
{
	if (UpdateData(TRUE))
	{
		m_pDoc->m_DeleteThread.Kill();
		m_pDoc->m_nDeleteDetectionsOlderThanDays = m_nDeleteDetectionsOlderThanDays;
		m_pDoc->m_DeleteThread.Start(THREAD_PRIORITY_LOWEST);
	}
}

void CMovementDetectionPage::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if (pScrollBar)
	{
		CSliderCtrl* pSlider = (CSliderCtrl*)pScrollBar;

		if ((SB_THUMBTRACK == nSBCode)		||	// Dragging Slider
			(SB_THUMBPOSITION == nSBCode)	||	// Wheel On Mouse And End Of Dragging Slider
			(SB_LINEUP  == nSBCode)			||	// Keyboard Arrow
			(SB_LINEDOWN  == nSBCode)		||	// Keyboard Arrow
			(SB_PAGEUP == nSBCode)			||	// Mouse Press Above Slider
			(SB_PAGEDOWN == nSBCode)		||	// Mouse Press Belove Slider
			(SB_LEFT == nSBCode)			||	// Home Button
			(SB_RIGHT == nSBCode))				// End Button  
		{
			if (pSlider->GetDlgCtrlID() == IDC_DETECTION_LEVEL)
			{
				m_pDoc->m_nDetectionLevel = m_DetectionLevel.GetPos();
				m_pDoc->m_nMovementDetectorIntensityLimit = 50 - m_pDoc->m_nDetectionLevel / 2;
				CEdit* pEdit = (CEdit*)GetDlgItem(IDC_DETECTION_LEVEL_NUM);
				CString sLevel;
				sLevel.Format(_T("%i"), m_DetectionLevel.GetPos());
				pEdit->SetWindowText(sLevel);
			}
		}
	}

	CPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CMovementDetectionPage::OnReleasedcaptureDetectionLevel(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UpdateData(TRUE);
	*pResult = 0;
}

void CMovementDetectionPage::OnCheckAdjacentZonesDet() 
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_ADJACENT_ZONES_DET);
	m_pDoc->m_bDoAdjacentZonesDetection = pCheck->GetCheck() > 0;
}

void CMovementDetectionPage::OnCheckFalseDet() 
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_FALSE_DET);
	m_pDoc->m_bDoFalseDetectionCheck = pCheck->GetCheck() > 0;
}

void CMovementDetectionPage::OnSelendokComboFalseDetAndor() 
{
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_FALSE_DET_ANDOR);
	if (pComboBox->GetCurSel() == 0)
		m_pDoc->m_bDoFalseDetectionAnd = TRUE;
	else
		m_pDoc->m_bDoFalseDetectionAnd = FALSE;
}

void CMovementDetectionPage::OnChangeEditFalseDetBlue() 
{
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_FALSE_DET_BLUE);
	CString sFalseDetBlueThreshold;
	pEdit->GetWindowText(sFalseDetBlueThreshold);
	int nFalseDetBlueThreshold = _ttoi(sFalseDetBlueThreshold);
	if (nFalseDetBlueThreshold >= 0)
		m_pDoc->m_nFalseDetectionBlueThreshold = nFalseDetBlueThreshold;
}

void CMovementDetectionPage::OnChangeEditFalseDetNoneblue() 
{
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_FALSE_DET_NONEBLUE);
	CString sFalseDetNoneBlueThreshold;
	pEdit->GetWindowText(sFalseDetNoneBlueThreshold);
	int nFalseDetNoneBlueThreshold = _ttoi(sFalseDetNoneBlueThreshold);
	if (nFalseDetNoneBlueThreshold >= 0)
		m_pDoc->m_nFalseDetectionNoneBlueThreshold = nFalseDetNoneBlueThreshold;
}

void CMovementDetectionPage::OnSaveSwfMovementDetection() 
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_SAVE_SWF_MOVEMENT_DETECTION);
	m_pDoc->m_bSaveSWFMovementDetection = pCheck->GetCheck() > 0;
}

void CMovementDetectionPage::OnSaveAviMovementDetection() 
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_SAVE_AVI_MOVEMENT_DETECTION);
	m_pDoc->m_bSaveAVIMovementDetection = pCheck->GetCheck() > 0;
}

void CMovementDetectionPage::OnSaveAnimGifMovementDetection() 
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_SAVE_ANIMATEDGIF_MOVEMENT_DETECTION);
	m_pDoc->m_bSaveAnimGIFMovementDetection = pCheck->GetCheck() > 0;
}

void CMovementDetectionPage::OnSwfConfigure() 
{
	// Stop Save Frame List Thread
	m_pDoc->m_SaveFrameListThread.Kill();

	// Swf Config Dialog
	CVideoFormatDlg VideoFormatDlg(this);
	VideoFormatDlg.m_dwVideoCompressorFourCC = m_pDoc->m_dwVideoDetSwfFourCC;
	VideoFormatDlg.m_nVideoCompressorDataRate = m_pDoc->m_nVideoDetSwfDataRate / 1000;
	VideoFormatDlg.m_nVideoCompressorKeyframesRate = m_pDoc->m_nVideoDetSwfKeyframesRate;
	VideoFormatDlg.m_fVideoCompressorQuality = m_pDoc->m_fVideoDetSwfQuality;
	VideoFormatDlg.m_nQualityBitrate = m_pDoc->m_nVideoDetSwfQualityBitrate;
	VideoFormatDlg.m_bDeinterlace = m_pDoc->m_bVideoDetSwfDeinterlace;
	VideoFormatDlg.m_bShowRawChoose = FALSE;
	VideoFormatDlg.m_nFileType = CVideoFormatDlg::FILETYPE_SWF;
	if (VideoFormatDlg.DoModal() == IDOK)
	{
		m_pDoc->m_bVideoDetSwfDeinterlace = VideoFormatDlg.m_bDeinterlace;
		m_pDoc->m_nVideoDetSwfQualityBitrate = VideoFormatDlg.m_nQualityBitrate;
		m_pDoc->m_fVideoDetSwfQuality = VideoFormatDlg.m_fVideoCompressorQuality;
		m_pDoc->m_nVideoDetSwfDataRate = VideoFormatDlg.m_nVideoCompressorDataRate * 1000;
		m_pDoc->m_nVideoDetSwfKeyframesRate = VideoFormatDlg.m_nVideoCompressorKeyframesRate;
		m_pDoc->m_dwVideoDetSwfFourCC = VideoFormatDlg.m_dwVideoCompressorFourCC;
	}

	// Restart Save Frame List Thread
	m_pDoc->m_SaveFrameListThread.Start();
}

void CMovementDetectionPage::OnAviConfigure() 
{
	// Stop Save Frame List Thread
	m_pDoc->m_SaveFrameListThread.Kill();

	// Avi Config Dialog
	CVideoFormatDlg VideoFormatDlg(this);
	VideoFormatDlg.m_dwVideoCompressorFourCC = m_pDoc->m_dwVideoDetFourCC;
	VideoFormatDlg.m_nVideoCompressorDataRate = m_pDoc->m_nVideoDetDataRate / 1000;
	VideoFormatDlg.m_nVideoCompressorKeyframesRate = m_pDoc->m_nVideoDetKeyframesRate;
	VideoFormatDlg.m_fVideoCompressorQuality = m_pDoc->m_fVideoDetQuality;
	VideoFormatDlg.m_nQualityBitrate = m_pDoc->m_nVideoDetQualityBitrate;
	VideoFormatDlg.m_bDeinterlace = m_pDoc->m_bVideoDetDeinterlace;
	VideoFormatDlg.m_bShowRawChoose = FALSE;
	if (VideoFormatDlg.DoModal() == IDOK)
	{
		m_pDoc->m_bVideoDetDeinterlace = VideoFormatDlg.m_bDeinterlace;
		m_pDoc->m_nVideoDetQualityBitrate = VideoFormatDlg.m_nQualityBitrate;
		m_pDoc->m_fVideoDetQuality = VideoFormatDlg.m_fVideoCompressorQuality;
		m_pDoc->m_nVideoDetDataRate = VideoFormatDlg.m_nVideoCompressorDataRate * 1000;
		m_pDoc->m_nVideoDetKeyframesRate = VideoFormatDlg.m_nVideoCompressorKeyframesRate;
		m_pDoc->m_dwVideoDetFourCC = VideoFormatDlg.m_dwVideoCompressorFourCC;
	}

	// Restart Save Frame List Thread
	m_pDoc->m_SaveFrameListThread.Start();
}

void CMovementDetectionPage::OnAnimatedgifSize() 
{
	// Stop Save Frame List Thread
	m_pDoc->m_SaveFrameListThread.Kill();

	CMovDetAnimGifConfigurationDlg dlg(	m_pDoc->m_DocRect.Width(), m_pDoc->m_DocRect.Height(),
										(int)m_pDoc->m_dwAnimatedGifWidth, (int)m_pDoc->m_dwAnimatedGifHeight,
										this);
	if (dlg.DoModal() == IDOK)
	{	
		m_pDoc->m_dwAnimatedGifWidth = (DWORD)dlg.m_nPixelsWidth;
		m_pDoc->m_dwAnimatedGifHeight = (DWORD)dlg.m_nPixelsHeight;
	}

	// Restart Save Frame List Thread
	m_pDoc->m_SaveFrameListThread.Start();
}

void CMovementDetectionPage::OnSendmailMovementDetection() 
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_SENDMAIL_MOVEMENT_DETECTION);
	m_pDoc->m_bSendMailMovementDetection = pCheck->GetCheck() > 0;
}

void CMovementDetectionPage::OnSendmailConfigure() 
{
	// Stop Save Frame List Thread
	m_pDoc->m_SaveFrameListThread.Kill();

	// SendMail Config Dialog
	CSendMailConfigurationDlg dlg(m_pDoc);
	dlg.DoModal();

	// Restart Save Frame List Thread
	m_pDoc->m_SaveFrameListThread.Start();
}

void CMovementDetectionPage::OnFtpMovementDetection()
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_FTP_MOVEMENT_DETECTION);
	m_pDoc->m_bFTPUploadMovementDetection = pCheck->GetCheck() > 0;
}

void CMovementDetectionPage::OnFtpConfigure() 
{
	// Stop Save Frame List Thread
	m_pDoc->m_SaveFrameListThread.Kill();

	// FTP Config Dialog
	CFTPUploadConfigurationDlg dlg(	&m_pDoc->m_MovDetFTPUploadConfiguration,
									IDD_MOVDET_FTP_CONFIGURATION);
	dlg.DoModal();

	// Restart Save Frame List Thread
	m_pDoc->m_SaveFrameListThread.Start();
}

void CMovementDetectionPage::OnExecMovementDetection() 
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_EXEC_MOVEMENT_DETECTION);
	m_pDoc->m_bExecCommandMovementDetection = pCheck->GetCheck() > 0;
}

void CMovementDetectionPage::OnChangeEditExe() 
{
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_EXE);
	pEdit->GetWindowText(m_pDoc->m_sExecCommandMovementDetection);
}

void CMovementDetectionPage::OnChangeEditParams() 
{
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PARAMS);
	pEdit->GetWindowText(m_pDoc->m_sExecParamsMovementDetection);
}

void CMovementDetectionPage::OnCheckHideExecCommand() 
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_HIDE_EXEC_COMMAND);
	m_pDoc->m_bHideExecCommandMovementDetection = pCheck->GetCheck() > 0;
}

void CMovementDetectionPage::OnCheckWaitExecCommand() 
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_WAIT_EXEC_COMMAND);
	m_pDoc->m_bWaitExecCommandMovementDetection = pCheck->GetCheck() > 0;
}

void CMovementDetectionPage::UpdateDetectionStartStopTimes()
{
	if (m_pDoc->m_bDetectionStartStop)
	{
		// Start Time
		m_pDoc->m_DetectionStartTime = CTime(	2000,
												1,
												1,
												m_DetectionStartTime.GetHour(),
												m_DetectionStartTime.GetMinute(),
												m_DetectionStartTime.GetSecond());

		// Stop Time
		m_pDoc->m_DetectionStopTime = CTime(	2000,
												1,
												1,
												m_DetectionStopTime.GetHour(),
												m_DetectionStopTime.GetMinute(),
												m_DetectionStopTime.GetSecond());
	}
}

void CMovementDetectionPage::OnCheckSchedulerDaily() 
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_SCHEDULER_DAILY);
	if (pCheck->GetCheck())
		m_pDoc->m_bDetectionStartStop = TRUE;
	else
		m_pDoc->m_bDetectionStartStop = FALSE;
	UpdateDetectionStartStopTimes();
}

void CMovementDetectionPage::OnDatetimechangeTimeDailyStart(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UpdateData(TRUE);
	UpdateDetectionStartStopTimes();
	*pResult = 0;
}

void CMovementDetectionPage::OnDatetimechangeTimeDailyStop(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UpdateData(TRUE);
	UpdateDetectionStartStopTimes();
	*pResult = 0;
}

#endif
