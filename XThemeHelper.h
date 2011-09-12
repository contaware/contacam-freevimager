// XThemeHelper.h  Version 1.0
//
// Author:  Hans Dietrich
//          hdietrich@gmail.com
//
// This software is released into the public domain.  You are free to use
// it in any way you like, except that you may not sell this source code.
//
// This software is provided "as is" with no expressed or implied warranty.
// I accept no liability for any damage or loss of business that this
// software may cause.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef  XTHEMEHELPER_H
#define  XTHEMEHELPER_H

#include "uxtheme.h"
#if defined(NTDDI_VERSION) && defined(NTDDI_LONGHORN) && (NTDDI_VERSION >= NTDDI_LONGHORN)
#include "vssym32.h"
#else
#include "tmschema.h"
#endif


///////////////////////////////////////////////////////////////////////////////
//
// typedefs for uxtheme functions
//
typedef HRESULT	(__stdcall *PFNCLOSETHEMEDATA)(HTHEME hTheme);
typedef HRESULT	(__stdcall *PFNDRAWTHEMEBACKGROUND)(HTHEME hTheme, HDC hdc,
								int iPartId, int iStateId,
								const RECT *pRect,  const RECT *pClipRect);
typedef HRESULT	(__stdcall *PFNDRAWTHEMETEXT)(HTHEME hTheme, HDC hdc,
								int iPartId, int iStateId, LPCWSTR pszText,
								int iCharCount, DWORD dwTextFlags,
								DWORD dwTextFlags2, const RECT *pRect);
typedef HRESULT	(__stdcall *PFNGETTHEMEBACKGROUNDCONTENTRECT)(HTHEME hTheme,
								HDC hdc, int iPartId, int iStateId,
								const RECT *pBoundingRect,
								RECT *pContentRect);
typedef BOOL	(__stdcall *PFNISAPPTHEMED)();
typedef BOOL	(__stdcall *PFNISTHEMEACTIVE)();
typedef HTHEME	(__stdcall *PFNOPENTHEMEDATA)(HWND hwnd,
								LPCWSTR pszClassList);
typedef HRESULT (__stdcall *PFNDRAWTHEMEPARENTBACKGROUND) (	HWND hwnd,
															HDC hdc,
															RECT *prc);
typedef BOOL (__stdcall *PFNISTHEMEBACKGROUNDPARTIALLYTRANSPARENT) (HTHEME hTheme,
																	int iPartId,
																	int iStateId);
typedef HRESULT (__stdcall *PFNGETTHEMECOLOR)(	HTHEME hTheme, int iPartId, 
												int iStateId, int iPropId,
												OUT COLORREF *pColor);
typedef HRESULT (__stdcall *PFNGETTHEMESYSCOLOR)(HTHEME hTheme, int iColorID);

///////////////////////////////////////////////////////////////////////////////
//
// CXThemeHelper class definition
//
class CXThemeHelper
{
// Construction
public:
	CXThemeHelper();
	virtual ~CXThemeHelper();

// Attributes
public:
	// User can disable theming for app by right-clicking on exe,
	// then clicking on Properties | Compatibility | Disable visual themes
	BOOL IsAppThemed();
	BOOL IsThemeActive();
	BOOL IsThemeLibAvailable() {return m_bThemeLibLoaded;};
	BOOL IsThemeComCtl32() {return m_nDLLVersion >= 6;};

// Operations
public:
	BOOL	CloseThemeData(HTHEME hTheme);
	BOOL	DrawThemeBackground(HTHEME hTheme,
								HDC hdc,
								int iPartId,
								int iStateId,
								const RECT *pRect,
								const RECT *pClipRect);
	BOOL	DrawThemeParentBackground(	HWND hwnd,
										HDC hdc,
										RECT *prc);
	BOOL	IsThemeBackgroundPartiallyTransparent(	HTHEME hTheme,
													int iPartId,
													int iStateId);
	BOOL	DrawThemeText(HTHEME hTheme,
						  HDC hdc,
						  int iPartId,
						  int iStateId,
						  LPCTSTR lpszText,
						  DWORD dwTextFlags,
						  DWORD dwTextFlags2,
						  const RECT *pRect);
	BOOL	GetThemeBackgroundContentRect(HTHEME hTheme,
										  HDC hdc,
										  int iPartId,
										  int iStateId,
										  const RECT *pBoundingRect,
										  RECT *pContentRect);
	HTHEME	OpenThemeData(HWND hWnd, LPCTSTR lpszClassList);
	BOOL	GetThemeColor(	HTHEME hTheme, int iPartId, 
							int iStateId, int iPropId,
							OUT COLORREF *pColor);
	COLORREF GetThemeSysColor(	HTHEME hTheme,
								int iColorID);

// Implementation
private:
	static PFNCLOSETHEMEDATA				m_CloseThemeData;
	static PFNDRAWTHEMEBACKGROUND			m_DrawThemeBackground;
	static PFNDRAWTHEMEPARENTBACKGROUND		m_DrawThemeParentBackground;
	static PFNISTHEMEBACKGROUNDPARTIALLYTRANSPARENT	m_IsThemeBackgroundPartiallyTransparent;
	static PFNDRAWTHEMETEXT					m_DrawThemeText;
	static PFNGETTHEMEBACKGROUNDCONTENTRECT	m_GetThemeBackgroundContentRect;
	static PFNISAPPTHEMED					m_IsAppThemed;
	static PFNISTHEMEACTIVE					m_IsThemeActive;
	static PFNOPENTHEMEDATA					m_OpenThemeData;
	static PFNGETTHEMECOLOR					m_GetThemeColor;
	static PFNGETTHEMESYSCOLOR				m_GetThemeSysColor;
	static HMODULE							m_hThemeLib;
	static BOOL								m_bThemeLibLoaded;
	static int								m_nUseCount;
	static int								m_nDLLVersion;

	void	Init();
	int		GetComCtl32Version();
};

///////////////////////////////////////////////////////////////////////////////
//
// CXThemeHelper instance
//
#ifndef XTHEMEHELPER_CPP
// include an instance in each file;  the namespace insures uniqueness
namespace { CXThemeHelper ThemeHelper; }
#endif

#endif // XTHEMEHELPER_H
