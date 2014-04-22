// uImager.h : main header file for the UIMAGER application
//

#if !defined(AFX_UIMAGER_H__FFD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_)
#define AFX_UIMAGER_H__FFD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include "ZipArchive.h"
#include "Staticlink.h"
#include "EnumPrinters.h"
#include "MainFrm.h"
#include "WorkerThread.h"
#include "XmpDlg.h"

// Fullscreen Browser
#define FULLSCREENBROWSER_NOEXT							_T("FullscreenBrowser")
#define FULLSCREENBROWSER_INI_NAME_EXT					_T("FullscreenBrowser.ini")
#define FULLSCREENBROWSER_EXE_NAME_EXT					_T("FullscreenBrowser.exe")
#define FULLSCREENBROWSER_INI_FILE						(CString(MYCOMPANY) + CString(_T("\\")) + CString(FULLSCREENBROWSER_NOEXT) + CString(_T("\\")) + CString(FULLSCREENBROWSER_INI_NAME_EXT))
#define FULLSCREENBROWSER_EXITSTRING_ENTRY				_T("ExitString")
#define FULLSCREENBROWSER_DEFAULT_EXITSTRING			_T("")

// Micro Apache
#define MICROAPACHE_DIR									_T("microapache")
#define MICROAPACHE_HTDOCS								(CString(MICROAPACHE_DIR) + CString(_T("\\htdocs")))
#define MICROAPACHE_FILENAME							_T("mapache.exe")
#define MICROAPACHE_RELPATH								(CString(MICROAPACHE_DIR) + CString(_T("\\")) + CString(MICROAPACHE_FILENAME))
#define MICROAPACHE_PWTOOL_FILENAME						_T("htpasswd.exe")
#define MICROAPACHE_PWTOOL_RELPATH						(CString(MICROAPACHE_DIR) + CString(_T("\\")) + CString(MICROAPACHE_PWTOOL_FILENAME))
#define MICROAPACHE_NUM_PROCESS							2
#define MICROAPACHE_TIMEOUT_MS							8000U
#define MICROAPACHE_WAITTIME_MS							100U
#define MICROAPACHE_CANCONNECT_ATTEMPTS					10
#define MICROAPACHE_PREFERRED_PORT						80
#define MICROAPACHE_DEFAULT_PORT						8800
#define MICROAPACHE_LOGNAME_EXT							_T("httpd_log.txt")
#define MICROAPACHE_PIDNAME_EXT							_T("httpd_pid.txt")
#define MICROAPACHE_CONFIGNAME_EXT						_T("httpd_conf.txt")
#define MICROAPACHE_EDITABLE_CONFIGNAME_EXT				_T("httpd_conf_editable.txt")
#define MICROAPACHE_PWNAME_EXT							_T("httpd_pw.txt")
#define MICROAPACHE_LOG_FILE							(CString(MYCOMPANY) + CString(_T("\\")) + CString(APPNAME_NOEXT) + CString(_T("\\")) + CString(MICROAPACHE_LOGNAME_EXT))
#define MICROAPACHE_PID_FILE							(CString(MYCOMPANY) + CString(_T("\\")) + CString(APPNAME_NOEXT) + CString(_T("\\")) + CString(MICROAPACHE_PIDNAME_EXT))
#define MICROAPACHE_CONFIG_FILE							(CString(MYCOMPANY) + CString(_T("\\")) + CString(APPNAME_NOEXT) + CString(_T("\\")) + CString(MICROAPACHE_CONFIGNAME_EXT))
#define MICROAPACHE_EDITABLE_CONFIG_FILE				(CString(MYCOMPANY) + CString(_T("\\")) + CString(APPNAME_NOEXT) + CString(_T("\\")) + CString(MICROAPACHE_EDITABLE_CONFIGNAME_EXT))
#define MICROAPACHE_PW_FILE								(CString(MYCOMPANY) + CString(_T("\\")) + CString(APPNAME_NOEXT) + CString(_T("\\")) + CString(MICROAPACHE_PWNAME_EXT))
#define MICROAPACHE_INDEX_ROOTDIR_FILENAME				_T("index_rootdir.php")
#define MICROAPACHE_LANGUAGES_DIR						_T("languages")
#define MICROAPACHE_STYLE_DIR							_T("styles")
#define MICROAPACHE_JS_DIR								_T("js")
#define MICROAPACHE_DEFAULT_AUTH_AREANAME				_T("Secure Area")

// Php
#define	PHP_CONFIGNAME_EXT								_T("configuration.php")

// Service
#define CONTACAMSERVICE_NOTINSTALLED					0
#define CONTACAMSERVICE_RUNNING							1
#define CONTACAMSERVICE_NOTRUNNING						2
#define CONTACAMSERVICE_TIMEOUT							20000U
#define CONTACAMSERVICE_STARTUP_SLEEP					3000U
#define CONTACAMSERVICE_CONTROL_START_PROC				133
#define CONTACAMSERVICE_CONTROL_END_PROC				134

// System Files
#define THUMBS_DB										_T("Thumbs.db")

// Maximum Number of Email Attached Files
#define MAX_ATTACHMENTS									16

// Big Picture Size Limit
#define BIG_PICTURE_SIZE_LIMIT							134217728U /* 128MB */

// Default Video Compression Quality: 2.0f best, 31.0f worst
#define DEFAULT_VIDEO_QUALITY							4.0f
#define DEFAULT_SHRINK_VIDEO_QUALITY					7.0f

// Default Video Codec
#define DEFAULT_VIDEO_FOURCC							FCC('DIVX')

// Default Video Compression Keyframes Rate
#define DEFAULT_KEYFRAMESRATE							30
							
// Default Video Compression Data Rate in Bits / Sec
#define DEFAULT_VIDEO_DATARATE							500000

// Default Jpeg Compression Quality
#ifndef DEFAULT_JPEGCOMPRESSION
#define DEFAULT_JPEGCOMPRESSION							80
#endif

// Default settings for auto shrink
#define	AUTO_SHRINK_MAX_SIZE							1024

// Coordinate Units
#define COORDINATES_PIX									0
#define COORDINATES_INCH								1
#define COORDINATES_CM									2
#define COORDINATES_MM									3

// New document
#define	DEFAULT_NEW_WIDTH								640
#define	DEFAULT_NEW_HEIGHT								480
#define DEFAULT_NEW_BPP									24
#define DEFAULT_NEW_DPI									300
#define DEFAULT_NEW_PHYS_UNIT							0		/* 0: cm , 1: inch */
#define DEFAULT_NEW_PAPER_SIZE							_T("")	/* _T("") is Custom */
#define DEFAULT_NEW_COLOR								RGB(255,255,255)

// Forward Declarations
class CVideoDeviceDoc;
class CPictureDoc;
class CVideoAviDoc;
class CUImagerDoc;
class CProgressDlg;
class CDiscRecorder;

// Interface ICDBurn of IMAPI
#ifndef __ICDBurn_FWD_DEFINED__
#define __ICDBurn_FWD_DEFINED__
typedef interface ICDBurn ICDBurn;

// [unique][uuid][object] 
const IID IID_ICDBurn =    {0x3d73a659,0xe5d0,0x4d42,{0xaf,0xc0,0x51,0x21,0xba,0x42,0x5c,0x8d}};
const CLSID CLSID_CDBurn = {0xfbeb8a05,0xbeee,0x4442,{0x80,0x4e,0x40,0x9d,0x6c,0x45,0x15,0xe9}};
   
MIDL_INTERFACE("3d73a659-e5d0-4d42-afc0-5121ba425c8d")
ICDBurn : public IUnknown
{
	public:
		virtual HRESULT STDMETHODCALLTYPE GetRecorderDriveLetter( 
											LPWSTR pszDrive, // [size_is][out]
											UINT cch) = 0; // [in]
    
		virtual HRESULT STDMETHODCALLTYPE Burn( 
											HWND hwnd) = 0; // [in]
    
		virtual HRESULT STDMETHODCALLTYPE HasRecordableDrive( 
											BOOL *pfHasRecorder) = 0; // [out]
};
#endif 	/* __ICDBurn_FWD_DEFINED__ */

#define ABOUTDLG_TIMER_MS		3000U
class CAboutDlg : public CDialog
{
public:
	CAboutDlg(BOOL bClickableLinks = TRUE);

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
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void DisplayMemStats();
	BOOL m_bClickableLinks;
	CStaticLink m_WebLink;		// Hyperlink
};

class CUImagerMultiDocTemplate : public CMultiDocTemplate
{
	DECLARE_DYNAMIC(CUImagerMultiDocTemplate)

// Constructors
public:
	CUImagerMultiDocTemplate(UINT nIDResource, CRuntimeClass* pDocClass,
		CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass);

// Implementation
public:
	virtual ~CUImagerMultiDocTemplate();
	virtual CDocument* OpenDocumentFile(LPCTSTR lpszPathName, BOOL bMakeVisible = TRUE);
};

class CUImagerApp : public CWinApp
{

public:
	// Command Line Parsing
	//
	// Supported Switches (they are case-sensitive!):
	//
	// /slideshow	 : start in slideshow only mode
	// /service      : started from the ContaCamService
	// /extracthere  : zip extraction to zip's directory
	// /play         : play given audio or video file
	// /close        : close app after finishing playing the audio file
	// /hide         : hide mainframe
	// /p			 : print preview open given file
	// /pt			 : print the given file to the specified printer
	class CUImagerCommandLineInfo : public CCommandLineInfo
	{
		public:
			CUImagerCommandLineInfo();
			~CUImagerCommandLineInfo();

			//plain char* version on UNICODE for source-code backwards compatibility
			virtual void ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast);
			virtual void ParseParam(const char* pszParam, BOOL bFlag, BOOL bLast);

			BOOL DoStartSlideShow() {return m_bStartSlideShow;};

			// For Multiple Files
			CStringArray m_strFileNames;

		protected:
			BOOL m_bStartSlideShow;

			void ParseParamFlag(const char* pszParam);
			void ParseParamNotFlag(const TCHAR* pszParam);
			void ParseParamNotFlag(const char* pszParam);
			void ParseLast(BOOL bLast);
	};
	void ParseCommandLine(CUImagerCommandLineInfo& rCmdInfo);

	// Command Line Processing
	BOOL ProcessShellCommand(CUImagerCommandLineInfo& rCmdInfo);


#ifdef VIDEODEVICEDOC
	// Micro Apache Watch Dog Thread
	class CMicroApacheWatchdogThread : public CWorkerThread
	{
		public:
			CMicroApacheWatchdogThread() {;};
			virtual ~CMicroApacheWatchdogThread() {Kill();};

		protected:
			int Work();
	};

	// Scheduler Entry Class
	class CSchedulerEntry
	{
		public:
			enum SchedulerEntryType {NONE, ONCE, DAILY, WEEKLY};

			CSchedulerEntry();
			virtual ~CSchedulerEntry(){;};

			// Start
			void Start();

			// Stop
			BOOL Stop();

			// The Scheduler Entry Type
			SchedulerEntryType m_Type;

			// Device Path Name Identifier
			CString m_sDevicePathName;

			// The Document
			CVideoDeviceDoc* m_pDoc;

			// Are we Running?
			BOOL m_bRunning;

			// The Times
			CTime m_StartTime;
			CTime m_StopTime;

		protected:
			BOOL m_bInsideStart;
			BOOL m_bInsideStop;
	};

	// Scheduler List Type
	typedef CList<CSchedulerEntry*,CSchedulerEntry*> SCHEDULERLIST;

	// Scheduler
	SCHEDULERLIST m_Scheduler;
	void AddSchedulerEntry(CSchedulerEntry* pSchedulerEntry);
	CSchedulerEntry* GetOnceSchedulerEntry(CString sDevicePathName);
	CSchedulerEntry* GetDailySchedulerEntry(CString sDevicePathName);
	void DeleteOnceSchedulerEntry(CString sDevicePathName);
	void DeleteDailySchedulerEntry(CString sDevicePathName);
#endif

	// Constructor & Destructor
	CUImagerApp();
	virtual ~CUImagerApp();

	// Templates Get Functions
	CUImagerMultiDocTemplate* GetVideoAviDocTemplate() {return m_pVideoAviDocTemplate;};
#ifdef VIDEODEVICEDOC
	CUImagerMultiDocTemplate* GetVideoDeviceDocTemplate() {return m_pVideoDeviceDocTemplate;};
#endif
	CUImagerMultiDocTemplate* GetPictureDocTemplate() {return m_pPictureDocTemplate;};
	CUImagerMultiDocTemplate* GetAudioMCIDocTemplate() {return m_pAudioMCIDocTemplate;};
	CUImagerMultiDocTemplate* GetCDAudioDocTemplate() {return m_pCDAudioDocTemplate;};
	CUImagerMultiDocTemplate* GetTemplateFromFileExtension(CString sFileName);
	static BOOL IsSupportedPictureFile(CString sFileName);
	static BOOL IsAVIFile(CString sFileName);
	static BOOL IsSWFFile(CString sFileName);
	static BOOL IsSupportedMusicFile(CString sFileName);
	static BOOL IsSupportedCDAudioFile(CString sFileName);

	// Suggest Direct X Version 7 or higher
	void SuggestDirectXVersion7();

	// Require Direct X Version 7 or higher
	BOOL RequireDirectXVersion7();

	// Close All Open Documents
	BOOL CloseAll();
	
	// Start Slideshow opening a new Picture Doc
	CPictureDoc* SlideShow(	LPCTSTR sStartDirName,
							BOOL bFullscreen,
							BOOL bRunSlideshow,
							BOOL bRecursive);
	// IMAPI
	BOOL HasRecordableDrive(ICDBurn* pICDBurn = NULL);
	CString GetRecorderDriveLetter(ICDBurn* pICDBurn = NULL);
	__forceinline CString GetBurnFolderPath();
	BOOL BurnDirContent(CString sDir);

	// IMAPI2
	// Note on annoying Bug:
	// Leaving the previously modal CBatchProcDlg open for more than 15-20
	// minutes and then closing it generated a strange WM_QUIT which exited
	// the main message loop. CBatchProcDlg when opened used to call the
	// IDiscMaster2's get_IsSupportedEnvironment() which internally initializes
	// all devices with IDiscRecorder2. I simulated the get_IsSupportedEnvironment()
	// behavior and tracked down the problem to the CoCreateInstance() of
	// the IDiscRecorder2 interface. I could not figure out what the problem
	// was, but making the CBatchProcDlg and the CIMAPI2Dlg dialogs modeless
	// and initializing the IDiscRecorder2 interfaces only once in the main
	// UI thread solved the problem -> call FreeDiscRecorders2() only once
	// in ExitInstance()!
	BOOL InitDiscRecorders2();
	void FreeDiscRecorders2();

	// Is Wait Cursor Showing?
	// Used by the OnSetCursor() functions of the Views
	// to make sure the cursor is not in the wait state
	// when setting it!
	BOOL IsWaitCursor() const {return (m_nWaitCursorCount > 0);};

	// Autostart Enable / Disable
	static BOOL Autostart(BOOL bEnable);
	static BOOL IsAutostart();

	// Zip File Handling

	// Get the Unique Top Directory (if any)
	// and the Zip File entries count (the count is files + dirs)
	// On error a MessageBox is displayed and 0 is returned
	int GetUniqueTopDirAndCount(CString sZipFileName, CString& sTopDir);

	// Compress File or Directory Content to the newly created zip file
	// (no add, overwrite if file existing!)
	BOOL CompressToZip(LPCTSTR szPath, LPCTSTR szZipFileName);

	// Popsup a Browse-Dir Dialog, extracts the archive to the selected dir and
	// returns the extraction directory or _T("") if an error occurred.
	CString ExtractZip(	LPCTSTR szZipFileName,
						volatile int* pPictureFilesCount = NULL); // *pPictureFilesCount has to be set to 0 by the caller!	
	
	// Extracts the archive to the given directory
	static BOOL ExtractZipToDir(LPCTSTR szDirPath,
								LPCTSTR szZipFileName,
								volatile int* pPictureFilesCount = NULL); // *pPictureFilesCount has to be set to 0 by the caller!

	// Send Email through MAPI.
	// The attachment may be a
	// directory or a single file. If a
	// directory is specified its content
	// is used as attachment.
	BOOL SendMail(LPCTSTR szAttachment);

	// Send Document(s) by e-mail
	void SendOpenDocsAsMailInit();
	void SendDocAsMailFinish(BOOL bOk);

	// Return Value:
	// -1 : Not Finished
	// 0  : Error
	// 1  : Ok
	int ShrinkOpenDocs(	LPCTSTR szDstDirPath,
						DWORD dwMaxSize,
						BOOL bMaxSizePercent,
						DWORD dwJpegQuality,
						BOOL bPictureExtChange,
						BOOL bShrinkPictures,
						BOOL bShrinkVideos,
						BOOL bOnlyCopyFiles);

	// Shrink Picture
	// (Called From UI Thread & Processing Thread)
	//
	// The bProgressSend parameter specifies whether to use
	// SendMessage or PostMessage For Progress Messages
	//
	// Return Value:
	// -1 : Just Copied
	// 0  : Error
	// 1  : Shrinked
	static int ShrinkPicture(	LPCTSTR szSrcFileName,
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
								CWorkerThread* pThread);

	// Set Shrink Status Text
	// (Only To be used From Main UI Thread!)
	void ShrinkStatusText(	CString sSrcFileName,
							CString sDstFileName);

	// From Source Extension get the Destination Extension
	static CString ShrinkGetDstExt(CString sSrcExt);

	// PaintDocTitles() has to be called when exiting
	// the tray icon minimized state
	BOOL PaintDocTitles();

	// Is At least One Document Open?
	BOOL AreDocsOpen();

	// Returns the count of all open documents
	int GetOpenDocsCount();

	// Is At least One Picture Document Open?
	BOOL ArePictureDocsOpen();

	// Is At least One Video Avi Document Open?
	BOOL AreVideoAviDocsOpen();
	
#ifdef VIDEODEVICEDOC
	// Is At least One Video Device Document Open?
	BOOL AreVideoDeviceDocsOpen();

	// Returns the count of all open Video Device Documents
	int GetTotalVideoDeviceDocs();

	// Get the total number of Video Device Documents
	// that have the movement detection enabled
	int GetTotalVideoDeviceDocsMovementDetecting();

	// Autorun Video Devices
	BOOL AutorunVideoDevicesDoWait(int nRetryCount);
	void AutorunVideoDevices(int nRetryCount = 0);

	// Get current ContaCamService state
	// returns:	CONTACAMSERVICE_NOTINSTALLED
	//			CONTACAMSERVICE_RUNNING
	//			CONTACAMSERVICE_NOTRUNNING
	static int GetContaCamServiceState();

	// CONTACAMSERVICE_CONTROL_START_PROC
	// CONTACAMSERVICE_CONTROL_END_PROC
	static DWORD ControlContaCamService(int nMsg);

	// Broser autostart
	void BrowserAutostart();

	// FullscreenBrowser Ini file handling
	CString GetProfileFullscreenBrowser(LPCTSTR lpszEntry, LPCTSTR lpszDefault = NULL);
	BOOL WriteProfileFullscreenBrowser(LPCTSTR lpszEntry, LPCTSTR lpszValue);

	// Enumerate all configured (in registry or ini file) devices
	void EnumConfiguredDevicePathNames(CStringArray& DevicePathNames);
#endif

	// Is the Given Doc Available?
	// Shows the reason of the unavailability of the current doc
	// if bShowMsgBoxIfNotAvailable is TRUE
	//
	// Not available means when dialogs open, when doc modified,
	// when a command will be executed with the full load of the picture,
	// when in print preview, when playing, when processing
	// (does not check m_pDib->IsValid(), which means fully loaded)
	BOOL IsDocAvailable(	CDocument* pDoc,
							BOOL bShowMsgBoxIfNotAvailable = FALSE);

	// Is the Given Doc Ready to Change Picture?
	// Shows the reason of the impossibility to slide
	// if bShowMsgBoxIfSlideNotPossible is TRUE
	//
	// Not ready to slide means when dialogs open, when doc modified,
	// when a command will be executed with the full load of the picture,
	// when in print preview
	// (does not check m_pDib->IsValid(), which means fully loaded)
	BOOL IsDocReadyToSlide(	CPictureDoc* pDoc,
							BOOL bShowMsgBoxIfSlideNotPossible = FALSE);

	// Is the given picture size (in bytes) to be considered Big?
	BOOL IsPictureSizeBig(DWORD dwImageSize);

	// Profile Settings
	void LoadPlacement(UINT showCmd = SW_SHOWNORMAL);	// pass SW_HIDE to override the stored placement (otherwise this parameter is ignored)
	void LoadSettings(UINT showCmd = SW_SHOWNORMAL);	// showCmd is handed over to LoadPlacement()
	void SavePlacement();								// store main window placement
	void SavePlacements();								// store main window placement + placement of all open video device documents

	// Video device finishes recording,
	// the most important settings are stored
	void SaveOnEndSession();

	// Is pDoc a Valid Document?
	BOOL IsDoc(CDocument* pDoc);

	// File Association Functions
	void UpdateFileAssociations();
	BOOL IsFileTypeAssociated(CString sExt);
	BOOL AssociateFileType(CString sExt, BOOL* pbHasUserChoice = NULL);
	BOOL UnassociateFileType(CString sExt);

	// Get Application Temporary Directory (the returned string ends with a _T('\\'))
	__forceinline CString GetAppTempDir() const {
#ifdef VIDEODEVICEDOC
		if (m_bUseCustomTempFolder)
		{
			CString sCustomTempDir = m_sMicroApacheDocRoot;
			sCustomTempDir.TrimRight(_T('\\'));
			sCustomTempDir += _T("\\Temp\\");
			if (!::IsExistingDir(sCustomTempDir))
			{
				if (!::CreateDir(sCustomTempDir))
					return m_sSysTempDir; // on failure return sys temp folder
			}
			return sCustomTempDir;
		}
		else
#endif
			return m_sSysTempDir;
	};

	// Show color dialog with custom
	// colors store in registry.
	// Inits the dialog with crColor and
	// returns the selected color in crColor.
	BOOL ShowColorDlg(	COLORREF& crColor,
						CWnd* pParentWnd = NULL);

	// Encrypted Write / Read to the registry
	BOOL WriteSecureProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue);
	CString GetSecureProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault = NULL);
	
	// Printer
	void InitPrinter();
	int GetCurrentPrinterIndex() {return m_PrinterControl.GetPrinterIndex(m_hDevMode, m_hDevNames);};

	// Capture entire virtual screen to clipboard
	static void CaptureScreenToClipboard();

	// Paste clipboard to given file
	static BOOL PasteToFile(LPCTSTR lpszFileName, COLORREF crBackgroundColor = RGB(255,255,255));

	// Application Runs as Slideshow-Only Program
	BOOL m_bSlideShowOnly;

	// Use Tray Icon
	BOOL m_bTrayIcon;

	// First time that the App runs after Install (or Upgrade)
	BOOL m_bFirstRun;

	// First time ever that the App runs or after a uninstall
	BOOL m_bFirstRunEver;

	// Silent install
	BOOL m_bSilentInstall;

	// Hide Mainframe
	// m_nCmdShow only could be used,
	// but DDE uses this variable as a pointer in the
	// Parse & Process of the command line!
	BOOL m_bHideMainFrame;

	// We need to be a separate instance
	// - Autorun capture devices disabled if this set
	// - Microapache start/stop disabled if this set
	// - General settings dialog disabled if this set (only for VIDEODEVICEDOC compile)
	// - Browser autostart disabled if this set
	// - Window placement store/restore disabled if this set
	// - Start/stop ContaCam.exe from service disabled if this set
	BOOL m_bForceSeparateInstance;

	// Single Instance Application Wanted
	BOOL m_bSingleInstance;

	// Top Most
	BOOL m_bTopMost;

	// Flag indicating whether it's possible to store window placements
	BOOL m_bCanSavePlacements;

	// Application is Shutting down
	BOOL m_bShuttingDownApplication;

	// Closing All
	BOOL m_bClosingAll;

	// Should the CPictureDoc Load Jpeg Faster by
	// loading a smaller version to the preview dib?
	// Usually enabled. After a while
	// the CJpegThread will automatically
	// load the full-sized dib.
	BOOL m_bUseLoadPreviewDib;

	// Placement Settings have been loaded
	BOOL m_bPlacementLoaded;

	// Ending session because of shutdown or log-off
	volatile BOOL m_bEndSession;

	// Cores count
	int m_nCoresCount;

	// Last Opened Directory
	CString m_sLastOpenedDir;

	// Display Advanced On-Screen Video Avi Info
	BOOL m_bVideoAviInfo;

	// Flags to execute final steps after video shrinking
	volatile BOOL m_bWaitingMailFinish;

	// The Zip File Handling Class
	CZipArchive m_Zip;

	// Do Extract Zip File Here Flag
	BOOL m_bExtractHere;

	// Start playing given file
	BOOL m_bStartPlay;

	// Close application after audio play of given file has terminated
	BOOL m_bCloseAfterAudioPlayDone;

	// File Open Preview Flag
	BOOL m_bFileDlgPreview;

	// Printers Enumeration
	BOOL m_bPrinterInit;
	CEnumPrinters m_PrinterControl;

	// MAPI Mail Support Available Flag
	BOOL m_bMailAvailable;

	// Xmp Dialogs
	CXmpDlg* m_pXmpImportDlg;
	CXmpDlg* m_pXmpDlg;

	// When starting program open document in full screen mode
	BOOL m_bStartFullScreenMode;

	// ESC to exit the program
	BOOL m_bEscExit;

	// Disable opening external program (for pdf, swf)
	BOOL m_bDisableExtProg;

	// Scan Vars
	int m_nPdfScanCompressionQuality;
	CString m_sScanToTiffFileName;
	CString m_sScanToPdfFileName;

	// Coordinate Units
	int m_nCoordinateUnit;

	// New Dialog
	int m_nNewWidth;
	int m_nNewHeight;
	int m_nNewXDpi;
	int m_nNewYDpi;
	int m_nNewPhysUnit;
	CString m_sNewPaperSize;
	COLORREF m_crNewBackgroundColor;

	// Start MDI Child Maximized Flag
	BOOL m_bStartMaximized;

	// IMAPI2 disc recorders array
	typedef CArray<CDiscRecorder*,CDiscRecorder*> DISCRECORDERARRAY;
	DISCRECORDERARRAY m_DiscRecorders2;

#ifdef VIDEODEVICEDOC
	// Use integrated Fullscreen Browser
	BOOL m_bFullscreenBrowser;
	BOOL m_bBrowserAutostart;
	CString m_sFullscreenBrowserExitString;

	// Priority to IPv6
	volatile BOOL m_bIPv6;

	// Device Autostart delay
	volatile DWORD m_dwAutostartDelayMs;

	// Use Custom Temp Folder
	volatile BOOL m_bUseCustomTempFolder;

	// Start the Micro Apache server
	BOOL m_bStartMicroApache;

	// Micro Apache Server has been started
	BOOL m_bMicroApacheStarted;

	// Micro Apache Document Root
	CString m_sMicroApacheDocRoot;

	// Micro Apache Server Port
	int m_nMicroApachePort;

	// Micro Apache Authentication
	BOOL m_bMicroApacheDigestAuth;
	CString m_sMicroApacheAreaname;
	CString m_sMicroApacheUsername;
	CString m_sMicroApachePassword;

	// Micro Apache Watchdog Thread
	CMicroApacheWatchdogThread m_MicroApacheWatchdogThread;

	// Service
	// - ContaCam's browser autostart disabled if this set
	//   (in this case a registry Run key is set to start the browser
	//    and that's made in BrowserAutostart() by the UI process)
	// - Window placement store/restore disabled if this set
	// - Start/stop ContaCam.exe from service disabled if this set
	// - Drawing disabled if this set
	volatile BOOL m_bServiceProcess;	// Set if SERVICENAME_EXT started us

	// Do start from service
	BOOL m_bDoStartFromService;
#endif

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUImagerApp)
	public:
	virtual BOOL InitInstance();
	virtual CDocument* OpenDocumentFile(LPCTSTR lpszFileName);
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CUImagerApp)
	afx_msg void OnAppAbout();
	afx_msg void OnFileOpen();
	afx_msg void OnFileOpenDir();
	afx_msg void OnFileNew();
	afx_msg void OnFileCloseall();
	afx_msg void OnEditPaste();
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
	afx_msg void OnFileSettings();
	afx_msg void OnAppLicense();
	afx_msg void OnAppCredits();
	afx_msg void OnUpdateFileSettings(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileCloseall(CCmdUI* pCmdUI);
	afx_msg void OnFileShrinkDirDocs();
	afx_msg void OnFileSendmailOpenDocs();
	afx_msg void OnUpdateFileSendmailOpenDocs(CCmdUI* pCmdUI);
	afx_msg void OnToolsAvimergeAs();
	afx_msg void OnToolsTrayicon();
	afx_msg void OnUpdateToolsTrayicon(CCmdUI* pCmdUI);
	afx_msg void OnAppFaq();
	afx_msg void OnUpdateFileNew(CCmdUI* pCmdUI);
	afx_msg void OnToolsViewLogfile();
	afx_msg void OnUpdateFileShrinkDirDocs(CCmdUI* pCmdUI);
	afx_msg void OnEditScreenshot();
	afx_msg void OnUpdateEditScreenshot(CCmdUI* pCmdUI);
	//}}AFX_MSG
	afx_msg void OnHelpTutorial(UINT nID);
#ifdef VIDEODEVICEDOC
	afx_msg void OnToolsMoveCamFolders();
	afx_msg void OnEditDelete();
	afx_msg void OnCaptureNetwork();
	afx_msg void OnFileDxVideoDevice(UINT nID);
#endif
	DECLARE_MESSAGE_MAP()
	
protected:

#ifndef CPJNSMTP_NOSSL
	static CCriticalSection* m_pOpenSSLCritSections;
	static void __cdecl OpenSSLLockingCallback(int mode, int type, const char* file, int line);
#endif

	CString PictureSlideMakeMsg(CPictureDoc* pDoc);
	CString PictureMakeMsg(CPictureDoc* pDoc);
	CString VideoAviMakeMsg(CVideoAviDoc* pDoc);

	// Show Message Box saying that the given
	// File Type (extension) is not supported
	void  FileTypeNotSupportedMessageBox(LPCTSTR lpszFileName);

	// Return Value:
	// 0  : Error
	// 1  : Ok
	// Shrink All Pages of a Multi-Page Tiff
	static int ShrinkPictureMultiPage(	LPCTSTR szSrcFileName,
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
										CWorkerThread* pThread);

	static BOOL CalcShrink(	const CDib& SrcDib,
							DWORD dwMaxSize,
							BOOL bMaxSizePercent,
							DWORD& dwResizeWidth,
							DWORD& dwResizeHeight);

	// System's Temporary Directory (it is _T('\\') terminated)
	CString m_sSysTempDir;

	// Mutex for the Installer / Uninstaller to check
	// whether this program is running
	HANDLE m_hAppMutex;
	
	// Shrinking & Email Sending Vars
	CString m_sZipFile;
	CString m_sShrinkDestination;

	// Autorun progress dialog
#ifdef VIDEODEVICEDOC
	CProgressDlg* m_pAutorunProgressDlg;
#endif

	// Doc Templates
	CUImagerMultiDocTemplate* m_pVideoAviDocTemplate;
#ifdef VIDEODEVICEDOC
	CUImagerMultiDocTemplate* m_pVideoDeviceDocTemplate;
#endif
	CUImagerMultiDocTemplate* m_pAudioMCIDocTemplate;
	CUImagerMultiDocTemplate* m_pCDAudioDocTemplate;
	CUImagerMultiDocTemplate* m_pPictureDocTemplate;
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UIMAGER_H__FFD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_)
