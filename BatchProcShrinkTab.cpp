// BatchProcShrinkTab.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "BatchProcShrinkTab.h"
#include "BatchProcDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBatchProcShrinkTab dialog


CBatchProcShrinkTab::CBatchProcShrinkTab(CWnd* pParent /*=NULL*/)
	: CTabPageSSL(CBatchProcShrinkTab::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBatchProcShrinkTab)
	m_bShrinkingPictures = FALSE;
	m_bSharpen = FALSE;
	m_nPixelsPercentSel = 1;
	m_nShrinkingPercent = 50;
	m_nShrinkingPixels = AUTO_SHRINK_MAX_SIZE;
	//}}AFX_DATA_INIT
}


void CBatchProcShrinkTab::DoDataExchange(CDataExchange* pDX)
{
	CTabPageSSL::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBatchProcShrinkTab)
	DDX_Check(pDX, IDC_CHECK_SHRINKINGPICTURES, m_bShrinkingPictures);
	DDX_Check(pDX, IDC_CHECK_SHARPEN, m_bSharpen);
	DDX_Radio(pDX, IDC_RADIO_PIXELS, m_nPixelsPercentSel);
	DDX_Text(pDX, IDC_EDIT_PERCENT, m_nShrinkingPercent);
	DDV_MinMaxInt(pDX, m_nShrinkingPercent, 1, 100);
	DDX_Text(pDX, IDC_EDIT_PIXELS, m_nShrinkingPixels);
	DDV_MinMaxInt(pDX, m_nShrinkingPixels, 1, 30000);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBatchProcShrinkTab, CTabPageSSL)
	//{{AFX_MSG_MAP(CBatchProcShrinkTab)
	ON_BN_CLICKED(IDC_CHECK_SHRINKINGPICTURES, OnCheckShrinkingpictures)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBatchProcShrinkTab message handlers

BOOL CBatchProcShrinkTab::OnInitDialog() 
{
	CTabPageSSL::OnInitDialog();
	
	// Set Spin Ctrls Range
	CSpinButtonCtrl* pSpin;
	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_PIXELS);
	if (pSpin)
		pSpin->SetRange(1, 30000);
	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_PERCENT);
	if (pSpin)
		pSpin->SetRange(1, 100);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CBatchProcShrinkTab::OnCheckShrinkingpictures() 
{
	((CBatchProcDlg*)GetParentOwner())->UpdateControls();
}
