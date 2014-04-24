#include "stdafx.h"
#include "uImager.h"

#include "MainFrm.h"
#include "LicenseDlg.h"
#include "CreditsDlg.h"
#include "NewDlg.h"
#include "ChildFrm.h"
#include "ToolBarChildFrm.h"
#include "uImagerDoc.h"
#include "PictureDoc.h"
#include "VideoAviDoc.h"
#include "VideoDeviceDoc.h"
#include "AudioMCIDoc.h"
#include "CDAudioDoc.h"
#include "PictureView.h"
#include "PicturePrintPreviewView.h"
#include "VideoAviView.h"
#include "VideoDeviceView.h"
#include "AudioMCIView.h"
#include "CDAudioView.h"
#include "PreviewFileDlg.h"
#include "SendMailDocsDlg.h"
#include "BatchProcDlg.h"
#include "DxCapture.h"
#include <mapi.h>
#include "ZipProgressDlg.h"
#include "BrowseDlg.h"
#include "SortableFileFind.h"
#include "getdxver.h"
#include "DirectX7Dlg.h"
#include "CPUCount.h"
#include "CPUSpeed.h"
#include "PostDelayedMessage.h"
#include "YuvToRgb.h"
#include "RgbToYuv.h"
#include "sinstance.h"
#include <atlbase.h>
#include "DiscMaster.h"
#include "DiscRecorder.h"
#include "ProgressDlg.h"
#ifdef VIDEODEVICEDOC
#include "DeleteCamFoldersDlg.h"
#include "SettingsDlgVideoDeviceDoc.h"
#include <WinSvc.h>
#include "ProgressDlg.h"
#include "HostPortDlg.h"
#else
#include "SettingsDlg.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// OpenSSL library
#ifndef CPJNSMTP_NOSSL
#ifdef _DEBUG
#pragma comment(lib, "openssl\\Build32MTd\\lib\\ssleay32.lib")
#pragma comment(lib, "openssl\\Build32MTd\\lib\\libeay32.lib")
#else
#pragma comment(lib, "openssl\\Build32MT\\lib\\ssleay32.lib")
#pragma comment(lib, "openssl\\Build32MT\\lib\\libeay32.lib")
#endif
CCriticalSection* CUImagerApp::m_pOpenSSLCritSections = NULL;
void __cdecl CUImagerApp::OpenSSLLockingCallback(int mode, int type, const char* /*file*/, int /*line*/)
{
	ASSERT(m_pOpenSSLCritSections);
	if (mode & CRYPTO_LOCK)
		m_pOpenSSLCritSections[type].Lock();
	else
		m_pOpenSSLCritSections[type].Unlock();
}
#endif

/////////////////////////////////////////////////////////////////////////////
// CUImagerApp

BEGIN_MESSAGE_MAP(CUImagerApp, CWinApp)
	//{{AFX_MSG_MAP(CUImagerApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_FILE_OPEN_DIR, OnFileOpenDir)
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	ON_COMMAND(ID_FILE_CLOSEALL, OnFileCloseall)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_COMMAND(ID_FILE_SETTINGS, OnFileSettings)
	ON_COMMAND(ID_APP_LICENSE, OnAppLicense)
	ON_COMMAND(ID_APP_CREDITS, OnAppCredits)
	ON_UPDATE_COMMAND_UI(ID_FILE_SETTINGS, OnUpdateFileSettings)
	ON_UPDATE_COMMAND_UI(ID_FILE_CLOSEALL, OnUpdateFileCloseall)
	ON_COMMAND(ID_FILE_SHRINK_DIR_DOCS, OnFileShrinkDirDocs)
	ON_COMMAND(ID_FILE_SENDMAIL_OPEN_DOCS, OnFileSendmailOpenDocs)
	ON_UPDATE_COMMAND_UI(ID_FILE_SENDMAIL_OPEN_DOCS, OnUpdateFileSendmailOpenDocs)
	ON_COMMAND(ID_TOOLS_AVIMERGE_AS, OnToolsAvimergeAs)
	ON_COMMAND(ID_TOOLS_TRAYICON, OnToolsTrayicon)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_TRAYICON, OnUpdateToolsTrayicon)
	ON_COMMAND(ID_APP_FAQ, OnAppFaq)
	ON_UPDATE_COMMAND_UI(ID_FILE_NEW, OnUpdateFileNew)
	ON_COMMAND(ID_TOOLS_VIEW_LOGFILE, OnToolsViewLogfile)
	ON_UPDATE_COMMAND_UI(ID_FILE_SHRINK_DIR_DOCS, OnUpdateFileShrinkDirDocs)
	ON_COMMAND(ID_EDIT_SCREENSHOT, OnEditScreenshot)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SCREENSHOT, OnUpdateEditScreenshot)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
	ON_COMMAND_RANGE(ID_HELP_TUTORIAL_FIRST, ID_HELP_TUTORIAL_LAST, OnHelpTutorial)
#ifdef VIDEODEVICEDOC
	ON_COMMAND(ID_TOOLS_MOVE_CAM_FOLDERS, OnToolsMoveCamFolders)
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
	m_sSysTempDir = _T("");
	m_sZipFile = _T("");
	m_sShrinkDestination = _T("");
	m_bExtractHere = FALSE;
	m_bStartPlay = FALSE;
	m_bCloseAfterAudioPlayDone = FALSE;
	m_pVideoAviDocTemplate = NULL;
	m_bForceSeparateInstance = FALSE;
#ifdef VIDEODEVICEDOC
	m_pVideoDeviceDocTemplate = NULL;
	m_bFullscreenBrowser = FALSE;
	m_bBrowserAutostart = FALSE;
	m_bIPv6 = FALSE;
	m_dwAutostartDelayMs = 0U;
	m_bUseCustomTempFolder = FALSE;
	m_bStartMicroApache = FALSE;
	m_bMicroApacheStarted = FALSE;
	m_nMicroApachePort = MICROAPACHE_DEFAULT_PORT;
	m_bMicroApacheDigestAuth = TRUE;
	m_sMicroApacheAreaname = MICROAPACHE_DEFAULT_AUTH_AREANAME;
	m_bSingleInstance = TRUE;
	m_bServiceProcess = FALSE;
	m_bDoStartFromService = FALSE;
	m_pAutorunProgressDlg = NULL;
#else
	m_bSingleInstance = FALSE;
#endif
	m_bTopMost = FALSE;
	m_pPictureDocTemplate = NULL;
	m_pAudioMCIDocTemplate = NULL;
	m_bWaitingMailFinish = FALSE;
	m_bUseLoadPreviewDib = TRUE;
	m_bFileDlgPreview = TRUE;
	m_bPlacementLoaded = FALSE;
	m_hAppMutex = NULL;
	m_bFirstRun = FALSE;
	m_bFirstRunEver = FALSE;
	m_bSilentInstall = FALSE;
	m_bMailAvailable = FALSE;
	m_bStartMaximized = FALSE;
	m_nCoresCount = 1;
	m_bVideoAviInfo = FALSE;
	m_sLastOpenedDir = _T("");
	m_nPdfScanCompressionQuality = DEFAULT_JPEGCOMPRESSION;
	m_sScanToPdfFileName = _T("");
	m_sScanToTiffFileName = _T("");
	m_bStartFullScreenMode = FALSE;
	m_bEscExit = FALSE;
	m_bDisableExtProg = FALSE;
	m_bTrayIcon = FALSE;
	m_bHideMainFrame = FALSE;
	m_bEndSession = FALSE;
	m_bSlideShowOnly = FALSE;
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

}

/////////////////////////////////////////////////////////////////////////////
// The one and only CUImagerApp object

CUImagerApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CUImagerApp initialization

CRITICAL_SECTION g_csAVCodec;
BOOL g_bAVCodecCSInited = FALSE;
int avcodec_open_thread_safe(AVCodecContext *avctx, AVCodec *codec)
{
	::EnterCriticalSection(&g_csAVCodec);
	int ret = avcodec_open(avctx, codec);
	::LeaveCriticalSection(&g_csAVCodec);
	return ret;
}
int avcodec_close_thread_safe(AVCodecContext *avctx)
{
	::EnterCriticalSection(&g_csAVCodec);
	int ret = avcodec_close(avctx);
	::LeaveCriticalSection(&g_csAVCodec);
	return ret;
}
static void my_av_log_trace(void* ptr, int level, const char* fmt, va_list vl)
{
	return;

	/*
	if(level > av_log_get_level())
		return;
	*/

	// Fix Format
	CString sFmt(fmt);
	sFmt.Replace(_T("%td"), _T("%d"));	// %td not supported by vc++
	sFmt.Replace(_T("%ti"), _T("%i"));	// %ti not supported by vc++
	
	// Convert fixed format to Ascii
	char* asciifmt = new char[sFmt.GetLength() + 1];
	if (!asciifmt)
		return;
	::wcstombs(asciifmt, (LPCTSTR)sFmt, sFmt.GetLength() + 1);
	asciifmt[sFmt.GetLength()] = '\0';
	
	// Make message string
	char s[1024];
	_vsnprintf(s, 1024, asciifmt, vl);
	delete [] asciifmt;
	s[1023] = '\0';

	// Output message string
	TRACE(CString(s));
}
static void my_av_log_empty(void* ptr, int level, const char* fmt, va_list vl)
{
	return;
}
extern "C"
{
extern int mm_support_mask;
extern int mm_flags;
extern int mm_support(void);
}
BOOL CUImagerApp::InitInstance() // Returning FALSE calls ExitInstance()!
{
#ifdef VIDEODEVICEDOC
	CProgressDlgThread* pProgressDlgThread = NULL;
#endif
	CInstanceChecker* pInstanceChecker = NULL;
	try
	{
		// A process can be started with hidden set
		if (m_nCmdShow == SW_HIDE)
			m_bHideMainFrame = TRUE;

		// InitCommonControlsEx() is required on Windows XP if an application
		// manifest specifies use of ComCtl32.dll version 6 or later to enable
		// visual styles.  Otherwise, any window creation will fail.
		INITCOMMONCONTROLSEX InitCtrls;
		InitCtrls.dwSize = sizeof(InitCtrls);
		// Set this to include all the common control classes you want to use
		// in your application.
		InitCtrls.dwICC = ICC_WIN95_CLASSES;
		InitCommonControlsEx(&InitCtrls);

		// Call base class implementation
		CWinApp::InitInstance();

		// Initialize OLE libraries
		// Note: AfxWinTerm() called after ExitInstance() will clean-up,
		// no need to do it in ExitInstance()!
		if (!AfxOleInit()) // This calls ::CoInitialize(NULL) internally
		{
			::AfxMessageBox(IDP_OLE_INIT_FAILED, MB_OK | MB_ICONSTOP);
			throw (int)0;
		}
		AfxEnableControlContainer();

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

		// Create System Temporary Directory
		TCHAR szTempPath[MAX_PATH];
		if (::GetTempPath(MAX_PATH, szTempPath))
		{
			// Do not delete temp directory to clean-up,
			// because other instances may be running!  
			m_sSysTempDir = CString(szTempPath) + sName + _T("\\");
			if (!::IsExistingDir(m_sSysTempDir))
			{
				if (!::CreateDir(m_sSysTempDir))
					::ShowLastError(TRUE);
			}
		}

		// Get AppData Folder
		CString sAppData = ::GetSpecialFolderPath(CSIDL_APPDATA);

#ifdef VIDEODEVICEDOC
		// Create our application folder
		CString sOurAppFolder = sAppData + _T("\\") +
				CString(MYCOMPANY) + CString(_T("\\")) + CString(APPNAME_NOEXT);
		if (!::IsExistingDir(sOurAppFolder))
		{
			if (!::CreateDir(sOurAppFolder))
				::ShowLastError(TRUE);
		}

		// Set default location for Document Root,
		// get a drive where we can write (usually it's C:)
		m_sMicroApacheDocRoot = ::GetDriveName(GetAppTempDir());
		m_sMicroApacheDocRoot.TrimRight(_T('\\'));
		m_sMicroApacheDocRoot += _T("\\") + CString(APPNAME_NOEXT);
#endif

		// Init Trace and Log Files Location
		// (Containing folder is created when Trace or Log Files are written)
		CString sLogFile = sAppData;
		sLogFile += _T("\\") + LOG_FILE;
		CString sTraceFile = sAppData;
		sTraceFile += _T("\\") + TRACE_FILE;
		::InitTraceLogFile(sTraceFile, sLogFile, MAX_LOG_FILE_SIZE);

		// Do not use registry if application is not installed
		// Note: on newer NT systems the ini files are not limited to 64k
		BOOL bUseRegistry = TRUE;
#ifndef _DEBUG
		CString sSoftwareCompany = CString(_T("Software\\")) + CString(MYCOMPANY) + CString(_T("\\"));
		if (::IsRegistryKey(HKEY_LOCAL_MACHINE, sSoftwareCompany + sName))
		{
			CString sInstallDir = ::GetRegistryStringValue(	HKEY_LOCAL_MACHINE,
															sSoftwareCompany + sName,
															_T("Install_Dir"));
			sInstallDir.TrimRight(_T('\\'));
			sInstallDir += _T("\\");
			if (sInstallDir.CompareNoCase(sDriveDir) != 0)
				bUseRegistry = FALSE;
		}
		else
			bUseRegistry = FALSE;
#endif

#ifndef CPJNSMTP_NOSSL
		// Standard OpenSSL initialization
		SSL_load_error_strings();
		SSL_library_init(); // it's normal that OpenSSL leaks 16 + 20 bytes +
							// sometimes more but not increasing with the email sends

		// Setup SSL to work correctly in a multithreaded environment
		ASSERT(m_pOpenSSLCritSections == NULL);
		m_pOpenSSLCritSections = new CCriticalSection[CRYPTO_num_locks()];
		if (m_pOpenSSLCritSections == NULL)
		{
			// Report the error
			::AfxMessageBox(_T("Failed to create SSL critical sections required for OpenSSL"), MB_OK | MB_ICONSTOP);
			throw (int)0;
		}
		CRYPTO_set_locking_callback(OpenSSLLockingCallback);
#endif

		// Parse command line for standard shell commands, DDE, file open
		CUImagerCommandLineInfo cmdInfo;
		ParseCommandLine(cmdInfo); // m_bHideMainFrame is ev. set to TRUE here

		// Slideshow only mode?
		CString sSlideshowName(SLIDESHOWNAME);
		int pos = sSlideshowName.Find(_T('.'));
		if (pos >= 0)
			sSlideshowName = sSlideshowName.Left(pos);
		if (cmdInfo.DoStartSlideShow() || sName.CompareNoCase(sSlideshowName) == 0)
		{
			m_bSlideShowOnly = TRUE;
			bUseRegistry = FALSE;
		}

		// Registry key under which the settings are stored.
		// This Will create the HKEY_CURRENT_USER\Software\Contaware key
		if (bUseRegistry)
			SetRegistryKey(MYCOMPANY);
		else
		{
			// First free the string allocated by MFC at CWinApp startup.
			// The string is allocated before InitInstance is called.
			free((void*)m_pszProfileName);

			// Change the name of the .INI file.
			// The CWinApp destructor will free the memory.
			m_pszProfileName = _tcsdup(sDriveDir + sName + _T(".ini"));

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

		// Separate Instance Necessary?
		m_bForceSeparateInstance =
			m_bExtractHere												||
			m_bStartPlay												||
			m_bCloseAfterAudioPlayDone									||
			cmdInfo.m_bRunEmbedded										||
			cmdInfo.m_bRunAutomated										||
			cmdInfo.m_nShellCommand == CCommandLineInfo::FilePrintTo	||
			cmdInfo.m_nShellCommand == CCommandLineInfo::FileDDE		||
			cmdInfo.m_nShellCommand == CCommandLineInfo::AppRegister	||
			cmdInfo.m_nShellCommand == CCommandLineInfo::AppUnregister	||
			m_bSlideShowOnly;

		// Single Instance
		// (if VIDEODEVICEDOC defined -> single instance is always set, see constructor)
#ifndef VIDEODEVICEDOC
		m_bSingleInstance = (BOOL)GetProfileInt(_T("GeneralApp"), _T("SingleInstance"), FALSE);
#endif
		if (!m_bForceSeparateInstance	&&
#ifdef VIDEODEVICEDOC			
			!m_bServiceProcess			&&
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

		// Set Tray Icon and Start FullScreen Mode flags
		if (!m_bHideMainFrame)
		{
#ifdef VIDEODEVICEDOC
			if (m_bFirstRunEver)
				WriteProfileInt(_T("GeneralApp"), _T("TrayIcon"), TRUE);
#endif
			m_bTrayIcon = (BOOL)GetProfileInt(_T("GeneralApp"), _T("TrayIcon"), FALSE);
			m_bStartFullScreenMode = (BOOL)GetProfileInt(_T("GeneralApp"), _T("StartFullScreenMode"), FALSE);
		}

		// Init Global Helper Functions
		// (inits OSs flags and processor instruction sets flags)
		::InitHelpers();

		// Enable the low-fragmenation heap (LFH) for XP and Windows 2003
		// (available for release build only)
#ifndef _DEBUG
		if (!g_bWinVistaOrHigher)
			::EnableLFHeap();
#endif

		// Init for the PostDelayedMessage() Function
		CPostDelayedMessageThread::Init();

		// Get the CPU Count
		unsigned int TotAvailLogical	= 0, // Number of available logical CPU in the system
					 TotAvailCore		= 0, // Number of available cores in the system
					 PhysicalNum		= 0; // Total number of physical processors in the system
		::CPUCount(&TotAvailLogical, &TotAvailCore, &PhysicalNum);
		m_nCoresCount = TotAvailCore;

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

		// Video Avi Doc Template Registration
		m_pVideoAviDocTemplate = new CUImagerMultiDocTemplate(
			IDR_VIDEOAVI,
			RUNTIME_CLASS(CVideoAviDoc),
			RUNTIME_CLASS(CVideoAviChildFrame),
			RUNTIME_CLASS(CVideoAviView));
		if (!m_pVideoAviDocTemplate)
			throw (int)0;
		AddDocTemplate(m_pVideoAviDocTemplate);

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

		// Audio MCI Doc Template Registration
		m_pAudioMCIDocTemplate = new CUImagerMultiDocTemplate(
			IDR_AUDIOMCI,
			RUNTIME_CLASS(CAudioMCIDoc),
			RUNTIME_CLASS(CAudioMCIChildFrame),
			RUNTIME_CLASS(CAudioMCIView));
		if (!m_pAudioMCIDocTemplate)
			throw (int)0;
		AddDocTemplate(m_pAudioMCIDocTemplate);

		// CD Audio Doc Template Registration
		m_pCDAudioDocTemplate = new CUImagerMultiDocTemplate(
			IDR_CDAUDIO,
			RUNTIME_CLASS(CCDAudioDoc),
			RUNTIME_CLASS(CCDAudioChildFrame),
			RUNTIME_CLASS(CCDAudioView));
		if (!m_pCDAudioDocTemplate)
			throw (int)0;
		AddDocTemplate(m_pCDAudioDocTemplate);

		// Create Named Mutex For Installer / Uninstaller
		m_hAppMutex = ::CreateMutex(NULL, FALSE, APPMUTEXNAME);

		// Is Mail Available?
		m_bMailAvailable =	(::GetProfileInt(_T("MAIL"), _T("MAPI"), 0) != 0) &&
							(SearchPath(NULL, _T("MAPI32.DLL"), NULL, 0, NULL, NULL) != 0);

		// Check for MMX
		if (!g_bMMX)
		{
			::AfxMessageBox(ML_STRING(1170, "Error: No MMX Processor"), MB_OK | MB_ICONSTOP);
			throw (int)0;
		}

		// DirectX check (it's a bit slow, do not run that each time)
		if (m_bFirstRun)
		{	
#ifdef VIDEODEVICEDOC
			RequireDirectXVersion7();
#else
			SuggestDirectXVersion7();
#endif
		}

		// Zip Settings
		m_Zip.SetAdvanced(65535 * 50, 16384 * 50, 32768 * 50);

		// Init YUV <-> RGB LUT
		::InitYUVToRGBTable();
		::InitRGBToYUVTable();

		// AVCODEC Init
		::InitializeCriticalSection(&g_csAVCodec);
		g_bAVCodecCSInited = TRUE;
#ifdef _DEBUG
		av_log_set_callback(my_av_log_trace);
#else
		av_log_set_callback(my_av_log_empty);
#endif
		/*	ffmpeg automatically detects the best instructions
			for the given CPU. Set mm_support_mask to limit the
			used instructions.
		FF_MM_MMX		// standard MMX
		FF_MM_3DNOW		// AMD 3DNOW
		FF_MM_MMXEXT	// SSE integer functions or AMD MMX ext
		FF_MM_SSE		// SSE functions
		FF_MM_SSE2		// PIV SSE2 functions
		FF_MM_3DNOWEXT	// AMD 3DNowExt
		FF_MM_SSE3		// Prescott SSE3 functions
		FF_MM_SSSE3		// Conroe SSSE3 functions
		*/
		// On newer systems I had some strange crashes ... better to always disable sse2 and higher!
		mm_support_mask = FF_MM_MMX | FF_MM_3DNOW | FF_MM_MMXEXT | FF_MM_SSE;
		av_register_all();
		// Initializing mm_flags is necessary when using deinterlacing,
		// otherwise the emms_c() macro in avpicture_deinterlace() of the imgconvert.c file
		// may be empty and the emms instruction is not called after using MMX!
		// Note: all codecs except the raw one call dsputil_init() which executes mm_flags = mm_support()
		mm_flags = mm_support();
		
#ifdef VIDEODEVICEDOC
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

		// Stop from Service flag
		BOOL bStopFromService = !m_bForceSeparateInstance	&&
								!m_bServiceProcess			&&
								GetContaCamServiceState() == CONTACAMSERVICE_RUNNING;

		// Stop from Service Progress Dialog
		if (bStopFromService && (!m_bTrayIcon || m_bFirstRun || m_bStartFullScreenMode))	// if m_bFirstRun or m_bStartFullScreenMode set
		{																					// we will not minimize to tray in CMainFrame::OnCreate()
			CString sStartingApp;
			sStartingApp.Format(ML_STRING(1764, "Starting %s..."), APPNAME_NOEXT);
			pProgressDlgThread = new CProgressDlgThread(sStartingApp, 0, CONTACAMSERVICE_TIMEOUT);
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

		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// ! Do not throw after this point, debugger asserts if returning FALSE !!
		// ! from this function when the MainFrame has already been created     !! 
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

		// If this is the first instance of our App then track it
		// so any other instances can find us
		if (pInstanceChecker)
		{
			pInstanceChecker->TrackFirstInstanceRunning();
			delete pInstanceChecker;
			pInstanceChecker = NULL;
		}

#ifdef VIDEODEVICEDOC
		// Do stop from Service, this may take some time...
		if (bStopFromService)
		{
			if (ControlContaCamService(CONTACAMSERVICE_CONTROL_END_PROC) == ERROR_SUCCESS)
				m_bDoStartFromService = TRUE;
		}

		// The mainframe must be created before you delete pProgressDlgThread
		// this to allow the AttachThreadInput to correctly pass the focus!
		if (pProgressDlgThread)
		{	
			delete pProgressDlgThread;
			pProgressDlgThread = NULL;
		}
#endif

		// Dispatch commands specified on the command line.
		// Returns FALSE if extracting zip file here,
		// if printing, if file opening fails.
		if (!ProcessShellCommand(cmdInfo))
		{
			m_pMainWnd->PostMessage(WM_CLOSE);
			return TRUE;
		}

		// Hiding mainframe?
		if (m_bHideMainFrame)
			m_nCmdShow = SW_HIDE;

		// The main window has been initialized, so show, place it and update it.
		pMainFrame->ShowWindow(m_nCmdShow);
		pMainFrame->UpdateWindow();

		// ProcessShellCommand() is called before the MainFrame is shown,
		// this to display the zip extraction dialog without MainFrame.
		// But that causes a problem with the document titles,
		// they are not shown!
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

		// Slideshow only mode
		if (m_bSlideShowOnly)
		{
			SlideShow(	sDriveDir,
						TRUE,
						TRUE,
						TRUE);
		}
		else
		{
#ifdef VIDEODEVICEDOC
			m_sFullscreenBrowserExitString = GetProfileFullscreenBrowser(	FULLSCREENBROWSER_EXITSTRING_ENTRY,
																			FULLSCREENBROWSER_DEFAULT_EXITSTRING);
#endif
			// Load Settings has to be here for the Window Placement restore to work!
			LoadSettings(m_nCmdShow);

			// First time that the App runs after Install (or Upgrade)
			if (m_bFirstRun)
			{
				// First time ever that the App runs or after a uninstall
				if (m_bFirstRunEver)
				{
#ifdef VIDEODEVICEDOC
					// Try to set the microapache server to MICROAPACHE_PREFERRED_PORT
					if (!CVideoDeviceDoc::MicroApacheIsPortUsed(MICROAPACHE_PREFERRED_PORT))
					{
						m_nMicroApachePort = MICROAPACHE_PREFERRED_PORT;
						WriteProfileInt(_T("GeneralApp"), _T("MicroApachePort"), m_nMicroApachePort);
					}

					// Enable autostart
					Autostart(TRUE);
#endif
					// Reset first run ever flag
					WriteProfileInt(_T("GeneralApp"), _T("FirstRunEver"), FALSE);
				}
		
				// Update file associations (necessary when changing the icons)
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
			// Redraw web server port
			::AfxGetMainFrame()->m_MDIClientWnd.Invalidate();

			// Auto-starts
			if (!m_bForceSeparateInstance)
			{
				// Update / create doc root index.php and config file for microapache
				CVideoDeviceDoc::MicroApacheUpdateMainFiles();

				// Start Micro Apache
				if (m_bStartMicroApache)
				{
					if (CVideoDeviceDoc::MicroApacheInitStart() && CVideoDeviceDoc::MicroApacheWaitStartDone())
					{
						m_bMicroApacheStarted = TRUE;
						m_MicroApacheWatchdogThread.Start(THREAD_PRIORITY_BELOW_NORMAL);
					}
					else
					{
						TRACE(ML_STRING(1475, "Failed to start the web server") + _T("\n"));
						::LogLine(ML_STRING(1475, "Failed to start the web server"));
					}
				}

				// Autorun Devices
				AutorunVideoDevices();

				// Start Browser
				if (m_bBrowserAutostart && !m_bServiceProcess)
				{
					BOOL bDoStartBrowser = TRUE;
					if (m_bStartMicroApache)
					{
						bDoStartBrowser =	m_bMicroApacheStarted &&
											CVideoDeviceDoc::MicroApacheWaitCanConnect();
					}
					if (bDoStartBrowser)
					{
						CString sUrl, sPort;
						sPort.Format(_T("%d"), m_nMicroApachePort);
						if (sPort != _T("80"))
							sUrl = _T("http://localhost:") + sPort + _T("/");
						else
							sUrl = _T("http://localhost/");
						if (m_bFullscreenBrowser)
						{
							CString sFullscreenExe = sDriveDir + CString(FULLSCREENBROWSER_EXE_NAME_EXT);
							::ShellExecute(	NULL,
											_T("open"),
											sFullscreenExe,
											sUrl,
											NULL,
											SW_SHOWNORMAL);
						}
						else
						{
							::ShellExecute(	NULL,
											_T("open"),
											sUrl,
											NULL,
											NULL,
											SW_SHOWNORMAL);
						}
					}
				}
			}
#endif

			// When starting program open document in full screen mode
			if (m_bStartFullScreenMode
#ifdef VIDEODEVICEDOC
				&& !m_bServiceProcess
#endif
				)
				::AfxGetMainFrame()->EnterExitFullscreen();
		}

		return TRUE;
	}
	catch (int)
	{
#ifdef VIDEODEVICEDOC
		if (pProgressDlgThread)
			delete pProgressDlgThread;
#endif
		if (pInstanceChecker)
			delete pInstanceChecker;
		return FALSE;
	}
	catch (CException* e)
	{
#ifdef VIDEODEVICEDOC
		if (pProgressDlgThread)
			delete pProgressDlgThread;
#endif
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

CAboutDlg::CAboutDlg(BOOL bClickableLinks/*=TRUE*/) : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
	m_bClickableLinks = bClickableLinks;
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

	// CPU Speed
	CEdit* pCpuSpeedEdit = (CEdit*)GetDlgItem(IDC_CPUSPEED);
	CString sCpuSpeedMHz;
	sCpuSpeedMHz.Format(_T("CPU  %u MHz"), ::GetProcessorSpeedMHzFast());
	pCpuSpeedEdit->SetWindowText(sCpuSpeedMHz);

	// CPU Count
	unsigned int TotAvailLogical	= 0, // Number of available logical CPU in the system
				 TotAvailCore		= 0, // Number of available cores in the system
				 PhysicalNum		= 0; // Total number of physical processors in the system
	::CPUCount(&TotAvailLogical, &TotAvailCore, &PhysicalNum);
	CEdit* pCpuCount = (CEdit*)GetDlgItem(IDC_CPUCOUNT);
	CString sCpuCount;
	sCpuCount.Format(	ML_STRING(1173, "%u phys. processor(s), %u core(s), %u logical processor(s)"),
						PhysicalNum, TotAvailCore, TotAvailLogical);
	pCpuCount->SetWindowText(sCpuCount);

	// Total Physical Memory
	CString sPhysMemMB;
	if (g_nInstalledPhysRamMB >= (g_nAvailablePhysRamMB + 16)) // allow some margin
		sPhysMemMB.Format(_T("RAM  %d ") + ML_STRING(1825, "MB") + _T(" (") + ML_STRING(1820, "usable") + _T(" %d ") + ML_STRING(1825, "MB") + _T(")"), g_nInstalledPhysRamMB, g_nAvailablePhysRamMB);
	else
		sPhysMemMB.Format(_T("RAM  %d ") + ML_STRING(1825, "MB"), g_nInstalledPhysRamMB);
	CEdit* pPhysMemMB = (CEdit*)GetDlgItem(IDC_PHYSMEM);
	pPhysMemMB->SetWindowText(sPhysMemMB);

	// Memory Stats
	DisplayMemStats();

	// Clickable Links?
	if (m_bClickableLinks)
		m_WebLink.SubclassDlgItem(IDC_WEB_LINK, this);

	// Compilation Time & Date
	CString sCompilationTime;
	sCompilationTime =	CString(_T("(")) +
						CString(_T(__TIME__)) +
						CString(_T("  ")) +
						CString(_T(__DATE__)) +
						CString(_T(")"));
	SetDlgItemText(IDC_VERSION, sCompilationTime);
	
	// Set Timer
	SetTimer(ID_TIMER_ABOUTDLG, ABOUTDLG_TIMER_MS, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAboutDlg::DisplayMemStats()
{
	// Get virtual memory stats
	int nReservedMB = 0;
	int nCommittedMB = 0;
	double dFragmentation = 0.0;
	::GetMemoryStats(NULL, NULL, &nReservedMB, &nCommittedMB, &dFragmentation);
	CString sReservedSize;
	sReservedSize.Format(_T("%d ") + ML_STRING(1825, "MB"), nReservedMB);
	CString sCommittedSize;
	sCommittedSize.Format(_T("%d ") + ML_STRING(1825, "MB"), nCommittedMB);
	CString sFragmentation;
	sFragmentation.Format(_T("%0.1f %%"), dFragmentation);

	// Update text on dialog if necessary
	CString sCurrentText, sNewTextStats;
	sNewTextStats.Format(ML_STRING(1821, "used %s, reserved %s, fragmented %s"),
						sCommittedSize, sReservedSize, sFragmentation);
	CEdit* pMemStats = (CEdit*)GetDlgItem(IDC_MEMSTATS);
	if (pMemStats)
	{
		pMemStats->GetWindowText(sCurrentText);
		if (sNewTextStats != sCurrentText)
			pMemStats->SetWindowText(sNewTextStats);
	}
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CAboutDlg::OnTimer(UINT_PTR nIDEvent)
{
	DisplayMemStats();
	CDialog::OnTimer(nIDEvent);
}

void CAboutDlg::OnDestroy()
{
	// Kill timer
	KillTimer(ID_TIMER_ABOUTDLG);

	// Base class
	CDialog::OnDestroy();
}

// App command to run the dialog
void CUImagerApp::OnAppAbout()
{	
	// Clickable Links only in Normal Screen Mode!
	CAboutDlg aboutDlg(!::AfxGetMainFrame()->m_bFullScreenMode);
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
	CLicenseDlg licenseDlg;
	licenseDlg.DoModal();
}

// Show the internet site about FAQs
void CUImagerApp::OnAppFaq() 
{
	::ShellExecute(	NULL,
					_T("open"),
					FAQ_ONLINE_PAGE,
					NULL, NULL, SW_SHOWNORMAL);
}

// Popup Settings Dialog
void CUImagerApp::OnFileSettings() 
{
#ifdef VIDEODEVICEDOC
	CSettingsDlgVideoDeviceDoc dlg;
#else
	CSettingsDlg dlg;
#endif
	dlg.DoModal();
}

void CUImagerApp::OnUpdateFileSettings(CCmdUI* pCmdUI) 
{
	// Enable if we are the main UI instance
#ifdef VIDEODEVICEDOC
	pCmdUI->Enable(!m_bForceSeparateInstance && !m_bServiceProcess);
#endif
}

void CUImagerApp::OnFileOpen()
{
	if (!::AfxGetMainFrame()->m_bFullScreenMode &&
		!m_bSlideShowOnly)
	{
		TCHAR* FileNames = new TCHAR[MAX_FILEDLG_PATH];
		TCHAR* InitDir = new TCHAR[MAX_FILEDLG_PATH];
		FileNames[0] = _T('\0');
		InitDir[0] = _T('\0');
		m_sLastOpenedDir.TrimRight(_T('\\'));
		if (::IsExistingDir(m_sLastOpenedDir))
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
					_T("Supported Files (*.bmp;*.gif;*.jpg;*.avi;*.tif;*.png;*.pcx;*.emf;*.mp3;*.wav;*.cda;*.wma;*.mid;*.au;*.aif;*.zip)\0")
					_T("*.bmp;*.dib;*.gif;*.png;*.jpg;*.jpeg;*.jpe;*.thm;*.tif;*.tiff;*.jfx;*.pcx;*.emf;*.avi;*.divx;")
					_T("*.mp3;*.wav;*.cda;*.wma;*.mid;*.rmi;*.au;*.aif;*.aiff;*.zip\0")
					_T("All Files (*.*)\0*.*\0")
					_T("Windows Bitmap (*.bmp;*.dib)\0*.bmp;*.dib\0")
					_T("Graphics Interchange Format (*.gif)\0*.gif\0")
					_T("Portable Network Graphics (*.png)\0*.png\0")
					_T("JPEG File Interchange Format (*.jpg;*.jpeg;*.jpe;*.thm)\0*.jpg;*.jpeg;*.jpe;*.thm\0")
					_T("Tag Image File Format (*.tif;*.tiff;*.jfx)\0*.tif;*.tiff;*.jfx\0")
					_T("PC Paintbrush (*.pcx)\0*.pcx\0")
					_T("Enhanced Metafile (*.emf)\0*.emf\0")
					_T("Audio Video Interchange (*.avi;*.divx)\0*.avi;*.divx\0")
					_T("Audio Files (*.mp3;*.wav;*.cda;*.wma;*.mid;*.au;*.aif)\0")
					_T("*.mp3;*.wav;*.cda;*.wma;*.mid;*.rmi;*.au;*.aif;*.aiff\0")
					_T("Zip File (*.zip)\0*.zip\0");
		dlgFile.m_ofn.Flags |= OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT;
		dlgFile.m_ofn.lpstrFile = FileNames;
		dlgFile.m_ofn.nMaxFile = MAX_FILEDLG_PATH;

		// Open File Dialog
		if (dlgFile.DoModal() == IDOK)
		{
			// In case of zip file(s) this is the
			// selected extraction directory
			CString sZipExtractDir(_T(""));
			volatile int nPictureFilesInZipsCount = 0;

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
				// Zip File Extraction
				if (::GetFileExt(Path) == _T(".zip"))
				{
					sZipExtractDir = ExtractZip(Path, &nPictureFilesInZipsCount);
				}
				else
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
						if (pDoc->IsKindOf(RUNTIME_CLASS(CVideoAviDoc)))
						{
							if (!((CVideoAviDoc*)pDoc)->LoadAVI(Path))
							{
								((CVideoAviDoc*)pDoc)->CloseDocumentForce();
								delete [] FileNames;
								delete [] InitDir;
								return;
							}
						}
						else if (pDoc->IsKindOf(RUNTIME_CLASS(CPictureDoc)))
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
								((CPictureDoc*)pDoc)->SlideShow(FALSE, FALSE); // No Recursive Slideshow in Paused State
							}
						}
						else if (pDoc->IsKindOf(RUNTIME_CLASS(CAudioMCIDoc)))
						{
							if (!((CAudioMCIDoc*)pDoc)->LoadAudio(Path))
							{
								((CAudioMCIDoc*)pDoc)->CloseDocumentForce();
								delete [] FileNames;
								delete [] InitDir;
								return;
							}
						}
						else if (pDoc->IsKindOf(RUNTIME_CLASS(CCDAudioDoc)))
						{
							if (!((CCDAudioDoc*)pDoc)->LoadCD(Path))
							{
								((CCDAudioDoc*)pDoc)->CloseDocumentForce();
								delete [] FileNames;
								delete [] InitDir;
								return;
							}
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
				// Just open one cd player!
				BOOL bCDAudioOpened = FALSE;

				while (*sSource != 0) // If 0 -> end of file list.
				{
					_tcscpy(FileName, (LPCTSTR)Path);
					_tcscat(FileName, (LPCTSTR)_T("\\"));
					_tcscat(FileName, (LPCTSTR)sSource);

					// Zip File Extraction
					if (::GetFileExt(FileName) == _T(".zip"))
					{
						if (sZipExtractDir == _T(""))
							sZipExtractDir = ExtractZip(FileName, &nPictureFilesInZipsCount);
						else
							ExtractZipToDir(sZipExtractDir, FileName, &nPictureFilesInZipsCount);
					}
					else
					{
						// Just open one cd player
						if (!bCDAudioOpened || ::GetFileExt(FileName) != _T(".cda"))
						{
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
								if (pDoc->IsKindOf(RUNTIME_CLASS(CVideoAviDoc)))
								{
									if (!((CVideoAviDoc*)pDoc)->LoadAVI(FileName))
									{
										((CVideoAviDoc*)pDoc)->CloseDocumentForce();
										delete [] FileNames;
										delete [] InitDir;
										return;
									}
								}
								else if (pDoc->IsKindOf(RUNTIME_CLASS(CPictureDoc)))
								{
									CZoomComboBox* pZoomCB = &(((CPictureToolBar*)((CToolBarChildFrame*)(((CPictureDoc*)pDoc)->GetFrame()))->GetToolBar())->m_ZoomComboBox);
									pZoomCB->SetCurSel(((CPictureDoc*)pDoc)->m_nZoomComboBoxIndex);
									pZoomCB->OnChangeZoomFactor(*((double*)(pZoomCB->GetItemDataPtr(((CPictureDoc*)pDoc)->m_nZoomComboBoxIndex))));

									if (!((CPictureDoc*)pDoc)->LoadPicture(	&((CPictureDoc*)pDoc)->m_pDib,
																			FileName,
																			FALSE,
																			FALSE	// Do not preload Prev & Next because LoadDibSectionEx
																					// of CDib has a OLE object problem when loading many
																					// files at the same time!
																			))
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
										((CPictureDoc*)pDoc)->SlideShow(FALSE, FALSE); // No Recursive Slideshow in Paused State
									}
								}
								else if (pDoc->IsKindOf(RUNTIME_CLASS(CAudioMCIDoc)))
								{
									if (!((CAudioMCIDoc*)pDoc)->LoadAudio(FileName))
									{
										((CAudioMCIDoc*)pDoc)->CloseDocumentForce();
										delete [] FileNames;
										delete [] InitDir;
										return;
									}
								}
								else if (pDoc->IsKindOf(RUNTIME_CLASS(CCDAudioDoc)))
								{
									if (!((CCDAudioDoc*)pDoc)->LoadCD(FileName))
									{
										((CCDAudioDoc*)pDoc)->CloseDocumentForce();
										delete [] FileNames;
										delete [] InitDir;
										return;
									}
									else
										bCDAudioOpened = TRUE;
								}
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

			// Open Zip File Content
			if (sZipExtractDir != _T("") && nPictureFilesInZipsCount > 0)
			{
				SlideShow(	sZipExtractDir,
							FALSE,
							FALSE,
							TRUE);
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
	if (::IsJPEGExt(sExt))
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
	else if (::IsTIFFExt(sExt))
		return Dib.SaveTIFF(lpszFileName, COMPRESSION_LZW);
	else if (sExt == _T(".gif"))
		return CPictureDoc::SaveGIF(lpszFileName, &Dib);
	else if (sExt == _T(".bmp") || sExt == _T(".dib"))
	{
		if (Dib.HasAlpha() && Dib.GetBitCount() == 32)
			Dib.BMIToBITMAPV4HEADER();
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

	// Store Last Opened Directory
	if (::IsExistingDir(lpszFileName))
		m_sLastOpenedDir = CString(lpszFileName);
	else
	{
		TCHAR szDrive[_MAX_DRIVE];
		TCHAR szDir[_MAX_DIR];
		_tsplitpath(lpszFileName, szDrive, szDir, NULL, NULL);
		m_sLastOpenedDir = CString(szDrive) + CString(szDir);
	}
	m_sLastOpenedDir.TrimRight(_T('\\'));
	WriteProfileString(	_T("GeneralApp"),
						_T("LastOpenedDir"),
						m_sLastOpenedDir);

	// Zip File Extraction
	if (::GetFileExt(lpszFileName) == _T(".zip"))
	{
		volatile int nPictureFilesInZipCount = 0;
		CString sZipExtractDir = ExtractZip(lpszFileName, &nPictureFilesInZipCount);

		if (m_bExtractHere)
			return NULL;
		else
		{
			// Open Zip File Content
			if (sZipExtractDir != _T("") && nPictureFilesInZipCount > 0)
			{
				// Maximize from Tray
				if (m_bTrayIcon &&
					::AfxGetMainFrame()->m_TrayIcon.IsMinimizedToTray())
				{
					::AfxGetMainFrame()->m_TrayIcon.MaximizeFromTray();
					::AfxGetMainFrame()->ShowOwnedWindows(TRUE);
					PaintDocTitles();
				}

				return SlideShow(	sZipExtractDir,
									FALSE,
									FALSE,
									TRUE);
			}
			else
				return NULL;
		}
	}
	else
	{
		// Maximize from Tray
		if (m_bTrayIcon &&
			::AfxGetMainFrame()->m_TrayIcon.IsMinimizedToTray())
		{
			::AfxGetMainFrame()->m_TrayIcon.MaximizeFromTray();
			::AfxGetMainFrame()->ShowOwnedWindows(TRUE);
			PaintDocTitles();
		}

		CUImagerMultiDocTemplate* curTemplate = GetTemplateFromFileExtension(lpszFileName);
		if (curTemplate == NULL)
		{
			// A Dir may have been dropped -> Start Recursive Slideshow
			if (::IsExistingDir(lpszFileName))
			{
				return SlideShow(	lpszFileName,
									FALSE,
									FALSE,
									TRUE);
			}
			else
			{
				FileTypeNotSupportedMessageBox(lpszFileName);
				return NULL;
			}
		}

		// If the Path is Already Full, GetFullPathName will not change it.
		TCHAR szFullPathName[MAX_PATH];
		LPTSTR lpFilePart;
		::GetFullPathName(lpszFileName, MAX_PATH, szFullPathName, &lpFilePart);

		// Open Doc
		CDocument* pDoc = curTemplate->OpenDocumentFile(NULL);
		if (pDoc)
		{
			if (pDoc->IsKindOf(RUNTIME_CLASS(CVideoAviDoc)))
			{
				if (m_bStartMaximized)
					((CVideoAviDoc*)pDoc)->GetFrame()->MDIMaximize();
				if (!((CVideoAviDoc*)pDoc)->LoadAVI(szFullPathName))
				{
					((CVideoAviDoc*)pDoc)->CloseDocumentForce();
					return NULL;
				}
			}
			else if (pDoc->IsKindOf(RUNTIME_CLASS(CPictureDoc)))
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
					((CPictureDoc*)pDoc)->SlideShow(FALSE, FALSE); // No Recursive Slideshow in Paused State
				}
			}
			else if (pDoc->IsKindOf(RUNTIME_CLASS(CAudioMCIDoc)))
			{
				if (!((CAudioMCIDoc*)pDoc)->LoadAudio(szFullPathName))
				{
					((CAudioMCIDoc*)pDoc)->CloseDocumentForce();
					return NULL;
				}
			}
			else if (pDoc->IsKindOf(RUNTIME_CLASS(CCDAudioDoc)))
			{
				if (!((CCDAudioDoc*)pDoc)->LoadCD(szFullPathName))
				{
					((CCDAudioDoc*)pDoc)->CloseDocumentForce();
					return NULL;
				}
			}
		}

		return pDoc;
	}
}

#ifdef VIDEODEVICEDOC

void CUImagerApp::OnCaptureNetwork() 
{
	CHostPortDlg dlg;
	if (dlg.DoModal() == IDOK)
	{
		CVideoDeviceDoc* pDoc = (CVideoDeviceDoc*)GetVideoDeviceDocTemplate()->OpenDocumentFile(NULL);
		if (pDoc)
		{
			if (!pDoc->OpenGetVideo(&dlg))
				pDoc->CloseDocument();
		}
	}
}

void CUImagerApp::OnFileDxVideoDevice(UINT nID)
{
	CVideoDeviceDoc* pDoc = (CVideoDeviceDoc*)GetVideoDeviceDocTemplate()->OpenDocumentFile(NULL);
	if (pDoc)
	{
		if (!pDoc->OpenVideoDevice(nID - ID_DIRECTSHOW_VIDEODEV_FIRST))
			pDoc->CloseDocument();
	}
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

void CUImagerApp::OnUpdateFileShrinkDirDocs(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(::AfxGetMainFrame()->m_pBatchProcDlg ? 1 : 0);
}

void CUImagerApp::OnFileSendmailOpenDocs() 
{
	// Check and prompt to open files if none open
	if (!m_bMailAvailable)
	{
		::AfxMessageBox(ML_STRING(1175, "No Email Program Installed."), MB_OK | MB_ICONINFORMATION);
		return;
	}
	else if (!ArePictureDocsOpen() && !AreVideoAviDocsOpen())
		OnFileOpen();

	// Send open doc(s)
	if (ArePictureDocsOpen() || AreVideoAviDocsOpen())
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
		CUImagerMultiDocTemplate* pVideoAviDocTemplate = GetVideoAviDocTemplate();
		POSITION posVideoAviDoc = pVideoAviDocTemplate->GetFirstDocPosition();
		while (posVideoAviDoc)
		{
			CVideoAviDoc* pVideoAviDoc = (CVideoAviDoc*)(pVideoAviDocTemplate->GetNextDoc(posVideoAviDoc));
			if (!IsDocAvailable(pVideoAviDoc, TRUE))
				return;
		}

		// Send init
		SendOpenDocsAsMailInit();
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

int CUImagerApp::GetOpenDocsCount()
{
	CDocument* pDoc;
	CUImagerMultiDocTemplate* curTemplate;
	POSITION posTemplate, posDoc;
	int nCount = 0;

	posTemplate = GetFirstDocTemplatePosition();
	while (posTemplate)
	{
		curTemplate = (CUImagerMultiDocTemplate*)GetNextDocTemplate(posTemplate);
		posDoc = curTemplate->GetFirstDocPosition();
		while (posDoc)
		{
			pDoc = curTemplate->GetNextDoc(posDoc);
			if (pDoc)
				++nCount;
		}
	}

	return nCount;
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

BOOL CUImagerApp::AreVideoAviDocsOpen()
{
	CUImagerMultiDocTemplate* pVideoAviDocTemplate = GetVideoAviDocTemplate();
	POSITION posVideoAviDoc = pVideoAviDocTemplate->GetFirstDocPosition();
	CVideoAviDoc* pVideoAviDoc;	
	while (posVideoAviDoc)
	{
		pVideoAviDoc = (CVideoAviDoc*)(pVideoAviDocTemplate->GetNextDoc(posVideoAviDoc));
		if (pVideoAviDoc)
			return TRUE;
	}

	return FALSE;
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

int CUImagerApp::GetTotalVideoDeviceDocs()
{
	CUImagerMultiDocTemplate* pVideoDeviceDocTemplate = GetVideoDeviceDocTemplate();
	POSITION posVideoDeviceDoc = pVideoDeviceDocTemplate->GetFirstDocPosition();
	int nCount = 0;
	while (posVideoDeviceDoc)
	{
		CVideoDeviceDoc* pVideoDeviceDoc = (CVideoDeviceDoc*)(pVideoDeviceDocTemplate->GetNextDoc(posVideoDeviceDoc));
		if (pVideoDeviceDoc)
			++nCount;
	}
	return nCount;
}

int CUImagerApp::GetTotalVideoDeviceDocsMovementDetecting()
{
	CUImagerMultiDocTemplate* pVideoDeviceDocTemplate = GetVideoDeviceDocTemplate();
	POSITION posVideoDeviceDoc = pVideoDeviceDocTemplate->GetFirstDocPosition();
	int nCount = 0;
	while (posVideoDeviceDoc)
	{
		CVideoDeviceDoc* pVideoDeviceDoc = (CVideoDeviceDoc*)(pVideoDeviceDocTemplate->GetNextDoc(posVideoDeviceDoc));
		if (pVideoDeviceDoc && pVideoDeviceDoc->m_dwVideoProcessorMode)
			++nCount;
			
	}
	return nCount;
}

#endif

void CUImagerApp::SaveOnEndSession()
{
	SavePlacement();
	m_PrinterControl.SavePrinterSelection(m_hDevMode, m_hDevNames);

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
			{
				if (pDoc->IsKindOf(RUNTIME_CLASS(CVideoAviDoc)))
					((CVideoAviDoc*)pDoc)->SaveSettings();
#ifdef VIDEODEVICEDOC
				else if (pDoc->IsKindOf(RUNTIME_CLASS(CVideoDeviceDoc)))
				{
					// Stop recording so that the index is not missing!
					if (((CVideoDeviceDoc*)pDoc)->m_pAVRec)
						((CVideoDeviceDoc*)pDoc)->CaptureRecord(FALSE); // No Message Box on Error
					((CVideoDeviceDoc*)pDoc)->SaveSettings();
				}
#endif
			}
		}
	}
#ifdef VIDEODEVICEDOC
	if (m_bMicroApacheStarted)
	{
		m_MicroApacheWatchdogThread.Kill();
		CVideoDeviceDoc::MicroApacheShutdown();
	}
	if (!m_bServiceProcess)
		BrowserAutostart();
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
	if (((dwImageSize >> 20) < (DWORD)(g_nAvailablePhysRamMB / 2)) &&
		(dwImageSize < BIG_PICTURE_SIZE_LIMIT))
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

CString CUImagerApp::VideoAviMakeMsg(CVideoAviDoc* pDoc)
{
	CString sMsg;

	if (!pDoc->m_pAVIPlay)
		sMsg = ML_STRING(1194, "No file loaded."); 
	else if (pDoc->IsModified())
		sMsg = ML_STRING(1195, "Try again after saving the file.");
	else if (pDoc->IsProcessing())
		sMsg = ML_STRING(1196, "Try again after the video processing has terminated.");
	else if (pDoc->m_PlayVideoFileThread.IsAlive() ||
			pDoc->m_PlayAudioFileThread.IsAlive())
		sMsg = ML_STRING(1197, "Stop playing the file and try again.");
	else
		sMsg = _T("");	
		
	return sMsg;
}

BOOL CUImagerApp::IsDocAvailable(CDocument* pDoc, BOOL bShowMsgBoxIfNotAvailable/*=FALSE*/)
{
	if (pDoc)
	{
		if (pDoc->IsKindOf(RUNTIME_CLASS(CPictureDoc)))
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
		else if (pDoc->IsKindOf(RUNTIME_CLASS(CVideoAviDoc)))
		{
			CString sMsg(VideoAviMakeMsg((CVideoAviDoc*)pDoc));
			if (!sMsg.IsEmpty())
			{
				if (bShowMsgBoxIfNotAvailable)
				{
					((CVideoAviDoc*)pDoc)->GetView()->ForceCursor();
					::AfxMessageBox(sMsg, MB_OK | MB_ICONINFORMATION);
					((CVideoAviDoc*)pDoc)->GetView()->ForceCursor(FALSE);
				}
				return FALSE;
			}
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

int CUImagerApp::GetUniqueTopDirAndCount(CString sZipFileName, CString& sTopDir)
{
	try
	{
		int i;

		// Reset Top Dir
		sTopDir = _T("");

		// Open Zip File
		m_Zip.Open(sZipFileName, CZipArchive::openReadOnly, 0);

		// Entries
		int nEntries = m_Zip.GetNoEntries();

		// Check for Unique Top Dir
		for (i = 0 ; i < nEntries ; i++)
		{
			if (m_Zip.IsFileDirectory((WORD)i))
			{
				// Get File Info
				CFileHeader fh;
				m_Zip.GetFileInfo(fh, (WORD)i);

				// Get Length
				int nLength = fh.m_szFileName.GetLength();
				if (nLength == 0)
					continue;

				// Is This a Top Dir?
				int pos = fh.m_szFileName.Find(_T('\\'));
				if (pos == (nLength - 1))
				{
					if (sTopDir == _T(""))
						sTopDir = fh.m_szFileName;
					else if (sTopDir != fh.m_szFileName)
					{
						m_Zip.Close();
						sTopDir = _T("");
						return nEntries;
					}
				}
			}
		}

		// Check whether all files are under the found unique top dir
		for (i = 0 ; i < nEntries ; i++)
		{
			// Get File Info
			CFileHeader fh;
			m_Zip.GetFileInfo(fh, (WORD)i);

			// If is file
			if (!m_Zip.IsFileDirectory((WORD)i))
			{
				int pos = fh.m_szFileName.Find(_T('\\'));
				if ((pos < 0) || (sTopDir != fh.m_szFileName.Left(pos+1)))
				{
					m_Zip.Close();
					sTopDir = _T("");
					return nEntries;
				}
			}
		}

		// Close Zip File
		m_Zip.Close();

		// Remove Trailing '\'
		sTopDir.TrimRight(_T('\\'));

		return nEntries;
		
	}
	catch (CZipException* e)
	{
		e->ReportZipError();
		e->Delete();
		m_Zip.Close(true);
		return 0;
	}
	catch (CException* e)
	{
		e->ReportError(MB_ICONSTOP);
		e->Delete();
		m_Zip.Close(true);
		return 0;
	}
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

BOOL CUImagerApp::ExtractZipToDir(LPCTSTR szDirPath, LPCTSTR szZipFileName, volatile int* pPictureFilesCount/*=NULL*/)
{
	CZipProgressDlg dlg(::AfxGetMainFrame(), TRUE);
	dlg.m_pPictureFilesCount = pPictureFilesCount;
	dlg.m_sZipFileName = CString(szZipFileName);
	dlg.m_sPath = CString(szDirPath);
	if (dlg.DoModal() == IDOK)
		return TRUE;
	else
		return FALSE;
}

CString CUImagerApp::ExtractZip(LPCTSTR szZipFileName, volatile int* pPictureFilesCount/*=NULL*/)
{
	// Directory Browse Dialog
	CString sExtractDir;

	if (m_bExtractHere)
	{
		TCHAR szDrive[_MAX_DRIVE];
		TCHAR szDir[_MAX_DIR];
		TCHAR szName[_MAX_FNAME];
		_tsplitpath(szZipFileName, szDrive, szDir, szName, NULL);
		
		CString sTopDir;
		int nZipEntries = GetUniqueTopDirAndCount(szZipFileName, sTopDir);
		if (nZipEntries == 0)
			return _T("");

		// If no Top Dir and at least 2 files,
		// use the Zip File Name as Top Dir
		if (sTopDir == _T("") && nZipEntries > 1)
		{
			sExtractDir = CString(szDrive) + CString(szDir) + CString(szName);
		}
		else
		{
			sExtractDir = CString(szDrive) + CString(szDir);
			sExtractDir.TrimRight(_T('\\'));
		}
	}
	else
	{
		sExtractDir = ::GetDriveAndDirName(szZipFileName);
		CBrowseDlg dlg(	::AfxGetMainFrame(),
						&sExtractDir,
						ML_STRING(1207, "Extract Files To Folder"),
						TRUE);
		if (dlg.DoModal() == IDCANCEL)
			return _T("");
	}
	
	if (ExtractZipToDir(sExtractDir, szZipFileName, pPictureFilesCount))
		return sExtractDir;
	else
		return _T("");
	
}

BOOL CUImagerApp::HasRecordableDrive(ICDBurn* pICDBurn/*=NULL*/)
{
	HRESULT hr;
	BOOL bDoRelease = FALSE;
	BOOL bHasRecordableDrive = FALSE;

	if (pICDBurn == NULL)
	{
		hr = ::CoCreateInstance(CLSID_CDBurn, NULL,CLSCTX_INPROC_SERVER,IID_ICDBurn,(LPVOID*)&pICDBurn);
		if (SUCCEEDED(hr))
			bDoRelease = TRUE;
		else
			return FALSE;
	}
	
	hr = pICDBurn->HasRecordableDrive(&bHasRecordableDrive);
	if (bDoRelease)
		pICDBurn->Release();
	return (SUCCEEDED(hr) && bHasRecordableDrive);
}

CString CUImagerApp::GetRecorderDriveLetter(ICDBurn* pICDBurn/*=NULL*/)
{
	HRESULT hr;
	BOOL bDoRelease = FALSE;
	WCHAR d[32] = {0};

	if (pICDBurn == NULL)
	{
		hr = ::CoCreateInstance(CLSID_CDBurn, NULL,CLSCTX_INPROC_SERVER,IID_ICDBurn,(LPVOID*)&pICDBurn);
		if (SUCCEEDED(hr))
			bDoRelease = TRUE;
		else
			return _T("");
	}

	hr = pICDBurn->GetRecorderDriveLetter(d, sizeof(d)/sizeof(d[0]));
	if (bDoRelease)
		pICDBurn->Release();
	if (SUCCEEDED(hr))
		return CString(d);
	else
		return _T("");
}

BOOL CUImagerApp::InitDiscRecorders2()
{
	// Altready init?
	if (m_DiscRecorders2.GetSize() > 0)
		return TRUE;

	// Init Disc Master
	CDiscMaster discMaster;
    if (!discMaster.Initialize())
        return FALSE;

    // Add Devices to array
    long totalDevices = discMaster.GetTotalDevices();
    for (long deviceIndex = 0 ; deviceIndex < totalDevices ; deviceIndex++)
    {
        CString recorderUniqueID = discMaster.GetDeviceUniqueID(deviceIndex);
        if (recorderUniqueID.IsEmpty())
            continue;
        CDiscRecorder* pDiscRecorder = new CDiscRecorder();
        if (pDiscRecorder == NULL)
            return FALSE;
        if (!pDiscRecorder->Initialize(recorderUniqueID))
        {
            delete pDiscRecorder;
            continue;
        }
		m_DiscRecorders2.Add(pDiscRecorder);
    }

	return (m_DiscRecorders2.GetSize() > 0);
}

void CUImagerApp::FreeDiscRecorders2()
{
	for (int i = 0 ; i < m_DiscRecorders2.GetSize() ; i++)
	{
		if (m_DiscRecorders2[i])
			delete m_DiscRecorders2[i];
	}
	m_DiscRecorders2.RemoveAll();
}

__forceinline CString CUImagerApp::GetBurnFolderPath()
{
	return ::GetSpecialFolderPath(CSIDL_CDBURN_AREA);
}

BOOL CUImagerApp::BurnDirContent(CString sDir) 
{
	ICDBurn* pICDBurn;
	HRESULT hr = ::CoCreateInstance(CLSID_CDBurn, NULL,CLSCTX_INPROC_SERVER,IID_ICDBurn,(LPVOID*)&pICDBurn);
	if (SUCCEEDED(hr))
	{
		if (HasRecordableDrive(pICDBurn))
		{
			CString sBurnerDriveLetter = GetRecorderDriveLetter(pICDBurn);
			CString sBurnFolder = GetBurnFolderPath();

			// Begin Wait Cursor
			BeginWaitCursor();

			// Empty the Burn Directory
			if (!::DeleteDirContent(sBurnFolder))
			{
				pICDBurn->Release();
				EndWaitCursor();
				::AfxMessageBox(ML_STRING(1208, "Error While Deleting The Burn Folder."), MB_OK | MB_ICONSTOP);
				return FALSE;
			}

			// Copy directory content to the burn directory
			if (!::CopyDirContent(sDir, sBurnFolder))
			{
				pICDBurn->Release();
				EndWaitCursor();
				::AfxMessageBox(ML_STRING(1209, "Error While Copying Files to The Burn Folder."), MB_OK | MB_ICONSTOP);
				return FALSE;
			}

			// End Wait Cursor
			EndWaitCursor();

			// Disable MainFrame
			CWnd* pParentWnd = ::AfxGetMainFrame();
			pParentWnd->EnableWindow(FALSE);

			// Burn The Files
			pICDBurn->Burn(::AfxGetMainFrame()->GetSafeHwnd());

			// Enable MainFrame And Set Focus
			pParentWnd->EnableWindow(TRUE);
			pParentWnd->SetActiveWindow();
			pParentWnd->SetFocus();

			// Release the Object
			pICDBurn->Release();

			// Empty the Burn Directory
			if (!::DeleteDirContent(sBurnFolder))
			{
				::AfxMessageBox(ML_STRING(1208, "Error While Deleting The Burn Folder."), MB_OK | MB_ICONSTOP);
				return FALSE;
			}
			else
				return TRUE;
		}
		else
		{
			pICDBurn->Release();
			::AfxMessageBox(ML_STRING(1210, "No Burner Was Detected."));
			return FALSE;
		}
	}
	else
	{
		::AfxMessageBox(ML_STRING(1211, "Burning Is Not Supported For Your Operating System."));
		return FALSE;
	}
}

int CUImagerApp::ExitInstance() 
{
	// Clean-up recorders array
	FreeDiscRecorders2();

#ifdef VIDEODEVICEDOC
	// Clean-Up Scheduler
	POSITION pos = m_Scheduler.GetHeadPosition();
	while (pos)
		delete m_Scheduler.GetNext(pos);
	m_Scheduler.RemoveAll();

	// Micro Apache shutdown
	if (m_bMicroApacheStarted)
	{
		m_MicroApacheWatchdogThread.Kill();
		CVideoDeviceDoc::MicroApacheShutdown();
	}

	// Browser autostart
	if (!m_bServiceProcess)
		BrowserAutostart();
#endif

	// Close The Application Mutex
	if (m_hAppMutex)
		::CloseHandle(m_hAppMutex);

	// Store last selected printer
	m_PrinterControl.SavePrinterSelection(m_hDevMode, m_hDevNames);

#ifndef CPJNSMTP_NOSSL
	// Clean up OpenSSL library
	// It's normal that OpenSSL leaks 16 + 20 bytes +
	// sometimes more but not increasing with the email sends
	ERR_free_strings();
	ERR_remove_state(0);
	EVP_cleanup();
	CRYPTO_cleanup_all_ex_data();

	// Clean up the SSL critical sections
	if (m_pOpenSSLCritSections)
	{
		delete [] m_pOpenSSLCritSections;
		m_pOpenSSLCritSections = NULL;
	}

	TRACE(_T("*** OPENSSL LEAKS 16 + 20 BYTES + SOMETIMES MORE, IT'S NORMAL ***\n"));
#endif

	// Clean-Up Trace Log File
	::EndTraceLogFile();

	// Delete Critical Section
	if (g_bAVCodecCSInited)
	{
		g_bAVCodecCSInited = FALSE;
		::DeleteCriticalSection(&g_csAVCodec);
	}

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

CPictureDoc* CUImagerApp::SlideShow(LPCTSTR sStartDirName,
									BOOL bFullscreen,
									BOOL bRunSlideshow,
									BOOL bRecursive)
{
	// Create New Picture Document
	CPictureDoc* pDoc = (CPictureDoc*)GetPictureDocTemplate()->OpenDocumentFile(NULL);
	if (pDoc == NULL)
		return NULL;

	// Set Zoom ComboBox
	CZoomComboBox* pZoomCB = &(((CPictureToolBar*)((CToolBarChildFrame*)(((CPictureDoc*)pDoc)->GetFrame()))->GetToolBar())->m_ZoomComboBox);
	pZoomCB->SetCurSel(pDoc->m_nZoomComboBoxIndex);
	pZoomCB->OnChangeZoomFactor(*((double*)(pZoomCB->GetItemDataPtr(pDoc->m_nZoomComboBoxIndex))));

	// Go to Full-Screen Mode
	if (bFullscreen)
		::AfxGetMainFrame()->EnterExitFullscreen();

	// Slideshow
	pDoc->m_SlideShowThread.SetStartName(sStartDirName);
	pDoc->m_SlideShowThread.SetRecursive(bRecursive);
	if (bRunSlideshow)
		pDoc->m_SlideShowThread.RunSlideshow();
	else
		pDoc->m_SlideShowThread.PauseSlideshow();

	return pDoc;
}

#ifdef VIDEODEVICEDOC

BOOL CUImagerApp::AutorunVideoDevicesDoWait(int nRetryCount)
{
	if (++nRetryCount <= AUTORUN_VIDEODEVICES_MAX_RETRIES)
	{
		// Retry in AUTORUN_VIDEODEVICES_RETRY_DELAY ms
		CPostDelayedMessageThread::PostDelayedMessage(	::AfxGetMainFrame()->GetSafeHwnd(),
														WM_AUTORUN_VIDEODEVICES,
														AUTORUN_VIDEODEVICES_RETRY_DELAY,
														(WPARAM)nRetryCount, 0);

		// Show starting progress dialog
		if (!m_bServiceProcess								&&
			!m_pAutorunProgressDlg							&&
			(!((CUImagerApp*)::AfxGetApp())->m_bTrayIcon	||
			!::AfxGetMainFrame()->m_TrayIcon.IsMinimizedToTray()))
		{
			m_pAutorunProgressDlg = new CProgressDlg(	::AfxGetMainFrame()->GetSafeHwnd(),
														ML_STRING(1720, "Starting Device..."),
														(nRetryCount - 1) * AUTORUN_VIDEODEVICES_RETRY_DELAY,
														AUTORUN_VIDEODEVICES_MAX_RETRIES * AUTORUN_VIDEODEVICES_RETRY_DELAY);
		}

		return TRUE;
	}
	else
		return FALSE;
}

void CUImagerApp::AutorunVideoDevices(int nRetryCount/*=0*/)
{
	CString sSection(_T("DeviceAutorun"));
	CWinApp* pApp = ::AfxGetApp();
	CString sKey;
	CString sDevRegistry;
	CVideoDeviceDoc* pDoc;
	unsigned int i;

	// If autorunning network devices or direct show devices
	// check whether they are ready, if not try again in
	// AUTORUN_VIDEODEVICES_RETRY_DELAY msec for a maximum of
	// AUTORUN_VIDEODEVICES_MAX_RETRIES retries
	for (i = 0 ; i < MAX_DEVICE_AUTORUN_KEYS ; i++)
	{
		sKey.Format(_T("%02u"), i);
		if ((sDevRegistry = pApp->GetProfileString(sSection, sKey, _T(""))) != _T(""))
		{
			CString sHost;
			if ((sHost = CVideoDeviceDoc::GetHostFromDevicePathName(sDevRegistry)) != _T(""))
			{
				// This function checks whether there is a network interface
				// that can connect to the given host
				if (!CNetCom::HasInterface(sHost))
				{
					if (AutorunVideoDevicesDoWait(nRetryCount))
						return;
					else
						break;
				}
			}
			else
			{
				CString sDev(sDevRegistry);
				sDev.Replace(_T('/'), _T('\\'));
				int nID = CDxCapture::GetDeviceID(sDev);
				if (nID < 0)
				{
					if (AutorunVideoDevicesDoWait(nRetryCount))
						return;
					else
						break;
				}
			}
		}
	}
	
	// Close progress dialog
	// Note: if application is exiting the mainframe closes this
	// child window and self-deletion will clean-up the memory
	if (m_pAutorunProgressDlg)
	{
		m_pAutorunProgressDlg->Close();	// self-deletion
		m_pAutorunProgressDlg = NULL;
	}

	// Start devices
	DWORD dwInitTickCount = ::GetTickCount();
	DWORD dwOpenNetworkDeviceCount = 0U;
	for (i = 0 ; i < MAX_DEVICE_AUTORUN_KEYS ; i++)
	{
		sKey.Format(_T("%02u"), i);
		if ((sDevRegistry = pApp->GetProfileString(sSection, sKey, _T(""))) != _T(""))
		{
			// Open Empty Document
			pDoc = (CVideoDeviceDoc*)((CUImagerApp*)::AfxGetApp())->GetVideoDeviceDocTemplate()->OpenDocumentFile(NULL);
			if (pDoc)
			{
				if (CVideoDeviceDoc::GetHostFromDevicePathName(sDevRegistry) != _T(""))
				{
					DWORD dwCurrentStartupDelay = ::GetTickCount() - dwInitTickCount;
					DWORD dwWantedNetworkDeviceStartupDelay = (dwOpenNetworkDeviceCount + 1U) * m_dwAutostartDelayMs;
					DWORD dwConnectDelay;
					if (dwWantedNetworkDeviceStartupDelay > dwCurrentStartupDelay)
						dwConnectDelay = dwWantedNetworkDeviceStartupDelay - dwCurrentStartupDelay;
					else
						dwConnectDelay = 0U;
					if (!pDoc->OpenGetVideo(sDevRegistry, dwConnectDelay))
						pDoc->CloseDocument();
					else
						dwOpenNetworkDeviceCount++;
				}
				else
				{
					CString sDev(sDevRegistry);
					sDev.Replace(_T('/'), _T('\\'));
					int nID = CDxCapture::GetDeviceID(sDev);
					if (nID >= 0)
					{
						if (!pDoc->OpenVideoDevice(nID))
							pDoc->CloseDocument();
					}
					else
					{
						pDoc->ConnectErr(ML_STRING(1568, "Unplugged"), sDevRegistry, sDevRegistry);
						pDoc->CloseDocument();
					}
				}
			}
		}
	}
}

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

void CUImagerApp::BrowserAutostart()
{
	::DeleteRegistryValue(	HKEY_CURRENT_USER,
							_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"),
							BROSERAUTORUN_NAME);
	if (m_bDoStartFromService && GetContaCamServiceState() == CONTACAMSERVICE_RUNNING &&
		!IsAutostart() && m_bBrowserAutostart)
	{
		TCHAR szDrive[_MAX_DRIVE];
		TCHAR szDir[_MAX_DIR];
		TCHAR szProgramName[MAX_PATH];
		if (::GetModuleFileName(NULL, szProgramName, MAX_PATH) != 0)
		{
			_tsplitpath(szProgramName, szDrive, szDir, NULL, NULL);
			CString sDriveDir = CString(szDrive) + CString(szDir);
			CString sUrl, sPort;
			sPort.Format(_T("%d"), m_nMicroApachePort);
			if (sPort != _T("80"))
				sUrl = _T("http://localhost:") + sPort + _T("/");
			else
				sUrl = _T("http://localhost/");
			CString sAutorunCommand;
			if (m_bFullscreenBrowser)
				sAutorunCommand = _T("\"") + sDriveDir + FULLSCREENBROWSER_EXE_NAME_EXT + _T("\" ") + sUrl;
			else
				sAutorunCommand = _T("rundll32.exe url.dll,FileProtocolHandler ") + sUrl;
			::SetRegistryStringValue(	HKEY_CURRENT_USER,
										_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"),
										BROSERAUTORUN_NAME, sAutorunCommand);
		}
	}
}
#endif

void CUImagerApp::OnFileOpenDir() 
{
	if (!::AfxGetMainFrame()->m_bFullScreenMode &&
		!m_bSlideShowOnly)
	{
		m_sLastOpenedDir.TrimRight(_T('\\'));
		if (!::IsExistingDir(m_sLastOpenedDir))
			m_sLastOpenedDir = ::GetSpecialFolderPath(CSIDL_MYPICTURES);
		CBrowseDlg dlg(	::AfxGetMainFrame(),
						&m_sLastOpenedDir,
						ML_STRING(1212, "Loading Folder's First Picture"),
						FALSE,
						TRUE,
						ML_STRING(963, "Recursive"),
						TRUE);
		if (dlg.DoModal() == IDOK)
		{
			SlideShow(	m_sLastOpenedDir,
						FALSE,
						FALSE,
						dlg.IsChecked());
			WriteProfileString(	_T("GeneralApp"),
								_T("LastOpenedDir"),
								m_sLastOpenedDir);
		}
	}
}

CUImagerApp::CUImagerCommandLineInfo::CUImagerCommandLineInfo()
{
	m_bStartSlideShow = FALSE;
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
			if (_tcscmp(pszParam, _T("slideshow")) == 0) // Case sensitive!
				m_bStartSlideShow = TRUE;
#ifdef VIDEODEVICEDOC
			else if (_tcscmp(pszParam, _T("service")) == 0) // Case sensitive!
				((CUImagerApp*)::AfxGetApp())->m_bServiceProcess = TRUE;
#endif
			else if (_tcscmp(pszParam, _T("extracthere")) == 0) // Case sensitive!
			{
				((CUImagerApp*)::AfxGetApp())->m_bExtractHere = TRUE;
				((CUImagerApp*)::AfxGetApp())->m_bHideMainFrame = TRUE;
			}
			else if (_tcscmp(pszParam, _T("play")) == 0) // Case sensitive!
				((CUImagerApp*)::AfxGetApp())->m_bStartPlay = TRUE;
			else if (_tcscmp(pszParam, _T("close")) == 0) // Case sensitive!
				((CUImagerApp*)::AfxGetApp())->m_bCloseAfterAudioPlayDone = TRUE;
			else if (_tcscmp(pszParam, _T("hide")) == 0) // Case sensitive!
				((CUImagerApp*)::AfxGetApp())->m_bHideMainFrame = TRUE;
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

		if (strcmp(pszParam, "slideshow") == 0) // Case sensitive!
			m_bStartSlideShow = TRUE;
#ifdef VIDEODEVICEDOC
		else if (strcmp(pszParam, "service") == 0) // Case sensitive!
			((CUImagerApp*)::AfxGetApp())->m_bServiceProcess = TRUE;
#endif
		else if (strcmp(pszParam, "exctracthere") == 0) // Case sensitive!
		{
			((CUImagerApp*)::AfxGetApp())->m_bExtractHere = TRUE;
			((CUImagerApp*)::AfxGetApp())->m_bHideMainFrame = TRUE;
		}
		else if (strcmp(pszParam, "play") == 0) // Case sensitive!
			((CUImagerApp*)::AfxGetApp())->m_bStartPlay = TRUE;
		else if (strcmp(pszParam, "close") == 0) // Case sensitive!
			((CUImagerApp*)::AfxGetApp())->m_bCloseAfterAudioPlayDone = TRUE;
		else if (strcmp(pszParam, "hide") == 0) // Case sensitive!
			((CUImagerApp*)::AfxGetApp())->m_bHideMainFrame = TRUE;
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
		// Example: uImager.exe "file1.jpg" "file2.jpg" /pt "Printer Name" "Driver Name" "Port Name"
		// (Printer parameters are optional)
		case CCommandLineInfo::FilePrintTo:
			ASSERT(m_pCmdInfo == NULL);
			m_bUseLoadPreviewDib = FALSE; // Load Full-Size Jpegs!
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
		// Example: uImager.exe "ad3.jpg" /p
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

void CUImagerApp::ShrinkStatusText(	CString sSrcFileName,
									CString sDstFileName)
{
	CString sStatusText;
	sStatusText.Format(_T("%s -> %s"),
				::GetShortFileName(sSrcFileName),
				::GetShortFileName(sDstFileName));
	::AfxGetMainFrame()->StatusText(sStatusText);
}

// Return Value:
// -1 : Not Finished
// 0  : Error
// 1  : Ok
int CUImagerApp::ShrinkOpenDocs( LPCTSTR szDstDirPath,
								 DWORD dwMaxSize,
								 BOOL bMaxSizePercent,
								 DWORD dwJpegQuality,
								 BOOL bPictureExtChange,
								 BOOL bShrinkPictures,
								 BOOL bShrinkVideos,
								 BOOL bOnlyCopyFiles)
{
	POSITION pos;
	CUImagerMultiDocTemplate* curTemplate = NULL;
	int res = 1;

	// Destination Directory Path
	CString sDstDirPath(szDstDirPath);
	sDstDirPath.TrimRight(_T('\\'));

	// Begin Wait Cursor
	BeginWaitCursor();

	// Keep track of all added file names to avoid duplicates
	CStringArray DstFileNames;

	// Picture Docs
	curTemplate = GetPictureDocTemplate();
	pos = curTemplate->GetFirstDocPosition();
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
		while (::InStringArray(sDstFileName, DstFileNames))
			sDstFileName.Format(_T("%s(%d)%s"), ::GetFileNameNoExt(sOrigDstFileName), ++i, ::GetFileExt(sOrigDstFileName));
		DstFileNames.Add(sDstFileName);
		sDstFileName = sDstDirPath + _T("\\") + sDstFileName;
		if (bPictureExtChange && !bOnlyCopyFiles)
		{
			CString sDstExt = ShrinkGetDstExt(::GetFileExt(sSrcFileName));
			sDstFileName = ::GetFileNameNoExt(sDstFileName) + sDstExt;
		}

		// Status Text
		ShrinkStatusText(sSrcFileName, sDstFileName);

		// Only Copy File?
		if (bOnlyCopyFiles)
		{
			if (!::CopyFile(sSrcFileName, sDstFileName, FALSE))
			{
				int nLastError = ::GetLastError();
				EndWaitCursor();
				::ShowError(nLastError, TRUE);
				return 0;
			}
			if (!::SetFileAttributes(sDstFileName, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_TEMPORARY))
			{
				int nLastError = ::GetLastError();
				EndWaitCursor();
				::ShowError(nLastError, TRUE);
				return 0;
			}
			continue;
		}

		// Shrink
		if (ShrinkPicture(	sSrcFileName,
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
			EndWaitCursor();
			return 0;
		}
	}

	// End Wait Cursor
	EndWaitCursor();

	// Video Docs
	curTemplate = GetVideoAviDocTemplate();
	pos = curTemplate->GetFirstDocPosition();
	while (pos)
	{
		CVideoAviDoc* pDoc = (CVideoAviDoc*)curTemplate->GetNextDoc(pos);

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
		while (::InStringArray(sDstFileName, DstFileNames))
			sDstFileName.Format(_T("%s(%d)%s"), ::GetFileNameNoExt(sOrigDstFileName), ++i, ::GetFileExt(sOrigDstFileName));
		DstFileNames.Add(sDstFileName);
		sDstFileName = sDstDirPath + _T("\\") + sDstFileName;
		
		// Status Text
		ShrinkStatusText(sSrcFileName, sDstFileName);

		// Only Copy File?
		if (bOnlyCopyFiles)
		{
			if (!::CopyFile(sSrcFileName, sDstFileName, FALSE))
			{
				::ShowLastError(TRUE);
				return 0;
			}
			if (!::SetFileAttributes(sDstFileName, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_TEMPORARY))
			{
				::ShowLastError(TRUE);
				return 0;
			}
			continue;
		}

		// Shrink
		if (bShrinkVideos)
		{
			pDoc->StartShrinkDocTo(sDstFileName);
			res = -1; // Not finished
		}
		else
		{
			if (!::CopyFile(sSrcFileName, sDstFileName, FALSE))
			{
				::ShowLastError(TRUE);
				return 0;
			}
			if (!::SetFileAttributes(sDstFileName, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_TEMPORARY))
			{
				::ShowLastError(TRUE);
				return 0;
			}
		}
	}

	return res;
}

BOOL CUImagerApp::CalcShrink(	const CDib& SrcDib,
								DWORD dwMaxSize,
								BOOL bMaxSizePercent,
								DWORD& dwShrinkWidth,
								DWORD& dwShrinkHeight)
{		
	BOOL bDoShrink = FALSE;
	double dAspectRatio = (double)SrcDib.GetWidth() / (double)SrcDib.GetHeight();

	// Landscape
	if (SrcDib.GetWidth() > SrcDib.GetHeight())
	{
		// From Percent to Pixels
		DWORD dwMaxSizePercent;
		if (bMaxSizePercent)
		{
			dwMaxSizePercent = dwMaxSize;
			dwMaxSize = (DWORD)Round(dwMaxSize / 100.0 * SrcDib.GetWidth());
		}
		
		// Resize to dwMaxSize x XYZ
		if (SrcDib.GetWidth() > dwMaxSize)
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
			dwMaxSize = (DWORD)Round(dwMaxSize / 100.0 * SrcDib.GetHeight());
		}

		// Resize to XYZ x dwMaxSize
		if (SrcDib.GetHeight() > dwMaxSize)
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
		!(::IsJPEG(szSrcFileName) && bForceJpegQuality)					&&
		!(::IsTIFF(szSrcFileName) && bTiffForceCompression))))
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
		::IsTIFF(szSrcFileName)					&&
		::IsTIFF(szDstFileName))
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
		BOOL bDoShrink = CalcShrink(SrcDib,
									dwMaxSize,
									bMaxSizePercent,
									dwShrinkWidth,
									dwShrinkHeight);

		// Shrink
		if (bDoShrink)
		{
			// Use Memory Mapped Load if Not Compressed and Not Old OS/2 Bmp File
			if (::GetFileExt(szSrcFileName) == _T(".bmp") ||
				::GetFileExt(szSrcFileName) == _T(".dib"))
			{
				if (!SrcDib.IsCompressed() &&
					!SrcDib.m_FileInfo.m_bBmpOS2Hdr)
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
				if (!DstDib1.ShrinkBits(dwShrinkWidth,
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
			if (!(::IsJPEG(szSrcFileName) && bForceJpegQuality) &&
				!(::IsTIFF(szSrcFileName) && bTiffForceCompression) &&
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
		// Use Memory Mapped Load if Not Compressed and Not Old OS/2 Bmp File
		if (::GetFileExt(szSrcFileName) == _T(".bmp") ||
			::GetFileExt(szSrcFileName) == _T(".dib"))
		{
			if (!SrcDib.IsCompressed() &&
				!SrcDib.m_FileInfo.m_bBmpOS2Hdr)
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
	if (::IsJPEGExt(sDstExt))
	{
		// Flatten
		if (pSaveDib->HasAlpha() && pSaveDib->GetBitCount() == 32)
		{
			pSaveDib->RenderAlphaWithSrcBackground();
			pSaveDib->SetAlpha(FALSE);
		}

#ifdef SUPPORT_LIBJPEG
		if (::IsJPEGExt(sSrcExt))
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
#endif
	}
	else if (::IsTIFFExt(sDstExt))
	{
#ifdef SUPPORT_LIBTIFF
		if (::IsTIFFExt(sSrcExt))
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
#endif
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
		// Store Alpha using the V4 Header
		else if (SrcDib.HasAlpha() && SrcDib.GetBitCount() == 32)
		{
			pSaveDib->BMIToBITMAPV4HEADER();
		}

		if (!pSaveDib->SaveBMP(	szDstFileName,
								pProgressWnd,
								bProgressSend,
								pThread))
			return 0;
	}
	else if (sDstExt == _T(".gif"))
	{
#ifdef SUPPORT_GIFLIB
		if (!CPictureDoc::SaveGIF(	szDstFileName,
									pSaveDib,
									pProgressWnd,
									bProgressSend,
									pThread))
			return 0;
#endif	
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

	return 1; // Saved
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
			BOOL bDoShrink = CalcShrink(*a[a.GetUpperBound()],
										dwMaxSize,
										bMaxSizePercent,
										dwShrinkWidth,
										dwShrinkHeight);

			// Shrink
			if (bDoShrink)
			{
				if (!a[a.GetUpperBound()]->ShrinkBits(	dwShrinkWidth,
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

	if ((sSrcExt == _T("bmp")) || (sSrcExt == _T("dib")) ||
		(sSrcExt == _T("emf")) || (sSrcExt == _T("pcx")) ||
		::IsJPEGExt(sSrcExt))
		return _T(".jpg");
	else if (::IsTIFFExt(sSrcExt))
		return _T(".tif");
	else if (sSrcExt == _T("gif"))
		return _T(".gif");
	else if (sSrcExt == _T("png"))
		return _T(".png");
	else if ((sSrcExt == _T("avi")) || (sSrcExt == _T("divx")))
		return _T(".avi");
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

	if (!m_bForceSeparateInstance
#ifdef VIDEODEVICEDOC
		&& !m_bServiceProcess
#endif
		)
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

	// MainFrame Placement
	LoadPlacement(showCmd);

	// Preview File Dialog
	m_bFileDlgPreview = (BOOL)GetProfileInt(sSection, _T("FileDlgPreview"), TRUE);
	g_nPreviewFileDlgViewMode = (int)GetProfileInt(sSection, _T("PreviewFileDlgViewMode"), SHVIEW_Default);

	// Display Advanced On-Screen Video Avi Info
	m_bVideoAviInfo = (BOOL)GetProfileInt(sSection, _T("VideoAviInfo"), FALSE);

	// Last Opened Directory
	m_sLastOpenedDir = GetProfileString(sSection, _T("LastOpenedDir"), _T(""));

	// ESC to exit the program
	m_bEscExit = (BOOL)GetProfileInt(sSection, _T("ESCExit"), FALSE);

	// Disable opening external program (for pdf, swf)
	m_bDisableExtProg = (BOOL)GetProfileInt(sSection, _T("DisableExtProg"), FALSE);

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
	// Browser
	m_bFullscreenBrowser = (BOOL)GetProfileInt(sSection, _T("FullscreenBrowser"), FALSE);
	m_bBrowserAutostart = (BOOL)GetProfileInt(sSection, _T("BrowserAutostart"), FALSE);

	// Priority to IPv6
	m_bIPv6 = (BOOL)GetProfileInt(sSection, _T("IPv6"), FALSE);

	// Device Autostart delay
	m_dwAutostartDelayMs = (DWORD)GetProfileInt(sSection, _T("AutostartDelayMs"), 0);

	// Use Custom Temp Folder
	m_bUseCustomTempFolder = (BOOL)GetProfileInt(sSection, _T("UseCustomTempFolder"), FALSE);

	// Start Micro Apache
	m_bStartMicroApache = (BOOL)GetProfileInt(sSection, _T("StartMicroApache"), TRUE);

	// Micro Apache Document Root
	m_sMicroApacheDocRoot = GetProfileString(sSection, _T("MicroApacheDocRoot"), m_sMicroApacheDocRoot);
	::CreateDir(m_sMicroApacheDocRoot);

	// Micro Apache Server Port
	m_nMicroApachePort = GetProfileInt(sSection, _T("MicroApachePort"), MICROAPACHE_DEFAULT_PORT);

	// Micro Apache Authentication
	m_bMicroApacheDigestAuth = (BOOL)GetProfileInt(sSection, _T("MicroApacheDigestAuth"), TRUE);
	m_sMicroApacheAreaname = GetProfileString(sSection, _T("MicroApacheAreaname"), MICROAPACHE_DEFAULT_AUTH_AREANAME);
	m_sMicroApacheUsername = GetSecureProfileString(sSection, _T("MicroApacheUsername"), _T(""));
	m_sMicroApachePassword = GetSecureProfileString(sSection, _T("MicroApachePassword"), _T(""));

	// Load Schedulers
	int nCount = GetProfileInt(sSection, _T("SchedulerCount"), 0);
	for (int i = 0 ; i < nCount ; i++)
	{
		sSection.Format(_T("Scheduler%010d"), i);
		CString sDevicePathName = GetProfileString(sSection, _T("DevicePathName"), _T(""));
		if (sDevicePathName != _T(""))
		{
			CUImagerApp::CSchedulerEntry* pSchedulerEntry = new CUImagerApp::CSchedulerEntry;
			if (pSchedulerEntry)
			{
				pSchedulerEntry->m_sDevicePathName = sDevicePathName;
				pSchedulerEntry->m_Type = (CSchedulerEntry::SchedulerEntryType)GetProfileInt(sSection, _T("Type"), (int)CUImagerApp::CSchedulerEntry::ONCE);
				pSchedulerEntry->m_StartTime = CTime(	GetProfileInt(sSection, _T("StartYear"), 2000),
														GetProfileInt(sSection, _T("StartMonth"), 1),
														GetProfileInt(sSection, _T("StartDay"), 1),
														GetProfileInt(sSection, _T("StartHour"), 12),
														GetProfileInt(sSection, _T("StartMin"), 0),
														GetProfileInt(sSection, _T("StartSec"), 0));
				pSchedulerEntry->m_StopTime = CTime(	GetProfileInt(sSection, _T("StopYear"), 2000),
														GetProfileInt(sSection, _T("StopMonth"), 1),
														GetProfileInt(sSection, _T("StopDay"), 1),
														GetProfileInt(sSection, _T("StopHour"), 12),
														GetProfileInt(sSection, _T("StopMin"), 0),
														GetProfileInt(sSection, _T("StopSec"), 0));
				m_Scheduler.AddTail(pSchedulerEntry);
			}
		}
	}
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

void CUImagerApp::SendOpenDocsAsMailInit()
{
	int res;

	CSendMailDocsDlg dlg(::AfxGetMainFrame());

	if (dlg.DoModal() == IDOK)
	{	
		// Check & Store Var
		if (dlg.m_bZipFile)
		{
			if (dlg.m_sZipFileName == _T(""))
				m_sZipFile = _T("Files.zip"); 
			else
				m_sZipFile = dlg.m_sZipFileName;
		}
		else
			m_sZipFile = _T("");

		// Create & Empty Email Temp Dir
		CString sTempEmailDir;
		sTempEmailDir.Format(_T("Email%X"), ::GetCurrentProcessId());
		sTempEmailDir = ((CUImagerApp*)::AfxGetApp())->GetAppTempDir() + sTempEmailDir;
		if (!::IsExistingDir(sTempEmailDir))
		{
			if (!::CreateDir(sTempEmailDir))
			{
				::ShowLastError(TRUE);
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
			res = ShrinkOpenDocs(sTempEmailDir,
								AUTO_SHRINK_MAX_SIZE,
								FALSE,
								DEFAULT_JPEGCOMPRESSION,
								TRUE,
								TRUE,
								TRUE,
								FALSE);
		}
		else if (dlg.m_nOptimizationSelection == CSendMailDocsDlg::NO_OPT) // Leave Unchanged
		{
			res = ShrinkOpenDocs(sTempEmailDir,
								0,
								FALSE,
								0,
								FALSE,
								FALSE,
								FALSE,
								TRUE);
		}
		else if (dlg.m_nOptimizationSelection == CSendMailDocsDlg::ADV_OPT) // Advanced Settings
		{
			res = ShrinkOpenDocs(sTempEmailDir,
								(dlg.m_nPixelsPercentSel == 0) ? dlg.m_nShrinkingPixels : dlg.m_nShrinkingPercent,
								(dlg.m_nPixelsPercentSel == 1),
								dlg.m_nJpegQuality,
								dlg.m_bPictureExtChange,
								dlg.m_bShrinkingPictures,
								dlg.m_bShrinkingVideos,
								FALSE);
		}

		// If No Avi Files, we can finish now,
		// if avi file we have to wait untill all shrinking threads terminate!
		// See CMainFrame::OnShrinkDocTerminated()
		if (res == 1)
			SendDocAsMailFinish(TRUE);		// Ok
		else if (res == -1)
			m_bWaitingMailFinish = TRUE;	// Wait
		else
			SendDocAsMailFinish(FALSE);		// Error
	}
}

void CUImagerApp::SendDocAsMailFinish(BOOL bOk)
{
	// Reset Flag
	m_bWaitingMailFinish = FALSE;

	if (bOk)
	{
		// The Email Temp Dirs
		CString sTempEmailDir;
		sTempEmailDir.Format(_T("Email%X"), ::GetCurrentProcessId());
		sTempEmailDir = ((CUImagerApp*)::AfxGetApp())->GetAppTempDir() + sTempEmailDir;
		CString sTempEmailZipDir;
		sTempEmailZipDir.Format(_T("EmailZip%X"), ::GetCurrentProcessId());
		sTempEmailZipDir = ((CUImagerApp*)::AfxGetApp())->GetAppTempDir() + sTempEmailZipDir;

		// Do Zip?
		if (m_sZipFile != _T(""))
		{
			// Adjust if no correct extension or no extension at all supplied
			m_sZipFile = ::GetFileNameNoExt(m_sZipFile);
			m_sZipFile += _T(".zip");

			// Create Email Zip Temp Directory if not existing
			if (!::IsExistingDir(sTempEmailZipDir))
			{
				if (!::CreateDir(sTempEmailZipDir))
				{
					::ShowLastError(TRUE);
					::AfxGetMainFrame()->StatusText();
					return;
				}
			}

			// Compress & Send
			if (CompressToZip(sTempEmailDir, sTempEmailZipDir + _T("\\") + m_sZipFile))
				SendMail(sTempEmailZipDir + _T("\\") + m_sZipFile);

			// Delete Email Zip Temp Directory
			::DeleteDir(sTempEmailZipDir);
		}
		else
			SendMail(sTempEmailDir); // Send Email Directory Content
	
		::DeleteDir(sTempEmailDir);
	}

	// Reset Status Text
	::AfxGetMainFrame()->StatusText();
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
		pMailState->m_hInstMail = ::LoadLibraryA("MAPI32.DLL");

	if (pMailState->m_hInstMail == NULL)
	{
		EndWaitCursor();
		::AfxMessageBox(AFX_IDP_FAILED_MAPI_LOAD);
		return FALSE;
	}
	ASSERT(pMailState->m_hInstMail != NULL);

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
			_stprintf(name, _T("%s\\*"), sAdjPath);
			HANDLE hFileSearch = ::FindFirstFile(name, &info);
			if (!hFileSearch || hFileSearch == INVALID_HANDLE_VALUE)
			{
				EndWaitCursor();
				return FALSE;
			}
			do
			{
				_stprintf(name, _T("%s\\%s"), sAdjPath, info.cFileName);
				if (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					continue;
				else
				{
					// File Count Check
					if (nFileCount == MAX_ATTACHMENTS)
					{
						EndWaitCursor();
						CString Str;
						Str.Format(	ML_STRING(1214, "To Many Attached Files. Max Number is %i.\n") +
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

void CUImagerApp::SuggestDirectXVersion7()
{
	DWORD dwDxVer = 0U;
	TCHAR szDxVer[16] = {0};
	::GetDXVersion(&dwDxVer, szDxVer, 16);
	CString strResult;
    if (dwDxVer > 0U)
		strResult.Format(ML_STRING(1216, "DirectX %s installed."), szDxVer);
    else
        strResult.Format(ML_STRING(1217, "DirectX not installed."));
	if (dwDxVer < 0x00070000)
	{
		CDirectX7Dlg dlg(::AfxGetMainFrame());
		dlg.m_sTextRow1 = strResult;
		dlg.m_sTextRow2 = ML_STRING(1219, "DirectX 7.0 or higher is required for best video playback performance!");
		dlg.m_sTextRow3 = ML_STRING(1220, "The latest version may be downloaded from:");
		dlg.m_sTextLink = _T("http://support.microsoft.com/kb/179113");
		dlg.DoModal();
	}
}

BOOL CUImagerApp::RequireDirectXVersion7()
{
	DWORD dwDxVer = 0U;
	TCHAR szDxVer[16] = {0};
	::GetDXVersion(&dwDxVer, szDxVer, 16);
	CString strResult;
    if (dwDxVer > 0U)
		strResult.Format(ML_STRING(1216, "DirectX %s installed."), szDxVer);
    else
        strResult.Format(ML_STRING(1217, "DirectX not installed."));
	if (dwDxVer < 0x00070000)
	{
		CDirectX7Dlg dlg(::AfxGetMainFrame());
		dlg.m_sTextRow1 = strResult;
		dlg.m_sTextRow2 = ML_STRING(1221, "DirectX 7.0 or higher is required!");
		dlg.m_sTextRow3 = ML_STRING(1220, "The latest version may be downloaded from:");
		dlg.m_sTextLink = _T("http://support.microsoft.com/kb/179113");
		dlg.DoModal();
		return FALSE;
	}
	else
		return TRUE;
}

BOOL CUImagerApp::IsSupportedPictureFile(CString sFileName)
{
	CString sExt = ::GetFileExt(sFileName);
	if ((sExt == _T(".bmp"))	||
		(sExt == _T(".dib"))	||
		(sExt == _T(".emf"))	||
		(sExt == _T(".png"))	||
		(sExt == _T(".pcx"))	||
		(sExt == _T(".gif"))	||
		::IsJPEGExt(sExt)		||
		::IsTIFFExt(sExt))
		return TRUE;
	else
		return FALSE;
}

BOOL CUImagerApp::IsAVIFile(CString sFileName)
{
	CString sExt = ::GetFileExt(sFileName);

	if (sExt == _T(".avi") || sExt == _T(".divx"))
		return TRUE;
	else
		return FALSE;
}

BOOL CUImagerApp::IsSWFFile(CString sFileName)
{
	CString sExt = ::GetFileExt(sFileName);

	if (sExt == _T(".swf"))
		return TRUE;
	else
		return FALSE;
}

BOOL CUImagerApp::IsSupportedMusicFile(CString sFileName)
{
	CString sExt = ::GetFileExt(sFileName);

	if (sExt == _T(".mp3"))
		return TRUE;
	else if (sExt == _T(".wav"))
		return TRUE;
	else if (sExt == _T(".wma"))
		return TRUE;
	else if ((sExt == _T(".mid")) || (sExt == _T(".rmi")))
		return TRUE;
	else if (sExt == _T(".au"))
		return TRUE;
	else if ((sExt == _T(".aif")) || (sExt == _T(".aiff")))
		return TRUE;
	else
		return FALSE;
}

BOOL CUImagerApp::IsSupportedCDAudioFile(CString sFileName)
{
	CString sExt = ::GetFileExt(sFileName);

	if (sExt == _T(".cda"))
		return TRUE;
	else
		return FALSE;
}

CUImagerMultiDocTemplate* CUImagerApp::GetTemplateFromFileExtension(CString sFileName)
{
	if (IsAVIFile(sFileName))
		return GetVideoAviDocTemplate();
	else if (IsSupportedPictureFile(sFileName))
		return GetPictureDocTemplate();
	else if (IsSupportedMusicFile(sFileName))
		return GetAudioMCIDocTemplate();
	else if (IsSupportedCDAudioFile(sFileName))
		return GetCDAudioDocTemplate();
	else
		return NULL;
}

void CUImagerApp::OnFileNew() 
{
	if (!::AfxGetMainFrame()->m_bFullScreenMode &&
		!m_bSlideShowOnly)
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

			// Set Zoom to Fit Big
			CZoomComboBox* pZoomCB = &(((CPictureToolBar*)((CToolBarChildFrame*)(pDoc->GetFrame()))->GetToolBar())->m_ZoomComboBox);
			pZoomCB->SetCurSel(pDoc->m_nZoomComboBoxIndex = 1); // Fit Big
			pZoomCB->OnChangeZoomFactor(*((double*)(pZoomCB->GetItemDataPtr(1))));

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
	pCmdUI->Enable(	(::AfxGetMainFrame() && !::AfxGetMainFrame()->m_bFullScreenMode)	&&
					!m_bSlideShowOnly);
}

void CUImagerApp::OnEditPaste() 
{
	if (!::AfxGetMainFrame()->m_bFullScreenMode &&
		!m_bSlideShowOnly)
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

			// Set Zoom to Fit Big
			CZoomComboBox* pZoomCB = &(((CPictureToolBar*)((CToolBarChildFrame*)(pDoc->GetFrame()))->GetToolBar())->m_ZoomComboBox);
			pZoomCB->SetCurSel(pDoc->m_nZoomComboBoxIndex = 1); // Fit Big
			pZoomCB->OnChangeZoomFactor(*((double*)(pZoomCB->GetItemDataPtr(1))));

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
	pCmdUI->Enable(	(::IsClipboardFormatAvailable(CF_DIB)								||
					::IsClipboardFormatAvailable(CF_ENHMETAFILE))						&&
					(::AfxGetMainFrame() && !::AfxGetMainFrame()->m_bFullScreenMode)	&&
					!m_bSlideShowOnly);
}

void CUImagerApp::OnEditScreenshot() 
{
	if (!::AfxGetMainFrame()->m_bFullScreenMode &&
		!m_bSlideShowOnly)
	{
		CaptureScreenToClipboard();
		OnEditPaste();
	}
}

void CUImagerApp::OnUpdateEditScreenshot(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(::AfxGetMainFrame() && !::AfxGetMainFrame()->m_bFullScreenMode) &&
					!m_bSlideShowOnly);
}

void CUImagerApp::UpdateFileAssociations()
{
	// Graphics
	BOOL bBmp =		IsFileTypeAssociated(_T("bmp"));
	BOOL bJpeg =	IsFileTypeAssociated(_T("jpg"))	&&
					IsFileTypeAssociated(_T("jpeg"))&&
					IsFileTypeAssociated(_T("jpe"))	&&
					IsFileTypeAssociated(_T("thm"));
	BOOL bPcx =		IsFileTypeAssociated(_T("pcx"));
	BOOL bEmf =		IsFileTypeAssociated(_T("emf"));
	BOOL bPng =		IsFileTypeAssociated(_T("png"));
	BOOL bTiff =	IsFileTypeAssociated(_T("tif"))	&&
					IsFileTypeAssociated(_T("tiff"))&&
					IsFileTypeAssociated(_T("jfx"));
	BOOL bGif =		IsFileTypeAssociated(_T("gif"));

	// Audio
	BOOL bAif =		IsFileTypeAssociated(_T("aif"))	&&
					IsFileTypeAssociated(_T("aiff"));
	BOOL bAu =		IsFileTypeAssociated(_T("au"));
	BOOL bMidi =	IsFileTypeAssociated(_T("mid"))	&&
					IsFileTypeAssociated(_T("rmi"));
	BOOL bMp3 =		IsFileTypeAssociated(_T("mp3"));
	BOOL bWav =		IsFileTypeAssociated(_T("wav"));
	BOOL bWma =		IsFileTypeAssociated(_T("wma"));
	BOOL bCda =		IsFileTypeAssociated(_T("cda"));

	// Others
	BOOL bAvi =		IsFileTypeAssociated(_T("avi")) &&
					IsFileTypeAssociated(_T("divx"));
	BOOL bZip =		IsFileTypeAssociated(_T("zip"));

	
	// Graphics

	if (bBmp)
		AssociateFileType(_T("bmp"));
	else
		UnassociateFileType(_T("bmp"));

	if (bJpeg)
	{
		AssociateFileType(_T("jpg"));
		AssociateFileType(_T("jpeg"));
		AssociateFileType(_T("jpe"));
		AssociateFileType(_T("thm"));
	}
	else
	{
		UnassociateFileType(_T("jpg"));
		UnassociateFileType(_T("jpeg"));
		UnassociateFileType(_T("jpe"));
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


	// Audio

	if (bAif)
	{
		AssociateFileType(_T("aif"));
		AssociateFileType(_T("aiff"));
	}
	else
	{
		UnassociateFileType(_T("aif"));
		UnassociateFileType(_T("aiff"));
	}

	if (bAu)
		AssociateFileType(_T("au"));
	else
		UnassociateFileType(_T("au"));

	if (bMidi)
	{
		AssociateFileType(_T("mid"));
		AssociateFileType(_T("rmi"));
	}
	else
	{
		UnassociateFileType(_T("mid"));
		UnassociateFileType(_T("rmi"));
	}

	if (bMp3)
		AssociateFileType(_T("mp3"));
	else
		UnassociateFileType(_T("mp3"));

	if (bWav)
		AssociateFileType(_T("wav"));
	else
		UnassociateFileType(_T("wav"));

	if (bWma)
		AssociateFileType(_T("wma"));
	else
		UnassociateFileType(_T("wma"));

	if (bCda)
		AssociateFileType(_T("cda"));
	else
		UnassociateFileType(_T("cda"));

	// Others

	if (bAvi)
	{
		AssociateFileType(_T("avi"));
		AssociateFileType(_T("divx"));
	}
	else
	{
		UnassociateFileType(_T("avi"));
		UnassociateFileType(_T("divx"));
	}

	if (bZip)
		AssociateFileType(_T("zip"));
	else
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

BOOL CUImagerApp::AssociateFileType(CString sExt, BOOL* pbHasUserChoice/*=NULL*/)
{
	// Reset value
	if (pbHasUserChoice)
		*pbHasUserChoice = FALSE;

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
		// and can prevent from being associated. Vista or higher do not let you delete
		// it, even as administrator ... we can delete it with regedit.exe
		if (pbHasUserChoice)
		{
			if (::IsRegistryKey(HKEY_CURRENT_USER, sCurrentUserFileExtsPath + _T("\\UserChoice")))
				*pbHasUserChoice = TRUE;
		}
	}

	// Create My Class Name or ProgID

	// Description, Shown In Explorer
	::SetRegistryStringValue(HKEY_CLASSES_ROOT, sMyFileClassName, _T(""), CString(szProgName) + _T(" ") + sExtNoPoint + _T(" file"));
	
	// Shell Open Command
	
	// Icon order
	// Note: IDR_PICTURE_NOHQ, IDR_BIGPICTURE and IDR_BIGPICTURE_NOHQ
	//       are not used anymore, but icons remain to keep the
	//       same order!
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

	// Zip
	if (sExtNoPoint == _T("zip"))
	{
		::SetRegistryStringValue(HKEY_CLASSES_ROOT, sMyFileClassName + _T("\\DefaultIcon"), _T(""), CString(szProgPath) + _T(",8"));

		::SetRegistryStringValue(HKEY_CLASSES_ROOT,
								sMyFileClassName +
								_T("\\shell\\open\\command"),
								_T(""),
								_T("\"") + CString(szProgPath) +
								_T("\"") + _T(" \"%1\""));
		::SetRegistryStringValue(HKEY_CLASSES_ROOT,
								sMyFileClassName +
								_T("\\shell\\Extract Here\\command"),
								_T(""),
								_T("\"") +  CString(szProgPath)
								+ _T("\"") + _T(" /extracthere") + _T(" \"%1\""));
	}
	// Audio
	else if (	sExtNoPoint == _T("mp3")	||
				sExtNoPoint == _T("wav")	||
				sExtNoPoint == _T("wma")	||
				sExtNoPoint == _T("mid")	||
				sExtNoPoint == _T("rmi")	||
				sExtNoPoint == _T("au")		||
				sExtNoPoint == _T("aif")	||
				sExtNoPoint == _T("aiff"))
	{
		::SetRegistryStringValue(HKEY_CLASSES_ROOT, sMyFileClassName + _T("\\DefaultIcon"), _T(""), CString(szProgPath) + _T(",1"));

		::SetRegistryStringValue(HKEY_CLASSES_ROOT,
								sMyFileClassName +
								_T("\\shell\\open\\command"),
								_T(""),
								_T("\"") + CString(szProgPath) +
								_T("\"") + _T(" \"%1\""));
	}
	// Audio CD
	else if (sExtNoPoint == _T("cda"))
	{
		::SetRegistryStringValue(HKEY_CLASSES_ROOT, sMyFileClassName + _T("\\DefaultIcon"), _T(""), CString(szProgPath) + _T(",2"));

		::SetRegistryStringValue(HKEY_CLASSES_ROOT,
								sMyFileClassName +
								_T("\\shell\\open\\command"),
								_T(""),
								_T("\"") + CString(szProgPath) +
								_T("\"") + _T(" \"%1\""));
	}
	// Video
	else if (sExtNoPoint == _T("avi")	||
			sExtNoPoint == _T("divx"))
	{
		::SetRegistryStringValue(HKEY_CLASSES_ROOT, sMyFileClassName + _T("\\DefaultIcon"), _T(""), CString(szProgPath) + _T(",3"));

		::SetRegistryStringValue(HKEY_CLASSES_ROOT,
								sMyFileClassName +
								_T("\\shell\\open\\command"),
								_T(""),
								_T("\"") + CString(szProgPath) +
								_T("\"") + _T(" \"%1\""));
	}
	// Graphics
	else
	{
		if (sExtNoPoint == _T("bmp"))
			::SetRegistryStringValue(HKEY_CLASSES_ROOT, sMyFileClassName + _T("\\DefaultIcon"), _T(""), CString(szProgPath) + _T(",17"));
		else if (sExtNoPoint == _T("gif"))
			::SetRegistryStringValue(HKEY_CLASSES_ROOT, sMyFileClassName + _T("\\DefaultIcon"), _T(""), CString(szProgPath) + _T(",18"));
		else if (::IsJPEGExt(sExtNoPoint))
			::SetRegistryStringValue(HKEY_CLASSES_ROOT, sMyFileClassName + _T("\\DefaultIcon"), _T(""), CString(szProgPath) + _T(",19"));
		else if (::IsTIFFExt(sExtNoPoint))
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
	}

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

void CUImagerApp::OnToolsAvimergeAs() 
{
	// Open Empty Video Document For Progress Display And Load Avi if Success
	CVideoAviDoc* pDoc = (CVideoAviDoc*)(GetVideoAviDocTemplate()->OpenDocumentFile(NULL));
	if (pDoc)
		pDoc->StartFileMergeAs();
}

void CUImagerApp::OnToolsViewLogfile() 
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
		const int MAX_SECTIONNAMES_BUFFER = 65535; // that's the maximum for Win95, Win98 and WinMe (bigger bufs are not working)
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

void CUImagerApp::OnToolsMoveCamFolders()
{
	// Check
	if (AreVideoDeviceDocsOpen())
	{
		::AfxMessageBox(ML_STRING(1872, "Try again after closing all devices"), MB_OK | MB_ICONERROR);
		return;
	}

	// Call browse for folder dialog
	CString sMicroApacheDocRoot = m_sMicroApacheDocRoot;
	sMicroApacheDocRoot.TrimRight(_T('\\'));
	CString sNewMicroApacheDocRoot = sMicroApacheDocRoot;
	CBrowseDlg dlg(	::AfxGetMainFrame(),
					&sNewMicroApacheDocRoot,
					ML_STRING(1871, "Move all camera folders to selected directory"),
					TRUE);
	if (dlg.DoModal() == IDOK)
	{
		// Fail if sNewMicroApacheDocRoot is a nested subdir of the old one
		sNewMicroApacheDocRoot.TrimRight(_T('\\'));
		if (::IsSubDir(sMicroApacheDocRoot, sNewMicroApacheDocRoot))
		{
			::AfxMessageBox(ML_STRING(1870, "The new folder cannot be a subfolder of the old one"), MB_OK | MB_ICONERROR);
			return;
		}

		// Begin Wait Cursor
		BeginWaitCursor();

		// Update all RecordAutoSaveDir configuration entries
		CStringArray DevicePathNames;
		EnumConfiguredDevicePathNames(DevicePathNames);
		for (int i = 0 ; i < DevicePathNames.GetSize() ; i++)
		{
			CString sRecordAutoSaveDir = GetProfileString(DevicePathNames[i], _T("RecordAutoSaveDir"), _T(""));
			sRecordAutoSaveDir.TrimRight(_T('\\'));
			int index;
			if ((index = sRecordAutoSaveDir.ReverseFind(_T('\\'))) >= 0)
				sRecordAutoSaveDir = sRecordAutoSaveDir.Right(sRecordAutoSaveDir.GetLength() - index - 1);
			WriteProfileString(DevicePathNames[i], _T("RecordAutoSaveDir"), sNewMicroApacheDocRoot + _T("\\") + sRecordAutoSaveDir);
		}

		// Merge if different directories
		if (sNewMicroApacheDocRoot.CompareNoCase(sMicroApacheDocRoot) != 0)
		{
			if (!::MergeDirContent(sMicroApacheDocRoot, sNewMicroApacheDocRoot)) // overwrite existing
			{
				DWORD dwLastError = ::GetLastError();
				EndWaitCursor();
				::ShowError(dwLastError, TRUE);
				BeginWaitCursor();
			}
			else
				::DeleteDir(sMicroApacheDocRoot); // no error message on failure
		}

		// Update doc root and reload web server
		m_sMicroApacheDocRoot = sNewMicroApacheDocRoot;
		WriteProfileString(	_T("GeneralApp"),
							_T("MicroApacheDocRoot"),
							m_sMicroApacheDocRoot);
		int nRet = CVideoDeviceDoc::MicroApacheReload();

		// End Wait Cursor
		EndWaitCursor();

		// Micro apache error message
		if (nRet <= 0)
		{
			if (nRet == 0)
				::AfxMessageBox(ML_STRING(1474, "Failed to stop the web server"), MB_ICONSTOP);
			else
				::AfxMessageBox(ML_STRING(1475, "Failed to start the web server"), MB_ICONSTOP);
		}
	}
}

void CUImagerApp::OnEditDelete() 
{
	CDeleteCamFoldersDlg dlg;
	dlg.DoModal();
}

int CUImagerApp::CMicroApacheWatchdogThread::Work()
{
	DWORD Event;
	int nLastCount = 0;

	// NOTE:
	// Up to Windows XP the first logged user and the services run both in session 0.
	// When logging off, the system sends a logoff event to all console applications
	// -> the mapache.exe processes terminate even if started from a service. Thus a
	// watchdog is necessary for these older systems. Starting from Windows 2003 Server
	// the services run in session 0, all the other processes in session 1 or higher.
	// Logging off will not terminate the mapache.exe processes started from the service
	// because they are session 0 processes. For these systems a watchdog is still good
	// in case the mapache.exe processes are killed or crash for some unknown reasons.
	for (;;)
	{
		Event = ::WaitForSingleObject(GetKillEvent(), MICROAPACHE_WATCHDOG_CHECK_TIME);
		switch (Event)
		{
			// Shutdown Event
			case WAIT_OBJECT_0 :
				return 0;

			// Check
			case WAIT_TIMEOUT :		
			{
				int nCount = ::EnumKillProcByName(MICROAPACHE_FILENAME);
				if (nCount == 0)
					CVideoDeviceDoc::MicroApacheInitStart();
				else if (	nCount == nLastCount	&&
							nCount > 0				&&
							nCount < MICROAPACHE_NUM_PROCESS)
					::EnumKillProcByName(MICROAPACHE_FILENAME, TRUE); // Kill it!
				nLastCount = nCount;
				break;
			}

			default:
				break;
		}
	}

	return 0;
}

CUImagerApp::CSchedulerEntry::CSchedulerEntry()
{
	m_Type = ONCE;
	m_sDevicePathName = _T("");
	m_pDoc = NULL;
	m_bRunning = FALSE;
	m_StartTime = 0;
	m_StopTime = 0;
	m_bInsideStart = FALSE;
	m_bInsideStop = FALSE;
}

void CUImagerApp::CSchedulerEntry::Start()
{
	// m_bInsideStart is for safety in case a dialog
	// which may pump messages is called from this function
	// (this causes calls to this function again and again
	// from the scheduler timer message)
	if (m_bInsideStart || m_bRunning)
		return;

	// Inside This Function
	m_bInsideStart = TRUE;

	// Is document open?
	CUImagerMultiDocTemplate* pVideoDeviceDocTemplate = ((CUImagerApp*)::AfxGetApp())->GetVideoDeviceDocTemplate();
	POSITION posVideoDeviceDoc = pVideoDeviceDocTemplate->GetFirstDocPosition();
	CVideoDeviceDoc* pDoc = NULL;
	BOOL bDocIsOpen = FALSE;
	while (posVideoDeviceDoc)
	{
		CVideoDeviceDoc* pVideoDeviceDoc = (CVideoDeviceDoc*)(pVideoDeviceDocTemplate->GetNextDoc(posVideoDeviceDoc));
		if (pVideoDeviceDoc)
		{
			if (pVideoDeviceDoc->GetDevicePathName() == m_sDevicePathName)
			{
				pDoc = pVideoDeviceDoc;
				bDocIsOpen = TRUE;
				break;
			}
		}
	}

	// Wait if not yet open
	if (!bDocIsOpen)
	{
		m_bInsideStart = FALSE;
		return;
	}

	// Start recording if not already started
	if (!pDoc->m_pAVRec)
	{
		// Ready?
		if (!pDoc->m_bCaptureStarted)
		{
			m_bInsideStart = FALSE;
			return;
		}

		// Start record
		if (!pDoc->CaptureRecord(FALSE)) // No Message Box on Error					
		{
			m_bInsideStart = FALSE;
			return;
		}
	}

	// Running
	m_bRunning = TRUE;

	// Store The Document Pointer
	m_pDoc = pDoc;

	// Exiting This Function
	m_bInsideStart = FALSE;
}

BOOL CUImagerApp::CSchedulerEntry::Stop()
{
	// m_bInsideStop is for safety in case a dialog
	// which may pump messages is called from this function
	// (this causes calls to this function again and again
	// from the scheduler timer message)
	if (m_bInsideStop)
		return FALSE;	// Do not delete this entry now, wait!

	// Inside This Function
	m_bInsideStop = TRUE;

	// Check
	if (!((CUImagerApp*)::AfxGetApp())->IsDoc(m_pDoc) ||
		!m_bRunning)
	{
		m_bRunning = FALSE;
		m_bInsideStop = FALSE;
		return TRUE;	// Done, delete this entry
	}

	// Stop recording if not already stopped
	if (m_pDoc->m_pAVRec)
		m_pDoc->CaptureRecord(FALSE); // No Message Box on Error
	
	// Not Running
	m_bRunning = FALSE;

	// Exiting This Function
	m_bInsideStop = FALSE;

	return TRUE;		// Done, delete this entry
}

void CUImagerApp::AddSchedulerEntry(CSchedulerEntry* pSchedulerEntry)
{
	// Pointer Check
	if (!pSchedulerEntry)
		return;

	// No Duplicated Entries:
	// It may happen that two Network Get Frame Docs
	// with same IP and Port are scheduled
	// -> Delete the existing one and Add the new one!
	if (pSchedulerEntry->m_Type == CSchedulerEntry::ONCE)
	{
		CSchedulerEntry* p = GetOnceSchedulerEntry(pSchedulerEntry->m_sDevicePathName);
		if (p)
			DeleteOnceSchedulerEntry(p->m_sDevicePathName);
		m_Scheduler.AddTail(pSchedulerEntry);
	}
	else if (pSchedulerEntry->m_Type == CSchedulerEntry::DAILY)
	{
		CSchedulerEntry* p = GetDailySchedulerEntry(pSchedulerEntry->m_sDevicePathName);
		if (p)
			DeleteDailySchedulerEntry(p->m_sDevicePathName);
		m_Scheduler.AddTail(pSchedulerEntry);
	}

	// Store Settings
	CString sSection;
	BOOL bFound = FALSE;
	int nCount = GetProfileInt(_T("GeneralApp"), _T("SchedulerCount"), 0);
		
	// Overwrite?
	for (int i = 0 ; i < nCount ; i++)
	{
		sSection.Format(_T("Scheduler%010d"), i);
		if (pSchedulerEntry->m_sDevicePathName == GetProfileString(sSection, _T("DevicePathName"), _T(""))	&&
			pSchedulerEntry->m_Type == GetProfileInt(sSection, _T("Type"), (int)CSchedulerEntry::NONE))
		{
			bFound = TRUE;
			break;
		}
	}
	if (!bFound)
	{
		// Find empty space?
		for (int i = 0 ; i < nCount ; i++)
		{
			sSection.Format(_T("Scheduler%010d"), i);
			if (GetProfileString(sSection, _T("DevicePathName"), _T("")) == _T(""))
			{
				bFound = TRUE;
				break;
			}
		}
		// Append
		if (!bFound)
		{
			sSection.Format(_T("Scheduler%010d"), nCount);
			nCount++;
			WriteProfileInt(_T("GeneralApp"), _T("SchedulerCount"), nCount);
		}
	}
	WriteProfileString(sSection, _T("DevicePathName"), pSchedulerEntry->m_sDevicePathName);
	WriteProfileInt(sSection, _T("Type"), (int)pSchedulerEntry->m_Type);
	WriteProfileInt(sSection, _T("StartYear"), (int)pSchedulerEntry->m_StartTime.GetYear());
	WriteProfileInt(sSection, _T("StartMonth"), (int)pSchedulerEntry->m_StartTime.GetMonth());
	WriteProfileInt(sSection, _T("StartDay"), (int)pSchedulerEntry->m_StartTime.GetDay());
	WriteProfileInt(sSection, _T("StartHour"), (int)pSchedulerEntry->m_StartTime.GetHour());
	WriteProfileInt(sSection, _T("StartMin"), (int)pSchedulerEntry->m_StartTime.GetMinute());
	WriteProfileInt(sSection, _T("StartSec"), (int)pSchedulerEntry->m_StartTime.GetSecond());
	WriteProfileInt(sSection, _T("StopYear"), (int)pSchedulerEntry->m_StopTime.GetYear());
	WriteProfileInt(sSection, _T("StopMonth"), (int)pSchedulerEntry->m_StopTime.GetMonth());
	WriteProfileInt(sSection, _T("StopDay"), (int)pSchedulerEntry->m_StopTime.GetDay());
	WriteProfileInt(sSection, _T("StopHour"), (int)pSchedulerEntry->m_StopTime.GetHour());
	WriteProfileInt(sSection, _T("StopMin"), (int)pSchedulerEntry->m_StopTime.GetMinute());
	WriteProfileInt(sSection, _T("StopSec"), (int)pSchedulerEntry->m_StopTime.GetSecond());
}

CUImagerApp::CSchedulerEntry* CUImagerApp::GetOnceSchedulerEntry(CString sDevicePathName)
{
	// Iterate Through the Scheduler List
	POSITION pos = m_Scheduler.GetHeadPosition();
	while (pos)
	{
		CSchedulerEntry* pSchedulerEntry = m_Scheduler.GetNext(pos);
		if (pSchedulerEntry &&
			pSchedulerEntry->m_sDevicePathName == sDevicePathName &&
			pSchedulerEntry->m_Type == CSchedulerEntry::ONCE)
			return pSchedulerEntry;
	}

	return NULL;
}

CUImagerApp::CSchedulerEntry* CUImagerApp::GetDailySchedulerEntry(CString sDevicePathName)
{
	// Iterate Through the Scheduler List
	POSITION pos = m_Scheduler.GetHeadPosition();
	while (pos)
	{
		CSchedulerEntry* pSchedulerEntry = m_Scheduler.GetNext(pos);
		if (pSchedulerEntry &&
			pSchedulerEntry->m_sDevicePathName == sDevicePathName &&
			pSchedulerEntry->m_Type == CSchedulerEntry::DAILY)
			return pSchedulerEntry;
	}

	return NULL;
}

void CUImagerApp::DeleteOnceSchedulerEntry(CString sDevicePathName)
{
	// Iterate Through the Scheduler List
	POSITION pos = m_Scheduler.GetHeadPosition();
	while (pos)
	{
		POSITION prevpos = pos;
		CSchedulerEntry* pSchedulerEntry = m_Scheduler.GetNext(pos);
		if (pSchedulerEntry &&
			pSchedulerEntry->m_sDevicePathName == sDevicePathName &&
			pSchedulerEntry->m_Type == CSchedulerEntry::ONCE)
		{
			CString sSection;
			BOOL bFound = FALSE;
			int nCount = GetProfileInt(_T("GeneralApp"), _T("SchedulerCount"), 0);
			for (int i = 0 ; i < nCount ; i++)
			{
				sSection.Format(_T("Scheduler%010d"), i);
				if (sDevicePathName == GetProfileString(sSection, _T("DevicePathName"), _T(""))	&&
					pSchedulerEntry->m_Type == GetProfileInt(sSection, _T("Type"), (int)CSchedulerEntry::NONE))
				{
					WriteProfileString(sSection, _T("DevicePathName"), _T(""));
					WriteProfileInt(sSection, _T("Type"), (int)CSchedulerEntry::ONCE);
					WriteProfileInt(sSection, _T("Rec"), (int)TRUE);
					WriteProfileInt(sSection, _T("StartYear"), 2000);
					WriteProfileInt(sSection, _T("StartMonth"), 1);
					WriteProfileInt(sSection, _T("StartDay"), 1);
					WriteProfileInt(sSection, _T("StartHour"), 12);
					WriteProfileInt(sSection, _T("StartMin"), 0);
					WriteProfileInt(sSection, _T("StartSec"), 0);
					WriteProfileInt(sSection, _T("StopYear"), 2000);
					WriteProfileInt(sSection, _T("StopMonth"), 1);
					WriteProfileInt(sSection, _T("StopDay"), 1);
					WriteProfileInt(sSection, _T("StopHour"), 12);
					WriteProfileInt(sSection, _T("StopMin"), 0);
					WriteProfileInt(sSection, _T("StopSec"), 0);
					break;
				}
			}
			delete pSchedulerEntry;
			m_Scheduler.RemoveAt(prevpos);
			break;
		}
	}
}

void CUImagerApp::DeleteDailySchedulerEntry(CString sDevicePathName)
{
	// Iterate Through the Scheduler List
	POSITION pos = m_Scheduler.GetHeadPosition();
	while (pos)
	{
		POSITION prevpos = pos;
		CSchedulerEntry* pSchedulerEntry = m_Scheduler.GetNext(pos);
		if (pSchedulerEntry &&
			pSchedulerEntry->m_sDevicePathName == sDevicePathName &&
			pSchedulerEntry->m_Type == CSchedulerEntry::DAILY)
		{
			CString sSection;
			BOOL bFound = FALSE;
			int nCount = GetProfileInt(_T("GeneralApp"), _T("SchedulerCount"), 0);
			for (int i = 0 ; i < nCount ; i++)
			{
				sSection.Format(_T("Scheduler%010d"), i);
				if (sDevicePathName == GetProfileString(sSection, _T("DevicePathName"), _T(""))	&&
					pSchedulerEntry->m_Type == GetProfileInt(sSection, _T("Type"), (int)CSchedulerEntry::NONE))
				{
					WriteProfileString(sSection, _T("DevicePathName"), _T(""));
					WriteProfileInt(sSection, _T("Type"), (int)CSchedulerEntry::ONCE);
					WriteProfileInt(sSection, _T("Rec"), (int)TRUE);
					WriteProfileInt(sSection, _T("StartYear"), 2000);
					WriteProfileInt(sSection, _T("StartMonth"), 1);
					WriteProfileInt(sSection, _T("StartDay"), 1);
					WriteProfileInt(sSection, _T("StartHour"), 12);
					WriteProfileInt(sSection, _T("StartMin"), 0);
					WriteProfileInt(sSection, _T("StartSec"), 0);
					WriteProfileInt(sSection, _T("StopYear"), 2000);
					WriteProfileInt(sSection, _T("StopMonth"), 1);
					WriteProfileInt(sSection, _T("StopDay"), 1);
					WriteProfileInt(sSection, _T("StopHour"), 12);
					WriteProfileInt(sSection, _T("StopMin"), 0);
					WriteProfileInt(sSection, _T("StopSec"), 0);
					break;
				}
			}
			delete pSchedulerEntry;
			m_Scheduler.RemoveAt(prevpos);
			break;
		}
	}
}

CString CUImagerApp::GetProfileFullscreenBrowser(LPCTSTR lpszEntry, LPCTSTR lpszDefault/*=NULL*/)
{
	CString sProfileName = ::GetSpecialFolderPath(CSIDL_APPDATA);
	sProfileName += _T("\\") + FULLSCREENBROWSER_INI_FILE;
	return ::GetProfileIniString(_T("General"), lpszEntry, lpszDefault, sProfileName);
}

BOOL CUImagerApp::WriteProfileFullscreenBrowser(LPCTSTR lpszEntry, LPCTSTR lpszValue)
{
	CString sProfileName = ::GetSpecialFolderPath(CSIDL_APPDATA);
	sProfileName += _T("\\") + FULLSCREENBROWSER_INI_FILE;
	CString sProfileNamePath = ::GetDriveAndDirName(sProfileName);
	if (!::IsExistingDir(sProfileNamePath))
		::CreateDir(sProfileNamePath);
	return ::WriteProfileIniString(_T("General"), lpszEntry, lpszValue, sProfileName);
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

void CUImagerApp::OnHelpTutorial(UINT nID)
{
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	TCHAR szProgramName[MAX_PATH];
	if (::GetModuleFileName(NULL, szProgramName, MAX_PATH) == 0)
		return;
	_tsplitpath(szProgramName, szDrive, szDir, NULL, NULL);
	CString sTutorialsPath = CString(szDrive) + CString(szDir);
	sTutorialsPath += _T("Tutorials");
	CSortableFileFind FileFind;
	FileFind.AddAllowedExtension(_T("htm"));
	if (FileFind.Init(sTutorialsPath + _T("\\*")))
	{
		::ShellExecute(	NULL,
						_T("open"),
						FileFind.GetFileName(nID - ID_HELP_TUTORIAL_FIRST),
						NULL, NULL, SW_SHOWNORMAL);
	}
}

void CUImagerApp::OnToolsTrayicon() 
{
	m_bTrayIcon = !m_bTrayIcon;
	::AfxGetMainFrame()->TrayIcon(m_bTrayIcon);
	WriteProfileInt(_T("GeneralApp"),
					_T("TrayIcon"),
					m_bTrayIcon);
}

void CUImagerApp::OnUpdateToolsTrayicon(CCmdUI* pCmdUI) 
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

typedef BOOL (WINAPI * FPCRYPTPROTECTDATA)(DATA_BLOB*, LPCWSTR, DATA_BLOB*, PVOID, CRYPTPROTECT_PROMPTSTRUCT*, DWORD, DATA_BLOB*);
BOOL CUImagerApp::WriteSecureProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue)
{
	HINSTANCE h = ::LoadLibrary(_T("crypt32.dll"));
	if (!h)
		return WriteProfileString(lpszSection, lpszEntry, lpszValue);
	FPCRYPTPROTECTDATA fpCryptProtectData = (FPCRYPTPROTECTDATA)::GetProcAddress(h, "CryptProtectData");
	if (fpCryptProtectData)
	{
		DATA_BLOB blobIn, blobOut, blobEntropy;
		blobIn.pbData = (BYTE*)lpszValue;
		blobIn.cbData = sizeof(TCHAR) * (_tcslen(lpszValue) + 1);
		blobOut.cbData = 0;
		blobOut.pbData = NULL;
		BYTE Entropy[] = {
			0x6B, 0x31, 0x20, 0x85, 0x08, 0x79, 0xA3, 0x1B, 0x53, 0xAB, 0x3D, 0x08, 0x67, 0xFD, 0x55, 0x66, 
			0x26, 0x7B, 0x46, 0x28, 0x91, 0xBB, 0x11, 0x8D, 0x8E, 0xB0, 0x2C, 0x99, 0x1E, 0x5B, 0x4A, 0x68};
		blobEntropy.pbData = Entropy;
		blobEntropy.cbData = sizeof(Entropy);

		if (fpCryptProtectData(	&blobIn,
								L"UNICODE",	// Windows 2000:  This parameter is required and cannot be set to NULL
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
CString CUImagerApp::GetSecureProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault/*=NULL*/)
{
	HINSTANCE h = ::LoadLibrary(_T("crypt32.dll"));
	if (!h)
		return GetProfileString(lpszSection, lpszEntry, lpszDefault);
	FPCRYPTUNPROTECTDATA fpCryptUnprotectData = (FPCRYPTUNPROTECTDATA)::GetProcAddress(h, "CryptUnprotectData");
	if (fpCryptUnprotectData)
	{
		DATA_BLOB blobIn, blobOut, blobEntropy;
		blobIn.cbData = 0;
		blobIn.pbData = NULL;
		blobOut.cbData = 0;
		blobOut.pbData = NULL;
		BYTE Entropy[] = {
			0x6B, 0x31, 0x20, 0x85, 0x08, 0x79, 0xA3, 0x1B, 0x53, 0xAB, 0x3D, 0x08, 0x67, 0xFD, 0x55, 0x66, 
			0x26, 0x7B, 0x46, 0x28, 0x91, 0xBB, 0x11, 0x8D, 0x8E, 0xB0, 0x2C, 0x99, 0x1E, 0x5B, 0x4A, 0x68};
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
