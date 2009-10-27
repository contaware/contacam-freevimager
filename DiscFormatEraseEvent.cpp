///////////////////////////////////////////////////////////////////////
// DiscFormatEraseEvent.cpp
//
// Wrapper for DDiscFormat2EraseEvents Interface Events
//
// Written by Eric Haddan
//

#include "stdafx.h"
#include "DiscFormatEraseEvent.h"
#include "DiscFormatData.h"
#include "DiscFormatErase.h"

BOOL AFXAPI CDiscFormatEraseEvent::ConnectionAdvise(LPUNKNOWN pUnkSrc, REFIID iid,
	LPUNKNOWN pUnkSink, BOOL bRefCount, DWORD* pdwCookie)
{
	ASSERT_POINTER(pUnkSrc, IUnknown);
	ASSERT_POINTER(pUnkSink, IUnknown);
	ASSERT_POINTER(pdwCookie, DWORD);

	BOOL bSuccess = FALSE;

	LPCONNECTIONPOINTCONTAINER pCPC;

	if (SUCCEEDED(pUnkSrc->QueryInterface(
					IID_IConnectionPointContainer,
					(LPVOID*)&pCPC)))
	{
		ASSERT_POINTER(pCPC, IConnectionPointContainer);

		LPCONNECTIONPOINT pCP;

		if (SUCCEEDED(pCPC->FindConnectionPoint(iid, &pCP)))
		{
			ASSERT_POINTER(pCP, IConnectionPoint);

			if (SUCCEEDED(pCP->Advise(pUnkSink, pdwCookie)))
				bSuccess = TRUE;

			pCP->Release();

			// The connection point just AddRef'ed us.  If we don't want to
			// keep this reference count (because it would prevent us from
			// being deleted; our reference count wouldn't go to zero), then
			// we need to cancel the effects of the AddRef by calling
			// Release.

			if (bSuccess && !bRefCount)
				pUnkSink->Release();
		}

		pCPC->Release();
	}

	return bSuccess;
}

BOOL AFXAPI CDiscFormatEraseEvent::ConnectionUnadvise(LPUNKNOWN pUnkSrc, REFIID iid,
	LPUNKNOWN pUnkSink, BOOL bRefCount, DWORD dwCookie)
{
	ASSERT_POINTER(pUnkSrc, IUnknown);
	ASSERT_POINTER(pUnkSink, IUnknown);

	// When we call Unadvise, the connection point will Release us.  If we
	// didn't keep the reference count when we called Advise, we need to
	// AddRef now, to keep our reference count consistent.  Note that if
	// the Unadvise fails, then we need to undo this extra AddRef by
	// calling Release before we return.

	if (!bRefCount)
		pUnkSink->AddRef();

	BOOL bSuccess = FALSE;

	LPCONNECTIONPOINTCONTAINER pCPC;

	if (SUCCEEDED(pUnkSrc->QueryInterface(
					IID_IConnectionPointContainer,
					(LPVOID*)&pCPC)))
	{
		ASSERT_POINTER(pCPC, IConnectionPointContainer);

		LPCONNECTIONPOINT pCP;

		if (SUCCEEDED(pCPC->FindConnectionPoint(iid, &pCP)))
		{
			ASSERT_POINTER(pCP, IConnectionPoint);

			if (SUCCEEDED(pCP->Unadvise(dwCookie)))
				bSuccess = TRUE;

			pCP->Release();
		}

		pCPC->Release();
	}

	// If we failed, undo the earlier AddRef.

	if (!bRefCount && !bSuccess)
		pUnkSink->Release();

	return bSuccess;
}

// CDiscFormatEraseEvent

IMPLEMENT_DYNAMIC(CDiscFormatEraseEvent, CCmdTarget)

BEGIN_INTERFACE_MAP(CDiscFormatEraseEvent, CCmdTarget)
	INTERFACE_PART(CDiscFormatEraseEvent, IID_IDispatch, FormatEraseEvents)
	INTERFACE_PART(CDiscFormatEraseEvent, IID_DDiscFormat2EraseEvents, FormatEraseEvents)
END_INTERFACE_MAP()

CDiscFormatEraseEvent::CDiscFormatEraseEvent()
: m_hNotifyWnd(NULL)
, m_ptinfo(NULL)
, m_dwCookie(0)
, m_pUnkSink(0)
, m_pUnkSrc(0)
{
}

CDiscFormatEraseEvent::~CDiscFormatEraseEvent()
{
	if (m_dwCookie && (m_pUnkSrc != NULL) && (m_pUnkSink != NULL))
	{
		ConnectionUnadvise(m_pUnkSrc, IID_DDiscFormat2EraseEvents, m_pUnkSink, TRUE, m_dwCookie);
	}
}


BEGIN_MESSAGE_MAP(CDiscFormatEraseEvent, CCmdTarget)
END_MESSAGE_MAP()



// CDiscFormatEraseEvent message handlers

ULONG FAR EXPORT CDiscFormatEraseEvent::XFormatEraseEvents::AddRef()
{
	METHOD_PROLOGUE(CDiscFormatEraseEvent, FormatEraseEvents)
	return pThis->ExternalAddRef();
}
ULONG FAR EXPORT CDiscFormatEraseEvent::XFormatEraseEvents::Release()
{
	METHOD_PROLOGUE(CDiscFormatEraseEvent, FormatEraseEvents)
	return pThis->ExternalRelease();
}
STDMETHODIMP CDiscFormatEraseEvent::XFormatEraseEvents::QueryInterface(REFIID riid,
												   LPVOID FAR* ppvObj)
{
	METHOD_PROLOGUE(CDiscFormatEraseEvent, FormatEraseEvents)
	return (HRESULT)pThis->ExternalQueryInterface(&riid, ppvObj);
}
STDMETHODIMP
CDiscFormatEraseEvent::XFormatEraseEvents::GetTypeInfoCount(UINT FAR* pctinfo)
{
	METHOD_PROLOGUE(CDiscFormatEraseEvent, FormatEraseEvents)
	*pctinfo = 1;
	return NOERROR;
}
STDMETHODIMP CDiscFormatEraseEvent::XFormatEraseEvents::GetTypeInfo(
	UINT itinfo,
	LCID lcid,
	ITypeInfo FAR* FAR* pptinfo)
{
	METHOD_PROLOGUE(CDiscFormatEraseEvent, FormatEraseEvents)
	*pptinfo = NULL;

	if(itinfo != 0)
		return ResultFromScode(DISP_E_BADINDEX);
	pThis->m_ptinfo->AddRef();
	*pptinfo = pThis->m_ptinfo;
	return NOERROR;
}
STDMETHODIMP CDiscFormatEraseEvent::XFormatEraseEvents::GetIDsOfNames(
	REFIID riid,
	OLECHAR FAR* FAR* rgszNames,
	UINT cNames,
	LCID lcid,
	DISPID FAR* rgdispid)
{
	METHOD_PROLOGUE(CDiscFormatEraseEvent, FormatEraseEvents)
	return DispGetIDsOfNames(pThis->m_ptinfo, rgszNames, cNames, rgdispid);
}
STDMETHODIMP CDiscFormatEraseEvent::XFormatEraseEvents::Invoke(
	DISPID dispidMember,
	REFIID riid,
	LCID lcid,
	WORD wFlags,
	DISPPARAMS FAR* pdispparams,
	VARIANT FAR* pvarResult,
	EXCEPINFO FAR* pexcepinfo,
	UINT FAR* puArgErr)
{
	METHOD_PROLOGUE(CDiscFormatEraseEvent, FormatEraseEvents)

	return DispInvoke(&pThis->m_xFormatEraseEvents, pThis->m_ptinfo,
		dispidMember, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
}


///////////////////////////////////////////////////////////////////////
//
// CDiscFormatEraseEvent::CreateEventSink
//
// Description:
//			Establishes a link between the CDiscFormatErase(IDiscFormat2Erase)
//			and the CDiscFormatEraseEvent(DDiscFormat2EraseEvents) so 
//			CDiscFormatEraseEvent can receive Update messages
//
CDiscFormatEraseEvent* CDiscFormatEraseEvent::CreateEventSink()
{
	// Create the event sink
	CDiscFormatEraseEvent* pDiscFormatEraseEvent = new CDiscFormatEraseEvent();

	pDiscFormatEraseEvent->EnableAutomation();
	pDiscFormatEraseEvent->ExternalAddRef();

	return pDiscFormatEraseEvent;
}

///////////////////////////////////////////////////////////////////////
//
// CDiscFormatEraseEvent::ConnectDiscFormatErase
//
// Description:
//			Establishes a link between the CDiscFormatErase(IDiscFormat2Erase)
//			and the CDiscFormatEraseEvent(DDiscFormat2EraseEvents) so 
//			CDiscFormatEraseEvent can receive Update messages
//
bool CDiscFormatEraseEvent::ConnectDiscFormatErase(CDiscFormatErase* pDiscFormatErase)
{
	m_pUnkSink = GetIDispatch(FALSE);
	m_pUnkSrc = pDiscFormatErase->GetInterface();

	LPTYPELIB ptlib = NULL;
	HRESULT hr = LoadRegTypeLib(LIBID_IMAPILib2, 
		IMAPILib2_MajorVersion, IMAPILib2_MinorVersion, 
		LOCALE_SYSTEM_DEFAULT, &ptlib);
	if (FAILED(hr))
	{
		return false;
	}
	hr = ptlib->GetTypeInfoOfGuid(IID_DDiscFormat2EraseEvents, &m_ptinfo);
    ptlib->Release();
	if (FAILED(hr))
	{
	    return false;
	}

	BOOL bRet = ConnectionAdvise(m_pUnkSrc, IID_DDiscFormat2EraseEvents, m_pUnkSink,
		TRUE, &m_dwCookie);
	if (bRet)
	{
		return true;
	}

	return false;
}


///////////////////////////////////////////////////////////////////////
//
// CDiscFormatEraseEvent::Update
//
// Description:
//			Receives update notifications from IDiscFormat2Erase
//
STDMETHODIMP_(HRESULT) CDiscFormatEraseEvent::XFormatEraseEvents::Update(IDispatch* objectDispatch, LONG elapsedSeconds, LONG estimatedTotalSeconds)
{
	METHOD_PROLOGUE(CDiscFormatEraseEvent, FormatEraseEvents)

	IDiscFormat2Erase* discFormatErase = NULL;
    HRESULT hr = objectDispatch->QueryInterface(IID_PPV_ARGS(&discFormatErase));

	IMAPI_STATUS imapiStatus = {0};
	imapiStatus.elapsedTime = elapsedSeconds;
	imapiStatus.remainingTime = estimatedTotalSeconds - elapsedSeconds;
	imapiStatus.totalTime = estimatedTotalSeconds;

	LRESULT ret = ::SendMessage(pThis->m_hNotifyWnd, WM_IMAPI_UPDATE, IMAPI_FORMAT2_ERASE, (LPARAM)(LPVOID)&imapiStatus);
	if (ret == RETURN_CANCEL_WRITE)
	{
		// There is not such a function -> User has to wait that the erase finishes...
		//discFormatErase->CancelErase();
	}

	return S_OK;
}
