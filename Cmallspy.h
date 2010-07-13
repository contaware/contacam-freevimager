#ifndef _CMallocSpy_h
#define _CMallocSpy_h

#ifndef NDEBUG

/*
OLE Automation caches allocations of BSTRs. This caching creates confusion
when attempting to locate COM related memory leaks. For more information on
this issue, see: http://support.microsoft.com/support/kb/articles/q139/0/71.asp
To disable BSTR caching, you will need to do the following:

Set the OANOCACHE environment variable

    1. In Control Panel, double-click the System icon. The System Properties dialog box appears.
    2. On the Advanced tab, click Environment Variables. The Environment Variables dialog box appears.
    3. Under System variables, click New. The New System Variable dialog box appears.
    4. Type OANOCACHE in the Variable Name box.
    5. Type 1 in the Variable Value box.
    6. Click OK three times.

Before you change the OANOCACHE environment variable, you must stop the following server processes if they are running:

    * The Inetinfo process
    * The Dllhost process
    * The W3wp process
    * The Aspnet_wp process

Note If you are not sure whether these server processes are running, set the environment variable OANOCACHE=1,
and then restart the computer.
*/

/*
In general, the rules for allocating and releasing memory allocated for BSTRs are as follows:

    * When you call into a function that expects a BSTR argument,
	you must allocate the memory for the BSTR before the call and release it afterwards.

	For example:

    BSTR bstrStatus = ::SysAllocString(L"Some text");
    if (bstrStatus == NULL)
        return E_OUTOFMEMORY;
    pBrowser->put_StatusText(bstrStatus);
    ::SysFreeString(bstrStatus);

    * When you call into a function that returns a BSTR, you must free the string yourself.

	For example:
    
    BSTR bstrStatus = NULL;
    pBrowser->get_StatusText(&bstrStatus); 
    ::SysFreeString(bstrStatus);

    * When you implement a function that returns a BSTR, allocate the string but do not free it.
	The receiving function releases the memory.
	
	For example:

	HRESULT CMyClass::get_StatusText(BSTR* pbstr)
    {
        try
        {
            // m_str is a CString in your class
            *pbstr = m_str.AllocSysString();
        }
        catch (...)
        {
            return E_OUTOFMEMORY;
        }

        // The client is now responsible for freeing pbstr.
        return(S_OK);
    }
*/

class CMallocSpy : public IMallocSpy
{
public:
    CMallocSpy(void);
    ~CMallocSpy(void);

    //
    // IUnknown methods
    //
    STDMETHOD(QueryInterface) (REFIID riid, LPVOID *ppUnk);
    STDMETHOD_(ULONG, AddRef) (void);
    STDMETHOD_(ULONG, Release) (void);

    //
    // IMallocSpy methods
    //
    STDMETHOD_(ULONG, PreAlloc) (ULONG cbRequest);
    STDMETHOD_(void*, PostAlloc) (void* pActual);

    STDMETHOD_(void*, PreFree) (void* pRequest, BOOL fSpyed);
    STDMETHOD_(void, PostFree) (BOOL fSpyed);

    STDMETHOD_(ULONG, PreRealloc) (void* pRequest, ULONG cbRequest,
                                   void** ppNewRequest, BOOL fSpyed);
    STDMETHOD_(void*, PostRealloc) (void* pActual, BOOL fSpyed);

    STDMETHOD_(void*, PreGetSize) (void* pRequest, BOOL fSpyed);
    STDMETHOD_(ULONG, PostGetSize) (ULONG cbActual, BOOL fSpyed);

    STDMETHOD_(void*, PreDidAlloc) (void* pRequest, BOOL fSpyed);
    STDMETHOD_(BOOL, PostDidAlloc) (void* pRequest, BOOL fSpyed, BOOL fActual);

    STDMETHOD_(void, PreHeapMinimize) (void);
    STDMETHOD_(void, PostHeapMinimize) (void);

    //
    // Utilities ...
    //
    void Clear();
    void Dump();
    void SetBreakAlloc(int allocNum);

protected:
    enum
    {
        HEADERSIZE = 4,
        MAX_ALLOCATIONS = 1000000   // cannot handle more than max
    };

    ULONG   m_cRef;
    ULONG   m_cbRequest;
    int     m_counter;
    int     m_breakAlloc;

    char    m_map[MAX_ALLOCATIONS];
};

#endif

#endif   // _CMallocSpy_h
