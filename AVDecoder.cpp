#include "stdafx.h"
#include "AviPlay.h"
#include "AVDecoder.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef VIDEODEVICEDOC

// Defined in uImager.cpp
int avcodec_open_thread_safe(AVCodecContext *avctx, AVCodec *codec);
int avcodec_close_thread_safe(AVCodecContext *avctx);

// Performances
// ------------
//
// CAVDecoder::Decode() is faster for:
// RGB32         -> I420
// YUY2         <-> I420
//
// CDib::Decompress() is faster for:
// I420 and YUY2 -> RGB32
// RGB32         -> YUY2

BOOL CAVDecoder::Open(LPBITMAPINFO pSrcBMI)
{
	// Already open?
	if (m_pCodecCtx)
		return TRUE;

	// Find the codec id for the video stream
	CodecID id = CAVIPlay::CAVIVideoStream::AVCodecFourCCToCodecID(pSrcBMI->bmiHeader.biCompression);

	// Allocate Context
	m_pCodecCtx = avcodec_alloc_context();
	if (!m_pCodecCtx)
		goto error;

	// Set Width & Height
	m_pCodecCtx->coded_width = pSrcBMI->bmiHeader.biWidth;
	m_pCodecCtx->coded_height = pSrcBMI->bmiHeader.biHeight;

	// Set Codec Id
	m_pCodecCtx->codec_id = id;

	// Set FourCC
	m_pCodecCtx->codec_tag = pSrcBMI->bmiHeader.biCompression;

	// Set some other values
	m_pCodecCtx->error_concealment = 3;
	m_pCodecCtx->error_recognition = 1;

	// Open codec
	if (id != CODEC_ID_NONE)
	{
		m_pCodec = avcodec_find_decoder(id);
		if (!m_pCodec)
			goto error;
		if (avcodec_open_thread_safe(m_pCodecCtx, m_pCodec) < 0)
		{
			m_pCodec = NULL;
			goto error;
		}
	}
	// Codec not necessary but use the codec context to store
	// width, height, pix_fmt and codec_tag
	else
	{
		m_pCodecCtx->pix_fmt = CAVIPlay::CAVIVideoStream::AVCodecBMIToPixFormat(pSrcBMI);
		if (m_pCodecCtx->pix_fmt == PIX_FMT_NONE)
			goto error;
		m_pCodecCtx->width = m_pCodecCtx->coded_width;
		m_pCodecCtx->height = m_pCodecCtx->coded_height;
	}

    // Allocate video frames
    m_pFrame = avcodec_alloc_frame();
	if (!m_pFrame)
        goto error;
    m_pFrameDst = avcodec_alloc_frame();
    if (!m_pFrameDst)
        goto error;

	return TRUE;

error:
	Close();
	return FALSE;
}

void CAVDecoder::Close()
{
	if (m_pCodecCtx)
	{
		if (m_pCodec) // only close if avcodec_open_thread_safe has been successfully called!
			avcodec_close_thread_safe(m_pCodecCtx);
		av_freep(&m_pCodecCtx);
	}
	m_pCodec = NULL;

	if (m_pFrameDst)
		av_freep(&m_pFrameDst);
	if (m_pFrame)
		av_freep(&m_pFrame);

	if (m_pImgConvertCtx)
	{
		sws_freeContext(m_pImgConvertCtx);
		m_pImgConvertCtx = NULL;
	}

	memset(&m_SrcBMI, 0, sizeof(BITMAPINFOFULL));
	memset(&m_DstBMI, 0, sizeof(BITMAPINFOFULL));
}

BOOL CAVDecoder::Decode(LPBITMAPINFO pSrcBMI,
						LPBYTE pSrcBits,
						DWORD dwSrcSize,
						CDib* pDstDib)
{
	// Check
	if (!pSrcBMI || !pSrcBits || dwSrcSize == 0 || !pDstDib)
		return FALSE;

	// Re-open?
	if (!CDib::IsSameBMI(pSrcBMI, (LPBITMAPINFO)(&m_SrcBMI)) ||
		!pDstDib->IsSameBMI((LPBITMAPINFO)(&m_DstBMI)))
		Close();

	// Open?
	if (!m_pCodecCtx)
	{
		if (!Open(pSrcBMI))
			return FALSE;
	}

	// Decode?
	if (m_pCodec)
	{
		int got_picture = 0;
		int len = avcodec_decode_video(	m_pCodecCtx,
										m_pFrame,
										&got_picture,
										(unsigned __int8 *)pSrcBits,
										dwSrcSize);
		if (len <= 0 || got_picture == 0)
			return FALSE;
	}
	else
	{
		// Assign appropriate parts of source buffer to image planes
		avpicture_fill(	(AVPicture*)m_pFrame,
						(unsigned __int8 *)pSrcBits,
						m_pCodecCtx->pix_fmt,
						m_pCodecCtx->width,
						m_pCodecCtx->height);
		// Source swap U <-> V?
		if (m_pCodecCtx->codec_tag == FCC('YV12')	||
			m_pCodecCtx->codec_tag == FCC('YV16')	||
			m_pCodecCtx->codec_tag == FCC('YVU9'))
		{
			uint8_t* pTemp = m_pFrame->data[1];
			m_pFrame->data[1] = m_pFrame->data[2];
			m_pFrame->data[2] = pTemp;
			// Line Sizes for U and V are the same no need to swap
		}
		// Source RGB flip vertically?
		else if (	m_pCodecCtx->codec_tag == BI_RGB	||
					m_pCodecCtx->codec_tag == BI_BITFIELDS)
		{
			m_pFrame->data[0] += m_pFrame->linesize[0] * (m_pCodecCtx->height - 1);
			m_pFrame->linesize[0] *= -1;
		}
	}

	// Allocate destination bits?
	if (!pDstDib->GetBits())
	{
		// If nothing specified default to RGB32 with source width and height
		if (!pDstDib->AllocateBitsFast(	pDstDib->GetBitCount() <= 0 ? 32 : pDstDib->GetBitCount(),
										pDstDib->GetCompression(),
										pDstDib->GetWidth() == 0 ? m_pCodecCtx->width : pDstDib->GetWidth(),
										pDstDib->GetHeight() == 0 ? m_pCodecCtx->height : pDstDib->GetHeight()))
			return FALSE;
	}

	// Get destination pixel format
	enum PixelFormat dst_pix_fmt = CAVIPlay::CAVIVideoStream::AVCodecBMIToPixFormat(pDstDib->GetBMI());
	if (dst_pix_fmt == PIX_FMT_NONE)
		return FALSE;

	// Now that we know the destination format store current formats
	memcpy(&m_SrcBMI, pSrcBMI, MIN(CDib::GetBMISize(pSrcBMI), sizeof(BITMAPINFOFULL)));
	memcpy(&m_DstBMI, pDstDib->GetBMI(), MIN(pDstDib->GetBMISize(), sizeof(BITMAPINFOFULL)));

	// Prepare Image Conversion Context
	m_pImgConvertCtx = sws_getCachedContext(m_pImgConvertCtx,				// re-use if already allocated
											m_pCodecCtx->width,				// Source Width
											m_pCodecCtx->height,			// Source Height
											m_pCodecCtx->pix_fmt,			// Source Format
											pDstDib->GetWidth(),			// Destination Width
											pDstDib->GetHeight(),			// Destination Height
											dst_pix_fmt,					// Destination Format
											SWS_BICUBIC,					// SWS_CPU_CAPS_MMX2, SWS_CPU_CAPS_MMX, SWS_CPU_CAPS_3DNOW
											NULL,							// No Src Filter
											NULL,							// No Dst Filter
											NULL);							// Param
	if (!m_pImgConvertCtx)
		return FALSE;

	// Assign appropriate parts of destination buffer to image planes
	avpicture_fill((AVPicture*)m_pFrameDst,
					(unsigned __int8 *)pDstDib->GetBits(),
					dst_pix_fmt,
					pDstDib->GetWidth(),
					pDstDib->GetHeight());
	// Destination swap U <-> V?
	if (pDstDib->GetCompression() == FCC('YV12')	||
		pDstDib->GetCompression() == FCC('YV16')	||
		pDstDib->GetCompression() == FCC('YVU9'))
	{
		uint8_t* pTemp = m_pFrameDst->data[1];
		m_pFrameDst->data[1] = m_pFrameDst->data[2];
		m_pFrameDst->data[2] = pTemp;
		// Line Sizes for U and V are the same no need to swap
	}
	// Destination RGB flip vertically?
	else if (	pDstDib->GetCompression() == BI_RGB	||
				pDstDib->GetCompression() == BI_BITFIELDS)
	{
		m_pFrameDst->data[0] += m_pFrameDst->linesize[0] * (pDstDib->GetHeight() - 1);
		m_pFrameDst->linesize[0] *= -1;
	}

	// Post Process
	if (m_pFrame->data[0])
	{
		int sws_scale_res = sws_scale(	m_pImgConvertCtx,		// Image Convert Context
										m_pFrame->data,			// Source Data
										m_pFrame->linesize,		// Source Stride
										0,						// Source Slice Y
										m_pCodecCtx->height,	// Source Height
										m_pFrameDst->data,		// Destination Data
										m_pFrameDst->linesize);	// Destination Stride
#ifdef SUPPORT_LIBSWSCALE
		return (sws_scale_res > 0 ? TRUE : FALSE);
#else
		return (sws_scale_res >= 0 ? TRUE : FALSE);
#endif
	}
	else
		return FALSE;
}

#endif