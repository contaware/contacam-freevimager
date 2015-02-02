#include "stdafx.h"
#include "AVRec.h"
#include "Round.h"
#include "Helpers.h"
#include "AviPlay.h"
#include "YuvToYuv.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Defined in uImager.cpp
int avcodec_open_thread_safe(AVCodecContext *avctx, AVCodec *codec);
int avcodec_close_thread_safe(AVCodecContext *avctx);

CAVRec::CAVRec()
{
	InitVars();
}

CAVRec::CAVRec(	LPCTSTR lpszFileName,
				bool bFastEncode/*=false*/)
{
	InitVars();
	Init(lpszFileName, bFastEncode);
}

void CAVRec::InitVars()
{
	::InitializeCriticalSection(&m_csAVI);

	m_pOutputFormat = NULL;
	m_pFormatCtx = NULL;
	m_sFileName = _T("");
	m_dwTotalVideoStreams = 0;
	m_dwTotalAudioStreams = 0;
	m_bFileOpened = false;
	m_bOpen = false;
	m_bFastEncode = false;
	
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
	::DeleteCriticalSection(&m_csAVI);
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

bool CAVRec::Init(	LPCTSTR lpszFileName,
					bool bFastEncode/*=false*/)
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
	m_bFastEncode = bFastEncode;
	
	// Allocate the output media context
	avformat_alloc_output_context2(&m_pFormatCtx, NULL, NULL, filename);
	if (!m_pFormatCtx)
		avformat_alloc_output_context2(&m_pFormatCtx, NULL, "avi", filename);
	if (!m_pFormatCtx)
		return false;
	m_pOutputFormat = m_pFormatCtx->oformat;

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
	m_pOutputFormat->video_codec = CAVIPlay::CAVIVideoStream::AVCodecFourCCToCodecID(pDstFormat->bmiHeader.biCompression);
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
		pix_fmt = CAVIPlay::CAVIVideoStream::AVCodecBMIToPixFormat(pDstFormat);
	else
	{
		pix_fmt = CAVIPlay::CAVIVideoStream::AVCodecBMIToPixFormat(pSrcFormat);
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
		// "placebo", "veryslow", "slower", "slow", "medium", "fast", "faster", "veryfast", "superfast", "ultrafast"
		if (m_bFastEncode)
			av_opt_set(pCodecCtx->priv_data, "preset", "ultrafast", 0);
		else
			av_opt_set(pCodecCtx->priv_data, "preset", "veryfast", 0);
	}
	else
	{
		// Quality: 2.0f best quality, 31.0f worst quality
        pCodecCtx->flags |= CODEC_FLAG_QSCALE;
		pCodecCtx->global_quality = (int)(FF_QP2LAMBDA * qscale);

		// Slow encoding
		if (!m_bFastEncode)
		{
			if (pCodecCtx->codec_id == AV_CODEC_ID_MPEG4)
			{
				pCodecCtx->mb_decision = 2;							// mbd:    macroblock decision mode
				pCodecCtx->me_cmp = 2;								// cmp:    fullpixel motion estimation compare function
				pCodecCtx->me_sub_cmp = 2;							// subcmp: subpixel motion estimation compare function
				pCodecCtx->trellis = 1;								// trell:  enable trellis quantization
				pCodecCtx->flags |= (CODEC_FLAG_AC_PRED |			// aic:    MPEG-4 AC prediction
									CODEC_FLAG_4MV);				// mv4:    4 MV per MB allowed
			}
			else if (	pCodecCtx->codec_id == AV_CODEC_ID_H263  ||
						pCodecCtx->codec_id == AV_CODEC_ID_H263P ||
						pCodecCtx->codec_id == AV_CODEC_ID_FLV1)
			{
				pCodecCtx->mb_decision = 2;							// mbd:    macroblock decision mode
				pCodecCtx->me_cmp = 2;								// cmp:    fullpixel motion estimation compare function
				pCodecCtx->me_sub_cmp = 2;							// subcmp: subpixel motion estimation compare function
				pCodecCtx->trellis = 1;								// trell:  enable trellis quantization
				pCodecCtx->flags |= (CODEC_FLAG_AC_PRED		|		// aic:    H.263 advanced intra coding
									CODEC_FLAG_4MV			|		// mv4:    advanced prediction for H.263
									CODEC_FLAG_MV0			|		// mv0:    try to encode each MB with MV=<0,0> and choose the better one (has no effect if mb_decision=0)
									CODEC_FLAG_LOOP_FILTER);		// lf:     use loop filter (h263+)
			}
		}
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
	m_pOutputFormat->audio_codec = CAVIPlay::CAVIAudioStream::AVCodecFormatTagToCodecID(pDstWaveFormat->wFormatTag,
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

	::EnterCriticalSection(&m_csAVI);

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
	m_bFastEncode = false;
	m_bOpen = false;
	m_bFileOpened = false;

	::LeaveCriticalSection(&m_csAVI);

	return res;
}

bool CAVRec::AddFrame(	DWORD dwStreamNum,
						LPBITMAPINFO pBmi,
						LPBYTE pBits,
						bool bInterleaved,
						int64_t pts/*=AV_NOPTS_VALUE*/)
{
	::EnterCriticalSection(&m_csAVI);

	// Check
	if (!m_pFormatCtx								||
		dwStreamNum >= m_pFormatCtx->nb_streams		||
		!m_pFormatCtx->streams[dwStreamNum]			||
		!m_pFormatCtx->streams[dwStreamNum]->codec	||
		m_pFormatCtx->streams[dwStreamNum]->codec->codec_type != AVMEDIA_TYPE_VIDEO)
	{
		::LeaveCriticalSection(&m_csAVI);
		return false;
	}

	// Skip frame if not strictly monotonically increasing pts
	if (pts != AV_NOPTS_VALUE							&&
		m_llLastCodecPTS[dwStreamNum] != AV_NOPTS_VALUE	&&
		pts <= m_llLastCodecPTS[dwStreamNum])
	{
		::LeaveCriticalSection(&m_csAVI);
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
		enum AVPixelFormat SrcPixFormat = CAVIPlay::CAVIVideoStream::AVCodecBMIToPixFormat(pBmi);
		if (SrcPixFormat == AV_PIX_FMT_NONE)
		{
			::LeaveCriticalSection(&m_csAVI);
			return false;
		}

		// Copy input bits to source buffer
		// (use an input buffer because it is aligned for the conversion)
		if (m_ppSrcBuf[dwStreamNum] == NULL)
		{
			m_ppSrcBuf[dwStreamNum] = (uint8_t**)av_mallocz(sizeof(uint8_t*));
			if (m_ppSrcBuf[dwStreamNum] == NULL)
			{
				::LeaveCriticalSection(&m_csAVI);
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
				::LeaveCriticalSection(&m_csAVI);
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
			// Get conversion context
			m_pImgConvertCtx[dwStreamNum] = sws_getCachedContext(m_pImgConvertCtx[dwStreamNum],	// Re-use if already allocated
																pBmi->bmiHeader.biWidth,		// Source Width
																pBmi->bmiHeader.biHeight,		// Source Height
																SrcPixFormat,					// Source Format
																pCodecCtx->width,				// Destination Width
																pCodecCtx->height,				// Destination Height
																pCodecCtx->pix_fmt,				// Destination Format
																SWS_BICUBIC,					// Interpolation
																NULL,							// No Source Filter
																NULL,							// No Destination Filter
																NULL);							// Param
			if (!m_pImgConvertCtx[dwStreamNum])
			{
				::LeaveCriticalSection(&m_csAVI);
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
						::LeaveCriticalSection(&m_csAVI);
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
					::LeaveCriticalSection(&m_csAVI);
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
					::LeaveCriticalSection(&m_csAVI);
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
					::LeaveCriticalSection(&m_csAVI);
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
						::LeaveCriticalSection(&m_csAVI);
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
	::LeaveCriticalSection(&m_csAVI);
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
					TRACE(_T("Stream%u: DTS adjusted non-strictly monotonically %I64d -> %I64d\n"),
							dwStreamNum, pkt->dts, m_llLastStreamDTS[dwStreamNum]);
					pkt->dts = m_llLastStreamDTS[dwStreamNum];
				}
			}
			// Format needs strictly monotonically increasing dts: -10,0,1,2,7,10,15,...
			else
			{
				if (pkt->dts <= m_llLastStreamDTS[dwStreamNum])
				{
					TRACE(_T("Stream%u: DTS adjusted strictly monotonically %I64d -> %I64d\n"),
							dwStreamNum, pkt->dts, m_llLastStreamDTS[dwStreamNum] + 1);
					pkt->dts = m_llLastStreamDTS[dwStreamNum] + 1;
				}
			}
		}
		if (pkt->pts != AV_NOPTS_VALUE && pkt->pts < pkt->dts)
		{
			TRACE(_T("Stream%u: PTS adjusted %I64d -> %I64d\n"),
						dwStreamNum, pkt->pts, pkt->dts);
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

bool CAVRec::AddAudioSamples(	DWORD dwStreamNum,
								DWORD dwNumSamples,
								LPBYTE pBuf,
								bool bInterleaved)
{
	::EnterCriticalSection(&m_csAVI);

	// Check
	if (!m_pFormatCtx								||
		dwStreamNum >= m_pFormatCtx->nb_streams		||
		!m_pAudioConvertCtx[dwStreamNum]			||
		!m_pFormatCtx->streams[dwStreamNum]			||
		!m_pFormatCtx->streams[dwStreamNum]->codec	||
		m_pFormatCtx->streams[dwStreamNum]->codec->codec_type != AVMEDIA_TYPE_AUDIO)
	{
		::LeaveCriticalSection(&m_csAVI);
		return false;
	}

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
			::LeaveCriticalSection(&m_csAVI);
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
	bool res;
	int nBufferedDstSamples;
	AVCodecContext* pCodecCtx = m_pFormatCtx->streams[dwStreamNum]->codec;
	do
	{
		// Convert to destination format
		// in and in_count can be set to 0 to flush the last few samples out at the end.
		// The samples may get buffered in swr if you provide insufficient
		// output space or if sample rate conversion is done, which requires "future" samples
		int nConvertedSamples = swr_convert(m_pAudioConvertCtx[dwStreamNum],
											m_ppDstBuf[dwStreamNum], m_nDstBufSize[dwStreamNum],
											(const uint8_t **)m_ppSrcBuf[dwStreamNum], (int)dwNumSamples);
		if (nConvertedSamples < 0)
		{
			::LeaveCriticalSection(&m_csAVI);
			return false;
		}
		else if (nConvertedSamples > 0)
		{
			m_pFrame[dwStreamNum]->nb_samples = nConvertedSamples;
			AVRational rational = {1, pCodecCtx->sample_rate};
			m_pFrame[dwStreamNum]->pts = av_rescale_q(m_llTotalFramesOrSamples[dwStreamNum], rational, pCodecCtx->time_base);
			int nDstBufSizeInBytes = av_samples_get_buffer_size(NULL, pCodecCtx->channels, m_nDstBufSize[dwStreamNum], pCodecCtx->sample_fmt, 0);
			avcodec_fill_audio_frame(	m_pFrame[dwStreamNum], pCodecCtx->channels, pCodecCtx->sample_fmt,
										*m_ppDstBuf[dwStreamNum], nDstBufSizeInBytes, 0);
		}

		// Encode and write samples to file, if nConvertedSamples is 0 -> flush
		res = EncodeAndWriteFrame(dwStreamNum, nConvertedSamples > 0 ? m_pFrame[dwStreamNum] : NULL, bInterleaved);
		dwNumSamples = 0; // with next loop get the swr buffered samples
		nBufferedDstSamples = (int)swr_get_delay(m_pAudioConvertCtx[dwStreamNum], pCodecCtx->sample_rate); // delay is in output samples unit
	}
	while (nBufferedDstSamples >= m_nDstBufSize[dwStreamNum]);

	::LeaveCriticalSection(&m_csAVI);

	return res;
}
