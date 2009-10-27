// AnimGifSaveDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "AnimGifSaveDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAnimGifSaveDlg dialog


CAnimGifSaveDlg::CAnimGifSaveDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAnimGifSaveDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAnimGifSaveDlg)
	m_bDitherColorConversion = TRUE;
	m_nColorTables = 0;
	m_nNumColors = 0;
	m_uiPlayTimes = 1;
	m_nLoopInfinite = 1;
	//}}AFX_DATA_INIT
}


void CAnimGifSaveDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAnimGifSaveDlg)
	DDX_Check(pDX, IDC_DITHERCOLORCONVERSION, m_bDitherColorConversion);
	DDX_Radio(pDX, IDC_RADIO_COLORTABLES, m_nColorTables);
	DDX_Radio(pDX, IDC_RADIO_NUMCOL_255, m_nNumColors);
	DDX_Text(pDX, IDC_EDIT_PLAY_TIMES, m_uiPlayTimes);
	DDV_MinMaxUInt(pDX, m_uiPlayTimes, 1, 10000);
	DDX_Radio(pDX, IDC_RADIO_LOOP, m_nLoopInfinite);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAnimGifSaveDlg, CDialog)
	//{{AFX_MSG_MAP(CAnimGifSaveDlg)
	ON_BN_CLICKED(IDC_RADIO_LOOP, OnRadioLoop)
	ON_BN_CLICKED(IDC_RADIO_LOOP_INFINITE, OnRadioLoopInfinite)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAnimGifSaveDlg message handlers

BOOL CAnimGifSaveDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PLAY_TIMES);
	CSpinButtonCtrl* pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_PLAY_TIMES);
	pSpin->SetRange(1, 10000);

	if (m_nLoopInfinite == 1)
	{
		pEdit->EnableWindow(FALSE);
		pSpin->EnableWindow(FALSE);
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAnimGifSaveDlg::OnRadioLoop() 
{
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PLAY_TIMES);
	pEdit->EnableWindow(TRUE);

	CSpinButtonCtrl* pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_PLAY_TIMES);
	pSpin->EnableWindow(TRUE);
}

void CAnimGifSaveDlg::OnRadioLoopInfinite() 
{
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PLAY_TIMES);
	pEdit->EnableWindow(FALSE);

	CSpinButtonCtrl* pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_PLAY_TIMES);
	pSpin->EnableWindow(FALSE);
}
