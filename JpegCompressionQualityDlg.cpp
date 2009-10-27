// JpegCompressionQualityDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "JpegCompressionQualityDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CJpegCompressionQualityDlg dialog

CJpegCompressionQualityDlg::CJpegCompressionQualityDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CJpegCompressionQualityDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CJpegCompressionQualityDlg)
	m_bSaveAsGrayscale = FALSE;
	//}}AFX_DATA_INIT
}


void CJpegCompressionQualityDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CJpegCompressionQualityDlg)
	DDX_Control(pDX, IDC_COMPRESSION_QUALITY_SLIDER, m_CompressionQualitySlider);
	DDX_Check(pDX, IDC_SAVE_GRAYSCALE, m_bSaveAsGrayscale);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CJpegCompressionQualityDlg, CDialog)
	//{{AFX_MSG_MAP(CJpegCompressionQualityDlg)
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CJpegCompressionQualityDlg message handlers

void CJpegCompressionQualityDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if (SB_THUMBPOSITION == nSBCode)	// Wheel On Mouse And End Of Dragging Slider
	{
		if (m_CompressionQualitySlider.GetPos() > m_nLastPos)
		{
			m_nLastPos += 5;
			if (m_nLastPos > 100)
				m_nLastPos = 100;
			m_CompressionQualitySlider.SetPos(m_nLastPos);
		}
		else if (m_CompressionQualitySlider.GetPos() < m_nLastPos)
		{
			m_nLastPos -= 5;
			if (m_nLastPos < 0)
				m_nLastPos = 0;
			m_CompressionQualitySlider.SetPos(m_nLastPos);
		}
		else
		{
			m_CompressionQualitySlider.SetPos(m_nLastPos);
			return;
		}
	}
	else if (SB_THUMBTRACK == nSBCode)	// Dragging Slider
	{
		if (m_CompressionQualitySlider.GetPos() > (m_nLastPos + 2))
		{
			m_nLastPos += 5;
			if (m_nLastPos > 100)
				m_nLastPos = 100;
			m_CompressionQualitySlider.SetPos(m_nLastPos);
		}
		else if (m_CompressionQualitySlider.GetPos() < (m_nLastPos - 2))
		{
			m_nLastPos -= 5;
			if (m_nLastPos < 0)
				m_nLastPos = 0;
			m_CompressionQualitySlider.SetPos(m_nLastPos);
		}
		else
		{
			m_CompressionQualitySlider.SetPos(m_nLastPos);
			return;
		}
	}
	
	m_nCompressionQuality = m_CompressionQualitySlider.GetPos();
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_COMPRESSION_QUALITY_EDIT);
	CString sQuality;
	sQuality.Format(_T("%i"), m_nCompressionQuality);
	pEdit->SetWindowText(sQuality);
				
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

BOOL CJpegCompressionQualityDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_CompressionQualitySlider.SetRange(0, 100);
	m_CompressionQualitySlider.SetTicFreq(10);
	m_CompressionQualitySlider.SetPageSize(5);
	m_CompressionQualitySlider.SetLineSize(5);
	m_CompressionQualitySlider.SetPos(m_nCompressionQuality);
	m_nLastPos = m_nCompressionQuality;
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_COMPRESSION_QUALITY_EDIT);
	CString sQuality;
	sQuality.Format(_T("%i"), m_nCompressionQuality);
	pEdit->SetWindowText(sQuality);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
