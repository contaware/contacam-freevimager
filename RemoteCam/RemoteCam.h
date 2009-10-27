#if !defined(AFX_REMOTECAM_H__D24FAC69_6EF6_44BF_9670_BA4D589E6C10__INCLUDED_)
#define AFX_REMOTECAM_H__D24FAC69_6EF6_44BF_9670_BA4D589E6C10__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// RemoteCam.h : main header file for REMOTECAM.DLL

#if !defined( __AFXCTL_H__ )
	#error include 'afxctl.h' before including this file
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CRemoteCamApp : See RemoteCam.cpp for implementation.

class CRemoteCamApp : public COleControlModule
{
public:
	BOOL InitInstance();
	int ExitInstance();
};

extern const GUID CDECL _tlid;
extern const WORD _wVerMajor;
extern const WORD _wVerMinor;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REMOTECAM_H__D24FAC69_6EF6_44BF_9670_BA4D589E6C10__INCLUDED)
