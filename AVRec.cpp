#include "stdafx.h"
#include "AVRec.h"
#include "Round.h"
#include "Helpers.h"
#include "AviPlay.h"

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
				int nPassNumber/*=0*/,
				bool bFastEncode/*=false*/)
{
	InitVars();
	Init(lpszFileName, nPassNumber, bFastEncode);
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
	m_nGlobalPassNumber = 0;
	m_bFastEncode = false;
	
	for (DWORD dwStreamNum = 0 ; dwStreamNum < MAX_STREAMS ; dwStreamNum++)
	{
		m_bCodecOpened[dwStreamNum] = false;
		m_pFrame[dwStreamNum] = NULL;
		m_pFrameTemp[dwStreamNum] = NULL;
		m_pImgConvertCtx[dwStreamNum] = NULL;
		m_pFrameBuf1[dwStreamNum] = NULL;
		m_nFrameBufSize1[dwStreamNum] = 0;
		m_pFrameBuf2[dwStreamNum] = NULL;
		m_nFrameBufSize2[dwStreamNum] = 0;
		m_p2PassLogFiles[dwStreamNum] = NULL;
		m_nPassNumber[dwStreamNum] = 0;
		
		m_nAudioInputFrameSize[dwStreamNum] = 0;
		m_pIntermediateSamplesBuf[dwStreamNum] = NULL;
		m_nIntermediateSamplesBufSize[dwStreamNum] = 0;
		m_nIntermediateSamplesBufPos[dwStreamNum] = 0;
		m_pTempSamplesBuf[dwStreamNum] = NULL;
		m_nTempSamplesBufSize[dwStreamNum] = 0;
		m_pTempSamplesBuf2[dwStreamNum] = NULL;
		m_nTempSamplesBufSize2[dwStreamNum] = 0;
		m_pAudioResampleCtx[dwStreamNum] = NULL;

		m_pOutbuf[dwStreamNum] = NULL;
		m_nOutbufSize[dwStreamNum] = 0;
		
		m_llTotalWrittenBytes[dwStreamNum] = 0;
		m_llTotalFramesOrSamples[dwStreamNum] = 0;
		m_pSrcWaveFormat[dwStreamNum] = NULL;
		m_pIntermediateWaveFormat[dwStreamNum] = NULL;
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

__forceinline void CAVRec::SetIntermediateWaveFormat(DWORD dwStreamNum, const LPWAVEFORMATEX pWaveFormat)					
{
	if (pWaveFormat == NULL)
		return;

	if (m_pIntermediateWaveFormat[dwStreamNum])
		delete [] m_pIntermediateWaveFormat[dwStreamNum];

	int nWaveFormatSize;
	if (pWaveFormat->wFormatTag == WAVE_FORMAT_PCM)
		nWaveFormatSize = sizeof(WAVEFORMATEX);
	else
		nWaveFormatSize = sizeof(WAVEFORMATEX) + pWaveFormat->cbSize;
 
	m_pIntermediateWaveFormat[dwStreamNum] = (LPWAVEFORMATEX)new BYTE[nWaveFormatSize];
	memcpy(m_pIntermediateWaveFormat[dwStreamNum], pWaveFormat, nWaveFormatSize);
}

AVStream* CAVRec::CreateAudioStream(CodecID codec_id,
									SampleFormat sample_fmt,
									int tag,
									int bitrate,
									int samplerate,
									int channels)
{
    AVCodecContext* pCodecCtx;
    AVStream* pStream = av_new_stream(m_pFormatCtx, 1);
    if (!pStream)
	{
        TRACE(_T("Could not alloc audio stream\n"));
        return NULL;
    }

    pCodecCtx = pStream->codec;
    pCodecCtx->codec_id = codec_id;
    pCodecCtx->codec_type = CODEC_TYPE_AUDIO;

    // Put sample parameters
	pCodecCtx->sample_fmt = sample_fmt;
    pCodecCtx->bit_rate = bitrate;
    pCodecCtx->sample_rate = samplerate;
    pCodecCtx->channels = channels;
	pCodecCtx->codec_tag = tag;

    return pStream;
}

AVStream* CAVRec::CreateVideoStream(CodecID codec_id,
									const LPBITMAPINFO pDstVideoFormat,
									DWORD dwRate,
									DWORD dwScale,
									PixelFormat pix_fmt,
									int bitrate,
									int keyframes_rate,
									float qscale)	// 0.0f use bitrate, 2.0f best quality, 31.0f worst quality
{
    AVCodecContext* pCodecCtx;
    AVStream* pStream = av_new_stream(m_pFormatCtx, 0);
    if (!pStream)
	{
        TRACE(_T("Could not alloc video stream\n"));
        return NULL;
    }

    pCodecCtx = pStream->codec;
    pCodecCtx->codec_id = codec_id;
    pCodecCtx->codec_type = CODEC_TYPE_VIDEO;
	pCodecCtx->codec_tag = pDstVideoFormat->bmiHeader.biCompression;

	// Reduce Rate / Scale rational
	int dst_rate, dst_scale;
	av_reduce(&dst_scale, &dst_rate, (int64_t)dwScale, (int64_t)dwRate, MAX_SIZE_FOR_RATIONAL);
	if (codec_id == CODEC_ID_RAWVIDEO)
	{
		switch (pDstVideoFormat->bmiHeader.biCompression)
		{
			case BI_RLE8 :
			{
				int nNumColors = pDstVideoFormat->bmiHeader.biClrUsed;
				if (nNumColors == 0)
					nNumColors = 256;
				pix_fmt = PIX_FMT_PAL8;
				pCodecCtx->extradata_size = nNumColors*sizeof(RGBQUAD);
				pCodecCtx->extradata = (uint8_t*)av_malloc(pCodecCtx->extradata_size + FF_INPUT_BUFFER_PADDING_SIZE);
				if (pCodecCtx->extradata)
					memcpy(pCodecCtx->extradata, pDstVideoFormat->bmiColors, nNumColors*sizeof(RGBQUAD));
				pCodecCtx->bits_per_coded_sample = 8;
				break;
			}
			case BI_RLE4 :
			{
				int nNumColors = pDstVideoFormat->bmiHeader.biClrUsed;
				if (nNumColors == 0)
					nNumColors = 16;
				pix_fmt = PIX_FMT_PAL8; // There is no PIX_FMT_PAL4 ... it works because we only raw write video data
				pCodecCtx->extradata_size = nNumColors*sizeof(RGBQUAD);
				pCodecCtx->extradata = (uint8_t*)av_malloc(pCodecCtx->extradata_size + FF_INPUT_BUFFER_PADDING_SIZE);
				if (pCodecCtx->extradata)
					memcpy(pCodecCtx->extradata, pDstVideoFormat->bmiColors, nNumColors*sizeof(RGBQUAD));
				pCodecCtx->bits_per_coded_sample = 4;
				break;
			}
			case BI_RGB :
			{
				if (pDstVideoFormat->bmiHeader.biBitCount == 24)
				{
					pix_fmt = PIX_FMT_BGR24;
					pCodecCtx->bits_per_coded_sample = 24;
				}
				else if (pDstVideoFormat->bmiHeader.biBitCount == 32)
				{
					pix_fmt = PIX_FMT_RGB32;
					pCodecCtx->bits_per_coded_sample = 32;
				}
				else if (pDstVideoFormat->bmiHeader.biBitCount == 16)
				{
					pix_fmt = PIX_FMT_RGB555;
					pCodecCtx->bits_per_coded_sample = 16;
				}
				else if (pDstVideoFormat->bmiHeader.biBitCount == 8)
				{
					int nNumColors = pDstVideoFormat->bmiHeader.biClrUsed;
					if (nNumColors == 0)
						nNumColors = 256;
					pix_fmt = PIX_FMT_PAL8;
					pCodecCtx->extradata_size = nNumColors*sizeof(RGBQUAD);
					pCodecCtx->extradata = (uint8_t*)av_malloc(pCodecCtx->extradata_size + FF_INPUT_BUFFER_PADDING_SIZE);
					if (pCodecCtx->extradata)
						memcpy(pCodecCtx->extradata, pDstVideoFormat->bmiColors, nNumColors*sizeof(RGBQUAD));
					pCodecCtx->bits_per_coded_sample = 8;
				}
				else if (pDstVideoFormat->bmiHeader.biBitCount == 4)
				{
					int nNumColors = pDstVideoFormat->bmiHeader.biClrUsed;
					if (nNumColors == 0)
						nNumColors = 16;
					pix_fmt = PIX_FMT_PAL8; // There is no PIX_FMT_PAL4 ... it works because we only raw write video data
					pCodecCtx->extradata_size = nNumColors*sizeof(RGBQUAD);
					pCodecCtx->extradata = (uint8_t*)av_malloc(pCodecCtx->extradata_size + FF_INPUT_BUFFER_PADDING_SIZE);
					if (pCodecCtx->extradata)
						memcpy(pCodecCtx->extradata, pDstVideoFormat->bmiColors, nNumColors*sizeof(RGBQUAD));
					pCodecCtx->bits_per_coded_sample = 4;
				}
				else if (pDstVideoFormat->bmiHeader.biBitCount == 1)
				{
					int nNumColors = pDstVideoFormat->bmiHeader.biClrUsed;
					if (nNumColors == 0)
						nNumColors = 2;
					pix_fmt = PIX_FMT_PAL8; // There is no PIX_FMT_PAL1 ... it works because we only raw write video data
					pCodecCtx->extradata_size = nNumColors*sizeof(RGBQUAD);
					pCodecCtx->extradata = (uint8_t*)av_malloc(pCodecCtx->extradata_size + FF_INPUT_BUFFER_PADDING_SIZE);
					if (pCodecCtx->extradata)
						memcpy(pCodecCtx->extradata, pDstVideoFormat->bmiColors, nNumColors*sizeof(RGBQUAD));
					pCodecCtx->bits_per_coded_sample = 1;
				}
				break;
			}
			case BI_BITFIELDS :
			{
				if (pDstVideoFormat->bmiHeader.biBitCount == 32)
				{
					pix_fmt = PIX_FMT_RGB32;
					pCodecCtx->bits_per_coded_sample = 32;
				}
				else if (pDstVideoFormat->bmiHeader.biBitCount == 16)
				{
					LPBYTE pDstMask = (LPBYTE)pDstVideoFormat + sizeof(BITMAPINFOHEADER);
					if (*((DWORD*)(pDstMask)) == 0xF800					&&	// Red Mask
						*((DWORD*)(pDstMask+sizeof(DWORD))) == 0x07E0	&&	// Green Mask
						*((DWORD*)(pDstMask+2*sizeof(DWORD))) == 0x001F)	// Blue Mask
					{
						pix_fmt = PIX_FMT_RGB565;
						pCodecCtx->extradata_size = 3*sizeof(DWORD);
						pCodecCtx->extradata = (uint8_t*)av_malloc(pCodecCtx->extradata_size + FF_INPUT_BUFFER_PADDING_SIZE);
						if (pCodecCtx->extradata)
						{
							*((DWORD*)(pCodecCtx->extradata)) = 0xF800;
							*((DWORD*)(pCodecCtx->extradata+sizeof(DWORD))) = 0x07E0;
							*((DWORD*)(pCodecCtx->extradata+2*sizeof(DWORD))) = 0x001F;
						}
					}
					else
					{
						pCodecCtx->codec_tag = BI_RGB;
						pix_fmt = PIX_FMT_RGB555;
					}
					pCodecCtx->bits_per_coded_sample = 16;
				}
				break;
			}
			default :
			{
				pix_fmt = CAVIPlay::CAVIVideoStream::AVCodecBMIToPixFormat(pDstVideoFormat);
				pCodecCtx->bits_per_coded_sample = ::FourCCToBpp(pDstVideoFormat->bmiHeader.biCompression);
				break;
			}
		}
	}

	// Use Quality
	if (qscale > 0.0f)
	{
		// Clip: 31 comes from pCodecCtx->qmax, and 2 from pCodecCtx->qmin for MJPG
		if (qscale > 31.0f)
			qscale = 31.0f;
		else if (qscale < 2.0f)
			qscale = 2.0f;

		pCodecCtx->flags |= CODEC_FLAG_QSCALE;
		pStream->quality = FF_QP2LAMBDA * qscale;
		pCodecCtx->global_quality = (int)pStream->quality;
		pCodecCtx->bit_rate = 0;
	}
	// Use Bitrate
	else
		pCodecCtx->bit_rate = MAX(1, bitrate);

    // Resolution must be a multiple of two
    pCodecCtx->width = pDstVideoFormat->bmiHeader.biWidth;
    pCodecCtx->height = pDstVideoFormat->bmiHeader.biHeight;

    /* time base: this is the fundamental unit of time (in seconds) in terms
       of which frame timestamps are represented. for fixed-fps content,
       timebase should be 1/framerate and timestamp increments should be
       identically 1. */
    pCodecCtx->time_base.den = dst_rate;
    pCodecCtx->time_base.num = dst_scale;
    
	// Emit one intra frame every given frames at most
	pCodecCtx->gop_size = MAX(1, keyframes_rate);

	// Pixel Format
	pCodecCtx->pix_fmt = pix_fmt;

	// Set m_nPassNumber
	if (pCodecCtx->codec_id != CODEC_ID_MPEG4	&&
		pCodecCtx->codec_id != CODEC_ID_H263	&&
		pCodecCtx->codec_id != CODEC_ID_H263P	&&
		pCodecCtx->codec_id != CODEC_ID_FLV1	&&
		pCodecCtx->codec_id != CODEC_ID_H264	&&
		pCodecCtx->codec_id != CODEC_ID_THEORA	&&
		pCodecCtx->codec_id != CODEC_ID_SNOW)
		m_nPassNumber[pStream->index] = 0; // No two pass mode supported
	else
		m_nPassNumber[pStream->index] = m_nGlobalPassNumber;

	if (!strcmp(m_pFormatCtx->oformat->name, "avi"))
		pCodecCtx->max_b_frames = 0;
	else
	{
		if (pCodecCtx->codec_id == CODEC_ID_MPEG2VIDEO)
			pCodecCtx->max_b_frames = 2;
	}
    if (pCodecCtx->codec_id == CODEC_ID_MPEG1VIDEO)
	{
        /* needed to avoid using macroblocks in which some coeffs overflow
           this doesnt happen with normal video, it just happens here as the
           motion of the chroma plane doesnt match the luma plane */
        pCodecCtx->mb_decision = 2;
    }
	else if (pCodecCtx->codec_id == CODEC_ID_SNOW)
	{
		pCodecCtx->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
		pCodecCtx->prediction_method = 0;
		pCodecCtx->me_cmp = 1;
		pCodecCtx->me_sub_cmp = 1;
		pCodecCtx->mb_cmp = 1;
		pCodecCtx->flags |= CODEC_FLAG_QPEL;
	}
	else if (pCodecCtx->codec_id == CODEC_ID_MPEG4)
	{
		if (!m_bFastEncode)
		{
			pCodecCtx->mb_decision = 2;								// mbd:    macroblock decision mode
			pCodecCtx->me_cmp = 2;									// cmp:    fullpixel motion estimation compare function
			pCodecCtx->me_sub_cmp = 2;								// subcmp: subpixel motion estimation compare function
			pCodecCtx->trellis = 1;									// trell:  enable trellis quantization
			pCodecCtx->flags |= (CODEC_FLAG_AC_PRED |				// aic:    MPEG-4 AC prediction
								CODEC_FLAG_4MV);					// mv4:    4 MV per MB allowed
		}
	}
	else if (	pCodecCtx->codec_id == CODEC_ID_H263  ||
				pCodecCtx->codec_id == CODEC_ID_H263P ||
				pCodecCtx->codec_id == CODEC_ID_FLV1)
	{
		if (!m_bFastEncode)
		{
			pCodecCtx->mb_decision = 2;								// mbd:    macroblock decision mode
			pCodecCtx->me_cmp = 2;									// cmp:    fullpixel motion estimation compare function
			pCodecCtx->me_sub_cmp = 2;								// subcmp: subpixel motion estimation compare function
			pCodecCtx->trellis = 1;									// trell:  enable trellis quantization
			pCodecCtx->flags |= (CODEC_FLAG_AC_PRED				|	// aic:    H.263 advanced intra coding
								CODEC_FLAG_4MV					|	// mv4:    advanced prediction for H.263
								CODEC_FLAG_CBP_RD				|	// cbp:    Use rate distortion optimization for cbp, this needs trellis enabled!
								CODEC_FLAG_MV0					|	// mv0:    try to encode each MB with MV=<0,0> and choose the better one (has no effect if mb_decision=0)
								CODEC_FLAG_LOOP_FILTER			|	// lf:     use loop filter (h263+)
								/*CODEC_FLAG_H263P_SLICE_STRUCT	|*/	// ssm:    necessary if multi-threading (h263+)
								CODEC_FLAG_H263P_AIV			|	// aiv:    H.263+ alternative inter VLC
								CODEC_FLAG_H263P_UMV);				// umv:    Enable Unlimited Motion Vector (h263+)
		}
	}
	else if (pCodecCtx->codec_id == CODEC_ID_THEORA)
	{
		if (m_bFastEncode)
			pCodecCtx->flags |= CODEC_FLAG2_FAST;
	}
	//
	// Notes:
	//
	// - cbp, mv0: Controls the selection of macroblocks. Small speed cost for small quality gain.
	// - cmp, subcmp, precmp: Comparison function for motion estimation.
	//   (precmp seems to do little or nothing, but slows down encoding)
	//   Experiment with values of 0 (default), 2 (hadamard), 3 (dct), and 6 (rate distortion).
	//   0 is fastest, and sufficient for precmp. For cmp and subcmp, 2 is good for anime,
	//   and 3 is good for live action. 6 may or may not be slightly better, but is slow.
	// - qpel: Quarter pixel motion estimation. MPEG-4 uses half pixel precision for its motion search by default,
	//   therefore this option comes with an overhead as more information will be stored in the encoded file.
	//   The compression gain/loss depends on the movie, but it is usually not very effective on Anime.
	//   qpel always incurs a significant cost in CPU decode time (+25% in practice). 
	// - mbd: Macroblock decision algorithm (this is the old vhq high quality mode option), encode each macro block
	//   using all available comparison functions and choose the best.
	//   This is slow but results in better quality and file size.
	//   When mbd is set to 1 or 2, the value of mbcmp is ignored when comparing macroblocks
	//   (the mbcmp value is still used in other places though, in particular the motion search
	//   algorithms). If any comparison setting (precmp, subcmp, cmp, or mbcmp) is nonzero,
	//   however, a slower but better half-pel motion search will be used, regardless of what
	//   mbd is set to. If qpel is set, quarter-pel motion search will be used regardless.
	//   0: Use comparison function given by mbcmp (default).
	//   1: Select the MB mode which needs the fewest bits (=vhq).
	//   2: Select the MB mode which has the best rate distortion.
	//   I can tell you that switching from 1 to 2 incurs a 10% speed penalty and produces a slightly
	//   smaller output file.
	//
	// Multi-threading compile with --enable-w32threads,
	// supported for CODEC_ID_MPEG1, CODEC_ID_MPEG2, CODEC_ID_MPEG4 and CODEC_ID_H263P.
	// Speed-up is around 10-20%, CODEC_ID_H263P makes errors in two pass mode
	// -> Do not use multi-threading!
	//avcodec_thread_init(pCodecCtx, ((CUImagerApp*)::AfxGetApp())->m_nCoresCount);
	//avcodec_thread_free(pCodecCtx); // called to free

    // Some formats want stream headers to be separate
    if (!strcmp(m_pFormatCtx->oformat->name, "mp4") ||
		!strcmp(m_pFormatCtx->oformat->name, "mov") ||
		!strcmp(m_pFormatCtx->oformat->name, "3gp"))
        pCodecCtx->flags |= CODEC_FLAG_GLOBAL_HEADER;

	// Two pass mode
	if (m_nPassNumber[pStream->index] > 0)
	{
		m_s2PassLogFileName[pStream->index].Format(_T("%s_stream%d_twopass.log"),
										::GetFileNameNoExt(m_sFileName), pStream->index);
		if (m_nPassNumber[pStream->index] == 2)
		{
			if (::IsExistingFile(m_s2PassLogFileName[pStream->index]))
				pCodecCtx->flags |= CODEC_FLAG_PASS2;
			else
				m_nPassNumber[pStream->index] = 0;
		}
		else if (m_nPassNumber[pStream->index] == 1)
			pCodecCtx->flags |= CODEC_FLAG_PASS1;
	}

    return pStream;
}

bool CAVRec::Init(	LPCTSTR lpszFileName,
					int nPassNumber/*=0*/,
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
#ifdef _UNICODE
	wcstombs(filename, sASCIICompatiblePath, 1024);
#else
	strncpy(filename, sASCIICompatiblePath, 1023);
#endif
	filename[1023] = '\0';

	Close();

	m_sFileName = lpszFileName;
	m_dwTotalVideoStreams = 0;
	m_dwTotalAudioStreams = 0;
	m_nGlobalPassNumber = nPassNumber;
	m_bFastEncode = bFastEncode;
	
    // Auto detect the output format from the name. Default is mpeg.
	AVOutputFormat* pGuessedFormat = guess_format(NULL, filename, NULL);
    if (!pGuessedFormat)
	{
        TRACE(_T("Could not deduce output format from file extension: using MPEG.\n"));
        pGuessedFormat = guess_format("mpeg", NULL, NULL);
    }
    if (!pGuessedFormat)
	{
        TRACE(_T("Could not find suitable output format\n"));
        return false;
    }

    // Allocate the output media context
    m_pFormatCtx = av_alloc_format_context();
    if (!m_pFormatCtx)
	{
        TRACE(_T("Memory error\n"));
        return false;
    }
    _snprintf(m_pFormatCtx->filename, sizeof(m_pFormatCtx->filename), "%s", filename);

	// Allocate the output format
	m_pOutputFormat = new AVOutputFormat;
	if (!m_pOutputFormat)
	{
		TRACE(_T("Memory error\n"));
        return false;
	}
	memcpy(m_pOutputFormat, pGuessedFormat, sizeof(AVOutputFormat));

	// Set the output format
	m_pFormatCtx->oformat = m_pOutputFormat;

	return true;
}

int CAVRec::AddRawVideoStream(	const LPBITMAPINFO pFormat,
								int nFormatSize,
								DWORD dwRate,
								DWORD dwScale)
{
	// Check
	if (!pFormat || nFormatSize < sizeof(BITMAPINFOHEADER) || dwScale == 0 ||
		dwRate == 0 || !m_pOutputFormat || !m_pFormatCtx)
		return -1;

	// Set the Codec id
	m_pOutputFormat->video_codec = CODEC_ID_RAWVIDEO;

	// Create the video stream
    AVStream* pStream = av_new_stream(m_pFormatCtx, 0);
    if (!pStream)
	{
        TRACE(_T("Could not alloc video stream\n"));
        return -1;
    }
	pStream->stream_copy = 1;	// This is a hack so that the entire extradata
								// is copied to strf of the avi file,
								// I modified the avienc.c source!
    AVCodecContext* pCodecCtx = pStream->codec;
    pCodecCtx->codec_id = m_pOutputFormat->video_codec;
    pCodecCtx->codec_type = CODEC_TYPE_VIDEO;
	pCodecCtx->codec_tag = pFormat->bmiHeader.biCompression;
	
	// Set format
	pCodecCtx->extradata_size = nFormatSize;
	pCodecCtx->extradata = (uint8_t*)av_malloc(pCodecCtx->extradata_size + FF_INPUT_BUFFER_PADDING_SIZE);
	if (!pCodecCtx->extradata)
	{
        TRACE(_T("Could not alloc video format buffer\n"));
        return -1;
    }
	memcpy(pCodecCtx->extradata, pFormat, nFormatSize);

    // Resolution must be a multiple of two
    pCodecCtx->width = pFormat->bmiHeader.biWidth;
    pCodecCtx->height = pFormat->bmiHeader.biHeight;

    // Rate & Scale
	// (see ff_parse_specific_params())
	int dst_rate, dst_scale;
	av_reduce(&dst_scale, &dst_rate, (int64_t)dwScale, (int64_t)dwRate, MAX_SIZE_FOR_RATIONAL);
    pCodecCtx->time_base.den = dst_rate;
    pCodecCtx->time_base.num = dst_scale;

	// Inc. streams count
	m_dwTotalVideoStreams++;

	return pStream->index;
}

int CAVRec::AddVideoStream(	const LPBITMAPINFO pSrcFormat,
							const LPBITMAPINFO pDstFormat,
							DWORD dwDstRate,
							DWORD dwDstScale,
							int bitrate,
							int keyframes_rate,
							float qscale)	// 0.0f use bitrate, 2.0f best quality, 31.0f worst quality
{
	int nStreamNum = -1;
	PixelFormat src_pix_fmt = CAVIPlay::CAVIVideoStream::AVCodecBMIToPixFormat(pSrcFormat);

	// Check
	if (!pSrcFormat || !pDstFormat || dwDstScale == 0 ||
		dwDstRate == 0 || !m_pOutputFormat || !m_pFormatCtx)
		return -1;

	// Set the Codec
	m_pOutputFormat->video_codec = CAVIPlay::CAVIVideoStream::AVCodecFourCCToCodecID(pDstFormat->bmiHeader.biCompression);
	
	// Only raw RLE data write is supported!
	if (m_pOutputFormat->video_codec == CODEC_ID_MSRLE ||
		m_pOutputFormat->video_codec == CODEC_ID_NONE)
		m_pOutputFormat->video_codec = CODEC_ID_RAWVIDEO;

	// Set encoder to H.263+
	if (m_pOutputFormat->video_codec == CODEC_ID_H263)
		m_pOutputFormat->video_codec = CODEC_ID_H263P;

	// Add the video stream
	AVCodec* pCodec = NULL;
	AVStream* pVideoStream = NULL;
	
	// Find the video encoder
	pCodec = avcodec_find_encoder(m_pOutputFormat->video_codec);
	if (!pCodec)
	{
		m_pOutputFormat->video_codec = CODEC_ID_RAWVIDEO;
		pCodec = avcodec_find_encoder(m_pOutputFormat->video_codec);
		if (!pCodec)
		{
			TRACE(_T("Video Codec not found\n"));
			return -1;
		}
	}

	// Check whether source pixel format is supported
	if (pCodec->pix_fmts)
	{
        const enum PixelFormat *p = pCodec->pix_fmts;
        for ( ; *p != -1 ; p++)
		{
            if (*p == src_pix_fmt)
                break;
        }
        if (*p == -1)
            src_pix_fmt = pCodec->pix_fmts[0];
    }
	else
	{
		if (m_pOutputFormat->video_codec != CODEC_ID_RAWVIDEO)
			src_pix_fmt = PIX_FMT_YUV420P;
	}

	// Create Video Stream
    pVideoStream = CreateVideoStream(m_pOutputFormat->video_codec,
									pDstFormat,
									dwDstRate,
									dwDstScale,
									src_pix_fmt,
									bitrate,
									keyframes_rate,
									qscale);
	if (!pVideoStream)
		return -1;
    
	// Stream Number
	nStreamNum = pVideoStream->index;

	// Get the attached video codec context	
	AVCodecContext* pCodecCtx = pVideoStream->codec;

	// Two pass mode
    if (pCodecCtx->flags & (CODEC_FLAG_PASS1 | CODEC_FLAG_PASS2))
	{
        FILE* f;
        if (pCodecCtx->flags & CODEC_FLAG_PASS1)
		{
			// Write the log file
			//
			// If CREATE_ALWAYS and FILE_ATTRIBUTE_NORMAL are specified, CreateFile fails and sets the last error to
			// ERROR_ACCESS_DENIED if the file exists and has the FILE_ATTRIBUTE_HIDDEN or FILE_ATTRIBUTE_SYSTEM
			// attribute. To avoid the error, specify the same attributes as the existing file.
			//
			// -> You can open hidden file using fopen("r") but you cannot do this with fopen("w").
			// The reason is that if you want to open a hidden file for writing you must provide the correct attribute
			// and fopen("w") always  calls CreateFile with FILE_ATTRIBUTE_NORMAL.
			//
			// -> Set the hidden attribute after the file is opened!
			::DeleteFile(m_s2PassLogFileName[nStreamNum]);
            f = _tfopen(m_s2PassLogFileName[nStreamNum], _T("w"));
            if (!f)
               return -1;
			::SetFileAttributes(m_s2PassLogFileName[nStreamNum], ::GetFileAttributes(m_s2PassLogFileName[nStreamNum]) | FILE_ATTRIBUTE_HIDDEN);
            m_p2PassLogFiles[nStreamNum] = f;
        }
		else
		{
            // Read the log file
            f = _tfopen(m_s2PassLogFileName[nStreamNum], _T("r"));
            if (!f)
                return -1;
            fseek(f, 0, SEEK_END);
            int size = ftell(f);
            fseek(f, 0, SEEK_SET);
            char* logbuffer = (char*)av_malloc(size + 1);
            if (!logbuffer)
                return -1;
            size = fread(logbuffer, 1, size, f);
            fclose(f);
            logbuffer[size] = '\0';
            pCodecCtx->stats_in = logbuffer;
        }
    }

	// Store codec_tag for raw video encoder
	unsigned int rawvideo_codec_tag = 0;
	if (pCodecCtx->codec_id == CODEC_ID_RAWVIDEO)
		rawvideo_codec_tag = pCodecCtx->codec_tag;

	// Open the video codec
	if (avcodec_open_thread_safe(pCodecCtx, pCodec) < 0)
	{
		if (pCodecCtx->flags & CODEC_FLAG_PASS2)
		{
			TRACE(_T("Cannot reach the wanted bit_rate, set to worst quality\n"));
			pCodecCtx->flags &= ~CODEC_FLAG_PASS2;
			pCodecCtx->flags |= CODEC_FLAG_QSCALE;
			pVideoStream->quality = (float)(FF_QP2LAMBDA * pCodecCtx->qmax);
			pCodecCtx->global_quality = (int)pVideoStream->quality;
			pCodecCtx->bit_rate = 0;
			if (avcodec_open_thread_safe(pCodecCtx, pCodec) < 0)
			{
				TRACE(_T("Could not open the video codec\n"));
				return -1;
			}
			else
				m_bCodecOpened[nStreamNum] = true;
		}
		else
		{
			TRACE(_T("Cannot reach the wanted bit_rate, set to best quality\n"));
			m_nPassNumber[nStreamNum] = 0;
			if (m_p2PassLogFiles[nStreamNum])
			{
				fclose(m_p2PassLogFiles[nStreamNum]);
				m_p2PassLogFiles[nStreamNum] = NULL;
			}
			if (::IsExistingFile(m_s2PassLogFileName[nStreamNum]))
				::DeleteFile(m_s2PassLogFileName[nStreamNum]);
			pCodecCtx->flags &= ~CODEC_FLAG_PASS1;
			pCodecCtx->flags |= CODEC_FLAG_QSCALE;
			pVideoStream->quality = (float)(FF_QP2LAMBDA * pCodecCtx->qmin);
			pCodecCtx->global_quality = (int)pVideoStream->quality;
			pCodecCtx->bit_rate = 0;
			if (avcodec_open_thread_safe(pCodecCtx, pCodec) < 0)
			{
				TRACE(_T("Could not open the video codec\n"));
				return -1;
			}
			else
				m_bCodecOpened[nStreamNum] = true;
		}
	}
	else
	{
		// Restore codec_tag because raw_init_encoder() under
		// rawenc.c changes the codec_tag if this is BI_RGB (=0)
		if (pCodecCtx->codec_id == CODEC_ID_RAWVIDEO)
			pCodecCtx->codec_tag = rawvideo_codec_tag;
		
		// Set opened flag
		m_bCodecOpened[nStreamNum] = true;
	}

	// Allocate video frames
	m_pFrame[nStreamNum] = avcodec_alloc_frame();
	if (!m_pFrame[nStreamNum])
		return -1;
	m_pFrameTemp[nStreamNum] = avcodec_alloc_frame();
	if (!m_pFrameTemp[nStreamNum])
		return -1;

	// Allocate the Output buffer
	m_nOutbufSize[nStreamNum] = 4 * pDstFormat->bmiHeader.biWidth * pDstFormat->bmiHeader.biHeight;
	if (m_nOutbufSize[nStreamNum] < FF_MIN_BUFFER_SIZE)
		m_nOutbufSize[nStreamNum] = FF_MIN_BUFFER_SIZE;
	m_pOutbuf[nStreamNum] = new uint8_t[m_nOutbufSize[nStreamNum] + FF_INPUT_BUFFER_PADDING_SIZE];
	if (!m_pOutbuf[nStreamNum])
	{
		m_nOutbufSize[nStreamNum] = 0;
		return -1;
	}

	// Inc. streams count
	m_dwTotalVideoStreams++;

	return nStreamNum;
}

int CAVRec::AddRawAudioStream(	const LPWAVEFORMATEX pFormat,
								int nFormatSize,
								DWORD dwSampleSize,	// Set in avi strh
								DWORD dwRate,		// Set in avi strh
								DWORD dwScale)		// Set in avi strh
{
	// Check
	if (!pFormat || nFormatSize < sizeof(WAVEFORMAT) ||
		!m_pOutputFormat || !m_pFormatCtx)
		return -1;

	// Set the Codec id, that's necessary otherwise av_write_header() in Open() sets a wrong block_align,
	// which is used in avi_write_header() (see avienc.c) by ff_parse_specific_params() to write the
	// sample size in the stream header (not the stream format which is hacked and ok)
	m_pOutputFormat->audio_codec = CAVIPlay::CAVIAudioStream::AVCodecFormatTagToCodecID(pFormat->wFormatTag,
																						pFormat->wBitsPerSample);
	if (m_pOutputFormat->audio_codec == CODEC_ID_NONE)
		m_pOutputFormat->audio_codec = CODEC_ID_PCM_S16LE;

	// Create the audio stream
    AVStream* pStream = av_new_stream(m_pFormatCtx, 0);
    if (!pStream)
	{
        TRACE(_T("Could not alloc audio stream\n"));
        return -1;
    }
	pStream->stream_copy = 1;	// This is a hack so that the entire extradata
								// is copied to strf of the avi file,
								// I modified the avienc.c source!
    AVCodecContext* pCodecCtx = pStream->codec;
    pCodecCtx->codec_id = m_pOutputFormat->audio_codec;
    pCodecCtx->codec_type = CODEC_TYPE_AUDIO;
	pCodecCtx->codec_tag = pFormat->wFormatTag;
	if (pFormat->wFormatTag == WAVE_FORMAT_PCM && pFormat->wBitsPerSample == 8)
		pCodecCtx->sample_fmt = SAMPLE_FMT_U8;
	else
		pCodecCtx->sample_fmt = SAMPLE_FMT_S16;
    pCodecCtx->channels = pFormat->nChannels;
	
	// Set format
	pCodecCtx->extradata_size = nFormatSize;
	pCodecCtx->extradata = (uint8_t*)av_malloc(pCodecCtx->extradata_size + FF_INPUT_BUFFER_PADDING_SIZE);
	if (!pCodecCtx->extradata)
	{
        TRACE(_T("Could not alloc audio format buffer\n"));
        return -1;
    }
	memcpy(pCodecCtx->extradata, pFormat, nFormatSize);

    // Block Align, Scale and Rate
	// (see ff_parse_specific_params() to understand the following)
	pCodecCtx->block_align = dwSampleSize;
	if (dwRate == 0 || dwScale == 0)
	{
        pCodecCtx->frame_size =		pFormat->nBlockAlign > 0 ?		// Scale
									pFormat->nBlockAlign :
									1;
        pCodecCtx->sample_rate =	pFormat->nAvgBytesPerSec > 0 ?	// Rate
									pFormat->nAvgBytesPerSec :
									(pFormat->nSamplesPerSec > 0 ?
									pFormat->nSamplesPerSec :
									8000);	// Just a value because sample_rate
											// should not be 0!
    }
	else
	{
		pCodecCtx->frame_size = dwScale;							// Scale
		pCodecCtx->sample_rate = dwRate;							// Rate
	}

	// Inc. streams count
	m_dwTotalAudioStreams++;

	return pStream->index;
}

int CAVRec::AddAudioStream(	const LPWAVEFORMATEX pSrcWaveFormat,
							const LPWAVEFORMATEX pDstWaveFormat)
{
	int qscale; // Only used for vorbis
	int nStreamNum = -1;

	// Check
	if (!pSrcWaveFormat || !pDstWaveFormat ||
		!m_pOutputFormat || !m_pFormatCtx)
		return -1;

	// Set the Codec
	m_pOutputFormat->audio_codec = CAVIPlay::CAVIAudioStream::AVCodecFormatTagToCodecID(pDstWaveFormat->wFormatTag,
																						pDstWaveFormat->wBitsPerSample);
	if (m_pOutputFormat->audio_codec == CODEC_ID_NONE)
	{
		TRACE(_T("No Audio Codec found for Format Tag: 0x%04X\n"), pDstWaveFormat->wFormatTag);
		return -1;
	}
	
	// Add the audio stream
	AVCodec* pCodec = NULL;
    AVStream* pAudioStream = NULL;
	
	// Find the audio encoder
	pCodec = avcodec_find_encoder(m_pOutputFormat->audio_codec);
	if (!pCodec)
	{
		TRACE(_T("Audio Codec not found\n"));
		return -1;
	}

	// Sample Format
	SampleFormat sample_fmt = pDstWaveFormat->wBitsPerSample == 8 ? SAMPLE_FMT_U8 : SAMPLE_FMT_S16;
	if (pDstWaveFormat->wFormatTag != WAVE_FORMAT_PCM)
		sample_fmt = SAMPLE_FMT_S16;

	// Create Audio Stream
	if (m_pOutputFormat->audio_codec == CODEC_ID_FLAC)
	{
		// Reset Avg Bytes Per Sec, so that the codec calculates it
		pDstWaveFormat->nAvgBytesPerSec = 0;
	}
	else if (m_pOutputFormat->audio_codec == CODEC_ID_VORBIS)
	{
		// Note: libvorbis has a quality scale 0..10, but it crashes with a division by 0
		// -> use ffmpeg internal vorbis encoder which has a different quality scale:
		// Author says: 10 to 30 are sane values, the higher the number,
		// the higher the bitrate and quality.
		// Only 2 channel is supported, and, in a psy sense,
		// 44100 and 48000 are best supported...
		if (pDstWaveFormat->nAvgBytesPerSec <= (60000 / 8))
			qscale = 10;											// After some tests quality 10 gives around 60 kbps (for 44.1KHz)
		else if (pDstWaveFormat->nAvgBytesPerSec <= (90000 / 8))
			qscale = 15;											// After some tests quality 15 gives around 90 kbps (for 44.1KHz)
		else if (pDstWaveFormat->nAvgBytesPerSec <= (120000 / 8))
			qscale = 22;											// After some tests quality 22 gives around 120 kbps (for 44.1KHz)
		else
			qscale = 35;											// After some tests quality 35 gives around 160 kbps (for 44.1KHz)
	}
    pAudioStream = CreateAudioStream(m_pOutputFormat->audio_codec,
									sample_fmt,
									pDstWaveFormat->wFormatTag,
									pDstWaveFormat->nAvgBytesPerSec * 8, // Bitrate
									pDstWaveFormat->nSamplesPerSec,
									pDstWaveFormat->nChannels);
	if (!pAudioStream)
		return -1;

	// Stream Number
	nStreamNum = pAudioStream->index;

	// Get the attached audio codec context	
	AVCodecContext* pCodecCtx = pAudioStream->codec;

	// Set the quality for internal vorbis encoder
	if (m_pOutputFormat->audio_codec == CODEC_ID_VORBIS)
	{
		pCodecCtx->flags |= CODEC_FLAG_QSCALE;
		pCodecCtx->global_quality = FF_QP2LAMBDA * qscale;
	}

	// Open the audio codec
	if (avcodec_open_thread_safe(pCodecCtx, pCodec) < 0)
	{
		// Try with another sample format
		if (pCodecCtx->sample_fmt == SAMPLE_FMT_S16)
			pCodecCtx->sample_fmt = SAMPLE_FMT_U8;
		else
			pCodecCtx->sample_fmt = SAMPLE_FMT_S16;
		if (avcodec_open_thread_safe(pCodecCtx, pCodec) < 0)
		{
			TRACE(_T("Could not open the audio codec\n"));
			return -1;
		}
		else
			m_bCodecOpened[nStreamNum] = true;
	}
	else
		m_bCodecOpened[nStreamNum] = true;

	// Set the correct bitrate for ADPCM
	if (m_pOutputFormat->audio_codec == CODEC_ID_ADPCM_IMA_WAV	||
		m_pOutputFormat->audio_codec == CODEC_ID_ADPCM_MS)
	{
		double dFrameTime = (double)(pCodecCtx->frame_size) / (double)(pDstWaveFormat->nSamplesPerSec);
		pDstWaveFormat->nAvgBytesPerSec = Round((double)(pCodecCtx->block_align) / dFrameTime);
		pCodecCtx->bit_rate = pDstWaveFormat->nAvgBytesPerSec * 8;
	}
	else if (m_pOutputFormat->audio_codec == CODEC_ID_ADPCM_YAMAHA)
	{
		// Use this because pCodecCtx->frame_size is not set correctly by LibAVCodec...
		pDstWaveFormat->nAvgBytesPerSec = pDstWaveFormat->nChannels * pDstWaveFormat->nSamplesPerSec / 2;
		pCodecCtx->bit_rate = pDstWaveFormat->nAvgBytesPerSec * 8;
	}

	// Allocate output buffer
	if (pCodecCtx->frame_size <= 1)
	{
		int nSamplesPerSec = pDstWaveFormat->nSamplesPerSec;
		int nBlockAlign = pDstWaveFormat->nBlockAlign;
		if (nSamplesPerSec <= 11025) 
			m_nOutbufSize[nStreamNum] = 1 * AUDIO_PCM_MIN_BUF_SIZE * nBlockAlign;
		else if (nSamplesPerSec <= 22050)
			m_nOutbufSize[nStreamNum] = 2 * AUDIO_PCM_MIN_BUF_SIZE * nBlockAlign;
		else if (nSamplesPerSec <= 32000)
			m_nOutbufSize[nStreamNum] = 3 * AUDIO_PCM_MIN_BUF_SIZE * nBlockAlign;
		else if (nSamplesPerSec <= 44100)
			m_nOutbufSize[nStreamNum] = 4 * AUDIO_PCM_MIN_BUF_SIZE * nBlockAlign;
		else if (nSamplesPerSec <= 48000)
			m_nOutbufSize[nStreamNum] = 5 * AUDIO_PCM_MIN_BUF_SIZE * nBlockAlign;
		else
			m_nOutbufSize[nStreamNum] = 8 * AUDIO_PCM_MIN_BUF_SIZE * nBlockAlign;
	}
	else
		m_nOutbufSize[nStreamNum] = AVCODEC_MAX_AUDIO_FRAME_SIZE;
	m_pOutbuf[nStreamNum] = new uint8_t[m_nOutbufSize[nStreamNum] + FF_INPUT_BUFFER_PADDING_SIZE];
	if (!m_pOutbuf[nStreamNum])
	{
		m_nOutbufSize[nStreamNum] = 0;
		return -1;
	}

	// Set Source & Intermediate Wave Formats
	SetSrcWaveFormat(m_pFormatCtx->nb_streams - 1, pSrcWaveFormat);				// Allocates memory!
	SetIntermediateWaveFormat(m_pFormatCtx->nb_streams - 1, pSrcWaveFormat);	// Allocates memory!
	m_pIntermediateWaveFormat[m_pFormatCtx->nb_streams - 1]->nChannels = pDstWaveFormat->nChannels;
	m_pIntermediateWaveFormat[m_pFormatCtx->nb_streams - 1]->nSamplesPerSec = pDstWaveFormat->nSamplesPerSec;
	if (m_pIntermediateWaveFormat[m_pFormatCtx->nb_streams - 1]->nSamplesPerSec != m_pSrcWaveFormat[m_pFormatCtx->nb_streams - 1]->nSamplesPerSec		||
		m_pIntermediateWaveFormat[m_pFormatCtx->nb_streams - 1]->nChannels != m_pSrcWaveFormat[m_pFormatCtx->nb_streams - 1]->nChannels)
		m_pIntermediateWaveFormat[m_pFormatCtx->nb_streams - 1]->wBitsPerSample = 16;
	if (m_pIntermediateWaveFormat[m_pFormatCtx->nb_streams - 1]->wBitsPerSample == 8)
		m_pIntermediateWaveFormat[m_pFormatCtx->nb_streams - 1]->nBlockAlign = m_pIntermediateWaveFormat[m_pFormatCtx->nb_streams - 1]->nChannels;
	else
		m_pIntermediateWaveFormat[m_pFormatCtx->nb_streams - 1]->nBlockAlign =	2 * m_pIntermediateWaveFormat[m_pFormatCtx->nb_streams - 1]->nChannels;
	m_pIntermediateWaveFormat[m_pFormatCtx->nb_streams - 1]->nAvgBytesPerSec =	m_pIntermediateWaveFormat[m_pFormatCtx->nb_streams - 1]->nSamplesPerSec *
																				m_pIntermediateWaveFormat[m_pFormatCtx->nb_streams - 1]->nBlockAlign;

	// Compute the input frame size in samples
	if (pCodecCtx->frame_size <= 1)
		m_nAudioInputFrameSize[nStreamNum] = m_nOutbufSize[nStreamNum] / pDstWaveFormat->nBlockAlign;
	else
		m_nAudioInputFrameSize[nStreamNum] = pCodecCtx->frame_size;

	// Allocate Intermediate Samples Buffer
	m_nIntermediateSamplesBufSize[nStreamNum] = m_pIntermediateWaveFormat[m_pFormatCtx->nb_streams - 1]->nBlockAlign * m_nAudioInputFrameSize[nStreamNum];
	m_pIntermediateSamplesBuf[nStreamNum] = new uint8_t[m_nIntermediateSamplesBufSize[nStreamNum] + FF_INPUT_BUFFER_PADDING_SIZE];

	m_dwTotalAudioStreams++;

	return nStreamNum;
}

bool CAVRec::SetInfo(LPCTSTR szTitle,
					 LPCTSTR szAuthor,
					 LPCTSTR szCopyright,
					 LPCTSTR szComment/*=_T("")*/,
					 LPCTSTR szAlbum/*=_T("")*/,
					 LPCTSTR szGenre/*=_T("")*/,
					 int nTrack/*=0*/,
					 int nYear/*=0*/)
{
	// Check
	if (!m_pFormatCtx)
		return false;

#ifdef _UNICODE
	wcstombs(m_pFormatCtx->title, szTitle, sizeof(m_pFormatCtx->title));
#else
	strncpy(m_pFormatCtx->title, szTitle, sizeof(m_pFormatCtx->title) - 1);
#endif
	m_pFormatCtx->title[sizeof(m_pFormatCtx->title) - 1] = '\0';

#ifdef _UNICODE
	wcstombs(m_pFormatCtx->author, szAuthor, sizeof(m_pFormatCtx->author));
#else
	strncpy(m_pFormatCtx->author, szAuthor, sizeof(m_pFormatCtx->author) - 1);
#endif
	m_pFormatCtx->author[sizeof(m_pFormatCtx->author) - 1] = '\0';

#ifdef _UNICODE
	wcstombs(m_pFormatCtx->copyright, szCopyright, sizeof(m_pFormatCtx->copyright));
#else
	strncpy(m_pFormatCtx->copyright, szCopyright, sizeof(m_pFormatCtx->copyright) - 1);
#endif
	m_pFormatCtx->copyright[sizeof(m_pFormatCtx->copyright) - 1] = '\0';
    
#ifdef _UNICODE
	wcstombs(m_pFormatCtx->comment, szComment, sizeof(m_pFormatCtx->comment));
#else
	strncpy(m_pFormatCtx->comment, szComment, sizeof(m_pFormatCtx->comment) - 1);
#endif
	m_pFormatCtx->comment[sizeof(m_pFormatCtx->comment) - 1] = '\0';
   
#ifdef _UNICODE
	wcstombs(m_pFormatCtx->album, szAlbum, sizeof(m_pFormatCtx->album));
#else
	strncpy(m_pFormatCtx->album, szAlbum, sizeof(m_pFormatCtx->album) - 1);
#endif
	m_pFormatCtx->album[sizeof(m_pFormatCtx->album) - 1] = '\0';

#ifdef _UNICODE
	wcstombs(m_pFormatCtx->genre, szGenre, sizeof(m_pFormatCtx->genre));
#else
	strncpy(m_pFormatCtx->genre, szGenre, sizeof(m_pFormatCtx->genre) - 1);
#endif
	m_pFormatCtx->genre[sizeof(m_pFormatCtx->genre) - 1] = '\0';

    m_pFormatCtx->track = nTrack;
	m_pFormatCtx->year = nYear;

	return true;
}

bool CAVRec::Open()
{
	// Check
	if (!m_pOutputFormat || !m_pFormatCtx)
		return false;

	// Set the output parameters (must be done even if no parameters)
	AVFormatParameters params;
	memset(&params, 0, sizeof(AVFormatParameters));
	if (av_set_parameters(m_pFormatCtx, &params) < 0)
	{
        TRACE(_T("Invalid output format parameters\n"));
        return false;
    }

	// Open the output file, if needed
    if (!(m_pOutputFormat->flags & AVFMT_NOFILE))
	{
		if (::url_fopen(&m_pFormatCtx->pb, m_pFormatCtx->filename, URL_WRONLY) < 0)
		{
            TRACE(_T("Could not open '%s'\n"), m_sFileName);
            return false;
        }
		else
			m_bFileOpened = true;
    }

    // Write the stream header, if any.
	// This sets the codec_tag if it is not 0!
    if (av_write_header(m_pFormatCtx) == 0)
	{
		m_bOpen = true;
		return true;
	}
	else
		return false;
}

void CAVRec::FlushAudio()
{
	if (m_pFormatCtx)
	{
		for (DWORD i = 0 ; i < (DWORD)m_pFormatCtx->nb_streams ; i++)
		{
			if (m_pFormatCtx->streams[i]		&&
				m_pFormatCtx->streams[i]->codec	&&
				m_pFormatCtx->streams[i]->codec->codec_type == CODEC_TYPE_AUDIO)
			{
				AddAudioSamplesDirect(i, 0, NULL, false);
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
		// Only write trailer if files has been opened!
		if (m_bFileOpened)
		{
			// Flush audio buffers
			FlushAudio();

			// Write the trailer, if any
			if (av_write_trailer(m_pFormatCtx) != 0)
				res = false;

			// Close the output file
			if (!(m_pOutputFormat->flags & AVFMT_NOFILE))
			{
				url_fclose(m_pFormatCtx->pb); // This frees pb->buffer and pb itself but doesn't set it to NULL!
				m_pFormatCtx->pb = NULL;
			}

			// Reset Flag
			m_bFileOpened = false;
		}

		// Free Current Packet
		if (m_pFormatCtx->cur_st && m_pFormatCtx->cur_st->parser)
			av_free_packet(&m_pFormatCtx->cur_pkt);

		// Close & Free the Codecs of all the Streams
		for (i = 0 ; i < (int)m_pFormatCtx->nb_streams ; i++)
		{
			if (m_pFormatCtx->streams[i])
			{
				// Close
				if (m_bCodecOpened[i] && m_pFormatCtx->streams[i]->codec)
				{
					avcodec_close_thread_safe(m_pFormatCtx->streams[i]->codec);
					m_bCodecOpened[i] = false;
				}
				if (m_pFormatCtx->streams[i]->parser)
					av_parser_close(m_pFormatCtx->streams[i]->parser);

				// Free
				if (m_pFormatCtx->streams[i]->codec->stats_in)
					av_freep(&m_pFormatCtx->streams[i]->codec->stats_in);
				if (m_pFormatCtx->streams[i]->index_entries)
					av_freep(&m_pFormatCtx->streams[i]->index_entries);
				if (m_pFormatCtx->streams[i]->codec->extradata)
					av_freep(&m_pFormatCtx->streams[i]->codec->extradata);
				m_pFormatCtx->streams[i]->codec->extradata_size = 0;
				if (m_pFormatCtx->streams[i]->codec)
					av_freep(&m_pFormatCtx->streams[i]->codec);
				av_freep(&m_pFormatCtx->streams[i]);
			}
		}

		// Free Packet Buffers
		AVPacketList* pktl;
		for (;;)
		{
			pktl = m_pFormatCtx->packet_buffer;
			if (!pktl)
				break;
			m_pFormatCtx->packet_buffer = pktl->next;
			av_free_packet(&pktl->pkt);
			av_free(pktl);
		}

		// Free Source Buffer
		if (m_pFormatCtx->pb)
		{
			if (m_pFormatCtx->pb->buffer)
				av_freep(&m_pFormatCtx->pb->buffer);
			m_pFormatCtx->pb->buffer_size = 0;
		}

		// Free Format Ctx
		av_freep(&m_pFormatCtx->priv_data);
		av_freep(&m_pFormatCtx);

		// Free Various
		for (DWORD dwStreamNum = 0 ; dwStreamNum < MAX_STREAMS ; dwStreamNum++)
		{
			// Free Conversion Context
			if (m_pImgConvertCtx[dwStreamNum])
			{
				sws_freeContext(m_pImgConvertCtx[dwStreamNum]);
				m_pImgConvertCtx[dwStreamNum] = NULL;
			}

			// Free the Frame Buffers
			if (m_pFrameBuf1[dwStreamNum])
			{
				delete [] m_pFrameBuf1[dwStreamNum];
				m_pFrameBuf1[dwStreamNum] = NULL;
				m_nFrameBufSize1[dwStreamNum] = 0;
			}
			if (m_pFrameBuf2[dwStreamNum])
			{
				delete [] m_pFrameBuf2[dwStreamNum];
				m_pFrameBuf2[dwStreamNum] = NULL;
				m_nFrameBufSize2[dwStreamNum] = 0;
			}
			if (m_pFrame[dwStreamNum])
			{
				av_free(m_pFrame[dwStreamNum]);
				m_pFrame[dwStreamNum] = NULL;
			}
			if (m_pFrameTemp[dwStreamNum])
			{
				av_free(m_pFrameTemp[dwStreamNum]);
				m_pFrameTemp[dwStreamNum] = NULL;
			}

			// Free Audio Buffers
			m_nAudioInputFrameSize[dwStreamNum] = 0;
			m_nIntermediateSamplesBufPos[dwStreamNum] = 0;
			if (m_pIntermediateSamplesBuf[dwStreamNum])
			{
				delete [] m_pIntermediateSamplesBuf[dwStreamNum];
				m_pIntermediateSamplesBuf[dwStreamNum] = NULL;
				m_nIntermediateSamplesBufSize[dwStreamNum] = 0;
			}
			if (m_pTempSamplesBuf[dwStreamNum])
			{
				delete [] m_pTempSamplesBuf[dwStreamNum];
				m_pTempSamplesBuf[dwStreamNum] = NULL;
				m_nTempSamplesBufSize[dwStreamNum] = 0;
			}
			if (m_pTempSamplesBuf2[dwStreamNum])
			{
				delete [] m_pTempSamplesBuf2[dwStreamNum];
				m_pTempSamplesBuf2[dwStreamNum] = NULL;
				m_nTempSamplesBufSize2[dwStreamNum] = 0;
			}
			if (m_pAudioResampleCtx[dwStreamNum])
			{
				audio_resample_close(m_pAudioResampleCtx[dwStreamNum]);
				m_pAudioResampleCtx[dwStreamNum] = NULL;
			}

			// Free Output Buffer
			if (m_pOutbuf[dwStreamNum])
			{
				delete [] m_pOutbuf[dwStreamNum];
				m_pOutbuf[dwStreamNum] = NULL;
				m_nOutbufSize[dwStreamNum] = 0;
			}
		}

		// Free the Output Format
		if (m_pOutputFormat)
		{
			delete [] m_pOutputFormat;
			m_pOutputFormat = NULL;
		}
	}

	for (DWORD dwStreamNum = 0 ; dwStreamNum < MAX_STREAMS ; dwStreamNum++)
	{
		// Free
		if (m_pSrcWaveFormat[dwStreamNum])
		{
			delete [] m_pSrcWaveFormat[dwStreamNum];
			m_pSrcWaveFormat[dwStreamNum] = NULL;
		}
		if (m_pIntermediateWaveFormat[dwStreamNum])
		{
			delete [] m_pIntermediateWaveFormat[dwStreamNum];
			m_pIntermediateWaveFormat[dwStreamNum] = NULL;
		}

		// Reset
		m_llTotalWrittenBytes[dwStreamNum] = 0;
		m_llTotalFramesOrSamples[dwStreamNum] = 0;

		// Close 2Pass Log Files
		if (m_p2PassLogFiles[dwStreamNum])
		{
			fclose(m_p2PassLogFiles[dwStreamNum]);
			m_p2PassLogFiles[dwStreamNum] = NULL;
		}

		// Delete File
		if (m_nPassNumber[dwStreamNum] == 2 && ::IsExistingFile(m_s2PassLogFileName[dwStreamNum]))
			::DeleteFile(m_s2PassLogFileName[dwStreamNum]);

		// Reset 2Pass Log File Name and Pass Number
		m_s2PassLogFileName[dwStreamNum] = _T("");
		m_nPassNumber[dwStreamNum] = 0;
	}

	m_sFileName = _T("");
	m_dwTotalVideoStreams = 0;
	m_dwTotalAudioStreams = 0;
	m_nGlobalPassNumber = 0;
	m_bFastEncode = false;
	m_bOpen = false;

	::LeaveCriticalSection(&m_csAVI);

	return res;
}

bool CAVRec::AddRawVideoPacket(DWORD dwStreamNum,
							   DWORD dwBytes,
							   LPBYTE pBuf,
							   bool bKeyframe,
							   bool bInterleaved)
{
	::EnterCriticalSection(&m_csAVI);

	// Check
	if (!m_pFormatCtx								||
		dwStreamNum >= m_pFormatCtx->nb_streams		||
		!m_pFormatCtx->streams[dwStreamNum]			||
		!m_pFormatCtx->streams[dwStreamNum]->codec	||
		m_pFormatCtx->streams[dwStreamNum]->codec->codec_type != CODEC_TYPE_VIDEO)
	{
		::LeaveCriticalSection(&m_csAVI);
		return false;
	}

	// Get the attached codec context
	AVCodecContext* pCodecCtx = m_pFormatCtx->streams[dwStreamNum]->codec;

	// Init Packet
	AVPacket pkt;
	av_init_packet(&pkt);
	pkt.size = dwBytes;
	pkt.data = (uint8_t *)pBuf;
	if (pCodecCtx->coded_frame && pCodecCtx->coded_frame->pts != AV_NOPTS_VALUE)
	{
		pkt.pts = av_rescale_q(	pCodecCtx->coded_frame->pts,
								pCodecCtx->time_base,
								m_pFormatCtx->streams[dwStreamNum]->time_base);
	}
	if (bKeyframe)
		pkt.flags |= PKT_FLAG_KEY;
	pkt.stream_index = m_pFormatCtx->streams[dwStreamNum]->index;

	// Write the packet to the media file
	if ((bInterleaved ?
		av_interleaved_write_frame(m_pFormatCtx, &pkt) :
		av_write_frame(m_pFormatCtx, &pkt)) != 0)
	{
		TRACE(_T("Error while writing video packet\n"));
		::LeaveCriticalSection(&m_csAVI);
		return false;
	}
	else
	{
		m_llTotalFramesOrSamples[dwStreamNum]++;
		m_llTotalWrittenBytes[dwStreamNum] += pkt.size;
		::LeaveCriticalSection(&m_csAVI);
		return true;
	}
}

bool CAVRec::AddFrame(	DWORD dwStreamNum,
						LPBITMAPINFO pBmi,
						LPBYTE pBits,
						bool bInterleaved)
{
	int ret;
	LONG lBytesWritten = 0;
	int nCurLine;

	::EnterCriticalSection(&m_csAVI);

	// Check
	if (!m_pFormatCtx								||
		dwStreamNum >= m_pFormatCtx->nb_streams		||
		!m_pFormatCtx->streams[dwStreamNum]			||
		!m_pFormatCtx->streams[dwStreamNum]->codec	||
		m_pFormatCtx->streams[dwStreamNum]->codec->codec_type != CODEC_TYPE_VIDEO)
	{
		::LeaveCriticalSection(&m_csAVI);
		return false;
	}

	// Get the attached codec context
	AVCodecContext* pCodecCtx = m_pFormatCtx->streams[dwStreamNum]->codec;

    if (!pBits || !pBmi)
	{
		// No more frame to compress. The codec has a latency of a few
		// frames if using B frames, so we get the last frames by
		// passing the same picture again
    }
	else
	{
		// Get Src Pixel Format
		enum PixelFormat SrcPixFormat = CAVIPlay::CAVIVideoStream::AVCodecBMIToPixFormat(pBmi);

		// Flip Vertically
		LPBYTE lpTopDownBits;
		if ((pBmi->bmiHeader.biCompression == BI_RGB		||	// (if input RGB)
			pBmi->bmiHeader.biCompression == BI_BITFIELDS)	
															&&	// and
			(pCodecCtx->codec_id != CODEC_ID_RAWVIDEO			// (output not RAW
															||	// or
			(pCodecCtx->codec_id == CODEC_ID_RAWVIDEO		&&	// (output RAW
			pCodecCtx->pix_fmt != PIX_FMT_RGB32				&&	// and not RGB))
			pCodecCtx->pix_fmt != PIX_FMT_BGR32				&&
			pCodecCtx->pix_fmt != PIX_FMT_RGB24				&&
			pCodecCtx->pix_fmt != PIX_FMT_BGR24				&&	 
			pCodecCtx->pix_fmt != PIX_FMT_RGB565			&&
			pCodecCtx->pix_fmt != PIX_FMT_RGB555			&&
			pCodecCtx->pix_fmt != PIX_FMT_BGR565			&&
			pCodecCtx->pix_fmt != PIX_FMT_BGR555			&&
			pCodecCtx->pix_fmt != PIX_FMT_PAL8)))
		{
			int nDWAlignedLineSize = DWALIGNEDWIDTHBYTES(pBmi->bmiHeader.biBitCount * pBmi->bmiHeader.biWidth);
			int nFlipBufSize = nDWAlignedLineSize * pBmi->bmiHeader.biHeight;
			if (!m_pFrameBuf1[dwStreamNum] || m_nFrameBufSize1[dwStreamNum] < nFlipBufSize)
			{
				if (m_pFrameBuf1[dwStreamNum])
					delete [] m_pFrameBuf1[dwStreamNum];
				m_pFrameBuf1[dwStreamNum] = new BYTE[nFlipBufSize + FF_INPUT_BUFFER_PADDING_SIZE];
				if (!m_pFrameBuf1[dwStreamNum])
				{
					m_nFrameBufSize1[dwStreamNum] = 0;
					::LeaveCriticalSection(&m_csAVI);
					return false;
				}
				m_nFrameBufSize1[dwStreamNum] = nFlipBufSize;
			}
			LPBYTE lpSrcBits = pBits;
			LPBYTE lpDstBits = m_pFrameBuf1[dwStreamNum] + (pBmi->bmiHeader.biHeight - 1) * nDWAlignedLineSize;
			for (nCurLine = 0 ; nCurLine < (int)pBmi->bmiHeader.biHeight ; nCurLine++)
			{
				memcpy((void*)lpDstBits, (void*)lpSrcBits, nDWAlignedLineSize); 
				lpSrcBits += nDWAlignedLineSize;
				lpDstBits -= nDWAlignedLineSize;
			}
			lpTopDownBits = m_pFrameBuf1[dwStreamNum];
		}
		else
			lpTopDownBits = pBits;

		// Convert to codec's pixel format or codec's size?
        if (pCodecCtx->pix_fmt != SrcPixFormat			||
			pBmi->bmiHeader.biWidth != pCodecCtx->width	||
			pBmi->bmiHeader.biHeight != pCodecCtx->height)
		{
            m_pImgConvertCtx[dwStreamNum] = sws_getCachedContext(m_pImgConvertCtx[dwStreamNum],
																pBmi->bmiHeader.biWidth,	// Src Width
																pBmi->bmiHeader.biHeight,	// Src Height
																SrcPixFormat,				// Src Format
																pCodecCtx->width,			// Dst Width
																pCodecCtx->height,			// Dst Height
																pCodecCtx->pix_fmt,			// Dst Format
																SWS_BICUBIC,
																NULL, NULL, NULL);
            if (!m_pImgConvertCtx[dwStreamNum])
			{
                TRACE(_T("Cannot initialize the conversion context\n"));
				::LeaveCriticalSection(&m_csAVI);
                return false;
            }

			// Init Src
			avpicture_fill(	(AVPicture*)(m_pFrameTemp[dwStreamNum]),
							(uint8_t*)lpTopDownBits,
							SrcPixFormat,
							pBmi->bmiHeader.biWidth,
							pBmi->bmiHeader.biHeight);

			// Flip U <-> V pointers
			if (pBmi->bmiHeader.biCompression == FCC('YV12') ||
				pBmi->bmiHeader.biCompression == FCC('YV16') ||
				pBmi->bmiHeader.biCompression == FCC('YVU9'))
			{
				uint8_t* pTemp = m_pFrameTemp[dwStreamNum]->data[1];
				m_pFrameTemp[dwStreamNum]->data[1] = m_pFrameTemp[dwStreamNum]->data[2];
				m_pFrameTemp[dwStreamNum]->data[2] = pTemp;
				// Line Sizes for U and V are the same no need to swap
			}

			// Init Dst
			int nDstSize = avpicture_get_size(	pCodecCtx->pix_fmt,
												pCodecCtx->width,
												pCodecCtx->height);
			int nNewFrameBufSize2 = avpicture_get_size(	pCodecCtx->pix_fmt,
														DOALIGN(pCodecCtx->width, 16),
														DOALIGN(pCodecCtx->height, 16));
			if (!m_pFrameBuf2[dwStreamNum] || m_nFrameBufSize2[dwStreamNum] < nNewFrameBufSize2)
			{
				if (m_pFrameBuf2[dwStreamNum])
					delete [] m_pFrameBuf2[dwStreamNum];
				m_pFrameBuf2[dwStreamNum] = new BYTE[nNewFrameBufSize2 + FF_INPUT_BUFFER_PADDING_SIZE];
				if (!m_pFrameBuf2[dwStreamNum])
				{
					m_nFrameBufSize2[dwStreamNum] = 0;
					::LeaveCriticalSection(&m_csAVI);
					return false;
				}
				m_nFrameBufSize2[dwStreamNum] = nNewFrameBufSize2;
			}
			avpicture_fill(	(AVPicture*)(m_pFrame[dwStreamNum]),
							(uint8_t*)(m_pFrameBuf2[dwStreamNum]),
							pCodecCtx->pix_fmt,
							pCodecCtx->width,
							pCodecCtx->height);

			// Convert
            int sws_scale_res = sws_scale(	m_pImgConvertCtx[dwStreamNum],
											m_pFrameTemp[dwStreamNum]->data,		// Src Pixels
											m_pFrameTemp[dwStreamNum]->linesize,	// Src Stride
											0,
											pBmi->bmiHeader.biHeight,
											m_pFrame[dwStreamNum]->data,			// Dst Pixels
											m_pFrame[dwStreamNum]->linesize);		// Dst Stride
#ifdef SUPPORT_LIBSWSCALE
			if (sws_scale_res <= 0)
			{
				::LeaveCriticalSection(&m_csAVI);
				return false;
			}
#else
			if (sws_scale_res < 0)
			{
				::LeaveCriticalSection(&m_csAVI);
				return false;
			}
#endif
        }
		else
		{
			avpicture_fill(	(AVPicture*)(m_pFrame[dwStreamNum]),
							(uint8_t*)lpTopDownBits,
							pCodecCtx->pix_fmt,
							pCodecCtx->width,
							pCodecCtx->height);

			// Flip U <-> V pointers
			if (pBmi->bmiHeader.biCompression == FCC('YV12') ||
				pBmi->bmiHeader.biCompression == FCC('YV16') ||
				pBmi->bmiHeader.biCompression == FCC('YVU9'))
			{
				uint8_t* pTemp = m_pFrame[dwStreamNum]->data[1];
				m_pFrame[dwStreamNum]->data[1] = m_pFrame[dwStreamNum]->data[2];
				m_pFrame[dwStreamNum]->data[2] = pTemp;
				// Line Sizes for U and V are the same no need to swap
			}
		}
    }

    // Raw video case
	if (pCodecCtx->codec_id == CODEC_ID_RAWVIDEO)
	{
		// Flip U <-> V pointers
		if (pCodecCtx->codec_tag == FCC('YV12') ||
			pCodecCtx->codec_tag == FCC('YV16') ||
			pCodecCtx->codec_tag == FCC('YVU9'))
		{
			uint8_t* pTemp = m_pFrame[dwStreamNum]->data[1];
			m_pFrame[dwStreamNum]->data[1] = m_pFrame[dwStreamNum]->data[2];
			m_pFrame[dwStreamNum]->data[2] = pTemp;
			// Line Sizes for U and V are the same no need to swap
		}

        AVPacket pkt;
        av_init_packet(&pkt);
		if (pCodecCtx->coded_frame && pCodecCtx->coded_frame->pts != AV_NOPTS_VALUE)
		{
			pkt.pts = av_rescale_q(	pCodecCtx->coded_frame->pts,
									pCodecCtx->time_base,
									m_pFormatCtx->streams[dwStreamNum]->time_base);
		}
		pkt.flags |= PKT_FLAG_KEY;
        pkt.stream_index = m_pFormatCtx->streams[dwStreamNum]->index;

		// Flip Vertically: if input not RGB and output RGB
		if ((pBmi->bmiHeader.biCompression != BI_RGB		&&
			pBmi->bmiHeader.biCompression != BI_BITFIELDS)	&&
			(pCodecCtx->pix_fmt == PIX_FMT_RGB32			||
			pCodecCtx->pix_fmt == PIX_FMT_BGR32				||
			pCodecCtx->pix_fmt == PIX_FMT_RGB24				||
			pCodecCtx->pix_fmt == PIX_FMT_BGR24				||	 
			pCodecCtx->pix_fmt == PIX_FMT_RGB565			||
			pCodecCtx->pix_fmt == PIX_FMT_RGB555			||
			pCodecCtx->pix_fmt == PIX_FMT_BGR565			||
			pCodecCtx->pix_fmt == PIX_FMT_BGR555			||
			pCodecCtx->pix_fmt == PIX_FMT_PAL8))
		{
			int nBitCount;
			if (pCodecCtx->pix_fmt == PIX_FMT_RGB32			||
				pCodecCtx->pix_fmt == PIX_FMT_BGR32)
				nBitCount = 32;
			else if (pCodecCtx->pix_fmt == PIX_FMT_RGB24	||
					pCodecCtx->pix_fmt == PIX_FMT_BGR24)
				nBitCount = 24;
			else if (pCodecCtx->pix_fmt == PIX_FMT_PAL8)
				nBitCount = 8;
			else
				nBitCount = 16;
			int nDWAlignedLineSize = DWALIGNEDWIDTHBYTES(nBitCount * pCodecCtx->width);
			LPBYTE lpSrcBits = (LPBYTE)m_pFrame[dwStreamNum]->data[0];
			LPBYTE lpDstBits = (LPBYTE)(m_pOutbuf[dwStreamNum] + (pCodecCtx->height - 1) * nDWAlignedLineSize);
			for (nCurLine = 0 ; nCurLine < pCodecCtx->height ; nCurLine++)
			{
				memcpy((void*)lpDstBits, (void*)lpSrcBits, m_pFrame[dwStreamNum]->linesize[0]); 
				lpSrcBits += m_pFrame[dwStreamNum]->linesize[0];
				lpDstBits -= nDWAlignedLineSize;
			}
			pkt.data = m_pOutbuf[dwStreamNum];
			pkt.size = nDWAlignedLineSize * pCodecCtx->height;
		}
		else
		{
			// Flip U <-> V Buffers
			if (m_pFrame[dwStreamNum]->data[1] > m_pFrame[dwStreamNum]->data[2])
			{
				LPBYTE lpSrcBits = (LPBYTE)m_pFrame[dwStreamNum]->data[0];
				LPBYTE lpDstBits = (LPBYTE)m_pOutbuf[dwStreamNum];
				for (nCurLine = 0 ; nCurLine < pCodecCtx->height ; nCurLine++)
				{
					memcpy((void*)lpDstBits, (void*)lpSrcBits, m_pFrame[dwStreamNum]->linesize[0]); 
					lpSrcBits += m_pFrame[dwStreamNum]->linesize[0];
					lpDstBits += m_pFrame[dwStreamNum]->linesize[0];
				}
				int nChromaHeight = pCodecCtx->height;
				if (pCodecCtx->codec_tag == FCC('YV16') || pCodecCtx->codec_tag == FCC('Y42B'))
					nChromaHeight >>= 0;
				else if (pCodecCtx->codec_tag == FCC('YV12') || pCodecCtx->codec_tag == FCC('I420') || pCodecCtx->codec_tag == FCC('IYUV'))
					nChromaHeight >>= 1;
				else
					nChromaHeight >>= 2;
				lpSrcBits = (LPBYTE)m_pFrame[dwStreamNum]->data[1];
				for (nCurLine = 0 ; nCurLine < nChromaHeight ; nCurLine++)
				{
					memcpy((void*)lpDstBits, (void*)lpSrcBits, m_pFrame[dwStreamNum]->linesize[1]); 
					lpSrcBits += m_pFrame[dwStreamNum]->linesize[1];
					lpDstBits += m_pFrame[dwStreamNum]->linesize[1];
				}
				lpSrcBits = (LPBYTE)m_pFrame[dwStreamNum]->data[2];
				for (nCurLine = 0 ; nCurLine < nChromaHeight ; nCurLine++)
				{
					memcpy((void*)lpDstBits, (void*)lpSrcBits, m_pFrame[dwStreamNum]->linesize[2]); 
					lpSrcBits += m_pFrame[dwStreamNum]->linesize[2];
					lpDstBits += m_pFrame[dwStreamNum]->linesize[2];
				}
				pkt.data = m_pOutbuf[dwStreamNum];
			}
			else
				pkt.data = m_pFrame[dwStreamNum]->data[0];
			if (pCodecCtx->pix_fmt == PIX_FMT_PAL8)
				pkt.size = DWALIGNEDWIDTHBYTES(8 * pCodecCtx->width) * pCodecCtx->height;
			else
				pkt.size = avpicture_get_size(	pCodecCtx->pix_fmt,
												pCodecCtx->width,
												pCodecCtx->height);
		}

		// Write the raw frame to the media file
        ret =	bInterleaved ?
				av_interleaved_write_frame(m_pFormatCtx, &pkt) :
				av_write_frame(m_pFormatCtx, &pkt);
		if (ret == 0)
			lBytesWritten += pkt.size;
    }
	else
	{
		// Set Quality
		if (pCodecCtx->flags & CODEC_FLAG_QSCALE)
			m_pFrame[dwStreamNum]->quality = (int)m_pFormatCtx->streams[dwStreamNum]->quality;

        // Encode the image
        int out_size = avcodec_encode_video(pCodecCtx,
											m_pOutbuf[dwStreamNum],
											m_nOutbufSize[dwStreamNum],
											m_pFrame[dwStreamNum]);
        
		// If zero size, it means the image was buffered (if B frames enabled)
		// or an empty delta frame was issued by libtheora for example
        if (out_size >= 0)
		{
            AVPacket pkt;
            av_init_packet(&pkt);
			if (pCodecCtx->coded_frame && pCodecCtx->coded_frame->pts != AV_NOPTS_VALUE)
			{
				pkt.pts = av_rescale_q(	pCodecCtx->coded_frame->pts,
										pCodecCtx->time_base,
										m_pFormatCtx->streams[dwStreamNum]->time_base);
			}
            if (pCodecCtx->coded_frame->key_frame)
                pkt.flags |= PKT_FLAG_KEY;
            pkt.stream_index = m_pFormatCtx->streams[dwStreamNum]->index;
            pkt.data = m_pOutbuf[dwStreamNum];
            pkt.size = out_size;

            // Write the compressed frame to the media file
			ret =	bInterleaved ?
					av_interleaved_write_frame(m_pFormatCtx, &pkt) :
					av_write_frame(m_pFormatCtx, &pkt);
			if (ret == 0)
				lBytesWritten += pkt.size;

			// Output Status to Log File
            if (m_p2PassLogFiles[dwStreamNum] && pCodecCtx->stats_out)
                fprintf(m_p2PassLogFiles[dwStreamNum], "%s", pCodecCtx->stats_out);
        }
		else
            ret = 0;
    }

    if (ret != 0)
	{
		::LeaveCriticalSection(&m_csAVI);
		return false;
	}
	else
	{
		m_llTotalFramesOrSamples[dwStreamNum]++;
		m_llTotalWrittenBytes[dwStreamNum] += lBytesWritten;
		::LeaveCriticalSection(&m_csAVI);
		return true;
	}
}

void CAVRec::TheoraStats(DWORD dwStreamNum)
{
	if (m_pFormatCtx && m_pFormatCtx->streams[dwStreamNum])
	{
		AVCodecContext* pCodecCtx = m_pFormatCtx->streams[dwStreamNum]->codec;
		if (pCodecCtx)
		{
			avcodec_encode_video(	pCodecCtx,
									m_pOutbuf[dwStreamNum],
									m_nOutbufSize[dwStreamNum],
									NULL);
			if (m_p2PassLogFiles[dwStreamNum] && pCodecCtx->stats_out)
				fprintf(m_p2PassLogFiles[dwStreamNum], "%s", pCodecCtx->stats_out);
		}
	}
}

bool CAVRec::AddRawAudioPacket(DWORD dwStreamNum,
							   DWORD dwBytes,
							   LPBYTE pBuf,
							   bool bInterleaved)
{
	::EnterCriticalSection(&m_csAVI);

	// Check
	if (!m_pFormatCtx								||
		dwStreamNum >= m_pFormatCtx->nb_streams		||
		!m_pFormatCtx->streams[dwStreamNum]			||
		!m_pFormatCtx->streams[dwStreamNum]->codec	||
		m_pFormatCtx->streams[dwStreamNum]->codec->codec_type != CODEC_TYPE_AUDIO)
	{
		::LeaveCriticalSection(&m_csAVI);
		return false;
	}

	// Get the attached codec context
	AVCodecContext* pCodecCtx = m_pFormatCtx->streams[dwStreamNum]->codec;

	// Init Packet
	AVPacket pkt;
	av_init_packet(&pkt);
	pkt.size = dwBytes;
	pkt.data = (uint8_t *)pBuf;
	if (pCodecCtx->coded_frame && pCodecCtx->coded_frame->pts != AV_NOPTS_VALUE)
	{
		pkt.pts = av_rescale_q(	pCodecCtx->coded_frame->pts,
								pCodecCtx->time_base,
								m_pFormatCtx->streams[dwStreamNum]->time_base);
	}
	pkt.flags |= PKT_FLAG_KEY;
	pkt.stream_index = m_pFormatCtx->streams[dwStreamNum]->index;

	// Write the packet to the media file
	if ((bInterleaved ?
		av_interleaved_write_frame(m_pFormatCtx, &pkt) :
		av_write_frame(m_pFormatCtx, &pkt)) != 0)
	{
		TRACE(_T("Error while writing audio packet\n"));
		::LeaveCriticalSection(&m_csAVI);
		return false;
	}
	else
	{
		if (pCodecCtx->codec_tag == WAVE_FORMAT_PCM && pCodecCtx->channels > 0)
		{
			if (pCodecCtx->sample_fmt == SAMPLE_FMT_U8)
				m_llTotalFramesOrSamples[dwStreamNum] += pkt.size / pCodecCtx->channels;
			else
				m_llTotalFramesOrSamples[dwStreamNum] += pkt.size / (2 * pCodecCtx->channels);
		}
		m_llTotalWrittenBytes[dwStreamNum] += pkt.size;
		::LeaveCriticalSection(&m_csAVI);
		return true;
	}
}

bool CAVRec::AddAudioSamples(	DWORD dwStreamNum,
								DWORD dwNumSamples,
								LPBYTE pBuf,
								bool bInterleaved)
{
	bool res;

	::EnterCriticalSection(&m_csAVI);

	// Check
	if (!m_pFormatCtx								||
		dwStreamNum >= m_pFormatCtx->nb_streams		||
		!m_pFormatCtx->streams[dwStreamNum]			||
		!m_pFormatCtx->streams[dwStreamNum]->codec	||
		m_pFormatCtx->streams[dwStreamNum]->codec->codec_type != CODEC_TYPE_AUDIO)
	{
		::LeaveCriticalSection(&m_csAVI);
		return false;
	}

	// No resample or channels count change?
	if ((m_pIntermediateWaveFormat[dwStreamNum]->nSamplesPerSec == m_pSrcWaveFormat[dwStreamNum]->nSamplesPerSec	&&
		m_pIntermediateWaveFormat[dwStreamNum]->nChannels == m_pSrcWaveFormat[dwStreamNum]->nChannels)				||
		dwNumSamples == 0																							||
		pBuf == NULL)
	{
		res = AddAudioSamplesDirect(dwStreamNum,
									dwNumSamples,
									pBuf,
									bInterleaved);
		::LeaveCriticalSection(&m_csAVI);
		return res;
	}
	else
	{
		// Current Samples Buffer
		uint8_t* pCurrentSamplesBuf = (uint8_t*)pBuf;

		// 8 bits -> 16 bits
		if (m_pSrcWaveFormat[dwStreamNum]->wBitsPerSample == 8)
		{
			int nTempBufSize = 2 * m_pSrcWaveFormat[dwStreamNum]->nChannels * (int)dwNumSamples;
			if (!m_pTempSamplesBuf[dwStreamNum] || m_nTempSamplesBufSize[dwStreamNum] < nTempBufSize)
			{
				nTempBufSize = 3 * nTempBufSize / 2; // To avoid a realloc next time!
				if (m_pTempSamplesBuf[dwStreamNum])
					delete [] m_pTempSamplesBuf[dwStreamNum];
				m_pTempSamplesBuf[dwStreamNum] = new uint8_t[nTempBufSize + FF_INPUT_BUFFER_PADDING_SIZE];
				if (!m_pTempSamplesBuf[dwStreamNum])
				{
					m_nTempSamplesBufSize[dwStreamNum] = 0;
					::LeaveCriticalSection(&m_csAVI);
					return false;
				}
				m_nTempSamplesBufSize[dwStreamNum] = nTempBufSize;
			}
			uint8_t* pi = pCurrentSamplesBuf;
			int16_t* po = (int16_t*)(m_pTempSamplesBuf[dwStreamNum]);
			for (DWORD dwSample = 0 ; dwSample < dwNumSamples ; dwSample++)
			{
				for (int ch = 0 ; ch < m_pSrcWaveFormat[dwStreamNum]->nChannels ; ch++)
				{
					*po = (int16_t)((*pi - 0x80) << 8);
					pi++; po++;
				}
			}

			// Update Current Samples Buffer
			pCurrentSamplesBuf = m_pTempSamplesBuf[dwStreamNum];
		}

		// Resample
		int nOutSamplesCount = 0;
		int nTempBufSize2 = 5 * ::MulDiv(	m_pIntermediateWaveFormat[dwStreamNum]->nSamplesPerSec,
											(int)dwNumSamples,
											m_pSrcWaveFormat[dwStreamNum]->nSamplesPerSec);
		if (!m_pTempSamplesBuf2[dwStreamNum] || m_nTempSamplesBufSize2[dwStreamNum] < nTempBufSize2)
		{
			nTempBufSize2 = 3 * nTempBufSize2 / 2; // To avoid a realloc next time!
			if (m_pTempSamplesBuf2[dwStreamNum])
				delete [] m_pTempSamplesBuf2[dwStreamNum];
			m_pTempSamplesBuf2[dwStreamNum] = new uint8_t[nTempBufSize2 + FF_INPUT_BUFFER_PADDING_SIZE];
			if (!m_pTempSamplesBuf2[dwStreamNum])
			{
				m_nTempSamplesBufSize2[dwStreamNum] = 0;
				::LeaveCriticalSection(&m_csAVI);
				return false;
			}
			m_nTempSamplesBufSize2[dwStreamNum] = nTempBufSize2;
		}
		if (!m_pAudioResampleCtx[dwStreamNum])
		{
			m_pAudioResampleCtx[dwStreamNum] =  audio_resample_init(m_pIntermediateWaveFormat[dwStreamNum]->nChannels,		// Output channels
																	m_pSrcWaveFormat[dwStreamNum]->nChannels,				// Input channels
																	m_pIntermediateWaveFormat[dwStreamNum]->nSamplesPerSec,	// Output samplerate
																	m_pSrcWaveFormat[dwStreamNum]->nSamplesPerSec);			// Input samplerate
		}
		if (m_pAudioResampleCtx[dwStreamNum])
		{
			// Note: first run always consumes 8 input samples, that's the audio_resample() queue
			nOutSamplesCount = audio_resample(m_pAudioResampleCtx[dwStreamNum],
											  (short*)m_pTempSamplesBuf2[dwStreamNum],								// Output buffer
											  (short*)pCurrentSamplesBuf,											// Input buffer
											  (int)dwNumSamples);													// Number of input samples
		}
		res = AddAudioSamplesDirect(	dwStreamNum,
										nOutSamplesCount,
										(LPBYTE)m_pTempSamplesBuf2[dwStreamNum],
										bInterleaved);
		::LeaveCriticalSection(&m_csAVI);
		return res;
	}
}

bool CAVRec::AddAudioSamplesDirect(	DWORD dwStreamNum,
									DWORD dwNumSamples,
									LPBYTE pBuf,
									bool bInterleaved)
{
	int nSamplesCopied = 0;
	LONG lBytesWritten = 0;
	DWORD dwSamplesWritten = 0;

	// Get the attached codec context
	AVCodecContext* pCodecCtx = m_pFormatCtx->streams[dwStreamNum]->codec;

	// Copy Samples to Buffer
	if (dwNumSamples > 0 && pBuf)
	{
		memcpy(	m_pIntermediateSamplesBuf[dwStreamNum] + m_pIntermediateWaveFormat[dwStreamNum]->nBlockAlign * m_nIntermediateSamplesBufPos[dwStreamNum],
				pBuf,
				m_pIntermediateWaveFormat[dwStreamNum]->nBlockAlign * MIN((int)dwNumSamples, m_nAudioInputFrameSize[dwStreamNum] - m_nIntermediateSamplesBufPos[dwStreamNum]));
		nSamplesCopied = MIN((int)dwNumSamples, m_nAudioInputFrameSize[dwStreamNum] - m_nIntermediateSamplesBufPos[dwStreamNum]);
		m_nIntermediateSamplesBufPos[dwStreamNum] += nSamplesCopied;
	}

	// Start encoding if enough samples
	while ((m_nIntermediateSamplesBufPos[dwStreamNum] == m_nAudioInputFrameSize[dwStreamNum])	||
			dwNumSamples == 0																	||
			pBuf == NULL)
	{
		// Encode Audio
		AVPacket pkt;
		av_init_packet(&pkt);

		// Encode Last Samples
		if (dwNumSamples == 0 || pBuf == NULL)
		{
			if (m_nIntermediateSamplesBufPos[dwStreamNum] == 0)
				return true;
			if (pCodecCtx->frame_size <= 1)
			{
				int nOutbufSize = m_nIntermediateSamplesBufPos[dwStreamNum] * pCodecCtx->channels * ((pCodecCtx->sample_fmt == SAMPLE_FMT_U8) ? 1 : 2);
				if (!EncodeSamples(dwStreamNum, m_nIntermediateSamplesBufPos[dwStreamNum], nOutbufSize, &pkt))
				{
					TRACE(_T("Error while encoding audio frame\n"));
					return false;
				}
			}
			else
			{
				int nZeroSamples = m_nAudioInputFrameSize[dwStreamNum] - m_nIntermediateSamplesBufPos[dwStreamNum];
				if (m_pIntermediateWaveFormat[m_pFormatCtx->nb_streams - 1]->wBitsPerSample == 8)
				{
					memset(	m_pIntermediateSamplesBuf[dwStreamNum] + m_pIntermediateWaveFormat[dwStreamNum]->nBlockAlign * m_nIntermediateSamplesBufPos[dwStreamNum],
							0x80,
							nZeroSamples * m_pIntermediateWaveFormat[m_pFormatCtx->nb_streams - 1]->nBlockAlign);
				}
				else
				{
					memset(	m_pIntermediateSamplesBuf[dwStreamNum] + m_pIntermediateWaveFormat[dwStreamNum]->nBlockAlign * m_nIntermediateSamplesBufPos[dwStreamNum],
							0,
							nZeroSamples * m_pIntermediateWaveFormat[m_pFormatCtx->nb_streams - 1]->nBlockAlign);
				}
				m_nIntermediateSamplesBufPos[dwStreamNum] += nZeroSamples;
				if (!EncodeSamples(dwStreamNum, m_nIntermediateSamplesBufPos[dwStreamNum], m_nOutbufSize[dwStreamNum], &pkt))
				{
					TRACE(_T("Error while encoding audio frame\n"));
					return false;
				}
			}
		}
		else
		{
			if (!EncodeSamples(dwStreamNum, m_nIntermediateSamplesBufPos[dwStreamNum], m_nOutbufSize[dwStreamNum], &pkt))
			{
				TRACE(_T("Error while encoding audio frame\n"));
				return false;
			}
		}

		// Write the compressed frame to the media file
		if ((bInterleaved ?
			av_interleaved_write_frame(m_pFormatCtx, &pkt) :
			av_write_frame(m_pFormatCtx, &pkt)) != 0)
		{
			TRACE(_T("Error while writing audio frame\n"));
			return false;
		}
		else
		{
			lBytesWritten += pkt.size;
			dwSamplesWritten += m_nIntermediateSamplesBufPos[dwStreamNum];
			m_nIntermediateSamplesBufPos[dwStreamNum] = 0; 
		}

		// Done?
		if (dwNumSamples == 0 || pBuf == NULL)
			break;

		// Copy Samples to Buffer
		if (((int)dwNumSamples - nSamplesCopied > 0) && pBuf)
		{
			memcpy(	m_pIntermediateSamplesBuf[dwStreamNum],
					pBuf + m_pIntermediateWaveFormat[dwStreamNum]->nBlockAlign * nSamplesCopied,
					m_pIntermediateWaveFormat[dwStreamNum]->nBlockAlign * MIN((int)dwNumSamples - nSamplesCopied, m_nAudioInputFrameSize[dwStreamNum]));
			m_nIntermediateSamplesBufPos[dwStreamNum] = MIN((int)dwNumSamples - nSamplesCopied, m_nAudioInputFrameSize[dwStreamNum]);
			nSamplesCopied += m_nIntermediateSamplesBufPos[dwStreamNum];
		}
	}

	m_llTotalFramesOrSamples[dwStreamNum] += dwSamplesWritten;
	m_llTotalWrittenBytes[dwStreamNum] += lBytesWritten;
	return true;
}

__forceinline bool CAVRec::EncodeSamples(DWORD dwStreamNum, int nInputSamplesCount, int nOutputBufSize, AVPacket* pPkt)
{
	// Get the attached codec context
	AVCodecContext* pCodecCtx = m_pFormatCtx->streams[dwStreamNum]->codec;

	// 8 bits -> 16 bits
	if (m_pIntermediateWaveFormat[dwStreamNum]->wBitsPerSample == 8 && pCodecCtx->sample_fmt == SAMPLE_FMT_S16)
	{
		int nTempBufSize = 2 * m_pIntermediateWaveFormat[dwStreamNum]->nChannels * nInputSamplesCount;
		if (!m_pTempSamplesBuf[dwStreamNum] || m_nTempSamplesBufSize[dwStreamNum] < nTempBufSize)
		{
			nTempBufSize = 3 * nTempBufSize / 2; // To avoid a realloc next time!
			if (m_pTempSamplesBuf[dwStreamNum])
				delete [] m_pTempSamplesBuf[dwStreamNum];
			m_pTempSamplesBuf[dwStreamNum] = new uint8_t[nTempBufSize + FF_INPUT_BUFFER_PADDING_SIZE];
			if (!m_pTempSamplesBuf[dwStreamNum])
			{
				m_nTempSamplesBufSize[dwStreamNum] = 0;
				return false;
			}
			m_nTempSamplesBufSize[dwStreamNum] = nTempBufSize;
		}
		uint8_t* pi = (uint8_t*)m_pIntermediateSamplesBuf[dwStreamNum];
		int16_t* po = (int16_t*)(m_pTempSamplesBuf[dwStreamNum]);
		for (int nSample = 0 ; nSample < nInputSamplesCount ; nSample++)
		{
			for (int ch = 0 ; ch < m_pIntermediateWaveFormat[dwStreamNum]->nChannels ; ch++)
			{
				*po = (int16_t)((*pi - 0x80) << 8);
				pi++; po++;
			}
		}

		// Encode
		if (pCodecCtx->codec_id == CODEC_ID_PCM_S16LE ||
			pCodecCtx->codec_id == CODEC_ID_PCM_U8)
		{
			memcpy(m_pOutbuf[dwStreamNum], m_pTempSamplesBuf[dwStreamNum], nOutputBufSize);
			pPkt->size = nOutputBufSize;
		}
		else
		{
			pPkt->size = avcodec_encode_audio(pCodecCtx,
											m_pOutbuf[dwStreamNum],
											nOutputBufSize,
											(const short*)m_pTempSamplesBuf[dwStreamNum]);
		}
	}
	// 16 bits -> 8 bits
	else if (m_pIntermediateWaveFormat[dwStreamNum]->wBitsPerSample == 16 && pCodecCtx->sample_fmt == SAMPLE_FMT_U8)
	{
		int nTempBufSize = pCodecCtx->channels * nInputSamplesCount;
		if (!m_pTempSamplesBuf[dwStreamNum] || m_nTempSamplesBufSize[dwStreamNum] < nTempBufSize)
		{
			nTempBufSize = 3 * nTempBufSize / 2; // To avoid a realloc next time!
			if (m_pTempSamplesBuf[dwStreamNum])
				delete [] m_pTempSamplesBuf[dwStreamNum];
			m_pTempSamplesBuf[dwStreamNum] = new uint8_t[nTempBufSize + FF_INPUT_BUFFER_PADDING_SIZE];
			if (!m_pTempSamplesBuf[dwStreamNum])
			{
				m_nTempSamplesBufSize[dwStreamNum] = 0;
				return false;
			}
			m_nTempSamplesBufSize[dwStreamNum] = nTempBufSize;
		}
		int16_t* pi = (int16_t*)m_pIntermediateSamplesBuf[dwStreamNum];
		uint8_t* po = (uint8_t*)(m_pTempSamplesBuf[dwStreamNum]);
		for (int nSample = 0 ; nSample < nInputSamplesCount ; nSample++)
		{
			for (int ch = 0 ; ch < pCodecCtx->channels ; ch++)
			{
				*po = (uint8_t)(((*pi)>>8) + 0x80);
				pi++; po++;
			}
		}

		// Encode
		if (pCodecCtx->codec_id == CODEC_ID_PCM_S16LE ||
			pCodecCtx->codec_id == CODEC_ID_PCM_U8)
		{
			memcpy(m_pOutbuf[dwStreamNum], m_pTempSamplesBuf[dwStreamNum], nOutputBufSize);
			pPkt->size = nOutputBufSize;
		}
		else
		{
			pPkt->size = avcodec_encode_audio(pCodecCtx,
											m_pOutbuf[dwStreamNum],
											nOutputBufSize,
											(const short*)m_pTempSamplesBuf[dwStreamNum]);
		}
	}
	else
	{
		// Encode
		if (pCodecCtx->codec_id == CODEC_ID_PCM_S16LE ||
			pCodecCtx->codec_id == CODEC_ID_PCM_U8)
		{
			memcpy(m_pOutbuf[dwStreamNum], m_pIntermediateSamplesBuf[dwStreamNum], nOutputBufSize);
			pPkt->size = nOutputBufSize;
		}
		else
		{
			pPkt->size = avcodec_encode_audio(pCodecCtx,
											m_pOutbuf[dwStreamNum],
											nOutputBufSize,
											(const short*)m_pIntermediateSamplesBuf[dwStreamNum]);
		}
	}

	// Rescale & set various data
	if (pCodecCtx->coded_frame && pCodecCtx->coded_frame->pts != AV_NOPTS_VALUE)
	{
		pPkt->pts = av_rescale_q(pCodecCtx->coded_frame->pts,
								pCodecCtx->time_base,
								m_pFormatCtx->streams[dwStreamNum]->time_base);
	}
	pPkt->flags |= PKT_FLAG_KEY;
	pPkt->stream_index = m_pFormatCtx->streams[dwStreamNum]->index;
	pPkt->data = m_pOutbuf[dwStreamNum];

	return true;
}
