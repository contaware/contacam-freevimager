// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__53BABAC5_8A1F_4462_8394_E4DFE06B0809__INCLUDED_)
#define AFX_STDAFX_H__53BABAC5_8A1F_4462_8394_E4DFE06B0809__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#define OEMRESOURCE

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
#include "MlString.h"
#include "Helpers.h"
#include "Performance.h"
#include "TraceLogFile.h"
#include "appconst.h"
#include "bigalloc.h"
#if (_MSC_VER > 1200)
#include <atlfile.h>
#include <atlenc.h>
#include <atlsocket.h>
#endif
#ifndef NDEBUG
#include "cmallspy.h"
extern CMallocSpy MallocSpy;
#endif

// App Command Defines
#ifndef WM_APPCOMMAND
#define WM_APPCOMMAND					0x0319
#endif
#ifndef APPCOMMAND_BROWSER_BACKWARD
#define APPCOMMAND_BROWSER_BACKWARD		1
#endif
#ifndef APPCOMMAND_BROWSER_FORWARD
#define APPCOMMAND_BROWSER_FORWARD		2
#endif
#ifndef FAPPCOMMAND_MASK
#define FAPPCOMMAND_MASK				0xF000
#endif
#ifndef GET_APPCOMMAND_LPARAM
#define GET_APPCOMMAND_LPARAM(lParam) ((short)(HIWORD(lParam) & ~FAPPCOMMAND_MASK))
#endif

// Transparency Defines
#ifndef WS_EX_LAYERED
#define WS_EX_LAYERED					0x00080000
#endif
#ifndef LWA_COLORKEY
#define LWA_COLORKEY					0x00000001
#endif
#ifndef LWA_ALPHA
#define LWA_ALPHA						0x00000002
#endif
#ifndef ULW_COLORKEY
#define ULW_COLORKEY					0x00000001
#endif
#ifndef ULW_ALPHA
#define ULW_ALPHA						0x00000002
#endif
#ifndef ULW_OPAQUE
#define ULW_OPAQUE						0x00000004
#endif

// PopUp Menu No-Animation define
#ifndef TPM_NOANIMATION
#define TPM_NOANIMATION					0x4000
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

typedef BOOL (CALLBACK* MONITORENUMPROC)(HMONITOR, HDC, LPRECT, LPARAM);
typedef BOOL (WINAPI * FPENUMDISPLAYMONITORS)(HDC hdc, LPCRECT lprcClip, MONITORENUMPROC lpfnEnum, LPARAM dwData);
typedef HMONITOR (WINAPI * FPMONITORFROMWINDOW)(HWND hwnd, DWORD dwFlags);
typedef HMONITOR (WINAPI * FPMONITORFROMPOINT)(POINT pt, DWORD dwFlags);
typedef HMONITOR (WINAPI * FPMONITORFROMRECT)(LPCRECT lprc, DWORD dwFlags);
typedef BOOL (WINAPI * FPGETMONITORINFO)(HMONITOR hMonitor, LPMONITORINFO lpmi);

// Session change notification
#ifndef NOTIFY_FOR_ALL_SESSIONS
#define NOTIFY_FOR_ALL_SESSIONS     1
#endif
#ifndef NOTIFY_FOR_THIS_SESSION
#define NOTIFY_FOR_THIS_SESSION     0
#endif
#ifndef WM_WTSSESSION_CHANGE
#define WM_WTSSESSION_CHANGE			0x02B1
#define WTS_CONSOLE_CONNECT				0x1
#define WTS_CONSOLE_DISCONNECT			0x2
#define WTS_REMOTE_CONNECT				0x3
#define WTS_REMOTE_DISCONNECT			0x4
#define WTS_SESSION_LOGON				0x5
#define WTS_SESSION_LOGOFF				0x6
#define WTS_SESSION_LOCK				0x7
#define WTS_SESSION_UNLOCK				0x8
#define WTS_SESSION_REMOTE_CONTROL		0x9
#endif

#ifndef MAX_FILEDLG_PATH
#define MAX_FILEDLG_PATH				65535 // If you put 65536 it's not working on win2000...
#endif

// Vista and higher Shield Icon
#ifndef BCM_FIRST
#define BCM_FIRST			0x1600
#endif
#ifndef BCM_SETSHIELD
#define BCM_SETSHIELD		(BCM_FIRST + 0x000C)
#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__53BABAC5_8A1F_4462_8394_E4DFE06B0809__INCLUDED_)