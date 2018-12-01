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

// Maximum file path size for file dialog
#define MAX_FILEDLG_PATH							1048576		// 1M

#if(WINVER < 0x0601)
/*
 * Gesture Message
 */
#define WM_GESTURE									0x0119

/*
 * Gesture flags - GESTUREINFO.dwFlags
 */
#define GF_BEGIN									0x00000001
#define GF_INERTIA									0x00000002
#define GF_END										0x00000004

/*
 * Gesture IDs
 */
#define GID_BEGIN									1
#define GID_END										2
#define GID_ZOOM									3
#define GID_PAN										4
#define GID_ROTATE									5
#define GID_TWOFINGERTAP							6
#define GID_PRESSANDTAP								7

/*
 * Zoom gesture configuration flags - set GESTURECONFIG.dwID to GID_ZOOM
 */
#define GC_ZOOM                                     0x00000001

/*
 * Pan gesture configuration flags - set GESTURECONFIG.dwID to GID_PAN
 */
#define GC_PAN                                      0x00000001
#define GC_PAN_WITH_SINGLE_FINGER_VERTICALLY        0x00000002
#define GC_PAN_WITH_SINGLE_FINGER_HORIZONTALLY      0x00000004
#define GC_PAN_WITH_GUTTER                          0x00000008
#define GC_PAN_WITH_INERTIA                         0x00000010

/*
 * Rotate gesture configuration flags - set GESTURECONFIG.dwID to GID_ROTATE
 */
#define GC_ROTATE                                   0x00000001

/*
 * Gesture argument helpers
 *   - Angle should be a double in the range of -2pi to +2pi
 *   - Argument should be an unsigned 16-bit value
 */
#define GID_ROTATE_ANGLE_TO_ARGUMENT(_arg_)			((USHORT)((((_arg_) + 2.0 * 3.14159265) / (4.0 * 3.14159265)) * 65535.0))
#define GID_ROTATE_ANGLE_FROM_ARGUMENT(_arg_)		((((double)(_arg_) / 65535.0) * 4.0 * 3.14159265) - 2.0 * 3.14159265)

/*
 * Two finger tap gesture configuration flags - set GESTURECONFIG.dwID to GID_TWOFINGERTAP
 */
#define GC_TWOFINGERTAP                             0x00000001

/*
 * PressAndTap gesture configuration flags - set GESTURECONFIG.dwID to GID_PRESSANDTAP
 */
#define GC_PRESSANDTAP                              0x00000001
#define GC_ROLLOVER                                 GC_PRESSANDTAP

/*
 * Gesture configuration structure
 *   - Used in SetGestureConfig and GetGestureConfig
 *   - Note that any setting not included in either GESTURECONFIG.dwWant or
 *     GESTURECONFIG.dwBlock will use the parent window's preferences or
 *     system defaults.
 */
typedef struct tagGESTURECONFIG {
    DWORD dwID;                     // gesture ID
    DWORD dwWant;                   // settings related to gesture ID that are to be turned on
    DWORD dwBlock;                  // settings related to gesture ID that are to be turned off
} GESTURECONFIG, *PGESTURECONFIG;

/*
 * Gesture information handle
 */
DECLARE_HANDLE(HGESTUREINFO_COMPATIBLE);

/*
 * Backward compatible gesture information structure because the original
 * one is defined as empty in afxwin.h:
 * typedef struct tagGESTUREINFO {
 * } GESTUREINFO, *PGESTUREINFO;
 *
 *   - Pass the HGESTUREINFO received in the WM_GESTURE message lParam into the
 *     GetGestureInfo function to retrieve this information.
 *   - If cbExtraArgs is non-zero, pass the HGESTUREINFO received in the WM_GESTURE
 *     message lParam into the GetGestureExtraArgs function to retrieve extended
 *     argument information.
 */
typedef struct tagGESTUREINFO_COMPATIBLE {
    UINT cbSize;                    // size, in bytes, of this structure (including variable length Args field)
    DWORD dwFlags;                  // see GF_* flags
    DWORD dwID;                     // gesture ID, see GID_* defines
    HWND hwndTarget;                // handle to window targeted by this gesture
    POINTS ptsLocation;             // current location of this gesture
    DWORD dwInstanceID;             // internally used
    DWORD dwSequenceID;             // internally used
    ULONGLONG ullArguments;         // arguments for gestures whose arguments fit in 8 BYTES
    UINT cbExtraArgs;               // size, in bytes, of extra arguments, if any, that accompany this gesture
} GESTUREINFO_COMPATIBLE, *PGESTUREINFO_COMPATIBLE;
typedef GESTUREINFO_COMPATIBLE const * PCGESTUREINFO_COMPATIBLE;

#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__53BABAC5_8A1F_4462_8394_E4DFE06B0809__INCLUDED_)