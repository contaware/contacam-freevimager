// AssistantPage.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "AssistantPage.h"
#include "VideoDeviceDoc.h"
#include "VideoDeviceView.h"
#include "VideoDevicePropertySheet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef VIDEODEVICEDOC

/////////////////////////////////////////////////////////////////////////////
// CAssistantPage property page

IMPLEMENT_DYNCREATE(CAssistantPage, CPropertyPage)

CAssistantPage::CAssistantPage()
	: CPropertyPage(CAssistantPage::IDD)
{
	//{{AFX_DATA_INIT(CAssistantPage)
	//}}AFX_DATA_INIT
	m_pDoc = NULL;
}

void CAssistantPage::SetDoc(CVideoDeviceDoc* pDoc)
{
	ASSERT(pDoc);
	m_pDoc = pDoc;
}

CAssistantPage::~CAssistantPage()
{
}

void CAssistantPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAssistantPage)
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


BEGIN_MESSAGE_MAP(CAssistantPage, CPropertyPage)
	//{{AFX_MSG_MAP(CAssistantPage)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_APPLY_SETTINGS, OnButtonApplySettings)
	ON_BN_CLICKED(IDC_RADIO_MOVDET, OnRadioMovdet)
	ON_BN_CLICKED(IDC_RADIO_SNAPSHOTHISTORY, OnRadioSnapshothistory)
	ON_BN_CLICKED(IDC_RADIO_SNAPSHOT, OnRadioSnapshot)
	ON_BN_CLICKED(IDC_RADIO_MANUAL, OnRadioManual)
	ON_BN_CLICKED(IDC_RADIO_NOCHANGE, OnRadioNochange)
	ON_WM_TIMER()
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAssistantPage message handlers

BOOL CAssistantPage::OnInitDialog() 
{
	// Check whether the web files exist in the given directory.
	CString sAutoSaveDir = m_pDoc->GetAutoSaveDir();
	m_pDoc->MicroApacheCheckWebFiles(sAutoSaveDir);

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
	sAutoSaveDir.TrimRight(_T('\\'));
	if (sAutoSaveDir != _T(""))
	{
		pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_LANGUAGE);
		if (pComboBox)
		{
			CSortableFileFind FileFind;
			FileFind.AddAllowedExtension(_T(".php"));
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
			FileFind.AddAllowedExtension(_T(".css"));
			if (FileFind.Init(sAutoSaveDir + _T("\\") + CString(MICROAPACHE_STYLE_DIR) + _T("\\") + _T("*")))
			{
				for (int pos = 0 ; pos < FileFind.GetFilesCount() ; pos++)
					pComboBox->AddString(::GetShortFileNameNoExt(FileFind.GetFileName(pos)));
			}
		}
	}

	// Init vars
	m_bDoApplySettings = FALSE;
	m_bCheck24hRec = Is24hRec();
	m_bCheckFullStretch = FALSE;
	m_sName = m_pDoc->GetAssignedDeviceName();
	m_sPhpConfigVersion = m_pDoc->PhpConfigFileGetParam(PHPCONFIG_VERSION);
	CString sInitDefaultPage = m_pDoc->PhpConfigFileGetParam(PHPCONFIG_DEFAULTPAGE);
	m_bCheckPrintCommand = (m_pDoc->PhpConfigFileGetParam(PHPCONFIG_SHOW_PRINTCOMMAND) == _T("1"));
	m_bCheckSaveCommand = (m_pDoc->PhpConfigFileGetParam(PHPCONFIG_SHOW_SAVECOMMAND) == _T("1"));
	if (sInitDefaultPage == PHPCONFIG_SUMMARYSNAPSHOT_NAME)
		m_nUsage = 0;
	else if (sInitDefaultPage == PHPCONFIG_SNAPSHOTHISTORY_NAME)
		m_nUsage = 1;
	else if (sInitDefaultPage == PHPCONFIG_SNAPSHOT_NAME)
	{
		m_nUsage = 2;
		m_bCheckFullStretch = FALSE;
	}
	else if (sInitDefaultPage == PHPCONFIG_SNAPSHOTFULL_NAME)
	{
		m_nUsage = 2;
		m_bCheckFullStretch = TRUE;
	}
	else if (sInitDefaultPage == PHPCONFIG_SUMMARYIFRAME_NAME)
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
		m_nComboSnapshotRate = 12;			// 5 Minutes
	else if (m_pDoc->m_nSnapshotRate > 180)
		m_nComboSnapshotRate = 11;			// 4 Minutes
	else if (m_pDoc->m_nSnapshotRate > 120)
		m_nComboSnapshotRate = 10;			// 3 Minutes
	else if (m_pDoc->m_nSnapshotRate > 60)
		m_nComboSnapshotRate = 9;			// 2 Minutes
	else if (m_pDoc->m_nSnapshotRate > 30)
		m_nComboSnapshotRate = 8;			// 1 Minute
	else if (m_pDoc->m_nSnapshotRate > 15)
		m_nComboSnapshotRate = 7;			// 30 Seconds
	else if (m_pDoc->m_nSnapshotRate > 10)
		m_nComboSnapshotRate = 6;			// 15 Seconds
	else if (m_pDoc->m_nSnapshotRate > 5)
		m_nComboSnapshotRate = 5;			// 10 Seconds
	else if (m_pDoc->m_nSnapshotRate > 4)
		m_nComboSnapshotRate = 4;			// 5 Seconds
	else if (m_pDoc->m_nSnapshotRate > 3)
		m_nComboSnapshotRate = 3;			// 4 Seconds
	else if (m_pDoc->m_nSnapshotRate > 2)
		m_nComboSnapshotRate = 2;			// 3 Seconds
	else if (m_pDoc->m_nSnapshotRate > 1)
		m_nComboSnapshotRate = 1;			// 2 Seconds
	else
		m_nComboSnapshotRate = 0;			// 1 Second
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

	// This calls UpdateData(FALSE)
	CPropertyPage::OnInitDialog();

	// Enable / Disable Controls
	EnableDisableCtrls();

	// Set Page Pointer to this
	m_pDoc->m_pAssistantPage = this;
	
	// Set Timer
	SetTimer(ID_TIMER_ASSISTANTDLG, ASSISTANTDLG_TIMER_MS, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAssistantPage::OnDestroy() 
{
	// Kill timer
	KillTimer(ID_TIMER_ASSISTANTDLG);

	// Base class
	CPropertyPage::OnDestroy();

	// Set Page Pointer to NULL
	m_pDoc->m_pAssistantPage = NULL;
}

void CAssistantPage::EnableDisableCtrls()
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
		pCheck->EnableWindow(!m_pDoc->m_pVideoAviDoc);
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
		pEdit = (CEdit*)GetDlgItem(IDC_LABEL_THUMBSPERPAGE);
		pEdit->EnableWindow(FALSE);
		pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_THUMBSPERPAGE);
		pComboBox->EnableWindow(FALSE);
	}
}

void CAssistantPage::EnableDisableAllCtrls(BOOL bEnable)
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
	CButton* pButton = (CButton*)GetDlgItem(IDC_BUTTON_APPLY_SETTINGS);
	pButton->EnableWindow(bEnable);
}

void CAssistantPage::OnRadioMovdet() 
{
	EnableDisableCtrls();
}

void CAssistantPage::OnRadioSnapshothistory() 
{
	EnableDisableCtrls();
}

void CAssistantPage::OnRadioSnapshot() 
{
	EnableDisableCtrls();
}

void CAssistantPage::OnRadioManual() 
{
	EnableDisableCtrls();
}

void CAssistantPage::OnRadioNochange() 
{
	EnableDisableCtrls();
}

void CAssistantPage::EnableDisable24hRec(BOOL bEnable)
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
#if _MFC_VER >= 0x0700
			CTime MaxTime(3000, 12, 31, 23, 59, 59);
#else
			CTime MaxTime(2037, 12, 31, 23, 59, 59);
#endif
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
			m_pDoc->m_bRecSizeSegmentation = m_pDoc->m_pGeneralPage->m_bRecSizeSegmentation = FALSE;
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
			m_pDoc->m_bRecSizeSegmentation = m_pDoc->m_pGeneralPage->m_bRecSizeSegmentation = FALSE;
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
#if _MFC_VER >= 0x0700
			pSchedulerEntry->m_StopTime = CTime(3000, 12, 31, 23, 59, 59);
#else
			pSchedulerEntry->m_StopTime = CTime(2037, 12, 31, 23, 59, 59);
#endif
			
			// 6 Hours Segmentation
			m_pDoc->m_bRecTimeSegmentation = TRUE;
			m_pDoc->m_nTimeSegmentationIndex = 3;
			m_pDoc->m_bRecSizeSegmentation = FALSE;
			m_pDoc->m_bRecAutoOpen = FALSE;

			// Add Scheduler Entry
			((CUImagerApp*)::AfxGetApp())->AddSchedulerEntry(pSchedulerEntry);
		}
		else
		{
			// No Segmentation
			m_pDoc->m_bRecTimeSegmentation = FALSE;
			m_pDoc->m_nTimeSegmentationIndex = 0;
			m_pDoc->m_bRecSizeSegmentation = FALSE;
			m_pDoc->m_bRecAutoOpen = TRUE;
		}
	}
}

BOOL CAssistantPage::Is24hRec() 
{
	if (!m_pDoc->m_pVideoAviDoc)
	{
		CUImagerApp::CSchedulerEntry* pOnceSchedulerEntry =
			((CUImagerApp*)::AfxGetApp())->GetOnceSchedulerEntry(m_pDoc->GetDevicePathName());
		if (pOnceSchedulerEntry											&&
			pOnceSchedulerEntry->m_StartTime <= CTime::GetCurrentTime()	&&
#if _MFC_VER >= 0x0700
			pOnceSchedulerEntry->m_StopTime == CTime(3000, 12, 31, 23, 59, 59))
#else
			pOnceSchedulerEntry->m_StopTime == CTime(2037, 12, 31, 23, 59, 59))
#endif
			return TRUE;
	}
	return FALSE;
}

void CAssistantPage::OnTimer(UINT nIDEvent) 
{
	if (m_bDoApplySettings && !m_pDoc->m_bClosing)
	{
		if (m_pDoc->m_pAVRec)
		{
			// Stop Rec
			if (!m_pDoc->m_bAboutToStopRec &&
				!m_pDoc->m_bAboutToStartRec)
			{
				m_pDoc->m_bRecAutoOpenAllowed = FALSE;
				m_pDoc->CaptureRecord();
			}
		}
		else
		{
			// Apply settings if we are not inside
			// the processing function
			if (m_pDoc->IsProcessFrameStopped())
				ApplySettings();
			else
				m_pDoc->StopProcessFrame();
		}
	}
	CPropertyPage::OnTimer(nIDEvent);
}

BOOL CAssistantPage::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
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

void CAssistantPage::OnButtonApplySettings() 
{
	if (m_pDoc->m_bClosing			||
		!m_pDoc->m_bCapture			||
		m_pDoc->m_bWatchDogAlarm)
		::MessageBeep(0xFFFFFFFF);
	else
	{
		// Begin wait cursor
		BeginWaitCursor();

		// Set flag
		m_bDoApplySettings = TRUE;

		// Disable all
		EnableDisableAllCtrls(FALSE);
	}
}

void CAssistantPage::Rename()
{
	// Store current name
	CString sOldName = m_pDoc->GetAssignedDeviceName();

	// Is ANSI?
	if (!::IsANSIConvertible(m_sName))
	{
		// Error Message
		::AfxMessageBox(ML_STRING(1767, "Only the ANSI character set is supported for the camera name"), MB_OK | MB_ICONERROR);
		
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
	
	// Adjust old dirs
	m_pDoc->m_sRecordAutoSaveDir.TrimRight(_T('\\'));
	m_pDoc->m_sDetectionAutoSaveDir.TrimRight(_T('\\'));
	m_pDoc->m_sSnapshotAutoSaveDir.TrimRight(_T('\\'));

	// Make new dirs
	CString sNewRecordAutoSaveDir = ::GetDriveAndDirName(m_pDoc->m_sRecordAutoSaveDir);
	CString sNewDetectionAutoSaveDir = ::GetDriveAndDirName(m_pDoc->m_sDetectionAutoSaveDir);
	CString sNewSnapshotAutoSaveDir = ::GetDriveAndDirName(m_pDoc->m_sSnapshotAutoSaveDir);
	sNewRecordAutoSaveDir.TrimRight(_T('\\'));
	sNewDetectionAutoSaveDir.TrimRight(_T('\\'));
	sNewSnapshotAutoSaveDir.TrimRight(_T('\\'));
	if (m_pDoc->m_sRecordAutoSaveDir.ReverseFind(_T('\\')) < 0)		// It's just the drive letter?
		sNewRecordAutoSaveDir = m_pDoc->m_sRecordAutoSaveDir;
	else
		sNewRecordAutoSaveDir += _T('\\') + m_sName;
	if (m_pDoc->m_sDetectionAutoSaveDir.ReverseFind(_T('\\')) < 0)	// It's just the drive letter?
		sNewDetectionAutoSaveDir = m_pDoc->m_sDetectionAutoSaveDir;
	else
		sNewDetectionAutoSaveDir += _T('\\') + m_sName;
	if (m_pDoc->m_sSnapshotAutoSaveDir.ReverseFind(_T('\\')) < 0)	// It's just the drive letter?
		sNewSnapshotAutoSaveDir = m_pDoc->m_sSnapshotAutoSaveDir;
	else
		sNewSnapshotAutoSaveDir += _T('\\') + m_sName;
	
	// Prompt for merging
	if ((::IsExistingDir(m_pDoc->m_sRecordAutoSaveDir)    && ::IsExistingDir(sNewRecordAutoSaveDir)    &&
		sNewRecordAutoSaveDir    != m_pDoc->m_sRecordAutoSaveDir)                                      ||

		(::IsExistingDir(m_pDoc->m_sDetectionAutoSaveDir) && ::IsExistingDir(sNewDetectionAutoSaveDir) &&
		sNewDetectionAutoSaveDir != m_pDoc->m_sDetectionAutoSaveDir)                                   ||

		(::IsExistingDir(m_pDoc->m_sSnapshotAutoSaveDir)  && ::IsExistingDir(sNewSnapshotAutoSaveDir)  &&
		sNewSnapshotAutoSaveDir  != m_pDoc->m_sSnapshotAutoSaveDir))
	{
		CString sMsg;
		sMsg.Format(ML_STRING(1765, "%s already exists.\nDo you want to proceed and merge the files?"), m_sName);
		if (::AfxMessageBox(sMsg, MB_YESNO | MB_ICONQUESTION) == IDNO)
		{
			// Restore old name
			m_sName = sOldName;
			return;
		}
	}

	// Error code
	DWORD dwLastError = ERROR_SUCCESS;

	// Record directory
	if (::IsExistingDir(m_pDoc->m_sRecordAutoSaveDir) && sNewRecordAutoSaveDir != m_pDoc->m_sRecordAutoSaveDir)
	{
		if (::IsExistingDir(sNewRecordAutoSaveDir) || !::MoveFile(m_pDoc->m_sRecordAutoSaveDir, sNewRecordAutoSaveDir))
		{
			if (!::MergeDirContent(m_pDoc->m_sRecordAutoSaveDir, sNewRecordAutoSaveDir))
			{
				dwLastError = ::GetLastError();
				sNewRecordAutoSaveDir = m_pDoc->m_sRecordAutoSaveDir;
			}
			else
				::DeleteDir(m_pDoc->m_sRecordAutoSaveDir); // No error message on failure
		}
	}
	else if (!::IsExistingDir(sNewRecordAutoSaveDir) && !::CreateDir(sNewRecordAutoSaveDir))
	{
		dwLastError = ::GetLastError();
		sNewRecordAutoSaveDir = m_pDoc->m_sRecordAutoSaveDir;
	}
	m_pDoc->m_sRecordAutoSaveDir = sNewRecordAutoSaveDir;
	if (m_pDoc->m_pGeneralPage)
	{			
		m_pDoc->m_pGeneralPage->m_DirLabel.SetLink(m_pDoc->m_sRecordAutoSaveDir);
		CEdit* pEdit = (CEdit*)m_pDoc->m_pGeneralPage->GetDlgItem(IDC_RECORD_SAVEAS_PATH);
		pEdit->SetWindowText(m_pDoc->m_sRecordAutoSaveDir);
	}

	// Detection directory
	if (::IsExistingDir(m_pDoc->m_sDetectionAutoSaveDir) && sNewDetectionAutoSaveDir != m_pDoc->m_sDetectionAutoSaveDir)
	{
		if (::IsExistingDir(sNewDetectionAutoSaveDir) || !::MoveFile(m_pDoc->m_sDetectionAutoSaveDir, sNewDetectionAutoSaveDir))
		{
			if (!::MergeDirContent(m_pDoc->m_sDetectionAutoSaveDir, sNewDetectionAutoSaveDir))
			{
				dwLastError = ::GetLastError();
				sNewDetectionAutoSaveDir = m_pDoc->m_sDetectionAutoSaveDir;
			}
			else
				::DeleteDir(m_pDoc->m_sDetectionAutoSaveDir); // No error message on failure
		}
	}
	else if (!::IsExistingDir(sNewDetectionAutoSaveDir) && !::CreateDir(sNewDetectionAutoSaveDir))
	{
		dwLastError = ::GetLastError();
		sNewDetectionAutoSaveDir = m_pDoc->m_sDetectionAutoSaveDir;
	}
	m_pDoc->m_sDetectionAutoSaveDir = sNewDetectionAutoSaveDir;
	if (m_pDoc->m_pMovementDetectionPage)
	{
		m_pDoc->m_pMovementDetectionPage->m_DirLabel.SetLink(m_pDoc->m_sDetectionAutoSaveDir);
		CEdit* pEdit = (CEdit*)m_pDoc->m_pMovementDetectionPage->GetDlgItem(IDC_DETECTION_SAVEAS_PATH);
		pEdit->SetWindowText(m_pDoc->m_sDetectionAutoSaveDir);
	}
	
	// Snapshot directory
	if (::IsExistingDir(m_pDoc->m_sSnapshotAutoSaveDir) && sNewSnapshotAutoSaveDir != m_pDoc->m_sSnapshotAutoSaveDir)
	{
		if (::IsExistingDir(sNewSnapshotAutoSaveDir) || !::MoveFile(m_pDoc->m_sSnapshotAutoSaveDir, sNewSnapshotAutoSaveDir))
		{
			if (!::MergeDirContent(m_pDoc->m_sSnapshotAutoSaveDir, sNewSnapshotAutoSaveDir))
			{
				dwLastError = ::GetLastError();
				sNewSnapshotAutoSaveDir = m_pDoc->m_sSnapshotAutoSaveDir;
			}
			else
				::DeleteDir(m_pDoc->m_sSnapshotAutoSaveDir); // No error message on failure
		}
	}
	else if (!::IsExistingDir(sNewSnapshotAutoSaveDir) && !::CreateDir(sNewSnapshotAutoSaveDir))
	{
		dwLastError = ::GetLastError();
		sNewSnapshotAutoSaveDir = m_pDoc->m_sSnapshotAutoSaveDir;
	}
	m_pDoc->m_sSnapshotAutoSaveDir = sNewSnapshotAutoSaveDir;
	if (m_pDoc->m_pSnapshotPage)
	{	
		m_pDoc->m_pSnapshotPage->m_DirLabel.SetLink(m_pDoc->m_sSnapshotAutoSaveDir);
		CEdit* pEdit = (CEdit*)m_pDoc->m_pSnapshotPage->GetDlgItem(IDC_SNAPSHOT_SAVEAS_PATH);
		pEdit->SetWindowText(m_pDoc->m_sSnapshotAutoSaveDir);
	}

	// On Error
	if (dwLastError != ERROR_SUCCESS)
	{
		// Error Message
		::ShowError(dwLastError, TRUE);

		// Restore old name
		m_sName = sOldName;
	}
}

void CAssistantPage::ApplySettingsUpdate(int nThumbWidth, int nThumbHeight, const CString& sSnapShotRate)
{
	if (m_pDoc->m_pSnapshotPage)
	{
		// Thumb size (this updates the controls and sets m_nSnapshotThumbWidth and m_nSnapshotThumbHeight)
		m_pDoc->m_pSnapshotPage->ChangeThumbSize(nThumbWidth, nThumbHeight);
		
		// Live snapshots
		CButton* pCheck = (CButton*)m_pDoc->m_pSnapshotPage->GetDlgItem(IDC_CHECK_SNAPSHOT_LIVE_JPEG);
		pCheck->SetCheck(m_pDoc->m_bSnapshotLiveJpeg ? 1 : 0);
		pCheck = (CButton*)m_pDoc->m_pSnapshotPage->GetDlgItem(IDC_CHECK_SNAPSHOT_THUMB);
		pCheck->SetCheck(m_pDoc->m_bSnapshotThumb ? 1 : 0);
		CEdit* pEdit = (CEdit*)m_pDoc->m_pSnapshotPage->GetDlgItem(IDC_EDIT_SNAPSHOT_RATE);
		pEdit->SetWindowText(sSnapShotRate);

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

void CAssistantPage::ApplySettings() 
{
	// Reset flag
	m_bDoApplySettings = FALSE;

	// Update data
	UpdateData(TRUE);

	// Disable mov. det.
	BOOL bDoMovDet;
	if (m_pDoc->m_VideoProcessorMode & MOVEMENT_DETECTOR)
	{
		bDoMovDet = TRUE;
		m_pDoc->m_VideoProcessorMode &= ~MOVEMENT_DETECTOR;
	}
	else
		bDoMovDet = FALSE;
	m_pDoc->m_SaveFrameListThread.Kill();
	m_pDoc->OneEmptyFrameList();
	m_pDoc->ResetMovementDetector();

	// Is 24h rec.?
	BOOL bDo24hRec = Is24hRec();

	// Stop the delete thread
	m_pDoc->m_DeleteThread.Kill();

	// Make sure snapshot threads are stopped
	// (at this point the process frame is stopped but
	// the snapshot threads may still be running)
	m_pDoc->m_SaveSnapshotThread.Kill();
	m_pDoc->m_SaveSnapshotFTPThread.Kill();

	// Rename
	Rename();

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

			// Init snapshot rate vars
			int nSnapshotRate = MIN_SNAPSHOT_RATE;
			CString sSnapShotRate;
			sSnapShotRate.Format(_T("%d"), nSnapshotRate);

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
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_DEFAULTPAGE, PHPCONFIG_SUMMARYSNAPSHOT_NAME);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_WIDTH, sWidth);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_HEIGHT, sHeight);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_THUMBWIDTH, sThumbWidth);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_THUMBHEIGHT, sThumbHeight);
			if (sMaxPerPage != _T(""))
				m_pDoc->PhpConfigFileSetParam(PHPCONFIG_MAX_PER_PAGE, sMaxPerPage);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_SNAPSHOT_THUMB, _T("1"));
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_SNAPSHOTHISTORY_THUMB, _T("0"));
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_SNAPSHOTREFRESHSEC, sSnapShotRate);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_SUMMARYTITLE, m_sName);

			// Enable live snapshots
			m_pDoc->m_bSnapshotLiveJpeg = TRUE;
			m_pDoc->m_bSnapshotThumb = TRUE;
			m_pDoc->m_nSnapshotRate = nSnapshotRate;

			// Disable snapshot history
			m_pDoc->m_bSnapshotHistorySwf = FALSE;
			::InterlockedExchange(&m_pDoc->m_bSnapshotHistoryCloseSwfFile, 1);

			// Update
			ApplySettingsUpdate(nThumbWidth, nThumbHeight, sSnapShotRate);

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
			CString sSnapShotRate;
			sSnapShotRate.Format(_T("%d"), nSnapshotRate);

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
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_DEFAULTPAGE, PHPCONFIG_SNAPSHOTHISTORY_NAME);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_WIDTH, sWidth);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_HEIGHT, sHeight);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_THUMBWIDTH, sThumbWidth);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_THUMBHEIGHT, sThumbHeight);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_SNAPSHOT_THUMB, _T("0"));
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_SNAPSHOTHISTORY_THUMB, bUseThumb ? _T("1") : _T("0"));
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_SNAPSHOTREFRESHSEC, sSnapShotRate);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_SNAPSHOTTITLE, m_sName);

			// Enable live snapshots
			m_pDoc->m_bSnapshotLiveJpeg = TRUE;
			m_pDoc->m_bSnapshotThumb = bUseThumb;
			m_pDoc->m_nSnapshotRate = nSnapshotRate;

			// Enable snapshot history
			m_pDoc->m_bSnapshotHistorySwf = TRUE;
			::InterlockedExchange(&m_pDoc->m_bSnapshotHistoryCloseSwfFile, 1); // Make new swf because Size or Name may have been changed

			// Update
			ApplySettingsUpdate(nThumbWidth, nThumbHeight, sSnapShotRate);

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
			int nSnapshotRate;
			switch (m_nComboSnapshotRate)
			{
				case 0  : nSnapshotRate = 1;    break;	// 1 Second
				case 1  : nSnapshotRate = 2;    break;	// 2 Seconds
				case 2  : nSnapshotRate = 3;    break;	// 3 Seconds
				case 3  : nSnapshotRate = 4;    break;	// 4 Seconds
				case 4  : nSnapshotRate = 5;    break;	// 5 Seconds
				case 5  : nSnapshotRate = 10;   break;	// 10 Seconds
				case 6  : nSnapshotRate = 15;   break;	// 15 Seconds
				case 7  : nSnapshotRate = 30;   break;	// 30 Seconds
				case 8  : nSnapshotRate = 60;   break;	// 1 Minute
				case 9  : nSnapshotRate = 120;  break;	// 2 Minutes
				case 10 : nSnapshotRate = 180;  break;	// 3 Minutes
				case 11 : nSnapshotRate = 240;  break;	// 4 Minutes
				default : nSnapshotRate = 300;  break;	// 5 Minutes
			}
			CString sSnapShotRate;
			sSnapShotRate.Format(_T("%d"), nSnapshotRate);

			// Init thumb vars
			int nThumbWidth = m_pDoc->m_nSnapshotThumbWidth;
			int nThumbHeight = m_pDoc->m_nSnapshotThumbHeight;
			CString sThumbWidth, sThumbHeight;
			sThumbWidth.Format(_T("%d"), nThumbWidth);
			sThumbHeight.Format(_T("%d"), nThumbHeight);

			// Update configuration.php
			if (m_bCheckFullStretch)
				m_pDoc->PhpConfigFileSetParam(PHPCONFIG_DEFAULTPAGE, PHPCONFIG_SNAPSHOTFULL_NAME);
			else
				m_pDoc->PhpConfigFileSetParam(PHPCONFIG_DEFAULTPAGE, PHPCONFIG_SNAPSHOT_NAME);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_WIDTH, sWidth);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_HEIGHT, sHeight);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_THUMBWIDTH, sThumbWidth);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_THUMBHEIGHT, sThumbHeight);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_SNAPSHOT_THUMB, _T("0"));
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_SNAPSHOTHISTORY_THUMB, _T("0"));
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_SNAPSHOTREFRESHSEC, sSnapShotRate);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_SNAPSHOTTITLE, m_sName);

			// Enable live snapshots
			m_pDoc->m_bSnapshotLiveJpeg = TRUE;
			m_pDoc->m_bSnapshotThumb = FALSE;
			m_pDoc->m_nSnapshotRate = nSnapshotRate;

			// Disable snapshot history
			m_pDoc->m_bSnapshotHistorySwf = FALSE;
			::InterlockedExchange(&m_pDoc->m_bSnapshotHistoryCloseSwfFile, 1);

			// Update
			ApplySettingsUpdate(nThumbWidth, nThumbHeight, sSnapShotRate);

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

			// Init snapshot rate vars
			int nSnapshotRate = DEFAULT_SNAPSHOT_RATE;
			CString sSnapShotRate;
			sSnapShotRate.Format(_T("%d"), nSnapshotRate);

			// Init thumb vars
			int nThumbWidth = m_pDoc->m_nSnapshotThumbWidth;
			int nThumbHeight = m_pDoc->m_nSnapshotThumbHeight;
			CString sThumbWidth, sThumbHeight;
			sThumbWidth.Format(_T("%d"), nThumbWidth);
			sThumbHeight.Format(_T("%d"), nThumbHeight);

			// Update configuration.php
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_DEFAULTPAGE, PHPCONFIG_SUMMARYIFRAME_NAME);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_WIDTH, sWidth);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_HEIGHT, sHeight);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_THUMBWIDTH, sThumbWidth);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_THUMBHEIGHT, sThumbHeight);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_SNAPSHOT_THUMB, _T("0"));
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_SNAPSHOTHISTORY_THUMB, _T("0"));
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_SNAPSHOTREFRESHSEC, sSnapShotRate);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_SUMMARYTITLE, m_sName);

			// Disable live snapshots
			m_pDoc->m_bSnapshotLiveJpeg = FALSE;
			m_pDoc->m_bSnapshotThumb = FALSE;
			m_pDoc->m_nSnapshotRate = nSnapshotRate;

			// Disable snapshot history
			m_pDoc->m_bSnapshotHistorySwf = FALSE;
			::InterlockedExchange(&m_pDoc->m_bSnapshotHistoryCloseSwfFile, 1);

			// Update
			ApplySettingsUpdate(nThumbWidth, nThumbHeight, sSnapShotRate);

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
	m_pDoc->m_nDeleteDetectionsOlderThanDays = nKeepForDays;
	m_pDoc->m_nDeleteSnapshotsOlderThanDays = nKeepForDays;
	if (m_pDoc->m_pGeneralPage)
	{
		m_pDoc->m_pGeneralPage->m_nDeleteRecordingsOlderThanDays = m_pDoc->m_nDeleteRecordingsOlderThanDays;
		CEdit* pEdit = (CEdit*)m_pDoc->m_pGeneralPage->GetDlgItem(IDC_EDIT_DELETE_RECORDINGS_DAYS);
		CString s;
		s.Format(_T("%d"), m_pDoc->m_pGeneralPage->m_nDeleteRecordingsOlderThanDays);
		pEdit->SetWindowText(s);
	}
	if (m_pDoc->m_pMovementDetectionPage)
	{
		m_pDoc->m_pMovementDetectionPage->m_nDeleteDetectionsOlderThanDays = m_pDoc->m_nDeleteDetectionsOlderThanDays;
		CEdit* pEdit = (CEdit*)m_pDoc->m_pMovementDetectionPage->GetDlgItem(IDC_EDIT_DELETE_DETECTIONS_DAYS);
		CString s;
		s.Format(_T("%d"), m_pDoc->m_pMovementDetectionPage->m_nDeleteDetectionsOlderThanDays);
		pEdit->SetWindowText(s);
	}
	if (m_pDoc->m_pSnapshotPage)
	{
		m_pDoc->m_pSnapshotPage->m_nDeleteSnapshotsOlderThanDays = m_pDoc->m_nDeleteSnapshotsOlderThanDays;
		CEdit* pEdit = (CEdit*)m_pDoc->m_pSnapshotPage->GetDlgItem(IDC_EDIT_DELETE_SNAPSHOTS_DAYS);
		CString s;
		s.Format(_T("%d"), m_pDoc->m_pSnapshotPage->m_nDeleteSnapshotsOlderThanDays);
		pEdit->SetWindowText(s);
	}

	// Restart delete thread
	m_pDoc->m_DeleteThread.Start(THREAD_PRIORITY_LOWEST);

	// Do mov. det.?
	if (bDoMovDet)
	{
		m_pDoc->m_VideoProcessorMode |= MOVEMENT_DETECTOR;
		if (m_pDoc->m_pMovementDetectionPage)
		{
			CButton* pCheck = (CButton*)m_pDoc->m_pMovementDetectionPage->GetDlgItem(IDC_CHECK_VIDEO_DETECTION_MOVEMENT);
			pCheck->SetCheck(1);
			CEdit* pEdit = (CEdit*)m_pDoc->m_pMovementDetectionPage->GetDlgItem(IDC_WARNING);
			if (m_pDoc->m_bUnsupportedVideoSizeForMovDet)
				pEdit->ShowWindow(TRUE);
			else
				pEdit->ShowWindow(FALSE);
		}
	}
	else
	{
		if (m_pDoc->m_pMovementDetectionPage)
		{
			CButton* pCheck = (CButton*)m_pDoc->m_pMovementDetectionPage->GetDlgItem(IDC_CHECK_VIDEO_DETECTION_MOVEMENT);
			pCheck->SetCheck(0);
			CEdit* pEdit = (CEdit*)m_pDoc->m_pMovementDetectionPage->GetDlgItem(IDC_WARNING);
			pEdit->ShowWindow(FALSE);
		}
	}
	m_pDoc->m_SaveFrameListThread.Start();

	// Enable/disable 24h rec.
	EnableDisable24hRec(bDo24hRec);

	// Restart process frame
	m_pDoc->ReStartProcessFrame();

	// Set Autorun
	if (!m_pDoc->m_pVideoAviDoc && m_nUsage >= 0 && m_nUsage <= 2)
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

	// New versions of the web files
	m_pDoc->MicroApacheCheckWebFiles(m_pDoc->GetAutoSaveDir(), TRUE);

	// End wait cursor
	EndWaitCursor();

	// Update data because m_sName may have been changed
	UpdateData(FALSE);

	// Close
	CVideoDevicePropertySheet* pPropertySheet = (CVideoDevicePropertySheet*)GetParent();
	pPropertySheet->Hide();

	// Restore var
	m_pDoc->m_bRecAutoOpenAllowed = TRUE;

	// Update title
	pPropertySheet->SetTitle(	m_pDoc->GetDeviceName() != m_pDoc->GetAssignedDeviceName() ?
								m_pDoc->GetAssignedDeviceName() + _T(" (") + m_pDoc->GetDeviceName() + _T(")") :
								m_pDoc->GetDeviceName());

	// Enable all
	EnableDisableAllCtrls(TRUE);
}

#endif
