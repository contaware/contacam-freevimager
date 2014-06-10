#ifndef _INC_AVREC
#define _INC_AVREC

#include "Dib.h"
#include <mmsystem.h>
#include "YuvToRgb.h"
#include "DxDraw.h"
extern "C"
{
#include "libavutil/opt.h"
#include "libavutil/mathematics.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
}

// Used to Convert From Double to Fractional Frame-Rate.
// Do not use a higher value, AV_CODEC_ID_MPEG4 is not working!
#define MAX_SIZE_FOR_RATIONAL				65535

// Also defined in AviPlay.h!
#define AUDIO_PCM_MIN_BUF_SIZE				8192

// Maximum number of streams
#define MAX_STREAMS							20

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
			bool bFastEncode = false);

	// Destructor
	virtual ~CAVRec();

	// Init
	bool Init(	LPCTSTR lpszFileName,
				bool bFastEncode = false);

	// Add Video Stream
	int AddVideoStream(	const LPBITMAPINFO pSrcFormat,
						const LPBITMAPINFO pDstFormat,
						DWORD dwDstRate,
						DWORD dwDstScale,
						int keyframes_rate,
						float qscale,	// 2.0f best quality, 31.0f worst quality
						int nThreadCount);

	// Add Audio Stream
	int AddAudioStream(	const LPWAVEFORMATEX pSrcWaveFormat,
						const LPWAVEFORMATEX pDstWaveFormat);

	// Open
	bool Open();

	// This function returns true if the above function has been execute successfully
	bool IsOpen() const {return m_bOpen;};

	// Close
	bool Close();

	// Avi File Name
	__forceinline CString GetFileName() const	{return m_sFileName;};

	// Total Written Bytes for the given stream
	__forceinline LONGLONG GetTotalWrittenBytes(DWORD dwStreamNum) const	{return m_llTotalWrittenBytes[dwStreamNum];};

	// Video Streams Count
	__forceinline DWORD GetVideoStreamsCount() const	{return m_dwTotalVideoStreams;};

	// Video Streams Number to Overall Stream Number
	__forceinline DWORD VideoStreamNumToStreamNum(DWORD dwVideoStreamNum);

	// Video Frame Write, for flushing set pDib to NULL or pBmi and pBits to NULL
	__forceinline bool AddFrame(DWORD dwStreamNum,
								CDib* pDib,
								bool bInterleaved)
	{
		return AddFrame(dwStreamNum,
						pDib ? pDib->GetBMI() : NULL,
						pDib ? pDib->GetBits() : NULL,
						bInterleaved);
	};
	bool AddFrame(	DWORD dwStreamNum,
					LPBITMAPINFO pBmi,
					LPBYTE pBits,
					bool bInterleaved);

	// Audio Samples Write, for flushing set pBuf to NULL and dwNumSamples to 0
	bool AddAudioSamples(DWORD dwStreamNum, DWORD dwNumSamples, LPBYTE pBuf, bool bInterleaved);

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
	
protected:
	__forceinline void SetSrcWaveFormat(DWORD dwStreamNum, const LPWAVEFORMATEX pWaveFormat);
	__forceinline void AdjustPTSDTS(DWORD dwStreamNum, AVPacket* pkt);
	bool EncodeAndWriteFrame(	DWORD dwStreamNum,
								AVFrame* pFrame, // set to NULL to flush
								bool bInterleaved);
	void InitVars();	// Called by constructors
	void Flush();

protected:

	// General Vars
	CString m_sFileName;
	CRITICAL_SECTION m_csAVI;
	AVOutputFormat* m_pOutputFormat;
    AVFormatContext* m_pFormatCtx;
	bool m_bFileOpened;
	bool m_bOpen;
	bool m_bFastEncode;
	volatile DWORD m_dwTotalVideoStreams;
	volatile DWORD m_dwTotalAudioStreams;

	// Conversion Contexts
	SwsContext* m_pImgConvertCtx[MAX_STREAMS];
	SwrContext* m_pAudioConvertCtx[MAX_STREAMS];

	// Buffers
	uint8_t** m_ppSrcBuf[MAX_STREAMS];
	int m_nSrcBufSize[MAX_STREAMS];
	uint8_t** m_ppDstBuf[MAX_STREAMS];
	int m_nDstBufSize[MAX_STREAMS];

	// Frames
	AVFrame* m_pFrame[MAX_STREAMS];
	AVFrame* m_pFrameTemp[MAX_STREAMS];

	// Wave Format
	LPWAVEFORMATEX volatile m_pSrcWaveFormat[MAX_STREAMS];

	// Counters
	volatile LONGLONG m_llTotalFramesOrSamples[MAX_STREAMS];
	volatile LONGLONG m_llTotalWrittenBytes[MAX_STREAMS];
	volatile LONGLONG m_llLastDTS[MAX_STREAMS];
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
		if (m_pFormatCtx->streams[dwStreamNum]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
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
		if (m_pFormatCtx->streams[dwStreamNum]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
			dwAudioStreamCount++;
		if (dwAudioStreamCount == dwAudioStreamNum + 1)
			return dwStreamNum;
	}

	return 0;
}

#endif //!_INC_AVREC