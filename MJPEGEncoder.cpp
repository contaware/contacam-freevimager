#include "stdafx.h"
#include "MJPEGEncoder.h"
#include "AVRec.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef VIDEODEVICEDOC

// Defined in uImager.cpp
int avcodec_open_thread_safe(AVCodecContext *avctx, AVCodec *codec);
int avcodec_close_thread_safe(AVCodecContext *avctx);

BOOL CMJPEGEncoder::Open(LPBITMAPINFO pSrcBMI, int nThreadCount)
{
	// Already open?
	if (m_pCodecCtx)
		return TRUE;

	// Get codec
	m_pCodec = avcodec_find_encoder(AV_CODEC_ID_MJPEG);
	if (!m_pCodec)
		goto error;

	// Allocate Context
	/* if m_pCodec non-NULL, allocate private data and initialize defaults
	 * for the given codec. It is illegal to then call avcodec_open2()
	 * with a different codec.
	 * If NULL, then the codec-specific defaults won't be initialized,
	 * which may result in suboptimal default settings (this is
	 * important mainly for encoders, e.g. libx264).
	 */
	m_pCodecCtx = avcodec_alloc_context3(m_pCodec);
	if (!m_pCodecCtx)
		goto error;

	// Set Format
	m_pCodecCtx->width = pSrcBMI->bmiHeader.biWidth;
	m_pCodecCtx->height = pSrcBMI->bmiHeader.biHeight;
	m_pCodecCtx->codec_id = AV_CODEC_ID_MJPEG;
	m_pCodecCtx->pix_fmt = CAVRec::AVCodecBMIToPixFormat(pSrcBMI);
	m_pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
	m_pCodecCtx->strict_std_compliance = FF_COMPLIANCE_UNOFFICIAL; // to allow the AV_PIX_FMT_YUV420P, AV_PIX_FMT_YUV422P and AV_PIX_FMT_YUV444P formats
	m_pCodecCtx->flags |= CODEC_FLAG_QSCALE;
	m_pCodecCtx->bit_rate = 0;
	m_pCodecCtx->time_base.den = 1;
	m_pCodecCtx->time_base.num = 1;

	// Set thread count
	m_pCodecCtx->thread_count = nThreadCount;
	if (m_pCodecCtx->thread_count > 1)
		m_pCodecCtx->thread_type = FF_THREAD_SLICE;

	// Open codec
	if (avcodec_open_thread_safe(m_pCodecCtx, m_pCodec) < 0)
		goto error;

    // Allocate video frame
    m_pFrame = av_frame_alloc();
	if (!m_pFrame)
        goto error;

	// Allocate Outbuf
	m_nOutbufSize = 4 * m_pCodecCtx->width * m_pCodecCtx->height;
	if (m_nOutbufSize < FF_MIN_BUFFER_SIZE)
		m_nOutbufSize = FF_MIN_BUFFER_SIZE;
	m_pOutbuf = (uint8_t*)av_malloc(m_nOutbufSize + FF_INPUT_BUFFER_PADDING_SIZE);
	if (!m_pOutbuf)
		goto error;

	return TRUE;

error:
	Close();
	return FALSE;
}

void CMJPEGEncoder::Close()
{
	if (m_pCodecCtx)
	{
		/*
		Close a given AVCodecContext and free all the data associated with it
		(but not the AVCodecContext itself).
		Calling this function on an AVCodecContext that hasn't been opened will free
		the codec-specific data allocated in avcodec_alloc_context3() /
		avcodec_get_context_defaults3() with a non-NULL codec. Subsequent calls will
		do nothing.
		*/
		avcodec_close_thread_safe(m_pCodecCtx);
		av_freep(&m_pCodecCtx);
	}
	m_pCodec = NULL;

	if (m_pFrame)
		av_frame_free(&m_pFrame);

	if (m_pOutbuf)
	{
		av_free(m_pOutbuf);
		m_pOutbuf = NULL;
	}
	m_nOutbufSize = 0;

	memset(&m_SrcBMI, 0, sizeof(BITMAPINFOFULL));
}

DWORD CMJPEGEncoder::Encode(int qscale, LPBITMAPINFO pSrcBMI, LPBYTE pSrcBits, int nThreadCount)
{
	// Check
	if (!pSrcBMI || !pSrcBits)
		return 0U;

	// Clip: 31 comes from m_pCodecCtx->qmax, and 2 from m_pCodecCtx->qmin
	if (qscale > 31)
		qscale = 31;
	else if (qscale < 2)
		qscale = 2;

	// Limit threads to use
	// Note: in ffmpeg source MAX_THREADS definition differs
	// from header to header ... 32 is a safe value
	nThreadCount = MIN(nThreadCount, 32);

	// Re-open?
	if (!CDib::IsSameBMI(pSrcBMI, (LPBITMAPINFO)(&m_SrcBMI)) ||
		(m_pCodecCtx && (m_pCodecCtx->thread_count != nThreadCount)))
		Close();

	// Open?
	if (!m_pCodecCtx)
	{
		if (!Open(pSrcBMI, nThreadCount))
			return 0U;
	}

	// Encode
	avpicture_fill(	(AVPicture*)m_pFrame,
					(uint8_t*)pSrcBits,
					CAVRec::AVCodecBMIToPixFormat(pSrcBMI),
					pSrcBMI->bmiHeader.biWidth,
					pSrcBMI->bmiHeader.biHeight);
	m_pFrame->quality = FF_QP2LAMBDA * qscale;
	AVPacket avpkt;
	av_init_packet(&avpkt);
	avpkt.data = m_pOutbuf;
	avpkt.size = m_nOutbufSize;
	int got_output = 0;
	if (avcodec_encode_video2(m_pCodecCtx, &avpkt, m_pFrame, &got_output) == 0 && got_output == 1)
	{
		// Store current format
		memcpy(&m_SrcBMI, pSrcBMI, MIN(CDib::GetBMISize(pSrcBMI), sizeof(BITMAPINFOFULL)));
		DWORD dwSize = avpkt.size;
		av_free_packet(&avpkt);
		return dwSize;
	}
	else
	{
		av_free_packet(&avpkt);
		return 0U;
	}
}

#endif