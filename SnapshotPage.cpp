// SnapshotPage.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "SnapshotPage.h"
#include "VideoDeviceDoc.h"
#include "VideoDeviceView.h"
#include "NetFrameHdr.h"
#include "NoVistaFileDlg.h"
#include "FTPUploadConfigurationDlg.h"
#include "BrowseDlg.h"
#include "ResizingDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef VIDEODEVICEDOC

/////////////////////////////////////////////////////////////////////////////
// CSnapshotPage property page

IMPLEMENT_DYNCREATE(CSnapshotPage, CPropertyPage)

CSnapshotPage::CSnapshotPage()
	: CPropertyPage(CSnapshotPage::IDD)
{
	// OnInitDialog() is called when first pressing the tab
	// OnInitDialog() inits the property page pointer in the doc
	// -> Move all inits to OnInitDialog() because the assistant
	// may change the doc vars between construction and OnInitDialog() call
	//{{AFX_DATA_INIT(CSnapshotPage)
	//}}AFX_DATA_INIT
	m_pDoc = NULL;
}

void CSnapshotPage::SetDoc(CVideoDeviceDoc* pDoc)
{
	ASSERT(pDoc);
	m_pDoc = pDoc;
}

CSnapshotPage::~CSnapshotPage()
{
}

void CSnapshotPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSnapshotPage)
	DDX_Control(pDX, IDC_VIDEO_COMPRESSION_QUALITY, m_VideoCompressorQuality);
	DDX_Control(pDX, IDC_COMPRESSION_QUALITY, m_CompressionQuality);
	DDX_DateTimeCtrl(pDX, IDC_TIME_DAILY_START, m_SnapshotStartTime);
	DDX_DateTimeCtrl(pDX, IDC_TIME_DAILY_STOP, m_SnapshotStopTime);
	DDX_Text(pDX, IDC_EDIT_DELETE_SNAPSHOTS_DAYS, m_nDeleteSnapshotsOlderThanDays);
	DDV_MinMaxInt(pDX, m_nDeleteSnapshotsOlderThanDays, 0, 4000000);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSnapshotPage, CPropertyPage)
	//{{AFX_MSG_MAP(CSnapshotPage)
	ON_WM_DESTROY()
	ON_EN_CHANGE(IDC_EDIT_SNAPSHOT_RATE, OnChangeEditSnapshotRate)
	ON_BN_CLICKED(IDC_BUTTON_SET_DIR, OnButtonSetDir)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BUTTON_THUMB_SIZE, OnButtonThumbSize)
	ON_BN_CLICKED(IDC_CHECK_SNAPSHOT_THUMB, OnCheckSnapshotThumb)
	ON_EN_CHANGE(IDC_EDIT_SNAPSHOT_HISTORY_FRAMERATE, OnChangeEditSnapshotHistoryFramerate)
	ON_BN_CLICKED(IDC_CHECK_SNAPSHOT_HISTORY_DEINTERLACE, OnCheckSnapshotHistoryDeinterlace)
	ON_BN_CLICKED(IDC_CHECK_SNAPSHOT_LIVE_JPEG, OnCheckSnapshotLiveJpeg)
	ON_BN_CLICKED(IDC_CHECK_SNAPSHOT_HISTORY_JPEG, OnCheckSnapshotHistoryJpeg)
	ON_BN_CLICKED(IDC_CHECK_SNAPSHOT_HISTORY_SWF, OnCheckSnapshotHistorySwf)
	ON_BN_CLICKED(IDC_CHECK_SCHEDULER_DAILY, OnCheckSchedulerDaily)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_TIME_DAILY_START, OnDatetimechangeTimeDailyStart)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_TIME_DAILY_STOP, OnDatetimechangeTimeDailyStop)
	ON_EN_CHANGE(IDC_EDIT_DELETE_SNAPSHOTS_DAYS, OnChangeEditDeleteSnapshotsDays)
	ON_BN_CLICKED(IDC_FTP_CONFIGURE, OnFtpConfigure)
	ON_BN_CLICKED(IDC_CHECK_FTP_SNAPSHOT, OnCheckFtpSnapshot)
	ON_BN_CLICKED(IDC_CHECK_FTP_SNAPSHOT_HISTORY_JPEG, OnCheckFtpSnapshotHistoryJpeg)
	ON_BN_CLICKED(IDC_CHECK_FTP_SNAPSHOT_HISTORY_SWF, OnCheckFtpSnapshotHistorySwf)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSnapshotPage message handlers

BOOL CSnapshotPage::OnInitDialog() 
{
	// Init vars
	m_SnapshotStartTime = m_pDoc->m_SnapshotStartTime;
	m_SnapshotStopTime = m_pDoc->m_SnapshotStopTime;
	m_nDeleteSnapshotsOlderThanDays = m_pDoc->m_nDeleteSnapshotsOlderThanDays;

	// This calls UpdateData(FALSE)
	CPropertyPage::OnInitDialog();

	// Thumb Check Box
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_SNAPSHOT_THUMB);
	pCheck->SetCheck(m_pDoc->m_bSnapshotThumb);

	// Live Snapshot Jpeg Check Box
	pCheck = (CButton*)GetDlgItem(IDC_CHECK_SNAPSHOT_LIVE_JPEG);
	pCheck->SetCheck(m_pDoc->m_bSnapshotLiveJpeg);

	// Snapshot History Jpeg Check Box
	pCheck = (CButton*)GetDlgItem(IDC_CHECK_SNAPSHOT_HISTORY_JPEG);
	pCheck->SetCheck(m_pDoc->m_bSnapshotHistoryJpeg);

	// Snapshot History Swf Check Box
	pCheck = (CButton*)GetDlgItem(IDC_CHECK_SNAPSHOT_HISTORY_SWF);
	pCheck->SetCheck(m_pDoc->m_bSnapshotHistorySwf);

	// Live Snapshot Jpeg Ftp Check Box
	pCheck = (CButton*)GetDlgItem(IDC_CHECK_FTP_SNAPSHOT);
	pCheck->SetCheck(m_pDoc->m_bSnapshotLiveJpegFtp);

	// Snapshot History Jpeg Ftp Check Box
	pCheck = (CButton*)GetDlgItem(IDC_CHECK_FTP_SNAPSHOT_HISTORY_JPEG);
	pCheck->SetCheck(m_pDoc->m_bSnapshotHistoryJpegFtp);

	// Snapshot History Swf Ftp Check Box
	pCheck = (CButton*)GetDlgItem(IDC_CHECK_FTP_SNAPSHOT_HISTORY_SWF);
	pCheck->SetCheck(m_pDoc->m_bSnapshotHistorySwfFtp);

	// Snapshot History Deinterlace Check Box
	pCheck = (CButton*)GetDlgItem(IDC_CHECK_SNAPSHOT_HISTORY_DEINTERLACE);
	pCheck->SetCheck(m_pDoc->m_bSnapshotHistoryDeinterlace);

	// Snapshot Scheduler Check Box
	pCheck = (CButton*)GetDlgItem(IDC_CHECK_SCHEDULER_DAILY);
	pCheck->SetCheck(m_pDoc->m_bSnapshotStartStop);

	// Snapshot rate
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_SNAPSHOT_RATE);
	CString sText;
	sText.Format(_T("%i"), m_pDoc->m_nSnapshotRate);
	pEdit->SetWindowText(sText);

	// Snapshot history framerate
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_SNAPSHOT_HISTORY_FRAMERATE);
	sText.Format(_T("%i"), m_pDoc->m_nSnapshotHistoryFrameRate);
	pEdit->SetWindowText(sText);

	// Snapshot Dir
	m_DirLabel.SubclassDlgItem(IDC_TEXT_DIR, this);
	m_DirLabel.SetVisitedColor(RGB(0, 0, 255));
	m_DirLabel.SetLink(m_pDoc->m_sSnapshotAutoSaveDir);
	pEdit = (CEdit*)GetDlgItem(IDC_SNAPSHOT_SAVEAS_PATH);
	pEdit->SetWindowText(m_pDoc->m_sSnapshotAutoSaveDir);

	// Compression Quality
	m_CompressionQuality.SetRange(0, 100);
	m_CompressionQuality.SetPageSize(20);
	m_CompressionQuality.SetLineSize(5);
	m_CompressionQuality.SetPos(m_pDoc->m_nSnapshotCompressionQuality);
	pEdit = (CEdit*)GetDlgItem(IDC_COMPRESSION_QUALITY_NUM);
	CString sQuality;
	sQuality.Format(_T("%i"), m_CompressionQuality.GetPos());
	pEdit->SetWindowText(sQuality);

	// Video Compression Quality
	m_VideoCompressorQuality.SetRange(2, 31);
	m_VideoCompressorQuality.SetPageSize(5);
	m_VideoCompressorQuality.SetLineSize(1);
	m_VideoCompressorQuality.SetPos(33 - (int)(m_pDoc->m_fSnapshotVideoCompressorQuality)); // m_fSnapshotVideoCompressorQuality has a range from 31.0f to 2.0f
	pEdit = (CEdit*)GetDlgItem(IDC_VIDEO_COMPRESSION_QUALITY_NUM);
	sQuality.Format(_T("%i"), (int)((m_VideoCompressorQuality.GetPos() - 2) * 3.45)); // 0 .. 100
	pEdit->SetWindowText(sQuality);

	// Thumbnail Size Button
	CString sSize;
	sSize.Format(_T("Thumbnail Size %i x %i"),	m_pDoc->m_nSnapshotThumbWidth,
												m_pDoc->m_nSnapshotThumbHeight);
	CButton* pButton = (CButton*)GetDlgItem(IDC_BUTTON_THUMB_SIZE);
	pButton->SetWindowText(sSize);

	// Disable Critical Controls?
	if (m_pDoc->GetView()->AreCriticalControlsDisabled())
		EnableDisableCriticalControls(FALSE);

	// Set Page Pointer to this
	m_pDoc->m_pSnapshotPage = this;
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSnapshotPage::OnDestroy() 
{
	CPropertyPage::OnDestroy();
	// Set Page Pointer to NULL
	m_pDoc->m_pSnapshotPage = NULL;
}

void CSnapshotPage::EnableDisableCriticalControls(BOOL bEnable)
{
	
}

void CSnapshotPage::OnCheckSnapshotThumb() 
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_SNAPSHOT_THUMB);
	if (pCheck->GetCheck())
		m_pDoc->m_bSnapshotThumb = TRUE;
	else
		m_pDoc->m_bSnapshotThumb = FALSE;
}

void CSnapshotPage::OnCheckSnapshotHistoryDeinterlace() 
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_SNAPSHOT_HISTORY_DEINTERLACE);
	if (pCheck->GetCheck())
		m_pDoc->m_bSnapshotHistoryDeinterlace = TRUE;
	else
		m_pDoc->m_bSnapshotHistoryDeinterlace = FALSE;
}

void CSnapshotPage::OnCheckSnapshotLiveJpeg() 
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_SNAPSHOT_LIVE_JPEG);
	if (pCheck->GetCheck())
		m_pDoc->m_bSnapshotLiveJpeg = TRUE;
	else
		m_pDoc->m_bSnapshotLiveJpeg = FALSE;
}

void CSnapshotPage::OnCheckSnapshotHistoryJpeg() 
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_SNAPSHOT_HISTORY_JPEG);
	if (pCheck->GetCheck())
		m_pDoc->m_bSnapshotHistoryJpeg = TRUE;
	else
		m_pDoc->m_bSnapshotHistoryJpeg = FALSE;
}

void CSnapshotPage::OnCheckSnapshotHistorySwf() 
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_SNAPSHOT_HISTORY_SWF);
	if (pCheck->GetCheck())
		m_pDoc->m_bSnapshotHistorySwf = TRUE;
	else
	{
		m_pDoc->m_bSnapshotHistorySwf = FALSE;
		::InterlockedExchange(&m_pDoc->m_bSnapshotHistoryCloseSwfFile, 1);
	}
}

void CSnapshotPage::OnCheckFtpSnapshot() 
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_FTP_SNAPSHOT);
	if (pCheck->GetCheck())
		m_pDoc->m_bSnapshotLiveJpegFtp = TRUE;
	else
		m_pDoc->m_bSnapshotLiveJpegFtp = FALSE;
}

void CSnapshotPage::OnCheckFtpSnapshotHistoryJpeg() 
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_FTP_SNAPSHOT_HISTORY_JPEG);
	if (pCheck->GetCheck())
		m_pDoc->m_bSnapshotHistoryJpegFtp = TRUE;
	else
		m_pDoc->m_bSnapshotHistoryJpegFtp = FALSE;
}

void CSnapshotPage::OnCheckFtpSnapshotHistorySwf() 
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_FTP_SNAPSHOT_HISTORY_SWF);
	if (pCheck->GetCheck())
		m_pDoc->m_bSnapshotHistorySwfFtp = TRUE;
	else
		m_pDoc->m_bSnapshotHistorySwfFtp = FALSE;
}

void CSnapshotPage::UpdateSnapshotStartStopTimes()
{
	if (m_pDoc->m_bSnapshotStartStop)
	{
		// Start Time
		m_pDoc->m_SnapshotStartTime = CTime(	2000,
												1,
												1,
												m_SnapshotStartTime.GetHour(),
												m_SnapshotStartTime.GetMinute(),
												m_SnapshotStartTime.GetSecond());

		// Stop Time
		m_pDoc->m_SnapshotStopTime = CTime(	2000,
												1,
												1,
												m_SnapshotStopTime.GetHour(),
												m_SnapshotStopTime.GetMinute(),
												m_SnapshotStopTime.GetSecond());
	}
}

void CSnapshotPage::OnCheckSchedulerDaily() 
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_SCHEDULER_DAILY);
	if (pCheck->GetCheck())
		m_pDoc->m_bSnapshotStartStop = TRUE;
	else
		m_pDoc->m_bSnapshotStartStop = FALSE;
	UpdateSnapshotStartStopTimes();
}

void CSnapshotPage::OnDatetimechangeTimeDailyStart(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UpdateData(TRUE);
	UpdateSnapshotStartStopTimes();
	*pResult = 0;
}

void CSnapshotPage::OnDatetimechangeTimeDailyStop(NMHDR* pNMHDR, LRESULT* pResult) 
{	
	UpdateData(TRUE);
	UpdateSnapshotStartStopTimes();
	*pResult = 0;
}

void CSnapshotPage::OnChangeEditSnapshotRate() 
{
	CString sText;
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_SNAPSHOT_RATE);
	pEdit->GetWindowText(sText);
	int nRate = _tcstol(sText.GetBuffer(0), NULL, 10);
	sText.ReleaseBuffer();
	if (nRate > 0)
	{
		if (nRate != m_pDoc->m_nSnapshotRate)
		{
			sText.Format(_T("%d"), nRate);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_SNAPSHOTREFRESHSEC, sText);
			m_pDoc->m_nSnapshotRate = nRate;
		}
	}
	else
	{
		if (DEFAULT_SNAPSHOT_RATE != m_pDoc->m_nSnapshotRate)
		{
			sText.Format(_T("%d"), DEFAULT_SNAPSHOT_RATE);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_SNAPSHOTREFRESHSEC, sText);
			m_pDoc->m_nSnapshotRate = DEFAULT_SNAPSHOT_RATE;
		}
	}
}

void CSnapshotPage::OnChangeEditSnapshotHistoryFramerate() 
{
	CString sText;
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_SNAPSHOT_HISTORY_FRAMERATE);
	pEdit->GetWindowText(sText);
	int nRate = _tcstol(sText.GetBuffer(0), NULL, 10);
	sText.ReleaseBuffer();
	int nPreviousRate = m_pDoc->m_nSnapshotHistoryFrameRate;
	if (nRate >= MIN_SNAPSHOT_HISTORY_FRAMERATE && nRate <= MAX_SNAPSHOT_HISTORY_FRAMERATE)
		m_pDoc->m_nSnapshotHistoryFrameRate = nRate;
	else
		m_pDoc->m_nSnapshotHistoryFrameRate = DEFAULT_SNAPSHOT_HISTORY_FRAMERATE;
	if (nPreviousRate != m_pDoc->m_nSnapshotHistoryFrameRate)
		::InterlockedExchange(&m_pDoc->m_bSnapshotHistoryCloseSwfFile, 1);
}

void CSnapshotPage::OnChangeEditDeleteSnapshotsDays() 
{
	if (UpdateData(TRUE))
	{
		m_pDoc->m_DeleteThread.Kill();
		m_pDoc->m_nDeleteSnapshotsOlderThanDays = m_nDeleteSnapshotsOlderThanDays;
		m_pDoc->m_DeleteThread.Start(THREAD_PRIORITY_LOWEST);
	}
}

void CSnapshotPage::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
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
			if (pSlider->GetDlgCtrlID() == IDC_COMPRESSION_QUALITY)
			{
				m_pDoc->m_nSnapshotCompressionQuality = m_CompressionQuality.GetPos();
				CEdit* pEdit = (CEdit*)GetDlgItem(IDC_COMPRESSION_QUALITY_NUM);
				CString sQuality;
				sQuality.Format(_T("%i"), m_CompressionQuality.GetPos());
				pEdit->SetWindowText(sQuality);
			}
			else if (pSlider->GetDlgCtrlID() == IDC_VIDEO_COMPRESSION_QUALITY)
			{
				float fPreviousVideoCompressorQuality = m_pDoc->m_fSnapshotVideoCompressorQuality;
				m_pDoc->m_fSnapshotVideoCompressorQuality = (float)(33 - m_VideoCompressorQuality.GetPos());
				CEdit* pEdit = (CEdit*)GetDlgItem(IDC_VIDEO_COMPRESSION_QUALITY_NUM);
				CString sQuality;
				sQuality.Format(_T("%i"), (int)((m_VideoCompressorQuality.GetPos() - 2) * 3.45)); // 0 .. 100
				pEdit->SetWindowText(sQuality);
				if (fPreviousVideoCompressorQuality != m_pDoc->m_fSnapshotVideoCompressorQuality)
					::InterlockedExchange(&m_pDoc->m_bSnapshotHistoryCloseSwfFile, 1);
			}
		}
	}
	CPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CSnapshotPage::OnButtonSetDir() 
{
	CBrowseDlg dlg(	::AfxGetMainFrame(),
					&m_pDoc->m_sSnapshotAutoSaveDir,
					ML_STRING(1419, "Select Folder For Snapshot Saving"),
					TRUE);
	if (dlg.DoModal() == IDOK)
	{
		m_DirLabel.SetLink(m_pDoc->m_sSnapshotAutoSaveDir);
		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_SNAPSHOT_SAVEAS_PATH);
		pEdit->SetWindowText(m_pDoc->m_sSnapshotAutoSaveDir);
	}
}

void CSnapshotPage::OnButtonThumbSize() 
{
	CResizingDlg dlg(	m_pDoc->m_DocRect.Width(), m_pDoc->m_DocRect.Height(),
						m_pDoc->m_nSnapshotThumbWidth, m_pDoc->m_nSnapshotThumbHeight, this);
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

void CSnapshotPage::ChangeThumbSize(int nNewWidth, int nNewHeight)
{
	int nPreviousThumbWidth = m_pDoc->m_nSnapshotThumbWidth;
	int nPreviousThumbHeight = m_pDoc->m_nSnapshotThumbHeight;
	m_pDoc->m_nSnapshotThumbWidth = nNewWidth & ~0x3;
	m_pDoc->m_nSnapshotThumbHeight = nNewHeight & ~0x3;
	CString sSize;
	sSize.Format(_T("Thumbnail Size %i x %i"),	m_pDoc->m_nSnapshotThumbWidth,
												m_pDoc->m_nSnapshotThumbHeight);
	CButton* pButton = (CButton*)GetDlgItem(IDC_BUTTON_THUMB_SIZE);
	pButton->SetWindowText(sSize);
	if ((nPreviousThumbWidth != m_pDoc->m_nSnapshotThumbWidth) ||
		(nPreviousThumbHeight != m_pDoc->m_nSnapshotThumbHeight))
		::InterlockedExchange(&m_pDoc->m_bSnapshotHistoryCloseSwfFile, 1);
}

void CSnapshotPage::OnFtpConfigure()
{
	// FTP Config Dialog
	CFTPUploadConfigurationDlg dlg(	&m_pDoc->m_SnapshotFTPUploadConfiguration,
									IDD_SNAPSHOT_FTP_CONFIGURATION);
	dlg.DoModal();
}

#endif