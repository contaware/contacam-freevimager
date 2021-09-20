// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__53BABAC5_8A1F_4462_8394_E4DFE06B0809__INCLUDED_)
#define AFX_STDAFX_H__53BABAC5_8A1F_4462_8394_E4DFE06B0809__INCLUDED_

#pragma once

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
#include <atomic>
#include <afxtempl.h>
#include <direct.h>
#include <string>
#include <Windns.h>
#include <wincrypt.h>
#include "MlString.h"
#include "MMSystem.h"
#include "Helpers.h"
#include "Round.h"
#include "TraceLogFile.h"
#include "Performance.h"
#include "appconst.h"
#include "bigalloc.h"
#include <atlfile.h>
#include <atlenc.h>
#include <atlsocket.h>
#include <atlimage.h>
#include <afxtaskdialog.h>

// Maximum file path size for file dialog
#define MAX_FILEDLG_PATH							1048576		// 1M

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__53BABAC5_8A1F_4462_8394_E4DFE06B0809__INCLUDED_)