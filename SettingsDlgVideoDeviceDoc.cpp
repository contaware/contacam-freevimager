// SettingsDlgVideoDeviceDoc.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "VideoDeviceDoc.h"
#include "sinstance.h"
#include "IniFile.h"
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

	// Graphics
	m_bCheckBmp =	((CUImagerApp*)::AfxGetApp())->IsFileTypeAssociated(_T("bmp"));
	m_bCheckJpeg =	((CUImagerApp*)::AfxGetApp())->IsFileTypeAssociated(_T("jpg"))	&&
					((CUImagerApp*)::AfxGetApp())->IsFileTypeAssociated(_T("jpeg"))	&&
					((CUImagerApp*)::AfxGetApp())->IsFileTypeAssociated(_T("jpe"))	&&
					((CUImagerApp*)::AfxGetApp())->IsFileTypeAssociated(_T("thm"));
	m_bCheckPcx =	((CUImagerApp*)::AfxGetApp())->IsFileTypeAssociated(_T("pcx"));
	m_bCheckEmf =	((CUImagerApp*)::AfxGetApp())->IsFileTypeAssociated(_T("emf"));
	m_bCheckPng =	((CUImagerApp*)::AfxGetApp())->IsFileTypeAssociated(_T("png"));
	m_bCheckTiff =	((CUImagerApp*)::AfxGetApp())->IsFileTypeAssociated(_T("tif"))	&&
					((CUImagerApp*)::AfxGetApp())->IsFileTypeAssociated(_T("tiff"))	&&
					((CUImagerApp*)::AfxGetApp())->IsFileTypeAssociated(_T("jfx"));
	m_bCheckGif =	((CUImagerApp*)::AfxGetApp())->IsFileTypeAssociated(_T("gif"));

	// Audio
	m_bCheckAif =	((CUImagerApp*)::AfxGetApp())->IsFileTypeAssociated(_T("aif"))	&&
					((CUImagerApp*)::AfxGetApp())->IsFileTypeAssociated(_T("aiff"));
	m_bCheckAu =	((CUImagerApp*)::AfxGetApp())->IsFileTypeAssociated(_T("au"));
	m_bCheckMidi =	((CUImagerApp*)::AfxGetApp())->IsFileTypeAssociated(_T("mid"))	&&
					((CUImagerApp*)::AfxGetApp())->IsFileTypeAssociated(_T("rmi"));
	m_bCheckMp3 =	((CUImagerApp*)::AfxGetApp())->IsFileTypeAssociated(_T("mp3"));
	m_bCheckWav =	((CUImagerApp*)::AfxGetApp())->IsFileTypeAssociated(_T("wav"));
	m_bCheckWma =	((CUImagerApp*)::AfxGetApp())->IsFileTypeAssociated(_T("wma"));
	m_bCheckCda =	((CUImagerApp*)::AfxGetApp())->IsFileTypeAssociated(_T("cda"));

	// Others
	m_bCheckAvi =	((CUImagerApp*)::AfxGetApp())->IsFileTypeAssociated(_T("avi")) &&
					((CUImagerApp*)::AfxGetApp())->IsFileTypeAssociated(_T("divx"));
	m_bCheckZip =	((CUImagerApp*)::AfxGetApp())->IsFileTypeAssociated(_T("zip"));

	// Global Settings
	m_bTopMost =		((CUImagerApp*)::AfxGetApp())->m_bTopMost;
	m_bTrayIcon =		((CUImagerApp*)::AfxGetApp())->m_bTrayIcon;
	m_bAutostart =		((CUImagerApp*)::AfxGetApp())->IsAutostart();
	m_bStartFromService = CUImagerApp::GetContaCamServiceState() > 0;
	m_bEscExit =		((CUImagerApp*)::AfxGetApp())->m_bEscExit;
	m_bDisableExtProg = ((CUImagerApp*)::AfxGetApp())->m_bDisableExtProg;
	m_bFullscreenBrowser = ((CUImagerApp*)::AfxGetApp())->m_bFullscreenBrowser;
	m_bBrowserAutostart = ((CUImagerApp*)::AfxGetApp())->m_bBrowserAutostart;
	m_sFullscreenBrowserExitString = ((CUImagerApp*)::AfxGetApp())->m_sFullscreenBrowserExitString;
	m_bStartMicroApache = ((CUImagerApp*)::AfxGetApp())->m_bStartMicroApache;
	m_sMicroApacheDocRoot = ((CUImagerApp*)::AfxGetApp())->m_sMicroApacheDocRoot;
	m_nMicroApachePort = ((CUImagerApp*)::AfxGetApp())->m_nMicroApachePort;
	m_sMicroApacheUsername = ((CUImagerApp*)::AfxGetApp())->m_sMicroApacheUsername;
	m_sMicroApachePassword = ((CUImagerApp*)::AfxGetApp())->m_sMicroApachePassword;
}

void CSettingsDlgVideoDeviceDoc::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSettingsDlgVideoDeviceDoc)
	DDX_Check(pDX, IDC_CHECK_AVI, m_bCheckAvi);
	DDX_Check(pDX, IDC_CHECK_BMP, m_bCheckBmp);
	DDX_Check(pDX, IDC_CHECK_JPEG, m_bCheckJpeg);
	DDX_Check(pDX, IDC_CHECK_PCX, m_bCheckPcx);
	DDX_Check(pDX, IDC_CHECK_EMF, m_bCheckEmf);
	DDX_Check(pDX, IDC_CHECK_PNG, m_bCheckPng);
	DDX_Check(pDX, IDC_CHECK_TIFF, m_bCheckTiff);
	DDX_Check(pDX, IDC_CHECK_GIF, m_bCheckGif);
	DDX_Check(pDX, IDC_CHECK_ZIP, m_bCheckZip);
	DDX_Check(pDX, IDC_CHECK_AIF, m_bCheckAif);
	DDX_Check(pDX, IDC_CHECK_AU, m_bCheckAu);
	DDX_Check(pDX, IDC_CHECK_MIDI, m_bCheckMidi);
	DDX_Check(pDX, IDC_CHECK_MP3, m_bCheckMp3);
	DDX_Check(pDX, IDC_CHECK_WAV, m_bCheckWav);
	DDX_Check(pDX, IDC_CHECK_WMA, m_bCheckWma);
	DDX_Check(pDX, IDC_CHECK_CDA, m_bCheckCda);
	DDX_Check(pDX, IDC_CHECK_ESC_EXIT, m_bEscExit);
	DDX_Check(pDX, IDC_CHECK_TRAYICON, m_bTrayIcon);
	DDX_Check(pDX, IDC_CHECK_STARTWITH_WINDOWS, m_bAutostart);
	DDX_Check(pDX, IDC_CHECK_DISABLE_EXTPROG, m_bDisableExtProg);
	DDX_Check(pDX, IDC_CHECK_TOPMOST, m_bTopMost);
	DDX_Check(pDX, IDC_CHECK_WEBSERVER, m_bStartMicroApache);
	DDX_Text(pDX, IDC_WEBSERVER_ROOTDIR_PATH, m_sMicroApacheDocRoot);
	DDX_Text(pDX, IDC_EDIT_PORT, m_nMicroApachePort);
	DDV_MinMaxInt(pDX, m_nMicroApachePort, 0, 65535);
	DDX_Text(pDX, IDC_AUTH_PASSWORD, m_sMicroApachePassword);
	DDX_Text(pDX, IDC_AUTH_USERNAME, m_sMicroApacheUsername);
	DDX_Check(pDX, IDC_CHECK_FULLSCREENBROWSER, m_bFullscreenBrowser);
	DDX_Check(pDX, IDC_CHECK_BROWSER_AUTOSTART, m_bBrowserAutostart);
	DDX_Text(pDX, IDC_EDIT_FULLSCREENBROWSER_EXITSTRING, m_sFullscreenBrowserExitString);
	DDX_Check(pDX, IDC_CHECK_STARTFROM_SERVICE, m_bStartFromService);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSettingsDlgVideoDeviceDoc, CDialog)
	//{{AFX_MSG_MAP(CSettingsDlgVideoDeviceDoc)
	ON_BN_CLICKED(IDC_BUTTON_CLEARALL, OnButtonClearall)
	ON_BN_CLICKED(IDC_BUTTON_SETALL, OnButtonSetall)
	ON_BN_CLICKED(IDC_WEBSERVER_ROOTDIR, OnWebserverRootdir)
	ON_BN_CLICKED(IDC_CHECK_FULLSCREENBROWSER, OnCheckFullscreenbrowser)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSettingsDlgVideoDeviceDoc message handlers

void CSettingsDlgVideoDeviceDoc::OnCheckFullscreenbrowser() 
{
	// Enable / Disable Fullscreen Browser Exit password field
	CButton* pCheckFullscreenBrowser = (CButton*)GetDlgItem(IDC_CHECK_FULLSCREENBROWSER);
	CEdit* pEditFullscreenBrowserExitString = (CEdit*)GetDlgItem(IDC_EDIT_FULLSCREENBROWSER_EXITSTRING);
	CEdit* pLabelFullscreenBrowser = (CEdit*)GetDlgItem(IDC_LABEL_FULLSCREENBROWSER);
	if (pCheckFullscreenBrowser				&&
		pEditFullscreenBrowserExitString	&&
		pLabelFullscreenBrowser)
	{
		if (pCheckFullscreenBrowser->GetCheck())
		{
			pEditFullscreenBrowserExitString->EnableWindow(TRUE);
			pLabelFullscreenBrowser->EnableWindow(TRUE);
		}
		else
		{
			pEditFullscreenBrowserExitString->EnableWindow(FALSE);
			pLabelFullscreenBrowser->EnableWindow(FALSE);
		}
	}
}

void CSettingsDlgVideoDeviceDoc::OnButtonClearall() 
{
	UpdateData(TRUE);

	// Graphics
	m_bCheckBmp = FALSE;
	m_bCheckJpeg = FALSE;
	m_bCheckPcx = FALSE;
	m_bCheckEmf = FALSE;
	m_bCheckPng = FALSE;
	m_bCheckTiff = FALSE;
	m_bCheckGif = FALSE;
	
	// Audio
	m_bCheckAif = FALSE;
	m_bCheckAu = FALSE;
	m_bCheckMidi =  FALSE;	
	m_bCheckMp3 =  FALSE;
	m_bCheckWav =  FALSE;
	m_bCheckWma =  FALSE;
	m_bCheckCda =  FALSE;

	// Others
	m_bCheckAvi = FALSE;
	m_bCheckZip = FALSE;

	UpdateData(FALSE);
}

void CSettingsDlgVideoDeviceDoc::OnButtonSetall() 
{
	UpdateData(TRUE);

	// Graphics
	m_bCheckBmp = TRUE;
	m_bCheckJpeg = TRUE;
	m_bCheckPcx = TRUE;
	m_bCheckEmf = TRUE;
	m_bCheckPng = TRUE;
	m_bCheckTiff = TRUE;
	m_bCheckGif = TRUE;

	// Audio
	m_bCheckAif = TRUE;
	m_bCheckAu = TRUE;
	m_bCheckMidi = TRUE;	
	m_bCheckMp3 = TRUE;
	m_bCheckWav = TRUE;
	m_bCheckWma = TRUE;
	m_bCheckCda = TRUE;

	// Others
	m_bCheckAvi = TRUE;
	m_bCheckZip = TRUE;

	UpdateData(FALSE);
}

void CSettingsDlgVideoDeviceDoc::OnOK() 
{
	CDialog::OnOK(); // It calls UpdateData(TRUE) for us

	CUImagerApp* pApp = (CUImagerApp*)::AfxGetApp();

	BeginWaitCursor();

	// File type association
	if (pApp->m_bUseSettings)
	{
		// Graphics

		if (m_bCheckBmp)
			pApp->AssociateFileType(_T("bmp"));
		else
			pApp->UnassociateFileType(_T("bmp"));

		if (m_bCheckJpeg)
		{
			pApp->AssociateFileType(_T("jpg"));
			pApp->AssociateFileType(_T("jpeg"));
			pApp->AssociateFileType(_T("jpe"));
			pApp->AssociateFileType(_T("thm"));
		}
		else
		{
			pApp->UnassociateFileType(_T("jpg"));
			pApp->UnassociateFileType(_T("jpeg"));
			pApp->UnassociateFileType(_T("jpe"));
			pApp->UnassociateFileType(_T("thm"));
		}

		if (m_bCheckPcx)
			pApp->AssociateFileType(_T("pcx"));
		else
			pApp->UnassociateFileType(_T("pcx"));

		if (m_bCheckEmf)
			pApp->AssociateFileType(_T("emf"));
		else
			pApp->UnassociateFileType(_T("emf"));

		if (m_bCheckPng)
			pApp->AssociateFileType(_T("png"));
		else
			pApp->UnassociateFileType(_T("png"));

		if (m_bCheckTiff)
		{
			pApp->AssociateFileType(_T("tif"));
			pApp->AssociateFileType(_T("tiff"));
			pApp->AssociateFileType(_T("jfx"));
		}
		else
		{
			pApp->UnassociateFileType(_T("tif"));
			pApp->UnassociateFileType(_T("tiff"));
			pApp->UnassociateFileType(_T("jfx"));
		}

		if (m_bCheckGif)
			pApp->AssociateFileType(_T("gif"));
		else
			pApp->UnassociateFileType(_T("gif"));


		// Audio

		if (m_bCheckAif)
		{
			pApp->AssociateFileType(_T("aif"));
			pApp->AssociateFileType(_T("aiff"));
		}
		else
		{
			pApp->UnassociateFileType(_T("aif"));
			pApp->UnassociateFileType(_T("aiff"));
		}

		if (m_bCheckAu)
			pApp->AssociateFileType(_T("au"));
		else
			pApp->UnassociateFileType(_T("au"));

		if (m_bCheckMidi)
		{
			pApp->AssociateFileType(_T("mid"));
			pApp->AssociateFileType(_T("rmi"));
		}
		else
		{
			pApp->UnassociateFileType(_T("mid"));
			pApp->UnassociateFileType(_T("rmi"));
		}

		if (m_bCheckMp3)
			pApp->AssociateFileType(_T("mp3"));
		else
			pApp->UnassociateFileType(_T("mp3"));

		if (m_bCheckWav)
			pApp->AssociateFileType(_T("wav"));
		else
			pApp->UnassociateFileType(_T("wav"));

		if (m_bCheckWma)
			pApp->AssociateFileType(_T("wma"));
		else
			pApp->UnassociateFileType(_T("wma"));

		if (m_bCheckCda)
			pApp->AssociateFileType(_T("cda"));
		else
			pApp->UnassociateFileType(_T("cda"));

		// Others

		if (m_bCheckAvi)
		{
			pApp->AssociateFileType(_T("avi"));
			pApp->AssociateFileType(_T("divx"));
		}
		else
		{
			pApp->UnassociateFileType(_T("avi"));
			pApp->UnassociateFileType(_T("divx"));
		}

		if (m_bCheckZip)
			pApp->AssociateFileType(_T("zip"));
		else
			pApp->UnassociateFileType(_T("zip"));

		// Notify Changes
		::SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
	}
	
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

	// ESC to exit the program
	pApp->m_bEscExit = m_bEscExit;

	// Disable opening external program (for pdf, swf)
	pApp->m_bDisableExtProg = m_bDisableExtProg;

	// Browser
	pApp->m_bFullscreenBrowser = m_bFullscreenBrowser;
	pApp->m_bBrowserAutostart = m_bBrowserAutostart;
	pApp->m_sFullscreenBrowserExitString = m_sFullscreenBrowserExitString;

	// Micro Apache
	if (pApp->m_bStartMicroApache != m_bStartMicroApache		||
		pApp->m_sMicroApacheDocRoot != m_sMicroApacheDocRoot	||
		pApp->m_nMicroApachePort != m_nMicroApachePort			||
		pApp->m_sMicroApacheUsername != m_sMicroApacheUsername	||
		pApp->m_sMicroApachePassword != m_sMicroApachePassword)
	{
		// Update vars
		pApp->m_bStartMicroApache = m_bStartMicroApache;
		pApp->m_sMicroApacheDocRoot = m_sMicroApacheDocRoot;
		pApp->m_nMicroApachePort = m_nMicroApachePort;
		pApp->m_sMicroApacheUsername = m_sMicroApacheUsername;
		pApp->m_sMicroApachePassword = m_sMicroApachePassword;

		// Start stopping server
		((CUImagerApp*)::AfxGetApp())->m_bMicroApacheStarted = FALSE;
		((CUImagerApp*)::AfxGetApp())->m_MicroApacheWatchdogThread.Kill();
		CVideoDeviceDoc::MicroApacheInitShutdown();

		// Update / create config file and root index.php for microapache
		pApp->MicroApacheUpdateFiles();

		// Wait till shutdown
		if (!CVideoDeviceDoc::MicroApacheFinishShutdown())
			::AfxMessageBox(ML_STRING(1474, "Failed to stop the web server"), MB_ICONSTOP);
		else
		{
			// Start server
			if (m_bStartMicroApache)
			{
				((CUImagerApp*)::AfxGetApp())->m_bMicroApacheStarted = TRUE;
				if (!CVideoDeviceDoc::MicroApacheInitStart())
				{
					EndWaitCursor();
					::AfxMessageBox(ML_STRING(1475, "Failed to start the web server"), MB_ICONSTOP);
					BeginWaitCursor();
				}
				else if (!CVideoDeviceDoc::MicroApacheWaitStartDone())
				{
					EndWaitCursor();
					::AfxMessageBox(ML_STRING(1475, "Failed to start the web server"), MB_ICONSTOP);
					BeginWaitCursor();
				}
				else
					((CUImagerApp*)::AfxGetApp())->m_MicroApacheWatchdogThread.Start(THREAD_PRIORITY_BELOW_NORMAL);
			}
		}
	}

	// Store settings
	pApp->WriteProfileFullscreenBrowser(FULLSCREENBROWSER_EXITSTRING_ENTRY, m_sFullscreenBrowserExitString);
	if (pApp->m_bUseSettings)
	{
		if (pApp->m_bUseRegistry)
		{
			pApp->WriteProfileInt(			_T("GeneralApp"),
											_T("TopMost"),
											m_bTopMost);
			pApp->WriteProfileInt(			_T("GeneralApp"),
											_T("TrayIcon"),
											m_bTrayIcon);
			pApp->WriteProfileInt(			_T("GeneralApp"),
											_T("ESCExit"),
											m_bEscExit);
			pApp->WriteProfileInt(			_T("GeneralApp"),
											_T("DisableExtProg"),
											m_bDisableExtProg);
			pApp->WriteProfileInt(			_T("GeneralApp"),
											_T("FullscreenBrowser"),
											m_bFullscreenBrowser);
			pApp->WriteProfileInt(			_T("GeneralApp"),
											_T("BrowserAutostart"),
											m_bBrowserAutostart);
			pApp->WriteProfileInt(			_T("GeneralApp"),
											_T("StartMicroApache"),
											m_bStartMicroApache);
			pApp->WriteProfileString(		_T("GeneralApp"),
											_T("MicroApacheDocRoot"),
											m_sMicroApacheDocRoot);
			pApp->WriteProfileInt(			_T("GeneralApp"),
											_T("MicroApachePort"),
											m_nMicroApachePort);
			pApp->WriteSecureProfileString(	_T("GeneralApp"),
											_T("MicroApacheUsername"),
											m_sMicroApacheUsername);
			pApp->WriteSecureProfileString(	_T("GeneralApp"),
											_T("MicroApachePassword"),
											m_sMicroApachePassword);
		}
		else
		{
			// Make a temporary copy because writing to memory sticks is so slow! 
			CString sTempFileName = ::MakeTempFileName(pApp->GetAppTempDir(), pApp->m_pszProfileName);
			::WritePrivateProfileString(NULL, NULL, NULL, pApp->m_pszProfileName); // recache
			::CopyFile(pApp->m_pszProfileName, sTempFileName, FALSE);
			::WriteProfileIniInt(			_T("GeneralApp"),
											_T("TopMost"),
											m_bTopMost,
											sTempFileName);
			::WriteProfileIniInt(			_T("GeneralApp"),
											_T("TrayIcon"),
											m_bTrayIcon,
											sTempFileName);
			::WriteProfileIniInt(			_T("GeneralApp"),
											_T("ESCExit"),
											m_bEscExit,
											sTempFileName);
			::WriteProfileIniInt(			_T("GeneralApp"),
											_T("DisableExtProg"),
											m_bDisableExtProg,
											sTempFileName);
			::WriteProfileIniInt(			_T("GeneralApp"),
											_T("FullscreenBrowser"),
											m_bFullscreenBrowser,
											sTempFileName);
			::WriteProfileIniInt(			_T("GeneralApp"),
											_T("BrowserAutostart"),
											m_bBrowserAutostart,
											sTempFileName);
			::WriteProfileIniInt(			_T("GeneralApp"),
											_T("StartMicroApache"),
											m_bStartMicroApache,
											sTempFileName);
			::WriteProfileIniString(		_T("GeneralApp"),
											_T("MicroApacheDocRoot"),
											m_sMicroApacheDocRoot,
											sTempFileName);
			::WriteProfileIniInt(			_T("GeneralApp"),
											_T("MicroApachePort"),
											m_nMicroApachePort,
											sTempFileName);
			::WriteSecureProfileIniString(	_T("GeneralApp"),
											_T("MicroApacheUsername"),
											m_sMicroApacheUsername,
											sTempFileName);
			::WriteSecureProfileIniString(	_T("GeneralApp"),
											_T("MicroApachePassword"),
											m_sMicroApachePassword,
											sTempFileName);

			// Move it
			::DeleteFile(pApp->m_pszProfileName);
			::WritePrivateProfileString(NULL, NULL, NULL, sTempFileName); // recache
			::MoveFile(sTempFileName, pApp->m_pszProfileName);
		}
	}

	EndWaitCursor();
}

BOOL CSettingsDlgVideoDeviceDoc::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Micro Apache Doc Root Dir
	m_MicroApacheDocRootLabel.SubclassDlgItem(IDC_TEXT_WEBSERVER_ROOTDIR, this);
	m_MicroApacheDocRootLabel.SetVisitedColor(RGB(0, 0, 255));
	m_MicroApacheDocRootLabel.SetLink(m_sMicroApacheDocRoot);
	
	// Enable / Disable Fullscreen Browser Exit password field
	CButton* pCheckFullscreenBrowser = (CButton*)GetDlgItem(IDC_CHECK_FULLSCREENBROWSER);
	CEdit* pEditFullscreenBrowserExitString = (CEdit*)GetDlgItem(IDC_EDIT_FULLSCREENBROWSER_EXITSTRING);
	CEdit* pLabelFullscreenBrowser = (CEdit*)GetDlgItem(IDC_LABEL_FULLSCREENBROWSER);
	if (pCheckFullscreenBrowser				&&
		pEditFullscreenBrowserExitString	&&
		pLabelFullscreenBrowser)
	{
		if (pCheckFullscreenBrowser->GetCheck())
		{
			pEditFullscreenBrowserExitString->EnableWindow(TRUE);
			pLabelFullscreenBrowser->EnableWindow(TRUE);
		}
		else
		{
			pEditFullscreenBrowserExitString->EnableWindow(FALSE);
			pLabelFullscreenBrowser->EnableWindow(FALSE);
		}
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSettingsDlgVideoDeviceDoc::OnWebserverRootdir() 
{
	UpdateData(TRUE);
	CBrowseDlg dlg(	::AfxGetMainFrame(),
					&m_sMicroApacheDocRoot,
					ML_STRING(1495, "Select Document Root Folder"),
					TRUE);
	dlg.DoModal();
	m_MicroApacheDocRootLabel.SetLink(m_sMicroApacheDocRoot);
	UpdateData(FALSE);
}

#endif
