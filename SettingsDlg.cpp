// SettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "sinstance.h"
#include "SettingsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifndef VIDEODEVICEDOC

// From ShObjIdl.h (IApplicationActivationManager not available in SDK 7.1)
#ifndef __IApplicationActivationManager_INTERFACE_DEFINED__
#define __IApplicationActivationManager_INTERFACE_DEFINED__
const IID IID_IApplicationActivationManager = { 0x2e941141,0x7f97,0x4756,{ 0xba,0x1d,0x9d,0xec,0xde,0x89,0x4a,0x3d } };
const CLSID CLSID_ApplicationActivationManager = { 0x45BA127D,0x10A8,0x46EA,{ 0x8A,0xB7,0x56,0xEA,0x90,0x78,0x94,0x3C } };
typedef enum ACTIVATEOPTIONS
{
	AO_NONE = 0,
	AO_DESIGNMODE = 0x1,
	AO_NOERRORUI = 0x2,
	AO_NOSPLASHSCREEN = 0x4
} 	ACTIVATEOPTIONS;
MIDL_INTERFACE("2e941141-7f97-4756-ba1d-9decde894a3d")
IApplicationActivationManager : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE ActivateApplication(
		/* [in] */ __RPC__in LPCWSTR appUserModelId,
		/* [unique][in] */ __RPC__in_opt LPCWSTR arguments,
		/* [in] */ ACTIVATEOPTIONS options,
		/* [out] */ __RPC__out DWORD *processId) = 0;

	virtual HRESULT STDMETHODCALLTYPE ActivateForFile(
		/* [in] */ __RPC__in LPCWSTR appUserModelId,
		/* [in] */ __RPC__in_opt IShellItemArray *itemArray,
		/* [unique][in] */ __RPC__in_opt LPCWSTR verb,
		/* [out] */ __RPC__out DWORD *processId) = 0;

	virtual HRESULT STDMETHODCALLTYPE ActivateForProtocol(
		/* [in] */ __RPC__in LPCWSTR appUserModelId,
		/* [in] */ __RPC__in_opt IShellItemArray *itemArray,
		/* [out] */ __RPC__out DWORD *processId) = 0;
};
#endif

/////////////////////////////////////////////////////////////////////////////
// CSettingsDlg dialog


CSettingsDlg::CSettingsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSettingsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSettingsDlg)
	//}}AFX_DATA_INIT

	// File Associations
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

	// Global Settings
	m_bSingleInstance =	((CUImagerApp*)::AfxGetApp())->m_bSingleInstance;
	m_bTrayIcon =		((CUImagerApp*)::AfxGetApp())->m_bTrayIcon;
	m_bAutostart =		((CUImagerApp*)::AfxGetApp())->IsAutostart();

	// Init COM: COM may fail if its already been inited with a different 
	// concurrency model. And if it fails you shouldn't release it!
	// Typically, the COM library is initialized on an apartment only once.
	// Subsequent calls will succeed, as long as they do not attempt to change
	// the concurrency model, but will return S_FALSE. To close the COM
	// library gracefully, each successful call to CoInitialize or CoInitializeEx,
	// including those that return S_FALSE, must be balanced by a corresponding
	// call to CoUninitialize.
	// Once the concurrency model for an apartment is set, it cannot be changed.
	// A call to CoInitialize on an apartment that was previously initialized as
	// multithreaded will fail and return RPC_E_CHANGED_MODE.
	HRESULT hr = ::CoInitialize(NULL);
	m_bCleanupCOM = ((hr == S_OK) || (hr == S_FALSE));
}

CSettingsDlg::~CSettingsDlg()
{
	if (m_bCleanupCOM)
		::CoUninitialize();
}

void CSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSettingsDlg)
	DDX_Check(pDX, IDC_CHECK_BMP, m_bCheckBmp);
	DDX_Check(pDX, IDC_CHECK_JPEG, m_bCheckJpeg);
	DDX_Check(pDX, IDC_CHECK_PCX, m_bCheckPcx);
	DDX_Check(pDX, IDC_CHECK_EMF, m_bCheckEmf);
	DDX_Check(pDX, IDC_CHECK_PNG, m_bCheckPng);
	DDX_Check(pDX, IDC_CHECK_TIFF, m_bCheckTiff);
	DDX_Check(pDX, IDC_CHECK_GIF, m_bCheckGif);
	DDX_Check(pDX, IDC_CHECK_SINGLEINSTANCE, m_bSingleInstance);
	DDX_Check(pDX, IDC_CHECK_TRAYICON, m_bTrayIcon);
	DDX_Check(pDX, IDC_CHECK_STARTWITH_WINDOWS, m_bAutostart);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSettingsDlg, CDialog)
	//{{AFX_MSG_MAP(CSettingsDlg)
	ON_BN_CLICKED(IDC_BUTTON_CLEARALL, OnButtonClearall)
	ON_BN_CLICKED(IDC_BUTTON_SETALL, OnButtonSetall)
	ON_BN_CLICKED(IDC_BUTTON_APPS_DEFAULTS, OnButtonAppsDefaults)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSettingsDlg message handlers

void CSettingsDlg::OnButtonClearall() 
{
	UpdateData(TRUE);

	m_bCheckBmp = FALSE;
	m_bCheckJpeg = FALSE;
	m_bCheckPcx = FALSE;
	m_bCheckEmf = FALSE;
	m_bCheckPng = FALSE;
	m_bCheckTiff = FALSE;
	m_bCheckGif = FALSE;

	UpdateData(FALSE);
}

void CSettingsDlg::OnButtonSetall() 
{
	UpdateData(TRUE);

	m_bCheckBmp = TRUE;
	m_bCheckJpeg = TRUE;
	m_bCheckPcx = TRUE;
	m_bCheckEmf = TRUE;
	m_bCheckPng = TRUE;
	m_bCheckTiff = TRUE;
	m_bCheckGif = TRUE;

	UpdateData(FALSE);
}

// sTarget (case sensitive):
// ""
// "SystemSettings_DefaultApps_Email"
// "SystemSettings_DefaultApps_Map"
// "SystemSettings_DefaultApps_Audio"
// "SystemSettings_DefaultApps_Photos"
// "SystemSettings_DefaultApps_Video"
// "SystemSettings_DefaultApps_Browser"
// "SettingsPageAppsDefaultsFileExtensionView"
// "SettingsPageAppsDefaultsProtocolView"
void CSettingsDlg::SettingsPageAppsDefaults(const CString& sTarget/*=_T("")*/)
{
	// Format target string
	CString sFullTarget;
	if (!sTarget.IsEmpty())
		sFullTarget.Format(_T("&target=%s"), sTarget);

	// Open chosen Settings Page
	IApplicationActivationManager* pActivator;
	if (SUCCEEDED(::CoCreateInstance(CLSID_ApplicationActivationManager,
									nullptr,
									CLSCTX_INPROC,
									IID_IApplicationActivationManager,
									(void**)&pActivator)))
	{
		DWORD pid;
		pActivator->ActivateApplication(L"Windows.ImmersiveControlPanel_cw5n1h2txyewy!microsoft.windows.immersivecontrolpanel",
										L"page=SettingsPageAppsDefaults" + sFullTarget,
										AO_NONE,
										&pid);
		pActivator->Release();
	}
	else
	{
		if (g_bWinVistaOrHigher)
		{
			if (sTarget == _T("SettingsPageAppsDefaultsFileExtensionView") ||
				sTarget == _T("SettingsPageAppsDefaultsProtocolView"))
			{
				::ShellExecute(	NULL, NULL,
								_T("control.exe"), _T("/name Microsoft.DefaultPrograms /page pageFileAssoc"),
								NULL, SW_SHOWNORMAL);
			}
			else
			{
				::ShellExecute(	NULL, NULL,
								_T("control.exe"), _T("/name Microsoft.DefaultPrograms /page pageDefaultProgram"),
								NULL, SW_SHOWNORMAL);
			}
		}
		else
		{
			::ShellExecute(	NULL, NULL,
							_T("control.exe"), _T("folders"),
							NULL, SW_SHOWNORMAL);
		}
	}
}

void CSettingsDlg::Apply()
{
	// Validate
	if (!UpdateData(TRUE))
		return;

	CUImagerApp* pApp = (CUImagerApp*)::AfxGetApp();

	BeginWaitCursor();

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

	// Single Instance
	if (m_bSingleInstance != pApp->m_bSingleInstance)
	{
		CInstanceChecker instanceChecker(CString(APPNAME_NOEXT) + CString(_T("_Unicode")));
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

	// Store settings
	pApp->WriteProfileInt(	_T("GeneralApp"),
							_T("SingleInstance"),
							m_bSingleInstance);
	pApp->WriteProfileInt(	_T("GeneralApp"),
							_T("TrayIcon"),
							m_bTrayIcon);

	EndWaitCursor();
}

void CSettingsDlg::OnOK()
{
	Apply();
	EndDialog(IDOK);
}

void CSettingsDlg::OnButtonAppsDefaults()
{
	Apply();
	EndDialog(IDOK);
	SettingsPageAppsDefaults(_T("SettingsPageAppsDefaultsFileExtensionView"));
}

#endif