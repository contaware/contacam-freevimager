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
	DDX_Check(pDX, IDC_CHECK_LIVE_ROTATE180, m_bRotate180);
	DDX_Check(pDX, IDC_CHECK_AUDIO_LISTEN, m_bAudioListen);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGeneralPage, CPropertyPage)
	//{{AFX_MSG_MAP(CGeneralPage)
	ON_BN_CLICKED(IDC_VIDEO_FORMAT, OnVideoFormat)
	ON_BN_CLICKED(IDC_VIDEO_SOURCE, OnVideoSource)
	ON_EN_CHANGE(IDC_FRAMERATE, OnChangeFrameRate)
	ON_BN_CLICKED(IDC_REC_AUDIO, OnRecAudio)
	ON_BN_CLICKED(IDC_REC_AUDIO_FROM_STREAM, OnRecAudioFromStream)
	ON_BN_CLICKED(IDC_REC_AUDIO_FROM_SOURCE, OnRecAudioFromSource)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_AUDIO_INPUT, OnAudioInput)
	ON_BN_CLICKED(IDC_AUDIO_MIXER, OnAudioMixer)
	ON_BN_CLICKED(IDC_VIDEO_INPUT, OnVideoInput)
	ON_BN_CLICKED(IDC_VIDEO_TUNER, OnVideoTuner)
	ON_BN_CLICKED(IDC_CHECK_LIVE_ROTATE180, OnCheckLiveRotate180)
	ON_BN_CLICKED(IDC_CHECK_AUDIO_LISTEN, OnCheckAudioListen)
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
	switch (::GetRevertedPos(&m_VideoRecQuality))
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

	// Init Live Rotate 180° Var
	m_bRotate180 = m_pDoc->m_bRotate180;

	// Init Audio Listen Var
	m_bAudioListen = m_pDoc->m_bAudioListen;

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

	// Capture Audio from Stream Check Box
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

	// Video Compressor Quality
	m_VideoRecQuality.SetRange((int)VIDEO_QUALITY_BEST, (int)VIDEO_QUALITY_LOW);
	m_VideoRecQuality.SetPageSize(1);
	m_VideoRecQuality.SetLineSize(1);
	m_pDoc->m_fVideoRecQuality = CAVRec::ClipVideoQuality(m_pDoc->m_fVideoRecQuality);
	::SetRevertedPos(&m_VideoRecQuality, (int)m_pDoc->m_fVideoRecQuality);
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

	// OnInitDialog() has been called
	m_bDlgInitialized = TRUE;

	// Set Page Pointer to this
	m_pDoc->m_pGeneralPage = this;

	// Set Timer
	SetTimer(ID_TIMER_GENERALPAGE, GENERALPAGE_TIMER_MS, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CGeneralPage::OnDestroy() 
{
	// Kill timer
	KillTimer(ID_TIMER_GENERALPAGE);

	// Base class
	CPropertyPage::OnDestroy();

	// Set Page Pointer to NULL
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

void CGeneralPage::OnRecAudioFromStream()
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

void CGeneralPage::OnRecAudioFromSource()
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

		// Show Process Frame Time
		pEdit = (CEdit*)GetDlgItem(IDC_PROCESS_TIME);
		if (pEdit)
		{
			CString sProcessFrameTime;
			sProcessFrameTime.Format(_T("%d"), m_pDoc->m_lProcessFrameTime);
			pEdit->SetWindowText(sProcessFrameTime);
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
				m_pDoc->m_fVideoRecQuality = (float)::GetRevertedPos(&m_VideoRecQuality);
				UpdateVideoQualityInfo();
			}
		}
	}
	
	CPropertyPage::OnHScroll(nSBCode, nPos, (CScrollBar*)pScrollBar);
}

void CGeneralPage::OnCheckLiveRotate180() 
{
	UpdateData(TRUE);
	m_pDoc->m_bRotate180 = m_bRotate180;
}

void CGeneralPage::OnCheckAudioListen()
{
	UpdateData(TRUE);
	m_pDoc->m_bAudioListen = m_bAudioListen;
}

void CGeneralPage::OnAudioInput() 
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
void CGeneralPage::OnAudioMixer() 
{
	::ShellExecute(	NULL, NULL,
					_T("control.exe"), _T("mmsys.cpl,,1"), NULL, SW_SHOWNORMAL);
}

#endif