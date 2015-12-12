#include "stdafx.h"
#include "uImager.h"
#include "twaincpp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*
Note:

With the microsoft virtual TWAIN driver for WIA
the debugger asserts in cmdtarg.cpp with m_dwRef == 2 or higher
when closing the application. I do not know how to release
the WIA com object to decrement its count...should be done
by the virtual driver!

CCmdTarget::~CCmdTarget()
{
#ifndef _AFX_NO_OLE_SUPPORT
	if (m_xDispatch.m_vtbl != 0)
		((COleDispatchImpl*)&m_xDispatch)->Disconnect();
	ASSERT(m_dwRef <= 1);
#endif
	m_pModuleState = NULL;
}
*/


/*
Constructor:
	Parameters : HWND 
				Window to subclass
				 
*/
CTwain::CTwain(HWND hWnd)
{
	m_TwainReturnCode = 0;
	m_hTwainMessageWnd = NULL;
	m_hTwainDLL = NULL;
	m_pTwainDSMProc = NULL;
	m_bTwainSourceSelected = FALSE;
	m_bTwainDSOpen = m_bTwainDSMOpen = FALSE;
	m_bTwainSourceEnabled = FALSE;
	m_bTwainModalUI = TRUE;
	m_nTwainImageCount = TWCPP_ANYCOUNT;
	memset(&m_TwainStatus, 0, sizeof(m_TwainStatus));
	memset(&m_TwainAppId, 0, sizeof(m_TwainAppId));
	memset(&m_TwainSource, 0, sizeof(m_TwainSource));
	if (hWnd)
		InitTwain(hWnd);
}

CTwain::~CTwain()
{
	ReleaseTwain();
}

/*
Initializes TWAIN interface . Is already called from the constructor. 
It should be called again if ReleaseTwain is called.

  hWnd is the window which has to subclassed in order to recieve
  Twain messaged. Normally - this would be your main application window.

*/
BOOL CTwain::InitTwain(HWND hWnd)
{
	if (TwainIsValidDriver()) 
		return TRUE;
	memset(&m_TwainAppId, 0, sizeof(m_TwainAppId));
	if (!IsWindow(hWnd))
		return FALSE;
	m_hTwainMessageWnd = hWnd;
	
	m_hTwainDLL = ::LoadLibrary(_T("TWAIN_32.DLL"));
	if (m_hTwainDLL)
	{
		if (!(m_pTwainDSMProc = (DSMENTRYPROC)::GetProcAddress(m_hTwainDLL, (LPCSTR)MAKEINTRESOURCE(1))))
		{
			::FreeLibrary(m_hTwainDLL);
			m_hTwainDLL = NULL;
		}
	}
	else
		m_pTwainDSMProc = NULL;

	if (TwainIsValidDriver())
	{
		TwainGetIdentity();
		m_bTwainDSMOpen = CallTwainProc(&m_TwainAppId, NULL, DG_CONTROL, DAT_PARENT, MSG_OPENDSM, (TW_MEMREF)&m_hTwainMessageWnd);
		return TRUE;
	}
	else
		return FALSE;
}


/*
Releases the twain interface . Need not be called unless you
want to specifically shut it down.
*/
void CTwain::ReleaseTwain()
{
	if (TwainIsValidDriver())
	{
		TwainCloseDSM();
		if (m_hTwainDLL)
		{
			::FreeLibrary(m_hTwainDLL);
			m_hTwainDLL = NULL;
		}
		m_pTwainDSMProc = NULL;
	}
}

/*
This function should ideally be overridden in the derived class . If only a 
few fields need to be updated , call CTwain::GetIdentity first in your
derived class
*/
void CTwain::TwainGetIdentity()
{
	memset(&m_TwainAppId, 0, sizeof(m_TwainAppId));

	// Expects all the fields in m_TwainAppId to be set except for the id field.
	m_TwainAppId.Id = 0; // Initialize to 0 (Source Manager will assign real value)
	m_TwainAppId.Version.MajorNum = 1;	// Your app's major version number
	m_TwainAppId.Version.MinorNum = 9;	// Your app's minor version number
	m_TwainAppId.Version.Language = TWLG_USA;
	m_TwainAppId.Version.Country = TWCY_USA;
	strcpy (m_TwainAppId.Version.Info, "3.5");
	m_TwainAppId.ProtocolMajor = TWON_PROTOCOLMAJOR;
	m_TwainAppId.ProtocolMinor = TWON_PROTOCOLMINOR;
	m_TwainAppId.SupportedGroups = DG_IMAGE | DG_CONTROL;
	strcpy(m_TwainAppId.Manufacturer, "MICSS");
	strcpy(m_TwainAppId.ProductFamily, "Generic");
	strcpy(m_TwainAppId.ProductName, APPNAME_NOEXT_ASCII);
}


/*
Called to display a dialog box to select the Twain source to use.
This can be overridden if a list of all sources is available
to the application. These sources can be enumerated by Twain.
it is not yet supportted by CTwain.
*/
BOOL CTwain::TwainSelectSource()
{
	memset(&m_TwainSource, 0, sizeof(m_TwainSource));
	if (!TwainIsSourceSelected())
		TwainSelectDefaultSource();
	if (CallTwainProc(&m_TwainAppId,NULL,DG_CONTROL,DAT_IDENTITY,MSG_USERSELECT,&m_TwainSource))
		m_bTwainSourceSelected = TRUE;
	return m_bTwainSourceSelected;
}

/*
Called to select the default source
*/
BOOL CTwain::TwainSelectDefaultSource()
{
	memset(&m_TwainSource, 0, sizeof(m_TwainSource));
	m_bTwainSourceSelected = CallTwainProc(&m_TwainAppId,NULL,DG_CONTROL,DAT_IDENTITY,MSG_GETDEFAULT,&m_TwainSource);
	return m_bTwainSourceSelected;
}

/*
Closes the Data Source
*/
void CTwain::TwainCloseDS()
{
	if (TwainIsDSOpen())
	{
		TwainDisableSource();
		CallTwainProc(&m_TwainAppId,NULL,DG_CONTROL,DAT_IDENTITY,MSG_CLOSEDS,(TW_MEMREF)&m_TwainSource);
		m_bTwainDSOpen = FALSE;
	}
}

/*
Closes the Data Source Manager
*/
void CTwain::TwainCloseDSM()
{
	if (TwainIsDSMOpen())
	{
		TwainCloseDS();
		CallTwainProc(&m_TwainAppId,NULL,DG_CONTROL,DAT_PARENT,MSG_CLOSEDSM,(TW_MEMREF)&m_hTwainMessageWnd);
		m_bTwainDSMOpen = FALSE;
	}
}

/*
Opens a Data Source supplied as the input parameter
*/
BOOL CTwain::TwainOpenSource(TW_IDENTITY *pSource)
{
	if (pSource) 
	{
		m_TwainSource = *pSource;
	}
	if (TwainIsDSMOpen())
	{
		if(!TwainIsSourceSelected())
		{
			TwainSelectDefaultSource();
		}
		m_bTwainDSOpen = CallTwainProc(&m_TwainAppId,NULL,DG_CONTROL,DAT_IDENTITY,MSG_OPENDS,(TW_MEMREF)&m_TwainSource);
	}
	return TwainIsDSOpen();
}

/*
Entry point into Twain. For a complete description of this
routine  please refer to the Twain specification 1.8
*/
BOOL CTwain::CallTwainProc(pTW_IDENTITY pOrigin,pTW_IDENTITY pDest,
						   TW_UINT32 DG,TW_UINT16 DAT,TW_UINT16 MSG,
						   TW_MEMREF pData)
{
	if (TwainIsValidDriver())
	{
		USHORT ret_val = (*m_pTwainDSMProc)(pOrigin,pDest,DG,DAT,MSG,pData);
		m_TwainReturnCode = ret_val;
		if (ret_val != TWRC_SUCCESS)
			(*m_pTwainDSMProc)(pOrigin,pDest,DG_CONTROL,DAT_STATUS,MSG_GET,&m_TwainStatus);
		return (ret_val == TWRC_SUCCESS);
	}
	else
	{
		m_TwainReturnCode = TWRC_FAILURE;
		return FALSE;
	}
}

TW_UINT16 CTwain::TwainCallDSMEntry(pTW_IDENTITY pApp, pTW_IDENTITY pSrc,
									TW_UINT32 DG, TW_UINT16 DAT, TW_UINT16 MSG,
									TW_MEMREF pData)
{
	TW_UINT16 twRC = (*m_pTwainDSMProc)(pApp, pSrc, DG, DAT, MSG, pData);
	if ((twRC != TWRC_SUCCESS) && (DAT != DAT_EVENT))
		(*m_pTwainDSMProc)(pApp,pSrc,DG_CONTROL,DAT_STATUS,MSG_GET,&m_TwainStatus);
	return twRC;
}

/*
Called by ProcessMessage to Translate a TWAIN message
*/
void CTwain::TwainTranslateMessage(TW_EVENT* ptwEvent)
{
	switch (ptwEvent->TWMessage)
	{
		case MSG_XFERREADY :
			TwainTransferImage();
			break;

		case MSG_CLOSEDSREQ :
			TwainCloseDS();
			if (::IsWindow(m_hTwainMessageWnd))
				::PostMessage(m_hTwainMessageWnd, WM_TWAIN_CLOSED, 0, 0);
			break;

		// No message from the Source to the App
		// (possible new message)
		case MSG_NULL :
		default :
			break;
	}
}

/*
Queries the capability of the Twain Data Source
*/
BOOL CTwain::TwainGetCapability(TW_CAPABILITY& twCap,TW_UINT16 cap,TW_UINT16 conType)
{
	if (TwainIsDSOpen())
	{
		twCap.Cap = cap;
		twCap.ConType = conType;
		twCap.hContainer = NULL;

		if (CallTwainProc(&m_TwainAppId,&m_TwainSource,DG_CONTROL,DAT_CAPABILITY,MSG_GET,(TW_MEMREF)&twCap))
		{
			return TRUE;
		}
	}
	return FALSE;
}

static double Fix32ToDouble(TW_FIX32 fix)
{
	TW_INT32 val = ((TW_INT32)fix.Whole << 16) | ((TW_UINT32)fix.Frac & 0xffff);
	return val / 65536.0;
}

static void DoubleToFix32(double r, TW_FIX32* pfix)
{
	// Note 1: This round-away-from-0 is new in TWAIN 1.7
	// Note 2: ANSI C converts float to int by truncating toward 0.
	TW_INT32 val = (TW_INT32)(r * 65536.0 + (r < 0 ? -0.5 : +0.5));
	pfix->Whole = (TW_INT16)(val >> 16);		// most significant 16 bits
	pfix->Frac = (TW_UINT16)(val & 0xffff);		// least
}

static TW_INT32 TWAIN_ToFix32(double r)
{
	TW_FIX32 fix;
	ASSERT(sizeof(TW_FIX32) == sizeof(TW_INT32));
	DoubleToFix32(r, &fix);
	return *(TW_INT32*)&fix;
}

/*
Queries the current X resolution in dots per current resolution
*/
double CTwain::TwainGetXRes()
{
	TW_CAPABILITY twCap;
	if (TwainGetCapability(twCap, ICAP_XRESOLUTION, TWTY_FIX32))
	{
		pTW_ONEVALUE pVal;
		pVal = (pTW_ONEVALUE)::GlobalLock(twCap.hContainer);
		if (pVal)
		{
			TW_FIX32 res;
			memcpy(&res, &pVal->Item, sizeof(TW_UINT32));
			GlobalUnlock(pVal);
			GlobalFree(twCap.hContainer);
			return Fix32ToDouble(res);
		}
	}
	return 0.0;
}

/*
Queries the current Y resolution in dots per current resolution
*/
double CTwain::TwainGetYRes()
{
	TW_CAPABILITY twCap;
	if (TwainGetCapability(twCap, ICAP_YRESOLUTION, TWTY_FIX32))
	{
		pTW_ONEVALUE pVal;
		pVal = (pTW_ONEVALUE)::GlobalLock(twCap.hContainer);
		if (pVal)
		{
			TW_FIX32 res;
			memcpy(&res, &pVal->Item, sizeof(TW_UINT32));
			GlobalUnlock(pVal);
			GlobalFree(twCap.hContainer);
			return Fix32ToDouble(res);
		}
	}
	return 0.0;
}

/*
Queries the capability of the Twain Data Source
cap: ICAP_UNITS     -> TWUN_INCHES, TWUN_CENTIMETERS, TWUN_PIXELS
	 ICAP_BITDEPTH  -> it is per channel (24 bpp returns 8)
	 ICAP_PIXELTYPE -> TWPT_BW, TWPT_GRAY, TWPT_RGB, TWPT_PALETTE
*/
BOOL CTwain::TwainGetCapability(TW_UINT16 cap,TW_UINT32& value)
{
	TW_CAPABILITY twCap;
	if (TwainGetCapability(twCap,cap))
	{
		pTW_ONEVALUE pVal;
		pVal = (pTW_ONEVALUE)::GlobalLock(twCap.hContainer);
		if (pVal)
		{
			value = pVal->Item;
			GlobalUnlock(pVal);
			GlobalFree(twCap.hContainer);
			return TRUE;
		}
	}
	return FALSE;
}


/*
Sets the capability of the Twain Data Source
*/
BOOL CTwain::TwainSetCapability(TW_UINT16 cap,TW_UINT16 value,BOOL sign)
{
	if (TwainIsDSOpen())
	{
		TW_CAPABILITY twCap;
		pTW_ONEVALUE pVal;
		BOOL ret_value = FALSE;

		twCap.Cap = cap;
		twCap.ConType = TWON_ONEVALUE;
		
		twCap.hContainer = ::GlobalAlloc(GHND,sizeof(TW_ONEVALUE));
		if (twCap.hContainer)
		{
			pVal = (pTW_ONEVALUE)::GlobalLock(twCap.hContainer);
			if (pVal)
			{
				pVal->ItemType = sign ? TWTY_INT16 : TWTY_UINT16;
				pVal->Item = (TW_UINT32)value;
			}
			::GlobalUnlock(twCap.hContainer);
			ret_value = TwainSetCapability(twCap);
			::GlobalFree(twCap.hContainer);
		}
		return ret_value;
	}
	return FALSE;
}

/*
Sets the capability of the Twain Data Source
*/
BOOL CTwain::TwainSetCapability(TW_CAPABILITY& cap)
{
	if (TwainIsDSOpen())
	{
		return CallTwainProc(&m_TwainAppId,&m_TwainSource,DG_CONTROL,DAT_CAPABILITY,MSG_SET,(TW_MEMREF)&cap);
	}
	return FALSE;
}

/*
Sets the number of images which can be accepted by the application at one time
*/
BOOL CTwain::TwainSetImageCount(TW_INT16 nCount)
{
	if (TwainSetCapability(CAP_XFERCOUNT,(TW_UINT16)nCount,TRUE))
	{
		m_nTwainImageCount = nCount;
		return TRUE;
	}
	else
	{
		if (TwainGetRC() == TWRC_CHECKSTATUS)
		{
			TW_UINT32 count;
			if (TwainGetCapability(CAP_XFERCOUNT,count))
			{
				nCount = (TW_INT16)count;
				if (TwainSetCapability(CAP_XFERCOUNT,nCount))
				{
					m_nTwainImageCount = nCount;
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

/*
Called to enable the Twain Acquire Dialog. This too can be
overridden but is a helluva job. 
*/
BOOL CTwain::TwainEnableSource(BOOL showUI)
{
	if (TwainIsDSOpen())
	{
		TW_USERINTERFACE twUI;
		twUI.ShowUI = showUI;
		twUI.hParent = (TW_HANDLE)m_hTwainMessageWnd;
		if (CallTwainProc(&m_TwainAppId,&m_TwainSource,DG_CONTROL,DAT_USERINTERFACE,MSG_ENABLEDS,(TW_MEMREF)&twUI))
		{
			m_bTwainSourceEnabled = TRUE;
			m_bTwainModalUI = twUI.ModalUI;
		}
		else
		{
			m_bTwainSourceEnabled = FALSE;
			m_bTwainModalUI = TRUE;
		}
		return m_bTwainSourceEnabled;
	}
	return FALSE;
}

/*
 Called to disable the source.
*/
BOOL CTwain::TwainDisableSource()
{
	if (TwainIsSourceEnabled())
	{
		TW_USERINTERFACE twUI;
		if (CallTwainProc(&m_TwainAppId,&m_TwainSource,DG_CONTROL,DAT_USERINTERFACE,MSG_DISABLEDS,&twUI))
		{
			m_bTwainSourceEnabled = FALSE;
			return TRUE;
		}
	}
	return FALSE;
}

/*
Called to acquire images from the source. parameter numImages is the
number of images that you an handle concurrently.
*/
BOOL CTwain::TwainAcquire(int numImages)
{
	if (TwainIsDSOpen() || TwainOpenSource())
	{
		TwainDisableSource();
		TwainSetImageCount(numImages);
		if (TwainEnableSource())
			return TRUE;
	}
	return FALSE;
}

/*
Gets Imageinfo for an image which is about to be transferred.
*/
BOOL CTwain::TwainGetImageInfo(TW_IMAGEINFO& info)
{
	if (TwainIsSourceEnabled())
	{
		return CallTwainProc(&m_TwainAppId,&m_TwainSource,DG_IMAGE,DAT_IMAGEINFO,MSG_GET,(TW_MEMREF)&info);
	}
	return FALSE;
}

/*
Trasnfers the image or cancels the transfer depending on the state of the
TWAIN system.
*/
void CTwain::TwainTransferImage()
{
	TW_IMAGEINFO info;
	BOOL bContinue=TRUE;

	while (bContinue)
	{
		if (TwainGetImageInfo(info))
		{
			int permission;
			permission = TwainShouldTransfer(info);
			switch (permission)
			{
				case TWCPP_CANCELTHIS:
					bContinue=TwainEndTransfer();
					break;
				case TWCPP_CANCELALL:
					TwainCancelTransfer();
					bContinue=FALSE;
					break;
				case TWCPP_DOTRANSFER:
					bContinue=TwainGetImage(info);
					break;
			}
		}
	}
}

/*
Ends the current transfer.
Returns TRUE if the more images are pending
*/
BOOL CTwain::TwainEndTransfer()
{
	TW_PENDINGXFERS twPend;
	if (CallTwainProc(&m_TwainAppId,&m_TwainSource,DG_CONTROL,DAT_PENDINGXFERS,MSG_ENDXFER,(TW_MEMREF)&twPend))
	{
		return twPend.Count != 0;
	}
	return FALSE;
}

/*
Aborts all transfers
*/
void CTwain::TwainCancelTransfer()
{
	TW_PENDINGXFERS twPend;
	CallTwainProc(&m_TwainAppId,&m_TwainSource,DG_CONTROL,DAT_PENDINGXFERS,MSG_RESET,(TW_MEMREF)&twPend);
}

/*
Calls TWAIN to actually get the image
*/
BOOL CTwain::TwainGetImage(TW_IMAGEINFO& info)
{
	BOOL res = FALSE;
	HANDLE hBitmap = NULL;
	CallTwainProc(&m_TwainAppId,&m_TwainSource,DG_IMAGE,DAT_IMAGENATIVEXFER,MSG_GET,&hBitmap);
	switch (m_TwainReturnCode)
	{
		case TWRC_XFERDONE :
			TwainCopyImage(hBitmap,info);
			res = TwainEndTransfer();
			GlobalFree(hBitmap);
			return res;
		case TWRC_CANCEL :
			TwainEndTransfer();
			GlobalFree(hBitmap);
			return FALSE;
		case TWRC_FAILURE :
		default :
			TwainCancelTransfer();
			GlobalFree(hBitmap);
			return FALSE;
	}
}