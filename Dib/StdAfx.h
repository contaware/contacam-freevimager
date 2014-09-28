// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__9B5E2BC8_D13C_4369_808E_459ECFD5BD3B__INCLUDED_)
#define AFX_STDAFX_H__9B5E2BC8_D13C_4369_808E_459ECFD5BD3B__INCLUDED_

#pragma once

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afx.h>
#include <afxwin.h>
#include <afxadv.h>
#include <afxole.h>
#include <wingdi.h>
#include <math.h>
#include <windowsx.h>
#include "..\appconst.h"
#include "..\bigalloc.h"

// Including both stdint.h and intsafe.h generates warnings in vs2010 (fixed in vs2012)
// http://connect.microsoft.com/VisualStudio/feedback/details/621653/including-stdint-after-intsafe-generates-warnings
#if _MSC_VER == 1600
#pragma warning(push)
#pragma warning(disable : 4005)
#include <intsafe.h>
#include <stdint.h>
#pragma warning(pop)
#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__9B5E2BC8_D13C_4369_808E_459ECFD5BD3B__INCLUDED_)
