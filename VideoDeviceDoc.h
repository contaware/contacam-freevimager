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
#if (_MSC_VER <= 1200)
#include "pjnsmtp_vc6.h"
#else
#include "pjnsmtp.h"
#endif
#include "NetCom.h"
#include "AVRec.h"
#include "AVDecoder.h"
#include "MJPEGEncoder.h"
#include "YuvToRgb.h"
#include "NetFrameHdr.h"
#include "SortableFileFind.h"
#include "FTPTransfer.h"
#include "HostPortDlg.h"
extern "C"
{
#include "ffmpeg\\libavcodec\\avcodec.h"
#include "ffmpeg\\libavformat\\avformat.h"
#include "ffmpeg\\libswscale\\swscale.h"
}

// Forward Declarations
class CVideoDeviceView;
class CVideoDeviceChildFrame;
class CDxCapture;
class CVideoAviDoc;
class CGeneralPage;
class CSnapshotPage;
class CVideoDevicePropertySheet;
class CNetworkPage;
class CMovementDetectionPage;

// General Settings
#define MIN_FRAMERATE						0.1			// fps
#define MAX_FRAMERATE						95.0		// fps
#define PROCESS_MAX_FRAMETIME				13000U		// ms, make sure that: 1000 / MIN_FRAMERATE < PROCESS_MAX_FRAMETIME
#define STARTUP_SETTLE_TIME_SEC				3			// sec
#define DEFAULT_FRAMERATE					10.0		// fps
#define HTTPSERVERPUSH_DEFAULT_FRAMERATE	4.0			// fps
#define HTTPSERVERPUSH_EDIMAX_DEFAULT_FRAMERATE	3.0		// fps
#define HTTPCLIENTPOLL_DEFAULT_FRAMERATE	1.0			// fps
#define MAX_DEVICE_AUTORUN_KEYS				100			// Maximum number of devices that can autorun at start-up
#define ACTIVE_VIDEO_STREAM					0			// Video stream 0 for recording and detection
#define ACTIVE_AUDIO_STREAM					0			// Audio stream 0 for recording and detection
#define MIN_DISKFREE_PERCENT				10			// Below this disk free percentage the oldest files are deleted
#define	FILES_DELETE_INTERVAL_MIN			600000	 	// in ms -> 10min
#define	FILES_DELETE_INTERVAL_RANGE			300000		// in ms -> each [10min,15min[ check whether we can delete old files
#define AUDIO_IN_MIN_BUF_SIZE				256			// bytes
#define AUDIO_MAX_LIST_SIZE					1024		// make sure that: 1 / MIN_FRAMERATE < AUDIO_IN_MIN_BUF_SIZE * AUDIO_MAX_LIST_SIZE / 11025
														// (see CCaptureAudioThread::OpenInAudio())
#define AUDIO_UNCOMPRESSED_BUFS_COUNT		16			// Number of audio buffers
 
// Frame time, date and count display constants
#define ADDFRAMETAG_REFFONTSIZE				9
#define ADDFRAMETAG_REFWIDTH				640
#define ADDFRAMETAG_REFHEIGHT				480
#define FRAMETIME_COLOR						RGB(0,0xFF,0)
#define FRAMEDATE_COLOR						RGB(0x80,0x80,0xFF)
#define FRAMECOUNT_COLOR					RGB(0xFF,0xFF,0xFF)

// Process Frame Stop Engine
#define PROCESSFRAME_MAX_RETRY_TIME			3500		// ms
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
#define WATCHDOG_THRESHOLD					15000U		// ms, make sure that: 1000 / MIN_FRAMERATE < WATCHDOG_THRESHOLD
#define HTTPWATCHDOG_RETRY_TIMEOUT			35000U		// ms
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
#define DEFAULT_SNAPSHOT_LIVE_FILE			_T("snapshot.jpg")
#define DEFAULT_SNAPSHOT_COMPR_QUALITY		60			// 0 Worst Quality, 100 Best Quality 
#define DEFAULT_SNAPSHOT_THUMB_WIDTH		228			// Must be a multiple of 4 because of swf
#define DEFAULT_SNAPSHOT_THUMB_HEIGHT		172			// Must be a multiple of 4 because of swf

// Movement Detection
#define NO_DETECTOR							0x00
#define TRIGGER_FILE_DETECTOR				0x01
#define SOFTWARE_MOVEMENT_DETECTOR			0x02
#define DEFAULT_PRE_BUFFER_MSEC				2000		// ms
#define DEFAULT_POST_BUFFER_MSEC			8000		// ms
#define MOVDET_BUFFER_COMPRESSIONQUALITY	4			// 2: best quality, 31: worst quality
#define DEFAULT_MOVDET_LEVEL				50			// Detection level default value (1 .. 100 = Max sensibility)
#define DEFAULT_MOVDET_INTENSITY_LIMIT		25			// Intensity difference default value
#define MOVDET_MAX_ZONES					8192		// Maximum number of zones
#define MOVDET_MIN_ZONES_XORY				4			// Minimum number of zones in X or Y direction
#define MOVDET_ZONE_FORMAT					_T("DoMovementDetection%03i")
#define MOVDET_SAVEFRAMES_POLL				1000U		// ms
#define MOVDET_MIN_FRAMES_IN_LIST			30			// Min. frames in list before saving the list in the
														// case of insufficient memory
#define MOVDET_MAX_FRAMES_IN_LIST			15000		// 16000 is the limit for swf files -> be safe and start
														// a new list with 15000
#define MOVDET_SAVE_MIN_FRAMERATE_RATIO		0.3			// Min ratio between calculated (last - first) and m_dEffectiveFrameRate
#define MOVDET_TIMEOUT						1000U		// Timeout in ms for detection zones
#define MOVDET_MEM_LOAD_THRESHOLD			25.0		// Above this load the detected frames are saved and freed
#define MOVDET_MEM_LOAD_CRITICAL			60.0		// Above this load the detected frames are dropped
#define MOVDET_MEM_MAX_MB					1536		// Maximum allocable memory in MB for 32 bits applications
														// (not 2048 because of fragmentation, stack and heap)
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
#define PHPCONFIG_SUMMARYSNAPSHOT_NAME		_T("summarysnapshot.php")
#define PHPCONFIG_SNAPSHOT_NAME				_T("snapshot.php")
#define PHPCONFIG_SNAPSHOTFULL_NAME			_T("snapshotfull.php")
#define PHPCONFIG_SNAPSHOTHISTORY_NAME		_T("snapshothistory.php")
#define PHPCONFIG_SUMMARYIFRAME_NAME		_T("summaryiframe.php")
#define PHPCONFIG_SUMMARYTITLE				_T("SUMMARYTITLE")
#define PHPCONFIG_SNAPSHOTTITLE				_T("SNAPSHOTTITLE")
#define PHPCONFIG_SNAPSHOTHISTORY_THUMB		_T("SNAPSHOTHISTORY_THUMB")
#define PHPCONFIG_SNAPSHOTREFRESHSEC		_T("SNAPSHOTREFRESHSEC")
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

// UDP Networking
/*
IPv4 and IPv6 define minimum reassembly buffer size, the minimum 
datagram size that we are guaranteed any implementation must support. 
For IPv4, this is 576 bytes. IPv6 raises this to 1500 bytes.

This pretty much means that you want to limit your datagram size to 
under 576 if you work over public internet. 

It is true that a typical IPv4 header is 20 bytes, and the UDP header is 
8 bytes. However it is possible to include IP options which can increase 
the size of the IP header to as much as 60 bytes. In addition, sometimes 
it is necessary for intermediate nodes to encapsulate datagrams inside 
of another protocol such as IPsec (used for VPNs and the like) in order 
to route the packet to its destination. So if you do not know the MTU on 
your particular network path, it is best to leave a reasonable margin 
for other header information that you may not have anticipated. A 
512-byte UDP payload is generally considered to do that, although even 
that does not leave quite enough space for a maximum size IP header.
*/
#define DEFAULT_SENDFRAME_FRAGMENT_SIZE		512			// bytes
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
#define HTTP_MAX_MULTIPART_SIZE				8388608		// 8 MB		
#define DEFAULT_HTTP_VIDEO_QUALITY			30			// 0 Best Quality, 100 Worst Quality
#define DEFAULT_HTTP_VIDEO_SIZE_CX			640
#define DEFAULT_HTTP_VIDEO_SIZE_CY			480
#define HTTPGETFRAME_CONNECTION_STARTDELAY	1000U		// ms
#define HTTPGETFRAME_MAXCOUNT_ALARM1		30
#define HTTPGETFRAME_MAXCOUNT_ALARM2		40
#define HTTPGETFRAME_MAXCOUNT_ALARM3		50
#define HTTPGETFRAME_DELAY_DEFAULT			500U								// ms
#define HTTPGETFRAME_MIN_DELAY_ALARM1		100U								// ms
#define HTTPGETFRAME_MIN_DELAY_ALARM2		400U								// ms
#define HTTPGETFRAME_MIN_DELAY_ALARM3		1000U								// ms
#define HTTPGETFRAME_MAX_DELAY_ALARM		((DWORD)(1000.0 / MIN_FRAMERATE))	// ms
#define HTTPGETFRAME_CONNECTION_TIMEOUT		20			// Connection timeout in sec

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
			virtual BOOL Parse(CNetCom* pNetCom, BOOL bLastCall);
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
					CSendFrameToEntry() {	memset(&m_Addr, 0, sizeof(sockaddr_in6));
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
					__forceinline void SetAddr(sockaddr* pAddr) {memcpy(&m_Addr, pAddr, SOCKADDRSIZE(pAddr)); m_bSet = TRUE;};
					__forceinline sockaddr* GetAddrPtr() {return (sockaddr*)(&m_Addr);};
					__forceinline BOOL IsAddrEqualTo(sockaddr* pAddr) {return IsAddrSet() ? (memcmp(pAddr, &m_Addr, SOCKADDRSIZE(pAddr)) == 0) : FALSE;};

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
					sockaddr_in6 m_Addr;						// Address to which to send frames, maybe IP4 or IP6
					volatile BOOL m_bSet;						// Is Table Entry Set (Valid)
			};

		public:
			CSendFrameParseProcess(CVideoDeviceDoc* pDoc) {	::InitializeCriticalSection(&m_csSendToTable); 
															m_pDoc = pDoc; Clear();};
			virtual ~CSendFrameParseProcess() {FreeAVCodec(); ::DeleteCriticalSection(&m_csSendToTable);};
			void Close() {FreeAVCodec(); Clear();};
			virtual BOOL Parse(CNetCom* pNetCom, BOOL bLastCall);
			void ClearTable();
			BOOL OpenAVCodec(LPBITMAPINFO pBMI);
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
														// CODEC_ID_H263 is only working with standard resolution of 176 x 144 or 352 x 288						
														// CODEC_ID_SNOW is locking with low resolutions...
							m_pFrame = NULL;
							m_pFrameI420 = NULL;
							m_pImgConvertCtx = NULL;
							m_pI420Buf = NULL;						
							m_dwI420BufSize = 0;
							m_dwI420ImageSize = 0;
							memset(&m_CurrentBMI, 0, sizeof(BITMAPINFOFULL));
							m_nCurrentDataRate = 0;
							m_nCurrentSizeDiv = 0;
							m_nCurrentFreqDiv = 1;
							m_dCurrentSendFrameRate = 0.0;
							m_pOutbuf = NULL;
							m_nOutbufSize = 0;
							m_dwEncryptionType = 0U;};

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
			BITMAPINFOFULL m_CurrentBMI;
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
			int Work();
			BOOL OpenInAudio();
			void CloseInAudio();
			BOOL DataInAudio();

			// Vars
			HWAVEIN m_hWaveIn;
			CVideoDeviceDoc* m_pDoc;
			WAVEINCAPS m_WaveInDevCaps;
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
			CDeleteThread(){m_pDoc = NULL; m_dwCounter = 0;};
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
#if (_MSC_VER > 1200)
		CPJNSMTPConnection::ConnectionType m_ConnectionType;
#endif
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
		DWORD			m_dwConnectionTimeout;
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
			CSaveFrameListThread(){m_pDoc = NULL; m_pFrameList = NULL; m_nNumFramesToSave = 0;
						m_nSendMailProgress = 100; m_nFTPUploadProgress = 100; m_bWorking = FALSE;};
			virtual ~CSaveFrameListThread() {Kill();};
			void SetDoc(CVideoDeviceDoc* pDoc) {m_pDoc = pDoc;};
			__forceinline void SetSendMailProgress(int nSendMailProgress) {m_nSendMailProgress = nSendMailProgress;};
			__forceinline int GetSendMailProgress() const {return m_nSendMailProgress;};
			__forceinline void SetFTPUploadProgress(int nFTPUploadProgress) {m_nFTPUploadProgress = nFTPUploadProgress;};
			__forceinline int GetFTPUploadProgress() const {return m_nFTPUploadProgress;};
			__forceinline BOOL IsWorking() const {return m_bWorking;};

		protected:
			// CalcMovementDetectionListsSize() must be called from this thread because
			// this thread changes/deletes the list's dibs not inside the lists critical section!
			void CalcMovementDetectionListsSize();
			BOOL DecodeFrame(CDib* pDib);
			int Work();
			CString SaveJpeg(	CDib* pDib,
								CString sJPGDir,
								BOOL bShowFrameTime,
								const CTime& RefTime,
								DWORD dwRefUpTime,
								int nMovDetSavesCount);
			BOOL SaveSingleGif(		CDib* pDib,
									const CString& sGIFFileName,
									RGBQUAD* pGIFColors,
									BOOL bShowFrameTime,
									const CTime& RefTime,
									DWORD dwRefUpTime,
									int nMovDetSavesCount);
			void AnimatedGifInit(	RGBQUAD* pGIFColors,
									double& dDelayMul,
									double& dSpeedMul,
									double dCalcFrameRate,
									BOOL bShowFrameTime,
									const CTime& RefTime,
									DWORD dwRefUpTime,
									int nMovDetSavesCount);
			__forceinline void To255Colors(	CDib* pDib,
											RGBQUAD* pGIFColors,
											BOOL bShowFrameTime,
											const CTime& RefTime,
											DWORD dwRefUpTime,
											int nMovDetSavesCount);
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
									BOOL bShowFrameTime,
									const CTime& RefTime,
									DWORD dwRefUpTime,
									int nMovDetSavesCount);
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
			CAVDecoder m_AVDecoder;
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
			CSaveSnapshotSWFThread(){m_ThreadExecutedForTime = CTime(0);};
			virtual ~CSaveSnapshotSWFThread() {Kill();};

			BOOL m_bSnapshotHistoryJpeg;
			BOOL m_bSnapshotHistorySwfFtp;
			float m_fSnapshotVideoCompressorQuality;
			double m_dSnapshotHistoryFrameRate;
			CTime m_Time;
			CTime m_ThreadExecutedForTime;
			CString m_sSnapshotAutoSaveDir;
			FTPUploadConfigurationStruct m_Config;

		protected:
			int Work();
			__forceinline CString MakeSwfHistoryFileName();
	};

	// The Save Snapshot Thread Class
	class CSaveSnapshotThread : public CWorkerThread
	{
		public:
			CSaveSnapshotThread(){;};
			virtual ~CSaveSnapshotThread() {Kill();};

			CDib m_Dib;
			BOOL m_bSnapshotHistoryJpeg;
			BOOL m_bSnapshotHistoryJpegFtp;
			BOOL m_bShowFrameTime;
			BOOL m_bSnapshotThumb;
			BOOL m_bSnapshotLiveJpeg;
			BOOL m_bSnapshotLiveJpegFtp;
			int m_nSnapshotThumbWidth;
			int m_nSnapshotThumbHeight;
			int m_nSnapshotCompressionQuality;
			CTime m_Time;
			CString m_sSnapshotAutoSaveDir;
			FTPUploadConfigurationStruct m_Config;

		protected:
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
	void SetDocumentTitle();
	CVideoDeviceView* GetView() const {return m_pView;};
	void SetView(CVideoDeviceView* pView) {m_pView = pView;};
	CVideoDeviceChildFrame* GetFrame() const {return m_pFrame;};
	void SetFrame(CVideoDeviceChildFrame* pFrame) {m_pFrame = pFrame;};
	static BOOL CreateCheckYearMonthDayDir(CTime Time, CString sBaseDir, CString& sYearMonthDayDir);

	// Open Video Device
	BOOL OpenVideoDevice(int nId);

	// Open Video From Avi Player
	BOOL OpenVideoAvi(CVideoAviDoc* pDoc, CDib* pDib);

	// Open Video From Network
	BOOL OpenGetVideo(CHostPortDlg* pDlg);

	// Open Video From Network
	BOOL OpenGetVideo(CString sAddress);	

	// Dialogs
	void CaptureAssistant();
	void VideoFormatDialog();
	void AudioFormatDialog();

	// Frame Tags
	static CTime CalcTime(DWORD dwUpTime, const CTime& RefTime, DWORD dwRefUpTime);
	static void AddFrameTime(CDib* pDib, CTime RefTime, DWORD dwRefUpTime);
	static void AddFrameCount(CDib* pDib, int nCount);
	
	// On Change Frame Rate
	void OnChangeFrameRate();

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
	__forceinline void SaveFrameList();								// Add new empty list to tail
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
	__forceinline void ShrinkNewestFrameListBy(	int nSize,			// Free oldest nSize frames from newest frame list
												DWORD& dwFirstUpTime,
												DWORD& dwLastUpTime);
	__forceinline int  GetNewestMovementDetectionsListCount();		// Get the newest list's count
	__forceinline void AddNewFrameToNewestList(CDib* pDib);			// Add new frame to newest list
	__forceinline void AddNewFrameToNewestListAndShrink(CDib* pDib);// Add new frame to newest list leaving in the list
																	// m_nMilliSecondsRecBeforeMovementBegin of frames

	// Main Decode & Process Functions
	void ProcessNoI420NoM420Frame(LPBYTE pData, DWORD dwSize);
	void ProcessM420Frame(LPBYTE pData, DWORD dwSize);
	void ProcessI420Frame(LPBYTE pData, DWORD dwSize);

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

	// Movement Detection
	void MovementDetectionProcessing(	CDib* pDib,
										DWORD dwVideoProcessorMode,
										BOOL b1SecTick);
	BOOL LumChangeDetector(CDib* pDib);
	BOOL MovementDetector(CDib* pDib, int nDetectionLevel);
	void ResetMovementDetector();
	void FreeMovementDetector();
	void ExecCommandMovementDetection(	BOOL bReplaceVars = FALSE,
										CTime StartTime = CTime(0),
										const CString& sAVIFileName = _T(""),
										const CString& sGIFFileName = _T(""),
										const CString& sSWFFileName = _T(""),
										int nMovDetSavesCount = 0);

	// Email Message Creation
	// The returned CPJNSMTPMessage* is allocated on the heap -> has to be deleted when done!
	static CPJNSMTPMessage* CreateEmailMessage(SendMailConfigurationStruct* pSendMailConfiguration);

	// Function called when the directx video grabbing format has been changed
	void OnChangeDxVideoFormat();

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
		EDIMAX_CP,			// Edimax Client Poll (jpegs)
		TPLINK_SP,			// TP-Link Server Push (mjpeg)
		TPLINK_CP,			// TP-Link Client Poll (jpegs)
		// Add more devices here...	
		LAST_DEVICE			// Placeholder for range check
	} NetworkDeviceTypeMode;
	BOOL ConnectGetFrame();
	
	// UDP Networking Functions
	BOOL ConnectSendFrameUDP(CNetCom* pNetCom, int nPort);
	BOOL ConnectGetFrameUDP(LPCTSTR pszHostName, int nPort);
	BOOL StoreUDPFrame(BYTE* Data, int Size, DWORD dwFrameUpTime, WORD wFrameSeq, BOOL bKeyFrame);
	BOOL ReSendUDPFrame(sockaddr* pTo, WORD wFrameSeq);
	void ClearReSendUDPFrameList();
	BOOL SendUDPFrame(	CNetCom* pNetCom,
						sockaddr* pTo, // if NULL send to all!
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
	void ExportDetectionZones(const CString& sFileName);
	void ImportDetectionZones(const CString& sFileName);

	// Autorun
	static CString AutorunGetDeviceKey(const CString& sDevicePathName);
	static void AutorunAddDevice(const CString& sDevicePathName);
	static void AutorunRemoveDevice(const CString& sDevicePathName);

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
	static BOOL MicroApacheShutdown();
	static int MicroApacheReload(); // Return Values: 1=OK, 0=Failed to stop the web server, -1=Failed to start the web server
	static CString MicroApacheConfigFileGetParam(const CString& sParam);						// sParam is case sensitive!
	static BOOL MicroApacheConfigFileSetParam(const CString& sParam, const CString& sValue);	// sParam is case sensitive!
	
	// Php
	CString PhpGetConfigFileName();
	BOOL PhpConfigFileSetParam(const CString& sParam, const CString& sValue);					// sParam is case sensitive!
	CString PhpConfigFileGetParam(const CString& sParam);										// sParam is case sensitive!

// Protected Functions
protected:
	BOOL InitOpenDxCapture(int nId);
	void Snapshot(CDib* pDib, const CTime& Time);
	BOOL EditCopy(CDib* pDib, const CTime& Time);
	BOOL EditSnapshot(CDib* pDib, const CTime& Time);
	BOOL Rotate180(CDib* pDib);
	BOOL Deinterlace(CDib* pDib);
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
	__forceinline int GetAppMemoryUsageMB();
	__forceinline double GetAppMemoryLoad();

	// Networking Functions
	static double GetDefaultNetworkFrameRate(NetworkDeviceTypeMode nNetworkDeviceTypeMode);
	__forceinline BOOL SendUDPFragment(	CNetCom* pNetCom,
										sockaddr* pTo, // if NULL send to all!
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
	void UpdateFrameSendToTableAndFlowControl(BOOL b4SecTick);

	// Micro Apache Functions
	static CString LoadMicroApacheConfigFile();
	static BOOL SaveMicroApacheConfigFile(const CString& sConfig);
	void MicroApacheViewOnWeb(CString sAutoSaveDir, const CString& sWebPageFileName);

	// Php
	CString LoadPhpConfigFile();
	BOOL SavePhpConfigFile(const CString& sConfig);

	// Manual Snapshot
	CString MakeJpegManualSnapshotFileName(const CTime& Time);

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
	volatile BOOL m_bCaptureStarted;					// Flag set when first frame has been processed
	CTime m_CaptureStartTime;							// Grabbing device started at this time
	volatile BOOL m_bVideoView;							// Flag indicating whether the frame grabbing is to be previewed
	volatile BOOL m_bShowFrameTime;						// Show / Hide Frame Time Inside the Frame (frame time is also recorded)
	volatile BOOL m_bDoEditCopy;						// Copy Frame to Clipboard in ProcessI420Frame()
	volatile BOOL m_bDoEditSnapshot;					// Manual Snapshot Frame to file
	volatile DWORD m_dwFrameCountUp;					// Captured Frames Count-Up, it can wrap around!
	volatile DWORD m_dwVideoProcessorMode;				// The Processor Mode Variable
	CVideoAviDoc* volatile m_pVideoAviDoc;				// Video source from a Avi Player Doc
	volatile BOOL m_bSizeToDoc;							// If no placement settings in registry size client window to frame size
	volatile BOOL m_bDeviceFirstRun;					// First Time that this device runs
	CTime m_1SecTime;									// For the 1 sec tick in ProcessI420Frame()
	CTime m_4SecTime;									// For the 4 sec tick in ProcessI420Frame()

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
	DWORD m_dwCaptureAudioDeviceID;						// Audio Capture Device ID
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
	volatile int m_nSendFrameMaxConnections;			// Current Send Frame max number of supported connections
	volatile int m_nSendFrameMaxConnectionsConfig;		// Send Frame max number of supported connections configuration
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
	volatile double m_dSendFrameDatarateCorrection;		// Correction factor
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
	volatile BOOL m_bManualSnapshotAutoOpen;			// Auto open after executing the manual snapshot command
	volatile int m_nSnapshotRate;						// Snapshot rate in seconds
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
	volatile int m_nDeleteSnapshotsOlderThanDays;		// Delete Snapshots older than the given amount of days,
														// 0 means never delete any file!

	// Movement Detector Vars
	CString m_sDetectionAutoSaveDir;					// The Detection Directory
	CString m_sDetectionTriggerFileName;				// The external detection trigger file name
	FILETIME m_DetectionTriggerLastWriteTime;			// Last known write time of detection trigger file
	BOOL m_bShowMovementDetections;						// Show / Hide Movement Detection Zones
	BOOL m_bShowEditDetectionZones;						// Show & Edit / Hide Movement Detection Zones
	BOOL m_bShowEditDetectionZonesMinus;				// Add / Remove Movement Detection Zone
	volatile BOOL m_bDetectingMovement;					// Flag Indicating a Detection
	volatile BOOL m_bFirstMovementDetection;			// Start Detecting when this is FALSE
	volatile int m_nDetectionLevel;						// Detection Level 1 .. 100 (100 Max Sensibility)
														// a high sensibility may Detect Video Noise!)
	volatile int m_nDetectionZoneSize;					// Configured detection zone size: 0->Big, 1->Medium, 2->Small
	volatile int m_nCurrentDetectionZoneSize;			// Current detection zone size: 0->Big, 1->Medium, 2->Small
	volatile DWORD m_dwWithoutMovementDetection;		// Uptime of last movement detection
	volatile int m_nMilliSecondsRecBeforeMovementBegin;	// Do record in the circular buffer list this amount of millisec. before det.
	volatile int m_nMilliSecondsRecAfterMovementEnd;	// Keep Recording this amount of millisec. after det. end
	volatile BOOL m_bDetectionCompressFrames;			// Compress detection frames?
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
	int* volatile m_LumChangeDetectorBkgY;				// Luminosity change background by zones (array allocated in constructor)
	int* volatile m_LumChangeDetectorDiffY;				// Luminosity change difference by zones (array allocated in constructor)
	int* volatile m_MovementDetectorCurrentIntensity;	// Current Intensity by zones (array allocated in constructor)
	DWORD* volatile m_MovementDetectionsUpTime;			// Detection Up-Time For each Zone (array allocated in constructor)
	BOOL* volatile m_MovementDetections;				// Detecting in Zone (array allocated in constructor)
	int* volatile m_DoMovementDetection;				// Do Movement Detection in this Zone with given relative sensibility
														// 0 -> No Detection, 1 -> Full Sensibility=100%, 2 -> 50%, 4 -> 25%, 10 -> 10%
														// (array allocated in constructor)
	volatile int m_nMovementDetectorIntensityLimit;		// Noise Floor
	volatile LONG m_lMovDetXZonesCount;					// Number of zones in X direction (never set to 0 to avoid division by 0)
	volatile LONG m_lMovDetYZonesCount;					// Number of zones in Y direction (never set to 0 to avoid division by 0)
	volatile LONG m_lMovDetTotalZones;					// Total Number of zones (set to 0 when a (re-)init of the zones is wanted)
	volatile BOOL m_bDoAdjacentZonesDetection;			// Fire a detection only if moving between two adjacent zones
	volatile BOOL m_bDoLumChangeDetection;				// Discard movement detection if a luminosity change happens
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
	volatile BOOL m_bDetectionStartStop;				// Enable / Disable Daily Timed Detection
	volatile BOOL m_bDetectionSunday;					// Do detections on Sunday
	volatile BOOL m_bDetectionMonday;					// Do detections on Monday
	volatile BOOL m_bDetectionTuesday;					// Do detections on Tuesday
	volatile BOOL m_bDetectionWednesday;				// Do detections on Wednesday
	volatile BOOL m_bDetectionThursday;					// Do detections on Thursday
	volatile BOOL m_bDetectionFriday;					// Do detections on Friday
	volatile BOOL m_bDetectionSaturday;					// Do detections on Saturday
	CTime m_DetectionStartTime;							// Daily Detection Start Time
	CTime m_DetectionStopTime;							// Daily Detection Stop Time
	volatile int m_nDeleteDetectionsOlderThanDays;		// Delete Detections older than the given amount of days,
														// 0 means never delete any file!
	BOOL m_bUnsupportedVideoSizeForMovDet;				// Flag indicating an unsupported resolution
	volatile int m_nMovDetFreqDiv;						// Current frequency divider
	volatile double m_dMovDetFrameRateFreqDivCalc;		// Framerate used to calculate the current frequency divider
	volatile int m_nMovDetSavesCount;					// Count of the saved movement detection movies during current day
	volatile int m_nMovDetSavesCountDay;				// Day of the above count
	volatile int m_nMovDetSavesCountMonth;				// Month of the above count
	volatile int m_nMovDetSavesCountYear;				// Year of the above count

	// Property Sheet Pointer
	CVideoDevicePropertySheet* volatile m_pVideoDevicePropertySheet;
	CSnapshotPage* volatile m_pSnapshotPage;
	CNetworkPage* volatile m_pNetworkPage;
	CGeneralPage* volatile m_pGeneralPage;
	CMovementDetectionPage* volatile m_pMovementDetectionPage;

	// Email sending
	SendMailConfigurationStruct m_MovDetSendMailConfiguration;

	// FTP Upload
	FTPUploadConfigurationStruct m_MovDetFTPUploadConfiguration;
	FTPUploadConfigurationStruct m_SnapshotFTPUploadConfiguration;
	CRITICAL_SECTION m_csSnapshotFTPUploadConfiguration;

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
	CDib* volatile m_pProcessFrameDib;
	CDib* volatile m_pProcessFrameExtraDib;
	CAVDecoder m_AVDecoder;
	CMJPEGEncoder m_MJPEGEncoder;
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
	afx_msg void OnEditCopy();
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
