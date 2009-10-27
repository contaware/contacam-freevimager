#ifndef _INC_AVDECODER
#define _INC_AVDECODER

#ifdef SUPPORT_LIBAVCODEC

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
		enum {SRC_BUFFER_SIZE = 2048000};
		CAVDecoder(){	m_dFrameRate = 25.0;
						m_nBytesRemaining = 0;
						m_pRawData = NULL;
						m_pSrcBuf = NULL;
						m_dwSrcBufSize = 0;
						m_pDstBuf = NULL;						
						m_dwDstBufSize = 0;
						m_dwDstFourCC = BI_RGB;
						m_pDeinterlaceBuf = NULL;
						m_dwDeinterlaceBufSize = 0;
						m_bOpen = FALSE;
						m_pFormatCtx = NULL;
						m_pCodec = NULL;
						m_pCodecCtx = NULL;
						m_Packet.data = NULL;
						m_Packet.size = 0;
						m_pFrame = NULL;
						m_pFrameDeinterlaced = NULL;
						m_pFrameUnc = NULL;
						m_pImgConvertCtx = NULL;
						}
		virtual ~CAVDecoder(){Close();};
		double GetFrameRate() const {return m_dFrameRate;};
		BOOL IsOpen() const {return m_bOpen;};
		BOOL Open(LPBITMAPINFOHEADER pBMIH);
		void Close(BOOL bNoClose = FALSE);
		BOOL Decode(LPBYTE pSrcBits,	// Compressed Mpeg2 Data
					DWORD dwSrcSize,	// Compressed Mpeg2 Data Size
					CDib* pDstDib,		// Destination Dib, already allocated!
					BOOL bDeinterlace = FALSE);
	protected:
		BOOL OpenFinish();
		BOOL GetFrame();
		BOOL InitImgConvert(CDib* pDstDib);

		double m_dFrameRate;
		int m_nBytesRemaining;
		uint8_t* m_pRawData;
		LPBYTE m_pSrcBuf;					
		DWORD m_dwSrcBufSize;
		LPBYTE m_pDstBuf;						
		DWORD m_dwDstBufSize;
		DWORD m_dwDstFourCC;
		LPBYTE m_pDeinterlaceBuf;
		DWORD m_dwDeinterlaceBufSize;
		BOOL m_bOpen;
		AVFormatContext* m_pFormatCtx;
		AVCodec* m_pCodec;
		AVCodecContext* m_pCodecCtx;
		AVPacket m_Packet;
		AVFrame* m_pFrame;
		AVFrame* m_pFrameDeinterlaced;
		AVFrame* m_pFrameUnc;
		SwsContext* m_pImgConvertCtx;
};

#endif
#endif //!_INC_AVDECODER