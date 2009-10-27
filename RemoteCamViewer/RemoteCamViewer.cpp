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
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
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

	// Is Registered?
	if (!IsRegistered())
		Register();

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

/////////////////////////////////////////////////////////////////////////////
// CRemoteCamViewerApp message handlers

