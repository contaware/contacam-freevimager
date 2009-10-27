// PdfSaveDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "dib.h"
#include "PdfSaveDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPdfSaveDlg dialog


CPdfSaveDlg::CPdfSaveDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPdfSaveDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPdfSaveDlg)
	//}}AFX_DATA_INIT
	m_sPdfScanPaperSize = _T("Fit");
	m_nCompressionQuality = DEFAULT_JPEGCOMPRESSION;
	m_nLastPos = DEFAULT_JPEGCOMPRESSION;
}


void CPdfSaveDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPdfSaveDlg)
	DDX_Control(pDX, IDC_COMPRESSION_QUALITY_SLIDER, m_CompressionQualitySlider);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPdfSaveDlg, CDialog)
	//{{AFX_MSG_MAP(CPdfSaveDlg)
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPdfSaveDlg message handlers

BOOL CPdfSaveDlg::OnInitDialog() 
{
	// Init Combo Box
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_PAPER_SIZE);
	if (pComboBox)
	{
		pComboBox->AddString(ML_STRING(1519, "Auto (Size Paper Automatically)"));
		pComboBox->AddString(_T("Executive"));
		pComboBox->AddString(_T("Letter"));
		pComboBox->AddString(_T("Legal"));
		pComboBox->AddString(_T("A4"));
		pComboBox->AddString(_T("A3"));
		pComboBox->AddString(_T("Ledger"));
		pComboBox->AddString(_T("Tabloid"));
		pComboBox->AddString(_T("Statement"));
		pComboBox->AddString(_T("Folio"));
		pComboBox->AddString(_T("Quarto"));
		pComboBox->AddString(_T("A3Extra"));
		pComboBox->AddString(_T("A4Extra"));
		pComboBox->AddString(_T("A"));
		pComboBox->AddString(_T("B"));
		pComboBox->AddString(_T("C"));
		pComboBox->AddString(_T("D"));
		pComboBox->AddString(_T("E"));
		pComboBox->AddString(_T("F"));
		pComboBox->AddString(_T("G"));
		pComboBox->AddString(_T("H"));
		pComboBox->AddString(_T("J"));
		pComboBox->AddString(_T("K"));
		pComboBox->AddString(_T("A10"));
		pComboBox->AddString(_T("A9"));
		pComboBox->AddString(_T("A8"));
		pComboBox->AddString(_T("A7"));
		pComboBox->AddString(_T("A6"));
		pComboBox->AddString(_T("A5"));
		pComboBox->AddString(_T("A2"));
		pComboBox->AddString(_T("A1"));
		pComboBox->AddString(_T("A0"));
		pComboBox->AddString(_T("2A0"));
		pComboBox->AddString(_T("4A0"));
		pComboBox->AddString(_T("2A"));
		pComboBox->AddString(_T("4A"));
		pComboBox->AddString(_T("B10"));
		pComboBox->AddString(_T("B9"));
		pComboBox->AddString(_T("B8"));
		pComboBox->AddString(_T("B7"));
		pComboBox->AddString(_T("B6"));
		pComboBox->AddString(_T("B5"));
		pComboBox->AddString(_T("B4"));
		pComboBox->AddString(_T("B3"));
		pComboBox->AddString(_T("B2"));
		pComboBox->AddString(_T("B1"));
		pComboBox->AddString(_T("B0"));
		pComboBox->AddString(_T("JISB10"));
		pComboBox->AddString(_T("JISB9"));
		pComboBox->AddString(_T("JISB8"));
		pComboBox->AddString(_T("JISB7"));
		pComboBox->AddString(_T("JISB6"));
		pComboBox->AddString(_T("JISB5"));
		pComboBox->AddString(_T("JISB4"));
		pComboBox->AddString(_T("JISB3"));
		pComboBox->AddString(_T("JISB2"));
		pComboBox->AddString(_T("JISB1"));
		pComboBox->AddString(_T("JISB0"));
		pComboBox->AddString(_T("C10"));
		pComboBox->AddString(_T("C9"));
		pComboBox->AddString(_T("C8"));
		pComboBox->AddString(_T("C7"));
		pComboBox->AddString(_T("C6"));
		pComboBox->AddString(_T("C5"));
		pComboBox->AddString(_T("C4"));
		pComboBox->AddString(_T("C3"));
		pComboBox->AddString(_T("C2"));
		pComboBox->AddString(_T("C1"));
		pComboBox->AddString(_T("C0"));
		pComboBox->AddString(_T("RA2"));
		pComboBox->AddString(_T("RA1"));
		pComboBox->AddString(_T("RA0"));
		pComboBox->AddString(_T("SRA4"));
		pComboBox->AddString(_T("SRA3"));
		pComboBox->AddString(_T("SRA2"));
		pComboBox->AddString(_T("SRA1"));
		pComboBox->AddString(_T("SRA0"));
	}

	CDialog::OnInitDialog();

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

	// Select Paper Size
	if (pComboBox)
	{
		if (m_sPdfScanPaperSize.CompareNoCase(_T("Fit")) == 0)
			pComboBox->SetCurSel(0);
		else
		{
			int nIndex = pComboBox->FindStringExact(-1, m_sPdfScanPaperSize);
			if (nIndex == CB_ERR)
				pComboBox->SetCurSel(0);
			else
				pComboBox->SetCurSel(nIndex);
		}
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPdfSaveDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
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

void CPdfSaveDlg::OnOK() 
{
	// Get Paper Size
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_PAPER_SIZE);
	if (pComboBox)
	{
		if (pComboBox->GetCurSel() <= 0)
			m_sPdfScanPaperSize = _T("Fit");
		else
			pComboBox->GetLBText(pComboBox->GetCurSel(), m_sPdfScanPaperSize);
	}
	
	CDialog::OnOK();
}
