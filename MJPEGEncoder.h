#ifndef _INC_MJPEGENCODER
#define _INC_MJPEGENCODER

#ifdef VIDEODEVICEDOC

#include "Dib.h"

extern "C"
{
#include "ffmpeg\\libavcodec\\avcodec.h"
#include "ffmpeg\\libavformat\\avformat.h"
#include "ffmpeg\\libswscale\\swscale.h"
}

// MJPEG Encoder
//
// Source pixel format must be
//
// PIX_FMT_YUVJ420P
// PIX_FMT_YUVJ422P
// PIX_FMT_YUV420P (FF_COMPLIANCE_INOFFICIAL)
// PIX_FMT_YUV422P (FF_COMPLIANCE_INOFFICIAL)
//
// Note: if using PIX_FMT_YUV420P or PIX_FMT_YUV422P ffmpeg adds a
// "CS=ITU601" string to the Jpeg COM marker emphasizing that a
// non-JPEG compliant colorspace is used. The ffmpeg decoder recognizes
// that, thus only use ITU601 colorspace if encoding AND decoding with ffmpeg!
class CMJPEGEncoder 
{
	public:
		CMJPEGEncoder(){memset(&m_SrcBMI, 0, sizeof(BITMAPINFOFULL));
						m_pCodec = NULL;
						m_pCodecCtx = NULL;
						m_pFrame = NULL;
						m_pOutbuf = NULL;
						m_nOutbufSize = 0;}
		virtual ~CMJPEGEncoder(){Close();}
		DWORD Encode(	int qscale,		// 2: best quality, 31: worst quality
						LPBITMAPINFO pSrcBMI,	
						LPBYTE pSrcBits);
		uint8_t* GetEncodedBuf() {return m_pOutbuf;};
	protected:
		BOOL Open(LPBITMAPINFO pSrcBMI);
		void Close();
		BITMAPINFOFULL m_SrcBMI;
		AVCodec* m_pCodec;
		AVCodecContext* m_pCodecCtx;
		AVFrame* m_pFrame;
		uint8_t* m_pOutbuf;
		int m_nOutbufSize;
};

#endif
#endif //!_INC_MJPEGENCODER