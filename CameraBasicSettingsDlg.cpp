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
	DDX_Check(pDX, IDC_CHECK_SNAPSHOT_HISTORY_VIDEO, m_bSnapshotHistoryVideo);
	DDX_Text(pDX, IDC_EDIT_SNAPSHOT_HISTORY_RATE, m_nSnapshotHistoryRate);
	DDV_MinMaxInt(pDX, m_nSnapshotHistoryRate, 1, INT_MAX);
	DDX_Check(pDX, IDC_CHECK_AUTORUN, m_bAutorun);
	DDX_Check(pDX, IDC_CHECK_FULL_STRETCH, m_bCheckFullStretch);
	DDX_Check(pDX, IDC_CHECK_TRASHCOMMAND, m_bCheckTrashCommand);
	DDX_Check(pDX, IDC_CHECK_CAMERACOMMANDS, m_bCheckCameraCommands);
	DDX_Check(pDX, IDC_CHECK_SENDMAIL_MALFUNCTION, m_bCheckSendMailMalfunction);
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
		pComboBox->AddString(ML_STRING(1880, "No attachment"));
		pComboBox->AddString(ML_STRING(1883, "Saved full video"));
		pComboBox->AddString(ML_STRING(1882, "Saved animated thumbnail"));
		pComboBox->AddString(ML_STRING(1881, "Saved start picture"));
		pComboBox->AddString(ML_STRING(1881, "Saved start picture") + _T(" + ") + ML_STRING(1883, "Saved full video"));
		pComboBox->AddString(ML_STRING(1881, "Saved start picture") + _T(" + ") + ML_STRING(1882, "Saved animated thumbnail"));
	}

	// Init vars
	m_bDoApplySettings = FALSE;
	m_nRetryTimeMs = 0;
	m_sName = m_pDoc->GetAssignedDeviceName();
	m_bCheckFullStretch = (m_pDoc->PhpConfigFileGetParam(PHPCONFIG_FULL_STRETCH) == _T("1"));
	m_bCheckTrashCommand = (m_pDoc->PhpConfigFileGetParam(PHPCONFIG_SHOW_TRASH_COMMAND) == _T("1"));
	m_bCheckCameraCommands = (m_pDoc->PhpConfigFileGetParam(PHPCONFIG_SHOW_CAMERA_COMMANDS) == _T("1"));
	m_bSnapshotHistoryVideo = m_pDoc->m_bSnapshotHistoryVideo;
	m_nSnapshotHistoryRate = m_pDoc->m_nSnapshotHistoryRate;
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
	CWnd* pwndChild = GetWindow(GW_CHILD);
	while (pwndChild)
	{
		// Note: close X button, Esc and Alt+F4 are working according to the
		//       IDCANCEL button state which is Enabled/Disabled in this loop
		TCHAR szClassName[8]; // one extra char to make sure it is exactly "Static"
		::GetClassName(pwndChild->GetSafeHwnd(), szClassName, 8);
		szClassName[7] = _T('\0');
		if (_tcsicmp(szClassName, _T("Static")) != 0)
			pwndChild->EnableWindow(bEnable);
		pwndChild = pwndChild->GetNextWindow();
	}
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
	// Update data -> view to vars
	if (UpdateData(TRUE))
	{
		// Begin wait cursor
		BeginWaitCursor();

		// Set vars
		m_bDoApplySettings = TRUE;
		m_nRetryTimeMs = 0;

		// Disable all
		EnableDisableAllCtrls(FALSE);
	}
}

void CCameraBasicSettingsDlg::Rename()
{
	// Store current name
	CString sOldName = m_pDoc->GetAssignedDeviceName();

	// Adjust new name
	m_sName = CVideoDeviceDoc::GetValidName(m_sName);
	m_sName.TrimLeft();
	m_sName.TrimRight();
	if (m_sName == _T(""))
		m_sName = CVideoDeviceDoc::GetValidName(m_pDoc->GetDeviceName());
	
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

void CCameraBasicSettingsDlg::ApplySettings()
{
	// Reset vars
	m_bDoApplySettings = FALSE;
	m_nRetryTimeMs = 0;

	// Disable saving
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
	m_pDoc->m_SaveSnapshotVideoThread.Kill();
	m_pDoc->m_SaveSnapshotHistoryThread.Kill();
	m_pDoc->m_SaveSnapshotThread.Kill();
	
	// Rename
	Rename();

	// Title
	m_pDoc->PhpConfigFileSetParam(PHPCONFIG_SUMMARYTITLE, m_sName);
	m_pDoc->PhpConfigFileSetParam(PHPCONFIG_SNAPSHOTTITLE, m_sName);

	// Make sure default page is PHPCONFIG_SUMMARYSNAPSHOT_PHP
	m_pDoc->PhpConfigFileSetParam(PHPCONFIG_DEFAULTPAGE, PHPCONFIG_SUMMARYSNAPSHOT_PHP);

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
	
	// Full stretch
	if (m_bCheckFullStretch)
		m_pDoc->PhpConfigFileSetParam(PHPCONFIG_FULL_STRETCH, _T("1"));
	else
		m_pDoc->PhpConfigFileSetParam(PHPCONFIG_FULL_STRETCH, _T("0"));

	// Trash command
	if (m_bCheckTrashCommand)
		m_pDoc->PhpConfigFileSetParam(PHPCONFIG_SHOW_TRASH_COMMAND, _T("1"));
	else
		m_pDoc->PhpConfigFileSetParam(PHPCONFIG_SHOW_TRASH_COMMAND, _T("0"));

	// Camera commands
	if (m_bCheckCameraCommands)
		m_pDoc->PhpConfigFileSetParam(PHPCONFIG_SHOW_CAMERA_COMMANDS, _T("1"));
	else
		m_pDoc->PhpConfigFileSetParam(PHPCONFIG_SHOW_CAMERA_COMMANDS, _T("0"));

	// Summary video
	m_pDoc->m_bSnapshotHistoryVideo = m_bSnapshotHistoryVideo;
	m_pDoc->m_nSnapshotHistoryRate = m_nSnapshotHistoryRate;

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

	// Enable saving
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

	// Update data from vars to view because m_sName may have been changed by the above Rename()
	UpdateData(FALSE);

	// Save Settings (BeginWaitCursor() / EndWaitCursor() called inside this function)
	m_pDoc->SaveSettings();

	// Update titles because of possible device name change
	m_pDoc->SetDocumentTitle();

	// This calls UpdateData(TRUE) -> view to vars
	CDialog::OnOK();
}

#endif
