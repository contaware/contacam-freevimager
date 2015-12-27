#include "stdafx.h"
#include "AVRec.h"
#include "Round.h"
#include "Helpers.h"
#include "YuvToYuv.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef VIDEODEVICEDOC

// Defined in uImager.cpp
int avcodec_open_thread_safe(AVCodecContext *avctx, AVCodec *codec);
int avcodec_close_thread_safe(AVCodecContext *avctx);

// Ffmpeg libs
#pragma comment(lib, "ffmpeg\\libavcodec\\libavcodec.a")
#pragma comment(lib, "ffmpeg\\libavformat\\libavformat.a")
#pragma comment(lib, "ffmpeg\\libavutil\\libavutil.a")
#pragma comment(lib, "ffmpeg\\libswscale\\libswscale.a")
#pragma comment(lib, "ffmpeg\\libswresample\\libswresample.a")
// In 2014 I was happy to be able to build ffmpeg 2.2.1 with vs2010,
// but the result was extremelly slow: video codecs slower by a
// factor of 1.5 - 2 and swscale by a factor of 3 compared to mingw
#ifdef FFMPEG_TOOLCHAIN_MSVC
#pragma comment(lib, "ffmpeg\\msvc\\mp3lame.lib")
#else
// libcmt.lib(_pow_.obj) : error LNK2005: _pow already defined in libmingwex.a(pow.o)
// -> to correctly link we have to remove pow.o from libmingwex.a,
// perform the following in visual studio command prompt:
// 1. cd uimager\ffmpeg\lib
// 2. lib -remove:pow.o libmingwex.a
// 3. rename libmingwex.lib libmingwex.a
#pragma comment(lib, "ffmpeg\\mingw\\libgcc.a")
#pragma comment(lib, "ffmpeg\\mingw\\libmingwex.a")
#pragma comment(lib, "ffmpeg\\mingw\\libmp3lame.a")
#pragma comment(lib, "ffmpeg\\mingw\\libx264.a")
#endif

CAVRec::CAVRec()
{
	InitVars();
}

CAVRec::CAVRec(LPCTSTR lpszFileName, BOOL bMovFragmented)
{
	InitVars();
	Init(lpszFileName, bMovFragmented);
}

void CAVRec::InitVars()
{
	::InitializeCriticalSection(&m_csAV);

	m_pOutputFormat = NULL;
	m_pFormatCtx = NULL;
	m_sFileName = _T("");
	m_dwTotalVideoStreams = 0;
	m_dwTotalAudioStreams = 0;
	m_bFileOpened = false;
	m_bOpen = false;
	
#ifdef PERFORMANCE_MEASUREMENT
	m_ullAddFrameTimeMicroSec = 0;
	m_ullAddFrameCount = 0;
#endif

	for (DWORD dwStreamNum = 0 ; dwStreamNum < MAX_STREAMS ; dwStreamNum++)
	{
		// Conversion Contexts
		m_pImgConvertCtx[dwStreamNum] = NULL;
		m_pAudioConvertCtx[dwStreamNum] = NULL;

		// Buffers
		m_ppSrcBuf[dwStreamNum] = NULL;
		m_nSrcBufSize[dwStreamNum] = 0;
		m_ppDstBuf[dwStreamNum] = NULL;
		m_nDstBufSize[dwStreamNum] = 0;
		m_pAVPalette[dwStreamNum] = NULL;

		// Frames
		m_pFrame[dwStreamNum] = NULL;
		m_pFrameTemp[dwStreamNum] = NULL;
		
		// Wave Format
		m_pSrcWaveFormat[dwStreamNum] = NULL;
		
		// Reset counters
		m_llTotalFramesOrSamples[dwStreamNum] = 0;
		m_llTotalWrittenBytes[dwStreamNum] = 0;
		m_llLastCodecPTS[dwStreamNum] = AV_NOPTS_VALUE;
		m_llLastStreamDTS[dwStreamNum] = AV_NOPTS_VALUE;
	}
}

CAVRec::~CAVRec()
{
	Close();
	::DeleteCriticalSection(&m_csAV);
}

__forceinline void CAVRec::SetSrcWaveFormat(DWORD dwStreamNum, const LPWAVEFORMATEX pWaveFormat)					
{
	if (pWaveFormat == NULL)
		return;

	if (m_pSrcWaveFormat[dwStreamNum])
		delete [] m_pSrcWaveFormat[dwStreamNum];

	int nWaveFormatSize;
	if (pWaveFormat->wFormatTag == WAVE_FORMAT_PCM)
		nWaveFormatSize = sizeof(WAVEFORMATEX);
	else
		nWaveFormatSize = sizeof(WAVEFORMATEX) + pWaveFormat->cbSize;
 
	m_pSrcWaveFormat[dwStreamNum] = (LPWAVEFORMATEX)new BYTE[nWaveFormatSize];
	memcpy(m_pSrcWaveFormat[dwStreamNum], pWaveFormat, nWaveFormatSize);
}

bool CAVRec::Init(LPCTSTR lpszFileName, BOOL bMovFragmented)
{
	// Make ffmpeg compatible file name
	if (!::IsExistingFile(lpszFileName))
	{
		const DWORD dwInit = 0U;
		DWORD NumberOfBytesWritten;
		HANDLE hFile = ::CreateFile(lpszFileName,
									GENERIC_WRITE, 0, NULL,
									CREATE_NEW,
									FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			::WriteFile(hFile, &dwInit, sizeof(DWORD), &NumberOfBytesWritten, NULL);
			::CloseHandle(hFile);
		}
	}
	CString sASCIICompatiblePath = ::GetASCIICompatiblePath(lpszFileName); // file must exist!
	char filename[1024];
	wcstombs(filename, sASCIICompatiblePath, 1024);
	filename[1023] = '\0';

	// Close
	Close();

	// Set vars
	m_sFileName = lpszFileName;
	m_dwTotalVideoStreams = 0;
	m_dwTotalAudioStreams = 0;
	
	// Allocate the output media context
	avformat_alloc_output_context2(&m_pFormatCtx, NULL, NULL, filename);
	if (!m_pFormatCtx)
		avformat_alloc_output_context2(&m_pFormatCtx, NULL, "avi", filename);
	if (!m_pFormatCtx)
		return false;
	m_pOutputFormat = m_pFormatCtx->oformat;

	// A mov/mp4 file can have either all the metadata about all packets stored
	// on top of the file (if setting the FF_MOV_FLAG_FASTSTART flag) or on bottom
	// of the file. In both cases the file must be enterly recorded before being
	// opened.
	// It can also be fragmented where packets and metadata about these packets
	// are stored together. Writing a fragmented file has the advantage that the
	// file is decodable even if the writing is still in progress or if it has
	// been interrupted (the downside is that it is less compatible with some
	// applications)
	if (strcmp(m_pFormatCtx->oformat->name, "mov") == 0 ||
		strcmp(m_pFormatCtx->oformat->name, "mp4") == 0)
	{
		MOVMuxContext* mov = (MOVMuxContext*)m_pFormatCtx->priv_data;
		if (mov)
		{
			if (bMovFragmented)
			{
				mov->flags |=	(FF_MOV_FLAG_FRAG_KEYFRAME |	// fragment at video keyframes
								FF_MOV_FLAG_EMPTY_MOOV);		// causes 100% fragmented output; without this the first fragment will
																// be muxed as a short movie (using moov) followed by the rest of the
																// media in fragments
			}
			else
			{
				mov->flags |= FF_MOV_FLAG_FASTSTART;			// run a second pass moving the moov atom to the top of the file making
																// pseudo-streaming possible
			}
		}
	}

	return true;
}

int CAVRec::AddVideoStream(	const LPBITMAPINFO pSrcFormat,
							const LPBITMAPINFO pDstFormat,
							DWORD dwDstRate,
							DWORD dwDstScale,
							int keyframes_rate,
							float qscale,	// 2.0f best quality, 31.0f worst quality, for H.264 clamped to [VIDEO_QUALITY_BEST, VIDEO_QUALITY_LOW]
							int nThreadCount)
{
	int nStreamNum = -1;

	// Check
	if (!pSrcFormat || !pDstFormat || dwDstScale == 0 ||
		dwDstRate == 0 || !m_pOutputFormat || !m_pFormatCtx)
		return -1;

	// Set the Codec ID
	m_pOutputFormat->video_codec = AVCodecFourCCToCodecID(pDstFormat->bmiHeader.biCompression);
	if (m_pOutputFormat->video_codec == AV_CODEC_ID_H263)
		m_pOutputFormat->video_codec = AV_CODEC_ID_H263P;		// set encoder to H.263+
	else if (m_pOutputFormat->video_codec == AV_CODEC_ID_NONE)
		m_pOutputFormat->video_codec = AV_CODEC_ID_RAWVIDEO;	// set encoder to raw
	
	// Find the video encoder
	AVCodec* pCodec = avcodec_find_encoder(m_pOutputFormat->video_codec);
	if (!pCodec)
		return -1;

	// Check whether pixel format is supported
	AVPixelFormat pix_fmt;
	if (m_pOutputFormat->video_codec == CODEC_ID_RAWVIDEO)
		pix_fmt = AVCodecBMIToPixFormat(pDstFormat);
	else
	{
		pix_fmt = AVCodecBMIToPixFormat(pSrcFormat);
		if (pCodec->pix_fmts)
		{
			const enum AVPixelFormat *p = pCodec->pix_fmts;
			for ( ; *p != -1 ; p++)
			{
				if (*p == AV_PIX_FMT_YUV420P)
					pix_fmt = AV_PIX_FMT_YUV420P; // YUV420P is always first choice
				if (*p == pix_fmt)
					break;
			}
			if (*p == -1)
				pix_fmt = pCodec->pix_fmts[0];
		}
		else
			pix_fmt = AV_PIX_FMT_YUV420P;
	}

	// Create Video Stream
	AVStream* pVideoStream = avformat_new_stream(m_pFormatCtx, pCodec);
	if (!pVideoStream)
		return -1;
	pVideoStream->id = m_pFormatCtx->nb_streams - 1;
	AVCodecContext* pCodecCtx = pVideoStream->codec;
	pCodecCtx->codec_id = m_pOutputFormat->video_codec;
	pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
	if (strcmp(m_pFormatCtx->oformat->name, "avi") == 0)
		pCodecCtx->codec_tag = pDstFormat->bmiHeader.biCompression;

	// Quality
	if (qscale > 31.0f)
		qscale = 31.0f;
	else if (qscale < 2.0f)
		qscale = 2.0f;
	pCodecCtx->bit_rate = 0; // only use quality and not bitrate

	// Encoder specific settings
	if (pCodecCtx->codec_id == AV_CODEC_ID_H264)
	{
		// Clamp to [VIDEO_QUALITY_BEST, VIDEO_QUALITY_LOW]
		if (qscale > VIDEO_QUALITY_LOW)
			qscale = VIDEO_QUALITY_LOW;
		else if (qscale < VIDEO_QUALITY_BEST)
			qscale = VIDEO_QUALITY_BEST;
		int nQScale = (int)qscale;

		// Quality 0-51: where 0 is lossless, 23 is default, and 51 is worst possible
		// Note: subjectively sane range is 18-28 (consider 18 to be visually lossless)
		int crf;
		switch (nQScale)
		{
			case 3 : crf = 22; break;
			case 4 : crf = 25; break;
			case 5 : crf = 27; break;
			default: crf = 29; break;
		}
		CStringA scrf;
		scrf.Format("%d.0", crf);
		av_opt_set(pCodecCtx->priv_data, "crf", scrf, 0);

		// Set profile to baseline so that all devices can play it
		// Note: setting a profile with lossless encoding is not working!
		if (crf > 0)
			av_opt_set(pCodecCtx->priv_data, "profile", "baseline", AV_OPT_SEARCH_CHILDREN);

		// Encoding speed
		// "placebo", "veryslow", "slower", "slow", "medium",
		// "fast", "faster", "veryfast", "superfast", "ultrafast"
		//
		// Notes
		// - faster presets create bigger files for the same quality
		//   (ultrafast is ~30% faster but produces 2.5x bigger files than veryfast)
		// - slower presets use more RAM because of the increasing complexity
		//   of the used algorithms, for example a Full HD video encoding uses
		//   ~270 MB with medium, ~130 MB with veryfast and ~65 MB with ultrafast
		av_opt_set(pCodecCtx->priv_data, "preset", "veryfast", 0);

		// Disable 256-bit instructions because mingw doesn't support 32-bytes stack alignment on Windows
		// Attention: remember to disable newer SIMD instructions when recompiling x264 (see x264.h)
		// Note:      to make sure the following command works, enable most verbose logging and search
		//            for the "using cpu capabilities" message generated by avcodec_open_thread_safe()
		uint32_t cpu = x264_cpu_detect();
		#define X264_CPU_LZCNT           0x0000200  /* Phenom support for "leading zero count" instruction. */
		#define X264_CPU_AVX             0x0000400  /* AVX support: requires OS support even if YMM registers aren't used. */
		#define X264_CPU_XOP             0x0000800  /* AMD XOP */
		#define X264_CPU_FMA4            0x0001000  /* AMD FMA4 */
		#define X264_CPU_FMA3            0x0002000  /* FMA3 */
		#define X264_CPU_AVX2            0x0004000  /* AVX2 */
		#define X264_CPU_BMI1            0x0008000  /* BMI1 */
		#define X264_CPU_BMI2            0x0010000  /* BMI2 */
		cpu &= ~(X264_CPU_LZCNT | X264_CPU_AVX | X264_CPU_XOP | X264_CPU_FMA4 | X264_CPU_FMA3 | X264_CPU_AVX2 | X264_CPU_BMI1 | X264_CPU_BMI2);
		CStringA sAsmOption;
		sAsmOption.Format("asm=%u", cpu);
		av_opt_set(pCodecCtx->priv_data, "x264-params", sAsmOption, 0); // accepts both numbers and names: "asm=15" is equivalent to "asm=SSE"
	}
	else
	{
		// Quality: 2.0f best quality, 31.0f worst quality
        pCodecCtx->flags |= CODEC_FLAG_QSCALE;
		pCodecCtx->global_quality = (int)(FF_QP2LAMBDA * qscale);
	}

	// Resolution must be a multiple of two
	pCodecCtx->width = pDstFormat->bmiHeader.biWidth;
	pCodecCtx->height = pDstFormat->bmiHeader.biHeight;

	// Time base
	// is the fundamental unit of time (in seconds) in terms of which frame
	// timestamps are represented. For fixed framerate content, timebase should
	// be 1/framerate and timestamp increments should be identically to 1.
	// For variable framerate we can set a ms resolution time base like:
	// pCodecCtx->time_base.num = 1 and pCodecCtx->time_base.den = 1000
	//
	// Most container formats are supporting VFR, except swf which is not.
	// Although avi is not designed for variable framerates, it is possible to 
	// use them without creating a non-standard file by using 0-byte chunks for 
	// skipped frames. However it requires framerate to be set to least common 
	// multiple of all framerates used, and produces slight overhead compared 
	// to true VFR (ffmpeg avi muxer correctly adds 0-byte chunks as delta
	// frames so that seeking works well)
	int dst_rate, dst_scale;
	av_reduce(&dst_scale, &dst_rate, (int64_t)dwDstScale, (int64_t)dwDstRate, MAX_SIZE_FOR_RATIONAL);
	pCodecCtx->time_base.den = dst_rate;
	pCodecCtx->time_base.num = dst_scale;
	
	// Emit one intra frame every given frames at most
	pCodecCtx->gop_size = keyframes_rate;	// keyframe interval(=GOP length) determines the maximum distance between I-frames.
											// Normally it defaults to 12, for H.264 it defaults to -1 which means there is
											// no upper limit, codec will still insert a keyframe with each scene change

	// Pixel Format
	pCodecCtx->pix_fmt = pix_fmt;

	// Some formats want stream headers to be separate
	if (m_pFormatCtx->oformat->flags & AVFMT_GLOBALHEADER)
		pCodecCtx->flags |= CODEC_FLAG_GLOBAL_HEADER;
	
	// Stream Number
	nStreamNum = pVideoStream->index;

	// Set thread count
	// Note: in ffmpeg source MAX_THREADS definition differs
	// from header to header ... 32 is a safe value
	nThreadCount = MIN(nThreadCount, 32);
	if (pCodecCtx->codec_id == AV_CODEC_ID_MJPEG		||
		pCodecCtx->codec_id == AV_CODEC_ID_MPEG1VIDEO	||
        pCodecCtx->codec_id == AV_CODEC_ID_MPEG2VIDEO	||
		pCodecCtx->codec_id == AV_CODEC_ID_H263P		||
		pCodecCtx->codec_id == AV_CODEC_ID_MPEG4		||
		pCodecCtx->codec_id == AV_CODEC_ID_H264)
	{
		pCodecCtx->thread_count = nThreadCount;
		if (pCodecCtx->thread_count > 1)
			pCodecCtx->thread_type = FF_THREAD_SLICE;
	}

	// Open the video codec
	if (avcodec_open_thread_safe(pCodecCtx, pCodec) < 0)
		return -1;

	// Allocate video frames
	m_pFrame[nStreamNum] = av_frame_alloc();
	if (!m_pFrame[nStreamNum])
		return -1;
	m_pFrameTemp[nStreamNum] = av_frame_alloc();
	if (!m_pFrameTemp[nStreamNum])
		return -1;

	// Inc. streams count
	m_dwTotalVideoStreams++;

	return nStreamNum;
}

int CAVRec::AddAudioStream(	const LPWAVEFORMATEX pSrcWaveFormat,
							const LPWAVEFORMATEX pDstWaveFormat)
{
	// Check
	if (!pSrcWaveFormat || !pDstWaveFormat ||
		!m_pOutputFormat || !m_pFormatCtx)
		return -1;

	// Set the Codec
	m_pOutputFormat->audio_codec = AVCodecFormatTagToCodecID(	pDstWaveFormat->wFormatTag,
																pDstWaveFormat->wBitsPerSample);
	if (m_pOutputFormat->audio_codec == AV_CODEC_ID_NONE)
		return -1;
	
	// Find the audio encoder
	AVCodec* pCodec = avcodec_find_encoder(m_pOutputFormat->audio_codec);
	if (!pCodec)
		return -1;

	// Create Audio Stream
	AVStream* pAudioStream = avformat_new_stream(m_pFormatCtx, pCodec);
	if (!pAudioStream)
		return -1;
	pAudioStream->id = m_pFormatCtx->nb_streams - 1;
	AVCodecContext* pCodecCtx = pAudioStream->codec;
	pCodecCtx->codec_id = m_pOutputFormat->audio_codec;
	pCodecCtx->codec_type = AVMEDIA_TYPE_AUDIO;
	pCodecCtx->sample_fmt = pCodec->sample_fmts ? pCodec->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
	pCodecCtx->bit_rate = pDstWaveFormat->nAvgBytesPerSec * 8;
	pCodecCtx->sample_rate = pDstWaveFormat->nSamplesPerSec;
	pCodecCtx->channels = pDstWaveFormat->nChannels;
	if (strcmp(m_pFormatCtx->oformat->name, "avi") == 0)
		pCodecCtx->codec_tag = pDstWaveFormat->wFormatTag;
	pCodecCtx->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL; // to enable AAC

	// Some formats want stream headers to be separate
	if (m_pFormatCtx->oformat->flags & AVFMT_GLOBALHEADER)
		pCodecCtx->flags |= CODEC_FLAG_GLOBAL_HEADER;

	// Stream Number
	DWORD dwStreamNum = pAudioStream->index;

	// Open the audio codec
	if (avcodec_open_thread_safe(pCodecCtx, pCodec) < 0)
		return -1;

	// Set the correct bitrate for ADPCM
	if (m_pOutputFormat->audio_codec == AV_CODEC_ID_ADPCM_IMA_WAV	||
		m_pOutputFormat->audio_codec == AV_CODEC_ID_ADPCM_MS)
	{
		double dFrameTime = (double)(pCodecCtx->frame_size) / (double)(pDstWaveFormat->nSamplesPerSec);
		pDstWaveFormat->nAvgBytesPerSec = Round((double)(pCodecCtx->block_align) / dFrameTime);
		pCodecCtx->bit_rate = pDstWaveFormat->nAvgBytesPerSec * 8;
	}

	// Source format
	SetSrcWaveFormat(dwStreamNum, pSrcWaveFormat); // allocates memory!
	AVSampleFormat src_sample_fmt = pSrcWaveFormat->wBitsPerSample == 16 ? AV_SAMPLE_FMT_S16 : AV_SAMPLE_FMT_U8;

	// Destination buffer
	// - m_nDstBufSize is the number of samples in one channel
	// - Audio in avi files must be either constant-bitrate (CBR)
	//   or constant-framesize (i.e. all frames decode to the same
	//   number of samples)
	if (pCodecCtx->codec->capabilities & CODEC_CAP_VARIABLE_FRAME_SIZE)
	{
		if (pCodecCtx->sample_rate <= 11025) 
			m_nDstBufSize[dwStreamNum] = 1 * AUDIO_PCM_MIN_BUF_SIZE;
		else if (pCodecCtx->sample_rate <= 22050)
			m_nDstBufSize[dwStreamNum] = 2 * AUDIO_PCM_MIN_BUF_SIZE;
		else if (pCodecCtx->sample_rate <= 32000)
			m_nDstBufSize[dwStreamNum] = 3 * AUDIO_PCM_MIN_BUF_SIZE;
		else if (pCodecCtx->sample_rate <= 44100)
			m_nDstBufSize[dwStreamNum] = 4 * AUDIO_PCM_MIN_BUF_SIZE;
		else if (pCodecCtx->sample_rate <= 48000)
			m_nDstBufSize[dwStreamNum] = 5 * AUDIO_PCM_MIN_BUF_SIZE;
		else
			m_nDstBufSize[dwStreamNum] = 8 * AUDIO_PCM_MIN_BUF_SIZE;
	}
	else
		m_nDstBufSize[dwStreamNum] = pCodecCtx->frame_size;
	if (av_samples_alloc_array_and_samples(	&m_ppDstBuf[dwStreamNum],
											NULL,
											pCodecCtx->channels,
											m_nDstBufSize[dwStreamNum],
											pCodecCtx->sample_fmt,
											0) < 0)
		return -1;
	m_pFrame[dwStreamNum] = av_frame_alloc();
	if (!m_pFrame[dwStreamNum])
		return -1;

	// Create resampler context
	m_pAudioConvertCtx[dwStreamNum] = swr_alloc();
	if (!m_pAudioConvertCtx[dwStreamNum])
		return -1;

	// Set options
	av_opt_set_int       (m_pAudioConvertCtx[dwStreamNum], "in_channel_count",   pSrcWaveFormat->nChannels, 0);
	av_opt_set_int       (m_pAudioConvertCtx[dwStreamNum], "in_sample_rate",     pSrcWaveFormat->nSamplesPerSec, 0);
	av_opt_set_sample_fmt(m_pAudioConvertCtx[dwStreamNum], "in_sample_fmt",      src_sample_fmt, 0);
	av_opt_set_int       (m_pAudioConvertCtx[dwStreamNum], "out_channel_count",  pCodecCtx->channels, 0);
	av_opt_set_int       (m_pAudioConvertCtx[dwStreamNum], "out_sample_rate",    pCodecCtx->sample_rate, 0);
	av_opt_set_sample_fmt(m_pAudioConvertCtx[dwStreamNum], "out_sample_fmt",     pCodecCtx->sample_fmt, 0);

	// Initialize the resampling context
	if (swr_init(m_pAudioConvertCtx[dwStreamNum]) < 0)
		return -1;

	// Inc. streams count
	m_dwTotalAudioStreams++;

	return dwStreamNum;
}

bool CAVRec::Open()
{
	// Check
	if (!m_pOutputFormat || !m_pFormatCtx)
		return false;

	// Open the output file, if needed
	if (!(m_pOutputFormat->flags & AVFMT_NOFILE))
	{
		if (avio_open(&m_pFormatCtx->pb, m_pFormatCtx->filename, AVIO_FLAG_WRITE) < 0)
			return false;
		else
			m_bFileOpened = true;
	}

	// Write the stream header, if any
	if (avformat_write_header(m_pFormatCtx, NULL) < 0)
		return false;
	else
	{
		m_bOpen = true;
		return true;
	}
}

void CAVRec::Flush()
{
	if (m_pFormatCtx)
	{
		for (DWORD i = 0 ; i < (DWORD)m_pFormatCtx->nb_streams ; i++)
		{
			if (m_pFormatCtx->streams[i] && m_pFormatCtx->streams[i]->codec)
			{
				if (m_pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
				{
					while (AddFrame(i, NULL, NULL, false));
				}
				else if (m_pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
				{
					while (AddAudioSamples(i, 0, NULL, false));
				}
			}
		}
	}
}

bool CAVRec::Close()
{
	int i;
	bool res = true;

	::EnterCriticalSection(&m_csAV);

#ifdef PERFORMANCE_MEASUREMENT
	if (m_ullAddFrameCount > 0)
	{
		m_ullAddFrameTimeMicroSec /= m_ullAddFrameCount;
		::LogLine(_T("Avg Video Frame Encode Time(%s) : %I64uus"), m_sFileName, m_ullAddFrameTimeMicroSec);
	}
	m_ullAddFrameTimeMicroSec = 0;
	m_ullAddFrameCount = 0;
#endif

	if (m_pFormatCtx)
	{
		// Flush audio buffers
		if (m_bFileOpened)
			Flush();

		// Write the trailer (only after a successful call to avformat_write_header)
		if (m_bOpen && av_write_trailer(m_pFormatCtx) != 0)
			res = false;

		// Close & Free the Codecs of all the Streams
		for (i = 0 ; i < (int)m_pFormatCtx->nb_streams ; i++)
		{
			if (m_pFormatCtx->streams[i])
			{
				// Close
				if (m_pFormatCtx->streams[i]->codec)
				{
					/*
					Close a given AVCodecContext and free all the data associated with it
					(but not the AVCodecContext itself).
					Calling this function on an AVCodecContext that hasn't been opened will free
					the codec-specific data allocated in avcodec_alloc_context3() /
					avcodec_get_context_defaults3() with a non-NULL codec. Subsequent calls will
					do nothing.
					*/
					avcodec_close_thread_safe(m_pFormatCtx->streams[i]->codec);
				}
			}
		}

		// Close the output file
		if (m_bFileOpened)
		{
			if (!(m_pOutputFormat->flags & AVFMT_NOFILE))
			{
				avio_close(m_pFormatCtx->pb); // This frees pb->buffer and pb itself but doesn't set it to NULL!
				m_pFormatCtx->pb = NULL;
			}
		}

		// Free Format Ctx
		avformat_free_context(m_pFormatCtx);
		m_pFormatCtx = NULL;
	}

	// Free Various
	for (DWORD dwStreamNum = 0 ; dwStreamNum < MAX_STREAMS ; dwStreamNum++)
	{
		// Free Conversion Contexts
		if (m_pImgConvertCtx[dwStreamNum])
		{
			sws_freeContext(m_pImgConvertCtx[dwStreamNum]);
			m_pImgConvertCtx[dwStreamNum] = NULL;
		}
		if (m_pAudioConvertCtx[dwStreamNum])
			swr_free(&m_pAudioConvertCtx[dwStreamNum]);

		// Free buffers
		if (m_ppSrcBuf[dwStreamNum])
		{
			if (*m_ppSrcBuf[dwStreamNum])
				av_freep(m_ppSrcBuf[dwStreamNum]);
			av_freep(&m_ppSrcBuf[dwStreamNum]);
		}
		m_nSrcBufSize[dwStreamNum] = 0;
		if (m_ppDstBuf[dwStreamNum])
		{
			if (*m_ppDstBuf[dwStreamNum])
				av_freep(m_ppDstBuf[dwStreamNum]);
			av_freep(&m_ppDstBuf[dwStreamNum]);
		}
		m_nDstBufSize[dwStreamNum] = 0;
		if (m_pAVPalette[dwStreamNum])
			av_freep(&m_pAVPalette[dwStreamNum]);

		// Free frames
		if (m_pFrame[dwStreamNum])
			av_frame_free(&m_pFrame[dwStreamNum]);
		if (m_pFrameTemp[dwStreamNum])
			av_frame_free(&m_pFrameTemp[dwStreamNum]);

		// Free Wave Format
		if (m_pSrcWaveFormat[dwStreamNum])
		{
			delete [] m_pSrcWaveFormat[dwStreamNum];
			m_pSrcWaveFormat[dwStreamNum] = NULL;
		}

		// Reset counters
		m_llTotalFramesOrSamples[dwStreamNum] = 0;
		m_llTotalWrittenBytes[dwStreamNum] = 0;
		m_llLastCodecPTS[dwStreamNum] = AV_NOPTS_VALUE;
		m_llLastStreamDTS[dwStreamNum] = AV_NOPTS_VALUE;
	}

	m_sFileName = _T("");
	m_dwTotalVideoStreams = 0;
	m_dwTotalAudioStreams = 0;
	m_bOpen = false;
	m_bFileOpened = false;

	::LeaveCriticalSection(&m_csAV);

	return res;
}

bool CAVRec::AddFrame(	DWORD dwStreamNum,
						LPBITMAPINFO pBmi,
						LPBYTE pBits,
						bool bInterleaved,
						int64_t pts/*=AV_NOPTS_VALUE*/)
{
	::EnterCriticalSection(&m_csAV);

	// Performance measurement init
#ifdef PERFORMANCE_MEASUREMENT
	CPerformance perf;
	perf.Init();
#endif

	// Check
	if (!m_pFormatCtx								||
		dwStreamNum >= m_pFormatCtx->nb_streams		||
		!m_pFormatCtx->streams[dwStreamNum]			||
		!m_pFormatCtx->streams[dwStreamNum]->codec	||
		m_pFormatCtx->streams[dwStreamNum]->codec->codec_type != AVMEDIA_TYPE_VIDEO)
	{
		::LeaveCriticalSection(&m_csAV);
		return false;
	}

	// Skip frame if not strictly monotonically increasing pts
	if (pts != AV_NOPTS_VALUE							&&
		m_llLastCodecPTS[dwStreamNum] != AV_NOPTS_VALUE	&&
		pts <= m_llLastCodecPTS[dwStreamNum])
	{
		::LeaveCriticalSection(&m_csAV);
		return true;
	}
	else
		m_llLastCodecPTS[dwStreamNum] = pts;

	// Get the attached codec context
	AVCodecContext* pCodecCtx = m_pFormatCtx->streams[dwStreamNum]->codec;

	// Prepare frame with correct pixel format, quality and pts
	bool bFlush = !pBits || !pBmi;
	if (!bFlush)
	{
		// Get Src Pixel Format
		enum AVPixelFormat SrcPixFormat = AVCodecBMIToPixFormat(pBmi);
		if (SrcPixFormat == AV_PIX_FMT_NONE)
		{
			::LeaveCriticalSection(&m_csAV);
			return false;
		}

		// Copy input bits to source buffer
		// (use an input buffer because it is aligned for the conversion)
		if (m_ppSrcBuf[dwStreamNum] == NULL)
		{
			m_ppSrcBuf[dwStreamNum] = (uint8_t**)av_mallocz(sizeof(uint8_t*));
			if (m_ppSrcBuf[dwStreamNum] == NULL)
			{
				::LeaveCriticalSection(&m_csAV);
				return false;
			}
		}
		int nNewSrcBufSize = avpicture_get_size(	SrcPixFormat,
													pBmi->bmiHeader.biWidth,
													pBmi->bmiHeader.biHeight);
		if (*m_ppSrcBuf[dwStreamNum] == NULL || m_nSrcBufSize[dwStreamNum] < nNewSrcBufSize)
		{
			if (*m_ppSrcBuf[dwStreamNum])
				av_free(*m_ppSrcBuf[dwStreamNum]);
			*m_ppSrcBuf[dwStreamNum] = (uint8_t*)av_malloc(nNewSrcBufSize + FF_INPUT_BUFFER_PADDING_SIZE);
			if (*m_ppSrcBuf[dwStreamNum] == NULL)
			{
				m_nSrcBufSize[dwStreamNum] = 0;
				::LeaveCriticalSection(&m_csAV);
				return false;
			}
			m_nSrcBufSize[dwStreamNum] = nNewSrcBufSize;
		}
		memcpy(*m_ppSrcBuf[dwStreamNum], pBits, nNewSrcBufSize);

		// Convert to codec's pixel format or codec's size?
		if (pCodecCtx->pix_fmt != SrcPixFormat			||
			pBmi->bmiHeader.biWidth != pCodecCtx->width	||
			pBmi->bmiHeader.biHeight != pCodecCtx->height)
		{
			// Check
			if (pBmi->bmiHeader.biWidth <= 0 || pBmi->bmiHeader.biHeight <= 0	||
				SrcPixFormat < 0 || SrcPixFormat >= AV_PIX_FMT_NB				||
				pCodecCtx->width <= 0 || pCodecCtx->height <= 0					||
				pCodecCtx->pix_fmt < 0 || pCodecCtx->pix_fmt >= AV_PIX_FMT_NB)
			{
				::LeaveCriticalSection(&m_csAV);
				return false;
			}

			// Get conversion context
			m_pImgConvertCtx[dwStreamNum] = sws_getCachedContext(m_pImgConvertCtx[dwStreamNum],	// Re-use if already allocated
																pBmi->bmiHeader.biWidth,		// Source Width
																pBmi->bmiHeader.biHeight,		// Source Height
																SrcPixFormat,					// Source Format
																pCodecCtx->width,				// Destination Width
																pCodecCtx->height,				// Destination Height
																pCodecCtx->pix_fmt,				// Destination Format
																SWS_BICUBIC,					// Interpolation (add SWS_PRINT_INFO to debug)
																NULL,							// No Source Filter
																NULL,							// No Destination Filter
																NULL);							// Param
			if (!m_pImgConvertCtx[dwStreamNum])
			{
				::LeaveCriticalSection(&m_csAV);
				return false;
			}

			// Init Src Frame
			avpicture_fill(	(AVPicture*)(m_pFrameTemp[dwStreamNum]),
							*m_ppSrcBuf[dwStreamNum],
							SrcPixFormat,
							pBmi->bmiHeader.biWidth,
							pBmi->bmiHeader.biHeight);
			if (SrcPixFormat == AV_PIX_FMT_PAL8)
			{
				if (m_pAVPalette[dwStreamNum] == NULL)
				{
					m_pAVPalette[dwStreamNum] = (uint8_t*)av_malloc(AVPALETTE_SIZE);
					if (m_pAVPalette[dwStreamNum] == NULL)
					{
						::LeaveCriticalSection(&m_csAV);
						return false;
					}
				}
				memset(m_pAVPalette[dwStreamNum], 0, AVPALETTE_SIZE);
				int nUsedColors = pBmi->bmiHeader.biClrUsed != 0 ? pBmi->bmiHeader.biClrUsed : 256;
				memcpy(m_pAVPalette[dwStreamNum], pBmi->bmiColors, MIN(AVPALETTE_SIZE, nUsedColors * sizeof(RGBQUAD)));
				m_pFrameTemp[dwStreamNum]->data[1] = m_pAVPalette[dwStreamNum];
			}

			// Flip U <-> V pointers?
			if (pBmi->bmiHeader.biCompression == FCC('YV12') ||
				pBmi->bmiHeader.biCompression == FCC('YV16') ||
				pBmi->bmiHeader.biCompression == FCC('YVU9'))
			{
				uint8_t* pTemp = m_pFrameTemp[dwStreamNum]->data[1];
				m_pFrameTemp[dwStreamNum]->data[1] = m_pFrameTemp[dwStreamNum]->data[2];
				m_pFrameTemp[dwStreamNum]->data[2] = pTemp;
				// Line Sizes for U and V are the same no need to swap
			}
			// RGB flip vertically?
			else if (pBmi->bmiHeader.biCompression == BI_RGB ||
					pBmi->bmiHeader.biCompression == BI_BITFIELDS)
			{
				m_pFrameTemp[dwStreamNum]->data[0] += m_pFrameTemp[dwStreamNum]->linesize[0] * (pBmi->bmiHeader.biHeight - 1);
				m_pFrameTemp[dwStreamNum]->linesize[0] *= -1;
			}

			// Init Dst Frame
			if (m_ppDstBuf[dwStreamNum] == NULL)
			{
				m_ppDstBuf[dwStreamNum] = (uint8_t**)av_mallocz(sizeof(uint8_t*));
				if (m_ppDstBuf[dwStreamNum] == NULL)
				{
					::LeaveCriticalSection(&m_csAV);
					return false;
				}
			}
			int nNewDstBufSize = avpicture_get_size(	pCodecCtx->pix_fmt,
														pCodecCtx->width,
														pCodecCtx->height);
			if (*m_ppDstBuf[dwStreamNum] == NULL || m_nDstBufSize[dwStreamNum] < nNewDstBufSize)
			{
				if (*m_ppDstBuf[dwStreamNum])
					av_free(*m_ppDstBuf[dwStreamNum]);
				*m_ppDstBuf[dwStreamNum] = (uint8_t*)av_malloc(nNewDstBufSize + FF_INPUT_BUFFER_PADDING_SIZE);
				if (*m_ppDstBuf[dwStreamNum] == NULL)
				{
					m_nDstBufSize[dwStreamNum] = 0;
					::LeaveCriticalSection(&m_csAV);
					return false;
				}
				m_nDstBufSize[dwStreamNum] = nNewDstBufSize;
			}
			avpicture_fill(	(AVPicture*)(m_pFrame[dwStreamNum]),
							*m_ppDstBuf[dwStreamNum],
							pCodecCtx->pix_fmt,
							pCodecCtx->width,
							pCodecCtx->height);

			// Convert (first try fast conversion, if not supported fall back to sws_scale)
			BOOL bOk = FALSE;
			if (pBmi->bmiHeader.biWidth == pCodecCtx->width &&
				pBmi->bmiHeader.biHeight == pCodecCtx->height)
			{
				bOk = ITU601JPEGConvert(	SrcPixFormat,							// Source Format
											pCodecCtx->pix_fmt,						// Destination Format
											m_pFrameTemp[dwStreamNum]->data,		// Source Data
											m_pFrameTemp[dwStreamNum]->linesize,	// Source Stride
											m_pFrame[dwStreamNum]->data,			// Destination Data
											m_pFrame[dwStreamNum]->linesize,		// Destination Stride
											pCodecCtx->width,						// Width
											pCodecCtx->height);						// Height
			}
			if (!bOk)
			{
				if (sws_scale(	m_pImgConvertCtx[dwStreamNum],
								m_pFrameTemp[dwStreamNum]->data,					// Source Data
								m_pFrameTemp[dwStreamNum]->linesize,				// Source Stride
								0,
								pBmi->bmiHeader.biHeight,
								m_pFrame[dwStreamNum]->data,						// Destination Data
								m_pFrame[dwStreamNum]->linesize) <= 0)				// Destination Stride
				{
					::LeaveCriticalSection(&m_csAV);
					return false;
				}
			}
		}
		else
		{
			// Init Src Frame
			avpicture_fill(	(AVPicture*)(m_pFrame[dwStreamNum]),
							*m_ppSrcBuf[dwStreamNum],
							SrcPixFormat,
							pBmi->bmiHeader.biWidth,
							pBmi->bmiHeader.biHeight);
			if (SrcPixFormat == AV_PIX_FMT_PAL8)
			{
				if (m_pAVPalette[dwStreamNum] == NULL)
				{
					m_pAVPalette[dwStreamNum] = (uint8_t*)av_malloc(AVPALETTE_SIZE);
					if (m_pAVPalette[dwStreamNum] == NULL)
					{
						::LeaveCriticalSection(&m_csAV);
						return false;
					}
				}
				memset(m_pAVPalette[dwStreamNum], 0, AVPALETTE_SIZE);
				int nUsedColors = pBmi->bmiHeader.biClrUsed != 0 ? pBmi->bmiHeader.biClrUsed : 256;
				memcpy(m_pAVPalette[dwStreamNum], pBmi->bmiColors, MIN(AVPALETTE_SIZE, nUsedColors * sizeof(RGBQUAD)));
				m_pFrame[dwStreamNum]->data[1] = m_pAVPalette[dwStreamNum];
			}

			// Flip U <-> V pointers?
			if (pBmi->bmiHeader.biCompression == FCC('YV12') ||
				pBmi->bmiHeader.biCompression == FCC('YV16') ||
				pBmi->bmiHeader.biCompression == FCC('YVU9'))
			{
				uint8_t* pTemp = m_pFrame[dwStreamNum]->data[1];
				m_pFrame[dwStreamNum]->data[1] = m_pFrame[dwStreamNum]->data[2];
				m_pFrame[dwStreamNum]->data[2] = pTemp;
				// Line Sizes for U and V are the same no need to swap
			}
			// RGB flip vertically?
			else if (pBmi->bmiHeader.biCompression == BI_RGB ||
					pBmi->bmiHeader.biCompression == BI_BITFIELDS)
			{
				m_pFrame[dwStreamNum]->data[0] += m_pFrame[dwStreamNum]->linesize[0] * (pBmi->bmiHeader.biHeight - 1);
				m_pFrame[dwStreamNum]->linesize[0] *= -1;
			}
		}

		// Set Quality
		if (pCodecCtx->flags & CODEC_FLAG_QSCALE)
			m_pFrame[dwStreamNum]->quality = pCodecCtx->global_quality;

		// Set pts here and not after encoding because of possible B-frames!
		m_pFrame[dwStreamNum]->pts = pts != AV_NOPTS_VALUE ? pts : m_llTotalFramesOrSamples[dwStreamNum];
	}

	// Encode and write frame to file
	bool res = EncodeAndWriteFrame(dwStreamNum, bFlush ? NULL : m_pFrame[dwStreamNum], bInterleaved);

	// Performance measurement end
#ifdef PERFORMANCE_MEASUREMENT
	if (!bFlush)
	{
		perf.End();
		m_ullAddFrameTimeMicroSec += perf.GetMicroSecDiff();
		++m_ullAddFrameCount;
	}
#endif

	::LeaveCriticalSection(&m_csAV);
	return res;
}

__forceinline void CAVRec::AdjustPTSDTS(DWORD dwStreamNum, AVPacket* pkt)
{
	AVCodecContext* pCodecCtx = m_pFormatCtx->streams[dwStreamNum]->codec;

	// Rescale output packet timestamp values from codec to stream timebase
	// pkt->pts = pkt->pts * pCodecCtx->time_base / m_pFormatCtx->streams[dwStreamNum]->time_base
	// pkt->dts = pkt->dts * pCodecCtx->time_base / m_pFormatCtx->streams[dwStreamNum]->time_base
	pkt->pts = av_rescale_q_rnd(pkt->pts, pCodecCtx->time_base, m_pFormatCtx->streams[dwStreamNum]->time_base, (enum AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
	pkt->dts = av_rescale_q_rnd(pkt->dts, pCodecCtx->time_base, m_pFormatCtx->streams[dwStreamNum]->time_base, (enum AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
	pkt->duration = (int)av_rescale_q(pkt->duration, pCodecCtx->time_base, m_pFormatCtx->streams[dwStreamNum]->time_base);
	pkt->stream_index = m_pFormatCtx->streams[dwStreamNum]->index;

	// Make sure that PTS and DTS are monotonically increasing, otherwise 
	// av_interleaved_write_frame and av_write_frame fail
	if (pkt->dts != AV_NOPTS_VALUE)
	{
		if (m_llLastStreamDTS[dwStreamNum] != AV_NOPTS_VALUE)
		{
			// Format supports non-strictly monotonically increasing dts: -10,1,1,2,6,7,7,15,...
			if (m_pOutputFormat->flags & AVFMT_TS_NONSTRICT)
			{
				if (pkt->dts < m_llLastStreamDTS[dwStreamNum])
				{
					if (g_nLogLevel > 0)
					{
						::LogLine(_T("Stream%u: DTS adjusted non-strictly monotonically %I64d -> %I64d"),
								dwStreamNum, pkt->dts, m_llLastStreamDTS[dwStreamNum]);
					}
					pkt->dts = m_llLastStreamDTS[dwStreamNum];
				}
			}
			// Format needs strictly monotonically increasing dts: -10,0,1,2,7,10,15,...
			else
			{
				if (pkt->dts <= m_llLastStreamDTS[dwStreamNum])
				{
					if (g_nLogLevel > 0)
					{
						::LogLine(_T("Stream%u: DTS adjusted strictly monotonically %I64d -> %I64d"),
								dwStreamNum, pkt->dts, m_llLastStreamDTS[dwStreamNum] + 1);
					}
					pkt->dts = m_llLastStreamDTS[dwStreamNum] + 1;
				}
			}
		}
		if (pkt->pts != AV_NOPTS_VALUE && pkt->pts < pkt->dts)
		{
			if (g_nLogLevel > 0)
			{
				::LogLine(_T("Stream%u: PTS adjusted %I64d -> %I64d"),
							dwStreamNum, pkt->pts, pkt->dts);
			}
			pkt->pts = pkt->dts;
		}
	}
	m_llLastStreamDTS[dwStreamNum] = pkt->dts;
}

bool CAVRec::EncodeAndWriteFrame(DWORD dwStreamNum,
								AVFrame* pFrame, // set to NULL to flush
								bool bInterleaved)
{
	AVPacket pkt = { 0 }; // data and size must be 0
	av_init_packet(&pkt);
	int got_packet = 0;
	AVCodecContext* pCodecCtx = m_pFormatCtx->streams[dwStreamNum]->codec;

	// Encode
	int ret = -1;
	if (pCodecCtx->codec_type == AVMEDIA_TYPE_VIDEO)
		ret = avcodec_encode_video2(pCodecCtx, &pkt, pFrame, &got_packet);
	else if (pCodecCtx->codec_type == AVMEDIA_TYPE_AUDIO)
		ret = avcodec_encode_audio2(pCodecCtx, &pkt, pFrame, &got_packet);
	if (ret < 0)
	{
		av_free_packet(&pkt);
		return false;
	}
	else if (pFrame)
	{
		if (pCodecCtx->codec_type == AVMEDIA_TYPE_VIDEO)
			m_llTotalFramesOrSamples[dwStreamNum]++;
		else if (pCodecCtx->codec_type == AVMEDIA_TYPE_AUDIO)
			m_llTotalFramesOrSamples[dwStreamNum] += pFrame->nb_samples;
	}
	if (!got_packet)
	{
		av_free_packet(&pkt);
		if (pFrame)
			return true;
		else
			return false; // nothing more to flush
	}

	// Write the packet to the media file
	AdjustPTSDTS(dwStreamNum, &pkt);
	ret =	bInterleaved ?
			av_interleaved_write_frame(m_pFormatCtx, &pkt) :
			av_write_frame(m_pFormatCtx, &pkt);
	if (ret < 0)
	{
		av_free_packet(&pkt);
		return false;
	}
	else
	{
		m_llTotalWrittenBytes[dwStreamNum] += pkt.size;
		av_free_packet(&pkt);
		return true;
	}
}

#if 0
int CAVRec::CalcSrcResampleDelay(DWORD dwStreamNum)
{
	// Init vars
	const int nSrcNumSamples = 256;
	const int nDstNumSamples = 4096;
	AVCodecContext* pCodecCtx = m_pFormatCtx->streams[dwStreamNum]->codec;
	uint8_t** ppSrcBuf[MAX_STREAMS];
	uint8_t** ppDstBuf[MAX_STREAMS];
	AVSampleFormat src_sample_fmt = m_pSrcWaveFormat[dwStreamNum]->wBitsPerSample == 16 ? AV_SAMPLE_FMT_S16 : AV_SAMPLE_FMT_U8;
	av_samples_alloc_array_and_samples(	&ppSrcBuf[dwStreamNum],
										NULL,
										m_pSrcWaveFormat[dwStreamNum]->nChannels,
										nSrcNumSamples,
										src_sample_fmt,
										0);
	av_samples_alloc_array_and_samples(	&ppDstBuf[dwStreamNum],
										NULL,
										pCodecCtx->channels,
										nDstNumSamples,
										pCodecCtx->sample_fmt,
										0);

	// How many samples remain buffer?
	swr_convert(m_pAudioConvertCtx[dwStreamNum],
				ppDstBuf[dwStreamNum], nDstNumSamples,
				(const uint8_t **)ppSrcBuf[dwStreamNum], nSrcNumSamples);
	int nBufferedSrcSamples = (int)swr_get_delay(m_pAudioConvertCtx[dwStreamNum], m_pSrcWaveFormat[dwStreamNum]->nSamplesPerSec);

	// Clean-up
	if (ppSrcBuf[dwStreamNum])
	{
		if (*ppSrcBuf[dwStreamNum])
			av_freep(ppSrcBuf[dwStreamNum]);
		av_freep(&ppSrcBuf[dwStreamNum]);
	}
	if (ppDstBuf[dwStreamNum])
	{
		if (*ppDstBuf[dwStreamNum])
			av_freep(ppDstBuf[dwStreamNum]);
		av_freep(&ppDstBuf[dwStreamNum]);
	}

	return nBufferedSrcSamples;
}
#endif

bool CAVRec::AddAudioSamples(	DWORD dwStreamNum,
								DWORD dwNumSamples,
								LPBYTE pBuf,
								bool bInterleaved)
{
	::EnterCriticalSection(&m_csAV);

	// Check
	if (!m_pFormatCtx								||
		dwStreamNum >= m_pFormatCtx->nb_streams		||
		!m_pAudioConvertCtx[dwStreamNum]			||
		!m_pFormatCtx->streams[dwStreamNum]			||
		!m_pFormatCtx->streams[dwStreamNum]->codec	||
		m_pFormatCtx->streams[dwStreamNum]->codec->codec_type != AVMEDIA_TYPE_AUDIO)
	{
		::LeaveCriticalSection(&m_csAV);
		return false;
	}

	// Flush?
	BOOL bFlush = FALSE;
	if (dwNumSamples == 0 || pBuf == NULL)
		bFlush = TRUE;

	// Re-alloc source buffer?
	AVSampleFormat src_sample_fmt = m_pSrcWaveFormat[dwStreamNum]->wBitsPerSample == 16 ? AV_SAMPLE_FMT_S16 : AV_SAMPLE_FMT_U8;
	if ((int)dwNumSamples > m_nSrcBufSize[dwStreamNum])
	{
		int ret;
		if (m_ppSrcBuf[dwStreamNum])
		{
			if (*m_ppSrcBuf[dwStreamNum])
				av_freep(m_ppSrcBuf[dwStreamNum]);
			ret = av_samples_alloc(	m_ppSrcBuf[dwStreamNum],
									NULL,
									m_pSrcWaveFormat[dwStreamNum]->nChannels,
									(int)dwNumSamples,
									src_sample_fmt,
									0);
		}
		else
		{
			ret = av_samples_alloc_array_and_samples(
									&m_ppSrcBuf[dwStreamNum],
									NULL,
									m_pSrcWaveFormat[dwStreamNum]->nChannels,
									(int)dwNumSamples,
									src_sample_fmt,
									0);
		}
		if (ret < 0)
		{
			::LeaveCriticalSection(&m_csAV);
			return false;
		}
		else
			m_nSrcBufSize[dwStreamNum] = (int)dwNumSamples; // number of samples in one channel
	}

	// Copy input samples to source buffer
	// (use an input buffer because it is aligned for the conversion)
	int nCopyBytes = m_pSrcWaveFormat[dwStreamNum]->nBlockAlign * (int)dwNumSamples;
	if (nCopyBytes > 0 && pBuf)
		memcpy(*m_ppSrcBuf[dwStreamNum], pBuf, nCopyBytes);

	// Encode and Write frame by frame
	// Note: swr_convert() buffers samples if you provide insufficient
	//       output space or if sample rate conversion is done
	AVCodecContext* pCodecCtx = m_pFormatCtx->streams[dwStreamNum]->codec;
	while (TRUE)
	{
		// Make sure we have enough source samples to get a
		// destination frame of m_nDstBufSize[dwStreamNum] samples
		if (!bFlush)
		{
			// Calculate the source samples which would be used if converting
			// Note: nResampleDelay value tested with CalcSrcResampleDelay()
			int nBufferedSrcSamples = (int)swr_get_delay(m_pAudioConvertCtx[dwStreamNum], m_pSrcWaveFormat[dwStreamNum]->nSamplesPerSec);
			int nResampleDelay = 0;
			if (pCodecCtx->sample_rate > (int)m_pSrcWaveFormat[dwStreamNum]->nSamplesPerSec)		// upsampling
				nResampleDelay = 16 + 4; // 16 bytes + rounding margin
			else if (pCodecCtx->sample_rate < (int)m_pSrcWaveFormat[dwStreamNum]->nSamplesPerSec)	// downsampling
				nResampleDelay = 16 * m_pSrcWaveFormat[dwStreamNum]->nSamplesPerSec / pCodecCtx->sample_rate + 4; // 16 bytes * sampling-ratio + rounding margin
			int nUsableSrcSamples = nBufferedSrcSamples + (int)dwNumSamples - nResampleDelay;

			// Calculate the usable destination samples from the source ones
			int nUsableDstSamples = 0;
			if (nUsableSrcSamples > 0)
			{
				nUsableDstSamples = (int)av_rescale_rnd(nUsableSrcSamples,
														pCodecCtx->sample_rate,
														m_pSrcWaveFormat[dwStreamNum]->nSamplesPerSec,
														AV_ROUND_UP);
			}

			// If not enough samples available then buffer the passed ones and exit
			if (nUsableDstSamples < m_nDstBufSize[dwStreamNum])
			{
				if (dwNumSamples > 0)
				{
					swr_convert(m_pAudioConvertCtx[dwStreamNum],
								NULL, 0,
								(const uint8_t **)m_ppSrcBuf[dwStreamNum], (int)dwNumSamples);
				}
				::LeaveCriticalSection(&m_csAV);
				return true;
			}
		}
	
		// Convert to destination format
		int nConvertedSamples = swr_convert(m_pAudioConvertCtx[dwStreamNum],
											m_ppDstBuf[dwStreamNum], m_nDstBufSize[dwStreamNum],
											(const uint8_t **)(bFlush ? NULL : m_ppSrcBuf[dwStreamNum]), (int)(bFlush ? 0 : dwNumSamples));
		if (nConvertedSamples < 0)
		{
			::LeaveCriticalSection(&m_csAV);
			return false;
		}
		else if (nConvertedSamples > 0)
		{
#ifdef _DEBUG
			if (!bFlush)
				ASSERT(nConvertedSamples == m_nDstBufSize[dwStreamNum]);
#endif
			m_pFrame[dwStreamNum]->nb_samples = nConvertedSamples;
			AVRational rational = {1, pCodecCtx->sample_rate};
			m_pFrame[dwStreamNum]->pts = av_rescale_q(m_llTotalFramesOrSamples[dwStreamNum], rational, pCodecCtx->time_base);
			int nDstBufSizeInBytes = av_samples_get_buffer_size(NULL, pCodecCtx->channels, m_nDstBufSize[dwStreamNum], pCodecCtx->sample_fmt, 0);
			avcodec_fill_audio_frame(	m_pFrame[dwStreamNum], pCodecCtx->channels, pCodecCtx->sample_fmt,
										*m_ppDstBuf[dwStreamNum], nDstBufSizeInBytes, 0);
		}

		// Encode and write samples to file, if nConvertedSamples is 0 -> flush
		if (!EncodeAndWriteFrame(dwStreamNum, nConvertedSamples > 0 ? m_pFrame[dwStreamNum] : NULL, bInterleaved))
		{
			::LeaveCriticalSection(&m_csAV);
			return false;
		}

		// With next loop get the swr buffered samples
		dwNumSamples = 0;
	}

	// Should never reach this point
	ASSERT(FALSE);
	::LeaveCriticalSection(&m_csAV);
	return false;
}

CString CAVRec::FourCCToString(DWORD dwFourCC)
{
	char ch0 = (char)(dwFourCC & 0xFF);
	char ch1 = (char)((dwFourCC >> 8) & 0xFF);
	char ch2 = (char)((dwFourCC >> 16) & 0xFF);
	char ch3 = (char)((dwFourCC >> 24) & 0xFF);
	WCHAR wch0, wch1, wch2, wch3;
	mbtowc(&wch0, &ch0, sizeof(WCHAR));
	mbtowc(&wch1, &ch1, sizeof(WCHAR));
	mbtowc(&wch2, &ch2, sizeof(WCHAR));
	mbtowc(&wch3, &ch3, sizeof(WCHAR));
	return (CString(wch0) + CString(wch1) + CString(wch2) + CString(wch3));
}

CString CAVRec::FourCCToStringUpperCase(DWORD dwFourCC)
{
	CString sFourCC = FourCCToString(dwFourCC);
	sFourCC.MakeUpper();
	return sFourCC;
}

enum AVPixelFormat CAVRec::AVCodecBMIToPixFormat(LPBITMAPINFO pBMI)
{
	if (pBMI)
	{
		if (pBMI->bmiHeader.biCompression == FCC('YV12')			||
			pBMI->bmiHeader.biCompression == FCC('I420')			||
			pBMI->bmiHeader.biCompression == FCC('IYUV'))
			return AV_PIX_FMT_YUV420P;	// For YV12 we have to invert the planes!
		else if (pBMI->bmiHeader.biCompression == FCC('J420'))
			return AV_PIX_FMT_YUVJ420P;
		else if (pBMI->bmiHeader.biCompression == FCC('NV12'))
			return AV_PIX_FMT_NV12;
		else if (pBMI->bmiHeader.biCompression == FCC('NV21'))
			return AV_PIX_FMT_NV21;
		else if (	pBMI->bmiHeader.biCompression == FCC('YUY2')	||
					pBMI->bmiHeader.biCompression == FCC('YUNV')	||
					pBMI->bmiHeader.biCompression == FCC('VYUY')	||
					pBMI->bmiHeader.biCompression == FCC('V422')	||
					pBMI->bmiHeader.biCompression == FCC('YUYV'))
			return AV_PIX_FMT_YUYV422;
		else if (	pBMI->bmiHeader.biCompression == FCC('UYVY')	||
					pBMI->bmiHeader.biCompression == FCC('Y422')	||
					pBMI->bmiHeader.biCompression == FCC('UYNV'))
			return AV_PIX_FMT_UYVY422;
		else if (	pBMI->bmiHeader.biCompression == FCC('YUV9')	||
					pBMI->bmiHeader.biCompression == FCC('YVU9'))
			return AV_PIX_FMT_YUV410P;	// For YVU9 we have to invert the planes!
		else if (pBMI->bmiHeader.biCompression == FCC('Y41B'))
			return AV_PIX_FMT_YUV411P;
		else if (	pBMI->bmiHeader.biCompression == FCC('YV16')	||
					pBMI->bmiHeader.biCompression == FCC('Y42B'))
			return AV_PIX_FMT_YUV422P; // For YV16 we have to invert the planes!
		else if (pBMI->bmiHeader.biCompression == FCC('J422'))
			return AV_PIX_FMT_YUVJ422P;
		else if (	pBMI->bmiHeader.biCompression == FCC('Y800')	||
					pBMI->bmiHeader.biCompression == FCC('  Y8')	||
					pBMI->bmiHeader.biCompression == FCC('Y8  ')	||
					pBMI->bmiHeader.biCompression == FCC('GREY'))
			return AV_PIX_FMT_GRAY8;
		else if (pBMI->bmiHeader.biCompression == mmioFOURCC('R','G','B',15))
			return AV_PIX_FMT_RGB555;
		else if (pBMI->bmiHeader.biCompression == mmioFOURCC('B','G','R',15))
			return AV_PIX_FMT_BGR555;
		else if (pBMI->bmiHeader.biCompression == mmioFOURCC('R','G','B',16))
			return AV_PIX_FMT_RGB565;
		else if (pBMI->bmiHeader.biCompression == mmioFOURCC('B','G','R',16))
			return AV_PIX_FMT_BGR565;
		else if (pBMI->bmiHeader.biCompression == BI_RGB)
		{
			switch (pBMI->bmiHeader.biBitCount)
			{
				case 8  : return AV_PIX_FMT_PAL8;
				case 16 : return AV_PIX_FMT_RGB555;
				case 24 : return AV_PIX_FMT_BGR24;
				case 32 : return AV_PIX_FMT_RGB32;
				default : return AV_PIX_FMT_NONE;
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
						return AV_PIX_FMT_RGB565;
					else
						return AV_PIX_FMT_RGB555;
				}
				case 32 : return AV_PIX_FMT_RGB32;
				default : return AV_PIX_FMT_NONE;
			}
		}
		else
			return AV_PIX_FMT_NONE;
	}
	else
		return AV_PIX_FMT_NONE;
}

enum AVCodecID CAVRec::AVCodecFourCCToCodecID(DWORD dwFourCC)
{
	CString sFourCC = FourCCToStringUpperCase(dwFourCC);
	if (sFourCC == _T("MJPG")	||
		sFourCC == _T("M601"))	// contaware introduced this fourcc to distinguish the unofficial jpeg ITU601 color space
		return AV_CODEC_ID_MJPEG;
	else if (sFourCC == _T("H264"))
		return AV_CODEC_ID_H264;
	else if (sFourCC == _T("DIVX"))
		return AV_CODEC_ID_MPEG4;
	else
		return AV_CODEC_ID_NONE;
}

enum AVCodecID CAVRec::AVCodecFormatTagToCodecID(WORD wFormatTag, int nPcmBits/*=16*/)
{
	switch (wFormatTag)
	{
		case WAVE_FORMAT_PCM :					return nPcmBits == 16 ? AV_CODEC_ID_PCM_S16LE : AV_CODEC_ID_PCM_U8;
		case WAVE_FORMAT_MPEGLAYER3 :			return AV_CODEC_ID_MP3;
		case WAVE_FORMAT_AAC2 :					return AV_CODEC_ID_AAC;
		case WAVE_FORMAT_DVI_ADPCM :			return AV_CODEC_ID_ADPCM_IMA_WAV;
		default :								return AV_CODEC_ID_NONE;
	}
}

float CAVRec::ClipVideoQuality(float fQuality)
{
	if (fQuality > VIDEO_QUALITY_LOW)
		fQuality = VIDEO_QUALITY_LOW;
	else if (fQuality < VIDEO_QUALITY_BEST)
		fQuality = VIDEO_QUALITY_BEST;
	return fQuality;
}

#endif