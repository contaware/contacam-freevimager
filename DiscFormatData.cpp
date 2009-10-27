///////////////////////////////////////////////////////////////////////
// DiscFormatData.cpp
//
// Wrapper for IDiscFormat2Data Interface
//
// Written by Eric Haddan
//

#include "StdAfx.h"
#include "DiscFormatData.h"
#include "DiscFormatDataEvent.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDiscFormatData::CDiscFormatData(void)
: m_discFormatData(NULL)
, m_mediaTypesArray(NULL)
, m_hResult(0)
, m_hNotificationWnd(NULL)
, m_closeMedia(true)
{
}

CDiscFormatData::~CDiscFormatData(void)
{
	if (m_discFormatData != NULL)
	{
		m_discFormatData->Release();
	}
}

bool CDiscFormatData::Initialize(CDiscRecorder* pDiscRecorder, const CString& clientName)
{
	ASSERT(m_discFormatData == NULL);
	ASSERT(pDiscRecorder != NULL);
	if (pDiscRecorder == NULL)
	{
		m_errorMessage = _T("CDiscFormatData::Initialize error - pDiscRecorder is NULL!");
		return false;
	}

	// Initialize the IDiscFormat2Data Interface
	m_hResult = CoCreateInstance(__uuidof(MsftDiscFormat2Data), NULL, CLSCTX_INPROC_SERVER,
			__uuidof(IDiscFormat2Data), (void**)&m_discFormatData);
	if (!SUCCEEDED(m_hResult))
	{
		m_errorMessage = _T("Unable to initialize the Disc Format Data interface");
		return false;
	}

	// Setup the Disc Format Information
	VARIANT_BOOL isSupported = VARIANT_FALSE;
	m_hResult = m_discFormatData->IsRecorderSupported(pDiscRecorder->GetInterface(), &isSupported);
	if (isSupported == VARIANT_FALSE)
	{
		m_errorMessage = _T("Recorder not supported");
		return false;
	}

	m_hResult = m_discFormatData->put_Recorder(pDiscRecorder->GetInterface());
	if (!SUCCEEDED(m_hResult))
	{
		m_errorMessage = _T("Recorder cannot be used");
		return false;
	}

	m_hResult = m_discFormatData->put_ClientName(clientName.AllocSysString());
	if (!SUCCEEDED(m_hResult))
	{
		m_errorMessage = _T("Recorder cannot be used");
		return false;
	}

	m_hResult = m_discFormatData->get_SupportedMediaTypes(&m_mediaTypesArray);
	if (!SUCCEEDED(m_hResult))
	{
		m_errorMessage = _T("Cannot determine the supported Media Types!");
		return false;
	}

	return true;
}

ULONG CDiscFormatData::GetTotalSupportedMediaTypes()
{
	if (m_mediaTypesArray == NULL)
		return 0;

	return m_mediaTypesArray->rgsabound[0].cElements;
}

int CDiscFormatData::GetSupportedMediaType(ULONG index)
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

bool CDiscFormatData::Burn(HWND hNotificationWnd, IStream* streamData)
{
	// Check
	if (m_discFormatData == NULL)
		return false;
	if (hNotificationWnd == NULL)
		return false;
	if (streamData == NULL)
		return false;

	// Set Stream Data
	m_streamData = streamData;

	// Set Notify Window
	m_hNotificationWnd = hNotificationWnd;

	// Create the event sink
	CDiscFormatDataEvent* eventSink = CDiscFormatDataEvent::CreateEventSink();
	if (eventSink == NULL)
	{
		m_errorMessage = _T("Unable to create event sink");
		return false;
	}
	if (!eventSink->ConnectDiscFormatData(this))
	{
		m_errorMessage = _T("Unable to connect event sink with interface");
		delete eventSink;
		return false;
	}
	eventSink->SetHwnd(m_hNotificationWnd);

	// Burn
	m_discFormatData->put_ForceMediaToBeClosed(m_closeMedia ? VARIANT_TRUE : VARIANT_FALSE);
	m_discFormatData->put_ForceOverwrite(VARIANT_TRUE);
	m_hResult = m_discFormatData->Write(m_streamData);

	// Clean-up
	delete eventSink;

	// Return
	if (SUCCEEDED(m_hResult))
		return true;
	else
	{
		m_errorMessage = _T("Write Failed!");
		return false;
	}
}

