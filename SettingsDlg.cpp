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
	DDX_Check(pDX, IDC_CHECK_SINGLEINSTANCE, m_bSingleInstance);
	DDX_Check(pDX, IDC_CHECK_TRAYICON, m_bTrayIcon);
	DDX_Check(pDX, IDC_CHECK_STARTWITH_WINDOWS, m_bAutostart);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSettingsDlg, CDialog)
	//{{AFX_MSG_MAP(CSettingsDlg)
	ON_BN_CLICKED(IDC_CHECK_STARTWITH_WINDOWS, &CSettingsDlg::OnBnClickedCheckStartwithWindows)
	ON_BN_CLICKED(IDC_CHECK_TRAYICON, &CSettingsDlg::OnBnClickedCheckTrayicon)
	ON_BN_CLICKED(IDC_CHECK_SINGLEINSTANCE, &CSettingsDlg::OnBnClickedCheckSingleinstance)
	ON_BN_CLICKED(IDC_BUTTON_ASSOCIATE, &CSettingsDlg::OnBnClickedButtonAssociate)
	ON_BN_CLICKED(IDC_BUTTON_UNASSOCIATE, &CSettingsDlg::OnBnClickedButtonUnassociate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSettingsDlg message handlers

void CSettingsDlg::OnBnClickedCheckStartwithWindows()
{
	// Validate
	if (!UpdateData(TRUE))
		return;

	CUImagerApp* pApp = (CUImagerApp*)::AfxGetApp();
	pApp->Autostart(m_bAutostart);
}

void CSettingsDlg::OnBnClickedCheckTrayicon()
{
	// Validate
	if (!UpdateData(TRUE))
		return;

	CUImagerApp* pApp = (CUImagerApp*)::AfxGetApp();
	pApp->m_bTrayIcon = m_bTrayIcon;
	::AfxGetMainFrame()->TrayIcon(m_bTrayIcon);
	pApp->WriteProfileInt(_T("GeneralApp"), _T("TrayIcon"), m_bTrayIcon);
}

void CSettingsDlg::OnBnClickedCheckSingleinstance()
{
	// Validate
	if (!UpdateData(TRUE))
		return;

	CUImagerApp* pApp = (CUImagerApp*)::AfxGetApp();
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
	pApp->WriteProfileInt(_T("GeneralApp"), _T("SingleInstance"), m_bSingleInstance);
}

void CSettingsDlg::OnBnClickedButtonAssociate()
{
	// Associate all supported file types
	FileAssociation(TRUE);

	// Confirm the association in the OS dialog
	if (!SettingsPageAppsDefaults(_T("SystemSettings_DefaultApps_Photos")))
	{
		if (g_bWinVistaOrHigher)
			::ShellExecute(NULL, NULL, _T("control.exe"), _T("/name Microsoft.DefaultPrograms /page pageFileAssoc"), NULL, SW_SHOWNORMAL);
	}

	// Close dialog
	EndDialog(IDOK);
}

void CSettingsDlg::OnBnClickedButtonUnassociate()
{
	// Unassociate all supported file types
	FileAssociation(FALSE);

	// Confirm the unassociation in the OS dialog
	if (!SettingsPageAppsDefaults(_T("SystemSettings_DefaultApps_Photos")))
	{
		if (g_bWinVistaOrHigher)
			::ShellExecute(NULL, NULL, _T("control.exe"), _T("/name Microsoft.DefaultPrograms /page pageFileAssoc"), NULL, SW_SHOWNORMAL);
	}

	// Close dialog
	EndDialog(IDOK);
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
BOOL CSettingsDlg::SettingsPageAppsDefaults(const CString& sTarget/*=_T("")*/)
{
	// Format target string
	CString sFullTarget;
	if (!sTarget.IsEmpty())
		sFullTarget.Format(_T("&target=%s"), sTarget);

	// Open chosen Settings Page
	IApplicationActivationManager* pActivator;
	HRESULT hr = ::CoCreateInstance(CLSID_ApplicationActivationManager,
									nullptr,
									CLSCTX_INPROC,
									IID_IApplicationActivationManager,
									(void**)&pActivator);
	if (SUCCEEDED(hr))
	{
		DWORD pid;
		hr = pActivator->ActivateApplication(	L"Windows.ImmersiveControlPanel_cw5n1h2txyewy!microsoft.windows.immersivecontrolpanel",
												L"page=SettingsPageAppsDefaults" + sFullTarget,
												AO_NONE,
												&pid);
		pActivator->Release();
		return SUCCEEDED(hr);
	}
	else
		return FALSE;
}

void CSettingsDlg::FileAssociation(BOOL bDoAssociation)
{
	CUImagerApp* pApp = (CUImagerApp*)::AfxGetApp();

	if (bDoAssociation)
	{
		// Bmp
		pApp->AssociateFileType(_T("bmp"));

		// Jpeg
		pApp->AssociateFileType(_T("jpg"));
		pApp->AssociateFileType(_T("jpeg"));
		pApp->AssociateFileType(_T("jpe"));
		pApp->AssociateFileType(_T("thm"));

		// Pcx
		pApp->AssociateFileType(_T("pcx"));

		// Emf
		pApp->AssociateFileType(_T("emf"));

		// Png
		pApp->AssociateFileType(_T("png"));

		// Tiff
		pApp->AssociateFileType(_T("tif"));
		pApp->AssociateFileType(_T("tiff"));
		pApp->AssociateFileType(_T("jfx"));

		// Gif
		pApp->AssociateFileType(_T("gif"));
	}
	else
	{
		// Bmp
		pApp->UnassociateFileType(_T("bmp"));

		// Jpeg
		pApp->UnassociateFileType(_T("jpg"));
		pApp->UnassociateFileType(_T("jpeg"));
		pApp->UnassociateFileType(_T("jpe"));
		pApp->UnassociateFileType(_T("thm"));

		// Pcx
		pApp->UnassociateFileType(_T("pcx"));

		// Emf
		pApp->UnassociateFileType(_T("emf"));

		// Png
		pApp->UnassociateFileType(_T("png"));

		// Tiff
		pApp->UnassociateFileType(_T("tif"));
		pApp->UnassociateFileType(_T("tiff"));
		pApp->UnassociateFileType(_T("jfx"));

		// Gif
		pApp->UnassociateFileType(_T("gif"));
	}

	// Notify Changes
	::SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
}

#endif
