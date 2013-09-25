/////////////////////////////////////////////////////////////////////////////
// stdafx.h: Precompiled headers
/////////////////////////////////////////////////////////////////////////////

#if !defined(StdAfx_defined)
#define StdAfx_defined

#pragma once

#include "tchar.h"

#define VC_EXTRALEAN

#include <afxwin.h>
#include <afxext.h>
#include <afxdisp.h>
#include <afxdtctl.h>
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>
#endif

typedef HMONITOR (WINAPI * FPMONITORFROMWINDOW)(HWND hwnd, DWORD dwFlags);
typedef BOOL (WINAPI * FPGETMONITORINFO)(HMONITOR hMonitor, LPMONITORINFO lpmi);

#endif // !defined(StdAfx_defined)
