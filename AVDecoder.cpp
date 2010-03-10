#include "stdafx.h"
#include "AviPlay.h"
#include "AVDecoder.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef SUPPORT_LIBAVCODEC

// Defined in uImager.cpp
int avcodec_open_thread_safe(AVCodecContext *avctx, AVCodec *codec);
int avcodec_close_thread_safe(AVCodecContext *avctx);

BOOL CAVDecoder::Open(LPBITMAPINFOHEADER pBMIH)
{
	// Close
	Close();

	// Find the codec id for the video stream
	CodecID id = CAVIPlay::CAVIVideoStream::AVCodecFourCCToCodecID(pBMIH->biCompression);

	// mpeg 1,2
	if (id == CODEC_ID_MPEG2VIDEO || id == CODEC_ID_MPEG1VIDEO)
	{
		// Allocate Fomat Context
		m_pFormatCtx = av_alloc_format_context();
		if (!m_pFormatCtx)
			goto error_noclose;
		memset(m_pFormatCtx, 0, sizeof(AVFormatContext));
	}
	// mjpeg
	else if (id == CODEC_ID_MJPEG || id == CODEC_ID_MJPEGB || id == CODEC_ID_SP5X)
	{
		// Allocate Context
		m_pCodecCtx = avcodec_alloc_context();
		if (!m_pCodecCtx)
			goto error_noclose;

		// Set Width & Height
		m_pCodecCtx->coded_width = pBMIH->biWidth;
		m_pCodecCtx->coded_height = pBMIH->biHeight;

		// Set Codec Id
		m_pCodecCtx->codec_id = id;

		// Set FourCC
		m_pCodecCtx->codec_tag = pBMIH->biCompression;

		// Open Finish
		if (!OpenFinish())
			goto error_noclose;
	}
	else
		goto error_noclose;

    // Allocate video frames
    m_pFrame = avcodec_alloc_frame();
	if (!m_pFrame)
        goto error;
	m_pFrameDeinterlaced = avcodec_alloc_frame();
	if (!m_pFrameDeinterlaced)
        goto error;
    m_pFrameUnc = avcodec_alloc_frame();
    if (!m_pFrameUnc)
        goto error;

	return TRUE;

error:
	Close();
	return FALSE;
error_noclose:
	Close(TRUE);
	return FALSE;
}

BOOL CAVDecoder::OpenFinish()
{
	// Check
	if (!m_pCodecCtx)
		return FALSE;

	// Some Settings
	m_pCodecCtx->error_concealment = 3;
	m_pCodecCtx->flags |= CODEC_FLAG_TRUNCATED;
	m_pCodecCtx->error_recognition = 1;

	// Allocate Src Buffer, be safe!
	m_dwSrcBufSize =	4 * m_pCodecCtx->width * m_pCodecCtx->height;
	if (m_pSrcBuf)
		delete [] m_pSrcBuf;
	m_pSrcBuf = new BYTE[m_dwSrcBufSize + FF_INPUT_BUFFER_PADDING_SIZE];

	// Find the decoder for the video stream
	m_pCodec = avcodec_find_decoder(m_pCodecCtx->codec_id);
	if (!m_pCodec)
		return FALSE;

    // Open codec
    if (avcodec_open_thread_safe(m_pCodecCtx, m_pCodec) < 0)
        return FALSE;
	else
	{
		m_bOpen = TRUE;
		return TRUE;
	}
}

void CAVDecoder::Close(BOOL bNoClose/*=FALSE*/)
{
	// Mpeg needs a format context
	if (m_pFormatCtx)
	{
		// Taken from av_close_input_file():

		// Free Current Packet
		if (m_pFormatCtx->cur_st && m_pFormatCtx->cur_st->parser)
			av_free_packet(&m_pFormatCtx->cur_pkt);

		// Read Close
		if (m_pFormatCtx->iformat && m_pFormatCtx->iformat->read_close)
			m_pFormatCtx->iformat->read_close(m_pFormatCtx);

		// Close & Free the Codecs of all the Streams
		for (int i = 0 ; i < (int)m_pFormatCtx->nb_streams ; i++)
		{
			if (m_pFormatCtx->streams[i])
			{
				// Close
				if (!bNoClose)
				{
					if (m_pFormatCtx->streams[i]->codec)
						avcodec_close_thread_safe(m_pFormatCtx->streams[i]->codec);
					if (m_pFormatCtx->streams[i]->parser)
						av_parser_close(m_pFormatCtx->streams[i]->parser);
				}
				
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
		m_pCodecCtx = NULL;
		
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
			av_free(m_pFormatCtx->pb);
			m_pFormatCtx->pb = NULL;
		}

		// Free Format Ctx
		av_freep(&m_pFormatCtx->priv_data);
		av_freep(&m_pFormatCtx);
	}
	// mjpeg has only a codec context
	else if (m_pCodecCtx)
	{
		if (!bNoClose)
			avcodec_close_thread_safe(m_pCodecCtx);
		av_free(m_pCodecCtx);
		m_pCodecCtx = NULL;
		m_pCodec = NULL;
	}

	if (m_Packet.data != NULL)
        av_free_packet(&m_Packet); // Size & data are reset by this function
	if (m_pFrameUnc)
		av_freep(&m_pFrameUnc);
	if (m_pFrameDeinterlaced)
		av_freep(&m_pFrameDeinterlaced);
	if (m_pFrame)
		av_freep(&m_pFrame);
	
	m_nBytesRemaining = 0;
	m_pRawData = NULL;
	if (m_pSrcBuf)
	{
		delete [] m_pSrcBuf;
		m_pSrcBuf = NULL;
	}
	m_dwSrcBufSize = 0;
	if (m_pDstBuf)
	{
		delete [] m_pDstBuf;
		m_pDstBuf = NULL;
	}
	m_dwDstBufSize = 0;
	if (m_pDeinterlaceBuf)
	{
		delete [] m_pDeinterlaceBuf;
		m_pDeinterlaceBuf = NULL;
	}
	m_dwDeinterlaceBufSize = 0;
	m_bOpen = FALSE;
	m_dwDstFourCC = BI_RGB;

	if (m_pImgConvertCtx)
	{
		sws_freeContext(m_pImgConvertCtx);
		m_pImgConvertCtx = NULL;
	}
}

int my_read_packet(void *opaque, uint8_t *buf, int buf_size)
{
	TRACE(	_T("\nShould never call this, if it's the case increase the\n")
			_T("buffer size in the call:\n")
			_T("url_setbufsize(&m_pFormatCtx->pb, BiggerBufferSize)\n"));
	return 0; 
}

// -1: error
// 0:  more data needed to decode the frame
// 1:  got frame!
int CAVDecoder::Decode(	LPBYTE pSrcBits,	// Compressed Mpeg2 Data
						DWORD dwSrcSize,	// Compressed Mpeg2 Data Size
						CDib* pDstDib,		// Destination Dib, already allocated!
						BOOL bDeinterlace/*=FALSE*/)		
{
	if (m_pFormatCtx)
	{
		// Open Second Part
		if (!m_bOpen)
		{
			// Init & Allocate buffer
			if (m_pFormatCtx->pb == NULL)
			{
				m_pFormatCtx->pb = (ByteIOContext*)av_mallocz(sizeof(ByteIOContext));
				if (!m_pFormatCtx->pb)
					return -1;
				m_pFormatCtx->pb->read_packet = my_read_packet;
				m_pFormatCtx->pb->is_streamed = 1;
				m_pFormatCtx->pb->opaque = this;
				url_setbufsize(m_pFormatCtx->pb, SRC_BUFFER_SIZE);
			}

			// Fill Buffer
			memcpy(m_pFormatCtx->pb->buf_end, pSrcBits, dwSrcSize);
			m_pFormatCtx->pb->buf_end += dwSrcSize;

			// First Fill the Buffer a Third
			if (m_pFormatCtx->pb->buf_end - m_pFormatCtx->pb->buffer < (SRC_BUFFER_SIZE / 3))
				return 0;

			// Open Input Stream
			AVFormatParameters ap;
			memset(&ap, 0, sizeof(ap));
			ap.prealloced_context = 1;
			AVInputFormat* fmt = av_find_input_format("mpegvideo");
			if (av_open_input_stream(	&m_pFormatCtx,
										m_pFormatCtx->pb,	// If NULL av_open_input_stream sets
															// m_pFormatCtx->pb to NULL, this way
															// it sets it to itself
										"",
										fmt,
										&ap) != 0)
				return -1;

			// Retrieve stream information
			if (av_find_stream_info(m_pFormatCtx) < 0)
				return -1;

			// Set Format Context Pointer
			m_pCodecCtx = m_pFormatCtx->streams[0]->codec;

			// Set Frame Rate
			m_dFrameRate = av_q2d(m_pFormatCtx->streams[0]->r_frame_rate);

			// Finish Opening: Allocate Src Buffer & Open Codec 
			if (!OpenFinish())
				return -1;
			else
				return 0;
		}
		else
		{
			// (Move) & Fill Buffer
			unsigned char* buffer_end = m_pFormatCtx->pb->buffer + m_pFormatCtx->pb->buffer_size;
			if (m_pFormatCtx->pb->buf_end + dwSrcSize > buffer_end)
			{
				int nMoveWidth = m_pFormatCtx->pb->buf_ptr - m_pFormatCtx->pb->buffer;
				int nMoveSize = m_pFormatCtx->pb->buf_end - m_pFormatCtx->pb->buf_ptr;
				memmove(m_pFormatCtx->pb->buffer,
						m_pFormatCtx->pb->buf_ptr,
						nMoveSize);
				m_pFormatCtx->pb->buf_ptr -= nMoveWidth;
				m_pFormatCtx->pb->buf_end -= nMoveWidth;
			}
			if (m_pFormatCtx->pb->buf_end + dwSrcSize > buffer_end)
				dwSrcSize = buffer_end - m_pFormatCtx->pb->buf_end; // Should increase Buffer Size...
			memcpy(m_pFormatCtx->pb->buf_end, pSrcBits, dwSrcSize);
			m_pFormatCtx->pb->buf_end += dwSrcSize;
		}

		// Only Decode if a Third Full, not really a good presentation time...
		if (m_pFormatCtx->pb->buf_end - m_pFormatCtx->pb->buf_ptr < (SRC_BUFFER_SIZE / 3))
			return 0;
	}

	// Check
	if (!m_pCodecCtx)
		return -1;

	// Decode
	if (!GetFrame())
		return 0;

	// Init Color Space Convert Context
	if (m_dwDstFourCC != pDstDib->GetCompression())
	{
		if (m_pImgConvertCtx)
		{
			sws_freeContext(m_pImgConvertCtx);
			m_pImgConvertCtx = NULL;
		}
	}
	if (!m_pImgConvertCtx)
	{
		if (!InitImgConvert(pDstDib))
			return -1;
	}

	// Post Process
	if (m_pFrame->data[0])
	{
		int sws_scale_res;

		// De-Interlace?
		if (bDeinterlace)
		{
			// Determine required buffer size and allocate buffer if necessary
			int nBufSize = avpicture_get_size(	m_pCodecCtx->pix_fmt,
												m_pCodecCtx->width,
												m_pCodecCtx->height);
			if ((int)(m_dwDeinterlaceBufSize) < nBufSize || m_pDeinterlaceBuf == NULL)
			{
				// Allocate Buffer
				if (m_pDeinterlaceBuf)
					delete [] m_pDeinterlaceBuf;
				m_pDeinterlaceBuf = new BYTE[nBufSize + FF_INPUT_BUFFER_PADDING_SIZE];
				if (!m_pDeinterlaceBuf)
					return -1;
				m_dwDeinterlaceBufSize = nBufSize;
			}

			// Assign appropriate parts of buffer to image planes
			avpicture_fill((AVPicture*)m_pFrameDeinterlaced,
							(unsigned __int8 *)m_pDeinterlaceBuf,
							m_pCodecCtx->pix_fmt,
							m_pCodecCtx->width,
							m_pCodecCtx->height);

			// De-Interlace
			if (avpicture_deinterlace(	(AVPicture*)m_pFrameDeinterlaced,	// Dst
										(AVPicture*)m_pFrame,				// Src
										m_pCodecCtx->pix_fmt,
										m_pCodecCtx->width,
										m_pCodecCtx->height) < 0)
				return -1; // Error

			// Color Space Conversion 
			sws_scale_res = sws_scale(	m_pImgConvertCtx,				// Image Convert Context
										m_pFrameDeinterlaced->data,		// Source Data
										m_pFrameDeinterlaced->linesize,	// Source Stride
										0,								// Source Slice Y
										m_pCodecCtx->height,			// Source Height
										m_pFrameUnc->data,				// Destination Data
										m_pFrameUnc->linesize);			// Destination Stride
		}
		else
		{
			// Color Space Conversion 
			sws_scale_res = sws_scale(	m_pImgConvertCtx,		// Image Convert Context
										m_pFrame->data,			// Source Data
										m_pFrame->linesize,		// Source Stride
										0,						// Source Slice Y
										m_pCodecCtx->height,	// Source Height
										m_pFrameUnc->data,		// Destination Data
										m_pFrameUnc->linesize);	// Destination Stride
		}
#ifdef SUPPORT_LIBSWSCALE
		int res = sws_scale_res > 0 ? 1 : -1;
#else
		int res = sws_scale_res >= 0 ? 1 : -1;
#endif			

		if (res == 1)
		{
			// Allocate Bits
			if (pDstDib->AllocateBits(	pDstDib->GetBitCount(),
										pDstDib->GetCompression(),
										m_pCodecCtx->width,
										m_pCodecCtx->height))
			{
				// Flip Vertically
				if (pDstDib->GetCompression() == BI_RGB ||
					pDstDib->GetCompression() == BI_BITFIELDS)
				{
					int nDWAlignedLineSize = DWALIGNEDWIDTHBYTES(	pDstDib->GetBitCount() *
																	pDstDib->GetWidth());
					LPBYTE lpBits = pDstDib->GetBits() + ((int)pDstDib->GetHeight() - 1) * nDWAlignedLineSize;
					for (int nCurLine = 0 ; nCurLine < (int)pDstDib->GetHeight() ; nCurLine++)
					{
						memcpy((void*)lpBits, (void*)(m_pDstBuf + nCurLine*nDWAlignedLineSize), nDWAlignedLineSize); 
						lpBits -= nDWAlignedLineSize;
					}
				}
				else
					memcpy(pDstDib->GetBits(), m_pDstBuf, pDstDib->GetImageSize());
			}
			else
				res = -1; // error
		}

		return res;
	}
	else
		return 0;
}

BOOL CAVDecoder::InitImgConvert(CDib* pDstDib)
{
	// Get Pix Format
	enum PixelFormat pix_fmt = CAVIPlay::CAVIVideoStream::AVCodecBMIToPixFormat(pDstDib->GetBMI());
	
	// Determine required buffer size and allocate buffer if necessary
	int nBufSize = avpicture_get_size(	pix_fmt,
										m_pCodecCtx->width,
										m_pCodecCtx->height);
	if ((int)(m_dwDstBufSize) < nBufSize || m_pDstBuf == NULL)
	{
		if (m_pDstBuf)
			delete [] m_pDstBuf;
		m_pDstBuf = new BYTE[nBufSize + FF_INPUT_BUFFER_PADDING_SIZE];
		if (!m_pDstBuf)
			return FALSE;
		m_dwDstBufSize = nBufSize;
	}

	// Assign appropriate parts of buffer to image planes
	avpicture_fill((AVPicture*)m_pFrameUnc,
					(unsigned __int8 *)m_pDstBuf,
					pix_fmt,
					m_pCodecCtx->width,
					m_pCodecCtx->height);

	// Prepare Image Conversion Context
	m_pImgConvertCtx = sws_getContext(	m_pCodecCtx->width,		// Source Width
										m_pCodecCtx->height,	// Source Height
										m_pCodecCtx->pix_fmt,	// Source Format
										m_pCodecCtx->width,		// Destination Width
										m_pCodecCtx->height,	// Destination Height
										pix_fmt,				// Destination Format
										SWS_BICUBIC,			// SWS_CPU_CAPS_MMX2, SWS_CPU_CAPS_MMX, SWS_CPU_CAPS_3DNOW
										NULL,					// No Src Filter
										NULL,					// No Dst Filter
										NULL);					// Param
	if (!m_pImgConvertCtx)
		return FALSE;

	// Flip U <-> V pointers
	if (pix_fmt == PIX_FMT_YUV420P && pDstDib->GetCompression() == FCC('YV12'))
	{
		uint8_t* pTemp = m_pFrameUnc->data[1];
		m_pFrameUnc->data[1] = m_pFrameUnc->data[2];
		m_pFrameUnc->data[2] = pTemp;
		// Line Sizes for U and V are the same no need to swap
	}

	// Set Dst FourCC
	m_dwDstFourCC = pDstDib->GetCompression();

	return TRUE;
}

BOOL CAVDecoder::GetFrame()
{
    int bytesDecoded = 0;
    int frameFinished = 0;

	// Reset Frame Structure
	avcodec_get_frame_defaults(m_pFrame);

    // Decode packets until we have decoded a complete frame
    while (true)
    {
        // Work on the current packet until we have decoded all of it
        while (m_nBytesRemaining > 0)
        {
            // Decode the next chunk of data
            bytesDecoded = avcodec_decode_video(	m_pCodecCtx,
													m_pFrame,
													&frameFinished,
													m_pRawData,
													m_nBytesRemaining);

            // Was there an error?
            if (bytesDecoded < 0)
                return FALSE;

			// Update Pos
            m_nBytesRemaining -= bytesDecoded;
            m_pRawData += bytesDecoded;

            // Did we finish the current frame? Then we can return
            if (frameFinished)
                return TRUE;
        }

        // Read the next packet,
		// skipping all packets that aren't for this stream
        do
        {
            // Free old packet
			if (m_Packet.data != NULL)
				av_free_packet(&m_Packet); // Size & data are reset by this function

            // Read new packet
            if (av_read_packet(m_pFormatCtx, &m_Packet) < 0)
                return FALSE;
			else
			{
				// Frame Delay = m_Packet.pts * av_q2d(video_st->time_base);
				// Extra Delay = m_pFrame->repeat_pict / (2*fps)
				// int64_t pts;
				if (m_Packet.pts != AV_NOPTS_VALUE	||
					m_Packet.dts != AV_NOPTS_VALUE	||
					m_pFrame->pts != AV_NOPTS_VALUE	||
					m_pFrame->repeat_pict > 0)
				{
					TRACE(_T("\nPacket: pts=%I64d , dts=%I64d  |  Frame: pts=%I64d , repeat_pict=%d\n"),
								m_Packet.pts,
								m_Packet.dts,
								m_pFrame->pts,
								m_pFrame->repeat_pict);
				}
			}
        }
		while (m_Packet.stream_index != 0); // First Stream

        m_nBytesRemaining = m_Packet.size;
        m_pRawData = m_Packet.data;
	}

    return frameFinished != 0;
}

#endif