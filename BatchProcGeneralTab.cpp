// BatchProcGeneralTab.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "BatchProcGeneralTab.h"
#include "BatchProcDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBatchProcGeneralTab dialog


CBatchProcGeneralTab::CBatchProcGeneralTab(CWnd* pParent /*=NULL*/)
	: CTabPageSSL(CBatchProcGeneralTab::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBatchProcGeneralTab)
	m_bMergeXmp = FALSE;
	m_nIptcPriority = 0;
	m_dFrameRate = 1.0;
	m_bExifTimeOffset = FALSE;
	m_nExifTimeOffsetDays = 0;
	m_nExifTimeOffsetHours = 0;
	m_nExifTimeOffsetMin = 0;
	m_nExifTimeOffsetSec = 0;
	m_nExifTimeOffsetSign = 0;
	//}}AFX_DATA_INIT
	m_nExtChangeType = NO_CHANGE;
}


void CBatchProcGeneralTab::DoDataExchange(CDataExchange* pDX)
{
	CTabPageSSL::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBatchProcGeneralTab)
	DDX_Check(pDX, IDC_CHECK_MERGE_XMP, m_bMergeXmp);
	DDX_Radio(pDX, IDC_RADIO_IPTCFROMXMP_PRIORITY, m_nIptcPriority);
	DDX_Text(pDX, IDC_EDIT_FRAMERATE, m_dFrameRate);
	DDX_Check(pDX, IDC_CHECK_EXIF_TIMEOFFSET, m_bExifTimeOffset);
	DDX_Text(pDX, IDC_EDIT_EXIF_TIMEOFFSET_DAYS, m_nExifTimeOffsetDays);
	DDV_MinMaxInt(pDX, m_nExifTimeOffsetDays, 0, 9999);
	DDX_Text(pDX, IDC_EDIT_EXIF_TIMEOFFSET_HOURS, m_nExifTimeOffsetHours);
	DDV_MinMaxInt(pDX, m_nExifTimeOffsetHours, 0, 23);
	DDX_Text(pDX, IDC_EDIT_EXIF_TIMEOFFSET_MIN, m_nExifTimeOffsetMin);
	DDV_MinMaxInt(pDX, m_nExifTimeOffsetMin, 0, 59);
	DDX_Text(pDX, IDC_EDIT_EXIF_TIMEOFFSET_SEC, m_nExifTimeOffsetSec);
	DDV_MinMaxInt(pDX, m_nExifTimeOffsetSec, 0, 59);
	DDX_CBIndex(pDX, IDC_COMBO_EXIF_TIMEOFFSET_SIGN, m_nExifTimeOffsetSign);
	//}}AFX_DATA_MAP
	DDX_CBIndex(pDX, IDC_COMBO_EXTENSION, m_nExtChangeType);
}


BEGIN_MESSAGE_MAP(CBatchProcGeneralTab, CTabPageSSL)
	//{{AFX_MSG_MAP(CBatchProcGeneralTab)
	ON_BN_CLICKED(IDC_BUTTON_MERGE_XMP, OnButtonMergeXmp)
	ON_BN_CLICKED(IDC_CHECK_MERGE_XMP, OnCheckMergeXmp)
	ON_BN_CLICKED(IDC_CHECK_EXIF_TIMEOFFSET, OnCheckExifTimeoffset)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBatchProcGeneralTab message handlers

BOOL CBatchProcGeneralTab::OnInitDialog() 
{
	// Init Combo Box
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_EXTENSION);
	if (pComboBox)
	{
		pComboBox->AddString(ML_STRING(1506, "No Image Extension Change"));
		pComboBox->AddString(ML_STRING(1507, "Auto Optimize Image Extensions"));
		pComboBox->AddString(ML_STRING(1508, "Convert All Images To Jpeg"));
		pComboBox->AddString(ML_STRING(1509, "Convert All Images To Png"));
		pComboBox->AddString(ML_STRING(1510, "Convert All Images To Gif"));
		pComboBox->AddString(ML_STRING(1511, "Convert All Images To Tiff"));
		pComboBox->AddString(ML_STRING(1512, "Convert All Images To Bmp"));
		pComboBox->AddString(ML_STRING(1513, "Convert All Images To Pcx"));
		pComboBox->AddString(ML_STRING(1711, "Convert All Images To Emf"));
	}

	CTabPageSSL::OnInitDialog();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CBatchProcGeneralTab::OnButtonMergeXmp() 
{
	// Call Xmp Import Dialog
	((CUImagerApp*)::AfxGetApp())->m_pXmpDlg->ClearExifDateCreated();
	((CUImagerApp*)::AfxGetApp())->m_pXmpDlg->DoModal();
}

void CBatchProcGeneralTab::OnCheckMergeXmp() 
{
	((CBatchProcDlg*)GetParentOwner())->UpdateControls();
}

void CBatchProcGeneralTab::OnCheckExifTimeoffset() 
{
	((CBatchProcDlg*)GetParentOwner())->UpdateControls();
}
