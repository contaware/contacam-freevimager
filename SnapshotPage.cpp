// SnapshotPage.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "SnapshotPage.h"
#include "VideoDeviceDoc.h"
#include "VideoDeviceView.h"
#include "FTPUploadConfigurationDlg.h"
#include "BrowseDlg.h"
#include "ResizingDlg.h"
#include "SnapshotNamesDlg.h"

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

BEGIN_MESSAGE_MAP(CSnapshotPage, CPropertyPage)
	ON_WM_DESTROY()
	ON_EN_CHANGE(IDC_EDIT_SNAPSHOT_RATE, OnChangeEditSnapshotRate)
	ON_BN_CLICKED(IDC_BUTTON_THUMB_SIZE, OnButtonThumbSize)
	ON_BN_CLICKED(IDC_CHECK_SNAPSHOT_HISTORY_VIDEO, OnCheckSnapshotHistoryVideo)
	ON_BN_CLICKED(IDC_FTP_CONFIGURE, OnFtpConfigure)
	ON_BN_CLICKED(IDC_CHECK_FTP_SNAPSHOT, OnCheckFtpSnapshot)
	ON_BN_CLICKED(IDC_CHECK_FTP_SNAPSHOT_HISTORY_VIDEO, OnCheckFtpSnapshotHistoryVideo)
	ON_BN_CLICKED(IDC_BUTTON_SNAPSHOT_NAMES, OnButtonSnapshotNames)
END_MESSAGE_MAP()

BOOL CSnapshotPage::OnInitDialog() 
{
	// This calls UpdateData(FALSE)
	CPropertyPage::OnInitDialog();

	// Snapshot History Video Check Box
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_SNAPSHOT_HISTORY_VIDEO);
	pCheck->SetCheck(m_pDoc->m_bSnapshotHistoryVideo);

	// Live Snapshot Jpeg Ftp Check Box
	pCheck = (CButton*)GetDlgItem(IDC_CHECK_FTP_SNAPSHOT);
	pCheck->SetCheck(m_pDoc->m_bSnapshotLiveJpegFtp);

	// Snapshot History Video Ftp Check Box
	pCheck = (CButton*)GetDlgItem(IDC_CHECK_FTP_SNAPSHOT_HISTORY_VIDEO);
	pCheck->SetCheck(m_pDoc->m_bSnapshotHistoryVideoFtp);

	// Snapshot rate
	DisplaySnapshotRate();

	// Thumbnail Size Button
	CString sSize;
	sSize.Format(ML_STRING(1769, "Thumbnail Size %i x %i"),	m_pDoc->m_nSnapshotThumbWidth,
															m_pDoc->m_nSnapshotThumbHeight);
	CButton* pButton = (CButton*)GetDlgItem(IDC_BUTTON_THUMB_SIZE);
	pButton->SetWindowText(sSize);

	// Set Page Pointer to this
	m_pDoc->m_pSnapshotPage = this;
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSnapshotPage::OnDestroy() 
{
	// Base class
	CPropertyPage::OnDestroy();

	// Set Page Pointer to NULL
	m_pDoc->m_pSnapshotPage = NULL;
}

void CSnapshotPage::DisplaySnapshotRate()
{
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_SNAPSHOT_RATE);
	CString sText;
	if (m_pDoc->m_nSnapshotRate >= 1)
		sText.Format(_T("%i"), m_pDoc->m_nSnapshotRate);
	else if (m_pDoc->m_nSnapshotRate == 0 && m_pDoc->m_nSnapshotRateMs == 0)
		sText = _T("0");
	else
		sText.Format(_T("%.3f"), (double)(m_pDoc->m_nSnapshotRate) + (double)(m_pDoc->m_nSnapshotRateMs) / 1000.0);
	pEdit->SetWindowText(sText);
}

void CSnapshotPage::OnChangeEditSnapshotRate()
{
	CString sText;
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_SNAPSHOT_RATE);
	pEdit->GetWindowText(sText);
	double dRate = _tcstod(sText.GetBuffer(0), NULL);
	sText.ReleaseBuffer();
	m_pDoc->SnapshotRate(dRate);
}

void CSnapshotPage::OnButtonSnapshotNames()
{
	// Snapshot Names Dialog
	CSnapshotNamesDlg dlg;
	dlg.m_sSnapshotLiveJpegName = m_pDoc->m_sSnapshotLiveJpegName;
	dlg.m_sSnapshotLiveJpegThumbName = m_pDoc->m_sSnapshotLiveJpegThumbName;
	if (dlg.DoModal() == IDOK)
	{
		::EnterCriticalSection(&m_pDoc->m_csSnapshotConfiguration);
		if (!dlg.m_sSnapshotLiveJpegName.IsEmpty())
			m_pDoc->m_sSnapshotLiveJpegName = dlg.m_sSnapshotLiveJpegName;
		if (!dlg.m_sSnapshotLiveJpegThumbName.IsEmpty())
			m_pDoc->m_sSnapshotLiveJpegThumbName = dlg.m_sSnapshotLiveJpegThumbName;
		::LeaveCriticalSection(&m_pDoc->m_csSnapshotConfiguration);
		m_pDoc->PhpConfigFileSetParam(PHPCONFIG_SNAPSHOTNAME, m_pDoc->m_sSnapshotLiveJpegName + _T(".jpg"));
		m_pDoc->PhpConfigFileSetParam(PHPCONFIG_SNAPSHOTTHUMBNAME, m_pDoc->m_sSnapshotLiveJpegThumbName + _T(".jpg"));
	}
}

void CSnapshotPage::ChangeThumbSize(int nNewWidth, int nNewHeight)
{
	// Init thumb vars: must be a multiple of 4 for some video codecs,
	// most efficient would be a multiple of 16 to fit the macro blocks
	m_pDoc->m_nSnapshotThumbWidth = CVideoDeviceDoc::MakeSizeMultipleOf4(nNewWidth);
	m_pDoc->m_nSnapshotThumbHeight = CVideoDeviceDoc::MakeSizeMultipleOf4(nNewHeight);
	CString sSize;
	sSize.Format(ML_STRING(1769, "Thumbnail Size %i x %i"),
				m_pDoc->m_nSnapshotThumbWidth,
				m_pDoc->m_nSnapshotThumbHeight);
	CButton* pButton = (CButton*)GetDlgItem(IDC_BUTTON_THUMB_SIZE);
	pButton->SetWindowText(sSize);
}

void CSnapshotPage::OnButtonThumbSize()
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

void CSnapshotPage::OnCheckSnapshotHistoryVideo() 
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_SNAPSHOT_HISTORY_VIDEO);
	if (pCheck->GetCheck())
		m_pDoc->m_bSnapshotHistoryVideo = TRUE;
	else
		m_pDoc->m_bSnapshotHistoryVideo = FALSE;
}

void CSnapshotPage::OnCheckFtpSnapshot() 
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_FTP_SNAPSHOT);
	if (pCheck->GetCheck())
		m_pDoc->m_bSnapshotLiveJpegFtp = TRUE;
	else
		m_pDoc->m_bSnapshotLiveJpegFtp = FALSE;
}

void CSnapshotPage::OnCheckFtpSnapshotHistoryVideo() 
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_FTP_SNAPSHOT_HISTORY_VIDEO);
	if (pCheck->GetCheck())
		m_pDoc->m_bSnapshotHistoryVideoFtp = TRUE;
	else
		m_pDoc->m_bSnapshotHistoryVideoFtp = FALSE;
}

void CSnapshotPage::OnFtpConfigure()
{
	// FTP Config Dialog
	CFTPUploadConfigurationDlg dlg;
	dlg.m_FTPUploadConfiguration = m_pDoc->m_SnapshotFTPUploadConfiguration;
	if (dlg.DoModal() == IDOK)
	{
		::EnterCriticalSection(&m_pDoc->m_csSnapshotConfiguration);
		m_pDoc->m_SnapshotFTPUploadConfiguration = dlg.m_FTPUploadConfiguration;
		::LeaveCriticalSection(&m_pDoc->m_csSnapshotConfiguration);
	}
}

#endif
