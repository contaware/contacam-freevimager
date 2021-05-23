#include "stdafx.h"
#include "YuvToYuv.h"
#include "AVDecoder.h"
#include "AVRec.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef VIDEODEVICEDOC

// TODO: remove the warnings suppression and use the new ffmpeg API interface
#pragma warning(push)
#pragma warning(disable : 4996)

// Defined in uImager.cpp
SwsContext *sws_getCachedContextHelper(	struct SwsContext *context,
										int srcW, int srcH, enum AVPixelFormat srcFormat,
                                        int dstW, int dstH, enum AVPixelFormat dstFormat,
										int flags);

BOOL CAVDecoder::Open(LPBITMAPINFO pSrcBMI)
{
	// Already open?
	if (m_pCodecCtx)
		return TRUE;

	// Find the codec id for the video stream
	AVCodecID id = CAVRec::AVCodecFourCCToCodecID(pSrcBMI->bmiHeader.biCompression);

	// Get codec
	if (id != AV_CODEC_ID_NONE)
	{
		m_pCodec = avcodec_find_decoder(id);
		if (!m_pCodec)
			goto error;
	}
	else
		m_pCodec = NULL;

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

	// Set Width & Height
	m_pCodecCtx->coded_width = pSrcBMI->bmiHeader.biWidth;
	m_pCodecCtx->coded_height = pSrcBMI->bmiHeader.biHeight;

	// Set Codec Id
	m_pCodecCtx->codec_id = id;

	// Set FourCC
	m_pCodecCtx->codec_tag = pSrcBMI->bmiHeader.biCompression;

	// Open codec
	if (id != AV_CODEC_ID_NONE)
	{
		if (avcodec_open2(m_pCodecCtx, m_pCodec, 0) < 0)
			goto error;
	}
	// Codec not necessary but use the codec context to store
	// width, height, pix_fmt and codec_tag
	else
	{
		m_pCodecCtx->pix_fmt = CAVRec::AVCodecBMIToPixFormat(pSrcBMI);
		if (m_pCodecCtx->pix_fmt == AV_PIX_FMT_NONE)
			goto error;
		m_pCodecCtx->width = m_pCodecCtx->coded_width;
		m_pCodecCtx->height = m_pCodecCtx->coded_height;
	}

    // Allocate video frames
    m_pFrame = av_frame_alloc();
	if (!m_pFrame)
        goto error;
    m_pFrameDst = av_frame_alloc();
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
		/*
		Close a given AVCodecContext and free all the data associated with it
		(but not the AVCodecContext itself).
		Calling this function on an AVCodecContext that hasn't been opened will free
		the codec-specific data allocated in avcodec_alloc_context3() /
		avcodec_get_context_defaults3() with a non-NULL codec. Subsequent calls will
		do nothing.
		*/
		avcodec_close(m_pCodecCtx);
		av_freep(&m_pCodecCtx);
	}
	m_pCodec = NULL;

	if (m_pFrameDst)
		av_frame_free(&m_pFrameDst);
	if (m_pFrame)
		av_frame_free(&m_pFrame);

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
	BOOL bSrcSame;
	if (CDib::GetBMISize(pSrcBMI) == sizeof(BITMAPINFOHEADER) &&
		CDib::GetBMISize((LPBITMAPINFO)(&m_SrcBMI)) == sizeof(BITMAPINFOHEADER))
	{
		// Size changes with each frame for compressed formats like mjpeg -> do not compare size!
		bSrcSame =	pSrcBMI->bmiHeader.biWidth			==	m_SrcBMI.bmiHeader.biWidth			&&
					pSrcBMI->bmiHeader.biHeight			==	m_SrcBMI.bmiHeader.biHeight			&&
					pSrcBMI->bmiHeader.biCompression	==	m_SrcBMI.bmiHeader.biCompression	&&
					pSrcBMI->bmiHeader.biBitCount		==	m_SrcBMI.bmiHeader.biBitCount;
	}
	else
		bSrcSame = CDib::IsSameBMI(pSrcBMI, (LPBITMAPINFO)(&m_SrcBMI));
	BOOL bDstSame = pDstDib->IsSameBMI((LPBITMAPINFO)(&m_DstBMI));
	if (!bSrcSame || !bDstSame)
		Close();

	// Open?
	if (!m_pCodecCtx)
	{
		if (!Open(pSrcBMI))
			return FALSE;
	}

	// Copy source data to the buffer created by av_new_packet
	// which is correctly aligned and ends with
	// FF_INPUT_BUFFER_PADDING_SIZE extra zero-bytes
	AVPacket avpkt;
    if (av_new_packet(&avpkt, dwSrcSize) < 0)
        return FALSE;
	memcpy(avpkt.data, pSrcBits, dwSrcSize);

	// Decode?
	if (m_pCodec)
	{
		av_frame_unref(m_pFrame);
		int got_picture = 0;
		int len = avcodec_decode_video2(m_pCodecCtx,
										m_pFrame,
										&got_picture,
										&avpkt);
		if (len <= 0 || got_picture == 0)
		{
			av_packet_unref(&avpkt);
			return FALSE;
		}
	}
	else
	{
		// Assign appropriate parts of source buffer to image planes
		avpicture_fill(	(AVPicture*)m_pFrame,
						(unsigned __int8 *)avpkt.data,
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

	// Check
	if (m_pCodecCtx->width <= 0 || m_pCodecCtx->height <= 0	||
		m_pCodecCtx->pix_fmt < 0 || m_pCodecCtx->pix_fmt >= AV_PIX_FMT_NB)
	{
		av_packet_unref(&avpkt);
		return FALSE;
	}

	// Allocate destination bits?
	if (!pDstDib->GetBits())
	{
		// If nothing specified default to RGB32 with source width and height
		if (!pDstDib->AllocateBitsFast(	pDstDib->GetBitCount() <= 0 ? 32 : pDstDib->GetBitCount(),
										pDstDib->GetCompression(),
										pDstDib->GetWidth() == 0 ? m_pCodecCtx->width : pDstDib->GetWidth(),
										pDstDib->GetHeight() == 0 ? m_pCodecCtx->height : pDstDib->GetHeight()))
		{	
			av_packet_unref(&avpkt);
			return FALSE;
		}
	}

	// Get destination pixel format
	enum AVPixelFormat dst_pix_fmt = CAVRec::AVCodecBMIToPixFormat(pDstDib->GetBMI());
	if (dst_pix_fmt == AV_PIX_FMT_NONE)
	{
		av_packet_unref(&avpkt);
		return FALSE;
	}

	// Now that we know the destination format store current formats
	memcpy(&m_SrcBMI, pSrcBMI, MIN(CDib::GetBMISize(pSrcBMI), sizeof(BITMAPINFOFULL)));
	memcpy(&m_DstBMI, pDstDib->GetBMI(), MIN(pDstDib->GetBMISize(), sizeof(BITMAPINFOFULL)));

	// Prepare Image Conversion Context
	m_pImgConvertCtx = sws_getCachedContextHelper(	m_pImgConvertCtx,		// Re-use if already allocated
													m_pCodecCtx->width,		// Source Width
													m_pCodecCtx->height,	// Source Height
													m_pCodecCtx->pix_fmt,	// Source Format
													pDstDib->GetWidth(),	// Destination Width
													pDstDib->GetHeight(),	// Destination Height
													dst_pix_fmt,			// Destination Format
													SWS_BICUBIC);			// Interpolation (add SWS_PRINT_INFO to debug)
	if (!m_pImgConvertCtx)
	{
		av_packet_unref(&avpkt);
		return FALSE;
	}

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

	// Post Process (first try fast conversion, if not supported fall back to sws_scale)
	if (m_pFrame->data[0])
	{
		BOOL bOk = FALSE;
		if (m_pCodecCtx->width == pDstDib->GetWidth() &&
			m_pCodecCtx->height == pDstDib->GetHeight())
		{
			bOk = ITU601JPEGConvert(	m_pCodecCtx->pix_fmt,	// Source Format
										dst_pix_fmt,			// Destination Format
										m_pFrame->data,			// Source Data
										m_pFrame->linesize,		// Source Stride
										m_pFrameDst->data,		// Destination Data
										m_pFrameDst->linesize,	// Destination Stride
										m_pCodecCtx->width,		// Width
										m_pCodecCtx->height);	// Height
		}
		if (!bOk)
		{
			bOk = sws_scale(m_pImgConvertCtx,					// Image Convert Context
							m_pFrame->data,						// Source Data
							m_pFrame->linesize,					// Source Stride
							0,									// Source Slice Y
							m_pCodecCtx->height,				// Source Height
							m_pFrameDst->data,					// Destination Data
							m_pFrameDst->linesize) > 0;			// Destination Stride
		}
		av_packet_unref(&avpkt);
		return bOk;
	}
	else
	{
		av_packet_unref(&avpkt);
		return FALSE;
	}
}

#pragma warning(pop)

#endif