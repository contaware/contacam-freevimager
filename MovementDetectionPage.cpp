// MovementDetectionPage.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "MainFrm.h"
#include "MovementDetectionPage.h"
#include "VideoDeviceDoc.h"
#include "VideoDeviceView.h"
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
	DDX_DateTimeCtrl(pDX, IDC_TIME_DAILY_START, m_DetectionStartTime);
	DDX_DateTimeCtrl(pDX, IDC_TIME_DAILY_STOP, m_DetectionStopTime);
	DDX_Text(pDX, IDC_EDIT_DETECTION_MIN_LENGTH, m_nDetectionMinLengthSeconds);
	DDV_MinMaxInt(pDX, m_nDetectionMinLengthSeconds, 0, 99);
	DDX_Text(pDX, IDC_EDIT_DETECTION_MAX_FRAMES, m_nDetectionMaxFrames);
	DDV_MinMaxInt(pDX, m_nDetectionMaxFrames, 1, MOVDET_MAX_MAX_FRAMES_IN_LIST);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMovementDetectionPage, CPropertyPage)
	//{{AFX_MSG_MAP(CMovementDetectionPage)
	ON_WM_DESTROY()
	ON_EN_CHANGE(IDC_SECONDS_AFTER_MOVEMENT_END, OnChangeSecondsAfterMovementEnd)
	ON_EN_CHANGE(IDC_SECONDS_BEFORE_MOVEMENT_BEGIN, OnChangeSecondsBeforeMovementBegin)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_ANIMATEDGIF_SIZE, OnAnimatedgifSize)
	ON_BN_CLICKED(IDC_SAVE_ANIMATEDGIF_MOVEMENT_DETECTION, OnSaveAnimGifMovementDetection)
	ON_BN_CLICKED(IDC_FTP_MOVEMENT_DETECTION, OnFtpMovementDetection)
	ON_BN_CLICKED(IDC_FTP_CONFIGURE, OnFtpConfigure)
	ON_CBN_SELCHANGE(IDC_COMBOBOX_DETECTION_SCHEDULER, OnCbnSelchangeComboboxDetectionScheduler)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_TIME_DAILY_START, OnDatetimechangeTimeDailyStart)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_TIME_DAILY_STOP, OnDatetimechangeTimeDailyStop)
	ON_BN_CLICKED(IDC_SAVE_VIDEO_MOVEMENT_DETECTION, OnSaveVideoMovementDetection)
	ON_BN_CLICKED(IDC_EXEC_MOVEMENT_DETECTION, OnExecMovementDetection)
	ON_EN_CHANGE(IDC_EDIT_EXE, OnChangeEditExe)
	ON_EN_CHANGE(IDC_EDIT_PARAMS, OnChangeEditParams)
	ON_BN_CLICKED(IDC_CHECK_HIDE_EXEC_COMMAND, OnCheckHideExecCommand)
	ON_BN_CLICKED(IDC_CHECK_WAIT_EXEC_COMMAND, OnCheckWaitExecCommand)
	ON_CBN_SELCHANGE(IDC_EXECMODE_MOVEMENT_DETECTION, OnSelchangeExecmodeMovementDetection)
	ON_BN_CLICKED(IDC_CHECK_SCHEDULER_SUNDAY, OnCheckSchedulerSunday)
	ON_BN_CLICKED(IDC_CHECK_SCHEDULER_MONDAY, OnCheckSchedulerMonday)
	ON_BN_CLICKED(IDC_CHECK_SCHEDULER_TUESDAY, OnCheckSchedulerTuesday)
	ON_BN_CLICKED(IDC_CHECK_SCHEDULER_WEDNESDAY, OnCheckSchedulerWednesday)
	ON_BN_CLICKED(IDC_CHECK_SCHEDULER_THURSDAY, OnCheckSchedulerThursday)
	ON_BN_CLICKED(IDC_CHECK_SCHEDULER_FRIDAY, OnCheckSchedulerFriday)
	ON_BN_CLICKED(IDC_CHECK_SCHEDULER_SATURDAY, OnCheckSchedulerSaturday)
	ON_EN_CHANGE(IDC_EDIT_DETECTION_MIN_LENGTH, OnChangeEditDetectionMinLength)
	ON_EN_CHANGE(IDC_EDIT_DETECTION_MAX_FRAMES, OnChangeEditDetectionMaxFrames)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMovementDetectionPage message handlers

BOOL CMovementDetectionPage::OnInitDialog() 
{
	// Init vars
	m_nSecondsBeforeMovementBegin = m_pDoc->m_nMilliSecondsRecBeforeMovementBegin / 1000;
	m_nSecondsAfterMovementEnd = m_pDoc->m_nMilliSecondsRecAfterMovementEnd / 1000;
	m_nDetectionMinLengthSeconds = m_pDoc->m_nDetectionMinLengthMilliSeconds / 1000;
	m_nDetectionMaxFrames = m_pDoc->m_nDetectionMaxFrames;
	m_DetectionStartTime = m_pDoc->m_DetectionStartTime;
	m_DetectionStopTime = m_pDoc->m_DetectionStopTime;

	// Init Combo Boxes
	CComboBox* pComboBoxDetectionScheduler = (CComboBox*)GetDlgItem(IDC_COMBOBOX_DETECTION_SCHEDULER);
	pComboBoxDetectionScheduler->AddString(ML_STRING(1874, "Detection always enabled (scheduler is off)"));
	pComboBoxDetectionScheduler->AddString(ML_STRING(1875, "Detection enabled:"));
	pComboBoxDetectionScheduler->AddString(ML_STRING(1876, "Detection disabled:"));
	CComboBox* pComboBoxExexMode = (CComboBox*)GetDlgItem(IDC_EXECMODE_MOVEMENT_DETECTION);
	pComboBoxExexMode->AddString(ML_STRING(1842, "After Detection Filter OK"));
	pComboBoxExexMode->AddString(ML_STRING(1843, "After Saving Done"));

	// This calls UpdateData(FALSE)
	CPropertyPage::OnInitDialog();

	// Detection Scheduler
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
	UpdateExecHelp();

	// Set Page Pointer to this
	m_pDoc->m_pMovementDetectionPage = this;
	
	// Set Timer
	SetTimer(ID_TIMER_MOVDETPAGE, MOVDETPAGE_TIMER_MS, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMovementDetectionPage::OnDestroy() 
{
	// Kill timer
	KillTimer(ID_TIMER_MOVDETPAGE);

	// Base class
	CPropertyPage::OnDestroy();

	// Set Page Pointer to NULL
	m_pDoc->m_pMovementDetectionPage = NULL;
}

void CMovementDetectionPage::OnChangeSecondsAfterMovementEnd() 
{
	if (UpdateData(TRUE))
		m_pDoc->m_nMilliSecondsRecAfterMovementEnd = m_nSecondsAfterMovementEnd * 1000;
}

void CMovementDetectionPage::OnChangeSecondsBeforeMovementBegin() 
{
	if (UpdateData(TRUE))
		m_pDoc->m_nMilliSecondsRecBeforeMovementBegin = m_nSecondsBeforeMovementBegin * 1000;
}

void CMovementDetectionPage::OnChangeEditDetectionMinLength() 
{
	if (UpdateData(TRUE))
		m_pDoc->m_nDetectionMinLengthMilliSeconds = m_nDetectionMinLengthSeconds * 1000;
}

void CMovementDetectionPage::OnChangeEditDetectionMaxFrames()
{
	if (UpdateData(TRUE))
		m_pDoc->m_nDetectionMaxFrames = m_nDetectionMaxFrames;
}

void CMovementDetectionPage::OnTimer(UINT nIDEvent)
{
	if (!m_pDoc->m_bClosing)
	{
		// Max Det Video Length
		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_MAX_MOVDET_VIDEOLENGTH);
		if (pEdit)
		{
			CString sMaxDetVideoLengthSec;
			double dEffectiveFrameRate = m_pDoc->m_dEffectiveFrameRate;
			if (dEffectiveFrameRate > 0.0)
				sMaxDetVideoLengthSec.Format(_T("%0.1f"), (double)m_pDoc->m_nDetectionMaxFrames / dEffectiveFrameRate);
			pEdit->SetWindowText(sMaxDetVideoLengthSec);
		}

		// Max Det Queue Size
		pEdit = (CEdit*)GetDlgItem(IDC_MAX_MOVDET_QUEUESIZE);
		if (pEdit)
		{
			CString sMaxDetQueueSizeGB;
			LONG lFrameSize = m_pDoc->m_ProcessFrameBMI.bmiHeader.biSizeImage;
			if (lFrameSize > 0)
				sMaxDetQueueSizeGB.Format(_T("%0.1f"), (double)lFrameSize * m_pDoc->m_nDetectionMaxFrames / 1073741824.0);
			pEdit->SetWindowText(sMaxDetQueueSizeGB);
		}
	}
	CPropertyPage::OnTimer(nIDEvent);
}

void CMovementDetectionPage::UpdateExecHelp() 
{
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_EXEC_HELP);
	CString s;
	if (m_pDoc->m_nExecModeMovementDetection == 0)
		s = ML_STRING(1719, "Example to play a sound on detection\r\nCmd\tPath to vlc.exe\r\nParams\t-I dummy --dummy-quiet --play-and-exit\r\n\t--no-loop --no-repeat\r\n\t\"audio file path\"");
	else
		s = ML_STRING(1862, "Params can include case sensitive variables\r\n%sec% %min% %hour%\r\n%day% %month% %year%\r\n%fullvideo% %smallvideo% %counter%");
	pEdit->SetWindowText(s);
}

void CMovementDetectionPage::OnSelchangeExecmodeMovementDetection() 
{
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_EXECMODE_MOVEMENT_DETECTION);
	m_pDoc->m_nExecModeMovementDetection = pComboBox->GetCurSel();
	UpdateExecHelp();
}

void CMovementDetectionPage::OnSaveVideoMovementDetection() 
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_SAVE_VIDEO_MOVEMENT_DETECTION);
	m_pDoc->m_bSaveVideoMovementDetection = pCheck->GetCheck() > 0;
}

void CMovementDetectionPage::OnSaveAnimGifMovementDetection() 
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_SAVE_ANIMATEDGIF_MOVEMENT_DETECTION);
	m_pDoc->m_bSaveAnimGIFMovementDetection = pCheck->GetCheck() > 0;
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
		CString sSize;
		sSize.Format(ML_STRING(1769, "Thumbnail Size %i x %i"),	m_pDoc->m_dwAnimatedGifWidth,
																m_pDoc->m_dwAnimatedGifHeight);
		CButton* pButton = (CButton*)GetDlgItem(IDC_ANIMATEDGIF_SIZE);
		pButton->SetWindowText(sSize);
	}

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
	if (m_pDoc->m_nDetectionStartStop > 0)
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

void CMovementDetectionPage::OnCbnSelchangeComboboxDetectionScheduler()
{
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBOBOX_DETECTION_SCHEDULER);
	m_pDoc->m_nDetectionStartStop = pComboBox->GetCurSel();
	UpdateDetectionStartStopTimes();
}

void CMovementDetectionPage::OnCheckSchedulerSunday() 
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_SCHEDULER_SUNDAY);
	pCheck->GetCheck() == 0 ? m_pDoc->m_bDetectionSunday = FALSE : m_pDoc->m_bDetectionSunday = TRUE;
}

void CMovementDetectionPage::OnCheckSchedulerMonday() 
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_SCHEDULER_MONDAY);
	pCheck->GetCheck() == 0 ? m_pDoc->m_bDetectionMonday = FALSE : m_pDoc->m_bDetectionMonday = TRUE;
}

void CMovementDetectionPage::OnCheckSchedulerTuesday() 
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_SCHEDULER_TUESDAY);
	pCheck->GetCheck() == 0 ? m_pDoc->m_bDetectionTuesday = FALSE : m_pDoc->m_bDetectionTuesday = TRUE;
}

void CMovementDetectionPage::OnCheckSchedulerWednesday() 
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_SCHEDULER_WEDNESDAY);
	pCheck->GetCheck() == 0 ? m_pDoc->m_bDetectionWednesday = FALSE : m_pDoc->m_bDetectionWednesday = TRUE;
}

void CMovementDetectionPage::OnCheckSchedulerThursday() 
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_SCHEDULER_THURSDAY);
	pCheck->GetCheck() == 0 ? m_pDoc->m_bDetectionThursday = FALSE : m_pDoc->m_bDetectionThursday = TRUE;
}

void CMovementDetectionPage::OnCheckSchedulerFriday() 
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_SCHEDULER_FRIDAY);
	pCheck->GetCheck() == 0 ? m_pDoc->m_bDetectionFriday = FALSE : m_pDoc->m_bDetectionFriday = TRUE;
}

void CMovementDetectionPage::OnCheckSchedulerSaturday() 
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_SCHEDULER_SATURDAY);
	pCheck->GetCheck() == 0 ? m_pDoc->m_bDetectionSaturday = FALSE : m_pDoc->m_bDetectionSaturday = TRUE;
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

