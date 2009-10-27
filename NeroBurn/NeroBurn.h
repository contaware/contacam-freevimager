/******************************************************************************
|* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
|* ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
|* THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
|* PARTICULAR PURPOSE.
|* 
|* Copyright 1995-2005 Nero AG. All Rights Reserved.
|*-----------------------------------------------------------------------------
|* PROJECT: NeroBurn NeroAPI Example
|*
|* FILE: NeroBurn.h
|*
|* PURPOSE: Declaration file for the application.
******************************************************************************/

// NeroBurn.h : main header file for the NeroBurn application
//

#if !defined(AFX_NEROBURN_H__78BE85B7_87B6_11D4_8056_00105ABB05E6__INCLUDED_)
#define AFX_NEROBURN_H__78BE85B7_87B6_11D4_8056_00105ABB05E6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

#define APPMUTEXNAME		_T("NeroBurnSInstAppMutex")

/////////////////////////////////////////////////////////////////////////////
// CNeroBurnApp:
// See NeroBurn.cpp for the implementation of this class
//

class CNeroBurnApp : public CWinApp
{
public:
	// Switche(s):
	//
	// /dvd	-> burn dvd instead of cd
	class CNeroBurnCommandLineInfo : public CCommandLineInfo
	{
		public:
			CNeroBurnCommandLineInfo() {m_bDoBurnDVD = FALSE;};
			~CNeroBurnCommandLineInfo() {;};
			
			//plain char* version on UNICODE for source-code backwards compatibility
			virtual void ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast);
#ifdef _UNICODE
			virtual void ParseParam(const char* pszParam, BOOL bFlag, BOOL bLast);
#endif

			BOOL DoBurnDVD() {return m_bDoBurnDVD;};

		protected:
			BOOL m_bDoBurnDVD;
	};

public:
	CNeroBurnApp();
	CString m_sPathName;
	BOOL m_bDoBurnDVD;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNeroBurnApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CNeroBurnApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEROBURN_H__78BE85B7_87B6_11D4_8056_00105ABB05E6__INCLUDED_)
