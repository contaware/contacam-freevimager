// BatchProcJpegTab.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "BatchProcJpegTab.h"
#include "BatchProcDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBatchProcJpegTab dialog


CBatchProcJpegTab::CBatchProcJpegTab(CWnd* pParent /*=NULL*/)
	: CTabPageSSL(CBatchProcJpegTab::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBatchProcJpegTab)
	m_nJpegQuality = DEFAULT_JPEGCOMPRESSION;
	m_bAutoOrientate = FALSE;
	m_bRemoveCom = FALSE;
	m_bRemoveExif = FALSE;
	m_bRemoveIcc = FALSE;
	m_bRemoveXmp = FALSE;
	m_bRemoveIptc = FALSE;
	m_bRemoveJfif = FALSE;
	m_bRemoveOtherAppSections = FALSE;
	m_bForceJpegQuality = FALSE;
	//}}AFX_DATA_INIT
	m_nExifThumbOperationType = THUMB_NONE;
}


void CBatchProcJpegTab::DoDataExchange(CDataExchange* pDX)
{
	CTabPageSSL::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBatchProcJpegTab)
	DDX_Text(pDX, IDC_EDIT_QUALITY, m_nJpegQuality);
	DDV_MinMaxInt(pDX, m_nJpegQuality, 0, 100);
	DDX_Check(pDX, IDC_CHECK_AUTOORIENTATE, m_bAutoOrientate);
	DDX_Check(pDX, IDC_CHECK_REMOVE_COM, m_bRemoveCom);
	DDX_Check(pDX, IDC_CHECK_REMOVE_EXIF, m_bRemoveExif);
	DDX_Check(pDX, IDC_CHECK_REMOVE_ICC, m_bRemoveIcc);
	DDX_Check(pDX, IDC_CHECK_REMOVE_XMP, m_bRemoveXmp);
	DDX_Check(pDX, IDC_CHECK_REMOVE_IPTC, m_bRemoveIptc);
	DDX_Check(pDX, IDC_CHECK_REMOVE_JFIF, m_bRemoveJfif);
	DDX_Check(pDX, IDC_CHECK_REMOVE_OTHER_APPSECTIONS, m_bRemoveOtherAppSections);
	DDX_Check(pDX, IDC_CHECK_FORCEQUALITY, m_bForceJpegQuality);
	//}}AFX_DATA_MAP
	DDX_CBIndex(pDX, IDC_COMBO_EXIF_THUMBNAIL, m_nExifThumbOperationType);
}


BEGIN_MESSAGE_MAP(CBatchProcJpegTab, CTabPageSSL)
	//{{AFX_MSG_MAP(CBatchProcJpegTab)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBatchProcJpegTab message handlers

BOOL CBatchProcJpegTab::OnInitDialog() 
{
	// Init Combo Box
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_EXIF_THUMBNAIL);
	if (pComboBox)
	{
		pComboBox->AddString(ML_STRING(1514, "Update EXIF Thumb if Necessary"));
		pComboBox->AddString(ML_STRING(1515, "Update EXIF Thumbnail"));
		pComboBox->AddString(ML_STRING(1516, "Add / Update EXIF Thumbnail"));
		pComboBox->AddString(ML_STRING(1517, "Remove EXIF Thumbnail"));
	}

	CTabPageSSL::OnInitDialog();
	
	// Set Spin Ctrl Range
	CSpinButtonCtrl* pSpin;
	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_QUALITY);
	if (pSpin)
		pSpin->SetRange(0, 100);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
