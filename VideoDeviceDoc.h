#if !defined(AFX_VIDEODEVICEDOC_H__24956A13_6E82_4959_931F_F512BE9B17A0__INCLUDED_)
#define AFX_VIDEODEVICEDOC_H__24956A13_6E82_4959_931F_F512BE9B17A0__INCLUDED_

#pragma once

#ifdef VIDEODEVICEDOC

// Includes
#include "uImagerDoc.h"
#include "WorkerThread.h"
#include "DxDraw.h"
#include "pjnsmtp.h"
#include "NetCom.h"
#include "AVRec.h"
#include "AVDecoder.h"
#include "MJPEGEncoder.h"
#include "YuvToRgb.h"
#include "SortableFileFind.h"
#include "FTPTransfer.h"
#include "HostPortDlg.h"
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
class CVideoDevicePropertySheet;
class CMovementDetectionPage;

// General Settings
#define MIN_FRAMERATE						0.1			// fps
#define MAX_FRAMERATE						95.0		// fps
#define PROCESS_MAX_FRAMETIME				15000U		// ms, make sure that: 1000 / MIN_FRAMERATE < PROCESS_MAX_FRAMETIME
#define STARTUP_SETTLE_TIME_SEC				3			// sec
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

// Frame time, date, count and thumb message constants
#define FRAMETAG_REFWIDTH					640
#define FRAMETAG_REFHEIGHT					480
#define FRAMETIME_COLOR						RGB(0,0xFF,0)
#define FRAMEDATE_COLOR						RGB(0x80,0x80,0xFF)
#define FRAMECOUNT_COLOR					RGB(0xFF,0xFF,0xFF)
#define THUMBMESSAGE_FONTSIZE				8

// Process Frame Stop Engine
#define PROCESSFRAME_MAX_RETRY_TIME			5000		// ms
#define PROCESSFRAME_ASSISTANT				0x01
#define PROCESSFRAME_DXFORMATDIALOG			0x02
#define PROCESSFRAME_DVFORMATDIALOG			0x04
#define PROCESSFRAME_CHANGEFRAMERATE		0x08
#define PROCESSFRAME_DXOPEN					0x10
#define PROCESSFRAME_DXREPLUGGED			0x20
#define PROCESSFRAME_CLOSE					0x40

// Watch Dog and Draw
#define WATCHDOG_LONGCHECK_TIME				1000U		// ms
#define WATCHDOG_SHORTCHECK_TIME			300U		// ms
#define WATCHDOG_THRESHOLD					30000U		// ms, make sure that: 1000 / MIN_FRAMERATE < WATCHDOG_THRESHOLD
#define DXDRAW_REINIT_TIMEOUT				5000U		// ms
#define DXDRAW_BKG_COLOR					RGB(0,0,0)	// do not change this because dxdraw background is cleared to 0
#define DXDRAW_MESSAGE_COLOR				RGB(0xFF,0xFF,0xFF)
#define DXDRAW_MESSAGE_SUCCESS_COLOR		RGB(0,0xFF,0)
#define DXDRAW_MESSAGE_ERROR_COLOR			RGB(0xFF,0,0)
#define DXDRAW_MESSAGE_BKG_COLOR			RGB(0x30,0x30,0x30)
#define DXDRAW_MESSAGE_SHOWTIME				1500U		// ms

// Snapshot
#define MIN_SNAPSHOT_RATE					1			// one snapshot per second
#define DEFAULT_SNAPSHOT_RATE				1			// each given seconds
#define DEFAULT_SNAPSHOT_HISTORY_FRAMERATE	15			// fps
#define MIN_SNAPSHOT_HISTORY_FRAMERATE		1			// fps
#define MAX_SNAPSHOT_HISTORY_FRAMERATE		95			// fps
#define DEFAULT_SNAPSHOT_LIVE_JPEGNAME		_T("snapshot")
#define DEFAULT_SNAPSHOT_LIVE_JPEGTHUMBNAME	_T("snapshot_thumb")
#define DEFAULT_SNAPSHOT_COMPR_QUALITY		60			// 0 Worst Quality, 100 Best Quality 
#define DEFAULT_SNAPSHOT_THUMB_WIDTH		228			// Must be a multiple of 4 because of swf
#define DEFAULT_SNAPSHOT_THUMB_HEIGHT		172			// Must be a multiple of 4 because of swf
#define DEFAULT_SERVERPUSH_POLLRATE_MS		200			// ms

// Movement Detection
#define NO_DETECTOR							0x00
#define TRIGGER_FILE_DETECTOR				0x01
#define SOFTWARE_MOVEMENT_DETECTOR			0x02
#define DEFAULT_PRE_BUFFER_MSEC				2000		// ms
#define DEFAULT_POST_BUFFER_MSEC			6000		// ms
#define MOVDET_MIN_LENGTH_MSEC				1000		// Minimum detection length in ms, below this value SaveFrameList() is not called
#define MOVDET_BUFFER_COMPRESSIONQUALITY	4			// 2: best quality, 31: worst quality
#define DEFAULT_MOVDET_LEVEL				50			// Detection level default value (1 .. 100 = Max sensibility)
#define DEFAULT_MOVDET_INTENSITY_LIMIT		25			// Intensity difference default value
#define MOVDET_MAX_ZONES_BLOCK_SIZE			1024		// Subdivide settings in blocks (MOVDET_MAX_ZONES must be a multiple of this)
#define MOVDET_MAX_ZONES					8192		// Maximum number of zones
#define MOVDET_MIN_ZONES_XORY				4			// Minimum number of zones in X or Y direction
#define MOVDET_ZONE_FORMAT					_T("DoMovementDetection%03i")
#define MOVDET_ZONES_BLOCK_FORMAT			_T("MovDetZones%i")
#define MOVDET_SAVEFRAMES_POLL				1000U		// ms
#define MOVDET_MIN_FRAMES_IN_LIST			10			// Min. frames in list before saving the list in the
														// case of insufficient memory
#define MOVDET_MAX_FRAMES_IN_LIST			15000		// 16000 is the limit for swf files -> be safe and start
														// a new list with 15000
#define MOVDET_SAVE_MIN_FRAMERATE_RATIO		0.3			// Min ratio between calculated (last - first) and m_dEffectiveFrameRate
#define MOVDET_TIMEOUT						1000U		// Timeout in ms for detection zones
#define MOVDET_MEM_LOAD_THRESHOLD			25.0		// Above this load the detected frames are saved and freed
#define MOVDET_MEM_LOAD_CRITICAL			60.0		// Above this load the detected frames are dropped
#define MOVDET_MEM_MAX_MB					1050		// Maximum allocable memory in MB for 32 bits applications
														// (not 2048 because of fragmentation, stack and heap)
#define MOVDET_MEM_MIN_MB					500			// Minimum memory in MB
#define MOVDET_BASE_MEM_USAGE_MB			50			// Base memory usage of a open video device doc (without detection buffer)
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
#define PHPCONFIG_SHOW_PRINTCOMMAND			_T("SHOW_PRINTCOMMAND")
#define PHPCONFIG_SHOW_SAVECOMMAND			_T("SHOW_SAVECOMMAND")
#define PHPCONFIG_LANGUAGEFILEPATH			_T("LANGUAGEFILEPATH")
#define PHPCONFIG_STYLEFILEPATH				_T("STYLEFILEPATH")
#define PHPCONFIG_MIN_THUMSPERPAGE			7
#define PHPCONFIG_MAX_THUMSPERPAGE			36
#define PHPCONFIG_DEFAULT_THUMSPERPAGE		27

// Http Networking
#define DEFAULT_TCP_PORT					80
#define HTTP_MAX_HEADER_SIZE				1400
#define HTTP_MAX_MULTIPART_BOUNDARY			128
#define HTTP_MIN_MULTIPART_SIZE				256
#define HTTP_MAX_MULTIPART_SIZE				8388608		// 8 MB		
#define DEFAULT_HTTP_VIDEO_QUALITY			30			// 0 Best Quality, 100 Worst Quality
#define DEFAULT_HTTP_VIDEO_SIZE_CX			640
#define DEFAULT_HTTP_VIDEO_SIZE_CY			480
#define HTTPGETFRAME_RECONNECTION_DELAY		5000U		// ms
#define HTTPGETFRAME_MAXCOUNT_ALARM1		30
#define HTTPGETFRAME_MAXCOUNT_ALARM2		40
#define HTTPGETFRAME_MAXCOUNT_ALARM3		50
#define HTTPGETFRAME_DELAY_DEFAULT			500U		// ms
#define HTTPGETFRAME_MIN_DELAY_ALARM1		100U		// ms
#define HTTPGETFRAME_MIN_DELAY_ALARM2		400U		// ms
#define HTTPGETFRAME_MIN_DELAY_ALARM3		1000U		// ms
#define HTTPGETFRAME_MAX_DELAY_ALARM		((DWORD)(1000.0 / MIN_FRAMERATE)) // ms
#define HTTPGETFRAME_CONNECTION_TIMEOUT		60			// Connection timeout in sec
#define HTTPGETFRAME_MIN_KEEPALIVE_REQUESTS	50			// Keep-alive support check
#define HTTPGETFRAME_USERNAME_PLACEHOLDER	_T("[USERNAME]") // only use letters, numbers, uri unreserved or uri reserved chars
#define HTTPGETFRAME_PASSWORD_PLACEHOLDER	_T("[PASSWORD]") // only use letters, numbers, uri unreserved or uri reserved chars

// The Document Class
class CVideoDeviceDoc : public CUImagerDoc
{
public:
	// Types
	typedef CList<CDib::LIST*,CDib::LIST*> DIBLISTLIST;
	typedef CList<CNetCom*,CNetCom*> NETCOMLIST;
	
	// Enums
	enum AttachmentType
	{
		ATTACHMENT_NONE				= 0,
		ATTACHMENT_AVI				= 1,
		ATTACHMENT_GIF				= 2,
		ATTACHMENT_JPG				= 3,
		ATTACHMENT_GIF_AVI			= 4,
		ATTACHMENT_JPG_AVI			= 5,
		ATTACHMENT_GIF_JPG			= 6,
		ATTACHMENT_GIF_JPG_AVI		= 7
	};
	enum FilesToUploadType
	{
		FILES_TO_UPLOAD_AVI			= 0,
		FILES_TO_UPLOAD_GIF			= 1,
		FILES_TO_UPLOAD_SWF			= 2,
		FILES_TO_UPLOAD_AVI_GIF		= 3,
		FILES_TO_UPLOAD_SWF_GIF		= 4,
		FILES_TO_UPLOAD_AVI_SWF_GIF	= 5
	};

	// The Http Networking Get Frame Parser & Processor Class
	class CHttpGetFrameParseProcess : public CNetCom::CParseProcess
	{
		public:
			CHttpGetFrameParseProcess(CVideoDeviceDoc* pDoc) {m_pDoc = pDoc; m_dwCNonceCount = 0U; Clear();};
			virtual ~CHttpGetFrameParseProcess() {FreeAVCodec();};
			void Close() {FreeAVCodec(); Clear();};
			BOOL SendRawRequest(CString sRequest);
			BOOL SendRequest();
			virtual BOOL Parse(CNetCom* pNetCom, BOOL bLastCall);
			virtual BOOL Process(unsigned char* pLinBuf, int nSize);
			BOOL HasResolution(const CSize& Size);
			
			CArray<CSize,CSize> m_Sizes;
			CString m_sRealm;
			CString m_sQop;
			CString m_sNonce;
			CString m_sAlgorithm;
			CString m_sOpaque;
			volatile enum {FORMATUNKNOWN = 0, FORMATJPEG, FORMATMJPEG} m_FormatType;
			typedef enum {AUTHNONE = 0, AUTHBASIC, AUTHDIGEST} AUTHTYPE;
			volatile AUTHTYPE m_AnswerAuthorizationType;
			volatile AUTHTYPE m_LastRequestAuthorizationType;
			volatile BOOL m_bQueryProperties;
			volatile BOOL m_bSetResolution;
			volatile BOOL m_bSetCompression;
			volatile BOOL m_bSetFramerate;
			volatile BOOL m_bFirstFrame;
			volatile BOOL m_bTryConnecting;
			volatile BOOL m_bConnectionKeepAlive;
			volatile BOOL m_bPollNextJpeg;
			volatile BOOL m_bOldVersion;

		protected:
			void Clear() {	m_bMultipartNoLength = FALSE;
							m_nMultipartBoundaryLength = 0;
							m_bFirstFrame = TRUE;
							m_bTryConnecting = FALSE;
							m_bQueryProperties = FALSE;
							m_bSetResolution = FALSE;
							m_bSetCompression = FALSE;
							m_bSetFramerate = FALSE;
							m_bPollNextJpeg = FALSE;
							m_FormatType = FORMATUNKNOWN;
							m_AnswerAuthorizationType = AUTHNONE;
							m_LastRequestAuthorizationType = AUTHNONE;
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

			CVideoDeviceDoc* m_pDoc;
			BOOL m_bMultipartNoLength;
			DWORD m_dwCNonceCount;
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
	typedef CList<CHttpGetFrameParseProcess*,CHttpGetFrameParseProcess*> NETCOMPARSEPROCESSLIST;

	// The Record Audio File Thread Class
	class CCaptureAudioThread : public CWorkerThread
	{
		public:
			// Functions
			CCaptureAudioThread();
			virtual ~CCaptureAudioThread();
			void AudioInSourceDialog();
			void SetDoc(CVideoDeviceDoc* pDoc) {m_pDoc = pDoc;};
			static void WaveInitFormat(WORD wCh, DWORD dwSampleRate, WORD wBitsPerSample, LPWAVEFORMATEX pWaveFormat);

			// Wave Formats
			LPWAVEFORMATEX m_pSrcWaveFormat;
			LPWAVEFORMATEX m_pDstWaveFormat;

			// Audio list
			CDib::USERLIST m_AudioList;
			CRITICAL_SECTION m_csAudioList;
			
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
	};

	// Http Get Frame Thread
	class CHttpGetFrameThread : public CWorkerThread
	{
		public:
			CHttpGetFrameThread() {	m_pDoc = NULL;
									m_dwConnectDelay	= 0U;										// Delay before starting connection
									m_hEventArray[0]	= GetKillEvent();							// Kill Event
									m_hEventArray[1]	= ::CreateEvent(NULL, TRUE, FALSE, NULL);	// Http Setup Connection Event
									m_hEventArray[2]	= ::CreateEvent(NULL, TRUE, FALSE, NULL);	// Http Connected Event
									m_hEventArray[3]	= ::CreateEvent(NULL, TRUE, FALSE, NULL);	// Http Read Event
									m_hEventArray[4]	= ::CreateEvent(NULL, TRUE, FALSE, NULL);	// Http Connect Failed Event
									::InitializeCriticalSection(&m_csConnectRequestParams);};
			virtual ~CHttpGetFrameThread() {Kill();
											::CloseHandle(m_hEventArray[1]);
											::CloseHandle(m_hEventArray[2]);
											::CloseHandle(m_hEventArray[3]);
											::CloseHandle(m_hEventArray[4]);
											::DeleteCriticalSection(&m_csConnectRequestParams);};
			void SetDoc(CVideoDeviceDoc* pDoc) {m_pDoc = pDoc;};
			__forceinline BOOL SetEventConnect(LPCTSTR lpszRequest = _T(""), DWORD dwConnectDelay = 0U)
			{
				::EnterCriticalSection(&m_csConnectRequestParams);
				m_sRequest = CString(lpszRequest);
				m_dwConnectDelay = dwConnectDelay;
				::LeaveCriticalSection(&m_csConnectRequestParams);
				return ::SetEvent(m_hEventArray[1]);	
			};
			__forceinline HANDLE GetHttpConnectedEvent() const {return m_hEventArray[2];};
			__forceinline HANDLE GetHttpReadEvent() const {return m_hEventArray[3];};
			__forceinline HANDLE GetHttpConnectFailedEvent() const {return m_hEventArray[4];};

		protected:
			int Work();
			int OnError();
			__forceinline BOOL Connect(	BOOL bSignalEvents,
										CNetCom* pNetCom,
										CHttpGetFrameParseProcess* pParseProcess,
										int nSocketFamily);
			BOOL PollAndClean(BOOL bDoNewPoll);
			void CleanUpAllConnections();
			CVideoDeviceDoc* m_pDoc;
			HANDLE m_hEventArray[5];
			volatile DWORD m_dwConnectDelay;
			CString m_sRequest;
			CRITICAL_SECTION m_csConnectRequestParams;
			NETCOMLIST m_HttpGetFrameNetComList;
			NETCOMPARSEPROCESSLIST m_HttpGetFrameParseProcessList;
	};

	// Watch Dog and Draw Thread
	class CWatchdogAndDrawThread : public CWorkerThread
	{
		public:
			CWatchdogAndDrawThread() {	m_pDoc = NULL;
										m_hEventArray[0] = GetKillEvent();
										m_hEventArray[1] = ::CreateEvent(NULL, TRUE, FALSE, NULL);};
			virtual ~CWatchdogAndDrawThread() {	Kill();
												::CloseHandle(m_hEventArray[1]);};
			void SetDoc(CVideoDeviceDoc* pDoc) {m_pDoc = pDoc;};
			__forceinline BOOL TriggerDraw() {return ::SetEvent(m_hEventArray[1]);};

		protected:
			int Work();
			CVideoDeviceDoc* m_pDoc;
			HANDLE m_hEventArray[2];
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
			BOOL DeleteIt(CString sAutoSaveDir, int nDeleteOlderThanDays);
			BOOL CalcOldestDir(	CSortableFileFind& FileFind,
								int nAutoSaveDirSize,
								CTime& OldestDirTime,
								const CTime& CurrentTime);
			BOOL DeleteOld(	CSortableFileFind& FileFind,
							int nAutoSaveDirSize,
							LONGLONG llDeleteOlderThanDays,
							const CTime& CurrentTime);
			CVideoDeviceDoc* m_pDoc;
	};

	// Email sending configuration structure
	typedef struct tagSendMailConfigurationStruct
	{
		CString			m_sBCC;
		CString			m_sBody;
		CString			m_sCC;
		CString			m_sFiles;
		AttachmentType	m_AttachmentType;
		CString			m_sSubject;
		CString			m_sTo;
		BOOL			m_bDirectly;
		BOOL			m_bDNSLookup;
		CString			m_sFrom;
		CString			m_sHost;
		CString			m_sFromName;
		int				m_nPort;
		CPJNSMTPConnection::AuthenticationMethod m_Auth;
		CPJNSMTPConnection::ConnectionType m_ConnectionType;
		CString			m_sUsername;
		CString			m_sPassword;
		CString			m_sBoundIP;
		CString			m_sEncodingFriendly;
		CString			m_sEncodingCharset;
		BOOL			m_bMime;
		BOOL			m_bHTML;
		CPJNSMTPMessage::PRIORITY m_Priority;
	} SendMailConfigurationStruct;

	// FTP upload configuration structure
	typedef struct tagFTPUploadConfigurationStruct
	{
		CString			m_sHost;
		CString			m_sRemoteDir;
		int				m_nPort;
		BOOL			m_bPasv;
		BOOL			m_bBinary;
		BOOL			m_bProxy;
		CString			m_sProxy;
		CString			m_sUsername;
		CString			m_sPassword;
		FilesToUploadType m_FilesToUpload;
	} FTPUploadConfigurationStruct;

	// The Save Frame List Thread Class
	class CSaveFrameListSMTPConnection; // forward declaration
	class CSaveFrameListThread : public CWorkerThread
	{
		public:
			CSaveFrameListThread(){	m_pDoc = NULL; m_pFrameList = NULL; m_nNumFramesToSave = 0;
									m_nSaveProgress = 100; m_nSendMailProgress = 100; m_nFTPUploadProgress = 100;
									m_bWorking = FALSE;};
			virtual ~CSaveFrameListThread() {Kill();};
			void SetDoc(CVideoDeviceDoc* pDoc) {m_pDoc = pDoc;};
			__forceinline int GetSaveProgress() const {return m_nSaveProgress;};
			__forceinline void SetSendMailProgress(int nSendMailProgress) {m_nSendMailProgress = nSendMailProgress;};
			__forceinline int GetSendMailProgress() const {return m_nSendMailProgress;};
			__forceinline void SetFTPUploadProgress(int nFTPUploadProgress) {m_nFTPUploadProgress = nFTPUploadProgress;};
			__forceinline int GetFTPUploadProgress() const {return m_nFTPUploadProgress;};
			__forceinline BOOL IsWorking() const {return m_bWorking;};

		protected:
			// CalcMovementDetectionListsSize() must be called from this thread because
			// this thread changes/deletes the list's dibs not inside the lists critical section!
			void CalcMovementDetectionListsSize();
			void DecodeFrame(CDib* pDib);
			int Work();
			CString SaveJpeg(	CDib* pDib,
								CString sJPGDir,
								const CTime& RefTime,
								DWORD dwRefUpTime);
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
			BOOL SendMailFTPUpload(	const CTime& Time,
									const CString& sAVIFileName,
									const CString& sGIFFileName,
									const CString& sSWFFileName,
									const CStringArray& sJPGFileNames);
			__forceinline BOOL SendMailMovementDetection(	const CTime& Time,
															const CString& sAVIFileName,
															const CString& sGIFFileName,
															const CStringArray& sJPGFileNames);
			__forceinline BOOL FTPUploadMovementDetection(	const CTime& Time,
															const CString& sAVIFileName,
															const CString& sGIFFileName,
															const CString& sSWFFileName);

			__forceinline BOOL DoMakeAvi() const {
							return m_pDoc->m_bSaveAVIMovementDetection				||

							(m_pDoc->m_bSendMailMovementDetection &&
							m_pDoc->m_MovDetSendMailConfiguration.m_AttachmentType ==
								CVideoDeviceDoc::ATTACHMENT_AVI)					||

							(m_pDoc->m_bSendMailMovementDetection &&
							m_pDoc->m_MovDetSendMailConfiguration.m_AttachmentType ==
								CVideoDeviceDoc::ATTACHMENT_GIF_AVI)				||

							(m_pDoc->m_bSendMailMovementDetection &&
							m_pDoc->m_MovDetSendMailConfiguration.m_AttachmentType ==
								CVideoDeviceDoc::ATTACHMENT_JPG_AVI)				||

							(m_pDoc->m_bSendMailMovementDetection &&
							m_pDoc->m_MovDetSendMailConfiguration.m_AttachmentType ==
								CVideoDeviceDoc::ATTACHMENT_GIF_JPG_AVI)			||

							(m_pDoc->m_bFTPUploadMovementDetection &&
							m_pDoc->m_MovDetFTPUploadConfiguration.m_FilesToUpload ==
								CVideoDeviceDoc::FILES_TO_UPLOAD_AVI)				||

							(m_pDoc->m_bFTPUploadMovementDetection &&
							m_pDoc->m_MovDetFTPUploadConfiguration.m_FilesToUpload ==
								CVideoDeviceDoc::FILES_TO_UPLOAD_AVI_GIF)			||

							(m_pDoc->m_bFTPUploadMovementDetection &&
							m_pDoc->m_MovDetFTPUploadConfiguration.m_FilesToUpload ==
								CVideoDeviceDoc::FILES_TO_UPLOAD_AVI_SWF_GIF);};

			__forceinline BOOL DoMakeSwf() const {
							return m_pDoc->m_bSaveSWFMovementDetection				||

							(m_pDoc->m_bFTPUploadMovementDetection &&
							m_pDoc->m_MovDetFTPUploadConfiguration.m_FilesToUpload ==
								CVideoDeviceDoc::FILES_TO_UPLOAD_SWF)				||

							(m_pDoc->m_bFTPUploadMovementDetection &&
							m_pDoc->m_MovDetFTPUploadConfiguration.m_FilesToUpload ==
								CVideoDeviceDoc::FILES_TO_UPLOAD_SWF_GIF)			||

							(m_pDoc->m_bFTPUploadMovementDetection &&
							m_pDoc->m_MovDetFTPUploadConfiguration.m_FilesToUpload ==
								CVideoDeviceDoc::FILES_TO_UPLOAD_AVI_SWF_GIF);};

			__forceinline BOOL DoMakeJpeg() const {
							return (m_pDoc->m_bSendMailMovementDetection &&
							m_pDoc->m_MovDetSendMailConfiguration.m_AttachmentType ==
								CVideoDeviceDoc::ATTACHMENT_JPG)					||
								
							(m_pDoc->m_bSendMailMovementDetection &&
							m_pDoc->m_MovDetSendMailConfiguration.m_AttachmentType ==
								CVideoDeviceDoc::ATTACHMENT_JPG_AVI)				||

							(m_pDoc->m_bSendMailMovementDetection &&
							m_pDoc->m_MovDetSendMailConfiguration.m_AttachmentType ==
								CVideoDeviceDoc::ATTACHMENT_GIF_JPG)				||

							(m_pDoc->m_bSendMailMovementDetection &&
							m_pDoc->m_MovDetSendMailConfiguration.m_AttachmentType ==
								CVideoDeviceDoc::ATTACHMENT_GIF_JPG_AVI);};

			__forceinline BOOL DoMakeGif() const {
							return	m_pDoc->m_bSaveAnimGIFMovementDetection			||

							(m_pDoc->m_bSendMailMovementDetection &&
							m_pDoc->m_MovDetSendMailConfiguration.m_AttachmentType ==
								CVideoDeviceDoc::ATTACHMENT_GIF)					||

							(m_pDoc->m_bSendMailMovementDetection &&
							m_pDoc->m_MovDetSendMailConfiguration.m_AttachmentType ==
								CVideoDeviceDoc::ATTACHMENT_GIF_AVI)				||

							(m_pDoc->m_bSendMailMovementDetection &&
							m_pDoc->m_MovDetSendMailConfiguration.m_AttachmentType ==
								CVideoDeviceDoc::ATTACHMENT_GIF_JPG)				||

							(m_pDoc->m_bSendMailMovementDetection &&
							m_pDoc->m_MovDetSendMailConfiguration.m_AttachmentType ==
								CVideoDeviceDoc::ATTACHMENT_GIF_JPG_AVI)			||

							(m_pDoc->m_bFTPUploadMovementDetection &&
							m_pDoc->m_MovDetFTPUploadConfiguration.m_FilesToUpload ==
								CVideoDeviceDoc::FILES_TO_UPLOAD_GIF)				||

							(m_pDoc->m_bFTPUploadMovementDetection &&
							m_pDoc->m_MovDetFTPUploadConfiguration.m_FilesToUpload ==
								CVideoDeviceDoc::FILES_TO_UPLOAD_AVI_GIF)			||

							(m_pDoc->m_bFTPUploadMovementDetection &&
							m_pDoc->m_MovDetFTPUploadConfiguration.m_FilesToUpload ==
								CVideoDeviceDoc::FILES_TO_UPLOAD_SWF_GIF)			||

							(m_pDoc->m_bFTPUploadMovementDetection &&
							m_pDoc->m_MovDetFTPUploadConfiguration.m_FilesToUpload ==
								CVideoDeviceDoc::FILES_TO_UPLOAD_AVI_SWF_GIF);};
			
			// Return Values
			// -1 : Do Exit Thread
			// 0  : Error Sending Email
			// 1  : Ok
			int SendMail(const CStringArray& sFiles);
			void SendMailMessage(const CString& sTempEmailFile, CSaveFrameListSMTPConnection& connection, CPJNSMTPMessage* pMessage);

			CVideoDeviceDoc* m_pDoc;
			CDib::LIST* m_pFrameList;
			int m_nNumFramesToSave;
			CAVDecoder m_AVDetDecoder;
			volatile int m_nSaveProgress;
			volatile int m_nSendMailProgress;
			volatile int m_nFTPUploadProgress;
			volatile BOOL m_bWorking;
	};

	// FTP Transfer Class
	class CSaveFrameListFTPTransfer : public CFTPTransfer
	{
		public:
			CSaveFrameListFTPTransfer(CSaveFrameListThread* pThread);
			virtual void OnTransferProgress(DWORD dwPercentage);
		protected:
			CSaveFrameListThread* m_pThread;
	};

	// The SMTP Connection Class
	class CSaveFrameListSMTPConnection : public CPJNSMTPConnection
	{
		public:
			CSaveFrameListSMTPConnection(CSaveFrameListThread* pThread = NULL){m_pThread = pThread; m_bDoExit = false;};
			virtual BOOL OnSendProgress(DWORD dwCurrentBytes, DWORD dwTotalBytes);
			volatile bool m_bDoExit;
		protected:
			CSaveFrameListThread* m_pThread;
	};

	// The Save Snapshot SWF Thread Class
	class CSaveSnapshotSWFThread : public CWorkerThread
	{
		public:
			CSaveSnapshotSWFThread(){m_pDoc = NULL; m_ThreadExecutedForTime = CTime(0);};
			virtual ~CSaveSnapshotSWFThread() {Kill();};
			void SetDoc(CVideoDeviceDoc* pDoc) {m_pDoc = pDoc;};

			BOOL m_bSnapshotHistoryJpeg;
			BOOL m_bSnapshotHistorySwfFtp;
			float m_fSnapshotVideoCompressorQuality;
			double m_dSnapshotHistoryFrameRate;
			CTime m_Time;
			CTime m_ThreadExecutedForTime;
			CString m_sSnapshotAutoSaveDir;
			FTPUploadConfigurationStruct m_Config;

		protected:
			CVideoDeviceDoc* m_pDoc;
			int Work();
			__forceinline CString MakeSwfHistoryFileName();
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
			BOOL m_bSnapshotThumb;
			BOOL m_bSnapshotLiveJpeg;
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
	void ConnectErr(LPCTSTR lpszText, const CString& sDevicePathName, const CString& sDeviceName); // Called when a device start fails
	void CloseDocument();														// Close Document by sending a WM_CLOSE to the Parent Frame
	CString GetAssignedDeviceName();											// Get User Assigned Device Name
	static CString GetHostFromDevicePathName(const CString& sDevicePathName);	// Returns host name or _T("") if it's not a network device
	CString GetDeviceName();													// Friendly Device Name
	CString GetDevicePathName();												// Used For Settings, Scheduler and Autorun
	static CString GetNetworkDevicePathName(const CString& sGetFrameVideoHost,	// GetDevicePathName() calls this for network devices
											int nGetFrameVideoPort,
											const CString& sHttpGetFrameLocation,
											int nNetworkDeviceTypeMode);
	void SetDocumentTitle();
	CVideoDeviceView* GetView() const {return m_pView;};
	void SetView(CVideoDeviceView* pView) {m_pView = pView;};
	CVideoDeviceChildFrame* GetFrame() const {return m_pFrame;};
	void SetFrame(CVideoDeviceChildFrame* pFrame) {m_pFrame = pFrame;};
	static BOOL CreateCheckYearMonthDayDir(CTime Time, CString sBaseDir, CString& sYearMonthDayDir);

	// Open Video Device
	BOOL OpenVideoDevice(int nId);

	// Open Video From Network
	BOOL OpenGetVideo(CHostPortDlg* pDlg);
	BOOL OpenGetVideo(CString sAddress, DWORD dwConnectDelay = 0U);

	// Connect to the chosen Networking Type and Mode
	typedef enum {
		OTHERONE_SP = 0,	// Other HTTP device (mjpeg)
		OTHERONE_CP,		// Other HTTP device (jpegs)
		AXIS_SP,			// Axis Server Push (mjpeg)
		AXIS_CP,			// Axis Client Poll (jpegs)
		PANASONIC_SP,		// Panasonic Server Push (mjpeg)
		PANASONIC_CP,		// Panasonic Client Poll (jpegs)
		PIXORD_SP,			// Pixord Server Push (mjpeg)
		PIXORD_CP,			// Pixord Client Poll (jpegs)
		EDIMAX_SP,			// Edimax Server Push (mjpeg)
		EDIMAX_CP,			// Edimax Client Poll (jpegs)
		TPLINK_SP,			// TP-Link Server Push (mjpeg)
		TPLINK_CP,			// TP-Link Client Poll (jpegs)
		FOSCAM_SP,			// Foscam Server Push (mjpeg)
		FOSCAM_CP,			// Foscam Client Poll (jpegs)
		// Add more devices here...	
		LAST_DEVICE			// Placeholder for range check
	} NetworkDeviceTypeMode;
	BOOL ConnectGetFrame(DWORD dwConnectDelay = 0U);

	// Dialogs
	void CaptureAssistant();
	void VideoFormatDialog();
	void AudioFormatDialog();

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

	// Show OSD message
	void ShowOSDMessage(const CString& sOSDMessage, COLORREF crOSDMessageColor);
	
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
	__forceinline DWORD GetTotalMovementDetectionListSize()  {	::EnterCriticalSection(&m_csMovementDetectionsList);
																DWORD dwTotalMovementDetectionListSize = m_dwTotalMovementDetectionListSize;
																::LeaveCriticalSection(&m_csMovementDetectionsList);
																return dwTotalMovementDetectionListSize;};
	__forceinline DWORD GetNewestMovementDetectionListSize()  {	::EnterCriticalSection(&m_csMovementDetectionsList);
																DWORD dwNewestMovementDetectionListSize = m_dwNewestMovementDetectionListSize;
																::LeaveCriticalSection(&m_csMovementDetectionsList);
																return dwNewestMovementDetectionListSize;};

	// Detection list handling
	__forceinline void ClearFrameList(CDib::LIST* pFrameList);		// Free all frames in list
	__forceinline void ClearNewestFrameList();						// Free all frames in newest list
	__forceinline void ShrinkNewestFrameList();						// Free oldest frames from newest frame list
																	// making the list m_nMilliSecondsRecBeforeMovementBegin long
	__forceinline void ShrinkNewestFrameListBy(	int nSize,			// Free oldest nSize frames from newest frame list
												DWORD& dwFirstUpTime,
												DWORD& dwLastUpTime);
	__forceinline int  GetNewestMovementDetectionsListCount();		// Get the newest list's count
	__forceinline CDib* AllocMJPGFrame(CDib* pDib,					// Allocate new MJPG frame compressing pDib or copying pMJPGData if available
								LPBYTE pMJPGData, DWORD dwMJPGSize);// (copies also audio samples)
	__forceinline void AddNewFrameToNewestList(CDib* pDib,			// Add new frame to newest list
								LPBYTE pMJPGData, DWORD dwMJPGSize);
	__forceinline void AddNewFrameToNewestListAndShrink(CDib* pDib,	// Add new frame to newest list leaving in the list
								LPBYTE pMJPGData, DWORD dwMJPGSize);// m_nMilliSecondsRecBeforeMovementBegin of frames
																	

	// Main Decode & Process Functions
	void ProcessNoI420NoM420Frame(LPBYTE pData, DWORD dwSize);
	void ProcessM420Frame(LPBYTE pData, DWORD dwSize);
	void ProcessI420Frame(LPBYTE pData, DWORD dwSize, LPBYTE pMJPGData, DWORD dwMJPGSize);
	static BOOL Rotate180(CDib* pDib);
	static BOOL Deinterlace(CDib* pDib);

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
	
	// Video / Audio Recording
	BOOL MakeAVRec(CAVRec** ppAVRec);
	void OpenAVIFile(const CString& sFileName);
	BOOL CaptureRecord(BOOL bShowMessageBoxOnError = TRUE);
	BOOL NextAviFile();
	void NextRecTime(CTime t);
	void CloseAndShowAviRec();
	void FreeAVIFile();

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
										const CString& sAVIFileName = _T(""),
										const CString& sGIFFileName = _T(""),
										const CString& sSWFFileName = _T(""),
										int nMovDetSavesCount = 0);
	void ShowDetectionZones();
	void HideDetectionZones(BOOL bSaveSettingsOnHiding);

	// Email Message Creation
	// The returned CPJNSMTPMessage* is allocated on the heap -> has to be deleted when done!
	static CPJNSMTPMessage* CreateEmailMessage(SendMailConfigurationStruct* pSendMailConfiguration);

	// FTP Upload
	// Return Values
	// -1 : Do Exit Thread (specified for CFTPTransfer constructor)
	// 0  : Error
	// 1  : Ok
	int FTPUpload(	CFTPTransfer* pFTP, FTPUploadConfigurationStruct* pConfig,
					CString sLocalFileName, CString sRemoteFileName);

	// Validate Name
	static CString GetValidName(CString sName);

	// Settings
	void LoadAndDeleteOldZonesSettings();
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

	// Micro Apache
	void ViewWeb();
	void ViewFiles();
	static CString MicroApacheGetConfigFileName();
	static CString MicroApacheGetEditableConfigFileName();
	static CString MicroApacheGetLogFileName();
	static CString MicroApacheGetPidFileName();
	static CString MicroApacheGetPwFileName();
	static void MicroApacheUpdateMainFiles();
	static BOOL MicroApacheUpdateWebFiles(CString sAutoSaveDir);
	static BOOL MicroApacheMakePasswordFile(BOOL bDigest, const CString& sAreaname, const CString& sUsername, const CString& sPassword);
	static BOOL MicroApacheIsPortUsed(int nPort);
	static BOOL MicroApacheInitStart();
	static BOOL MicroApacheWaitStartDone();
	static BOOL MicroApacheWaitCanConnect();
	static BOOL MicroApacheShutdown();
	static int MicroApacheReload(); // Return Values: 1=OK, 0=Failed to stop the web server, -1=Failed to start the web server
	
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
	BOOL EditSnapshot(CDib* pDib, const CTime& Time);
	CString MakeJpegManualSnapshotFileName(const CTime& Time);
	BOOL ThumbMessage(	const CString& sMessage1,
						const CString& sMessage2,
						const CString& sMessage3,
						DWORD dwFirstUpTime,
						DWORD dwLastUpTime);
	__forceinline int SummRectArea(	CDib* pDib,
									int width,
									int posX,
									int posY,
									int rx,
									int ry);

	// Networking Functions
	void InitHttpGetFrameLocations();
	static double GetDefaultNetworkFrameRate(NetworkDeviceTypeMode nNetworkDeviceTypeMode);

// Public Variables
public:
	// General Vars
	CAVRec* volatile m_pAVRec;							// Pointer to the currently recording Avi File
	CRITICAL_SECTION m_csAVRec;							// Critical section for the Avi File
	volatile BOOL m_bInterleave;						// Do not interleave because while recording the frame rate is not yet exactly known!
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
	volatile DWORD m_dwVideoProcessorMode;				// The Processor Mode Variable
	volatile BOOL m_bSizeToDoc;							// If no placement settings in registry size client window to frame size
	volatile BOOL m_bDeviceFirstRun;					// First Time that this device runs
	CTime m_1SecTime;									// For the 1 sec tick in ProcessI420Frame()

	// Threads
	CHttpGetFrameThread m_HttpGetFrameThread;			// Http Networking Helper Thread
	CWatchdogAndDrawThread m_WatchdogAndDrawThread;		// Video Capture Watchdog and Draw Thread
	CDeleteThread m_DeleteThread;						// Delete files older than a given amount of days Thread
	CCaptureAudioThread m_CaptureAudioThread;			// Audio Capture Thread
	CSaveFrameListThread m_SaveFrameListThread;			// Thread which saves the frames in m_FrameArray
	CSaveSnapshotThread m_SaveSnapshotThread;			// Thread which saves the snapshots
	CSaveSnapshotSWFThread m_SaveSnapshotSWFThread;		// Thread which creates the history SWF video file and FTP uploads it

	// Drawing
	CDxDraw* m_pDxDraw;									// Direct Draw Object
	volatile BOOL m_bDecodeFramesForPreview;			// Decode the frames from YUV to RGB32 for display
														// because the format isn't supported by the graphics card
	CRITICAL_SECTION m_csOSDMessage;					// Critical Section for the OSD message vars
	volatile DWORD m_dwOSDMessageUpTime;				// OSD message UpTime
	CString m_sOSDMessage;								// OSD message string
	volatile COLORREF m_crOSDMessageColor;				// OSD message color

	// Watchdog vars
	volatile LONG m_lCurrentInitUpTime;					// Uptime set in ProcessI420Frame()
	volatile BOOL m_bWatchDogAlarm;						// WatchDog Alarm

	// DirectShow Capture Vars
	volatile BOOL m_bStopAndChangeFormat;				// Flag indicating that we are changing the DV format
	volatile BOOL m_bDxDeviceUnplugged;					// Device Has Been Unplugged
	CDxCapture* volatile m_pDxCapture;					// DirectShow Capture Object
	int m_nDeviceInputId;								// Input ID
	int m_nDeviceFormatId;								// Format ID
	int m_nDeviceFormatWidth;							// Format Width
	int m_nDeviceFormatHeight;							// Format Height
	
	// Audio Capture Vars
	volatile DWORD m_dwCaptureAudioDeviceID;			// Audio Capture Device ID
	volatile BOOL m_bCaptureAudio;						// Do Capture Audio Flag

	// Audio / Video Rec
	volatile DWORD m_dwRecFirstUpTime;					// Up-Time of First Recorded Frame
	volatile DWORD m_dwRecLastUpTime;					// Up-Time of Last Recorded Frame
	volatile BOOL m_bRecFirstFrame;						// Recording Just Started
	volatile DWORD m_nRecordedFrames;					// Recorded Frames Count
														// -> For Frame Rate Calculation
	volatile BOOL m_bRecAutoOpen;						// Auto open avi after recording
	volatile BOOL m_bRecTimeSegmentation;				// Enable / Disable Time Segmentation
	volatile int m_nTimeSegmentationIndex;				// Time segmentation combo box index
	CTime m_NextRecTime;								// Next Rec Time for segmentation
	CString m_sRecordAutoSaveDir;						// The Record Directory
	volatile DWORD m_dwVideoRecFourCC;					// Video Compressor FourCC
	volatile int m_nVideoRecDataRate;					// Data Rate in Bits / Sec
	volatile int m_nVideoRecKeyframesRate;				// Keyframes Rate
	volatile float m_fVideoRecQuality;					// 2.0f best quality, 31.0f worst quality
	volatile int m_nVideoRecQualityBitrate;				// 0 -> use quality, 1 -> use bitrate
	volatile int m_nDeleteRecordingsOlderThanDays;		// Delete Recordings older than the given amount of days,
														// 0 means never delete any file!
	// HTTP Get Frame Networking
	CNetCom* volatile m_pGetFrameNetCom;				// Get Frame Instance
	volatile NetworkDeviceTypeMode m_nNetworkDeviceTypeMode;// Get Frame Network Device Type and Mode
	CString m_sGetFrameVideoHost;						// Get Frame video host
	volatile int m_nGetFrameVideoPort;					// Get Frame video port
	CString m_sHttpGetFrameUsername;					// HTTP Username
	CString m_sHttpGetFramePassword;					// HTTP Password
	CHttpGetFrameParseProcess* volatile m_pHttpGetFrameParseProcess; // HTTP Get Frame Parse & Process
	volatile int m_nHttpVideoQuality;					// 0 Best Quality, 100 Worst Quality
	volatile int m_nHttpVideoSizeX;						// Video width
	volatile int m_nHttpVideoSizeY;						// Video height
	CRITICAL_SECTION m_csHttpParams;					// Critical Section for Size and Compression
	CRITICAL_SECTION m_csHttpProcess;					// Critical Section for Processing Image Data
	volatile int m_nHttpGetFrameLocationPos;			// Automatic camera type detection position
	CStringArray m_HttpGetFrameLocations;				// Automatic camera type detection query string

	// Snapshot Vars
	volatile BOOL m_bSnapshotLiveJpeg;					// Live snapshot save as Jpeg
	volatile BOOL m_bSnapshotHistoryJpeg;				// Snapshot history save Jpegs
	volatile BOOL m_bSnapshotHistorySwf;				// Snapshot history save Swf
	volatile BOOL m_bSnapshotLiveJpegFtp;				// Upload Jpeg Live snapshot files
	volatile BOOL m_bSnapshotHistoryJpegFtp;			// Upload Jpeg Snapshot history files
	volatile BOOL m_bSnapshotHistorySwfFtp;				// Upload Swf Snapshot history files
	volatile BOOL m_bManualSnapshotAutoOpen;			// Auto open after executing the manual snapshot command
	volatile int m_nSnapshotRate;						// Snapshot rate in seconds
	volatile int m_nSnapshotRateMs;						// Snapshot rate in ms, effective: 1000 * m_nSnapshotRate + m_nSnapshotRateMs
	volatile int m_nSnapshotHistoryFrameRate;			// Snapshot history framerate
	volatile int m_nSnapshotCompressionQuality;			// Snapshot compression quality
	volatile float m_fSnapshotVideoCompressorQuality;	// Snapshots swf compression quality
	volatile BOOL m_bSnapshotThumb;						// Snapshot thumbnail enable / disable flag
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
	CString m_sDetectionTriggerFileName;				// The external detection trigger file name
	FILETIME m_DetectionTriggerLastWriteTime;			// Last known write time of detection trigger file
	BOOL m_bShowMovementDetections;						// Show / Hide Movement Detection Zones
	BOOL m_bShowEditDetectionZones;						// Show & Edit / Hide Movement Detection Zones
	BOOL m_bShowEditDetectionZonesMinus;				// Add / Remove Movement Detection Zone
	volatile BOOL m_bDetectingMovement;					// Flag Indicating a Detection
	volatile BOOL m_bDetectingMinLengthMovement;		// Flag Indicating a Movement of at least m_nDetectionMinLengthMilliSeconds
	volatile int m_nDetectionLevel;						// Detection Level 1 .. 100 (100 Max Sensibility)
														// a high sensibility may Detect Video Noise!)
	volatile int m_nDetectionZoneSize;					// Configured detection zone size: 0->Big, 1->Medium, 2->Small
	volatile int m_nCurrentDetectionZoneSize;			// Current detection zone size: 0->Big, 1->Medium, 2->Small
	volatile DWORD m_dwFirstDetFrameUpTime;				// Uptime of first movement detection frame
	volatile DWORD m_dwLastDetFrameUpTime;				// Uptime of last movement detection frame
	volatile int m_nMilliSecondsRecBeforeMovementBegin;	// Do record in the circular buffer list this amount of millisec. before det.
	volatile int m_nMilliSecondsRecAfterMovementEnd;	// Keep Recording this amount of millisec. after det. end
	volatile int m_nDetectionMinLengthMilliSeconds;		// Minimum detection length in ms, below this value SaveFrameList() is not called
	volatile int m_nDetectionMaxFrames;					// Maximum number of frames for a detection sequence
	volatile BOOL m_bSaveSWFMovementDetection;			// Save Movement Detections as SWF
	volatile BOOL m_bSaveAVIMovementDetection;			// Save Movement Detections as AVI
	volatile BOOL m_bSaveAnimGIFMovementDetection;		// Save Movement Detections as Animated GIF
	volatile BOOL m_bSendMailMovementDetection;			// Send Email of Movement Detections
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
	volatile DWORD m_dwTotalMovementDetectionListSize;	// The total size in bytes of all movement detection lists
	volatile DWORD m_dwNewestMovementDetectionListSize;	// The size in bytes of the newest movement detection list
	DIBLISTLIST m_MovementDetectionsList;				// The List of Movement Detection Frame Grabbing Lists
	CRITICAL_SECTION m_csMovementDetectionsList;		// Critical Section of the Movement Detections List
	volatile DWORD m_dwAnimatedGifWidth;				// Width of Detection Animated Gif 
	volatile DWORD m_dwAnimatedGifHeight;				// Height of Detection Animated Gif
	CDib* volatile m_pDifferencingDib;					// Differencing Dib
	int* volatile m_MovementDetectorCurrentIntensity;	// Current Intensity by zones (array allocated in constructor)
	DWORD* volatile m_MovementDetectionsUpTime;			// Detection Up-Time For each Zone (array allocated in constructor)
	BYTE* volatile m_MovementDetections;				// Detecting in Zone (array allocated in constructor)
	BYTE* volatile m_DoMovementDetection;				// Do Movement Detection in this Zone with given relative sensibility
														// 0 -> No Detection, 1 -> Full Sensibility=100%, 2 -> 50%, 4 -> 25%, 10 -> 10%
														// (array allocated in constructor)
	volatile int m_nMovementDetectorIntensityLimit;		// Noise Floor
	volatile LONG m_lMovDetXZonesCount;					// Number of zones in X direction (never set to 0 to avoid division by 0)
	volatile LONG m_lMovDetYZonesCount;					// Number of zones in Y direction (never set to 0 to avoid division by 0)
	volatile LONG m_lMovDetTotalZones;					// Total Number of zones (set to 0 when a (re-)init of the zones is wanted)
	volatile DWORD m_dwVideoDetFourCC;					// Video Compressor FourCC
	volatile int m_nVideoDetDataRate;					// Data Rate in Bits / Sec
	volatile int m_nVideoDetKeyframesRate;				// Keyframes Rate
	volatile float m_fVideoDetQuality;					// 2.0f best quality, 31.0f worst quality
	volatile int m_nVideoDetQualityBitrate;				// 0 -> use quality, 1 -> use bitrate
	volatile DWORD m_dwVideoDetSwfFourCC;				// Video Compressor FourCC
	volatile int m_nVideoDetSwfDataRate;				// Data Rate in Bits / Sec
	volatile int m_nVideoDetSwfKeyframesRate;			// Keyframes Rate
	volatile float m_fVideoDetSwfQuality;				// 2.0f best quality, 31.0f worst quality
	volatile int m_nVideoDetSwfQualityBitrate;			// 0 -> use quality, 1 -> use bitrate
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
	SendMailConfigurationStruct m_MovDetSendMailConfiguration;
	FTPUploadConfigurationStruct m_MovDetFTPUploadConfiguration;

	// Property Sheet Pointer
	CVideoDevicePropertySheet* volatile m_pVideoDevicePropertySheet;
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
	afx_msg void OnCaptureSettings();
	afx_msg void OnViewVideo();
	afx_msg void OnUpdateViewVideo(CCmdUI* pCmdUI);
	afx_msg void OnViewFrametime();
	afx_msg void OnUpdateViewFrametime(CCmdUI* pCmdUI);
	afx_msg void OnFileClose();
	afx_msg void OnViewDetections();
	afx_msg void OnUpdateViewDetections(CCmdUI* pCmdUI);
	afx_msg void OnUpdateCaptureSettings(CCmdUI* pCmdUI);
	afx_msg void OnViewDetectionZones();
	afx_msg void OnUpdateViewDetectionZones(CCmdUI* pCmdUI);
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
	afx_msg void OnCaptureAssistant();
	afx_msg void OnUpdateCaptureAssistant(CCmdUI* pCmdUI);
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
