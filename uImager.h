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

// Default toolbar images are 16x15 pixels
// CToolBar::SetSizes() would support enlarging the buttons while keeping the
// standard 16x15 images, but when performing a user switch and then back or
// connecting and disconnecting through RDP the buttons are resized to the
// minimum leaving the toolbar at the previous size ... The solution is to
// enlarge the images while keeping the minimum button increment:
// + 7 pixels on X
// + 6 pixels on Y
// Note: toolbar resource with images different from the standard 16x15 are not
//       supported by the visual studio editor (we placed them in uImager.rc2)
#define TOOLBAR_IMAGE_SIZE_X							22
#define TOOLBAR_IMAGE_SIZE_Y							21
#define TOOLBAR_BUTTON_SIZE_X							(TOOLBAR_IMAGE_SIZE_X+7)
#define TOOLBAR_BUTTON_SIZE_Y							(TOOLBAR_IMAGE_SIZE_Y+6)
#define TOOLBAR_IMAGE_SIZE_2X							44
#define TOOLBAR_IMAGE_SIZE_2Y							42
#define TOOLBAR_BUTTON_SIZE_2X							(TOOLBAR_IMAGE_SIZE_2X+7)
#define TOOLBAR_BUTTON_SIZE_2Y							(TOOLBAR_IMAGE_SIZE_2Y+6)
#define TOOLBAR_IMAGE_SIZE_3X							66
#define TOOLBAR_IMAGE_SIZE_3Y							63
#define TOOLBAR_BUTTON_SIZE_3X							(TOOLBAR_IMAGE_SIZE_3X+7)
#define TOOLBAR_BUTTON_SIZE_3Y							(TOOLBAR_IMAGE_SIZE_3Y+6)
#define TOOLBAR_COMBOBOX_FONTFACENAME					_T("MS Shell Dlg 2")
#define TOOLBAR_COMBOBOX_FONTHEIGHT						-14
#define TOOLBAR_ZOOMCOMBOBOX_WIDTH						80

// Micro Apache
#define MICROAPACHE_DIR									_T("microapache")
#define MICROAPACHE_HTDOCS								(CString(MICROAPACHE_DIR) + CString(_T("\\htdocs")))
#define MICROAPACHE_FILENAME							_T("mapache.exe")
#define MICROAPACHE_RELPATH								(CString(MICROAPACHE_DIR) + CString(_T("\\")) + CString(MICROAPACHE_FILENAME))
#define MICROAPACHE_NUM_PROCESS							2
#define MICROAPACHE_TIMEOUT_MS							15000U
#define MICROAPACHE_STARTUP_TIMEOUT_MS					30000U
#define MICROAPACHE_WAITTIME_MS							100U
#define MICROAPACHE_DEFAULT_PORT						8800
#define MICROAPACHE_DEFAULT_PORT_SSL					8443
#define MICROAPACHE_DEFAULT_CERTFILE_SSL				_T("https.crt")
#define MICROAPACHE_DEFAULT_KEYFILE_SSL					_T("https.key")
#define MICROAPACHE_LOGNAME_EXT							_T("httpd_log.txt")
#define MICROAPACHE_PIDNAME_EXT							_T("httpd_pid.txt")
#define MICROAPACHE_CONFIGNAME_EXT						_T("httpd_conf.txt")
#define MICROAPACHE_EDITABLE_CONFIGNAME_EXT				_T("httpd_conf_editable.txt")

// Mailer
#define MAILPROG_DIR									_T("mail")
#define MAILPROG_FILENAME								_T("mailsend.exe")
#define MAILPROG_RELPATH								(CString(MAILPROG_DIR) + CString(_T("\\")) + CString(MAILPROG_FILENAME))
#define MAILPROG_TIMEOUT_SEC							20 /* connect and read timeouts (both default to 5 sec which is too low) */

// Ftp
#define FTPPROG_DIR										_T("ftp")
#define FTPPROG_FILENAME								_T("lftp.exe")
#define FTPPROG_RELPATH									(CString(FTPPROG_DIR) + CString(_T("\\")) + CString(FTPPROG_FILENAME))
#define FTPPROG_TIMEOUT_SEC								10
#define FTPPROG_JPEGUPLOAD_WAIT_TIMEOUT_MS				10000 /* FTPPROG_JPEGUPLOAD_WAIT_TIMEOUT_MS < NETCOM_BLOCKING_TIMEOUT, see CVideoDeviceChildFrame::OnClose() */

// Php, Css and icons
#define	PHP_CONFIGNAME_EXT								_T("configuration.php")
#define PHP_INDEXROOTDIRNAME_EXT						_T("index_rootdir.php")
#define PHP_INDEXNAME_EXT								_T("index.php")
#define IMAGE_FAVICONNAME_EXT							_T("favicon.ico")
#define IMAGE_CONTAWAREICONNAME_EXT						_T("contaware_256x256.png")
#define JSON_MANIFESTNAME_EXT							_T("manifest.json")
#define	PHP_AUTHENTICATENAME_EXT						_T("authenticate.php")
#define PHP_LANGUAGES_DIR								_T("languages")
#define CSS_STYLE_DIR									_T("styles")
#define CAMERA_IS_OBSCURED_FILENAME						_T("CAMERA_IS_OBSCURED.txt")

// Default wait time before autostarting first device
#define DEFAULT_FIRSTSTART_DELAY_MS						0U

// Maximum number of simultaneous savings
// Note: two cameras saving at the same time is a good value
// for RAM usage (H.264 consumes a lot of heap memory while
// encoding) and optimal threads distribution
#define MOVDET_MAX_SIMULTANEOUS_SAVINGS					2

// Service
#define CONTACAMSERVICE_NOTINSTALLED					0
#define CONTACAMSERVICE_RUNNING							1
#define CONTACAMSERVICE_NOTRUNNING						2
#define CONTACAMSERVICE_TIMEOUT							20000U
#define CONTACAMSERVICE_STARTUP_SLEEP					3000U
#define CONTACAMSERVICE_CONTROL_START_PROC				133
#define CONTACAMSERVICE_CONTROL_END_PROC				134

// That's the maximum for Win95, Win98 and WinMe (bigger bufs are not working)
#define MAX_SECTIONNAMES_BUFFER							65535

// Maximum Number of Email Attached Files
#define MAX_ATTACHMENTS									16

// Big Picture Size Limit
#define BIG_PICTURE_SIZE_LIMIT							134217728U /* 128MB */

// Default Video
#define DEFAULT_VIDEO_FILEEXT							_T(".mp4")
#define DEFAULT_VIDEO_FOURCC							FCC('H264')
#define DEFAULT_VIDEO_QUALITY							VIDEO_QUALITY_GOOD

// Default Audio
#define	DEFAULT_AUDIO_FORMAT_TAG						WAVE_FORMAT_AAC2
#define DEFAULT_AUDIO_BITRATE							192000
#define DEFAULT_AUDIO_CHANNELS							2
#define DEFAULT_AUDIO_SAMPLINGRATE						44100
#define DEFAULT_AUDIO_BITS								16

// Default Jpeg Compression Quality
#ifndef DEFAULT_JPEGCOMPRESSION
#define DEFAULT_JPEGCOMPRESSION							80
#endif

// Maximum size of movement detection snapshot used for mail attachment
#define	MOVDET_MAX_SNAPSHOT_SIZE						640

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
class CUImagerDoc;

class CAboutDlg : public CDialog
{
public:
	CAboutDlg(CWnd* pParent = NULL);

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
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	CStaticLink m_WebLink;	// Hyperlink
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
	// /service      : started from the ContaCamService
	// /p			 : print preview the given file
	// /pt			 : print the given file(s) to the specified printer
	class CUImagerCommandLineInfo : public CCommandLineInfo
	{
		public:
			CUImagerCommandLineInfo();
			~CUImagerCommandLineInfo();

			//plain char* version on UNICODE for source-code backwards compatibility
			virtual void ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast);
			virtual void ParseParam(const char* pszParam, BOOL bFlag, BOOL bLast);

			// For Multiple Files
			CStringArray m_strFileNames;

		protected:
			void ParseParamFlag(const char* pszParam);
			void ParseParamNotFlag(const TCHAR* pszParam);
			void ParseParamNotFlag(const char* pszParam);
			void ParseLast(BOOL bLast);
	};
	void ParseCommandLine(CUImagerCommandLineInfo& rCmdInfo);

	// Command Line Processing
	BOOL ProcessShellCommand(CUImagerCommandLineInfo& rCmdInfo);


#ifdef VIDEODEVICEDOC
	//  Service Control End Process Thread
	class CServiceControlEndProcThread : public CWorkerThread
	{
		public:
			CServiceControlEndProcThread(){;};
			virtual ~CServiceControlEndProcThread(){Kill();};

		protected:
			int Work() {ControlContaCamService(CONTACAMSERVICE_CONTROL_END_PROC); return 0;};
	};
#endif

	// Constructor & Destructor
	CUImagerApp();
	virtual ~CUImagerApp();

	// Returns whether to use the registry or an ini file:
	// -1 : not set
	// 0  : use ini file
	// 1  : use registry
	static int GetConfiguredUseRegistry();

	// Returns the configuration files directory with no trailing backslash,
	// optionally informs whether it was taken from MASTERCONFIG_INI_NAME_EXT
	// (folder will not be created if not existing)
	static CString GetConfigFilesDir(BOOL* pbIsConfigured = NULL);

	// Returns the configured temporary directory with no trailing backslash
	// (folder will not be created if not existing)
	static CString GetConfiguredTempDir();

	// Templates Get Functions
#ifdef VIDEODEVICEDOC
	CUImagerMultiDocTemplate* GetVideoDeviceDocTemplate() {return m_pVideoDeviceDocTemplate;};
#endif
	CUImagerMultiDocTemplate* GetPictureDocTemplate() {return m_pPictureDocTemplate;};
	CUImagerMultiDocTemplate* GetTemplateFromFileExtension(CString sFileName);
	static BOOL IsSupportedPictureFile(CString sFileName);

	// Close All Open Documents
	BOOL CloseAll();
	
	// Start Slideshow opening a new Picture Doc
	CPictureDoc* SlideShow(LPCTSTR sStartDirName, BOOL bRecursive);

	// Is Wait Cursor Showing?
	// Used by the OnSetCursor() functions of the Views
	// to make sure the cursor is not in the wait state
	// when setting it!
	BOOL IsWaitCursor() const {return (m_nWaitCursorCount > 0);};

	// Autostart Enable / Disable
	static BOOL Autostart(BOOL bEnable);
	static BOOL IsAutostart();

	// Compress File or Directory Content to the newly created zip file
	// (no add, overwrite if file existing!)
	static BOOL CompressToZip(LPCTSTR szPath, LPCTSTR szZipFileName);	
	
	// Extracts the archive to the given directory
	static BOOL ExtractZipToDir(LPCTSTR szDirPath, LPCTSTR szZipFileName);

	// Send Email through MAPI.
	// The attachment may be a
	// directory or a single file. If a
	// directory is specified its content
	// is used as attachment.
	BOOL SendMail(LPCTSTR szAttachment);

	// Send Document(s) by e-mail
	void SendOpenDocsAsMail();
	void ShrinkOpenDocs(LPCTSTR szDstDirPath,
						DWORD dwMaxSize,
						BOOL bMaxSizePercent,
						DWORD dwJpegQuality,
						BOOL bPictureExtChange,
						BOOL bShrinkPictures,
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

	// From Source Extension get the Destination Extension
	static CString ShrinkGetDstExt(CString sSrcExt);

	// PaintDocTitles() has to be called when exiting
	// the tray icon minimized state
	BOOL PaintDocTitles();

	// Is At least One Document Open?
	BOOL AreDocsOpen();

	// Is At least One Picture Document Open?
	BOOL ArePictureDocsOpen();
	
#ifdef VIDEODEVICEDOC
	// Is At least One Video Device Document Open?
	BOOL AreVideoDeviceDocsOpen();

	// Get current ContaCamService state
	// returns:	CONTACAMSERVICE_NOTINSTALLED
	//			CONTACAMSERVICE_RUNNING
	//			CONTACAMSERVICE_NOTRUNNING
	static int GetContaCamServiceState();

	// CONTACAMSERVICE_CONTROL_START_PROC
	// CONTACAMSERVICE_CONTROL_END_PROC
	static DWORD ControlContaCamService(int nMsg);

	// Movement detection save reservation queue
	BOOL MovDetSaveReservation(DWORD dwId);
	void MovDetSaveReservationRemove(DWORD dwId);

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
	BOOL SettingsPageAppsDefaults(const CString& sTarget = _T(""));
	void UpdateFileAssociations();
	BOOL IsFileTypeAssociated(CString sExt);
	BOOL AssociateFileType(CString sExt);
	BOOL UnassociateFileType(CString sExt);

	// Get Application Temporary Directory (the returned string ends with a _T('\\'))
	__forceinline CString GetAppTempDir() const {return m_sAppTempDir;};

	// Show color dialog with custom
	// colors store in registry.
	// Inits the dialog with crColor and
	// returns the selected color in crColor.
	BOOL ShowColorDlg(	COLORREF& crColor,
						CWnd* pParentWnd = NULL);

	// INI file / registry additional functions
	BOOL IsExistingSection(const CString& sSection);
	BOOL WriteProfileInt64(LPCTSTR lpszSection, LPCTSTR lpszEntry, __int64 nValue);
	__int64 GetProfileInt64(LPCTSTR lpszSection, LPCTSTR lpszEntry, __int64 nDefault);
	void WriteSecureProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue);
	CString GetSecureProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry);
	
	// Printer
	void InitPrinter();
	int GetCurrentPrinterIndex() {return m_PrinterControl.GetPrinterIndex(m_hDevMode, m_hDevNames);};

	// Capture entire virtual screen to clipboard
	static void CaptureScreenToClipboard();

	// Paste clipboard to given file
	static BOOL PasteToFile(LPCTSTR lpszFileName, COLORREF crBackgroundColor = RGB(255,255,255));

	// Use Tray Icon
	volatile BOOL m_bTrayIcon;

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

	// Cores count
	volatile int m_nCoresCount;

	// Last Opened Directory
	CString m_sLastOpenedDir;

	// The Zip File Handling Class
	CZipArchive m_Zip;

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

#ifdef VIDEODEVICEDOC
	// Flag indicating that the auto-starts have been executed
	volatile BOOL m_bAutostartsExecuted;

	// Wait time before autostarting first device
	volatile DWORD m_dwFirstStartDelayMs;

	// mov/mp4 saving fragmented
	volatile BOOL m_bMovFragmented;

	// Start the Micro Apache server
	BOOL m_bStartMicroApache;

	// Micro Apache Document Root
	CString m_sMicroApacheDocRoot;

	// Micro Apache Server Ports
	int m_nMicroApachePort;
	int m_nMicroApachePortSSL;

	// Authentication
	CString m_sMicroApacheUsername;
	CString m_sMicroApachePassword;

	// SSL certificate files
	CString	m_sMicroApacheCertFileSSL;
	CString	m_sMicroApacheKeyFileSSL;

	// Global flag indicating the necessity to drop
	// movement detection frames in case of memory problems
	volatile BOOL m_bMovDetDropFrames;

	// Movement detection save reservation queue
	typedef CList<DWORD,DWORD> MOVDETSAVERESERVATIONQUEUE;
	MOVDETSAVERESERVATIONQUEUE m_MovDetSaveReservationQueue;
	CRITICAL_SECTION m_csMovDetSaveReservation;

	// Service
	// - Window placement store/restore disabled if this set
	// - Start/stop ContaCam.exe from service disabled if this set
	// - Drawing disabled if this set
	volatile BOOL m_bServiceProcess; // set if SERVICENAME_EXT started us

	// Do start from service
	BOOL m_bDoStartFromService;

	// 0..100: On battery with the given charge remaining
	// 255:    On AC line
	volatile int m_nBatteryOrACLine;
#endif

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUImagerApp)
	public:
	void Pump();
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
	afx_msg void OnUpdateFileCloseall(CCmdUI* pCmdUI);
	afx_msg void OnFileShrinkDirDocs();
	afx_msg void OnFileSendmailOpenDocs();
	afx_msg void OnUpdateFileSendmailOpenDocs(CCmdUI* pCmdUI);
	afx_msg void OnSettingsTrayicon();
	afx_msg void OnUpdateSettingsTrayicon(CCmdUI* pCmdUI);
	afx_msg void OnAppFaq();
	afx_msg void OnAppManual();
	afx_msg void OnUpdateFileNew(CCmdUI* pCmdUI);
	afx_msg void OnSettingsLogNormal();
	afx_msg void OnUpdateSettingsLogNormal(CCmdUI* pCmdUI);
	afx_msg void OnSettingsLogVerbose();
	afx_msg void OnUpdateSettingsLogVerbose(CCmdUI* pCmdUI);
	afx_msg void OnSettingsLogAllMessages();
	afx_msg void OnUpdateSettingsLogAllMessages(CCmdUI* pCmdUI);
	afx_msg void OnSettingsViewLogfile();
	afx_msg void OnSettingsBrowseConfigLogFiles();
	afx_msg void OnUpdateFileShrinkDirDocs(CCmdUI* pCmdUI);
	afx_msg void OnEditScreenshot();
	afx_msg void OnUpdateEditScreenshot(CCmdUI* pCmdUI);
	//}}AFX_MSG
#ifdef VIDEODEVICEDOC
	afx_msg void OnEditDelete();
	afx_msg void OnCaptureNetwork();
	afx_msg void OnFileDxVideoDevice(UINT nID);
#endif
	DECLARE_MESSAGE_MAP()
	
protected:
	CString PictureSlideMakeMsg(CPictureDoc* pDoc);
	CString PictureMakeMsg(CPictureDoc* pDoc);

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

	// Application's Temporary Directory (it is _T('\\') terminated)
	CString m_sAppTempDir;

	// Mutex for the Installer / Uninstaller to check
	// whether this program is running
	HANDLE m_hAppMutex;

	// Gdiplus token
	ULONG_PTR m_GdiplusToken;

	// Doc Templates
#ifdef VIDEODEVICEDOC
	CUImagerMultiDocTemplate* m_pVideoDeviceDocTemplate;
#endif
	CUImagerMultiDocTemplate* m_pPictureDocTemplate;
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UIMAGER_H__FFD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_)
