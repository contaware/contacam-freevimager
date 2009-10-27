///////////////////////////////////////////////////////////////////////
// DiscFormatErase.cpp
//
// Wrapper for IDiscFormat2Erase Interface
//
// Written by Eric Haddan
//

#include "StdAfx.h"
#include "DiscFormatErase.h"
#include "DiscFormatEraseEvent.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDiscFormatErase::CDiscFormatErase(void)
: m_discFormatErase(NULL)
, m_mediaTypesArray(NULL)
, m_hResult(0)
, m_fullErase(false)
, m_hNotificationWnd(NULL)
{
}

CDiscFormatErase::~CDiscFormatErase(void)
{
	if (m_discFormatErase != NULL)
	{
		m_discFormatErase->Release();
	}
}

bool CDiscFormatErase::Initialize(CDiscRecorder* pDiscRecorder, const CString& clientName)
{
	ASSERT(m_discFormatErase == NULL);
	ASSERT(pDiscRecorder != NULL);
	if (pDiscRecorder == NULL)
	{
		m_errorMessage = _T("CDiscFormatErase::Initialize error - pDiscRecorder is NULL!");
		return false;
	}

	// Initialize the IDiscFormat2Erase Interface
	m_hResult = CoCreateInstance(__uuidof(MsftDiscFormat2Erase), NULL, CLSCTX_INPROC_SERVER,
			__uuidof(IDiscFormat2Erase), (void**)&m_discFormatErase);
	if (!SUCCEEDED(m_hResult))
	{
		m_errorMessage = _T("Unable to initialize the Disc Format Erase interface");
		return false;
	}

	// Setup the Disc Format Erase Information
	VARIANT_BOOL isSupported = VARIANT_FALSE;
	m_hResult = m_discFormatErase->IsRecorderSupported(pDiscRecorder->GetInterface(), &isSupported);
	if (isSupported == VARIANT_FALSE)
	{
		m_errorMessage = _T("Recorder not supported");
		return false;
	}

	m_hResult = m_discFormatErase->put_Recorder(pDiscRecorder->GetInterface());
	if (!SUCCEEDED(m_hResult))
	{
		m_errorMessage = _T("Recorder cannot be used");
		return false;
	}

	m_hResult = m_discFormatErase->put_ClientName(clientName.AllocSysString());
	if (!SUCCEEDED(m_hResult))
	{
		m_errorMessage = _T("Recorder cannot be used");
		return false;
	}

	m_hResult = m_discFormatErase->get_SupportedMediaTypes(&m_mediaTypesArray);
	if (!SUCCEEDED(m_hResult))
	{
		m_errorMessage = _T("Cannot determine the supported Media Types!");
		return false;
	}

	return true;
}

ULONG CDiscFormatErase::GetTotalSupportedMediaTypes()
{
	if (m_mediaTypesArray == NULL)
		return 0;

	return m_mediaTypesArray->rgsabound[0].cElements;
}

int CDiscFormatErase::GetSupportedMediaType(ULONG index)
{
	ASSERT(index < GetTotalSupportedMediaTypes());
	if (index < GetTotalSupportedMediaTypes())
	{
		if (m_mediaTypesArray)
		{
			return ((VARIANT*)(m_mediaTypesArray->pvData))[index].intVal;
		}
	}

	return 0;
}

bool CDiscFormatErase::Erase(HWND hNotificationWnd)
{
	// Check
	if (m_discFormatErase == NULL)
		return false;
	if (hNotificationWnd == NULL)
		return false;

	// Set Notify Window
	m_hNotificationWnd = hNotificationWnd;

	// Create the event sink
	CDiscFormatEraseEvent* eventSink = CDiscFormatEraseEvent::CreateEventSink();
	if (eventSink == NULL)
	{
		m_errorMessage = _T("Unable to create event sink");
		return false;
	}
	if (!eventSink->ConnectDiscFormatErase(this))
	{
		m_errorMessage = _T("Unable to connect event sink with interface");
		delete eventSink;
		return false;
	}
	eventSink->SetHwnd(m_hNotificationWnd);

	// Erase
	m_discFormatErase->put_FullErase(m_fullErase ? VARIANT_TRUE : VARIANT_FALSE);
	m_hResult = m_discFormatErase->EraseMedia();

	// Clean-up
	delete eventSink;

	// Return
	if (SUCCEEDED(m_hResult))
		return true;
	else
	{
		if (m_hResult == E_IMAPI_ERASE_MEDIA_IS_NOT_SUPPORTED)
			m_errorMessage = _T("Erase Failed, Media is not ReWritable!");
		else
			m_errorMessage = _T("Erase Failed!");
		return false;
	}
}

