// TiffSaveDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "dib.h"
#include "TiffSaveDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTiffSaveDlg dialog


CTiffSaveDlg::CTiffSaveDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTiffSaveDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTiffSaveDlg)
	m_nCompression = 0;
	//}}AFX_DATA_INIT
	m_nBpp = 0;
	m_bGrayscale = FALSE;
	m_nCompressionQuality = DEFAULT_JPEGCOMPRESSION;
	m_nLastPos = DEFAULT_JPEGCOMPRESSION;
}


void CTiffSaveDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTiffSaveDlg)
	DDX_Control(pDX, IDC_COMPRESSION_QUALITY_SLIDER, m_CompressionQualitySlider);
	DDX_Radio(pDX, IDC_RADIO_COMPRESSION, m_nCompression);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTiffSaveDlg, CDialog)
	//{{AFX_MSG_MAP(CTiffSaveDlg)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_RADIO_COMPRESSION, OnRadioCompression)
	ON_BN_CLICKED(IDC_RADIO_COMPRESSION_CCITTFAX4, OnRadioCompressionCcittfax4)
	ON_BN_CLICKED(IDC_RADIO_COMPRESSION_LZW, OnRadioCompressionLzw)
	ON_BN_CLICKED(IDC_RADIO_COMPRESSION_JPEG, OnRadioCompressionJpeg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTiffSaveDlg message handlers

BOOL CTiffSaveDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	ASSERT(m_nBpp > 0);

	// Init Quality Slider
	m_CompressionQualitySlider.SetRange(0, 100);
	m_CompressionQualitySlider.SetTicFreq(10);
	m_CompressionQualitySlider.SetPageSize(5);
	m_CompressionQualitySlider.SetLineSize(5);
	m_CompressionQualitySlider.SetPos(m_nCompressionQuality);
	m_nLastPos = m_nCompressionQuality;
	
	// Init Quality Edit
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_COMPRESSION_QUALITY_EDIT);
	CString sQuality;
	sQuality.Format(_T("%i"), m_nCompressionQuality);
	pEdit->SetWindowText(sQuality);

	// Check, disable and change compression if necessary
	CButton* pRadio;
	if (m_nBpp <= 8)
	{
		if (m_nBpp < 8 || !m_bGrayscale)
		{
			pRadio = (CButton*)GetDlgItem(IDC_RADIO_COMPRESSION_JPEG);
			pRadio->EnableWindow(FALSE);
			if (m_nCompression == 3)	// JPEG
			{
				pRadio->SetCheck(0);
				pRadio = (CButton*)GetDlgItem(IDC_RADIO_COMPRESSION_LZW);
				pRadio->SetCheck(1);
				m_nCompression = 2;		// LZW
			}
		}
	}
	if (m_nBpp != 1)
	{
		pRadio = (CButton*)GetDlgItem(IDC_RADIO_COMPRESSION_CCITTFAX4);
		pRadio->EnableWindow(FALSE);
		if (m_nCompression == 1)	// CCITTFAX4
		{
			pRadio->SetCheck(0);
			pRadio = (CButton*)GetDlgItem(IDC_RADIO_COMPRESSION_LZW);
			pRadio->SetCheck(1);
			m_nCompression = 2;		// LZW
		}
	}

	// Disable Slider?
	if (m_nCompression != 3)	// Not JPEG
	{
		m_CompressionQualitySlider.EnableWindow(FALSE);
		pEdit->EnableWindow(FALSE);
		pEdit = (CEdit*)GetDlgItem(IDC_COMPRESSION_QUALITY_TEXT);
		pEdit->EnableWindow(FALSE);
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTiffSaveDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
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

void CTiffSaveDlg::OnRadioCompression() 
{
	m_CompressionQualitySlider.EnableWindow(FALSE);
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_COMPRESSION_QUALITY_EDIT);
	pEdit->EnableWindow(FALSE);
	pEdit = (CEdit*)GetDlgItem(IDC_COMPRESSION_QUALITY_TEXT);
	pEdit->EnableWindow(FALSE);
}

void CTiffSaveDlg::OnRadioCompressionCcittfax4() 
{
	m_CompressionQualitySlider.EnableWindow(FALSE);
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_COMPRESSION_QUALITY_EDIT);
	pEdit->EnableWindow(FALSE);
	pEdit = (CEdit*)GetDlgItem(IDC_COMPRESSION_QUALITY_TEXT);
	pEdit->EnableWindow(FALSE);
}

void CTiffSaveDlg::OnRadioCompressionLzw() 
{
	m_CompressionQualitySlider.EnableWindow(FALSE);
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_COMPRESSION_QUALITY_EDIT);
	pEdit->EnableWindow(FALSE);
	pEdit = (CEdit*)GetDlgItem(IDC_COMPRESSION_QUALITY_TEXT);
	pEdit->EnableWindow(FALSE);
}

void CTiffSaveDlg::OnRadioCompressionJpeg() 
{
	m_CompressionQualitySlider.EnableWindow(TRUE);
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_COMPRESSION_QUALITY_EDIT);
	pEdit->EnableWindow(TRUE);
	pEdit = (CEdit*)GetDlgItem(IDC_COMPRESSION_QUALITY_TEXT);
	pEdit->EnableWindow(TRUE);
}
