// SnapshotPage.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "SnapshotPage.h"
#include "VideoDeviceDoc.h"
#include "VideoDeviceView.h"
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
END_MESSAGE_MAP()

BOOL CSnapshotPage::OnInitDialog() 
{
	// This calls UpdateData(FALSE)
	CPropertyPage::OnInitDialog();

	// Snapshot History Video Check Box
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_SNAPSHOT_HISTORY_VIDEO);
	pCheck->SetCheck(m_pDoc->m_bSnapshotHistoryVideo);

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

#endif
