// XThemeHelper.cpp  Version 1.0
//
// Author:  Hans Dietrich
//          hdietrich@gmail.com
//
// Description:
//     XThemeHelper implements CXThemeHelper, a singleton helper class that
//     wraps the functions of UXTHEME.DLL.
//
// History
//     Version 1.0 - 2005 March 22
//     - Initial public release
//
// Public APIs:
//                  NAME                               DESCRIPTION
//     ------------------------------   -------------------------------------------
//     IsAppThemed()                    Reports whether the current application's 
//                                      user interface displays using visual styles.
//     IsThemeActive()                  Tests if a visual style for the current 
//                                      application is active.
//     IsThemeLibAvailable()            Test whether UXTHEME.DLL (and its functions) 
//                                      are accessible.
//     CloseThemeData()                 Closes the theme data handle.
//     DrawThemeBackground()            Draws the background image defined by the 
//                                      visual style for the specified control part.
//     DrawThemeText()                  Draws text using the color and font 
//                                      defined by the visual style.
//     GetThemeBackgroundContentRect()  Retrieves the size of the content area 
//                                      for the background defined by the visual 
//                                      style.
//     OpenThemeData()                  Opens the theme data for a window and 
//                                      its associated class.
// Oliver Pfister APIs:
//
// I added some more functions.
//
// License:
//     This software is released into the public domain.  You are free to use
//     it in any way you like, except that you may not sell this source code.
//
//     This software is provided "as is" with no expressed or implied warranty.
//     I accept no liability for any damage or loss of business that this
//     software may cause.
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#define XTHEMEHELPER_CPP
#include "XThemeHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Slass static data initialization
PFNCLOSETHEMEDATA					CXThemeHelper::m_CloseThemeData = NULL;
PFNDRAWTHEMEBACKGROUND				CXThemeHelper::m_DrawThemeBackground = NULL;
PFNDRAWTHEMEPARENTBACKGROUND		CXThemeHelper::m_DrawThemeParentBackground = NULL;
PFNISTHEMEBACKGROUNDPARTIALLYTRANSPARENT CXThemeHelper::m_IsThemeBackgroundPartiallyTransparent = NULL;
PFNDRAWTHEMETEXT					CXThemeHelper::m_DrawThemeText = NULL;
PFNGETTHEMEBACKGROUNDCONTENTRECT	CXThemeHelper::m_GetThemeBackgroundContentRect = NULL;
PFNISAPPTHEMED						CXThemeHelper::m_IsAppThemed = NULL;
PFNISTHEMEACTIVE					CXThemeHelper::m_IsThemeActive = NULL;
PFNOPENTHEMEDATA					CXThemeHelper::m_OpenThemeData = NULL;
PFNGETTHEMECOLOR					CXThemeHelper::m_GetThemeColor = NULL;
PFNGETTHEMESYSCOLOR					CXThemeHelper::m_GetThemeSysColor = NULL;
HMODULE 							CXThemeHelper::m_hThemeLib = NULL;
BOOL								CXThemeHelper::m_bThemeLibLoaded = FALSE;
int									CXThemeHelper::m_nUseCount = 0;
int									CXThemeHelper::m_nDLLVersion = -1;

CXThemeHelper::CXThemeHelper()
{
	m_nUseCount++;

	if (!m_bThemeLibLoaded)
	{
		Init();
	}
}

void CXThemeHelper::Init()
{
	m_hThemeLib = LoadLibrary(_T("UXTHEME.DLL"));

	if (m_hThemeLib)
	{
		m_CloseThemeData                = (PFNCLOSETHEMEDATA)GetProcAddress(m_hThemeLib,
																			"CloseThemeData");
		m_DrawThemeBackground           = (PFNDRAWTHEMEBACKGROUND)GetProcAddress(m_hThemeLib,
																			"DrawThemeBackground");
		m_DrawThemeText                 = (PFNDRAWTHEMETEXT)GetProcAddress(m_hThemeLib,
																			"DrawThemeText");
		m_GetThemeBackgroundContentRect = (PFNGETTHEMEBACKGROUNDCONTENTRECT)GetProcAddress(m_hThemeLib,
																			"GetThemeBackgroundContentRect");
		m_IsAppThemed                   = (PFNISAPPTHEMED)GetProcAddress(m_hThemeLib,
																			"IsAppThemed");
		m_IsThemeActive                 = (PFNISTHEMEACTIVE)GetProcAddress(m_hThemeLib,
																			"IsThemeActive");
		m_OpenThemeData                 = (PFNOPENTHEMEDATA)GetProcAddress(m_hThemeLib,
																			"OpenThemeData");
		m_DrawThemeParentBackground		= (PFNDRAWTHEMEPARENTBACKGROUND)GetProcAddress(m_hThemeLib,
																			"DrawThemeParentBackground");
		m_IsThemeBackgroundPartiallyTransparent = (PFNISTHEMEBACKGROUNDPARTIALLYTRANSPARENT)
																			GetProcAddress(m_hThemeLib,
																			"IsThemeBackgroundPartiallyTransparent");
		m_GetThemeColor					= (PFNGETTHEMECOLOR)GetProcAddress(	m_hThemeLib,
																			"GetThemeColor");
		m_GetThemeSysColor					= (PFNGETTHEMESYSCOLOR)GetProcAddress(	m_hThemeLib,
																					"GetThemeSysColor");

		if (m_CloseThemeData &&
			m_DrawThemeBackground &&
			m_DrawThemeText &&
			m_GetThemeBackgroundContentRect &&
			m_IsAppThemed &&
			m_IsThemeActive &&
			m_OpenThemeData &&
			m_DrawThemeParentBackground &&
			m_IsThemeBackgroundPartiallyTransparent &&
			m_GetThemeColor &&
			m_GetThemeSysColor)
		{
			m_bThemeLibLoaded = TRUE;
		}
		else
		{
			TRACE(_T("ERROR - failed to locate theme library function\n"));
			FreeLibrary(m_hThemeLib);
			m_hThemeLib = NULL;
		}
	}

	// Get Used DLL Version
	m_nDLLVersion = GetComCtl32Version();
}

// The new COMCTL32.DLL will only be available to applications that
// tell the operating system that the application has been tested
// and works with the new COMCTL32.DLL. The user-experience COMCTL32.DLL
// will not be installed in the SYSTEM32 directory.
// Instead, it will be installed as a managed assembly and access
// to the component will be via an application manifest.
// The new COMCTL32.DLL (version 6.0.0.0), and the old COMCTL32.DLL
// (version 5.81) will both ship with Windows XP and will be installed
// and run side by side with each other.
int CXThemeHelper::GetComCtl32Version()
{
	typedef struct _DllVersionInfo
	{
			DWORD cbSize;
			DWORD dwMajorVersion;                   // Major version
			DWORD dwMinorVersion;                   // Minor version
			DWORD dwBuildNumber;                    // Build number
			DWORD dwPlatformID;                     // DLLVER_PLATFORM_*
	} DLLVERSIONINFO;

	int ret = -1;

	typedef HRESULT (CALLBACK *DLLGETVERSION)(DLLVERSIONINFO*);
	DLLGETVERSION pDLLGETVERSION = NULL;
	HMODULE hModComCtl = ::LoadLibrary(_T("comctl32.dll"));
    if (hModComCtl)
    {
        pDLLGETVERSION = (DLLGETVERSION)(
			::GetProcAddress(hModComCtl, "DllGetVersion"));
        if (pDLLGETVERSION)
        {
            DLLVERSIONINFO dvi = {0};
            dvi.cbSize = sizeof dvi;
            if (pDLLGETVERSION(&dvi) == NOERROR)
            {
                ret = dvi.dwMajorVersion;
            }
        }
		::FreeLibrary(hModComCtl);                 
    }

	return ret;
}

CXThemeHelper::~CXThemeHelper()
{
	if (m_nUseCount > 0)
		m_nUseCount--;

	if (m_nUseCount == 0)
	{
		if (m_hThemeLib)
		{
			FreeLibrary(m_hThemeLib);
			m_hThemeLib = NULL;
		}
		m_bThemeLibLoaded = FALSE;
	}
}

BOOL CXThemeHelper::CloseThemeData(HTHEME hTheme)
{
	BOOL ok = FALSE;

	if (m_bThemeLibLoaded && hTheme)
	{
		HRESULT hr = m_CloseThemeData(hTheme);
		if (SUCCEEDED(hr))
			ok = TRUE;
	}

	return ok;
}

BOOL CXThemeHelper::DrawThemeBackground(HTHEME hTheme,
										HDC hdc,
										int iPartId,
										int iStateId,
										const RECT *pRect,
										const RECT *pClipRect)
{
	BOOL ok = FALSE;

	if (m_bThemeLibLoaded && hTheme)
	{
		HRESULT hr = m_DrawThemeBackground(hTheme, hdc, iPartId, iStateId, pRect,  pClipRect);
		if (SUCCEEDED(hr))
			ok = TRUE;
	}

	return ok;
}

BOOL CXThemeHelper::DrawThemeParentBackground(	HWND hwnd,
												HDC hdc,
												RECT *prc)
{
	BOOL ok = FALSE;

	if (m_bThemeLibLoaded)
	{
		HRESULT hr = m_DrawThemeParentBackground(hwnd, hdc, prc);
		if (SUCCEEDED(hr))
			ok = TRUE;
	}

	return ok;
}

BOOL CXThemeHelper::IsThemeBackgroundPartiallyTransparent(	HTHEME hTheme,
															int iPartId,
															int iStateId)
{
	if (m_bThemeLibLoaded && hTheme)
		return m_IsThemeBackgroundPartiallyTransparent(hTheme, iPartId, iStateId);
	else
		return FALSE;
}

BOOL CXThemeHelper::DrawThemeText(HTHEME hTheme,
								  HDC hdc,
								  int iPartId,
								  int iStateId,
								  LPCTSTR lpszText,
								  DWORD dwTextFlags,
								  DWORD dwTextFlags2,
								  const RECT *pRect)
{
	BOOL ok = FALSE;

	if (m_bThemeLibLoaded && hTheme)
	{
		HRESULT hr = S_OK;

#ifdef _UNICODE

		hr = m_DrawThemeText(hTheme, hdc, iPartId, iStateId,
						lpszText, wcslen(lpszText),
						dwTextFlags, dwTextFlags2, pRect);

#else

		int nLen = ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, lpszText, _tcslen(lpszText)+1, NULL, 0);
		WCHAR * pszWide = new WCHAR[nLen];
		if (pszWide)
		{
			::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, lpszText, _tcslen(lpszText)+1, pszWide, nLen);
			hr = m_DrawThemeText(hTheme, hdc, iPartId, iStateId,
							pszWide, wcslen(pszWide),
							dwTextFlags, dwTextFlags2, pRect);
			delete pszWide;
		}

#endif

		if (SUCCEEDED(hr))
			ok = TRUE;
	}

	return ok;
}

BOOL CXThemeHelper::GetThemeBackgroundContentRect(HTHEME hTheme,
												  HDC hdc,
												  int iPartId,
												  int iStateId,
												  const RECT *pBoundingRect,
												  RECT *pContentRect)
{
	BOOL ok = FALSE;

	if (m_bThemeLibLoaded && hTheme)
	{
		HRESULT hr = m_GetThemeBackgroundContentRect(hTheme, hdc, iPartId, 
							iStateId, pBoundingRect, pContentRect);
		if (SUCCEEDED(hr))
			ok = TRUE;
	}

	return ok;
}

// User can disable theming
// for app by right-clicking
// on exe, then clicking on
// Properties | Compatibility |
// Disable visual themes
BOOL CXThemeHelper::IsAppThemed()
{
	BOOL ok = FALSE;

	if (m_bThemeLibLoaded)
	{
		ok = m_IsAppThemed();
	}

	return ok;
}

BOOL CXThemeHelper::IsThemeActive()
{
	BOOL ok = FALSE;

	if (m_bThemeLibLoaded)
	{
		ok = m_IsThemeActive();
	}

	return ok;
}

HTHEME CXThemeHelper::OpenThemeData(HWND hWnd, LPCTSTR lpszClassList)
{
	HTHEME hTheme = NULL;

	if (m_bThemeLibLoaded)
	{

#ifdef _UNICODE

		hTheme = m_OpenThemeData(hWnd, lpszClassList);

#else
		int nLen = ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED,
										lpszClassList, 
										_tcslen(lpszClassList) + 1,
										NULL,
										0);
		WCHAR* pszWide = new WCHAR[nLen];
		if (pszWide)
		{
			::MultiByteToWideChar(	CP_ACP, MB_PRECOMPOSED,
									lpszClassList, 
									_tcslen(lpszClassList) + 1,
									pszWide,
									nLen);
			hTheme = m_OpenThemeData(hWnd, pszWide);
			delete pszWide;
		}

#endif

	}

	return hTheme;
}

BOOL CXThemeHelper::GetThemeColor(	HTHEME hTheme, int iPartId, 
									int iStateId, int iPropId,
									OUT COLORREF *pColor)
{
	BOOL ok = FALSE;

	if (m_bThemeLibLoaded && hTheme)
	{
		HRESULT hr = m_GetThemeColor(	hTheme,
										iPartId, 
										iStateId,
										iPropId,
										pColor);
		if (SUCCEEDED(hr))
			ok = TRUE;
	}

	return ok;
}

COLORREF CXThemeHelper::GetThemeSysColor(HTHEME hTheme,
										 int iColorID)
{
	if (m_bThemeLibLoaded && hTheme)
	{
		return m_GetThemeSysColor(	hTheme,
									iColorID);
	}
	else
		return ::GetSysColor(iColorID);
}


