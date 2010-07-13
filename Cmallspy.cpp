#include "stdafx.h"
#include "cmallspy.h"
#include <stdio.h>


// ******************************************************************
// ******************************************************************
// Constructor/Destructor
// ******************************************************************
// ******************************************************************

#ifndef NDEBUG

CMallocSpy::CMallocSpy(void)
{
    m_cRef = 1;
    m_counter = 0;
    memset(m_map, 0, MAX_ALLOCATIONS);
}


CMallocSpy::~CMallocSpy(void)
{
	Dump();
}


// ******************************************************************
// ******************************************************************
// IUnknown support ...
// ******************************************************************
// ******************************************************************


HRESULT CMallocSpy::QueryInterface(REFIID riid, LPVOID *ppUnk)
{
    HRESULT hr = S_OK;

    if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppUnk = (IUnknown *) this;
    }
    else if (IsEqualIID(riid, IID_IMallocSpy))
    {
        *ppUnk =  (IMalloc *) this;
    }
    else
    {
        *ppUnk = NULL;
        hr =  E_NOINTERFACE;
    }
    AddRef();
    return hr;
}


ULONG CMallocSpy::AddRef(void)
{
    return ++m_cRef;
}


ULONG CMallocSpy::Release(void)
{
    return --m_cRef;
}


// ******************************************************************
// ******************************************************************
// Utilities ...
// ******************************************************************
// ******************************************************************


void CMallocSpy::SetBreakAlloc(int allocNum)
{
    m_breakAlloc = allocNum;
}


void CMallocSpy::Clear()
{
    memset(m_map, 0, MAX_ALLOCATIONS);
}


void CMallocSpy::Dump()
{
    TCHAR buff[64];
    ::OutputDebugString(_T("CMallocSpy dump ->\n"));
	int nLeaksCount = 0;
    for (int i = 0 ; i <= m_counter ; i++)
    {
        if (m_map[i] != 0)
        {
			if (nLeaksCount++ == 0)
				_stprintf(buff, _T("IMalloc memory leak at: %d"), i);
			else
				_stprintf(buff, _T(",%d"), i);
            ::OutputDebugString(buff);
        }
    }
	if (nLeaksCount == 0)
		::OutputDebugString(_T("OK no IMalloc memory leaks"));
	else
	{
		_stprintf(buff, _T("\n(%d leaks detected)"), nLeaksCount);
		::OutputDebugString(buff);
	}
    ::OutputDebugString(_T("\n<- CMallocSpy dump complete.\n"));
}


// ******************************************************************
// ******************************************************************
// IMallocSpy methods ...
// ******************************************************************
// ******************************************************************


ULONG CMallocSpy::PreAlloc(ULONG cbRequest)
{
    m_cbRequest = cbRequest;
    return cbRequest + HEADERSIZE;
}


void *CMallocSpy::PostAlloc(void *pActual)
{
    m_counter++;

    if (m_breakAlloc == m_counter)
        ::DebugBreak();

    //
    // Store the allocation counter and note that this allocation
    // is active in the map.
    //
    memcpy(pActual, &m_counter, 4);
	ASSERT(m_counter < MAX_ALLOCATIONS);
    m_map[m_counter] = 1;

    return (void*)((BYTE*)pActual + HEADERSIZE);
}


void *CMallocSpy::PreFree(void *pRequest, BOOL fSpyed)
{
    if (pRequest == NULL)
    {
        return NULL;
    }

    if (fSpyed)
    {
        //
        // Mark the allocation as inactive in the map.
        //
        int counter;
        pRequest = (void*)(((BYTE*)pRequest) - HEADERSIZE);
        memcpy(&counter, pRequest, 4);
		ASSERT(counter < MAX_ALLOCATIONS);
        m_map[counter] = 0;

        return pRequest;
    }
    else
    {
        return pRequest;
    }
}


void CMallocSpy::PostFree(BOOL fSpyed)
{
    return;
}


ULONG CMallocSpy::PreRealloc(void *pRequest,
                             ULONG cbRequest,
                             void **ppNewRequest,
                             BOOL fSpyed)
{
    if (fSpyed  &&  pRequest != NULL)
    {
        //
        // Mark the allocation as inactive in the map since IMalloc::Realloc()
        // frees the originally allocated block.
        //
        int counter;
        BYTE* actual = (BYTE*)pRequest - HEADERSIZE;
        memcpy(&counter, actual, 4);
		ASSERT(counter < MAX_ALLOCATIONS);
        m_map[counter] = 0;

        *ppNewRequest = (void*)(((BYTE*)pRequest) - HEADERSIZE);
        return cbRequest + HEADERSIZE;
    }
    else
    {
        *ppNewRequest = pRequest;
        return cbRequest;
    }
}


void *CMallocSpy::PostRealloc(void *pActual, BOOL fSpyed)
{
    if (fSpyed)
    {
        m_counter++;

        if (m_breakAlloc == m_counter)
            ::DebugBreak();

        //
        // Store the allocation counter and note that this allocation
        // is active in the map.
        //
        memcpy(pActual, &m_counter, 4);
		ASSERT(m_counter < MAX_ALLOCATIONS);
        m_map[m_counter] = 1;

        return (void*)((BYTE*)pActual + HEADERSIZE);
    }
    else
    {
        return pActual;
    }
}


void *CMallocSpy::PreGetSize(void *pRequest, BOOL fSpyed)
{
    if (fSpyed)
        return (void *) (((BYTE *) pRequest) - HEADERSIZE);
    else
        return pRequest;
}


ULONG CMallocSpy::PostGetSize(ULONG cbActual, BOOL fSpyed)
{
    if (fSpyed)
        return cbActual - HEADERSIZE;
    else
        return cbActual;
}


void *CMallocSpy::PreDidAlloc(void *pRequest, BOOL fSpyed)
{
    if (fSpyed)
        return (void *) (((BYTE *) pRequest) - HEADERSIZE);
    else
        return pRequest;
}


BOOL CMallocSpy::PostDidAlloc(void *pRequest, BOOL fSpyed, BOOL fActual)
{
    return fActual;
}


void CMallocSpy::PreHeapMinimize(void)
{
    return;
}


void CMallocSpy::PostHeapMinimize(void)
{
    return;
}

#endif
