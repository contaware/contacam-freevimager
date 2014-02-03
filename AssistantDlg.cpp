// AssistantDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "AssistantDlg.h"
#include "VideoDeviceDoc.h"
#include "VideoDeviceView.h"
#include "VideoDevicePropertySheet.h"
#include "BrowseDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef VIDEODEVICEDOC

/////////////////////////////////////////////////////////////////////////////
// CAssistantDlg dialog

CAssistantDlg::CAssistantDlg(CVideoDeviceDoc* pDoc, CWnd* pParent /*=NULL*/)
	: CDialog(CAssistantDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAssistantDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_pDoc = pDoc;
}

CAssistantDlg::~CAssistantDlg()
{
}

void CAssistantDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAssistantDlg)
	DDX_Check(pDX, IDC_CHECK_24H_REC, m_bCheck24hRec);
	DDX_CBIndex(pDX, IDC_COMBO_KEEPFOR, m_nComboKeepFor);
	DDX_Text(pDX, IDC_EDIT_NAME, m_sName);
	DDX_Radio(pDX, IDC_RADIO_MOVDET, m_nUsage);
	DDX_CBIndex(pDX, IDC_COMBO_SNAPSHOT_RATE, m_nComboSnapshotRate);
	DDX_CBIndex(pDX, IDC_COMBO_SNAPSHOTHISTORY_RATE, m_nComboSnapshotHistoryRate);
	DDX_CBIndex(pDX, IDC_COMBO_SNAPSHOTHISTORY_SIZE, m_nComboSnapshotHistorySize);
	DDX_Check(pDX, IDC_CHECK_FULLSTRETCH, m_bCheckFullStretch);
	DDX_Check(pDX, IDC_CHECK_PRINTCOMMAND, m_bCheckPrintCommand);
	DDX_Check(pDX, IDC_CHECK_SAVECOMMAND, m_bCheckSaveCommand);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAssistantDlg, CDialog)
	//{{AFX_MSG_MAP(CAssistantDlg)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_RADIO_MOVDET, OnRadioMovdet)
	ON_BN_CLICKED(IDC_RADIO_SNAPSHOTHISTORY, OnRadioSnapshothistory)
	ON_BN_CLICKED(IDC_RADIO_SNAPSHOT, OnRadioSnapshot)
	ON_BN_CLICKED(IDC_RADIO_MANUAL, OnRadioManual)
	ON_WM_TIMER()
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_RADIO_NOCHANGE, OnRadioNochange)
	ON_BN_CLICKED(IDC_BUTTON_PARENT_DIR, OnButtonParentDir)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAssistantDlg message handlers

BOOL CAssistantDlg::OnInitDialog()
{
	// Get a copy of m_sRecordAutoSaveDir
	// (do not modify it now because various threads
	// using it are still running)
	CString sAutoSaveDir = m_pDoc->m_sRecordAutoSaveDir;
	sAutoSaveDir.TrimRight(_T('\\'));

	// Overwrite web files in given directory
	m_pDoc->MicroApacheUpdateWebFiles(sAutoSaveDir);

	// Get parent directory
	m_sParentDir = sAutoSaveDir;
	int index;
	if ((index = m_sParentDir.ReverseFind(_T('\\'))) >= 0)
		m_sParentDir = m_sParentDir.Left(index);

	// Init Combo Boxes
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_SNAPSHOTHISTORY_RATE);
	if (pComboBox)
	{
		pComboBox->AddString(ML_STRING(1733, "15 Seconds Rate"));
		pComboBox->AddString(ML_STRING(1734, "30 Seconds Rate"));
		pComboBox->AddString(ML_STRING(1735, "1 Minute Rate"));
		pComboBox->AddString(ML_STRING(1736, "2 Minutes Rate"));
		pComboBox->AddString(ML_STRING(1737, "3 Minutes Rate"));
		pComboBox->AddString(ML_STRING(1738, "4 Minutes Rate"));
		pComboBox->AddString(ML_STRING(1739, "5 Minutes Rate"));
	}
	pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_SNAPSHOTHISTORY_SIZE);
	if (pComboBox)
	{
		pComboBox->AddString(ML_STRING(1555, "Full"));
		pComboBox->AddString(_T("3/4"));
		pComboBox->AddString(_T("1/2"));
		pComboBox->AddString(_T("1/4"));
	}
	pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_SNAPSHOT_RATE);
	if (pComboBox)
	{
		pComboBox->AddString(ML_STRING(1863, "Fast Rate"));
		pComboBox->AddString(ML_STRING(1741, "1 Second Rate"));
		pComboBox->AddString(ML_STRING(1742, "2 Seconds Rate"));
		pComboBox->AddString(ML_STRING(1743, "3 Seconds Rate"));
		pComboBox->AddString(ML_STRING(1744, "4 Seconds Rate"));
		pComboBox->AddString(ML_STRING(1745, "5 Seconds Rate"));
		pComboBox->AddString(ML_STRING(1746, "10 Seconds Rate"));
		pComboBox->AddString(ML_STRING(1733, "15 Seconds Rate"));
		pComboBox->AddString(ML_STRING(1734, "30 Seconds Rate"));
		pComboBox->AddString(ML_STRING(1735, "1 Minute Rate"));
		pComboBox->AddString(ML_STRING(1736, "2 Minutes Rate"));
		pComboBox->AddString(ML_STRING(1737, "3 Minutes Rate"));
		pComboBox->AddString(ML_STRING(1738, "4 Minutes Rate"));
		pComboBox->AddString(ML_STRING(1739, "5 Minutes Rate"));
	}
	pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_KEEPFOR);
	if (pComboBox)
	{
		pComboBox->AddString(ML_STRING(1721, "1 Day"));
		pComboBox->AddString(ML_STRING(1722, "2 Days"));
		pComboBox->AddString(ML_STRING(1723, "3 Days"));
		pComboBox->AddString(ML_STRING(1724, "1 Week"));
		pComboBox->AddString(ML_STRING(1725, "2 Weeks"));
		pComboBox->AddString(ML_STRING(1726, "3 Weeks"));
		pComboBox->AddString(ML_STRING(1727, "1 Month"));
		pComboBox->AddString(ML_STRING(1728, "2 Month"));
		pComboBox->AddString(ML_STRING(1729, "3 Month"));
		pComboBox->AddString(ML_STRING(1730, "Half Year"));
		pComboBox->AddString(ML_STRING(1731, "1 Year"));
		pComboBox->AddString(ML_STRING(1732, "Unlimited"));
	}
	pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_THUMBSPERPAGE);
	if (pComboBox)
	{
		TCHAR sNum[34];
		for (int i = PHPCONFIG_MIN_THUMSPERPAGE ; i <= PHPCONFIG_MAX_THUMSPERPAGE ; i++)
		{
			_itot(i, sNum, 10);
			pComboBox->AddString(sNum);
		}
	}
	if (sAutoSaveDir != _T(""))
	{
		pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_LANGUAGE);
		if (pComboBox)
		{
			CSortableFileFind FileFind;
			FileFind.AddAllowedExtension(_T("php"));
			if (FileFind.Init(sAutoSaveDir + _T("\\") + CString(MICROAPACHE_LANGUAGES_DIR) + _T("\\") + _T("*")))
			{
				for (int pos = 0 ; pos < FileFind.GetFilesCount() ; pos++)
					pComboBox->AddString(::GetShortFileNameNoExt(FileFind.GetFileName(pos)));
			}
		}
		pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_STYLE);
		if (pComboBox)
		{
			CSortableFileFind FileFind;
			FileFind.AddAllowedExtension(_T("css"));
			if (FileFind.Init(sAutoSaveDir + _T("\\") + CString(MICROAPACHE_STYLE_DIR) + _T("\\") + _T("*")))
			{
				for (int pos = 0 ; pos < FileFind.GetFilesCount() ; pos++)
					pComboBox->AddString(::GetShortFileNameNoExt(FileFind.GetFileName(pos)));
			}
		}
	}

	// Init vars
	m_bDoApplySettings = FALSE;
	m_nRetryTimeMs = 0;
	m_bCheck24hRec = Is24hRec();
	m_bCheckFullStretch = FALSE;
	m_sName = m_pDoc->GetAssignedDeviceName();
	CString sInitDefaultPage = m_pDoc->PhpConfigFileGetParam(PHPCONFIG_DEFAULTPAGE);
	m_bCheckPrintCommand = (m_pDoc->PhpConfigFileGetParam(PHPCONFIG_SHOW_PRINTCOMMAND) == _T("1"));
	m_bCheckSaveCommand = (m_pDoc->PhpConfigFileGetParam(PHPCONFIG_SHOW_SAVECOMMAND) == _T("1"));
	if (sInitDefaultPage.CompareNoCase(PHPCONFIG_SUMMARYSNAPSHOT_PHP) == 0)
		m_nUsage = 0;
	else if (sInitDefaultPage.CompareNoCase(PHPCONFIG_SNAPSHOTHISTORY_PHP) == 0)
		m_nUsage = 1;
	else if (sInitDefaultPage.CompareNoCase(PHPCONFIG_SNAPSHOT_PHP) == 0)
	{
		m_nUsage = 2;
		m_bCheckFullStretch = FALSE;
	}
	else if (sInitDefaultPage.CompareNoCase(PHPCONFIG_SNAPSHOTFULL_PHP) == 0)
	{
		m_nUsage = 2;
		m_bCheckFullStretch = TRUE;
	}
	else if (sInitDefaultPage.CompareNoCase(PHPCONFIG_SUMMARYIFRAME_PHP) == 0)
		m_nUsage = 3;
	else
		m_nUsage = 4;
	if (m_pDoc->m_nDeleteRecordingsOlderThanDays > 366)
		m_nComboKeepFor = 11;	// Infinite
	else if (m_pDoc->m_nDeleteRecordingsOlderThanDays > 183)
		m_nComboKeepFor = 10;	// One Year
	else if (m_pDoc->m_nDeleteRecordingsOlderThanDays > 93)
		m_nComboKeepFor = 9;	// Half Year
	else if (m_pDoc->m_nDeleteRecordingsOlderThanDays > 62)
		m_nComboKeepFor = 8;	// 3 Month
	else if (m_pDoc->m_nDeleteRecordingsOlderThanDays > 31)
		m_nComboKeepFor = 7;	// 2 Month
	else if (m_pDoc->m_nDeleteRecordingsOlderThanDays > 21)
		m_nComboKeepFor = 6;	// 1 Month
	else if (m_pDoc->m_nDeleteRecordingsOlderThanDays > 14)
		m_nComboKeepFor = 5;	// 3 Weeks
	else if (m_pDoc->m_nDeleteRecordingsOlderThanDays > 7)
		m_nComboKeepFor = 4;	// 2 Weeks
	else if (m_pDoc->m_nDeleteRecordingsOlderThanDays > 3)
		m_nComboKeepFor = 3;	// 1 Week
	else if (m_pDoc->m_nDeleteRecordingsOlderThanDays == 3)
		m_nComboKeepFor = 2;	// 3 Days
	else if (m_pDoc->m_nDeleteRecordingsOlderThanDays == 2)
		m_nComboKeepFor = 1;	// 2 Days
	else if (m_pDoc->m_nDeleteRecordingsOlderThanDays == 1)
		m_nComboKeepFor = 0;	// 1 Day
	else
		m_nComboKeepFor = 11;	// Infinite
	if (m_pDoc->m_nSnapshotRate > 240)
		m_nComboSnapshotHistoryRate = 6;	// 5 Minutes
	else if (m_pDoc->m_nSnapshotRate > 180)
		m_nComboSnapshotHistoryRate = 5;	// 4 Minutes
	else if (m_pDoc->m_nSnapshotRate > 120)
		m_nComboSnapshotHistoryRate = 4;	// 3 Minutes
	else if (m_pDoc->m_nSnapshotRate > 60)
		m_nComboSnapshotHistoryRate = 3;	// 2 Minutes
	else if (m_pDoc->m_nSnapshotRate > 30)
		m_nComboSnapshotHistoryRate = 2;	// 1 Minute
	else if (m_pDoc->m_nSnapshotRate > 15)
		m_nComboSnapshotHistoryRate = 1;	// 30 Seconds
	else
		m_nComboSnapshotHistoryRate = 0;	// 15 Seconds
	if (!m_pDoc->m_bSnapshotThumb)
		m_nComboSnapshotHistorySize = 0;	// Full Size
	else
	{
		int nSizeRatio = 75;
		if (m_pDoc->m_DocRect.right > 0)
			nSizeRatio = 100 * m_pDoc->m_nSnapshotThumbWidth / m_pDoc->m_DocRect.right;
		if (nSizeRatio > 75)
			m_nComboSnapshotHistorySize = 0;// Full Size
		else if (nSizeRatio > 50)
			m_nComboSnapshotHistorySize = 1;// 3 / 4 Size
		else if (nSizeRatio > 25)
			m_nComboSnapshotHistorySize = 2;// 1 / 2 Size
		else
			m_nComboSnapshotHistorySize = 3;// 1 / 4 Size
	}
	if (m_pDoc->m_nSnapshotRate > 240)
		m_nComboSnapshotRate = 13;			// 5 Minutes
	else if (m_pDoc->m_nSnapshotRate > 180)
		m_nComboSnapshotRate = 12;			// 4 Minutes
	else if (m_pDoc->m_nSnapshotRate > 120)
		m_nComboSnapshotRate = 11;			// 3 Minutes
	else if (m_pDoc->m_nSnapshotRate > 60)
		m_nComboSnapshotRate = 10;			// 2 Minutes
	else if (m_pDoc->m_nSnapshotRate > 30)
		m_nComboSnapshotRate = 9;			// 1 Minute
	else if (m_pDoc->m_nSnapshotRate > 15)
		m_nComboSnapshotRate = 8;			// 30 Seconds
	else if (m_pDoc->m_nSnapshotRate > 10)
		m_nComboSnapshotRate = 7;			// 15 Seconds
	else if (m_pDoc->m_nSnapshotRate > 5)
		m_nComboSnapshotRate = 6;			// 10 Seconds
	else if (m_pDoc->m_nSnapshotRate > 4)
		m_nComboSnapshotRate = 5;			// 5 Seconds
	else if (m_pDoc->m_nSnapshotRate > 3)
		m_nComboSnapshotRate = 4;			// 4 Seconds
	else if (m_pDoc->m_nSnapshotRate > 2)
		m_nComboSnapshotRate = 3;			// 3 Seconds
	else if (m_pDoc->m_nSnapshotRate > 1)
		m_nComboSnapshotRate = 2;			// 2 Seconds
	else if (m_pDoc->m_nSnapshotRate > 0)
		m_nComboSnapshotRate = 1;			// 1 Second
	else
		m_nComboSnapshotRate = 0;			// Fast Rate
	CString sMaxPerPage = m_pDoc->PhpConfigFileGetParam(PHPCONFIG_MAX_PER_PAGE);
	int nMaxPerPage = PHPCONFIG_DEFAULT_THUMSPERPAGE;
	if (sMaxPerPage != _T(""))
		nMaxPerPage = MAX(PHPCONFIG_MIN_THUMSPERPAGE, MIN(PHPCONFIG_MAX_THUMSPERPAGE, _ttoi(sMaxPerPage)));
	pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_THUMBSPERPAGE);
	if (pComboBox)
		pComboBox->SetCurSel(nMaxPerPage - PHPCONFIG_MIN_THUMSPERPAGE);
	CString sLanguageFilePath = m_pDoc->PhpConfigFileGetParam(PHPCONFIG_LANGUAGEFILEPATH);
	pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_LANGUAGE);
	if (pComboBox)
	{
		if (pComboBox->SelectString(-1, ::GetShortFileNameNoExt(sLanguageFilePath)) == CB_ERR)
			pComboBox->SetCurSel(0);
	}
	CString sStyleFilePath = m_pDoc->PhpConfigFileGetParam(PHPCONFIG_STYLEFILEPATH);
	pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_STYLE);
	if (pComboBox)
	{
		if (pComboBox->SelectString(-1, ::GetShortFileNameNoExt(sStyleFilePath)) == CB_ERR)
			pComboBox->SetCurSel(0);
	}

	// This calls UpdateData(FALSE) -> vars to view
	CDialog::OnInitDialog();

	// Enable / Disable Controls
	EnableDisableCtrls();
	
	// Set Timer
	SetTimer(ID_TIMER_ASSISTANTDLG, ASSISTANTDLG_TIMER_MS, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAssistantDlg::OnDestroy() 
{
	// Kill timer
	KillTimer(ID_TIMER_ASSISTANTDLG);

	// Base class
	CDialog::OnDestroy();
}

void CAssistantDlg::EnableDisableCtrls()
{
	CButton* pCheckMovDet = (CButton*)GetDlgItem(IDC_RADIO_MOVDET);
	CButton* pCheckSnapshotHistory = (CButton*)GetDlgItem(IDC_RADIO_SNAPSHOTHISTORY);
	CButton* pCheckSnapshot = (CButton*)GetDlgItem(IDC_RADIO_SNAPSHOT);
	if (pCheckMovDet->GetCheck())
	{
		CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_SNAPSHOTHISTORY_RATE);
		pComboBox->EnableWindow(FALSE);
		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_LABEL_SNAPSHOTHISTORY_SIZE);
		pEdit->EnableWindow(FALSE);
		pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_SNAPSHOTHISTORY_SIZE);
		pComboBox->EnableWindow(FALSE);
		pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_SNAPSHOT_RATE);
		pComboBox->EnableWindow(FALSE);
		CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_FULLSTRETCH);
		pCheck->EnableWindow(FALSE);
		pCheck = (CButton*)GetDlgItem(IDC_CHECK_24H_REC);
		pCheck->EnableWindow(TRUE);
		pEdit = (CEdit*)GetDlgItem(IDC_LABEL_PLUS_SIGN);
		pEdit->EnableWindow(TRUE);
		pEdit = (CEdit*)GetDlgItem(IDC_LABEL_THUMBSPERPAGE);
		pEdit->EnableWindow(TRUE);
		pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_THUMBSPERPAGE);
		pComboBox->EnableWindow(TRUE);
	}
	else if (pCheckSnapshotHistory->GetCheck())
	{
		CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_SNAPSHOTHISTORY_RATE);
		pComboBox->EnableWindow(TRUE);
		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_LABEL_SNAPSHOTHISTORY_SIZE);
		pEdit->EnableWindow(TRUE);
		pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_SNAPSHOTHISTORY_SIZE);
		pComboBox->EnableWindow(TRUE);
		pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_SNAPSHOT_RATE);
		pComboBox->EnableWindow(FALSE);
		CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_FULLSTRETCH);
		pCheck->EnableWindow(FALSE);
		pCheck = (CButton*)GetDlgItem(IDC_CHECK_24H_REC);
		pCheck->EnableWindow(FALSE);
		pEdit = (CEdit*)GetDlgItem(IDC_LABEL_PLUS_SIGN);
		pEdit->EnableWindow(FALSE);
		pEdit = (CEdit*)GetDlgItem(IDC_LABEL_THUMBSPERPAGE);
		pEdit->EnableWindow(FALSE);
		pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_THUMBSPERPAGE);
		pComboBox->EnableWindow(FALSE);
	}
	else if (pCheckSnapshot->GetCheck())
	{
		CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_SNAPSHOTHISTORY_RATE);
		pComboBox->EnableWindow(FALSE);
		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_LABEL_SNAPSHOTHISTORY_SIZE);
		pEdit->EnableWindow(FALSE);
		pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_SNAPSHOTHISTORY_SIZE);
		pComboBox->EnableWindow(FALSE);
		pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_SNAPSHOT_RATE);
		pComboBox->EnableWindow(TRUE);
		CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_FULLSTRETCH);
		pCheck->EnableWindow(TRUE);
		pCheck = (CButton*)GetDlgItem(IDC_CHECK_24H_REC);
		pCheck->EnableWindow(FALSE);
		pEdit = (CEdit*)GetDlgItem(IDC_LABEL_PLUS_SIGN);
		pEdit->EnableWindow(FALSE);
		pEdit = (CEdit*)GetDlgItem(IDC_LABEL_THUMBSPERPAGE);
		pEdit->EnableWindow(FALSE);
		pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_THUMBSPERPAGE);
		pComboBox->EnableWindow(FALSE);
	}
	else
	{
		CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_SNAPSHOTHISTORY_RATE);
		pComboBox->EnableWindow(FALSE);
		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_LABEL_SNAPSHOTHISTORY_SIZE);
		pEdit->EnableWindow(FALSE);
		pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_SNAPSHOTHISTORY_SIZE);
		pComboBox->EnableWindow(FALSE);
		pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_SNAPSHOT_RATE);
		pComboBox->EnableWindow(FALSE);
		CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_FULLSTRETCH);
		pCheck->EnableWindow(FALSE);
		pCheck = (CButton*)GetDlgItem(IDC_CHECK_24H_REC);
		pCheck->EnableWindow(FALSE);
		pEdit = (CEdit*)GetDlgItem(IDC_LABEL_PLUS_SIGN);
		pEdit->EnableWindow(FALSE);
		pEdit = (CEdit*)GetDlgItem(IDC_LABEL_THUMBSPERPAGE);
		pEdit->EnableWindow(FALSE);
		pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_THUMBSPERPAGE);
		pComboBox->EnableWindow(FALSE);
	}
}

void CAssistantDlg::EnableDisableAllCtrls(BOOL bEnable)
{
	if (bEnable)
		EnableDisableCtrls();
	else
	{
		CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_SNAPSHOTHISTORY_RATE);
		pComboBox->EnableWindow(FALSE);
		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_LABEL_SNAPSHOTHISTORY_SIZE);
		pEdit->EnableWindow(FALSE);
		pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_SNAPSHOTHISTORY_SIZE);
		pComboBox->EnableWindow(FALSE);
		pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_SNAPSHOT_RATE);
		pComboBox->EnableWindow(FALSE);
		CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_FULLSTRETCH);
		pCheck->EnableWindow(FALSE);
		pCheck = (CButton*)GetDlgItem(IDC_CHECK_24H_REC);
		pCheck->EnableWindow(FALSE);
		pEdit = (CEdit*)GetDlgItem(IDC_LABEL_PLUS_SIGN);
		pEdit->EnableWindow(FALSE);
		pEdit = (CEdit*)GetDlgItem(IDC_LABEL_THUMBSPERPAGE);
		pEdit->EnableWindow(FALSE);
		pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_THUMBSPERPAGE);
		pComboBox->EnableWindow(FALSE);
	}
	CButton* pCheck = (CButton*)GetDlgItem(IDC_RADIO_MOVDET);
	pCheck->EnableWindow(bEnable);
	pCheck = (CButton*)GetDlgItem(IDC_RADIO_SNAPSHOTHISTORY);
	pCheck->EnableWindow(bEnable);
	pCheck = (CButton*)GetDlgItem(IDC_RADIO_SNAPSHOT);
	pCheck->EnableWindow(bEnable);
	pCheck = (CButton*)GetDlgItem(IDC_RADIO_MANUAL);
	pCheck->EnableWindow(bEnable);
	pCheck = (CButton*)GetDlgItem(IDC_RADIO_NOCHANGE);
	pCheck->EnableWindow(bEnable);
	pCheck = (CButton*)GetDlgItem(IDC_CHECK_PRINTCOMMAND);
	pCheck->EnableWindow(bEnable);
	pCheck = (CButton*)GetDlgItem(IDC_CHECK_SAVECOMMAND);
	pCheck->EnableWindow(bEnable);
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_NAME);
	pEdit->EnableWindow(bEnable);
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_LANGUAGE);
	pComboBox->EnableWindow(bEnable);
	pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_STYLE);
	pComboBox->EnableWindow(bEnable);
	pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_KEEPFOR);
	pComboBox->EnableWindow(bEnable);
	CButton* pButton = (CButton*)GetDlgItem(IDOK);
	pButton->EnableWindow(bEnable);
	pButton = (CButton*)GetDlgItem(IDCANCEL);
	pButton->EnableWindow(bEnable);
}

void CAssistantDlg::OnRadioMovdet() 
{
	EnableDisableCtrls();
}

void CAssistantDlg::OnRadioSnapshothistory() 
{
	EnableDisableCtrls();
}

void CAssistantDlg::OnRadioSnapshot() 
{
	EnableDisableCtrls();
}

void CAssistantDlg::OnRadioManual() 
{
	EnableDisableCtrls();
}

void CAssistantDlg::OnRadioNochange() 
{
	EnableDisableCtrls();
}

void CAssistantDlg::EnableDisable24hRec(BOOL bEnable)
{
	if (m_pDoc->m_pGeneralPage)
	{
		CButton* pCheckOnce = (CButton*)m_pDoc->m_pGeneralPage->GetDlgItem(IDC_CHECK_SCHEDULER_ONCE);
		if (pCheckOnce->GetCheck())
			m_pDoc->m_pGeneralPage->SetCheckSchedulerOnce(FALSE);
		if (bEnable)
		{
			// Start Time
			CTime CurrentTime = CTime::GetCurrentTime();
			m_pDoc->m_pGeneralPage->m_SchedulerOnceDateStart = CTime(	CurrentTime.GetYear(),
																		CurrentTime.GetMonth(),
																		CurrentTime.GetDay(),
																		12, 0, 0);
			m_pDoc->m_pGeneralPage->m_SchedulerOnceTimeStart = CTime(	2000, 1, 1,
																		CurrentTime.GetHour(),
																		CurrentTime.GetMinute(),
																		CurrentTime.GetSecond());

			// Stop Time
			CTime MaxTime(3000, 12, 31, 23, 59, 59);
			m_pDoc->m_pGeneralPage->m_SchedulerOnceDateStop = CTime(	MaxTime.GetYear(),
																		MaxTime.GetMonth(),
																		MaxTime.GetDay(),
																		12, 0, 0);
			m_pDoc->m_pGeneralPage->m_SchedulerOnceTimeStop = CTime(	2000, 1, 1,
																		MaxTime.GetHour(),
																		MaxTime.GetMinute(),
																		MaxTime.GetSecond());

			// 6 Hours Segmentation
			m_pDoc->m_bRecTimeSegmentation = m_pDoc->m_pGeneralPage->m_bRecTimeSegmentation = TRUE;
			m_pDoc->m_nTimeSegmentationIndex = m_pDoc->m_pGeneralPage->m_nTimeSegmentationIndex = 3;
			m_pDoc->m_bRecAutoOpen = m_pDoc->m_pGeneralPage->m_bRecAutoOpen = FALSE;

			// Update Data from vars to view
			m_pDoc->m_pGeneralPage->UpdateData(FALSE);

			// Enable Scheduler
			m_pDoc->m_pGeneralPage->SetCheckSchedulerOnce(TRUE);
		}
		else
		{
			// No Segmentation
			m_pDoc->m_bRecTimeSegmentation = m_pDoc->m_pGeneralPage->m_bRecTimeSegmentation = FALSE;
			m_pDoc->m_nTimeSegmentationIndex = m_pDoc->m_pGeneralPage->m_nTimeSegmentationIndex = 0;
			m_pDoc->m_bRecAutoOpen = m_pDoc->m_pGeneralPage->m_bRecAutoOpen = TRUE;

			// Update Data from vars to view
			m_pDoc->m_pGeneralPage->UpdateData(FALSE);
		}
	}
	else
	{
		CUImagerApp::CSchedulerEntry* pOnceSchedulerEntry =
			((CUImagerApp*)::AfxGetApp())->GetOnceSchedulerEntry(m_pDoc->GetDevicePathName());
		if (pOnceSchedulerEntry)
			((CUImagerApp*)::AfxGetApp())->DeleteOnceSchedulerEntry(m_pDoc->GetDevicePathName());
		if (bEnable)
		{
			// Scheduler Entry Init
			CUImagerApp::CSchedulerEntry* pSchedulerEntry = new CUImagerApp::CSchedulerEntry;
			if (!pSchedulerEntry)
				return;
			pSchedulerEntry->m_Type = CUImagerApp::CSchedulerEntry::ONCE;
			pSchedulerEntry->m_sDevicePathName = m_pDoc->GetDevicePathName();

			// Start Time
			pSchedulerEntry->m_StartTime = CTime::GetCurrentTime();

			// Stop Time
			pSchedulerEntry->m_StopTime = CTime(3000, 12, 31, 23, 59, 59);
			
			// 6 Hours Segmentation
			m_pDoc->m_bRecTimeSegmentation = TRUE;
			m_pDoc->m_nTimeSegmentationIndex = 3;
			m_pDoc->m_bRecAutoOpen = FALSE;

			// Add Scheduler Entry
			((CUImagerApp*)::AfxGetApp())->AddSchedulerEntry(pSchedulerEntry);
		}
		else
		{
			// No Segmentation
			m_pDoc->m_bRecTimeSegmentation = FALSE;
			m_pDoc->m_nTimeSegmentationIndex = 0;
			m_pDoc->m_bRecAutoOpen = TRUE;
		}
	}
}

BOOL CAssistantDlg::Is24hRec() 
{
	CUImagerApp::CSchedulerEntry* pOnceSchedulerEntry =
		((CUImagerApp*)::AfxGetApp())->GetOnceSchedulerEntry(m_pDoc->GetDevicePathName());
	if (pOnceSchedulerEntry											&&
		pOnceSchedulerEntry->m_StartTime <= CTime::GetCurrentTime()	&&
		pOnceSchedulerEntry->m_StopTime == CTime(3000, 12, 31, 23, 59, 59))
		return TRUE;
	else
		return FALSE;
}

void CAssistantDlg::OnTimer(UINT nIDEvent) 
{
	if (m_bDoApplySettings)
	{
		// Stop Rec
		if (m_pDoc->m_pAVRec)
			m_pDoc->CaptureRecord();
		else
		{
			// Apply settings if we are not inside the processing function
			m_pDoc->StopProcessFrame(PROCESSFRAME_ASSISTANT);
			if (m_nRetryTimeMs > PROCESSFRAME_MAX_RETRY_TIME || m_pDoc->IsProcessFrameStopped(PROCESSFRAME_ASSISTANT))
				ApplySettings();
			else
				m_nRetryTimeMs += ASSISTANTDLG_TIMER_MS;
		}
	}
	CDialog::OnTimer(nIDEvent);
}

BOOL CAssistantDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	// If Wait Cursor leave it!
	if (((CUImagerApp*)::AfxGetApp())->IsWaitCursor())
	{
		RestoreWaitCursor();
		return TRUE;
	}
	else
		return CDialog::OnSetCursor(pWnd, nHitTest, message);
}

void CAssistantDlg::OnOK() 
{
	// Begin wait cursor
	BeginWaitCursor();

	// Set vars
	m_bDoApplySettings = TRUE;
	m_nRetryTimeMs = 0;

	// Disable all
	EnableDisableAllCtrls(FALSE);
}

void CAssistantDlg::OnButtonParentDir() 
{
	CBrowseDlg dlg(	::AfxGetMainFrame(),
					&m_sParentDir,
					ML_STRING(1869, "Move camera folder to selected directory. ATTENTION: all camera folders have to be moved to this directory!"),
					TRUE);
	dlg.DoModal();
}

void CAssistantDlg::Rename()
{
	// Store current name
	CString sOldName = m_pDoc->GetAssignedDeviceName();

	// Is ANSI?
	if (!::IsANSIConvertible(m_sName))
	{
		// Error Message
		EndWaitCursor();
		::AfxMessageBox(ML_STRING(1767, "Only the ANSI character set is supported for the camera name"), MB_OK | MB_ICONERROR);
		BeginWaitCursor();
		
		// Restore old name
		m_sName = sOldName;
		return;
	}

	// Adjust new name
	m_sName = CVideoDeviceDoc::GetValidName(m_sName);
	m_sName.TrimLeft();
	m_sName.TrimRight();
	if (m_sName == _T(""))
		m_sName = m_pDoc->GetDeviceName();
	
	// Adjust old dir
	m_pDoc->m_sRecordAutoSaveDir.TrimRight(_T('\\'));

	// Make new dir
	CString sNewRecordAutoSaveDir = m_sParentDir;
	sNewRecordAutoSaveDir.TrimRight(_T('\\'));
	sNewRecordAutoSaveDir += _T('\\') + m_sName;
	
	// Fail if sNewRecordAutoSaveDir is a nested subdir of the old one
	if (::IsSubDir(m_pDoc->m_sRecordAutoSaveDir, sNewRecordAutoSaveDir))
	{
		// Error Message
		EndWaitCursor();
		::AfxMessageBox(ML_STRING(1870, "The new folder cannot be a subfolder of the old one"), MB_OK | MB_ICONERROR);
		BeginWaitCursor();

		// Restore old name
		m_sName = sOldName;
		return;
	}

	// Prompt for merging
	if (::IsExistingDir(m_pDoc->m_sRecordAutoSaveDir)	&&
		::IsExistingDir(sNewRecordAutoSaveDir)			&&
		sNewRecordAutoSaveDir.CompareNoCase(m_pDoc->m_sRecordAutoSaveDir) != 0)
	{
		CString sMsg;
		sMsg.Format(ML_STRING(1765, "%s already exists.\nDo you want to proceed and merge the files?"), m_sName);
		EndWaitCursor();
		if (::AfxMessageBox(sMsg, MB_YESNO | MB_ICONQUESTION) == IDNO)
		{
			// Restore cursor
			BeginWaitCursor();

			// Restore old name
			m_sName = sOldName;
			return;
		}
		else
			BeginWaitCursor();
	}

	// Error code
	DWORD dwLastError = ERROR_SUCCESS;

	// Merge or move
	if (::IsExistingDir(m_pDoc->m_sRecordAutoSaveDir) &&
		sNewRecordAutoSaveDir.CompareNoCase(m_pDoc->m_sRecordAutoSaveDir) != 0)
	{
		if (::IsExistingDir(sNewRecordAutoSaveDir) || !::MoveFile(m_pDoc->m_sRecordAutoSaveDir, sNewRecordAutoSaveDir))
		{
			if (!::MergeDirContent(m_pDoc->m_sRecordAutoSaveDir, sNewRecordAutoSaveDir)) // overwrite existing
			{
				dwLastError = ::GetLastError();
				sNewRecordAutoSaveDir = m_pDoc->m_sRecordAutoSaveDir;
			}
			else
				::DeleteDir(m_pDoc->m_sRecordAutoSaveDir); // No error message on failure
		}
	}
	// Rename if just case changed
	else if (::IsExistingDir(m_pDoc->m_sRecordAutoSaveDir)							&&
			sNewRecordAutoSaveDir.CompareNoCase(m_pDoc->m_sRecordAutoSaveDir) == 0	&&	
			sNewRecordAutoSaveDir.Compare(m_pDoc->m_sRecordAutoSaveDir) != 0)
	{
		if (!::MoveFile(m_pDoc->m_sRecordAutoSaveDir, sNewRecordAutoSaveDir))
		{
			dwLastError = ::GetLastError();
			sNewRecordAutoSaveDir = m_pDoc->m_sRecordAutoSaveDir;
		}
	}
	// Create if nothing exists
	else if (!::IsExistingDir(sNewRecordAutoSaveDir) && !::CreateDir(sNewRecordAutoSaveDir))
	{
		dwLastError = ::GetLastError();
		sNewRecordAutoSaveDir = m_pDoc->m_sRecordAutoSaveDir;
	}
	m_pDoc->m_sRecordAutoSaveDir = sNewRecordAutoSaveDir;

	// On Error
	if (dwLastError != ERROR_SUCCESS)
	{
		// Error Message
		EndWaitCursor();
		::ShowError(dwLastError, TRUE);
		BeginWaitCursor();

		// Restore old name
		m_sName = sOldName;
	}
	// Update Doc Root
	else if (m_sParentDir != ((CUImagerApp*)::AfxGetApp())->m_sMicroApacheDocRoot)
	{
		((CUImagerApp*)::AfxGetApp())->m_sMicroApacheDocRoot = m_sParentDir;
		int nRet = CVideoDeviceDoc::MicroApacheReload();
		if (nRet <= 0)
		{
			EndWaitCursor();
			if (nRet == 0)
				::AfxMessageBox(ML_STRING(1474, "Failed to stop the web server"), MB_ICONSTOP);
			else
				::AfxMessageBox(ML_STRING(1475, "Failed to start the web server"), MB_ICONSTOP);
			BeginWaitCursor();
		}
		::AfxGetApp()->WriteProfileString(	_T("GeneralApp"),
											_T("MicroApacheDocRoot"),
											((CUImagerApp*)::AfxGetApp())->m_sMicroApacheDocRoot);
	}
}

void CAssistantDlg::ApplySettingsSnapshot(int nThumbWidth, int nThumbHeight, double dSnapshotRate)
{
	m_pDoc->SnapshotRate(dSnapshotRate);
	if (m_pDoc->m_pSnapshotPage)
	{
		// Thumb size (this updates the controls and sets m_nSnapshotThumbWidth and m_nSnapshotThumbHeight)
		m_pDoc->m_pSnapshotPage->ChangeThumbSize(nThumbWidth, nThumbHeight);
		
		// Live snapshots
		CButton* pCheck = (CButton*)m_pDoc->m_pSnapshotPage->GetDlgItem(IDC_CHECK_SNAPSHOT_LIVE_JPEG);
		pCheck->SetCheck(m_pDoc->m_bSnapshotLiveJpeg ? 1 : 0);
		pCheck = (CButton*)m_pDoc->m_pSnapshotPage->GetDlgItem(IDC_CHECK_SNAPSHOT_THUMB);
		pCheck->SetCheck(m_pDoc->m_bSnapshotThumb ? 1 : 0);
		m_pDoc->m_pSnapshotPage->DisplaySnapshotRate();

		// Snapshot history
		pCheck = (CButton*)m_pDoc->m_pSnapshotPage->GetDlgItem(IDC_CHECK_SNAPSHOT_HISTORY_SWF);
		pCheck->SetCheck(m_pDoc->m_bSnapshotHistorySwf ? 1 : 0);
	}
	else
	{
		// Thumb size
		m_pDoc->m_nSnapshotThumbWidth = nThumbWidth;
		m_pDoc->m_nSnapshotThumbHeight = nThumbHeight;
	}
}

void CAssistantDlg::ApplySettings()
{
	// Reset vars
	m_bDoApplySettings = FALSE;
	m_nRetryTimeMs = 0;

	// Update data -> view to vars
	UpdateData(TRUE);

	// Disable mov. det.
	BOOL bDoMovDet;
	if (m_pDoc->m_dwVideoProcessorMode & SOFTWARE_MOVEMENT_DETECTOR)
	{
		bDoMovDet = TRUE;
		m_pDoc->m_dwVideoProcessorMode &= ~SOFTWARE_MOVEMENT_DETECTOR;
	}
	else
		bDoMovDet = FALSE;
	m_pDoc->m_SaveFrameListThread.Kill();
	m_pDoc->OneEmptyFrameList();
	m_pDoc->FreeMovementDetector();

	// Is 24h rec.?
	BOOL bDo24hRec = Is24hRec();

	// Stop the delete thread
	m_pDoc->m_DeleteThread.Kill();

	// Make sure snapshot threads are stopped
	// (at this point the process frame is stopped but
	// the snapshot threads may still be running)
	m_pDoc->m_SaveSnapshotThread.Kill();
	m_pDoc->m_SaveSnapshotSWFThread.Kill();

	// Rename
	Rename();

	// Title
	m_pDoc->PhpConfigFileSetParam(PHPCONFIG_SUMMARYTITLE, m_sName);
	m_pDoc->PhpConfigFileSetParam(PHPCONFIG_SNAPSHOTTITLE, m_sName);

	// Language
	CString sLanguageName;
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_LANGUAGE);
	if (pComboBox && pComboBox->GetCurSel() >= 0)
		pComboBox->GetLBText(pComboBox->GetCurSel(), sLanguageName);
	if (sLanguageName != _T(""))
		m_pDoc->PhpConfigFileSetParam(PHPCONFIG_LANGUAGEFILEPATH, CString(MICROAPACHE_LANGUAGES_DIR) + _T("/") + sLanguageName + _T(".php"));

	// Style
	CString sStyleName;
	pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_STYLE);
	if (pComboBox && pComboBox->GetCurSel() >= 0)
		pComboBox->GetLBText(pComboBox->GetCurSel(), sStyleName);
	if (sStyleName != _T(""))
		m_pDoc->PhpConfigFileSetParam(PHPCONFIG_STYLEFILEPATH, CString(MICROAPACHE_STYLE_DIR) + _T("/") + sStyleName + _T(".css"));

	// Print command
	if (m_bCheckPrintCommand)
		m_pDoc->PhpConfigFileSetParam(PHPCONFIG_SHOW_PRINTCOMMAND, _T("1"));
	else
		m_pDoc->PhpConfigFileSetParam(PHPCONFIG_SHOW_PRINTCOMMAND, _T("0"));

	// Save command
	if (m_bCheckSaveCommand)
		m_pDoc->PhpConfigFileSetParam(PHPCONFIG_SHOW_SAVECOMMAND, _T("1"));
	else
		m_pDoc->PhpConfigFileSetParam(PHPCONFIG_SHOW_SAVECOMMAND, _T("0"));

	// Snapshot file names
	m_pDoc->PhpConfigFileSetParam(PHPCONFIG_SNAPSHOTNAME, m_pDoc->m_sSnapshotLiveJpegName + _T(".jpg"));
	m_pDoc->PhpConfigFileSetParam(PHPCONFIG_SNAPSHOTTHUMBNAME, m_pDoc->m_sSnapshotLiveJpegThumbName + _T(".jpg"));

	// Usage
	switch (m_nUsage)
	{
		case 0 :
		{
			// Set enable mov. det. flag and set/reset 24h rec. flag
			bDoMovDet = TRUE;
			bDo24hRec = m_bCheck24hRec;
			
			// Init size vars
			CString sWidth, sHeight;
			sWidth.Format(_T("%d"), m_pDoc->m_DocRect.right);
			sHeight.Format(_T("%d"), m_pDoc->m_DocRect.bottom);

			// Init thumb vars
			int nThumbWidth =	(m_pDoc->m_nSnapshotThumbWidth < 4 * DEFAULT_SNAPSHOT_THUMB_WIDTH / 3 &&
								m_pDoc->m_nSnapshotThumbWidth > 2 * DEFAULT_SNAPSHOT_THUMB_WIDTH / 3) ?
								m_pDoc->m_nSnapshotThumbWidth : DEFAULT_SNAPSHOT_THUMB_WIDTH;
			int nThumbHeight =	(m_pDoc->m_nSnapshotThumbHeight < 4 * DEFAULT_SNAPSHOT_THUMB_HEIGHT / 3 &&
								m_pDoc->m_nSnapshotThumbHeight > 2 * DEFAULT_SNAPSHOT_THUMB_HEIGHT / 3) ?
								m_pDoc->m_nSnapshotThumbHeight : DEFAULT_SNAPSHOT_THUMB_HEIGHT;
			CString sThumbWidth, sThumbHeight;
			sThumbWidth.Format(_T("%d"), nThumbWidth);
			sThumbHeight.Format(_T("%d"), nThumbHeight);
			CString sMaxPerPage;
			pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_THUMBSPERPAGE);
			if (pComboBox && pComboBox->GetCurSel() >= 0)
				pComboBox->GetLBText(pComboBox->GetCurSel(), sMaxPerPage);

			// Update configuration.php
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_DEFAULTPAGE, PHPCONFIG_SUMMARYSNAPSHOT_PHP);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_WIDTH, sWidth);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_HEIGHT, sHeight);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_THUMBWIDTH, sThumbWidth);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_THUMBHEIGHT, sThumbHeight);
			if (sMaxPerPage != _T(""))
				m_pDoc->PhpConfigFileSetParam(PHPCONFIG_MAX_PER_PAGE, sMaxPerPage);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_SNAPSHOTHISTORY_THUMB, _T("0"));

			// Enable live snapshots
			m_pDoc->m_bSnapshotLiveJpeg = TRUE;
			m_pDoc->m_bSnapshotThumb = TRUE;

			// Disable snapshot history
			m_pDoc->m_bSnapshotHistorySwf = FALSE;

			// Update snapshot settings
			ApplySettingsSnapshot(nThumbWidth, nThumbHeight, MIN_SNAPSHOT_RATE);

			break;
		}
		case 1 :
		{
			// Reset mov. det. and 24h rec. flags
			bDoMovDet = FALSE;
			bDo24hRec = FALSE;

			// Init size vars
			CString sWidth, sHeight;
			sWidth.Format(_T("%d"), m_pDoc->m_DocRect.right);
			sHeight.Format(_T("%d"), m_pDoc->m_DocRect.bottom);

			// Init snapshot rate vars
			int nSnapshotRate;
			switch (m_nComboSnapshotHistoryRate)
			{
				case 0  : nSnapshotRate = 15;   break;	// 15 Seconds
				case 1  : nSnapshotRate = 30;   break;	// 30 Seconds
				case 2  : nSnapshotRate = 60;   break;	// 1 Minute
				case 3  : nSnapshotRate = 120;  break;	// 2 Minutes
				case 4  : nSnapshotRate = 180;  break;	// 3 Minutes
				case 5  : nSnapshotRate = 240;  break;	// 4 Minutes
				default : nSnapshotRate = 300;  break;	// 5 Minutes
			}

			// Init thumb vars, must be a multiple of 4 because of swf
			BOOL bUseThumb = TRUE;
			int nThumbWidth = m_pDoc->m_DocRect.right & ~0x3;
			int nThumbHeight = m_pDoc->m_DocRect.bottom & ~0x3;
			switch (m_nComboSnapshotHistorySize)
			{
				// Full Size
				case 0	:	bUseThumb = FALSE;
							break;
				// 3 / 4 Size
				case 1	:	nThumbWidth = (3 * nThumbWidth / 4) & ~0x3;
							nThumbHeight = (3 * nThumbHeight / 4) & ~0x3;
							break;
				// 1 / 2 Size
				case 2	:	nThumbWidth = (nThumbWidth / 2) & ~0x3;
							nThumbHeight = (nThumbHeight / 2) & ~0x3;
							break;
				// 1 / 4 Size
				default	:	nThumbWidth = (nThumbWidth / 4) & ~0x3;
							nThumbHeight = (nThumbHeight / 4) & ~0x3;
							break;
			}
			CString sThumbWidth, sThumbHeight;
			sThumbWidth.Format(_T("%d"), nThumbWidth);
			sThumbHeight.Format(_T("%d"), nThumbHeight);

			// Update configuration.php
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_DEFAULTPAGE, PHPCONFIG_SNAPSHOTHISTORY_PHP);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_WIDTH, sWidth);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_HEIGHT, sHeight);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_THUMBWIDTH, sThumbWidth);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_THUMBHEIGHT, sThumbHeight);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_SNAPSHOTHISTORY_THUMB, bUseThumb ? _T("1") : _T("0"));

			// Enable live snapshots
			m_pDoc->m_bSnapshotLiveJpeg = TRUE;
			m_pDoc->m_bSnapshotThumb = TRUE;

			// Enable snapshot history
			m_pDoc->m_bSnapshotHistorySwf = TRUE;

			// Update snapshot settings
			ApplySettingsSnapshot(nThumbWidth, nThumbHeight, nSnapshotRate);

			break;
		}
		case 2 :
		{
			// Reset mov. det. and 24h rec. flags
			bDoMovDet = FALSE;
			bDo24hRec = FALSE;

			// Init size vars
			CString sWidth, sHeight;
			sWidth.Format(_T("%d"), m_pDoc->m_DocRect.right);
			sHeight.Format(_T("%d"), m_pDoc->m_DocRect.bottom);

			// Init snapshot rate vars
			double dSnapshotRate;
			switch (m_nComboSnapshotRate)
			{
				case 0  : dSnapshotRate = DEFAULT_SERVERPUSH_POLLRATE_MS / 1000.0;	break;	// Fast Rate
				case 1  : dSnapshotRate = 1.0;										break;	// 1 Second
				case 2  : dSnapshotRate = 2.0;										break;	// 2 Seconds
				case 3  : dSnapshotRate = 3.0;										break;	// 3 Seconds
				case 4  : dSnapshotRate = 4.0;										break;	// 4 Seconds
				case 5  : dSnapshotRate = 5.0;										break;	// 5 Seconds
				case 6  : dSnapshotRate = 10.0;										break;	// 10 Seconds
				case 7  : dSnapshotRate = 15.0;										break;	// 15 Seconds
				case 8  : dSnapshotRate = 30.0;										break;	// 30 Seconds
				case 9  : dSnapshotRate = 60.0;										break;	// 1 Minute
				case 10 : dSnapshotRate = 120.0;									break;	// 2 Minutes
				case 11 : dSnapshotRate = 180.0;									break;	// 3 Minutes
				case 12 : dSnapshotRate = 240.0;									break;	// 4 Minutes
				default : dSnapshotRate = 300.0;									break;	// 5 Minutes
			}

			// Init thumb vars
			int nThumbWidth =	(m_pDoc->m_nSnapshotThumbWidth < 4 * DEFAULT_SNAPSHOT_THUMB_WIDTH / 3 &&
								m_pDoc->m_nSnapshotThumbWidth > 2 * DEFAULT_SNAPSHOT_THUMB_WIDTH / 3) ?
								m_pDoc->m_nSnapshotThumbWidth : DEFAULT_SNAPSHOT_THUMB_WIDTH;
			int nThumbHeight =	(m_pDoc->m_nSnapshotThumbHeight < 4 * DEFAULT_SNAPSHOT_THUMB_HEIGHT / 3 &&
								m_pDoc->m_nSnapshotThumbHeight > 2 * DEFAULT_SNAPSHOT_THUMB_HEIGHT / 3) ?
								m_pDoc->m_nSnapshotThumbHeight : DEFAULT_SNAPSHOT_THUMB_HEIGHT;
			CString sThumbWidth, sThumbHeight;
			sThumbWidth.Format(_T("%d"), nThumbWidth);
			sThumbHeight.Format(_T("%d"), nThumbHeight);

			// Update configuration.php
			if (m_bCheckFullStretch)
				m_pDoc->PhpConfigFileSetParam(PHPCONFIG_DEFAULTPAGE, PHPCONFIG_SNAPSHOTFULL_PHP);
			else
				m_pDoc->PhpConfigFileSetParam(PHPCONFIG_DEFAULTPAGE, PHPCONFIG_SNAPSHOT_PHP);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_WIDTH, sWidth);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_HEIGHT, sHeight);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_THUMBWIDTH, sThumbWidth);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_THUMBHEIGHT, sThumbHeight);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_SNAPSHOTHISTORY_THUMB, _T("0"));

			// Enable live snapshots
			m_pDoc->m_bSnapshotLiveJpeg = TRUE;
			m_pDoc->m_bSnapshotThumb = TRUE;

			// Disable snapshot history
			m_pDoc->m_bSnapshotHistorySwf = FALSE;

			// Update snapshot settings
			ApplySettingsSnapshot(nThumbWidth, nThumbHeight, dSnapshotRate);

			break;
		}
		case 3 :
		{
			// Reset mov. det. and 24h rec. flags
			bDoMovDet = FALSE;
			bDo24hRec = FALSE;

			// Init size vars
			CString sWidth, sHeight;
			sWidth.Format(_T("%d"), m_pDoc->m_DocRect.right);
			sHeight.Format(_T("%d"), m_pDoc->m_DocRect.bottom);

			// Init thumb vars
			int nThumbWidth =	(m_pDoc->m_nSnapshotThumbWidth < 4 * DEFAULT_SNAPSHOT_THUMB_WIDTH / 3 &&
								m_pDoc->m_nSnapshotThumbWidth > 2 * DEFAULT_SNAPSHOT_THUMB_WIDTH / 3) ?
								m_pDoc->m_nSnapshotThumbWidth : DEFAULT_SNAPSHOT_THUMB_WIDTH;
			int nThumbHeight =	(m_pDoc->m_nSnapshotThumbHeight < 4 * DEFAULT_SNAPSHOT_THUMB_HEIGHT / 3 &&
								m_pDoc->m_nSnapshotThumbHeight > 2 * DEFAULT_SNAPSHOT_THUMB_HEIGHT / 3) ?
								m_pDoc->m_nSnapshotThumbHeight : DEFAULT_SNAPSHOT_THUMB_HEIGHT;
			CString sThumbWidth, sThumbHeight;
			sThumbWidth.Format(_T("%d"), nThumbWidth);
			sThumbHeight.Format(_T("%d"), nThumbHeight);

			// Update configuration.php
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_DEFAULTPAGE, PHPCONFIG_SUMMARYIFRAME_PHP);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_WIDTH, sWidth);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_HEIGHT, sHeight);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_THUMBWIDTH, sThumbWidth);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_THUMBHEIGHT, sThumbHeight);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_SNAPSHOTHISTORY_THUMB, _T("0"));

			// Enable live snapshots
			m_pDoc->m_bSnapshotLiveJpeg = TRUE;
			m_pDoc->m_bSnapshotThumb = TRUE;

			// Disable snapshot history
			m_pDoc->m_bSnapshotHistorySwf = FALSE;

			// Update snapshot settings
			ApplySettingsSnapshot(nThumbWidth, nThumbHeight, MIN_SNAPSHOT_RATE);

			break;
		}
		default :
			break;
	}

	// Keep files for
	int nKeepForDays;
	switch (m_nComboKeepFor)
	{
		case 0  : nKeepForDays = 1;   break;
		case 1  : nKeepForDays = 2;   break;
		case 2  : nKeepForDays = 3;   break;
		case 3  : nKeepForDays = 7;   break;
		case 4  : nKeepForDays = 14;  break;
		case 5  : nKeepForDays = 21;  break;
		case 6  : nKeepForDays = 31;  break;
		case 7  : nKeepForDays = 62;  break;
		case 8  : nKeepForDays = 93;  break;
		case 9  : nKeepForDays = 183; break;
		case 10 : nKeepForDays = 366; break;
		default : nKeepForDays = 0;   break;
	}
	m_pDoc->m_nDeleteRecordingsOlderThanDays = nKeepForDays;

	// Restart delete thread
	m_pDoc->m_DeleteThread.Start(THREAD_PRIORITY_LOWEST);

	// Do mov. det.?
	if (bDoMovDet)
		m_pDoc->m_dwVideoProcessorMode |= SOFTWARE_MOVEMENT_DETECTOR;
	if (m_pDoc->GetFrame() && m_pDoc->GetFrame()->GetToolBar())
		((CVideoDeviceToolBar*)(m_pDoc->GetFrame()->GetToolBar()))->m_DetComboBox.SetCurSel(m_pDoc->m_dwVideoProcessorMode);
	if (m_pDoc->m_pMovementDetectionPage)
		m_pDoc->m_pMovementDetectionPage->UpdateDetectionState();
	m_pDoc->m_SaveFrameListThread.Start();

	// Enable/disable 24h rec.
	EnableDisable24hRec(bDo24hRec);

	// Restart process frame
	m_pDoc->StartProcessFrame(PROCESSFRAME_ASSISTANT);

	// Set Autorun
	if (m_nUsage >= 0 && m_nUsage <= 2)
	{
		if (m_pDoc->m_pGeneralPage)
		{
			CButton* pCheck = (CButton*)m_pDoc->m_pGeneralPage->GetDlgItem(IDC_CHECK_AUTORUN);
			pCheck->SetCheck(1);
			m_pDoc->m_pGeneralPage->m_bAutorun = TRUE;
		}
		CVideoDeviceDoc::AutorunAddDevice(m_pDoc->GetDevicePathName());
	}
	// Clear Autorun
	else if (m_nUsage == 3)
	{
		if (m_pDoc->m_pGeneralPage)
		{
			CButton* pCheck = (CButton*)m_pDoc->m_pGeneralPage->GetDlgItem(IDC_CHECK_AUTORUN);
			pCheck->SetCheck(0);
			m_pDoc->m_pGeneralPage->m_bAutorun = FALSE;
		}
		CVideoDeviceDoc::AutorunRemoveDevice(m_pDoc->GetDevicePathName());
	}

	// End wait cursor
	EndWaitCursor();

	// Update data from vars to view because m_sName may have been changed
	UpdateData(FALSE);

	// Save Settings (BeginWaitCursor() / EndWaitCursor() called inside this function)
	m_pDoc->SaveSettings();

	// Update titles because of possible device name change
	m_pDoc->SetDocumentTitle();

	// This calls UpdateData(TRUE) -> view to vars
	CDialog::OnOK();
}

#endif
