//
// TryEnterCriticalSection.cpp
//

#include "stdafx.h"
#include "TryEnterCriticalSection.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CTryEnterCriticalSection::CTryEnterCriticalSection()
{
	// Mutex or Critical Section?
	m_hKernel32 = ::LoadLibrary(_T("kernel32.dll"));
	if (m_hKernel32)
		m_fpTryEnterCriticalSection = (FPTRYENTERCRITICALSECTION)
										::GetProcAddress(m_hKernel32,
														"TryEnterCriticalSection");
	else
		m_fpTryEnterCriticalSection = NULL;
	if (m_fpTryEnterCriticalSection)
	{
		::InitializeCriticalSection(&m_cs);
		
		// Even if TryEnterCriticalSection is found, it is not
		// necessarily working..., we have to check it!
		if (m_fpTryEnterCriticalSection(&m_cs))
		{
			::LeaveCriticalSection(&m_cs);
			m_hMutex = NULL;
		}
		else
		{
			::DeleteCriticalSection(&m_cs);
			m_fpTryEnterCriticalSection = NULL;
			::FreeLibrary(m_hKernel32);
			m_hKernel32 = NULL;
			m_hMutex = ::CreateMutex(NULL, FALSE, NULL);
		}
	}
	else
	{
		if (m_hKernel32)
		{
			::FreeLibrary(m_hKernel32);
			m_hKernel32 = NULL;
		}
		m_hMutex = ::CreateMutex(NULL, FALSE, NULL);
	}
}

CTryEnterCriticalSection::~CTryEnterCriticalSection()
{
	if (m_fpTryEnterCriticalSection)
		::DeleteCriticalSection(&m_cs);
	if (m_hMutex)
		::CloseHandle(m_hMutex);
	if (m_hKernel32)
		::FreeLibrary(m_hKernel32);
}

void CTryEnterCriticalSection::EnableTimeout()
{
	if (m_fpTryEnterCriticalSection)
	{
		::DeleteCriticalSection(&m_cs);
		m_fpTryEnterCriticalSection = NULL;
	}
	if (m_hKernel32)
	{
		::FreeLibrary(m_hKernel32);
		m_hKernel32 = NULL;
	}
	if (m_hMutex == NULL)
		m_hMutex = ::CreateMutex(NULL, FALSE, NULL);
}

