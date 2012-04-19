#ifndef __TWAINCPP_
#define __TWAINCPP_

#include "Twain.h"

#define TWCPP_ANYCOUNT		(-1)
#define TWCPP_CANCELTHIS	(1)
#define TWCPP_CANCELALL		(2)
#define TWCPP_DOTRANSFER	(0)
#define WM_TWAIN_CLOSED		WM_USER + 114

class CTwain
{
public:

	CTwain(HWND hWnd = NULL);
	virtual ~CTwain();
	BOOL InitTwain(HWND hWnd);
	void ReleaseTwain();


	/*  
	  This routine must be implemented by the dervied class 
	  After setting the required values in the m_AppId structure,
	  the derived class should call the parent class implementation
	  Refer Pg: 51 of the Twain Specification version 1.8
	*/
	virtual void TwainGetIdentity();
	virtual BOOL TwainSelectSource();
	virtual BOOL TwainOpenSource(TW_IDENTITY *pSource = NULL);
	virtual int  TwainShouldTransfer(TW_IMAGEINFO& info) { return TWCPP_DOTRANSFER;};

	BOOL TwainSelectDefaultSource();

	__forceinline BOOL TwainIsValidDriver() const {return (m_hTwainDLL && m_pTwainDSMProc);};
	__forceinline BOOL TwainIsSourceSelected() const {return m_bTwainSourceSelected;};
	__forceinline BOOL TwainIsDSMOpen() const {return TwainIsValidDriver() && m_bTwainDSMOpen;};
	__forceinline BOOL TwainIsDSOpen() const {return TwainIsValidDriver() && TwainIsDSMOpen() && m_bTwainDSOpen;};
	__forceinline BOOL TwainIsSourceEnabled() const {return m_bTwainSourceEnabled;};
	__forceinline BOOL TwainIsModalUI() const {return m_bTwainModalUI;};

	__forceinline TW_INT16 TwainGetRC() const {return m_TwainReturnCode;};
	__forceinline TW_STATUS TwainGetStatus() const {return m_TwainStatus;};

	BOOL TwainSetImageCount(TW_INT16 nCount = 1);
	BOOL TwainAcquire(int numImages = 1);

protected:
	BOOL CallTwainProc(pTW_IDENTITY pOrigin, pTW_IDENTITY pDest,
					   TW_UINT32 DG, TW_UINT16 DAT, TW_UINT16 MSG,
					   TW_MEMREF pData);
	TW_UINT16 TwainCallDSMEntry(pTW_IDENTITY pApp, pTW_IDENTITY pSrc,
					   TW_UINT32 DG, TW_UINT16 DAT, TW_UINT16 MSG,
					   TW_MEMREF pData);

	void TwainCloseDSM();
	void TwainCloseDS();

	BOOL TwainGetCapability(TW_CAPABILITY& twCap, TW_UINT16 cap, TW_UINT16 conType = TWON_DONTCARE16);
	BOOL TwainGetCapability(TW_UINT16 cap, TW_UINT32& value);
	BOOL TwainSetCapability(TW_UINT16 cap, TW_UINT16 value, BOOL sign = FALSE);
	BOOL TwainSetCapability(TW_CAPABILITY& twCap);
	double TwainGetXRes();
	double TwainGetYRes();
	BOOL TwainEnableSource(BOOL showUI = TRUE);

	BOOL TwainGetImageInfo(TW_IMAGEINFO& info);

	virtual BOOL TwainDisableSource();

	void TwainTranslateMessage(TW_EVENT* ptwEvent);
	void TwainTransferImage();
	BOOL TwainEndTransfer();
	void TwainCancelTransfer();
	BOOL TwainShouldContinue();
	BOOL TwainGetImage(TW_IMAGEINFO& info);

	virtual void TwainCopyImage(HANDLE hBitmap, TW_IMAGEINFO& info) = 0;

	/*
	Should be called from the main message loop of the application. Can always be called,
	it will not process the message unless a scan is in progress.
	*/
	__forceinline BOOL TwainProcessMessage(MSG* pMsg)
	{
		if (TwainIsSourceEnabled())
		{
			// Make a Copy For Safety
			MSG Msg;
			memcpy(&Msg, pMsg, sizeof(MSG));
			TW_UINT16 twRC = TWRC_NOTDSEVENT;
			TW_EVENT twEvent;
			memset(&twEvent, 0, sizeof(TW_EVENT));
			twEvent.pEvent = (TW_MEMREF)&Msg;
		
			twRC = TwainCallDSMEntry(&m_TwainAppId,
									&m_TwainSource,
									DG_CONTROL,
									DAT_EVENT,
									MSG_PROCESSEVENT,
									(TW_MEMREF)&twEvent);

			TwainTranslateMessage(&twEvent);

			return (twRC == TWRC_DSEVENT);
		}
		else
			return FALSE;
	}

protected:
	HINSTANCE m_hTwainDLL;
	DSMENTRYPROC m_pTwainDSMProc;

	TW_IDENTITY m_TwainAppId;
	TW_IDENTITY m_TwainSource;
	TW_STATUS m_TwainStatus;
	TW_INT16  m_TwainReturnCode;
	HWND m_hTwainMessageWnd;

	BOOL m_bTwainSourceSelected;
	BOOL m_bTwainDSMOpen;
	BOOL m_bTwainDSOpen;
	BOOL m_bTwainSourceEnabled;
	BOOL m_bTwainModalUI;

	int m_nTwainImageCount;
};


#endif