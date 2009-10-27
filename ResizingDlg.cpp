// ResizingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "ResizingDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CResizingDlg dialog

#ifndef MAX
#  define MAX(a,b)  ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#  define MIN(a,b)  ((a) < (b) ? (a) : (b))
#endif

#define UPPER_VALUE				1048576.0
#define MAX_RESIZE_FACTOR		128.0

CResizingDlg::CResizingDlg(int nFullWidth, int nFullHeight,
						   int nCurrentWidth, int nCurrentHeight,
						   CWnd* pParent/*=NULL*/)
	: CDialog(CResizingDlg::IDD, pParent)
{
	// OnInitDialog() has not been called at this point
	m_bDlgInitialized = FALSE;

	// Check params
	if (nFullWidth < 1)
		nFullWidth = 1;
	if (nFullHeight < 1)
		nFullHeight = 1;
	if (nCurrentWidth < 1)
		nCurrentWidth = 1;
	if (nCurrentHeight < 1)
		nCurrentHeight = 1;

	//{{AFX_DATA_INIT(CResizingDlg)
	m_nPixelsWidth = nCurrentWidth;
	m_nPixelsHeight = nCurrentHeight;
	m_nPercentWidth = MAX(1, Round(nCurrentWidth * 100.0 / nFullWidth));
	m_nPercentHeight = MAX(1, Round(nCurrentHeight * 100.0 / nFullHeight));
	m_bRetainAspect = m_nPercentWidth == m_nPercentHeight ? TRUE : FALSE;
	//}}AFX_DATA_INIT

	m_nFullWidth = nFullWidth;
	m_nFullHeight = nFullHeight;
	m_dFullRatio = (double)nFullWidth / (double)nFullHeight;
	double dMaxResizeFactorX = UPPER_VALUE / (double)nFullWidth;
	double dMaxResizeFactorY = UPPER_VALUE / (double)nFullHeight;
	double dMaxResizeFactorXY = MIN(dMaxResizeFactorX, dMaxResizeFactorY);
	m_dMaxResizeFactor = MIN(MAX_RESIZE_FACTOR, dMaxResizeFactorXY);

	// Only use normal validation in OnOK()
	m_bNormalValidate = FALSE;
}

void CResizingDlg::DoDataExchange(CDataExchange* pDX)
{
	// Additional empty check
	if (!m_bNormalValidate)
	{
		CString sText;
		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PERCENT_HEIGHT);
		pEdit->GetWindowText(sText);
		if (sText == _T(""))
			return;
		pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PERCENT_WIDTH);
		pEdit->GetWindowText(sText);
		if (sText == _T(""))
			return;
		pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PIXELS_HEIGHT);
		pEdit->GetWindowText(sText);
		if (sText == _T(""))
			return;
		pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PIXELS_WIDTH);
		pEdit->GetWindowText(sText);
		if (sText == _T(""))
			return;
	}

	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CResizingDlg)
	DDX_Text(pDX, IDC_EDIT_PERCENT_HEIGHT, m_nPercentHeight);
	DDX_Text(pDX, IDC_EDIT_PERCENT_WIDTH, m_nPercentWidth);
	DDX_Text(pDX, IDC_EDIT_PIXELS_HEIGHT, m_nPixelsHeight);
	DDX_Text(pDX, IDC_EDIT_PIXELS_WIDTH, m_nPixelsWidth);
	DDX_Check(pDX, IDC_CHECK_RETAIN_ASPECT, m_bRetainAspect);
	//}}AFX_DATA_MAP
	DDV_MinMaxInt(pDX, m_nPercentHeight, 1, (int)(m_dMaxResizeFactor * 100.0));
	DDV_MinMaxInt(pDX, m_nPercentWidth, 1, (int)(m_dMaxResizeFactor * 100.0));
	DDV_MinMaxInt(pDX, m_nPixelsHeight, 1, (int)(m_dMaxResizeFactor * (double)m_nFullHeight));
	DDV_MinMaxInt(pDX, m_nPixelsWidth, 1, (int)(m_dMaxResizeFactor * (double)m_nFullWidth));
}

BEGIN_MESSAGE_MAP(CResizingDlg, CDialog)
	//{{AFX_MSG_MAP(CResizingDlg)
	ON_EN_CHANGE(IDC_EDIT_PERCENT_HEIGHT, OnChangeEditPercentHeight)
	ON_EN_CHANGE(IDC_EDIT_PERCENT_WIDTH, OnChangeEditPercentWidth)
	ON_EN_CHANGE(IDC_EDIT_PIXELS_HEIGHT, OnChangeEditPixelsHeight)
	ON_EN_CHANGE(IDC_EDIT_PIXELS_WIDTH, OnChangeEditPixelsWidth)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResizingDlg message handlers

BOOL CResizingDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// Init Spin Controls
	CSpinButtonCtrl* pSpin;
	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_PIXELS_WIDTH);
	pSpin->SetRange32(1, (int)(m_dMaxResizeFactor * (double)m_nFullWidth));
	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_PIXELS_HEIGHT);
	pSpin->SetRange32(1, (int)(m_dMaxResizeFactor * (double)m_nFullHeight));
	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_PERCENT_WIDTH);
	pSpin->SetRange32(1, (int)(m_dMaxResizeFactor * 100.0));
	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_PERCENT_HEIGHT);
	pSpin->SetRange32(1, (int)(m_dMaxResizeFactor * 100.0));

	// OnInitDialog() has been called
	m_bDlgInitialized = TRUE;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CResizingDlg::OnChangeEditPercentHeight() 
{
	if (m_bDlgInitialized)
	{
		if (UpdateData(TRUE))
		{
			if (m_bRetainAspect)
			{
				m_nPercentWidth = m_nPercentHeight;
				m_nPixelsWidth = Round(m_nFullWidth * m_nPercentWidth / 100.0);
				m_nPixelsHeight = Round(m_nFullHeight * m_nPercentHeight / 100.0);
			}
			else
				m_nPixelsHeight = Round(m_nFullHeight * m_nPercentHeight / 100.0);

			if (m_nPercentWidth == 0) m_nPercentWidth = 1;
			if (m_nPixelsWidth == 0) m_nPixelsWidth = 1;
			if (m_nPixelsHeight == 0) m_nPixelsHeight = 1;

			UpdateData(FALSE);
		}
		else
		{
			CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PERCENT_HEIGHT);
			CString sValue(_T("100"));
			pEdit->SetWindowText(sValue);
			pEdit->SetFocus();
			pEdit->SetSel(0xFFFF0000);
		}
	}
}

void CResizingDlg::OnChangeEditPercentWidth() 
{
	if (m_bDlgInitialized)
	{
		if (UpdateData(TRUE))
		{
			if (m_bRetainAspect)
			{
				m_nPercentHeight = m_nPercentWidth;
				m_nPixelsWidth = Round(m_nFullWidth * m_nPercentWidth / 100.0);
				m_nPixelsHeight = Round(m_nFullHeight * m_nPercentHeight / 100.0);
			}
			else
				m_nPixelsWidth = Round(m_nFullWidth * m_nPercentWidth / 100.0);

			if (m_nPercentHeight == 0) m_nPercentHeight = 1;
			if (m_nPixelsWidth == 0) m_nPixelsWidth = 1;
			if (m_nPixelsHeight == 0) m_nPixelsHeight = 1;

			UpdateData(FALSE);
		}
		else
		{
			CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PERCENT_WIDTH);
			CString sValue(_T("100"));
			pEdit->SetWindowText(sValue);
			pEdit->SetFocus();
			pEdit->SetSel(0xFFFF0000);
		}
	}
}

void CResizingDlg::OnChangeEditPixelsHeight() 
{
	if (m_bDlgInitialized)
	{
		if (UpdateData(TRUE))
		{
			if (m_bRetainAspect)
			{
				m_nPixelsWidth = Round(m_nPixelsHeight * m_dFullRatio);
				m_nPercentWidth = Round(m_nPixelsWidth * 100.0 / m_nFullWidth);
				m_nPercentHeight = Round(m_nPixelsHeight * 100.0 / m_nFullHeight);
			}
			else
				m_nPercentHeight = Round(m_nPixelsHeight * 100.0 / m_nFullHeight);

			if (m_nPixelsWidth == 0) m_nPixelsWidth = 1;
			if (m_nPercentWidth == 0) m_nPercentWidth = 1;
			if (m_nPercentHeight == 0) m_nPercentHeight = 1;

			UpdateData(FALSE);
		}
		else
		{
			CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PIXELS_HEIGHT);
			CString sValue;
			sValue.Format(_T("%i"), m_nFullHeight);
			pEdit->SetWindowText(sValue);
			pEdit->SetFocus();
			pEdit->SetSel(0xFFFF0000);
		}
	}
}

void CResizingDlg::OnChangeEditPixelsWidth() 
{
	if (m_bDlgInitialized)
	{
		if (UpdateData(TRUE))
		{
			if (m_bRetainAspect)
			{
				m_nPixelsHeight = Round(m_nPixelsWidth / m_dFullRatio);
				m_nPercentWidth = Round(m_nPixelsWidth * 100.0 / m_nFullWidth);
				m_nPercentHeight = Round(m_nPixelsHeight * 100.0 / m_nFullHeight);
			}
			else
				m_nPercentWidth = Round(m_nPixelsWidth * 100.0 / m_nFullWidth);

			if (m_nPixelsHeight == 0) m_nPixelsHeight = 1;
			if (m_nPercentWidth == 0) m_nPercentWidth = 1;
			if (m_nPercentHeight == 0) m_nPercentHeight = 1;

			UpdateData(FALSE);
		}
		else
		{
			CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PIXELS_WIDTH);
			CString sValue;
			sValue.Format(_T("%i"), m_nFullWidth);
			pEdit->SetWindowText(sValue);
			pEdit->SetFocus();
			pEdit->SetSel(0xFFFF0000);
		}
	}
}

void CResizingDlg::OnOK() 
{
	m_bNormalValidate = TRUE;
	CDialog::OnOK();
	m_bNormalValidate = FALSE;
}
