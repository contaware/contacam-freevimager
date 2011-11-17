#ifndef _INC_AVREC
#define _INC_AVREC

#include "Dib.h"
#include <mmsystem.h>
#include "YuvToRgb.h"
#include "DxDraw.h"
extern "C"
{
#include "ffmpeg\\libavcodec\\avcodec.h"
#include "ffmpeg\\libavformat\\avformat.h"
#include "ffmpeg\\libswscale\\swscale.h"
}

// Used to Convert From Double to Fractional Frame-Rate.
// Do not use a higher value, CODEC_ID_MPEG4 is not working!
#define MAX_SIZE_FOR_RATIONAL				65535

// Also defined in AviPlay.h!
#define AUDIO_PCM_MIN_BUF_SIZE				8192

// Frame date / time display constants
#define ADDFRAMETIME_REFFONTSIZE			9
#define ADDFRAMETIME_REFWIDTH				640
#define ADDFRAMETIME_REFHEIGHT				480
#define FRAMETIME_COLOR						RGB(0,0xff,0)
#define FRAMEDATE_COLOR						RGB(0x80,0x80,0xff)

class CAVRec
{
public:
	///////////////////////
	// General Functions //
	///////////////////////

	// Simple Constructors
	CAVRec();

	// Construction which calls Init()
	CAVRec(	LPCTSTR lpszFileName,
			int nPassNumber = 0,
			LPCTSTR lpszTempDir = _T(""),
			bool bFastEncode = false);

	// Destructor
	virtual ~CAVRec();

	// Init
	// nPassNumber 0: Single Pass, 1: First Pass, 2: Second Pass
	bool Init(	LPCTSTR lpszFileName,
				int nPassNumber = 0,
				LPCTSTR lpszTempDir = _T(""),
				bool bFastEncode = false);

	// Add Video Stream
	int AddVideoStream(	const LPBITMAPINFO pSrcFormat,
						const LPBITMAPINFO pDstFormat,
						DWORD dwDstRate,
						DWORD dwDstScale,
						int bitrate,
						int keyframes_rate,
						float qscale);	// 0.0f use bitrate, 2.0f best quality, 31.0f worst quality

	// Add a Raw Video Stream, no compressor
	// Only use AddRawVideoPacket and not AddFrame!
	int AddRawVideoStream(	const LPBITMAPINFO pFormat,
							int nFormatSize,
							DWORD dwRate,
							DWORD dwScale);

	// Add Audio Stream
	int AddAudioStream(	const LPWAVEFORMATEX pSrcWaveFormat,
						const LPWAVEFORMATEX pDstWaveFormat);

	// Add a Raw Audio Stream, no compressor
	// Only use AddRawAudioPacket and not AddAudioSamples!
	int AddRawAudioStream(	const LPWAVEFORMATEX pFormat,
							int nFormatSize,
							DWORD dwSampleSize,	// Set in avi strh
							DWORD dwRate,		// Set in avi strh
							DWORD dwScale);		// Set in avi strh

	// Set Info
	bool SetInfo(	LPCTSTR szTitle,
					LPCTSTR szAuthor,
					LPCTSTR szCopyright,
					LPCTSTR szComment,
					LPCTSTR szAlbum,
					LPCTSTR szGenre,
					int nTrack = 0,
					int nYear = 0);

	// Open
	bool Open();

	// This function returns true if the above function has been execute successfully
	bool IsOpen() const {return m_bOpen;};

	// Close
	bool Close();

	// Avi File Name
	__forceinline CString GetFileName() const	{return m_sFileName;};

	// Get Current File Size
	ULARGE_INTEGER GetFileSize();

	// Total Written Bytes for the given stream
	__forceinline LONGLONG GetTotalWrittenBytes(DWORD dwStreamNum) const	{return m_llTotalWrittenBytes[dwStreamNum];};

	// Video Streams Count
	__forceinline DWORD GetVideoStreamsCount() const	{return m_dwTotalVideoStreams;};

	// Video Streams Number to Overall Stream Number
	__forceinline DWORD VideoStreamNumToStreamNum(DWORD dwVideoStreamNum);

	// Add frame time
	static bool AddFrameTime(CDib* pDib, CTime RefTime, DWORD dwRefUpTime);
	static bool AddFrameTime(LPBYTE pBits,
							DWORD dwWidth,
							DWORD dwHeight,
							WORD wBitCount,
							DWORD dwFourCC,
							DWORD dwSizeImage,
							DWORD dwUpTime,
							CTime RefTime,
							DWORD dwRefUpTime);

	// Video Frame Write
	// Attention: if bAddFrameTime is set and if
	// not deinterlacing the time is added to the source bits!
	__forceinline bool AddFrame(DWORD dwStreamNum,
								CDib* pDib,
								bool bInterleaved,
								bool bDeinterlace = false,
								bool bAddFrameTime = false,
								CTime RefTime = CTime(2000, 1, 1, 12, 0, 0),
								DWORD dwRefUpTime = 0)
	{
		return AddFrame(dwStreamNum,
						pDib ? pDib->GetBMI() : NULL,
						pDib ? pDib->GetBits() : NULL,
						bInterleaved,
						bDeinterlace,
						bAddFrameTime,
						pDib ? pDib->GetUpTime() : 0,
						RefTime,
						dwRefUpTime);
	};
	bool AddFrame(	DWORD dwStreamNum,
					LPBITMAPINFO pBmi,
					LPBYTE pBits,
					bool bInterleaved,
					bool bDeinterlace = false,
					bool bAddFrameTime = false,
					DWORD dwUpTime = 0,
					CTime RefTime = CTime(2000, 1, 1, 12, 0, 0),
					DWORD dwRefUpTime = 0);
	
	// Get theora encoder statistics after first pass
	void TheoraStats(DWORD dwStreamNum);

	// Raw Video Packet Write
	bool AddRawVideoPacket(DWORD dwStreamNum, DWORD dwBytes, LPBYTE pBuf, bool bKeyframe, bool bInterleaved);

	// Audio Samples Write
	bool AddAudioSamples(DWORD dwStreamNum, DWORD dwNumSamples, LPBYTE pBuf, bool bInterleaved);

	// Raw Audio Packet Write
	bool AddRawAudioPacket(DWORD dwStreamNum, DWORD dwBytes, LPBYTE pBuf, bool bInterleaved);

	// Video Frame Manipulation Functions
	__forceinline LONGLONG GetFrameCount(DWORD dwStreamNum) const					{return m_llTotalFramesOrSamples[dwStreamNum];};
	__forceinline double GetFrameRate(DWORD dwStreamNum) const						{return (double)(GetRate(dwStreamNum)) / (double)(GetScale(dwStreamNum));};
	__forceinline bool GetFrameRate(DWORD dwStreamNum, DWORD* pdwRate, DWORD* pdwScale) const
	{
		if (pdwRate)
			*pdwRate = GetRate(dwStreamNum);
		if (pdwScale)
			*pdwScale = GetScale(dwStreamNum);
		return (pdwRate || pdwScale);
	};
	__forceinline DWORD GetRate(DWORD dwStreamNum) const {return	(m_pFormatCtx								&&
																	dwStreamNum < m_pFormatCtx->nb_streams		&&
																	m_pFormatCtx->streams[dwStreamNum]			&&
																	m_pFormatCtx->streams[dwStreamNum]->codec) ?
																	m_pFormatCtx->streams[dwStreamNum]->codec->time_base.den :
																	1;};
	__forceinline DWORD GetScale(DWORD dwStreamNum) const {return	(m_pFormatCtx								&&
																	dwStreamNum < m_pFormatCtx->nb_streams		&&
																	m_pFormatCtx->streams[dwStreamNum]			&&
																	m_pFormatCtx->streams[dwStreamNum]->codec) ?
																	m_pFormatCtx->streams[dwStreamNum]->codec->time_base.num :
																	1;};

	// Video Frame Sizes
	__forceinline DWORD GetWidth(DWORD dwStreamNum)	const {return	(m_pFormatCtx								&&
																	dwStreamNum < m_pFormatCtx->nb_streams		&&
																	m_pFormatCtx->streams[dwStreamNum]			&&
																	m_pFormatCtx->streams[dwStreamNum]->codec) ?
																	m_pFormatCtx->streams[dwStreamNum]->codec->width :
																	0;};
	__forceinline DWORD GetHeight(DWORD dwStreamNum) const {return	(m_pFormatCtx								&&
																	dwStreamNum < m_pFormatCtx->nb_streams		&&
																	m_pFormatCtx->streams[dwStreamNum]			&&
																	m_pFormatCtx->streams[dwStreamNum]->codec) ?
																	m_pFormatCtx->streams[dwStreamNum]->codec->height :
																	0;};
	
	// Audio Streams Count
	__forceinline DWORD GetAudioStreamsCount() const								{return m_dwTotalAudioStreams;};

	// Audio Streams Number to Overall Stream Number
	__forceinline DWORD AudioStreamNumToStreamNum(DWORD dwAudioStreamNum);

	// Audio Samples Manipulation Functions
	__forceinline LONGLONG GetSampleCount(DWORD dwStreamNum) const					{return m_llTotalFramesOrSamples[dwStreamNum];};

	// Audio Format Functions
	__forceinline LPWAVEFORMATEX GetSrcWaveFormat(DWORD dwStreamNum)				{return m_pSrcWaveFormat[dwStreamNum];};
	__forceinline LPWAVEFORMATEX GetIntermediateWaveFormat(DWORD dwStreamNum)		{return m_pIntermediateWaveFormat[dwStreamNum];};
	
protected:
	bool AddFrameInternal(	DWORD dwStreamNum,
							LPBITMAPINFO pBmi,
							LPBYTE pBits,
							bool bBitsReadonly,
							bool bInterleaved,
							bool bDeinterlace = false,
							bool bAddFrameTime = false,
							DWORD dwUpTime = 0,
							CTime RefTime = CTime(2000, 1, 1, 12, 0, 0),
							DWORD dwRefUpTime = 0);
	static __forceinline WORD DstDeinterlacePixFormatToBitsCount(PixelFormat pix_fmt);
	static __forceinline DWORD DstDeinterlacePixFormatToFourCC(PixelFormat pix_fmt);
	AVStream* CreateVideoStream(CodecID codec_id,
								const LPBITMAPINFO pDstVideoFormat,
								DWORD dwRate,
								DWORD dwScale,
								PixelFormat pix_fmt,
								int bitrate,
								int keyframes_rate,
								float qscale); // 0.0f use bitrate, 2.0f best quality, 31.0f worst quality
	AVStream* CreateAudioStream(CodecID codec_id,
								SampleFormat sample_fmt,
								int tag,
								int bitrate,
								int samplerate,
								int channels);
	bool AddAudioSamplesDirect(DWORD dwStreamNum, DWORD dwNumSamples, LPBYTE pBuf, bool bInterleaved);
	__forceinline bool EncodeSamples(DWORD dwStreamNum, int nInputSamplesCount, int nOutputBufSize, AVPacket* pPkt);
	__forceinline void SetSrcWaveFormat(DWORD dwStreamNum, const LPWAVEFORMATEX pWaveFormat);
	__forceinline void SetIntermediateWaveFormat(DWORD dwStreamNum, const LPWAVEFORMATEX pWaveFormat);
	void InitVars();	// Called by constructors
	void FlushAudio();

protected:

	// General Vars
	CString m_sFileName;
	CRITICAL_SECTION m_csAVI;
	volatile LONGLONG m_llTotalFramesOrSamples[MAX_STREAMS];
	volatile LONGLONG m_llTotalWrittenBytes[MAX_STREAMS];
	AVOutputFormat* m_pOutputFormat;
    AVFormatContext* m_pFormatCtx;
	uint8_t* m_pOutbuf[MAX_STREAMS];
	int m_nOutbufSize[MAX_STREAMS];		// In Bytes
	bool m_bFileOpened;
	bool m_bOpen;
	bool m_bCodecOpened[MAX_STREAMS];
	int m_nGlobalPassNumber;
	bool m_bFastEncode;

	// Video Vars
	volatile DWORD m_dwTotalVideoStreams;
	SwsContext* m_pImgConvertCtx[MAX_STREAMS];
	SwsContext* m_pDeinterlaceImgConvertCtx[MAX_STREAMS];
	AVFrame* m_pFrame[MAX_STREAMS];
	AVFrame* m_pFrameTemp[MAX_STREAMS];
	LPBYTE m_pFrameBuf1[MAX_STREAMS];
	int m_nFrameBufSize1[MAX_STREAMS];
	LPBYTE m_pFrameBuf2[MAX_STREAMS];
	int m_nFrameBufSize2[MAX_STREAMS];
	LPBYTE m_pFrameBuf3[MAX_STREAMS];
	int m_nFrameBufSize3[MAX_STREAMS];
	LPBYTE m_pFrameBuf4[MAX_STREAMS];
	int m_nFrameBufSize4[MAX_STREAMS];
	FILE* m_p2PassLogFiles[MAX_STREAMS];
	CString m_s2PassLogFileName[MAX_STREAMS];
	int m_nPassNumber[MAX_STREAMS];
	CString m_sTempDir;

	// LIBAVCodec Audio Vars
	volatile DWORD m_dwTotalAudioStreams;
	LPWAVEFORMATEX volatile m_pSrcWaveFormat[MAX_STREAMS];
	LPWAVEFORMATEX volatile m_pIntermediateWaveFormat[MAX_STREAMS];
	int m_nAudioInputFrameSize[MAX_STREAMS];		// In Samples
	int m_nIntermediateSamplesBufPos[MAX_STREAMS];	// In Samples
	uint8_t* m_pIntermediateSamplesBuf[MAX_STREAMS];
	int m_nIntermediateSamplesBufSize[MAX_STREAMS];	// In Bytes
	uint8_t* m_pTempSamplesBuf[MAX_STREAMS];
	int m_nTempSamplesBufSize[MAX_STREAMS];		// In Bytes
	uint8_t* m_pTempSamplesBuf2[MAX_STREAMS];
	int m_nTempSamplesBufSize2[MAX_STREAMS];	// In Bytes
	ReSampleContext* m_pAudioResampleCtx[MAX_STREAMS];
};

__forceinline DWORD CAVRec::VideoStreamNumToStreamNum(DWORD dwVideoStreamNum)
{
	// Check
	if (!m_pFormatCtx)
		return 0;

	// Check
	if (dwVideoStreamNum >= m_dwTotalVideoStreams)
		return m_pFormatCtx->nb_streams - 1;
		
	DWORD dwVideoStreamCount = 0;
	for (DWORD dwStreamNum = 0 ; dwStreamNum < m_pFormatCtx->nb_streams ; dwStreamNum++)
	{
		if (m_pFormatCtx->streams[dwStreamNum]->codec->codec_type == CODEC_TYPE_VIDEO)
			dwVideoStreamCount++;
		if (dwVideoStreamCount == dwVideoStreamNum + 1)
			return dwStreamNum;
	}

	return 0;
}

__forceinline DWORD CAVRec::AudioStreamNumToStreamNum(DWORD dwAudioStreamNum)
{
	// Check
	if (!m_pFormatCtx)
		return 0;

	// Check
	if (dwAudioStreamNum >= m_dwTotalAudioStreams)
		return m_pFormatCtx->nb_streams - 1;
		
	DWORD dwAudioStreamCount = 0;
	for (DWORD dwStreamNum = 0 ; dwStreamNum < m_pFormatCtx->nb_streams ; dwStreamNum++)
	{
		if (m_pFormatCtx->streams[dwStreamNum]->codec->codec_type == CODEC_TYPE_AUDIO)
			dwAudioStreamCount++;
		if (dwAudioStreamCount == dwAudioStreamNum + 1)
			return dwStreamNum;
	}

	return 0;
}

#endif //!_INC_AVREC