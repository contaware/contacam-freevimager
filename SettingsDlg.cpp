// SettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "sinstance.h"
#include "IniFile.h"
#include "SettingsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifndef VIDEODEVICEDOC

/////////////////////////////////////////////////////////////////////////////
// CSettingsDlg dialog


CSettingsDlg::CSettingsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSettingsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSettingsDlg)
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
	m_bSingleInstance =	((CUImagerApp*)::AfxGetApp())->m_bSingleInstance;
	m_bTrayIcon =		((CUImagerApp*)::AfxGetApp())->m_bTrayIcon;
	m_bAutostart =		((CUImagerApp*)::AfxGetApp())->IsAutostart();
	m_bEscExit =		((CUImagerApp*)::AfxGetApp())->m_bEscExit;
	m_bDisableExtProg = ((CUImagerApp*)::AfxGetApp())->m_bDisableExtProg;
}

void CSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSettingsDlg)
	DDX_Check(pDX, IDC_CHECK_AVI, m_bCheckAvi);
	DDX_Check(pDX, IDC_CHECK_BMP, m_bCheckBmp);
	DDX_Check(pDX, IDC_CHECK_JPEG, m_bCheckJpeg);
	DDX_Check(pDX, IDC_CHECK_PCX, m_bCheckPcx);
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
	DDX_Check(pDX, IDC_CHECK_SINGLEINSTANCE, m_bSingleInstance);
	DDX_Check(pDX, IDC_CHECK_TRAYICON, m_bTrayIcon);
	DDX_Check(pDX, IDC_CHECK_STARTWITH_WINDOWS, m_bAutostart);
	DDX_Check(pDX, IDC_CHECK_DISABLE_EXTPROG, m_bDisableExtProg);
	DDX_Check(pDX, IDC_CHECK_EMF, m_bCheckEmf);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSettingsDlg, CDialog)
	//{{AFX_MSG_MAP(CSettingsDlg)
	ON_BN_CLICKED(IDC_BUTTON_CLEARALL, OnButtonClearall)
	ON_BN_CLICKED(IDC_BUTTON_SETALL, OnButtonSetall)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSettingsDlg message handlers

void CSettingsDlg::OnButtonClearall() 
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

void CSettingsDlg::OnButtonSetall() 
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

void CSettingsDlg::OnOK() 
{
	CDialog::OnOK(); // It calls UpdateData(TRUE) for us

	CUImagerApp* pApp = (CUImagerApp*)::AfxGetApp();

	BeginWaitCursor();

	// File type association
	if (pApp->m_bUseSettings)
	{
		// Init vars
		BOOL bBmpHasUserChoice = FALSE;  BOOL bJpgHasUserChoice = FALSE;  BOOL bJpegHasUserChoice = FALSE;
		BOOL bJpeHasUserChoice = FALSE;  BOOL bThmHasUserChoice = FALSE;  BOOL bPcxHasUserChoice = FALSE;
		BOOL bEmfHasUserChoice = FALSE;  BOOL bPngHasUserChoice = FALSE;  BOOL bTifHasUserChoice = FALSE;
		BOOL bTiffHasUserChoice = FALSE; BOOL bJfxHasUserChoice = FALSE;  BOOL bGifHasUserChoice = FALSE;
		BOOL bAifHasUserChoice = FALSE;  BOOL bAiffHasUserChoice = FALSE; BOOL bAuHasUserChoice = FALSE;
		BOOL bMidHasUserChoice = FALSE;  BOOL bRmiHasUserChoice = FALSE;  BOOL bMp3HasUserChoice = FALSE;
		BOOL bWavHasUserChoice = FALSE;  BOOL bWmaHasUserChoice = FALSE;  BOOL bCdaHasUserChoice = FALSE;
		BOOL bAviHasUserChoice = FALSE;  BOOL bDivxHasUserChoice = FALSE; BOOL bZipHasUserChoice = FALSE;

		// Graphics

		if (m_bCheckBmp)
			pApp->AssociateFileType(_T("bmp"), &bBmpHasUserChoice);
		else
			pApp->UnassociateFileType(_T("bmp"));

		if (m_bCheckJpeg)
		{
			pApp->AssociateFileType(_T("jpg"), &bJpgHasUserChoice);
			pApp->AssociateFileType(_T("jpeg"), &bJpegHasUserChoice);
			pApp->AssociateFileType(_T("jpe"), &bJpeHasUserChoice);
			pApp->AssociateFileType(_T("thm"), &bThmHasUserChoice);
		}
		else
		{
			pApp->UnassociateFileType(_T("jpg"));
			pApp->UnassociateFileType(_T("jpeg"));
			pApp->UnassociateFileType(_T("jpe"));
			pApp->UnassociateFileType(_T("thm"));
		}

		if (m_bCheckPcx)
			pApp->AssociateFileType(_T("pcx"), &bPcxHasUserChoice);
		else
			pApp->UnassociateFileType(_T("pcx"));

		if (m_bCheckEmf)
			pApp->AssociateFileType(_T("emf"), &bEmfHasUserChoice);
		else
			pApp->UnassociateFileType(_T("emf"));

		if (m_bCheckPng)
			pApp->AssociateFileType(_T("png"), &bPngHasUserChoice);
		else
			pApp->UnassociateFileType(_T("png"));

		if (m_bCheckTiff)
		{
			pApp->AssociateFileType(_T("tif"), &bTifHasUserChoice);
			pApp->AssociateFileType(_T("tiff"), &bTiffHasUserChoice);
			pApp->AssociateFileType(_T("jfx"), &bJfxHasUserChoice);
		}
		else
		{
			pApp->UnassociateFileType(_T("tif"));
			pApp->UnassociateFileType(_T("tiff"));
			pApp->UnassociateFileType(_T("jfx"));
		}

		if (m_bCheckGif)
			pApp->AssociateFileType(_T("gif"), &bGifHasUserChoice);
		else
			pApp->UnassociateFileType(_T("gif"));


		// Audio

		if (m_bCheckAif)
		{
			pApp->AssociateFileType(_T("aif"), &bAifHasUserChoice);
			pApp->AssociateFileType(_T("aiff"), &bAiffHasUserChoice);
		}
		else
		{
			pApp->UnassociateFileType(_T("aif"));
			pApp->UnassociateFileType(_T("aiff"));
		}

		if (m_bCheckAu)
			pApp->AssociateFileType(_T("au"), &bAuHasUserChoice);
		else
			pApp->UnassociateFileType(_T("au"));

		if (m_bCheckMidi)
		{
			pApp->AssociateFileType(_T("mid"), &bMidHasUserChoice);
			pApp->AssociateFileType(_T("rmi"), &bRmiHasUserChoice);
		}
		else
		{
			pApp->UnassociateFileType(_T("mid"));
			pApp->UnassociateFileType(_T("rmi"));
		}

		if (m_bCheckMp3)
			pApp->AssociateFileType(_T("mp3"), &bMp3HasUserChoice);
		else
			pApp->UnassociateFileType(_T("mp3"));

		if (m_bCheckWav)
			pApp->AssociateFileType(_T("wav"), &bWavHasUserChoice);
		else
			pApp->UnassociateFileType(_T("wav"));

		if (m_bCheckWma)
			pApp->AssociateFileType(_T("wma"), &bWmaHasUserChoice);
		else
			pApp->UnassociateFileType(_T("wma"));

		if (m_bCheckCda)
			pApp->AssociateFileType(_T("cda"), &bCdaHasUserChoice);
		else
			pApp->UnassociateFileType(_T("cda"));

		// Others

		if (m_bCheckAvi)
		{
			pApp->AssociateFileType(_T("avi"), &bAviHasUserChoice);
			pApp->AssociateFileType(_T("divx"), &bDivxHasUserChoice);
		}
		else
		{
			pApp->UnassociateFileType(_T("avi"));
			pApp->UnassociateFileType(_T("divx"));
		}

		if (m_bCheckZip)
			pApp->AssociateFileType(_T("zip"), &bZipHasUserChoice);
		else
			pApp->UnassociateFileType(_T("zip"));

		// For Vista or higher there is also a key under
		// HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.ext
		// called UserChoice that can have a ProgID value: this has the highest priority and can
		// prevent from being associated. Vista or higher do not let you delete it, even as
		// administrator ... we can delete it with regedit.exe:
		if (g_bWinVistaOrHigher &&
			(bBmpHasUserChoice || bJpgHasUserChoice	|| bJpegHasUserChoice	|| bJpeHasUserChoice ||
			bThmHasUserChoice || bPcxHasUserChoice	|| bEmfHasUserChoice	|| bPngHasUserChoice ||
			bTifHasUserChoice || bTiffHasUserChoice	|| bJfxHasUserChoice	|| bGifHasUserChoice ||
			bAifHasUserChoice || bAiffHasUserChoice	|| bAuHasUserChoice		|| bMidHasUserChoice ||
			bRmiHasUserChoice || bMp3HasUserChoice	|| bWavHasUserChoice	|| bWmaHasUserChoice ||
			bCdaHasUserChoice || bAviHasUserChoice	|| bDivxHasUserChoice	|| bZipHasUserChoice))
		{
			try
			{
				CString sTempRegFileName = ::MakeTempFileName(pApp->GetAppTempDir(), _T("extfix.reg"));
				CStdioFile RegFile(sTempRegFileName, CFile::modeCreate | CFile::modeWrite | CFile::typeText);
				RegFile.WriteString(_T("REGEDIT4\n\n"));
				if (bBmpHasUserChoice)
					RegFile.WriteString(_T("[-HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.bmp\\UserChoice]\n"));
				if (bJpgHasUserChoice)
					RegFile.WriteString(_T("[-HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.jpg\\UserChoice]\n"));
				if (bJpegHasUserChoice)
					RegFile.WriteString(_T("[-HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.jpeg\\UserChoice]\n"));
				if (bJpeHasUserChoice)
					RegFile.WriteString(_T("[-HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.jpe\\UserChoice]\n"));
				if (bThmHasUserChoice)
					RegFile.WriteString(_T("[-HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.thm\\UserChoice]\n"));
				if (bPcxHasUserChoice)
					RegFile.WriteString(_T("[-HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.pcx\\UserChoice]\n"));
				if (bEmfHasUserChoice)
					RegFile.WriteString(_T("[-HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.emf\\UserChoice]\n"));
				if (bPngHasUserChoice)
					RegFile.WriteString(_T("[-HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.png\\UserChoice]\n"));
				if (bTifHasUserChoice)
					RegFile.WriteString(_T("[-HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.tif\\UserChoice]\n"));
				if (bTiffHasUserChoice)
					RegFile.WriteString(_T("[-HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.tiff\\UserChoice]\n"));
				if (bJfxHasUserChoice)
					RegFile.WriteString(_T("[-HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.jfx\\UserChoice]\n"));
				if (bGifHasUserChoice)
					RegFile.WriteString(_T("[-HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.gif\\UserChoice]\n"));
				if (bAifHasUserChoice)
					RegFile.WriteString(_T("[-HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.aif\\UserChoice]\n"));
				if (bAiffHasUserChoice)
					RegFile.WriteString(_T("[-HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.aiff\\UserChoice]\n"));
				if (bAuHasUserChoice)
					RegFile.WriteString(_T("[-HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.au\\UserChoice]\n"));
				if (bMidHasUserChoice)
					RegFile.WriteString(_T("[-HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.mid\\UserChoice]\n"));
				if (bRmiHasUserChoice)
					RegFile.WriteString(_T("[-HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.rmi\\UserChoice]\n"));
				if (bMp3HasUserChoice)
					RegFile.WriteString(_T("[-HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.mp3\\UserChoice]\n"));
				if (bWavHasUserChoice)
					RegFile.WriteString(_T("[-HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.wav\\UserChoice]\n"));
				if (bWmaHasUserChoice)
					RegFile.WriteString(_T("[-HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.wma\\UserChoice]\n"));
				if (bCdaHasUserChoice)
					RegFile.WriteString(_T("[-HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.cda\\UserChoice]\n"));
				if (bAviHasUserChoice)
					RegFile.WriteString(_T("[-HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.avi\\UserChoice]\n"));
				if (bDivxHasUserChoice)
					RegFile.WriteString(_T("[-HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.divx\\UserChoice]\n"));
				if (bZipHasUserChoice)
					RegFile.WriteString(_T("[-HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.zip\\UserChoice]\n"));
				RegFile.Close();
				CString sParams;
				sParams.Format(_T("/S \"%s\""), sTempRegFileName);
				::ExecHiddenApp(_T("regedit.exe"), sParams, TRUE);
				::DeleteFile(sTempRegFileName);
			}
			catch (CFileException* e)
			{
				e->Delete();
			}
		}

		// Notify Changes
		::SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
	}

	// Single Instance
	if (m_bSingleInstance != pApp->m_bSingleInstance)
	{
#ifdef _UNICODE
		CInstanceChecker instanceChecker(CString(APPNAME_NOEXT) + CString(_T("_Unicode")));
#else
		CInstanceChecker instanceChecker(CString(APPNAME_NOEXT) + CString(_T("_Ascii")));
#endif
		if (m_bSingleInstance)
		{
			instanceChecker.ActivateChecker();
			if (!instanceChecker.PreviousInstanceRunning())
				instanceChecker.TrackFirstInstanceRunning();
		}
		// else the constructor above closed the handle of the mm file
	}
	pApp->m_bSingleInstance = m_bSingleInstance;
	
	// Tray Icon
	pApp->m_bTrayIcon = m_bTrayIcon;
	::AfxGetMainFrame()->TrayIcon(m_bTrayIcon);

	// Autostart
	pApp->Autostart(m_bAutostart);

	// ESC to exit the program
	pApp->m_bEscExit = m_bEscExit;

	// Disable opening external program (for pdf, swf)
	pApp->m_bDisableExtProg = m_bDisableExtProg;

	// Store settings
	if (pApp->m_bUseSettings)
	{
		if (pApp->m_bUseRegistry)
		{
			pApp->WriteProfileInt(	_T("GeneralApp"),
									_T("SingleInstance"),
									m_bSingleInstance);
			pApp->WriteProfileInt(	_T("GeneralApp"),
									_T("TrayIcon"),
									m_bTrayIcon);
			pApp->WriteProfileInt(	_T("GeneralApp"),
									_T("ESCExit"),
									m_bEscExit);
			pApp->WriteProfileInt(	_T("GeneralApp"),
									_T("DisableExtProg"),
									m_bDisableExtProg);
		}
		else
		{
			// Make a temporary copy because writing to memory sticks is so slow! 
			CString sTempFileName = ::MakeTempFileName(pApp->GetAppTempDir(), pApp->m_pszProfileName);
			::WritePrivateProfileString(NULL, NULL, NULL, pApp->m_pszProfileName); // recache
			::CopyFile(pApp->m_pszProfileName, sTempFileName, FALSE);
			::WriteProfileIniInt(	_T("GeneralApp"),
									_T("SingleInstance"),
									m_bSingleInstance,
									sTempFileName);
			::WriteProfileIniInt(	_T("GeneralApp"),
									_T("TrayIcon"),
									m_bTrayIcon,
									sTempFileName);
			::WriteProfileIniInt(	_T("GeneralApp"),
									_T("ESCExit"),
									m_bEscExit,
									sTempFileName);
			::WriteProfileIniInt(	_T("GeneralApp"),
									_T("DisableExtProg"),
									m_bDisableExtProg,
									sTempFileName);

			// Move it
			::DeleteFile(pApp->m_pszProfileName);
			::WritePrivateProfileString(NULL, NULL, NULL, sTempFileName); // recache
			::MoveFile(sTempFileName, pApp->m_pszProfileName);
		}
	}

	EndWaitCursor();
}

#endif
