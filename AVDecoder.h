#ifndef _INC_AVDECODER
#define _INC_AVDECODER

#ifdef VIDEODEVICEDOC

#include "Dib.h"

extern "C"
{
#include "ffmpeg\\libavcodec\\avcodec.h"
#include "ffmpeg\\libavformat\\avformat.h"
#include "ffmpeg\\libswscale\\swscale.h"
}

// AV Decoder
class CAVDecoder 
{
	public:
		CAVDecoder(){	memset(&m_SrcBMI, 0, sizeof(BITMAPINFOFULL));
						memset(&m_DstBMI, 0, sizeof(BITMAPINFOFULL));
						m_pCodec = NULL;
						m_pCodecCtx = NULL;
						m_pFrame = NULL;
						m_pFrameDst = NULL;
						m_pImgConvertCtx = NULL;}
		virtual ~CAVDecoder(){Close();}
		BOOL Decode(LPBITMAPINFO pSrcBMI,
					LPBYTE pSrcBits,
					DWORD dwSrcSize,
					CDib* pDstDib);
		__forceinline AVCodecID GetCodecId() {return (m_pCodecCtx ? m_pCodecCtx->codec_id : AV_CODEC_ID_NONE);};

	protected:
		BOOL Open(LPBITMAPINFO pSrcBMI);
		void Close();
		BITMAPINFOFULL m_SrcBMI;
		BITMAPINFOFULL m_DstBMI;
		AVCodec* m_pCodec;
		AVCodecContext* m_pCodecCtx;
		AVFrame* m_pFrame;
		AVFrame* m_pFrameDst;
		SwsContext* m_pImgConvertCtx;
};

#endif
#endif //!_INC_AVDECODER