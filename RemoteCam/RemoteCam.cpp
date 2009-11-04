// RemoteCam.cpp : Implementation of CRemoteCamApp and DLL registration.

#include "stdafx.h"
#include "RemoteCam.h"
extern "C"
{
#include "ffmpeg\\libavcodec\\avcodec.h"
}

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// for libmingwex.a
#if (_MSC_VER < 1400)
extern "C" int _get_output_format(void)
{
	return 0;
}
#endif
#pragma comment(lib, "libavcodec.a")
#pragma comment(lib, "libavutil.a")
#pragma comment(lib, "libgcc.a")
#pragma comment(lib, "libmingwex.a")

CRemoteCamApp NEAR theApp;

const GUID CDECL BASED_CODE _tlid =
		{ 0xd53ad502, 0x626b, 0x4b71, { 0x94, 0x8c, 0x4f, 0x69, 0xb8, 0xb4, 0xc2, 0xa3 } };
const WORD _wVerMajor = 1;
const WORD _wVerMinor = 0;


////////////////////////////////////////////////////////////////////////////
// CRemoteCamApp::InitInstance - DLL initialization

static void my_av_log_trace(void* ptr, int level, const char* fmt, va_list vl)
{
	// Fix Format
	CString sFmt(fmt);
	sFmt.Replace(_T("%td"), _T("%d"));	// %td not supported by vc++
	sFmt.Replace(_T("%ti"), _T("%i"));	// %ti not supported by vc++
	
	// Convert fixed format to Ascii
#ifdef _UNICODE
	char* asciifmt = new char[sFmt.GetLength() + 1];
	if (!asciifmt)
		return;
	::wcstombs(asciifmt, (LPCTSTR)sFmt, sFmt.GetLength() + 1);
	asciifmt[sFmt.GetLength()] = '\0';
#endif
	
	// Make message string
	char s[1024];
#ifdef _UNICODE
	_vsnprintf(s, 1024, asciifmt, vl);
	delete [] asciifmt;
#else
	_vsnprintf(s, 1024, (LPCTSTR)sFmt, vl);
#endif
	s[1023] = '\0';

	// Output message string
	TRACE(CString(s));
}
static void my_av_log_empty(void* ptr, int level, const char* fmt, va_list vl)
{
	return;
}
extern "C"
{
extern int mm_support_mask;
extern AVCodec h263_decoder;
}

BOOL CRemoteCamApp::InitInstance()
{
	BOOL bInit = COleControlModule::InitInstance();

	if (bInit)
	{
#ifdef _DEBUG
		av_log_set_callback(my_av_log_trace);
#else
		av_log_set_callback(my_av_log_empty);
#endif
		// Win95 and NT4 always crash if enabling sse2 and on newer systems
		// I had some strange crashes...better to always disable sse2 and higher
		//mm_support_mask = FF_MM_MMX | FF_MM_3DNOW | FF_MM_MMXEXT | FF_MM_SSE | FF_MM_SSE2 | FF_MM_3DNOWEXT | FF_MM_SSE3 | FF_MM_SSSE3;
		mm_support_mask = FF_MM_MMX | FF_MM_3DNOW | FF_MM_MMXEXT | FF_MM_SSE;
		avcodec_init();
		register_avcodec(&h263_decoder);
	}

	return bInit;
}


////////////////////////////////////////////////////////////////////////////
// CRemoteCamApp::ExitInstance - DLL termination

int CRemoteCamApp::ExitInstance()
{
	return COleControlModule::ExitInstance();
}


/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer - Adds entries to the system registry

STDAPI DllRegisterServer(void)
{
	AFX_MANAGE_STATE(_afxModuleAddrThis);

	if (!AfxOleRegisterTypeLib(AfxGetInstanceHandle(), _tlid))
		return ResultFromScode(SELFREG_E_TYPELIB);

	if (!COleObjectFactoryEx::UpdateRegistryAll(TRUE))
		return ResultFromScode(SELFREG_E_CLASS);

	return NOERROR;
}


/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
{
	AFX_MANAGE_STATE(_afxModuleAddrThis);

	if (!AfxOleUnregisterTypeLib(_tlid, _wVerMajor, _wVerMinor))
		return ResultFromScode(SELFREG_E_TYPELIB);

	if (!COleObjectFactoryEx::UpdateRegistryAll(FALSE))
		return ResultFromScode(SELFREG_E_CLASS);

	return NOERROR;
}
