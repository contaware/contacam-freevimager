// HttpVideoFormatDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "HttpVideoFormatDlg.h"
#include "VideoDeviceDoc.h"
#include "VideoDeviceView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef VIDEODEVICEDOC

/////////////////////////////////////////////////////////////////////////////
// CHttpVideoFormatDlg dialog


CHttpVideoFormatDlg::CHttpVideoFormatDlg(CVideoDeviceDoc* pDoc)
	: CDialog(CHttpVideoFormatDlg::IDD, NULL)
{
	//{{AFX_DATA_INIT(CHttpVideoFormatDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	ASSERT_VALID(pDoc);
	m_pDoc = pDoc;
}


void CHttpVideoFormatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHttpVideoFormatDlg)
	DDX_Control(pDX, IDC_COMBO_VIDEOSIZE, m_VideoSize);
	DDX_Control(pDX, IDC_COMPRESSION_QUALITY_SLIDER, m_VideoQualitySlider);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CHttpVideoFormatDlg, CDialog)
	//{{AFX_MSG_MAP(CHttpVideoFormatDlg)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_APPLY, OnApply)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHttpVideoFormatDlg message handlers

BOOL CHttpVideoFormatDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Quality
	m_VideoQualitySlider.SetRange(0, 100);
	m_VideoQualitySlider.SetPageSize(5);
	m_VideoQualitySlider.SetLineSize(1);
	m_VideoQualitySlider.SetPos(100 - m_pDoc->m_nHttpVideoQuality);
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_COMPRESSION_QUALITY_EDIT);
	CString sQuality;
	sQuality.Format(_T("%i"), 100 - m_pDoc->m_nHttpVideoQuality);
	pEdit->SetWindowText(sQuality);

	// Sizes
	int nCurrentSelection = 0;
	for (int nSize = 0 ; nSize < m_pDoc->m_pHttpGetFrameParseProcess->m_Sizes.GetSize() ; nSize++)
	{
		CString sSize;
		sSize.Format(_T("%i x %i"), m_pDoc->m_pHttpGetFrameParseProcess->m_Sizes[nSize].cx,
									m_pDoc->m_pHttpGetFrameParseProcess->m_Sizes[nSize].cy);
		m_VideoSize.AddString(sSize);
		if (CSize(m_pDoc->m_nHttpVideoSizeX, m_pDoc->m_nHttpVideoSizeY) ==
			m_pDoc->m_pHttpGetFrameParseProcess->m_Sizes[nSize])
			nCurrentSelection = nSize;
	}
	m_VideoSize.SetCurSel(nCurrentSelection);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CHttpVideoFormatDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
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
			if (pSlider->GetDlgCtrlID() == IDC_COMPRESSION_QUALITY_SLIDER)
			{
				CEdit* pEdit = (CEdit*)GetDlgItem(IDC_COMPRESSION_QUALITY_EDIT);
				CString sQuality;
				sQuality.Format(_T("%i"), m_VideoQualitySlider.GetPos());
				pEdit->SetWindowText(sQuality);
			}
		}
	}
	
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CHttpVideoFormatDlg::OnOK() 
{
	::EnterCriticalSection(&m_pDoc->m_csHttpParams);
	m_pDoc->m_nHttpVideoQuality = 100 - m_VideoQualitySlider.GetPos();
	if (m_VideoSize.GetCurSel() >= 0 &&
		m_VideoSize.GetCurSel() < m_pDoc->m_pHttpGetFrameParseProcess->m_Sizes.GetSize())
	{
		m_pDoc->m_nHttpVideoSizeX = m_pDoc->m_pHttpGetFrameParseProcess->m_Sizes[m_VideoSize.GetCurSel()].cx;
		m_pDoc->m_nHttpVideoSizeY = m_pDoc->m_pHttpGetFrameParseProcess->m_Sizes[m_VideoSize.GetCurSel()].cy;
	}
	::LeaveCriticalSection(&m_pDoc->m_csHttpParams);
	m_pDoc->m_bSizeToDoc = TRUE;
	if (m_pDoc->m_pHttpGetFrameParseProcess->m_FormatType == CVideoDeviceDoc::CHttpGetFrameParseProcess::FORMATMJPEG)
		m_pDoc->ConnectGetFrameHTTP(m_pDoc->m_sGetFrameVideoHost, m_pDoc->m_nGetFrameVideoPort);
	CDialog::OnOK();
}

void CHttpVideoFormatDlg::OnApply() 
{
	::EnterCriticalSection(&m_pDoc->m_csHttpParams);
	m_pDoc->m_nHttpVideoQuality = 100 - m_VideoQualitySlider.GetPos();
	if (m_VideoSize.GetCurSel() >= 0 &&
		m_VideoSize.GetCurSel() < m_pDoc->m_pHttpGetFrameParseProcess->m_Sizes.GetSize())
	{
		m_pDoc->m_nHttpVideoSizeX = m_pDoc->m_pHttpGetFrameParseProcess->m_Sizes[m_VideoSize.GetCurSel()].cx;
		m_pDoc->m_nHttpVideoSizeY = m_pDoc->m_pHttpGetFrameParseProcess->m_Sizes[m_VideoSize.GetCurSel()].cy;
	}
	::LeaveCriticalSection(&m_pDoc->m_csHttpParams);
	m_pDoc->m_bSizeToDoc = TRUE;
	if (m_pDoc->m_pHttpGetFrameParseProcess->m_FormatType == CVideoDeviceDoc::CHttpGetFrameParseProcess::FORMATMJPEG)
		m_pDoc->ConnectGetFrameHTTP(m_pDoc->m_sGetFrameVideoHost, m_pDoc->m_nGetFrameVideoPort);
}

#endif
