#ifndef _INC_AVIFILE
#define _INC_AVIFILE

#include <mmsystem.h>
#include <vfw.h>
#include "Dib.h"
#include "SortableStringArray.h"

#define WM_AVIFILE_PROGRESS			WM_USER + 500
#define MAX_AUDIO_STREAMS			10
#define MAX_VIDEO_STREAMS			10

class CAVIFile
{
public:
	typedef struct {
	volatile DWORD	fccType;			// stream type, for consistency
	volatile DWORD	fccHandler;			// compressor
	volatile DWORD	dwKeyFrameEvery;	// keyframe rate
	volatile DWORD	dwQuality;			// compress quality 0-10,000
	volatile DWORD	dwBytesPerSecond;	// bytes per second
	volatile DWORD	dwFlags;			// flags... see below
	volatile LPVOID	lpFormat;			// save format
	volatile DWORD	cbFormat;			// save format size
	volatile LPVOID	lpParms;			// compressor options
	volatile DWORD	cbParms;			// compressor options size
	volatile DWORD	dwInterleaveEvery;	// for non-video streams only
} AVICOMPRESSOPTIONSVOLATILE, FAR *LPAVICOMPRESSOPTIONSVOLATILE;

public:
	///////////////////////
	// General Functions //
	///////////////////////

	// Simple Constructors
	CAVIFile();
	CAVIFile(	CWnd* pParentWnd);
	CAVIFile(	CWnd* pParentWnd,
				bool bShowMessageBoxOnError);

	// Constructor To Write An Avi File
	CAVIFile(	CWnd* pParentWnd,
				LPCTSTR lpszFileName,
				DWORD dwRate,
				DWORD dwScale,
				const LPWAVEFORMATEX pWaveFormat,
				bool bCreateFile,
				bool bTruncateFile,
				bool bShowMessageBoxOnError);
	
	// Constructor To Read An Avi File
	CAVIFile(	CWnd* pParentWnd,
				LPCTSTR lpszFileName,
				bool bShowMessageBoxOnError = true);

	// Destructor
	virtual ~CAVIFile();

	// Free & Close
	void Close();

	// Function used to give access to the file
	void CloseFile();

	// Avi File Name
	CString GetFileName() const					{return m_sFileName;};
	void SetFileName(CString sFileName)			{m_sFileName = sFileName;};

	// Parent Wnd (For Compressor Dialog)
	CWnd* GetParentWnd() const					{return m_pParentWnd;};
	void SetParentWnd(CWnd* pParentWnd)			{m_pParentWnd = pParentWnd;};

	// The ShowMessageBoxOnError Flag
	void SetShowMessageBoxOnError(bool bShowMessageBoxOnError)
												{m_bShowMessageBoxOnError =
													bShowMessageBoxOnError;};
	bool IsShowMessageBoxOnError() const {return m_bShowMessageBoxOnError;};

	// Exit The ReCompress Process
	void SetKillEvent(HANDLE hKillEvent)		{m_hKillEvent = hKillEvent;};

	// Get Current File Size
	ULARGE_INTEGER GetFileSize();

	///////////////////////////////
	// Video And Audio Functions //
	///////////////////////////////

	// Initialize Audio / Video Reading
	bool InitRead(UINT uiMode = (OF_READ | OF_SHARE_DENY_NONE));	// uiMode : OF_READ, OF_READWRITE
																	//			OF_WRITE, OF_CREATE 
																	//			OF_SHARE_DENY_WRITE,
																	//			OF_SHARE_DENY_READ,
																	//			OF_SHARE_EXCLUSIVE
																	//			OF_SHARE_DENY_NONE 

	// If No Error Occured, OK is set
	bool IsOk() const							{return m_bOk;};
	void SetOk(bool bOk)						{m_bOk = bOk;};

	// Rewind
	void Rew(); // All Streams

	// Audio Video Player Sync Function
	void PlaySyncAudioFromVideo(DWORD dwAudioStreamNum, DWORD dwVideoStreamNum, CAVIFile* pAVIVideo = NULL);
	void PlaySyncVideoFromAudio(DWORD dwVideoStreamNum, DWORD dwAudioStreamNum, CAVIFile* pAVIAudio = NULL);

	// Stream Cloning Function (Saves The Specified Streams To A New File)
	enum {SAVECOPYAS_VIDEO, SAVECOPYAS_AUDIO, SAVECOPYAS_BOTH, SAVECOPYAS_UNKNOWN};
	static LONG WINAPI SaveCopyAsCallback(int nPercent);
	bool SaveCopyAs(CString sFileName, int nStreams, PAVISTREAM* ppAviStreams);
	
	// Merge dwFilesCount Avi Files Streams in Parallel
	bool MergeAviFilesParallel(	CString sOutputFileName,
								CSortableStringArray* pAviFileNames);

	///////////
	// Video //
	///////////

	// Avi File Has Video Stream?
	bool HasVideo() const								{return ((m_pVideoGetFrame[0] != NULL) && (m_dwTotalFrames[0] > 0));};

	// Initialize Video Writing
	bool InitVideoWrite(DWORD dwStreamNum,
						LPBITMAPINFO pBMI,
						LPCTSTR szStreamName = NULL);

	// Get Stream
	PAVISTREAM GetVideoStream(DWORD dwStreamNum) const	{return m_pVideoStream[dwStreamNum];};

	// Streams Count
	DWORD GetVideoStreamsCount() const					{return m_dwTotalVideoStreams;};

	// Stream Name
	CString GetVideoStreamName(DWORD dwStreamNum) const	{return m_VideoStreamName[dwStreamNum];};

	// Frame Read / Write And Skip
	bool AddFrame(DWORD dwStreamNum, CDib* pDib);										// Dib
	bool GetFrame(DWORD dwStreamNum, CDib* pDib);										// Dib
	bool GetFrameAt(DWORD dwStreamNum, CDib* pDib, DWORD dwFrame);						// Dib
	bool SkipFrame(DWORD dwStreamNum);

	// Video Frame Manipulation Functions
	void RewVideo(DWORD dwStreamNum);
	void RewVideo();
	DWORD GetTotalWrittenVideoBytes(DWORD dwStreamNum) const			{return m_dwTotalWrittenVideoBytes[dwStreamNum];};
	DWORD GetFrameCount(DWORD dwStreamNum) const						{return m_dwTotalFrames[dwStreamNum];};
	double GetFrameRate(DWORD dwStreamNum) const						{return (double)(m_dwRate[dwStreamNum]) / (double)(m_dwScale[dwStreamNum]);};
	bool GetFrameRate(DWORD dwStreamNum, DWORD* pdwRate, DWORD* pdwScale) const;
	DWORD GetRate(DWORD dwStreamNum) const {return m_dwRate[dwStreamNum];};
	DWORD GetScale(DWORD dwStreamNum) const {return m_dwScale[dwStreamNum];};
	void SetFrameRate(DWORD dwStreamNum, DWORD dwRate, DWORD dwScale)	{m_dwRate[dwStreamNum] = dwRate; m_dwScale[dwStreamNum] = dwScale;};
	DWORD GetStartVideoOffset(DWORD dwStreamNum) const				{return m_dwStartVideoOffset[dwStreamNum];};
	int GetReadCurrentFramePos(DWORD dwStreamNum) const				{return (int)m_dwReadNextFrame[dwStreamNum] - 1;};
	DWORD GetReadNextFramePos(DWORD dwStreamNum) const				{return m_dwReadNextFrame[dwStreamNum];};
	int GetWriteCurrentFramePos(DWORD dwStreamNum) const				{return (int)m_dwWriteNextFrame[dwStreamNum] - 1;};
	DWORD GetWriteNextFramePos(DWORD dwStreamNum) const				{return m_dwWriteNextFrame[dwStreamNum];};
	bool SetReadCurrentFramePos(DWORD dwStreamNum, int nCurrentFramePos);
	bool SetReadNextFramePos(DWORD dwStreamNum, DWORD dwNextFramePos);
	bool SetWriteCurrentFramePos(DWORD dwStreamNum, int nCurrentFramePos);
	bool SetWriteNextFramePos(DWORD dwStreamNum, DWORD dwNextFramePos);

	// Video Frame Sizes
	DWORD GetWidth(DWORD dwStreamNum)	const						{return m_dwWidth[dwStreamNum];};
	DWORD GetHeight(DWORD dwStreamNum)	const						{return m_dwHeight[dwStreamNum];};	
	
	// Video Compressor Functions
	LPAVICOMPRESSOPTIONSVOLATILE GetVideoCompressorOptions(DWORD dwStreamNum)
								{return &m_VideoCompressorOptions[dwStreamNum];};
	
	// Interleave Audio Samples every Video Frame
	static bool Interleave(	CString sInputFileName,	
							CString sOutputFileName,
							CWnd* pWnd = NULL,
							HANDLE hKillEvent = NULL,
							bool bShowMessageBoxOnError = true);

	// Recompress To Output File:
	// - Streams marked with the save flag are copied,
	// - Streams not marked are dropped,
	// - Streams marked with the save and the change flags
	//   are recompressed using the given compression parameters,
	//   if the parameters are NULL a dialog box is popped up for each stream!
	bool ReCompress(CString sOutputFileName,
					bool* pbVideoStreamsSave,
					bool* pbVideoStreamsChange,
					bool* pbAudioStreamsSave,
					bool* pbAudioStreamsChange,
					const DWORD* pdwFourCC,				// May be NULL
					const LPWAVEFORMATEX pWaveFormat,	// May be NULL
					const DWORD* pdwQuality,			// May be NULL
					const DWORD* pdwKeyframesRate,		// May be NULL
					const DWORD* pdwDataRate,			// May be NULL
					const BYTE* pVideoConfigData,		// May be NULL
					DWORD dwVideoConfigDataSize);		// May be 0

	// Recompress To Current File:
	// - Streams marked with the save flag are copied,
	// - Streams not marked are dropped,
	// - Streams marked with the save and the change flags
	//   are recompressed using the given compression parameters,
	//   if the parameters are NULL a dialog box is popped up for each stream!
	bool ReCompress(bool* pbVideoStreamsSave,
					bool* pbVideoStreamsChange,
					bool* pbAudioStreamsSave,
					bool* pbAudioStreamsChange,
					const DWORD* pdwFourCC,				// May be NULL
					const LPWAVEFORMATEX pWaveFormat,	// May be NULL
					const DWORD* pdwQuality,			// May be NULL
					const DWORD* pdwKeyframesRate,		// May be NULL
					const DWORD* pdwDataRate,			// May be NULL
					const BYTE* pVideoConfigData,		// May be NULL
					DWORD dwVideoConfigDataSize);		// May be 0

	bool VideoCompressorDialog(DWORD dwStreamNum);
	static bool HasVideoCompressorAboutDialog(HIC hIC);
	bool VideoCompressorAboutDialog(HIC hIC);
	static bool HasVideoCompressorConfigDialog(HIC hIC);
	bool VideoCompressorConfigDialog(HIC hIC);
	HIC OpenVideoCompressor(DWORD dwStreamNum);
	void CloseVideoCompressor(HIC hIC);
	bool SetVideoCompressorConfig(DWORD dwStreamNum, HIC hIC, const BYTE* pConfigData = NULL, DWORD dwSize = 0);
	bool GetVideoCompressorConfig(DWORD dwStreamNum, HIC hIC, BYTE** ppConfigData = NULL, DWORD* pSize = NULL);
	static bool EnumVideoCodecs(CStringArray &Names,
								CDWordArray &fcc,
								CDWordArray &QualitySupport,
								CDWordArray &KeyframeRateSupport,
								CDWordArray &DataRateSupport,
								CDWordArray &AboutDlgSupport,
								CDWordArray &ConfigDlgSupport);
	DWORD ChooseVideoCompressorForSave(DWORD dwFourCC);
	DWORD AutoChooseVideoCompressor();
	CString GetVideoCompressorName(DWORD dwStreamNum) const;
	CString GetVideoCompressorDriverName(DWORD dwStreamNum) const;
	CString GetVideoCompressorDescription(DWORD dwStreamNum) const;
	bool SetVideoCompressor(DWORD dwStreamNum, CString sFourCC);
	bool SetVideoCompressor(DWORD dwStreamNum, DWORD dwFourCC);
	void SetVideoCompressorQuality(DWORD dwStreamNum, int nVideoCompressorQuality)
				{m_VideoCompressorOptions[dwStreamNum].dwQuality = (DWORD)nVideoCompressorQuality;};
	int GetVideoCompressorQuality(DWORD dwStreamNum) const
				{return (int)m_VideoCompressorOptions[dwStreamNum].dwQuality;};
	void SetVideoCompressorKeyframesRate(DWORD dwStreamNum, int nVideoCompressorKeyframesRate)
				{m_VideoCompressorOptions[dwStreamNum].dwKeyFrameEvery = (DWORD)nVideoCompressorKeyframesRate;};
	int GetVideoCompressorKeyframesRate(DWORD dwStreamNum) const
				{return (int)m_VideoCompressorOptions[dwStreamNum].dwKeyFrameEvery;};
	void SetVideoCompressorDataRate(DWORD dwStreamNum, int nVideoCompressorDataRate)
				{m_VideoCompressorOptions[dwStreamNum].dwBytesPerSecond = (DWORD)nVideoCompressorDataRate;};
	int GetVideoCompressorDataRate(DWORD dwStreamNum) const
				{return (int)m_VideoCompressorOptions[dwStreamNum].dwBytesPerSecond;};
	
	// Video FourCC Functions
	DWORD GetFourCC(DWORD dwStreamNum)	const						{return m_dwFourCC[dwStreamNum];};
	CString GetFourCCString(DWORD dwStreamNum)	const				{return FourCCToString(m_dwFourCC[dwStreamNum]);};
	static CString FourCCToString(DWORD dwFourCC);
	static CString FourCCToStringLowerCase(DWORD dwFourCC);
	static CString FourCCToStringUpperCase(DWORD dwFourCC);
	static DWORD StringToFourCC(CString sFourCC);
	static DWORD FourCCMakeLowerCase(DWORD dwFourCC);
	static DWORD FourCCMakeUpperCase(DWORD dwFourCC);

	///////////
	// Audio //
	///////////

	// Avi File Has Audio Stream?
	bool HasAudio() const								{return ((m_pAudioStream[0] != NULL) && (m_dwTotalSamples > 0));};

	bool IsAudioReadable(DWORD dwStreamNum);

	// Initialize Audio Writing
	bool InitAudioWrite(DWORD dwStreamNum, LPCTSTR szStreamName = NULL);

	// Get Stream
	PAVISTREAM GetAudioStream(DWORD dwStreamNum) const	{return m_pAudioStream[dwStreamNum];};

	// Streams Count
	DWORD GetAudioStreamsCount() const					{return m_dwTotalAudioStreams;};

	// Stream Name
	CString GetAudioStreamName(DWORD dwStreamNum) const	{return m_AudioStreamName[dwStreamNum];};

	// Audio Samples Read / Write
	bool AddAudioSamples(DWORD dwStreamNum, DWORD dwNumSamples, LPBYTE pBuf);
	bool GetAudioSamples(DWORD dwStreamNum, DWORD dwNumSamples, LPBYTE pBuf, DWORD dwBufSize, DWORD* pWrittenBytes, DWORD* pWrittenSamples);
	bool GetAudioSamplesAt(DWORD dwStreamNum, DWORD dwStartSample, DWORD dwNumSamples, LPBYTE pBuf, DWORD dwBufSize, DWORD* pWrittenBytes, DWORD* pWrittenSamples);

	// Audio Samples Manipulation Functions
	void RewAudio(DWORD dwStreamNum);
	void RewAudio();
	DWORD GetTotalWrittenAudioBytes(DWORD dwStreamNum) const			{return m_dwTotalWrittenAudioBytes[dwStreamNum];};
	DWORD GetSampleCount(DWORD dwStreamNum) const						{return m_dwTotalSamples[dwStreamNum];};
	int GetReadCurrentSamplePos(DWORD dwStreamNum) const				{return (int)m_dwReadNextSample[dwStreamNum] - 1;};
	DWORD GetReadNextSamplePos(DWORD dwStreamNum) const					{return m_dwReadNextSample[dwStreamNum];};
	int GetWriteCurrentSamplePos(DWORD dwStreamNum) const				{return (int)m_dwWriteNextSample[dwStreamNum] - 1;};
	DWORD GetWriteNextSamplePos(DWORD dwStreamNum) const				{return m_dwWriteNextSample[dwStreamNum];};
	bool SetReadCurrentSamplePos(DWORD dwStreamNum, int nCurrentSamplePos);
	bool SetReadNextSamplePos(DWORD dwStreamNum, DWORD dwNextSamplePos);
	bool SetWriteCurrentSamplePos(DWORD dwStreamNum, int nCurrentSamplePos);
	bool SetWriteNextSamplePos(DWORD dwStreamNum, DWORD dwNextSamplePos);
	
	// Audio Properties Functions
	DWORD GetAudioSkew(DWORD dwStreamNum) const							{return m_dwAudioSkew[dwStreamNum];};
	DWORD GetSuggestedAudioBufferSize(DWORD dwStreamNum) const			{return m_dwSuggestedAudioBufferSize[dwStreamNum];};
	DWORD GetStartAudioOffset(DWORD dwStreamNum) const					{return m_dwStartAudioOffset[dwStreamNum];};

	// Audio Format Functions
	LPWAVEFORMATEX GetWaveFormat(DWORD dwStreamNum)						{return m_pWaveFormat[dwStreamNum];};
	WORD GetWaveFormatTag(DWORD dwStreamNum) const						{return m_pWaveFormat[dwStreamNum]->wFormatTag;}; // PCM is WAVE_FORMAT_PCM
	void InitWaveFormatTagTable();
	CString GetWaveFormatTagString(WORD wFormatTag);
	DWORD GetWaveSampleRate(DWORD dwStreamNum) const					{return m_pWaveFormat[dwStreamNum]->nSamplesPerSec;}; // In Hz
	WORD GetWaveBits(DWORD dwStreamNum) const							{return m_pWaveFormat[dwStreamNum]->wBitsPerSample;};  // 8 or 16 bits
	WORD GetWaveNumOfChannels(DWORD dwStreamNum) const					{return m_pWaveFormat[dwStreamNum]->nChannels;}; // Mono = 1, Stereo = 2
	WORD GetWaveSampleSize(DWORD dwStreamNum) const						{return m_pWaveFormat[dwStreamNum]->nBlockAlign;}; // In Bytes
	DWORD GetWaveBytesPerSeconds(DWORD dwStreamNum) const				{return m_pWaveFormat[dwStreamNum]->nAvgBytesPerSec;}; // If PCM:  nSamplesPerSec * nBlockAlign
	void SetWaveFormat(DWORD dwStreamNum, const LPWAVEFORMATEX pWaveFormat);
	void SetWaveFormatTag(DWORD dwStreamNum, WORD wFormatTag)			{m_pWaveFormat[dwStreamNum]->wFormatTag = wFormatTag;}; // PCM is WAVE_FORMAT_PCM
	void SetWaveSampleRate(DWORD dwStreamNum, DWORD nSamplesPerSec)		{m_pWaveFormat[dwStreamNum]->nSamplesPerSec = nSamplesPerSec;}; // In Hz
	void SetWaveBits(DWORD dwStreamNum, WORD wBitsPerSample)			{m_pWaveFormat[dwStreamNum]->wBitsPerSample = wBitsPerSample;}; // 8 or 16 bits
	void SetWaveNumOfChannels(DWORD dwStreamNum, WORD nChannels)		{m_pWaveFormat[dwStreamNum]->nChannels = nChannels;}; // Mono = 1, Stereo = 2
	void SetWaveSampleSize(DWORD dwStreamNum, WORD nBlockAlign)			{m_pWaveFormat[dwStreamNum]->nBlockAlign = nBlockAlign;}; // In Bytes
	void SetWaveBytesPerSeconds(DWORD dwStreamNum, DWORD nAvgBytesPerSec){m_pWaveFormat[dwStreamNum]->nAvgBytesPerSec = nAvgBytesPerSec;}; // If PCM:  nSamplesPerSec * nBlockAlign

	// Audio Compressor Functions
	static BOOL WINAPI AcmFormatEnumCallback(HACMDRIVERID hadid, LPACMFORMATDETAILS pafd, DWORD dwInstance, DWORD fdwSupport);
	bool AutoChooseAudioCompressor(DWORD dwStreamNum, LPWAVEFORMATEX* ppWaveFormat);
	bool AudioCompressorDialog(DWORD dwStreamNum);
	bool AudioCodec(DWORD dwInStreamNum, DWORD dwOutStreamNum, CAVIFile* pOutAvi, bool bForceOnePass = false);

	// Audio Stream Copy From Source To This
	bool AudioCopyFrom(DWORD dwSrcStreamNum, DWORD dwDstStreamNum, CAVIFile* pSourceAVIFile);

protected:

	// Tests the Exit Event, for Long Processing Interruption
	__forceinline bool DoExit()
	{
		if (m_hKillEvent)
		{
			DWORD Event = ::WaitForSingleObject(m_hKillEvent, 0);
			switch (Event)
			{
				// Shutdown Event?
				case WAIT_OBJECT_0 :	return true;
				case WAIT_TIMEOUT :		return false;
				default:				return true;
			}
		}
		else
			return false;
	};

protected:

	// General Vars
	CWnd* volatile m_pParentWnd;
	CString m_sFileName;
	volatile bool m_bOk;
	volatile bool m_bInitRead;
	PAVIFILE volatile m_pFile;
	HANDLE volatile m_hKillEvent;
	CRITICAL_SECTION m_csAVI;
	volatile bool m_bCreateFile;
	volatile bool m_bTruncateFile;
	volatile UINT m_uiFileMode;
	volatile bool m_bShowMessageBoxOnError;// Display MessageBox on Error

	// Video Vars
	volatile DWORD m_dwTotalVideoStreams;
	CStringArray m_VideoStreamName;
	volatile DWORD m_dwRate[MAX_VIDEO_STREAMS];
	volatile DWORD m_dwScale[MAX_VIDEO_STREAMS];
	volatile DWORD m_dwStartVideoOffset[MAX_VIDEO_STREAMS];
	volatile DWORD m_dwReadNextFrame[MAX_VIDEO_STREAMS];
	volatile DWORD m_dwWriteNextFrame[MAX_VIDEO_STREAMS];
	volatile DWORD m_dwTotalWrittenVideoBytes[MAX_VIDEO_STREAMS];
	volatile DWORD m_dwTotalFrames[MAX_VIDEO_STREAMS];
	volatile DWORD m_dwHeight[MAX_VIDEO_STREAMS];
	volatile DWORD m_dwWidth[MAX_VIDEO_STREAMS];
	PAVISTREAM volatile m_pVideoStream[MAX_VIDEO_STREAMS];
	PAVISTREAM volatile m_pVideoStreamCompressed[MAX_VIDEO_STREAMS];
	PGETFRAME volatile m_pVideoGetFrame[MAX_VIDEO_STREAMS];
	AVICOMPRESSOPTIONSVOLATILE m_VideoCompressorOptions[MAX_VIDEO_STREAMS];
	volatile DWORD m_dwFourCC[MAX_VIDEO_STREAMS];
	LPBITMAPINFOHEADER volatile m_pCompressedFormat[MAX_VIDEO_STREAMS];
	volatile DWORD m_dwCompressedFormatSize[MAX_VIDEO_STREAMS];
	LPBYTE volatile m_pCompressedBuf[MAX_VIDEO_STREAMS];
	LPBYTE volatile m_pPrevUncompressedBuf[MAX_VIDEO_STREAMS];
	HIC volatile m_hCompressionIC[MAX_VIDEO_STREAMS];
	volatile int m_nKeyRateCounter[MAX_VIDEO_STREAMS];

	// Audio Vars
	volatile DWORD m_dwTotalAudioStreams;
	CStringArray m_AudioStreamName;
	CDWordArray m_AudioChunkSize;
	CDWordArray m_AudioChunkOffset;
	volatile DWORD m_dwTotalWrittenAudioBytes[MAX_AUDIO_STREAMS];
	volatile DWORD m_dwTotalSamples[MAX_AUDIO_STREAMS];
	volatile DWORD m_dwReadNextSample[MAX_AUDIO_STREAMS];
	volatile DWORD m_dwWriteNextSample[MAX_AUDIO_STREAMS];
	PAVISTREAM volatile m_pAudioStream[MAX_AUDIO_STREAMS];
	LPWAVEFORMATEX volatile m_pWaveFormat[MAX_AUDIO_STREAMS];
	volatile DWORD m_dwAudioSkew[MAX_AUDIO_STREAMS];
	volatile DWORD m_dwSuggestedAudioBufferSize[MAX_AUDIO_STREAMS];
	volatile DWORD m_dwStartAudioOffset[MAX_AUDIO_STREAMS];

private:
	static volatile int m_nPrevPercentDone;
	static volatile int m_nSaveCopyAsIsProcessing;
	static CAVIFile* volatile pThis;
};

#endif //!_INC_AVIFILE
