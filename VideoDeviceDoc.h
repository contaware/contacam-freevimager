#if !defined(AFX_VIDEODEVICEDOC_H__24956A13_6E82_4959_931F_F512BE9B17A0__INCLUDED_)
#define AFX_VIDEODEVICEDOC_H__24956A13_6E82_4959_931F_F512BE9B17A0__INCLUDED_

#pragma once

#ifdef VIDEODEVICEDOC

// Includes
#include "uImagerDoc.h"
#include "WorkerThread.h"
#include "NetCom.h"
#include "AVRec.h"
#include "AVDecoder.h"
#include "MJPEGEncoder.h"
#include "YuvToRgb.h"
#include "HelpersAudio.h"
#include "SortableFileFind.h"
#include "HostPortDlg.h"
#include "SendMailConfigurationDlg.h"
#include "FTPUploadConfigurationDlg.h"
extern "C"
{
#include "libavutil/opt.h"
#include "libavutil/mathematics.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
}

// Forward Declarations
class CVideoDeviceView;
class CVideoDeviceChildFrame;
class CDxCapture;
class CGeneralPage;
class CSnapshotPage;
class CCameraAdvancedSettingsPropertySheet;
class CMovementDetectionPage;

// General Settings
#define MIN_FRAMERATE						0.1			// fps
#define MAX_FRAMERATE						95.0		// fps
#define PROCESS_MAX_FRAMETIME				15000U		// ms, make sure that: 1000 / MIN_FRAMERATE < PROCESS_MAX_FRAMETIME
#define STARTUP_SETTLE_TIME_SEC				3			// movdet and rec startup delay in sec (especially to sync. audio / video)
#define DEFAULT_FRAMERATE					10.0		// fps
#define HTTPSERVERPUSH_DEFAULT_FRAMERATE	4.0			// fps
#define HTTPSERVERPUSH_EDIMAX_DEFAULT_FRAMERATE	3.0		// fps
#define HTTPCLIENTPOLL_DEFAULT_FRAMERATE	1.0			// fps
#define MAX_DEVICE_AUTORUN_KEYS				128			// Maximum number of devices that can autorun at start-up
#define ACTIVE_VIDEO_STREAM					0			// Video stream 0 for recording and detection
#define ACTIVE_AUDIO_STREAM					0			// Audio stream 0 for recording and detection
#define	FILES_DELETE_INTERVAL_MIN			600000	 	// in ms -> 10min
#define	FILES_DELETE_INTERVAL_RANGE			300000		// in ms -> each [10min,15min[ check whether we can delete old files
#define AUDIO_IN_MIN_BUF_SIZE				256			// bytes
#define AUDIO_MAX_LIST_SIZE					1024		// make sure that: 1 / MIN_FRAMERATE < AUDIO_IN_MIN_BUF_SIZE * AUDIO_MAX_LIST_SIZE / 11025
														// (see CCaptureAudioThread::OpenInAudio())
#define AUDIO_UNCOMPRESSED_BUFS_COUNT		16			// Number of audio buffers
#define AUDIO_RECONNECTION_DELAY			1000U		// ms
#define FRAME_USER_FLAG_MOTION				0x01		// mark the frame as a motion frame
#define FRAME_USER_FLAG_DEINTERLACE			0x02		// mark the frame as being deinterlaced
#define FRAME_USER_FLAG_ROTATE180			0x04		// mark the frame as being rotated by 180°
#define FRAME_USER_FLAG_LAST				0x08		// mark the frame as being the last frame of the detection sequence
#define DEFAULT_DEL_RECS_OLDER_THAN_DAYS	31			// by default delete recordings older than a month
#define MIN_DISK_FREE_PERMILLION			50000		// 5%
#define DEFAULT_VIDEO_FILEEXT				_T(".mp4")	// default file extension
#define DEFAULT_EMAIL_SUBJECT				_T("%name%: %date% %time% %note%")
#define PLACEMENT_THRESHOLD_PIXELS			50			// to make sure that the saved placement is visible

// Frame tag, thumb message and draw message
#define FRAMETAG_REFWIDTH					640
#define FRAMETAG_REFHEIGHT					480
#define FRAMETIME_COLOR						RGB(0,0xFF,0)
#define FRAMEDATE_COLOR						RGB(0x80,0x80,0xFF)
#define FRAMECOUNT_COLOR					RGB(0xFF,0xFF,0xFF)
#define THUMBMESSAGE_FONTSIZE				8
#define DRAW_BKG_COLOR						RGB(0,0,0)
#define DRAW_MESSAGE_COLOR					RGB(0xFF,0xFF,0xFF)
#define DRAW_MESSAGE_SUCCESS_COLOR			RGB(0,0xFF,0)
#define DRAW_MESSAGE_ERROR_COLOR			RGB(0xFF,0,0)

// Process Frame Stop Engine
#define PROCESSFRAME_MAX_RETRY_TIME			3500		// maximum retry time in ms for Process Frame Stop Engine
#define PROCESSFRAME_CAMERABASICSETTINGS	0x01
#define PROCESSFRAME_DXFORMATDIALOG			0x02
#define PROCESSFRAME_DVFORMATDIALOG			0x04
#define PROCESSFRAME_CHANGEFRAMERATE		0x08
#define PROCESSFRAME_DXOPEN					0x10
#define PROCESSFRAME_DXREPLUGGED			0x20
#define PROCESSFRAME_CLOSE					0x40

// Watch Dog
#define WATCHDOG_CHECK_TIME					1000U		// ms
#define WATCHDOG_THRESHOLD					30000U		// ms, make sure that: 1000 / MIN_FRAMERATE < WATCHDOG_THRESHOLD
#define WATCHDOG_ALERT_SEND_TIMEOUT			900			// sec, 15 min

// Snapshot
#define MIN_SNAPSHOT_RATE					1			// one snapshot per second
#define DEFAULT_SNAPSHOT_RATE				1			// each given seconds
#define DEFAULT_SNAPSHOT_HISTORY_FRAMERATE	15			// fps
#define MIN_SNAPSHOT_HISTORY_FRAMERATE		1			// fps
#define MAX_SNAPSHOT_HISTORY_FRAMERATE		95			// fps
#define DEFAULT_SNAPSHOT_LIVE_JPEGNAME		_T("snapshot")
#define DEFAULT_SNAPSHOT_LIVE_JPEGTHUMBNAME	_T("snapshot_thumb")
#define DEFAULT_SNAPSHOT_COMPR_QUALITY		60			// 0 Worst Quality, 100 Best Quality 
#define DEFAULT_SNAPSHOT_THUMB_WIDTH		228			// Must be a multiple of 4 because of video
#define DEFAULT_SNAPSHOT_THUMB_HEIGHT		172			// Must be a multiple of 4 because of video
#define DEFAULT_SERVERPUSH_POLLRATE_MS		200			// ms

// Movement Detection
#define DEFAULT_PRE_BUFFER_MSEC				2000		// ms
#define DEFAULT_POST_BUFFER_MSEC			6000		// ms
#define MOVDET_MIN_LENGTH_MSEC				1000		// Minimum detection length in ms, below this value SaveFrameList() is not called
#define MOVDET_BUFFER_COMPRESSIONQUALITY	4			// 2: best quality, 31: worst quality
#define DEFAULT_MOVDET_LEVEL				50			// Detection level default value (1 .. 100 = Max Sensitivity)
#define DEFAULT_MOVDET_INTENSITY_LIMIT		25			// Intensity difference default value
#define MOVDET_MAX_ZONES_BLOCK_SIZE			1024		// Subdivide settings in blocks (MOVDET_MAX_ZONES must be a multiple of this)
#define MOVDET_MAX_ZONES					8192		// Maximum number of zones
#define MOVDET_MIN_ZONES_XORY				4			// Minimum number of zones in X or Y direction
#define MOVDET_ZONE_FORMAT					_T("DoMovementDetection%03i")
#define MOVDET_ZONES_BLOCK_FORMAT			_T("MovDetZones%i")
#define MOVDET_SAVEFRAMES_POLL				1000U		// ms
#define MOVDET_MIN_FRAMES_IN_LIST			10			// Min. frames in list before saving the list in the
														// case of insufficient memory
#define MOVDET_MAX_FRAMES_IN_LIST			15000		// Default maximum frames per list
#define MOVDET_SAVE_MIN_FRAMERATE_RATIO		0.3			// Min ratio between calculated (last - first) and m_dEffectiveFrameRate
#define MOVDET_TIMEOUT						1000U		// Timeout in ms for detection zones
#define MOVDET_ANIMGIF_MAX_FRAMES			60			// Maximum number of frames per animated gif
#define MOVDET_ANIMGIF_MAX_LENGTH			6000.0		// ms, MOVDET_ANIMGIF_MAX_LENGTH / MOVDET_ANIMGIF_MAX_FRAMES must be >= 100
#define MOVDET_ANIMGIF_DELAY				500.0		// ms (frame time)
#define MOVDET_ANIMGIF_FIRST_FRAME_DELAY	1000		// ms (first frame time)
#define MOVDET_ANIMGIF_LAST_FRAME_DELAY		1000		// ms (last frame time)
#define MOVDET_ANIMGIF_DIFF_MINLEVEL		10			// determines the "inter-frame-compression" of animated gifs
														// higher values better compression but worse quality
#define MOVDET_ANIMGIF_DEFAULT_WIDTH		128			// Default animated gif width
#define MOVDET_ANIMGIF_DEFAULT_HEIGHT		96			// Default animated gif height
#define MOVDET_DETECTING_ZONES_COLOR		RGB(0xFF,0x00,0x00)
#define MOVDET_SELECTED_ZONES_COLOR			RGB(0x00,0x00,0xFF)
#define MOVDET_SENSITIVITY_BKGCOLOR			RGB(0xEE,0xEE,0xEE)
#define MOVDET_MIX_THRESHOLD				4.0			// Above this engine frequency switch from 3To1 to the 7To1 mixer
#define MOVDET_WANTED_FREQ					5.0			// Wanted motion detection engine frequency (calculations / sec)
														// Half of DEFAULT_FRAMERATE

// configuration.php
#define PHPCONFIG_VERSION					_T("VERSION")
#define PHPCONFIG_DEFAULTPAGE				_T("DEFAULTPAGE")
#define PHPCONFIG_SUMMARYSNAPSHOT_PHP		_T("summarysnapshot.php")
#define PHPCONFIG_SNAPSHOT_PHP				_T("snapshot.php")
#define PHPCONFIG_SNAPSHOTFULL_PHP			_T("snapshotfull.php")
#define PHPCONFIG_SNAPSHOTHISTORY_PHP		_T("snapshothistory.php")
#define PHPCONFIG_SUMMARYIFRAME_PHP			_T("summaryiframe.php")
#define PHPCONFIG_SUMMARYTITLE				_T("SUMMARYTITLE")
#define PHPCONFIG_SNAPSHOTTITLE				_T("SNAPSHOTTITLE")
#define PHPCONFIG_SNAPSHOTNAME				_T("SNAPSHOTNAME")
#define PHPCONFIG_SNAPSHOTTHUMBNAME			_T("SNAPSHOTTHUMBNAME")
#define PHPCONFIG_SNAPSHOTHISTORY_THUMB		_T("SNAPSHOTHISTORY_THUMB")
#define PHPCONFIG_SNAPSHOTREFRESHSEC		_T("SNAPSHOTREFRESHSEC")
#define PHPCONFIG_SERVERPUSH_POLLRATE_MS	_T("SERVERPUSH_POLLRATE_MS")
#define PHPCONFIG_THUMBWIDTH				_T("THUMBWIDTH")
#define PHPCONFIG_THUMBHEIGHT				_T("THUMBHEIGHT")
#define PHPCONFIG_WIDTH						_T("WIDTH")
#define PHPCONFIG_HEIGHT					_T("HEIGHT")
#define PHPCONFIG_MAX_PER_PAGE				_T("MAX_PER_PAGE")
#define PHPCONFIG_SHOW_TRASH_COMMAND		_T("SHOW_TRASH_COMMAND")
#define PHPCONFIG_LANGUAGEFILEPATH			_T("LANGUAGEFILEPATH")
#define PHPCONFIG_STYLEFILEPATH				_T("STYLEFILEPATH")
#define PHPCONFIG_MIN_THUMSPERPAGE			7
#define PHPCONFIG_MAX_THUMSPERPAGE			39
#define PHPCONFIG_DEFAULT_THUMSPERPAGE		27

// Http Networking
#define DEFAULT_TCP_PORT					80
#define HTTP_MAX_HEADER_SIZE				1400		// bytes
#define HTTP_MAX_MULTIPART_BOUNDARY			128			// boundary string buffer size in bytes
#define HTTP_MIN_MULTIPART_SIZE				256			// minimum size of a multipart content
#define HTTP_MAX_MULTIPART_SIZE				8388608		// 8 MB		
#define HTTP_DEFAULT_VIDEO_QUALITY			30			// 0 Best Quality, 100 Worst Quality
#define HTTP_DEFAULT_VIDEO_SIZE_CX			640			// pixels
#define HTTP_DEFAULT_VIDEO_SIZE_CY			480			// pixels
#define HTTP_RECONNECTION_DELAY				5000U		// in case of a unhandled HTTP return code wait that time in ms before reconnecting
#define HTTP_MAXPOLLS_ALARM1				30			// Maximum polling connections: alarm level 1
#define HTTP_MAXPOLLS_ALARM2				40			// Maximum polling connections: alarm level 2
#define HTTP_MAXPOLLS_ALARM3				50			// Maximum polling connections: alarm level 3
#define HTTP_THREAD_DEFAULT_DELAY			500U		// ms
#define HTTP_THREAD_MIN_DELAY_ALARM1		100U		// ms
#define HTTP_THREAD_MIN_DELAY_ALARM2		400U		// ms
#define HTTP_THREAD_MIN_DELAY_ALARM3		1000U		// ms
#define HTTP_THREAD_MAX_DELAY_ALARM			((DWORD)(1000.0 / MIN_FRAMERATE)) // ms
#define HTTP_CONNECTION_TIMEOUT				60			// Connection timeout in sec, used:
														// - remove the oldest connections in network poll mode
														// - network setup try connecting timeout
														// - network reconnect in watchdog
#define HTTP_MIN_KEEPALIVE_REQUESTS			50			// Keep-alive support check
#define HTTP_USERNAME_PLACEHOLDER			_T("[USERNAME]") // only use letters, numbers, uri unreserved or uri reserved chars
#define HTTP_PASSWORD_PLACEHOLDER			_T("[PASSWORD]") // only use letters, numbers, uri unreserved or uri reserved chars

// The Document Class
class CVideoDeviceDoc : public CUImagerDoc
{
public:
	// Types
	typedef CList<CDib::LIST*,CDib::LIST*> DIBLISTLIST;
	typedef CList<CNetCom*,CNetCom*> NETCOMLIST;
	
	// Email Attachment Type
	enum AttachmentType
	{
		ATTACHMENT_NONE			= 0,
		ATTACHMENT_VIDEO		= 1,
		ATTACHMENT_GIF			= 2,
		ATTACHMENT_JPG			= 3,
		ATTACHMENT_JPG_VIDEO	= 4,
		ATTACHMENT_JPG_GIF		= 5
	};

	// The Http Networking Parser & Processor Class
	class CHttpParseProcess : public CNetCom::CParseProcess
	{
		public:
			CHttpParseProcess(CVideoDeviceDoc* pDoc) {m_pDoc = pDoc; m_dwCNonceCount = 0U; Clear();};
			virtual ~CHttpParseProcess() {FreeAVCodec();};
			void Close() {FreeAVCodec(); Clear();};
			BOOL SendRawRequest(CString sRequest);
			BOOL SendRequest();
			virtual BOOL Parse(CNetCom* pNetCom, BOOL bLastCall);
			virtual void Process(unsigned char* pLinBuf, int nSize);
			BOOL HasResolution(const CSize& Size);
			
			CArray<CSize,CSize> m_Sizes;
			CString m_sRealm;
			CString m_sQop;
			CString m_sNonce;
			CString m_sAlgorithm;
			CString m_sOpaque;
			DWORD m_dwCNonceCount;
			volatile enum {FORMATVIDEO_UNKNOWN = 0, FORMATVIDEO_JPEG, FORMATVIDEO_MJPEG} m_FormatType;
			typedef enum {AUTHNONE = 0, AUTHBASIC, AUTHDIGEST} AUTHTYPE;
			volatile AUTHTYPE m_AnswerAuthorizationType; // authorization type chosen by parsing WWW-Authenticate header
			volatile BOOL m_bQueryVideoProperties;
			volatile BOOL m_bSetVideoResolution;
			volatile BOOL m_bSetVideoCompression;
			volatile BOOL m_bSetVideoFramerate;
			volatile BOOL m_bFirstProcessing;
			volatile BOOL m_bTryConnecting;
			volatile BOOL m_bConnectionKeepAlive;
			volatile BOOL m_bPollNextJpeg;
			volatile BOOL m_bOldVersion;

		protected:
			void Clear() {	m_bMultipartNoLength = FALSE;
							m_nMultipartBoundaryLength = 0;
							m_bFirstProcessing = TRUE;
							m_bTryConnecting = FALSE;
							m_bQueryVideoProperties = FALSE;
							m_bSetVideoResolution = FALSE;
							m_bSetVideoCompression = FALSE;
							m_bSetVideoFramerate = FALSE;
							m_bPollNextJpeg = FALSE;
							m_FormatType = FORMATVIDEO_UNKNOWN;
							m_AnswerAuthorizationType = AUTHNONE;
							m_bOldVersion = FALSE;
							m_bConnectionKeepAlive = FALSE;
							m_pCodec = NULL;
							m_pCodecCtx = NULL;
							m_pFrame = NULL;
							m_pFrameI420 = NULL;
							m_pImgConvertCtx = NULL;
							m_pI420Buf = NULL;						
							m_dwI420BufSize = 0;
							m_dwI420ImageSize = 0;};
			BOOL ParseSingle(	BOOL bLastCall,
								int nSize,
								const CString& sMsg,
								const CString& sMsgLowerCase);
			BOOL ParseMultipart(CNetCom* pNetCom,
								int nPos,
								int nSize,
								const char* pMsg,
								const CString& sMsg,
								const CString& sMsgLowerCase);
			__forceinline int FindMultipartBoundary(int nPos,
													int nSize,
													const char* pMsg);
			__forceinline int FindSOI(	int nPos,
										int nSize,
										const char* pMsg);
			__forceinline int FindEndOfLine(const CString& s,
											int nStart);
			BOOL OpenAVCodec();
			void FreeAVCodec();
			BOOL InitImgConvert();
			BOOL DecodeVideo(AVPacket* avpkt);
			
			CVideoDeviceDoc* m_pDoc;
			BOOL m_bMultipartNoLength;
			CString m_sLastRequest;
			CString m_sMultipartBoundary;
			char m_szMultipartBoundary[HTTP_MAX_MULTIPART_BOUNDARY];
			int m_nMultipartBoundaryLength;
			AVCodec* m_pCodec;
			AVCodecContext* m_pCodecCtx;
			AVFrame* m_pFrame;
			AVFrame* m_pFrameI420;
			SwsContext* m_pImgConvertCtx;
			LPBYTE m_pI420Buf;						
			DWORD m_dwI420BufSize;
			DWORD m_dwI420ImageSize;
	};
	typedef CList<CHttpParseProcess*,CHttpParseProcess*> NETCOMPARSEPROCESSLIST;

	// The Record Audio File Thread Class
	class CCaptureAudioThread : public CWorkerThread
	{
		public:
			// Functions
			CCaptureAudioThread();
			virtual ~CCaptureAudioThread();
			void AudioInSourceDialog();
			void SetDoc(CVideoDeviceDoc* pDoc) {m_pDoc = pDoc;};
			
		protected:
			// Functions
			int Loop();
			int Work();
			BOOL OpenInAudio();
			void CloseInAudio();
			BOOL DataInAudio();

			// Vars
			HWAVEIN m_hWaveIn;
			CVideoDeviceDoc* m_pDoc;
			WAVEHDR m_WaveHeader[AUDIO_UNCOMPRESSED_BUFS_COUNT];
			HANDLE m_hWaveInEvent;
			HANDLE m_hEventArray[2];
			unsigned int m_uiWaveInBufPos;
			LPBYTE m_pUncompressedBuf[AUDIO_UNCOMPRESSED_BUFS_COUNT];
			DWORD m_dwUncompressedBufSize;
			CAudioTools* m_pAudioTools;
			CAudioPlay* m_pAudioPlay;
	};

	// Http Thread
	class CHttpThread : public CWorkerThread
	{
		public:
			CHttpThread() {	m_pDoc = NULL;
							m_dwVideoConnectDelayMs	= 0U;									// Delay before starting video connection
							m_hEventArray[0]	= GetKillEvent();							// Kill Event
							m_hEventArray[1]	= ::CreateEvent(NULL, TRUE, FALSE, NULL);	// Http Setup Video Connection Event
							m_hEventArray[2]	= ::CreateEvent(NULL, TRUE, FALSE, NULL);	// Http Video Connected Event
							m_hEventArray[3]	= ::CreateEvent(NULL, TRUE, FALSE, NULL);	// Http Video Connect Failed Event
							m_hEventArray[4]	= ::CreateEvent(NULL, TRUE, FALSE, NULL);	// Http Video Read Event
							::InitializeCriticalSection(&m_csVideoConnectRequestParams);};
			virtual ~CHttpThread() {Kill();
									::CloseHandle(m_hEventArray[1]);
									::CloseHandle(m_hEventArray[2]);
									::CloseHandle(m_hEventArray[3]);
									::CloseHandle(m_hEventArray[4]);
									::DeleteCriticalSection(&m_csVideoConnectRequestParams);};
			void SetDoc(CVideoDeviceDoc* pDoc) {m_pDoc = pDoc;};
			__forceinline BOOL SetEventVideoConnect(LPCTSTR lpszRequest = _T(""), DWORD dwConnectDelayMs = 0U)
			{
				::EnterCriticalSection(&m_csVideoConnectRequestParams);
				m_sVideoRequest = CString(lpszRequest);
				m_dwVideoConnectDelayMs = dwConnectDelayMs;
				::LeaveCriticalSection(&m_csVideoConnectRequestParams);
				return ::SetEvent(m_hEventArray[1]);	
			};

		protected:
			int Work();
			int OnError(BOOL bCloseDocument);
			__forceinline BOOL Connect(	CNetCom* pNetCom,
										CHttpParseProcess* pParseProcess,
										int nSocketFamily,
										HANDLE hConnectedEvent = NULL,
										HANDLE hConnectFailedEvent = NULL,
										HANDLE hReadEvent = NULL);
			BOOL PollAndClean(BOOL bDoNewPoll);
			void CleanUpAllConnections();
			CVideoDeviceDoc* m_pDoc;
			HANDLE m_hEventArray[5];
			volatile DWORD m_dwVideoConnectDelayMs;
			CString m_sVideoRequest;
			CRITICAL_SECTION m_csVideoConnectRequestParams;
			NETCOMLIST m_HttpVideoNetComList;
			NETCOMPARSEPROCESSLIST m_HttpVideoParseProcessList;
	};

	// Http Thread
	class CRtspThread : public CWorkerThread
	{
		public:
			CRtspThread() { m_pDoc = NULL; m_dwConnectDelayMs = 0U; m_nVideoCodecID = -1; m_nAudioCodecID = -1; };
			virtual ~CRtspThread() {Kill();};
			void SetDoc(CVideoDeviceDoc* pDoc) { m_pDoc = pDoc; };
			CString m_sURL;
			DWORD m_dwConnectDelayMs;
			volatile int m_nVideoCodecID;
			volatile int m_nAudioCodecID;

		protected:
			int Work();
			CVideoDeviceDoc* m_pDoc;
	};

	// Watch Dog Thread
	class CWatchdogThread : public CWorkerThread
	{
		public:
			CWatchdogThread() {m_pDoc = NULL; m_bPollCaptureStarted = TRUE;};
			virtual ~CWatchdogThread() {Kill();};
			void SetDoc(CVideoDeviceDoc* pDoc) {m_pDoc = pDoc;};

		protected:
			int Work();
			CVideoDeviceDoc* m_pDoc;
			BOOL m_bPollCaptureStarted;
	};

	// Delete Thread
	class CDeleteThread : public CWorkerThread
	{
		public:
			CDeleteThread(){m_pDoc = NULL;};
			virtual ~CDeleteThread(){Kill();};
			void SetDoc(CVideoDeviceDoc* pDoc) {m_pDoc = pDoc;};

		protected:
			int Work();
			BOOL CalcOldestDir(	CSortableFileFind& FileFind,
								CTime& OldestDirTime,
								const CTime& CurrentTime);
			BOOL DeleteOld(	CSortableFileFind& FileFind,
							LONGLONG llDeleteOlderThanDays,
							const CTime& CurrentTime);
			CVideoDeviceDoc* m_pDoc;
	};

	// The Save Frame List Thread Class
	class CSaveFrameListThread : public CWorkerThread
	{
		public:
			CSaveFrameListThread(){	m_pDoc = NULL; m_pFrameList = NULL; m_nNumFramesToSave = 0;
									m_nSaveProgress = 100; m_bWorking = FALSE;};
			virtual ~CSaveFrameListThread() {Kill();};
			void SetDoc(CVideoDeviceDoc* pDoc) {m_pDoc = pDoc;};
			__forceinline int GetSaveProgress() const {return m_nSaveProgress;};
			__forceinline BOOL IsWorking() const {return m_bWorking;};

		protected:
			void LoadAndDecodeFrame(CDib* pDib);
			int Work();
			BOOL SaveSingleGif(		CDib* pDib,
									const CString& sGIFFileName,
									RGBQUAD* pGIFColors,
									const CTime& RefTime,
									DWORD dwRefUpTime);
			void AnimatedGifInit(	RGBQUAD* pGIFColors,
									double& dDelayMul,
									double& dSpeedMul,
									double dCalcFrameRate,
									const CTime& RefTime,
									DWORD dwRefUpTime);
			__forceinline void To255Colors(	CDib* pDib,
											RGBQUAD* pGIFColors,
											const CTime& RefTime,
											DWORD dwRefUpTime);
			BOOL SaveAnimatedGif(	CDib* pGIFSaveDib,
									CDib* pGIFDib,
									CDib** ppGIFDibPrev,
									const CString& sGIFFileName, 
									BOOL* pbFirstGIFSave,
									BOOL bLastGIFSave,
									double dDelayMul,
									double dSpeedMul,
									RGBQUAD* pGIFColors,
									int nDiffMinLevel,
									const CTime& RefTime,
									DWORD dwRefUpTime);
			void FTPUploadMovementDetection(const CTime& Time,
											const CString& sVideoFileName,
											const CString& sGIFFileName);

			CVideoDeviceDoc* m_pDoc;
			CDib::LIST* m_pFrameList;
			int m_nNumFramesToSave;
			CAVDecoder m_AVDetDecoder;
			volatile int m_nSaveProgress;
			volatile BOOL m_bWorking;
	};

	// The Save Snapshot Video Thread Class
	class CSaveSnapshotVideoThread : public CWorkerThread
	{
		public:
			CSaveSnapshotVideoThread(){m_pDoc = NULL; m_ThreadExecutedForTime = CTime(0);};
			virtual ~CSaveSnapshotVideoThread() {Kill();};
			void SetDoc(CVideoDeviceDoc* pDoc) {m_pDoc = pDoc;};

			BOOL m_bSnapshotHistoryJpeg;
			BOOL m_bSnapshotHistoryVideoFtp;
			float m_fSnapshotVideoCompressorQuality;
			CString m_sSnapshotVideoFileExt;
			double m_dSnapshotHistoryFrameRate;
			CTime m_Time;
			CTime m_ThreadExecutedForTime;
			CString m_sSnapshotAutoSaveDir;
			FTPUploadConfigurationStruct m_Config;

		protected:
			CVideoDeviceDoc* m_pDoc;
			int Work();
			__forceinline CString MakeVideoHistoryFileName();
	};

	// The Save Snapshot Thread Class
	class CSaveSnapshotThread : public CWorkerThread
	{
		public:
			CSaveSnapshotThread(){m_pDoc = NULL;};
			virtual ~CSaveSnapshotThread() {Kill();};
			void SetDoc(CVideoDeviceDoc* pDoc) {m_pDoc = pDoc;};

			CDib m_Dib;
			BOOL m_bSnapshotHistoryJpeg;
			BOOL m_bSnapshotHistoryJpegFtp;
			BOOL m_bShowFrameTime;
			int m_nRefFontSize;
			BOOL m_bSnapshotLiveJpegFtp;
			int m_nSnapshotThumbWidth;
			int m_nSnapshotThumbHeight;
			int m_nSnapshotCompressionQuality;
			CTime m_Time;
			CString m_sSnapshotAutoSaveDir;
			CString m_sSnapshotLiveJpegName;
			CString m_sSnapshotLiveJpegThumbName;
			FTPUploadConfigurationStruct m_Config;

		protected:
			CVideoDeviceDoc* m_pDoc;
			CMJPEGEncoder m_MJPEGEncoder;
			CMJPEGEncoder m_MJPEGThumbEncoder;
			int Work();
			__forceinline CString MakeJpegHistoryFileName();
	};

protected: // create from serialization only
	DECLARE_DYNCREATE(CVideoDeviceDoc)
	CVideoDeviceDoc();
	virtual ~CVideoDeviceDoc();
	

// Public Functions
public:
	
	// General Functions
	static void ConnectErr(LPCTSTR lpszText, const CString& sDevicePathName, const CString& sDeviceName); // Called when a device start fails
	void CloseDocument();														// Close Document by sending a WM_CLOSE to the Parent Frame
	CString GetAssignedDeviceName();											// Get User Assigned Device Name
	static CString GetHostFromDevicePathName(const CString& sDevicePathName);	// Returns host name or _T("") if it's not a network device
	CString GetDeviceName();													// Friendly Device Name
	CString GetDevicePathName();												// Used For Settings, Scheduler and Autorun
	static CString GetNetworkDevicePathName(const CString& sGetFrameVideoHost,	// GetDevicePathName() calls this for network devices
											int nGetFrameVideoPort,
											const CString& sGetFrameLocation,
											int nNetworkDeviceTypeMode);
	void SetDocumentTitle();
	CVideoDeviceView* GetView() const {return m_pView;};
	void SetView(CVideoDeviceView* pView) {m_pView = pView;};
	CVideoDeviceChildFrame* GetFrame() const {return m_pFrame;};
	void SetFrame(CVideoDeviceChildFrame* pFrame) {m_pFrame = pFrame;};
	static BOOL CreateCheckYearMonthDayDir(CTime Time, CString sBaseDir, CString& sYearMonthDayDir);

	// Open Dx Video Device
	BOOL OpenDxVideoDevice(int nId);

	// Open Network Video Device
	BOOL OpenNetVideoDevice(CHostPortDlg* pDlg);
	BOOL OpenNetVideoDevice(CString sAddress, DWORD dwConnectDelayMs = 0U);

	// Connect to the chosen Networking Type and Mode
	// ATTENTION: DO NOT CHANGE THE ASSOCIATED NUMBERS!
	// (SP: Server Push, CP: Client Poll, RTSP: Realtime streaming protocol)
	typedef enum {
		OTHERONE_SP = 0,		// Other HTTP motion jpeg devices
		OTHERONE_CP,			// Other HTTP jpeg snapshots devices
		AXIS_SP,				// Axis HTTP motion jpeg
		AXIS_CP,				// Axis HTTP jpeg snapshots
		PANASONIC_SP,			// Panasonic HTTP motion jpeg
		PANASONIC_CP,			// Panasonic HTTP jpeg snapshots
		PIXORD_SP,				// Pixord HTTP motion jpeg
		PIXORD_CP,				// Pixord HTTP jpeg snapshots
		EDIMAX_SP,				// Edimax HTTP motion jpeg
		EDIMAX_CP,				// Edimax HTTP jpeg snapshots
		TPLINK_SP,				// TP-Link HTTP motion jpeg
		TPLINK_CP,				// TP-Link HTTP jpeg snapshots
		FOSCAM_SP,				// Foscam HTTP motion jpeg
		FOSCAM_CP,				// Foscam HTTP jpeg snapshots
		// Add more http devices here...
		URL_RTSP = 1000,		// URL RTSP
		OTHERONE_RTSP = 1010,	// Other RTSP device
		ABUS_RTSP = 1020,		// Abus RTSP
		ACTI_RTSP = 1030,		// ACTi RTSP
		AMCREST_RTSP = 1035,	// Amcrest RTSP
		ARECONT_RTSP = 1040,	// Arecont Vision RTSP
		AXIS_RTSP = 1050,		// Axis RTSP
		BOSCH_RTSP = 1060,		// Bosch RTSP
		CANON_RTSP = 1070,		// Canon RTSP
		DLINK_RTSP = 1080,		// D-Link RTSP
		DAHUA_RTSP = 1090,		// Dahua RTSP
		EDIMAX_RTSP = 1110,		// Edimax RTSP
		FOSCAM_RTSP = 1120,		// Foscam RTSP
		HIKVISION_RTSP = 1130,	// Hikvision RTSP
		LINKSYS_RTSP = 1140,	// Linksys RTSP
		LOGITECH_RTSP = 1150,	// Logitech RTSP
		MONACOR_RTSP = 1154,	// Monacor 3MP/4MP/5MP/.. Series RTSP
		MONACOR_2MP_RTSP = 1155,// Monacor 2MP Series RTSP
		PANASONIC_RTSP = 1160,	// Panasonic RTSP
		PIXORD_RTSP = 1170,		// Pixord RTSP
		PLANET_RTSP = 1180,		// Planet RTSP
		SAMSUNG_RTSP = 1190,	// Samsung RTSP
		SONY_RTSP = 1200,		// Sony RTSP
		SRICAM_RTSP = 1205,		// Sricam RTSP
		TOSHIBA_RTSP = 1210,	// Toshiba RTSP
		TPLINK_RTSP = 1220,		// TP-Link RTSP
		TRENDNET_RTSP = 1230,	// TrendNet RTSP
		UBIQUITI_RTSP = 1235,	// Ubiquiti RTSP
		VIVOTEK_RTSP = 1240,	// Vivotek RTSP
		XIAOMI_RTSP = 1245,		// Xiaomi RTSP
		YCAM_RTSP = 1250,		// Y-cam RTSP
		ZAVIO_RTSP = 1255,		// Zavio RTSP
		ZMODO_RTSP = 1260,		// Zmodo RTSP
		// Add more rtsp devices here...
		LAST_DEVICE				// Placeholder for range check
	} NetworkDeviceTypeMode;
	BOOL ConnectHttp(DWORD dwConnectDelayMs = 0U);
	BOOL ConnectRtsp(DWORD dwConnectDelayMs = 0U);

	// Dialogs
	void CaptureCameraBasicSettings();
	void VideoFormatDialog();

	// Frame Tags
	static CTime CalcTime(DWORD dwUpTime, const CTime& RefTime, DWORD dwRefUpTime);
	static void AddFrameTime(CDib* pDib, CTime RefTime, DWORD dwRefUpTime, int nRefFontSize);
	static void AddFrameCount(CDib* pDib, int nCount, int nRefFontSize);
	
	// Function called when the directx video grabbing format has been changed
	void OnChangeDxVideoFormat();

	// On Change Frame Rate
	void OnChangeFrameRate();

	// Changes the snapshot rate
	void SnapshotRate(double dRate);
	
	// List Convention
	//
	// Tail <-> New
	// Head <-> Old
	//

	// Detection lists
	__forceinline void OneEmptyFrameList();							// One empty frame list
	__forceinline void ClearMovementDetectionsList();				// Free and remove all lists
	__forceinline void RemoveOldestMovementDetectionList();			// Free and remove oldest list
	__forceinline void SaveFrameList(BOOL bDetectionSequenceDone);	// Add new empty list to tail

	// Detection list handling
	__forceinline void ClearFrameList(CDib::LIST* pFrameList);		// Free all frames in list
	__forceinline void ClearNewestFrameList();						// Free all frames in newest list
	__forceinline void ShrinkNewestFrameList();						// Free oldest frames from newest frame list
																	// making the list m_nMilliSecondsRecBeforeMovementBegin long
	__forceinline int GetNewestMovementDetectionsListCount();		// Get the newest list's count
	__forceinline CDib* AllocMJPGFrame(CDib* pDib,					// Allocate new MJPG frame compressing pDib or copying pMJPGData if available
								LPBYTE pMJPGData, DWORD dwMJPGSize);// (copies also audio samples)
	__forceinline void AddNewFrameToNewestList(CDib* pDib,			// Add new frame to newest list
								LPBYTE pMJPGData, DWORD dwMJPGSize);
	__forceinline void AddNewFrameToNewestListAndShrink(CDib* pDib,	// Add new frame to newest list leaving in the list
								LPBYTE pMJPGData, DWORD dwMJPGSize);// m_nMilliSecondsRecBeforeMovementBegin of frames
																	
	// Main Decode & Process Functions
	void ProcessOtherFrame(LPBYTE pData, DWORD dwSize);
	void ProcessNV12Frame(LPBYTE pData, DWORD dwSize, BOOL bFlipUV);
	void ProcessM420Frame(LPBYTE pData, DWORD dwSize);
	void ProcessI420Frame(LPBYTE pData, DWORD dwSize, LPBYTE pMJPGData, DWORD dwMJPGSize);
	static BOOL Rotate180(CDib* pDib);
	static BOOL Deinterlace(CDib* pDib);
	BOOL IsInMovDetSchedule(const CTime& Time);

	// To Start / Stop Frame Processing and Avoid Dead-Locks!
	__forceinline void StopProcessFrame(DWORD dwMask) {		::EnterCriticalSection(&m_csProcessFrameStop);
															m_dwStopProcessFrame |= dwMask;		// set
															::LeaveCriticalSection(&m_csProcessFrameStop);};
	__forceinline void StartProcessFrame(DWORD dwMask) {	::EnterCriticalSection(&m_csProcessFrameStop);
															m_dwStopProcessFrame &= ~dwMask;	// clear
															::LeaveCriticalSection(&m_csProcessFrameStop);};
	__forceinline BOOL IsProcessFrameStopped(DWORD dwMask) {::EnterCriticalSection(&m_csProcessFrameStop);
															BOOL res = ((m_dwProcessFrameStopped & dwMask) == dwMask);
															::LeaveCriticalSection(&m_csProcessFrameStop);
															return res;};
	
	// Audio listen
	BOOL AudioListen(	LPBYTE pData, DWORD dwSizeInBytes,
						CAudioTools* pAudioTools, CAudioPlay* pAudioPlay);

	// Video / Audio Recording
	BOOL MakeAVRec(CAVRec** ppAVRec);
	void OpenVideoFile(const CString& sFileName);
	BOOL CaptureRecord();
	BOOL NextVideoFile();
	void NextRecTime(CTime t);
	void CloseAndShowVideoFile();
	void FreeVideoFile();
	static void WaveInitFormat(WORD wCh, DWORD dwSampleRate, WORD wBitsPerSample, LPWAVEFORMATEX pWaveFormat);
	void UpdateDstWaveFormat();
	UINT EffectiveCaptureAudioDeviceID();
	static CString CaptureAudioDeviceIDToName(UINT uiID);
	static UINT CaptureAudioNameToDeviceID(const CString& sName);

	// Fast bicubic resize
	// Source and destination Dibs must already have the bits allocated! 
	static BOOL ResizeFast(CDib* pSrcDib, CDib* pDstDib);

	// Fast jpeg saving
	// Quality ranges from 0 to 100 (0: worst, 100: best)
	static BOOL SaveJpegFast(CDib* pDib, CMJPEGEncoder* pMJPEGEncoder, const CString& sFileName, int quality);

	// Movement Detection
	void MovementDetectionProcessing(	CDib* pDib,
										LPBYTE pMJPGData,
										DWORD dwMJPGSize,
										DWORD dwVideoProcessorMode,
										BOOL b1SecTick);
	BOOL MovementDetector(CDib* pDib, int nDetectionLevel);
	void FreeMovementDetector();
	void ExecCommandMovementDetection(	BOOL bReplaceVars = FALSE,
										CTime StartTime = CTime(0),
										const CString& sVideoFileName = _T(""),
										const CString& sGIFFileName = _T(""),
										int nMovDetSavesCount = 0);
	void HideDetectionZones();

	// FTP
	// Both functions return the handle of the started lftp process
	// (remember to call CloseHandle() for the returned handle if != NULL)
	static HANDLE FTPCall(	CString sParams,					// command line params for lftp.exe
							BOOL bShow = FALSE);				// show / hide the console window
	static HANDLE FTPUpload(const FTPUploadConfigurationStruct& Config,
							CString sLocalFileName1,			// first file to upload
							CString sRemoteFileName1,			// remote name of first file
							CString sLocalFileName2 = _T(""),	// optional second file to upload
							CString sRemoteFileName2 = _T(""));	// remote name of second file

	// Validate Name
	static CString GetValidName(CString sName);

	// Settings
	BOOL LoadZonesSettings();
	void SaveZonesSettings();
	void LoadSettings(double dDefaultFrameRate, CString sSection, CString sDeviceName);
	void SavePlacement();
	void SaveSettings();
	void ExportDetectionZones(const CString& sFileName);
	void ImportDetectionZones(const CString& sFileName);

	// Autorun
	static CString AutorunGetDeviceKey(const CString& sDevicePathName);
	static void AutorunAddDevice(const CString& sDevicePathName);
	static void AutorunRemoveDevice(const CString& sDevicePathName);

	// NOTE for console applications started by a service
	// (this holds for microapache which is a console app)
	//
	// Up to Windows XP and Windows Server 2003 the first logged on user and the services
	// both run in session 0. When logging off, the system sends a logoff event to all
	// console applications from all users which run in session 0 -> the mapache.exe
	// processes terminate even if started from a service!
	// Starting from Windows Vista and Windows Server 2008 the services run in session 0,
	// all the other processes in session 1 or higher. Logging off will not terminate
	// the mapache.exe processes started from the service because they are session 0
	// processes.

	// Micro Apache
	void ViewWeb();
	void ViewFiles();
	static CString MicroApacheGetConfigFileName();
	static CString MicroApacheGetEditableConfigFileName();
	static CString MicroApacheGetLogFileName();
	static CString MicroApacheGetPidFileName();
	static void MicroApacheUpdateMainFiles();
	static BOOL MicroApacheUpdateWebFiles(CString sAutoSaveDir);
	static BOOL MicroApacheStart(DWORD dwTimeoutMs);
	static BOOL MicroApacheShutdown(DWORD dwTimeoutMs);
	
	// Mailer
	// returns the handle of the started mailsend process
	// (remember to call CloseHandle() for the returned handle if != NULL)
	static HANDLE SendMailCall(	CString sParams,			// command line params for mailsend.exe
								BOOL bShow = FALSE);		// show / hide the console window

	// Send Mail
	static BOOL SendMail(const SendMailConfigurationStruct& Config,
						const CString& sName,				// sName is replaced in subject if %name% present
						const CTime& Time,					// Time is replaced in subject if %date% and/or %time% present
						const CString& sNote,				// sNote is replaced in subject if %note% present
						CString sBody = _T(""),				// if no body given use the format "name: date time note"
						const CString& sFileName = _T(""),	// attachment
						BOOL bShow = FALSE);				// show / hide the console window

	// Php config file manipulation
	CString PhpGetConfigFileName();
	CString PhpLoadConfigFile();
	BOOL PhpSaveConfigFile(const CString& sConfig);
	BOOL PhpConfigFileSetParam(const CString& sParam, const CString& sValue);	// sParam is case sensitive!
	CString PhpConfigFileGetParam(const CString& sParam);						// sParam is case sensitive!

// Protected Functions
protected:
	BOOL InitOpenDxCapture(int nId);
	void Snapshot(CDib* pDib, const CTime& Time);
	BOOL EditCopy(CDib* pDib, const CTime& Time);
	void EditSnapshot(CDib* pDib, const CTime& Time);
	CString SaveJpegMail(CDib* pDib, const CTime& RefTime, DWORD dwRefUpTime);
	CString MakeJpegManualSnapshotFileName(const CTime& Time);
	CString MakeJpegMailSnapshotFileName(const CTime& Time);
	__forceinline int SummRectArea(	CDib* pDib,
									int width,
									int posX,
									int posY,
									int rx,
									int ry);
	void InitHttpGetFrameLocations();
	static double GetDefaultNetworkFrameRate(NetworkDeviceTypeMode nNetworkDeviceTypeMode);

// Public Variables
public:
	// General Vars
	CAVRec* volatile m_pAVRec;							// Pointer to the currently recording file
	CRITICAL_SECTION m_csAVRec;							// Critical section for the recording file
	volatile BOOL m_bDeinterlace;						// De-Interlace Video
	volatile BOOL m_bRotate180;							// Rotate Video by 180°
	volatile double m_dFrameRate;						// Set Capture Frame Rate
	volatile double m_dEffectiveFrameRate;				// Current Calculated Frame Rate
	volatile LONG m_lProcessFrameTime;					// Time in ms inside ProcessI420Frame()
	volatile LONG m_lCompressedDataRate;				// Compressed data rate in bytes / sec
	volatile LONG m_lCompressedDataRateSum;				// Compressed data rate sum
	BITMAPINFOFULL m_CaptureBMI;						// Capture source format
	BITMAPINFOFULL m_ProcessFrameBMI;					// BMI of Frame reaching ProcessI420Frame()
	volatile BOOL m_bPlacementLoaded;					// Placement Settings have been loaded
	volatile BOOL m_bCaptureStarted;					// Flag set when first frame has been processed
	CTime m_CaptureStartTime;							// Grabbing device started at this time
	volatile BOOL m_bVideoView;							// Flag indicating whether the frame grabbing is to be previewed
	volatile BOOL m_bShowFrameTime;						// Show / Hide Frame Time Inside the Frame (frame time is also recorded)
	volatile int m_nRefFontSize;						// Minimum font size for frame time, detection indicator, save/email/ftp progress
	volatile BOOL m_bDoEditCopy;						// Copy Frame to Clipboard in ProcessI420Frame()
	volatile BOOL m_bDoEditSnapshot;					// Manual Snapshot Frame to file
	volatile DWORD m_dwFrameCountUp;					// Captured Frames Count-Up, it can wrap around!
	volatile BOOL m_bSizeToDoc;							// If no placement settings in registry size client window to frame size
	volatile BOOL m_bDeviceFirstRun;					// First Time that this device runs
	CTime m_1SecTime;									// For the 1 sec tick in ProcessI420Frame()

	// Threads
	CHttpThread m_HttpThread;							// Http Networking Helper Thread
	CRtspThread m_RtspThread;							// Rtsp Networking Thread
	CWatchdogThread m_WatchdogThread;					// Video/Audio Watchdog Thread
	CDeleteThread m_DeleteThread;						// Delete old files Thread
	CCaptureAudioThread m_CaptureAudioThread;			// Audio Capture Thread
	CSaveFrameListThread m_SaveFrameListThread;			// Thread which saves the detection frames
	CSaveSnapshotThread m_SaveSnapshotThread;			// Thread which saves the snapshots
	CSaveSnapshotVideoThread m_SaveSnapshotVideoThread;	// Thread which creates the history video file

	// Drawing
	CDib* volatile m_pDrawDibRGB32;						// Frame in RGB32 format for drawing

	// Watchdog vars
	volatile LONG m_lCurrentInitUpTime;					// Uptime set in ProcessI420Frame()
	volatile BOOL m_bWatchDogVideoAlarm;				// WatchDog Video Alarm

	// DirectShow Capture Vars
	volatile BOOL m_bStopAndChangeFormat;				// Flag indicating that we are changing the DV format
	volatile BOOL m_bDxDeviceUnplugged;					// Device Has Been Unplugged
	CDxCapture* volatile m_pDxCapture;					// DirectShow Capture Object
	int m_nDeviceInputId;								// Input ID
	int m_nDeviceFormatId;								// Format ID
	int m_nDeviceFormatWidth;							// Format Width
	int m_nDeviceFormatHeight;							// Format Height
	
	// Audio Vars
	volatile DWORD m_dwCaptureAudioDeviceID;			// Audio Capture Device ID
	CString m_sCaptureAudioDeviceName;					// Audio Capture Device Name
	volatile BOOL m_bCaptureAudio;						// Do Capture Audio Flag
	volatile BOOL m_bAudioListen;						// Listen audio source
	LPWAVEFORMATEX m_pSrcWaveFormat;					// Uncompressed audio source format
	LPWAVEFORMATEX m_pDstWaveFormat;					// Wanted save format
	CDib::USERLIST m_AudioList;							// Audio buffers
	CRITICAL_SECTION m_csAudioList;						// Critical section to access the audio buffers

	// Audio / Video Rec
	volatile DWORD m_dwRecFirstUpTime;					// Up-Time of First Recorded Frame
	volatile DWORD m_dwRecLastUpTime;					// Up-Time of Last Recorded Frame
	volatile BOOL m_bRecFirstFrame;						// Recording Just Started
	volatile BOOL m_bRecAutoOpen;						// Auto open video file after recording
	volatile BOOL m_bRecTimeSegmentation;				// Enable / Disable Time Segmentation
	volatile int m_nTimeSegmentationIndex;				// Time segmentation combo box index
	CTime m_NextRecTime;								// Next Rec Time for segmentation
	CString m_sRecordAutoSaveDir;						// The Record Directory
	CString m_sAVRecFileExt;							// Extension of video file (lowercase with dot)
	volatile float m_fVideoRecQuality;					// 2.0f best quality, 31.0f worst quality, for H.264 clamped to [VIDEO_QUALITY_BEST, VIDEO_QUALITY_LOW]
	volatile int m_nDeleteRecordingsOlderThanDays;		// Delete Recordings older than the given amount of days,
														// 0 means never delete any file
	volatile int m_nMaxCameraFolderSizeMB;				// Maximum size of a camera folder, after that oldest files are removed,
														// 0 means no limit
	volatile int m_nMinDiskFreePermillion;				// Minimum disk free size in permillion, if the free space is lower than that the oldest files are removed

	// HTTP Networking
	CNetCom* volatile m_pVideoNetCom;					// HTTP Video Instance
	volatile NetworkDeviceTypeMode m_nNetworkDeviceTypeMode;// Video Network Device Type and Mode
	CString m_sGetFrameVideoHost;						// HTTP Host
	volatile int m_nGetFrameVideoPort;					// HTTP Port
	CString m_sHttpGetFrameUsername;					// HTTP Username
	CString m_sHttpGetFramePassword;					// HTTP Password
	CHttpParseProcess* volatile m_pHttpVideoParseProcess; // HTTP Video Parse & Process
	volatile int m_nHttpVideoQuality;					// 0 Best Quality, 100 Worst Quality
	volatile int m_nHttpVideoSizeX;						// Video width
	volatile int m_nHttpVideoSizeY;						// Video height
	CRITICAL_SECTION m_csHttpParams;					// Critical Section for Size and Compression params
	CRITICAL_SECTION m_csHttpProcess;					// Critical Section used while processing HTTP data
	volatile int m_nHttpGetFrameLocationPos;			// Automatic camera type detection position
	CStringArray m_HttpGetFrameLocations;				// Automatic camera type detection query string

	// Snapshot Vars
	volatile BOOL m_bSnapshotHistoryJpeg;				// Snapshot history save Jpegs
	volatile BOOL m_bSnapshotHistoryVideo;				// Snapshot history save Video
	volatile BOOL m_bSnapshotLiveJpegFtp;				// Upload Jpeg Live snapshot files
	volatile BOOL m_bSnapshotHistoryJpegFtp;			// Upload Jpeg Snapshot history files
	volatile BOOL m_bSnapshotHistoryVideoFtp;			// Upload Video Snapshot history files
	volatile BOOL m_bManualSnapshotAutoOpen;			// Auto open after executing the manual snapshot command
	volatile int m_nSnapshotRate;						// Snapshot rate in seconds
	volatile int m_nSnapshotRateMs;						// Snapshot rate in ms, effective: 1000 * m_nSnapshotRate + m_nSnapshotRateMs
	volatile int m_nSnapshotHistoryFrameRate;			// Snapshot history framerate
	volatile int m_nSnapshotCompressionQuality;			// Snapshot compression quality
	volatile int m_nSnapshotThumbWidth;					// Snapshot thumbnail width
	volatile int m_nSnapshotThumbHeight;				// Snapshot thumbnail height
	volatile DWORD m_dwNextSnapshotUpTime;				// The up-time of the next snapshot
	volatile BOOL m_bSnapshotStartStop;					// Enable / Disable Daily Timed Snapshots
	CTime m_SnapshotStartTime;							// Daily Snapshots Start Time
	CTime m_SnapshotStopTime;							// Daily Snapshots Stop Time
	CString m_sSnapshotLiveJpegName;					// Live snapshot jpeg name (without .jpg extension)
	CString m_sSnapshotLiveJpegThumbName;				// Live snapshot jpeg thumb name (without .jpg extension)
	FTPUploadConfigurationStruct m_SnapshotFTPUploadConfiguration;
	CRITICAL_SECTION m_csSnapshotConfiguration;			// Critical section for snapshot configurations

	// Movement Detector Vars
	volatile DWORD m_dwVideoProcessorMode;				// 0 = Off, 1 = software + file trigger detectors On
	CString m_sDetectionTriggerFileName;				// The external detection trigger file name
	FILETIME m_DetectionTriggerLastWriteTime;			// Last known write time of detection trigger file
	BOOL m_bShowMovementDetections;						// Show / Hide Movement Detection Zones
	int m_nShowEditDetectionZones;						// Edit Detection zones, 0: disabled, 1: add, 2: remove
	volatile BOOL m_bDetectingMovement;					// Flag indicating a Detection
	volatile BOOL m_bDetectingMinLengthMovement;		// Flag indicating a Movement of at least m_nDetectionMinLengthMilliSeconds
	volatile int m_nDetectionLevel;						// 0 = Off .. 100 = Max Sensitivity
	volatile int m_nDetectionZoneSize;					// Configured detection zone size: 0->Big, 1->Medium, 2->Small
	volatile int m_nCurrentDetectionZoneSize;			// Current detection zone size: 0->Big, 1->Medium, 2->Small
	volatile DWORD m_dwFirstDetFrameUpTime;				// Uptime of first movement detection frame
	volatile DWORD m_dwLastDetFrameUpTime;				// Uptime of last movement detection frame
	volatile int m_nMilliSecondsRecBeforeMovementBegin;	// Do record in the circular buffer list this amount of millisec. before det.
	volatile int m_nMilliSecondsRecAfterMovementEnd;	// Keep Recording this amount of millisec. after det. end
	volatile int m_nDetectionMinLengthMilliSeconds;		// Minimum detection length in ms, below this value SaveFrameList() is not called
	volatile int m_nDetectionMaxFrames;					// Maximum number of frames for a detection sequence
	volatile BOOL m_bSaveVideoMovementDetection;		// Save Movement Detections as Video File
	volatile BOOL m_bSaveAnimGIFMovementDetection;		// Save Movement Detections as Animated GIF
	volatile BOOL m_bSendMailDeviceOK;					// Send Email when device Works Properly
	volatile BOOL m_bSendMailMalfunction;				// Send Email on device Malfunction
	volatile BOOL m_bSendMailMovementDetection;			// Send Email on Movement Detections
	volatile BOOL m_bFTPUploadMovementDetection;		// FTP Upload Movement Detections
	volatile BOOL m_bExecCommandMovementDetection;		// Execute Command on Movement Detection
	volatile BOOL m_nExecModeMovementDetection;			// Determines when to execute the command
	CString m_sExecCommandMovementDetection;			// Command to execute on Movement Detection
	CString m_sExecParamsMovementDetection;				// Params for command execution
	volatile BOOL m_bHideExecCommandMovementDetection;	// Hide command's window
	volatile BOOL m_bWaitExecCommandMovementDetection;	// Wait that last command has terminated
	HANDLE volatile m_hExecCommandMovementDetection;	// Exec command handle
	CRITICAL_SECTION m_csExecCommandMovementDetection;	// Command Exec critical section
	CDib* volatile m_pMovementDetectorBackgndDib;		// Moving Background Dib
	DIBLISTLIST m_MovementDetectionsList;				// The List of Movement Detection Frame Grabbing Lists
	CRITICAL_SECTION m_csMovementDetectionsList;		// Critical Section of the Movement Detections List
	volatile DWORD m_dwAnimatedGifWidth;				// Width of Detection Animated Gif 
	volatile DWORD m_dwAnimatedGifHeight;				// Height of Detection Animated Gif
	CDib* volatile m_pDifferencingDib;					// Differencing Dib
	int* volatile m_MovementDetectorCurrentIntensity;	// Current Intensity by zones (array allocated in constructor)
	DWORD* volatile m_MovementDetectionsUpTime;			// Detection Up-Time For each Zone (array allocated in constructor)
	BYTE* volatile m_MovementDetections;				// Detecting in Zone (array allocated in constructor)
	BYTE* volatile m_DoMovementDetection;				// Do Movement Detection in this Zone with given relative sensitivity
														// 0 -> No Detection, 1 -> Full Sensitivity=100%, 2 -> 50%, 4 -> 25%, 10 -> 10%
														// (array allocated in constructor)
	volatile int m_nMovementDetectorIntensityLimit;		// Noise Floor
	volatile LONG m_lMovDetXZonesCount;					// Number of zones in X direction (never set to 0 to avoid division by 0)
	volatile LONG m_lMovDetYZonesCount;					// Number of zones in Y direction (never set to 0 to avoid division by 0)
	volatile LONG m_lMovDetTotalZones;					// Total Number of zones (set to 0 when a (re-)init of the zones is wanted)
	volatile int m_nDetectionStartStop;					// 0 -> Detection Alwayn on
														// 1 -> Enable detection on specified times and days
														// 2 -> Disable detection on specified times and days
	volatile BOOL m_bDetectionSunday;					// Do detections on Sunday
	volatile BOOL m_bDetectionMonday;					// Do detections on Monday
	volatile BOOL m_bDetectionTuesday;					// Do detections on Tuesday
	volatile BOOL m_bDetectionWednesday;				// Do detections on Wednesday
	volatile BOOL m_bDetectionThursday;					// Do detections on Thursday
	volatile BOOL m_bDetectionFriday;					// Do detections on Friday
	volatile BOOL m_bDetectionSaturday;					// Do detections on Saturday
	CTime m_DetectionStartTime;							// Daily Detection Start Time
	CTime m_DetectionStopTime;							// Daily Detection Stop Time
	BOOL m_bUnsupportedVideoSizeForMovDet;				// Flag indicating an unsupported resolution
	volatile int m_nMovDetFreqDiv;						// Current frequency divider
	volatile double m_dMovDetFrameRateFreqDivCalc;		// Framerate used to calculate the current frequency divider
	volatile int m_nMovDetSavesCount;					// Detection sequences counter for current day
	volatile int m_nMovDetSavesCountDay;				// Day of the above count
	volatile int m_nMovDetSavesCountMonth;				// Month of the above count
	volatile int m_nMovDetSavesCountYear;				// Year of the above count
	volatile AttachmentType m_MovDetAttachmentType;		// The email attachment type
	volatile int m_nMovDetSendMailSecBetweenMsg;		// Minimum seconds between detection emails
	CTime m_MovDetLastMailTime;							// Time of last sent detection email with no attachment
	CTime m_MovDetLastJPGMailTime;						// Time of last sent detection email with JPG attachment
	CTime m_MovDetLastVideoMailTime;					// Time of last sent detection email with video attachment
	CTime m_MovDetLastGIFMailTime;						// Time of last sent detection email with GIF attachment
	FTPUploadConfigurationStruct m_MovDetFTPUploadConfiguration;

	// Send Mail
	SendMailConfigurationStruct m_SendMailConfiguration;
	CTime m_LastDeviceNotifyTime;

	// Property Sheet Pointer
	CCameraAdvancedSettingsPropertySheet* volatile m_pCameraAdvancedSettingsPropertySheet;
	CSnapshotPage* volatile m_pSnapshotPage;
	CGeneralPage* volatile m_pGeneralPage;
	CMovementDetectionPage* volatile m_pMovementDetectionPage;

// Protected Variables
protected:
	CVideoDeviceView* m_pView;
	CVideoDeviceChildFrame* m_pFrame;
	CDib* volatile m_pProcessFrameDib;
	CDib* volatile m_pProcessFrameExtraDib;
	CAVDecoder m_AVDecoder;
	CAVDecoder m_DrawDecoder;
	CMJPEGEncoder m_MJPEGDetEncoder;
	volatile DWORD m_dwStopProcessFrame;
	volatile DWORD m_dwProcessFrameStopped;
	CRITICAL_SECTION m_csProcessFrameStop;

	// For Frame Rate and Data Rate Calculation
	double m_dEffectiveFrameTimeSum;
	DWORD m_dwEffectiveFrameTimeCountUp;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVideoDeviceDoc)
	//}}AFX_VIRTUAL

// Debug
#ifdef _DEBUG
public:
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	//{{AFX_MSG(CVideoDeviceDoc)
	afx_msg void OnCaptureRecord();
	afx_msg void OnUpdateCaptureRecord(CCmdUI* pCmdUI);
	afx_msg void OnCaptureMovDet();
	afx_msg void OnUpdateCaptureMovDet(CCmdUI* pCmdUI);
	afx_msg void OnCaptureCameraAdvancedSettings();
	afx_msg void OnViewVideo();
	afx_msg void OnUpdateViewVideo(CCmdUI* pCmdUI);
	afx_msg void OnViewFrametime();
	afx_msg void OnUpdateViewFrametime(CCmdUI* pCmdUI);
	afx_msg void OnFileClose();
	afx_msg void OnViewDetections();
	afx_msg void OnUpdateViewDetections(CCmdUI* pCmdUI);
	afx_msg void OnUpdateCaptureCameraAdvancedSettings(CCmdUI* pCmdUI);
	afx_msg void OnViewDetectionZonesAdd();
	afx_msg void OnUpdateViewDetectionZonesAdd(CCmdUI* pCmdUI);
	afx_msg void OnViewDetectionZonesRemove();
	afx_msg void OnUpdateViewDetectionZonesRemove(CCmdUI* pCmdUI);
	afx_msg void OnEditDelete();
	afx_msg void OnEditCopy();
	afx_msg void OnFileSave();
	afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
	afx_msg void OnViewFit();
	afx_msg void OnUpdateViewFit(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnFileSaveAs();
	afx_msg void OnUpdateFileSaveAs(CCmdUI* pCmdUI);
	afx_msg void OnEditSnapshot();
	afx_msg void OnUpdateEditSnapshot(CCmdUI* pCmdUI);
	afx_msg void OnEditExportZones();
	afx_msg void OnEditImportZones();
	afx_msg void OnCaptureCameraBasicSettings();
	afx_msg void OnUpdateCaptureCameraBasicSettings(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

__forceinline void CVideoDeviceDoc::ClearFrameList(CDib::LIST* pFrameList)
{
	if (pFrameList)
		CDib::FreeList(*pFrameList);
}

__forceinline void CVideoDeviceDoc::OneEmptyFrameList()
{
	::EnterCriticalSection(&m_csMovementDetectionsList);
	if (m_MovementDetectionsList.IsEmpty())
	{
		CDib::LIST* pNewList = new CDib::LIST;
		if (pNewList)
			m_MovementDetectionsList.AddTail(pNewList);
	}
	else
	{
		while (m_MovementDetectionsList.GetCount() > 1)
		{
			CDib::LIST* pFrameList = m_MovementDetectionsList.GetTail();
			ClearFrameList(pFrameList);
			delete pFrameList;
			m_MovementDetectionsList.RemoveTail();
		}
		CDib::LIST* pFrameList = m_MovementDetectionsList.GetHead();
		ClearFrameList(pFrameList);
	}
	::LeaveCriticalSection(&m_csMovementDetectionsList);
}

#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIDEODEVICEDOC_H__24956A13_6E82_4959_931F_F512BE9B17A0__INCLUDED_)
