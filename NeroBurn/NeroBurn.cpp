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
|* FILE: NeroBurn.cpp
|*
|* PURPOSE: Defines the class behaviors for the application.
******************************************************************************/


#include "stdafx.h"
#include "NeroBurn.h"
#include "NeroBurnDlg.h"
#include "atlconv.h"	// For T2CA, USES_CONVERSION

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNeroBurnApp

BEGIN_MESSAGE_MAP(CNeroBurnApp, CWinApp)
	//{{AFX_MSG_MAP(CNeroBurnApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNeroBurnApp construction

CNeroBurnApp::CNeroBurnApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CNeroBurnApp object

CNeroBurnApp theApp;

void CNeroBurnApp::CNeroBurnCommandLineInfo::ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast)
{
	if (bFlag)
	{
		USES_CONVERSION;
		ParseParamFlag(T2CA(pszParam));

		if (_tcscmp(pszParam, _T("dvd")) == 0) // Case sensitive!
			m_bDoBurnDVD = TRUE;
	}
	else
		ParseParamNotFlag(pszParam);

	ParseLast(bLast);
}

#ifdef _UNICODE
void CNeroBurnApp::CNeroBurnCommandLineInfo::ParseParam(const char* pszParam, BOOL bFlag, BOOL bLast)
{
	if (bFlag)
	{
		ParseParamFlag(pszParam);

		if (strcmp(pszParam, "dvd") == 0) // Case sensitive!
			m_bDoBurnDVD = TRUE;
	}
	else
		ParseParamNotFlag(pszParam);

	ParseLast(bLast);
}
#endif // UNICODE

/////////////////////////////////////////////////////////////////////////////
// CNeroBurnApp initialization

BOOL CNeroBurnApp::InitInstance()
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	// Create Named Mutex For Single Instance
	HANDLE hAppMutex = ::CreateMutex(NULL, FALSE, APPMUTEXNAME);
    switch (::GetLastError())
    {
		case ERROR_SUCCESS:
			// Mutex created successfully. There is
			// no instances running
			break;

		case ERROR_ALREADY_EXISTS:
			// Mutex already exists so there is a
			// running instance of our app.
			::CloseHandle(hAppMutex);
			return FALSE;

		default:
			// Failed to create mutex by unknown reason
			::CloseHandle(hAppMutex);
			return FALSE;
    }

	// Parse command line
	CNeroBurnCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	m_bDoBurnDVD = cmdInfo.DoBurnDVD();
	if (cmdInfo.m_strFileName == "")
	{
#ifdef _DEBUG
		m_sPathName = "C:\test";
#else
		::CloseHandle(hAppMutex);
		return FALSE;
#endif
	}
	else
		m_sPathName = cmdInfo.m_strFileName;

	CNeroBurnDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	::CloseHandle(hAppMutex);

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
