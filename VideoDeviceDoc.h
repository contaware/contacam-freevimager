#if !defined(AFX_VIDEODEVICEDOC_H__24956A13_6E82_4959_931F_F512BE9B17A0__INCLUDED_)
#define AFX_VIDEODEVICEDOC_H__24956A13_6E82_4959_931F_F512BE9B17A0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef VIDEODEVICEDOC

// Includes
#include "uImagerDoc.h"
#include "WorkerThread.h"
#include "DxDraw.h"
#include "pjnsmtp.h"
#include "NetCom.h"
#include "AVRec.h"
#include "AVDecoder.h"
#include "YuvToRgb.h"
#include "NetFrameHdr.h"
#include "SortableFileFind.h"
#include "FTPTransfer.h"
#include "TryEnterCriticalSection.h"
extern "C"
{
#include "ffmpeg\\libavcodec\\avcodec.h"
#include "ffmpeg\\libavformat\\avformat.h"
#include "ffmpeg\\libswscale\\swscale.h"
}

// Forward Declarations
class CVideoDeviceView;
class CVideoDeviceChildFrame;
class CColorDetectionPage;
class CDxCapture;
class CDxCaptureVMR9;
class CVideoAviDoc;
class CAssistantPage;
class CGeneralPage;
class CSnapshotPage;
class CVideoDevicePropertySheet;
class CNetworkPage;
class CMovementDetectionPage;

// General Settings
#define MIN_FRAMERATE						0.1			// fps
#define MAX_FRAMERATE						95.0		// fps
#define PROCESS_MAX_FRAMETIME				13000U		// ms, make sure that: 1000 / MIN_FRAMERATE < PROCESS_MAX_FRAMETIME
#define VFW_MIN_TRIGGEREDCAP_FRAMETIME		200			// ms
#define DEFAULT_FRAMERATE					8.0			// fps
#define HTTPSERVERPUSH_DEFAULT_FRAMERATE	4.0			// fps
#define HTTPSERVERPUSH_EDIMAX_DEFAULT_FRAMERATE	3.0		// fps
#define HTTPCLIENTPOLL_DEFAULT_FRAMERATE	1.0			// fps
#define DX_VMR9_FRAMERATE					30.0		// fps, this is the max. framerate, vmr9 thread can grab with a lower rate
#define DEFAULT_REC_AVIFILE_COUNT			8			// files
#define DEFAULT_REC_AVIFILE_SIZE_MB			1000		// 1000 MB
#define DEFAULT_REC_AVIFILE_SIZE			(1000 * 1024 * 1024)// 1000 MB
#define DEFAULT_AVIREC_DIR					_T("AVIRec")
#define PART_POSTFIX						_T("_part%04d.avi")
#define POSTREC_POSTFIX						_T("_postrec")
#define MAX_DEVICE_AUTORUN_KEYS				32			// Maximum number of devices that can autorun at start-up
#define ACTIVE_VIDEO_STREAM					0			// Video stream 0 for recording and detection
#define ACTIVE_AUDIO_STREAM					0			// Audio stream 0 for recording and detection
#define MIN_DISKFREE_PERCENT				10			// Belove this disk free percentage the oldest files are deleted
#define	FILES_DELETE_INTERVAL				10800000U 	// in ms -> each 3 hours check whether we can delete old detections
#define AUDIO_IN_MIN_BUF_SIZE				8192		// Bytes
#define AUDIO_IN_MIN_SMALL_BUF_SIZE			1024		// Bytes
#define DEFAULT_VIDEOREC_FOURCC				FCC('theo')	// Encoder
#define DEFAULT_VIDEOREC_QUALITY			DEFAULT_THEO_QUALITY
#define DEFAULT_VIDEOPOSTREC_FOURCC			FCC('theo')	// Encoder
#define DEFAULT_VIDEOPOSTREC_QUALITY		DEFAULT_THEO_QUALITY
#define MAX_DX_DIALOGS_RETRY_TIME			3500		// ms
#define DXDRAW_REINIT_TIMEOUT				3000U		// ms

// Watchdog
#define WATCHDOG_DRAW_THRESHOLD				300U		// ms
#define WATCHDOG_CHECK_TIME					110U		// ms
#define WATCHDOG_THRESHOLD					15000U		// ms, make sure that: 1000 / MIN_FRAMERATE < WATCHDOG_THRESHOLD
#define HTTPWATCHDOG_RETRY_TIMEOUT			35000U		// ms, all re-connects after 35s

// Detection Flags
#define NO_DETECTOR							0x00
#define COLOR_DETECTOR						0x01
#define MOVEMENT_DETECTOR					0x02

// Snapshot
#define MIN_SNAPSHOT_RATE					1			// one snapshot per second
#define DEFAULT_SNAPSHOT_RATE				300			// each given seconds
#define DEFAULT_SNAPSHOT_HISTORY_FRAMERATE	15			// fps
#define MIN_SNAPSHOT_HISTORY_FRAMERATE		1			// fps
#define MAX_SNAPSHOT_HISTORY_FRAMERATE		95			// fps
#define DEFAULT_SNAPSHOT_LIVE_FILE			_T("snapshot.jpg")
#define DEFAULT_SNAPSHOT_COMPR_QUALITY		60			// 0 Worst Quality, 100 Best Quality 
#define DEFAULT_SNAPSHOT_THUMB_WIDTH		228			// Must be a multiple of 4 because of swf
#define DEFAULT_SNAPSHOT_THUMB_HEIGHT		172			// Must be a multiple of 4 because of swf

// Movement Detection
#define DEFAULT_VIDEODET_FOURCC				FCC('theo')	// Encoder
#define DEFAULT_VIDEODET_QUALITY			DEFAULT_THEO_QUALITY
#define DEFAULT_PRE_BUFFER_MSEC				2000		// ms
#define DEFAULT_POST_BUFFER_MSEC			8000		// ms
#define DEFAULT_MOVDET_LEVEL				50			// Detection level default value (1 .. 100 = Max sensibility)
#define DEFAULT_MOVDET_INTENSITY_LIMIT		25			// Intensity difference default value
#define MOVDET_MAX_ZONES					1024		// Maximum Number of zones
#define MOVDET_MIN_ZONESX					10			// Minimum Number of zones in X direction
#define MOVDET_MIN_ZONESY					8			// Minimum Number of zones in Y direction
#define MOVDET_MIN_FRAMES_IN_LIST			30			// Min. frames in list before saving the list in the
														// case of insufficient memory
#define MOVDET_MAX_FRAMES_IN_LIST			15000		// 16000 is the limit for swf files -> be safe and start
														// a new list with 15000
#define MOVDET_SAVE_MIN_FRAMERATE_RATIO		0.3			// Min ratio between calculated (last - first) and m_dEffectiveFrameRate
#define MOVDET_TIMEOUT						1000		// Timeout in ms for detection zones
#define MOVDET_MEM_LOAD_THRESHOLD			25.0		// Above this load the detected frames are saved and freed
#define MOVDET_MEM_LOAD_CRITICAL			60.0		// Above this load the detected frames are dropped
#define MOVDET_EXEC_COMMAND_WAIT_TIMEOUT	100			// ms
#define MOVDET_ANIMGIF_MAX_FRAMES			60			// Maximum number of frames per animated gif
#define MOVDET_ANIMGIF_MAX_LENGTH			6000.0		// ms, MOVDET_ANIMGIF_MAX_LENGTH / MOVDET_ANIMGIF_MAX_FRAMES must be >= 100
#define MOVDET_ANIMGIF_DELAY				500.0		// ms (frame time)
#define MOVDET_ANIMGIF_FIRST_FRAME_DELAY	1000		// ms (first frame time)
#define MOVDET_ANIMGIF_LAST_FRAME_DELAY		1000		// ms (last frame time)
#define MOVDET_ANIMGIF_DIFF_MINLEVEL		10			// determines the "inter-frame-compression" of animated gifs
														// higher values better compression but worse quality
#define MOVDET_ANIMGIF_DEFAULT_WIDTH		128			// Default animated gif width
#define MOVDET_ANIMGIF_DEFAULT_HEIGHT		96			// Default animated gif height

// configuration.php
#define PHPCONFIG_VERSION					_T("VERSION")
#define PHPCONFIG_DEFAULTPAGE				_T("DEFAULTPAGE")
#define PHPCONFIG_SUMMARYSNAPSHOT_NAME		_T("summarysnapshot.php")
#define PHPCONFIG_SNAPSHOT_NAME				_T("snapshot.php")
#define PHPCONFIG_SNAPSHOTFULL_NAME			_T("snapshotfull.php")
#define PHPCONFIG_SNAPSHOTHISTORY_NAME		_T("snapshothistory.php")
#define PHPCONFIG_SUMMARYIFRAME_NAME		_T("summaryiframe.php")
#define PHPCONFIG_SUMMARYTITLE				_T("SUMMARYTITLE")
#define PHPCONFIG_SNAPSHOTTITLE				_T("SNAPSHOTTITLE")
#define PHPCONFIG_SNAPSHOT_THUMB			_T("SNAPSHOT_THUMB")
#define PHPCONFIG_SNAPSHOTHISTORY_THUMB		_T("SNAPSHOTHISTORY_THUMB")
#define PHPCONFIG_SNAPSHOTREFRESHSEC		_T("SNAPSHOTREFRESHSEC")
#define PHPCONFIG_THUMBWIDTH				_T("THUMBWIDTH")
#define PHPCONFIG_THUMBHEIGHT				_T("THUMBHEIGHT")
#define PHPCONFIG_WIDTH						_T("WIDTH")
#define PHPCONFIG_HEIGHT					_T("HEIGHT")
#define PHPCONFIG_MAX_PER_PAGE				_T("MAX_PER_PAGE")
#define PHPCONFIG_SHOW_PRINTCOMMAND			_T("SHOW_PRINTCOMMAND")
#define PHPCONFIG_LANGUAGEFILEPATH			_T("LANGUAGEFILEPATH")
#define PHPCONFIG_STYLEFILEPATH				_T("STYLEFILEPATH")
#define PHPCONFIG_MIN_THUMSPERPAGE			7
#define PHPCONFIG_MAX_THUMSPERPAGE			36
#define PHPCONFIG_DEFAULT_THUMSPERPAGE		27

// Color Detection
#define COLDET_MAX_COLORS					8
#define DEFAULT_COLDET_WAITTIME				3000U		// ms
#define COLDET_RADIUS_MARGINE				1.3
#define COLDET_MAX_HUE_RADIUS				80
#define COLDET_MAX_SATURATION_RADIUS		80
#define COLDET_MAX_VALUE_RADIUS				80
#define COLDET_MIN_HUE_RADIUS				25
#define COLDET_MIN_SATURATION_RADIUS		30
#define COLDET_MIN_VALUE_RADIUS				35

// UDP Networking
#define DEFAULT_SENDFRAME_FRAGMENT_SIZE		1400		// bytes
#define SENDFRAME_MIN_FRAGMENT_SIZE			256			// bytes
#define SENDFRAME_MAX_FRAGMENT_SIZE			1400		// bytes
#define DEFAULT_SENDFRAME_DATARATE			300000		// Bits / Sec
#define SENDFRAME_MIN_DATARATE				5000		// Bits / Sec
#define SENDFRAME_MAX_DATARATE				5000000		// Bits / Sec
#define SENDFRAME_MIN_FREQDIV				1
#define SENDFRAME_MAX_FREQDIV				99
#define SENDFRAME_MIN_CONNECTIONS			1
#define SENDFRAME_MAX_CONNECTIONS			99
#define DEFAULT_SENDFRAME_CONNECTIONS		8
#define SENDFRAME_MAX_CONNECTION_TIMEOUT	10000U		// ms
#define SENDFRAME_EXTRADATA_SENDRATE		5000U		// ms
#define SENDFRAME_MIN_KEYFRAME_TIMEDIFF		500U		// Minimum time difference between keyframes in ms
#define SENDFRAME_MAX_KEYFRAME_TIMEDIFF		5000U		// Maximum time difference between keyframes in ms
#define SENDFRAME_MIN_KEYFRAME_BYTESDIFF	100U		// Minimum bytes difference between keyframes
#define SENDFRAME_AUTH_TIMEOUT				60000U		// ms
#define GETFRAME_GENERATOR_RATE				3000U		// ms

// Http Networking
#define DEFAULT_TCP_PORT					80
#define HTTP_MAX_HEADER_SIZE				1400
#define HTTP_MAX_MULTIPART_BOUNDARY			128
#define HTTP_MIN_MULTIPART_SIZE				256
#define DEFAULT_HTTP_VIDEO_QUALITY			30			// 0 Best Quality, 100 Worst Quality
#define DEFAULT_HTTP_VIDEO_SIZE_CX			640
#define DEFAULT_HTTP_VIDEO_SIZE_CY			480
#define HTTPGETFRAME_MAXCOUNT_ALARM1		30
#define HTTPGETFRAME_MAXCOUNT_ALARM2		40
#define HTTPGETFRAME_MAXCOUNT_ALARM3		50
#define HTTPGETFRAME_DELAY_DEFAULT			500U		// ms
#define HTTPGETFRAME_DELAY_ALARM2			1000U		// ms
#define HTTPGETFRAME_DELAY_ALARM3			2000U		// ms
#define HTTPGETFRAME_CONNECTION_TIMEOUT		15000U		// Setup connection timeout in ms


// Post-Recording Parameters
class CPostRecParams
{
public:
	CString m_sSaveFileName;				// File Name
	DWORD m_dwVideoCompressorFourCC;		// Video Compressor FourCC
	int m_nVideoCompressorDataRate;			// Data Rate in Bits / Sec
	int m_nVideoCompressorKeyframesRate;	// Keyframes Rate
	float m_fVideoCompressorQuality;		// 2.0f best quality, 31.0f worst quality
	int m_nVideoCompressorQualityBitrate;	// 0 -> use quality, 1 -> use bitrate
	BOOL m_bDeinterlace;					// De-Interlace
	BOOL m_bCloseWhenDone;					// Close document when done
};

// The Document Class
class CVideoDeviceDoc : public CUImagerDoc
{
public:
	// Types
	typedef CArray<CAVRec*,CAVRec*> AVRECARRAY;
	typedef CList<CDib::LIST*,CDib::LIST*> DIBLISTLIST;
	typedef CList<CNetCom*,CNetCom*> NETCOMLIST;
	
	// Enums
	enum AttachmentType
	{
		ATTACHMENT_NONE				= 0,
		ATTACHMENT_AVI				= 1,
		ATTACHMENT_ANIMGIF			= 2,
		ATTACHMENT_AVI_ANIMGIF		= 3
	};
	enum FilesToUploadType
	{
		FILES_TO_UPLOAD_AVI			= 0,
		FILES_TO_UPLOAD_ANIMGIF		= 1,
		FILES_TO_UPLOAD_SWF			= 2,
		FILES_TO_UPLOAD_AVI_ANIMGIF	= 3,
		FILES_TO_UPLOAD_SWF_ANIMGIF	= 4
	};

	// Get Frame Generator Class
	class CGetFrameGenerator : public CNetCom::CIdleGenerator
	{
		public:
			CGetFrameGenerator() {;};
			BOOL Generate(CNetCom* pNetCom);
	};

	// The Networking Get Frame Parser & Processor Class
	class CGetFrameParseProcess : public CNetCom::CParseProcess
	{
		public:
			CGetFrameParseProcess(CVideoDeviceDoc* pDoc) {m_pDoc = pDoc; Clear();};
			virtual ~CGetFrameParseProcess() {Free();};
			void Close() {Free(); Clear();};
			virtual BOOL Parse(CNetCom* pNetCom);
			__forceinline DWORD GetLostCount() const {return m_dwLostCount;};
#ifdef _DEBUG
			void TraceReSendCount();
#endif

		protected:
			void Free() {	for (DWORD dwFrame = 0U ; dwFrame < NETFRAME_MAX_FRAMES ; dwFrame++)
								FreeFrameFragments(dwFrame);
							if (m_pExtradata)
								av_freep(&m_pExtradata);
							m_nExtradataSize = 0;
							m_nMaxExtradata = 0;
							FreeAVCodec();};
			void Clear() {	for (DWORD dwFrame = 0U ; dwFrame < NETFRAME_MAX_FRAMES ; dwFrame++)
							{
								m_nTotalFragments[dwFrame] = 0;
								m_dwUpTime[dwFrame] = 0U;
								m_dwFrameSize[dwFrame] = 0U;
								m_wFrameSeq[dwFrame] = 0U;
								m_bKeyFrame[dwFrame] = FALSE;
							}
							memset(&m_ReSendCount, 0, NETFRAME_RESEND_ARRAY_SIZE);
							memset(&m_ReSendCountDown, 0, NETFRAME_RESEND_ARRAY_SIZE);
							memset(m_Fragment, 0, sizeof(m_Fragment));
							m_dwPingRT = 0U;
							m_dwLostCount = 0U;
							m_dwLastReSendUpTime = 0U;
							m_dwLastFrameUpTime = 0U;
							m_dwAvgFrameTime = NETFRAME_MIN_FRAME_TIME;
							m_dwLastPresentationUpTime = 0U;
							m_wPrevSeq = 0U;
							m_bInitialized = FALSE;
							m_bFirstFrame = TRUE;
							m_bSeekToKeyFrame = FALSE;
							m_pCodec = NULL;
							m_pCodecCtx = NULL;
							m_CodecId = CODEC_ID_NONE;
							m_pFrame = NULL;
							m_pOutbuf = NULL;
							m_nOutbufSize = 0;
							m_pExtradata = NULL;
							m_nExtradataSize = 0;
							m_nMaxExtradata = 0;
							m_dwEncryptionType = 0U;};
			BOOL SendConfirmation(	CNetCom* pNetCom,
									DWORD dwUpTime,
									DWORD dwFrameSize,
									WORD wSeq,
									BOOL bKeyFrame);
			BOOL SendLostCount(	CNetCom* pNetCom,
								WORD wFirstLostSeq,
								WORD wLastLostSeq,
								WORD wLostCount);
			__forceinline BOOL ReSendFrame(CNetCom* pNetCom, WORD wSeq);
			__forceinline BYTE ReSendCountDown(	int nReSendCount,
												int nCount,
												int nCountOffset);
			__forceinline DWORD CalcFrameSize(DWORD dwFrame);
			__forceinline BOOL IsFrameReady(WORD wSeq);
			__forceinline void FreeFrameFragments(DWORD dwFrame)
			{
				for (int i = 0 ; i < NETFRAME_MAX_FRAGMENTS ; i++)
				{
					if (m_Fragment[dwFrame][i])
					{
						delete m_Fragment[dwFrame][i];
						m_Fragment[dwFrame][i] = NULL;
					}
				}
			}
			// Use the following count function and not a variable
			// because of possible duplicated fragments!
			__forceinline int GetReceivedFragmentsCount(DWORD dwFrame);
			BOOL OpenAVCodec(enum CodecID CodecId, int width, int height);
			void FreeAVCodec(BOOL bNoClose = FALSE);
			BOOL DecodeAndProcess(LPBYTE pFrame, DWORD dwFrameSize);
#ifdef _DEBUG
			void TraceIncompleteFrame(DWORD dwFrame);
#endif

			CVideoDeviceDoc* m_pDoc;
			AVCodec* m_pCodec;
			AVCodecContext* m_pCodecCtx;
			AVFrame* m_pFrame;
			uint8_t* m_pOutbuf;
			int m_nOutbufSize;		// In Bytes
			uint8_t* m_pExtradata;
			int m_nMaxExtradata;
			int m_nExtradataSize;
			CNetCom::CBuf* m_Fragment[NETFRAME_MAX_FRAMES][NETFRAME_MAX_FRAGMENTS];
			int m_nTotalFragments[NETFRAME_MAX_FRAMES];
			DWORD m_dwFrameSize[NETFRAME_MAX_FRAMES];
			DWORD m_dwUpTime[NETFRAME_MAX_FRAMES];
			DWORD m_dwPingRT;
			volatile DWORD m_dwLostCount;
			DWORD m_dwLastReSendUpTime;
			DWORD m_dwLastFrameUpTime;
			DWORD m_dwLastPresentationUpTime;
			DWORD m_dwAvgFrameTime;
			WORD m_wFrameSeq[NETFRAME_MAX_FRAMES];
			WORD m_wPrevSeq;
			BOOL m_bKeyFrame[NETFRAME_MAX_FRAMES];
			BYTE m_ReSendCount[NETFRAME_RESEND_ARRAY_SIZE];
			BYTE m_ReSendCountDown[NETFRAME_RESEND_ARRAY_SIZE];
			BOOL m_bInitialized;
			BOOL m_bFirstFrame;
			BOOL m_bSeekToKeyFrame;
			enum CodecID m_CodecId;
			DWORD m_dwEncryptionType;
	};

	// The Http Networking Get Frame Parser & Processor Class
	class CHttpGetFrameParseProcess : public CNetCom::CParseProcess
	{
		public:
			CHttpGetFrameParseProcess(CVideoDeviceDoc* pDoc) {m_pDoc = pDoc; m_dwCNonceCount = 0U; Clear();};
			virtual ~CHttpGetFrameParseProcess() {FreeAVCodec();};
			void Close() {FreeAVCodec(); Clear();};
			BOOL SendRequest(const CString& sRequest);
			BOOL SendFrameRequest();
			virtual BOOL Parse(CNetCom* pNetCom);
			virtual BOOL Process(unsigned char* pLinBuf, int nSize);
			BOOL HasResolution(const CSize& Size);
			
			CArray<CSize,CSize> m_Sizes;
			CString m_sRealm;
			CString m_sQop;
			CString m_sNonce;
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
			BOOL ParseSingle(	int nSize,
								const CString& sMsg,
								const CString& sMsgLowerCase);
			BOOL ParseMultipart(int nPos,
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
			BOOL OpenAVCodec();
			void FreeAVCodec(BOOL bNoClose = FALSE);
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

	// Re-Send Frame
	class CReSendFrame
	{
		public:
			CReSendFrame(	BYTE* Data,
							int Size,
							DWORD dwFrameUpTime,
							WORD wFrameSeq,
							BOOL bKeyFrame)
						{
							if (Data && Size)
							{
								m_Data = new BYTE[Size];
								if (m_Data)
								{
									memcpy(m_Data, Data, Size);
									m_Size = Size;
								}
							}
							m_dwFrameUpTime = dwFrameUpTime;
							m_wFrameSeq = wFrameSeq;
							m_bKeyFrame = bKeyFrame;
						};
			virtual ~CReSendFrame() {	if (m_Data)
											delete [] m_Data;
									};
			BYTE* m_Data;
			int m_Size;
			DWORD m_dwFrameUpTime;
			BOOL m_bKeyFrame;
			WORD m_wFrameSeq;
	};
	typedef CList<CReSendFrame*,CReSendFrame*> NETCOMRESENDFRAMELIST;

	// The Networking Send Frame Parser & Processor Class
	class CSendFrameParseProcess : public CNetCom::CParseProcess
	{
		public:
			class CSendFrameToEntry
			{
				public:
					CSendFrameToEntry() {	memset(&m_Addr, 0, sizeof(sockaddr_in));
											Clear();};
					virtual ~CSendFrameToEntry() {;};
					__forceinline void Clear() {m_bSendingKeyFrame = FALSE;
												m_bDoSendFirstFrame = FALSE;
												m_dwSentFrameCount = 0U;
												m_dwReSentFrameCount = 0U;
												m_dwLostFrameCount = 0U;
												m_dwConfirmedFrameCount = 0U;
												m_bAuthSent = FALSE;
												m_bAuthFailed = FALSE;
												m_bSet = FALSE;};
					__forceinline BOOL IsAddrSet() {return m_bSet;};
					__forceinline void SetCurrentKeepAliveUpTime() {m_dwKeepAliveUpTime = ::timeGetTime();};
					__forceinline BOOL IsKeepAliveOlderThan(DWORD dwMilliSeconds) {return (::timeGetTime() - m_dwKeepAliveUpTime > dwMilliSeconds);};
					__forceinline void SetAddr(sockaddr_in* pAddr) {memcpy(&m_Addr, pAddr, sizeof(sockaddr_in)); m_bSet = TRUE;};
					__forceinline sockaddr_in* GetAddrPtr() {return &m_Addr;};
					__forceinline BOOL IsAddrEqualTo(sockaddr_in* pAddr) {return IsAddrSet() ? (memcmp(pAddr, &m_Addr, sizeof(sockaddr_in)) == 0) : FALSE;};

					// Sending Keyframe Flag
					volatile BOOL m_bSendingKeyFrame;

					// Do Init Sending First Frame
					volatile BOOL m_bDoSendFirstFrame;

					// Variable updated with each Sent Frame
					volatile DWORD m_dwSentFrameCount;

					// Variable updated with each Re-Sent Frame
					volatile DWORD m_dwReSentFrameCount;

					// Client informs about the lost frames with a message
					volatile DWORD m_dwLostFrameCount;
					
					// Variables Update with each Confirmed Frame
					volatile DWORD m_dwConfirmedFrameCount;

					// Authentication up-time
					volatile DWORD m_dwAuthUpTime;

					// Authentication sequence number
					volatile WORD m_wAuthSeq;

					// Authentication request has been sent
					volatile BOOL m_bAuthSent;
					
					// Last received authentication was wrong
					volatile BOOL m_bAuthFailed;

				protected:
					volatile DWORD m_dwKeepAliveUpTime;			// Up-Time of the last received keep alive packet 
					sockaddr_in m_Addr;							// Address to which to send frames
					volatile BOOL m_bSet;						// Is Table Entry Set (Valid)
			};

		public:
			CSendFrameParseProcess(CVideoDeviceDoc* pDoc) {	::InitializeCriticalSection(&m_csSendToTable); 
															m_pDoc = pDoc; Clear();};
			virtual ~CSendFrameParseProcess() {FreeAVCodec(); ::DeleteCriticalSection(&m_csSendToTable);};
			void Close() {FreeAVCodec(); Clear();};
			virtual BOOL Parse(CNetCom* pNetCom);
			void ClearTable();
			BOOL OpenAVCodec(LPBITMAPINFOHEADER pBMI);
			void FreeAVCodec(BOOL bNoClose = FALSE);
			__forceinline double GetSendFrameRate() const {		return m_pDoc->m_dEffectiveFrameRate > 0.0 ?
																m_pDoc->m_dEffectiveFrameRate / (double)m_nCurrentFreqDiv :
																m_pDoc->m_dFrameRate / (double)m_nCurrentFreqDiv;};
			__forceinline double GetSendKeyFrameRate() const {	return m_pCodecCtx->gop_size > 0 ?
																GetSendFrameRate() / m_pCodecCtx->gop_size :
																GetSendFrameRate();};
			int Encode(CDib* pDib, CTime RefTime, DWORD dwRefUpTime);
			__forceinline BOOL IsKeyFrame() const {return m_pCodecCtx && m_pCodecCtx->coded_frame ?
								(m_pCodecCtx->coded_frame->key_frame == 1 ? TRUE : FALSE) : FALSE;};
			__forceinline unsigned char* GetEncodedDataBuf() const {return (unsigned char*)m_pOutbuf;};

			CSendFrameToEntry m_SendToTable[SENDFRAME_MAX_CONNECTIONS];
			CRITICAL_SECTION m_csSendToTable;
			
		protected:
			BOOL Authenticate(	CNetCom* pNetCom,
								CNetCom::CBuf* pBuf,
								NetFrameHdrPingAuth* pHdr,
								CSendFrameToEntry* pTableEntry);
			void Clear() {	m_pCodec = NULL;
							m_pCodecCtx = NULL;
							m_CodecID = CODEC_ID_H263P;	// Working well: CODEC_ID_MJPEG, CODEC_ID_H263P, CODEC_ID_MPEG4
														// CODEC_ID_THEORA not optimal because it sends the quantization tables with
														// the frames in the extra data field, that uses some bandwidth more!
														// CODEC_ID_H263 is only working with standard resolution of 176 x 144 or 352 x 266						
														// CODEC_ID_SNOW is locking with low resolutions...
							m_pFrame = NULL;
							m_pFrameI420 = NULL;
							m_pImgConvertCtx = NULL;
							m_pI420Buf = NULL;						
							m_dwI420BufSize = 0;
							m_dwI420ImageSize = 0;
							m_pFlipBuf = NULL;
							m_dwFlipBufSize = 0;
							memset(&m_CurrentBMI, 0, sizeof(BITMAPINFOHEADER));
							m_nCurrentDataRate = 0;
							m_nCurrentSizeDiv = 0;
							m_nCurrentFreqDiv = 1;
							m_dCurrentSendFrameRate = 0.0;
							m_pOutbuf = NULL;
							m_nOutbufSize = 0;
							m_dwEncryptionType = 0U;};
			BOOL AddFrameTime(	LPBYTE pBits,
								DWORD dwWidth,
								DWORD dwHeight,
								WORD wBitCount,
								DWORD dwFourCC,
								DWORD dwSizeImage,
								DWORD dwUpTime,
								CTime RefTime,
								DWORD dwRefUpTime);

			double m_dCurrentSendFrameRate;
			CVideoDeviceDoc* m_pDoc;
			AVCodec* m_pCodec;
			AVCodecContext* m_pCodecCtx;
			enum CodecID m_CodecID;
			AVFrame* m_pFrame;
			AVFrame* m_pFrameI420;
			SwsContext* m_pImgConvertCtx;
			LPBYTE m_pI420Buf;						
			DWORD m_dwI420BufSize;
			DWORD m_dwI420ImageSize;
			LPBYTE m_pFlipBuf;
			DWORD m_dwFlipBufSize;
			BITMAPINFOHEADER m_CurrentBMI;
			int m_nCurrentDataRate;
			int m_nCurrentSizeDiv;
			int m_nCurrentFreqDiv;
			uint8_t* m_pOutbuf;
			int m_nOutbufSize;		// In Bytes
			DWORD m_dwLastExtradataSendUpTime;
			DWORD m_dwEncryptionType;
	};

	// The Record Audio File Thread Class
	class CCaptureAudioThread : public CWorkerThread
	{
		public:
			class CMixerIn
			{
				public:
					CMixerIn();
					virtual ~CMixerIn();
					BOOL Open(HWAVEIN hWaveIn, HWND hWndCallBack = NULL);
					void Close();
					BOOL IsOpen() {return (m_hMixer != NULL);};
					BOOL IsWithWndHandleOpen() {return ((m_hMixer != NULL) && (m_hWndMixerCallback != NULL));};
					HMIXER GetHandle() const {return m_hMixer;};
					DWORD GetMux() const;		// Get Current Selected Source
					DWORD GetMuxControlID() const {return m_dwMuxControlID;};
					BOOL GetDstMute(BOOL bForceMono = FALSE) const;
					BOOL GetSrcMute(BOOL bForceMono = FALSE) const;
					BOOL GetDstVolume(DWORD& dwVolumeLeft, DWORD& dwVolumeRight) const;
					BOOL GetSrcVolume(DWORD& dwVolumeLeft, DWORD& dwVolumeRight) const;
					BOOL SetDstVolume(DWORD dwVolumeLeft, DWORD dwVolumeRight);
					BOOL SetSrcVolume(DWORD dwVolumeLeft, DWORD dwVolumeRight);
					DWORD GetDstVolumeControlMin() const {return m_dwVolumeControlMin;};
					DWORD GetDstVolumeControlMax() const {return m_dwVolumeControlMax;};
					DWORD GetSrcVolumeControlMin() const;
					DWORD GetSrcVolumeControlMax() const;
					DWORD GetDstVolumeControlID() const {return m_dwVolumeControlID;};
					DWORD GetSrcVolumeControlID() const;
					DWORD GetDstMuteControlID() const {return m_dwMuteControlID;};
					DWORD GetSrcMuteControlID() const;
					DWORD GetDstNumOfChannels() const;
					DWORD GetSrcNumOfChannels() const;

				protected:
					HMIXER m_hMixer;
					HWND m_hWndMixerCallback;
					UINT m_uiMixerID;
					DWORD m_dwVolumeControlID;
					DWORD m_dwMuteControlID;
					LPDWORD m_pVolumeControlID;
					LPDWORD m_pMuteControlID;
					DWORD m_dwVolumeControlMin;
					DWORD m_dwVolumeControlMax;
					LPDWORD m_pVolumeControlMin;
					LPDWORD m_pVolumeControlMax;
					DWORD m_dwMuxControlID;
					DWORD m_dwMuxControlMin;
					DWORD m_dwMuxControlMax;
					DWORD m_dwMuxMultipleItems;
					DWORD m_dwSourcesCount;
					DWORD m_dwChannels;
					LPDWORD m_pChannels;
					LPDWORD m_pLineID;
			};

			// General Functions
			CCaptureAudioThread();
			virtual ~CCaptureAudioThread();
			void AudioInSourceDialog();
			void SetDoc(CVideoDeviceDoc* pDoc) {m_pDoc = pDoc;};
			void SetDeviceID(UINT uiDeviceID) {m_pDoc->m_dwCaptureAudioDeviceID = uiDeviceID;};
			UINT GetDeviceID() const {return m_pDoc->m_dwCaptureAudioDeviceID;};
			HWAVEIN GetWaveHandle() const {return m_hWaveIn;};
			BOOL IsOpen() const {return (m_hWaveIn != NULL);};
			void SetSmallBuffers(BOOL bSmallBuffers); // Used for fast reaction, like fft calculation for peak meter
			BOOL IsSmallBuffers() {return m_bSmallBuffers;};
			BOOL OpenInAudio();
			void CloseInAudio();
			BOOL DataInAudio(LPBYTE lpData, DWORD dwSize);
			void WaveInitFormat(WORD wCh, DWORD dwSampleRate, WORD wBitsPerSample, LPWAVEFORMATEX pWaveFormat);
			CTime GetMeanLevelTime() const {return m_MeanLevelTime;}; // System Time When Mean Level Has Been Calculated

			// Wave Formats
			LPWAVEFORMATEX m_pSrcWaveFormat;
			LPWAVEFORMATEX m_pDstWaveFormat;

			// Switch to Next AVI File for Segmented Recording
			BOOL NextAviFile(DWORD dwSize, LPBYTE pBuf);

			// Input Mixer Var
			CMixerIn m_Mixer;
			
		protected:
			
			// Thread Functions
			int Work();
			void CalcMeanLevel(DWORD dwSize, LPBYTE pBuf);
			__forceinline void CalcPeak(int nNumOfSamples,
										int nNumOfChannels,
										int nAudioBits, 
										LPBYTE pBuf,
										double& dPeakLeft,
										double& dPeakRight);
			BOOL Record(DWORD dwSize, LPBYTE pBuf);

			// General Vars
			HWAVEIN m_hWaveIn;
			CVideoDeviceDoc* m_pDoc;
			WAVEINCAPS m_WaveInDevCaps;
			WAVEHDR m_WaveHeader[2];
			HANDLE m_hRestartEvent;
			HANDLE m_hWaveInEvent;
			HANDLE m_hEventArray[3];
			int m_nWaveInToggle;
			CTime m_MeanLevelTime; // System Time At Mean Level Calculation
			volatile BOOL m_bSmallBuffers;

			// ACM
			LPBYTE m_pUncompressedBuf[2];
			DWORD m_dwUncompressedBufSize;

			// Buffers for Peak Meter
			double m_dInLeft[AUDIO_IN_MIN_SMALL_BUF_SIZE];
			double m_dInRight[AUDIO_IN_MIN_SMALL_BUF_SIZE];
			double m_dOutRe[AUDIO_IN_MIN_SMALL_BUF_SIZE];
			double m_dOutIm[AUDIO_IN_MIN_SMALL_BUF_SIZE];
	};

	// The VfW Video Capture Thread Class
	class CVfWCaptureVideoThread : public CWorkerThread
	{
		public:
			// General Functions
			CVfWCaptureVideoThread();
			virtual ~CVfWCaptureVideoThread();
			void SetDoc(CVideoDeviceDoc* pDoc) {m_pDoc = pDoc;};
			BOOL CreateCaptureWnd();
			BOOL DestroyCaptureWnd();
			BOOL Connect(double dFrameRate);
			BOOL Disconnect();
			BOOL ConnectForce(double dFrameRate);
			BOOL StartCapture();
			BOOL StopCapture();
			BOOL StartTriggeredFrameCapture();
			BOOL StopTriggeredFrameCapture();
			int GetDroppedFrames();
		
			// Capture Wnd Var
			HWND m_hCapWnd;

		protected:
			int Work();
			static LRESULT CALLBACK OnCaptureVideo(HWND hWnd, LPVIDEOHDR lpVHdr);
			static LRESULT CALLBACK OnFrame(HWND hWnd, LPVIDEOHDR lpVHdr);

			CVideoDeviceDoc* m_pDoc;
			volatile BOOL m_bTriggeredCapture;
	};

	// The VMR9 Video Capture Grab Thread Class
	class CVMR9CaptureVideoThread : public CWorkerThread
	{
		public:
			CVMR9CaptureVideoThread(){m_pDoc = NULL;};
			virtual ~CVMR9CaptureVideoThread(){Kill();};
			void SetDoc(CVideoDeviceDoc* pDoc) {m_pDoc = pDoc;};

		protected:
			int Work();
			CVideoDeviceDoc* m_pDoc;
	};

	// Http Get Frame Thread
	class CHttpGetFrameThread : public CWorkerThread
	{
		public:
			CHttpGetFrameThread() {	m_pDoc = NULL;
									m_hEventArray[0]	= GetKillEvent();							// Kill Event
									m_hEventArray[1]	= ::CreateEvent(NULL, TRUE, FALSE, NULL);	// Http Setup Connection Event
									m_hEventArray[2]	= ::CreateEvent(NULL, TRUE, FALSE, NULL);	// Http Connected Event
									m_hEventArray[3]	= ::CreateEvent(NULL, TRUE, FALSE, NULL);	// Http Read Event
									m_hEventArray[4]	= ::CreateEvent(NULL, TRUE, FALSE, NULL);};	// Http Connect Failed Event
			virtual ~CHttpGetFrameThread() {Kill();
											::CloseHandle(m_hEventArray[1]);
											::CloseHandle(m_hEventArray[2]);
											::CloseHandle(m_hEventArray[3]);
											::CloseHandle(m_hEventArray[4]);};
			void SetDoc(CVideoDeviceDoc* pDoc) {m_pDoc = pDoc;};
			__forceinline HANDLE GetHttpConnectedEvent() const {return m_hEventArray[2];};
			__forceinline BOOL SetEventConnectGetFrameHTTP(LPCTSTR pszHostName, int nPort, LPCTSTR lpszRequest = _T(""))
			{
				m_sHostName = CString(pszHostName);
				m_nPort = nPort;
				m_sRequest = CString(lpszRequest);
				return ::SetEvent(m_hEventArray[1]);	
			};
			__forceinline HANDLE GetHttpReadEvent() const {return m_hEventArray[3];};
			__forceinline HANDLE GetHttpConnectFailedEvent() const {return m_hEventArray[4];};

		protected:
			int Work();
			int OnError();
			__forceinline BOOL Connect(	BOOL bSignalEvents,
										CNetCom* pNetCom,
										CHttpGetFrameParseProcess* pParseProcess,
										LPCTSTR pszHostName,
										int nPort);
			BOOL PollAndClean(BOOL bDoNewPoll);
			void CleanUpAllConnections();
			CVideoDeviceDoc* m_pDoc;
			HANDLE m_hEventArray[5];
			CString m_sHostName;
			int m_nPort;
			CString m_sRequest;
			NETCOMLIST m_HttpGetFrameNetComList;
			NETCOMPARSEPROCESSLIST m_HttpGetFrameParseProcessList;
	};

	// Watch Dog Thread
	class CWatchdogThread : public CWorkerThread
	{
		public:
			CWatchdogThread() {m_pDoc = NULL;};
			virtual ~CWatchdogThread() {Kill();};
			void SetDoc(CVideoDeviceDoc* pDoc) {m_pDoc = pDoc;};

		protected:
			int Work();
			CVideoDeviceDoc* m_pDoc;
	};

	// Delete Thread
	class CDeleteThread : public CWorkerThread
	{
		public:
			CDeleteThread(){m_pDoc = NULL; m_dwCounter = 0;};
			virtual ~CDeleteThread(){Kill();};
			void SetDoc(CVideoDeviceDoc* pDoc) {m_pDoc = pDoc;};

		protected:
			int Work();
			BOOL DeleteDetections();
			BOOL DeleteRecordings();
			BOOL DeleteSnapshots();
			BOOL CalcOldestDir(	CSortableFileFind& FileFind,
								int nDetectionAutoSaveDirSize,
								CTime& OldestDirTime,
								const CTime& CurrentTime);
			BOOL DeleteOld(	CSortableFileFind& FileFind,
							int nDetectionAutoSaveDirSize,
							LONGLONG llDeleteDetectionsOlderThanDays,
							const CTime& CurrentTime);
			CVideoDeviceDoc* m_pDoc;
			DWORD m_dwCounter;
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
		CString			m_sUsername;
		CString			m_sPassword;
		BOOL			m_bAutoDial;
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
		DWORD			m_dwConnectionTimeout;
		BOOL			m_bProxy;
		CString			m_sProxy;
		CString			m_sUsername;
		CString			m_sPassword;
		FilesToUploadType m_FilesToUpload;
	} FTPUploadConfigurationStruct;

	// The Save Frame List Thread Class
	class CSaveFrameListThread : public CWorkerThread
	{
		public:
			CSaveFrameListThread(){m_pDoc = NULL; m_nNumFramesToSave = 0; m_nSendMailProgress = 100; m_nFTPUploadProgress = 100;};
			virtual ~CSaveFrameListThread() {Kill();};
			void SetDoc(CVideoDeviceDoc* pDoc) {m_pDoc = pDoc;};
			void SetFrameList(CDib::LIST* pFrameList) {m_pFrameList = pFrameList;};
			void SetNumFramesToSave(int nNumFramesToSave) {m_nNumFramesToSave = nNumFramesToSave;};
			__forceinline void SetSendMailProgress(int nSendMailProgress) {m_nSendMailProgress = nSendMailProgress;};
			__forceinline int GetSendMailProgress() const {return m_nSendMailProgress;};
			__forceinline void SetFTPUploadProgress(int nFTPUploadProgress) {m_nFTPUploadProgress = nFTPUploadProgress;};
			__forceinline int GetFTPUploadProgress() const {return m_nFTPUploadProgress;};

		protected:
			int Work();
			BOOL SaveSingleGif(		CDib* pDib,
									const CString& sGIFFileName,
									RGBQUAD* pGIFColors);
			void AnimatedGIFInit(	RGBQUAD** ppGIFColors,
									int& nAnimGifLastFrameToSave,
									double& dDelayMul,
									double& dSpeedMul,
									double dCalcFrameRate,
									BOOL bShowFrameTime,
									const CTime& RefTime,
									DWORD dwRefUpTime);
			BOOL SaveAnimatedGif(	CDib* pGIFSaveDib,
									CDib** ppGIFDib,
									CDib** ppGIFDibPrev,
									const CString& sGIFFileName, 
									BOOL* pbFirstGIFSave,
									BOOL bLastGIFSave,
									double dDelayMul,
									double dSpeedMul,
									RGBQUAD* pGIFColors,
									int nDiffMinLevel);
			BOOL SendMailFTPUpload(	const CTime& Time,
									const CString& sAVIFileName,
									const CString& sGIFFileName,
									const CString& sSWFFileName);
			__forceinline BOOL SendMailMovementDetection(	const CTime& Time,
															const CString& sAVIFileName,
															const CString& sGIFFileName);
			__forceinline BOOL FTPUploadMovementDetection(	const CTime& Time,
															const CString& sAVIFileName,
															const CString& sGIFFileName,
															const CString& sSWFFileName);

			__forceinline BOOL DoSaveAvi() const {
							return m_pDoc->m_bSaveAVIMovementDetection					||

							(m_pDoc->m_bSendMailMovementDetection &&
							m_pDoc->m_MovDetSendMailConfiguration.m_AttachmentType ==
											CVideoDeviceDoc::ATTACHMENT_AVI)			||

							(m_pDoc->m_bSendMailMovementDetection &&
							m_pDoc->m_MovDetSendMailConfiguration.m_AttachmentType ==
											CVideoDeviceDoc::ATTACHMENT_AVI_ANIMGIF)	||

							(m_pDoc->m_bFTPUploadMovementDetection &&
							m_pDoc->m_MovDetFTPUploadConfiguration.m_FilesToUpload ==
											CVideoDeviceDoc::FILES_TO_UPLOAD_AVI)		||

							(m_pDoc->m_bFTPUploadMovementDetection &&
							m_pDoc->m_MovDetFTPUploadConfiguration.m_FilesToUpload ==
								CVideoDeviceDoc::FILES_TO_UPLOAD_AVI_ANIMGIF);};

			__forceinline BOOL DoSaveSwf() const {
							return m_pDoc->m_bSaveSWFMovementDetection					||

							(m_pDoc->m_bFTPUploadMovementDetection &&
							m_pDoc->m_MovDetFTPUploadConfiguration.m_FilesToUpload ==
											CVideoDeviceDoc::FILES_TO_UPLOAD_SWF)		||

							(m_pDoc->m_bFTPUploadMovementDetection &&
							m_pDoc->m_MovDetFTPUploadConfiguration.m_FilesToUpload ==
								CVideoDeviceDoc::FILES_TO_UPLOAD_SWF_ANIMGIF);};

			__forceinline BOOL DoSaveGif() const {
							return	m_pDoc->m_bSaveAnimGIFMovementDetection				||

							(m_pDoc->m_bSendMailMovementDetection &&
							m_pDoc->m_MovDetSendMailConfiguration.m_AttachmentType ==
											CVideoDeviceDoc::ATTACHMENT_ANIMGIF)		||

							(m_pDoc->m_bSendMailMovementDetection &&
							m_pDoc->m_MovDetSendMailConfiguration.m_AttachmentType ==
											CVideoDeviceDoc::ATTACHMENT_AVI_ANIMGIF)	||

							(m_pDoc->m_bFTPUploadMovementDetection &&
							m_pDoc->m_MovDetFTPUploadConfiguration.m_FilesToUpload ==
											CVideoDeviceDoc::FILES_TO_UPLOAD_ANIMGIF)	||

							(m_pDoc->m_bFTPUploadMovementDetection &&
							m_pDoc->m_MovDetFTPUploadConfiguration.m_FilesToUpload ==
								CVideoDeviceDoc::FILES_TO_UPLOAD_AVI_ANIMGIF)			||
								
							(m_pDoc->m_bFTPUploadMovementDetection &&
							m_pDoc->m_MovDetFTPUploadConfiguration.m_FilesToUpload ==
								CVideoDeviceDoc::FILES_TO_UPLOAD_SWF_ANIMGIF);};

			// Return Values
			// -1 : Do Exit Thread
			// 0  : Error Sending Email
			// 1  : Ok
			int SendEmail(CString sAVIFile, CString sGIFFile);

			CVideoDeviceDoc* m_pDoc;
			CDib::LIST* m_pFrameList;
			int m_nNumFramesToSave;
			volatile int m_nSendMailProgress;
			volatile int m_nFTPUploadProgress;
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

	// The Save Snapshot Ftp Upload Thread Class
	class CSaveSnapshotFTPThread : public CWorkerThread
	{
		public:
			CSaveSnapshotFTPThread(){;};
			virtual ~CSaveSnapshotFTPThread() {Kill();};
			CString m_sLocalFileName;
			CString m_sRemoteFileName;
			CString m_sLocalThumbFileName;
			CString m_sRemoteThumbFileName;
			FTPUploadConfigurationStruct m_Config;

		protected:
			int Work();
	};

	// The Save Snapshot Thread Class
	class CSaveSnapshotThread : public CWorkerThread
	{
		public:
			CSaveSnapshotThread(){m_pSaveSnapshotFTPThread = NULL; m_pAVRecSwf = NULL; m_pAVRecThumbSwf = NULL;};
			virtual ~CSaveSnapshotThread() {Kill(); SWFFreeCopyFtp();};

			CDib m_Dib;
			BOOL m_bShowFrameTime;
			BOOL m_bSnapshotThumb;
			BOOL m_bSnapshotLiveJpeg;
			BOOL m_bSnapshotHistoryJpeg;
			BOOL m_bSnapshotHistorySwf;
			BOOL m_bSnapshotLiveJpegFtp;
			BOOL m_bSnapshotHistoryJpegFtp;
			BOOL m_bSnapshotHistorySwfFtp;
			BOOL m_bSnapshotHistoryDeinterlace;
			int m_nSnapshotThumbWidth;
			int m_nSnapshotThumbHeight;
			int m_nSnapshotCompressionQuality;
			float m_fSnapshotVideoCompressorQuality;
			double m_dSnapshotHistoryFrameRate;
			CTime m_Time;
			CTime m_NextRecTime;
			CString m_sSnapshotAutoSaveDir;
			CSaveSnapshotFTPThread* m_pSaveSnapshotFTPThread;
			FTPUploadConfigurationStruct m_Config;

		protected:
			int Work();
			__forceinline void SWFFreeCopyFtp(BOOL bFtp = FALSE);
			__forceinline CString MakeJpegHistoryFileName();
			__forceinline CString MakeSwfHistoryFileName();

			CAVRec* m_pAVRecSwf;
			CAVRec* m_pAVRecThumbSwf;
			CString m_sSWFFileName;
			CString m_sSWFTempFileName;
			CString m_sSWFThumbFileName;
			CString m_sSWFTempThumbFileName;
	};

	// The Color Detection Class
	class CColorDetection
	{
		public:
			struct HsvEntry
			{
				int hue;
				int saturation;
				int value;
			};
			typedef CArray<HsvEntry,HsvEntry&> HSVARRAY;
			class CColDetEntry
			{
				public:
					CColDetEntry() {Clear();};
					virtual ~CColDetEntry() {;};
					void Clear() {	red = 0;
									green = 0;
									blue = 0;
									hue = 0;
									huemin = 0;
									huemax = 0;
									saturation = 0;
									saturationmin = 0;
									saturationmax = 0;
									value = 0;
									valuemin = 0;
									valuemax = 0;};
					BOOL SetHSVArray(HSVARRAY& a);
					volatile int red;
					volatile int green;
					volatile int blue;
					volatile int hue;
					volatile int huemax;
					volatile int huemin;
					volatile int saturation;
					volatile int saturationmax;
					volatile int saturationmin;
					volatile int value;
					volatile int valuemax;
					volatile int valuemin;
			};
		public:
			CColorDetection();
			virtual ~CColorDetection();
			void ResetCounter();
			BOOL Detector(CDib* pDib, DWORD dwDetectionAccuracy, BOOL bColorImage = FALSE);
			static COLORREF CalcMeanValue(CDib* pDib, DWORD dwCalcAccuracy);
			int AppendColor(HSVARRAY& a);
			BOOL ReplaceColor(DWORD dwIndex, HSVARRAY& a);
			BOOL RemoveColor(DWORD dwIndex);
			DWORD GetColorsCount();
			COLORREF GetColor(DWORD dwIndex);
			BOOL SetDetectionThreshold(DWORD dwIndex, DWORD dwThreshold); // 0 .. 10000
			BOOL SetHueRadius(DWORD dwIndex, DWORD dwRadius);
			BOOL SetSaturationRadius(DWORD dwIndex, DWORD dwRadius);
			BOOL SetValueRadius(DWORD dwIndex, DWORD dwRadius);
			int GetHueRadius(DWORD dwIndex);
			int GetSaturationRadius(DWORD dwIndex);
			int GetValueRadius(DWORD dwIndex);
			int GetDetectionLevel(DWORD dwIndex); // 0 .. 10000, -1 if error
			DWORD GetDetectionCountup(DWORD dwIndex) {	::EnterCriticalSection(&m_cs);
														DWORD countup = m_DetectionCountup[dwIndex];
														::LeaveCriticalSection(&m_cs);
														return countup;};
			DWORD GetTimeBetweenCounts(DWORD dwIndex);
			DWORD GetShortestTimeBetweenCounts(DWORD dwIndex);
			DWORD GetLongestTimeBetweenCounts(DWORD dwIndex);
			DWORD GetMaxCountsColorIndexes() {	::EnterCriticalSection(&m_cs);
												DWORD index = m_dwMaxCountsColorIndexes;
												::LeaveCriticalSection(&m_cs);
												return index;};
			DWORD GetShortestTimeBetweenCountsColorIndexes() {	::EnterCriticalSection(&m_cs);
																DWORD index = m_dwShortestTimeBetweenCountsColorIndexes;
																::LeaveCriticalSection(&m_cs);
																return index;};
			DWORD GetLongestTimeBetweenCountsColorIndexes() {	::EnterCriticalSection(&m_cs);
																DWORD index = m_dwLongestTimeBetweenCountsColorIndexes;
																::LeaveCriticalSection(&m_cs);
																return index;};
			void SetWaitCount(DWORD dwMaxWaitCount) {	::EnterCriticalSection(&m_cs);
														m_dwMaxWaitCount = dwMaxWaitCount;
														::LeaveCriticalSection(&m_cs);};

		protected:
			CRITICAL_SECTION m_cs;
			CColDetEntry m_ColDetTable[COLDET_MAX_COLORS];
			volatile int m_nColDetCount;

			// Detection Level : 0 .. 10000
			DWORD volatile m_DetectionLevels[COLDET_MAX_COLORS];
			
			// If a Detection Level is greater o equal the threshold -> the detection countup is incremented
			DWORD volatile m_DetectionLevelsThresholds[COLDET_MAX_COLORS];
			DWORD volatile m_DetectionCountup[COLDET_MAX_COLORS];
			DWORD volatile m_WaitCountup[COLDET_MAX_COLORS];
			volatile DWORD m_dwMaxWaitCount;
			DWORD volatile m_DetectionTime[COLDET_MAX_COLORS];
			DWORD volatile m_TimeBetweenCounts[COLDET_MAX_COLORS];
			DWORD volatile m_ShortestTimeBetweenCounts[COLDET_MAX_COLORS];
			DWORD volatile m_LongestTimeBetweenCounts[COLDET_MAX_COLORS];
			volatile DWORD m_dwMaxCountsColorIndexes;
			volatile DWORD m_dwShortestTimeBetweenCountsColorIndexes;
			volatile DWORD m_dwLongestTimeBetweenCountsColorIndexes;
	};

protected: // create from serialization only
	DECLARE_DYNCREATE(CVideoDeviceDoc)
	CVideoDeviceDoc();
	virtual ~CVideoDeviceDoc();
	

// Public Functions
public:
	
	// General Functions
	void CloseDocument();				// Close Document by sending a WM_CLOSE to the Parent Frame
	void CloseDocRemoveAutorunDev();	// Closes Document and remove device from autorun
	CString GetAssignedDeviceName();	// Get User Assigned Device Name	
	CString GetDeviceName();			// Friendly Device Name
	CString GetDevicePathName();		// Used For Settings, Scheduler and Autorun
	void SetDocumentTitle();
	CVideoDeviceView* GetView() const {return m_pView;};
	void SetView(CVideoDeviceView* pView) {m_pView = pView;};
	CVideoDeviceChildFrame* GetFrame() const {return m_pFrame;};
	void SetFrame(CVideoDeviceChildFrame* pFrame) {m_pFrame = pFrame;};
	static __forceinline BOOL CreateCheckYearMonthDayDir(CTime Time, CString sBaseDir, CString& sYearMonthDayDir);
	static BOOL AddFrameTime(CDib* pDib, CTime RefTime, DWORD dwRefUpTime);
	void ViewVideo();					// Show / Hide Video Preview

	// Open Video Device
	BOOL OpenVideoDevice(int nId);

	// Open Video From Avi Player
	BOOL OpenVideoAvi(CVideoAviDoc* pDoc, CDib* pDib);

	// Open Video From Network, Pop-Up
	// a Dialog for address, port and
	// connection type selection
	BOOL OpenGetVideo();

	// Open Video From Network
	BOOL OpenGetVideo(CString sAddress);	

	// Dialogs
	void CaptureSettings();
	void VideoFormatDialog();
	void VideoSourceDialog();
	void VideoInputDialog();
	void VideoTunerDialog();
	void AudioFormatDialog();
	void VfWVideoFormatDialog();
	void VfWVideoSourceDialog();
	
	// On Change Frame Rate
	void OnChangeFrameRate();
	
	// List Convention
	//
	// Tail <-> New
	// Head <-> Old
	//

	// Detection lists
	__forceinline void OneFrameList();							// ClearMovementDetectionsList() + Create a New Frame List (all in one critical section) 
	__forceinline void ClearMovementDetectionsList();			// Free and remove all lists
	__forceinline void RemoveOldestMovementDetectionList();		// Free and remove oldest list
	__forceinline BOOL SaveFrameList();							// Add new empty list + Start thread which saves the oldest list(s)
	__forceinline int  GetTotalMovementDetectionFrames();		// Get the total frames over all lists

	// Detection list handling
	__forceinline void ClearFrameList(CDib::LIST* pFrameList);	// Free all frames in list
	__forceinline void ClearNewestFrameList();					// Free all frames in newest list
	__forceinline void ShrinkNewestFrameListTo(int nMinSize);	// Free oldest frames and leave the newest nMinSize from newest frame list
	__forceinline void ShrinkNewestFrameListBy(int nSize);		// Free oldest nSize frames from newest frame list
	__forceinline int  GetNewestMovementDetectionsListCount();	// Get the newest list's count
	__forceinline void AddNewFrameToNewestList(CDib* pDib);		// Add new frame to newest list
	__forceinline void RemoveOldestFrameFromNewestList();		// Free and remove oldest frame from newest list

	// Main Decode & Process Functions
	static void HCWToI420(	unsigned char *src,
							unsigned char *dst,
							int width,
							int height,
							int srcbufsize);
	BOOL DecodeFrameToRgb24(LPBYTE pSrcBits, DWORD dwSrcSize, CDib* pDstDib);
	BOOL DecodeMpeg2Frame(LPBYTE pSrcBits, DWORD dwSrcSize, CDib* pDstDib);
	BOOL Snapshot(CDib* pDib, const CTime& Time);
	BOOL EditCopy(CDib* pDib, const CTime& Time);
	BOOL ProcessFrame(LPBYTE pData, DWORD dwSize);

	// To Start / Stop Frame Processing and Avoid Dead-Locks!
	__forceinline void StopProcessFrame()	{	if (!m_bProcessFrameStopped)
													::InterlockedExchange(&m_bStopProcessFrame, 1);};		// Stop Processing, No Blocking
	__forceinline void ReStartProcessFrame() {	if (IsProcessFrameStopped())								// Restart Processing
												{
													// Reset Frame Rate Calculation
													m_dwEffectiveFrameTimeCountUp = 0U;
													m_dEffectiveFrameTimeSum = 0.0;
													::InterlockedExchange(&m_bProcessFrameStopped, 0);		// Reset stopped flag
												}
											};
	__forceinline LONG IsProcessFrameStopped() const {return (	m_bProcessFrameStopped  &&					// Processing Terminated if 1
																!m_bStopProcessFrame);};
	__forceinline void SetProcessFrameStopped()	{	// Do not invert the order of these two instructions!
													::InterlockedExchange(&m_bProcessFrameStopped, 1);		// Set stopped state
													::InterlockedExchange(&m_bStopProcessFrame, 0);};
	
	// Video / Audio Recording
	__forceinline BOOL MakeAVRec(const CString& sFileName, CAVRec** ppAVRec);
	__forceinline CString MakeRecFileName();
	__forceinline void ChangeRecFileFrameRate(double dFrameRate = 0.0);
	__forceinline void OpenAndPostProcess();
	BOOL CaptureRecord(BOOL bShowMessageBoxOnError = TRUE);
	void CaptureRecordPause();
	__forceinline BOOL IsRecording() {return m_pAVRec != NULL;};
	void AllocateCaptureFiles();
	BOOL NextAviFile();
	void NextRecTime(CTime t);
	void CloseAndShowAviRec();
	void FreeAVIFiles();

	// Movement Detection
	void MovementDetectionOn();
	void MovementDetectionOff();
	void MovementDetectionProcessing(	CDib* pDib,
										BOOL bMovementDetectorPreview,
										BOOL bDoDetection);
	BOOL MovementDetector(	CDib* pDib,
							BOOL bPlanar,					
							int nDetectionLevel);
	void ResetMovementDetector();
	void FreeMovementDetector();

	// Email Message Creation
	CPJNSMTPMessage* CreateEmailMessage();

	// Color Detection
	void ColorDetectionProcessing(CDib* pDib, BOOL bColorDetectionPreview);
	void SetColorDetectionWaitTime(DWORD dwWaitMilliseconds);
	__forceinline DWORD GetColorDetectionWaitTime() const {return m_dwColorDetectionWaitTime;};

	// Functin called when the video grabbing format has been changed
	void OnChangeVideoFormat();

	// Get Vfw Capture Driver Information
	static BOOL GetCaptureDriverDescription(WORD nIndex, CString& sName, CString& sVersion);

	// Networking Type and Mode
	typedef enum {
		INTERNAL_UDP = 0,	// Internal UDP Server
		OTHERONE,			// Other HTTP device
		AXIS_SP,			// Axis Server Push (mjpeg)
		AXIS_CP,			// Axis Client Poll (jpegs)
		PANASONIC_SP,		// Panasonic Server Push (mjpeg)
		PANASONIC_CP,		// Panasonic Client Poll (jpegs)
		PIXORD_SP,			// Pixord Server Push (mjpeg)
		PIXORD_CP,			// Pixord Client Poll (jpegs)
		EDIMAX_SP,			// Edimax Server Push (mjpeg)
		//EDIMAX_CP			not supported because of different resolution, compression and framerate set
		// Add more devices here...	
		LAST_DEVICE			// Placeholder for range check
	} NetworkDeviceTypeMode;
	BOOL ConnectGetFrame();

	// Http Networking Function
	__forceinline BOOL ConnectGetFrameHTTP(LPCTSTR pszHostName, int nPort, LPCTSTR lpszRequest = _T("")){
				return m_HttpGetFrameThread.SetEventConnectGetFrameHTTP(pszHostName, nPort, lpszRequest);};
	
	// UDP Networking Functions
	BOOL ConnectSendFrameUDP(CNetCom* pNetCom, int nPort);
	BOOL ConnectGetFrameUDP(LPCTSTR pszHostName, int nPort);
	BOOL StoreUDPFrame(BYTE* Data, int Size, DWORD dwFrameUpTime, WORD wFrameSeq, BOOL bKeyFrame);
	BOOL ReSendUDPFrame(sockaddr_in* pTo, WORD wFrameSeq);
	void ClearReSendUDPFrameList();
	BOOL SendUDPFrame(	CNetCom* pNetCom,
						sockaddr_in* pTo, // if NULL send to all!
						BYTE* Data,
						int Size,
						DWORD dwFrameUpTime,
						WORD wFrameSeq,
						BOOL bKeyFrame,
						int nMaxFragmentSize,
						BOOL bHighPriority,
						BOOL bReSending);
	void ShowSendFrameMsg();

	// Validate Name
	static CString GetValidName(CString sName);

	// Settings
	void LoadSettings(double dDefaultFrameRate, CString sSection, CString sDeviceName);
	void SaveSettings();

	// Autorun
	static CString AutorunGetDeviceKey(const CString& sDevicePathName);
	static CString AutorunAddDevice(const CString& sDevicePathName);
	static CString AutorunRemoveDevice(const CString& sDevicePathName);

	// Get common auto-save directory
	CString GetAutoSaveDir();

	// Micro Apache
	static CString MicroApacheGetConfigFileName();
	static CString MicroApacheGetLogFileName();
	static CString MicroApacheGetPidFileName();
	static CString MicroApacheGetPwFileName();
	static BOOL MicroApacheCheckConfigFile();
	static BOOL MicroApacheCheckWebFiles(CString sAutoSaveDir, BOOL bOverwrite = FALSE);
	static BOOL MicroApacheMakePasswordFile(BOOL bDigest, const CString& sUsername, const CString& sPassword);
	static BOOL MicroApacheInitStart();
	static BOOL MicroApacheWaitStartDone();
	static BOOL MicroApacheWaitCanConnect();
	static void MicroApacheInitShutdown();
	static BOOL MicroApacheFinishShutdown();
	static CString MicroApacheConfigFileGetParam(const CString& sParam);						// sParam is case sensitive!
	static BOOL MicroApacheConfigFileSetParam(const CString& sParam, const CString& sValue);	// sParam is case sensitive!
	
	// Php
	CString PhpGetConfigFileName();
	BOOL PhpConfigFileSetParam(const CString& sParam, const CString& sValue);					// sParam is case sensitive!
	CString PhpConfigFileGetParam(const CString& sParam);										// sParam is case sensitive!

// Protected Functions
protected:
	BOOL InitOpenDxCapture(int nId);
	BOOL InitOpenDxCaptureVMR9(int nId);
	static __forceinline BOOL IsDeinterlaceSupported(LPBITMAPINFO pBmi);
	static __forceinline BOOL IsDeinterlaceSupported(CDib* pDib);
	BOOL Deinterlace(CDib* pDib);											// Inplace De-Interlace
	BOOL Deinterlace(CDib* pDstDib, LPBITMAPINFO pSrcBMI, LPBYTE pSrcBits);	// De-Interlace Src and put it to Dst,
																			// Dst bits are Allocate by the function
	BOOL RecError(BOOL bShowMessageBoxOnError, CAVRec* pAVRec = NULL);
	void CheckRecDir();
	__forceinline void MovementDetectorPreview(CDib* pDib);
	__forceinline int SummRectArea(	CDib* pDib,
									BOOL bPlanar,
									int width,
									int posX,
									int posY,
									int rx,
									int ry);
	__forceinline int GetAppMemoryUsageMB();
	__forceinline double GetAppMemoryLoad();

	// Networking Functions
	static double GetDefaultNetworkFrameRate(NetworkDeviceTypeMode nNetworkDeviceTypeMode);
	__forceinline BOOL SendUDPFragment(	CNetCom* pNetCom,
										sockaddr_in* pTo, // if NULL send to all!
										BYTE* Hdr,
										int HdrSize,
										BYTE* Data,
										int DataSize,
										BOOL bHighPriority,
										BOOL bReSending);
	__forceinline void SendUDPFragmentInternal(	CNetCom* pNetCom,
												int nTo,
												BYTE* Hdr,
												int HdrSize,
												BYTE* Data,
												int DataSize,
												BOOL bHighPriority,
												BOOL bReSending);
	int UpdateFrameSendToTable();

	// Micro Apache Functions
	static CString LoadMicroApacheConfigFile();
	static BOOL SaveMicroApacheConfigFile(const CString& sConfig);
	void MicroApacheViewOnWeb(CString sAutoSaveDir, const CString& sWebPageFileName);

	// Php
	CString LoadPhpConfigFile();
	BOOL SavePhpConfigFile(const CString& sConfig);

// Public Variables
public:
	// Bitmap info full struct
	typedef struct tagBITMAPINFOFULL {
		BITMAPINFOHEADER    bmiHeader;
		RGBQUAD             bmiColors[256];
	} BITMAPINFOFULL;

	// General Vars
	CAVRec* volatile m_pAVRec;							// Pointer to the currently recording Avi File
	CRITICAL_SECTION m_csAVRec;							// Critical section for the Avi File
	volatile BOOL m_bInterleave;						// Do not interleave because while recording the frame rate is not yet exactly known!
	volatile BOOL m_bDeinterlace;						// De-Interlace Video
	AVRECARRAY m_AVRecs;								// Array of Opened files for segmented recording
	volatile double m_dFrameRate;						// Set Capture Frame Rate
	volatile double m_dEffectiveFrameRate;				// Current Calculated Frame Rate
	volatile BOOL m_bRgb24Frame;						// Current Frame is RGB24 (Converted to or originally 24 bpp)
	volatile BOOL m_bI420Frame;							// Current Frame is I420
	volatile LONG m_lProcessFrameTime;					// Time in ms inside ProcessFrame()
	volatile LONG m_lCompressedDataRate;				// Compressed data rate in bytes / sec
	volatile LONG m_lCompressedDataRateSum;				// Compressed data rate sum
	BITMAPINFOFULL m_OrigBMI;							// Original BMI of Frame
	volatile BOOL m_bCapture;							// Flag indicating whether the grabbing device is running
	volatile LONG m_bCaptureStarted;					// Flag set when first frame has been processed 
	CTime m_CaptureStartTime;							// Grabbing device started at this time
	volatile BOOL m_bVideoView;							// Flag indicating whether the frame grabbing is to be previewed
	volatile BOOL m_bShowFrameTime;						// Show / Hide Frame Time Inside the Frame (frame time is also recorded)
	volatile BOOL m_bDoEditCopy;						// Copy Frame to Clipboard in ProcessFrame()
	volatile BOOL m_bDoEditPaste;						// Paste Frame when copy done
	volatile DWORD m_dwFrameCountUp;					// Captured Frames Count-Up, it can wrap around!
	volatile DWORD m_VideoProcessorMode;				// The Processor Mode Variable
	CString m_sDetectionAutoSaveDir;					// The Detection Directory
	CVideoAviDoc* volatile m_pVideoAviDoc;				// Video source from a Avi Player Doc
	volatile BOOL m_bSizeToDoc;							// If no placement settings in registry size client window to frame size
	volatile BOOL m_bFirstRun;							// First Time that this device runs

	// Threads
	CHttpGetFrameThread m_HttpGetFrameThread;			// Http Networking Helper Thread
	CWatchdogThread m_WatchdogThread;					// Video Capture Watchdog Thread
	CDeleteThread m_DeleteThread;						// Delete files older than a given amount of days Thread
	CVfWCaptureVideoThread m_VfWCaptureVideoThread;		// VfW Video Capture Thread
	CVMR9CaptureVideoThread m_VMR9CaptureVideoThread;	// VMR9 Video Capture Thread
	CCaptureAudioThread m_CaptureAudioThread;			// Audio Capture Thread
	CSaveFrameListThread m_SaveFrameListThread;			// Thread which saves the frames in m_FrameArray
	CSaveSnapshotThread m_SaveSnapshotThread;			// Thread which saves the snapshots
	CSaveSnapshotFTPThread m_SaveSnapshotFTPThread;		// Thread which ftp uploads the swf snapshots history

	// Mpeg Video Decoder
	CAVDecoder m_AVDecoder;								// Mpeg Video Decoder

	// Drawing
	CDxDraw m_DxDraw;									// Direct Draw Object
	volatile BOOL m_bDecodeFramesForPreview;			// Decode the frames from YUV to RGB for display
														// because the format isn't supported for display

	// Watchdog vars
	volatile LONG m_lCurrentInitUpTime;					// Uptime set in ProcessFrame()
	volatile LONG m_bWatchDogAlarm;						// WatchDog Alarm

	// DirectShow Capture Vars
	volatile LONG m_bDxDeviceUnplugged;					// Device Has Been Unplugged
	volatile LONG m_bStopAndChangeFormat;				// Flag indicating that we are changing the DV format
	volatile LONG m_bStopAndCallVideoSourceDialog;		// Flag indicating that we are calling the video source dialog
	volatile BOOL m_bDxFrameGrabCaptureFirst;			// Try dx frame grab capture first
	CDxCapture* volatile m_pDxCapture;					// DirectShow Capture Object
	CDxCaptureVMR9* volatile m_pDxCaptureVMR9;			// DirectShow Capture Object Through VMR9
	int m_nDeviceInputId;								// Input ID
	int m_nDeviceFormatId;								// Format ID
	int m_nDeviceFormatWidth;							// Format Width
	int m_nDeviceFormatHeight;							// Format Height

	// Vfw Capture Vars
	DWORD m_dwVfWCaptureVideoDeviceID;					// VfW Video Capture Device ID
	volatile BOOL m_bVideoFormatApplyPressed;			// VfW Format Dialog Hack
	volatile BOOL m_bVfWDialogDisplaying;				// VfW is Displaying right now
	
	// Audio Capture Vars
	DWORD m_dwCaptureAudioDeviceID;						// Audio Capture Device ID
	volatile BOOL m_bCaptureAudio;						// Do Capture Audio Flag

	// Audio / Video Rec
	volatile BOOL m_bAudioRecWait;						// If set the Audio Recording is Waiting, not recording
	volatile BOOL m_bVideoRecWait;						// If set the Video Recording is Waiting, not recording
	volatile BOOL m_bStopRec;							// Do Start Stopping Recording
	volatile BOOL m_bCaptureRecordPause;				// Recording Paused
	volatile BOOL m_bRecResume;							// Resume After Recording
	volatile BOOL m_bAboutToStopRec;					// Recording is Stopping
	volatile BOOL m_bAboutToStartRec;					// Recording is Starting
	volatile DWORD m_dwRecFirstUpTime;					// Up-Time of First Recorded Frame
	volatile DWORD m_dwRecLastUpTime;					// Up-Time of Last Recorded Frame
	volatile BOOL m_bRecFirstFrame;						// Recording Just Started
	volatile DWORD m_nRecordedFrames;					// Recorded Frames Count
														// -> For Frame Rate Calculation
	volatile BOOL m_bRecAutoOpen;						// Auto open avi after recording
	volatile BOOL m_bRecAutoOpenAllowed;				// Flag which allows auto open of avi after recording
	volatile int m_nRecFilePos;							// 0 = Current recording file is the first one
														// 1 = This is the second recording file,
														//     already recorded one file of the given max size
														// 2 = ...
	volatile BOOL m_bRecSizeSegmentation;				// Enable / Disable Size Segmentation
	volatile BOOL m_bRecTimeSegmentation;				// Enable / Disable Time Segmentation
	volatile int m_nTimeSegmentationIndex;				// Time segmentation combo box index
	CTime m_NextRecTime;								// Next Rec Time for segmentation
	volatile int m_nRecFileCount;						// The Maximum Number of Segments to Record
	volatile LONGLONG m_llRecFileSize;					// Maximum size of one Segment in Bytes
	CString m_sFirstRecFileName;						// The First Recording File Name
	CString m_sRecordAutoSaveDir;						// The Record Directory
	volatile BOOL m_bRecDeinterlace;					// Recording De-Interlace
	volatile DWORD m_dwVideoRecFourCC;					// Video Compressor FourCC
	volatile int m_nVideoRecDataRate;					// Data Rate in Bits / Sec
	volatile int m_nVideoRecKeyframesRate;				// Keyframes Rate
	volatile float m_fVideoRecQuality;					// 2.0f best quality, 31.0f worst quality
	volatile int m_nVideoRecQualityBitrate;				// 0 -> use quality, 1 -> use bitrate
	volatile DWORD m_dwVideoPostRecFourCC;				// Video Compressor FourCC
	volatile int m_nVideoPostRecDataRate;				// Data Rate in Bits / Sec
	volatile int m_nVideoPostRecKeyframesRate;			// Keyframes Rate
	volatile float m_fVideoPostRecQuality;				// 2.0f best quality, 31.0f worst quality
	volatile int m_nVideoPostRecQualityBitrate;			// 0 -> use quality, 1 -> use bitrate
	volatile BOOL m_bPostRec;							// Enable / Disable Post Recording Compress
	volatile int m_nDeleteRecordingsOlderThanDays;		// Delete Recordings older than the given amount of days,
														// 0 means never delete any file!
	// Get Frame Networking for both UDP and HTTP
	CNetCom* volatile m_pGetFrameNetCom;				// Get Frame Instance
	volatile NetworkDeviceTypeMode m_nNetworkDeviceTypeMode;// Get Frame Network Device Type and Mode
	CString m_sGetFrameVideoHost;						// Get Frame video host
	volatile int m_nGetFrameVideoPort;					// Get Frame video port

	// UDP Get Frame Networking
	CGetFrameParseProcess* volatile m_pGetFrameParseProcess;// UDP Get Frame Parse & Process
	CGetFrameGenerator* volatile m_pGetFrameGenerator;	// UDP Get Frame Generator (Keep Alive Pings)
	volatile DWORD m_dwGetFrameMaxFrames;				// Buffering queue size
	volatile BOOL m_bGetFrameDisableResend;				// Enable / Disable re-send feature
	CString m_sGetFrameUsername;						// UDP Username
	CString m_sGetFramePassword;						// UDP Password

	// HTTP Get Frame Networking
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

	// Send Frame Networking
	CNetCom* volatile m_pSendFrameNetCom;				// UDP Send Frame Instance
	CSendFrameParseProcess* volatile m_pSendFrameParseProcess;// UDP Send Frame Parse & Process	
	CRITICAL_SECTION m_csSendFrameNetCom;				// UDP Send Frame Critical Section
	volatile BOOL m_bSendVideoFrame;					// Enable / Disable UDP Send Frame
	volatile int m_nSendFrameVideoPort;					// Send Frame Video Port
	volatile int m_nSendFrameMaxConnections;			// Send Frame max number of supported connections
	volatile int m_nSendFrameMTU;						// Send Frame max fragment size
	volatile int m_nSendFrameDataRate;					// Data Rate in Bits / Sec
	volatile int m_nSendFrameSizeDiv;					// Size divider (0 = full size, 1 = half size, 2 = 1/4 size, ...)
	volatile int m_nSendFrameFreqDiv;					// Frequency divider (1 = full freq., 2 = half freq., 3 = 1/3 freq., ...)
	CString m_sSendFrameMsg;							// Send Frame message shown in networking tab
	volatile int m_nSendFrameConnectionsCount;			// The current number of send frame active connections
	volatile DWORD m_dwLastSendUDPKeyFrameUpTime;		// The up-time of the last sent key-frame
	volatile WORD m_wLastSendUDPFrameSeq;				// The sequence of the last sent frame
	NETCOMRESENDFRAMELIST m_ReSendUDPFrameList;			// List of frame to be re-sent if asked for
	CRITICAL_SECTION m_csReSendUDPFrameList;			// Critical section of the re-send list
	volatile DWORD m_dwMaxSendFrameFragmentsPerFrame;	// Max sent fragments / frame
	volatile DWORD m_dwSendFrameTotalSentBytes;			// Both wrap around, no problem, using only the difference
	volatile DWORD m_dwSendFrameTotalLastSentBytes;		// of them to calculate the overall datarate
	volatile DWORD m_dwSendFrameOverallDatarate;		// bytes / sec
	volatile DWORD m_dwSendFrameDatarateCorrection;		// bytes / sec
	CString m_sSendFrameUsername;						// UDP Username
	CString m_sSendFramePassword;						// UDP Password

	// Snapshot Vars
	CString m_sSnapshotAutoSaveDir;						// The directory for the snapshots
	volatile BOOL m_bSnapshotLiveJpeg;					// Live snapshot save as Jpeg
	volatile BOOL m_bSnapshotHistoryJpeg;				// Snapshot history save Jpegs
	volatile BOOL m_bSnapshotHistorySwf;				// Snapshot history save Swf
	volatile BOOL m_bSnapshotLiveJpegFtp;				// Upload Jpeg Live snapshot files
	volatile BOOL m_bSnapshotHistoryJpegFtp;			// Upload Jpeg Snapshot history files
	volatile BOOL m_bSnapshotHistorySwfFtp;				// Upload Swf Snapshot history files
	volatile BOOL m_bSnapshotHistoryDeinterlace;		// Snapshot history deinterlace
	volatile int m_nSnapshotRate;						// Snapshot rate in seconds
	volatile int m_nSnapshotHistoryFrameRate;			// Snapshot history framerate
	volatile LONG m_bSnapshotHistoryCloseSwfFile;		// Close the Snapshot history swf file
	volatile int m_nSnapshotCompressionQuality;			// Snapshot compression quality
	volatile float m_fSnapshotVideoCompressorQuality;	// Snapshots swf compression quality
	volatile BOOL m_bSnapshotThumb;						// Snapshot thumbnail enable / disable flag
	volatile int m_nSnapshotThumbWidth;					// Snapshot thumbnail width
	volatile int m_nSnapshotThumbHeight;				// Snapshot thumbnail height
	volatile DWORD m_dwNextSnapshotUpTime;				// The up-time of the next snapshot
	volatile BOOL m_bSnapshotStartStop;					// Enable / Disable Daily Timed Snapshots
	CTime m_SnapshotStartTime;							// Daily Snapshots Start Time
	CTime m_SnapshotStopTime;							// Daily Snapshots Stop Time
	volatile int m_nDeleteSnapshotsOlderThanDays;		// Delete Snapshots older than the given amount of days,
														// 0 means never delete any file!

	// Movement Detector Vars
	BOOL m_bShowMovementDetections;						// Show / Hide Movement Detection Zones
	BOOL m_bShowEditDetectionZones;						// Show & Edit / Hide Movement Detection Zones
	BOOL m_bShowEditDetectionZonesMinus;				// Add / Remove Movement Detection Zone
	volatile BOOL m_bDetectingMovement;					// Flag Indicating a Detection
	volatile BOOL m_bFirstMovementDetection;			// Start Detecting when this is FALSE
	volatile int m_nDetectionLevel;						// Detection Level 1 .. 100 (100 Max Sensibility)
														// a high sensibility may Detect Video Noise!)
	__int64 m_nMilliSecondsSinceMovementDetection;
	__int64 m_nMilliSecondsWithoutMovementDetection;
	__int64 m_nMilliSecondsBeforeMovementDetection;
	volatile int m_nMilliSecondsRecBeforeMovementBegin;	// Do record in the circular buffer list this amount of millisec. before det.
	volatile int m_nMilliSecondsRecAfterMovementEnd;	// Keep Recording this amount of millisec. after det. end
	volatile BOOL m_bSaveSWFMovementDetection;			// Save Movement Detections as SWF
	volatile BOOL m_bSaveAVIMovementDetection;			// Save Movement Detections as AVI
	volatile BOOL m_bSaveAnimGIFMovementDetection;		// Save Movement Detections as Animated GIF
	volatile BOOL m_bSendMailMovementDetection;			// Send Email of Movement Detections
	volatile BOOL m_bFTPUploadMovementDetection;		// FTP Upload Movement Detections
	volatile BOOL m_bExecCommandMovementDetection;		// Execute Command on Movement Detection
	CString m_sExecCommandMovementDetection;			// Command to execute on Movement Detection
	CString m_sExecParamsMovementDetection;				// Params for command execution
	volatile BOOL m_bHideExecCommandMovementDetection;	// Hide command's window
	volatile BOOL m_bWaitExecCommandMovementDetection;	// Wait that last command has terminated
	HANDLE volatile m_hExecCommandMovementDetection;	// Exec command handle
	CRITICAL_SECTION m_csExecCommandMovementDetection;	// Command Exec critical section
	volatile BOOL m_bMovementDetectorPreview;			// Enable Preview
	CDib* volatile m_pMovementDetectorBackgndDib;		// Moving Background Dib
	CDib* volatile m_pMovementDetectorY800Dib;			// If source Dib is in RGB format that's the converted Y800 Dib
	DIBLISTLIST m_MovementDetectionsList;				// The List of Movement Detection Frame Grabbing Lists
	CRITICAL_SECTION m_csMovementDetectionsList;		// Critical Section of the Movement Detections List
	volatile DWORD m_dwAnimatedGifWidth;				// Width of Detection Animated Gif 
	volatile DWORD m_dwAnimatedGifHeight;				// Height of Detection Animated Gif
	CDib* volatile m_pDifferencingDib;					// Differencing Dib
	int* volatile m_MovementDetectorCurrentIntensity;	// Current Intensity by zones (array allocated in constructor)
	DWORD* volatile m_MovementDetectionsUpTime;			// Detection Up-Time For each Zone (array allocated in constructor)
	BOOL* volatile m_MovementDetections;				// Detecting in Zone (array allocated in constructor)
	BOOL* volatile m_DoMovementDetection;				// Do Movement Detection in this Zone (array allocated in constructor)
	volatile int m_nMovementDetectorIntensityLimit;		// Noise Floor
	volatile LONG m_lMovDetXZonesCount;					// Number of zones in X direction (never set to 0 to avoid division by 0)
	volatile LONG m_lMovDetYZonesCount;					// Number of zones in Y direction (never set to 0 to avoid division by 0)
	volatile LONG m_lMovDetTotalZones;					// Total Number of zones (set to 0 when a (re-)init of the zones is wanted)
	volatile BOOL m_bDoAdjacentZonesDetection;			// Fire a detection only if moving between two adjacent zones
	volatile BOOL m_bDoFalseDetectionCheck;				// Do a false detection check if set
	volatile BOOL m_bDoFalseDetectionAnd;				// And / Or between Blue and None Blue zones
	volatile int m_nFalseDetectionBlueThreshold;		// False detections blue zones threshold value
	volatile int m_nFalseDetectionNoneBlueThreshold;	// False detections none blue zones threshold value
	volatile int m_nBlueMovementDetectionsCount;		// Count the simultaneously detected blue zones
	volatile int m_nNoneBlueMovementDetectionsCount;	// Count the simultaneously detected none blue zones
	volatile DWORD m_dwVideoDetFourCC;					// Video Compressor FourCC
	volatile BOOL m_bVideoDetDeinterlace;				// Deinterlace video when saving
	volatile int m_nVideoDetDataRate;					// Data Rate in Bits / Sec
	volatile int m_nVideoDetKeyframesRate;				// Keyframes Rate
	volatile float m_fVideoDetQuality;					// 2.0f best quality, 31.0f worst quality
	volatile int m_nVideoDetQualityBitrate;				// 0 -> use quality, 1 -> use bitrate
	volatile DWORD m_dwVideoDetSwfFourCC;				// Video Compressor FourCC
	volatile BOOL m_bVideoDetSwfDeinterlace;			// Deinterlace video when saving
	volatile int m_nVideoDetSwfDataRate;				// Data Rate in Bits / Sec
	volatile int m_nVideoDetSwfKeyframesRate;			// Keyframes Rate
	volatile float m_fVideoDetSwfQuality;				// 2.0f best quality, 31.0f worst quality
	volatile int m_nVideoDetSwfQualityBitrate;			// 0 -> use quality, 1 -> use bitrate
	volatile BOOL m_bDetectionStartStop;				// Enable / Disable Daily Timed Detection
	CTime m_DetectionStartTime;							// Daily Detection Start Time
	CTime m_DetectionStopTime;							// Daily Detection Stop Time
	volatile int m_nDeleteDetectionsOlderThanDays;		// Delete Detections older than the given amount of days,
														// 0 means never delete any file!
	BOOL m_bUnsupportedVideoSizeForMovDet;				// Flag indicating an unsupported resolution
	
	// Color Detector Vars
	CColorDetection m_ColorDetection;					// Color Detection Object
	int m_nDoColorPickup;								// Color Pickup Var
	volatile BOOL m_bColorDetectionPreview;				// Show Detection Preview in View Window
	volatile DWORD m_dwColorDetectionAccuracy;			// Accuracy Var
	volatile DWORD m_dwColorDetectionWaitTime;			// Wait time between detection count-ups (like racing laps)	

	// Property Sheet Pointer
	CVideoDevicePropertySheet* volatile m_pVideoDevicePropertySheet;
	CAssistantPage* volatile m_pAssistantPage;
	CSnapshotPage* volatile m_pSnapshotPage;
	CNetworkPage* volatile m_pNetworkPage;
	CGeneralPage* volatile m_pGeneralPage;
	CColorDetectionPage* volatile m_pColorDetectionPage;
	CMovementDetectionPage* volatile m_pMovementDetectionPage;

	// Email sending
	SendMailConfigurationStruct m_MovDetSendMailConfiguration;

	// FTP Upload
	FTPUploadConfigurationStruct m_MovDetFTPUploadConfiguration;
	FTPUploadConfigurationStruct m_SnapshotFTPUploadConfiguration;

	// Return Values
	// -1 : Do Exit Thread (specified for CFTPTransfer constructor)
	// 0  : Error
	// 1  : Ok
	static int FTPUpload(	CFTPTransfer* pFTP, FTPUploadConfigurationStruct* pConfig,
							CString sLocalFileName, CString sRemoteFileName);

// Protected Variables
protected:
	BOOL m_bResetSettings;
	CVideoDeviceView* m_pView;
	CVideoDeviceChildFrame* m_pFrame;
	volatile LONG m_bStopProcessFrame;
	volatile LONG m_bProcessFrameStopped;
	LPBYTE m_pHCWBuf; // HCW conversion buffer
	CTryEnterCriticalSection m_csProcessFrame;

	// For Frame Rate and Data Rate Calculation
	volatile double m_dEffectiveFrameTimeSum;
	volatile DWORD m_dwEffectiveFrameTimeCountUp;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVideoDeviceDoc)
	protected:
	virtual BOOL SaveModified();
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
	afx_msg void OnCaptureRecordPause();
	afx_msg void OnUpdateCaptureRecordPause(CCmdUI* pCmdUI);
	afx_msg void OnEditCopy();
	afx_msg void OnCaptureDeinterlace();
	afx_msg void OnUpdateCaptureDeinterlace(CCmdUI* pCmdUI);
	afx_msg void OnFileSave();
	afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
	afx_msg void OnViewFit();
	afx_msg void OnUpdateViewFit(CCmdUI* pCmdUI);
	afx_msg void OnViewWeb();
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnFileSaveAs();
	afx_msg void OnUpdateFileSaveAs(CCmdUI* pCmdUI);
	afx_msg void OnCaptureReset();
	afx_msg void OnEditSnapshot();
	afx_msg void OnUpdateEditSnapshot(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

__forceinline void CVideoDeviceDoc::ClearFrameList(CDib::LIST* pFrameList)
{
	if (pFrameList)
		CDib::FreeList(*pFrameList);
}

__forceinline void CVideoDeviceDoc::OneFrameList()
{
	::EnterCriticalSection(&m_csMovementDetectionsList);
	while (!m_MovementDetectionsList.IsEmpty())
	{
		CDib::LIST* pFrameList = m_MovementDetectionsList.GetTail();
		ClearFrameList(pFrameList);
		delete pFrameList;
		m_MovementDetectionsList.RemoveTail();
	}
	CDib::LIST* pNewList = new CDib::LIST;
	if (pNewList)
		m_MovementDetectionsList.AddTail(pNewList);
	::LeaveCriticalSection(&m_csMovementDetectionsList);
}

#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIDEODEVICEDOC_H__24956A13_6E82_4959_931F_F512BE9B17A0__INCLUDED_)
