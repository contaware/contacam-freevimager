// SendMailDocsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "SendMailDocsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSendMailDocsDlg dialog


CSendMailDocsDlg::CSendMailDocsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSendMailDocsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSendMailDocsDlg)
	m_nOptimizationSelection = EMAIL_OPT;
	m_nPixelsPercentSel = 1;
	m_bShrinkingPictures = TRUE;
	m_bZipFile = FALSE;
	m_nShrinkingPercent = 50;
	m_nShrinkingPixels = AUTO_SHRINK_MAX_SIZE;
	m_nJpegQuality = DEFAULT_JPEGCOMPRESSION;
	m_sZipFileName = _T("Files.zip");
	m_bShrinkingVideos = TRUE;
	m_bPictureExtChange = TRUE;
	//}}AFX_DATA_INIT
}

void CSendMailDocsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSendMailDocsDlg)
	DDX_Radio(pDX, IDC_RADIO_OPTIMIZE_EMAIL, m_nOptimizationSelection);
	DDX_Check(pDX, IDC_CHECK_ZIP, m_bZipFile);
	DDX_Text(pDX, IDC_EDIT_ZIPFILE_NAME, m_sZipFileName);
	DDX_Check(pDX, IDC_CHECK_SHRINKINGVIDEOS, m_bShrinkingVideos);
	DDX_Radio(pDX, IDC_RADIO_PIXELS, m_nPixelsPercentSel);
	DDX_Check(pDX, IDC_CHECK_SHRINKINGPICTURES, m_bShrinkingPictures);
	DDX_Text(pDX, IDC_EDIT_PERCENT, m_nShrinkingPercent);
	DDV_MinMaxInt(pDX, m_nShrinkingPercent, 1, 100);
	DDX_Text(pDX, IDC_EDIT_PIXELS, m_nShrinkingPixels);
	DDV_MinMaxInt(pDX, m_nShrinkingPixels, 1, 30000);
	DDX_Text(pDX, IDC_EDIT_QUALITY, m_nJpegQuality);
	DDV_MinMaxInt(pDX, m_nJpegQuality, 1, 100);
	DDX_Check(pDX, IDC_CHECK_PICTURE_EXTCHANGE, m_bPictureExtChange);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSendMailDocsDlg, CDialog)
	//{{AFX_MSG_MAP(CSendMailDocsDlg)
	ON_BN_CLICKED(IDC_RADIO_OPTIMIZE_ADVANCED, OnRadioOptimizeAdvanced)
	ON_BN_CLICKED(IDC_RADIO_OPTIMIZE_EMAIL, OnRadioOptimizeEmail)
	ON_BN_CLICKED(IDC_RADIO_OPTIMIZE_UNCHANGED, OnRadioOptimizeUnchanged)
	ON_BN_CLICKED(IDC_CHECK_SHRINKINGPICTURES, OnCheckShrinkingpictures)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSendMailDocsDlg message handlers

BOOL CSendMailDocsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CSpinButtonCtrl* pSpin;
	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_PIXELS);
	if (pSpin)
		pSpin->SetRange(1, 30000);
	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_PERCENT);
	if (pSpin)
		pSpin->SetRange(1, 100);
	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_QUALITY);
	if (pSpin)
		pSpin->SetRange(1, 100);

	// Gray Advanced Settings
	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_PIXELS);
	if (pSpin)
		pSpin->EnableWindow(FALSE);
	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_PERCENT);
	if (pSpin)
		pSpin->EnableWindow(FALSE);
	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_QUALITY);
	if (pSpin)
		pSpin->EnableWindow(FALSE);

	CEdit* pEdit;
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PIXELS);
	if (pEdit)
		pEdit->EnableWindow(FALSE);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PERCENT);
	if (pEdit)
		pEdit->EnableWindow(FALSE);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_QUALITY);
	if (pEdit)
		pEdit->EnableWindow(FALSE);

	CButton* pCheck;
	pCheck = (CButton*)GetDlgItem(IDC_CHECK_PICTURE_EXTCHANGE);
	if (pCheck)
		pCheck->EnableWindow(FALSE);
	pCheck = (CButton*)GetDlgItem(IDC_CHECK_SHRINKINGPICTURES);
	if (pCheck)
		pCheck->EnableWindow(FALSE);
	pCheck = (CButton*)GetDlgItem(IDC_CHECK_SHRINKINGVIDEOS);
	if (pCheck)
		pCheck->EnableWindow(FALSE);

	CButton* pRadio;
	pRadio = (CButton*)GetDlgItem(IDC_RADIO_PIXELS);
	if (pRadio)
		pRadio->EnableWindow(FALSE);
	pRadio = (CButton*)GetDlgItem(IDC_RADIO_PERCENT);
	if (pRadio)
		pRadio->EnableWindow(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSendMailDocsDlg::OnRadioOptimizeAdvanced() 
{
	UpdateData(TRUE);

	CSpinButtonCtrl* pSpin;
	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_PIXELS);
	if (pSpin)
		pSpin->EnableWindow(m_bShrinkingPictures);
	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_PERCENT);
	if (pSpin)
		pSpin->EnableWindow(m_bShrinkingPictures);
	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_QUALITY);
	if (pSpin)
		pSpin->EnableWindow(TRUE);

	CEdit* pEdit;
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PIXELS);
	if (pEdit)
		pEdit->EnableWindow(m_bShrinkingPictures);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PERCENT);
	if (pEdit)
		pEdit->EnableWindow(m_bShrinkingPictures);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_QUALITY);
	if (pEdit)
		pEdit->EnableWindow(TRUE);

	CButton* pCheck;
	pCheck = (CButton*)GetDlgItem(IDC_CHECK_PICTURE_EXTCHANGE);
	if (pCheck)
		pCheck->EnableWindow(TRUE);
	pCheck = (CButton*)GetDlgItem(IDC_CHECK_SHRINKINGPICTURES);
	if (pCheck)
		pCheck->EnableWindow(TRUE);
	pCheck = (CButton*)GetDlgItem(IDC_CHECK_SHRINKINGVIDEOS);
	if (pCheck)
		pCheck->EnableWindow(TRUE);

	CButton* pRadio;
	pRadio = (CButton*)GetDlgItem(IDC_RADIO_PIXELS);
	if (pRadio)
		pRadio->EnableWindow(m_bShrinkingPictures);
	pRadio = (CButton*)GetDlgItem(IDC_RADIO_PERCENT);
	if (pRadio)
		pRadio->EnableWindow(m_bShrinkingPictures);
}

void CSendMailDocsDlg::OnRadioOptimizeEmail() 
{
	CSpinButtonCtrl* pSpin;
	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_PIXELS);
	if (pSpin)
		pSpin->EnableWindow(FALSE);
	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_PERCENT);
	if (pSpin)
		pSpin->EnableWindow(FALSE);
	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_QUALITY);
	if (pSpin)
		pSpin->EnableWindow(FALSE);

	CEdit* pEdit;
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PIXELS);
	if (pEdit)
		pEdit->EnableWindow(FALSE);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PERCENT);
	if (pEdit)
		pEdit->EnableWindow(FALSE);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_QUALITY);
	if (pEdit)
		pEdit->EnableWindow(FALSE);

	CButton* pCheck;
	pCheck = (CButton*)GetDlgItem(IDC_CHECK_PICTURE_EXTCHANGE);
	if (pCheck)
		pCheck->EnableWindow(FALSE);
	pCheck = (CButton*)GetDlgItem(IDC_CHECK_SHRINKINGPICTURES);
	if (pCheck)
		pCheck->EnableWindow(FALSE);
	pCheck = (CButton*)GetDlgItem(IDC_CHECK_SHRINKINGVIDEOS);
	if (pCheck)
		pCheck->EnableWindow(FALSE);

	CButton* pRadio;
	pRadio = (CButton*)GetDlgItem(IDC_RADIO_PIXELS);
	if (pRadio)
		pRadio->EnableWindow(FALSE);
	pRadio = (CButton*)GetDlgItem(IDC_RADIO_PERCENT);
	if (pRadio)
		pRadio->EnableWindow(FALSE);
}

void CSendMailDocsDlg::OnRadioOptimizeUnchanged() 
{
	CSpinButtonCtrl* pSpin;
	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_PIXELS);
	if (pSpin)
		pSpin->EnableWindow(FALSE);
	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_PERCENT);
	if (pSpin)
		pSpin->EnableWindow(FALSE);
	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_QUALITY);
	if (pSpin)
		pSpin->EnableWindow(FALSE);

	CEdit* pEdit;
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PIXELS);
	if (pEdit)
		pEdit->EnableWindow(FALSE);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PERCENT);
	if (pEdit)
		pEdit->EnableWindow(FALSE);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_QUALITY);
	if (pEdit)
		pEdit->EnableWindow(FALSE);

	CButton* pCheck;
	pCheck = (CButton*)GetDlgItem(IDC_CHECK_PICTURE_EXTCHANGE);
	if (pCheck)
		pCheck->EnableWindow(FALSE);
	pCheck = (CButton*)GetDlgItem(IDC_CHECK_SHRINKINGPICTURES);
	if (pCheck)
		pCheck->EnableWindow(FALSE);
	pCheck = (CButton*)GetDlgItem(IDC_CHECK_SHRINKINGVIDEOS);
	if (pCheck)
		pCheck->EnableWindow(FALSE);

	CButton* pRadio;
	pRadio = (CButton*)GetDlgItem(IDC_RADIO_PIXELS);
	if (pRadio)
		pRadio->EnableWindow(FALSE);
	pRadio = (CButton*)GetDlgItem(IDC_RADIO_PERCENT);
	if (pRadio)
		pRadio->EnableWindow(FALSE);
}

void CSendMailDocsDlg::OnCheckShrinkingpictures() 
{
	UpdateData(TRUE);

	CSpinButtonCtrl* pSpin;
	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_PIXELS);
	if (pSpin)
		pSpin->EnableWindow(m_bShrinkingPictures);
	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_PERCENT);
	if (pSpin)
		pSpin->EnableWindow(m_bShrinkingPictures);

	CEdit* pEdit;
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PIXELS);
	if (pEdit)
		pEdit->EnableWindow(m_bShrinkingPictures);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PERCENT);
	if (pEdit)
		pEdit->EnableWindow(m_bShrinkingPictures);

	CButton* pRadio;
	pRadio = (CButton*)GetDlgItem(IDC_RADIO_PIXELS);
	if (pRadio)
		pRadio->EnableWindow(m_bShrinkingPictures);
	pRadio = (CButton*)GetDlgItem(IDC_RADIO_PERCENT);
	if (pRadio)
		pRadio->EnableWindow(m_bShrinkingPictures);
}
