// GeneralPage.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "AVRec.h"
#include "GeneralPage.h"
#include "VideoDeviceDoc.h"
#include "VideoDeviceView.h"
#include "DxCapture.h"
#include "DxVideoInputDlg.h"
#include "NetCom.h"
#include "MainFrm.h"
#include "BrowseDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef VIDEODEVICEDOC

/////////////////////////////////////////////////////////////////////////////
// CGeneralPage dialog

CGeneralPage::CGeneralPage()
	: CPropertyPage(CGeneralPage::IDD)
{
	// OnInitDialog() is called when first pressing the tab
	// OnInitDialog() inits the property page pointer in the doc
	//{{AFX_DATA_INIT(CGeneralPage)
	//}}AFX_DATA_INIT
	m_pDoc = NULL;
	m_bDlgInitialized = FALSE;
}

void CGeneralPage::SetDoc(CVideoDeviceDoc* pDoc)
{
	ASSERT(pDoc);
	m_pDoc = pDoc;
}

CGeneralPage::~CGeneralPage()
{
}

void CGeneralPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGeneralPage)
	DDX_Control(pDX, IDC_VIDEO_COMPRESSION_QUALITY, m_VideoRecQuality);
	DDX_Control(pDX, IDC_FRAMERATE, m_FrameRate);
	DDX_Control(pDX, IDC_SPIN_FRAMERATE, m_SpinFrameRate);
	DDX_DateTimeCtrl(pDX, IDC_DATE_ONCE_START, m_SchedulerOnceDateStart);
	DDX_DateTimeCtrl(pDX, IDC_TIME_ONCE_START, m_SchedulerOnceTimeStart);
	DDX_DateTimeCtrl(pDX, IDC_TIME_ONCE_STOP, m_SchedulerOnceTimeStop);
	DDX_DateTimeCtrl(pDX, IDC_DATE_ONCE_STOP, m_SchedulerOnceDateStop);
	DDX_DateTimeCtrl(pDX, IDC_TIME_DAILY_START, m_SchedulerDailyTimeStart);
	DDX_DateTimeCtrl(pDX, IDC_TIME_DAILY_STOP, m_SchedulerDailyTimeStop);
	DDX_Check(pDX, IDC_CHECK_TIME_SEGMENTATION, m_bRecTimeSegmentation);
	DDX_Check(pDX, IDC_CHECK_AUTORUN, m_bAutorun);
	DDX_CBIndex(pDX, IDC_TIME_SEGMENTATION, m_nTimeSegmentationIndex);
	DDX_Check(pDX, IDC_CHECK_LIVE_DEINTERLACE, m_bDeinterlace);
	DDX_Check(pDX, IDC_CHECK_LIVE_ROTATE180, m_bRotate180);
	DDX_Check(pDX, IDC_CHECK_AUTOOPEN, m_bRecAutoOpen);
	DDX_Check(pDX, IDC_CHECK_AUDIO_LISTEN, m_bAudioListen);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGeneralPage, CPropertyPage)
	//{{AFX_MSG_MAP(CGeneralPage)
	ON_BN_CLICKED(IDC_VIDEO_FORMAT, OnVideoFormat)
	ON_BN_CLICKED(IDC_VIDEO_SOURCE, OnVideoSource)
	ON_EN_CHANGE(IDC_FRAMERATE, OnChangeFrameRate)
	ON_BN_CLICKED(IDC_REC_AUDIO, OnRecAudio)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_AUDIO_INPUT, OnAudioInput)
	ON_BN_CLICKED(IDC_AUDIO_MIXER, OnAudioMixer)
	ON_BN_CLICKED(IDC_VIDEO_INPUT, OnVideoInput)
	ON_BN_CLICKED(IDC_VIDEO_TUNER, OnVideoTuner)
	ON_BN_CLICKED(IDC_CHECK_SCHEDULER_ONCE, OnCheckSchedulerOnce)
	ON_BN_CLICKED(IDC_CHECK_SCHEDULER_DAILY, OnCheckSchedulerDaily)
	ON_BN_CLICKED(IDC_CHECK_TIME_SEGMENTATION, OnCheckTimeSegmentation)
	ON_BN_CLICKED(IDC_CHECK_AUTORUN, OnCheckAutorun)
	ON_CBN_SELCHANGE(IDC_TIME_SEGMENTATION, OnSelchangeTimeSegmentation)
	ON_BN_CLICKED(IDC_CHECK_AUTOOPEN, OnCheckAutoopen)
	ON_BN_CLICKED(IDC_CHECK_LIVE_DEINTERLACE, OnCheckLiveDeinterlace)
	ON_BN_CLICKED(IDC_CHECK_LIVE_ROTATE180, OnCheckLiveRotate180)
	ON_BN_CLICKED(IDC_CHECK_AUDIO_LISTEN, OnCheckAudioListen)
	ON_CBN_SELCHANGE(IDC_REF_FONTSIZE, OnSelchangeRefFontsize)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_TIME_ONCE_START, OnDatetimechangeTimeOnceStart)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_TIME_ONCE_STOP, OnDatetimechangeTimeOnceStop)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATE_ONCE_START, OnDatetimechangeDateOnceStart)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATE_ONCE_STOP, OnDatetimechangeDateOnceStop)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_TIME_DAILY_START, OnDatetimechangeTimeDailyStart)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_TIME_DAILY_STOP, OnDatetimechangeTimeDailyStop)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGeneralPage message handlers

void CGeneralPage::OnVideoFormat() 
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
		m_pDoc->SetDocumentTitle();
	}
}

void CGeneralPage::OnVideoSource() 
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
			m_pDoc->SetDocumentTitle();
		}
	}
}

void CGeneralPage::OnVideoInput() 
{
	// Open the video input dialog
	if (m_pDoc->m_pDxCapture)
	{
		CDxVideoInputDlg dlg(m_pDoc);
		dlg.DoModal();
	}
}

void CGeneralPage::OnVideoTuner() 
{
	// Open the tv tuner dialog
	if (m_pDoc->m_pDxCapture)
		m_pDoc->m_pDxCapture->ShowVideoTVTunerDlg();
}

void CGeneralPage::UpdateVideoQualityInfo()
{
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_VIDEO_COMPRESSION_QUALITY_INFO);
	CString sQuality;
	switch (GetRevertedPos(m_VideoRecQuality))
	{
		case 3 : sQuality = ML_STRING(1544, "Best"); break;
		case 4 : sQuality = ML_STRING(1543, "Good"); break;
		case 5 : sQuality = ML_STRING(1542, "Medium"); break;
		default: sQuality = ML_STRING(1541, "Low"); break;
	}
	pEdit->SetWindowText(sQuality);
}

BOOL CGeneralPage::OnInitDialog() 
{	
	// Frame Rate Change Flag
	m_bDoChangeFrameRate = FALSE;

	// Init Autorun Var
	if (CVideoDeviceDoc::AutorunGetDeviceKey(m_pDoc->GetDevicePathName()) != _T(""))
		m_bAutorun = TRUE;
	else
		m_bAutorun = FALSE;

	// Init Live Deinterlace and Live Rotate 180° Vars
	m_bDeinterlace = m_pDoc->m_bDeinterlace;
	m_bRotate180 = m_pDoc->m_bRotate180;

	// Init Rec Auto Open Var
	m_bRecAutoOpen = m_pDoc->m_bRecAutoOpen;

	// Init Segmentation Vars
	m_bRecTimeSegmentation = m_pDoc->m_bRecTimeSegmentation;
	m_nTimeSegmentationIndex = m_pDoc->m_nTimeSegmentationIndex;

	// Init Audio Listen Var
	m_bAudioListen = m_pDoc->m_bAudioListen;

	// Init Scheduler Values
	CUImagerApp::CSchedulerEntry* pOnceSchedulerEntry =
		((CUImagerApp*)::AfxGetApp())->GetOnceSchedulerEntry(m_pDoc->GetDevicePathName());

	CUImagerApp::CSchedulerEntry* pDailySchedulerEntry =
		((CUImagerApp*)::AfxGetApp())->GetDailySchedulerEntry(m_pDoc->GetDevicePathName());

	if (pOnceSchedulerEntry)
	{
		// Start Time & Date
		m_SchedulerOnceDateStart = CTime(pOnceSchedulerEntry->m_StartTime.GetYear(),
										pOnceSchedulerEntry->m_StartTime.GetMonth(),
										pOnceSchedulerEntry->m_StartTime.GetDay(),
										12, 0, 0);
		m_SchedulerOnceTimeStart = CTime(2000, 1, 1,
										pOnceSchedulerEntry->m_StartTime.GetHour(),
										pOnceSchedulerEntry->m_StartTime.GetMinute(),
										pOnceSchedulerEntry->m_StartTime.GetSecond());

		// Stop Time & Date
		m_SchedulerOnceDateStop = CTime(pOnceSchedulerEntry->m_StopTime.GetYear(),
										pOnceSchedulerEntry->m_StopTime.GetMonth(),
										pOnceSchedulerEntry->m_StopTime.GetDay(),
										12, 0, 0);
		m_SchedulerOnceTimeStop = CTime(2000, 1, 1,
										pOnceSchedulerEntry->m_StopTime.GetHour(),
										pOnceSchedulerEntry->m_StopTime.GetMinute(),
										pOnceSchedulerEntry->m_StopTime.GetSecond());
	}
	else
	{
		CTime t = CTime::GetCurrentTime();
		m_SchedulerOnceDateStart = t;
		m_SchedulerOnceTimeStart = t;
		m_SchedulerOnceDateStop = t;
		m_SchedulerOnceTimeStop = t;
	}
	
	if (pDailySchedulerEntry)
	{
		// Start Time
		m_SchedulerDailyTimeStart = CTime(2000, 1, 1,
										pDailySchedulerEntry->m_StartTime.GetHour(),
										pDailySchedulerEntry->m_StartTime.GetMinute(),
										pDailySchedulerEntry->m_StartTime.GetSecond());

		// Stop Time
		m_SchedulerDailyTimeStop = CTime(2000, 1, 1,
										pDailySchedulerEntry->m_StopTime.GetHour(),
										pDailySchedulerEntry->m_StopTime.GetMinute(),
										pDailySchedulerEntry->m_StopTime.GetSecond());
	}
	else
	{
		CTime t = CTime::GetCurrentTime();
		m_SchedulerDailyTimeStart = t;
		m_SchedulerDailyTimeStop = t;
	}

	// Init Combo Boxes
	CComboBox* pComboBoxTimeSeg = (CComboBox*)GetDlgItem(IDC_TIME_SEGMENTATION);
	if (pComboBoxTimeSeg)
	{
		pComboBoxTimeSeg->AddString(ML_STRING(1556, "15 minutes"));
		pComboBoxTimeSeg->AddString(ML_STRING(1557, "30 minutes"));
		pComboBoxTimeSeg->AddString(ML_STRING(1558, "1 hour"));
		pComboBoxTimeSeg->AddString(ML_STRING(1559, "2 hours"));
		pComboBoxTimeSeg->AddString(ML_STRING(1560, "3 hours"));
		pComboBoxTimeSeg->AddString(ML_STRING(1561, "6 hours"));
		pComboBoxTimeSeg->AddString(ML_STRING(1562, "12 hours"));
		pComboBoxTimeSeg->AddString(ML_STRING(1563, "24 hours"));
	}
	CComboBox* pComboBoxRefFontSize = (CComboBox*)GetDlgItem(IDC_REF_FONTSIZE);
	if (pComboBoxRefFontSize)
	{
		pComboBoxRefFontSize->AddString(_T("1"));
		pComboBoxRefFontSize->AddString(_T("2"));
		pComboBoxRefFontSize->AddString(_T("3"));
		pComboBoxRefFontSize->AddString(_T("4"));
		pComboBoxRefFontSize->AddString(_T("5"));
		pComboBoxRefFontSize->AddString(_T("6"));
		pComboBoxRefFontSize->AddString(_T("7"));
		pComboBoxRefFontSize->AddString(_T("8"));
		pComboBoxRefFontSize->AddString(_T("9"));
		pComboBoxRefFontSize->AddString(_T("10"));
		pComboBoxRefFontSize->AddString(_T("11"));
		pComboBoxRefFontSize->AddString(_T("12"));
		pComboBoxRefFontSize->AddString(_T("13"));
		pComboBoxRefFontSize->AddString(_T("14"));
		pComboBoxRefFontSize->AddString(_T("15"));
		pComboBoxRefFontSize->AddString(_T("16"));
	}

	// Init Codec's Supports

	m_VideoCompressionFcc.Add((DWORD)FCC('FFVH')); 
	m_VideoCompressionFastEncodeAndKeyframesRateSupport.Add((DWORD)0);
	m_VideoCompressionQualitySupport.Add((DWORD)0);

	m_VideoCompressionFcc.Add((DWORD)FCC('MJPG')); 
	m_VideoCompressionFastEncodeAndKeyframesRateSupport.Add((DWORD)0);
	m_VideoCompressionQualitySupport.Add((DWORD)1);

	m_VideoCompressionFcc.Add((DWORD)FCC('DIVX')); 
	m_VideoCompressionFastEncodeAndKeyframesRateSupport.Add((DWORD)1);
	m_VideoCompressionQualitySupport.Add((DWORD)1);

	m_VideoCompressionFcc.Add((DWORD)FCC('H264')); 
	m_VideoCompressionFastEncodeAndKeyframesRateSupport.Add((DWORD)1);
	m_VideoCompressionQualitySupport.Add((DWORD)1);

	// This calls UpdateData(FALSE)
	CPropertyPage::OnInitDialog();

	// Frame Rate
	m_FrameRate.SetMinNumberOfNumberAfterPoint(1);
	m_FrameRate.SetMaxNumberOfNumberAfterPoint(1);
	m_SpinFrameRate.SetBuddy(&m_FrameRate);
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_FRAMERATE);
	if (m_pDoc->m_pDxCapture)
	{
		if (m_pDoc->m_pDxCapture->GetFrameRate() <= 0.0) // Not Settable
		{
			m_SpinFrameRate.SetRange(0.0, 0.0);
			m_SpinFrameRate.EnableWindow(FALSE);
			pEdit->EnableWindow(FALSE);
		}
		else
			m_SpinFrameRate.SetRange(MIN_FRAMERATE, MAX_FRAMERATE);
	}
	else if (m_pDoc->m_pVideoNetCom)
	{
		// Axis and Edimax support only integer values starting at 1 fps
		if (m_pDoc->m_nNetworkDeviceTypeMode == CVideoDeviceDoc::AXIS_SP ||
			m_pDoc->m_nNetworkDeviceTypeMode == CVideoDeviceDoc::EDIMAX_SP)
			m_SpinFrameRate.SetRange(1.0, MAX_FRAMERATE);
		// Pixord or Foscam
		else if (m_pDoc->m_nNetworkDeviceTypeMode == CVideoDeviceDoc::PIXORD_SP ||
				m_pDoc->m_nNetworkDeviceTypeMode == CVideoDeviceDoc::FOSCAM_SP)
			m_SpinFrameRate.SetRange(MIN_FRAMERATE, MAX_FRAMERATE);
		// Disable all other HTTP motion jpeg devices,
		// HTTP jpeg snapshots devices will be enabled in OnTimer()
		else
		{
			m_SpinFrameRate.SetRange(0.0, 0.0);
			m_SpinFrameRate.EnableWindow(FALSE);
			pEdit->EnableWindow(FALSE);
		}
	}
	else
	{
		m_SpinFrameRate.SetRange(0.0, 0.0);
		m_SpinFrameRate.EnableWindow(FALSE);
		pEdit->EnableWindow(FALSE);
	}
	m_SpinFrameRate.SetDelta(1.0);
	CString sFrameRate;
	sFrameRate.Format(_T("%0.1f"), m_pDoc->m_dFrameRate);
	if (pEdit->IsWindowEnabled())
		pEdit->SetWindowText(sFrameRate);
	else
		pEdit->SetWindowText(_T(""));

	// Capture Audio Check Box
	CButton* pCheck = (CButton*)GetDlgItem(IDC_REC_AUDIO);
	if (m_pDoc->m_bCaptureAudio)
		pCheck->SetCheck(1);
	else
		pCheck->SetCheck(0);

	// Audio Listen only supported on Vista or higher
	pCheck = (CButton*)GetDlgItem(IDC_CHECK_AUDIO_LISTEN);
	if (!g_bWinVistaOrHigher)
		pCheck->ShowWindow(SW_HIDE);

	// Set reference font size
	pComboBoxRefFontSize->SetCurSel(m_pDoc->m_nRefFontSize - 1);

	// Video Compressor Quality
	m_VideoRecQuality.SetRange((int)VIDEO_QUALITY_BEST, (int)VIDEO_QUALITY_LOW);
	m_VideoRecQuality.SetPageSize(1);
	m_VideoRecQuality.SetLineSize(1);
	m_pDoc->m_fVideoRecQuality = CAVRec::ClipVideoQuality(m_pDoc->m_fVideoRecQuality);
	SetRevertedPos(m_VideoRecQuality, (int)m_pDoc->m_fVideoRecQuality);
	UpdateVideoQualityInfo();

	// Enable Format Button?
	CButton* pButton = (CButton*)GetDlgItem(IDC_VIDEO_FORMAT);
	if ((m_pDoc->m_pDxCapture && m_pDoc->m_pDxCapture->HasFormats())										||
		(m_pDoc->m_pDxCapture && m_pDoc->m_pDxCapture->IsDV() && m_pDoc->m_pDxCapture->HasDVFormatDlg())	||
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

	// Enable Audio Source Button?
	pButton = (CButton*)GetDlgItem(IDC_AUDIO_INPUT);
	if (m_pDoc->m_pDxCapture)
		pButton->EnableWindow(TRUE);
	else
		pButton->EnableWindow(FALSE);

	// Enable Audio Mixer Buttom?
	pButton = (CButton*)GetDlgItem(IDC_AUDIO_MIXER);
	if (m_pDoc->m_pDxCapture)
		pButton->EnableWindow(TRUE);
	else
		pButton->EnableWindow(FALSE);

	// Set scheduler date range
	CDateTimeCtrl* pDateTimeCtrl = (CDateTimeCtrl*)GetDlgItem(IDC_DATE_ONCE_START);
	CTime t1(CTime::GetCurrentTime());
	CTime t2(3000, 12, 31, 23, 59, 59);
	pDateTimeCtrl->SetRange(&t1, &t2);
	pDateTimeCtrl = (CDateTimeCtrl*)GetDlgItem(IDC_DATE_ONCE_STOP);
	pDateTimeCtrl->SetRange(&t1, &t2);
	
	// Init Once Scheduler's Check-Box
	if (pOnceSchedulerEntry)
	{
		pCheck = (CButton*)GetDlgItem(IDC_CHECK_SCHEDULER_ONCE);
		pCheck->SetCheck(1);
	}

	// Init Daily Scheduler's Check-Box
	if (pDailySchedulerEntry)
	{
		pCheck = (CButton*)GetDlgItem(IDC_CHECK_SCHEDULER_DAILY);
		pCheck->SetCheck(1);
	}

	// OnInitDialog() has been called
	m_bDlgInitialized = TRUE;

	// Set Page Pointer to this
	m_pDoc->m_pGeneralPage = this;

	// Set Timer
	SetTimer(ID_TIMER_GENERALDLG, GENERALDLG_TIMER_MS, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CGeneralPage::OnDestroy() 
{
	// Kill timer
	KillTimer(ID_TIMER_GENERALDLG);

	// Base class
	CPropertyPage::OnDestroy();

	// Set Dialog Pointer to NULL
	m_pDoc->m_pGeneralPage = NULL;
}

void CGeneralPage::OnChangeFrameRate() 
{
	if (m_bDlgInitialized && ::IsWindow(m_SpinFrameRate.GetSafeHwnd()))
	{
		CString sFrameRate;
		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_FRAMERATE);
		pEdit->GetWindowText(sFrameRate);
		double dFrameRate = _tcstod(sFrameRate, NULL);
		if (sFrameRate != _T("") && dFrameRate != m_pDoc->m_dFrameRate)
		{
			m_pDoc->StopProcessFrame(PROCESSFRAME_CHANGEFRAMERATE);
			m_nFrameRateChangeTimeout = FRAMERATE_CHANGE_TIMEOUT;
			if (!m_bDoChangeFrameRate)
			{
				// Done in OnTimer()
				m_bDoChangeFrameRate = TRUE;
			}
		}
	}
}

void CGeneralPage::OnRecAudio() 
{
	// Stop watchdog thread
	m_pDoc->m_WatchdogThread.Kill();

	// Stop Save Frame List Thread
	m_pDoc->m_SaveFrameListThread.Kill();

	// Stop Rec
	if (m_pDoc->m_pAVRec)
		m_pDoc->CaptureRecord();

	// Start/Stop Capture Audio Thread
	CButton* pCheck = (CButton*)GetDlgItem(IDC_REC_AUDIO);
	BOOL bDoCaptureAudio = (pCheck->GetCheck() == 1);
	if (bDoCaptureAudio)
	{
		::InterlockedExchange(&m_pDoc->m_lLastAudioFramesUpTime, (LONG)::timeGetTime());
		m_pDoc->m_bCaptureAudio = TRUE;
		if (m_pDoc->m_pAudioNetCom)
		{
			m_pDoc->m_pHttpAudioParseProcess->m_bTryConnecting = TRUE;
			m_pDoc->m_HttpThread.SetEventAudioConnect();
			::Sleep(200); // wait to let CHttpThread process the event
		}
		else if (m_pDoc->m_pDxCapture)
			m_pDoc->m_CaptureAudioThread.Start();
	}
	else
	{
		m_pDoc->m_bCaptureAudio = FALSE;
		if (m_pDoc->m_pAudioNetCom)
		{
			m_pDoc->m_pAudioNetCom->ShutdownConnection_NoBlocking();
			::Sleep(200); // wait to let CMsgThread process the event
		}
		else if (m_pDoc->m_pDxCapture)
			m_pDoc->m_CaptureAudioThread.Kill();
	}

	// Restart Save Frame List Thread
	m_pDoc->m_SaveFrameListThread.Start();

	// Restart watchdog thread
	m_pDoc->m_WatchdogThread.Start();
}

void CGeneralPage::OnTimer(UINT nIDEvent) 
{
	if (!m_pDoc->m_bClosing)
	{
		// Show Calculated Frame Rate
		double dEffectiveFrameRate = m_pDoc->m_dEffectiveFrameRate;
		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EFFECTIVE_FRAMERATE);
		CString sEffectiveFrameRate;
		sEffectiveFrameRate.Format(_T("%0.1f"), dEffectiveFrameRate);
		pEdit->SetWindowText(sEffectiveFrameRate);

		// Show Dropped Frames
		pEdit = (CEdit*)GetDlgItem(IDC_DROPPED_FRAMES);
		if (pEdit)
		{
			CString sDroppedFrames;
			if (m_pDoc->m_pDxCapture)
			{
				LONG lDroppedFrames = m_pDoc->m_pDxCapture->GetDroppedFrames();
				if (lDroppedFrames >= 0)
					sDroppedFrames.Format(_T("%d"), lDroppedFrames);
				else
					sDroppedFrames = _T("0");	// Unsupported
			}
			else
				sDroppedFrames = _T("0");	// Unsupported
			pEdit->SetWindowText(sDroppedFrames);
		}

		// Show Process Frame Time
		pEdit = (CEdit*)GetDlgItem(IDC_PROCESS_TIME);
		if (pEdit)
		{
			CString sProcessFrameTime;
			sProcessFrameTime.Format(_T("%d"), m_pDoc->m_lProcessFrameTime);
			pEdit->SetWindowText(sProcessFrameTime);
		}

		// Show Data Rate
		pEdit = (CEdit*)GetDlgItem(IDC_DATA_RATE);
		if (pEdit)
		{
			CString sDataRate(_T("xxx"));
			LONG lUncompressedAvgFrameSize = m_pDoc->m_ProcessFrameBMI.bmiHeader.biSizeImage;
			if (lUncompressedAvgFrameSize > 0)
			{
				if (m_pDoc->m_lCompressedDataRate > 0)
				{
					sDataRate.Format(_T("%0.1f -> %0.1f"),	(double)m_pDoc->m_lCompressedDataRate / 1024.0,						// KB / Sec
															(double)lUncompressedAvgFrameSize * dEffectiveFrameRate / 1024.0);	// KB / Sec
				}
				else
				{
					sDataRate.Format(_T("%0.1f"),			(double)lUncompressedAvgFrameSize * dEffectiveFrameRate / 1024.0);	// KB / Sec
				}
			}
			pEdit->SetWindowText(sDataRate);
		}

		// Enable Frame Rate Edit Control for HTTP jpeg snapshots devices
		CString sFrameRate;
		pEdit = (CEdit*)GetDlgItem(IDC_FRAMERATE);
		if (m_pDoc->m_pVideoNetCom &&
			m_pDoc->m_pHttpVideoParseProcess->m_FormatType == CVideoDeviceDoc::CHttpParseProcess::FORMATVIDEO_JPEG &&
			!pEdit->IsWindowEnabled())
		{
			m_SpinFrameRate.SetRange(MIN_FRAMERATE, MAX_FRAMERATE);
			m_SpinFrameRate.EnableWindow(TRUE);
			pEdit->EnableWindow(TRUE);
			sFrameRate.Format(_T("%0.1f"), m_pDoc->m_dFrameRate);
			pEdit->SetWindowText(sFrameRate);
		}

		// Change The Frame Rate if Necessary
		if (m_bDoChangeFrameRate)
		{
			--m_nFrameRateChangeTimeout;
			if (m_nFrameRateChangeTimeout <= 0 && m_pDoc->IsProcessFrameStopped(PROCESSFRAME_CHANGEFRAMERATE))
			{
				// Reset flag
				m_bDoChangeFrameRate = FALSE;

				// Frame Rate Edit Control
				pEdit->GetWindowText(sFrameRate);
				BOOL bOk = FALSE;
				BOOL bRestore = FALSE;
				double dFrameRate = _tcstod(sFrameRate, NULL);
				double dMinFrameRate, dMaxFrameRate;
				m_SpinFrameRate.GetRange(dMinFrameRate, dMaxFrameRate);
				if (sFrameRate != _T(""))
				{
					if (dFrameRate >= dMinFrameRate && dFrameRate <= dMaxFrameRate)
						bOk = TRUE;
					else
						bRestore = TRUE;
				}
				if (bOk)
				{
					m_pDoc->m_dFrameRate = dFrameRate;
					m_pDoc->OnChangeFrameRate();
				}
				else if (bRestore)
				{
					m_pDoc->StartProcessFrame(PROCESSFRAME_CHANGEFRAMERATE);
					sFrameRate.Format(_T("%0.1f"), m_pDoc->m_dFrameRate);
					pEdit->SetWindowText(sFrameRate);
					pEdit->SetFocus();
					pEdit->SetSel(0xFFFF0000);
				}
			}
		}
	}
	CPropertyPage::OnTimer(nIDEvent);
}

void CGeneralPage::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if (pScrollBar)
	{
		CSliderCtrl* pSlider = (CSliderCtrl*)pScrollBar;

		if ((SB_THUMBTRACK == nSBCode) ||	// Dragging Slider
			(SB_THUMBPOSITION == nSBCode) ||// Wheel On Mouse And End Of Dragging Slider
			(SB_LINEUP  == nSBCode) ||		// Keyboard Arrow
			(SB_LINEDOWN  == nSBCode) ||	// Keyboard Arrow
			(SB_PAGEUP == nSBCode) ||		// Mouse Press Above Slider
			(SB_PAGEDOWN == nSBCode) ||		// Mouse Press Below Slider
			(SB_LEFT == nSBCode)	||		// Home Button
			(SB_RIGHT == nSBCode))			// End Button  
		{
			if (pSlider->GetDlgCtrlID() == IDC_VIDEO_COMPRESSION_QUALITY)
			{
				m_pDoc->m_fVideoRecQuality = (float)GetRevertedPos(m_VideoRecQuality);
				UpdateVideoQualityInfo();
			}
		}
	}
	
	CPropertyPage::OnHScroll(nSBCode, nPos, (CScrollBar*)pScrollBar);
}

void CGeneralPage::OnCheckLiveDeinterlace() 
{
	UpdateData(TRUE);
	m_pDoc->m_bDeinterlace = m_bDeinterlace;
}

void CGeneralPage::OnCheckLiveRotate180() 
{
	UpdateData(TRUE);
	m_pDoc->m_bRotate180 = m_bRotate180;
}

void CGeneralPage::OnCheckAutoopen() 
{
	UpdateData(TRUE);
	m_pDoc->m_bRecAutoOpen = m_bRecAutoOpen;
}

void CGeneralPage::OnCheckTimeSegmentation() 
{
	UpdateData(TRUE);
	m_pDoc->m_bRecTimeSegmentation = m_bRecTimeSegmentation;
}

void CGeneralPage::OnCheckAudioListen()
{
	UpdateData(TRUE);
	m_pDoc->m_bAudioListen = m_bAudioListen;
}

void CGeneralPage::OnSelchangeTimeSegmentation() 
{
	UpdateData(TRUE);
	m_pDoc->m_nTimeSegmentationIndex = m_nTimeSegmentationIndex;
}

void CGeneralPage::OnCheckAutorun() 
{
	UpdateData(TRUE);
	if (m_bAutorun)
		CVideoDeviceDoc::AutorunAddDevice(m_pDoc->GetDevicePathName());
	else
		CVideoDeviceDoc::AutorunRemoveDevice(m_pDoc->GetDevicePathName());
}

void CGeneralPage::OnAudioInput() 
{
	m_pDoc->m_CaptureAudioThread.AudioInSourceDialog();	
}

void CGeneralPage::OnSelchangeRefFontsize()
{
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_REF_FONTSIZE);
	m_pDoc->m_nRefFontSize = pComboBox->GetCurSel() + 1;
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
void CGeneralPage::OnAudioMixer() 
{
	if (g_bWinVistaOrHigher)
	{
		::ShellExecute(	NULL, NULL,
						_T("control.exe"), _T("mmsys.cpl,,1"), NULL, SW_SHOWNORMAL);
	}
	else
	{
		CString params;
		params.Format(_T("/r /d%u"), m_pDoc->EffectiveCaptureAudioDeviceID());
		::ShellExecute(	NULL, NULL,
						_T("sndvol32.exe"), params, NULL, SW_SHOWNORMAL);
	}
}

void CGeneralPage::SetCheckSchedulerOnce(BOOL bCheck) 
{
	CButton* pCheckOnce = (CButton*)GetDlgItem(IDC_CHECK_SCHEDULER_ONCE);
	if (pCheckOnce->GetCheck())
	{
		if (!bCheck)
		{
			pCheckOnce->SetCheck(0);
			ApplySchedulerOnce();
		}
	}
	else
	{
		if (bCheck)
		{
			pCheckOnce->SetCheck(1);
			ApplySchedulerOnce();
		}
	}
}

void CGeneralPage::ApplySchedulerOnce()
{
	CButton* pCheckOnce = (CButton*)GetDlgItem(IDC_CHECK_SCHEDULER_ONCE);
	CButton* pCheckDaily = (CButton*)GetDlgItem(IDC_CHECK_SCHEDULER_DAILY);
	if (pCheckOnce->GetCheck())
	{
		// Clear daily
		SetCheckSchedulerDaily(FALSE);

		// Scheduler Entry Init
		CUImagerApp::CSchedulerEntry* pSchedulerEntry = new CUImagerApp::CSchedulerEntry;
		if (!pSchedulerEntry)
			return;
		pSchedulerEntry->m_Type = CUImagerApp::CSchedulerEntry::ONCE;
		pSchedulerEntry->m_sDevicePathName = m_pDoc->GetDevicePathName();

		// Update Data From Dialog to Vars
		UpdateData(TRUE);

		// Start Time
		pSchedulerEntry->m_StartTime = CTime(	m_SchedulerOnceDateStart.GetYear(),
												m_SchedulerOnceDateStart.GetMonth(),
												m_SchedulerOnceDateStart.GetDay(),
												m_SchedulerOnceTimeStart.GetHour(),
												m_SchedulerOnceTimeStart.GetMinute(),
												m_SchedulerOnceTimeStart.GetSecond());

		// Stop Time
		pSchedulerEntry->m_StopTime = CTime(	m_SchedulerOnceDateStop.GetYear(),
												m_SchedulerOnceDateStop.GetMonth(),
												m_SchedulerOnceDateStop.GetDay(),
												m_SchedulerOnceTimeStop.GetHour(),
												m_SchedulerOnceTimeStop.GetMinute(),
												m_SchedulerOnceTimeStop.GetSecond());

		// Add Scheduler Entry
		((CUImagerApp*)::AfxGetApp())->AddSchedulerEntry(pSchedulerEntry);
	}
	else
	{
		// Delete Once Scheduler Entry
		((CUImagerApp*)::AfxGetApp())->DeleteOnceSchedulerEntry(m_pDoc->GetDevicePathName());
	}
}

void CGeneralPage::OnCheckSchedulerOnce() 
{
	ApplySchedulerOnce();
}

void CGeneralPage::OnDatetimechangeTimeOnceStart(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
	ApplySchedulerOnce();
	*pResult = 0;
}

void CGeneralPage::OnDatetimechangeTimeOnceStop(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
	ApplySchedulerOnce();
	*pResult = 0;
}

void CGeneralPage::OnDatetimechangeDateOnceStart(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
	ApplySchedulerOnce();
	*pResult = 0;
}

void CGeneralPage::OnDatetimechangeDateOnceStop(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
	ApplySchedulerOnce();
	*pResult = 0;
}

void CGeneralPage::ClearOnceScheduler()
{
	CTime t = CTime::GetCurrentTime();
	m_SchedulerOnceDateStart = t;
	m_SchedulerOnceTimeStart = t;
	m_SchedulerOnceDateStop = t;
	m_SchedulerOnceTimeStop = t;
	UpdateData(FALSE);

	// Uncheck
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_SCHEDULER_ONCE);
	pCheck->SetCheck(0);
}

void CGeneralPage::SetCheckSchedulerDaily(BOOL bCheck) 
{
	CButton* pCheckDaily = (CButton*)GetDlgItem(IDC_CHECK_SCHEDULER_DAILY);
	if (pCheckDaily->GetCheck())
	{
		if (!bCheck)
		{
			pCheckDaily->SetCheck(0);
			ApplySchedulerDaily();
		}
	}
	else
	{
		if (bCheck)
		{
			pCheckDaily->SetCheck(1);
			ApplySchedulerDaily();
		}
	}
}

void CGeneralPage::ApplySchedulerDaily()
{
	CButton* pCheckOnce = (CButton*)GetDlgItem(IDC_CHECK_SCHEDULER_ONCE);
	CButton* pCheckDaily = (CButton*)GetDlgItem(IDC_CHECK_SCHEDULER_DAILY);	
	if (pCheckDaily->GetCheck())
	{
		// Clear once
		SetCheckSchedulerOnce(FALSE);

		// Scheduler Entry Init
		CUImagerApp::CSchedulerEntry* pSchedulerEntry = new CUImagerApp::CSchedulerEntry;
		if (!pSchedulerEntry)
			return;
		pSchedulerEntry->m_Type = CUImagerApp::CSchedulerEntry::DAILY;
		pSchedulerEntry->m_sDevicePathName = m_pDoc->GetDevicePathName();

		// Update Data From Dialog to Vars
		UpdateData(TRUE);

		// Start Time
		pSchedulerEntry->m_StartTime = CTime(	2000,
												1,
												1,
												m_SchedulerDailyTimeStart.GetHour(),
												m_SchedulerDailyTimeStart.GetMinute(),
												m_SchedulerDailyTimeStart.GetSecond());

		// Stop Time
		pSchedulerEntry->m_StopTime = CTime(	2000,
												1,
												1,
												m_SchedulerDailyTimeStop.GetHour(),
												m_SchedulerDailyTimeStop.GetMinute(),
												m_SchedulerDailyTimeStop.GetSecond());

		// Add Scheduler Entry
		((CUImagerApp*)::AfxGetApp())->AddSchedulerEntry(pSchedulerEntry);
	}
	else
	{
		// Delete Daily Scheduler Entry
		((CUImagerApp*)::AfxGetApp())->DeleteDailySchedulerEntry(m_pDoc->GetDevicePathName());
	}
}

void CGeneralPage::OnCheckSchedulerDaily() 
{
	ApplySchedulerDaily();
}

void CGeneralPage::OnDatetimechangeTimeDailyStart(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
	ApplySchedulerDaily();
	*pResult = 0;
}

void CGeneralPage::OnDatetimechangeTimeDailyStop(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
	ApplySchedulerDaily();
	*pResult = 0;
}

#endif