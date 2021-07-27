// SettingsDlgVideoDeviceDoc.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "VideoDeviceDoc.h"
#include "sinstance.h"
#include "BrowseDlg.h"
#include "SettingsDlgVideoDeviceDoc.h"
#include "NoVistaFileDlg.h"
#include "GetDirContentSize.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef VIDEODEVICEDOC

/////////////////////////////////////////////////////////////////////////////
// CSettingsDlgVideoDeviceDoc dialog

CSettingsDlgVideoDeviceDoc::CSettingsDlgVideoDeviceDoc(CWnd* pParent /*=NULL*/)
	: CDialog(CSettingsDlgVideoDeviceDoc::IDD, pParent)
{
	// Global Settings
	m_bTopMost =		((CUImagerApp*)::AfxGetApp())->m_bTopMost;
	m_bTrayIcon =		((CUImagerApp*)::AfxGetApp())->m_bTrayIcon;
	m_bAutostart =		((CUImagerApp*)::AfxGetApp())->IsAutostart();
	m_bStartFromService = CUImagerApp::GetContaCamServiceState() > 0;
	m_nFirstStartDelay = ((CUImagerApp*)::AfxGetApp())->m_dwFirstStartDelayMs / 1000;
	m_bStartMicroApache = ((CUImagerApp*)::AfxGetApp())->m_bStartMicroApache;
	m_nMicroApachePort = ((CUImagerApp*)::AfxGetApp())->m_nMicroApachePort;
	m_nMicroApachePortSSL = ((CUImagerApp*)::AfxGetApp())->m_nMicroApachePortSSL;
	m_sMicroApacheUsername = ((CUImagerApp*)::AfxGetApp())->m_sMicroApacheUsername;
	m_sMicroApachePassword = ((CUImagerApp*)::AfxGetApp())->m_sMicroApachePassword;
	m_sMicroApacheCertFileSSL = ((CUImagerApp*)::AfxGetApp())->m_sMicroApacheCertFileSSL;
	m_sMicroApacheKeyFileSSL = ((CUImagerApp*)::AfxGetApp())->m_sMicroApacheKeyFileSSL;
	m_sMicroApacheDocRootOld = ((CUImagerApp*)::AfxGetApp())->m_sMicroApacheDocRoot;
	m_sMicroApacheDocRootOld = ::UNCPath(m_sMicroApacheDocRootOld);
	m_sMicroApacheDocRootOld.TrimRight(_T('\\'));
	m_sMicroApacheDocRoot = m_sMicroApacheDocRootOld;

	// Micro Apache directory old files count
	m_nMicroApacheDocRootOldFilesCount = 0;

	// Applying the settings
	m_bDoApplySettings = FALSE;
}

void CSettingsDlgVideoDeviceDoc::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSettingsDlgVideoDeviceDoc)
	DDX_Check(pDX, IDC_CHECK_TRAYICON, m_bTrayIcon);
	DDX_Check(pDX, IDC_CHECK_STARTWITH_WINDOWS, m_bAutostart);
	DDX_Check(pDX, IDC_CHECK_TOPMOST, m_bTopMost);
	DDX_Check(pDX, IDC_CHECK_WEBSERVER, m_bStartMicroApache);
	DDX_Text(pDX, IDC_EDIT_PORT, m_nMicroApachePort);
	DDV_MinMaxInt(pDX, m_nMicroApachePort, 0, 65535);
	DDX_Text(pDX, IDC_EDIT_PORT_SSL, m_nMicroApachePortSSL);
	DDV_MinMaxInt(pDX, m_nMicroApachePortSSL, 0, 65535);
	DDX_Text(pDX, IDC_AUTH_USERNAME, m_sMicroApacheUsername);
	DDX_Text(pDX, IDC_AUTH_PASSWORD, m_sMicroApachePassword);
	DDX_Text(pDX, IDC_EDIT_CERT_SSL, m_sMicroApacheCertFileSSL);
	DDX_Text(pDX, IDC_EDIT_KEY_SSL, m_sMicroApacheKeyFileSSL);
	DDX_Text(pDX, IDC_EDIT_DOCROOT, m_sMicroApacheDocRoot);
	DDX_Check(pDX, IDC_CHECK_STARTFROM_SERVICE, m_bStartFromService);
	DDX_Text(pDX, IDC_EDIT_FIRSTSTART_DELAY, m_nFirstStartDelay);
	DDV_MinMaxInt(pDX, m_nFirstStartDelay, 0, 600);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSettingsDlgVideoDeviceDoc, CDialog)
	//{{AFX_MSG_MAP(CSettingsDlgVideoDeviceDoc)
	ON_BN_CLICKED(IDC_BUTTON_DOCROOT, OnButtonDocRoot)
	ON_BN_CLICKED(IDC_BUTTON_CERT_SSL, OnButtonCertSsl)
	ON_BN_CLICKED(IDC_BUTTON_KEY_SSL, OnButtonKeySsl)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_HTTPD_CONF_EDITABLE, OnSyslinkHttpdConfEditable)
	ON_NOTIFY(NM_RETURN, IDC_SYSLINK_HTTPD_CONF_EDITABLE, OnSyslinkHttpdConfEditable)
	ON_WM_SETCURSOR()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSettingsDlgVideoDeviceDoc message handlers

void CSettingsDlgVideoDeviceDoc::ApplySettingsInit()
{
	CUImagerApp* pApp = (CUImagerApp*)::AfxGetApp();
	
	// Tray Icon
	pApp->m_bTrayIcon = m_bTrayIcon;
	::AfxGetMainFrame()->TrayIcon(m_bTrayIcon);

	// Autostart
	pApp->Autostart(m_bAutostart);

	// Service
	if (m_bStartFromService)
	{
		// Install
		if (CUImagerApp::GetContaCamServiceState() == CONTACAMSERVICE_NOTINSTALLED)
		{
			TCHAR szDrive[_MAX_DRIVE];
			TCHAR szDir[_MAX_DIR];
			TCHAR szProgramName[MAX_PATH];
			if (::GetModuleFileName(NULL, szProgramName, MAX_PATH) != 0)
			{
				_tsplitpath(szProgramName, szDrive, szDir, NULL, NULL);
				CString sContaCamServicePath = CString(szDrive) + CString(szDir) + SERVICENAME_EXT;
				::ShellExecute(NULL, _T("open"), sContaCamServicePath, _T("-i"), NULL, SW_SHOWNORMAL);
				pApp->m_bDoStartFromService = TRUE;
			}	
		}
	}
	else
	{
		// Uninstall
		if (CUImagerApp::GetContaCamServiceState()) // if CONTACAMSERVICE_RUNNING or CONTACAMSERVICE_NOTRUNNING
		{
			TCHAR szDrive[_MAX_DRIVE];
			TCHAR szDir[_MAX_DIR];
			TCHAR szProgramName[MAX_PATH];
			if (::GetModuleFileName(NULL, szProgramName, MAX_PATH) != 0)
			{
				_tsplitpath(szProgramName, szDrive, szDir, NULL, NULL);
				CString sContaCamServicePath = CString(szDrive) + CString(szDir) + SERVICENAME_EXT;
				HANDLE h = ::ExecApp(sContaCamServicePath, _T("-u"), _T(""), FALSE, FALSE, TRUE, CONTACAMSERVICE_TIMEOUT);
				if (h)
					CloseHandle(h);
			}
		}
	}

	// Top Most
	pApp->m_bTopMost = m_bTopMost;
	if (m_bTopMost)
		::AfxGetMainFrame()->SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	else
		::AfxGetMainFrame()->SetWindowPos(&wndNoTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	// Wait time before autostarting first device
	pApp->m_dwFirstStartDelayMs = 1000 * m_nFirstStartDelay;

	// Micro Apache
	if (pApp->m_bStartMicroApache != m_bStartMicroApache				||
		pApp->m_nMicroApachePort != m_nMicroApachePort					||
		pApp->m_nMicroApachePortSSL != m_nMicroApachePortSSL			||
		pApp->m_sMicroApacheUsername != m_sMicroApacheUsername			||
		pApp->m_sMicroApachePassword != m_sMicroApachePassword			||
		pApp->m_sMicroApacheCertFileSSL != m_sMicroApacheCertFileSSL	||
		pApp->m_sMicroApacheKeyFileSSL != m_sMicroApacheKeyFileSSL		||
		m_sMicroApacheDocRoot.CompareNoCase(m_sMicroApacheDocRootOld) != 0)
	{
		// Stop Micro Apache
		if (pApp->m_bStartMicroApache)
			CVideoDeviceDoc::MicroApacheShutdown(MICROAPACHE_TIMEOUT_MS);

		// Document root changed?
		if (m_sMicroApacheDocRoot.CompareNoCase(m_sMicroApacheDocRootOld) != 0)
		{
			// Update all RecordAutoSaveDir configuration entries
			CStringArray DevicePathNames;
			pApp->EnumConfiguredDevicePathNames(DevicePathNames);
			for (int i = 0 ; i < DevicePathNames.GetSize() ; i++)
			{
				CString sRecordAutoSaveDir = pApp->GetProfileString(DevicePathNames[i], _T("RecordAutoSaveDir"), _T(""));
				sRecordAutoSaveDir.TrimRight(_T('\\'));
				int index;
				if ((index = sRecordAutoSaveDir.ReverseFind(_T('\\'))) >= 0)
					sRecordAutoSaveDir = sRecordAutoSaveDir.Right(sRecordAutoSaveDir.GetLength() - index - 1);
				pApp->WriteProfileString(DevicePathNames[i], _T("RecordAutoSaveDir"), m_sMicroApacheDocRoot + _T("\\") + sRecordAutoSaveDir);
			}

			// Start Move Thread
			m_MoveDirThread.m_sFromDir = m_sMicroApacheDocRootOld;
			m_MoveDirThread.m_sToDir = m_sMicroApacheDocRoot;
			m_MoveDirThread.Start();
		}
	}
}

void CSettingsDlgVideoDeviceDoc::ApplySettingsEnd()
{
	CUImagerApp* pApp = (CUImagerApp*)::AfxGetApp();

	// Micro Apache
	if (pApp->m_bStartMicroApache != m_bStartMicroApache				||
		pApp->m_nMicroApachePort != m_nMicroApachePort					||
		pApp->m_nMicroApachePortSSL != m_nMicroApachePortSSL			||
		pApp->m_sMicroApacheUsername != m_sMicroApacheUsername			||
		pApp->m_sMicroApachePassword != m_sMicroApachePassword			||
		pApp->m_sMicroApacheCertFileSSL != m_sMicroApacheCertFileSSL	||
		pApp->m_sMicroApacheKeyFileSSL != m_sMicroApacheKeyFileSSL		||
		m_sMicroApacheDocRoot.CompareNoCase(m_sMicroApacheDocRootOld) != 0)
	{
		// Document root changed?
		if (m_sMicroApacheDocRoot.CompareNoCase(m_sMicroApacheDocRootOld) != 0)
		{
			// Move error?
			if (m_MoveDirThread.GetMoveDirLastError() != 0)
			{
				EndWaitCursor();
				::ShowErrorMsg(m_MoveDirThread.GetMoveDirLastError(), TRUE);
				BeginWaitCursor();
			}
		}

		// Update vars
		pApp->m_bStartMicroApache = m_bStartMicroApache;
		pApp->m_nMicroApachePort = m_nMicroApachePort;
		pApp->m_nMicroApachePortSSL = m_nMicroApachePortSSL;
		pApp->m_sMicroApacheUsername = m_sMicroApacheUsername;
		pApp->m_sMicroApachePassword = m_sMicroApachePassword;
		pApp->m_sMicroApacheCertFileSSL = m_sMicroApacheCertFileSSL;
		pApp->m_sMicroApacheKeyFileSSL = m_sMicroApacheKeyFileSSL;
		pApp->m_sMicroApacheDocRoot = m_sMicroApacheDocRoot;

		// Update / create doc root index.php and config file for microapache
		CVideoDeviceDoc::MicroApacheUpdateMainFiles();

		// Start Micro Apache
		if (m_bStartMicroApache && !CVideoDeviceDoc::MicroApacheStart(MICROAPACHE_TIMEOUT_MS))
		{
			EndWaitCursor();
			::AfxMessageBox(ML_STRING(1475, "Failed to start the web server, try other HTTP(S) ports and allow them in your firewall!"), MB_ICONSTOP);
			BeginWaitCursor();
		}
	}

	// Store settings
	pApp->WriteProfileInt(			_T("GeneralApp"),
									_T("TopMost"),
									m_bTopMost);
	pApp->WriteProfileInt(			_T("GeneralApp"),
									_T("TrayIcon"),
									m_bTrayIcon);
	pApp->WriteProfileInt(			_T("GeneralApp"),
									_T("FirstStartDelayMs"),
									1000 * m_nFirstStartDelay);
	pApp->WriteProfileInt(			_T("GeneralApp"),
									_T("StartMicroApache"),
									m_bStartMicroApache);
	pApp->WriteProfileInt(			_T("GeneralApp"),
									_T("MicroApachePort"),
									m_nMicroApachePort);
	pApp->WriteProfileInt(			_T("GeneralApp"),
									_T("MicroApachePortSSL"),
									m_nMicroApachePortSSL);
	pApp->WriteSecureProfileString(	_T("GeneralApp"),
									_T("MicroApacheUsernameExportable"),
									m_sMicroApacheUsername);
	pApp->WriteSecureProfileString(	_T("GeneralApp"),
									_T("MicroApachePasswordExportable"),
									m_sMicroApachePassword);
	pApp->WriteProfileString(		_T("GeneralApp"),
									_T("MicroApacheCertFileSSL"),
									m_sMicroApacheCertFileSSL);
	pApp->WriteProfileString(		_T("GeneralApp"),
									_T("MicroApacheKeyFileSSL"),
									m_sMicroApacheKeyFileSSL);
	pApp->WriteProfileString(		_T("GeneralApp"),
									_T("MicroApacheDocRoot"),
									m_sMicroApacheDocRoot);
}

void CSettingsDlgVideoDeviceDoc::OnTimer(UINT nIDEvent) 
{
	if (m_bDoApplySettings)
	{
		if (m_MoveDirThread.IsAlive())
		{
			CString sProgress;
			sProgress.Format(_T("%d / %d"), m_MoveDirThread.GetMoveDirFilesCount(), m_nMicroApacheDocRootOldFilesCount);
			SetWindowText(sProgress);
		}
		else
		{
			m_bDoApplySettings = FALSE;
			ApplySettingsEnd();
			::AfxGetMainFrame()->m_MDIClientWnd.Invalidate(); // redraw web server port
			EndWaitCursor();
			EndDialog(IDOK);
		}
	}
	CDialog::OnTimer(nIDEvent);
}

BOOL CSettingsDlgVideoDeviceDoc::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
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

void CSettingsDlgVideoDeviceDoc::OnOK() 
{
	// Validate
	if (!UpdateData(TRUE))
		return;

	// Begin wait cursor
	BeginWaitCursor();

	// Start to apply the settings
	ApplySettingsInit();
	m_bDoApplySettings = TRUE; // must be set after the ApplySettingsInit() call!

	// Disable all
	EnableDisableAllCtrls(FALSE);
}

void CSettingsDlgVideoDeviceDoc::EnableDisableAllCtrls(BOOL bEnable)
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

void CSettingsDlgVideoDeviceDoc::OnButtonDocRoot() 
{
	// Fail if there are running cameras
	if (((CUImagerApp*)::AfxGetApp())->AreVideoDeviceDocsOpen())
	{
		::AfxMessageBox(ML_STRING(1872, "Try again after closing all cameras"), MB_OK | MB_ICONERROR);
		return;
	}

	// Validate entered data
	if (!UpdateData(TRUE))
		return;

	// Calculate currently used size for all cameras
	BeginWaitCursor();
	m_nMicroApacheDocRootOldFilesCount = 0;
	ULONGLONG ullDirContentSize = ::GetDirContentSize(m_sMicroApacheDocRootOld, &m_nMicroApacheDocRootOldFilesCount).QuadPart;
	CString sMsg;
	if (ullDirContentSize >= (1024*1024*1024))
		sMsg.Format(CString(_T("(")) + ML_STRING(1873, "usage of all cameras") + _T(" %I64u ") + ML_STRING(1826, "GB") + CString(_T(")")), ullDirContentSize >> 30);
	else if (ullDirContentSize >= (1024*1024))
		sMsg.Format(CString(_T("(")) + ML_STRING(1873, "usage of all cameras") + _T(" %I64u ") + ML_STRING(1825, "MB") + CString(_T(")")), ullDirContentSize >> 20);
	else
		sMsg.Format(CString(_T("(")) + ML_STRING(1873, "usage of all cameras") + _T(" %I64u ") + ML_STRING(1243, "KB") + CString(_T(")")), ullDirContentSize >> 10);
	EndWaitCursor();

	// Pop-up browse for folder dialog
	CString sNewMicroApacheDocRoot = m_sMicroApacheDocRoot;
	CBrowseDlg dlg(	this,
					&sNewMicroApacheDocRoot,
					ML_STRING(1871, "Move all camera folders to selected directory") + _T("\n") + sMsg,
					TRUE);
	if (dlg.DoModal() == IDOK)
	{
		// If it's a valid drive mount path convert it to a UNC path which is also working in service mode
		sNewMicroApacheDocRoot = ::UNCPath(sNewMicroApacheDocRoot);

		// Trim trailing backslash
		sNewMicroApacheDocRoot.TrimRight(_T('\\'));

		// Fail if sNewMicroApacheDocRoot is a nested subdir of the old one
		if (::IsSubDir(m_sMicroApacheDocRootOld, sNewMicroApacheDocRoot))
		{
			::AfxMessageBox(ML_STRING(1870, "The new folder cannot be a subfolder of the old one"), MB_OK | MB_ICONERROR);
			return;
		}

		// Fail if sNewMicroApacheDocRoot is not empty
		// Note: PathIsDirectoryEmpty fails also if the folder only contains desktop.ini or Thumbs.db
		if (sNewMicroApacheDocRoot.CompareNoCase(m_sMicroApacheDocRootOld) != 0 &&
			!::PathIsDirectoryEmpty(sNewMicroApacheDocRoot))
		{
			::AfxMessageBox(ML_STRING(1869, "The chosen folder is not empty"), MB_OK | MB_ICONERROR);
			return;
		}

		// Fail if we do not have write access to the sNewMicroApacheDocRoot folder
		if (!::HasWriteAccess(::MakeTempFileName(sNewMicroApacheDocRoot, CString(APPNAME_NOEXT) + _T(".tmp"))))
		{
			sMsg.Format(ML_STRING(1251, "Access denied to %s"), sNewMicroApacheDocRoot);
			::AfxMessageBox(sMsg, MB_OK | MB_ICONERROR);
			return;
		}

		// Fail if sNewMicroApacheDocRoot is not an ASCII path
		if (!::IsASCIICompatiblePath(sNewMicroApacheDocRoot))
		{
			::AfxMessageBox(ML_STRING(1766, "Only letters, numbers or spaces are allowed"), MB_OK | MB_ICONERROR);
			return;
		}

		// Update displayed path
		m_sMicroApacheDocRoot = sNewMicroApacheDocRoot;
		UpdateData(FALSE);
	}
}

void CSettingsDlgVideoDeviceDoc::OnButtonCertSsl()
{
	// Validate entered data
	if (!UpdateData(TRUE))
		return;

	CNoVistaFileDlg fd(	TRUE,
						_T("crt"),
						m_sMicroApacheCertFileSSL,
						OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, // file must exist and hide the read-only check box
						_T("Cert Files (*.crt)|*.crt||"));
	if (fd.DoModal() == IDOK)
	{
		m_sMicroApacheCertFileSSL = fd.GetPathName();
		UpdateData(FALSE);
	}
}

void CSettingsDlgVideoDeviceDoc::OnButtonKeySsl()
{
	// Validate entered data
	if (!UpdateData(TRUE))
		return;

	CNoVistaFileDlg fd(	TRUE,
						_T("key"),
						m_sMicroApacheKeyFileSSL,
						OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, // file must exist and hide the read-only check box
						_T("Key Files (*.key)|*.key||"));
	if (fd.DoModal() == IDOK)
	{
		m_sMicroApacheKeyFileSSL = fd.GetPathName();
		UpdateData(FALSE);
	}
}

void CSettingsDlgVideoDeviceDoc::OnSyslinkHttpdConfEditable(NMHDR* pNMHDR, LRESULT* pResult)
{
	::ShellExecute(NULL, _T("open"), CUImagerApp::GetConfigFilesDir(), NULL, NULL, SW_SHOWNORMAL);
	*pResult = 0;
}

BOOL CSettingsDlgVideoDeviceDoc::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Shield Icon on OK Button
	CButton* pOK = (CButton*)GetDlgItem(IDOK);
	pOK->SendMessage(BCM_SETSHIELD, 0, TRUE);

	// Init timer
	SetTimer(ID_TIMER_SETTINGSDLG, SETTINGSDLG_TIMER_MS, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSettingsDlgVideoDeviceDoc::OnDestroy() 
{
	// Kill timer
	KillTimer(ID_TIMER_SETTINGSDLG);

	// Base class
	CDialog::OnDestroy();
}

#endif
