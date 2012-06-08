// MovementDetectionPage.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "MainFrm.h"
#include "MovementDetectionPage.h"
#include "VideoDeviceDoc.h"
#include "VideoDeviceView.h"
#include "ResizingDlg.h"
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
	DDV_MinMaxInt(pDX, m_nSecondsBeforeMovementBegin, 1, 99);
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
	ON_BN_CLICKED(IDC_EXEC_MOVEMENT_DETECTION, OnExecMovementDetection)
	ON_EN_CHANGE(IDC_EDIT_EXE, OnChangeEditExe)
	ON_EN_CHANGE(IDC_EDIT_PARAMS, OnChangeEditParams)
	ON_BN_CLICKED(IDC_CHECK_HIDE_EXEC_COMMAND, OnCheckHideExecCommand)
	ON_BN_CLICKED(IDC_CHECK_WAIT_EXEC_COMMAND, OnCheckWaitExecCommand)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_CHECK_LUMCHANGE_DET, OnCheckLumChangeDet)
	ON_CBN_SELCHANGE(IDC_DETECTION_ZONE_SIZE, OnSelchangeDetectionZoneSize)
	ON_EN_CHANGE(IDC_DETECTION_TRIGGER_FILENAME, OnChangeDetectionTriggerFilename)
	ON_CBN_SELCHANGE(IDC_EXECMODE_MOVEMENT_DETECTION, OnSelchangeExecmodeMovementDetection)
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

	// Init Combo Boxes
	CComboBox* pComboBoxZoneSize = (CComboBox*)GetDlgItem(IDC_DETECTION_ZONE_SIZE);
	pComboBoxZoneSize->AddString(ML_STRING(1836, "Big"));
	pComboBoxZoneSize->AddString(ML_STRING(1837, "Medium"));
	pComboBoxZoneSize->AddString(ML_STRING(1838, "Small"));
	CComboBox* pComboBoxExexMode = (CComboBox*)GetDlgItem(IDC_EXECMODE_MOVEMENT_DETECTION);
	pComboBoxExexMode->AddString(ML_STRING(1842, "On first movement frame"));
	pComboBoxExexMode->AddString(ML_STRING(1843, "After Save,Email,Ftp"));

	// This calls UpdateData(FALSE)
	CPropertyPage::OnInitDialog();

	// Detection Dir
	m_DirLabel.SubclassDlgItem(IDC_TEXT_VIDEO_DET, this);
	m_DirLabel.SetVisitedColor(RGB(0, 0, 255));
	m_DirLabel.SetLink(m_pDoc->m_sDetectionAutoSaveDir);
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_DETECTION_SAVEAS_PATH);
	pEdit->SetWindowText(m_pDoc->m_sDetectionAutoSaveDir);

	// The external detection trigger file name
	pEdit = (CEdit*)GetDlgItem(IDC_DETECTION_TRIGGER_FILENAME);
	pEdit->SetWindowText(m_pDoc->m_sDetectionTriggerFileName);
	
	// Movement Detection Save Seconds Before & After Detection Spin Controls
	m_SpinSecondsBeforeMovementBegin.SetRange(1, 99);
	m_SpinSecondsAfterMovementEnd.SetRange(1, 99);

	// Detection Level Slider & Edit Controls
	m_DetectionLevel.SetRange(1, 100, TRUE);
	m_DetectionLevel.SetPos(m_pDoc->m_nDetectionLevel);
	pEdit = (CEdit*)GetDlgItem(IDC_DETECTION_LEVEL_NUM);
	CString sLevel;
	sLevel.Format(_T("%i"), m_pDoc->m_nDetectionLevel);
	pEdit->SetWindowText(sLevel);

	// Detection Zone Size
	pComboBoxZoneSize->SetCurSel(m_pDoc->m_nDetectionZoneSize);

	// Detection Scheduler Check Box
	CButton* pCheckScheduler = (CButton*)GetDlgItem(IDC_CHECK_SCHEDULER_DAILY);
	pCheckScheduler->SetCheck(m_pDoc->m_bDetectionStartStop);

	// Adjacent Zones Detection
	CButton* pCheckAdjacentZonesDetection = (CButton*)GetDlgItem(IDC_CHECK_ADJACENT_ZONES_DET);
	pCheckAdjacentZonesDetection->SetCheck(m_pDoc->m_bDoAdjacentZonesDetection ? 1 : 0);

	// Discard movement detection if a luminosity change happens
	CButton* pCheckLumChangeDetection = (CButton*)GetDlgItem(IDC_CHECK_LUMCHANGE_DET);
	pCheckLumChangeDetection->SetCheck(m_pDoc->m_bDoLumChangeDetection ? 1 : 0);

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
	UpdateExecHelp();

	// Update Detection State
	UpdateDetectionState();

	// Set Page Pointer to this
	m_pDoc->m_pMovementDetectionPage = this;
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMovementDetectionPage::UpdateDetectionState()
{
	// Detection Mode
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_DETECTION_MODE);
	if (pEdit)
	{
		CString sDetectionMode;
		switch (m_pDoc->m_dwVideoProcessorMode)
		{
			case NO_DETECTOR :						sDetectionMode = ML_STRING(1845, "Off"); break;
			case TRIGGER_FILE_DETECTOR :			sDetectionMode = ML_STRING(1846, "Trigger File"); break;
			case SOFTWARE_MOVEMENT_DETECTOR :		sDetectionMode = ML_STRING(1847, "Software"); break;
			case (	TRIGGER_FILE_DETECTOR |
					SOFTWARE_MOVEMENT_DETECTOR):	sDetectionMode = ML_STRING(1848, "Trigger File + Software"); break;
			default: break;
		}
		pEdit->SetWindowText(sDetectionMode);
	}

	// Warning
	pEdit = (CEdit*)GetDlgItem(IDC_WARNING);
	if (pEdit)
	{
		if ((m_pDoc->m_dwVideoProcessorMode & SOFTWARE_MOVEMENT_DETECTOR) &&
			m_pDoc->m_bUnsupportedVideoSizeForMovDet)
			pEdit->ShowWindow(TRUE);
		else
			pEdit->ShowWindow(FALSE);
	}
}

void CMovementDetectionPage::OnDestroy() 
{
	CPropertyPage::OnDestroy();
	// Set Page Pointer to NULL
	m_pDoc->m_pMovementDetectionPage = NULL;
}

void CMovementDetectionPage::OnDetectionSaveas() 
{
	// Stop Threads
	m_pDoc->m_SaveFrameListThread.Kill();
	m_pDoc->m_DeleteThread.Kill();

	CBrowseDlg dlg(	::AfxGetMainFrame(),
					&m_pDoc->m_sDetectionAutoSaveDir,
					ML_STRING(1399, "Select Folder For Detection Saving"),
					TRUE);
	dlg.DoModal();
	m_DirLabel.SetLink(m_pDoc->m_sDetectionAutoSaveDir);
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_DETECTION_SAVEAS_PATH);
	pEdit->SetWindowText(m_pDoc->m_sDetectionAutoSaveDir);

	// Restart Threads
	m_pDoc->m_DeleteThread.Start(THREAD_PRIORITY_LOWEST);
	m_pDoc->m_SaveFrameListThread.Start();
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
		BOOL bRunning = m_pDoc->m_DeleteThread.IsRunning();
		if (bRunning)
			m_pDoc->m_DeleteThread.Kill();
		m_pDoc->m_nDeleteDetectionsOlderThanDays = m_nDeleteDetectionsOlderThanDays;
		if (bRunning)
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
			(SB_PAGEDOWN == nSBCode)		||	// Mouse Press Below Slider
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

void CMovementDetectionPage::OnSelchangeDetectionZoneSize() 
{
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_DETECTION_ZONE_SIZE);
	m_pDoc->m_nDetectionZoneSize = pComboBox->GetCurSel();
}

void CMovementDetectionPage::UpdateExecHelp() 
{
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_EXEC_HELP);
	CString s;
	if (m_pDoc->m_nExecModeMovementDetection == 0)
		s.Format(ML_STRING(1719, "Example to play a sound on detection\r\nCmd\tmplay32.exe (or path to %s)\r\nParams\t/play /close \"audio file path\""), APPNAME_EXT);
	else
		s = ML_STRING(1862, "Params can include case sensitive variables\r\n%sec% %min% %hour% %day% %month% %year%\r\n%avi% %gif% %swf% %counter%");
	pEdit->SetWindowText(s);
}

void CMovementDetectionPage::OnSelchangeExecmodeMovementDetection() 
{
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_EXECMODE_MOVEMENT_DETECTION);
	m_pDoc->m_nExecModeMovementDetection = pComboBox->GetCurSel();
	UpdateExecHelp();
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

void CMovementDetectionPage::OnCheckLumChangeDet() 
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_LUMCHANGE_DET);
	m_pDoc->m_bDoLumChangeDetection = pCheck->GetCheck() > 0;
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
	VideoFormatDlg.m_bShowRawChoose = FALSE;
	VideoFormatDlg.m_nFileType = CVideoFormatDlg::FILETYPE_SWF;
	if (VideoFormatDlg.DoModal() == IDOK)
	{
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
	VideoFormatDlg.m_bShowRawChoose = FALSE;
	if (VideoFormatDlg.DoModal() == IDOK)
	{
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

	CResizingDlg dlg(m_pDoc->m_DocRect.Width(), m_pDoc->m_DocRect.Height(),
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
	CSendMailConfigurationDlg dlg;
	dlg.m_SendMailConfiguration = m_pDoc->m_MovDetSendMailConfiguration;
	if (dlg.DoModal() == IDOK)
		m_pDoc->m_MovDetSendMailConfiguration = dlg.m_SendMailConfiguration;

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
	CFTPUploadConfigurationDlg dlg(IDD_MOVDET_FTP_CONFIGURATION);
	dlg.m_FTPUploadConfiguration = m_pDoc->m_MovDetFTPUploadConfiguration;
	if (dlg.DoModal() == IDOK)
		m_pDoc->m_MovDetFTPUploadConfiguration = dlg.m_FTPUploadConfiguration;

	// Restart Save Frame List Thread
	m_pDoc->m_SaveFrameListThread.Start();
}

void CMovementDetectionPage::OnExecMovementDetection() 
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_EXEC_MOVEMENT_DETECTION);
	m_pDoc->m_bExecCommandMovementDetection = pCheck->GetCheck() > 0;
}

void CMovementDetectionPage::OnChangeDetectionTriggerFilename() 
{
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_DETECTION_TRIGGER_FILENAME);
	pEdit->GetWindowText(m_pDoc->m_sDetectionTriggerFileName);
}

void CMovementDetectionPage::OnChangeEditExe() 
{
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_EXE);
	::EnterCriticalSection(&m_pDoc->m_csExecCommandMovementDetection);
	pEdit->GetWindowText(m_pDoc->m_sExecCommandMovementDetection);
	::LeaveCriticalSection(&m_pDoc->m_csExecCommandMovementDetection);
}

void CMovementDetectionPage::OnChangeEditParams() 
{
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PARAMS);
	::EnterCriticalSection(&m_pDoc->m_csExecCommandMovementDetection);
	pEdit->GetWindowText(m_pDoc->m_sExecParamsMovementDetection);
	::LeaveCriticalSection(&m_pDoc->m_csExecCommandMovementDetection);
}

void CMovementDetectionPage::OnCheckHideExecCommand() 
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_HIDE_EXEC_COMMAND);
	::EnterCriticalSection(&m_pDoc->m_csExecCommandMovementDetection);
	m_pDoc->m_bHideExecCommandMovementDetection = pCheck->GetCheck() > 0;
	::LeaveCriticalSection(&m_pDoc->m_csExecCommandMovementDetection);
}

void CMovementDetectionPage::OnCheckWaitExecCommand() 
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_WAIT_EXEC_COMMAND);
	::EnterCriticalSection(&m_pDoc->m_csExecCommandMovementDetection);
	m_pDoc->m_bWaitExecCommandMovementDetection = pCheck->GetCheck() > 0;
	::LeaveCriticalSection(&m_pDoc->m_csExecCommandMovementDetection);
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

