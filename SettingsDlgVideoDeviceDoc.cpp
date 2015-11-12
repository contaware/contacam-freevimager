// SettingsDlgVideoDeviceDoc.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "VideoDeviceDoc.h"
#include "sinstance.h"
#include "BrowseDlg.h"
#include "SettingsDlgVideoDeviceDoc.h"

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
	//{{AFX_DATA_INIT(CSettingsDlgVideoDeviceDoc)
	//}}AFX_DATA_INIT

	// Global Settings
	m_bTopMost =		((CUImagerApp*)::AfxGetApp())->m_bTopMost;
	m_bTrayIcon =		((CUImagerApp*)::AfxGetApp())->m_bTrayIcon;
	m_bAutostart =		((CUImagerApp*)::AfxGetApp())->IsAutostart();
	m_bStartFromService = CUImagerApp::GetContaCamServiceState() > 0;
	m_bBrowserAutostart = ((CUImagerApp*)::AfxGetApp())->m_bBrowserAutostart;
	m_bIPv6 = ((CUImagerApp*)::AfxGetApp())->m_bIPv6;
	m_nAutostartDelay = ((CUImagerApp*)::AfxGetApp())->m_dwAutostartDelayMs / 1000;
	m_nFirstStartDelay = ((CUImagerApp*)::AfxGetApp())->m_dwFirstStartDelayMs / 1000;
	m_bStartMicroApache = ((CUImagerApp*)::AfxGetApp())->m_bStartMicroApache;
	m_nMicroApachePort = ((CUImagerApp*)::AfxGetApp())->m_nMicroApachePort;
	m_bMicroApacheDigestAuth = ((CUImagerApp*)::AfxGetApp())->m_bMicroApacheDigestAuth;
	m_sMicroApacheAreaname = ((CUImagerApp*)::AfxGetApp())->m_sMicroApacheAreaname;;
	m_sMicroApacheUsername = ((CUImagerApp*)::AfxGetApp())->m_sMicroApacheUsername;
	m_sMicroApachePassword = ((CUImagerApp*)::AfxGetApp())->m_sMicroApachePassword;
	m_sMicroApacheDocRootOld = ((CUImagerApp*)::AfxGetApp())->m_sMicroApacheDocRoot;
	m_sMicroApacheDocRootOld = ::UNCPath(m_sMicroApacheDocRootOld);
	m_sMicroApacheDocRootOld.TrimRight(_T('\\'));
	m_sMicroApacheDocRoot = m_sMicroApacheDocRootOld;

	// For validating apache user name
	m_bRejectingApacheUsernameChange = FALSE;
	m_sLastValidApacheUsername = m_sMicroApacheUsername;
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
	DDX_Text(pDX, IDC_AUTH_AREANAME, m_sMicroApacheAreaname);
	DDX_Text(pDX, IDC_AUTH_USERNAME, m_sMicroApacheUsername);
	DDX_Text(pDX, IDC_AUTH_PASSWORD, m_sMicroApachePassword);
	DDX_Text(pDX, IDC_EDIT_DOCROOT, m_sMicroApacheDocRoot);
	DDX_Check(pDX, IDC_CHECK_BROWSER_AUTOSTART, m_bBrowserAutostart);
	DDX_Check(pDX, IDC_CHECK_STARTFROM_SERVICE, m_bStartFromService);
	DDX_Check(pDX, IDC_CHECK_DIGESTAUTH, m_bMicroApacheDigestAuth);
	DDX_Check(pDX, IDC_CHECK_IPV6, m_bIPv6);
	DDX_Text(pDX, IDC_EDIT_AUTOSTART_DELAY, m_nAutostartDelay);
	DDV_MinMaxInt(pDX, m_nAutostartDelay, 0, 600);
	DDX_Text(pDX, IDC_EDIT_FIRSTSTART_DELAY, m_nFirstStartDelay);
	DDV_MinMaxInt(pDX, m_nFirstStartDelay, 0, 600);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSettingsDlgVideoDeviceDoc, CDialog)
	//{{AFX_MSG_MAP(CSettingsDlgVideoDeviceDoc)
	ON_EN_UPDATE(IDC_AUTH_USERNAME, OnUpdateAuthUsername)
	ON_BN_CLICKED(IDC_BUTTON_DOCROOT, OnButtonDocRoot)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSettingsDlgVideoDeviceDoc message handlers

void CSettingsDlgVideoDeviceDoc::OnOK() 
{
	// Validate
	if (!UpdateData(TRUE))
		return;

	CUImagerApp* pApp = (CUImagerApp*)::AfxGetApp();

	BeginWaitCursor();

	// Unassociate Graphics Files (remove associations from older program versions)
	pApp->UnassociateFileType(_T("bmp"));
	pApp->UnassociateFileType(_T("jpg")); pApp->UnassociateFileType(_T("jpeg")); pApp->UnassociateFileType(_T("jpe")); pApp->UnassociateFileType(_T("thm"));
	pApp->UnassociateFileType(_T("pcx"));
	pApp->UnassociateFileType(_T("emf"));
	pApp->UnassociateFileType(_T("png"));
	pApp->UnassociateFileType(_T("tif")); pApp->UnassociateFileType(_T("tiff")); pApp->UnassociateFileType(_T("jfx"));
	pApp->UnassociateFileType(_T("gif"));

	// Remove associations from older program versions
	pApp->UnassociateFileType(_T("aif")); pApp->UnassociateFileType(_T("aiff"));
	pApp->UnassociateFileType(_T("au"));
	pApp->UnassociateFileType(_T("mid")); pApp->UnassociateFileType(_T("rmi"));
	pApp->UnassociateFileType(_T("mp3"));
	pApp->UnassociateFileType(_T("wav"));
	pApp->UnassociateFileType(_T("wma"));
	pApp->UnassociateFileType(_T("cda"));
	pApp->UnassociateFileType(_T("avi")); pApp->UnassociateFileType(_T("divx"));
	pApp->UnassociateFileType(_T("zip"));

	// Notify Changes
	::SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
	
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
				::ExecHiddenApp(sContaCamServicePath, _T("-u"), TRUE, CONTACAMSERVICE_TIMEOUT);
			}
		}
	}

	// Top Most
	pApp->m_bTopMost = m_bTopMost;
	if (m_bTopMost)
		::AfxGetMainFrame()->SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	else
		::AfxGetMainFrame()->SetWindowPos(&wndNoTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	// Browser
	pApp->m_bBrowserAutostart = m_bBrowserAutostart;

	// Priority to IPv6
	pApp->m_bIPv6 = m_bIPv6;

	// Wait time between network devices start
	pApp->m_dwAutostartDelayMs = 1000 * m_nAutostartDelay;

	// Wait time before autostarting first device
	pApp->m_dwFirstStartDelayMs = 1000 * m_nFirstStartDelay;

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

		// Merge
		if (!::MergeDirContent(m_sMicroApacheDocRootOld, m_sMicroApacheDocRoot)) // overwrite existing
		{
			DWORD dwLastError = ::GetLastError();
			EndWaitCursor();
			::ShowError(dwLastError, TRUE);
			BeginWaitCursor();
		}
		else
			::DeleteDir(m_sMicroApacheDocRootOld); // no error message on failure
	}

	// Micro Apache
	if (m_sMicroApacheAreaname.IsEmpty())
		m_sMicroApacheAreaname = MICROAPACHE_DEFAULT_AUTH_AREANAME;
	if (pApp->m_bStartMicroApache != m_bStartMicroApache			||
		pApp->m_nMicroApachePort != m_nMicroApachePort				||
		pApp->m_bMicroApacheDigestAuth != m_bMicroApacheDigestAuth	||
		pApp->m_sMicroApacheAreaname != m_sMicroApacheAreaname		||
		pApp->m_sMicroApacheUsername != m_sMicroApacheUsername		||
		pApp->m_sMicroApachePassword != m_sMicroApachePassword		||
		m_sMicroApacheDocRoot.CompareNoCase(m_sMicroApacheDocRootOld) != 0)
	{
		// Stop Micro Apache
		if (pApp->m_bStartMicroApache && !CVideoDeviceDoc::MicroApacheShutdown(MICROAPACHE_TIMEOUT_MS))
		{
			EndWaitCursor();
			::AfxMessageBox(ML_STRING(1474, "Failed to stop the web server"),
							MB_ICONSTOP);
			BeginWaitCursor();
		}

		// Update vars
		pApp->m_bStartMicroApache = m_bStartMicroApache;
		pApp->m_nMicroApachePort = m_nMicroApachePort;
		pApp->m_bMicroApacheDigestAuth = m_bMicroApacheDigestAuth;
		pApp->m_sMicroApacheAreaname = m_sMicroApacheAreaname;
		pApp->m_sMicroApacheUsername = m_sMicroApacheUsername;
		pApp->m_sMicroApachePassword = m_sMicroApachePassword;
		pApp->m_sMicroApacheDocRoot = m_sMicroApacheDocRoot;

		// Update / create doc root index.php and config file for microapache
		CVideoDeviceDoc::MicroApacheUpdateMainFiles();

		// Start Micro Apache
		if (m_bStartMicroApache													&&
			!(CVideoDeviceDoc::MicroApacheInitStart()							&&
			CVideoDeviceDoc::MicroApacheWaitStartDone(MICROAPACHE_TIMEOUT_MS)	&&
			CVideoDeviceDoc::MicroApacheWaitCanConnect(MICROAPACHE_TIMEOUT_MS)))
		{
			EndWaitCursor();
			::AfxMessageBox(ML_STRING(1475, "Failed to start the web server") + _T("\n") + 
							ML_STRING(1476, "(change the Port number to an unused one)"),
							MB_ICONSTOP);
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
									_T("BrowserAutostart"),
									m_bBrowserAutostart);
	pApp->WriteProfileInt(			_T("GeneralApp"),
									_T("IPv6"),
									m_bIPv6);
	pApp->WriteProfileInt(			_T("GeneralApp"),
									_T("AutostartDelayMs"),
									1000 * m_nAutostartDelay);
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
									_T("MicroApacheDigestAuth"),
									m_bMicroApacheDigestAuth);
	pApp->WriteProfileString(		_T("GeneralApp"),
									_T("MicroApacheAreaname"),
									m_sMicroApacheAreaname);
	pApp->WriteSecureProfileString(	_T("GeneralApp"),
									_T("MicroApacheUsername"),
									m_sMicroApacheUsername);
	pApp->WriteSecureProfileString(	_T("GeneralApp"),
									_T("MicroApachePassword"),
									m_sMicroApachePassword);
	pApp->WriteProfileString(		_T("GeneralApp"),
									_T("MicroApacheDocRoot"),
									m_sMicroApacheDocRoot);

	// Redraw web server port
	::AfxGetMainFrame()->m_MDIClientWnd.Invalidate();

	EndWaitCursor();

	EndDialog(IDOK);
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
	ULONGLONG ullDirContentSize = ::GetDirContentSize(m_sMicroApacheDocRootOld).QuadPart;
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
	CBrowseDlg dlg(	::AfxGetMainFrame(),
					&sNewMicroApacheDocRoot,
					ML_STRING(1871, "Move all camera folders to selected directory") + _T("\n") + sMsg,
					TRUE);
	if (dlg.DoModal() == IDOK)
	{
		// If it's a valid drive mount path convert it to a UNC path which is also working in service mode
		sNewMicroApacheDocRoot = ::UNCPath(sNewMicroApacheDocRoot);

		// Trim trailing backslash
		sNewMicroApacheDocRoot.TrimRight(_T('\\'));

		// Fail if sNewMicroApacheDocRoot is the system drive
		CString sSysDrive;
		::GetWindowsDirectory(sSysDrive.GetBuffer(MAX_PATH), MAX_PATH);
		sSysDrive.ReleaseBuffer();
		sSysDrive = ::GetDriveName(sSysDrive);
		if (sSysDrive.CompareNoCase(sNewMicroApacheDocRoot) == 0)
		{
			sMsg.Format(ML_STRING(1869, "Choose a directory under the %s drive"), sSysDrive);
			::AfxMessageBox(sMsg, MB_OK | MB_ICONERROR);
			return;
		}

		// Fail if sNewMicroApacheDocRoot is not an ASCII path
		if (!::IsASCIICompatiblePath(sNewMicroApacheDocRoot))
		{
			::AfxMessageBox(ML_STRING(1766, "Only ASCII characters allowed in path"), MB_OK | MB_ICONERROR);
			return;
		}

		// Fail if sNewMicroApacheDocRoot is a nested subdir of the old one
		if (::IsSubDir(m_sMicroApacheDocRootOld, sNewMicroApacheDocRoot))
		{
			::AfxMessageBox(ML_STRING(1870, "The new folder cannot be a subfolder of the old one"), MB_OK | MB_ICONERROR);
			return;
		}

		// Update displayed path
		m_sMicroApacheDocRoot = sNewMicroApacheDocRoot;
		UpdateData(FALSE);
	}
}

BOOL CSettingsDlgVideoDeviceDoc::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Shield Icon on OK Button
	if (g_bWinVistaOrHigher)
	{
		CButton* pOK = (CButton*)GetDlgItem(IDOK);
		if (pOK)
			pOK->SendMessage(BCM_SETSHIELD, 0, TRUE);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSettingsDlgVideoDeviceDoc::OnUpdateAuthUsername() 
{
	if (!m_bRejectingApacheUsernameChange)
	{
		// Get new text which is not yet shown
		CString s;
		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_AUTH_USERNAME);
		pEdit->GetWindowText(s);
		
		// New text has invalid char?
		// Note: user cannot contain the ':' separator char
		// (password is the last field and thus it can contain ':')
		if (s.Find(_T(':')) >= 0)
		{
			// Get new caret position and calc. offset to restore old position
			int nStart, nEnd;
			pEdit->GetSel(nStart, nEnd);
			int nOffset = s.GetLength() - m_sLastValidApacheUsername.GetLength();
			
			// Restore previous text
			m_bRejectingApacheUsernameChange = TRUE;
			pEdit->SetWindowText(m_sLastValidApacheUsername); // this calls OnUpdateAuthUsername()
			m_bRejectingApacheUsernameChange = FALSE;
			
			// Restore previous caret position
			pEdit->SetSel(nStart - nOffset, nEnd - nOffset);

			// Alert sound
			::MessageBeep((DWORD)-1);
		}
		else
			m_sLastValidApacheUsername = s;
	}
}

#endif
