// RemoteCamViewer.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "RemoteCamViewer.h"

#include "MainFrm.h"
#include "RemoteCamViewerDoc.h"
#include "RemoteCamViewerView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MYCOMPANY		_T("Contaware")

/////////////////////////////////////////////////////////////////////////////
// CRemoteCamViewerApp

BEGIN_MESSAGE_MAP(CRemoteCamViewerApp, CWinApp)
	//{{AFX_MSG_MAP(CRemoteCamViewerApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRemoteCamViewerApp construction

CRemoteCamViewerApp::CRemoteCamViewerApp()
{
	m_bWin2000OrHigher = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CRemoteCamViewerApp object

CRemoteCamViewerApp theApp;

BOOL CRemoteCamViewerApp::IsRegistered()
{
	const GUID CLSID_RemoteCam = {0x08CF6679,0x5BB3,0x4EF4,{0xA7,0x73,0xC4,0x10,0xDC,0x56,0x53,0x1C}};
	BSTR bstrClassName;
	HRESULT hr = ::OleRegGetUserType(CLSID_RemoteCam, USERCLASSTYPE_FULL, &bstrClassName);
	return (hr == S_OK);
}

BOOL CRemoteCamViewerApp::Register() 
{
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	TCHAR szName[_MAX_FNAME];
	TCHAR szProgramName[MAX_PATH];
	if (::GetModuleFileName(NULL, szProgramName, MAX_PATH) == 0)
		return FALSE;
	_tsplitpath(szProgramName, szDrive, szDir, szName, NULL);
	CString sDrive(szDrive);
	CString sDir(szDir);
	CString sName(szName);
	CString sDriveDir = sDrive + sDir;
	CFileDialog fd(	TRUE,
					_T("ocx"),
					sDriveDir + _T("RemoteCam.ocx"),
					OFN_HIDEREADONLY, // Hides the Read Only check box
					_T("ActiveX Files (*.ocx)|*.ocx||"));
	fd.m_ofn.lpstrTitle = _T("Register ActiveX Control");
	if (fd.DoModal() == IDOK)
	{
		HINSTANCE hLib = ::LoadLibrary(fd.GetPathName());
		if (hLib)
		{
			typedef HRESULT (STDAPICALLTYPE *FPDLLREGSVR)(void);
			FPDLLREGSVR lpDllEntryPoint = (FPDLLREGSVR)::GetProcAddress(hLib, _T("DllRegisterServer"));
			if (lpDllEntryPoint && lpDllEntryPoint() == NOERROR)
			{
				::FreeLibrary(hLib);
				return TRUE;
			}
			::FreeLibrary(hLib);
		}
		::AfxMessageBox(_T("Registration failed!"), MB_OK | MB_ICONERROR);
	}
	return FALSE;
}

BOOL CRemoteCamViewerApp::Unregister() 
{
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	TCHAR szName[_MAX_FNAME];
	TCHAR szProgramName[MAX_PATH];
	if (::GetModuleFileName(NULL, szProgramName, MAX_PATH) == 0)
		return FALSE;
	_tsplitpath(szProgramName, szDrive, szDir, szName, NULL);
	CString sDrive(szDrive);
	CString sDir(szDir);
	CString sName(szName);
	CString sDriveDir = sDrive + sDir;
	CFileDialog fd(	TRUE,
					_T("ocx"),
					sDriveDir + _T("RemoteCam.ocx"),
					OFN_HIDEREADONLY, // Hides the Read Only check box
					_T("ActiveX Files (*.ocx)|*.ocx||"));
	fd.m_ofn.lpstrTitle = _T("Unregister ActiveX Control");
	if (fd.DoModal() == IDOK)
	{
		HINSTANCE hLib = ::LoadLibrary(fd.GetPathName());
		if (hLib)
		{
			typedef HRESULT (STDAPICALLTYPE *FPDLLUNREGSVR)(void);
			FPDLLUNREGSVR lpDllEntryPoint = (FPDLLUNREGSVR)::GetProcAddress(hLib, _T("DllUnregisterServer"));
			if (lpDllEntryPoint && lpDllEntryPoint() == NOERROR)
			{
				::FreeLibrary(hLib);
				return TRUE;
			}
			::FreeLibrary(hLib);
		}
		::AfxMessageBox(_T("Unregistration failed!"), MB_OK | MB_ICONERROR);
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CRemoteCamViewerApp initialization

BOOL CRemoteCamViewerApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();

#if _MFC_VER < 0x0700
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif
#endif

	// Registry key under which the settings are stored
	SetRegistryKey(MYCOMPANY);

	// Load standard INI file options (including MRU)
	LoadStdProfileSettings();

	// Is ActiveX Registered?
	if (!IsRegistered())
		Register();

	// Get Win Version
	OSVERSIONINFO ovi = {0};
    ovi.dwOSVersionInfoSize = sizeof(ovi);
	GetVersionEx(&ovi);
	m_bWin2000OrHigher =	(ovi.dwPlatformId == 2)	&&
							(ovi.dwMajorVersion >= 5);

	// Register the application's document templates. Document templates
	// serve as the connection between documents, frame windows and views.
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CRemoteCamViewerDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CRemoteCamViewerView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it.
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CRemoteCamViewerApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

typedef BOOL (WINAPI * FPCRYPTPROTECTDATA)(DATA_BLOB*, LPCWSTR, DATA_BLOB*, PVOID, CRYPTPROTECT_PROMPTSTRUCT*, DWORD, DATA_BLOB*);
BOOL CRemoteCamViewerApp::WriteSecureProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue)
{
	HINSTANCE h = ::LoadLibrary(_T("crypt32.dll"));
	if (!h)
		return WriteProfileString(lpszSection, lpszEntry, lpszValue);
	FPCRYPTPROTECTDATA fpCryptProtectData = (FPCRYPTPROTECTDATA)::GetProcAddress(h, "CryptProtectData");
	if (fpCryptProtectData && m_bWin2000OrHigher) // System version check necessary because win98 is returning a function pointer which does nothing!
	{
		DATA_BLOB blobIn, blobOut, blobEntropy;
		blobIn.pbData = (BYTE*)lpszValue;
		blobIn.cbData = sizeof(TCHAR) * (_tcslen(lpszValue) + 1);
		blobOut.cbData = 0;
		blobOut.pbData = NULL;
		BYTE Entropy[] = {
			0xFC, 0x2A, 0x04, 0xDC, 0x3D, 0x89, 0x36, 0x1E, 0xDB, 0x41, 0x3F, 0x7F, 0xC7, 0x7C, 0x80, 0x63, 
			0x90, 0x8C, 0xD0, 0x60, 0xA8, 0x69, 0x70, 0xA8, 0xEA, 0x90, 0x0D, 0x7A, 0x21, 0xA9, 0x66, 0x38};
		blobEntropy.pbData = Entropy;
		blobEntropy.cbData = sizeof(Entropy);

		if (fpCryptProtectData(	&blobIn,
#ifdef _UNICODE
								L"UNICODE",	// Windows 2000:  This parameter is required and cannot be set to NULL
#else
								L"ASCII",	// Windows 2000:  This parameter is required and cannot be set to NULL
#endif
								&blobEntropy,
								NULL,
								NULL,
								0,
								&blobOut))
		{
			BOOL res = WriteProfileBinary(lpszSection, lpszEntry, (LPBYTE)blobOut.pbData, (UINT)blobOut.cbData);
			::LocalFree(blobOut.pbData);
			::FreeLibrary(h);
			return res;
		}
		else
		{
			::LocalFree(blobOut.pbData);
			::FreeLibrary(h);
			return FALSE;
		}
	}
	else
	{
		::FreeLibrary(h);
		return WriteProfileString(lpszSection, lpszEntry, lpszValue);
	}
}

typedef BOOL (WINAPI * FPCRYPTUNPROTECTDATA)(DATA_BLOB*, LPWSTR*, DATA_BLOB*, PVOID*, CRYPTPROTECT_PROMPTSTRUCT*, DWORD, DATA_BLOB*);
CString CRemoteCamViewerApp::GetSecureProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault/*=NULL*/)
{
	HINSTANCE h = ::LoadLibrary(_T("crypt32.dll"));
	if (!h)
		return GetProfileString(lpszSection, lpszEntry, lpszDefault);
	FPCRYPTUNPROTECTDATA fpCryptUnprotectData = (FPCRYPTUNPROTECTDATA)::GetProcAddress(h, "CryptUnprotectData");
	if (fpCryptUnprotectData && m_bWin2000OrHigher) // System version check necessary because win98 is returning a function pointer which does nothing!
	{
		DATA_BLOB blobIn, blobOut, blobEntropy;
		blobIn.cbData = 0;
		blobIn.pbData = NULL;
		blobOut.cbData = 0;
		blobOut.pbData = NULL;
		BYTE Entropy[] = {
			0xFC, 0x2A, 0x04, 0xDC, 0x3D, 0x89, 0x36, 0x1E, 0xDB, 0x41, 0x3F, 0x7F, 0xC7, 0x7C, 0x80, 0x63, 
			0x90, 0x8C, 0xD0, 0x60, 0xA8, 0x69, 0x70, 0xA8, 0xEA, 0x90, 0x0D, 0x7A, 0x21, 0xA9, 0x66, 0x38};
		blobEntropy.pbData = Entropy;
		blobEntropy.cbData = sizeof(Entropy);
		LPWSTR pDescrOut = (LPWSTR)0xbaadf00d ; // Not NULL!

		GetProfileBinary(lpszSection, lpszEntry, &blobIn.pbData, (UINT*)&blobIn.cbData);
		if (blobIn.pbData && (blobIn.cbData > 0))
		{
			if (fpCryptUnprotectData(	&blobIn,
										&pDescrOut,
										&blobEntropy,
										NULL,
										NULL,
										0,
										&blobOut))
			{
				CString s;
				CString sType(pDescrOut);
				if (sType == L"UNICODE")
					s = CString((LPCWSTR)blobOut.pbData);
				else if (sType == L"ASCII")
					s = CString((LPCSTR)blobOut.pbData);
				delete [] blobIn.pbData;
				::LocalFree(pDescrOut);
				::LocalFree(blobOut.pbData);
				::FreeLibrary(h);
				return s;
			}
			else
			{
				delete [] blobIn.pbData;
				::LocalFree(pDescrOut);
				::LocalFree(blobOut.pbData);
			}
		}
		else
		{
			if (blobIn.pbData)
				delete [] blobIn.pbData;
		}
		::FreeLibrary(h);
		return _T("");
	}
	else
	{
		::FreeLibrary(h);
		return GetProfileString(lpszSection, lpszEntry, lpszDefault);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CRemoteCamViewerApp message handlers

