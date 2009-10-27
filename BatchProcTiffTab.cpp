// BatchProcTiffTab.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "BatchProcTiffTab.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBatchProcTiffTab dialog


CBatchProcTiffTab::CBatchProcTiffTab(CWnd* pParent /*=NULL*/)
	: CTabPageSSL(CBatchProcTiffTab::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBatchProcTiffTab)
	m_nCompression = 2;
	m_nJpegQuality = DEFAULT_JPEGCOMPRESSION;
	m_bForceCompression = FALSE;
	m_bWorkOnAllPages = FALSE;
	//}}AFX_DATA_INIT
	m_sPdfPaperSize = _T("Fit");
}

void CBatchProcTiffTab::DoDataExchange(CDataExchange* pDX)
{
	CTabPageSSL::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBatchProcTiffTab)
	DDX_Radio(pDX, IDC_RADIO_COMPRESSION, m_nCompression);
	DDX_Text(pDX, IDC_EDIT_QUALITY, m_nJpegQuality);
	DDV_MinMaxInt(pDX, m_nJpegQuality, 0, 100);
	DDX_Check(pDX, IDC_CHECK_FORCECOMPRESSION, m_bForceCompression);
	DDX_Check(pDX, IDC_CHECK_WORKONALLPAGES, m_bWorkOnAllPages);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBatchProcTiffTab, CTabPageSSL)
	//{{AFX_MSG_MAP(CBatchProcTiffTab)
	ON_CBN_SELENDOK(IDC_COMBO_PAPER_SIZE, OnSelendokComboPaperSize)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBatchProcTiffTab message handlers

BOOL CBatchProcTiffTab::OnInitDialog() 
{
	// Init Combo Box
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_PAPER_SIZE);
	if (pComboBox)
	{
		pComboBox->AddString(ML_STRING(1518, "Auto"));
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

	CTabPageSSL::OnInitDialog();
	
	// Set Spin Ctrl Range
	CSpinButtonCtrl* pSpin;
	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_QUALITY);
	if (pSpin)
		pSpin->SetRange(0, 100);
	
	// Select Paper Size
	if (pComboBox)
	{
		if (m_sPdfPaperSize.CompareNoCase(_T("Fit")) == 0)
			pComboBox->SetCurSel(0);
		else
		{
			int nIndex = pComboBox->FindStringExact(-1, m_sPdfPaperSize);
			if (nIndex == CB_ERR)
				pComboBox->SetCurSel(0);
			else
				pComboBox->SetCurSel(nIndex);
		}
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CBatchProcTiffTab::OnSelendokComboPaperSize() 
{
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_PAPER_SIZE);
	if (pComboBox)
	{
		if (pComboBox->GetCurSel() <= 0)
			m_sPdfPaperSize = _T("Fit");
		else
			pComboBox->GetLBText(pComboBox->GetCurSel(), m_sPdfPaperSize);
	}
}
