/////////////////////////////////////////////////////////////////////////////
// stdafx.h: Precompiled headers
/////////////////////////////////////////////////////////////////////////////

#if !defined(StdAfx_defined)
#define StdAfx_defined

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "tchar.h"

#define VC_EXTRALEAN

#include <afxwin.h>
#include <afxext.h>
#include <afxdisp.h>
#include <afxdtctl.h>
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>
#endif

#ifndef SM_CMONITORS

	#define MONITOR_DEFAULTTONULL       0x00000000
	#define MONITOR_DEFAULTTOPRIMARY    0x00000001
	#define MONITOR_DEFAULTTONEAREST    0x00000002
	#define MONITORINFOF_PRIMARY        0x00000001
	#define SM_CMONITORS				80
	#define SM_SAMEDISPLAYFORMAT		81
	#if !defined(HMONITOR_DECLARED) && (WINVER < 0x0500)
        #define HMONITOR_DECLARED
        DECLARE_HANDLE(HMONITOR);
    #endif

	typedef struct tagMONITORINFO
	{
		DWORD   cbSize;
		RECT    rcMonitor;
		RECT    rcWork;
		DWORD   dwFlags;
	} MONITORINFO, *LPMONITORINFO;

	typedef struct tagMONITORINFOEXA : public tagMONITORINFO
	{
		CHAR        szDevice[CCHDEVICENAME];
	} MONITORINFOEXA, *LPMONITORINFOEXA;
	typedef struct tagMONITORINFOEXW : public tagMONITORINFO
	{
		WCHAR       szDevice[CCHDEVICENAME];
	} MONITORINFOEXW, *LPMONITORINFOEXW;
#ifdef UNICODE
	typedef MONITORINFOEXW MONITORINFOEX;
	typedef LPMONITORINFOEXW LPMONITORINFOEX;
#else
	typedef MONITORINFOEXA MONITORINFOEX;
	typedef LPMONITORINFOEXA LPMONITORINFOEX;
#endif

#endif
typedef HMONITOR (WINAPI * FPMONITORFROMWINDOW)(HWND hwnd, DWORD dwFlags);
typedef BOOL (WINAPI * FPGETMONITORINFO)(HMONITOR hMonitor, LPMONITORINFO lpmi);

#endif // !defined(StdAfx_defined)
