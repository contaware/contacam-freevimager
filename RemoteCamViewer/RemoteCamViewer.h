// RemoteCamViewer.h : main header file for the REMOTECAMVIEWER application
//

#if !defined(AFX_REMOTECAMVIEWER_H__F65CD5A3_7F3C_43D8_BE35_7690D364AC4D__INCLUDED_)
#define AFX_REMOTECAMVIEWER_H__F65CD5A3_7F3C_43D8_BE35_7690D364AC4D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CRemoteCamViewerApp:
// See RemoteCamViewer.cpp for the implementation of this class
//

class CRemoteCamViewerApp : public CWinApp
{
public:
	CRemoteCamViewerApp();
	BOOL IsRegistered();
	BOOL Register();
	BOOL Unregister();
	BOOL WriteSecureProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue);
	CString GetSecureProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault = NULL);
	BOOL m_bWin2000OrHigher;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRemoteCamViewerApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CRemoteCamViewerApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REMOTECAMVIEWER_H__F65CD5A3_7F3C_43D8_BE35_7690D364AC4D__INCLUDED_)
