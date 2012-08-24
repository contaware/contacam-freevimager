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

// AV Encoder
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
		DWORD Encode(	int qscale,		// 2: best, 31: worst
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