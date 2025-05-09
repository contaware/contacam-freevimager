﻿#include "stdafx.h"
#include "uImager.h"
#include "MainFrm.h"
#include "LicenseHelper.h"
#include "LicenseDlg.h"
#include "CreditsDlg.h"
#include "NewDlg.h"
#include "ChildFrm.h"
#include "ToolBarChildFrm.h"
#include "uImagerDoc.h"
#include "PictureDoc.h"
#include "VideoDeviceDoc.h"
#include "PictureView.h"
#include "PicturePrintPreviewView.h"
#include "VideoDeviceView.h"
#include "PreviewFileDlg.h"
#include "SendMailDocsDlg.h"
#include "BatchProcDlg.h"
#include "DxCapture.h"
#include <mapi.h>
#include "ZipProgressDlg.h"
#include "BrowseDlg.h"
#include "SortableFileFind.h"
#include "CPUSpeed.h"
#include "PostDelayedMessage.h"
#include "YuvToRgb.h"
#include "RgbToYuv.h"
#include "YuvToYuv.h"
#include "sinstance.h"
#include "CEncryptDecrypt.h"
#include "SortableStringArray.h"
#include <atlbase.h>
#include <signal.h>
#include <gdiplus.h>
#include <thread>
#ifdef VIDEODEVICEDOC
#include "DeleteCamFoldersDlg.h"
#include "SettingsDlgVideoDeviceDoc.h"
#include <WinSvc.h>
#include "HostPortDlg.h"
#include <PowrProf.h>
#pragma comment(lib, "PowrProf.lib")
#else
#include "SettingsDlg.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifndef __IApplicationActivationManager_INTERFACE_DEFINED__
#define __IApplicationActivationManager_INTERFACE_DEFINED__
const IID IID_IApplicationActivationManager = { 0x2e941141,0x7f97,0x4756,{ 0xba,0x1d,0x9d,0xec,0xde,0x89,0x4a,0x3d } };
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
// CUImagerApp

BEGIN_MESSAGE_MAP(CUImagerApp, CWinApp)
	//{{AFX_MSG_MAP(CUImagerApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_FILE_OPEN_DIR, OnFileOpenDir)
	ON_COMMAND(ID_FILE_MRU_CLEAR, OnClearRecentFileList)
	ON_UPDATE_COMMAND_UI(ID_FILE_MRU_CLEAR, OnUpdateClearRecentFileList)
	ON_UPDATE_COMMAND_UI(ID_FILE_MRU_FILE1, OnUpdateRecentFileMenu)
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	ON_COMMAND(ID_FILE_CLOSEALL, OnFileCloseall)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_COMMAND(ID_FILE_SETTINGS, OnFileSettings)
	ON_COMMAND(ID_APP_LICENSE, OnAppLicense)
	ON_COMMAND(ID_APP_CREDITS, OnAppCredits)
	ON_UPDATE_COMMAND_UI(ID_FILE_CLOSEALL, OnUpdateFileCloseall)
	ON_COMMAND(ID_FILE_SHRINK_DIR_DOCS, OnFileShrinkDirDocs)
	ON_COMMAND(ID_FILE_SENDMAIL_OPEN_DOCS, OnFileSendmailOpenDocs)
	ON_UPDATE_COMMAND_UI(ID_FILE_SENDMAIL_OPEN_DOCS, OnUpdateFileSendmailOpenDocs)
	ON_COMMAND(ID_SETTINGS_TRAYICON, OnSettingsTrayicon)
	ON_UPDATE_COMMAND_UI(ID_SETTINGS_TRAYICON, OnUpdateSettingsTrayicon)
	ON_COMMAND(ID_APP_MANUAL, OnAppManual)
	ON_UPDATE_COMMAND_UI(ID_FILE_NEW, OnUpdateFileNew)
	ON_COMMAND(ID_SETTINGS_LOG_NORMAL, OnSettingsLogNormal)
	ON_UPDATE_COMMAND_UI(ID_SETTINGS_LOG_NORMAL, OnUpdateSettingsLogNormal)
	ON_COMMAND(ID_SETTINGS_LOG_VERBOSE, OnSettingsLogVerbose)
	ON_UPDATE_COMMAND_UI(ID_SETTINGS_LOG_VERBOSE, OnUpdateSettingsLogVerbose)
	ON_COMMAND(ID_SETTINGS_LOG_ALL_MESSAGES, OnSettingsLogAllMessages)
	ON_UPDATE_COMMAND_UI(ID_SETTINGS_LOG_ALL_MESSAGES, OnUpdateSettingsLogAllMessages)
	ON_COMMAND(ID_SETTINGS_VIEW_LOGFILE, OnSettingsViewLogfile)
	ON_COMMAND(ID_EDIT_SCREENSHOT, OnEditScreenshot)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SCREENSHOT, OnUpdateEditScreenshot)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
#ifdef VIDEODEVICEDOC
	ON_COMMAND(ID_EDIT_DELETE, OnEditDelete)
	ON_COMMAND_RANGE(ID_DIRECTSHOW_VIDEODEV_FIRST, ID_DIRECTSHOW_VIDEODEV_LAST, OnFileDxVideoDevice)
	ON_COMMAND(ID_CAPTURE_NETWORK, OnCaptureNetwork)
#endif
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CUImagerApp construction

CUImagerApp::CUImagerApp()
{
	m_bCanSavePlacements = TRUE;
	m_bShuttingDownApplication = FALSE;
	m_bClosingAll = FALSE;
	m_sAppTempDir = _T("");
#ifdef VIDEODEVICEDOC
	m_pVideoDeviceDocTemplate = NULL;
	m_bAutostartsExecuted = FALSE;
	m_dwFirstStartDelayMs = DEFAULT_FIRSTSTART_DELAY_MS;
	m_nSimultaneousSaving = DEFAULT_SIMULTANEOUS_SAVING;
	m_bMovFragmented = FALSE;
	m_bStartMicroApache = FALSE;
	m_nMicroApachePort = MICROAPACHE_DEFAULT_PORT;
	m_nMicroApachePortSSL = MICROAPACHE_DEFAULT_PORT_SSL;
	m_nDetectionMaxMaxFrames = MOVDET_MAX_MAX_FRAMES_IN_LIST;
	m_bMovDetDropFrames = FALSE;
	::InitializeCriticalSection(&m_csSaveReservation);
	m_sDeleteDaysAgoDir = _T("");
	m_llDeleteDaysAgoUptime = 0;
	m_llDeleteDaysAgo = 0;
	::InitializeCriticalSection(&m_csDeleteDaysAgo);
	m_bSingleInstance = TRUE;
	m_bServiceProcess = FALSE;
	m_bDoStartFromService = FALSE;
#else
	m_bSingleInstance = FALSE;
#endif
	m_bTopMost = FALSE;
	m_pPictureDocTemplate = NULL;
	m_bUseLoadPreviewDibGlobal = TRUE;
	m_bFileDlgPreview = TRUE;
	m_bPlacementLoaded = FALSE;
	m_hAppMutex = NULL;
	m_GdiplusToken = 0;
	m_bFirstRun = FALSE;
	m_bFirstRunEver = FALSE;
	m_bSilentInstall = FALSE;
	m_bStartMaximized = FALSE;
	m_nThreadCount = 1;
	m_sLastOpenedDir = _T("");
	m_nPdfScanCompressionQuality = DEFAULT_JPEGCOMPRESSION;
	m_sScanToPdfFileName = _T("");
	m_sScanToTiffFileName = _T("");
	m_bTrayIcon = FALSE;
	m_bHideMainFrame = FALSE;
	m_bPrinterInit = FALSE;
	m_nCoordinateUnit = COORDINATES_PIX;
	m_nNewWidth = DEFAULT_NEW_WIDTH;
	m_nNewHeight = DEFAULT_NEW_HEIGHT;
	m_nNewXDpi = DEFAULT_NEW_DPI;
	m_nNewYDpi = DEFAULT_NEW_DPI;
	m_nNewPhysUnit = DEFAULT_NEW_PHYS_UNIT;
	m_sNewPaperSize = DEFAULT_NEW_PAPER_SIZE;
	m_crNewBackgroundColor = DEFAULT_NEW_COLOR;
}

CUImagerApp::~CUImagerApp()
{
#ifdef VIDEODEVICEDOC
	::DeleteCriticalSection(&m_csSaveReservation);
	::DeleteCriticalSection(&m_csDeleteDaysAgo);
#endif
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CUImagerApp object

CUImagerApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CUImagerApp initialization

int CUImagerApp::GetConfiguredUseRegistry()
{
	int nUseRegistry = -1;
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	TCHAR szProgramName[MAX_PATH];
	if (::GetModuleFileName(NULL, szProgramName, MAX_PATH) != 0)
	{
		_tsplitpath(szProgramName, szDrive, szDir, NULL, NULL);
		nUseRegistry = ::GetProfileIniInt(_T("General"), _T("UseRegistry"), -1, CString(szDrive) + CString(szDir) + MASTERCONFIG_INI_NAME_EXT);
	}
	return nUseRegistry;
}

CString CUImagerApp::GetConfigFilesDir(BOOL* pbIsConfigured/*=NULL*/)
{
	CString sConfigFilesDir;
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	TCHAR szProgramName[MAX_PATH];
	if (::GetModuleFileName(NULL, szProgramName, MAX_PATH) != 0)
	{
		_tsplitpath(szProgramName, szDrive, szDir, NULL, NULL);
		sConfigFilesDir = ::GetProfileIniString(_T("General"), _T("ConfigFilesDir"), _T(""), CString(szDrive) + CString(szDir) + MASTERCONFIG_INI_NAME_EXT);
	}
	if (sConfigFilesDir.IsEmpty())
	{
		if (pbIsConfigured)
			*pbIsConfigured = FALSE;
		sConfigFilesDir = ::GetSpecialFolderPath(CSIDL_APPDATA); // returns the path with no trailing backslash
		sConfigFilesDir += CString(_T("\\")) + MYCOMPANY + _T("\\") + APPNAME_NOEXT;
	}
	else
	{
		if (pbIsConfigured)
			*pbIsConfigured = TRUE;
		sConfigFilesDir.TrimRight(_T('\\'));
	}
	return sConfigFilesDir;
}

CString CUImagerApp::GetConfiguredTempDir()
{
	CString sConfiguredTempDir;
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	TCHAR szProgramName[MAX_PATH];
	if (::GetModuleFileName(NULL, szProgramName, MAX_PATH) != 0)
	{
		_tsplitpath(szProgramName, szDrive, szDir, NULL, NULL);
		sConfiguredTempDir = ::GetProfileIniString(_T("General"), _T("TempDir"), _T(""), CString(szDrive) + CString(szDir) + MASTERCONFIG_INI_NAME_EXT);
	}
	sConfiguredTempDir.TrimRight(_T('\\'));
	return sConfiguredTempDir;
}

#ifdef VIDEODEVICEDOC

BOOL AVErrorToString(int nErrorCode, CString& sError)
{
	CStringA sBufAnsi;
	LPSTR pBufAnsi = sBufAnsi.GetBuffer(AV_ERROR_MAX_STRING_SIZE);
	int ret = av_strerror(nErrorCode, pBufAnsi, AV_ERROR_MAX_STRING_SIZE);
	sBufAnsi.ReleaseBuffer();
	sError = sBufAnsi;
	return (ret == 0);
}

static int handle_jpeg_helper(enum AVPixelFormat *format)
{
    switch (*format) {
    case AV_PIX_FMT_YUVJ420P:
        *format = AV_PIX_FMT_YUV420P;
        return 1;
    case AV_PIX_FMT_YUVJ411P:
        *format = AV_PIX_FMT_YUV411P;
        return 1;
    case AV_PIX_FMT_YUVJ422P:
        *format = AV_PIX_FMT_YUV422P;
        return 1;
    case AV_PIX_FMT_YUVJ444P:
        *format = AV_PIX_FMT_YUV444P;
        return 1;
    case AV_PIX_FMT_YUVJ440P:
        *format = AV_PIX_FMT_YUV440P;
        return 1;
    case AV_PIX_FMT_GRAY8:
    case AV_PIX_FMT_GRAY16LE:
    case AV_PIX_FMT_GRAY16BE:
        return 1;
    default:
        return 0;
    }
}

// av_lockmgr_register is not creating a mutex for the sws scaler,
// sws_setColorspaceDetails and sws_init_context are not thread safe,
// protect them with a critical section
CRITICAL_SECTION g_csSWScaler;
BOOL g_bSWScalerCSInited = FALSE;

// To avoid the "deprecated pixel format used, make sure you did set range correctly"
// warning in sws_init_context() use this instead of sws_getContext()
SwsContext *sws_getContextHelper(	int srcW, int srcH, enum AVPixelFormat srcFormat,
									int dstW, int dstH, enum AVPixelFormat dstFormat,
									int flags)
{
	SwsContext *c;
	if (!(c = sws_alloc_context()))
		return NULL;

	// YUV range
	// 0 = limited MPEG YUV range
	// 1 = full JPEG YUV range
	int srcRange = handle_jpeg_helper(&srcFormat);
    int dstRange = handle_jpeg_helper(&dstFormat);
	
	av_opt_set_int(c, "srcw",       srcW, 0);
	av_opt_set_int(c, "srch",       srcH, 0);
	av_opt_set_int(c, "src_format", srcFormat, 0);
	av_opt_set_int(c, "src_range",  srcRange, 0);
	av_opt_set_int(c, "dstw",       dstW, 0);
	av_opt_set_int(c, "dsth",       dstH, 0);
	av_opt_set_int(c, "dst_format", dstFormat, 0);
	av_opt_set_int(c, "dst_range",  dstRange, 0);
	av_opt_set_int(c, "sws_flags",  flags, 0);

	EnterCriticalSection(&g_csSWScaler);

	sws_setColorspaceDetails(c,
							sws_getCoefficients(SWS_CS_DEFAULT), srcRange,
							sws_getCoefficients(SWS_CS_DEFAULT), dstRange,
							0, 1<<16, 1<<16);

	if (sws_init_context(c, NULL, NULL) < 0)
	{
		sws_freeContext(c);
		LeaveCriticalSection(&g_csSWScaler);
		return NULL;
	}
	else
	{
		LeaveCriticalSection(&g_csSWScaler);
		return c;
	}
}

// To avoid the "deprecated pixel format used, make sure you did set range correctly"
// warning in sws_init_context() use this instead of sws_getCachedContext()
SwsContext *sws_getCachedContextHelper(	struct SwsContext *context,
										int srcW, int srcH, enum AVPixelFormat srcFormat,
                                        int dstW, int dstH, enum AVPixelFormat dstFormat,
										int flags)
{
    int64_t src_h_chr_pos = -513, dst_h_chr_pos = -513,
            src_v_chr_pos = -513, dst_v_chr_pos = -513;

	// YUV range
	// 0 = limited MPEG YUV range
	// 1 = full JPEG YUV range
	int srcRange = handle_jpeg_helper(&srcFormat);
	int dstRange = handle_jpeg_helper(&dstFormat);

	// If a param changed free context
    if (context)
	{
		int64_t current_srcW;
		int64_t current_srcH;
		int64_t current_srcFormat;
		int64_t current_srcRange;
		int64_t current_dstW;
		int64_t current_dstH;
		int64_t current_dstFormat;
		int64_t current_dstRange;
		int64_t current_flags;
		av_opt_get_int(context, "srcw",       0, &current_srcW);
		av_opt_get_int(context, "srch",       0, &current_srcH);
		av_opt_get_int(context, "src_format", 0, &current_srcFormat);
		av_opt_get_int(context, "src_range",  0, &current_srcRange);
		av_opt_get_int(context, "dstw",       0, &current_dstW);
		av_opt_get_int(context, "dsth",       0, &current_dstH);
		av_opt_get_int(context, "dst_format", 0, &current_dstFormat);
		av_opt_get_int(context, "dst_range",  0, &current_dstRange);
		av_opt_get_int(context, "sws_flags",  0, &current_flags);
        if (current_srcW		!= srcW			||
			current_srcH		!= srcH			||
			current_srcFormat	!= srcFormat	||
			current_srcRange	!= srcRange		||
			current_dstW		!= dstW			||
			current_dstH		!= dstH			||
			current_dstFormat	!= dstFormat	||
			current_dstRange	!= dstRange		||
			current_flags		!= flags)
		{
			av_opt_get_int(context, "src_h_chr_pos", 0, &src_h_chr_pos);
			av_opt_get_int(context, "src_v_chr_pos", 0, &src_v_chr_pos);
			av_opt_get_int(context, "dst_h_chr_pos", 0, &dst_h_chr_pos);
			av_opt_get_int(context, "dst_v_chr_pos", 0, &dst_v_chr_pos);
			sws_freeContext(context);
			context = NULL;
		}
	}

	// Allocate and init context if not yet done
    if (!context)
	{
		if (!(context = sws_alloc_context()))
			return NULL;

		av_opt_set_int(context, "srcw",       srcW, 0);
		av_opt_set_int(context, "srch",       srcH, 0);
		av_opt_set_int(context, "src_format", srcFormat, 0);
		av_opt_set_int(context, "src_range",  srcRange, 0);
		av_opt_set_int(context, "dstw",       dstW, 0);
		av_opt_set_int(context, "dsth",       dstH, 0);
		av_opt_set_int(context, "dst_format", dstFormat, 0);
		av_opt_set_int(context, "dst_range",  dstRange, 0);
		av_opt_set_int(context, "sws_flags",  flags, 0);

        av_opt_set_int(context, "src_h_chr_pos", src_h_chr_pos, 0);
        av_opt_set_int(context, "src_v_chr_pos", src_v_chr_pos, 0);
        av_opt_set_int(context, "dst_h_chr_pos", dst_h_chr_pos, 0);
        av_opt_set_int(context, "dst_v_chr_pos", dst_v_chr_pos, 0);

		EnterCriticalSection(&g_csSWScaler);

		sws_setColorspaceDetails(context,
								sws_getCoefficients(SWS_CS_DEFAULT), srcRange,
								sws_getCoefficients(SWS_CS_DEFAULT), dstRange,
								0, 1<<16, 1<<16);

        if (sws_init_context(context, NULL, NULL) < 0)
		{
			sws_freeContext(context);
			LeaveCriticalSection(&g_csSWScaler);
			return NULL;
		}
		else
			LeaveCriticalSection(&g_csSWScaler);
    }

    return context;
}

/* level can be one of the following increasing values:
AV_LOG_PANIC
AV_LOG_FATAL
AV_LOG_ERROR
AV_LOG_WARNING
AV_LOG_INFO
AV_LOG_VERBOSE
AV_LOG_DEBUG
*/
extern "C" void my_av_log_trace(void* ptr, int level, const char* fmt, va_list vl)
{
	// for g_nLogLevel <= 0: log panic and fatal levels
	// for g_nLogLevel == 1: log panic, fatal and error levels
	// for g_nLogLevel >= 2: log panic ... verbose levels
	if ((g_nLogLevel <= 0 && level >= AV_LOG_ERROR)		||
		(g_nLogLevel == 1 && level >= AV_LOG_WARNING)	||
		(g_nLogLevel >= 2 && level >= AV_LOG_DEBUG))
		return;
	
	// Make the message string limiting its size
	// Note: ffmpeg uses UTF8, I tested _vsnprintf() with
	//       a UTF8 format string and it works well 
	char sUtf8[1024];
	memset(sUtf8, 0, 1024);
	_vsnprintf(sUtf8, 1023, fmt, vl); // set buffer count to 1023 (and not 1024) so we are sure sUtf8[1023] stays NULL (from the above memset)
	CString sMsg(::FromUTF8((const unsigned char *)sUtf8, strlen(sUtf8)));	// strlen() interprets the string as a single-byte character string,
																			// so its return value is always equal to the number of bytes, even
																			// if the string contains multibyte characters

	// Avoid spamming users with api deprecated warnings
#ifndef _DEBUG
	CString sMsgLowerCase(sMsg);
	sMsgLowerCase.MakeLower();
	if (sMsgLowerCase.Find(_T("deprecate")) >= 0)
		return;
#endif

	// Output message string
	::LogLine(_T("%s"), sMsg);
}

extern "C" int my_av_lock_callback(void **mutex, enum AVLockOp op)
{
	// Check
	if (!mutex)
		return 1;  // Error

	LPCRITICAL_SECTION pCS;

	switch (op)
	{
		case AV_LOCK_CREATE :
			pCS = new CRITICAL_SECTION;
			if (pCS)
			{
				InitializeCriticalSection(pCS);
				*mutex = (void*)pCS;
				return 0; // OK
			}
			else
			{
				*mutex = NULL;
				return 1; // Error
			}
		case AV_LOCK_OBTAIN :
			pCS = (LPCRITICAL_SECTION)(*mutex);
			if (pCS)
				EnterCriticalSection(pCS);
			return 0; // OK
		case AV_LOCK_RELEASE :
			pCS = (LPCRITICAL_SECTION)(*mutex);
			if (pCS)
				LeaveCriticalSection(pCS);
			return 0; // OK
		case AV_LOCK_DESTROY :
			pCS = (LPCRITICAL_SECTION)(*mutex);
			if (pCS)
			{
				DeleteCriticalSection(pCS);
				delete pCS;
				*mutex = NULL;
			}
			return 0; // OK
		default :
			return 1; // Error
	}
}

#endif

// Handle the abort() calls
// Note: when returning from this function
// the program terminates with an exit code of 3
extern "C" void my_handle_aborts(int signal)
{
	if (signal == SIGABRT)
	{
		LogLine(_T("%s"), ML_STRING(1818, "Aborting") + _T(" ") + APPNAME_NOEXT);
#ifdef VIDEODEVICEDOC
		if (!((CUImagerApp*)AfxGetApp())->m_bServiceProcess)
#endif
		{
			ShellExecute(NULL, _T("open"), g_sLogFileName, NULL, NULL, SW_SHOWNORMAL);
		}
	}
}

BOOL CUImagerApp::Pump()
{
	// Pump messages until none are left in the queue
	MSG msg;
	while (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) // returns TRUE also for WM_QUIT messages
	{
		// Note: from the UI thread all the following pump functions call AfxInternalPumpMessage()
		//       AfxGetApp()->PumpMessage() = AfxGetThread()->PumpMessage() = AfxPumpMessage()
		if (!::AfxGetThread()->PumpMessage()) // returns FALSE for WM_QUIT messages
		{
#ifdef _DEBUG
			// Avoid the ASSERT(FALSE) in AfxInternalPumpMessage()
			::AfxGetThreadState()->m_nDisablePumpCount = 0;
#endif

			// If we get a WM_QUIT then CWinThread::PumpMessage() removes WM_QUIT
			// from the message queue and returns FALSE. But we cannot exit 
			// CWinThread::Run() from this point, we must repost WM_QUIT so that
			// CWinThread::PumpMessage() in CWinThread::Run() returns FALSE and 
			// the app can terminate
			::PostQuitMessage(0);

			return FALSE;
		}
	}

	return TRUE;
}

BOOL CUImagerApp::InitInstance() // Returning FALSE calls ExitInstance()!
{
	CInstanceChecker* pInstanceChecker = NULL;
	try
	{
		// A process can be started with hidden set
		if (m_nCmdShow == SW_HIDE)
			m_bHideMainFrame = TRUE;

		// Init common controls
		INITCOMMONCONTROLSEX InitCtrls;
		InitCtrls.dwSize = sizeof(InitCtrls);
		// Set this to include all the common control classes you want to use
		// in your application
		InitCtrls.dwICC = ICC_WIN95_CLASSES | ICC_LINK_CLASS;
		InitCommonControlsEx(&InitCtrls);

		// Call base class implementation
		CWinApp::InitInstance();

		// Initialize OLE libraries
		// Note: AfxWinTerm() (called after ExitInstance()) will invoke AfxOleTerm()
		//       which does the clean-up, no need to do that in ExitInstance()
		if (!AfxOleInit()) // This calls ::CoInitialize(NULL) internally
		{
			::AfxMessageBox(IDP_OLE_INIT_FAILED, MB_OK | MB_ICONSTOP);
			throw (int)0;
		}

		// Avoid the "Server Busy" dialog while scanning
		AfxOleGetMessageFilter()->SetMessagePendingDelay(60000);		// set to 60 sec just in case the below two calls are not working
		AfxOleGetMessageFilter()->EnableNotRespondingDialog(FALSE);		// https://support.microsoft.com/en-us/kb/2971655
		AfxOleGetMessageFilter()->EnableBusyDialog(FALSE);				// https://support.microsoft.com/en-us/kb/248019
		
		// Enable support for containment of OLE controls
		AfxEnableControlContainer();

		// Init RichEdit2
		AfxInitRichEdit2();

		// Get Module Name and Split it
		TCHAR szDrive[_MAX_DRIVE];
		TCHAR szDir[_MAX_DIR];
		TCHAR szName[_MAX_FNAME];
		TCHAR szProgramName[MAX_PATH];
		if (::GetModuleFileName(NULL, szProgramName, MAX_PATH) == 0)
			throw (int)0;
		_tsplitpath(szProgramName, szDrive, szDir, szName, NULL);
		CString sDrive(szDrive);
		CString sDir(szDir);
		CString sName(szName);
		CString sDriveDir = sDrive + sDir;

		// Set the current directory to the program's directory.
		// When double-clicking a file the current directory is set
		// to the clicked file's directory -> change the current
		// directory to avoid locking the directory from which we
		// double-clicked!
		::SetCurrentDirectory(sDriveDir); // Locks program's dir

		// Get configuration files directory
		// Note: create directory only if VIDEODEVICEDOC defined
		BOOL bIsConfigFilesDirConfigured;
		CString sConfigFilesDir = GetConfigFilesDir(&bIsConfigFilesDirConfigured);
#ifdef VIDEODEVICEDOC
		if (!::IsExistingDir(sConfigFilesDir))
		{
			if (!::CreateDir(sConfigFilesDir))
				::ShowErrorMsg(::GetLastError(), TRUE);
		}
#endif

		// Get system temporary folder
		TCHAR szTempPath[MAX_PATH];
		memset(szTempPath, 0, MAX_PATH * sizeof(TCHAR));
		::GetTempPath(MAX_PATH, szTempPath); // the returned string ends with a backslash
		CString sSysTempDir(szTempPath);
		if (sSysTempDir.IsEmpty())
			sSysTempDir = CString(_T("C:\\Temp\\"));

		// Set default location for Document Root
		// Note: sSysTempDir is supposed to be on a drive where we can write
#ifdef VIDEODEVICEDOC
		m_sMicroApacheDocRoot = ::GetDriveName(sSysTempDir);
		m_sMicroApacheDocRoot.TrimRight(_T('\\'));
		m_sMicroApacheDocRoot += _T("\\") + CString(APPNAME_NOEXT);
#endif

		// Init Debug Trace and Log File
		// (containing folder is only created when the Log File is written)
		::InitTraceLogFile(	sConfigFilesDir + _T("\\") + LOGNAME_EXT,
							MAX_LOG_FILE_SIZE);

		// Handle the abort() calls
		signal(SIGABRT, &my_handle_aborts);

		// Don't show the abort message box and don't call Dr. Watson to make a crash dump
		_set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);

		// Use registry?
		BOOL bUseRegistry = TRUE;
#ifndef _DEBUG
		CString sSoftwareCompany = CString(_T("Software\\")) + CString(MYCOMPANY) + CString(_T("\\"));
		if (::IsRegistryKey(HKEY_LOCAL_MACHINE, sSoftwareCompany + sName)) // is application installed?
		{
			CString sInstallDir = ::GetRegistryStringValue(	HKEY_LOCAL_MACHINE,
															sSoftwareCompany + sName,
															_T("Install_Dir"));
			if (!::AreSamePath(sInstallDir, sDriveDir))
				bUseRegistry = FALSE;
		}
		else
			bUseRegistry = FALSE;
#endif
		int nUseRegistry = GetConfiguredUseRegistry();
		if (nUseRegistry == 0)
			bUseRegistry = FALSE;
		else if (nUseRegistry == 1)
			bUseRegistry = TRUE;

		// Parse command line for standard shell commands, DDE, file open
		CUImagerCommandLineInfo cmdInfo;
		ParseCommandLine(cmdInfo); // m_bHideMainFrame is ev. set to TRUE here

		// Registry key under which the settings are stored.
		// This Will create the HKEY_CURRENT_USER\Software\MYCOMPANY key
		if (bUseRegistry)
			SetRegistryKey(MYCOMPANY);
		else
		{
			// First free the string allocated by MFC at CWinApp startup.
			// The string is allocated before InitInstance is called.
			free((void*)m_pszProfileName);

			// Change the name of the .INI file.
			// The CWinApp destructor will free the memory.
			if (bIsConfigFilesDirConfigured)
				m_pszProfileName = _tcsdup(sConfigFilesDir + _T("\\") + sName + _T(".ini"));
			else
				m_pszProfileName = _tcsdup(sDriveDir + sName + _T(".ini"));

			// Create ini file's directory if not existing
			CString sProfileNamePath = ::GetDriveAndDirName(m_pszProfileName);
			if (!::IsExistingDir(sProfileNamePath))
			{
				if (!::CreateDir(sProfileNamePath))
					::ShowErrorMsg(::GetLastError(), TRUE);
			}

			// Force a unicode ini file by writing the UTF16-LE BOM (FFFE)
			if (!::IsExistingFile(m_pszProfileName))
			{
				const WORD wBOM = 0xFEFF;
				DWORD NumberOfBytesWritten;
				HANDLE hFile = ::CreateFile(m_pszProfileName,
											GENERIC_WRITE, 0, NULL,
											CREATE_NEW,
											FILE_ATTRIBUTE_NORMAL, NULL);
				if (hFile != INVALID_HANDLE_VALUE)
				{
					::WriteFile(hFile, &wBOM, sizeof(WORD), &NumberOfBytesWritten, NULL);
					::CloseHandle(hFile);
				}
			}
		}

		// Let the possibly running UI process terminate,
		// especially when doing a logoff
#ifdef VIDEODEVICEDOC
		if (m_bServiceProcess)
			::Sleep(CONTACAMSERVICE_STARTUP_SLEEP);
#endif

		// Single Instance
		// (if VIDEODEVICEDOC defined -> single instance is always set, see constructor)
#ifndef VIDEODEVICEDOC
		m_bSingleInstance = (BOOL)GetProfileInt(_T("GeneralApp"), _T("SingleInstance"), FALSE);
#endif
		if (
#ifdef VIDEODEVICEDOC			
			!m_bServiceProcess &&
#endif
			m_bSingleInstance)
		{
			// Cannot pass filenames between Ascii and Unicode
			// -> two different instances for them (old non-UNICODE build was using "_Ascii")
			pInstanceChecker = new CInstanceChecker(CString(APPNAME_NOEXT) + CString(_T("_Unicode")));
			if (!pInstanceChecker)
				throw (int)0;
			pInstanceChecker->ActivateChecker();
			if (pInstanceChecker->PreviousInstanceRunning())
			{
				// Send file name(s) and shell command to previous instance
				CString sFileNames;
				if (cmdInfo.m_strFileNames.GetSize() <= 1)
					sFileNames = cmdInfo.m_strFileName;
				else
				{
					sFileNames = CString(_T("\"")) + cmdInfo.m_strFileName + CString(_T("\""));
					for (int i = 1 ; i < cmdInfo.m_strFileNames.GetSize() ; i++)
						sFileNames += CString(_T(" \"")) + cmdInfo.m_strFileNames[i] + CString(_T("\""));
				}
				pInstanceChecker->ActivatePreviousInstance(sFileNames,
														(ULONG_PTR)cmdInfo.m_nShellCommand);
				throw (int)0;
			}
		}
		
		// First Time that the App runs after Install (or Upgrade)
		m_bFirstRun = (BOOL)GetProfileInt(_T("GeneralApp"), _T("FirstRun"), FALSE);

		// First run ever or after an Uninstall
		m_bFirstRunEver = (BOOL)GetProfileInt(_T("GeneralApp"), _T("FirstRunEver"), TRUE);

		// Fix inconsistency
		if (m_bFirstRunEver && !m_bFirstRun)
			m_bFirstRun = TRUE;

		// Silent install
		m_bSilentInstall = (BOOL)GetProfileInt(_T("GeneralApp"), _T("SilentInstall"), FALSE);

		// Set Tray Icon flag
		if (!m_bHideMainFrame)
		{
#ifdef VIDEODEVICEDOC
			if (m_bFirstRunEver)
				WriteProfileInt(_T("GeneralApp"), _T("TrayIcon"), TRUE);
#endif
			m_bTrayIcon = (BOOL)GetProfileInt(_T("GeneralApp"), _T("TrayIcon"), FALSE);
		}

		// Init Global Helper Functions
		::InitHelpers();

		// Init for the PostDelayedMessage() Function
		CPostDelayedMessageThread::Init();

		// Get the optimal number of threads to use for a task
		if ((m_nThreadCount = std::thread::hardware_concurrency()) == 0)
			m_nThreadCount = 1; // guard against std::thread::hardware_concurrency() returning 0

		// Loads the 6 MRU Files and loads also the m_nNumPreviewPages
		// variable for the PrintPreview.
		// This Function uses 2 Keys (or Sections for INI Files):
		// Recent File List and Settings (to store m_nNumPreviewPages)
		LoadStdProfileSettings(6);

		// Picture Doc Template Registration
		m_pPictureDocTemplate = new CUImagerMultiDocTemplate(
			IDR_PICTURE,
			RUNTIME_CLASS(CPictureDoc),
			RUNTIME_CLASS(CPictureChildFrame),
			RUNTIME_CLASS(CPictureView));
		if (!m_pPictureDocTemplate)
			throw (int)0;
		AddDocTemplate(m_pPictureDocTemplate);

		// Video Device Doc Template Registration
#ifdef VIDEODEVICEDOC
		m_pVideoDeviceDocTemplate = new CUImagerMultiDocTemplate(
			IDR_VIDEODEVICE,
			RUNTIME_CLASS(CVideoDeviceDoc),
			RUNTIME_CLASS(CVideoDeviceChildFrame),
			RUNTIME_CLASS(CVideoDeviceView));
		if (!m_pVideoDeviceDocTemplate)
			throw (int)0;
		AddDocTemplate(m_pVideoDeviceDocTemplate);
#endif

		// Create Named Mutex For Installer / Uninstaller
		m_hAppMutex = ::CreateMutex(NULL, FALSE, APPMUTEXNAME);

		// Check for MMX
		if (!g_bMMX)
		{
			::AfxMessageBox(ML_STRING(1170, "Error: No MMX Processor"), MB_OK | MB_ICONSTOP);
			throw (int)0;
		}

		// Zip Settings
		m_Zip.SetAdvanced(65535 * 50, 16384 * 50, 32768 * 50);

		// Init Gdiplus
		// (GDI+ is not designed for multithreading, use it only from the main UI thread)
		Gdiplus::GdiplusStartupInput gdiplusStartupInput;
		Gdiplus::GdiplusStartup(&m_GdiplusToken, &gdiplusStartupInput, NULL);

		// Init YUV <-> RGB LUT
		::InitYUVToRGBTable();
		::InitRGBToYUVTable();

#ifdef VIDEODEVICEDOC
		// Init YUV <-> YUV LUT
		::InitYUVToYUVTable();

		// The lock manager creates a avcodec mutex for avcodec_open2
		// and a avformat mutex for avisynth_read_header, avisynth_read_close,
		// ff_tls_init, ff_tls_deinit
		av_lockmgr_register(my_av_lock_callback);

		// av_lockmgr_register is not creating a mutex for the sws scaler,
		// sws_setColorspaceDetails and sws_init_context are not thread safe,
		// protect them with a critical section
		::InitializeCriticalSection(&g_csSWScaler);
		g_bSWScalerCSInited = TRUE;

		// ffmpeg register log messages callback
		av_log_set_callback(my_av_log_trace);

		// AV format register all and init network
		av_register_all();
		avformat_network_init();

		// Init WinSock 2.2
		WSADATA wsadata;
		if (::WSAStartup(MAKEWORD(2, 2), &wsadata) == 0)
		{
			/* Confirm that the WinSock DLL supports 2.2.*/
			/* Note that if the DLL supports versions greater    */
			/* than 2.2 in addition to 2.2, it will still return */
			/* 2.2 in wVersion since that is the version we      */
			/* requested.                                        */
			if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wVersion) != 2)
			{
				::WSACleanup();
				::AfxMessageBox(ML_STRING(1171, "No usable WinSock DLL found"), MB_OK | MB_ICONSTOP);
				throw (int)0;
			}
		}
		else
		{
			::AfxMessageBox(ML_STRING(1171, "No usable WinSock DLL found"), MB_OK | MB_ICONSTOP);
			throw (int)0;
		}
#endif

		// Create main MDI Frame window (before stopping service so that the tray icon gets created)
		CMainFrame* pMainFrame = new CMainFrame;
		if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
		{
			delete pMainFrame;
			throw (int)0;
		}
		m_pMainWnd = pMainFrame;

		/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		!! Do not throw after this point, debugger asserts if returning FALSE !!
		!! from this function when the MainFrame has already been created     !! 
		!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

		// If this is the first instance of our App then track it
		// so any other instances can find us
		if (pInstanceChecker)
		{
			pInstanceChecker->TrackFirstInstanceRunning();
			delete pInstanceChecker;
			pInstanceChecker = NULL;
		}

		// Do stop from Service, this may take some time...
#ifdef VIDEODEVICEDOC
		if (!m_bServiceProcess && GetContaCamServiceState() == CONTACAMSERVICE_RUNNING)
		{
			CServiceControlEndProcThread ServiceControlEndProcThread;
			::AfxGetMainFrame()->PopupNotificationWnd(	ML_STRING(1764, "Starting") + _T(" ") + APPNAME_NOEXT,
														ML_STRING(1565, "Please wait..."),
														0);
			ServiceControlEndProcThread.Start();
			do 
			{
				Pump();	// pump messages to show notification window
				::Sleep(10);
			}
			while (ServiceControlEndProcThread.IsAlive());
			::AfxGetMainFrame()->CloseNotificationWnd();
			Pump();		// pump messages to hide notification window
			m_bDoStartFromService = TRUE;
		}
#endif

		// Application temporary directory is _T('\\') terminated
		// (its content must be delete exactly here, that's after the service
		// process stopped and before ProcessShellCommand() execution)
		// Note: m_sAppTempDir is used later on in BatchProcDlg.cpp, ImageInfoDlg.cpp,
		//       MainFrm.cpp (for scan), PictureDoc.cpp, SettingsDlg.cpp,
		//       uImager.cpp (for shrink pictures and send mail), VideoDeviceDoc.cpp
		CString sSharedTempFolderPostfix;
		if (!m_bSingleInstance)
			sSharedTempFolderPostfix = _T("Shared");
		m_sAppTempDir = GetConfiguredTempDir();											// returns no trailing backslash
		if (m_sAppTempDir.IsEmpty())
			m_sAppTempDir = sSysTempDir + sName + sSharedTempFolderPostfix + _T("\\");	// sSysTempDir ends with a backslash
		else
			m_sAppTempDir = m_sAppTempDir + _T("\\") + sName + sSharedTempFolderPostfix + _T("\\");
		if (::IsExistingDir(m_sAppTempDir))
		{
			if (sSharedTempFolderPostfix.IsEmpty())
				::DeleteDirContent(m_sAppTempDir);
		}
		else
		{
			if (!::CreateDir(m_sAppTempDir))
				::ShowErrorMsg(::GetLastError(), TRUE);
		}

		// Dispatch commands specified on the command line,
		// returns FALSE if printing or if file opening fails
		if (!ProcessShellCommand(cmdInfo))
		{
			m_pMainWnd->PostMessage(WM_CLOSE);
			return TRUE;
		}

		// Hiding mainframe?
		if (m_bHideMainFrame)
			m_nCmdShow = SW_HIDE;

		// The main window has been initialized, so show, place it and update it
		pMainFrame->ShowWindow(m_nCmdShow);
		pMainFrame->UpdateWindow();

		// ProcessShellCommand() is called before the MainFrame is shown,
		// that causes a problem with the document titles, they are not shown!
		// -> Update Title here:
		CMDIChildWnd* pChild = pMainFrame->MDIGetActive();
		if (pChild)
		{
			CView* pView = (CView*)pChild->GetActiveView();
			if (pView)
			{
				CDocument* pDoc = pView->GetDocument();
				if (pDoc)
					pDoc->UpdateFrameCounts(); // will cause name change in views
			}
		}

		// Load Settings has to be here for the Window Placement restore to work!
		LoadSettings(m_nCmdShow);

		// First time that the App runs after Install (or Upgrade)
		if (m_bFirstRun)
		{
			// First time ever that the App runs or after a uninstall
			if (m_bFirstRunEver)
			{
#ifdef VIDEODEVICEDOC
				// Enable autostart
				Autostart(TRUE);
#endif
				// Reset first run ever flag
				WriteProfileInt(_T("GeneralApp"), _T("FirstRunEver"), FALSE);
			}
		
			// - Update file associations #ifndef VIDEODEVICEDOC
			//   (necessary when changing the icons)
			// - Unassociate all file types #ifdef VIDEODEVICEDOC
			//   (file association has been removed with version 5.0.0)
			UpdateFileAssociations();

#ifndef VIDEODEVICEDOC
			// Open Settings Dialog for file association and other preferences
			if (!m_bSilentInstall)
				OnFileSettings();
#endif
			// Reset first run flag
			WriteProfileInt(_T("GeneralApp"), _T("FirstRun"), FALSE);
		}

		// Reset silent install flag
		WriteProfileInt(_T("GeneralApp"), _T("SilentInstall"), FALSE);

#ifdef VIDEODEVICEDOC
		// Log the starting of the application
		CString sMsg(CString(APPNAME_NOEXT) + _T(" ") + APPVERSION + _T(" (") + CString(_T(__TIME__)) + CString(_T(" ")) + CString(_T(__DATE__)) + _T(")"));
		if (m_bServiceProcess)
			::LogLine(_T("%s"), ML_STRING(1764, "Starting") + _T(" ") + sMsg + _T(" - SERVICE MODE"));
		else
			::LogLine(_T("%s"), ML_STRING(1764, "Starting") + _T(" ") + sMsg);

		// Warn if Sleep and/or Hibernation is enabled
		GUID* pActivePolicyGuid = NULL;
		if (::PowerGetActiveScheme(NULL, &pActivePolicyGuid) == ERROR_SUCCESS)
		{
			SYSTEM_POWER_CAPABILITIES SystemPowerCapabilities = {};
			::GetPwrCapabilities(&SystemPowerCapabilities);
			const GUID subGUID = GUID_SLEEP_SUBGROUP;

			// Sleep?
			GUID activeGUID = GUID_STANDBY_TIMEOUT;
			DWORD dwStandbyTimeout = 0;
			if (SystemPowerCapabilities.SystemS1 || SystemPowerCapabilities.SystemS2 || SystemPowerCapabilities.SystemS3)
			{
				// The if-check is necessary because PowerReadACValueIndex can return a value other than 0 also if SystemS1, SystemS2 and SystemS3 are all FALSE
				::PowerReadACValueIndex(NULL, pActivePolicyGuid, &subGUID, &activeGUID, &dwStandbyTimeout);
			}

			// Hibernation?
			// Note: Windows 10 adds a hiber file type:
			// - Full: Supports hibernate, hybrid sleep, fast startup (hiberboot)
			//   (powercfg /h /type full)
			// - Reduced: only supports fast startup (hiberboot)
			//   (first run powercfg /h /size 0 and then powercfg /h /type reduced)
#ifndef HIBERFILE_TYPE_REDUCED
#define HIBERFILE_TYPE_REDUCED    0x01
#endif
#ifdef NTDDI_WINTHRESHOLD // only defined when using Windows 10 SDK or higher
#if (NTDDI_VERSION < NTDDI_WINTHRESHOLD)
			BYTE HiberFileType = SystemPowerCapabilities.spare3[0];		// in Windows 10 it's HiberFileType in older system it is set to 0
#else
			BYTE HiberFileType = SystemPowerCapabilities.HiberFileType;	// added in Windows 10
#endif
#else
			BYTE HiberFileType = SystemPowerCapabilities.spare3[0];		// in Windows 10 it's HiberFileType in older system it is set to 0
#endif
			activeGUID = GUID_HIBERNATE_TIMEOUT;
			DWORD dwHibernateTimeout = 0;
			if (SystemPowerCapabilities.SystemS4 && SystemPowerCapabilities.HiberFilePresent && HiberFileType != HIBERFILE_TYPE_REDUCED)
			{
				// The if-check is necessary because PowerReadACValueIndex can return a value other than 0 also if
				// SystemS4 == FALSE or
				// HiberFilePresent == FALSE or
				// HiberFileType == HIBERFILE_TYPE_REDUCED
				::PowerReadACValueIndex(NULL, pActivePolicyGuid, &subGUID, &activeGUID, &dwHibernateTimeout);
			}

			// Warning
			if (dwStandbyTimeout != 0 && dwHibernateTimeout != 0)
			{
				sMsg = ML_STRING(1570, "In Power Options disable: ") + ML_STRING(1571, "Sleep") + _T(" + ") + ML_STRING(1572, "Hibernate");
				if (!m_bServiceProcess)
					::AfxGetMainFrame()->PopupNotificationWnd(APPNAME_NOEXT, sMsg);
				::LogLine(_T("%s"), sMsg);
			}
			else if (dwStandbyTimeout != 0)
			{
				sMsg = ML_STRING(1570, "In Power Options disable: ") + ML_STRING(1571, "Sleep");
				if (!m_bServiceProcess)
					::AfxGetMainFrame()->PopupNotificationWnd(APPNAME_NOEXT, sMsg);
				::LogLine(_T("%s"), sMsg);
			}
			else if (dwHibernateTimeout != 0)
			{
				sMsg = ML_STRING(1570, "In Power Options disable: ") + ML_STRING(1572, "Hibernate");
				if (!m_bServiceProcess)
					::AfxGetMainFrame()->PopupNotificationWnd(APPNAME_NOEXT, sMsg);
				::LogLine(_T("%s"), sMsg);
			}
		}
		if (pActivePolicyGuid)
			::LocalFree(pActivePolicyGuid);

		// Update / create doc root index.php and config file for microapache
		CVideoDeviceDoc::MicroApacheUpdateMainFiles();

		// Start Micro Apache
		// Note: make sure the web server is running because one of the below started
		//       devices could take the feed from localhost's push.php or poll.php
		if (m_bStartMicroApache)
		{
			// Make sure nothing is left over from an abruptly terminated ContaCam
			CVideoDeviceDoc::MicroApacheShutdown(MICROAPACHE_TIMEOUT_MS);

			// Go
			if (!CVideoDeviceDoc::MicroApacheStart(MICROAPACHE_TIMEOUT_MS))
			{
				sMsg = ML_STRING(1475, "Failed to start the web server!");
				if (!m_bServiceProcess)
				{
					CString sMicroapacheConfigFile = CVideoDeviceDoc::MicroApacheGetConfigFileName();
					CString sMicroapacheLogFile = ::GetDriveAndDirName(sMicroapacheConfigFile) + MICROAPACHE_LOGNAME_EXT;
					if (::IsExistingFile(sMicroapacheConfigFile) && ::IsExistingFile(sMicroapacheLogFile))
						::AfxGetMainFrame()->PopupNotificationWnd(sMsg, sMicroapacheLogFile, 0);
					else
					{
						CString sReInstall;
						sReInstall.Format(ML_STRING(1832, "Please re-install %s."), APPNAME_NOEXT);
						::AfxGetMainFrame()->PopupNotificationWnd(sMsg, sReInstall, 0);
					}
				}
				::LogLine(_T("%s"), sMsg);
			}
		}

		// Autorun devices with a delay of m_dwFirstStartDelayMs
		CPostDelayedMessageThread::PostDelayedMessage(	::AfxGetMainFrame()->GetSafeHwnd(),
														WM_AUTORUN_VIDEODEVICES,
														m_dwFirstStartDelayMs,
														0, 0);
		sMsg.Format(ML_STRING(1569, "Cameras autostart delay %d sec"), (int)(m_dwFirstStartDelayMs / 1000));
		if (!m_bServiceProcess)
			::AfxGetMainFrame()->StatusText(sMsg);
		::LogLine(_T("%s"), sMsg);

		// Flag indicating that the auto-starts have been executed
		m_bAutostartsExecuted = TRUE;
#endif

		// Redraw program's background
		::AfxGetMainFrame()->m_MDIClientWnd.Invalidate();

		// Start Donor Email Validation Thread
		g_DonorEmailValidateThread.Start();

		return TRUE;
	}
	catch (int)
	{
		if (pInstanceChecker)
			delete pInstanceChecker;
		return FALSE;
	}
	catch (CException* e)
	{
		if (pInstanceChecker)
			delete pInstanceChecker;
		e->ReportError();
		e->Delete();
		return FALSE;
	}
}

CUImagerMultiDocTemplate::CUImagerMultiDocTemplate(	UINT nIDResource, CRuntimeClass* pDocClass,
													CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass) :
													CMultiDocTemplate(nIDResource, pDocClass, pFrameClass, pViewClass)
{

}

CUImagerMultiDocTemplate::~CUImagerMultiDocTemplate()
{

}

CDocument* CUImagerMultiDocTemplate::OpenDocumentFile(	LPCTSTR lpszPathName,
														BOOL bMakeVisible/*=TRUE*/)
{
	// Open Document
	CDocument* pDoc = CMultiDocTemplate::OpenDocumentFile(	lpszPathName,
															bMakeVisible);

	// Init Menu Positions
	::AfxGetMainFrame()->InitMenuPositions(pDoc);
	
	return pDoc;
}

IMPLEMENT_DYNAMIC(CUImagerMultiDocTemplate, CMultiDocTemplate)



/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

CAboutDlg::CAboutDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAboutDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAboutDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Application Name
	CEdit* pAppName = (CEdit*)GetDlgItem(IDC_APPNAME);
	pAppName->SetWindowText(APPNAME_NOEXT);

	// Application Version
	CEdit* pAppVer = (CEdit*)GetDlgItem(IDC_APPVER);
	pAppVer->SetWindowText(APPVERSION);

	// CPU
	CEdit* pCpuEdit = (CEdit*)GetDlgItem(IDC_CPU);
	CString sCpu;
	sCpu.Format(_T("CPU  %u MHz (%d thread)"), ::GetProcessorSpeedMHz(), ((CUImagerApp*)::AfxGetApp())->m_nThreadCount);
	pCpuEdit->SetWindowText(sCpu);

	// Total Physical Memory
	CString sPhysMem;
	sPhysMem.Format(	_T("RAM  %0.1f ") + ML_STRING(1826, "GB") + _T(" (") +
						ML_STRING(1820, "usable") + _T(" %0.1f ") + ML_STRING(1826, "GB") + _T(", ") +
						ML_STRING(1821, "addressable") + _T(" %0.1f ") + ML_STRING(1826, "GB") + _T(")"),
						(double)g_nPCInstalledPhysRamMB / 1024.0,
						(double)g_nOSUsablePhysRamMB / 1024.0,
						(double)g_nAppUsableAddressSpaceMB / 1024.0);
	CEdit* pPhysMem = (CEdit*)GetDlgItem(IDC_PHYSMEM);
	pPhysMem->SetWindowText(sPhysMem);

	// Compilation Time and Compiler Version
	CString sCompVer;
	sCompVer.Format(_T("%02d.%02d.%05d"), _MSC_VER / 100, _MSC_VER % 100, _MSC_FULL_VER % 100000);
	CString sCompilation;
	sCompilation =	CString(_T("(")) +
					CString(_T(__TIME__)) +
					CString(_T("  ")) +
					CString(_T(__DATE__)) +
					CString(_T("  ")) +
					sCompVer +
					CString(_T(")"));
	SetDlgItemText(IDC_VERSION, sCompilation);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAboutDlg::OnSyslinkHomepage(NMHDR* pNMHDR, LRESULT* pResult)
{
	::ShellExecute(NULL, _T("open"), MYCOMPANY_PAGE, NULL, NULL, SW_SHOWNORMAL);
	*pResult = 0;
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_HOMEPAGE, OnSyslinkHomepage)
	ON_NOTIFY(NM_RETURN, IDC_SYSLINK_HOMEPAGE, OnSyslinkHomepage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// Popup About Dialog
void CUImagerApp::OnAppAbout()
{	
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// Popup Credits Dialog
void CUImagerApp::OnAppCredits() 
{
	CCreditsDlg creditsDlg;
	creditsDlg.DoModal();
}

// Popup License Dialog
void CUImagerApp::OnAppLicense() 
{
	// Attention: never read or modify m_sDonorEmail while the thread is running
	g_DonorEmailValidateThread.Kill();

	// Init m_sEmail variable and popup dialog
	CLicenseDlg licenseDlg;
	licenseDlg.m_sEmail = g_DonorEmailValidateThread.m_sDonorEmail;
	if (licenseDlg.DoModal() == IDOK)
	{
		// Update and validate email
		BeginWaitCursor();
		licenseDlg.m_sEmail.Trim();
		g_DonorEmailValidateThread.m_sDonorEmail = licenseDlg.m_sEmail;
		WriteProfileString(_T("GeneralApp"), _T("DonorEmail"), g_DonorEmailValidateThread.m_sDonorEmail);
		int ret = g_DonorEmailValidateThread.DonorEmailValidate();
		EndWaitCursor();

		// Inform user
		if (ret == 0)
		{
			CTaskDialog dlg(ML_STRING(1738, "Please contact us with the support email received after the donation."),
							ML_STRING(1737, "Provided email is unknown"),
							APPNAME_NOEXT,
							TDCBF_OK_BUTTON,
							TDF_ENABLE_HYPERLINKS | TDF_USE_COMMAND_LINKS | TDF_SIZE_TO_CONTENT);
			dlg.SetMainIcon(TD_ERROR_ICON);
			dlg.DoModal(::AfxGetMainFrame()->GetSafeHwnd());
		}
		else if (ret == -1)
		{
			CTaskDialog dlg(ML_STRING(1740, "Please connect to the internet and try again."),
							ML_STRING(1739, "Provided email cannot be verified"),
							APPNAME_NOEXT,
							TDCBF_OK_BUTTON,
							TDF_ENABLE_HYPERLINKS | TDF_USE_COMMAND_LINKS | TDF_SIZE_TO_CONTENT);
			dlg.SetMainIcon(TD_ERROR_ICON);
			dlg.DoModal(::AfxGetMainFrame()->GetSafeHwnd());
		}
	}
}

// Show the internet site of Manual & FAQ
void CUImagerApp::OnAppManual() 
{
	::ShellExecute(NULL, _T("open"), MANUAL_ONLINE_PAGE, NULL, NULL, SW_SHOWNORMAL);
}

// Popup Settings Dialog
void CUImagerApp::OnFileSettings() 
{
#ifdef VIDEODEVICEDOC
	CSettingsDlgVideoDeviceDoc dlg;
	dlg.DoModal();
#else
	CSettingsDlg dlg;
	if (dlg.DoModal() == IDOK)
	{
		// Confirm the association in the OS dialog
		if (!SettingsPageAppsDefaults(_T("SystemSettings_DefaultApps_Photos")))
		{
			// Fallback for older Windows
			::ShellExecute(	NULL, NULL, _T("control.exe"),
							_T("/name Microsoft.DefaultPrograms /page pageFileAssoc"),
							NULL, SW_SHOWNORMAL);
		}
	}
#endif
}

void CUImagerApp::OnFileOpen()
{
	if (!::AfxGetMainFrame()->m_bFullScreenMode)
	{
		TCHAR* FileNames = new TCHAR[MAX_FILEDLG_PATH];
		TCHAR* InitDir = new TCHAR[MAX_FILEDLG_PATH];
		FileNames[0] = _T('\0');
		InitDir[0] = _T('\0');
		CString sActiveDocDriveAndDir(GetActiveDocDriveAndDirName());
		sActiveDocDriveAndDir.TrimRight(_T('\\'));
		m_sLastOpenedDir.TrimRight(_T('\\'));
		if (::IsExistingDir(sActiveDocDriveAndDir))
			_tcscpy(InitDir, (LPCTSTR)sActiveDocDriveAndDir);
		else if (::IsExistingDir(m_sLastOpenedDir))
			_tcscpy(InitDir, (LPCTSTR)m_sLastOpenedDir);
		else
			_tcscpy(InitDir, (LPCTSTR)::GetSpecialFolderPath(CSIDL_MYPICTURES));
		CPreviewFileDlg dlgFile(TRUE,
								m_bFileDlgPreview,
								NULL,
								NULL,
								NULL,
								::AfxGetMainFrame());
		dlgFile.m_ofn.lpstrInitialDir = InitDir;
		dlgFile.m_ofn.lpstrDefExt = _T("bmp");
		dlgFile.m_ofn.lpstrCustomFilter = NULL;
		dlgFile.m_ofn.lpstrFilter = 
					_T("Supported Files (*.bmp;*.gif;*.jpg;*.tif;*.png;*.pcx;*.emf;*.jxr;*.webp;*.heic;*.avif)\0")
					_T("*.bmp;*.dib;*.gif;*.png;*.jpg;*.jpeg;*.jpe;*.jfif;*.thm;*.tif;*.tiff;*.jfx;*.pcx;*.emf;*.jxr;*.webp;*.heic;*.avif\0")
					_T("All Files (*.*)\0*.*\0")
					_T("Windows Bitmap (*.bmp;*.dib)\0*.bmp;*.dib\0")
					_T("Graphics Interchange Format (*.gif)\0*.gif\0")
					_T("Portable Network Graphics (*.png)\0*.png\0")
					_T("JPEG File Interchange Format (*.jpg;*.jpeg;*.jpe;*.jfif;*.thm)\0*.jpg;*.jpeg;*.jpe;*.jfif;*.thm\0")
					_T("Tag Image File Format (*.tif;*.tiff;*.jfx)\0*.tif;*.tiff;*.jfx\0")
					_T("PC Paintbrush (*.pcx)\0*.pcx\0")
					_T("Enhanced Metafile (*.emf)\0*.emf\0")
					_T("JPEG XR (*.jxr)\0*.jxr\0")
					_T("Weppy Format (*.webp)\0*.webp\0")
					_T("High Efficiency Image Format (*.heic)\0*.heic\0")
					_T("AV1 Image File Format (*.avif)\0*.avif\0");
		dlgFile.m_ofn.Flags |= OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT;
		dlgFile.m_ofn.lpstrFile = FileNames;
		dlgFile.m_ofn.nMaxFile = MAX_FILEDLG_PATH;

		// Open File Dialog
		if (dlgFile.DoModal() == IDOK)
		{
			// Update preview flag
			m_bFileDlgPreview = dlgFile.m_bPreview;
			WriteProfileInt(_T("GeneralApp"),
							_T("FileDlgPreview"),
							m_bFileDlgPreview);
			
			// Get File(s)
			TCHAR* sSource = FileNames;
			TCHAR FileName[MAX_PATH];
			FileName[0] = _T('\0');
			TCHAR Path[MAX_PATH];
			Path[0] = _T('\0');
			_tcscpy(Path, (LPCTSTR)sSource);
			while (*sSource != 0)
				sSource++;
			sSource++; // Skip the 0.
			if (*sSource == 0) // If two zeros -> single file selected
			{
				CUImagerMultiDocTemplate* curTemplate = GetTemplateFromFileExtension(Path);
				if (curTemplate == NULL)
				{
					FileTypeNotSupportedMessageBox(Path);
					delete [] FileNames;
					delete [] InitDir;
					return;
				}

				CDocument* pDoc = curTemplate->OpenDocumentFile(NULL);
				if (pDoc)
				{
					if (pDoc->IsKindOf(RUNTIME_CLASS(CPictureDoc)))
					{
						CZoomComboBox* pZoomCB = &(((CPictureToolBar*)((CToolBarChildFrame*)(((CPictureDoc*)pDoc)->GetFrame()))->GetToolBar())->m_ZoomComboBox);
						pZoomCB->SetCurSel(((CPictureDoc*)pDoc)->m_nZoomComboBoxIndex);
						pZoomCB->OnChangeZoomFactor(*((double*)(pZoomCB->GetItemDataPtr(((CPictureDoc*)pDoc)->m_nZoomComboBoxIndex))));

						if (!((CPictureDoc*)pDoc)->LoadPicture(&((CPictureDoc*)pDoc)->m_pDib, Path))
						{
							((CPictureDoc*)pDoc)->CloseDocumentForce();
							delete [] FileNames;
							delete [] InitDir;
							return;
						}
						else
						{
							// Fit to document
							if (!((CPictureDoc*)pDoc)->GetFrame()->IsZoomed())
							{
								((CPictureDoc*)pDoc)->GetView()->GetParentFrame()->SetWindowPos(NULL,
																								0, 0, 0, 0,
																								SWP_NOSIZE |
																								SWP_NOZORDER);
								((CPictureDoc*)pDoc)->GetView()->UpdateWindowSizes(FALSE, FALSE, TRUE);
							}
							((CPictureDoc*)pDoc)->SlideShow(); // No Recursive Slideshow in Paused State
						}
					}
				}

				// Store Last Opened Directory
				TCHAR szDrive[_MAX_DRIVE];
				TCHAR szDir[_MAX_DIR];
				_tsplitpath(Path, szDrive, szDir, NULL, NULL);
				m_sLastOpenedDir = CString(szDrive) + CString(szDir);
				m_sLastOpenedDir.TrimRight(_T('\\'));
				WriteProfileString(	_T("GeneralApp"),
									_T("LastOpenedDir"),
									m_sLastOpenedDir);
			}
			else // multiple files selected
			{
				while (*sSource != 0) // If 0 -> end of file list.
				{
					_tcscpy(FileName, (LPCTSTR)Path);
					_tcscat(FileName, (LPCTSTR)_T("\\"));
					_tcscat(FileName, (LPCTSTR)sSource);

					CUImagerMultiDocTemplate* curTemplate = GetTemplateFromFileExtension(FileName);
					if (curTemplate == NULL)
					{
						FileTypeNotSupportedMessageBox(FileName);
						delete [] FileNames;
						delete [] InitDir;
						return;
					}
					CDocument* pDoc = curTemplate->OpenDocumentFile(NULL);
					if (pDoc)
					{
						if (pDoc->IsKindOf(RUNTIME_CLASS(CPictureDoc)))
						{
							CZoomComboBox* pZoomCB = &(((CPictureToolBar*)((CToolBarChildFrame*)(((CPictureDoc*)pDoc)->GetFrame()))->GetToolBar())->m_ZoomComboBox);
							pZoomCB->SetCurSel(((CPictureDoc*)pDoc)->m_nZoomComboBoxIndex);
							pZoomCB->OnChangeZoomFactor(*((double*)(pZoomCB->GetItemDataPtr(((CPictureDoc*)pDoc)->m_nZoomComboBoxIndex))));

							if (!((CPictureDoc*)pDoc)->LoadPicture(	&((CPictureDoc*)pDoc)->m_pDib,
																	FileName,
																	FALSE,
																	FALSE))
							{
								((CPictureDoc*)pDoc)->CloseDocumentForce();
								delete [] FileNames;
								delete [] InitDir;
								return;
							}
							else
							{
								// Fit to document
								if (!((CPictureDoc*)pDoc)->GetFrame()->IsZoomed())
								{
									((CPictureDoc*)pDoc)->GetView()->GetParentFrame()->SetWindowPos(NULL,
																									0, 0, 0, 0,
																									SWP_NOSIZE |
																									SWP_NOZORDER);
									((CPictureDoc*)pDoc)->GetView()->UpdateWindowSizes(FALSE, FALSE, TRUE);
								}
								((CPictureDoc*)pDoc)->SlideShow(); // No Recursive Slideshow in Paused State
							}
						}
					}

					while (*sSource != 0)
						sSource++;
					sSource++; // Skip the 0.
				}

				// Store Last Opened Directory
				m_sLastOpenedDir = CString(Path);
				m_sLastOpenedDir.TrimRight(_T('\\'));
				WriteProfileString(	_T("GeneralApp"),
									_T("LastOpenedDir"),
									m_sLastOpenedDir);
			}
		}
		else
		{
			m_bFileDlgPreview = dlgFile.m_bPreview;
			WriteProfileInt(_T("GeneralApp"),
							_T("FileDlgPreview"),
							m_bFileDlgPreview);
		}

		// Free
		delete [] FileNames;
		delete [] InitDir;
	}
}

#ifndef CAPTUREBLT
#define CAPTUREBLT	(DWORD)0x40000000 /* Include layered windows like Vista Sidebar */
#endif
void CUImagerApp::CaptureScreenToClipboard()
{
	// Desktop rectangle
	CRect rcDesktop;
	if (::GetSystemMetrics(SM_CXVIRTUALSCREEN) <= 0 ||
		::GetSystemMetrics(SM_CYVIRTUALSCREEN) <= 0)
	{
		rcDesktop.left = 0;
		rcDesktop.top = 0;
		rcDesktop.right = ::GetSystemMetrics(SM_CXSCREEN);
		rcDesktop.bottom = ::GetSystemMetrics(SM_CYSCREEN);
	}
	else
	{
		rcDesktop.left = ::GetSystemMetrics(SM_XVIRTUALSCREEN);
		rcDesktop.top = ::GetSystemMetrics(SM_YVIRTUALSCREEN);
		rcDesktop.right = rcDesktop.left + ::GetSystemMetrics(SM_CXVIRTUALSCREEN);
		rcDesktop.bottom = rcDesktop.top + ::GetSystemMetrics(SM_CYVIRTUALSCREEN);
	}

	// Copy screen to bitmap
	HDC hScreenDC = ::GetDC(NULL);
	HDC hMemDC = ::CreateCompatibleDC(hScreenDC);
	HBITMAP hBitmap = ::CreateCompatibleBitmap(hScreenDC, rcDesktop.Width(), rcDesktop.Height());
	HGDIOBJ hOldBitmap = ::SelectObject(hMemDC, hBitmap);
	::BitBlt(	hMemDC, 0, 0, rcDesktop.Width(), rcDesktop.Height(), hScreenDC,
				rcDesktop.left, rcDesktop.top, SRCCOPY | CAPTUREBLT);

	// Save bitmap to clipboard
	CDib Dib;
	Dib.SetShowMessageBoxOnError(FALSE);
	Dib.SetBitsFromDDB(hBitmap, NULL);
	Dib.EditCopy();

	// Clean-up
	::SelectObject(hMemDC, hOldBitmap);
	::DeleteObject(hBitmap);
	::DeleteDC(hMemDC);
	::ReleaseDC(NULL, hScreenDC);
}

BOOL CUImagerApp::PasteToFile(LPCTSTR lpszFileName, COLORREF crBackgroundColor/*=RGB(255,255,255)*/)
{
	CDib Dib;
	Dib.SetShowMessageBoxOnError(FALSE);
	Dib.SetBackgroundColor(crBackgroundColor);
	Dib.EditPaste();
	if (!Dib.IsValid())
		Dib.AllocateBits(32, BI_RGB, 1, 1, crBackgroundColor);
	CString sExt = ::GetFileExt(lpszFileName);
	if (CDib::IsJPEGExt(sExt))
	{
		if (Dib.HasAlpha() && Dib.GetBitCount() == 32)
		{
			Dib.RenderAlphaWithSrcBackground();
			Dib.SetAlpha(FALSE);
		}
		return Dib.SaveJPEG(lpszFileName, DEFAULT_JPEGCOMPRESSION);
	}
	else if (sExt == _T(".png"))
		return CPictureDoc::SavePNG(lpszFileName, &Dib, FALSE);
	else if (CDib::IsTIFFExt(sExt))
		return Dib.SaveTIFF(lpszFileName, COMPRESSION_LZW);
	else if (sExt == _T(".gif"))
		return CPictureDoc::SaveGIF(lpszFileName, &Dib);
	else if (sExt == _T(".bmp") || sExt == _T(".dib"))
	{
		if (Dib.HasAlpha() && Dib.GetBitCount() == 32)
			Dib.ToBITMAPV5HEADER();
		return Dib.SaveBMP(lpszFileName);
	}
	else if (sExt == _T(".pcx"))
	{
		if (Dib.HasAlpha() && Dib.GetBitCount() == 32)
		{
			Dib.RenderAlphaWithSrcBackground();
			Dib.SetAlpha(FALSE);
		}
		return Dib.SavePCX(lpszFileName);
	}

	return FALSE;
}

void CUImagerApp::RichEditCtrlAppendText(	CRichEditCtrl* pRichEditCtrl,
											CString sText,
											COLORREF crColor, // for default pass ::GetSysColor(COLOR_WINDOWTEXT)
											BOOL bBold/*=FALSE*/,
											BOOL bItalic/*=FALSE*/,
											BOOL bUnderline/*=FALSE*/)
{
	if (pRichEditCtrl)
	{
		CHARFORMAT cf = { 0 };
		cf.cbSize = sizeof(cf);
		cf.dwMask = CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_COLOR;
		cf.dwEffects = (bBold ? CFE_BOLD : 0) | (bItalic ? CFE_ITALIC : 0) | (bUnderline ? CFM_UNDERLINE : 0);
		cf.crTextColor = crColor;
		pRichEditCtrl->SetSel(-1, -1);				// set the caret to the end and deselect everything
		pRichEditCtrl->SetSelectionCharFormat(cf);	// assign the format to the caret pos
		pRichEditCtrl->ReplaceSel(sText);			// insert at the caret pos
	}
}

void CUImagerApp::FileTypeNotSupportedMessageBox(LPCTSTR lpszFileName)
{
	CString sMsg;
	CString sExt = ::GetFileExt(lpszFileName);
	sMsg.Format(ML_STRING(1174, "Failed to Open:\n%s\nThe following extension is not supported:\n%s"),
				lpszFileName,
				sExt);
	::AfxMessageBox(sMsg, MB_OK | MB_ICONSTOP);
}

// This function is called from the Recent File Open Call,
// from The Drag And Drop and from the double-click File Open
CDocument* CUImagerApp::OpenDocumentFile(LPCTSTR lpszFileName) 
{
	// Check
	if (lpszFileName == NULL || lpszFileName[0] == _T('\0'))
		return NULL;
	
	// In case of .lnk file return its target file/folder
	CString sTargetPath = ::GetShortcutTarget(lpszFileName);

	// Store Last Opened Directory
	if (::IsExistingDir(sTargetPath))
		m_sLastOpenedDir = sTargetPath;
	else
	{
		TCHAR szDrive[_MAX_DRIVE];
		TCHAR szDir[_MAX_DIR];
		_tsplitpath(sTargetPath, szDrive, szDir, NULL, NULL);
		m_sLastOpenedDir = CString(szDrive) + CString(szDir);
	}
	m_sLastOpenedDir.TrimRight(_T('\\'));
	WriteProfileString(	_T("GeneralApp"),
						_T("LastOpenedDir"),
						m_sLastOpenedDir);

	// Maximize from Tray
	if (m_bTrayIcon &&
		::AfxGetMainFrame()->m_TrayIcon.IsMinimizedToTray())
	{
		::AfxGetMainFrame()->m_TrayIcon.MaximizeFromTray();
		::AfxGetMainFrame()->ShowOwnedWindows(TRUE);
		PaintDocTitles();
	}

	CUImagerMultiDocTemplate* curTemplate = GetTemplateFromFileExtension(sTargetPath);
	if (curTemplate == NULL)
	{
		// A Dir may have been dropped -> Start Recursive Slideshow
		if (::IsExistingDir(sTargetPath))
			return SlideShow(sTargetPath, TRUE);
		else
		{
			FileTypeNotSupportedMessageBox(sTargetPath);
			return NULL;
		}
	}

	// If the Path is Already Full, GetFullPathName will not change it.
	TCHAR szFullPathName[MAX_PATH];
	LPTSTR lpFilePart;
	::GetFullPathName(sTargetPath, MAX_PATH, szFullPathName, &lpFilePart);

	// Open Doc
	CDocument* pDoc = curTemplate->OpenDocumentFile(NULL);
	if (pDoc)
	{
		if (pDoc->IsKindOf(RUNTIME_CLASS(CPictureDoc)))
		{
			if (m_bStartMaximized)
				((CPictureDoc*)pDoc)->GetFrame()->MDIMaximize();

			CZoomComboBox* pZoomCB = &(((CPictureToolBar*)((CToolBarChildFrame*)(((CPictureDoc*)pDoc)->GetFrame()))->GetToolBar())->m_ZoomComboBox);
			pZoomCB->SetCurSel(((CPictureDoc*)pDoc)->m_nZoomComboBoxIndex);
			pZoomCB->OnChangeZoomFactor(*((double*)(pZoomCB->GetItemDataPtr(((CPictureDoc*)pDoc)->m_nZoomComboBoxIndex))));

			if (!((CPictureDoc*)pDoc)->LoadPicture(	&((CPictureDoc*)pDoc)->m_pDib,
													szFullPathName,
													FALSE,
													FALSE,	// Do not preload Prev & Next
													FALSE
													))
			{
				((CPictureDoc*)pDoc)->CloseDocumentForce();
				return NULL;
			}
			else
			{
				// Fit to document
				if (!((CPictureDoc*)pDoc)->GetFrame()->IsZoomed())
				{
					((CPictureDoc*)pDoc)->GetView()->GetParentFrame()->SetWindowPos(NULL,
																					0, 0, 0, 0,
																					SWP_NOSIZE |
																					SWP_NOZORDER);
					((CPictureDoc*)pDoc)->GetView()->UpdateWindowSizes(FALSE, FALSE, TRUE);
				}
				((CPictureDoc*)pDoc)->SlideShow(); // No Recursive Slideshow in Paused State
			}
		}
	}

	return pDoc;
}

#ifdef VIDEODEVICEDOC

void CUImagerApp::OnCaptureNetwork() 
{
	CHostPortDlg dlg;
	if (dlg.DoModal() == IDOK)
	{
		CVideoDeviceDoc* pDoc = (CVideoDeviceDoc*)GetVideoDeviceDocTemplate()->OpenDocumentFile(NULL);
		if (pDoc)
			pDoc->OpenNetVideoDevice(&dlg);
	}
}

void CUImagerApp::OnFileDxVideoDevice(UINT nID)
{
	int nDxDeviceId = nID - ID_DIRECTSHOW_VIDEODEV_FIRST;

	// If device already running, set the focus to it and return
	CString sDevicePath(CDxCapture::GetDevicePath(nDxDeviceId));
	CUImagerMultiDocTemplate* pVideoDeviceDocTemplate = GetVideoDeviceDocTemplate();
	POSITION posVideoDeviceDoc = pVideoDeviceDocTemplate->GetFirstDocPosition();
	while (posVideoDeviceDoc)
	{
		CVideoDeviceDoc* pVideoDeviceDoc = (CVideoDeviceDoc*)pVideoDeviceDocTemplate->GetNextDoc(posVideoDeviceDoc);
		if (pVideoDeviceDoc && pVideoDeviceDoc->m_pDxCapture &&
			pVideoDeviceDoc->m_pDxCapture->GetDevicePath() == sDevicePath)
		{
			if (pVideoDeviceDoc->GetFrame()->IsIconic())
				pVideoDeviceDoc->GetFrame()->MDIRestore();
			else if (!pVideoDeviceDoc->GetFrame()->IsZoomed())
				pVideoDeviceDoc->GetFrame()->MDIActivate();
			return;
		}
	}

	// Open device
	CVideoDeviceDoc* pDoc = (CVideoDeviceDoc*)pVideoDeviceDocTemplate->OpenDocumentFile(NULL);
	if (pDoc)
		pDoc->OpenDxVideoDevice(nDxDeviceId, _T(""), _T(""));
}

#endif

void CUImagerApp::OnFileShrinkDirDocs() 
{
	if (!::AfxGetMainFrame()->m_pBatchProcDlg)
	{
		::AfxGetMainFrame()->m_pBatchProcDlg = new CBatchProcDlg(::AfxGetMainFrame());
		::AfxGetMainFrame()->m_pBatchProcDlg->ShowWindow(SW_RESTORE);
	}
	else
	{
		::MessageBeep(0xFFFFFFFF);
		::AfxGetMainFrame()->m_pBatchProcDlg->SetActiveWindow();
		::AfxGetMainFrame()->m_pBatchProcDlg->SetFocus();
	}
}

void CUImagerApp::OnFileSendmailOpenDocs() 
{
	// Prompt to open files if none open
	if (!ArePictureDocsOpen())
		OnFileOpen();

	// Send open doc(s)
	if (ArePictureDocsOpen())
	{
		// Make sure open doc(s) are available
		CUImagerMultiDocTemplate* pPictureDocTemplate = GetPictureDocTemplate();
		POSITION posPictureDoc = pPictureDocTemplate->GetFirstDocPosition();
		while (posPictureDoc)
		{
			CPictureDoc* pPictureDoc = (CPictureDoc*)(pPictureDocTemplate->GetNextDoc(posPictureDoc));
			if (!IsDocAvailable(pPictureDoc, TRUE))
				return;
		}

		// Send them
		SendOpenDocsAsMail();
	}
}

void CUImagerApp::OnUpdateFileSendmailOpenDocs(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(TRUE);
}

BOOL CUImagerApp::PaintDocTitles()
{
	CDocument* pDoc;
	CUImagerMultiDocTemplate* curTemplate;
	POSITION posTemplate, posDoc;

	posTemplate = GetFirstDocTemplatePosition();
	while (posTemplate)
	{
		curTemplate = (CUImagerMultiDocTemplate*)GetNextDocTemplate(posTemplate);
		posDoc = curTemplate->GetFirstDocPosition();
		while (posDoc)
		{
			// SetDocumentTitle() calls CDocument::SetTitle()
			// which calls UpdateFrameCounts(), but if the view is not
			// visible the title is not updated. When the view becomes
			// visible exiting the tray icon minimized state
			// UpdateFrameCounts() has to be called!
			pDoc = curTemplate->GetNextDoc(posDoc);
			if (pDoc)
				pDoc->UpdateFrameCounts();        // will cause name change in views
		}
	}

	return FALSE;
}

void CUImagerApp::InitPrinter()
{
	if (!m_bPrinterInit)
	{
		// Select last used printer
		m_PrinterControl.RestorePrinterSelection(m_hDevMode, m_hDevNames);
		m_bPrinterInit = TRUE;
	}
}

BOOL CUImagerApp::AreDocsOpen()
{
	CDocument* pDoc;
	CUImagerMultiDocTemplate* curTemplate;
	POSITION posTemplate, posDoc;

	posTemplate = GetFirstDocTemplatePosition();
	while (posTemplate)
	{
		curTemplate = (CUImagerMultiDocTemplate*)GetNextDocTemplate(posTemplate);
		posDoc = curTemplate->GetFirstDocPosition();
		while (posDoc)
		{
			pDoc = curTemplate->GetNextDoc(posDoc);
			if (pDoc)
				return TRUE;
		}
	}

	return FALSE;
}

BOOL CUImagerApp::ArePictureDocsOpen()
{
	CUImagerMultiDocTemplate* pPictureDocTemplate = GetPictureDocTemplate();
	POSITION posPictureDoc = pPictureDocTemplate->GetFirstDocPosition();
	CPictureDoc* pPictureDoc;	
	while (posPictureDoc)
	{
		pPictureDoc = (CPictureDoc*)(pPictureDocTemplate->GetNextDoc(posPictureDoc));
		if (pPictureDoc)
			return TRUE;
	}

	return FALSE;
}

CString CUImagerApp::GetActiveDocDriveAndDirName()
{
	CString sDriveDir;
	CMDIChildWnd* pChild = ::AfxGetMainFrame()->MDIGetActive();
	if (pChild)
	{
		CUImagerDoc* pDoc = (CUImagerDoc*)pChild->GetActiveDocument();
		if (pDoc)
			sDriveDir = ::GetDriveAndDirName(pDoc->m_sFileName);
	}
	return sDriveDir;
}

#ifdef VIDEODEVICEDOC

BOOL CUImagerApp::AreVideoDeviceDocsOpen()
{
	CUImagerMultiDocTemplate* pVideoDeviceDocTemplate = GetVideoDeviceDocTemplate();
	POSITION posVideoDeviceDoc = pVideoDeviceDocTemplate->GetFirstDocPosition();	
	while (posVideoDeviceDoc)
	{
		CVideoDeviceDoc* pVideoDeviceDoc = (CVideoDeviceDoc*)(pVideoDeviceDocTemplate->GetNextDoc(posVideoDeviceDoc));
		if (pVideoDeviceDoc)
			return TRUE;
	}

	return FALSE;
}

#endif

void CUImagerApp::SaveOnEndSession()
{
	/*
	This is not a normal termination, we will not get a WM_CLOSE. 
	But fortunately all threads are still running, this is really 
	necessary to avoid locking us (the UI thread). For example
	SaveSavesCount() called by SaveSettings() uses a critical
	section to serialize the access to the saves counter variables
	updated by the m_SaveFrameListThread thread.
	*/

	SavePlacement();
	m_PrinterControl.SavePrinterSelection(m_hDevMode, m_hDevNames);
#ifdef VIDEODEVICEDOC
	CUImagerMultiDocTemplate* pVideoDeviceDocTemplate = GetVideoDeviceDocTemplate();
	POSITION posVideoDeviceDoc = pVideoDeviceDocTemplate->GetFirstDocPosition();
	while (posVideoDeviceDoc)
	{
		CVideoDeviceDoc* pVideoDeviceDoc = (CVideoDeviceDoc*)(pVideoDeviceDocTemplate->GetNextDoc(posVideoDeviceDoc));
		if (pVideoDeviceDoc)
			pVideoDeviceDoc->SaveSettings();
	}
	if (m_bAutostartsExecuted)
	{
		if (m_bStartMicroApache)
			CVideoDeviceDoc::MicroApacheShutdown(MICROAPACHE_TIMEOUT_MS);
		::LogLine(_T("%s"), ML_STRING(1566, "Closing") + _T(" ") + APPNAME_NOEXT + _T(" (Session End)"));
	}
	if (m_bDoStartFromService && GetContaCamServiceState() == CONTACAMSERVICE_RUNNING)
		ControlContaCamService(CONTACAMSERVICE_CONTROL_START_PROC);
#endif
}

BOOL CUImagerApp::IsDoc(CDocument* pDoc)
{
	CDocument* pDocument;
	CUImagerMultiDocTemplate* curTemplate;
	POSITION posTemplate, posDoc;

	// Check
	if (pDoc == NULL)
		return FALSE;

	posTemplate = GetFirstDocTemplatePosition();
	while (posTemplate)
	{
		curTemplate = (CUImagerMultiDocTemplate*)GetNextDocTemplate(posTemplate);
		posDoc = curTemplate->GetFirstDocPosition();
		while (posDoc)
		{
			pDocument = curTemplate->GetNextDoc(posDoc);
			if (pDocument->IsKindOf(RUNTIME_CLASS(CUImagerDoc)))
			{
				if ((pDoc == pDocument)	&&
					!(((CUImagerDoc*)pDocument)->m_bClosing))
					return TRUE;
			}
			else
			{
				if (pDoc == pDocument)
					return TRUE;
			}
		}
	}

	return FALSE;
}

BOOL CUImagerApp::IsPictureSizeBig(DWORD dwImageSize)
{
	if (dwImageSize < BIG_PICTURE_SIZE_LIMIT)
		return FALSE;
	else
		return TRUE;
}

CString CUImagerApp::PictureSlideMakeMsg(CPictureDoc* pDoc)
{
	CString sMsg;

	if (!pDoc->m_pDib)
		sMsg = ML_STRING(1178, "No picture file loaded."); 	
	else if (pDoc->m_dwIDAfterFullLoadCommand != 0)
		sMsg = ML_STRING(1179, "Please try again later, a command is executed when picture fully loaded.");
	else if (pDoc->IsModified())
		sMsg = ML_STRING(1180, "Try again after saving the picture file.");
	else if (pDoc->m_bMetadataModified)
		sMsg = ML_STRING(1181, "Before proceeding save the modified metadatas.");
	else if (pDoc->m_pRotationFlippingDlg)
		sMsg = ML_STRING(1182, "Close the Rotation / Flipping dialog and try again.");
	else if (pDoc->m_pWndPalette)
		sMsg = ML_STRING(1183, "Close the Palette dialog and try again.");
	else if (pDoc->m_pHLSDlg)
		sMsg = ML_STRING(1184, "Close the Brightness, Contrast... dialog and try again.");
	else if (pDoc->m_pRedEyeDlg)
		sMsg = ML_STRING(1185, "Finish the redeye remove and try again.");
	else if (pDoc->m_bDoRedEyeColorPickup)
		sMsg = ML_STRING(1186, "Finish the redeye picking / remove, then try again.");
	else if (pDoc->m_pMonochromeConversionDlg)
		sMsg = ML_STRING(1187, "Close the Monochrome Conversion dialog and try again.");
	else if (pDoc->m_pSharpenDlg)
		sMsg = ML_STRING(1188, "Close the Sharpen dialog and try again.");
	else if (pDoc->m_pSoftenDlg)
		sMsg = ML_STRING(1189, "Close the Soften dialog and try again.");
	else if (pDoc->m_pSoftBordersDlg)
		sMsg = ML_STRING(1190, "Close the Soft Borders dialog and try again.");
	else if (pDoc->m_bCrop)
		sMsg = ML_STRING(1191, "Finish cropping, then try again.");
	else if (pDoc->m_bPrintPreviewMode)
		sMsg = ML_STRING(1192, "Exit Print Preview, then try again.");
	else
		sMsg = _T("");

	return sMsg;
}

CString CUImagerApp::PictureMakeMsg(CPictureDoc* pDoc)
{
	CString sMsg;

	sMsg = PictureSlideMakeMsg(pDoc);
	if (sMsg == _T(""))
	{
		if (pDoc->m_SlideShowThread.IsSlideshowRunning() ||
			pDoc->m_bDoRestartSlideshow)
			sMsg = ML_STRING(1193, "Stop the slideshow and try again.");
	}

	return sMsg;
}

BOOL CUImagerApp::IsDocAvailable(CDocument* pDoc, BOOL bShowMsgBoxIfNotAvailable/*=FALSE*/)
{
	if (pDoc && pDoc->IsKindOf(RUNTIME_CLASS(CPictureDoc)))
	{
		CString sMsg(PictureMakeMsg((CPictureDoc*)pDoc));
		if (!sMsg.IsEmpty())
		{
			if (bShowMsgBoxIfNotAvailable)
			{
				((CPictureDoc*)pDoc)->GetView()->ForceCursor();
				::AfxMessageBox(sMsg, MB_OK | MB_ICONINFORMATION);
				((CPictureDoc*)pDoc)->GetView()->ForceCursor(FALSE);
			}
			return FALSE;
		}
	}
	return TRUE;
}

BOOL CUImagerApp::IsDocReadyToSlide(CPictureDoc* pDoc, BOOL bShowMsgBoxIfSlideNotPossible/*=FALSE*/)
{
	if (pDoc)
	{
		CString sMsg(PictureSlideMakeMsg(pDoc));
		if (!sMsg.IsEmpty())
		{
			if (bShowMsgBoxIfSlideNotPossible)
			{
				pDoc->GetView()->ForceCursor();
				::AfxMessageBox(sMsg, MB_OK | MB_ICONINFORMATION);
				pDoc->GetView()->ForceCursor(FALSE);
			}
			return FALSE;
		}
	}
	return TRUE;
}

BOOL CUImagerApp::CompressToZip(LPCTSTR szPath, LPCTSTR szZipFileName)
{	
	CZipProgressDlg dlg(::AfxGetMainFrame(), FALSE);
	dlg.m_sZipFileName = CString(szZipFileName);
	dlg.m_sPath = CString(szPath);
	if (dlg.DoModal() == IDOK)
		return TRUE;
	else
		return FALSE;
}

BOOL CUImagerApp::ExtractZipToDir(LPCTSTR szDirPath, LPCTSTR szZipFileName)
{
	CZipProgressDlg dlg(::AfxGetMainFrame(), TRUE);
	dlg.m_sZipFileName = CString(szZipFileName);
	dlg.m_sPath = CString(szDirPath);
	if (dlg.DoModal() == IDOK)
		return TRUE;
	else
		return FALSE;
}

int CUImagerApp::ExitInstance() 
{
	// Stop Donor Email Validation Thread
	g_DonorEmailValidateThread.Kill();

#ifdef VIDEODEVICEDOC
	// Clean-up auto-starts
	if (m_bAutostartsExecuted)
	{
		// Micro Apache shutdown
		if (m_bStartMicroApache)
			CVideoDeviceDoc::MicroApacheShutdown(MICROAPACHE_TIMEOUT_MS);

		// Log the stopping of the application
		::LogLine(_T("%s"), ML_STRING(1566, "Closing") + _T(" ") + APPNAME_NOEXT);
	}

	// Clean-up AV format network
	avformat_network_deinit();
#endif

	// Close The Application Mutex
	if (m_hAppMutex)
		::CloseHandle(m_hAppMutex);

	// Store last selected printer
	m_PrinterControl.SavePrinterSelection(m_hDevMode, m_hDevNames);

	// Clean-up gdiplus
	Gdiplus::GdiplusShutdown(m_GdiplusToken);

	// Clean-Up Trace Log File
	::EndTraceLogFile();

	// ffmpeg delete Critical Section
#ifdef VIDEODEVICEDOC
	if (g_bSWScalerCSInited)
	{
		g_bSWScalerCSInited = FALSE;
		::DeleteCriticalSection(&g_csSWScaler);
	}
	av_lockmgr_register(NULL);
#endif

	// From CWinApp::ExitInstance(), I modified it:

	// If we remember that we're unregistering,
	// don't save our profile settings
	if (m_pCmdInfo == NULL ||
		(m_pCmdInfo->m_nShellCommand != CCommandLineInfo::AppUnregister &&
		 m_pCmdInfo->m_nShellCommand != CCommandLineInfo::AppRegister))
	{
		if (!afxContextIsDLL)
			SaveStdProfileSettings();
	}

	// Start from Service after writing with SaveStdProfileSettings()!
#ifdef VIDEODEVICEDOC
	if (m_bDoStartFromService && GetContaCamServiceState() == CONTACAMSERVICE_RUNNING)
		ControlContaCamService(CONTACAMSERVICE_CONTROL_START_PROC);
#endif

	// Cleanup DAO if necessary
	if (m_lpfnDaoTerm != NULL)
	{
		// If a DLL, YOU must call AfxDaoTerm prior to ExitInstance
		ASSERT(!afxContextIsDLL);
		(*m_lpfnDaoTerm)();
	}

	if (m_hLangResourceDLL != NULL)
	{
		::FreeLibrary(m_hLangResourceDLL);
		m_hLangResourceDLL = NULL;
	}

	int nReturnValue=0;
	if (::AfxGetCurrentMessage())
	{
		nReturnValue = static_cast<int>(::AfxGetCurrentMessage()->wParam);
	}

	return nReturnValue; // returns the value from PostQuitMessage
}

CPictureDoc* CUImagerApp::SlideShow(LPCTSTR sStartDirName, BOOL bRecursive)
{
	// Create New Picture Document
	CPictureDoc* pDoc = (CPictureDoc*)GetPictureDocTemplate()->OpenDocumentFile(NULL);
	if (pDoc == NULL)
		return NULL;

	// Set Zoom ComboBox
	CZoomComboBox* pZoomCB = &(((CPictureToolBar*)((CToolBarChildFrame*)(((CPictureDoc*)pDoc)->GetFrame()))->GetToolBar())->m_ZoomComboBox);
	pZoomCB->SetCurSel(pDoc->m_nZoomComboBoxIndex);
	pZoomCB->OnChangeZoomFactor(*((double*)(pZoomCB->GetItemDataPtr(pDoc->m_nZoomComboBoxIndex))));

	// Slideshow
	pDoc->m_SlideShowThread.SetStartName(sStartDirName);
	pDoc->m_SlideShowThread.SetRecursive(bRecursive);
	pDoc->m_SlideShowThread.PauseSlideshow();

	return pDoc;
}

#ifdef VIDEODEVICEDOC

int CUImagerApp::GetContaCamServiceState()
{ 
	int nCurrentState = CONTACAMSERVICE_NOTINSTALLED;
	SC_HANDLE schSCManager = ::OpenSCManager(NULL, NULL, 0); 
	if (schSCManager)
	{
		SC_HANDLE schService = ::OpenService(schSCManager, SERVICENAME_NOEXT, SERVICE_QUERY_STATUS);
		if (schService)
		{
			SERVICE_STATUS status;
			if (::QueryServiceStatus(schService, &status))
			{
				switch (status.dwCurrentState)
				{
					case SERVICE_CONTINUE_PENDING	:
					case SERVICE_RUNNING			:
					case SERVICE_START_PENDING		:
						nCurrentState = CONTACAMSERVICE_RUNNING;
						break;
					default :
						nCurrentState = CONTACAMSERVICE_NOTRUNNING;
						break;
				}
			}
			::CloseServiceHandle(schService);
		}
		::CloseServiceHandle(schSCManager);
	}
	return nCurrentState;
}

// CONTACAMSERVICE_CONTROL_START_PROC
// CONTACAMSERVICE_CONTROL_END_PROC
DWORD CUImagerApp::ControlContaCamService(int nMsg)
{ 
	DWORD dwError = ERROR_SUCCESS;
    SC_HANDLE schSCManager = ::OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (schSCManager == 0) 
		dwError = ::GetLastError();
	else
	{
		SC_HANDLE schService = ::OpenService(schSCManager, SERVICENAME_NOEXT, SERVICE_USER_DEFINED_CONTROL);
		if (schService == 0)
			dwError = ::GetLastError();
		else
		{
			SERVICE_STATUS status;
			if (!::ControlService(schService, nMsg, &status))
				dwError = ::GetLastError();
			::CloseServiceHandle(schService); 
		}
		::CloseServiceHandle(schSCManager); 
	}
	return dwError;
}

BOOL CUImagerApp::SaveReservation(DWORD dwId, BOOL bLowPriority/*=FALSE*/)
{
	int i;

	// Enter critical section
	::EnterCriticalSection(&m_csSaveReservation);

	// Are we already in the queue?
	POSITION pos = m_SaveReservationQueue.GetHeadPosition();
	for (i = 0; i < m_SaveReservationQueue.GetCount(); i++)
	{
		if (m_SaveReservationQueue.GetNext(pos) == dwId)
		{
			::LeaveCriticalSection(&m_csSaveReservation);
			return (i < m_nSimultaneousSaving); // saving position?
		}
	}
	
	// Add to Tail if not low priority or if queue is empty
	if (!bLowPriority || m_SaveReservationQueue.GetCount() == 0)
	{
		// Add to Tail
		m_SaveReservationQueue.AddTail(dwId);

		// Go if we are in the first m_nSimultaneousSaving
		pos = m_SaveReservationQueue.GetHeadPosition();
		for (i = 0; i < MIN(m_nSimultaneousSaving, m_SaveReservationQueue.GetCount()); i++)
		{
			if (m_SaveReservationQueue.GetNext(pos) == dwId)
			{
				::LeaveCriticalSection(&m_csSaveReservation);
				return TRUE;
			}
		}
	}

	// Leave critical section
	::LeaveCriticalSection(&m_csSaveReservation);

	return FALSE;
}

void CUImagerApp::SaveReservationRemove(DWORD dwId)
{
	int i = 0;

	// Enter critical section
	::EnterCriticalSection(&m_csSaveReservation);

	// Loop
	POSITION pos = m_SaveReservationQueue.GetHeadPosition();
	while (i < m_SaveReservationQueue.GetCount())
	{
		POSITION prevpos = pos;
		if (m_SaveReservationQueue.GetNext(pos) == dwId)
		{
			// Remove
			m_SaveReservationQueue.RemoveAt(prevpos);

			// Reset
			i = 0;
			pos = m_SaveReservationQueue.GetHeadPosition();
		}
		else
			i++;
	}

	// Leave critical section
	::LeaveCriticalSection(&m_csSaveReservation);
}

#endif

void CUImagerApp::OnFileOpenDir() 
{
	if (!::AfxGetMainFrame()->m_bFullScreenMode)
	{
		CString sInitDir;
		CString sActiveDocDriveAndDir(GetActiveDocDriveAndDirName());
		sActiveDocDriveAndDir.TrimRight(_T('\\'));
		m_sLastOpenedDir.TrimRight(_T('\\'));
		if (::IsExistingDir(sActiveDocDriveAndDir))
			sInitDir = sActiveDocDriveAndDir;
		else if (::IsExistingDir(m_sLastOpenedDir))
			sInitDir = m_sLastOpenedDir;
		else
			sInitDir = ::GetSpecialFolderPath(CSIDL_MYPICTURES);
		CBrowseDlg dlg(	::AfxGetMainFrame(),
						&sInitDir,
						ML_STRING(1212, "Loading Folder's First Picture"),
						FALSE,
						TRUE,
						ML_STRING(963, "Recursive"),
						TRUE);
		if (dlg.DoModal() == IDOK)
		{
			// Open Dir
			SlideShow(sInitDir, dlg.IsChecked());

			// Store Last Opened Directory
			m_sLastOpenedDir = sInitDir;
			WriteProfileString(	_T("GeneralApp"),
								_T("LastOpenedDir"),
								m_sLastOpenedDir);
		}
	}
}

CUImagerApp::CUImagerCommandLineInfo::CUImagerCommandLineInfo()
{
}

CUImagerApp::CUImagerCommandLineInfo::~CUImagerCommandLineInfo()
{
}

void CUImagerApp::CUImagerCommandLineInfo::ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast)
{
	if (bFlag)
	{
		USES_CONVERSION;
		ParseParamFlag(T2CA(pszParam));
		if (pszParam)
		{
#ifdef VIDEODEVICEDOC
			if (_tcscmp(pszParam, _T("service")) == 0) // Case sensitive!
				((CUImagerApp*)::AfxGetApp())->m_bServiceProcess = TRUE;
#endif
		}
	}
	else
		ParseParamNotFlag(pszParam);

	ParseLast(bLast);
}

void CUImagerApp::CUImagerCommandLineInfo::ParseParam(const char* pszParam, BOOL bFlag, BOOL bLast)
{
	if (bFlag)
	{
		ParseParamFlag(pszParam);
		if (pszParam)
		{
#ifdef VIDEODEVICEDOC
			if (strcmp(pszParam, "service") == 0) // Case sensitive!
				((CUImagerApp*)::AfxGetApp())->m_bServiceProcess = TRUE;
#endif
		}
	}
	else
		ParseParamNotFlag(pszParam);

	ParseLast(bLast);
}

void CUImagerApp::CUImagerCommandLineInfo::ParseParamFlag(const char* pszParam)
{
	// OLE command switches are case insensitive, while
	// shell command switches are case sensitive

	if (lstrcmpA(pszParam, "pt") == 0)
	{
		m_nShellCommand = FilePrintTo;
		((CUImagerApp*)::AfxGetApp())->m_bHideMainFrame = TRUE;
	}
	else if (lstrcmpA(pszParam, "p") == 0)
		m_nShellCommand = FilePrint;
    else if (::AfxInvariantStrICmp(pszParam, "Register") == 0 ||
             ::AfxInvariantStrICmp(pszParam, "Regserver") == 0)
		m_nShellCommand = AppRegister;
    else if (::AfxInvariantStrICmp(pszParam, "Unregister") == 0 ||
             ::AfxInvariantStrICmp(pszParam, "Unregserver") == 0)
		m_nShellCommand = AppUnregister;
	else if (lstrcmpA(pszParam, "dde") == 0)
	{
		AfxOleSetUserCtrl(FALSE);
		m_nShellCommand = FileDDE;
		((CUImagerApp*)::AfxGetApp())->m_bHideMainFrame = TRUE;
	}
    else if (::AfxInvariantStrICmp(pszParam, "Embedding") == 0)
	{
		AfxOleSetUserCtrl(FALSE);
		m_bRunEmbedded = TRUE;
		m_bShowSplash = FALSE;
	}
    else if (::AfxInvariantStrICmp(pszParam, "Automation") == 0)
	{
		AfxOleSetUserCtrl(FALSE);
		m_bRunAutomated = TRUE;
		m_bShowSplash = FALSE;
	}
}

void CUImagerApp::CUImagerCommandLineInfo::ParseParamNotFlag(const TCHAR* pszParam)
{
	if (m_strFileName.IsEmpty())
	{
		m_strFileName = pszParam;
		m_strFileNames.Add(pszParam);
	}
	else if (m_nShellCommand == FilePrintTo && m_strPrinterName.IsEmpty())
		m_strPrinterName = pszParam;
	else if (m_nShellCommand == FilePrintTo && m_strDriverName.IsEmpty())
		m_strDriverName = pszParam;
	else if (m_nShellCommand == FilePrintTo && m_strPortName.IsEmpty())
		m_strPortName = pszParam;
	else
		m_strFileNames.Add(pszParam);
}

void CUImagerApp::CUImagerCommandLineInfo::ParseParamNotFlag(const char* pszParam)
{
	if (m_strFileName.IsEmpty())
	{
		m_strFileName = pszParam;
		m_strFileNames.Add(CString(pszParam));
	}
	else if (m_nShellCommand == FilePrintTo && m_strPrinterName.IsEmpty())
		m_strPrinterName = pszParam;
	else if (m_nShellCommand == FilePrintTo && m_strDriverName.IsEmpty())
		m_strDriverName = pszParam;
	else if (m_nShellCommand == FilePrintTo && m_strPortName.IsEmpty())
		m_strPortName = pszParam;
	else
		m_strFileNames.Add(CString(pszParam));
}

void CUImagerApp::CUImagerCommandLineInfo::ParseLast(BOOL bLast)
{
	if (bLast)
	{
		if (m_nShellCommand == FileNew && !m_strFileName.IsEmpty())
			m_nShellCommand = FileOpen;
		m_bShowSplash = !m_bRunEmbedded && !m_bRunAutomated;
	}
}

void CUImagerApp::ParseCommandLine(CUImagerCommandLineInfo& rCmdInfo)
{
	for (int i = 1; i < __argc; i++)
	{
		LPCTSTR pszParam = __targv[i];
		BOOL bFlag = FALSE;
		BOOL bLast = ((i + 1) == __argc);
		if (pszParam[0] == '-' || pszParam[0] == '/')
		{
			// remove flag specifier
			bFlag = TRUE;
			++pszParam;
		}
		rCmdInfo.ParseParam(pszParam, bFlag, bLast);
	}
}

BOOL CUImagerApp::ProcessShellCommand(CUImagerCommandLineInfo& rCmdInfo)
{
	BOOL bResult = TRUE;

	switch (rCmdInfo.m_nShellCommand)
	{
		// New File
		case CCommandLineInfo::FileNew:
			// No new empty document
			//if (!AfxGetApp()->OnCmdMsg(ID_FILE_NEW, 0, NULL, NULL))
			//	OnFileNew();

			// Check
			if (m_pMainWnd == NULL)
				bResult = FALSE;	// Error -> Exit Program

			break;

		// If we've been asked to open a file, call OpenDocumentFile()
		case CCommandLineInfo::FileOpen:
			if (rCmdInfo.m_strFileNames.GetSize() <= 1)
			{
				// If the file is empty or not existing try to paste the clipboard into it
				if (::GetFileSize64(rCmdInfo.m_strFileName).QuadPart == 0)
					PasteToFile(rCmdInfo.m_strFileName);
				m_bStartMaximized = TRUE;
				if (!OpenDocumentFile(rCmdInfo.m_strFileName))
					bResult = FALSE;	// Error -> Exit Program
				m_bStartMaximized = FALSE;
			}
			else
			{
				BOOL bAtLeastOneOpened = FALSE;
				for (int i = 0 ; i < rCmdInfo.m_strFileNames.GetSize() ; i++)
				{
					if (OpenDocumentFile(rCmdInfo.m_strFileNames[i]))
						bAtLeastOneOpened = TRUE;
				}
				if (!bAtLeastOneOpened)
					bResult = FALSE;	// Error -> Exit Program
			}
			break;

		// If the user wanted to print to the given file(s), case sensitive:
		// Example: Program.exe "file1.jpg" "file2.jpg" /pt "Printer Name" "Driver Name" "Port Name"
		// (Printer parameters are optional)
		case CCommandLineInfo::FilePrintTo:
			ASSERT(m_pCmdInfo == NULL);
			m_bUseLoadPreviewDibGlobal = FALSE; // force full-size jpeg loads!
			m_pCmdInfo = &rCmdInfo;
			if (rCmdInfo.m_strFileNames.GetSize() <= 1)
			{
				CDocument* pDoc = OpenDocumentFile(rCmdInfo.m_strFileName);
				if (pDoc && pDoc->IsKindOf(RUNTIME_CLASS(CPictureDoc)))
					((CPictureDoc*)pDoc)->GetView()->SendMessage(WM_COMMAND, ID_FILE_PRINT_DIRECT);
			}
			else
			{
				for (int i = 0 ; i < rCmdInfo.m_strFileNames.GetSize() ; i++)
				{
					CDocument* pDoc = OpenDocumentFile(rCmdInfo.m_strFileNames[i]);
					if (pDoc && pDoc->IsKindOf(RUNTIME_CLASS(CPictureDoc)))
						((CPictureDoc*)pDoc)->GetView()->SendMessage(WM_COMMAND, ID_FILE_PRINT_DIRECT);
				}
			}
			m_pCmdInfo = NULL;
			bResult = FALSE; // Done -> Exit Program
			break;

		// If the user wanted to print preview the given file, case sensitive:
		// Example: Program.exe "file.jpg" /p
		case CCommandLineInfo::FilePrint:
			{
				ASSERT(m_pCmdInfo == NULL);
				CDocument* pDoc = OpenDocumentFile(rCmdInfo.m_strFileName);
				if (!pDoc)
					bResult = FALSE;	// Error -> Exit Program
				else
				{
					m_pCmdInfo = &rCmdInfo;
					if (pDoc->IsKindOf(RUNTIME_CLASS(CPictureDoc)))
						((CPictureDoc*)pDoc)->GetView()->SendMessage(WM_COMMAND, ID_FILE_PRINT_PREVIEW);
					m_pCmdInfo = NULL;
				}
			}
			break;

		// If we're doing DDE set m_pCmdInfo
		case CCommandLineInfo::FileDDE:
			m_pCmdInfo = (CCommandLineInfo*)m_nCmdShow;
			break;

		// If we've been asked to unregister, unregister and then terminate
		case CCommandLineInfo::AppUnregister:
			{
				// Not used...
				
				//UnregisterShellFileTypes();
				//BOOL bUnregistered = Unregister();

				// if you specify /EMBEDDED, we won't make an success/failure box
				// this use of /EMBEDDED is not related to OLE

				//if (!rCmdInfo.m_bRunEmbedded)
				//{
				//	if (bUnregistered)
				//		::AfxMessageBox(AFX_IDP_UNREG_DONE);
				//	else
				//		::AfxMessageBox(AFX_IDP_UNREG_FAILURE);
				//}
				bResult = FALSE;    // that's all we do

				// If nobody is using it already, we can use it.
				// We'll flag that we're unregistering and not save our state
				// on the way out. This new object gets deleted by the
				// app object destructor.

				//if (m_pCmdInfo == NULL)
				//{
				//	m_pCmdInfo = new CCommandLineInfo;
				//	m_pCmdInfo->m_nShellCommand = CCommandLineInfo::AppUnregister;
				//}
			}
			break;

		default:
			bResult = FALSE;    // that's all we do
			break;
	}
	return bResult;
}

void CUImagerApp::ShrinkOpenDocs(LPCTSTR szDstDirPath,
								 DWORD dwMaxSize,
								 BOOL bMaxSizePercent,
								 DWORD dwJpegQuality,
								 BOOL bShrinkPictures,
								 BOOL bOnlyCopyFiles)
{
	// Destination Directory Path
	CString sDstDirPath(szDstDirPath);
	sDstDirPath.TrimRight(_T('\\'));

	// Begin Wait Cursor
	BeginWaitCursor();

	// Keep track of all added file names to avoid duplicates
	CSortableStringArray DstFileNames;

	// Picture Docs
	CUImagerMultiDocTemplate* curTemplate = GetPictureDocTemplate();
	POSITION pos = curTemplate->GetFirstDocPosition();
	while (pos)
	{
		CPictureDoc* pDoc = (CPictureDoc*)curTemplate->GetNextDoc(pos);

		// Source Directory Path
		TCHAR szDrive[_MAX_DRIVE];
		TCHAR szDir[_MAX_DIR];
		_tsplitpath(pDoc->m_sFileName, szDrive, szDir, NULL, NULL);
		CString sSrcDirPath = CString(szDrive) + CString(szDir);
		sSrcDirPath.TrimRight(_T('\\'));

		// Source File Name
		CString sSrcFileName = pDoc->m_sFileName;
		sSrcFileName.TrimRight(_T('\\'));

		// Destination File Name
		CString sDstFileName = sSrcFileName;
		sDstFileName = sDstFileName.Mid(sSrcDirPath.GetLength() + 1);
		CString sOrigDstFileName = sDstFileName;
		int i = 0;
		while (DstFileNames.InStringArrayNoCase(sDstFileName))
			sDstFileName.Format(_T("%s(%d)%s"), ::GetFileNameNoExt(sOrigDstFileName), ++i, ::GetFileExt(sOrigDstFileName));
		DstFileNames.Add(sDstFileName);
		sDstFileName = sDstDirPath + _T("\\") + sDstFileName;
		if (!bOnlyCopyFiles)
		{
			CString sDstExt = ShrinkGetDstExt(::GetFileExt(sSrcFileName));
			sDstFileName = ::GetFileNameNoExt(sDstFileName) + sDstExt;
		}

		// Status Text
		CString sStatusText;
		sStatusText.Format(_T("%s -> %s"), ::GetShortFileName(sSrcFileName), ::GetShortFileName(sDstFileName));
		::AfxGetMainFrame()->StatusText(sStatusText);

		// Only Copy File?
		if (bOnlyCopyFiles || ShrinkPicture(sSrcFileName,
											sDstFileName,
											dwMaxSize,
											bMaxSizePercent,
											dwJpegQuality,
											FALSE,				// Do not force jpeg quality change if already a jpeg
											COMPRESSION_JPEG,	// Use Jpeg Compression inside Tiff
											TRUE,				// Force the change to Jpeg Compression for Tiff files
											bShrinkPictures,
											FALSE,				// Do not sharpen after shrink
											TRUE,				// Work on all pages of a multi-page Tiff
											::AfxGetMainFrame(),
											TRUE,
											NULL) == 0)
		{
			::CopyFile(sSrcFileName, sDstFileName, FALSE);
			::SetFileAttributes(sDstFileName, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_TEMPORARY);
		}
	}

	// Done
	EndWaitCursor();
	::AfxGetMainFrame()->StatusText();
}

BOOL CUImagerApp::CalcShrink(DWORD dwOrigWidth,
							DWORD dwOrigHeight,
							DWORD dwMaxSize,
							BOOL bMaxSizePercent,
							DWORD& dwShrinkWidth,
							DWORD& dwShrinkHeight)
{
	// Init
	BOOL bDoShrink = FALSE;
	dwShrinkWidth = dwOrigWidth;
	dwShrinkHeight = dwOrigHeight;

	// Check
	if (dwOrigWidth == 0 || dwOrigHeight == 0)
		return FALSE;

	// Calc. aspect ratio
	double dAspectRatio = (double)dwOrigWidth / (double)dwOrigHeight;

	// Landscape
	if (dwOrigWidth > dwOrigHeight)
	{
		// From Percent to Pixels
		DWORD dwMaxSizePercent;
		if (bMaxSizePercent)
		{
			dwMaxSizePercent = dwMaxSize;
			dwMaxSize = (DWORD)Round(dwMaxSize / 100.0 * dwOrigWidth);
		}

		// Resize to dwMaxSize x XYZ
		if (dwOrigWidth > dwMaxSize)
		{
			bDoShrink = TRUE;
			dwShrinkWidth = dwMaxSize;
			dwShrinkHeight = (DWORD)Round(dwMaxSize / dAspectRatio);
		}
	}
	// Portrait
	else
	{
		// From Percent to Pixels
		DWORD dwMaxSizePercent;
		if (bMaxSizePercent)
		{
			dwMaxSizePercent = dwMaxSize;
			dwMaxSize = (DWORD)Round(dwMaxSize / 100.0 * dwOrigHeight);
		}

		// Resize to XYZ x dwMaxSize
		if (dwOrigHeight > dwMaxSize)
		{
			bDoShrink = TRUE;
			dwShrinkWidth = (DWORD)Round(dwMaxSize * dAspectRatio);
			dwShrinkHeight = dwMaxSize;
		}
	}

	return bDoShrink;
}

// Return Value:
// -1 : Just Copied
// 0  : Error
// 1  : Shrinked
int CUImagerApp::ShrinkPicture(	LPCTSTR szSrcFileName,
								LPCTSTR szDstFileName,
								DWORD dwMaxSize,
								BOOL bMaxSizePercent,
								DWORD dwJpegQuality,
								BOOL bForceJpegQuality,
								int nTiffCompression,
								BOOL bTiffForceCompression,
								BOOL bShrinkPictureSize,
								BOOL bSharpenAfterShrink,
								BOOL bWorkOnAllPages,
								CWnd* pProgressWnd,
								BOOL bProgressSend,
								CWorkerThread* pThread)
{
	BOOL res = FALSE;
	CDib SrcDib, DstDib1, DstDib2;
	SrcDib.SetShowMessageBoxOnError(FALSE);
	DstDib1.SetShowMessageBoxOnError(FALSE);
	DstDib2.SetShowMessageBoxOnError(FALSE);
	CDib* pSaveDib = &SrcDib;

	// Just Copy If Same Extension and:
	// - Animated GIFs
	// Or If:
	// - Not Shrinking Picture Size
	//   and not Jpeg with Force Quality
	//   and not Tiff with Force Compression
	if (::GetFileExt(szSrcFileName) == ::GetFileExt(szDstFileName)		&&

		(CDib::IsAnimatedGIF(szSrcFileName, FALSE)						||

		(!bShrinkPictureSize											&&
		!(CDib::IsJPEG(szSrcFileName) && bForceJpegQuality)				&&
		!(CDib::IsTIFF(szSrcFileName) && bTiffForceCompression))))
	{
		if (!::CopyFile(szSrcFileName, szDstFileName, FALSE))
		{
			return 0;
		}
		if (!::SetFileAttributes(szDstFileName,
								FILE_ATTRIBUTE_NORMAL |
								FILE_ATTRIBUTE_TEMPORARY))
		{
			return 0;
		}
		return -1; // Copied
	}

	// Load Only Header
	if (!SrcDib.LoadImage(szSrcFileName, 0, 0, 0, TRUE, TRUE))
		return 0;

	// Set initial value to not shrink
	DWORD dwShrinkWidth = SrcDib.GetWidth();
	DWORD dwShrinkHeight = SrcDib.GetHeight();
	
	// Work On All Pages of a Multi-Page TIFF?
	if (bWorkOnAllPages							&&
		(SrcDib.m_FileInfo.m_nImageCount > 1)	&&
		CDib::IsTIFF(szSrcFileName)				&&
		CDib::IsTIFF(szDstFileName))
	{
		return ShrinkPictureMultiPage(	szSrcFileName,
										szDstFileName,
										dwMaxSize,
										bMaxSizePercent,
										dwJpegQuality,
										nTiffCompression,
										bTiffForceCompression,
										bShrinkPictureSize,
										bSharpenAfterShrink,
										pProgressWnd,
										bProgressSend,
										pThread);
	}

	// Shrink Picture Size?
	if (bShrinkPictureSize)
	{
		// Calc. Shrink
		BOOL bDoShrink = CalcShrink(SrcDib.GetWidth(),
									SrcDib.GetHeight(),
									dwMaxSize,
									bMaxSizePercent,
									dwShrinkWidth,
									dwShrinkHeight);

		// Shrink
		if (bDoShrink)
		{
			// Use Memory Mapped Load if Not Compressed and Not Old OS/2 Bmp and Not top-down Bmp
			if (::GetFileExt(szSrcFileName) == _T(".bmp") ||
				::GetFileExt(szSrcFileName) == _T(".dib"))
			{
				if (!SrcDib.IsCompressed()			&&
					!SrcDib.m_FileInfo.m_bBmpOS2Hdr	&&
					SrcDib.m_FileInfo.m_nHeight > 0)
					res = SrcDib.MapBMP(szSrcFileName, TRUE);
			}
			
			// Normal Load
			if (res == FALSE)
			{
				res = SrcDib.LoadImage(	szSrcFileName,
										0, 0, 0,
										TRUE,
										FALSE,
										pProgressWnd,
										bProgressSend,
										pThread);
			}

			// Shrink
			if (res)
			{
				if (!DstDib1.StretchBits(	dwShrinkWidth,
											dwShrinkHeight,
											&SrcDib,
											pProgressWnd,
											bProgressSend,
											pThread))
					return 0;
				else
				{						
					// Sharpen
					if (bSharpenAfterShrink)
					{
						int Kernel[] = {-1,-1,-1,
										-1,20,-1,
										-1,-1,-1};
						if (!DstDib2.FilterFast(Kernel, 12,
												&DstDib1,
												pProgressWnd,
												bProgressSend,
												pThread))
							return 0;
						else
							pSaveDib = &DstDib2;
					}
					else
						pSaveDib = &DstDib1;
				}
			}
			else
				return 0;
		}
		else
		{
			// Just Copy If:
			// - Not Jpeg with Force Quality
			//   and not Tiff with Force Compression
			//   and Same Extension
			if (!(CDib::IsJPEG(szSrcFileName) && bForceJpegQuality) &&
				!(CDib::IsTIFF(szSrcFileName) && bTiffForceCompression) &&
				(::GetFileExt(szSrcFileName) == ::GetFileExt(szDstFileName)))
			{
				if (!::CopyFile(szSrcFileName, szDstFileName, FALSE))
				{
					return 0;
				}
				if (!::SetFileAttributes(szDstFileName,
										FILE_ATTRIBUTE_NORMAL |
										FILE_ATTRIBUTE_TEMPORARY))
				{
					return 0;
				}
				return -1; // Copied
			}
		}
	}
	
	// Not Yet Loaded?
	if (!res)
	{
		// Use Memory Mapped Load if Not Compressed and Not Old OS/2 Bmp and Not top-down Bmp
		if (::GetFileExt(szSrcFileName) == _T(".bmp") ||
			::GetFileExt(szSrcFileName) == _T(".dib"))
		{
			if (!SrcDib.IsCompressed()			&&
				!SrcDib.m_FileInfo.m_bBmpOS2Hdr	&&
				SrcDib.m_FileInfo.m_nHeight > 0)
				res = SrcDib.MapBMP(szSrcFileName, TRUE);
		}
		
		// Normal Load
		if (res == FALSE)
		{
			res = SrcDib.LoadImage(	szSrcFileName,
									0, 0, 0,
									TRUE,
									FALSE,
									pProgressWnd,
									bProgressSend,
									pThread);
		}
		if (!res)
			return 0;
	}

	// Set new DPI
	pSaveDib->SetXDpi(Round((double)dwShrinkWidth  * (double)SrcDib.GetXDpi() / (double)SrcDib.GetWidth()));
	pSaveDib->SetYDpi(Round((double)dwShrinkHeight * (double)SrcDib.GetYDpi() / (double)SrcDib.GetHeight()));

	// Auto Orientate
	CDib::AutoOrientateDib(pSaveDib);

	// Clear Orientation
	pSaveDib->GetExifInfo()->Orientation = 1;

	// Save Image
	CString sSrcExt = ::GetFileExt(szSrcFileName);
	CString sDstExt = ::GetFileExt(szDstFileName);
	sSrcExt.MakeLower();
	sDstExt.MakeLower();
	if (CDib::IsJPEGExt(sDstExt))
	{
		// Flatten
		if (pSaveDib->HasAlpha() && pSaveDib->GetBitCount() == 32)
		{
			pSaveDib->RenderAlphaWithSrcBackground();
			pSaveDib->SetAlpha(FALSE);
		}

		if (CDib::IsJPEGExt(sSrcExt))
		{
			if (!pSaveDib->SaveJPEG(szDstFileName,
									dwJpegQuality,
									SrcDib.IsGrayscale(),
									szSrcFileName,
									FALSE,
									TRUE,
									pProgressWnd,
									bProgressSend,
									pThread))
				return 0;

			// Clear Orientation,
			// this because orientation is copied from szSrcFileName!
			CDib::JPEGSetOrientationInplace(szDstFileName,
											1,
											FALSE);
		}
		else
		{
			if (!pSaveDib->SaveJPEG(szDstFileName,
									dwJpegQuality,
									SrcDib.IsGrayscale(),
									_T(""),
									TRUE,
									FALSE,
									pProgressWnd,
									bProgressSend,
									pThread))
				return 0;
		}
	}
	else if (CDib::IsTIFFExt(sDstExt))
	{
		if (CDib::IsTIFFExt(sSrcExt))
		{
			if (SrcDib.m_FileInfo.m_nImageCount > 1)
			{
				if (!pSaveDib->SaveMultiPageTIFF(	szDstFileName,
													szSrcFileName,
													((CUImagerApp*)::AfxGetApp())->GetAppTempDir(),
													bTiffForceCompression ? nTiffCompression : SrcDib.m_FileInfo.m_nCompression,
													dwJpegQuality,
													pProgressWnd,
													bProgressSend,
													pThread))
					return 0;

			}
			else
			{
				if (!pSaveDib->SaveTIFF(szDstFileName,
										bTiffForceCompression ? nTiffCompression : SrcDib.m_FileInfo.m_nCompression,
										dwJpegQuality,
										pProgressWnd,
										bProgressSend,
										pThread))
					return 0;
			}

		}
		else
		{
			if (!pSaveDib->SaveTIFF(szDstFileName,
									nTiffCompression,
									dwJpegQuality,
									pProgressWnd,
									bProgressSend,
									pThread))
				return 0;
		}
	}
	else if (sDstExt == _T(".bmp") || sDstExt == _T(".dib"))
	{
		// RLE Compress
		if ((SrcDib.GetBitCount() == 4						||
			SrcDib.GetBitCount() == 8)						&&
			(SrcDib.m_FileInfo.m_nCompression == BI_RLE4	||
			SrcDib.m_FileInfo.m_nCompression == BI_RLE8))
		{
			pSaveDib->Compress((SrcDib.GetBitCount() == 4)
								? BI_RLE4 : BI_RLE8);
		}
		// Store Alpha using the V5 Header
		else if (SrcDib.HasAlpha() && SrcDib.GetBitCount() == 32)
		{
			pSaveDib->ToBITMAPV5HEADER();
		}

		if (!pSaveDib->SaveBMP(	szDstFileName,
								pProgressWnd,
								bProgressSend,
								pThread))
			return 0;
	}
	else if (sDstExt == _T(".gif"))
	{
		if (!CPictureDoc::SaveGIF(	szDstFileName,
									pSaveDib,
									pProgressWnd,
									bProgressSend,
									pThread))
			return 0;
	}
	else if (sDstExt == _T(".png"))
	{
		if (!CPictureDoc::SavePNG(	szDstFileName,
									pSaveDib,
									SrcDib.m_FileInfo.m_bHasBackgroundColor,
									pProgressWnd,
									bProgressSend,
									pThread))
			return 0;
	}
	else if (sDstExt == _T(".pcx"))
	{
		// Flatten
		if (pSaveDib->HasAlpha() && pSaveDib->GetBitCount() == 32)
		{
			pSaveDib->RenderAlphaWithSrcBackground();
			pSaveDib->SetAlpha(FALSE);
		}

		if (!pSaveDib->SavePCX(	szDstFileName,
								pProgressWnd,
								bProgressSend,
								pThread))
			return 0;
	}
	else if (sDstExt == _T(".emf"))
	{
		// Flatten
		if (pSaveDib->HasAlpha() && pSaveDib->GetBitCount() == 32)
		{
			pSaveDib->RenderAlphaWithSrcBackground();
			pSaveDib->SetAlpha(FALSE);
		}

		if (!pSaveDib->SaveEMF(szDstFileName))
			return 0;
	}
	else
		return 0; // unsupported extension

	return 1; // saved
}

// Return Value:
// 0  : Error
// 1  : Ok
// Shrink All Pages of a Multi-Page Tiff
int CUImagerApp::ShrinkPictureMultiPage(LPCTSTR szSrcFileName,
										LPCTSTR szDstFileName,
										DWORD dwMaxSize,
										BOOL bMaxSizePercent,
										DWORD dwJpegQuality,
										int nTiffCompression,
										BOOL bTiffForceCompression,
										BOOL bShrinkPictureSize,
										BOOL bSharpenAfterShrink,
										CWnd* pProgressWnd,
										BOOL bProgressSend,
										CWorkerThread* pThread)
{
	BOOL res = FALSE;
	int nPageNum;
	int nPageCount;
	CDib::ARRAY a;
	CArray<int,int> Compression;

	// Load First Page
	CDib* pSrcDib = new CDib;
	if (!pSrcDib)
		return 0;
	pSrcDib->SetShowMessageBoxOnError(FALSE);
	if (!pSrcDib->LoadTIFF(szSrcFileName, 0, FALSE, pProgressWnd, bProgressSend, pThread))
	{
		delete pSrcDib;
		return 0;
	}
	nPageCount = pSrcDib->m_FileInfo.m_nImageCount;

	// Create array of dibs
	for (nPageNum = 0 ; nPageNum < nPageCount ; nPageNum++)
	{	
		// Load
		if (nPageNum == 0)
		{
			a.Add(pSrcDib);
			Compression.Add(bTiffForceCompression ? nTiffCompression : pSrcDib->m_FileInfo.m_nCompression);
		}
		else
		{
			// Load
			a.Add(new CDib);
			if (!a[a.GetUpperBound()])
			{
				CDib::FreeArray(a);
				return 0;
			}	
			a[a.GetUpperBound()]->SetShowMessageBoxOnError(FALSE);
			if (!a[a.GetUpperBound()]->LoadTIFF(szSrcFileName,
												nPageNum,
												FALSE,
												pProgressWnd,
												bProgressSend,
												pThread))
			{
				CDib::FreeArray(a);
				return 0;
			}
			Compression.Add(bTiffForceCompression ? nTiffCompression : a[a.GetUpperBound()]->m_FileInfo.m_nCompression);
		}

		// Init Vars
		int nSrcXDpi = a[a.GetUpperBound()]->GetXDpi();
		int nSrcYDpi = a[a.GetUpperBound()]->GetYDpi();
		DWORD dwSrcWidth = a[a.GetUpperBound()]->GetWidth();
		DWORD dwSrcHeight = a[a.GetUpperBound()]->GetHeight();
		DWORD dwShrinkWidth = dwSrcWidth;
		DWORD dwShrinkHeight = dwSrcHeight;

		// Shrink Picture Size?
		if (bShrinkPictureSize)
		{
			// Calc Shrink
			BOOL bDoShrink = CalcShrink(a[a.GetUpperBound()]->GetWidth(),
										a[a.GetUpperBound()]->GetHeight(),
										dwMaxSize,
										bMaxSizePercent,
										dwShrinkWidth,
										dwShrinkHeight);

			// Shrink
			if (bDoShrink)
			{
				if (!a[a.GetUpperBound()]->StretchBits(	dwShrinkWidth,
														dwShrinkHeight,
														NULL,
														pProgressWnd,
														bProgressSend,
														pThread))
				{
					CDib::FreeArray(a);
					return 0;
				}
				else
				{						
					// Sharpen
					if (bSharpenAfterShrink)
					{
						int Kernel[] = {-1,-1,-1,
										-1,20,-1,
										-1,-1,-1};
						if (!a[a.GetUpperBound()]->FilterFast(	Kernel, 12,
																NULL,
																pProgressWnd,
																bProgressSend,
																pThread))
						{
							CDib::FreeArray(a);
							return 0;
						}
					}
				}
			}
		}

		// Set new DPI
		a[a.GetUpperBound()]->SetXDpi(Round((double)dwShrinkWidth  * (double)nSrcXDpi / (double)dwSrcWidth));
		a[a.GetUpperBound()]->SetYDpi(Round((double)dwShrinkHeight * (double)nSrcYDpi / (double)dwSrcHeight));

		// Auto Orientate
		CDib::AutoOrientateDib(a[a.GetUpperBound()]);

		// Clear Orientation
		a[a.GetUpperBound()]->GetExifInfo()->Orientation = 1;
	}

	// Save all TIFF pages
	res = CDib::SaveMultiPageTIFF(	szDstFileName,
									a,
									Compression,
									dwJpegQuality,
									pProgressWnd,
									bProgressSend,
									pThread);

	// Free
	CDib::FreeArray(a);

	if (res)
		return 1; // Saved
	else
		return 0; // Error
}

CString CUImagerApp::ShrinkGetDstExt(CString sSrcExt)
{
	sSrcExt.TrimLeft(_T('.'));
	sSrcExt.MakeLower();

	if ((sSrcExt == _T("bmp"))	||
		(sSrcExt == _T("dib"))	||
		(sSrcExt == _T("emf"))	||
		(sSrcExt == _T("pcx"))	||
		(sSrcExt == _T("jxr"))	||
		(sSrcExt == _T("webp"))	||
		(sSrcExt == _T("heic"))	||
		(sSrcExt == _T("avif")) ||
		CDib::IsJPEGExt(sSrcExt))
		return _T(".jpg");
	else if (CDib::IsTIFFExt(sSrcExt))
		return _T(".tif");
	else if (sSrcExt == _T("gif"))
		return _T(".gif");
	else if (sSrcExt == _T("png"))
		return _T(".png");
	else
		return (_T(".") + sSrcExt);
}

void CUImagerApp::OnFileCloseall() 
{
	if (SaveAllModified())
		CloseAll();
}

void CUImagerApp::OnUpdateFileCloseall(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(!m_bClosingAll);
}

BOOL CUImagerApp::CloseAll() 
{
	BOOL bAllClosed = TRUE;
	CView* pView;
	CDocument* pDoc;
	CUImagerMultiDocTemplate* curTemplate;
	POSITION posTemplate, posDoc, posView;
	posTemplate = GetFirstDocTemplatePosition();
	while (posTemplate)
	{
		curTemplate = (CUImagerMultiDocTemplate*)GetNextDocTemplate(posTemplate);
		posDoc = curTemplate->GetFirstDocPosition();
		while (posDoc)
		{
			pDoc = (CDocument*)(curTemplate->GetNextDoc(posDoc));
			if (pDoc)
			{
				posView = pDoc->GetFirstViewPosition();
				while (posView != NULL)
				{
					pDoc->SetModifiedFlag(FALSE);
					pView = pDoc->GetNextView(posView);
					ASSERT_VALID(pView);
					if (pView && pView->GetParentFrame())
						pView->GetParentFrame()->PostMessage(WM_CLOSE, 0, 0);
					bAllClosed = FALSE;
				}   
			}
		}
	}

	// The First Time?
	if (!m_bClosingAll)
	{
		if (!bAllClosed)
		{
			m_bClosingAll = TRUE;
			::AfxGetMainFrame()->SetTimer(ID_TIMER_CLOSING_ALL, CLOSING_CHECK_INTERVAL_TIMER_MS, NULL);
		}
		else
		{
			// Post a Message to the Main Frame to notify
			// that all Documents Are Closed
			::AfxGetMainFrame()->PostMessage(WM_ALL_CLOSED, 0, 0);
		}
	}
	else
	{
		// We Are Done :-)
		if (bAllClosed)
		{
			m_bClosingAll = FALSE;

			// Kill Timer
			::AfxGetMainFrame()->KillTimer(ID_TIMER_CLOSING_ALL);

			// Post a Message to the Main Frame to notify
			// that all Documents Are Closed
			::AfxGetMainFrame()->PostMessage(WM_ALL_CLOSED, 0, 0);
		}
	}

	return bAllClosed;
}

void CUImagerApp::LoadPlacement(UINT showCmd/*=SW_SHOWNORMAL*/)
{
	CString sSection(_T("GeneralApp"));

#ifdef VIDEODEVICEDOC
	if (!m_bServiceProcess)
#endif
	{
		LPBYTE pData = NULL;
		UINT nBytes = 0;
		GetProfileBinary(sSection, _T("WindowPlacement"), &pData, &nBytes);
		WINDOWPLACEMENT *pwp = (WINDOWPLACEMENT*)pData;
		if (pwp && (nBytes == sizeof(WINDOWPLACEMENT)) &&
			::IntersectsValidMonitor(&(pwp->rcNormalPosition)))
		{
			// Hide? If not hide -> use previous Show Command
			if (showCmd == SW_HIDE)
				pwp->showCmd = SW_HIDE;
			
			// Restore if Minimized
			if (pwp->showCmd == SW_SHOWMINIMIZED)
				pwp->showCmd = SW_RESTORE;

			// Set
			m_pMainWnd->SetWindowPlacement(pwp);
		}
		if (pData)
			delete [] pData;

		// Set flag
		m_bPlacementLoaded = TRUE;
	}
}

void CUImagerApp::LoadSettings(UINT showCmd/*=SW_SHOWNORMAL*/)
{
	CString sSection(_T("GeneralApp"));

	// Log Level
	g_nLogLevel = MIN(2, MAX(0, GetProfileInt(sSection, _T("LogLevel"), 0)));

	// Donor E-mail
	g_DonorEmailValidateThread.m_sDonorEmail = GetProfileString(sSection, _T("DonorEmail"), _T(""));

	// MainFrame Placement
	LoadPlacement(showCmd);

	// Preview File Dialog
	m_bFileDlgPreview = (BOOL)GetProfileInt(sSection, _T("FileDlgPreview"), TRUE);
	g_nPreviewFileDlgViewMode = GetProfileInt(sSection, _T("PreviewFileDlgViewMode"), SHVIEW_Default);

	// Last Opened Directory
	m_sLastOpenedDir = GetProfileString(sSection, _T("LastOpenedDir"), _T(""));

	// Coordinate Units
	m_nCoordinateUnit = GetProfileInt(sSection, _T("CoordinateUnit"), COORDINATES_PIX);

	// New Dialog
	m_nNewWidth = GetProfileInt(sSection, _T("NewWidth"), DEFAULT_NEW_WIDTH);
	m_nNewHeight = GetProfileInt(sSection, _T("NewHeight"), DEFAULT_NEW_HEIGHT);
	m_nNewXDpi = GetProfileInt(sSection, _T("NewXDpi"), DEFAULT_NEW_DPI);
	m_nNewYDpi = GetProfileInt(sSection, _T("NewYDpi"), DEFAULT_NEW_DPI);
	if (m_nCoordinateUnit == COORDINATES_INCH)
		m_nNewPhysUnit = GetProfileInt(sSection, _T("NewPhysUnit"), 1);	// inch
	else
		m_nNewPhysUnit = GetProfileInt(sSection, _T("NewPhysUnit"), 0);	// cm
	m_sNewPaperSize = GetProfileString(sSection, _T("NewPaperSize"), DEFAULT_NEW_PAPER_SIZE);
	m_crNewBackgroundColor = (COLORREF)GetProfileInt(sSection, _T("NewBackgroundColor"), DEFAULT_NEW_COLOR);

#ifdef VIDEODEVICEDOC
	// Wait time before autostarting first device
	m_dwFirstStartDelayMs = (DWORD)GetProfileInt(sSection, _T("FirstStartDelayMs"), DEFAULT_FIRSTSTART_DELAY_MS);

	// Simultaneous saving
	m_nSimultaneousSaving = GetProfileInt(sSection, _T("SimultaneousSaving"), DEFAULT_SIMULTANEOUS_SAVING);

	// mov/mp4 saving fragmented
	m_bMovFragmented = (BOOL)GetProfileInt(sSection, _T("MovFragmented"), FALSE);

	// Start Micro Apache
	m_bStartMicroApache = (BOOL)GetProfileInt(sSection, _T("StartMicroApache"), TRUE);

	// Micro Apache Document Root
	m_sMicroApacheDocRoot = GetProfileString(sSection, _T("MicroApacheDocRoot"), m_sMicroApacheDocRoot);
	::CreateDir(m_sMicroApacheDocRoot);

	// Micro Apache Server Ports
	m_nMicroApachePort = GetProfileInt(sSection, _T("MicroApachePort"), MICROAPACHE_DEFAULT_PORT);
	m_nMicroApachePortSSL = GetProfileInt(sSection, _T("MicroApachePortSSL"), MICROAPACHE_DEFAULT_PORT_SSL);

	// Authentication
	m_sMicroApacheUsername = GetSecureProfileString(sSection, _T("MicroApacheUsernameExportable"));
	m_sMicroApachePassword = GetSecureProfileString(sSection, _T("MicroApachePasswordExportable"));

	// SSL certificate files
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	TCHAR szProgramName[MAX_PATH];
	CString sDefaultCertFileSSL;
	CString sDefaultKeyFileSSL;
	if (::GetModuleFileName(NULL, szProgramName, MAX_PATH) != 0)
	{
		_tsplitpath(szProgramName, szDrive, szDir, NULL, NULL);
		sDefaultCertFileSSL = CString(szDrive) + CString(szDir) + MICROAPACHE_DEFAULT_CERTFILE_SSL;
		sDefaultKeyFileSSL = CString(szDrive) + CString(szDir) + MICROAPACHE_DEFAULT_KEYFILE_SSL;
	}
	m_sMicroApacheCertFileSSL = GetProfileString(sSection, _T("MicroApacheCertFileSSL"), sDefaultCertFileSSL);
	m_sMicroApacheKeyFileSSL = GetProfileString(sSection, _T("MicroApacheKeyFileSSL"), sDefaultKeyFileSSL);
#endif
}

void CUImagerApp::SavePlacement()
{
	if (m_bPlacementLoaded && m_bCanSavePlacements)
	{
		WINDOWPLACEMENT wndpl;
		memset(&wndpl, 0, sizeof(wndpl));
		wndpl.length = sizeof(wndpl);
		if (::AfxGetMainFrame()->GetWindowPlacement(&wndpl))
			WriteProfileBinary(_T("GeneralApp"), _T("WindowPlacement"), (BYTE*)&wndpl, sizeof(wndpl));
	}
}

void CUImagerApp::SavePlacements()
{
	SavePlacement(); // store main window placement
#ifdef VIDEODEVICEDOC
	POSITION posTemplate = GetFirstDocTemplatePosition();
	while (posTemplate)
	{
		CUImagerMultiDocTemplate* curTemplate = (CUImagerMultiDocTemplate*)GetNextDocTemplate(posTemplate);
		POSITION posDoc = curTemplate->GetFirstDocPosition();
		while (posDoc)
		{
			CDocument* pDoc = curTemplate->GetNextDoc(posDoc);
			if (pDoc && pDoc->IsKindOf(RUNTIME_CLASS(CVideoDeviceDoc)))
				((CVideoDeviceDoc*)pDoc)->SavePlacement(); // store video device window placement
		}
	}
#endif
}

void CUImagerApp::SendOpenDocsAsMail()
{
	CSendMailDocsDlg dlg(::AfxGetMainFrame());
	if (dlg.DoModal() == IDOK)
	{
		// Create & Do Empty Email Temp Dir
		CString sTempEmailDir;
		sTempEmailDir.Format(_T("Email%X"), ::GetCurrentProcessId());
		sTempEmailDir = ((CUImagerApp*)::AfxGetApp())->GetAppTempDir() + sTempEmailDir;
		if (!::IsExistingDir(sTempEmailDir))
		{
			if (!::CreateDir(sTempEmailDir))
			{
				::ShowErrorMsg(::GetLastError(), TRUE);
				return;
			}
		}
		else
		{
			if (!::DeleteDirContent(sTempEmailDir))
			{
				::AfxMessageBox(ML_STRING(1225, "Error While Deleting The Temporary Folder."), MB_OK | MB_ICONSTOP);
				return;
			}
		}

		// Shrink
		if (dlg.m_nOptimizationSelection == CSendMailDocsDlg::EMAIL_OPT) // Send Email Optimized
		{
			ShrinkOpenDocs(	sTempEmailDir,
							AUTO_SHRINK_MAX_SIZE,
							FALSE,
							DEFAULT_JPEGCOMPRESSION,
							TRUE,
							FALSE);
		}
		else if (dlg.m_nOptimizationSelection == CSendMailDocsDlg::NO_OPT) // Leave Unchanged
		{
			ShrinkOpenDocs(	sTempEmailDir,
							0,
							FALSE,
							0,
							FALSE,
							TRUE);
		}
		else if (dlg.m_nOptimizationSelection == CSendMailDocsDlg::ADV_OPT) // Advanced Settings
		{
			ShrinkOpenDocs(	sTempEmailDir,
							(dlg.m_nPixelsPercentSel == 0) ? dlg.m_nShrinkingPixels : dlg.m_nShrinkingPercent,
							(dlg.m_nPixelsPercentSel == 1),
							dlg.m_nJpegQuality,
							dlg.m_bShrinkingPictures,
							FALSE);
		}
		
		// Do Zip?
		if (dlg.m_bZipFile)
		{
			CString sZipFileName;
			if (dlg.m_sZipFileName == _T(""))
				sZipFileName = _T("Files.zip"); 
			else
				sZipFileName = dlg.m_sZipFileName;

			// Adjust if no correct extension or no extension at all supplied
			sZipFileName = ::GetFileNameNoExt(sZipFileName);
			sZipFileName += _T(".zip");

			// Create Email Zip Temp Directory if not existing
			CString sTempEmailZipDir;
			sTempEmailZipDir.Format(_T("EmailZip%X"), ::GetCurrentProcessId());
			sTempEmailZipDir = ((CUImagerApp*)::AfxGetApp())->GetAppTempDir() + sTempEmailZipDir;
			if (!::IsExistingDir(sTempEmailZipDir))
			{
				if (!::CreateDir(sTempEmailZipDir))
				{
					::ShowErrorMsg(::GetLastError(), TRUE);
					return;
				}
			}

			// Compress & Send
			if (CompressToZip(sTempEmailDir, sTempEmailZipDir + _T("\\") + sZipFileName))
				SendMail(sTempEmailZipDir + _T("\\") + sZipFileName);

			// Delete Email Zip Temp Directory
			::DeleteDir(sTempEmailZipDir);
		}
		// Send Email Directory Content
		else
			SendMail(sTempEmailDir);
		
		// Delete Email Temp Directory
		::DeleteDir(sTempEmailDir);
	}
}

class CMailState : public CNoTrackObject
{
public:
	HINSTANCE m_hInstMail;      // handle to MAPI32.DLL
	virtual ~CMailState();
};

CMailState::~CMailState()
{
	if (m_hInstMail != NULL)
		::FreeLibrary(m_hInstMail);
}

EXTERN_PROCESS_LOCAL(CMailState, MailState)
BOOL CUImagerApp::SendMail(LPCTSTR szAttachment)
{
	// Begin Wait Cursor
	BeginWaitCursor();

	TCHAR szName[_MAX_FNAME];
	TCHAR szExt[_MAX_EXT];
	TCHAR szTitle[MAX_ATTACHMENTS][_MAX_PATH];
	TCHAR szTempName[MAX_ATTACHMENTS][_MAX_PATH];
	char szTitleA[MAX_ATTACHMENTS][_MAX_PATH];
	char szTempNameA[MAX_ATTACHMENTS][_MAX_PATH];
	int nFileCount = 0;
	MapiFileDesc fileDesc[MAX_ATTACHMENTS];
	
	CMailState* pMailState = MailState;
	if (pMailState->m_hInstMail == NULL)
		pMailState->m_hInstMail = ::LoadLibraryFromSystem32(_T("MAPI32.DLL"));

	if (pMailState->m_hInstMail == NULL)
	{
		EndWaitCursor();
		::AfxMessageBox(AFX_IDP_FAILED_MAPI_LOAD);
		return FALSE;
	}
	ASSERT(pMailState->m_hInstMail != NULL);

	// When our 32-bit application calls MAPISendMail on a computer with 64-bit Windows and 64-bit Outlook:
	// the MAPI library makes a COM call to launch a 64-bit Fixmapi.exe application. The Fixmapi application
	// implicitly links to the MAPI library, which routes the function call to the Windows MAPI stub, which in
	// turn forwards the call to the Outlook MAPI stub, enabling the MAPISendMail function call to succeed. 
	// https://msdn.microsoft.com/en-us/library/office/dd941355%28v=office.15%29.aspx
	// Note: I successfully tested MAPISendMail on Windows 10 64-bit with Outlook 2013 64-bit
	ULONG (PASCAL *lpfnSendMail)(ULONG, ULONG, MapiMessage*, FLAGS, ULONG);
	(FARPROC&)lpfnSendMail = ::GetProcAddress(pMailState->m_hInstMail, "MAPISendMail");
	if (lpfnSendMail == NULL)
	{
		EndWaitCursor();
		::AfxMessageBox(AFX_IDP_INVALID_MAPI_DLL);
		return FALSE;
	}

	DWORD dwAttrib = ::GetFileAttributes(szAttachment);
	if (dwAttrib != 0xFFFFFFFF)
	{
		if (dwAttrib & FILE_ATTRIBUTE_DIRECTORY) // Directory
		{
			TCHAR name[MAX_PATH];
			WIN32_FIND_DATA info;

			// Adjust Directory Name
			CString sAdjPath(szAttachment);
			sAdjPath.TrimRight(_T('\\'));

			// Start with File Search
			_stprintf(name, _T("%s\\*"), (LPCTSTR)sAdjPath);
			HANDLE hFileSearch = ::FindFirstFile(name, &info);
			if (!hFileSearch || hFileSearch == INVALID_HANDLE_VALUE)
			{
				EndWaitCursor();
				return FALSE;
			}
			do
			{
				_stprintf(name, _T("%s\\%s"), (LPCTSTR)sAdjPath, info.cFileName);
				if (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					continue;
				else
				{
					// File Count Check
					if (nFileCount == MAX_ATTACHMENTS)
					{
						EndWaitCursor();
						CString Str;
						Str.Format(	ML_STRING(1214, "Too Many Attached Files. Max Number is %i.\n") +
									ML_STRING(1215, "Tip: Use a Zip File to put all your files in it."),
									MAX_ATTACHMENTS);
						::AfxMessageBox(Str);
						::FindClose(hFileSearch);
						return FALSE;
					}

					// File Path
					_tcscpy(szTempName[nFileCount], name);
					_wcstombsz(szTempNameA[nFileCount], szTempName[nFileCount], _MAX_PATH);

					// Build an appropriate title for the attachment
					_tsplitpath(name, NULL, NULL, szName, szExt);
					_tcscpy(szTitle[nFileCount], CString(szName) + CString(szExt));
					_wcstombsz(szTitleA[nFileCount], szTitle[nFileCount], _MAX_PATH);

					// Prepare the file description (for the attachment)
					memset(&fileDesc[nFileCount], 0, sizeof(MapiFileDesc));
					fileDesc[nFileCount].nPosition = (ULONG)-1;
					fileDesc[nFileCount].lpszPathName = szTempNameA[nFileCount];
					fileDesc[nFileCount].lpszFileName = szTitleA[nFileCount];

					nFileCount++;
				}
			}
			while (::FindNextFile(hFileSearch, &info));

			// Close File Search
			::FindClose(hFileSearch);
		}
		else // Normal File
		{
			// File Path
			_tcscpy(szTempName[0], szAttachment);
			_wcstombsz(szTempNameA[0], szTempName[0], _MAX_PATH);

			// Build an appropriate title for the attachment
			_tsplitpath(szAttachment, NULL, NULL, szName, szExt);
			_tcscpy(szTitle[0], CString(szName) + CString(szExt));
			_wcstombsz(szTitleA[0], szTitle[0], _MAX_PATH);

			// Prepare the file description (for the attachment)
			memset(&fileDesc[0], 0, sizeof(MapiFileDesc));
			fileDesc[0].nPosition = (ULONG)-1;
			fileDesc[0].lpszPathName = szTempNameA[0];
			fileDesc[0].lpszFileName = szTitleA[0];

			nFileCount = 1;
		}
	}
	else
	{
		EndWaitCursor();
		return FALSE;
	}

	// Prepare the message
	MapiMessage message;
	memset(&message, 0, sizeof(message));
	message.nFileCount = nFileCount;
	message.lpFiles = fileDesc;	

	// Prepare for modal dialog box
	EnableModeless(FALSE);
	HWND hWndTop;
	CWnd* pParentWnd = CWnd::GetSafeOwner(NULL, &hWndTop);

	// Some extra precautions are required to use MAPISendMail as it
	// tends to enable the parent window in between dialogs (after
	// the login dialog, but before the send note dialog).
	pParentWnd->SetCapture();
	::SetFocus(NULL);
	pParentWnd->m_nFlags |= WF_STAYDISABLED;

	// End Wait Cursor
	EndWaitCursor();

	// Send Mail
	int nError = lpfnSendMail(0, (ULONG)pParentWnd->GetSafeHwnd(),
		&message, MAPI_LOGON_UI|MAPI_DIALOG, 0);

	// After returning from the MAPISendMail call, the window must
	// be re-enabled and focus returned to the frame to undo the workaround
	// done before the MAPI call.
	::ReleaseCapture();
	pParentWnd->m_nFlags &= ~WF_STAYDISABLED;

	pParentWnd->EnableWindow(TRUE);
	::SetActiveWindow(NULL);
	pParentWnd->SetActiveWindow();
	pParentWnd->SetFocus();
	if (hWndTop != NULL)
		::EnableWindow(hWndTop, TRUE);
	EnableModeless(TRUE);

	if (nError != SUCCESS_SUCCESS &&
		nError != MAPI_USER_ABORT &&
		nError != MAPI_E_LOGIN_FAILURE)
	{
		::AfxMessageBox(AFX_IDP_FAILED_MAPI_SEND);
		return FALSE;
	}
	else
		return TRUE;
}
PROCESS_LOCAL(CMailState, MailState)

BOOL CUImagerApp::IsSupportedPictureFile(CString sFileName)
{
	CString sExt = ::GetFileExt(sFileName);
	if ((sExt == _T(".bmp"))	||
		(sExt == _T(".dib"))	||
		(sExt == _T(".emf"))	||
		(sExt == _T(".png"))	||
		(sExt == _T(".pcx"))	||
		(sExt == _T(".gif"))	||
		(sExt == _T(".jxr"))	||
		(sExt == _T(".webp"))	||
		(sExt == _T(".heic"))	||
		(sExt == _T(".avif"))	||
		CDib::IsJPEGExt(sExt)	||
		CDib::IsTIFFExt(sExt))
		return TRUE;
	else
		return FALSE;
}

BOOL CUImagerApp::IsLoadOnlyPictureFile(CString sFileName)
{
	CString sExt = ::GetFileExt(sFileName);
	if ((sExt == _T(".jxr"))	||
		(sExt == _T(".webp"))	||
		(sExt == _T(".heic"))	||
		(sExt == _T(".avif")))
		return TRUE;
	else
		return FALSE;
}

CUImagerMultiDocTemplate* CUImagerApp::GetTemplateFromFileExtension(CString sFileName)
{
	if (IsSupportedPictureFile(sFileName))
		return GetPictureDocTemplate();
	else
		return NULL;
}

void CUImagerApp::OnClearRecentFileList()
{
	if (m_pRecentFileList)
	{
		// Clear
		int iMRU = m_pRecentFileList->GetSize();
		while (iMRU > 0)
			m_pRecentFileList->Remove(--iMRU);

		// Write to registry or ini file now
		// (done also in ExitInstance() by SaveStdProfileSettings())
		m_pRecentFileList->WriteList();
	}
}

void CUImagerApp::OnUpdateClearRecentFileList(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_pRecentFileList && !((*m_pRecentFileList)[0].IsEmpty()));
}

void CUImagerApp::OnUpdateRecentFileMenu(CCmdUI* pCmdUI)
{
	// Taken from CWinApp::OnUpdateRecentFileMenu(CCmdUI* pCmdUI)
	ASSERT_VALID(this);
	ENSURE_ARG(pCmdUI != NULL);
	if (m_pRecentFileList == NULL) // no MRU files
		pCmdUI->Enable(FALSE);
	else
	{
		// When the Recent File List is cleared we must delete all menu items except the
		// starting ID_FILE_MRU_FILE1 because m_pRecentFileList->UpdateMenu() is not doing it
		if ((*m_pRecentFileList)[0].IsEmpty())
		{
			for (int iMRU = 1; iMRU < m_pRecentFileList->GetSize(); iMRU++)
				pCmdUI->m_pMenu->DeleteMenu(pCmdUI->m_nID + iMRU, MF_BYCOMMAND);
		}
		m_pRecentFileList->UpdateMenu(pCmdUI);
	}
}

void CUImagerApp::OnFileNew() 
{
	if (!::AfxGetMainFrame()->m_bFullScreenMode)
	{
		// Make New Document
		CPictureDoc* pDoc = (CPictureDoc*)GetPictureDocTemplate()->OpenDocumentFile(NULL);
		if (pDoc)
		{
			// Check
			if (!pDoc->m_pDib)
			{
				pDoc->CloseDocument();
				return;
			}

			// Ask size
			CNewDlg dlg(m_nNewWidth,
						m_nNewHeight,
						m_nNewXDpi,
						m_nNewYDpi,
						m_nNewPhysUnit,
						m_sNewPaperSize,
						m_crNewBackgroundColor,
						::AfxGetMainFrame());
			if (dlg.DoModal() == IDOK)
			{
				m_nNewWidth = dlg.m_nPixelsWidth;
				m_nNewHeight = dlg.m_nPixelsHeight;
				m_nNewXDpi = dlg.m_nXDpi;
				m_nNewYDpi = dlg.m_nYDpi;
				m_nNewPhysUnit = dlg.m_PhysUnit;
				m_sNewPaperSize = dlg.m_sPaperSize;
				m_crNewBackgroundColor = dlg.m_crBackgroundColor;
				WriteProfileInt(_T("GeneralApp"), _T("NewWidth"), m_nNewWidth);
				WriteProfileInt(_T("GeneralApp"), _T("NewHeight"), m_nNewHeight);
				WriteProfileInt(_T("GeneralApp"), _T("NewXDpi"), m_nNewXDpi);
				WriteProfileInt(_T("GeneralApp"), _T("NewYDpi"), m_nNewYDpi);
				WriteProfileInt(_T("GeneralApp"), _T("NewPhysUnit"), m_nNewPhysUnit);
				WriteProfileString(_T("GeneralApp"), _T("NewPaperSize"), m_sNewPaperSize);
				WriteProfileInt(_T("GeneralApp"), _T("NewBackgroundColor"), m_crNewBackgroundColor);
			}
			else
			{
				pDoc->CloseDocument();
				return;
			}

			// Allocate
			pDoc->m_pDib->SetBackgroundColor(pDoc->m_crBackgroundColor);
			if (!pDoc->m_pDib->AllocateBits(DEFAULT_NEW_BPP,
											BI_RGB,
											dlg.m_nPixelsWidth,
											dlg.m_nPixelsHeight,
											dlg.m_crBackgroundColor))
			{
				pDoc->CloseDocument();
				return;
			}

			// Set DPI
			pDoc->m_pDib->SetXDpi(dlg.m_nXDpi);
			pDoc->m_pDib->SetYDpi(dlg.m_nYDpi);
			
			// Init Doc Rect
			pDoc->m_DocRect.top = 0;
			pDoc->m_DocRect.left = 0;
			pDoc->m_DocRect.right = pDoc->m_pDib->GetWidth();
			pDoc->m_DocRect.bottom = pDoc->m_pDib->GetHeight();
			
			// Update
			pDoc->SetModifiedFlag();
			pDoc->SetDocumentTitle();
			pDoc->UpdateAlphaRenderedDib();

			// Zoom
			CZoomComboBox* pZoomCB = &(((CPictureToolBar*)((CToolBarChildFrame*)(pDoc->GetFrame()))->GetToolBar())->m_ZoomComboBox);
			pZoomCB->SetCurSel(pDoc->m_nZoomComboBoxIndex);
			pZoomCB->OnChangeZoomFactor(*((double*)(pZoomCB->GetItemDataPtr(pDoc->m_nZoomComboBoxIndex))));

			// Fit to document
			if (!pDoc->GetFrame()->IsZoomed())
			{
				pDoc->GetView()->GetParentFrame()->SetWindowPos(NULL,
																0, 0, 0, 0,
																SWP_NOSIZE |
																SWP_NOZORDER);
				pDoc->GetView()->UpdateWindowSizes(FALSE, FALSE, TRUE);
			}
			else
				pDoc->GetView()->UpdateWindowSizes(TRUE, TRUE, FALSE);
		}
	}
}

void CUImagerApp::OnUpdateFileNew(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(::AfxGetMainFrame() && !::AfxGetMainFrame()->m_bFullScreenMode);
}

void CUImagerApp::OnEditPaste() 
{
	if (!::AfxGetMainFrame()->m_bFullScreenMode)
	{
		// Make New Document
		CPictureDoc* pDoc = (CPictureDoc*)GetPictureDocTemplate()->OpenDocumentFile(NULL);
		if (pDoc)
		{
			// Check
			if (!pDoc->m_pDib)
			{
				pDoc->CloseDocument();
				return;
			}

			// Paste
			pDoc->m_pDib->SetBackgroundColor(pDoc->m_crBackgroundColor);
			pDoc->m_pDib->EditPaste();
			
			// Init Doc Rect
			pDoc->m_DocRect.top = 0;
			pDoc->m_DocRect.left = 0;
			pDoc->m_DocRect.right = pDoc->m_pDib->GetWidth();
			pDoc->m_DocRect.bottom = pDoc->m_pDib->GetHeight();
			
			// Update
			pDoc->SetModifiedFlag();
			pDoc->SetDocumentTitle();
			pDoc->UpdateAlphaRenderedDib();

			// Zoom
			CZoomComboBox* pZoomCB = &(((CPictureToolBar*)((CToolBarChildFrame*)(pDoc->GetFrame()))->GetToolBar())->m_ZoomComboBox);
			pZoomCB->SetCurSel(pDoc->m_nZoomComboBoxIndex);
			pZoomCB->OnChangeZoomFactor(*((double*)(pZoomCB->GetItemDataPtr(pDoc->m_nZoomComboBoxIndex))));

			// Fit to document
			if (!pDoc->GetFrame()->IsZoomed())
			{
				pDoc->GetView()->GetParentFrame()->SetWindowPos(NULL,
																0, 0, 0, 0,
																SWP_NOSIZE |
																SWP_NOZORDER);
				pDoc->GetView()->UpdateWindowSizes(FALSE, FALSE, TRUE);
			}
			else
				pDoc->GetView()->UpdateWindowSizes(TRUE, TRUE, FALSE);
		}
	}
}

void CUImagerApp::OnUpdateEditPaste(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(::IsClipboardFormatAvailable(CF_DIBV5)			||
					::IsClipboardFormatAvailable(CF_DIB)			||
					::IsClipboardFormatAvailable(CF_ENHMETAFILE)	||
					::IsClipboardFormatAvailable(CF_HDROP))			&&
					(::AfxGetMainFrame() && !::AfxGetMainFrame()->m_bFullScreenMode));
}

void CUImagerApp::OnEditScreenshot() 
{
	if (!::AfxGetMainFrame()->m_bFullScreenMode)
	{
		CaptureScreenToClipboard();
		OnEditPaste();
	}
}

void CUImagerApp::OnUpdateEditScreenshot(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(::AfxGetMainFrame() && !::AfxGetMainFrame()->m_bFullScreenMode);
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
BOOL CUImagerApp::SettingsPageAppsDefaults(const CString& sTarget/*=_T("")*/)
{
	// Open chosen Settings Page
	IApplicationActivationManager* pActivator;
	HRESULT hr = ::CoCreateInstance(CLSID_ApplicationActivationManager,
									nullptr,
									CLSCTX_INPROC,
									IID_IApplicationActivationManager,
									(void**)&pActivator);
	if (SUCCEEDED(hr))
	{
		// Launch the Immersive Control Panel in two steps: first at the "Default apps" page
		// and then try to open it again at the target. This allows us to at least show the
		// "Default apps" page in case that targets are not supported like for Windows 11.
		//
		// Note: 
		// Starting with Windows 10 we can use the url scheme, but a target cannot be specified:
		// ::ShellExecute(NULL, NULL, _T("ms-settings:defaultapps"), NULL, NULL, SW_SHOWNORMAL);
		DWORD pid;
		hr = pActivator->ActivateApplication(	L"Windows.ImmersiveControlPanel_cw5n1h2txyewy!microsoft.windows.immersivecontrolpanel",
												L"page=SettingsPageAppsDefaults",
												AO_NONE,
												&pid);
		if (SUCCEEDED(hr) && !sTarget.IsEmpty())
		{
			// Target string
			CString sFullTarget;
			sFullTarget.Format(_T("&target=%s"), sTarget);

			// Activate target, note that we do not need to check for errors here because 
			// with the above command we could at least open the "Default apps" page
			pActivator->ActivateApplication(L"Windows.ImmersiveControlPanel_cw5n1h2txyewy!microsoft.windows.immersivecontrolpanel",
											L"page=SettingsPageAppsDefaults" + sFullTarget,
											AO_NONE,
											&pid);
		}
		pActivator->Release();
		return SUCCEEDED(hr);
	}
	else
		return FALSE;
}

void CUImagerApp::UpdateFileAssociations()
{
	BOOL bBmp = FALSE;
	BOOL bJpeg = FALSE;
	BOOL bPcx = FALSE;
	BOOL bEmf = FALSE;
	BOOL bPng = FALSE;
	BOOL bTiff = FALSE;
	BOOL bGif = FALSE;
	BOOL bJxr = FALSE;
	BOOL bWebp = FALSE;
	BOOL bHeic = FALSE;
	BOOL bAvif = FALSE;

#ifndef VIDEODEVICEDOC
	bBmp =		IsFileTypeAssociated(_T("bmp"));
	bJpeg =		IsFileTypeAssociated(_T("jpg"))		&&
				IsFileTypeAssociated(_T("jpeg"))	&&
				IsFileTypeAssociated(_T("jpe"))		&&
				IsFileTypeAssociated(_T("jfif"))	&&
				IsFileTypeAssociated(_T("thm"));
	bPcx =		IsFileTypeAssociated(_T("pcx"));
	bEmf =		IsFileTypeAssociated(_T("emf"));
	bPng =		IsFileTypeAssociated(_T("png"));
	bTiff =		IsFileTypeAssociated(_T("tif"))		&&
				IsFileTypeAssociated(_T("tiff"))	&&
				IsFileTypeAssociated(_T("jfx"));
	bGif =		IsFileTypeAssociated(_T("gif"));
	bJxr =		IsFileTypeAssociated(_T("jxr"));
	bWebp =		IsFileTypeAssociated(_T("webp"));
	bHeic =		IsFileTypeAssociated(_T("heic"));
	bAvif =		IsFileTypeAssociated(_T("avif"));
#endif
	
	if (bBmp)
		AssociateFileType(_T("bmp"));
	else
		UnassociateFileType(_T("bmp"));

	if (bJpeg)
	{
		AssociateFileType(_T("jpg"));
		AssociateFileType(_T("jpeg"));
		AssociateFileType(_T("jpe"));
		AssociateFileType(_T("jfif"));
		AssociateFileType(_T("thm"));
	}
	else
	{
		UnassociateFileType(_T("jpg"));
		UnassociateFileType(_T("jpeg"));
		UnassociateFileType(_T("jpe"));
		UnassociateFileType(_T("jfif"));
		UnassociateFileType(_T("thm"));
	}

	if (bPcx)
		AssociateFileType(_T("pcx"));
	else
		UnassociateFileType(_T("pcx"));

	if (bEmf)
		AssociateFileType(_T("emf"));
	else
		UnassociateFileType(_T("emf"));

	if (bPng)
		AssociateFileType(_T("png"));
	else
		UnassociateFileType(_T("png"));

	if (bTiff)
	{
		AssociateFileType(_T("tif"));
		AssociateFileType(_T("tiff"));
		AssociateFileType(_T("jfx"));
	}
	else
	{
		UnassociateFileType(_T("tif"));
		UnassociateFileType(_T("tiff"));
		UnassociateFileType(_T("jfx"));
	}

	if (bGif)
		AssociateFileType(_T("gif"));
	else
		UnassociateFileType(_T("gif"));

	if (bJxr)
		AssociateFileType(_T("jxr"));
	else
		UnassociateFileType(_T("jxr"));

	if (bWebp)
		AssociateFileType(_T("webp"));
	else
		UnassociateFileType(_T("webp"));

	if (bHeic)
		AssociateFileType(_T("heic"));
	else
		UnassociateFileType(_T("heic"));

	if (bAvif)
		AssociateFileType(_T("avif"));
	else
		UnassociateFileType(_T("avif"));

	// Remove associations from older program versions
	UnassociateFileType(_T("aif")); UnassociateFileType(_T("aiff"));
	UnassociateFileType(_T("au"));
	UnassociateFileType(_T("mid")); UnassociateFileType(_T("rmi"));
	UnassociateFileType(_T("mp3"));
	UnassociateFileType(_T("wav"));
	UnassociateFileType(_T("wma"));
	UnassociateFileType(_T("cda"));
	UnassociateFileType(_T("avi")); UnassociateFileType(_T("divx"));
	UnassociateFileType(_T("zip"));	

	// Notify Changes
	::SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
}

BOOL CUImagerApp::IsFileTypeAssociated(CString sExt)
{
	// Program Name & Path
	TCHAR szProgName[_MAX_FNAME];
	TCHAR szProgExt[_MAX_EXT];
	TCHAR szProgPath[MAX_PATH];
	if (::GetModuleFileName(NULL, szProgPath, MAX_PATH) == 0)
		return FALSE;
	_tsplitpath(szProgPath, NULL, NULL, szProgName, szProgExt);

	// Check Extension
	if (sExt.IsEmpty())
		return FALSE;

	// Lower Case
	sExt.MakeLower();

	// Extension Point Adjust
	CString sExtNoPoint;
	if (sExt[0] != _T('.'))
	{
		sExtNoPoint = sExt;
		sExt = _T('.') + sExt;
	}
	else
		sExtNoPoint = sExt.Right(sExt.GetLength() - 1);

	// Current User FileExts Path
	CString sCurrentUserFileExtsPath = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\") + sExt;

	// My Class Name or ProgID
	CString sMyFileClassName = CString(szProgName) + _T(".") + sExtNoPoint + _T(".1");

	// My Application Name
	CString sMyFileApplicationName = CString(szProgName) + CString(szProgExt);

	// *** Global Settings ***
	CString sCurrentFileClassName = ::GetRegistryStringValue(HKEY_CLASSES_ROOT, sExt, _T(""));

	// *** Current User Settings ***
	// which are higher priority than the Global Settings!
	if (::IsRegistryKey(HKEY_CURRENT_USER, sCurrentUserFileExtsPath))
	{
		// ProgID
		CString sCurrentUserProgID = ::GetRegistryStringValue(HKEY_CURRENT_USER, sCurrentUserFileExtsPath, _T("ProgID"));
		
		// Application
		CString sCurrentUserApplication = ::GetRegistryStringValue(HKEY_CURRENT_USER, sCurrentUserFileExtsPath, _T("Application"));

		// Priority Order from Highest to Lowest:
		//
		// ProgID, Application, Global
		//
		if (sCurrentUserProgID.CompareNoCase(sMyFileClassName) == 0)
			return TRUE;
		else if (sCurrentUserProgID != _T(""))
			return FALSE; // Another Program is associated

		if (sCurrentUserApplication.CompareNoCase(sMyFileApplicationName) == 0)
			return TRUE;
		else if (sCurrentUserApplication != _T(""))
			return FALSE; // Another Program is associated
	}

	return (sCurrentFileClassName.CompareNoCase(sMyFileClassName) == 0);	
}

BOOL CUImagerApp::AssociateFileType(CString sExt)
{
	// Program Name & Path
	TCHAR szProgName[_MAX_FNAME];
	TCHAR szProgExt[_MAX_EXT];
	TCHAR szProgPath[MAX_PATH];
	if (::GetModuleFileName(NULL, szProgPath, MAX_PATH) == 0)
		return FALSE;
	_tsplitpath(szProgPath, NULL, NULL, szProgName, szProgExt);

	// Check Extension
	if (sExt.IsEmpty())
		return TRUE;

	// Lower Case
	sExt.MakeLower();

	// Extension Point Adjust
	CString sExtNoPoint;
	if (sExt[0] != _T('.'))
	{
		sExtNoPoint = sExt;
		sExt = _T('.') + sExt;
	}
	else
		sExtNoPoint = sExt.Right(sExt.GetLength() - 1);

	// My Class Name or ProgID
	CString sMyFileClassName = CString(szProgName) + _T(".") + sExtNoPoint + _T(".1");

	// My Application Name
	CString sMyFileApplicationName = CString(szProgName) + CString(szProgExt);

	////////////////////////////////////////////////////////////////
	// Global Setting for classic systems.                        //
	// Beginning with Vista or higher (because of virtualization) //
	// the HKEY_CLASSES_ROOT is in some cases mapped to           //
	// HKEY_CURRENT_USER instead of HKEY_LOCAL_MACHINE.           //
	// For classes root operations the following set/delete       //
	// registry functions try the local machine place then if     //
	// they fail the current user location is used.               //
	////////////////////////////////////////////////////////////////
	CString sCurrentFileClassName = ::GetRegistryStringValue(HKEY_CLASSES_ROOT, sExt, _T(""));
	
	// Another Program or no Program is associated with the given extension
	if (sCurrentFileClassName.CompareNoCase(sMyFileClassName) != 0)
	{
		// Store Previous File Association in my UninstallClassNameDefault for Restore
		WriteProfileString(_T("UninstallClassNameDefault"), sExtNoPoint, sCurrentFileClassName);

		// Register File Extension
		::SetRegistryStringValue(HKEY_CLASSES_ROOT, sExt, _T(""), sMyFileClassName);

		// Write the Old Value to the Open With ProgIDs,
		// this List contains alternate Class Names
		// that can open the file extension
		if (sCurrentFileClassName != _T(""))
			::SetRegistryEmptyValue(HKEY_CLASSES_ROOT, sExt + _T("\\OpenWithProgids"), sCurrentFileClassName);
	}

	//////////////////////////////////
	// Current User Settings        //
	//                              //
	// Which have higher priorities //
	// than the Global Setting!!!   //
	//////////////////////////////////
	CString sCurrentUserFileExtsPath = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\") + sExt;

	// Does the Registry Key Exist?
	if (::IsRegistryKey(HKEY_CURRENT_USER, sCurrentUserFileExtsPath))
	{
		// ProgID has higher Priority than Application!!!

		// ProgID
		CString sCurrentUserProgID = ::GetRegistryStringValue(HKEY_CURRENT_USER, sCurrentUserFileExtsPath, _T("ProgID"));
		if (sCurrentUserProgID != _T("") && sCurrentUserProgID.CompareNoCase(sMyFileClassName) != 0)
		{
			// Store Previous File Association in my UninstallUserProgID for Restore
			WriteProfileString(_T("UninstallUserProgID"), sExtNoPoint, sCurrentUserProgID);

			// Register File Extension
			::SetRegistryStringValue(HKEY_CURRENT_USER, sCurrentUserFileExtsPath, _T("ProgID"), sMyFileClassName);

			// Write the Old Value to the Open With ProgIDs,
			// this List contains alternate Class Names
			// that can open the file extension
			if (sCurrentUserProgID != _T(""))
				::SetRegistryEmptyValue(HKEY_CURRENT_USER, sCurrentUserFileExtsPath + _T("\\OpenWithProgids"), sCurrentUserProgID);
		}

		// Application
		// (All Applications should be Registered under HKEY_CLASSES_ROOT\\Applications with name and extension like uImager.exe)
		CString sCurrentUserApplication = ::GetRegistryStringValue(HKEY_CURRENT_USER, sCurrentUserFileExtsPath, _T("Application"));
		if (sCurrentUserApplication != _T("") && sCurrentUserApplication.CompareNoCase(sMyFileApplicationName) != 0)
		{
			// Store Previous File Association in my UninstallUserApplication for Restore
			WriteProfileString(_T("UninstallUserApplication"), sExtNoPoint, sCurrentUserApplication);

			// Register File Extension
			::SetRegistryStringValue(HKEY_CURRENT_USER, sCurrentUserFileExtsPath, _T("Application"), sMyFileApplicationName);
		}

		// NOTE:
		// For Vista or higher there is also a key under sCurrentUserFileExtsPath
		// called UserChoice that can have a ProgID value: this has the highest priority
		// and can prevent from being associated. Only the OS through an user action is
		// meant to change the UserChoice, we should not touch it. Latest Windows versions
		// enforce that by using hashed values impossible to generate programmatically.
		// The current user is prevented from setting values under the UserChoice key by
		// a Deny permission entry.	
	}

	// Create My Class Name or ProgID

	// Description, Shown In Explorer
	::SetRegistryStringValue(HKEY_CLASSES_ROOT, sMyFileClassName, _T(""), CString(szProgName) + _T(" ") + sExtNoPoint + _T(" file"));
	
	// Shell Open Command
	
	// Icon order
	// Note: IDR_AUDIOMCI, IDR_CDAUDIO, IDR_VIDEOAVI, IDR_PICTURE_NOHQ, IDR_BIGPICTURE, IDR_BIGPICTURE_NOHQ
	//       and IDI_ZIP are not used anymore, but icons remain to keep the same order!
	/*
	IDR_MAINFRAME			0
	IDR_AUDIOMCI			1
	IDR_CDAUDIO				2
	IDR_VIDEOAVI			3
	IDR_PICTURE				4
	IDR_PICTURE_NOHQ		5
	IDR_BIGPICTURE			6
	IDR_BIGPICTURE_NOHQ		7
	IDI_ZIP					8
	IDR_VIDEODEVICE         9
	IDI_DEVICE              10
	IDI_MAGNIFYPLUS         11
	IDI_MAGNIFY             12
	IDI_MAGNIFYMINUS        13
	IDI_PLAY                14
	IDI_STOP                15
	IDI_PAUSE               16
	IDI_BMP                 17
	IDI_GIF                 18
	IDI_JPG                 19
	IDI_TIF                 20
	IDI_PNG                 21
	IDI_PCX                 22
	IDI_EMF					23
	*/
	if (sExtNoPoint == _T("bmp"))
		::SetRegistryStringValue(HKEY_CLASSES_ROOT, sMyFileClassName + _T("\\DefaultIcon"), _T(""), CString(szProgPath) + _T(",17"));
	else if (sExtNoPoint == _T("gif"))
		::SetRegistryStringValue(HKEY_CLASSES_ROOT, sMyFileClassName + _T("\\DefaultIcon"), _T(""), CString(szProgPath) + _T(",18"));
	else if (CDib::IsJPEGExt(sExtNoPoint))
		::SetRegistryStringValue(HKEY_CLASSES_ROOT, sMyFileClassName + _T("\\DefaultIcon"), _T(""), CString(szProgPath) + _T(",19"));
	else if (CDib::IsTIFFExt(sExtNoPoint))
		::SetRegistryStringValue(HKEY_CLASSES_ROOT, sMyFileClassName + _T("\\DefaultIcon"), _T(""), CString(szProgPath) + _T(",20"));
	else if (sExtNoPoint == _T("png"))
		::SetRegistryStringValue(HKEY_CLASSES_ROOT, sMyFileClassName + _T("\\DefaultIcon"), _T(""), CString(szProgPath) + _T(",21"));
	else if (sExtNoPoint == _T("pcx"))
		::SetRegistryStringValue(HKEY_CLASSES_ROOT, sMyFileClassName + _T("\\DefaultIcon"), _T(""), CString(szProgPath) + _T(",22"));
	else if (sExtNoPoint == _T("emf"))
		::SetRegistryStringValue(HKEY_CLASSES_ROOT, sMyFileClassName + _T("\\DefaultIcon"), _T(""), CString(szProgPath) + _T(",23"));
	else
		::SetRegistryStringValue(HKEY_CLASSES_ROOT, sMyFileClassName + _T("\\DefaultIcon"), _T(""), CString(szProgPath) + _T(",4"));

	::SetRegistryStringValue(HKEY_CLASSES_ROOT,
							sMyFileClassName +
							_T("\\shell\\open\\command"),
							_T(""),
							_T("\"") + CString(szProgPath) +
							_T("\"") + _T(" \"%1\""));
	::SetRegistryStringValue(HKEY_CLASSES_ROOT,
							sMyFileClassName +
							_T("\\shell\\edit\\command"),
							_T(""),
							_T("\"") +  CString(szProgPath)
							+ _T("\"") + _T(" \"%1\""));
	::SetRegistryStringValue(HKEY_CLASSES_ROOT,
							sMyFileClassName +
							_T("\\shell\\print\\command"),
							_T(""),
							_T("\"") +  CString(szProgPath)
							+ _T("\"") + _T(" /p") + _T(" \"%1\""));
	::SetRegistryStringValue(HKEY_CLASSES_ROOT,
							sMyFileClassName +
							_T("\\shell\\printto\\command"),
							_T(""),
							_T("\"") +  CString(szProgPath) +
							_T("\"") + _T(" /pt") + _T(" \"%1\""));
	::SetRegistryStringValue(HKEY_CLASSES_ROOT,
							sMyFileClassName +
							_T("\\shell\\preview\\command"),
							_T(""),
							_T("\"") +  CString(szProgPath) +
							_T("\"") + _T(" \"%1\""));

	return TRUE;
}

BOOL CUImagerApp::UnassociateFileType(CString sExt)
{
	// Program Name & Path
	TCHAR szProgName[_MAX_FNAME];
	TCHAR szProgExt[_MAX_EXT];
	TCHAR szProgPath[MAX_PATH];
	if (::GetModuleFileName(NULL, szProgPath, MAX_PATH) == 0)
		return FALSE;
	_tsplitpath(szProgPath, NULL, NULL, szProgName, szProgExt);

	// Check Extension
	if (sExt.IsEmpty())
		return TRUE;

	// Lower Case
	sExt.MakeLower();

	// Extension Point Adjust
	CString sExtNoPoint;
	if (sExt[0] != _T('.'))
	{
		sExtNoPoint = sExt;
		sExt = _T('.') + sExt;
	}
	else
		sExtNoPoint = sExt.Right(sExt.GetLength() - 1);

	// My Class Name or ProgID
	CString sMyFileClassName = CString(szProgName) + _T(".") + sExtNoPoint + _T(".1");

	// My Application Name
	CString sMyFileApplicationName = CString(szProgName) + CString(szProgExt);

	////////////////////
	// Global Setting //
	////////////////////
	CString sCurrentFileClassName = ::GetRegistryStringValue(HKEY_CLASSES_ROOT, sExt, _T(""));
	if (sCurrentFileClassName.CompareNoCase(sMyFileClassName) == 0)
	{
		// Delete / Restore Previous File Association for the Global Setting
		CString sPreviousFileClassName = GetProfileString(_T("UninstallClassNameDefault"), sExtNoPoint, _T(""));
		if (sPreviousFileClassName == _T(""))
			::DeleteRegistryKey(HKEY_CLASSES_ROOT, sExt);
		else
			::SetRegistryStringValue(HKEY_CLASSES_ROOT, sExt, _T(""), sPreviousFileClassName);
	}

	//////////////////////////////////
	// Current User Settings        //
	//                              //
	// Which have higher priorities //
	// than the Global Setting!!!   //
	//////////////////////////////////
	CString sCurrentUserFileExtsPath = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\") + sExt;
	if (::IsRegistryKey(HKEY_CURRENT_USER, sCurrentUserFileExtsPath))
	{
		CString sCurrentUserProgID = ::GetRegistryStringValue(HKEY_CURRENT_USER, sCurrentUserFileExtsPath, _T("ProgID"));
		if (sCurrentUserProgID.CompareNoCase(sMyFileClassName) == 0)
		{
			// Delete / Restore Previous File Association for the User Level ProgID
			CString sPreviousUserProgID = GetProfileString(_T("UninstallUserProgID"), sExtNoPoint, _T(""));
			if (sPreviousUserProgID == _T(""))
				::DeleteRegistryValue(HKEY_CURRENT_USER, sCurrentUserFileExtsPath, _T("ProgID"));
			else
				::SetRegistryStringValue(HKEY_CURRENT_USER, sCurrentUserFileExtsPath, _T("ProgID"), sPreviousUserProgID);
		}

		CString sCurrentUserApplication = ::GetRegistryStringValue(HKEY_CURRENT_USER, sCurrentUserFileExtsPath, _T("Application"));
		if (sCurrentUserApplication.CompareNoCase(sMyFileApplicationName) == 0)
		{
			// Delete / Restore Previous File Association for the User Level Application
			CString sPreviousUserApplication = GetProfileString(_T("UninstallUserApplication"), sExtNoPoint, _T(""));
			if (sPreviousUserApplication == _T(""))
				::DeleteRegistryValue(HKEY_CURRENT_USER, sCurrentUserFileExtsPath, _T("Application"));
			else
				::SetRegistryStringValue(HKEY_CURRENT_USER, sCurrentUserFileExtsPath, _T("Application"), sPreviousUserApplication);
		}
	}

	// Clean-Up
	::DeleteRegistryKey(HKEY_CLASSES_ROOT, sMyFileClassName);
	WriteProfileString(_T("UninstallClassNameDefault"), sExtNoPoint, _T(""));
	WriteProfileString(_T("UninstallUserProgID"), sExtNoPoint, _T(""));
	WriteProfileString(_T("UninstallUserApplication"), sExtNoPoint, _T(""));

	return TRUE;
}

void CUImagerApp::OnSettingsLogNormal()
{
	g_nLogLevel = 0;
	WriteProfileInt(_T("GeneralApp"), _T("LogLevel"), g_nLogLevel);
	::AfxGetMainFrame()->m_MDIClientWnd.Invalidate();
}

void CUImagerApp::OnUpdateSettingsLogNormal(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(g_nLogLevel == 0 ? 1 : 0);
}

void CUImagerApp::OnSettingsLogVerbose()
{
	g_nLogLevel = 1;
	WriteProfileInt(_T("GeneralApp"), _T("LogLevel"), g_nLogLevel);
	::AfxGetMainFrame()->m_MDIClientWnd.Invalidate();
}

void CUImagerApp::OnUpdateSettingsLogVerbose(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(g_nLogLevel == 1 ? 1 : 0);
}

void CUImagerApp::OnSettingsLogAllMessages()
{
	g_nLogLevel = 2;
	WriteProfileInt(_T("GeneralApp"), _T("LogLevel"), g_nLogLevel);
	::AfxGetMainFrame()->m_MDIClientWnd.Invalidate();
}

void CUImagerApp::OnUpdateSettingsLogAllMessages(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(g_nLogLevel == 2 ? 1 : 0);
}

void CUImagerApp::OnSettingsViewLogfile()
{
	if (::IsExistingFile(g_sLogFileName))
		::ShellExecute(NULL, _T("open"), g_sLogFileName, NULL, NULL, SW_SHOWNORMAL);
	else
		::AfxMessageBox(ML_STRING(1760, "Application Log File has not yet been created"), MB_OK | MB_ICONINFORMATION);
}

#ifdef VIDEODEVICEDOC

void CUImagerApp::EnumConfiguredDevicePathNames(CStringArray& DevicePathNames)
{
	if (m_pszRegistryKey)
	{
		const int MAX_KEY_BUFFER = 257; // http://www.sepago.de/e/holger/2010/07/20/how-long-can-a-registry-key-name-really-be
		HKEY hKey;
		if (::RegOpenKeyEx(	HKEY_CURRENT_USER,
							_T("Software\\") + CString(MYCOMPANY) + CString(_T("\\")) + CString(APPNAME_NOEXT),
							0, KEY_READ, &hKey) == ERROR_SUCCESS)
		{
			DWORD cSubKeys = 0;
			::RegQueryInfoKey(hKey, NULL, NULL, NULL, &cSubKeys, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
			TCHAR achKey[MAX_KEY_BUFFER];
			DWORD cbName;
			for (DWORD i = 0 ; i < cSubKeys; i++)
			{ 
				cbName = MAX_KEY_BUFFER;
				::RegEnumKeyEx(hKey, i, achKey, &cbName, NULL, NULL, NULL, NULL);
				CString sRecordAutoSaveDir = GetProfileString(achKey, _T("RecordAutoSaveDir"), _T(""));
				if (sRecordAutoSaveDir != _T(""))
					DevicePathNames.Add(achKey);
			}
			::RegCloseKey(hKey);
		}
	}
	else
	{
		TCHAR* pSectionNames = new TCHAR[MAX_SECTIONNAMES_BUFFER];
		memset(pSectionNames, 0, MAX_SECTIONNAMES_BUFFER * sizeof(TCHAR));
		::GetPrivateProfileSectionNames(pSectionNames, MAX_SECTIONNAMES_BUFFER, m_pszProfileName);
		TCHAR* sSource = pSectionNames;
		while (*sSource != 0) // If 0 -> end of list
		{
			CString sRecordAutoSaveDir = GetProfileString(sSource, _T("RecordAutoSaveDir"), _T(""));
			if (sRecordAutoSaveDir != _T(""))
				DevicePathNames.Add(sSource);
			while (*sSource != 0)
				sSource++;
			sSource++; // Skip the 0
		}
		delete [] pSectionNames;
	}
}

void CUImagerApp::OnEditDelete() 
{
	CDeleteCamFoldersDlg dlg;
	dlg.DoModal();
}

#endif

BOOL CUImagerApp::ShowColorDlg(	COLORREF& crColor,
								CWnd* pParentWnd/*=NULL*/)
{
	BOOL res;
	CColorDialog dlg(crColor, CC_FULLOPEN | CC_ANYCOLOR, pParentWnd);
	COLORREF clCustomColors[16];
	CString szTemp;
	for (int i = 0 ; i < 16 ; i++)
	{
		szTemp.Format(_T("BKG_CUSTOM_COLOR_%02d"), i);
		clCustomColors[i] = (COLORREF)GetProfileInt(_T("GeneralApp"), szTemp, RGB(255,255,255));
	}
	dlg.m_cc.lpCustColors = clCustomColors;

    if (dlg.DoModal() == IDOK)
	{
		res = TRUE;
        crColor = dlg.GetColor();
	}
    else
		res = FALSE;

	for (int i = 0 ; i < 16 ; i++)
	{
		szTemp.Format(_T("BKG_CUSTOM_COLOR_%02d"), i);
		WriteProfileInt(_T("GeneralApp"), szTemp, clCustomColors[i]);
	}

	return res;
}

void CUImagerApp::OnSettingsTrayicon() 
{
	m_bTrayIcon = !m_bTrayIcon;
	::AfxGetMainFrame()->TrayIcon(m_bTrayIcon);
	WriteProfileInt(_T("GeneralApp"),
					_T("TrayIcon"),
					m_bTrayIcon);
}

void CUImagerApp::OnUpdateSettingsTrayicon(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_bTrayIcon ? 1 : 0);
}

BOOL CUImagerApp::Autostart(BOOL bEnable)
{
	CRegKey RegKey;

	// Set
	if (bEnable)
	{
		// Open or Create key
		if (RegKey.Create(HKEY_CURRENT_USER, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run")) != ERROR_SUCCESS)
			return FALSE;

		// The program location
		TCHAR lpFilename[MAX_PATH];
		DWORD res = ::GetModuleFileName(
			NULL,		// handle to module to find filename for, NULL means current process' module
			lpFilename, // pointer to buffer to receive module path
			MAX_PATH    // size of buffer, in characters
			);

		// Set the value
		if (RegKey.SetStringValue(APPNAME_NOEXT, lpFilename) != ERROR_SUCCESS)
		{
			RegKey.Close();
			return FALSE;
		}
	}
	// Clear
	else
	{
		// Open key
		if (RegKey.Open(HKEY_CURRENT_USER, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run")) != ERROR_SUCCESS)
			return FALSE;

		// Set the value
		if (RegKey.DeleteValue(APPNAME_NOEXT) != ERROR_SUCCESS)
		{
			RegKey.Close();
			return FALSE;
		}
	}

	// Close key
	RegKey.Close();

	return TRUE;
}

BOOL CUImagerApp::IsAutostart()
{
	CRegKey RegKey;

	// Open key
	if (RegKey.Open(HKEY_CURRENT_USER, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run")) != ERROR_SUCCESS)
		return FALSE;

	// The program location
	TCHAR lpFilename[MAX_PATH];
	TCHAR lpRegFilename[MAX_PATH];
	DWORD res = ::GetModuleFileName(
		NULL,		// handle to module to find filename for, NULL means current process' module
		lpFilename, // pointer to buffer to receive module path
		MAX_PATH    // size of buffer, in characters
		);

	// Get the value
	ULONG nChars = MAX_PATH;
	if (RegKey.QueryStringValue(APPNAME_NOEXT, lpRegFilename, &nChars) == ERROR_SUCCESS)
	{
		lpRegFilename[MAX_PATH - 1] = _T('\0');
		CString sFilename(lpFilename);
		CString sRegFilename(lpRegFilename);
		RegKey.Close();
		if (sFilename.CompareNoCase(sRegFilename) == 0)
			return TRUE;
		else
			return FALSE;
	}
	else
	{
		RegKey.Close();
		return FALSE;
	}
}

BOOL CUImagerApp::IsExistingSection(const CString& sSection)
{
	if (m_pszRegistryKey)
	{
		return ::IsRegistryKey(	HKEY_CURRENT_USER,
								_T("Software\\") +
								CString(MYCOMPANY) + CString(_T("\\")) +
								CString(APPNAME_NOEXT) + _T("\\") +
								sSection);
	}
	else
	{
		BOOL res = FALSE;
		TCHAR* pSectionNames = new TCHAR[MAX_SECTIONNAMES_BUFFER];
		memset(pSectionNames, 0, MAX_SECTIONNAMES_BUFFER * sizeof(TCHAR));
		::GetPrivateProfileSectionNames(pSectionNames, MAX_SECTIONNAMES_BUFFER, m_pszProfileName);
		TCHAR* s = pSectionNames;
		while (*s != 0) // If 0 -> end of list
		{
			if (sSection.CompareNoCase(s) == 0)
			{
				res = TRUE;
				break;
			}
			while (*s != 0)
				s++;
			s++; // Skip the 0
		}
		delete [] pSectionNames;
		return res;
	}
}

void CUImagerApp::WriteSecureProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue)
{
	// Exportable (depends only on the MAGIC_KEY defined in CEncryptDecrypt.h)
	CEncryptDecrypt Encrypt;
	Encrypt.Encrypt(CString(lpszValue)); // note: encrypting _T("") returns _T("")
	WriteProfileString(lpszSection, lpszEntry, Encrypt.GetEncryptedValues());
}

CString CUImagerApp::GetSecureProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry)
{
	// Exportable (depends only on the MAGIC_KEY defined in CEncryptDecrypt.h)
	CString sDecrypted;
	CEncryptDecrypt Decrypt;
	Decrypt.SetEncryptedValues(GetProfileString(lpszSection, lpszEntry));
	Decrypt.Decrypt(sDecrypted); // note: decrypting _T("") returns _T("")
	return sDecrypted;
}
