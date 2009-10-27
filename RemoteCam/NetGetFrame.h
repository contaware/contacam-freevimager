#if !defined(AFX_NETGETFRAME_H__14956A13_6E82_4959_931F_F512BE9B17A0__INCLUDED_)
#define AFX_NETGETFRAME_H__14956A13_6E82_4959_931F_F512BE9B17A0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NetCom.h"
#include "NetFrameHdr.h"
extern "C"
{
#include "ffmpeg\\libavcodec\\avcodec.h"
#include "ffmpeg\\libswscale\\swscale.h"
}

class CRemoteCamCtrl;

class CGetFrameGenerator : public CNetCom::CIdleGenerator
{
	public:
		CGetFrameGenerator() {;};
		BOOL Generate(CNetCom* pNetCom);
};

// The Networking Get Frame Parser & Processor Class
class CGetFrameParseProcess : public CNetCom::CParseProcess
{
	public:
		CGetFrameParseProcess() {	for (DWORD dwFrame = 0 ; dwFrame < NETFRAME_MAX_FRAMES ; dwFrame++)
									{
										m_nTotalFragments[dwFrame] = 0;
										m_dwUpTime[dwFrame] = 0U;
										m_dwFrameSize[dwFrame] = 0U;
										m_wFrameSeq[dwFrame] = 0U;
										m_bKeyFrame[dwFrame] = FALSE;
									}
									memset(&m_ReSendCount, 0, NETFRAME_RESEND_ARRAY_SIZE);
									memset(&m_ReSendCountDown, 0, NETFRAME_RESEND_ARRAY_SIZE);
									memset(m_Fragment, 0, sizeof(m_Fragment));
									m_dwPingRT = 0U;
									m_dwPrevDibWidth = 0U;
									m_dwPrevDibHeight = 0U;
									m_dwLastReSendUpTime = 0U;
									m_dwLastFrameUpTime = 0U;
									m_dwAvgFrameTime = NETFRAME_MIN_FRAME_TIME;
									m_dwLastPresentationUpTime = 0U;
									m_wPrevSeq = 0U;
									m_bInitialized = FALSE;
									m_bFirstFrame = TRUE;
									m_bSeekToKeyFrame = FALSE;
									m_pCodec = NULL;
									m_pCodecCtx = NULL;
									m_CodecId = CODEC_ID_NONE;
									m_pFrame = NULL;
									m_pFrameRGB = NULL;
									m_pImgConvertCtx = NULL;
									m_pRGBBuf = NULL;
									m_dwRGBBufSize = 0;
									m_dwRGBImageSize = 0;
									m_pOutbuf = NULL;
									m_nOutbufSize = 0;
									m_pExtradata = NULL;
									m_nExtradataSize = 0;
									m_nMaxExtradata = 0;
									m_dwEncryptionType = 0U;};
		virtual ~CGetFrameParseProcess() {	for (DWORD dwFrame = 0U ; dwFrame < NETFRAME_MAX_FRAMES ; dwFrame++)
												FreeFrameFragments(dwFrame);
											if (m_pExtradata)
												av_freep(&m_pExtradata);
											m_nExtradataSize = 0;
											m_nMaxExtradata = 0;
											FreeAVCodec();};
		void SetCtrl(CRemoteCamCtrl* pCtrl) {m_pCtrl = pCtrl;};
		virtual BOOL Parse(CNetCom* pNetCom);
		
	protected:
		BOOL SendConfirmation(	CNetCom* pNetCom,
								DWORD dwUpTime,
								DWORD dwFrameSize,
								WORD wSeq,
								BOOL bKeyFrame);
		BOOL SendLostCount(	CNetCom* pNetCom,
							WORD wFirstLostSeq,
							WORD wLastLostSeq,
							WORD wLostCount);
		__forceinline BOOL ReSendFrame(CNetCom* pNetCom, WORD wSeq);
		__forceinline BYTE ReSendCountDown(	int nReSendCount,
											int nCount,
											int nCountOffset);
		__forceinline DWORD CalcFrameSize(DWORD dwFrame);
		__forceinline BOOL IsFrameReady(WORD wSeq);
		__forceinline void FreeFrameFragments(DWORD dwFrame)
		{
			for (int i = 0 ; i < NETFRAME_MAX_FRAGMENTS ; i++)
			{
				if (m_Fragment[dwFrame][i])
				{
					delete m_Fragment[dwFrame][i];
					m_Fragment[dwFrame][i] = NULL;
				}
			}
		}
		// Use the following count function and not a variable
		// because of possible duplicated fragments!
		__forceinline int GetReceivedFragmentsCount(DWORD dwFrame);
		BOOL OpenAVCodec(enum CodecID CodecId, int width, int height);
		void FreeAVCodec(BOOL bNoClose = FALSE);
		BOOL Decode(LPBYTE pFrame, DWORD dwFrameSize);
		BOOL InitImgConvert();
#ifdef _DEBUG
		void TraceIncompleteFrame(DWORD dwFrame);
#endif

		DWORD m_dwPrevDibWidth;
		DWORD m_dwPrevDibHeight;
		CRemoteCamCtrl* m_pCtrl;
		AVCodec* m_pCodec;
		AVCodecContext* m_pCodecCtx;
		AVFrame* m_pFrame;
		AVFrame* m_pFrameRGB;
		SwsContext* m_pImgConvertCtx;
		LPBYTE m_pRGBBuf;						
		DWORD m_dwRGBBufSize;
		DWORD m_dwRGBImageSize;
		uint8_t* m_pOutbuf;
		int m_nOutbufSize;		// In Bytes
		uint8_t* m_pExtradata;
		int m_nMaxExtradata;
		int m_nExtradataSize;
		CNetCom::CBuf* m_Fragment[NETFRAME_MAX_FRAMES][NETFRAME_MAX_FRAGMENTS];
		int m_nTotalFragments[NETFRAME_MAX_FRAMES];
		DWORD m_dwFrameSize[NETFRAME_MAX_FRAMES];
		DWORD m_dwUpTime[NETFRAME_MAX_FRAMES];
		DWORD m_dwPingRT;
		DWORD m_dwLastReSendUpTime;
		DWORD m_dwLastFrameUpTime;
		DWORD m_dwLastPresentationUpTime;
		DWORD m_dwAvgFrameTime;
		WORD m_wFrameSeq[NETFRAME_MAX_FRAMES];
		WORD m_wPrevSeq;
		BOOL m_bKeyFrame[NETFRAME_MAX_FRAMES];
		BYTE m_ReSendCount[NETFRAME_RESEND_ARRAY_SIZE];
		BYTE m_ReSendCountDown[NETFRAME_RESEND_ARRAY_SIZE];
		BOOL m_bInitialized;
		BOOL m_bFirstFrame;
		BOOL m_bSeekToKeyFrame;
		enum CodecID m_CodecId;
		DWORD m_dwEncryptionType;
};


#endif // !defined(AFX_NETGETFRAME_H__14956A13_6E82_4959_931F_F512BE9B17A0__INCLUDED_)