#if !defined(AFX_STDAFX_H__DE37A14B_3478_4F41_A965_3D28499791A8__INCLUDED_)
#define AFX_STDAFX_H__DE37A14B_3478_4F41_A965_3D28499791A8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// stdafx.h : include file for standard system include files,
//      or project specific include files that are used frequently,
//      but are changed infrequently

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#include <winsock2.h>
#include <afxpriv.h>
#include <afxcmn.h>			// MFC support for Windows Common Controls
#include <afxole.h>
#include <afxinet.h>
#include <locale.h>
#include <afxmt.h>
#include <math.h>
#include <afxtempl.h>
#include <direct.h>
#include <string>
#include <Windns.h>
#include <wincrypt.h>
#include <afxctl.h>
#include "Helpers.h"

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

typedef BOOL (CALLBACK* MONITORENUMPROC)(HMONITOR, HDC, LPRECT, LPARAM);
typedef BOOL (WINAPI * FPENUMDISPLAYMONITORS)(HDC hdc, LPCRECT lprcClip, MONITORENUMPROC lpfnEnum, LPARAM dwData);
typedef HMONITOR (WINAPI * FPMONITORFROMWINDOW)(HWND hwnd, DWORD dwFlags);
typedef HMONITOR (WINAPI * FPMONITORFROMPOINT)(POINT pt, DWORD dwFlags);
typedef BOOL (WINAPI * FPGETMONITORINFO)(HMONITOR hMonitor, LPMONITORINFO lpmi);

// Delete the two includes below if you do not wish to use the MFC
//  database classes
#include <afxdb.h>			// MFC database classes
#include <afxdao.h>			// MFC DAO database classes

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__DE37A14B_3478_4F41_A965_3D28499791A8__INCLUDED_)
