#ifndef _INC_AVIPLAY
#define _INC_AVIPLAY

#include <mmsystem.h>
#include <mmreg.h>
#include <msacm.h>
#include <vfw.h>
#include "Dib.h"
#include "YuvToRgb.h"
#include "DxDraw.h"
#if (_MSC_VER <= 1200)
#include "BigFile.h"
#endif
extern "C"
{
#include "ffmpeg\\libavcodec\\avcodec.h"
#include "ffmpeg\\libavformat\\avformat.h"
#include "ffmpeg\\libswscale\\swscale.h"
}

#ifndef BI_BGR16
#define BI_BGR16							mmioFOURCC('B','G','R',16)
#endif

#ifndef BI_BGR15
#define BI_BGR15							mmioFOURCC('B','G','R',15)
#endif

#ifndef BI_RGB16
#define BI_RGB16							mmioFOURCC('R','G','B',16)
#endif

#ifndef BI_RGB15
#define BI_RGB15							mmioFOURCC('R','G','B',15)
#endif

// Maximum number of supported streams
#define MAX_AUDIO_STREAMS					10
#define MAX_VIDEO_STREAMS					10

// If no index use the given amount of chunks to guess the total number of them
#define NOINDEX_STAT_CHUNKS					300

// Also defined in AVRec.h!
#define AUDIO_PCM_MIN_BUF_SIZE				8192

// This Is Used When the Suggested
// Buffer Size is not valid (usually 0)
#define AUDIO_DEFAULT_SUGGESTED_BUFFER_SIZE	8192

#define AUDIO_VBR_MIN_BUF_SIZE				8192

// Used to Convert From Double to Fractional Frame-Rate
#define	AVI_SCALE_INT						10000000
#define	AVI_SCALE_DOUBLE					10000000.0

// Maximum gop size to avoid locking the seek to long
#define MAX_KEYFRAMES_SPACING				1024

// Lib AVCodec Defines: Codec Id as Parameter
#define mpeg_codec(x)		((x)==CODEC_ID_MPEG1VIDEO || (x)==CODEC_ID_MPEG2VIDEO)
#define mjpeg_codex(x)		((x)==CODEC_ID_MJPEG || (x)==CODEC_ID_MJPEGB || (x)==CODEC_ID_SP5X)

/*
Timing of an AVI stream is governed by several variables:

- The sample rate (dwRate/dwScale) of the stream determines
  the spacing in time between the samples. For instance,
  a video stream might have a sample rate of 25 samples
  per second, so each sample is 1/25th of a second or
  40 milliseconds apart. This is actually stored as a fraction
  of two 32-bit values so many values that would ordinarily
  have to be approximated in integer or floating-point math
  can still be represented exactly; for instance, NTSC frame
  rate is 30000/1001. VirtualDub tries to use fraction math
  whenever possible to preserve the exact rate.
- The start (dwStart) of the stream determines when the first
  sample in the stream starts. A start value of 2 for a 25 sample/sec
  stream would mean 80ms of dead time before the first sample starts.
  Generally this is filled in by extending the first sample backwards
  in time.
- The sample size (dwSampleSize) determines the sample-to-chunk mapping.
  If it is zero, one sample is stored per chunk, and each sample can
  have a different size. This is used for video streams.
  If it is non-zero, each sample is the same number of bytes in size,
  and the number of samples in a chunk is determined from the size of
  the chunk. Audio streams use this mode, with the sample size being
  the same as the block size in the audio format
  (WAVEFORMATEX::nBlockAlign).

About VBR Audio:

  You might think that setting dwSampleSize=0 for an audio stream would
  allow it to be encoded as variable bitrate (VBR) like a video stream,
  where each sample has a different size. Unfortunately, this is not
  the case -- Microsoft AVI parsers simply ignore dwSampleSize for audio
  streams and use nBlockAlign from the WAVEFORMATEX audio structure instead,
  which cannot be zero. Nuts. So how is it done, then?

  The key is in the translation from chunks to samples.
  Earlier, I said that the number of samples in a chunk is determined from
  the size of the chunk in bytes, since samples are a fixed size.
  But what happens if the chunk size is not evenly divisible by the
  sample size? Well, DirectShow, the engine behind Windows Media Player
  and a number of third-party video players that run on Windows, rounds up.
  This means that if you set nBlockAlign to be higher than the size of any
  chunk in the stream, DirectShow will regard all of them as holding one sample,
  even though they are all different sizes. Thus, to encode VBR MP3,
  you simply have to set nBlockAlign to at least 960, the maximum frame size
  for MPEG layer III, and then store each MPEG audio frame in its own chunk.
  Since each audio frame encodes a constant amount of audio data
  -- 1152 samples for 32KHz or higher, 576 samples for 24Khz or lower --
  this permits proper timing and seeking despite the variable bitrate.
  This can also be done for other compressed audio formats, provided that
  the encoding application is able to determine the compressed block
  boundaries and the maximum block size, and the decoders accept non-standard
  values for the nBlockAlign header field.

  AVIFile incompatibility. The Microsoft AVIFile APIs in the Video for Windows API
  round down instead of up when computing samples-per-chunk, so any AVIFile-based
  program will see zero samples in a VBR audio stream and thus be unable to read
  the audio data. Despite its deprecated status, there are still a surprisingly
  high number of apps using the Video for Windows API, some of which are even new.
  Unfortunately, it is not always apparent whether an application is Video for Windows
  or DirectShow based.

Typical 48000 Hz CBR Mp3:
  StreamHdr
	- dwSampleSize = 1
    - dwScale = 1
    - dwRate = 48000
  WAVEFORMATEX
    - nBlockAlign = 1
	- nSamplesPerSec = 48000
	- nChannels = 2

Typical 48000 Hz VBR Mp3:
  StreamHdr
	- dwSampleSize = 0
    - dwScale = 1152
    - dwRate = 48000
  WAVEFORMATEX
    - nBlockAlign = 1152
	- nSamplesPerSec = 48000
	- nChannels = 2

Typical 48000 Hz CBR AC3:
  StreamHdr
	- dwSampleSize = 1
    - dwScale = 1
    - dwRate = 48000
  WAVEFORMATEX
    - nBlockAlign = 1
	- nSamplesPerSec = 48000
	- nChannels = 5

Typical 48000 Hz VBR AC3: -> Never Seen!!!!!!!!!!!
  StreamHdr
	- dwSampleSize = ?
    - dwScale = ?
    - dwRate = ?
  WAVEFORMATEX
    - nBlockAlign = ?
	- nSamplesPerSec = ?
	- nChannels = ?

------------------------------------------------------------------------------------

Note:
	Fraunhofer-IIS's MP3 codec has a compression delay of 1373 samples at the start.
	I do not know if the decompressor removes this delay or not...have to test it!
	LPMPEGLAYER3WAVEFORMAT pMp3WaveFormat = (LPMPEGLAYER3WAVEFORMAT)m_pSrcFormat;
	DWORD dwInitialSamplesToThrow = pMp3WaveFormat->nCodecDelay;
*/

// Additional defines not found in mmreg.h
#define WAVE_FORMAT_AMR			0x00FE
#define WAVE_FORMAT_WMA8		0x0161
#define WAVE_FORMAT_INDEOAUDIO	0x0402
#define WAVE_FORMAT_DOLBY_AC3	0x2000 // Dolby AC3
#define WAVE_FORMAT_DVD_DTS		0x2001 // DTS
#define WAVE_FORMAT_14_4		0x2002
#define WAVE_FORMAT_28_8		0x2003
#define WAVE_FORMAT_COOK		0x2004
#define WAVE_FORMAT_DNET		0x2005
#define WAVE_FORMAT_AVIS		0x3313
#define	WAVE_FORMAT_VORBIS		0x566F
#define	WAVE_FORMAT_OGG1		0x674F // Ogg Vorbis 1
#define	WAVE_FORMAT_OGG2		0x6750 // Ogg Vorbis 2
#define	WAVE_FORMAT_OGG3		0x6751 // Ogg Vorbis 3
#define	WAVE_FORMAT_OGG1P		0x676F // Ogg Vorbis 1+
#define	WAVE_FORMAT_OGG2P		0x6770 // Ogg Vorbis 2+
#define	WAVE_FORMAT_OGG3P		0x6771 // Ogg Vorbis 3+
#define WAVE_FORMAT_GSMAMRCBR	0x7A21 // GSM-AMR (CBR, no SID)
#define WAVE_FORMAT_GSMAMRVBR	0x7A22 // GSM-AMR (VBR, including SID)
#define WAVE_FORMAT_FLAC		0xF1AC
#define WAVE_FORMAT_AAC1		0xAAC0
#define WAVE_FORMAT_AAC2		0x00FF
#define WAVE_FORMAT_AAC4		0x706D
#define WAVE_FORMAT_TTA			0x77A1
#if !defined(WAVE_FORMAT_EXTENSIBLE)
#define WAVE_FORMAT_EXTENSIBLE  0xFFFE
#endif // !defined(WAVE_FORMAT_EXTENSIBLE)

class CAVIPlay
{
	public:
	/*
	 * here's the general layout of an AVI riff file (new format)
	 * (see also aviriff.h)
	 *
	 * RIFF (3F??????) AVI       <- not more than 1 GB in size
	 *     LIST (size) hdrl
	 *         avih (0038)
	 *         LIST (size) strl
	 *             strh (0038)
	 *             strf (????)
	 *             indx (3ff8)   <- size may vary, should be sector sized
	 *         LIST (size) strl
	 *             strh (0038)
	 *             strf (????)
	 *             indx (3ff8)   <- size may vary, should be sector sized
	 *         LIST (size) odml
	 *             dmlh (????)
	 *         JUNK (size)       <- fill to align to sector - 12
	 *     LIST (7f??????) movi  <- aligned on sector - 12
	 *         00dc (size)       <- sector aligned
	 *         01wb (size)       <- sector aligned
	 *         ix00 (size)       <- sector aligned
	 *     idx1 (00??????)       <- sector aligned
	 * RIFF (7F??????) AVIX
	 *     JUNK (size)           <- fill to align to sector -12
	 *     LIST (size) movi
	 *         00dc (size)       <- sector aligned
	 * RIFF (7F??????) AVIX      <- not more than 2GB in size
	 *     JUNK (size)           <- fill to align to sector - 12
	 *     LIST (size) movi
	 *         00dc (size)       <- sector aligned
	 *
	 *-===================================================================*/

	// Error Codes
	enum {	UNEXPECTED_EOF,
			WRONG_LIST_TYPE,
			WRONG_CHUNK_TYPE,
			WRONG_AVIMAINHEADER_SIZE,
			WRONG_VIDEOSTREAM_HEADER,
			WRONG_AUDIOSTREAM_HEADER,
			WRONG_SUPERINDEX_HEADER,
			WRONG_STANDARDINDEX_HEADER,
			OUT_OF_MEM};

#ifndef FCC
	#define FCC(ch4) ((((DWORD)(ch4) & 0xFF) << 24) |     \
					  (((DWORD)(ch4) & 0xFF00) << 8) |    \
					  (((DWORD)(ch4) & 0xFF0000) >> 8) |  \
					  (((DWORD)(ch4) & 0xFF000000) >> 24))
#endif

	typedef struct _riffchunk {
	   FOURCC fcc;			// Chunk Type
	   DWORD  cb;			// Size without this header
	} RIFFCHUNK, * LPRIFFCHUNK;
	typedef struct _rifflist {
	   FOURCC fcc;			// FCC('RIFF') or FCC('LIST')
	   DWORD  cb;			// Size includes size of fccListType
	   FOURCC fccListType;	// List Type
	} RIFFLIST, * LPRIFFLIST;

	#define RIFFROUND(cb) ((cb) + ((cb)&1))

	// Main Header
	typedef struct _avimainhdr {
		DWORD  dwMicroSecPerFrame;     // frame display rate (or 0L)
		DWORD  dwMaxBytesPerSec;       // max. transfer rate
		DWORD  dwPaddingGranularity;   // pad to multiples of this size; normally 2K.
		DWORD  dwFlags;                // the ever-present flags
		#define AVIF_HASINDEX        0x00000010 // Index at end of file?
		#define AVIF_MUSTUSEINDEX    0x00000020
		#define AVIF_ISINTERLEAVED   0x00000100
		#define AVIF_TRUSTCKTYPE     0x00000800 // Use CKType to find key frames
		#define AVIF_WASCAPTUREFILE  0x00010000
		#define AVIF_COPYRIGHTED     0x00020000
		DWORD  dwTotalFrames;          // # frames in first movi list
		DWORD  dwInitialFrames;
		DWORD  dwStreams;
		DWORD  dwSuggestedBufferSize;
		DWORD  dwWidth;
		DWORD  dwHeight;
		DWORD  dwReserved[4];
	} AVIMAINHDR,  * LPAVIMAINHDR;

	// Stream Header
	typedef struct _avistreamhdr {
	   FOURCC fccType;      // stream type codes

	   #ifndef streamtypeVIDEO
	   #define streamtypeVIDEO FCC('vids')
	   #define streamtypeAUDIO FCC('auds')
	   #define streamtypeMIDI  FCC('mids')
	   #define streamtypeTEXT  FCC('txts')
	   #endif

	   FOURCC fccHandler;
	   DWORD  dwFlags;
	   #define AVISF_DISABLED          0x00000001
	   #define AVISF_VIDEO_PALCHANGES  0x00010000

	   WORD   wPriority;
	   WORD   wLanguage;
	   DWORD  dwInitialFrames;
	   DWORD  dwScale;
	   DWORD  dwRate;       // dwRate/dwScale is stream tick rate in ticks/sec
	   DWORD  dwStart;
	   DWORD  dwLength;
	   DWORD  dwSuggestedBufferSize;
	   DWORD  dwQuality;
	   DWORD  dwSampleSize;
	   struct {
		  short int left;
		  short int top;
		  short int right;
		  short int bottom;
		  }   rcFrame;
	} AVISTREAMHDR, * LPAVISTREAMHDR;

	// Legacy Index
	typedef struct _avioldindexentry {
		DWORD   dwChunkId;
		DWORD   dwFlags;
		#ifndef AVIIF_LIST
		#define AVIIF_LIST			0x00000001
		#endif
		#ifndef AVIIF_TWOCC
		#define AVIIF_TWOCC			0x00000002
		#endif
		#ifndef AVIIF_KEYFRAME
		#define AVIIF_KEYFRAME		0x00000010
		#endif
		#ifndef	AVIIF_CONTROLFRAME
		#define AVIIF_CONTROLFRAME  0x00000200	// This is a control frame
		#endif
		#ifndef AVIIF_NO_TIME
		#define AVIIF_NO_TIME		0x00000100
		#endif
		#ifndef AVIIF_COMPRESSOR
		#define AVIIF_COMPRESSOR	0x0FFF0000  // Bits For Compressor Use
		#endif
		DWORD   dwOffset;			// offset of riff chunk header for the data
		DWORD   dwSize;				// size of the data (excluding riff header size)
	} AVIOLDINDEXENTRY, * LPAVIOLDINDEXENTRY;

	// Index Defines
	#define AVI_INDEX_OF_INDEXES		0x00
	#define AVI_INDEX_OF_CHUNKS			0x01
	#define AVI_INDEX_DEFAULT			0x00
	#define AVI_INDEX_2FIELD			0x01

	// Super Index
	typedef struct _avisuperindexhdr {// 'ix##'
		WORD     wLongsPerEntry;    // ==4
		BYTE     bIndexSubType;     // ==AVI_INDEX_DEFAULT (frame index) or AVI_INDEX_2FIELD 
		BYTE     bIndexType;        // ==AVI_INDEX_OF_INDEXES
		DWORD    nEntriesInUse;     // offset of next unused entry in aIndex
		DWORD    dwChunkId;         // chunk ID of chunks being indexed, '##dc', '##db' or '##wb'
		DWORD    dwReserved[3];     // must be 0
	} AVISUPERINDEXHDR, * LPAVISUPERINDEXHDR;

	typedef struct _avisuperindexentry {
		DWORDLONG qwOffset;			 // 64 bit offset to sub index chunk
		DWORD    dwSize;			 // 32 bit size of sub index chunk
		DWORD    dwDuration;		 // time span of subindex chunk (in stream ticks)
	} AVISUPERINDEXENTRY, *LPAVISUPERINDEXENTRY;

	// Pack structures to DWORD boundaries,
	// otherwise before qwBaseOffset
	// and at the end a empty DWORD would
	// be introduced!!
#pragma pack(push, 4)

	// Standard Index
	typedef struct _avistdindexhdr {// 'ix##'
		WORD     wLongsPerEntry;    // ==2
		BYTE     bIndexSubType;     // ==AVI_INDEX_DEFAULT
		BYTE     bIndexType;        // ==AVI_INDEX_OF_CHUNKS
		DWORD    nEntriesInUse;     // offset of next unused entry in aIndex
		DWORD    dwChunkId;         // chunk ID of chunks being indexed, '##dc', '##db' or '##wb'
		DWORDLONG qwBaseOffset;     // base offset that all index entries are relative to
		DWORD    dwReserved_3;      // must be 0
	} AVISTDINDEXHDR, * LPAVISTDINDEXHDR;

	// Index to Fields (half frames)
	typedef struct _avifieldindexhdr {// 'ix##'
		WORD     wLongsPerEntry;    // ==3
		BYTE     bIndexSubType;     // ==AVI_INDEX_2FIELD
		BYTE     bIndexType;        // ==AVI_INDEX_OF_CHUNKS
		DWORD    nEntriesInUse;     // offset of next unused entry in aIndex
		DWORD    dwChunkId;         // chunk ID of chunks being indexed, '##dc' or '##db'
		DWORDLONG qwBaseOffset;     // base offset that all index entries are relative to
		DWORD    dwReserved3;       // must be 0
	} AVIFIELDINDEXHDR, * LPAVIFIELDINDEXHDR;
	
	// Restore Original Packing
#pragma pack(pop)

	typedef struct _avistdindexentry {
		DWORD dwOffset;				// 32 bit offset to data (points to data, not riff header)
		DWORD dwSize;				// 31 bit size of data (does not include size of riff header),
									// bit 31 is deltaframe bit
	} AVISTDINDEXENTRY, *LPAVISTDINDEXENTRY;

	typedef struct _avifieldindexentry {
		DWORD    dwOffset;			// 32 bit offset to data (points to data, not riff header)
		DWORD    dwSize;			// size of all fields
									// bit 31 set for NON-keyframes
		DWORD    dwOffsetField2;	// offset to second field
	} AVIFIELDINDEXENTRY, *LPAVIFIELDINDEXENTRY;

	// AVI Stream Class
	class CAVIStream
	{
		friend class CAVIPlay;

		public:
#if (_MSC_VER <= 1200)
			CAVIStream(DWORD dwStreamNum, CAVIPlay* pAVIPlay, CBigFile* pFile) :
#else
			CAVIStream(DWORD dwStreamNum, CAVIPlay* pAVIPlay, CFile* pFile) :
#endif
				m_dwStreamNum(dwStreamNum),
				m_pAVIPlay(pAVIPlay),
				m_pFile(pFile)
							{	memset(&m_Hdr, 0, sizeof(AVISTREAMHDR));
								m_pSrcFormat = NULL;
								m_dwSrcFormatSize = 0;
								memset(&m_SuperIndexHdr, 0, sizeof(AVISUPERINDEXHDR));
								m_pSuperIndexTable = NULL;
								m_pStdIndexHdrs = NULL;
								m_ppStdIndexTables = NULL;
								m_pOldIndexTable = NULL;
								m_llOldIndexBase = 0;
								m_dwOldIndexEntries = 0;
								m_dwChunksCount = 0;
								m_llBytesCount = 0;
								m_bHasDecompressor = false;
								m_dwMinChunkSize = 0xFFFFFFFF;
								m_dwMaxChunkSize = 0;
								memset(&m_FirstMoviChunk, 0, sizeof(RIFFCHUNK));
								m_llFirstMoviChunkOffset = 0;
								m_LastReadMoviChunk = m_FirstMoviChunk;
								m_llLastReadMoviChunkOffset = 0;
								m_dwLastReadMoviChunkNum = 0;};
			virtual ~CAVIStream() {Free();};
			virtual void Free();
			__forceinline AVISTREAMHDR* GetHdr() {return &m_Hdr;};
			__forceinline DWORD GetRate() const {return m_Hdr.dwRate;};
			__forceinline DWORD GetScale() const {return m_Hdr.dwScale;};
			__forceinline DWORD GetStart() const {return m_Hdr.dwStart;};
			__forceinline DWORD GetTotalChunks() const {return m_dwChunksCount;};
			__forceinline LONGLONG GetTotalBytes() const {return m_llBytesCount;};
			bool HasDecompressor() const {return m_bHasDecompressor;};
			virtual CString GetStreamName() const = 0;
			virtual int GetStreamType() const = 0;

			// As input param if *pSize is not 0 it indicates the buffer size, and as output it indicates the chunk size
			// If pData is NULL the chunk size is returned
			bool GetChunkData(DWORD dwChunkNum, LPBYTE pData, DWORD* pSize);

			// Get the chunk statistics
			__forceinline DWORD GetMinChunkSize()
			{
				if (m_dwMinChunkSize == 0xFFFFFFFF)
					CalcMinChunkSize();
				return m_dwMinChunkSize;
			};
			__forceinline DWORD GetMaxChunkSize()
			{
				if (m_dwMaxChunkSize == 0)
					CalcMaxChunkSize();
				return m_dwMaxChunkSize;
			};

		protected:
			void CalcMinChunkSize();
			void CalcMaxChunkSize();
#if (_MSC_VER <= 1200)
			CBigFile*			m_pFile;
#else
			CFile*				m_pFile;
#endif
			CAVIPlay*			m_pAVIPlay;
			AVISTREAMHDR		m_Hdr;
			LPBYTE				m_pSrcFormat;
			volatile DWORD		m_dwSrcFormatSize;
			AVISUPERINDEXHDR	m_SuperIndexHdr;
			AVISUPERINDEXENTRY*	m_pSuperIndexTable;
			AVISTDINDEXHDR*		m_pStdIndexHdrs;
			AVISTDINDEXENTRY**	m_ppStdIndexTables;
			AVIOLDINDEXENTRY*	m_pOldIndexTable;
			volatile LONGLONG	m_llOldIndexBase;
			volatile DWORD		m_dwOldIndexEntries;
			volatile DWORD		m_dwChunksCount;
			volatile LONGLONG	m_llBytesCount;
			volatile DWORD		m_dwStreamNum;
			volatile bool		m_bHasDecompressor;
			volatile DWORD		m_dwMinChunkSize;
			volatile DWORD		m_dwMaxChunkSize;

			// If no index
			RIFFCHUNK m_FirstMoviChunk;
			LONGLONG m_llFirstMoviChunkOffset;
			RIFFCHUNK m_LastReadMoviChunk;
			LONGLONG m_llLastReadMoviChunkOffset;
			DWORD m_dwLastReadMoviChunkNum;
	};

	// AVI Audio Stream Class
	class CAVIAudioStream : public CAVIStream
	{
		friend class CAVIPlay;

		public:
			// Constructor
#if (_MSC_VER <= 1200)
			CAVIAudioStream(DWORD dwStreamNum, CAVIPlay* pAVIPlay, CBigFile* pFile) :
#else
			CAVIAudioStream(DWORD dwStreamNum, CAVIPlay* pAVIPlay, CFile* pFile) :
#endif
				CAVIStream(dwStreamNum, pAVIPlay, pFile)
								{	m_dwDstMinBytesCount = 0;
									m_dwSrcMinBytesCount = 0;
									m_pUncompressedWaveFormat = NULL;
									m_pSrcBuf = NULL;
									m_pDstBuf = NULL;
									m_dwSrcBufSize = 0;
									m_dwDstBufSize = 0;
									m_hAcmStream = NULL;
									m_bVBR = false;
									m_nVBRSamplesPerChunk = 0;
									m_dwNextChunk = 0;
									m_nCurrentChunk = -1;
									m_llCurrentSample = -1;
									m_nCurrentChunksCount = 0;
									m_dwDstBufSizeUsed = 0;
									m_dwDstBufOffset = 0;
									m_bFirstConversion = true;
									m_dwSrcBufUnconvertedBytesCount = 0;
									memset(&m_MpegAudioFrameHdr, 0, sizeof(MpegAudioFrameHdr));
									m_pCodec = NULL;
									m_pCodecCtx = NULL;
									m_pParser = NULL;
								}; 
			
			// Destructor						
			virtual ~CAVIAudioStream() {Free();};
			
			// Open
			bool OpenDecompression();
		
			// Free
			virtual void Free();

			// Get Data Functions
			bool GetSamples(LONGLONG llSampleNum);
			bool GetChunksSamples(DWORD dwChunkNum);
			bool GetNextChunksSamples();
			__forceinline DWORD GetBufSize() const {return m_dwDstBufSize;};
			__forceinline LPBYTE GetBufData() const	{return (m_pDstBuf + m_dwDstBufOffset);};
			__forceinline DWORD GetBufBytesCount() const{return ((m_dwDstBufSizeUsed >= m_dwDstBufOffset) ?
														(m_dwDstBufSizeUsed - m_dwDstBufOffset) : 0);};
			__forceinline DWORD GetBufSamplesCount() const;
			__forceinline int GetBufChunksCount() const {return m_nCurrentChunksCount;};
			__forceinline int GetCurrentChunkPos() const {return m_nCurrentChunk;};

			// Position Functions
			__forceinline bool Rew() {return GetChunksSamples(0);};
			__forceinline bool IsStart() const {return (m_dwNextChunk == 0);};
			__forceinline bool IsEnd() const {return (m_llCurrentSample >= GetTotalSamples());};
			__forceinline void SetStart() {m_dwNextChunk = 0;};
			__forceinline LONGLONG GetCurrentSamplePos() const {return m_llCurrentSample;};
			
			// Helper for interleaving
			DWORD SampleToChunk(LONGLONG llSampleNum);
			
			// Wave Format Functions,
			// bSource = true	-> Original Format
			// bSource = false	-> Uncompressed Format (for compressed streams)
			__forceinline LPWAVEFORMATEX GetFormat(bool bSource) const {	return (bSource ?
																					(LPWAVEFORMATEX)m_pSrcFormat :
																					(m_pUncompressedWaveFormat ?
																					m_pUncompressedWaveFormat :
																					(LPWAVEFORMATEX)m_pSrcFormat));};
			__forceinline DWORD GetFormatSize(bool bSource) const {			return (bSource ?
																					m_dwSrcFormatSize :
																					(m_pUncompressedWaveFormat ?
																					sizeof(WAVEFORMATEX) :
																					m_dwSrcFormatSize));};
			WORD GetFormatTag(bool bSource) const;
			WORD GetChannels(bool bSource) const;
			DWORD GetSampleRate(bool bSource) const;
			DWORD GetBytesPerSeconds(bool bSource) const;
			DWORD CalcVBRBytesPerSeconds() const;
			WORD GetSampleSize(bool bSource) const;
			WORD GetBits(bool bSource) const;

			// Statistics
			LONGLONG GetTotalSamples() const;
			double GetTotalTime() const; // In Seconds

			// MPEG Audio
			__forceinline bool IsVBR() const {return m_bVBR;};
			__forceinline int GetVBRSamplesPerChunk() const {return m_nVBRSamplesPerChunk;};
			__forceinline int GetMpegAudioLayer() const {return m_MpegAudioFrameHdr.lay;};

			// Stream Name and Type
			CString GetStreamName() const {	CString s;
											s.Format(_T("Audio Stream %u"), m_dwStreamNum);
											return s;};
			int GetStreamType() const {return 1;};

			// ACM Codec Name
			CString GetACMDecompressorShortName();
			CString GetACMDecompressorLongName();

			// ACM
			__forceinline bool IsUsingACM() {return m_hAcmStream != NULL;};

			// AV Codec
			__forceinline bool IsUsingAVCodec() {return m_pCodecCtx != NULL;};
			__forceinline AVCodec* GetAVCodec() {return m_pCodec;};
			__forceinline AVCodecContext* GetAVCodecCtx() {return m_pCodecCtx;};
			static enum CodecID AVCodecFormatTagToCodecID(WORD wFormatTag, int nPcmBits = 16);

		protected:
			bool OpenDecompressionACM();
			bool ReOpenDecompressACM();
			bool CalcMinBytesCount();
			bool SetIsVBR();
			bool ByteToChunk(LONGLONG* pByteNum, DWORD* pChunkNum);
			bool ChunkToByte(DWORD dwChunkNum, LONGLONG* pByteNum);
			bool VBRSampleToChunk(LONGLONG* pSample, DWORD* pChunkNum);
			bool VBRChunkToSample(DWORD dwChunkNum, LONGLONG* pSampleNum);
			bool CBRSampleToByte(LONGLONG llSampleNum, LONGLONG* pByteNum);
			bool CBRByteToSample(LONGLONG* pByteNum, LONGLONG* pSampleNum);

			LPBYTE m_pSrcBuf;
			volatile DWORD m_dwSrcBufSize;
			volatile DWORD m_dwSrcBufUnconvertedBytesCount;

			LPBYTE m_pDstBuf;
			volatile DWORD m_dwDstBufSize;
			volatile DWORD m_dwDstBufSizeUsed;
			volatile DWORD m_dwDstBufOffset;

			volatile DWORD m_dwNextChunk;
			volatile int m_nCurrentChunk;
			volatile int m_nCurrentChunksCount;
			volatile LONGLONG m_llCurrentSample;
			volatile bool m_bFirstConversion;

			// This sets a minimum for the GetChunksSamples() and
			// the GetNextChunksSamples() functions in returning
			// uncompressed bytes.
			volatile DWORD m_dwDstMinBytesCount;	// For the Output PCM Data

			// Calculated from m_dwDstMinBytesCount through acmStreamSize
			volatile DWORD m_dwSrcMinBytesCount;
			
			// Wave Format
			LPWAVEFORMATEX m_pUncompressedWaveFormat;

			// ACM Vars
			HACMSTREAM m_hAcmStream;
			ACMSTREAMHEADER m_AcmStreamHeader;
			
			// VBR MPEG Audio
			volatile bool m_bVBR;
			volatile int m_nVBRSamplesPerChunk;

			// MPEG Header Definitions - Mode Values
			enum MPegModeValues
			{
				MPEG_MD_STEREO = 0,
				MPEG_MD_JOINT_STEREO,
				MPEG_MD_DUAL_CHANNEL,
				MPEG_MD_MONO
			};

			// MPEG Audio Header
			typedef struct {
				int version;
				int lay;				// 1 = Layer I, 2 = Layer II, 3 = Layer III
				int error_protection;	// 0 = has error protection, 1 = no error protection
				int bitrate_index;
				int sampling_frequency;
				int padding;
				int extension;
				int mode;
				int mode_ext;
				int copyright;
				int original;
				int emphasis;
			} MpegAudioFrameHdr;

			// MPEG Audio Version
			enum MPegAudioVersion
			{
				MPEG25 = 0,
				MPEGReserved,
				MPEG2,
				MPEG1		
			};

			// MPEG Audio Header Var
			MpegAudioFrameHdr m_MpegAudioFrameHdr;

			// MPEG Audio Tables
			static const int m_nFreq[4][3];
			static const int m_nSamplesPerFrames[2][3];
			static const int m_nCoefficients[2][3];
			static const int m_nBitrate[2][3][15];
			static const int m_nSlotSizes[3];

			// Get MPEG Audio Header
			int GetMpegAudioFrameHdr(	LPBYTE pBuf, int* pSize,
										int nSearchForLayer = 0, // If 0 search all layers otherwise consider only Layer 1,2 or 3
										MpegAudioFrameHdr* pHdrFound = NULL,
										MpegAudioFrameHdr* pHdrCompare = NULL);

			// AVCodec Vars
			AVCodec* m_pCodec;
			AVCodecContext* m_pCodecCtx;
			AVCodecParserContext* m_pParser;

			// AVCodec Functions
			bool OpenDecompressionAVCodec();
			void FreeAVCodec();
	};

	// AVI Palette Stream Class
	class CAVIPaletteStream : public CAVIStream
	{
		friend class CAVIPlay;

		public:
			// Constructor
#if (_MSC_VER <= 1200)
			CAVIPaletteStream(DWORD dwStreamNum, CAVIPlay* pAVIPlay, CBigFile* pFile) :
#else
			CAVIPaletteStream(DWORD dwStreamNum, CAVIPlay* pAVIPlay, CFile* pFile) :
#endif
				CAVIStream(dwStreamNum, pAVIPlay, pFile)
								{m_pChangePaletteTable = NULL;
								m_dwChangePaletteTableSize = 0;}; 
			
			// Destructor						
			virtual ~CAVIPaletteStream()	{
												if (m_pChangePaletteTable)
													delete [] m_pChangePaletteTable;
											};

			// Stream Name and Type
			CString GetStreamName() const {	CString s;
											s.Format(_T("Palette Stream %u"), m_dwStreamNum);
											return s;};
			int GetStreamType() const {return 2;};

			// Do Change Palette?
			__forceinline bool DoChangePalette(DWORD dwChunkNum) const {return dwChunkNum < m_dwChangePaletteTableSize ?
																		m_pChangePaletteTable[dwChunkNum] >= 0 :
																		false;};

			// Get Palette, as input pass the buffer size, as output it will return the effective palette size
			bool GetPalette(DWORD dwChunkNum, LPBYTE pData, DWORD* pSize);

		protected:
			__forceinline void RGBA2BGRA(void* palette, int entries);
			int* m_pChangePaletteTable;
			DWORD m_dwChangePaletteTableSize;
	};

	// AVI Video Stream Class
	class CAVIVideoStream : public CAVIStream
	{
		friend class CAVIPlay;

		public:
			// Constructor
#if (_MSC_VER <= 1200)
			CAVIVideoStream(DWORD dwStreamNum, CAVIPlay* pAVIPlay, CBigFile* pFile) :
#else
			CAVIVideoStream(DWORD dwStreamNum, CAVIPlay* pAVIPlay, CFile* pFile) :
#endif
				CAVIStream(dwStreamNum, pAVIPlay, pFile)		
								{	m_pPaletteStream = NULL;
									m_pSrcBuf = NULL;
									m_dwSrcBufSize = 0;
									m_pOrigSrcBMI = NULL;
									m_dwOrigSrcBMISize = 0;
									m_pDstBMI = NULL;
									m_pDstBuf = NULL;
									m_dwDstBufSize = 0;
									m_dwDstFormatSize = 0;
									m_hIC = NULL;
									::memset(&m_IcInfo, 0, sizeof(ICINFO));
									m_dwNextFrame = 0;
									m_dwKeyFramesCount = 0;
									m_bNoDecompression = false;
									m_bYuvToRgb32 = false;
									m_nLastDecompressedDibFrame = -2;
									m_nLastDecompressedDxDrawFrame = -2;
									m_bNoBitCountChangeVCM = false;
									m_bForceRgb = false;
									m_pPrevRLEDib = NULL;
									m_pCurrentRLEDib = NULL;
									m_nOneFrameDelay = 0;
									m_pCodec = NULL;
									m_pCodecCtx = NULL;
									m_pFrame = NULL;
									m_pFrameGdi = NULL;
									m_pFrameDxDraw = NULL;
									m_dwPrevFourCCDxDraw = 0;
									m_nPrevPitchDxDraw = 0;
									m_nPrevBppDxDraw = 0;
									m_pPrevSurfaceDxDraw = NULL;
									m_bDecodeExtraData = false;
									memset(&m_Palette, 0, sizeof(AVPaletteControl));
									m_pImgConvertCtxGdi = NULL;
									m_pImgConvertCtxDxDraw = NULL;
#ifdef SUPPORT_LIBSWSCALE
									m_pFilterGdi = NULL;
									m_pFilterDxDraw = NULL;
#endif
								}; 
			
			// Destructor						
			virtual ~CAVIVideoStream() {Free();
										if (m_pOrigSrcBMI)
											delete [] m_pOrigSrcBMI;
										};
			
			// Open
			bool OpenDecompression(bool bForceRgb);
		
			// Free
			virtual void Free();

			// VCM
			__forceinline bool IsUsingVCM() {return m_hIC != NULL;};

			// AV Codec
			__forceinline bool IsUsingAVCodec() {return m_pCodecCtx != NULL;};
			__forceinline AVCodec* GetAVCodec() {return m_pCodec;};
			__forceinline AVCodecContext* GetAVCodecCtx() {return m_pCodecCtx;};
			static __forceinline enum PixelFormat AVCodecBMIToPixFormat(LPBITMAPINFO pBMI);
			static __forceinline enum PixelFormat AVCodecDxDrawToPixFormat(CDxDraw* pDxDraw);
			static __forceinline enum CodecID AVCodecFourCCToCodecID(DWORD dwFourCC);

			// Width & Height: some corrupted Avis have different values for width & height...
			// Priority of choice: 1. Format, 2. rcFrame, 3. Main Hdr
			__forceinline DWORD GetWidth() const {return (	m_pSrcFormat ?
															((LPBITMAPINFOHEADER)m_pSrcFormat)->biWidth :
															(m_Hdr.rcFrame.right - m_Hdr.rcFrame.left) > 0 ?
															(m_Hdr.rcFrame.right - m_Hdr.rcFrame.left) :
															m_pAVIPlay->GetMainHdr()->dwWidth);};
			__forceinline DWORD GetHeight() const {return (	m_pSrcFormat ?
															((LPBITMAPINFOHEADER)m_pSrcFormat)->biHeight :
															(m_Hdr.rcFrame.bottom - m_Hdr.rcFrame.top) > 0 ?
															(m_Hdr.rcFrame.bottom - m_Hdr.rcFrame.top) :
															m_pAVIPlay->GetMainHdr()->dwHeight);};

			// Frame Rate
			__forceinline double GetFrameRate() const {return	((m_Hdr.dwScale > 0) ?
																((double)m_Hdr.dwRate / (double)m_Hdr.dwScale) :
																0.0);};
			__forceinline bool GetFrameRate(DWORD* pdwRate, DWORD* pdwScale) const
			{
				if (pdwRate)
					*pdwRate = m_Hdr.dwRate;
				if (pdwScale)
					*pdwScale = m_Hdr.dwScale;
				return (pdwRate || pdwScale);
			};
			void SetFrameRate(DWORD dwRate, DWORD dwScale);

			// Key Frames
			__forceinline bool IsKeyFrame(int nFrame)
			{
				if (nFrame < 0)
					return false;
				else if (nFrame >= (int)GetTotalFrames())
					return false;
				if (m_pOldIndexTable)
					return ((m_pOldIndexTable[nFrame].dwFlags & AVIIF_KEYFRAME) == AVIIF_KEYFRAME);
				else if (m_pStdIndexHdrs && m_ppStdIndexTables)
				{
					// Find the Right Index
					DWORD dwEntries = 0;
					int index = 0;
					for (index = 0 ; index < (int)m_SuperIndexHdr.nEntriesInUse ; index++)
					{
						dwEntries += (DWORD)(m_pStdIndexHdrs[index].nEntriesInUse);
						if ((DWORD)nFrame < dwEntries)
							break;
					}

					// Find the Right Position
					for (int i = 0 ; i < index ; i++)
						nFrame -= (int)(m_pStdIndexHdrs[i].nEntriesInUse);
					
					// Bit 31 is the delta frame flag, which is the opposite of a key frame
					return ((m_ppStdIndexTables[index][nFrame].dwSize & 0x80000000) != 0x80000000);
				}
				else
					return true;
			};
			__forceinline int GetPrevKeyFrame(int nFrame)
			{
				do
				{
					if (IsKeyFrame(nFrame))
						return nFrame;	
				}
				while (--nFrame >= 0);

				// Some Brocken Avis do not set the Frame 0 as Key-Frame!
				return 0;
			};
			__forceinline int GetNextKeyFrame(int nFrame)
			{
				do
				{
					if (IsKeyFrame(nFrame))
						return nFrame;	
				}
				while (++nFrame < (int)GetTotalFrames());

				return -1;
			};
			__forceinline DWORD GetTotalKeyFrames() const {return m_dwKeyFramesCount;};

			// Get Data Functions
			bool SkipFrame(int nNumOfFrames = 1, BOOL bForceDecompress = FALSE); // Skip the given amount of frames
			bool GetFrame(CDib* pDib);
			bool GetFrameAt(CDib* pDib, DWORD dwFrame);
			bool GetFrame(CDxDraw* pDxDraw, CRect rc);
			bool GetFrameAt(CDxDraw* pDxDraw, DWORD dwFrame, CRect rc);

			// Position Functions
			__forceinline void Rew() {	m_dwNextFrame = 0;
										m_nLastDecompressedDibFrame = -2;
										m_nLastDecompressedDxDrawFrame = -2;};
			__forceinline int GetCurrentFramePos() const {return (int)m_dwNextFrame - 1 - m_nOneFrameDelay;};
			__forceinline int GetNextFramePos() const {return (int)m_dwNextFrame - m_nOneFrameDelay;};
			bool SetCurrentFramePos(int nCurrentFramePos);

			// Get Video Format
			// bSource = true	-> Original Format
			// bSource = false	-> Uncompressed Format (for compressed streams)
			__forceinline LPBITMAPINFO GetFormat(bool bSource) const {	return	(bSource ?
																				(LPBITMAPINFO)m_pSrcFormat :
																				(m_pDstBMI ?
																				m_pDstBMI :
																				(LPBITMAPINFO)m_pSrcFormat));};
			__forceinline DWORD GetFormatSize(bool bSource) const {		return (bSource ?
																				m_dwSrcFormatSize :
																				(m_pDstBMI ?
																				m_dwDstFormatSize :
																				m_dwSrcFormatSize));};

			// Codec & FourCC
			__forceinline bool IsRgb() const {return	m_pSrcFormat ?
														(((LPBITMAPINFOHEADER)m_pSrcFormat)->biCompression == BI_RGB ||
														((LPBITMAPINFOHEADER)m_pSrcFormat)->biCompression == BI_BITFIELDS) :
														false;};
			__forceinline bool IsYuv() const {return	m_pSrcFormat ?
														::IsSupportedYuvToRgbFormat(((LPBITMAPINFOHEADER)m_pSrcFormat)->biCompression) :
														false;};
			__forceinline DWORD GetFourCC(bool bSource) const {	if (bSource)
																		return GetFormat(true) ? GetFormat(true)->bmiHeader.biCompression : 0;
																	else
																		return GetFormat(false) ? GetFormat(false)->bmiHeader.biCompression : 0;
																};
			CString GetFourCCString(bool bSource) const {return CAVIPlay::FourCCToString(GetFourCC(bSource));};
			bool IsYUV420Out(DWORD dwFourCC);
			CString GetDecompressorName() const {return CString(m_IcInfo.szName);};
			CString GetDecompressorDescription() const {return CString(m_IcInfo.szDescription);};
			CString GetDecompressorDriver() const {return CString(m_IcInfo.szDriver);};

			// Statistics
			__forceinline DWORD GetTotalFrames() const {return m_dwChunksCount;};
			double GetTotalTime() const {return ((GetFrameRate() > 0.0) ?	// In Seconds
												((double)GetTotalFrames() / GetFrameRate()) :
												0.0);};

			// Stream Name and Type
			CString GetStreamName() const {	CString s;
											s.Format(_T("Video Stream %u"), m_dwStreamNum);
											return s;};
			int GetStreamType() const {return 0;};

		protected:
			__forceinline bool SkipFrameHelper(BOOL bForceDecompress);	// Do not use it directly, has no CS!
			bool GetFrameAtDirect(CDib* pDib, DWORD dwFrame);
			bool GetFrameAtDirect(CDxDraw* pDxDraw, DWORD dwFrame, CRect rc);
			bool GetUncompressedFrameAt(CDib* pDib, DWORD dwFrame,					// Direct decompress: RGB, RLE4, RLE8, YV12, YUY2
										volatile int& nLastDecompressedFrame);
			bool GetUncompressedFrameAt(CDxDraw* pDxDraw, DWORD dwFrame, CRect rc);	// Direct decompress: RGB, RLE4, RLE8, YV12, YUY2
			bool GetYUVFrameAt(CDib* pDib, DWORD dwFrame);							// Internal YUV -> RGB
			bool GetYUVFrameAt(CDxDraw* pDxDraw, DWORD dwFrame, CRect rc);			// Internal YUV -> RGB
			__forceinline bool IsRLE(DWORD dwFourCC) {return	(dwFourCC == BI_RLE4		||
																dwFourCC == FCC('rle4')		||
																dwFourCC == FCC('RLE4')		||
																dwFourCC == BI_RLE8			||
																dwFourCC == FCC('rle8')		||
																dwFourCC == FCC('RLE8')		||
																dwFourCC == FCC('mrle')		||
																dwFourCC == FCC('MRLE'))	?
																true : false;};
			__forceinline void UpdatePalette(DWORD dwFrame);
			void SetPaletteStreamPointer(CAVIPaletteStream* pPaletteStream) {m_pPaletteStream = pPaletteStream;};
			bool ReOpenDecompressVCM();
			bool FindDecompressorVCM();
			bool ChangeGetFrameBitsVCM(WORD wNewBitCount);
			static __forceinline int GetStride(LPBITMAPINFOHEADER pBMIH);
			bool PrepareWantedDstBMI(LPBITMAPINFOHEADER pDstBMIH);
			bool IsSrcFormatSupported();
			bool IsDstFormatSupported(LPBITMAPINFO pDstBMI);
			bool InitDstFormat(LPBITMAPINFO pDstBMI = NULL);
			LPBITMAPINFO GetSuggestedFormat();
			static __forceinline void FreeSuggestedFormat(LPBITMAPINFO pSuggestedBMIH) {if (pSuggestedBMIH)
																							delete [] pSuggestedBMIH;}
			void ClearDstBuf();
			__forceinline bool AllocateDstDib(CDib* pDib);
			bool GetUncompressedDib(CDib* pDib);
			CAVIPaletteStream* m_pPaletteStream;
			LPBYTE m_pSrcBuf;
			volatile DWORD m_dwSrcBufSize;
			LPBITMAPINFO m_pOrigSrcBMI;
			DWORD m_dwOrigSrcBMISize;
			LPBITMAPINFO m_pDstBMI;
			LPBYTE m_pDstBuf;
			volatile DWORD m_dwDstBufSize;
			volatile DWORD m_dwDstFormatSize;
			volatile DWORD m_dwNextFrame;
			volatile bool m_bNoDecompression;
			volatile bool m_bYuvToRgb32;
			volatile int m_nLastDecompressedDibFrame;
			volatile int m_nLastDecompressedDxDrawFrame;
			volatile DWORD m_dwKeyFramesCount;
			volatile bool m_bNoBitCountChangeVCM;
			volatile bool m_bForceRgb;
			HIC m_hIC;
			ICINFO m_IcInfo;
			CDib* m_pPrevRLEDib;
			CDib* m_pCurrentRLEDib;
			int m_nOneFrameDelay;

			bool OpenDecompressionAVCodec();
			void FreeAVCodec(bool bNoClose = false);
			__forceinline bool AVCodecHandle8bpp(bool bVFlip);
			__forceinline bool AVCodecDecompressDib(bool bKeyFrame,
													bool bSkipFrame,
													bool bSeek);
			__forceinline bool AVCodecDecompressDxDraw(	bool bKeyFrame,
														bool bSkipFrame,
														bool bSeek,
														CDxDraw* pDxDraw,
														CRect rc);

			AVCodec* m_pCodec;
			AVCodecContext* m_pCodecCtx;
			AVFrame* m_pFrame;
			AVFrame* m_pFrameGdi;
			AVFrame* m_pFrameDxDraw;
			DWORD m_dwPrevFourCCDxDraw;
			int m_nPrevPitchDxDraw;
			int m_nPrevBppDxDraw;
			LPVOID m_pPrevSurfaceDxDraw;
			AVPaletteControl m_Palette;
			bool m_bDecodeExtraData;
			SwsContext* m_pImgConvertCtxGdi;
			SwsContext* m_pImgConvertCtxDxDraw;
#ifdef SUPPORT_LIBSWSCALE
			SwsFilter* m_pFilterGdi;
			SwsFilter* m_pFilterDxDraw;
#endif
	};

	typedef CArray<CAVIVideoStream*,CAVIVideoStream*> VIDEOSTREAMARRAY;
	typedef CArray<CAVIAudioStream*,CAVIAudioStream*> AUDIOSTREAMARRAY;
	typedef CArray<CAVIPaletteStream*,CAVIPaletteStream*> PALETTESTREAMARRAY;

public:

	// Constructors
	CAVIPlay(bool bShowMessageBoxOnError = true);	// Does nothing than init few vars
	CAVIPlay(LPCTSTR lpszFileName,					// This calls Open()
			bool bShowMessageBoxOnError = true);
	
	// Destructor
	virtual ~CAVIPlay() {Close(); ::DeleteCriticalSection(&m_csAVI);};

	// Audio Format Tags Table
	void InitWaveFormatTagTable();
	static CString GetWaveFormatTagString(WORD wFormatTag);

	// Open and Parse Avi File
	bool Open(	LPCTSTR szFileName,
				bool bOnlyHeaders,
				bool bShowMessageBoxOnError);
	bool Open(	LPCTSTR szFileName,
				bool bOnlyHeader = false);

	// Free & Close
	void Close();

	// Function used to give access to the file
	void CloseFile();

	// Avi File Name
	CString GetFileName() const	{return m_sFileName;};

	// Avi File Size
	ULARGE_INTEGER GetFileSize();

	// Get Min / Max Chunk Size over all streams
	DWORD GetMinChunkSize();
	DWORD GetMaxChunkSize();

	// Get Main Header
	AVIMAINHDR* GetMainHdr() {return &m_AviMainHdr;};

	// Streams
	CAVIVideoStream* GetVideoStream(int nStreamNum) const;
	CAVIAudioStream* GetAudioStream(int nStreamNum) const;
	CAVIPaletteStream* GetPaletteStream(int nStreamNum) const;

	// Has at least one Video Stream?
	bool HasVideo() const {	if (m_VideoStreams.GetSize() > 0)
							{
								CAVIStream* p = m_VideoStreams.GetAt(0);
								if (p)
									return (p->m_dwChunksCount > 0);
							}
							return false;};
	
	// Has at least one Audio Stream?
	bool HasAudio() const {	if (m_AudioStreams.GetSize() > 0)
							{
								CAVIStream* p = m_AudioStreams.GetAt(0);
								if (p)
									return (p->m_dwChunksCount > 0);
							}
							return false;};

	// Has a Changing Palette Stream?
	bool HasPaletteStream() const {	if (m_PaletteStreams.GetSize() > 0)
							{
								CAVIStream* p = m_PaletteStreams.GetAt(0);
								if (p)
									return (p->m_dwChunksCount > 0);
							}
							return false;};

	// Has at least one VBR Mp3 Audio Stream?
	bool HasVBRAudio() const {	for (int i = 0 ; i < m_AudioStreams.GetSize() ; i++)
								{
									CAVIAudioStream* p = GetAudioStream(i);
									if (p && p->IsVBR())
										return true;
								}
								return false;};

	// Streams Count
	DWORD GetVideoStreamsCount() const {return m_VideoStreams.GetSize();};
	DWORD GetAudioStreamsCount() const {return m_AudioStreams.GetSize();};
	DWORD GetPaletteStreamsCount() const {return m_PaletteStreams.GetSize();};

	// The ShowMessageBoxOnError Flag
	void SetShowMessageBoxOnError(bool bShowMessageBoxOnError) {m_bShowMessageBoxOnError =
																bShowMessageBoxOnError;};
	bool IsShowMessageBoxOnError() const {return m_bShowMessageBoxOnError;};

	// Some helpers
	static CString FourCCToString(DWORD dwFourCC);
	static CString FourCCToStringLowerCase(DWORD dwFourCC);
	static CString FourCCToStringUpperCase(DWORD dwFourCC);
	static DWORD StringToFourCC(CString sFourCC);
	static DWORD FourCCMakeLowerCase(DWORD dwFourCC);
	static DWORD FourCCMakeUpperCase(DWORD dwFourCC);

	// Change the Frame Rate of the given video stream
	static bool AviChangeVideoFrameRate(LPCTSTR szFileName,
										DWORD dwVideoStreamNum,
										DWORD dwRate,
										DWORD dwScale,
										bool bShowMessageBoxOnError);

	// Change the Frame Rate of the given video stream
	static bool AviChangeVideoFrameRate(LPCTSTR szFileName,
										DWORD dwVideoStreamNum,
										double dFrameRate,
										bool bShowMessageBoxOnError);

	// Change the Start Offset of the given video stream
	static bool AviChangeVideoStartOffset(	LPCTSTR szFileName,
											DWORD dwVideoStreamNum,
											DWORD dwStart,
											bool bShowMessageBoxOnError);

	// Change the Start Offset of the given audio stream
	static bool AviChangeAudioStartOffset(	LPCTSTR szFileName,
											DWORD dwAudioStreamNum,
											DWORD dwStart,
											bool bShowMessageBoxOnError);

	// Avi File Parsing
	static bool IsFourCCInArray(DWORD dwFourCC, DWORD* pFourCCs, DWORD nSize);
	static __forceinline int GetChunkType(DWORD fcc);
#if (_MSC_VER <= 1200)
	static __forceinline LONGLONG SeekToNextChunk(CBigFile* pFile, RIFFCHUNK& chunk);
	static __forceinline LONGLONG SeekToNextChunk(CBigFile* pFile, RIFFCHUNK& chunk, LONGLONG llNextListPos);
	static __forceinline LONGLONG SeekToNextChunkFromList(CBigFile* pFile, const RIFFLIST& list, RIFFCHUNK& chunk);
	static __forceinline LONGLONG SeekToNextList(CBigFile* pFile, RIFFLIST& list);
#else
	static __forceinline LONGLONG SeekToNextChunk(CFile* pFile, RIFFCHUNK& chunk);
	static __forceinline LONGLONG SeekToNextChunk(CFile* pFile, RIFFCHUNK& chunk, LONGLONG llNextListPos);
	static __forceinline LONGLONG SeekToNextChunkFromList(CFile* pFile, const RIFFLIST& list, RIFFCHUNK& chunk);
	static __forceinline LONGLONG SeekToNextList(CFile* pFile, RIFFLIST& list);
#endif

	// Use AVCodecs as first choice flag
	bool m_bAVCodecPriority;

	// Critical section for synchronizing accesses to avi file and codecs
	// (some audio and video codecs cannot be access at the same time)
	CRITICAL_SECTION m_csAVI;

protected:

	// No Index
	void InitNoIndex(const RIFFCHUNK& firstmovichunk, LONGLONG llFirstMoviChunkOffset);

	// Legacy Index
	bool ParseLegacyIndex(int nSize, LONGLONG llMoviAddr);

	// Clean-up
	void ClearStreamsArrays();

	// Vars
	CString m_sFileName;
	AVIMAINHDR m_AviMainHdr;
	VIDEOSTREAMARRAY m_VideoStreams;
	AUDIOSTREAMARRAY m_AudioStreams;
	PALETTESTREAMARRAY m_PaletteStreams;
	volatile UINT m_uiOpenFlags;
	bool m_bShowMessageBoxOnError; // Display MessageBox on Error
#if (_MSC_VER <= 1200)
	CBigFile* m_pFile;
#else
	CFile* m_pFile;
#endif
};

__forceinline DWORD CAVIPlay::CAVIAudioStream::GetBufSamplesCount() const
{
	if (!m_pSrcFormat)
		return 0;

	if (((LPWAVEFORMATEX)m_pSrcFormat)->wFormatTag == WAVE_FORMAT_PCM)
		return (GetBufBytesCount() / ((LPWAVEFORMATEX)m_pSrcFormat)->nBlockAlign);
	else if (m_pUncompressedWaveFormat) 
		return (GetBufBytesCount() / m_pUncompressedWaveFormat->nBlockAlign);
	else
		return 0;
}

__forceinline enum PixelFormat CAVIPlay::CAVIVideoStream::AVCodecBMIToPixFormat(LPBITMAPINFO pBMI)
{
	if (pBMI)
	{
		if (pBMI->bmiHeader.biCompression == FCC('YV12')			||
			pBMI->bmiHeader.biCompression == FCC('I420')			||
			pBMI->bmiHeader.biCompression == FCC('IYUV'))
			return PIX_FMT_YUV420P;	// For YV12 we have to invert the planes!
		else if (	pBMI->bmiHeader.biCompression == FCC('YUY2')	||
					pBMI->bmiHeader.biCompression == FCC('YUNV')	||
					pBMI->bmiHeader.biCompression == FCC('VYUY')	||
					pBMI->bmiHeader.biCompression == FCC('V422')	||
					pBMI->bmiHeader.biCompression == FCC('YUYV'))
			return PIX_FMT_YUYV422;
		else if (	pBMI->bmiHeader.biCompression == FCC('UYVY')	||
					pBMI->bmiHeader.biCompression == FCC('Y422')	||
					pBMI->bmiHeader.biCompression == FCC('UYNV'))
			return PIX_FMT_UYVY422;
		else if (	pBMI->bmiHeader.biCompression == FCC('YUV9')	||
					pBMI->bmiHeader.biCompression == FCC('YVU9'))
			return PIX_FMT_YUV410P;	// For YVU9 we have to invert the planes!
		else if (pBMI->bmiHeader.biCompression == FCC('Y41B'))
			return PIX_FMT_YUV411P;
		else if (	pBMI->bmiHeader.biCompression == FCC('YV16')	||
					pBMI->bmiHeader.biCompression == FCC('Y42B'))
			return PIX_FMT_YUV422P; // For YV16 we have to invert the planes!
		else if (	pBMI->bmiHeader.biCompression == FCC('  Y8')	||
					pBMI->bmiHeader.biCompression == FCC('Y800')	||
					pBMI->bmiHeader.biCompression == FCC('GREY'))
			return PIX_FMT_GRAY8;
		else if (pBMI->bmiHeader.biCompression == mmioFOURCC('R','G','B',15))
			return PIX_FMT_RGB555;
		else if (pBMI->bmiHeader.biCompression == mmioFOURCC('B','G','R',15))
			return PIX_FMT_BGR555;
		else if (pBMI->bmiHeader.biCompression == mmioFOURCC('R','G','B',16))
			return PIX_FMT_RGB565;
		else if (pBMI->bmiHeader.biCompression == mmioFOURCC('B','G','R',16))
			return PIX_FMT_BGR565;
		else if (pBMI->bmiHeader.biCompression == BI_RGB)
		{
			switch (pBMI->bmiHeader.biBitCount)
			{
				case 8  : return PIX_FMT_PAL8;
				case 16 : return PIX_FMT_RGB555;
				case 24 : return PIX_FMT_BGR24;
				case 32 : return PIX_FMT_RGB32;
				default : return PIX_FMT_NONE;
			}
		}
		else if (pBMI->bmiHeader.biCompression == BI_BITFIELDS)
		{
			switch (pBMI->bmiHeader.biBitCount)
			{
				case 16 :
				{
					LPBITMAPINFOBITFIELDS pBmiBf = (LPBITMAPINFOBITFIELDS)pBMI;
					if ((pBmiBf->biBlueMask == 0x001F)	&&
						(pBmiBf->biGreenMask == 0x07E0)	&&
						(pBmiBf->biRedMask == 0xF800))
						return PIX_FMT_RGB565;
					else
						return PIX_FMT_RGB555;
				}
				case 32 : return PIX_FMT_RGB32;
				default : return PIX_FMT_NONE;
			}
		}
		else
			return PIX_FMT_NONE;
	}
	else
		return PIX_FMT_NONE;
}

__forceinline enum PixelFormat CAVIPlay::CAVIVideoStream::AVCodecDxDrawToPixFormat(CDxDraw* pDxDraw)
{
	if (pDxDraw)
	{
		if (pDxDraw->GetCurrentSrcFourCC() == FCC('YV12'))
			return PIX_FMT_YUV420P;
		else if (pDxDraw->GetCurrentSrcFourCC() == FCC('YUY2'))
			return PIX_FMT_YUYV422;
		else if (pDxDraw->GetCurrentSrcFourCC() == BI_RGB)
		{
			switch (pDxDraw->GetCurrentSrcBpp())
			{
				case 8  : return PIX_FMT_RGB8;
				case 16 : return pDxDraw->IsCurrentSrcRgb15() ? PIX_FMT_RGB555 : PIX_FMT_RGB565;
				case 24 : return PIX_FMT_BGR24;
				case 32 : return PIX_FMT_RGB32;
				default : return PIX_FMT_RGB32;
			}
		}
		else
			return PIX_FMT_RGB32;
	}
	else
		return PIX_FMT_RGB32;
}

__forceinline enum CodecID CAVIPlay::CAVIVideoStream::AVCodecFourCCToCodecID(DWORD dwFourCC)
{
	if (FourCCToStringUpperCase(dwFourCC) == _T("XVID"))		// XVID
		return CODEC_ID_MPEG4;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("XVIX"))	// XVIX
		return CODEC_ID_MPEG4;

	else if (FourCCToStringUpperCase(dwFourCC) == _T("DIVX"))	// DIVX
		return CODEC_ID_MPEG4;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("MP4S"))	// MP4S
		return CODEC_ID_MPEG4;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("MP4V"))	// MP4V
		return CODEC_ID_MPEG4;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("M4S2"))	// M4S2
		return CODEC_ID_MPEG4;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("3IV1"))	// 3IV1
		return CODEC_ID_MPEG4;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("3IV2"))	// 3IV2
		return CODEC_ID_MPEG4;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("3IVX"))	// 3IVX
		return CODEC_ID_MPEG4;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("RMP4"))	// RMP4
		return CODEC_ID_MPEG4;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("DM4V"))	// DM4V
		return CODEC_ID_MPEG4;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("WV1F"))	// WV1F
		return CODEC_ID_MPEG4;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("FMP4"))	// FMP4
		return CODEC_ID_MPEG4;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("HDX4"))	// HDX4
		return CODEC_ID_MPEG4;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("SMP4"))	// SMP4
		return CODEC_ID_MPEG4;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("MP45"))	// MP45
		return CODEC_ID_MPEG4;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("UMP4"))	// UMP4
		return CODEC_ID_MPEG4;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("SEDG"))	// SEDG
		return CODEC_ID_MPEG4;
	else if (dwFourCC == mmioFOURCC(0x04, 0, 0, 0))				// Some broken avis
		return CODEC_ID_MPEG4;

	else if (FourCCToStringUpperCase(dwFourCC) == _T("FFDS"))	// FFDS
		return CODEC_ID_MPEG4;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("FVFW"))	// FVFW
		return CODEC_ID_MPEG4;

	else if (FourCCToStringUpperCase(dwFourCC) == _T("DX50"))	// DX50
		return CODEC_ID_MPEG4;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("BLZ0"))	// BLZ0
		return CODEC_ID_MPEG4;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("DXGM"))	// DXGM
		return CODEC_ID_MPEG4;

	else if (FourCCToStringUpperCase(dwFourCC) == _T("MP41"))	// MP41
		return CODEC_ID_MSMPEG4V1;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("MPG4"))	// MPG4
		return CODEC_ID_MSMPEG4V1;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("DIV1"))	// DIV1
		return CODEC_ID_MSMPEG4V1;

	else if (FourCCToStringUpperCase(dwFourCC) == _T("MP42"))	// MP42
		return CODEC_ID_MSMPEG4V2;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("DIV2"))	// DIV2
		return CODEC_ID_MSMPEG4V2;

	// Note CODEC_ID_MSMPEG4 = CODEC_ID_MSMPEG4V3
	else if (FourCCToStringUpperCase(dwFourCC) == _T("DIV3"))	// DIV3
		return CODEC_ID_MSMPEG4V3;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("DIV4"))	// DIV4
		return CODEC_ID_MSMPEG4V3;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("DIV5"))	// DIV5
		return CODEC_ID_MSMPEG4V3;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("DIV6"))	// DIV6
		return CODEC_ID_MSMPEG4V3;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("AP41"))	// AP41
		return CODEC_ID_MSMPEG4V3;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("COL0"))	// COL0
		return CODEC_ID_MSMPEG4V3;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("COL1"))	// COL1
		return CODEC_ID_MSMPEG4V3;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("DVX3"))	// DVX3
		return CODEC_ID_MSMPEG4V3;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("MPG3"))	// MPG3
		return CODEC_ID_MSMPEG4V3;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("MP43"))	// MP43
		return CODEC_ID_MSMPEG4V3;

	else if (FourCCToStringUpperCase(dwFourCC) == _T("SNOW"))	// SNOW
		return CODEC_ID_SNOW;

	else if (FourCCToStringUpperCase(dwFourCC) == _T("WMV1"))	// WMV1
		return CODEC_ID_WMV1;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("WMV2"))	// WMV2
		return CODEC_ID_WMV2;

	else if (FourCCToStringUpperCase(dwFourCC) == _T("FLV1"))	// FLV1
		return CODEC_ID_FLV1;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("VP30"))	// VP30
		return CODEC_ID_VP3;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("VP31"))	// VP31
		return CODEC_ID_VP3;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("THEO"))	// Theora
		return CODEC_ID_THEORA;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("VP50"))	// VP50
		return CODEC_ID_VP5;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("VP6F"))	// VP6F
		return CODEC_ID_VP6F;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("VP60"))	// VP60
		return CODEC_ID_VP6;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("VP61"))	// VP61
		return CODEC_ID_VP6;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("VP62"))	// VP62
		return CODEC_ID_VP6;

	else if (FourCCToStringUpperCase(dwFourCC) == _T("MJPG"))	// MJPG
		return CODEC_ID_MJPEG;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("IJPG"))	// IJPG
		return CODEC_ID_MJPEG;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("DMB1"))	// DMB1
		return CODEC_ID_MJPEG;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("JPGL"))	// JPGL
		return CODEC_ID_MJPEG;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("QIVG"))	// QIVG
		return CODEC_ID_MJPEG;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("SP54"))	// SP54
		return CODEC_ID_SP5X;

	else if (FourCCToStringUpperCase(dwFourCC) == _T("LJPG"))	// LJPG
		return CODEC_ID_LJPEG;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("MJLS"))	// MJLS
		return CODEC_ID_JPEGLS;

	else if (FourCCToStringUpperCase(dwFourCC) == _T("QPEG"))	// QPEG
		return CODEC_ID_QPEG;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("Q1.0"))	// QPEG
		return CODEC_ID_QPEG;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("Q1.1"))	// QPEG
		return CODEC_ID_QPEG;

	else if (FourCCToStringUpperCase(dwFourCC) == _T("DVC "))	// DV NTSC
		return CODEC_ID_DVVIDEO;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("DVCP"))	// DV PAL
		return CODEC_ID_DVVIDEO;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("DVPP"))	// DVCPRO PAL produced by FCP
		return CODEC_ID_DVVIDEO;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("DVH5"))	// DVCPRO HD 50i produced by FCP
		return CODEC_ID_DVVIDEO;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("DVH6"))	// DVCPRO HD 60i produced by FCP
		return CODEC_ID_DVVIDEO;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("DV5P"))	// DVCPRO50 PAL produced by FCP
		return CODEC_ID_DVVIDEO;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("DV5N"))	// DVCPRO50 NTSC produced by FCP
		return CODEC_ID_DVVIDEO;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("AVDV"))	// AVID DV
		return CODEC_ID_DVVIDEO;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("DVSD"))	// DVSD
		return CODEC_ID_DVVIDEO;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("DVHD"))	// DVHD
		return CODEC_ID_DVVIDEO;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("DVSL"))	// DVSL
		return CODEC_ID_DVVIDEO;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("DV25"))	// DV25
		return CODEC_ID_DVVIDEO;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("DV50"))	// DV50
		return CODEC_ID_DVVIDEO;

	else if (FourCCToStringUpperCase(dwFourCC) == _T("H261"))	// H261
		return CODEC_ID_H261;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("M261"))	// Microsoft M261
		return CODEC_ID_H261;

	else if (FourCCToStringUpperCase(dwFourCC) == _T("H263"))	// H263
		return CODEC_ID_H263;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("S263"))	// S263
		return CODEC_ID_H263;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("M263"))	// Microsoft M263
		return CODEC_ID_H263;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("U263"))	// U263
		return CODEC_ID_H263;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("VIV1"))	// VIV1
		return CODEC_ID_H263;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("I263"))	// Intel I263
		return CODEC_ID_H263I;

	else if (FourCCToStringUpperCase(dwFourCC) == _T("H264"))	// H264
		return CODEC_ID_H264;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("X264"))	// X264
		return CODEC_ID_H264;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("VSSH"))	// VSSH
		return CODEC_ID_H264;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("DAVC"))	// DAVC
		return CODEC_ID_H264;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("AVC1"))	// AVC1
		return CODEC_ID_H264;

	else if (FourCCToStringUpperCase(dwFourCC) == _T("VC-1"))	// SMPTE RP 2025
		return CODEC_ID_VC1;

	else if (FourCCToStringUpperCase(dwFourCC) == _T("HFYU"))	// HFYU
		return CODEC_ID_HUFFYUV;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("FFVH"))	// FFVH
		return CODEC_ID_FFVHUFF;

	else if (FourCCToStringUpperCase(dwFourCC) == _T("FFV1"))	// FFV1
		return CODEC_ID_FFV1;

	else if (dwFourCC == BI_RLE8)								// RLE8
		return CODEC_ID_MSRLE;
	else if (dwFourCC == BI_RLE4)								// RLE4
		return CODEC_ID_MSRLE;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("RLE "))	// RLE8 or RLE4
		return CODEC_ID_MSRLE;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("RLE8"))	// RLE8
		return CODEC_ID_MSRLE;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("RLE4"))	// RLE4
		return CODEC_ID_MSRLE;

	else if (FourCCToStringUpperCase(dwFourCC) == _T("MPNG"))	// MPNG
		return CODEC_ID_PNG;
	/*else if (FourCCToStringUpperCase(dwFourCC) == _T("PNG1"))	// PNG1
		return CODEC_ID_COREPNG;*/
	else if (FourCCToStringUpperCase(dwFourCC) == _T("ZLIB"))	// ZLIB
		return CODEC_ID_ZLIB;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("QRLE"))	// QRLE
		return CODEC_ID_QTRLE;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("TSCC"))	// TSCC
		return CODEC_ID_TSCC;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("CSCD"))	// CSCD
		return CODEC_ID_CSCD;
	
	else if (FourCCToStringUpperCase(dwFourCC) == _T("CVID"))	// Cinepak
		return CODEC_ID_CINEPAK;

	else if (FourCCToStringUpperCase(dwFourCC) == _T("CRAM"))	// MS Video 1
		return CODEC_ID_MSVIDEO1;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("MSVC"))	// MS Video 1
		return CODEC_ID_MSVIDEO1;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("WHAM"))	// MS Video 1
		return CODEC_ID_MSVIDEO1;

	else if (FourCCToStringUpperCase(dwFourCC) == _T("IV31"))	// IV31
		return CODEC_ID_INDEO3;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("IV32"))	// IV32
		return CODEC_ID_INDEO3;

	else if (FourCCToStringUpperCase(dwFourCC) == _T("MPEG"))	// MPEG
		return CODEC_ID_MPEG1VIDEO;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("MPG1"))	// MPG1
		return CODEC_ID_MPEG1VIDEO;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("PIM1"))	// PIM1
		return CODEC_ID_MPEG1VIDEO;
	else if (dwFourCC == 0x10000001)
		return CODEC_ID_MPEG1VIDEO;

	/* Implemented in YuvToRgb.cpp
	else if (FourCCToStringUpperCase(dwFourCC) == _T("VCR1"))	// VCR1
		return CODEC_ID_VCR1;*/

	else if (FourCCToStringUpperCase(dwFourCC) == _T("VCR2"))	// VCR2, only intra frame compression
		return CODEC_ID_MPEG2VIDEO;

	else if (FourCCToStringUpperCase(dwFourCC) == _T("MPG2"))	// MPG2
		return CODEC_ID_MPEG2VIDEO;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("HDV2"))	// MPEG2 produced by Sony HD camera
		return CODEC_ID_MPEG2VIDEO;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("HDV3"))	// HDV produced by FCP
		return CODEC_ID_MPEG2VIDEO;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("MX5N"))	// MPEG2 IMX NTSC 525/60 50mb/s produced by FCP
		return CODEC_ID_MPEG2VIDEO;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("MX5P"))	// MPEG2 IMX PAL 625/50 50mb/s produced by FCP
		return CODEC_ID_MPEG2VIDEO;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("MX3N"))	// MPEG2 IMX NTSC 525/60 30mb/s produced by FCP
		return CODEC_ID_MPEG2VIDEO;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("MX3P"))	// MPEG2 IMX PAL 625/50 30mb/s produced by FCP
		return CODEC_ID_MPEG2VIDEO;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("EM2V"))	// EM2V
		return CODEC_ID_MPEG2VIDEO;
	else if (FourCCToStringUpperCase(dwFourCC) == _T("DVR "))	// DVR
		return CODEC_ID_MPEG2VIDEO;
	else if (dwFourCC == 0x10000002)
		return CODEC_ID_MPEG2VIDEO;

	else
		return CODEC_ID_NONE;
}

#endif //!_INC_AVIPLAY