// DecreaseBppDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "DecreaseBppDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDecreaseBppDlg dialog


CDecreaseBppDlg::CDecreaseBppDlg(UINT uiMaxColorsMin, UINT uiMaxColorsMax, CWnd* pParent /*=NULL*/)
	: CDialog(CDecreaseBppDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDecreaseBppDlg)
	m_uiMaxColors = 0;
	m_bDitherColorConversion = FALSE;
	//}}AFX_DATA_INIT
	m_uiMaxColorsMin = uiMaxColorsMin;
	m_uiMaxColorsMax = uiMaxColorsMax;
}


void CDecreaseBppDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDecreaseBppDlg)
	DDX_Text(pDX, IDC_EDIT_MAXCOLORS, m_uiMaxColors);
	DDV_MinMaxUInt(pDX, m_uiMaxColors, m_uiMaxColorsMin, m_uiMaxColorsMax);
	DDX_Check(pDX, IDC_DITHERCOLORCONVERSION, m_bDitherColorConversion);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDecreaseBppDlg, CDialog)
	//{{AFX_MSG_MAP(CDecreaseBppDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDecreaseBppDlg message handlers

BOOL CDecreaseBppDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CSpinButtonCtrl* pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_MAXCOLORS);
	pSpin->SetRange(m_uiMaxColorsMin, m_uiMaxColorsMax);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
