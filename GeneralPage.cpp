// GeneralPage.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "GeneralPage.h"
#include "AviFile.h"
#include "VideoDeviceDoc.h"
#include "VideoDeviceView.h"
#include "VideoAviDoc.h"
#include "DxCapture.h"
#include "DxCaptureVMR9.h"
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
	// -> Move all inits to OnInitDialog() because the assistant
	// may change the doc vars between construction and OnInitDialog() call
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
	DDX_Control(pDX, IDC_VIDEO_POSTREC_COMPRESSION_QUALITY, m_VideoPostRecQuality);
	DDX_Control(pDX, IDC_VIDEO_POSTREC_COMPRESSION_CHOOSE, m_VideoPostRecCompressionChoose);
	DDX_Control(pDX, IDC_VIDEO_COMPRESSION_QUALITY, m_VideoRecQuality);
	DDX_Control(pDX, IDC_VIDEO_COMPRESSION_CHOOSE, m_VideoCompressionChoose);
	DDX_Control(pDX, IDC_REC_VOL_RIGHT, m_RecVolumeRight);
	DDX_Control(pDX, IDC_REC_VOL_LEFT, m_RecVolumeLeft);
	DDX_Control(pDX, IDC_FRAMERATE, m_FrameRate);
	DDX_Control(pDX, IDC_SPIN_FRAMERATE, m_SpinFrameRate);
	DDX_DateTimeCtrl(pDX, IDC_DATE_ONCE_START, m_SchedulerOnceDateStart);
	DDX_DateTimeCtrl(pDX, IDC_TIME_ONCE_START, m_SchedulerOnceTimeStart);
	DDX_DateTimeCtrl(pDX, IDC_TIME_ONCE_STOP, m_SchedulerOnceTimeStop);
	DDX_DateTimeCtrl(pDX, IDC_DATE_ONCE_STOP, m_SchedulerOnceDateStop);
	DDX_DateTimeCtrl(pDX, IDC_TIME_DAILY_START, m_SchedulerDailyTimeStart);
	DDX_DateTimeCtrl(pDX, IDC_TIME_DAILY_STOP, m_SchedulerDailyTimeStop);
	DDX_Text(pDX, IDC_EDIT_DATARATE, m_nVideoRecDataRate);
	DDX_Text(pDX, IDC_EDIT_KEYFRAMES_RATE, m_nVideoRecKeyframesRate);
	DDX_Check(pDX, IDC_CHECK_SIZE_SEGMENTATION, m_bRecSizeSegmentation);
	DDX_Text(pDX, IDC_RECFILE_COUNT, m_nRecFileCount);
	DDV_MinMaxInt(pDX, m_nRecFileCount, 1, 99999);
	DDX_Text(pDX, IDC_RECFILE_SIZE, m_nRecFileSizeMB);
	DDV_MinMaxInt(pDX, m_nRecFileSizeMB, 1, 9999999);
	DDX_Check(pDX, IDC_CHECK_POSTREC, m_bPostRec);
	DDX_Text(pDX, IDC_EDIT_POSTREC_KEYFRAMES_RATE, m_nVideoPostRecKeyframesRate);
	DDX_Text(pDX, IDC_EDIT_POSTREC_DATARATE, m_nVideoPostRecDataRate);
	DDX_Radio(pDX, IDC_RADIO_QUALITY, m_nVideoRecQualityBitrate);
	DDX_Radio(pDX, IDC_RADIO_POSTREC_QUALITY, m_nVideoPostRecQualityBitrate);
	DDX_Check(pDX, IDC_CHECK_TIME_SEGMENTATION, m_bRecTimeSegmentation);
	DDX_Check(pDX, IDC_CHECK_AUTORUN, m_bAutorun);
	DDX_Text(pDX, IDC_EDIT_DELETE_RECORDINGS_DAYS, m_nDeleteRecordingsOlderThanDays);
	DDV_MinMaxInt(pDX, m_nDeleteRecordingsOlderThanDays, 0, 4000000);
	DDX_CBIndex(pDX, IDC_TIME_SEGMENTATION, m_nTimeSegmentationIndex);
	DDX_Check(pDX, IDC_CHECK_AUTOOPEN, m_bRecAutoOpen);
	DDX_Check(pDX, IDC_CHECK_DEINTERLACE, m_bRecDeinterlace);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGeneralPage, CPropertyPage)
	//{{AFX_MSG_MAP(CGeneralPage)
	ON_BN_CLICKED(IDC_VIDEO_FORMAT, OnVideoFormat)
	ON_BN_CLICKED(IDC_VIDEO_SOURCE, OnVideoSource)
	ON_EN_CHANGE(IDC_FRAMERATE, OnChangeFrameRate)
	ON_BN_CLICKED(IDC_AUDIO_FORMAT, OnAudioFormat)
	ON_BN_CLICKED(IDC_REC_AUDIO, OnRecAudio)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_AUDIO_INPUT, OnAudioInput)
	ON_CBN_SELCHANGE(IDC_VIDEO_COMPRESSION_CHOOSE, OnSelchangeVideoCompressionChoose)
	ON_BN_CLICKED(IDC_RECORD_SAVEAS, OnRecordSaveas)
	ON_BN_CLICKED(IDC_AUDIO_MIXER, OnAudioMixer)
	ON_BN_CLICKED(IDC_VIDEO_INPUT, OnVideoInput)
	ON_BN_CLICKED(IDC_VIDEO_TUNER, OnVideoTuner)
	ON_BN_CLICKED(IDC_CHECK_SCHEDULER_ONCE, OnCheckSchedulerOnce)
	ON_BN_CLICKED(IDC_CHECK_SCHEDULER_DAILY, OnCheckSchedulerDaily)
	ON_EN_CHANGE(IDC_EDIT_KEYFRAMES_RATE, OnChangeEditKeyframesRate)
	ON_EN_CHANGE(IDC_EDIT_DATARATE, OnChangeEditDatarate)
	ON_EN_CHANGE(IDC_RECFILE_COUNT, OnChangeRecfileCount)
	ON_EN_CHANGE(IDC_RECFILE_SIZE, OnChangeRecfileSize)
	ON_BN_CLICKED(IDC_CHECK_SIZE_SEGMENTATION, OnCheckSizeSegmentation)
	ON_BN_CLICKED(IDC_CHECK_POSTREC, OnCheckPostrec)
	ON_EN_CHANGE(IDC_EDIT_POSTREC_DATARATE, OnChangeEditPostrecDatarate)
	ON_EN_CHANGE(IDC_EDIT_POSTREC_KEYFRAMES_RATE, OnChangeEditPostrecKeyframesRate)
	ON_CBN_SELCHANGE(IDC_VIDEO_POSTREC_COMPRESSION_CHOOSE, OnSelchangeVideoPostrecCompressionChoose)
	ON_BN_CLICKED(IDC_RADIO_QUALITY, OnRadioQuality)
	ON_BN_CLICKED(IDC_RADIO_POSTREC_QUALITY, OnRadioPostrecQuality)
	ON_BN_CLICKED(IDC_RADIO_BITRATE, OnRadioBitrate)
	ON_BN_CLICKED(IDC_RADIO_POSTREC_BITRATE, OnRadioPostrecBitrate)
	ON_BN_CLICKED(IDC_CHECK_TIME_SEGMENTATION, OnCheckTimeSegmentation)
	ON_BN_CLICKED(IDC_CHECK_AUTORUN, OnCheckAutorun)
	ON_EN_CHANGE(IDC_EDIT_DELETE_RECORDINGS_DAYS, OnChangeEditDeleteRecordingsDays)
	ON_CBN_SELCHANGE(IDC_TIME_SEGMENTATION, OnSelchangeTimeSegmentation)
	ON_BN_CLICKED(IDC_CHECK_AUTOOPEN, OnCheckAutoopen)
	ON_BN_CLICKED(IDC_CHECK_DEINTERLACE, OnCheckDeinterlace)
	//}}AFX_MSG_MAP
	ON_MESSAGE(MM_MIXM_CONTROL_CHANGE, OnMixerCtrlChange)
	ON_MESSAGE(WM_PEAKMETER_UPDATE, OnPeakMeterUpdate)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGeneralPage message handlers

void CGeneralPage::OnVideoFormat() 
{
	// Open the video format dialog,
	// critical controls enable/disable is handled
	// inside the VideoFormatDialog() function
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
	// Open the video source dialog,
	// critical controls enable/disable is handled
	// inside the VideoSourceDialog() function
	m_pDoc->VideoSourceDialog();

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

void CGeneralPage::OnVideoInput() 
{
	// Disable Critical Controls
	::SendMessage(	m_pDoc->GetView()->GetSafeHwnd(),
					WM_ENABLE_DISABLE_CRITICAL_CONTROLS,
					(WPARAM)FALSE,	// Disable Them
					(LPARAM)0);

	// Open the video input dialog
	m_pDoc->VideoInputDialog();
	
	// Enable Critical Controls
	::SendMessage(	m_pDoc->GetView()->GetSafeHwnd(),
					WM_ENABLE_DISABLE_CRITICAL_CONTROLS,
					(WPARAM)TRUE,	// Enable Them
					(LPARAM)0);
}

void CGeneralPage::OnVideoTuner() 
{
	// Disable Critical Controls
	::SendMessage(	m_pDoc->GetView()->GetSafeHwnd(),
					WM_ENABLE_DISABLE_CRITICAL_CONTROLS,
					(WPARAM)FALSE,	// Disable Them
					(LPARAM)0);

	// Open the tv tuner dialog
	m_pDoc->VideoTunerDialog();
	
	// Enable Critical Controls
	::SendMessage(	m_pDoc->GetView()->GetSafeHwnd(),
					WM_ENABLE_DISABLE_CRITICAL_CONTROLS,
					(WPARAM)TRUE,	// Enable Them
					(LPARAM)0);
}

void CGeneralPage::OnAudioFormat() 
{
	// Open the audio format dialog
	m_pDoc->AudioFormatDialog();
}

void CGeneralPage::ShowHideCtrls()
{
	// Keyframes Rate
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_KEYFRAMES_RATE);
	if (m_VideoCompressionKeyframesRateSupport[m_VideoCompressionChoose.GetCurSel()])
	{
		pEdit->ShowWindow(SW_SHOW);
		pEdit = (CEdit*)GetDlgItem(IDC_LABEL_KEYFRAMES_RATE0);
		pEdit->ShowWindow(SW_SHOW);
		pEdit = (CEdit*)GetDlgItem(IDC_LABEL_KEYFRAMES_RATE1);
		pEdit->ShowWindow(SW_SHOW);
	}
	else
	{
		pEdit->ShowWindow(SW_HIDE);
		pEdit = (CEdit*)GetDlgItem(IDC_LABEL_KEYFRAMES_RATE0);
		pEdit->ShowWindow(SW_HIDE);
		pEdit = (CEdit*)GetDlgItem(IDC_LABEL_KEYFRAMES_RATE1);
		pEdit->ShowWindow(SW_HIDE);
	}
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_POSTREC_KEYFRAMES_RATE);
	if (m_VideoCompressionKeyframesRateSupport[m_VideoPostRecCompressionChoose.GetCurSel()])
	{
		pEdit->ShowWindow(SW_SHOW);
		pEdit = (CEdit*)GetDlgItem(IDC_LABEL_POSTREC_KEYFRAMES_RATE0);
		pEdit->ShowWindow(SW_SHOW);
		pEdit = (CEdit*)GetDlgItem(IDC_LABEL_POSTREC_KEYFRAMES_RATE1);
		pEdit->ShowWindow(SW_SHOW);
	}
	else
	{
		pEdit->ShowWindow(SW_HIDE);
		pEdit = (CEdit*)GetDlgItem(IDC_LABEL_POSTREC_KEYFRAMES_RATE0);
		pEdit->ShowWindow(SW_HIDE);
		pEdit = (CEdit*)GetDlgItem(IDC_LABEL_POSTREC_KEYFRAMES_RATE1);
		pEdit->ShowWindow(SW_HIDE);
	}

	// Datarate
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_DATARATE);
	if (m_VideoCompressionDataRateSupport[m_VideoCompressionChoose.GetCurSel()] &&
		(m_nVideoRecQualityBitrate == 1											||
		!m_VideoCompressionQualitySupport[m_VideoCompressionChoose.GetCurSel()]))
	{	
		pEdit->ShowWindow(SW_SHOW);
		pEdit = (CEdit*)GetDlgItem(IDC_LABEL_DATARATE0);
		pEdit->ShowWindow(SW_SHOW);
		pEdit = (CEdit*)GetDlgItem(IDC_LABEL_DATARATE1);
		pEdit->ShowWindow(SW_SHOW);
	}
	else
	{
		pEdit->ShowWindow(SW_HIDE);
		pEdit = (CEdit*)GetDlgItem(IDC_LABEL_DATARATE0);
		pEdit->ShowWindow(SW_HIDE);
		pEdit = (CEdit*)GetDlgItem(IDC_LABEL_DATARATE1);
		pEdit->ShowWindow(SW_HIDE);
	}
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_POSTREC_DATARATE);
	if (m_VideoCompressionDataRateSupport[m_VideoPostRecCompressionChoose.GetCurSel()]	&&
		(m_nVideoPostRecQualityBitrate == 1												||
		!m_VideoCompressionQualitySupport[m_VideoPostRecCompressionChoose.GetCurSel()]))
	{	
		pEdit->ShowWindow(SW_SHOW);
		pEdit = (CEdit*)GetDlgItem(IDC_LABEL_POSTREC_DATARATE0);
		pEdit->ShowWindow(SW_SHOW);
		pEdit = (CEdit*)GetDlgItem(IDC_LABEL_POSTREC_DATARATE1);
		pEdit->ShowWindow(SW_SHOW);
	}
	else
	{
		pEdit->ShowWindow(SW_HIDE);
		pEdit = (CEdit*)GetDlgItem(IDC_LABEL_POSTREC_DATARATE0);
		pEdit->ShowWindow(SW_HIDE);
		pEdit = (CEdit*)GetDlgItem(IDC_LABEL_POSTREC_DATARATE1);
		pEdit->ShowWindow(SW_HIDE);
	}

	// Quality
	CSliderCtrl* pSlider = (CSliderCtrl*)GetDlgItem(IDC_VIDEO_COMPRESSION_QUALITY);
	if (m_VideoCompressionQualitySupport[m_VideoCompressionChoose.GetCurSel()]	&&
		(m_nVideoRecQualityBitrate == 0											||
		!m_VideoCompressionDataRateSupport[m_VideoCompressionChoose.GetCurSel()]))
	{
		pSlider->ShowWindow(SW_SHOW);
		pEdit = (CEdit*)GetDlgItem(IDC_VIDEO_COMPRESSION_QUALITY_NUM);
		pEdit->ShowWindow(SW_SHOW);
	}
	else
	{
		pSlider->ShowWindow(SW_HIDE);
		pEdit = (CEdit*)GetDlgItem(IDC_VIDEO_COMPRESSION_QUALITY_NUM);
		pEdit->ShowWindow(SW_HIDE);
	}
	pSlider = (CSliderCtrl*)GetDlgItem(IDC_VIDEO_POSTREC_COMPRESSION_QUALITY);
	if (m_VideoCompressionQualitySupport[m_VideoPostRecCompressionChoose.GetCurSel()]	&&
		(m_nVideoPostRecQualityBitrate == 0												||
		!m_VideoCompressionDataRateSupport[m_VideoPostRecCompressionChoose.GetCurSel()]))
	{
		pSlider->ShowWindow(SW_SHOW);
		pEdit = (CEdit*)GetDlgItem(IDC_VIDEO_POSTREC_COMPRESSION_QUALITY_NUM);
		pEdit->ShowWindow(SW_SHOW);
	}
	else
	{
		pSlider->ShowWindow(SW_HIDE);
		pEdit = (CEdit*)GetDlgItem(IDC_VIDEO_POSTREC_COMPRESSION_QUALITY_NUM);
		pEdit->ShowWindow(SW_HIDE);
	}

	// Quality / Bitrate radio
	CButton* pRadio;
	if (m_VideoCompressionDataRateSupport[m_VideoCompressionChoose.GetCurSel()] &&
		m_VideoCompressionQualitySupport[m_VideoCompressionChoose.GetCurSel()])
	{
		pRadio = (CButton*)GetDlgItem(IDC_RADIO_QUALITY);
		pRadio->ShowWindow(SW_SHOW);
		pRadio = (CButton*)GetDlgItem(IDC_RADIO_BITRATE);
		pRadio->ShowWindow(SW_SHOW);
	}
	else
	{
		pRadio = (CButton*)GetDlgItem(IDC_RADIO_QUALITY);
		pRadio->ShowWindow(SW_HIDE);
		pRadio = (CButton*)GetDlgItem(IDC_RADIO_BITRATE);
		pRadio->ShowWindow(SW_HIDE);
	}
	if (m_VideoCompressionDataRateSupport[m_VideoPostRecCompressionChoose.GetCurSel()] &&
		m_VideoCompressionQualitySupport[m_VideoPostRecCompressionChoose.GetCurSel()])
	{
		pRadio = (CButton*)GetDlgItem(IDC_RADIO_POSTREC_QUALITY);
		pRadio->ShowWindow(SW_SHOW);
		pRadio = (CButton*)GetDlgItem(IDC_RADIO_POSTREC_BITRATE);
		pRadio->ShowWindow(SW_SHOW);
	}
	else
	{
		pRadio = (CButton*)GetDlgItem(IDC_RADIO_POSTREC_QUALITY);
		pRadio->ShowWindow(SW_HIDE);
		pRadio = (CButton*)GetDlgItem(IDC_RADIO_POSTREC_BITRATE);
		pRadio->ShowWindow(SW_HIDE);
	}
}

BOOL CGeneralPage::OnInitDialog() 
{
	int i;
	
	// Init vars
	m_bRecDeinterlace = FALSE;
	m_bRecSizeSegmentation = FALSE;
	m_bPostRec = FALSE;
	m_nVideoPostRecKeyframesRate = 0;
	m_nVideoPostRecDataRate = 0;
	m_nVideoRecQualityBitrate = 0;
	m_nVideoPostRecQualityBitrate = 0;
	m_bRecTimeSegmentation = FALSE;
	m_nTimeSegmentationIndex = 0;
	m_bRecAutoOpen = TRUE;

	// Frame Rate Change Flag
	m_bDoChangeFrameRate = FALSE;

	// Init Autorun Var
	if (CVideoDeviceDoc::AutorunGetDeviceKey(m_pDoc->GetDevicePathName()) != _T(""))
		m_bAutorun = TRUE;
	else
		m_bAutorun = FALSE;

	// Init Rec Auto Open Var
	m_bRecAutoOpen = m_pDoc->m_bRecAutoOpen;

	// Init Segmentation Vars
	m_bRecTimeSegmentation = m_pDoc->m_bRecTimeSegmentation;
	m_nTimeSegmentationIndex = m_pDoc->m_nTimeSegmentationIndex;
	m_bRecSizeSegmentation = m_pDoc->m_bRecSizeSegmentation;
	m_nRecFileCount = m_pDoc->m_nRecFileCount;
	m_nRecFileSizeMB = (int)(m_pDoc->m_llRecFileSize >> 20);

	// Init Rec Vars
	m_nVideoRecKeyframesRate = m_pDoc->m_nVideoRecKeyframesRate;
	m_nVideoRecDataRate = m_pDoc->m_nVideoRecDataRate / 1000;
	m_nVideoRecQualityBitrate = m_pDoc->m_nVideoRecQualityBitrate;
	m_bRecDeinterlace = m_pDoc->m_bRecDeinterlace;
	
	// Init Post Rec Vars
	m_bPostRec = m_pDoc->m_bPostRec;
	m_nVideoPostRecKeyframesRate = m_pDoc->m_nVideoPostRecKeyframesRate;
	m_nVideoPostRecDataRate = m_pDoc->m_nVideoPostRecDataRate / 1000;
	m_nVideoPostRecQualityBitrate = m_pDoc->m_nVideoPostRecQualityBitrate;

	// Recordings Delete
	m_nDeleteRecordingsOlderThanDays = m_pDoc->m_nDeleteRecordingsOlderThanDays;

	// Init Scheduler Values
	if (!m_pDoc->m_pVideoAviDoc)
	{
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
	}
	else
	{
		CTime t = CTime::GetCurrentTime();
		m_SchedulerOnceDateStart = t;
		m_SchedulerOnceTimeStart = t;
		m_SchedulerOnceDateStop = t;
		m_SchedulerOnceTimeStop = t;
		m_SchedulerDailyTimeStart = t;
		m_SchedulerDailyTimeStop = t;
	}

	// Init Combo Box
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_TIME_SEGMENTATION);
	if (pComboBox)
	{
		pComboBox->AddString(ML_STRING(1558, "1 hour"));
		pComboBox->AddString(ML_STRING(1559, "2 hours"));
		pComboBox->AddString(ML_STRING(1560, "3 hours"));
		pComboBox->AddString(ML_STRING(1561, "6 hours"));
		pComboBox->AddString(ML_STRING(1562, "12 hours"));
		pComboBox->AddString(ML_STRING(1563, "24 hours"));
	}

	// Init Codec's Supports

	m_VideoCompressionFcc.Add((DWORD)BI_RGB);
	m_VideoCompressionKeyframesRateSupport.Add((DWORD)0);
	m_VideoCompressionDataRateSupport.Add((DWORD)0);
	m_VideoCompressionQualitySupport.Add((DWORD)0);

	m_VideoCompressionFcc.Add((DWORD)FCC('HFYU')); 
	m_VideoCompressionKeyframesRateSupport.Add((DWORD)0);
	m_VideoCompressionDataRateSupport.Add((DWORD)0);
	m_VideoCompressionQualitySupport.Add((DWORD)0);

	m_VideoCompressionFcc.Add((DWORD)FCC('FFVH')); 
	m_VideoCompressionKeyframesRateSupport.Add((DWORD)0);
	m_VideoCompressionDataRateSupport.Add((DWORD)0);
	m_VideoCompressionQualitySupport.Add((DWORD)0);

	m_VideoCompressionFcc.Add((DWORD)FCC('FFV1')); 
	m_VideoCompressionKeyframesRateSupport.Add((DWORD)0);
	m_VideoCompressionDataRateSupport.Add((DWORD)0);
	m_VideoCompressionQualitySupport.Add((DWORD)0);

	m_VideoCompressionFcc.Add((DWORD)FCC('MJPG')); 
	m_VideoCompressionKeyframesRateSupport.Add((DWORD)0);
	m_VideoCompressionDataRateSupport.Add((DWORD)0);
	m_VideoCompressionQualitySupport.Add((DWORD)1);

	if (((CUImagerApp*)::AfxGetApp())->m_bFFMpeg4VideoEnc)
	{
		m_VideoCompressionFcc.Add((DWORD)FCC('DIVX')); 
		m_VideoCompressionKeyframesRateSupport.Add((DWORD)1);
		m_VideoCompressionDataRateSupport.Add((DWORD)1);
		m_VideoCompressionQualitySupport.Add((DWORD)1);
	}

	if (((CUImagerApp*)::AfxGetApp())->m_bFFTheoraVideoEnc)
	{
		m_VideoCompressionFcc.Add((DWORD)FCC('theo')); 
		m_VideoCompressionKeyframesRateSupport.Add((DWORD)1);
		m_VideoCompressionDataRateSupport.Add((DWORD)1);
		m_VideoCompressionQualitySupport.Add((DWORD)1);
	}

	if (((CUImagerApp*)::AfxGetApp())->m_bFFSnowVideoEnc)
	{
		m_VideoCompressionFcc.Add((DWORD)FCC('SNOW')); 
		m_VideoCompressionKeyframesRateSupport.Add((DWORD)1);
		m_VideoCompressionDataRateSupport.Add((DWORD)1);
		m_VideoCompressionQualitySupport.Add((DWORD)1);
	}

	// Update Current Selected Codec
	int nVideoCompressionSelection = -1;
	for (i = 0 ; i < m_VideoCompressionFcc.GetSize() ; i++)
	{
		if (m_VideoCompressionFcc[i] == m_pDoc->m_dwVideoRecFourCC)
		{
			nVideoCompressionSelection = i;
			break;
		}
	}
	if (nVideoCompressionSelection == -1)
	{
		m_pDoc->m_dwVideoRecFourCC = FCC('MJPG');
		for (i = 0 ; i < m_VideoCompressionFcc.GetSize() ; i++)
		{
			if (m_VideoCompressionFcc[i] == m_pDoc->m_dwVideoRecFourCC)
			{
				nVideoCompressionSelection = i;
				break;
			}
		}
	}
	int nVideoPostRecCompressionSelection = -1;
	for (i = 0 ; i < m_VideoCompressionFcc.GetSize() ; i++)
	{
		if (m_VideoCompressionFcc[i] == m_pDoc->m_dwVideoPostRecFourCC)
		{
			nVideoPostRecCompressionSelection = i;
			break;
		}
	}
	if (nVideoPostRecCompressionSelection == -1)
	{
		m_pDoc->m_dwVideoPostRecFourCC = FCC('MJPG');
		for (i = 0 ; i < m_VideoCompressionFcc.GetSize() ; i++)
		{
			if (m_VideoCompressionFcc[i] == m_pDoc->m_dwVideoPostRecFourCC)
			{
				nVideoPostRecCompressionSelection = i;
				break;
			}
		}
	}

	// Update Quality / Bitrate selection radios
	if (m_VideoCompressionDataRateSupport[nVideoCompressionSelection] &&
		!m_VideoCompressionQualitySupport[nVideoCompressionSelection])
		m_pDoc->m_nVideoRecQualityBitrate = m_nVideoRecQualityBitrate = 1;
	else if (!m_VideoCompressionDataRateSupport[nVideoCompressionSelection] &&
			m_VideoCompressionQualitySupport[nVideoCompressionSelection])
		m_pDoc->m_nVideoRecQualityBitrate = m_nVideoRecQualityBitrate = 0;
	if (m_VideoCompressionDataRateSupport[nVideoPostRecCompressionSelection] &&
		!m_VideoCompressionQualitySupport[nVideoPostRecCompressionSelection])
		m_pDoc->m_nVideoPostRecQualityBitrate = m_nVideoPostRecQualityBitrate = 1;
	else if (!m_VideoCompressionDataRateSupport[nVideoPostRecCompressionSelection] &&
			m_VideoCompressionQualitySupport[nVideoPostRecCompressionSelection])
		m_pDoc->m_nVideoPostRecQualityBitrate = m_nVideoPostRecQualityBitrate = 0;

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
	else if (m_pDoc->m_pDxCaptureVMR9)
	{
		m_SpinFrameRate.SetRange(MIN_FRAMERATE, MAX_FRAMERATE);
	}
	else if (::IsWindow(m_pDoc->m_VfWCaptureVideoThread.m_hCapWnd))
	{
		m_SpinFrameRate.SetRange(MIN_FRAMERATE, MAX_FRAMERATE);
	}
	else if (m_pDoc->m_pGetFrameNetCom && m_pDoc->m_pGetFrameNetCom->IsClient())
	{
		// Axis and Edimax support only integer values starting at 1 fps
		if (m_pDoc->m_nNetworkDeviceTypeMode == CVideoDeviceDoc::AXIS_SP	||
			m_pDoc->m_nNetworkDeviceTypeMode == CVideoDeviceDoc::EDIMAX_SP)
			m_SpinFrameRate.SetRange(1.0, MAX_FRAMERATE);
		// Frequency not settable on client side by panasonic devices in server push mode
		else if (m_pDoc->m_nNetworkDeviceTypeMode == CVideoDeviceDoc::PANASONIC_SP)
		{
			m_SpinFrameRate.SetRange(0.0, 0.0);
			m_SpinFrameRate.EnableWindow(FALSE);
			pEdit->EnableWindow(FALSE);
		}
		else
			m_SpinFrameRate.SetRange(MIN_FRAMERATE, MAX_FRAMERATE); // Pixord in server push mode and all client poll
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

	// Recording Dir
	m_DirLabel.SubclassDlgItem(IDC_TEXT_VIDEO_REC, this);
	m_DirLabel.SetVisitedColor(RGB(0, 0, 255));
	m_DirLabel.SetLink(m_pDoc->m_sRecordAutoSaveDir);
	pEdit = (CEdit*)GetDlgItem(IDC_RECORD_SAVEAS_PATH);
	pEdit->SetWindowText(m_pDoc->m_sRecordAutoSaveDir);

	// Capture Audio Check Box
	CButton* pCheck = (CButton*)GetDlgItem(IDC_REC_AUDIO);
	if (m_pDoc->m_bCaptureAudio)
		pCheck->SetCheck(1);
	else
		pCheck->SetCheck(0);

	// Initialize Peak-Meter Control
	CStatic* pStatic = static_cast<CStatic*>(GetDlgItem(IDC_PEAKMETER));
	ASSERT(pStatic != NULL);
	CRect rc;
	pStatic->GetWindowRect(rc);
	pStatic->DestroyWindow(); // destroy previous window- was used for position
	ScreenToClient(rc);
	m_PeakMeter.Create(WS_CHILD|WS_VISIBLE|PMS_VERTICAL, rc, this, IDC_PEAKMETER);
	m_PeakMeter.SetMeterBands(2, 15);
	m_PeakMeter.Start(1000 / 25); // 25 fps

	// Video Compressor Quality
	m_VideoRecQuality.SetRange(2, 31);
	m_VideoRecQuality.SetPageSize(5);
	m_VideoRecQuality.SetLineSize(1);
	m_VideoRecQuality.SetPos(33 - (int)(m_pDoc->m_fVideoRecQuality)); // m_fVideoRecQuality has a range from 31.0f to 2.0f
	pEdit = (CEdit*)GetDlgItem(IDC_VIDEO_COMPRESSION_QUALITY_NUM);
	CString sQuality;
	sQuality.Format(_T("%i"), (int)((m_VideoRecQuality.GetPos() - 2) * 3.45)); // 0 .. 100
	pEdit->SetWindowText(sQuality);
	m_VideoPostRecQuality.SetRange(2, 31);
	m_VideoPostRecQuality.SetPageSize(5);
	m_VideoPostRecQuality.SetLineSize(1);
	m_VideoPostRecQuality.SetPos(33 - (int)(m_pDoc->m_fVideoPostRecQuality)); // m_fVideoPostRecQuality has a range from 31.0f to 2.0f
	pEdit = (CEdit*)GetDlgItem(IDC_VIDEO_POSTREC_COMPRESSION_QUALITY_NUM);
	sQuality.Format(_T("%i"), (int)((m_VideoPostRecQuality.GetPos() - 2) * 3.45)); // 0 .. 100
	pEdit->SetWindowText(sQuality);

	// Add Codec strings to ComboBoxes
	m_VideoCompressionChoose.AddString(_T("Raw"));
	m_VideoCompressionChoose.AddString(_T("Huffman YUV 16 bits/pix"));
	m_VideoCompressionChoose.AddString(_T("Huffman YUV 12 bits/pix"));
	m_VideoCompressionChoose.AddString(_T("FFV1 Lossless YUV 12 bits/pix"));
	m_VideoCompressionChoose.AddString(_T("Motion JPEG"));
	if (((CUImagerApp*)::AfxGetApp())->m_bFFMpeg4VideoEnc)
		m_VideoCompressionChoose.AddString(_T("MPEG-4"));
	if (((CUImagerApp*)::AfxGetApp())->m_bFFTheoraVideoEnc)
		m_VideoCompressionChoose.AddString(_T("Theora"));
	if (((CUImagerApp*)::AfxGetApp())->m_bFFSnowVideoEnc)
		m_VideoCompressionChoose.AddString(_T("SNOW"));
	m_VideoPostRecCompressionChoose.AddString(_T("Raw"));
	m_VideoPostRecCompressionChoose.AddString(_T("Huffman YUV 16 bits/pix"));
	m_VideoPostRecCompressionChoose.AddString(_T("Huffman YUV 12 bits/pix"));
	m_VideoPostRecCompressionChoose.AddString(_T("FFV1 Lossless YUV 12 bits/pix"));
	m_VideoPostRecCompressionChoose.AddString(_T("Motion JPEG"));
	if (((CUImagerApp*)::AfxGetApp())->m_bFFMpeg4VideoEnc)
		m_VideoPostRecCompressionChoose.AddString(_T("MPEG-4"));
	if (((CUImagerApp*)::AfxGetApp())->m_bFFTheoraVideoEnc)
		m_VideoPostRecCompressionChoose.AddString(_T("Theora"));
	if (((CUImagerApp*)::AfxGetApp())->m_bFFSnowVideoEnc)
		m_VideoPostRecCompressionChoose.AddString(_T("SNOW"));

	// Set Current Selections
	m_VideoCompressionChoose.SetCurSel(nVideoCompressionSelection);
	m_VideoPostRecCompressionChoose.SetCurSel(nVideoPostRecCompressionSelection);

	// Show Hide Ctrl
	ShowHideCtrls();

	// Enable Format Button?
	CButton* pButton = (CButton*)GetDlgItem(IDC_VIDEO_FORMAT);
	if ((m_pDoc->m_pDxCapture && m_pDoc->m_pDxCapture->HasFormats())	||
		(m_pDoc->m_pDxCapture && m_pDoc->m_pDxCapture->IsDV() && m_pDoc->m_pDxCapture->HasDVFormatDlg()) ||
		::IsWindow(m_pDoc->m_VfWCaptureVideoThread.m_hCapWnd)			||
		(m_pDoc->m_pGetFrameNetCom										&&
		m_pDoc->m_pGetFrameNetCom->IsClient()							&&
		m_pDoc->m_nNetworkDeviceTypeMode != CVideoDeviceDoc::OTHERONE))
		pButton->EnableWindow(TRUE);
	else
		pButton->EnableWindow(FALSE);

	// Enable Source Button?
	pButton = (CButton*)GetDlgItem(IDC_VIDEO_SOURCE);
	if (m_pDoc->m_pDxCapture)
	{
		if (m_pDoc->m_pDxCapture->HasVideoCaptureFilterDlg())
			pButton->EnableWindow(TRUE);
		else
			pButton->EnableWindow(FALSE);
	}
	else if (::IsWindow(m_pDoc->m_VfWCaptureVideoThread.m_hCapWnd))
		pButton->EnableWindow(TRUE); // VfW Has This Button
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
	else if (m_pDoc->m_pDxCaptureVMR9)
	{
		if (m_pDoc->m_pDxCaptureVMR9->HasVideoTVTunerDlg())
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
	else if (m_pDoc->m_pDxCaptureVMR9)
	{
		if (m_pDoc->m_pDxCaptureVMR9->GetInputsCount() > 0)
			pButton->EnableWindow(TRUE);
		else
			pButton->EnableWindow(FALSE);
	}
	else
		pButton->EnableWindow(FALSE);

	// Set scheduler date range
	CDateTimeCtrl* pDateTimeCtrl = (CDateTimeCtrl*)GetDlgItem(IDC_DATE_ONCE_START);
	CTime t1(CTime::GetCurrentTime());
#if _MFC_VER >= 0x0700
	CTime t2(3000, 12, 31, 23, 59, 59);
#else
	CTime t2(2037, 12, 31, 23, 59, 59);
#endif
	pDateTimeCtrl->SetRange(&t1, &t2);
	pDateTimeCtrl = (CDateTimeCtrl*)GetDlgItem(IDC_DATE_ONCE_STOP);
	pDateTimeCtrl->SetRange(&t1, &t2);

	// Disable Scheduler For AVI File Source Device
	if (m_pDoc->m_pVideoAviDoc)
	{
		pCheck = (CButton*)GetDlgItem(IDC_CHECK_SCHEDULER_ONCE);
		pCheck->EnableWindow(FALSE);
		pCheck = (CButton*)GetDlgItem(IDC_CHECK_SCHEDULER_DAILY);
		pCheck->EnableWindow(FALSE);

		CWnd* pWnd = (CWnd*)GetDlgItem(IDC_TIME_ONCE_START);
		pWnd->EnableWindow(FALSE);
		pWnd = (CWnd*)GetDlgItem(IDC_TIME_ONCE_STOP);
		pWnd->EnableWindow(FALSE);
		pWnd = (CWnd*)GetDlgItem(IDC_DATE_ONCE_START);
		pWnd->EnableWindow(FALSE);
		pWnd = (CWnd*)GetDlgItem(IDC_DATE_ONCE_STOP);
		pWnd->EnableWindow(FALSE);
		pWnd = (CWnd*)GetDlgItem(IDC_TIME_DAILY_START);
		pWnd->EnableWindow(FALSE);
		pWnd = (CWnd*)GetDlgItem(IDC_TIME_DAILY_STOP);
		pWnd->EnableWindow(FALSE);
	}
	// Init Scheduler Values if set
	else
	{
		CUImagerApp::CSchedulerEntry* pOnceSchedulerEntry =
			((CUImagerApp*)::AfxGetApp())->GetOnceSchedulerEntry(m_pDoc->GetDevicePathName());

		CUImagerApp::CSchedulerEntry* pDailySchedulerEntry =
			((CUImagerApp*)::AfxGetApp())->GetDailySchedulerEntry(m_pDoc->GetDevicePathName());
		
		// Init Once Scheduler's Check-Boxes
		if (pOnceSchedulerEntry)
		{
			pCheck = (CButton*)GetDlgItem(IDC_CHECK_SCHEDULER_ONCE);
			pCheck->SetCheck(1);
			CWnd* pWnd = (CWnd*)GetDlgItem(IDC_TIME_ONCE_START);
			pWnd->EnableWindow(FALSE);
			pWnd = (CWnd*)GetDlgItem(IDC_TIME_ONCE_STOP);
			pWnd->EnableWindow(FALSE);
			pWnd = (CWnd*)GetDlgItem(IDC_DATE_ONCE_START);
			pWnd->EnableWindow(FALSE);
			pWnd = (CWnd*)GetDlgItem(IDC_DATE_ONCE_STOP);
			pWnd->EnableWindow(FALSE);
		}

		// Init Daily Scheduler's Check-Boxes
		if (pDailySchedulerEntry)
		{
			pCheck = (CButton*)GetDlgItem(IDC_CHECK_SCHEDULER_DAILY);
			pCheck->SetCheck(1);
			CWnd* pWnd = (CWnd*)GetDlgItem(IDC_TIME_DAILY_START);
			pWnd->EnableWindow(FALSE);
			pWnd = (CWnd*)GetDlgItem(IDC_TIME_DAILY_STOP);
			pWnd->EnableWindow(FALSE);
		}
	}

	// OnInitDialog() has been called
	m_bDlgInitialized = TRUE;

	// Disable Critical Controls?
	if (m_pDoc->GetView()->AreCriticalControlsDisabled())
		EnableDisableCriticalControls(FALSE);

	// Set Page Pointer to this
	m_pDoc->m_pGeneralPage = this;

	// Set Timer
	SetTimer(ID_TIMER_GENERALDLG, GENERALDLG_TIMER_MS, NULL);

	// Disable Autorun?
	if (m_pDoc->m_pVideoAviDoc || !((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
	{
		pCheck = (CButton*)GetDlgItem(IDC_CHECK_AUTORUN);
		pCheck->EnableWindow(FALSE);
	}

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
			m_pDoc->StopProcessFrame();
			m_nFrameRateChangeTimeout = FRAMERATE_CHANGE_TIMEOUT;
			if (!m_bDoChangeFrameRate)
			{
				// Done in OnTimer()
				m_bDoChangeFrameRate = TRUE;

				// Disable Critical Controls
				::SendMessage(	m_pDoc->GetView()->GetSafeHwnd(),
								WM_ENABLE_DISABLE_CRITICAL_CONTROLS,
								(WPARAM)FALSE,	// Disable Them
								(LPARAM)0);
			}
		}
	}
}

void CGeneralPage::OnRecAudio() 
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_REC_AUDIO);
	m_pDoc->m_bCaptureAudio = (pCheck->GetCheck() == 1);
	if (m_pDoc->m_bCaptureAudio)
	{
		m_pDoc->m_CaptureAudioThread.m_Mixer.Close();
		m_pDoc->m_CaptureAudioThread.Start();
	}
	else
		m_pDoc->m_CaptureAudioThread.Kill_NoBlocking();
}

void CGeneralPage::OnTimer(UINT nIDEvent) 
{
	if (!m_pDoc->m_bClosing)
	{	
		// Open Mixer If Not Open
		if (m_pDoc->m_CaptureAudioThread.IsOpen() && !m_pDoc->m_CaptureAudioThread.m_Mixer.IsWithWndHandleOpen())
		{
			// Open Mixer
			m_pDoc->m_CaptureAudioThread.m_Mixer.Open(m_pDoc->m_CaptureAudioThread.GetWaveHandle(), GetSafeHwnd());

			// Adjust Volume Slider
			DWORD dwVolLeft, dwVolRight;
			if (m_pDoc->m_CaptureAudioThread.m_Mixer.GetDstVolumeControlID() != 0xFFFFFFFF)
			{
				m_RecVolumeLeft.SetRange(	m_pDoc->m_CaptureAudioThread.m_Mixer.GetDstVolumeControlMin(),
											m_pDoc->m_CaptureAudioThread.m_Mixer.GetDstVolumeControlMax(), TRUE);
				m_RecVolumeRight.SetRange(	m_pDoc->m_CaptureAudioThread.m_Mixer.GetDstVolumeControlMin(),
											m_pDoc->m_CaptureAudioThread.m_Mixer.GetDstVolumeControlMax(), TRUE);
				if (m_pDoc->m_CaptureAudioThread.m_Mixer.GetDstVolume(dwVolLeft, dwVolRight))
				{
					m_RecVolumeLeft.SetPos(dwVolLeft);	
					m_RecVolumeRight.SetPos(dwVolRight);
				}
			}
			else if (m_pDoc->m_CaptureAudioThread.m_Mixer.GetSrcVolumeControlID() != 0xFFFFFFFF)
			{
				m_RecVolumeLeft.SetRange(	m_pDoc->m_CaptureAudioThread.m_Mixer.GetSrcVolumeControlMin(),
											m_pDoc->m_CaptureAudioThread.m_Mixer.GetSrcVolumeControlMax(), TRUE);
				m_RecVolumeRight.SetRange(	m_pDoc->m_CaptureAudioThread.m_Mixer.GetSrcVolumeControlMin(),
											m_pDoc->m_CaptureAudioThread.m_Mixer.GetSrcVolumeControlMax(), TRUE);
				if (m_pDoc->m_CaptureAudioThread.m_Mixer.GetSrcVolume(dwVolLeft, dwVolRight))
				{
					m_RecVolumeLeft.SetPos(dwVolLeft);	
					m_RecVolumeRight.SetPos(dwVolRight);
				}
			}

			// If Device Has Both Dst & Src, Turn Src Volume to Maximum!
			if (m_pDoc->m_CaptureAudioThread.m_Mixer.GetDstVolumeControlID() != 0xFFFFFFFF &&
				m_pDoc->m_CaptureAudioThread.m_Mixer.GetSrcVolumeControlID() != 0xFFFFFFFF)
			{
				m_pDoc->m_CaptureAudioThread.m_Mixer.SetSrcVolume(	m_pDoc->m_CaptureAudioThread.m_Mixer.GetSrcVolumeControlMax(),
																	m_pDoc->m_CaptureAudioThread.m_Mixer.GetSrcVolumeControlMax());
			}

			// Set Muted Warning Label if muted
			CEdit* pEdit = (CEdit*)GetDlgItem(IDC_VOLUME_MUTED);
			if (m_pDoc->m_CaptureAudioThread.m_Mixer.GetDstMute() ||
				m_pDoc->m_CaptureAudioThread.m_Mixer.GetSrcMute())
				pEdit->SetWindowText(ML_STRING(1455, "Muted!"));
			else
				pEdit->SetWindowText(_T(""));
		}

		// Enable / Disable Volume Slider
		if (!m_pDoc->m_CaptureAudioThread.IsOpen())
		{
			CSliderCtrl* pSlider;
			pSlider = (CSliderCtrl*)GetDlgItem(IDC_REC_VOL_LEFT);
			pSlider->SetPos(0);
			pSlider->EnableWindow(FALSE);
			pSlider = (CSliderCtrl*)GetDlgItem(IDC_REC_VOL_RIGHT);
			pSlider->SetPos(0);
			pSlider->EnableWindow(FALSE);
		}
		else
		{
			CSliderCtrl* pSlider;
			pSlider = (CSliderCtrl*)GetDlgItem(IDC_REC_VOL_LEFT);
			pSlider->EnableWindow(TRUE);
			pSlider = (CSliderCtrl*)GetDlgItem(IDC_REC_VOL_RIGHT);
			pSlider->EnableWindow(TRUE);
		}

		// If Mean Level is Older Than a Second -> Set Peak Meter To Zero
		CTime CurrentTime = CTime::GetCurrentTime();
		CTimeSpan ElapsedTime = CurrentTime - m_pDoc->m_CaptureAudioThread.GetMeanLevelTime();
		int Data[2];
		if (ElapsedTime.GetTotalSeconds() > 1)
		{
			Data[1] = Data[0] = 0;
			m_PeakMeter.SetData(Data, 0, 2);
		}

		// Inconsistency Detection:
		// audio input may not be able to start
		if ((m_pDoc->m_CaptureAudioThread.IsRunning()	&&
			m_pDoc->m_bCaptureAudio == FALSE)			||
			(!m_pDoc->m_CaptureAudioThread.IsRunning()	&&
			m_pDoc->m_bCaptureAudio == TRUE))
			m_nAudioCaptureInconsistencyTimeout--;
		else
			m_nAudioCaptureInconsistencyTimeout = AUDIOCAPTURE_INCONSISTENCY_TIMEOUT;
		if (m_nAudioCaptureInconsistencyTimeout <= 0)
		{
			CButton* pCheck = (CButton*)GetDlgItem(IDC_REC_AUDIO);
			if (m_pDoc->m_CaptureAudioThread.IsRunning())
			{
				pCheck->SetCheck(1);
				m_pDoc->m_bCaptureAudio = TRUE;
			}
			else
			{
				pCheck->SetCheck(0);
				m_pDoc->m_bCaptureAudio = FALSE;
			}
		}
		
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
			else if (m_pDoc->m_pDxCaptureVMR9)
			{
				LONG lDroppedFrames = m_pDoc->m_pDxCaptureVMR9->GetDroppedFrames();
				if (lDroppedFrames >= 0)
					sDroppedFrames.Format(_T("%d"), lDroppedFrames);
				else
					sDroppedFrames = _T("0");	// Unsupported
			}
			else if (::IsWindow(m_pDoc->m_VfWCaptureVideoThread.m_hCapWnd))
			{
				int nDroppedFrames = m_pDoc->m_VfWCaptureVideoThread.GetDroppedFrames();
				if (nDroppedFrames >= 0)
					sDroppedFrames.Format(_T("%d"), nDroppedFrames);
				else
					sDroppedFrames = _T("0");	// Unsupported
			}
			else if (m_pDoc->m_pGetFrameParseProcess)
				sDroppedFrames.Format(_T("%u"), m_pDoc->m_pGetFrameParseProcess->GetLostCount());
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
			LONG lUncompressedAvgFrameSize = m_pDoc->m_OrigBMI.bmiHeader.biSizeImage;
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

		// Change The Frame Rate if Necessary
		if (m_bDoChangeFrameRate)
		{
			--m_nFrameRateChangeTimeout;
			if (m_nFrameRateChangeTimeout <= 0 && m_pDoc->IsProcessFrameStopped())
			{
				// Reset flag
				m_bDoChangeFrameRate = FALSE;

				// Frame Rate Edit Control
				CString sFrameRate;
				pEdit = (CEdit*)GetDlgItem(IDC_FRAMERATE);
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
					m_pDoc->ReStartProcessFrame();
					sFrameRate.Format(_T("%0.1f"), m_pDoc->m_dFrameRate);
					pEdit->SetWindowText(sFrameRate);
					pEdit->SetFocus();
					pEdit->SetSel(0xFFFF0000);
				}

				// Enable Critical Controls
				::SendMessage(	m_pDoc->GetView()->GetSafeHwnd(),
								WM_ENABLE_DISABLE_CRITICAL_CONTROLS,
								(WPARAM)TRUE,	// Enable Them
								(LPARAM)0);
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
			(SB_PAGEDOWN == nSBCode) ||		// Mouse Press Belove Slider
			(SB_LEFT == nSBCode)	||		// Home Button
			(SB_RIGHT == nSBCode))			// End Button  
		{
			if (pSlider->GetDlgCtrlID() == IDC_REC_VOL_LEFT)
			{
				DWORD dwVolLeft = 0;
				DWORD dwVolRight = 0;
				if (m_pDoc->m_CaptureAudioThread.m_Mixer.GetDstVolumeControlID() != 0xFFFFFFFF)
				{
					m_pDoc->m_CaptureAudioThread.m_Mixer.GetDstVolume(dwVolLeft, dwVolRight);
					dwVolLeft = (DWORD)((double)m_RecVolumeLeft.GetPos() / (double)m_RecVolumeLeft.GetRangeMax() *
									(double)m_pDoc->m_CaptureAudioThread.m_Mixer.GetDstVolumeControlMax());
					m_pDoc->m_CaptureAudioThread.m_Mixer.SetDstVolume(dwVolLeft, dwVolRight);
				}
				else if (m_pDoc->m_CaptureAudioThread.m_Mixer.GetSrcVolumeControlID() != 0xFFFFFFFF)
				{
					m_pDoc->m_CaptureAudioThread.m_Mixer.GetSrcVolume(dwVolLeft, dwVolRight);
					dwVolLeft = (DWORD)((double)m_RecVolumeLeft.GetPos() / (double)m_RecVolumeLeft.GetRangeMax() *
									(double)m_pDoc->m_CaptureAudioThread.m_Mixer.GetSrcVolumeControlMax());
					m_pDoc->m_CaptureAudioThread.m_Mixer.SetSrcVolume(dwVolLeft, dwVolRight);
				}
			}
			else if (pSlider->GetDlgCtrlID() == IDC_REC_VOL_RIGHT)
			{
				DWORD dwVolLeft = 0;
				DWORD dwVolRight = 0;
				if (m_pDoc->m_CaptureAudioThread.m_Mixer.GetDstVolumeControlID() != 0xFFFFFFFF)
				{
					m_pDoc->m_CaptureAudioThread.m_Mixer.GetDstVolume(dwVolLeft, dwVolRight);
					dwVolRight = (DWORD)((double)m_RecVolumeRight.GetPos() / (double)m_RecVolumeRight.GetRangeMax() *
										 (double)m_pDoc->m_CaptureAudioThread.m_Mixer.GetDstVolumeControlMax());
					m_pDoc->m_CaptureAudioThread.m_Mixer.SetDstVolume(dwVolLeft, dwVolRight);
				}
				else if (m_pDoc->m_CaptureAudioThread.m_Mixer.GetSrcVolumeControlID() != 0xFFFFFFFF)
				{
					m_pDoc->m_CaptureAudioThread.m_Mixer.GetSrcVolume(dwVolLeft, dwVolRight);
					dwVolRight = (DWORD)((double)m_RecVolumeRight.GetPos() / (double)m_RecVolumeRight.GetRangeMax() *
										 (double)m_pDoc->m_CaptureAudioThread.m_Mixer.GetSrcVolumeControlMax());
					m_pDoc->m_CaptureAudioThread.m_Mixer.SetSrcVolume(dwVolLeft, dwVolRight);
				}
			}
			else if (pSlider->GetDlgCtrlID() == IDC_VIDEO_COMPRESSION_QUALITY)
			{
				m_pDoc->m_fVideoRecQuality = (float)(33 - m_VideoRecQuality.GetPos());
				CEdit* pEdit = (CEdit*)GetDlgItem(IDC_VIDEO_COMPRESSION_QUALITY_NUM);
				CString sQuality;
				sQuality.Format(_T("%i"), (int)((m_VideoRecQuality.GetPos() - 2) * 3.45)); // 0 .. 100
				pEdit->SetWindowText(sQuality);
			}
			else if (pSlider->GetDlgCtrlID() == IDC_VIDEO_POSTREC_COMPRESSION_QUALITY)
			{
				m_pDoc->m_fVideoPostRecQuality = (float)(33 - m_VideoPostRecQuality.GetPos());
				CEdit* pEdit = (CEdit*)GetDlgItem(IDC_VIDEO_POSTREC_COMPRESSION_QUALITY_NUM);
				CString sQuality;
				sQuality.Format(_T("%i"), (int)((m_VideoPostRecQuality.GetPos() - 2) * 3.45)); // 0 .. 100
				pEdit->SetWindowText(sQuality);
			}
		}
	}
	
	CPropertyPage::OnHScroll(nSBCode, nPos, (CScrollBar*)pScrollBar);
}

void CGeneralPage::OnCheckDeinterlace() 
{
	UpdateData(TRUE);
	m_pDoc->m_bRecDeinterlace = m_bRecDeinterlace;
}

void CGeneralPage::OnChangeEditKeyframesRate() 
{
	UpdateData(TRUE);
	m_pDoc->m_nVideoRecKeyframesRate = m_nVideoRecKeyframesRate;
}

void CGeneralPage::OnChangeEditDatarate() 
{
	UpdateData(TRUE);
	m_pDoc->m_nVideoRecDataRate = m_nVideoRecDataRate * 1000;
}

void CGeneralPage::OnChangeEditPostrecDatarate() 
{
	UpdateData(TRUE);
	m_pDoc->m_nVideoPostRecDataRate = m_nVideoPostRecDataRate * 1000;
}

void CGeneralPage::OnChangeEditPostrecKeyframesRate() 
{
	UpdateData(TRUE);
	m_pDoc->m_nVideoPostRecKeyframesRate = m_nVideoPostRecKeyframesRate;
}

void CGeneralPage::OnChangeEditDeleteRecordingsDays() 
{
	if (UpdateData(TRUE))
	{
		m_pDoc->m_DeleteThread.Kill();
		m_pDoc->m_nDeleteRecordingsOlderThanDays = m_nDeleteRecordingsOlderThanDays;
		m_pDoc->m_DeleteThread.Start(THREAD_PRIORITY_LOWEST);
	}
}

void CGeneralPage::OnCheckAutoopen() 
{
	UpdateData(TRUE);
	m_pDoc->m_bRecAutoOpen = m_bRecAutoOpen;
}

void CGeneralPage::OnCheckSizeSegmentation() 
{
	UpdateData(TRUE);
	m_pDoc->m_bRecSizeSegmentation = m_bRecSizeSegmentation;

	// Clear Time Segmentation
	if (m_bRecSizeSegmentation && m_bRecTimeSegmentation)
	{
		m_pDoc->m_bRecTimeSegmentation = m_bRecTimeSegmentation = FALSE;
		UpdateData(FALSE);
	}
}

void CGeneralPage::OnCheckTimeSegmentation() 
{
	UpdateData(TRUE);
	m_pDoc->m_bRecTimeSegmentation = m_bRecTimeSegmentation;
	
	// Clear Size Segmentation
	if (m_bRecTimeSegmentation && m_bRecSizeSegmentation)
	{
		m_pDoc->m_bRecSizeSegmentation = m_bRecSizeSegmentation = FALSE;
		UpdateData(FALSE);
	}
}

void CGeneralPage::OnSelchangeTimeSegmentation() 
{
	UpdateData(TRUE);
	m_pDoc->m_nTimeSegmentationIndex = m_nTimeSegmentationIndex;
}

void CGeneralPage::OnChangeRecfileCount() 
{
	UpdateData(TRUE);
	m_pDoc->m_nRecFileCount = m_nRecFileCount;
}

void CGeneralPage::OnChangeRecfileSize() 
{
	UpdateData(TRUE);
	m_pDoc->m_llRecFileSize = ((LONGLONG)m_nRecFileSizeMB) << 20;
}

void CGeneralPage::OnCheckPostrec() 
{
	UpdateData(TRUE);
	m_pDoc->m_bPostRec = m_bPostRec;
}

void CGeneralPage::OnCheckAutorun() 
{
	UpdateData(TRUE);
	if (m_bAutorun)
		CVideoDeviceDoc::AutorunAddDevice(m_pDoc->GetDevicePathName());
	else
		CVideoDeviceDoc::AutorunRemoveDevice(m_pDoc->GetDevicePathName());
}

void CGeneralPage::OnRadioQuality() 
{
	UpdateData(TRUE);
	m_pDoc->m_nVideoRecQualityBitrate = m_nVideoRecQualityBitrate;
	ShowHideCtrls();
}

void CGeneralPage::OnRadioPostrecQuality() 
{
	UpdateData(TRUE);
	m_pDoc->m_nVideoPostRecQualityBitrate = m_nVideoPostRecQualityBitrate;
	ShowHideCtrls();
}

void CGeneralPage::OnRadioBitrate() 
{
	UpdateData(TRUE);
	m_pDoc->m_nVideoRecQualityBitrate = m_nVideoRecQualityBitrate;
	ShowHideCtrls();
}

void CGeneralPage::OnRadioPostrecBitrate() 
{
	UpdateData(TRUE);
	m_pDoc->m_nVideoPostRecQualityBitrate = m_nVideoPostRecQualityBitrate;
	ShowHideCtrls();
}

void CGeneralPage::EnableDisableCriticalControls(BOOL bEnable)
{
	CEdit* pEdit;
	CSpinButtonCtrl* pSpin;
	CButton* pButton;
	CButton* pCheck;
	CComboBox* pComboBox;
	CButton* pRadio;
	CSliderCtrl* pSlider;

	// Enable Framerate?
	if (!m_bDoChangeFrameRate)
	{
		pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_FRAMERATE);
		pEdit = (CEdit*)GetDlgItem(IDC_FRAMERATE);
		if (bEnable)
		{
			if (m_pDoc->m_pDxCapture)
			{
				if (m_pDoc->m_pDxCapture->GetFrameRate() <= 0.0) // Not Settable
				{
					pSpin->EnableWindow(FALSE);
					pEdit->EnableWindow(FALSE);
				}
				else
				{
					pSpin->EnableWindow(TRUE);
					pEdit->EnableWindow(TRUE);
				}
			}
			else if (m_pDoc->m_pDxCaptureVMR9)
			{
				pSpin->EnableWindow(TRUE);
				pEdit->EnableWindow(TRUE);
			}
			else if (::IsWindow(m_pDoc->m_VfWCaptureVideoThread.m_hCapWnd))
			{
				pSpin->EnableWindow(TRUE);
				pEdit->EnableWindow(TRUE);
			}
			else if (m_pDoc->m_pGetFrameNetCom && m_pDoc->m_pGetFrameNetCom->IsClient())
			{
				if (m_pDoc->m_nNetworkDeviceTypeMode == CVideoDeviceDoc::PANASONIC_SP)
				{
					pSpin->EnableWindow(FALSE);
					pEdit->EnableWindow(FALSE);
				}
				else
				{
					pSpin->EnableWindow(TRUE);
					pEdit->EnableWindow(TRUE);
				}
			}
			else
			{
				pSpin->EnableWindow(FALSE);
				pEdit->EnableWindow(FALSE);
			}
		}
		else
		{
			pSpin->EnableWindow(FALSE);
			pEdit->EnableWindow(FALSE);
		}
	}

	// Enable Rec. Dir Button?
	pButton = (CButton*)GetDlgItem(IDC_RECORD_SAVEAS);
	pButton->EnableWindow(bEnable);

	// Enable Time Segmentation Check Box?
	pCheck = (CButton*)GetDlgItem(IDC_CHECK_TIME_SEGMENTATION);
	pCheck->EnableWindow(bEnable);

	// Enable Time Segmentation Combo Box?
	pComboBox = (CComboBox*)GetDlgItem(IDC_TIME_SEGMENTATION);
	pComboBox->EnableWindow(bEnable);

	// Enable Size Segmentation Check Box?
	pCheck = (CButton*)GetDlgItem(IDC_CHECK_SIZE_SEGMENTATION);
	pCheck->EnableWindow(bEnable);

	// Enable Rec. File Count Edit Box?
	pEdit = (CEdit*)GetDlgItem(IDC_RECFILE_COUNT);
	pEdit->EnableWindow(bEnable);

	// Enable Rec. File Size Edit Box?
	pEdit = (CEdit*)GetDlgItem(IDC_RECFILE_SIZE);
	pEdit->EnableWindow(bEnable);

	// Enable Rec Audio Check Box?
	pCheck = (CButton*)GetDlgItem(IDC_REC_AUDIO);
	pCheck->EnableWindow(bEnable);

	// Enable Audio Source Button?
	pButton = (CButton*)GetDlgItem(IDC_AUDIO_INPUT);
	pButton->EnableWindow(bEnable);

	// Enable Audio Format Button?
	pButton = (CButton*)GetDlgItem(IDC_AUDIO_FORMAT);
	pButton->EnableWindow(bEnable);

	// Enable Format Button?
	pButton = (CButton*)GetDlgItem(IDC_VIDEO_FORMAT);
	if (bEnable)
	{
		if ((m_pDoc->m_pDxCapture && m_pDoc->m_pDxCapture->HasFormats())	||
			(m_pDoc->m_pDxCapture && m_pDoc->m_pDxCapture->IsDV() && m_pDoc->m_pDxCapture->HasDVFormatDlg()) ||
			::IsWindow(m_pDoc->m_VfWCaptureVideoThread.m_hCapWnd)			||
			(m_pDoc->m_pGetFrameNetCom										&&
			m_pDoc->m_pGetFrameNetCom->IsClient()							&&
			m_pDoc->m_nNetworkDeviceTypeMode != CVideoDeviceDoc::OTHERONE))
			pButton->EnableWindow(TRUE);
		else
			pButton->EnableWindow(FALSE);
	}
	else
		pButton->EnableWindow(FALSE);

	// Enable Source Button?
	pButton = (CButton*)GetDlgItem(IDC_VIDEO_SOURCE);
	if (bEnable)
	{
		if (m_pDoc->m_pDxCapture)
		{
			if (m_pDoc->m_pDxCapture->HasVideoCaptureFilterDlg())
				pButton->EnableWindow(TRUE);
			else
				pButton->EnableWindow(FALSE);
		}
		else if (::IsWindow(m_pDoc->m_VfWCaptureVideoThread.m_hCapWnd))
			pButton->EnableWindow(TRUE); // VfW Has This Button
		else
			pButton->EnableWindow(FALSE);
	}
	else
		pButton->EnableWindow(FALSE);

	// Enable TV-Tuner Button?
	pButton = (CButton*)GetDlgItem(IDC_VIDEO_TUNER);
	if (bEnable)
	{
		if (m_pDoc->m_pDxCapture)
		{
			if (m_pDoc->m_pDxCapture->HasVideoTVTunerDlg())
				pButton->EnableWindow(TRUE);
			else
				pButton->EnableWindow(FALSE);
		}
		else if (m_pDoc->m_pDxCaptureVMR9)
		{
			if (m_pDoc->m_pDxCaptureVMR9->HasVideoTVTunerDlg())
				pButton->EnableWindow(TRUE);
			else
				pButton->EnableWindow(FALSE);
		}
		else
			pButton->EnableWindow(FALSE);	// VfW Has Not This Button
	}
	else
		pButton->EnableWindow(FALSE);

	// Enable Input Button?
	// (It's for selecting video inputs like S-Video, TV-Tuner,...)
	pButton = (CButton*)GetDlgItem(IDC_VIDEO_INPUT);
	if (bEnable)
	{
		if (m_pDoc->m_pDxCapture)
		{
			if (m_pDoc->m_pDxCapture->GetInputsCount() > 0)
				pButton->EnableWindow(TRUE);
			else
				pButton->EnableWindow(FALSE);
		}
		else if (m_pDoc->m_pDxCaptureVMR9)
		{
			if (m_pDoc->m_pDxCaptureVMR9->GetInputsCount() > 0)
				pButton->EnableWindow(TRUE);
			else
				pButton->EnableWindow(FALSE);
		}
		else
			pButton->EnableWindow(FALSE);	// VfW Has Not This Button!
	}
	else
		pButton->EnableWindow(FALSE);

	// Video Compression Choose?
	pComboBox = (CComboBox*)GetDlgItem(IDC_VIDEO_COMPRESSION_CHOOSE);
	pComboBox->EnableWindow(bEnable);

	// Video Compression Data Rate?
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_DATARATE);
	pEdit->EnableWindow(bEnable);

	// Video Compression Keyframes Rate?
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_KEYFRAMES_RATE);
	pEdit->EnableWindow(bEnable);

	// Video Compression Quality Slider?
	pSlider = (CSliderCtrl*)GetDlgItem(IDC_VIDEO_COMPRESSION_QUALITY);
	pSlider->EnableWindow(bEnable);

	// Quality Radio Button 
	pRadio = (CButton*)GetDlgItem(IDC_RADIO_QUALITY);
	pRadio->EnableWindow(bEnable);

	// Data Rate Radio Button 
	pRadio = (CButton*)GetDlgItem(IDC_RADIO_BITRATE);
	pRadio->EnableWindow(bEnable);

	// Video Post Record Check Box?
	pCheck = (CButton*)GetDlgItem(IDC_CHECK_POSTREC);
	pCheck->EnableWindow(bEnable);

	// Video Compression Choose?
	pComboBox = (CComboBox*)GetDlgItem(IDC_VIDEO_POSTREC_COMPRESSION_CHOOSE);
	pComboBox->EnableWindow(bEnable);

	// Video Compression Data Rate?
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_POSTREC_DATARATE);
	pEdit->EnableWindow(bEnable);

	// Video Compression Keyframes Rate?
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_POSTREC_KEYFRAMES_RATE);
	pEdit->EnableWindow(bEnable);

	// Video Compression Quality Slider?
	pSlider = (CSliderCtrl*)GetDlgItem(IDC_VIDEO_POSTREC_COMPRESSION_QUALITY);
	pSlider->EnableWindow(bEnable);

	// Quality Radio Button 
	pRadio = (CButton*)GetDlgItem(IDC_RADIO_POSTREC_QUALITY);
	pRadio->EnableWindow(bEnable);

	// Data Rate Radio Button 
	pRadio = (CButton*)GetDlgItem(IDC_RADIO_POSTREC_BITRATE);
	pRadio->EnableWindow(bEnable);
}

void CGeneralPage::OnAudioInput() 
{
	m_pDoc->m_CaptureAudioThread.AudioInSourceDialog();	
}

LONG CGeneralPage::OnPeakMeterUpdate(WPARAM wParam, LPARAM lParam)
{
	int nLevels[2];
	nLevels[0] = (DWORD)wParam;
	nLevels[1] = (DWORD)lParam;
	m_PeakMeter.SetData(nLevels, 0, 2);
	return 0;
}

LRESULT CGeneralPage::OnMixerCtrlChange(WPARAM wParam, LPARAM lParam)
{
	if ((HMIXER)wParam == m_pDoc->m_CaptureAudioThread.m_Mixer.GetHandle() &&
		((DWORD)lParam == m_pDoc->m_CaptureAudioThread.m_Mixer.GetDstVolumeControlID() ||
		(DWORD)lParam == m_pDoc->m_CaptureAudioThread.m_Mixer.GetDstMuteControlID() ||
		(DWORD)lParam == m_pDoc->m_CaptureAudioThread.m_Mixer.GetSrcVolumeControlID() ||
		(DWORD)lParam == m_pDoc->m_CaptureAudioThread.m_Mixer.GetSrcMuteControlID() ||
		(DWORD)lParam == m_pDoc->m_CaptureAudioThread.m_Mixer.GetMuxControlID()))
	{
		// Adjust Volume Slider
		DWORD dwVolLeft, dwVolRight;
		if (m_pDoc->m_CaptureAudioThread.m_Mixer.GetDstVolumeControlID() != 0xFFFFFFFF)
		{
			m_RecVolumeLeft.SetRange(	m_pDoc->m_CaptureAudioThread.m_Mixer.GetDstVolumeControlMin(),
										m_pDoc->m_CaptureAudioThread.m_Mixer.GetDstVolumeControlMax(), TRUE);
			m_RecVolumeRight.SetRange(	m_pDoc->m_CaptureAudioThread.m_Mixer.GetDstVolumeControlMin(),
										m_pDoc->m_CaptureAudioThread.m_Mixer.GetDstVolumeControlMax(), TRUE);
			if (m_pDoc->m_CaptureAudioThread.m_Mixer.GetDstVolume(dwVolLeft, dwVolRight))
			{
				m_RecVolumeLeft.SetPos(dwVolLeft);	
				m_RecVolumeRight.SetPos(dwVolRight);
			}
		}
		else if (m_pDoc->m_CaptureAudioThread.m_Mixer.GetSrcVolumeControlID() != 0xFFFFFFFF)
		{
			m_RecVolumeLeft.SetRange(	m_pDoc->m_CaptureAudioThread.m_Mixer.GetSrcVolumeControlMin(),
										m_pDoc->m_CaptureAudioThread.m_Mixer.GetSrcVolumeControlMax(), TRUE);
			m_RecVolumeRight.SetRange(	m_pDoc->m_CaptureAudioThread.m_Mixer.GetSrcVolumeControlMin(),
										m_pDoc->m_CaptureAudioThread.m_Mixer.GetSrcVolumeControlMax(), TRUE);
			if (m_pDoc->m_CaptureAudioThread.m_Mixer.GetSrcVolume(dwVolLeft, dwVolRight))
			{
				m_RecVolumeLeft.SetPos(dwVolLeft);	
				m_RecVolumeRight.SetPos(dwVolRight);
			}
		}

		// Set Muted Warning Label
		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_VOLUME_MUTED);
		if (m_pDoc->m_CaptureAudioThread.m_Mixer.GetDstMute() ||
			m_pDoc->m_CaptureAudioThread.m_Mixer.GetSrcMute())
			pEdit->SetWindowText(ML_STRING(1455, "Muted!"));
		else
			pEdit->SetWindowText(_T(""));
	}

	return 0;
}

void CGeneralPage::OnSelchangeVideoCompressionChoose() 
{
	UpdateData(TRUE);
	m_pDoc->m_dwVideoRecFourCC = m_VideoCompressionFcc[m_VideoCompressionChoose.GetCurSel()];
	ShowHideCtrls();
}

void CGeneralPage::OnSelchangeVideoPostrecCompressionChoose() 
{
	UpdateData(TRUE);
	m_pDoc->m_dwVideoPostRecFourCC = m_VideoCompressionFcc[m_VideoPostRecCompressionChoose.GetCurSel()];
	ShowHideCtrls();
}

void CGeneralPage::OnRecordSaveas() 
{
	CBrowseDlg dlg(	::AfxGetMainFrame(),
					&m_pDoc->m_sRecordAutoSaveDir,
					ML_STRING(1453, "Select Folder For Record Saving"),
					TRUE);
	dlg.DoModal();
	m_DirLabel.SetLink(m_pDoc->m_sRecordAutoSaveDir);
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_RECORD_SAVEAS_PATH);
	pEdit->SetWindowText(m_pDoc->m_sRecordAutoSaveDir);
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
		params.Format(_T("/r /d%u"), m_pDoc->m_dwCaptureAudioDeviceID);
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
			OnCheckSchedulerOnce();
		}
	}
	else
	{
		if (bCheck)
		{
			pCheckOnce->SetCheck(1);
			OnCheckSchedulerOnce();
		}
	}
}

void CGeneralPage::OnCheckSchedulerOnce() 
{
	CButton* pCheckOnce = (CButton*)GetDlgItem(IDC_CHECK_SCHEDULER_ONCE);
	CButton* pCheckDaily = (CButton*)GetDlgItem(IDC_CHECK_SCHEDULER_DAILY);
	if (pCheckOnce->GetCheck())
	{
		// Clear daily
		if (pCheckDaily->GetCheck())
		{
			pCheckDaily->SetCheck(0);
			OnCheckSchedulerDaily();
		}

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

		// Disable
		CWnd* pWnd = (CWnd*)GetDlgItem(IDC_TIME_ONCE_START);
		pWnd->EnableWindow(FALSE);
		pWnd = (CWnd*)GetDlgItem(IDC_TIME_ONCE_STOP);
		pWnd->EnableWindow(FALSE);
		pWnd = (CWnd*)GetDlgItem(IDC_DATE_ONCE_START);
		pWnd->EnableWindow(FALSE);
		pWnd = (CWnd*)GetDlgItem(IDC_DATE_ONCE_STOP);
		pWnd->EnableWindow(FALSE);
	}
	else
	{
		// Delete Once Scheduler Entry
		((CUImagerApp*)::AfxGetApp())->DeleteOnceSchedulerEntry(m_pDoc->GetDevicePathName());

		// Enable
		CWnd* pWnd = (CWnd*)GetDlgItem(IDC_TIME_ONCE_START);
		pWnd->EnableWindow(TRUE);
		pWnd = (CWnd*)GetDlgItem(IDC_TIME_ONCE_STOP);
		pWnd->EnableWindow(TRUE);
		pWnd = (CWnd*)GetDlgItem(IDC_DATE_ONCE_START);
		pWnd->EnableWindow(TRUE);
		pWnd = (CWnd*)GetDlgItem(IDC_DATE_ONCE_STOP);
		pWnd->EnableWindow(TRUE);
	}
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

	// Enable
	CWnd* pWnd = (CWnd*)GetDlgItem(IDC_TIME_ONCE_START);
	pWnd->EnableWindow(TRUE);
	pWnd = (CWnd*)GetDlgItem(IDC_TIME_ONCE_STOP);
	pWnd->EnableWindow(TRUE);
	pWnd = (CWnd*)GetDlgItem(IDC_DATE_ONCE_START);
	pWnd->EnableWindow(TRUE);
	pWnd = (CWnd*)GetDlgItem(IDC_DATE_ONCE_STOP);
	pWnd->EnableWindow(TRUE);
}

void CGeneralPage::SetCheckSchedulerDaily(BOOL bCheck) 
{
	CButton* pCheckDaily = (CButton*)GetDlgItem(IDC_CHECK_SCHEDULER_DAILY);
	if (pCheckDaily->GetCheck())
	{
		if (!bCheck)
		{
			pCheckDaily->SetCheck(0);
			OnCheckSchedulerDaily();
		}
	}
	else
	{
		if (bCheck)
		{
			pCheckDaily->SetCheck(1);
			OnCheckSchedulerDaily();
		}
	}
}

void CGeneralPage::OnCheckSchedulerDaily() 
{
	CButton* pCheckOnce = (CButton*)GetDlgItem(IDC_CHECK_SCHEDULER_ONCE);
	CButton* pCheckDaily = (CButton*)GetDlgItem(IDC_CHECK_SCHEDULER_DAILY);	
	if (pCheckDaily->GetCheck())
	{
		// Clear once
		if (pCheckOnce->GetCheck())
		{
			pCheckOnce->SetCheck(0);
			OnCheckSchedulerOnce();
		}

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

		// Disable
		CWnd* pWnd = (CWnd*)GetDlgItem(IDC_TIME_DAILY_START);
		pWnd->EnableWindow(FALSE);
		pWnd = (CWnd*)GetDlgItem(IDC_TIME_DAILY_STOP);
		pWnd->EnableWindow(FALSE);
	}
	else
	{
		// Delete Daily Scheduler Entry
		((CUImagerApp*)::AfxGetApp())->DeleteDailySchedulerEntry(m_pDoc->GetDevicePathName());

		// Enable
		CWnd* pWnd = (CWnd*)GetDlgItem(IDC_TIME_DAILY_START);
		pWnd->EnableWindow(TRUE);
		pWnd = (CWnd*)GetDlgItem(IDC_TIME_DAILY_STOP);
		pWnd->EnableWindow(TRUE);
	}
}

#endif
