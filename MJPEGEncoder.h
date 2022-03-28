#ifndef _INC_MJPEGENCODER
#define _INC_MJPEGENCODER

#ifdef VIDEODEVICEDOC

#include "Dib.h"

extern "C"
{
#include "libavutil/opt.h"
#include "libavutil/mathematics.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
}

// MJPEG Encoder
//
// Source pixel format must be
//
// AV_PIX_FMT_YUVJ420P
// AV_PIX_FMT_YUVJ422P
// AV_PIX_FMT_YUVJ444P
// AV_PIX_FMT_YUV420P (FF_COMPLIANCE_UNOFFICIAL)
// AV_PIX_FMT_YUV422P (FF_COMPLIANCE_UNOFFICIAL)
// AV_PIX_FMT_YUV444P (FF_COMPLIANCE_UNOFFICIAL)
//
// Note: if using AV_PIX_FMT_YUV420P, AV_PIX_FMT_YUV422P or AV_PIX_FMT_YUV444P
// ffmpeg adds a "CS=ITU601" string to the Jpeg COM marker emphasizing that a
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
		DWORD Encode(	int qscale,						// 2: best quality, 31: worst quality
						LPBITMAPINFO pSrcBMI,			// source format containing fourcc, width and height
						LPBYTE pSrcBits,				// pSrcBits must be correctly aligned and have AV_INPUT_BUFFER_PADDING_SIZE bytes padding 
						int nThreadCount);				// set the wanted thread count
		uint8_t* GetEncodedBuf() {return m_pOutbuf;};	// returns the encoded data buffer pointer valid till next Encode() call
	protected:
		BOOL Open(LPBITMAPINFO pSrcBMI, int nThreadCount);
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