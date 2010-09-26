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

CRITICAL_SECTION g_csAVCodec;
BOOL g_bAVCodecCSInited = FALSE;
int avcodec_open_thread_safe(AVCodecContext *avctx, AVCodec *codec)
{
	::EnterCriticalSection(&g_csAVCodec);
	int ret = avcodec_open(avctx, codec);
	::LeaveCriticalSection(&g_csAVCodec);
	return ret;
}
int avcodec_close_thread_safe(AVCodecContext *avctx)
{
	::EnterCriticalSection(&g_csAVCodec);
	int ret = avcodec_close(avctx);
	::LeaveCriticalSection(&g_csAVCodec);
	return ret;
}
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
extern int mm_flags;
extern int mm_support(void);
}

// Called only once also if multiple controls in a application!
BOOL CRemoteCamApp::InitInstance()
{
	BOOL bInit = COleControlModule::InitInstance();

	if (bInit)
	{
		// Init Global Helper Functions
		// (inits OSs flags and processor instruction sets flags)
		::InitHelpers();

		// AVCODEC Init
		::InitializeCriticalSection(&g_csAVCodec);
		g_bAVCodecCSInited = TRUE;
#ifdef _DEBUG
		av_log_set_callback(my_av_log_trace);
#else
		av_log_set_callback(my_av_log_empty);
#endif
		/*	ffmpeg automatically detects the best instructions
			for the given CPU. Set mm_support_mask to limit the
			used instructions.
		FF_MM_MMX		// standard MMX
		FF_MM_3DNOW		// AMD 3DNOW
		FF_MM_MMXEXT	// SSE integer functions or AMD MMX ext
		FF_MM_SSE		// SSE functions
		FF_MM_SSE2		// PIV SSE2 functions
		FF_MM_3DNOWEXT	// AMD 3DNowExt
		FF_MM_SSE3		// Prescott SSE3 functions
		FF_MM_SSSE3		// Conroe SSSE3 functions
		*/
		// Win95 and NT4 (or older NT) always crash if enabling higher than mmx.
		// On newer systems I had some strange crashes...better to always disable sse2 and higher!
		if (g_bWin95 || g_bNT4OrOlder)
			mm_support_mask = FF_MM_MMX;
		else
			mm_support_mask = FF_MM_MMX | FF_MM_3DNOW | FF_MM_MMXEXT | FF_MM_SSE;
		avcodec_init();
		register_avcodec(&h263_decoder);
		// Initializing mm_flags is necessary when using deinterlacing,
		// otherwise the emms_c() macro in avpicture_deinterlace() of the imgconvert.c file
		// may be empty and the emms instruction is not called after using MMX!
		// Note: all codecs except the raw one call dsputil_init() which executes mm_flags = mm_support()
		mm_flags = mm_support();
	}

	return bInit;
}

// Called only once also if multiple controls in a application!
int CRemoteCamApp::ExitInstance()
{
	// Clean-Up Global Helper Functions
	::EndHelpers();

	// Delete Critical Section
	if (g_bAVCodecCSInited)
	{
		g_bAVCodecCSInited = FALSE;
		::DeleteCriticalSection(&g_csAVCodec);
	}

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
