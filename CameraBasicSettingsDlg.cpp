// CameraBasicSettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "CameraBasicSettingsDlg.h"
#include "VideoDeviceDoc.h"
#include "VideoDeviceView.h"
#include "CameraAdvancedSettingsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef VIDEODEVICEDOC

/////////////////////////////////////////////////////////////////////////////
// CCameraBasicSettingsDlg dialog

CCameraBasicSettingsDlg::CCameraBasicSettingsDlg(CVideoDeviceDoc* pDoc, CWnd* pParent /*=NULL*/)
	: CDialog(CCameraBasicSettingsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCameraBasicSettingsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_pDoc = pDoc;
}

CCameraBasicSettingsDlg::~CCameraBasicSettingsDlg()
{
}

void CCameraBasicSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCameraBasicSettingsDlg)
	DDX_CBIndex(pDX, IDC_COMBO_KEEPFOR, m_nComboKeepFor);
	DDX_Text(pDX, IDC_EDIT_NAME, m_sName);
	DDX_Radio(pDX, IDC_RADIO_MOVDET, m_nUsage);
	DDX_Check(pDX, IDC_CHECK_AUTORUN, m_bAutorun);
	DDX_Check(pDX, IDC_CHECK_CAMERACOMMANDS, m_bCheckCameraCommands);
	DDX_Check(pDX, IDC_CHECK_SENDMAIL_MALFUNCTION, m_bCheckSendMailMalfunction);
	DDX_Check(pDX, IDC_CHECK_SENDMAIL_SNAPSHOT, m_bCheckSendMailSnapshot);
	DDX_Check(pDX, IDC_CHECK_SENDMAIL_SNAPSHOT_HISTORY, m_bCheckSendMailSnapshotHistory);
	DDX_Check(pDX, IDC_CHECK_SENDMAIL_RECORDING, m_bCheckSendMailRecording);
	DDX_CBIndex(pDX, IDC_ATTACHMENT, m_nComboSendMailAttachment);
	DDX_Text(pDX, IDC_EDIT_SENDMAIL_SEC_BETWEEN_MSG, m_nSendMailSecBetweenMsg);
	DDV_MinMaxInt(pDX, m_nSendMailSecBetweenMsg, 0, INT_MAX);
	DDX_Text(pDX, IDC_EDIT_MAX_CAMERA_FOLDER_SIZE, m_sMaxCameraFolderSizeGB);
	DDX_Text(pDX, IDC_EDIT_MIN_DISK_FREE_PERCENT, m_sMinDiskFreePercent);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCameraBasicSettingsDlg, CDialog)
	//{{AFX_MSG_MAP(CCameraBasicSettingsDlg)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_SENDMAIL_CONFIGURE, OnSendmailConfigure)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCameraBasicSettingsDlg message handlers

BOOL CCameraBasicSettingsDlg::OnInitDialog()
{
	// Get a copy of m_sRecordAutoSaveDir
	// (do not modify it now because various threads
	// using it are still running)
	CString sAutoSaveDir = m_pDoc->m_sRecordAutoSaveDir;
	sAutoSaveDir.TrimRight(_T('\\'));

	// Overwrite web files in given directory
	m_pDoc->MicroApacheUpdateWebFiles(sAutoSaveDir);

	// Init Combo Boxes
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_KEEPFOR);
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
		pComboBox->SetMinVisibleItems(PHPCONFIG_MAX_THUMSPERPAGE - PHPCONFIG_MIN_THUMSPERPAGE + 1);
	}
	if (sAutoSaveDir != _T(""))
	{
		pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_LANGUAGE);
		if (pComboBox)
		{
			CSortableFileFind FileFind;
			FileFind.AddAllowedExtension(_T("php"));
			if (FileFind.Init(sAutoSaveDir + _T("\\") + CString(PHP_LANGUAGES_DIR) + _T("\\") + _T("*")))
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
			if (FileFind.Init(sAutoSaveDir + _T("\\") + CString(CSS_STYLE_DIR) + _T("\\") + _T("*")))
			{
				for (int pos = 0 ; pos < FileFind.GetFilesCount() ; pos++)
					pComboBox->AddString(::GetShortFileNameNoExt(FileFind.GetFileName(pos)));
			}
		}
	}
	pComboBox = (CComboBox*)GetDlgItem(IDC_ATTACHMENT);
	if (pComboBox)
	{
		pComboBox->AddString(ML_STRING(1880, "No Attachment"));
		pComboBox->AddString(ML_STRING(1883, "Saved Full Video"));
		pComboBox->AddString(ML_STRING(1882, "Saved Small Video"));
		pComboBox->AddString(ML_STRING(1881, "Snapshot"));
		pComboBox->AddString(ML_STRING(1881, "Snapshot") + _T(" + ") + ML_STRING(1883, "Saved Full Video"));
		pComboBox->AddString(ML_STRING(1881, "Snapshot") + _T(" + ") + ML_STRING(1882, "Saved Small Video"));
	}

	// Init vars
	m_bDoApplySettings = FALSE;
	m_nRetryTimeMs = 0;
	m_sName = m_pDoc->GetAssignedDeviceName();
	m_bCheckCameraCommands = (m_pDoc->PhpConfigFileGetParam(PHPCONFIG_SHOW_CAMERA_COMMANDS) == _T("1"));
	CString sInitDefaultPage = m_pDoc->PhpConfigFileGetParam(PHPCONFIG_DEFAULTPAGE);
	if (sInitDefaultPage.CompareNoCase(PHPCONFIG_SUMMARYSNAPSHOT_PHP) == 0)
		m_nUsage = 0;
	else if (sInitDefaultPage.CompareNoCase(PHPCONFIG_SNAPSHOTHISTORY_PHP) == 0	||
			sInitDefaultPage.CompareNoCase(PHPCONFIG_SNAPSHOT_PHP) == 0			||
			sInitDefaultPage.CompareNoCase(PHPCONFIG_SNAPSHOTFULL_PHP) == 0)
		m_nUsage = 1;
	else
		m_nUsage = 2;
	if (CVideoDeviceDoc::AutorunGetDeviceKey(m_pDoc->GetDevicePathName()) != _T(""))
		m_bAutorun = TRUE;
	else
		m_bAutorun = FALSE;
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
	m_sMaxCameraFolderSizeGB.Format(_T("%.1f"), (double)m_pDoc->m_nMaxCameraFolderSizeMB / 1024.0);
	m_sMinDiskFreePercent.Format(_T("%.3f"), (double)m_pDoc->m_nMinDiskFreePermillion / 10000.0);
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
	m_bCheckSendMailMalfunction = m_pDoc->m_bSendMailMalfunction;
	m_bCheckSendMailRecording = m_pDoc->m_bSendMailRecording;
	m_bCheckSendMailSnapshot = m_pDoc->m_bSendMailSnapshot;
	m_bCheckSendMailSnapshotHistory = m_pDoc->m_bSendMailSnapshotHistory;
	m_nComboSendMailAttachment = m_pDoc->m_AttachmentType;
	m_nSendMailSecBetweenMsg = m_pDoc->m_nMovDetSendMailSecBetweenMsg;
	m_CurrentSendMailConfiguration = m_pDoc->m_SendMailConfiguration;

	// This calls UpdateData(FALSE) -> vars to view
	CDialog::OnInitDialog();
	
	// Set Timer
	SetTimer(ID_TIMER_CAMERABASICSETTINGSDLG, CAMERABASICSETTINGSDLG_TIMER_MS, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCameraBasicSettingsDlg::OnDestroy() 
{
	// Kill timer
	KillTimer(ID_TIMER_CAMERABASICSETTINGSDLG);

	// Base class
	CDialog::OnDestroy();
}

void CCameraBasicSettingsDlg::EnableDisableAllCtrls(BOOL bEnable)
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_RADIO_MOVDET);
	pCheck->EnableWindow(bEnable);
	pCheck = (CButton*)GetDlgItem(IDC_RADIO_SNAPSHOT);
	pCheck->EnableWindow(bEnable);
	pCheck = (CButton*)GetDlgItem(IDC_RADIO_MANUAL);
	pCheck->EnableWindow(bEnable);
	pCheck = (CButton*)GetDlgItem(IDC_CHECK_AUTORUN);
	pCheck->EnableWindow(bEnable);
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_NAME);
	pEdit->EnableWindow(bEnable);
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_LANGUAGE);
	pComboBox->EnableWindow(bEnable);
	pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_STYLE);
	pComboBox->EnableWindow(bEnable);
	pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_THUMBSPERPAGE);
	pComboBox->EnableWindow(bEnable);
	pCheck = (CButton*)GetDlgItem(IDC_CHECK_CAMERACOMMANDS);
	pCheck->EnableWindow(bEnable);
	pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_KEEPFOR);
	pComboBox->EnableWindow(bEnable);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_MAX_CAMERA_FOLDER_SIZE);
	pEdit->EnableWindow(bEnable);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_MIN_DISK_FREE_PERCENT);
	pEdit->EnableWindow(bEnable);
	pCheck = (CButton*)GetDlgItem(IDC_CHECK_SENDMAIL_MALFUNCTION);
	pCheck->EnableWindow(bEnable);
	pCheck = (CButton*)GetDlgItem(IDC_CHECK_SENDMAIL_SNAPSHOT);
	pCheck->EnableWindow(bEnable);
	pCheck = (CButton*)GetDlgItem(IDC_CHECK_SENDMAIL_SNAPSHOT_HISTORY);
	pCheck->EnableWindow(bEnable);
	pCheck = (CButton*)GetDlgItem(IDC_CHECK_SENDMAIL_RECORDING);
	pCheck->EnableWindow(bEnable);
	pComboBox = (CComboBox*)GetDlgItem(IDC_ATTACHMENT);
	pComboBox->EnableWindow(bEnable);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_SENDMAIL_SEC_BETWEEN_MSG);
	pEdit->EnableWindow(bEnable);
	CButton* pButton = (CButton*)GetDlgItem(IDC_SENDMAIL_CONFIGURE);
	pButton->EnableWindow(bEnable);
	pButton = (CButton*)GetDlgItem(IDOK);
	pButton->EnableWindow(bEnable);
	pButton = (CButton*)GetDlgItem(IDCANCEL);
	pButton->EnableWindow(bEnable);
}

void CCameraBasicSettingsDlg::OnTimer(UINT nIDEvent) 
{
	if (m_bDoApplySettings)
	{
		// Apply settings if we are not inside the processing function
		m_pDoc->StopProcessFrame(PROCESSFRAME_CAMERABASICSETTINGS);
		if (m_nRetryTimeMs > PROCESSFRAME_MAX_RETRY_TIME || m_pDoc->IsProcessFrameStopped(PROCESSFRAME_CAMERABASICSETTINGS))
			ApplySettings();
		else
			m_nRetryTimeMs += CAMERABASICSETTINGSDLG_TIMER_MS;
	}
	CDialog::OnTimer(nIDEvent);
}

BOOL CCameraBasicSettingsDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
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

void CCameraBasicSettingsDlg::OnOK() 
{
	// Begin wait cursor
	BeginWaitCursor();

	// Set vars
	m_bDoApplySettings = TRUE;
	m_nRetryTimeMs = 0;

	// Disable all
	EnableDisableAllCtrls(FALSE);
}

void CCameraBasicSettingsDlg::Rename()
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
	
	// Adjust old dir name
	m_pDoc->m_sRecordAutoSaveDir.TrimRight(_T('\\'));

	// Make new dir name
	CString sNewRecordAutoSaveDir = m_pDoc->m_sRecordAutoSaveDir;
	int index;
	if ((index = sNewRecordAutoSaveDir.ReverseFind(_T('\\'))) >= 0)
		sNewRecordAutoSaveDir = sNewRecordAutoSaveDir.Left(index);
	sNewRecordAutoSaveDir += _T('\\') + m_sName;

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

	// Merge or move
	DWORD dwLastError = ERROR_SUCCESS;
	if (::IsExistingDir(m_pDoc->m_sRecordAutoSaveDir) &&
		sNewRecordAutoSaveDir.CompareNoCase(m_pDoc->m_sRecordAutoSaveDir) != 0)
	{
		if (::IsExistingDir(sNewRecordAutoSaveDir) || !::MoveFile(m_pDoc->m_sRecordAutoSaveDir, sNewRecordAutoSaveDir))
		{
			if (!::MergeDirContent(m_pDoc->m_sRecordAutoSaveDir, sNewRecordAutoSaveDir)) // overwrite existing
				dwLastError = ::GetLastError();
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
			dwLastError = ::GetLastError();
	}
	// Create if nothing exists
	else if (!::IsExistingDir(sNewRecordAutoSaveDir) && !::CreateDir(sNewRecordAutoSaveDir))
		dwLastError = ::GetLastError();

	// OK?
	if (dwLastError == ERROR_SUCCESS)
		m_pDoc->m_sRecordAutoSaveDir = sNewRecordAutoSaveDir;
	else
	{
		// Error Message
		EndWaitCursor();
		::ShowErrorMsg(dwLastError, TRUE);
		BeginWaitCursor();

		// Restore old name
		m_sName = sOldName;
	}
}

void CCameraBasicSettingsDlg::OnSendmailConfigure()
{
	CSendMailConfigurationDlg dlg(m_pDoc->GetAssignedDeviceName());
	dlg.m_SendMailConfiguration = m_CurrentSendMailConfiguration;
	if (dlg.DoModal() == IDOK)
		m_CurrentSendMailConfiguration = dlg.m_SendMailConfiguration;
}

void CCameraBasicSettingsDlg::ApplySettingsSnapshot(int nThumbWidth, int nThumbHeight, double dSnapshotRate)
{
	m_pDoc->SnapshotRate(dSnapshotRate);
	if (m_pDoc->m_pCameraAdvancedSettingsDlg)
	{
		// Thumb size (this updates the controls and sets m_nSnapshotThumbWidth and m_nSnapshotThumbHeight)
		m_pDoc->m_pCameraAdvancedSettingsDlg->ChangeThumbSize(nThumbWidth, nThumbHeight);
		
		// Display snapshot rate
		m_pDoc->m_pCameraAdvancedSettingsDlg->DisplaySnapshotRate();

		// Snapshot history
		CButton* pCheck = (CButton*)m_pDoc->m_pCameraAdvancedSettingsDlg->GetDlgItem(IDC_CHECK_SNAPSHOT_HISTORY_VIDEO);
		pCheck->SetCheck(m_pDoc->m_bSnapshotHistoryVideo ? 1 : 0);
	}
	else
	{
		// Thumb size
		m_pDoc->m_nSnapshotThumbWidth = CVideoDeviceDoc::MakeSizeMultipleOf4(nThumbWidth);
		m_pDoc->m_nSnapshotThumbHeight = CVideoDeviceDoc::MakeSizeMultipleOf4(nThumbHeight);
	}
}

void CCameraBasicSettingsDlg::ApplySettings()
{
	// Reset vars
	m_bDoApplySettings = FALSE;
	m_nRetryTimeMs = 0;

	// Update data -> view to vars
	UpdateData(TRUE);

	// Disable recording
	BOOL bDoRecording;
	if (m_pDoc->m_dwVideoProcessorMode)
	{
		bDoRecording = TRUE;
		m_pDoc->m_dwVideoProcessorMode = 0;
		::AfxGetApp()->WriteProfileInt(m_pDoc->GetDevicePathName(), _T("VideoProcessorMode"), m_pDoc->m_dwVideoProcessorMode);
	}
	else
		bDoRecording = FALSE;
	m_pDoc->m_SaveFrameListThread.Kill();
	m_pDoc->OneEmptyFrameList();
	m_pDoc->FreeMovementDetector();

	// Stop watchdog thread
	m_pDoc->m_WatchdogThread.Kill();

	// Stop delete thread
	m_pDoc->m_DeleteThread.Kill();

	// Stop audio
	BOOL bDoCaptureAudio;
	if (m_pDoc->m_bCaptureAudio)
	{
		bDoCaptureAudio = TRUE;
		m_pDoc->m_bCaptureAudio = FALSE;
		if (!m_pDoc->m_bCaptureAudioFromStream)
			m_pDoc->m_CaptureAudioThread.Kill();
	}
	else
		bDoCaptureAudio = FALSE;

	// Make sure snapshot threads are stopped
	// (at this point the process frame is stopped but
	// the snapshot threads may still be running)
	m_pDoc->m_SaveSnapshotThread.Kill();
	m_pDoc->m_SaveSnapshotVideoThread.Kill();

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
		m_pDoc->PhpConfigFileSetParam(PHPCONFIG_LANGUAGEFILEPATH, CString(PHP_LANGUAGES_DIR) + _T("/") + sLanguageName + _T(".php"));

	// Style
	CString sStyleName;
	pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_STYLE);
	if (pComboBox && pComboBox->GetCurSel() >= 0)
		pComboBox->GetLBText(pComboBox->GetCurSel(), sStyleName);
	if (sStyleName != _T(""))
		m_pDoc->PhpConfigFileSetParam(PHPCONFIG_STYLEFILEPATH, CString(CSS_STYLE_DIR) + _T("/") + sStyleName + _T(".css"));

	// Thumbnails per page
	CString sMaxPerPage;
	pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_THUMBSPERPAGE);
	if (pComboBox && pComboBox->GetCurSel() >= 0)
		pComboBox->GetLBText(pComboBox->GetCurSel(), sMaxPerPage);
	if (sMaxPerPage != _T(""))
		m_pDoc->PhpConfigFileSetParam(PHPCONFIG_MAX_PER_PAGE, sMaxPerPage);
	
	// Camera commands
	if (m_bCheckCameraCommands)
		m_pDoc->PhpConfigFileSetParam(PHPCONFIG_SHOW_CAMERA_COMMANDS, _T("1"));
	else
		m_pDoc->PhpConfigFileSetParam(PHPCONFIG_SHOW_CAMERA_COMMANDS, _T("0"));

	// Usage
	switch (m_nUsage)
	{
		case 0 :
		{
			// Enable recording
			bDoRecording = TRUE;
			if (m_pDoc->m_nDetectionLevel == 100)
				m_pDoc->m_nDetectionLevel = DEFAULT_MOVDET_LEVEL;
			if (m_pDoc->m_nMilliSecondsRecBeforeMovementBegin == 1000)
				m_pDoc->m_nMilliSecondsRecBeforeMovementBegin = DEFAULT_PRE_BUFFER_MSEC;
			if (m_pDoc->m_nMilliSecondsRecAfterMovementEnd == 1000)
				m_pDoc->m_nMilliSecondsRecAfterMovementEnd = DEFAULT_POST_BUFFER_MSEC;
			if (m_pDoc->m_nDetectionMinLengthMilliSeconds == 0)
				m_pDoc->m_nDetectionMinLengthMilliSeconds = MOVDET_MIN_LENGTH_MSEC;
			if (m_pDoc->m_pCameraAdvancedSettingsDlg)
			{
				m_pDoc->m_pCameraAdvancedSettingsDlg->m_nSecondsBeforeMovementBegin = m_pDoc->m_nMilliSecondsRecBeforeMovementBegin / 1000;
				m_pDoc->m_pCameraAdvancedSettingsDlg->m_nSecondsAfterMovementEnd = m_pDoc->m_nMilliSecondsRecAfterMovementEnd / 1000;
				m_pDoc->m_pCameraAdvancedSettingsDlg->m_nDetectionMinLengthSeconds = m_pDoc->m_nDetectionMinLengthMilliSeconds / 1000;
				m_pDoc->m_pCameraAdvancedSettingsDlg->UpdateData(FALSE); // update data from vars to view
			}

			// Init size vars
			CString sWidth, sHeight;
			sWidth.Format(_T("%d"), m_pDoc->m_DocRect.right);
			sHeight.Format(_T("%d"), m_pDoc->m_DocRect.bottom);

			// Init snapshot rate var
			double dCurrentSnapshotRate = (double)(m_pDoc->m_nSnapshotRate) + (double)(m_pDoc->m_nSnapshotRateMs) / 1000.0;
			double dSnapshotRate = dCurrentSnapshotRate < MIN_SNAPSHOT_RATE ? dCurrentSnapshotRate : MIN_SNAPSHOT_RATE;

			// Init thumb vars
			int nThumbWidth = (	m_pDoc->m_nSnapshotThumbWidth < 4 * DEFAULT_SNAPSHOT_THUMB_WIDTH / 3 &&
								m_pDoc->m_nSnapshotThumbWidth > 2 * DEFAULT_SNAPSHOT_THUMB_WIDTH / 3) ?
								m_pDoc->m_nSnapshotThumbWidth : DEFAULT_SNAPSHOT_THUMB_WIDTH;
			int nThumbHeight = (m_pDoc->m_nSnapshotThumbHeight < 4 * DEFAULT_SNAPSHOT_THUMB_HEIGHT / 3 &&
								m_pDoc->m_nSnapshotThumbHeight > 2 * DEFAULT_SNAPSHOT_THUMB_HEIGHT / 3) ?
								m_pDoc->m_nSnapshotThumbHeight : DEFAULT_SNAPSHOT_THUMB_HEIGHT;
			CString sThumbWidth, sThumbHeight;
			sThumbWidth.Format(_T("%d"), nThumbWidth);
			sThumbHeight.Format(_T("%d"), nThumbHeight);

			// Update configuration.php
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_DEFAULTPAGE, PHPCONFIG_SUMMARYSNAPSHOT_PHP);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_WIDTH, sWidth);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_HEIGHT, sHeight);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_THUMBWIDTH, sThumbWidth);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_THUMBHEIGHT, sThumbHeight);

			// Disable snapshot history
			m_pDoc->m_bSnapshotHistoryVideo = FALSE;

			// Update snapshot settings
			ApplySettingsSnapshot(nThumbWidth, nThumbHeight, dSnapshotRate);

			break;
		}
		case 1 :
		{
			// Disable recording
			bDoRecording = FALSE;
			if (m_pDoc->m_nDetectionLevel != 100)
				m_pDoc->m_nDetectionLevel = 100;
			if (m_pDoc->m_nMilliSecondsRecBeforeMovementBegin != 1000)
				m_pDoc->m_nMilliSecondsRecBeforeMovementBegin = 1000;
			if (m_pDoc->m_nMilliSecondsRecAfterMovementEnd != 1000)
				m_pDoc->m_nMilliSecondsRecAfterMovementEnd = 1000;
			if (m_pDoc->m_nDetectionMinLengthMilliSeconds != 0)
				m_pDoc->m_nDetectionMinLengthMilliSeconds = 0;
			if (m_pDoc->m_pCameraAdvancedSettingsDlg)
			{
				m_pDoc->m_pCameraAdvancedSettingsDlg->m_nSecondsBeforeMovementBegin = m_pDoc->m_nMilliSecondsRecBeforeMovementBegin / 1000;
				m_pDoc->m_pCameraAdvancedSettingsDlg->m_nSecondsAfterMovementEnd = m_pDoc->m_nMilliSecondsRecAfterMovementEnd / 1000;
				m_pDoc->m_pCameraAdvancedSettingsDlg->m_nDetectionMinLengthSeconds = m_pDoc->m_nDetectionMinLengthMilliSeconds / 1000;
				m_pDoc->m_pCameraAdvancedSettingsDlg->UpdateData(FALSE); // update data from vars to view
			}

			// Init size vars
			CString sWidth, sHeight;
			sWidth.Format(_T("%d"), m_pDoc->m_DocRect.right);
			sHeight.Format(_T("%d"), m_pDoc->m_DocRect.bottom);

			// Init snapshot rate var
			double dCurrentSnapshotRate = (double)(m_pDoc->m_nSnapshotRate) + (double)(m_pDoc->m_nSnapshotRateMs) / 1000.0;
			double dSnapshotRate = dCurrentSnapshotRate > MIN_SNAPSHOT_RATE ? dCurrentSnapshotRate : 300.0; // 5 Minutes

			// Init thumb vars
			int nThumbWidth = (	m_pDoc->m_nSnapshotThumbWidth < 4 * DEFAULT_SNAPSHOT_THUMB_WIDTH / 3 &&
								m_pDoc->m_nSnapshotThumbWidth > 2 * DEFAULT_SNAPSHOT_THUMB_WIDTH / 3) ?
								m_pDoc->m_nSnapshotThumbWidth : DEFAULT_SNAPSHOT_THUMB_WIDTH;
			int nThumbHeight = (m_pDoc->m_nSnapshotThumbHeight < 4 * DEFAULT_SNAPSHOT_THUMB_HEIGHT / 3 &&
								m_pDoc->m_nSnapshotThumbHeight > 2 * DEFAULT_SNAPSHOT_THUMB_HEIGHT / 3) ?
								m_pDoc->m_nSnapshotThumbHeight : DEFAULT_SNAPSHOT_THUMB_HEIGHT;
			CString sThumbWidth, sThumbHeight;
			sThumbWidth.Format(_T("%d"), nThumbWidth);
			sThumbHeight.Format(_T("%d"), nThumbHeight);

			// Update configuration.php
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_DEFAULTPAGE, PHPCONFIG_SNAPSHOTHISTORY_PHP);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_WIDTH, sWidth);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_HEIGHT, sHeight);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_THUMBWIDTH, sThumbWidth);
			m_pDoc->PhpConfigFileSetParam(PHPCONFIG_THUMBHEIGHT, sThumbHeight);

			// Enable snapshot history
			m_pDoc->m_bSnapshotHistoryVideo = TRUE;

			// Update snapshot settings
			ApplySettingsSnapshot(nThumbWidth, nThumbHeight, dSnapshotRate);

			break;
		}
		case 2 :
		{
			// Disable recording
			bDoRecording = FALSE;
			if (m_pDoc->m_nDetectionLevel != 100)
				m_pDoc->m_nDetectionLevel = 100;
			if (m_pDoc->m_nMilliSecondsRecBeforeMovementBegin != 1000)
				m_pDoc->m_nMilliSecondsRecBeforeMovementBegin = 1000;
			if (m_pDoc->m_nMilliSecondsRecAfterMovementEnd != 1000)
				m_pDoc->m_nMilliSecondsRecAfterMovementEnd = 1000;
			if (m_pDoc->m_nDetectionMinLengthMilliSeconds != 0)
				m_pDoc->m_nDetectionMinLengthMilliSeconds = 0;
			if (m_pDoc->m_pCameraAdvancedSettingsDlg)
			{
				m_pDoc->m_pCameraAdvancedSettingsDlg->m_nSecondsBeforeMovementBegin = m_pDoc->m_nMilliSecondsRecBeforeMovementBegin / 1000;
				m_pDoc->m_pCameraAdvancedSettingsDlg->m_nSecondsAfterMovementEnd = m_pDoc->m_nMilliSecondsRecAfterMovementEnd / 1000;
				m_pDoc->m_pCameraAdvancedSettingsDlg->m_nDetectionMinLengthSeconds = m_pDoc->m_nDetectionMinLengthMilliSeconds / 1000;
				m_pDoc->m_pCameraAdvancedSettingsDlg->UpdateData(FALSE); // update data from vars to view
			}

			// Init size vars
			CString sWidth, sHeight;
			sWidth.Format(_T("%d"), m_pDoc->m_DocRect.right);
			sHeight.Format(_T("%d"), m_pDoc->m_DocRect.bottom);

			// Init snapshot rate var
			double dCurrentSnapshotRate = (double)(m_pDoc->m_nSnapshotRate) + (double)(m_pDoc->m_nSnapshotRateMs) / 1000.0;
			double dSnapshotRate = dCurrentSnapshotRate < MIN_SNAPSHOT_RATE ? dCurrentSnapshotRate : MIN_SNAPSHOT_RATE;

			// Init thumb vars
			int nThumbWidth = (	m_pDoc->m_nSnapshotThumbWidth < 4 * DEFAULT_SNAPSHOT_THUMB_WIDTH / 3 &&
								m_pDoc->m_nSnapshotThumbWidth > 2 * DEFAULT_SNAPSHOT_THUMB_WIDTH / 3) ?
								m_pDoc->m_nSnapshotThumbWidth : DEFAULT_SNAPSHOT_THUMB_WIDTH;
			int nThumbHeight = (m_pDoc->m_nSnapshotThumbHeight < 4 * DEFAULT_SNAPSHOT_THUMB_HEIGHT / 3 &&
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

			// Disable snapshot history
			m_pDoc->m_bSnapshotHistoryVideo = FALSE;

			// Update snapshot settings
			ApplySettingsSnapshot(nThumbWidth, nThumbHeight, dSnapshotRate);

			break;
		}
		default :
			break;
	}

	// Keep files for
	switch (m_nComboKeepFor)
	{
		case 0  : m_pDoc->m_nDeleteRecordingsOlderThanDays = 1;   break;
		case 1  : m_pDoc->m_nDeleteRecordingsOlderThanDays = 2;   break;
		case 2  : m_pDoc->m_nDeleteRecordingsOlderThanDays = 3;   break;
		case 3  : m_pDoc->m_nDeleteRecordingsOlderThanDays = 7;   break;
		case 4  : m_pDoc->m_nDeleteRecordingsOlderThanDays = 14;  break;
		case 5  : m_pDoc->m_nDeleteRecordingsOlderThanDays = 21;  break;
		case 6  : m_pDoc->m_nDeleteRecordingsOlderThanDays = 31;  break;
		case 7  : m_pDoc->m_nDeleteRecordingsOlderThanDays = 62;  break;
		case 8  : m_pDoc->m_nDeleteRecordingsOlderThanDays = 93;  break;
		case 9  : m_pDoc->m_nDeleteRecordingsOlderThanDays = 183; break;
		case 10 : m_pDoc->m_nDeleteRecordingsOlderThanDays = 366; break;
		default : m_pDoc->m_nDeleteRecordingsOlderThanDays = 0;   break;
	}

	// Maximum camera folder size
	double dMaxCameraFolderSizeGB = _tcstod(m_sMaxCameraFolderSizeGB.GetBuffer(0), NULL);
	m_sMaxCameraFolderSizeGB.ReleaseBuffer();
	m_pDoc->m_nMaxCameraFolderSizeMB = Round(dMaxCameraFolderSizeGB * 1024.0);
	if (m_pDoc->m_nMaxCameraFolderSizeMB < 0)
		m_pDoc->m_nMaxCameraFolderSizeMB = 0;

	// Minimum disk free size
	double dMinDiskFreePercent = _tcstod(m_sMinDiskFreePercent.GetBuffer(0), NULL);
	m_sMinDiskFreePercent.ReleaseBuffer();
	m_pDoc->m_nMinDiskFreePermillion = Round(dMinDiskFreePercent * 10000.0);
	if (m_pDoc->m_nMinDiskFreePermillion < 0)
		m_pDoc->m_nMinDiskFreePermillion = 0;
	else if (m_pDoc->m_nMinDiskFreePermillion > 1000000)
		m_pDoc->m_nMinDiskFreePermillion = 1000000;

	// Update send mail variables
	m_pDoc->m_bSendMailMalfunction = m_bCheckSendMailMalfunction;
	m_pDoc->m_bSendMailRecording = m_bCheckSendMailRecording;
	m_pDoc->m_bSendMailSnapshot = m_bCheckSendMailSnapshot;
	m_pDoc->m_bSendMailSnapshotHistory = m_bCheckSendMailSnapshotHistory;
	m_pDoc->m_AttachmentType = (CVideoDeviceDoc::AttachmentType)m_nComboSendMailAttachment;
	m_pDoc->m_nMovDetSendMailSecBetweenMsg = m_nSendMailSecBetweenMsg;
	m_pDoc->m_SendMailConfiguration = m_CurrentSendMailConfiguration;

	// Restart audio
	if (bDoCaptureAudio)
	{
		m_pDoc->m_bCaptureAudio = TRUE;
		if (!m_pDoc->m_bCaptureAudioFromStream)
			m_pDoc->m_CaptureAudioThread.Start();
	}

	// Restart delete thread
	m_pDoc->m_DeleteThread.Start(THREAD_PRIORITY_LOWEST);

	// Restart watchdog thread
	m_pDoc->m_WatchdogThread.Start();

	// Do recording?
	if (bDoRecording)
	{
		m_pDoc->m_dwVideoProcessorMode = 1;
		::AfxGetApp()->WriteProfileInt(m_pDoc->GetDevicePathName(), _T("VideoProcessorMode"), m_pDoc->m_dwVideoProcessorMode);
	}
	m_pDoc->m_SaveFrameListThread.Start();

	// Restart process frame
	m_pDoc->StartProcessFrame(PROCESSFRAME_CAMERABASICSETTINGS);

	// Autorun
	if (m_bAutorun)
		CVideoDeviceDoc::AutorunAddDevice(m_pDoc->GetDevicePathName());
	else
		CVideoDeviceDoc::AutorunRemoveDevice(m_pDoc->GetDevicePathName());

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
