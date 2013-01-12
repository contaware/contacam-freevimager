#include "stdafx.h"
#include "AviPlay.h"
#include "MJPEGEncoder.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef VIDEODEVICEDOC

// Defined in uImager.cpp
int avcodec_open_thread_safe(AVCodecContext *avctx, AVCodec *codec);
int avcodec_close_thread_safe(AVCodecContext *avctx);

BOOL CMJPEGEncoder::Open(LPBITMAPINFO pSrcBMI)
{
	// Already open?
	if (m_pCodecCtx)
		return TRUE;

	// Allocate Context
	m_pCodecCtx = avcodec_alloc_context();
	if (!m_pCodecCtx)
		goto error;

	// Set Format
	m_pCodecCtx->width = pSrcBMI->bmiHeader.biWidth;
	m_pCodecCtx->height = pSrcBMI->bmiHeader.biHeight;
	m_pCodecCtx->codec_id = CODEC_ID_MJPEG;
	m_pCodecCtx->pix_fmt = CAVIPlay::CAVIVideoStream::AVCodecBMIToPixFormat(pSrcBMI);
	m_pCodecCtx->codec_type = CODEC_TYPE_VIDEO;
	m_pCodecCtx->strict_std_compliance = FF_COMPLIANCE_INOFFICIAL; // to allow the PIX_FMT_YUV420P and the PIX_FMT_YUV422P formats
	m_pCodecCtx->flags |= CODEC_FLAG_QSCALE;
	m_pCodecCtx->bit_rate = 0;
	m_pCodecCtx->time_base.den = 1;
	m_pCodecCtx->time_base.num = 1;

	// Open codec
	m_pCodec = avcodec_find_encoder(m_pCodecCtx->codec_id);
	if (!m_pCodec)
		goto error;
	if (avcodec_open_thread_safe(m_pCodecCtx, m_pCodec) < 0)
	{
		m_pCodec = NULL;
		goto error;
	}

    // Allocate video frame
    m_pFrame = avcodec_alloc_frame();
	if (!m_pFrame)
        goto error;

	// Allocate Outbuf
	m_nOutbufSize = 4 * m_pCodecCtx->width * m_pCodecCtx->height;
	if (m_nOutbufSize < FF_MIN_BUFFER_SIZE)
		m_nOutbufSize = FF_MIN_BUFFER_SIZE;
	m_pOutbuf = new uint8_t[m_nOutbufSize + FF_INPUT_BUFFER_PADDING_SIZE];
	if (!m_pOutbuf)
	{
		m_nOutbufSize = 0;
		return FALSE;
	}

	return TRUE;

error:
	Close();
	return FALSE;
}

void CMJPEGEncoder::Close()
{
	if (m_pCodecCtx)
	{
		if (m_pCodec) // only close if avcodec_open_thread_safe has been successfully called!
			avcodec_close_thread_safe(m_pCodecCtx);
		av_freep(&m_pCodecCtx);
	}
	m_pCodec = NULL;

	if (m_pFrame)
		av_freep(&m_pFrame);

	if (m_pOutbuf)
	{
		delete [] m_pOutbuf;
		m_pOutbuf = NULL;
	}
	m_nOutbufSize = 0;

	memset(&m_SrcBMI, 0, sizeof(BITMAPINFOFULL));
}

DWORD CMJPEGEncoder::Encode(int qscale, LPBITMAPINFO pSrcBMI, LPBYTE pSrcBits)
{
	// Check
	if (!pSrcBMI || !pSrcBits)
		return 0U;

	// Clip: 31 comes from m_pCodecCtx->qmax, and 2 from m_pCodecCtx->qmin
	if (qscale > 31)
		qscale = 31;
	else if (qscale < 2)
		qscale = 2;

	// Re-open?
	if (!CDib::IsSameBMI(pSrcBMI, (LPBITMAPINFO)(&m_SrcBMI)))
		Close();

	// Open?
	if (!m_pCodecCtx)
	{
		if (!Open(pSrcBMI))
			return 0U;
	}

	// Encode
	avpicture_fill(	(AVPicture*)m_pFrame,
					(uint8_t*)pSrcBits,
					CAVIPlay::CAVIVideoStream::AVCodecBMIToPixFormat(pSrcBMI),
					pSrcBMI->bmiHeader.biWidth,
					pSrcBMI->bmiHeader.biHeight);
	m_pFrame->quality = FF_QP2LAMBDA * qscale;
	int len = avcodec_encode_video(	m_pCodecCtx,
									m_pOutbuf,
									m_nOutbufSize,
									m_pFrame);
	if (len > 0)
	{
		// Store current format
		memcpy(&m_SrcBMI, pSrcBMI, MIN(CDib::GetBMISize(pSrcBMI), sizeof(BITMAPINFOFULL)));
		return (DWORD)len;
	}
	else
		return 0U;
}

#endif