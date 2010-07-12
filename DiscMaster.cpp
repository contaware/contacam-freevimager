///////////////////////////////////////////////////////////////////////
// DiscMaster.cpp
//
// Wrapper for IDiscMaster2 Interface
//
// Written by Eric Haddan
//
#include "StdAfx.h"
#include "DiscMaster.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDiscMaster::CDiscMaster(void)
: m_discMaster(NULL)
, m_hResult(0)
{
}

CDiscMaster::~CDiscMaster(void)
{
	if (m_discMaster)
		m_discMaster->Release();
}

///////////////////////////////////////////////////////////////////////
//
// CDiscMaster::Initialize()
//
// Description:
//		Creates and initializes the IDiscMaster2 interface
//
bool CDiscMaster::Initialize()
{
	ASSERT(m_discMaster == NULL);

	//
	// Initialize the IDiscMaster2 Interface
	//
	if (m_discMaster == NULL)
	{
		m_hResult = CoCreateInstance(__uuidof(MsftDiscMaster2), NULL, CLSCTX_INPROC_SERVER,
			__uuidof(IDiscMaster2), (void**)&m_discMaster);
		if (!SUCCEEDED(m_hResult))
		{
			m_errorMessage = _T("Unable to initialize IMAPIv2!");
			return false;
		}
	}

	//
	// Verify that we have some device that uses this interface
	//
	if (GetTotalDevices() <= 0)
	{
		m_errorMessage = ML_STRING(1813, "There were no writable devices detected!");
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////
//
// CDiscMaster::GetTotalDevices()
//
// Description:
//		Returns the total number of installed CD/DVD devices
//
long CDiscMaster::GetTotalDevices()
{
	ASSERT(m_discMaster != NULL);
	if (m_discMaster == NULL)
		return 0;

	long totalDevices = 0;
	m_hResult = m_discMaster->get_Count(&totalDevices);
	if (FAILED(m_hResult))
	{
		m_errorMessage = _T("Failed to get the Drives count!");
		return 0;
	}

	return totalDevices;
}

///////////////////////////////////////////////////////////////////////
//
// CDiscMaster::GetDeviceUniqueID()
//
// Description:
//		Returns the unique id of the device
//
CString CDiscMaster::GetDeviceUniqueID(long index)
{
	ASSERT(m_discMaster != NULL);
	ASSERT(index < GetTotalDevices());

	BSTR	uniqueID = NULL;
	m_hResult = m_discMaster->get_Item(index, &uniqueID);
	if (FAILED(m_hResult))
	{
		m_errorMessage.Format(_T("Failed to get the Unique ID of Drive %d!"), index);
		return _T("");
	}

	return uniqueID;
}
