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

	BeginWaitCursor();

	// File type association
	if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
	{
		// Graphics

		if (m_bCheckBmp)
			((CUImagerApp*)::AfxGetApp())->AssociateFileType(_T("bmp"));
		else
			((CUImagerApp*)::AfxGetApp())->UnassociateFileType(_T("bmp"));

		if (m_bCheckJpeg)
		{
			((CUImagerApp*)::AfxGetApp())->AssociateFileType(_T("jpg"));
			((CUImagerApp*)::AfxGetApp())->AssociateFileType(_T("jpeg"));
			((CUImagerApp*)::AfxGetApp())->AssociateFileType(_T("jpe"));
			((CUImagerApp*)::AfxGetApp())->AssociateFileType(_T("thm"));
		}
		else
		{
			((CUImagerApp*)::AfxGetApp())->UnassociateFileType(_T("jpg"));
			((CUImagerApp*)::AfxGetApp())->UnassociateFileType(_T("jpeg"));
			((CUImagerApp*)::AfxGetApp())->UnassociateFileType(_T("jpe"));
			((CUImagerApp*)::AfxGetApp())->UnassociateFileType(_T("thm"));
		}

		if (m_bCheckPcx)
			((CUImagerApp*)::AfxGetApp())->AssociateFileType(_T("pcx"));
		else
			((CUImagerApp*)::AfxGetApp())->UnassociateFileType(_T("pcx"));

		if (m_bCheckEmf)
			((CUImagerApp*)::AfxGetApp())->AssociateFileType(_T("emf"));
		else
			((CUImagerApp*)::AfxGetApp())->UnassociateFileType(_T("emf"));

		if (m_bCheckPng)
			((CUImagerApp*)::AfxGetApp())->AssociateFileType(_T("png"));
		else
			((CUImagerApp*)::AfxGetApp())->UnassociateFileType(_T("png"));

		if (m_bCheckTiff)
		{
			((CUImagerApp*)::AfxGetApp())->AssociateFileType(_T("tif"));
			((CUImagerApp*)::AfxGetApp())->AssociateFileType(_T("tiff"));
			((CUImagerApp*)::AfxGetApp())->AssociateFileType(_T("jfx"));
		}
		else
		{
			((CUImagerApp*)::AfxGetApp())->UnassociateFileType(_T("tif"));
			((CUImagerApp*)::AfxGetApp())->UnassociateFileType(_T("tiff"));
			((CUImagerApp*)::AfxGetApp())->UnassociateFileType(_T("jfx"));
		}

		if (m_bCheckGif)
			((CUImagerApp*)::AfxGetApp())->AssociateFileType(_T("gif"));
		else
			((CUImagerApp*)::AfxGetApp())->UnassociateFileType(_T("gif"));


		// Audio

		if (m_bCheckAif)
		{
			((CUImagerApp*)::AfxGetApp())->AssociateFileType(_T("aif"));
			((CUImagerApp*)::AfxGetApp())->AssociateFileType(_T("aiff"));
		}
		else
		{
			((CUImagerApp*)::AfxGetApp())->UnassociateFileType(_T("aif"));
			((CUImagerApp*)::AfxGetApp())->UnassociateFileType(_T("aiff"));
		}

		if (m_bCheckAu)
			((CUImagerApp*)::AfxGetApp())->AssociateFileType(_T("au"));
		else
			((CUImagerApp*)::AfxGetApp())->UnassociateFileType(_T("au"));

		if (m_bCheckMidi)
		{
			((CUImagerApp*)::AfxGetApp())->AssociateFileType(_T("mid"));
			((CUImagerApp*)::AfxGetApp())->AssociateFileType(_T("rmi"));
		}
		else
		{
			((CUImagerApp*)::AfxGetApp())->UnassociateFileType(_T("mid"));
			((CUImagerApp*)::AfxGetApp())->UnassociateFileType(_T("rmi"));
		}

		if (m_bCheckMp3)
			((CUImagerApp*)::AfxGetApp())->AssociateFileType(_T("mp3"));
		else
			((CUImagerApp*)::AfxGetApp())->UnassociateFileType(_T("mp3"));

		if (m_bCheckWav)
			((CUImagerApp*)::AfxGetApp())->AssociateFileType(_T("wav"));
		else
			((CUImagerApp*)::AfxGetApp())->UnassociateFileType(_T("wav"));

		if (m_bCheckWma)
			((CUImagerApp*)::AfxGetApp())->AssociateFileType(_T("wma"));
		else
			((CUImagerApp*)::AfxGetApp())->UnassociateFileType(_T("wma"));

		if (m_bCheckCda)
			((CUImagerApp*)::AfxGetApp())->AssociateFileType(_T("cda"));
		else
			((CUImagerApp*)::AfxGetApp())->UnassociateFileType(_T("cda"));

		// Others

		if (m_bCheckAvi)
		{
			((CUImagerApp*)::AfxGetApp())->AssociateFileType(_T("avi"));
			((CUImagerApp*)::AfxGetApp())->AssociateFileType(_T("divx"));
		}
		else
		{
			((CUImagerApp*)::AfxGetApp())->UnassociateFileType(_T("avi"));
			((CUImagerApp*)::AfxGetApp())->UnassociateFileType(_T("divx"));
		}

		if (m_bCheckZip)
			((CUImagerApp*)::AfxGetApp())->AssociateFileType(_T("zip"));
		else
			((CUImagerApp*)::AfxGetApp())->UnassociateFileType(_T("zip"));

		// Notify Changes
		::SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
	}

	// Single Instance
	if (m_bSingleInstance != ((CUImagerApp*)::AfxGetApp())->m_bSingleInstance)
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
	((CUImagerApp*)::AfxGetApp())->m_bSingleInstance = m_bSingleInstance;
	
	// Tray Icon
	((CUImagerApp*)::AfxGetApp())->m_bTrayIcon = m_bTrayIcon;
	::AfxGetMainFrame()->TrayIcon(m_bTrayIcon);

	// Autostart
	((CUImagerApp*)::AfxGetApp())->Autostart(m_bAutostart);

	// ESC to exit the program
	((CUImagerApp*)::AfxGetApp())->m_bEscExit = m_bEscExit;

	// Disable opening external program (for pdf, swf)
	((CUImagerApp*)::AfxGetApp())->m_bDisableExtProg = m_bDisableExtProg;

	// Store settings
	if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
	{
		if (((CUImagerApp*)::AfxGetApp())->m_bUseRegistry)
		{
			::AfxGetApp()->WriteProfileInt(	_T("GeneralApp"),
											_T("SingleInstance"),
											m_bSingleInstance);
			::AfxGetApp()->WriteProfileInt(	_T("GeneralApp"),
											_T("TrayIcon"),
											m_bTrayIcon);
			::AfxGetApp()->WriteProfileInt(	_T("GeneralApp"),
											_T("ESCExit"),
											m_bEscExit);
			::AfxGetApp()->WriteProfileInt(	_T("GeneralApp"),
											_T("DisableExtProg"),
											m_bDisableExtProg);
		}
		else
		{
			// Make a temporary copy because writing to memory sticks is so slow! 
			CString sTempFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), ::AfxGetApp()->m_pszProfileName);
			::WritePrivateProfileString(NULL, NULL, NULL, ::AfxGetApp()->m_pszProfileName); // recache
			::CopyFile(::AfxGetApp()->m_pszProfileName, sTempFileName, FALSE);
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
			::DeleteFile(::AfxGetApp()->m_pszProfileName);
			::WritePrivateProfileString(NULL, NULL, NULL, sTempFileName); // recache
			::MoveFile(sTempFileName, ::AfxGetApp()->m_pszProfileName);
		}
	}

	EndWaitCursor();
}

#endif
