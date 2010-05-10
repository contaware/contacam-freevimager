#include "stdafx.h"
#include "DxDraw.h"
#include "getdxver.h"
#include "resource.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#pragma comment(lib, "dxguid.lib")

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDxDraw::CDxDraw()
{
	m_bInit = FALSE;
	::ZeroMemory(&m_guidNull, sizeof(GUID));
	m_nCurrentDevice = 0;
	m_nSrcWidth = 0;
	m_nSrcHeight = 0;
	m_bFontTransparent = FALSE;
	m_FontTableSize.cx = 0;
	m_FontTableSize.cy = 0;
	m_FontSize.cx = 0;
	m_FontSize.cy = 0;
	m_hWnd = NULL;
	m_bFullScreen = FALSE;
	m_bTripleBuffering = FALSE;
	m_bDisplayModeChanged = FALSE;
	m_hDirectDraw = ::LoadLibrary(_T("ddraw.dll"));
	m_cs.EnableTimeout();

	if (m_hDirectDraw)
	{
		// Enumerate Screens
		if (!EnumerateScreens())
		{
			TRACE(_T("Failed while Enumerating Screens\n"));
			m_bDx7 = FALSE;
		}
		else
		{
			// Verify the Presence of DirectDraw 7 or Higher
			LPDIRECTDRAWCREATEEX pDirectDrawCreateEx = (LPDIRECTDRAWCREATEEX)::GetProcAddress(m_hDirectDraw, "DirectDrawCreateEx");
			if (pDirectDrawCreateEx)
			{
				LPDIRECTDRAW7 pDD;
				HRESULT hRet = pDirectDrawCreateEx(	NULL,
													(void**)(&pDD),
													IID_IDirectDraw7,
													NULL);
				if (Error(hRet, _T("Failed to create the DirectDraw7 Object")))
					m_bDx7 = FALSE;
				else
				{
					pDD->Release();
					m_bDx7 = TRUE;
				}
			}
			else
				m_bDx7 = FALSE;
		}
	}
	else
		m_bDx7 = FALSE;
}

CDxDraw::~CDxDraw()
{
	Free();
	DeleteScreenArray();
	if (m_hDirectDraw)
		::FreeLibrary(m_hDirectDraw);
}

void WINAPI GetMonitorRect(HMONITOR hMonitor, RECT& rcMonitor)
{
	FPGETMONITORINFO fpGetMonitorInfo;
	MONITORINFO monInfo;
	monInfo.cbSize = sizeof(MONITORINFO);
	HINSTANCE h = ::LoadLibrary(_T("user32.dll"));
	if (!h)
	{
		rcMonitor.left = 0;
		rcMonitor.top = 0;
		rcMonitor.right = ::GetSystemMetrics(SM_CXSCREEN);
		rcMonitor.bottom = ::GetSystemMetrics(SM_CYSCREEN);
		return;
	}
#ifdef _UNICODE
	fpGetMonitorInfo = (FPGETMONITORINFO)::GetProcAddress(h, "GetMonitorInfoW");
#else
	fpGetMonitorInfo = (FPGETMONITORINFO)::GetProcAddress(h, "GetMonitorInfoA");
#endif
	if (fpGetMonitorInfo)
	{
		if (fpGetMonitorInfo(hMonitor, &monInfo) == 0)
		{
			rcMonitor.left = 0;
			rcMonitor.top = 0;
			rcMonitor.right = ::GetSystemMetrics(SM_CXSCREEN);
			rcMonitor.bottom = ::GetSystemMetrics(SM_CYSCREEN);
		}
		else
			rcMonitor = monInfo.rcMonitor;
	}
	else
	{
		rcMonitor.left = 0;
		rcMonitor.top = 0;
		rcMonitor.right = ::GetSystemMetrics(SM_CXSCREEN);
		rcMonitor.bottom = ::GetSystemMetrics(SM_CYSCREEN);
	}

	::FreeLibrary(h);
}

// Note:
// For Single Monitor Systems this callback is called once
// with guid NULL.
// For a two monitor system, it is called 3x times,
// first time with guid NULL, second time for primary monitor
// with a specific guid and third time for the secondary monitor
// with a specific guid
// -> For multi-monitor the first call is redundant, remove it from
// the monitor list! 
BOOL WINAPI DDEnumCallbackExA(GUID* pGuid, LPSTR pszDesc, LPSTR pszDriverName,
                              VOID* pContext, HMONITOR hmon)
{
	CDxDraw* p = (CDxDraw*)pContext;
	CDxDraw::CScreen* pScreen = new CDxDraw::CScreen;
	if (!pScreen)
		return FALSE;
	
    if (pGuid == NULL)
        pScreen->m_guid = pScreen->m_guidNull;
    else
        pScreen->m_guid = *pGuid;
#ifdef _UNICODE
	mbstowcs(pScreen->m_szDesc, pszDesc, MAX_DD7_DEVICE_DESCRIPTION_SIZE);
#else
    _tcsncpy(pScreen->m_szDesc, pszDesc, MAX_DD7_DEVICE_DESCRIPTION_SIZE);
#endif
    pScreen->m_hMonitor = hmon;
	::GetMonitorRect(hmon, pScreen->m_rcMonitor);

	p->m_ScreenArray.Add(pScreen);

    return TRUE; // Keep enumerating
}

BOOL WINAPI DDEnumCallbackA(GUID* pGuid, LPSTR pszDesc, LPSTR pszDriverName, 
                            VOID* pContext)
{
    DDEnumCallbackExA(pGuid, pszDesc, pszDriverName, pContext, NULL);
    return FALSE; // Stop enumerating -- For this sample, we don't want any non-display devices
}

/*
Microsoft DirectDraw 7.0 SDK Errata File September 2002

(c) Microsoft Corporation, 2002. All rights reserved.

DirectDrawEnumerateEx does not support Unicode. 
*/
BOOL CDxDraw::EnumerateScreens()
{
    HRESULT hRet;
    LPDIRECTDRAWENUMERATEEXA pDirectDrawEnumerateExA = (LPDIRECTDRAWENUMERATEEXA)::GetProcAddress(m_hDirectDraw, "DirectDrawEnumerateExA");
    if (pDirectDrawEnumerateExA == NULL)
    {
        // We must be running on an old version of DirectDraw.
        // Therefore MultiMon isn't supported. Fall back on
        // DirectDrawEnumerate to enumerate standard devices on a 
        // single-monitor system.
		LPDIRECTDRAWENUMERATEA DirectDrawEnumerateA = (LPDIRECTDRAWENUMERATEA)::GetProcAddress(m_hDirectDraw, "DirectDrawEnumerateA");
        if (DirectDrawEnumerateA)
		{
			hRet = DirectDrawEnumerateA(DDEnumCallbackA, this);
			Error(hRet, _T("DirectDrawEnumerate() Failed"));
		}
		else
			return FALSE;
    }
    else
	{
        hRet = pDirectDrawEnumerateExA(DDEnumCallbackExA, this, DDENUM_ATTACHEDSECONDARYDEVICES);
		Error(hRet, _T("DirectDrawEnumerateEx() Failed")); 
	}

    return SUCCEEDED(hRet);
}

HRESULT WINAPI EnumCurrentModesCallback(LPDDSURFACEDESC2 pddsd, VOID* pContext)
{
	CDxDraw* p = (CDxDraw*)pContext;
	if ((int)pddsd->dwWidth >= p->GetSrcWidth() &&
		(int)pddsd->dwHeight >= p->GetSrcHeight())
	{
		HRESULT hRet = p->m_ScreenArray[p->GetCurrentDevice()]->m_pDD->SetDisplayMode(
																pddsd->dwWidth, 
																pddsd->dwHeight,
																0, 0, 0);
		if (!p->Error(hRet, _T("EnumCurrentModesCallback() Failed")))
		{
			// Update New Monitor Rect
			::GetMonitorRect(	p->m_ScreenArray[p->GetCurrentDevice()]->m_hMonitor,
								p->m_ScreenArray[p->GetCurrentDevice()]->m_rcMonitor);
		}
		return DDENUMRET_CANCEL;
	}
	else
		return DDENUMRET_OK;
}

BOOL CDxDraw::SetSrcClosestDisplayMode()
{
	HRESULT hRet = m_ScreenArray[m_nCurrentDevice]->m_pDD->EnumDisplayModes(0,
																			NULL,
																			LPVOID(this),
																			(LPDDENUMMODESCALLBACK2)EnumCurrentModesCallback);
	if (Error(hRet, _T("SetSrcClosestDisplayMode() Failed")))
		return FALSE;
	else
	{
		m_bDisplayModeChanged = TRUE;
		return TRUE;
	}
}

BOOL CDxDraw::RestoreDisplayMode()
{
	if ((m_ScreenArray.GetSize() > 0) && m_bFullScreen && m_bDisplayModeChanged)
	{
		HRESULT hRet = m_ScreenArray[m_nCurrentDevice]->m_pDD->RestoreDisplayMode();
		if (Error(hRet, _T("RestoreDisplayMode() Failed")))
			return FALSE;
		else
		{
			m_bDisplayModeChanged = FALSE;
			return TRUE;
		}
	}
	else
		return FALSE;
}

// DirectDraw Initialization stuff
BOOL CDxDraw::Init(	HWND hWnd,
					int nSrcWidth,
					int nSrcHeight,
					DWORD dwSrcFourCC,
					UINT uiFontTableID)
{
	HRESULT hRet;
	DDSURFACEDESC2 ddsd;
	BOOL bFlipUV = FALSE;

	// Clean-up
	Free();

	// Check
	if (!m_bDx7)
		return FALSE;

	// Check
	if (hWnd == NULL)
		return FALSE;

	// Check
    if (m_hDirectDraw == NULL)
        return FALSE;

	// Set to BI_RGB
	if (dwSrcFourCC == BI_BITFIELDS	||
		dwSrcFourCC == BI_RLE4		||
		dwSrcFourCC == FCC('RLE4')	||
		dwSrcFourCC == BI_RLE8		||
		dwSrcFourCC == FCC('RLE8'))
		dwSrcFourCC = BI_RGB;

	// YUY2 Equivalents
	if (dwSrcFourCC == FCC('YUNV')	||
		dwSrcFourCC == FCC('VYUY')	||
		dwSrcFourCC == FCC('V422')	||
		dwSrcFourCC == FCC('YUYV'))
		dwSrcFourCC = FCC('YUY2');

	// Set To YV12 and set the flip U <-> V flag
	if (dwSrcFourCC == FCC('I420') ||
		dwSrcFourCC == FCC('IYUV'))
	{
		dwSrcFourCC = FCC('YV12');
		bFlipUV = TRUE;
	}

	// Final Check
	// Note: UYVY seems to be supported by some
	// graphic cards (this function succeeds)
	// but only junk is displayed...
	// do not supported it!
	if (dwSrcFourCC != BI_RGB		&&
		dwSrcFourCC != FCC('YV12')	&&
		dwSrcFourCC != FCC('YUY2'))
		return FALSE;

	// Handle to Window
	m_hWnd = hWnd;

	// Source Size
	m_nSrcWidth = nSrcWidth;
	m_nSrcHeight = nSrcHeight;

	// Reset the Full-Screen Mode Flag
	m_bFullScreen = FALSE;

	// Load The Font Table
	LoadFontDib(uiFontTableID);

	// Create DirectDraw Objects and Surfaces for
	// the NULL device and for each monitor:
	for (m_nCurrentDevice = 0 ; m_nCurrentDevice < m_ScreenArray.GetSize() ; m_nCurrentDevice++)
	{
		// Create Direct Draw
		// There are some MultiMon behaviors that will vary depending
		// on whether the application specifies a GUID or NULL during
		// DirectDraw object creation. The most noteable one being that
		// if the application specifies NULL, DirectDraw always clips
		// the cursor to the primary monitor.
		LPDIRECTDRAWCREATEEX pDirectDrawCreateEx = (LPDIRECTDRAWCREATEEX)::GetProcAddress(m_hDirectDraw, "DirectDrawCreateEx");
		if (pDirectDrawCreateEx == NULL)
			return FALSE;
		hRet = pDirectDrawCreateEx(&(m_ScreenArray[m_nCurrentDevice]->m_guid),
									(void**)&(m_ScreenArray[m_nCurrentDevice]->m_pDD),
									IID_IDirectDraw7,
									NULL);
		if (Error(hRet, _T("Failed to create the DirectDraw7 Object")))
			return FALSE;

		// Check whether Blt() can color space convert
		DDCAPS ddcaps;
		memset(&ddcaps, 0, sizeof(DDCAPS));
		ddcaps.dwSize = sizeof(DDCAPS);
		hRet = m_ScreenArray[m_nCurrentDevice]->m_pDD->GetCaps(&ddcaps, NULL);
		if (Error(hRet, _T("GetCaps Failed")))
		{
			Free();
			return FALSE;
		}
		if ((dwSrcFourCC != BI_RGB) && !(ddcaps.dwCaps & DDCAPS_BLTFOURCC))
		{	
			Free();
			return FALSE;
		}

		// Set DDSCL_NORMAL to use windowed mode
		hRet = m_ScreenArray[m_nCurrentDevice]->m_pDD->SetCooperativeLevel(m_hWnd, DDSCL_NORMAL | DDSCL_MULTITHREADED);
		if (Error(hRet, _T("SetCooperativeLevel Windowed Failed")))
		{
			Free();
			return FALSE;
		}
    
		// Create the Front surface
		::ZeroMemory(&ddsd, sizeof(ddsd)); 
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags        = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
		hRet = m_ScreenArray[m_nCurrentDevice]->m_pDD->CreateSurface(&ddsd, &(m_ScreenArray[m_nCurrentDevice]->m_pFrontBuffer), NULL);
		if (Error(hRet, _T("Failed to create the Primary Surface")))
		{
			Free();
			return FALSE;		
		}

		// Get Front surface description
		::ZeroMemory(&ddsd, sizeof(ddsd)); 
		ddsd.dwSize = sizeof(ddsd);
		hRet = m_ScreenArray[m_nCurrentDevice]->m_pFrontBuffer->GetSurfaceDesc(&ddsd);
		if (Error(hRet, _T("Failed to get Primary Surface Description")))
		{
			Free();
			return FALSE;		
		}
		m_ScreenArray[m_nCurrentDevice]->m_nMonitorBpp = ddsd.ddpfPixelFormat.dwRGBBitCount;

		// Create the Off-Screen back surface(s)
		// By default, DirectDraw creates a surface in display memory unless it will not fit,
		// in which case it creates the surface in system memory.
		// You can explicitly choose display or system memory by including the DDSCAPS_SYSTEMMEMORY or
		// DDSCAPS_VIDEOMEMORY flags in the dwCaps member of the DDSCAPS2 structure.
		// The method fails, returning an error, if it can't create the surface in the specified location.
		if (dwSrcFourCC == BI_RGB)
		{
			// Back Off-screen (the surface with the pixel format of the primary surface)
			::ZeroMemory(&ddsd, sizeof(ddsd)); 
			ddsd.dwSize = sizeof(ddsd);
			ddsd.dwFlags        = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
			ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
			ddsd.dwWidth = m_nSrcWidth;
			ddsd.dwHeight = m_nSrcHeight;
			hRet = m_ScreenArray[m_nCurrentDevice]->m_pDD->CreateSurface(&ddsd, &(m_ScreenArray[m_nCurrentDevice]->m_pBackBuffer), NULL);
			if (Error(hRet, _T("Failed to create the Back Surface")))
			{
				Free();
				return FALSE;		
			}

			// Get RGB surface description
			::ZeroMemory(&ddsd, sizeof(ddsd)); 
			ddsd.dwSize = sizeof(ddsd);
			hRet = m_ScreenArray[m_nCurrentDevice]->m_pBackBuffer->GetSurfaceDesc(&ddsd);
			if (Error(hRet, _T("Failed to get Back Surface Description")))
			{
				Free();
				return FALSE;		
			}
			m_ScreenArray[m_nCurrentDevice]->m_nSrcBpp = ddsd.ddpfPixelFormat.dwRGBBitCount;
			m_ScreenArray[m_nCurrentDevice]->m_nSrcPitch = ddsd.lPitch;
			m_ScreenArray[m_nCurrentDevice]->m_nSrcMinPitch = DWALIGNEDWIDTHBYTES(	ddsd.ddpfPixelFormat.dwRGBBitCount *
																					m_nSrcWidth);
			m_ScreenArray[m_nCurrentDevice]->m_bSameSrcPitch =	m_ScreenArray[m_nCurrentDevice]->m_nSrcPitch ==
																m_ScreenArray[m_nCurrentDevice]->m_nSrcMinPitch;
			m_ScreenArray[m_nCurrentDevice]->m_dwSrcFourCC = BI_RGB;
			m_ScreenArray[m_nCurrentDevice]->m_bFlipUV = FALSE;
			if (ddsd.ddpfPixelFormat.dwRGBBitCount == 16)
				m_ScreenArray[m_nCurrentDevice]->m_bRgb15 = (ddsd.ddpfPixelFormat.dwGBitMask == 0x03E0); // RGB16 555 ?
			else
				m_ScreenArray[m_nCurrentDevice]->m_bRgb15 = FALSE;
		}
		else
		{
			// RGB Back Off-screen
			::ZeroMemory(&ddsd, sizeof(ddsd)); 
			ddsd.dwSize = sizeof(ddsd);
			ddsd.dwFlags        = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
			ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
			ddsd.dwWidth = m_nSrcWidth;
			ddsd.dwHeight = m_nSrcHeight;
			hRet = m_ScreenArray[m_nCurrentDevice]->m_pDD->CreateSurface(&ddsd, &(m_ScreenArray[m_nCurrentDevice]->m_pBackBuffer), NULL);
			if (Error(hRet, _T("Failed to create the Back Surface")))
			{
				Free();
				return FALSE;		
			}

			// YUV Off-screen
			::ZeroMemory(&ddsd, sizeof(ddsd)); 
			ddsd.dwSize = sizeof(ddsd);
			ddsd.dwFlags        = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
			ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
			ddsd.dwWidth = m_nSrcWidth;
			ddsd.dwHeight = m_nSrcHeight;
			ddsd.ddpfPixelFormat.dwSize  = sizeof(DDPIXELFORMAT);
			ddsd.ddpfPixelFormat.dwFlags = DDPF_FOURCC;
			ddsd.ddpfPixelFormat.dwFourCC = dwSrcFourCC;
			hRet = m_ScreenArray[m_nCurrentDevice]->m_pDD->CreateSurface(&ddsd, &(m_ScreenArray[m_nCurrentDevice]->m_pOffscreenBuffer), NULL);
			if (Error(hRet, _T("Failed to create the Off-Screen Surface")))
			{
				Free();
				return FALSE;		
			}

			// Get YUV surface description
			::ZeroMemory(&ddsd, sizeof(ddsd)); 
			ddsd.dwSize = sizeof(ddsd);
			hRet = m_ScreenArray[m_nCurrentDevice]->m_pOffscreenBuffer->GetSurfaceDesc(&ddsd);
			if (Error(hRet, _T("Failed to get Off-Screen Surface Description")))
			{
				Free();
				return FALSE;		
			}
			m_ScreenArray[m_nCurrentDevice]->m_nSrcBpp = ddsd.ddpfPixelFormat.dwYUVBitCount;
			m_ScreenArray[m_nCurrentDevice]->m_nSrcPitch = ddsd.lPitch;
			m_ScreenArray[m_nCurrentDevice]->m_nSrcMinPitch = ::CalcYUVStride(	ddsd.ddpfPixelFormat.dwFourCC,
																				m_nSrcWidth);
			m_ScreenArray[m_nCurrentDevice]->m_bSameSrcPitch =	m_ScreenArray[m_nCurrentDevice]->m_nSrcPitch ==
															m_ScreenArray[m_nCurrentDevice]->m_nSrcMinPitch;
			m_ScreenArray[m_nCurrentDevice]->m_dwSrcFourCC = ddsd.ddpfPixelFormat.dwFourCC;
			m_ScreenArray[m_nCurrentDevice]->m_bFlipUV = bFlipUV;
			m_ScreenArray[m_nCurrentDevice]->m_bRgb15 = FALSE;
		}

		// Create the Off-Screen Font Surface
		if (m_FontDib.IsValid())
		{
			::ZeroMemory(&ddsd, sizeof(ddsd)); 
			ddsd.dwSize = sizeof(ddsd);
			
			// RGB Font Table
			if (m_bFontTransparent)
				ddsd.dwFlags        = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_CKSRCBLT;
			else
				ddsd.dwFlags        = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
			ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
			ddsd.dwWidth = m_FontTableSize.cx;
			ddsd.dwHeight = m_FontTableSize.cy;
			hRet = m_ScreenArray[m_nCurrentDevice]->m_pDD->CreateSurface(&ddsd, &(m_ScreenArray[m_nCurrentDevice]->m_pFontBuffer), NULL);
			if (Error(hRet, _T("Failed to create the Off-Screen Font Surface")))
			{
				Free();
				return FALSE;		
			}

			// Copy Font Dib to Font Off-Screen Surface
			if (!CopyFontDib())
			{
				Free();
				return FALSE;
			}

			// Set Source Color Key
			if (m_bFontTransparent)
			{
				// Set the source color key for the surface,
				// the pixel in the upper-left corner of the source is used.
				hRet = SetSrcColorKey(m_ScreenArray[m_nCurrentDevice]->m_pFontBuffer, CLR_INVALID);
				if (Error(hRet, _T("Failed to set the color key for the transparent font")))
				{
					Free();
					return FALSE;		
				}
			}
		}

		// Create Clipper
		hRet = m_ScreenArray[m_nCurrentDevice]->m_pDD->CreateClipper(0, &(m_ScreenArray[m_nCurrentDevice]->m_pClipper), NULL);
		if (Error(hRet, _T("Failed to create the Clipper")))
		{
			Free();
			return FALSE;		
		}

		// Associate the m_hWnd to the Clipper
		hRet = m_ScreenArray[m_nCurrentDevice]->m_pClipper->SetHWnd(0, m_hWnd);
		if (Error(hRet, _T("Failed associating the Window to the Clipper")))
		{
			Free();
			return FALSE;		
		}

		// Associate the Clipper to the Front Buffer
		hRet = m_ScreenArray[m_nCurrentDevice]->m_pFrontBuffer->SetClipper(m_ScreenArray[m_nCurrentDevice]->m_pClipper);
		if (Error(hRet, _T("Failed to associate the Clipper with the Primary Surface")))
		{
			Free();
			return FALSE;		
		}
	}

	// Init the m_nCurrentDevice variable to the current monitor
	UpdateCurrentDevice();

	// Flip To GDI Surface
	FlipCurrentToGDISurface();

	// Set init flag
	m_bInit = TRUE;

	return TRUE;
}

// DirectDraw Full-Screen Initialization stuff,
// Note: Full-screen exclusive mode applications should
// use the WS_EX_TOPMOST extended window style and the WS_VISIBLE
// window style to display properly.
// Use SetWindowPos() to add/remove the WS_EX_TOPMOST attribute.
// These styles keep the application at the front of the window z-order
// and prevent GDI from drawing on the primary surface.
BOOL CDxDraw::InitFullScreen(	HWND hWnd,
								int nSrcWidth,
								int nSrcHeight,
								BOOL bAdaptResolution,
								DWORD dwSrcFourCC,
								UINT uiFontTableID)
{
	HRESULT hRet;
	DDSURFACEDESC2 ddsd;
	BOOL bFlipUV = FALSE;

	// Clean-up
	Free();

	// Check
	if (!m_bDx7)
		return FALSE;

	// Check
	if (hWnd == NULL)
		return FALSE;

	// Check
    if (m_hDirectDraw == NULL)
        return FALSE;

	// Set to BI_RGB
	if (dwSrcFourCC == BI_BITFIELDS	||
		dwSrcFourCC == BI_RLE4		||
		dwSrcFourCC == FCC('RLE4')	||
		dwSrcFourCC == BI_RLE8		||
		dwSrcFourCC == FCC('RLE8'))
		dwSrcFourCC = BI_RGB;

	// YUY2 Equivalents
	if (dwSrcFourCC == FCC('YUNV')	||
		dwSrcFourCC == FCC('VYUY')	||
		dwSrcFourCC == FCC('V422')	||
		dwSrcFourCC == FCC('YUYV'))
		dwSrcFourCC = FCC('YUY2');

	// Set To YV12 and set the flip U <-> V flag
	if (dwSrcFourCC == FCC('I420') ||
		dwSrcFourCC == FCC('IYUV'))
	{
		dwSrcFourCC = FCC('YV12');
		bFlipUV = TRUE;
	}

	// Final Check
	// Note: UYVY seems to be supported by some
	// graphic cards (this function succeeds)
	// but only junk is displayed...
	// do not supported it!
	if (dwSrcFourCC != BI_RGB		&&
		dwSrcFourCC != FCC('YV12')	&&
		dwSrcFourCC != FCC('YUY2'))
		return FALSE;

	// Handle to Window
	m_hWnd = hWnd;

	// Source Width and Height
	m_nSrcWidth = nSrcWidth;
	m_nSrcHeight = nSrcHeight;

	// Init the m_nCurrentDevice variable
	UpdateCurrentDevice();

	// Set the Full-Screen Mode Flag
	m_bFullScreen = TRUE;

	// Load The Font Table
	LoadFontDib(uiFontTableID);

	// Create DirectDraw Object
	LPDIRECTDRAWCREATEEX pDirectDrawCreateEx = (LPDIRECTDRAWCREATEEX)::GetProcAddress(m_hDirectDraw, "DirectDrawCreateEx");
	if (pDirectDrawCreateEx == NULL)
		return FALSE;
	hRet = pDirectDrawCreateEx(&(m_ScreenArray[m_nCurrentDevice]->m_guid),
								(void**)&(m_ScreenArray[m_nCurrentDevice]->m_pDD),
								IID_IDirectDraw7,
								NULL);
	if (Error(hRet, _T("Failed to create the DirectDraw7 Object")))
		return FALSE;

	// Check whether Blt() can color space convert
	DDCAPS ddcaps;
	memset(&ddcaps, 0, sizeof(DDCAPS));
	ddcaps.dwSize = sizeof(DDCAPS);
	hRet = m_ScreenArray[m_nCurrentDevice]->m_pDD->GetCaps(&ddcaps, NULL);
	if (Error(hRet, _T("GetCaps Failed")))
	{
		Free();
		return FALSE;
	}
	if ((dwSrcFourCC != BI_RGB) && !(ddcaps.dwCaps & DDCAPS_BLTFOURCC))
	{	
		Free();
		return FALSE;
	}

	// Change Display Resolution to match as close
	// as possible nSrcWidth and nSrcHeight?
	if (bAdaptResolution)
		SetSrcClosestDisplayMode();

	// Set DDSCL_EXCLUSIVE  | DDSCL_FULLSCREEN for full-screen mode
	hRet = m_ScreenArray[m_nCurrentDevice]->m_pDD->SetCooperativeLevel(m_hWnd,	DDSCL_EXCLUSIVE				|
																				DDSCL_FULLSCREEN			|
																				DDSCL_MULTITHREADED			|
																				/*DDSCL_NOWINDOWCHANGES		|*/
																				DDSCL_ALLOWREBOOT);
	if (Error(hRet, _T("SetCooperativeLevel Windowed Failed")))
	{
		Free();
		return FALSE;
	}

	// Create the Front Surface
	::ZeroMemory(&ddsd, sizeof(ddsd)); 
	ddsd.dwSize = sizeof(ddsd);
	DWORD dwTotal, dwFree;
	if (GetCurrentVideoMem(dwTotal, dwFree))
	{
		// In Bytes assuming the worst case of 32 bpp
		DWORD dwFullScreenSurfaceSize = (DWORD)((m_ScreenArray[m_nCurrentDevice]->m_rcMonitor.right - 
												m_ScreenArray[m_nCurrentDevice]->m_rcMonitor.left) *
												(m_ScreenArray[m_nCurrentDevice]->m_rcMonitor.bottom -
												m_ScreenArray[m_nCurrentDevice]->m_rcMonitor.top) * 4);
		DWORD dwOffScreenSurfaceSize = (DWORD)(m_nSrcWidth * m_nSrcHeight * 4);
		DWORD dwFontSurfaceSize = (DWORD)(m_FontTableSize.cx * m_FontTableSize.cy * 4);

		// We need 5 surfaces for triple-buffering:
		// 1 x front, 2 x back, 1 x offscreen, 1 x font
		// Add a margin of 1 more full-screen surface!
		if (dwFree >= (4 * dwFullScreenSurfaceSize + dwOffScreenSurfaceSize + dwFontSurfaceSize))
			m_bTripleBuffering = TRUE;
		else
			m_bTripleBuffering = FALSE;
	}
	else
		m_bTripleBuffering = FALSE;
	if (m_bTripleBuffering)
		ddsd.dwBackBufferCount	= 2;
	else
		ddsd.dwBackBufferCount	= 1;
	ddsd.dwFlags				= DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
	ddsd.ddsCaps.dwCaps			= DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
	hRet = m_ScreenArray[m_nCurrentDevice]->m_pDD->CreateSurface(&ddsd, &(m_ScreenArray[m_nCurrentDevice]->m_pFrontBuffer), NULL);
	if (Error(hRet, _T("Failed to create the Primary Surface")))
	{
		Free();
		return FALSE;		
	}

	// Get Front surface description
	::ZeroMemory(&ddsd, sizeof(ddsd)); 
	ddsd.dwSize = sizeof(ddsd);
	hRet = m_ScreenArray[m_nCurrentDevice]->m_pFrontBuffer->GetSurfaceDesc(&ddsd);
	if (Error(hRet, _T("Failed to get Primary Surface Description")))
	{
		Free();
		return FALSE;		
	}
	m_ScreenArray[m_nCurrentDevice]->m_nMonitorBpp = ddsd.ddpfPixelFormat.dwRGBBitCount;

	// Get the Attached Back Surface
	DDSCAPS2 ddscaps;
	::ZeroMemory(&ddscaps, sizeof(DDSCAPS2));
	ddscaps.dwCaps = DDSCAPS_BACKBUFFER; 
	hRet = m_ScreenArray[m_nCurrentDevice]->m_pFrontBuffer->GetAttachedSurface(&ddscaps, &(m_ScreenArray[m_nCurrentDevice]->m_pBackBuffer)); 
	if (Error(hRet, _T("Failed to get the Back Surface")))
	{
		Free();
		return FALSE;		
	}

	// Create the Off-Screen buffer used to stretch
	if (!FullScreenCreateOffscreen(	nSrcWidth,
									nSrcHeight,
									dwSrcFourCC,
									bFlipUV))
		return FALSE;

	// Create Clipper
	// Clipper is needed so that the Blt Works Well!!!
	hRet = m_ScreenArray[m_nCurrentDevice]->m_pDD->CreateClipper(0, &(m_ScreenArray[m_nCurrentDevice]->m_pClipper), NULL);
	if (Error(hRet, _T("Failed to create the Clipper")))
	{
		Free();
		return FALSE;		
	}

	// Associate the m_hWnd to the Clipper
	hRet = m_ScreenArray[m_nCurrentDevice]->m_pClipper->SetHWnd(0, m_hWnd);
	if (Error(hRet, _T("Failed associating the Window to the Clipper")))
	{
		Free();
		return FALSE;		
	}

	// Flip To GDI Surface
	FlipCurrentToGDISurface();

	// Set Clipper to Front Buffer
	hRet = m_ScreenArray[m_nCurrentDevice]->m_pFrontBuffer->SetClipper(m_ScreenArray[m_nCurrentDevice]->m_pClipper);
	if (Error(hRet, _T("Failed to set the Clipper to the Primary Surface")))
	{
		Free();
		return FALSE;		
	}

	// Set init flag
	m_bInit = TRUE;

	return TRUE;
}

BOOL CDxDraw::FullScreenCreateOffscreen(int nSrcWidth,
										int nSrcHeight,
										DWORD dwSrcFourCC,
										BOOL bFlipUV)
{
	HRESULT hRet;
	DDSURFACEDESC2 ddsd;

	// Clean-up
	if (m_ScreenArray[m_nCurrentDevice]->m_pFontBuffer != NULL)
	{
		m_ScreenArray[m_nCurrentDevice]->m_pFontBuffer->Release();
		m_ScreenArray[m_nCurrentDevice]->m_pFontBuffer = NULL;
	}
	if (m_ScreenArray[m_nCurrentDevice]->m_pOffscreenBuffer != NULL)
	{
		m_ScreenArray[m_nCurrentDevice]->m_pOffscreenBuffer->Release();
		m_ScreenArray[m_nCurrentDevice]->m_pOffscreenBuffer = NULL;
	}

	// Source Width and Height
	m_nSrcWidth = nSrcWidth;
	m_nSrcHeight = nSrcHeight;

	// Create a Off-Screen buffer used to stretch
	if (dwSrcFourCC == BI_RGB)
	{
		// RGB Off-screen
		::ZeroMemory(&ddsd, sizeof(ddsd)); 
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags        = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
		ddsd.dwWidth = m_nSrcWidth;
		ddsd.dwHeight = m_nSrcHeight;
		hRet = m_ScreenArray[m_nCurrentDevice]->m_pDD->CreateSurface(&ddsd, &(m_ScreenArray[m_nCurrentDevice]->m_pOffscreenBuffer), NULL);
		if (Error(hRet, _T("Failed to create the Off-Screen Surface")))
		{
			Free();
			return FALSE;		
		}

		// Get RGB surface description
		::ZeroMemory(&ddsd, sizeof(ddsd)); 
		ddsd.dwSize = sizeof(ddsd);
		hRet = m_ScreenArray[m_nCurrentDevice]->m_pOffscreenBuffer->GetSurfaceDesc(&ddsd);
		if (Error(hRet, _T("Failed to get Off-Screen Surface Description")))
		{
			Free();
			return FALSE;		
		}
		m_ScreenArray[m_nCurrentDevice]->m_nSrcBpp = ddsd.ddpfPixelFormat.dwRGBBitCount;
		m_ScreenArray[m_nCurrentDevice]->m_nSrcPitch = ddsd.lPitch;
		m_ScreenArray[m_nCurrentDevice]->m_nSrcMinPitch = DWALIGNEDWIDTHBYTES(	ddsd.ddpfPixelFormat.dwRGBBitCount *
																				m_nSrcWidth);
		m_ScreenArray[m_nCurrentDevice]->m_bSameSrcPitch =	m_ScreenArray[m_nCurrentDevice]->m_nSrcPitch ==
															m_ScreenArray[m_nCurrentDevice]->m_nSrcMinPitch;
		m_ScreenArray[m_nCurrentDevice]->m_dwSrcFourCC = BI_RGB;
		m_ScreenArray[m_nCurrentDevice]->m_bFlipUV = FALSE;
		if (ddsd.ddpfPixelFormat.dwRGBBitCount == 16)
			m_ScreenArray[m_nCurrentDevice]->m_bRgb15 = (ddsd.ddpfPixelFormat.dwGBitMask == 0x03E0); // RGB16 555 ?
		else
			m_ScreenArray[m_nCurrentDevice]->m_bRgb15 = FALSE;
	}
	else
	{
		// YUV Off-screen
		::ZeroMemory(&ddsd, sizeof(ddsd)); 
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags        = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
		ddsd.dwWidth = m_nSrcWidth;
		ddsd.dwHeight = m_nSrcHeight;
		ddsd.ddpfPixelFormat.dwSize  = sizeof(DDPIXELFORMAT);
		ddsd.ddpfPixelFormat.dwFlags = DDPF_FOURCC;
		ddsd.ddpfPixelFormat.dwFourCC = dwSrcFourCC;
		hRet = m_ScreenArray[m_nCurrentDevice]->m_pDD->CreateSurface(&ddsd, &(m_ScreenArray[m_nCurrentDevice]->m_pOffscreenBuffer), NULL);
		if (Error(hRet, _T("Failed to create the Off-Screen Surface")))
		{
			Free();
			return FALSE;		
		}

		// Get YUV surface description
		::ZeroMemory(&ddsd, sizeof(ddsd)); 
		ddsd.dwSize = sizeof(ddsd);
		hRet = m_ScreenArray[m_nCurrentDevice]->m_pOffscreenBuffer->GetSurfaceDesc(&ddsd);
		if (Error(hRet, _T("Failed to get Off-Screen Surface Description")))
		{
			Free();
			return FALSE;		
		}
		m_ScreenArray[m_nCurrentDevice]->m_nSrcBpp = ddsd.ddpfPixelFormat.dwYUVBitCount;
		m_ScreenArray[m_nCurrentDevice]->m_nSrcPitch = ddsd.lPitch;
		m_ScreenArray[m_nCurrentDevice]->m_nSrcMinPitch = ::CalcYUVStride(	ddsd.ddpfPixelFormat.dwFourCC,
																			m_nSrcWidth);
		m_ScreenArray[m_nCurrentDevice]->m_bSameSrcPitch =	m_ScreenArray[m_nCurrentDevice]->m_nSrcPitch ==
														m_ScreenArray[m_nCurrentDevice]->m_nSrcMinPitch;
		m_ScreenArray[m_nCurrentDevice]->m_dwSrcFourCC = ddsd.ddpfPixelFormat.dwFourCC;
		m_ScreenArray[m_nCurrentDevice]->m_bFlipUV = bFlipUV;
		m_ScreenArray[m_nCurrentDevice]->m_bRgb15 = FALSE;
	}

	// Create the Off-Screen Font Surface
	if (m_FontDib.IsValid())
	{
		::ZeroMemory(&ddsd, sizeof(ddsd)); 
		ddsd.dwSize = sizeof(ddsd);
		if (m_bFontTransparent)
			ddsd.dwFlags        = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_CKSRCBLT;
		else
			ddsd.dwFlags        = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
		ddsd.dwWidth = m_FontTableSize.cx;
		ddsd.dwHeight = m_FontTableSize.cy;
		hRet = m_ScreenArray[m_nCurrentDevice]->m_pDD->CreateSurface(&ddsd, &(m_ScreenArray[m_nCurrentDevice]->m_pFontBuffer), NULL);
		if (Error(hRet, _T("Failed to create the Off-Screen Font Surface")))
		{
			Free();
			return FALSE;		
		}

		// Copy Font Dib
		if (!CopyFontDib())
		{
			Free();
			return FALSE;	
		}

		// Set Source Color Key
		if (m_bFontTransparent)
		{
			// Set the source color key for the surface,
			// the pixel in the upper-left corner of the source is used.
			hRet = SetSrcColorKey(m_ScreenArray[m_nCurrentDevice]->m_pFontBuffer, CLR_INVALID);
			if (Error(hRet, _T("Failed to set the color key for the transparent font")))
			{
				Free();
				return FALSE;		
			}
		}
	}

	return TRUE;
}

// Clean-Up
void CDxDraw::Free()
{
	// Clear init flag
	m_bInit = FALSE;

	// Restore GDI Surface
	if (m_bFullScreen)
		FlipCurrentToGDISurface();

	for (int i = 0 ; i < m_ScreenArray.GetSize() ; i++)
	{
		if (m_ScreenArray[i])
		{
			m_ScreenArray[i]->m_nSrcBpp = 0;
			m_ScreenArray[i]->m_bFlipUV = FALSE;
			m_ScreenArray[i]->m_bRgb15 = FALSE;
			m_ScreenArray[i]->m_nSrcPitch = 0;
			m_ScreenArray[i]->m_nSrcMinPitch = 0;
			m_ScreenArray[i]->m_dwSrcFourCC = 0;
			m_ScreenArray[i]->m_bSameSrcPitch = FALSE;
			m_ScreenArray[i]->m_nMonitorBpp = 0;
			m_ScreenArray[i]->m_bFirstBlt = TRUE;
			if (m_ScreenArray[i]->m_pDD != NULL)
			{
				// If in FullScreen restore to normal mode.
				// (this will also restore the mode if it has been changed)
				if (m_bFullScreen)
				{
					m_ScreenArray[i]->m_pDD->SetCooperativeLevel(m_hWnd, DDSCL_NORMAL | DDSCL_MULTITHREADED);
					m_bDisplayModeChanged = FALSE;
				}
				if (m_ScreenArray[i]->m_pFrontBuffer != NULL)
				{
					if (m_ScreenArray[i]->m_pFontBuffer != NULL)
					{
						m_ScreenArray[i]->m_pFontBuffer->Release();
						m_ScreenArray[i]->m_pFontBuffer = NULL;
					}

					if (m_ScreenArray[i]->m_pOffscreenBuffer != NULL)
					{
						m_ScreenArray[i]->m_pOffscreenBuffer->Release();
						m_ScreenArray[i]->m_pOffscreenBuffer = NULL;
					}

					if (m_ScreenArray[i]->m_pBackBuffer != NULL)
					{
						m_ScreenArray[i]->m_pBackBuffer->Release();
						m_ScreenArray[i]->m_pBackBuffer = NULL;
					}

					if (m_ScreenArray[i]->m_pClipper != NULL)
					{
						m_ScreenArray[i]->m_pClipper->Release();
						m_ScreenArray[i]->m_pClipper = NULL;
					}

					m_ScreenArray[i]->m_pFrontBuffer->Release();
					m_ScreenArray[i]->m_pFrontBuffer = NULL;
				}
				m_ScreenArray[i]->m_pDD->Release();
				m_ScreenArray[i]->m_pDD = NULL;
			}
		}
	}

	// Reset Vars
	m_nCurrentDevice = 0;
	m_nSrcWidth = 0;
	m_nSrcHeight = 0;
	m_FontTableSize.cx = 0;
	m_FontTableSize.cy = 0;
	m_FontSize.cx = 0;
	m_FontSize.cy = 0;
	m_hWnd = NULL;
	m_bFullScreen = FALSE;
	m_bTripleBuffering = FALSE;
}

void CDxDraw::DeleteScreenArray()
{
	for (int i = 0 ; i < m_ScreenArray.GetSize() ; i++)
	{
		if (m_ScreenArray[i])
			delete m_ScreenArray[i];
	}
	m_ScreenArray.RemoveAll();
}

BOOL CDxDraw::GetCurrentVideoMem(DWORD& dwTotal, DWORD& dwFree)
{
	DDSCAPS2 ddsCaps2;
	::ZeroMemory(&ddsCaps2, sizeof(ddsCaps2));
	ddsCaps2.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM; // Don't Count AGP Mem!
 
	// Init?
	if (m_ScreenArray.GetSize() <= 0)
		return FALSE;
	
	dwTotal = 0;
	dwFree = 0;
	HRESULT hRet = m_ScreenArray[m_nCurrentDevice]->m_pDD->GetAvailableVidMem(&ddsCaps2, &dwTotal, &dwFree); 
	if (Error(hRet, _T("GetCurrentVideoMem()")))
		return FALSE;
	else
		return TRUE;
}

// Clear Front Buffer rc
void CDxDraw::ClearFront(const RECT* prc/*=NULL*/)
{
	HRESULT hRet;
	DDBLTFX fx;
	::ZeroMemory(&fx, sizeof(fx)); 
	fx.dwSize = sizeof(fx);
	fx.dwFillColor = 0;

	// Init?
	if (m_ScreenArray.GetSize() <= 0)
		return;

	// Find out where our window lives
	RECT rc;
	if (prc)
	{
		rc = *prc;
		ClientToMonitor(&rc);
	}

	if (m_ScreenArray[m_nCurrentDevice]->m_pFrontBuffer)
	{
		for (int loop = 0 ; loop < DXDRAW_MAX_RETRY ; loop++)
		{
			hRet = m_ScreenArray[m_nCurrentDevice]->m_pFrontBuffer->Blt(prc ? &rc : NULL,
																		NULL,
																		NULL,
																		DDBLT_COLORFILL,
																		&fx);
			if (hRet == DD_OK)
				break;
			else if (hRet == DDERR_SURFACELOST)
			{
				if (!RestoreSurfaces())
					break;
			}
			else if (hRet != DDERR_WASSTILLDRAWING)
			{
				Error(hRet, _T("ClearFront()"));
				break;
			}
			::Sleep(DXDRAW_RETRY_SLEEP);
		}
	}
}

// Clear Back Buffer rc
void CDxDraw::ClearBack(const RECT* prc/*=NULL*/)
{
	HRESULT hRet;
	DDBLTFX fx;
	::ZeroMemory(&fx, sizeof(fx)); 
	fx.dwSize = sizeof(fx);
	fx.dwFillColor = 0;

	// Init?
	if (m_ScreenArray.GetSize() <= 0)
		return;

	if (m_ScreenArray[m_nCurrentDevice]->m_pBackBuffer)
	{
		for (int loop = 0 ; loop < DXDRAW_MAX_RETRY ; loop++)
		{
			hRet = m_ScreenArray[m_nCurrentDevice]->m_pBackBuffer->Blt(	(LPRECT)prc,
																		NULL,
																		NULL,
																		DDBLT_COLORFILL,
																		&fx);
			if (hRet == DD_OK)
				break;
			else if (hRet == DDERR_SURFACELOST)
			{
				if (!RestoreSurfaces())
					break;
			}
			else if (hRet != DDERR_WASSTILLDRAWING)
			{
				Error(hRet, _T("ClearBack()"));
				break;
			}
			::Sleep(DXDRAW_RETRY_SLEEP);
		}
	}
}

// Clear Offscreen Buffer rc
void CDxDraw::ClearOffscreen(const RECT* prc/*=NULL*/)
{
	HRESULT hRet;
	DDBLTFX fx;
	::ZeroMemory(&fx, sizeof(fx)); 
	fx.dwSize = sizeof(fx);
	fx.dwFillColor = 0;

	// Init?
	if (m_ScreenArray.GetSize() <= 0)
		return;

	if (m_ScreenArray[m_nCurrentDevice]->m_pOffscreenBuffer)
	{
		for (int loop = 0 ; loop < DXDRAW_MAX_RETRY ; loop++)
		{
			hRet = m_ScreenArray[m_nCurrentDevice]->m_pOffscreenBuffer->Blt((LPRECT)prc,
																			NULL,
																			NULL,
																			DDBLT_COLORFILL,
																			&fx);
			if (hRet == DD_OK)
				break;
			else if (hRet == DDERR_SURFACELOST)
			{
				if (!RestoreSurfaces())
					break;
			}
			else if (hRet != DDERR_WASSTILLDRAWING)
			{
				Error(hRet, _T("ClearOffscreen()"));
				break;
			}
			::Sleep(DXDRAW_RETRY_SLEEP);
		}
	}
}

void CDxDraw::UpdateCurrentDevice()
{
	// In Full-Screen Mode the Device is Fix!
	if (!m_bFullScreen)
	{
		RECT rcIntersection;

		if (m_ScreenArray.GetSize() <= 1)
			m_nCurrentDevice = 0;
		else
		{
			int nCurrentDevice = 0;
			for (int i = 1 ; i < m_ScreenArray.GetSize() ; i++)
			{
				if (m_ScreenArray[i])
				{
					RECT rcClient;
					::GetClientRect(m_hWnd, &rcClient);
					POINT p = {0, 0};
					::ClientToScreen(m_hWnd, &p);
					OffsetRect(&rcClient, p.x, p.y);
					if (IntersectRect(	&rcIntersection,
										&rcClient,
										&(m_ScreenArray[i]->m_rcMonitor)))
					{
						// If Already Set
						if (nCurrentDevice > 0)
						{
							m_nCurrentDevice = 0;
							return;
						}
						else
							nCurrentDevice = i;	// Ok First Monitor that intersects
					}
				}
			}
			m_nCurrentDevice = nCurrentDevice;
		}
	}
}

// Copy bits from off-screen buffer
// to primary buffer for display
BOOL CDxDraw::Blt(RECT rcDest, RECT rcSrc)
{
	HRESULT hRet;
	DDSURFACEDESC2 ddsd;
    ::ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);

	// Init?
	if (m_ScreenArray.GetSize() <= 0)
		return FALSE;

	// Find out where our window lives
	ClientToMonitor(&rcDest);

    for (int loop = 0 ; loop < DXDRAW_MAX_RETRY ; loop++)
    {
		// WaitForVerticalBlank() is used to synchronise the flipping
		// with the refresh rate of the monitor. Without it,
		// you get an effect called tearing which happens because
		// the monitor is displaying part of the screen with the old frame memory
		// and the remaining part of the screen is filled with the new frame memory.
		// Enabling this rises the CPU Usage dramatically and is not working anyway with Blt!!!
		//m_ScreenArray[m_nCurrentDevice]->m_pDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, NULL);
	
		// Blt
		hRet = m_ScreenArray[m_nCurrentDevice]->m_pFrontBuffer->Blt(
															&rcDest,		// Dest Rect:
																			// NULL = the entire destination surface is used
															m_ScreenArray[m_nCurrentDevice]->m_pBackBuffer,	// Src Surface
															&rcSrc,			// Src Rect:
																			// NULL = the entire source surface is used
															DDBLT_WAIT,		// Flags
															NULL);			// Blt Function
		if (hRet == DD_OK)
			return TRUE;
        else if (hRet == DDERR_SURFACELOST)
		{
			if (!RestoreSurfaces())
				return FALSE;
		}
        else if (hRet != DDERR_WASSTILLDRAWING)
		{
			Error(hRet, _T("Blt()"));
            return FALSE;
		}
		::Sleep(DXDRAW_RETRY_SLEEP);
	}

	return FALSE;
}

// Flip bits from back buffer
// to primary buffer for display
BOOL CDxDraw::Flip(BOOL bDoBlt/*=FALSE*/)
{
	HRESULT hRet;
	DDSURFACEDESC2 ddsd;
    ::ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);

	// Init?
	if (m_ScreenArray.GetSize() <= 0)
		return FALSE;

	// Must be FullScreen!
	if (!m_bFullScreen)
		return FALSE;

    for (int loop = 0 ; loop < DXDRAW_MAX_RETRY ; loop++)
    {		
		if (bDoBlt)
		{
			// Enabling this gives no better results for video playing with Blt...
			//m_ScreenArray[m_nCurrentDevice]->m_pDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, NULL);
			hRet = m_ScreenArray[m_nCurrentDevice]->m_pFrontBuffer->Blt(NULL,
																		m_ScreenArray[m_nCurrentDevice]->m_pBackBuffer,
																		NULL, DDBLT_WAIT, NULL);

			// Is First Blt?
			if (m_ScreenArray[m_nCurrentDevice]->m_bFirstBlt)
			{
				if (m_bTripleBuffering)
				{
					// Front And Back have the same data,
					// but the third buffer has not yet been updated,
					// and it may be the GDI Surface!
					// -> make sure that this third buffer contains
					// also the same data.
					hRet = m_ScreenArray[m_nCurrentDevice]->m_pFrontBuffer->Flip(NULL, DDFLIP_WAIT);
					hRet = m_ScreenArray[m_nCurrentDevice]->m_pBackBuffer->Blt(	NULL,
																				m_ScreenArray[m_nCurrentDevice]->m_pFrontBuffer,
																				NULL, DDBLT_WAIT, NULL);
				}

				// Flip To GDI Surface
				m_ScreenArray[m_nCurrentDevice]->m_pDD->FlipToGDISurface();
				m_ScreenArray[m_nCurrentDevice]->m_bFirstBlt = FALSE;
			}
		}
		else
		{
			// Reset
			m_ScreenArray[m_nCurrentDevice]->m_bFirstBlt = TRUE;

			// Not Needed to call WaitForVerticalBlank, Flip waits the vertical Blank!
			hRet = m_ScreenArray[m_nCurrentDevice]->m_pFrontBuffer->Flip(NULL, DDFLIP_WAIT);
		}

		if (hRet == DD_OK)
			return TRUE;
        else if (hRet == DDERR_SURFACELOST)
		{
			if (!RestoreSurfaces())
				return FALSE;
		}
        else if (hRet != DDERR_WASSTILLDRAWING)
		{
			Error(hRet, _T("Flip()"));
            return FALSE;
		}
		::Sleep(DXDRAW_RETRY_SLEEP);
	}

	return FALSE;
}

BOOL CDxDraw::RestoreSurfaces()
{
	HRESULT hRet;
	BOOL res = TRUE;

	if (m_ScreenArray[m_nCurrentDevice]->m_pFrontBuffer)
	{
		hRet = m_ScreenArray[m_nCurrentDevice]->m_pFrontBuffer->Restore();
		if (Error(hRet, _T("Display Surface Lost -> Front Buffer Restore Failed!"))) 
			res = FALSE;
	}

	if (m_ScreenArray[m_nCurrentDevice]->m_pBackBuffer)
	{
		hRet = m_ScreenArray[m_nCurrentDevice]->m_pBackBuffer->Restore();
		if (Error(hRet, _T("Display Surface Lost -> Back Buffer Restore Failed!")))
			res = FALSE;
	}

	if (m_ScreenArray[m_nCurrentDevice]->m_pOffscreenBuffer)
	{
		hRet = m_ScreenArray[m_nCurrentDevice]->m_pOffscreenBuffer->Restore();
		if (Error(hRet, _T("Display Surface Lost -> Offscreen Buffer Full Restore Failed!")))
			res = FALSE;
	}

	if (m_ScreenArray[m_nCurrentDevice]->m_pFontBuffer)
	{
		hRet = m_ScreenArray[m_nCurrentDevice]->m_pFontBuffer->Restore();
		if (Error(hRet, _T("Display Surface Lost -> Font Buffer Full Restore Failed!")))
			res = FALSE;
		else if (!CopyFontDib(FALSE)) // Restore font without calling ourself if surfaces have to be restored!
			res = FALSE;
	}
	
	if (res)
		::PostMessage(m_hWnd, WM_RESTORE_FRAME, 0, 0);

	return res;
}

__forceinline BOOL CDxDraw::IsCurrentSrcSameRgbFormat(LPBITMAPINFO pBmi)
{
	// Current Src Bpp
	int nSrcBpp = GetCurrentSrcBpp();

	// No 8 bpp direct copy support
	if (nSrcBpp < 16)
		return FALSE;

	// No Same Bpp?
	if (nSrcBpp != pBmi->bmiHeader.biBitCount)
		return FALSE;

	// For 24 Bpp and 32 Bpp we are ok.
	if (nSrcBpp >= 24)
		return TRUE;
	// For 16 Bpp
	else
	{
		// TRUE if both 555 or both 565 format
		return (IsCurrentSrcRgb15() == CDib::IsRgb16_555(pBmi));
	}
}

BOOL CDxDraw::RenderDib(LPBITMAPINFO pBmi, LPBYTE pSrc, CRect rc)
{
	// Check
	if (!pBmi || !pSrc)
		return FALSE;

	// Mem. Copy for YUV Surfaces
	DWORD dwSrcFourCC = GetCurrentSrcFourCC();
	if (dwSrcFourCC == FCC('YV12') ||
		dwSrcFourCC == FCC('YUY2'))
	{
		// Lock Direct Draw Video Memory
		DDSURFACEDESC2 ddsd;
		if (!LockSrc(&ddsd))
			return FALSE;

		// Direct Mem. Copy only if Same Pitch!
		if (HasSameSrcPitch() && !GetCurrentSrcFlipUV())
			memcpy(ddsd.lpSurface, pSrc, pBmi->bmiHeader.biSizeImage);
		else
		{
			LPBYTE pDst = (LPBYTE)ddsd.lpSurface;
			int nMinPitch = GetCurrentSrcMinPitch(); // The output from the VCM or from capture is always with the minimum possible pitch
			if (dwSrcFourCC == FCC('YV12'))
			{
				int line;
				int nMinPitch2 = nMinPitch>>1;
				int nPitch2 = (ddsd.lPitch)>>1;

				// Copy Y Plane
				for (line = 0 ; line < pBmi->bmiHeader.biHeight ; line++)
				{
					memcpy(pDst, pSrc, nMinPitch);
					pSrc += nMinPitch;
					pDst += ddsd.lPitch;
				}

				// Copy V and U Planes
				if (GetCurrentSrcFlipUV())
				{
					int nHeight2 = (pBmi->bmiHeader.biHeight)>>1;
					pSrc += (nMinPitch2 * nHeight2);
					for (line = 0 ; line < nHeight2 ; line++)
					{
						memcpy(pDst, pSrc, nMinPitch2);
						pSrc += nMinPitch2;
						pDst += nPitch2;
					}
					pSrc -= (2 * nMinPitch2 * nHeight2);
					for (line = 0 ; line < nHeight2 ; line++)
					{
						memcpy(pDst, pSrc, nMinPitch2);
						pSrc += nMinPitch2;
						pDst += nPitch2;
					}
				}
				else
				{				
					for (line = 0 ; line < pBmi->bmiHeader.biHeight ; line++)
					{
						memcpy(pDst, pSrc, nMinPitch2);
						pSrc += nMinPitch2;
						pDst += nPitch2;
					}
				}
			}
			else
			{
				for (int line = 0 ; line < pBmi->bmiHeader.biHeight ; line++)
				{
					memcpy(pDst, pSrc, nMinPitch);
					pSrc += nMinPitch;
					pDst += ddsd.lPitch;
				}
			}
		}
		
		// Unlock Video Memory
		UnlockSrc();

		// Blt From Offscreen Buffer to the Back Buffer
		// (if Offscreen initialized)
		UpdateBackSurface(rc);
	}
	// RGB Surface
	else 
	{	
		// FullScreen Direct Copy and Two Steps Copy Vars
		BOOL bFullScreenDirectCopy = FALSE;
		BOOL bTwoStepsCopy = FALSE;
		if (IsFullScreen())
		{
			if ((rc.Width() != m_nSrcWidth)		||	// If Stretching
				(rc.Height() != m_nSrcHeight)	||	// If Stretching
				IsClippingRect(rc))					// If Displaying outside monitor borders
				bTwoStepsCopy = TRUE;
			else
				bFullScreenDirectCopy = TRUE;
		}

		// Direct Copy with Flip
		if (IsCurrentSrcSameRgbFormat(pBmi))
		{
			// Pixel Size in Bytes
			int nPixelsSize = GetCurrentSrcBpp() / 8;

			// Lock Direct Draw Video Memory
			DDSURFACEDESC2 ddsd;
			if (bTwoStepsCopy)
			{
				if (!LockOffscreen(&ddsd))
					return FALSE;
			}
			else
			{
				if (!LockBack(&ddsd))
					return FALSE;
			}

			// Destination Bits Pointer
			LPBYTE pDst;
			if (bFullScreenDirectCopy)
				pDst = (LPBYTE)ddsd.lpSurface +
						((int)ddsd.lPitch * (((int)pBmi->bmiHeader.biHeight - 1) + rc.top))
						+ rc.left * nPixelsSize;
			else
				pDst = (LPBYTE)ddsd.lpSurface +
						((int)ddsd.lPitch * ((int)pBmi->bmiHeader.biHeight - 1));

			// Bottom-Up to Top-Down Conversion
			int nMinPitch = GetCurrentSrcMinPitch();
			for (int line = 0 ; line < pBmi->bmiHeader.biHeight ; line++)
			{
				memcpy(pDst, pSrc, nMinPitch);
				pSrc += nMinPitch;
				pDst -= (DWORD)ddsd.lPitch;
			}

			// Unlock Video Memory
			if (bTwoStepsCopy)
				UnlockOffscreen();
			else
				UnlockBack();
		}
		// Copy through DC
		else
		{	
			// Get DC
			HDC hDC;
			if (bTwoStepsCopy)
				hDC = GetOffscreenDC();
			else
				hDC = GetBackDC();

			// Copy Bits
			if (bFullScreenDirectCopy)
			{
				::SetDIBitsToDevice(hDC,						// hDC
								   rc.left,						// DestX
								   rc.top,						// DestY
								   pBmi->bmiHeader.biWidth,		// nSrcWidth
								   pBmi->bmiHeader.biHeight,	// nSrcHeight
								   0,							// SrcX
								   0,							// SrcY
								   0,							// nStartScan
								   pBmi->bmiHeader.biHeight,	// nNumScans
								   pSrc,						// Bits
								   pBmi,						// Bitmap Info
								   DIB_RGB_COLORS);				// wUsage
			}
			else
			{
				::SetDIBitsToDevice(hDC,						// hDC
								   0,							// DestX
								   0,							// DestY
								   pBmi->bmiHeader.biWidth,		// nSrcWidth
								   pBmi->bmiHeader.biHeight,	// nSrcHeight
								   0,							// SrcX
								   0,							// SrcY
								   0,							// nStartScan
								   pBmi->bmiHeader.biHeight,	// nNumScans
								   pSrc,						// Bits
								   pBmi,						// Bitmap Info
								   DIB_RGB_COLORS);				// wUsage
			}


			// Release DC
			if (bTwoStepsCopy)
				ReleaseOffscreenDC(hDC);
			else
				ReleaseBackDC(hDC);
		}

		// Blt From Offscreen Buffer to the Back Buffer
		if (bTwoStepsCopy)
			UpdateBackSurface(rc);
	}

	return TRUE;
}

BOOL CDxDraw::UpdateBackSurface(CRect rc)
{
	// Blt From Offscreen Buffer to the Back Buffer
	// (if Offscreen initialized)
	if (GetOffscreenSurface())
	{
		// Find out where our window lives
		ClientToMonitor(&rc);

		// Copy From Offscreen Buffer to Back Buffer
		for (int loop = 0 ; loop < DXDRAW_MAX_RETRY ; loop++)
		{
			if (GetBackSurface() && GetOffscreenSurface())
			{
				HRESULT hRet;
				if (m_bFullScreen)
				{
					if (IsClippingRect(rc))
					{
						CRect rcDstClip(0,0,0,0);
						CRect rcDstClipped(rc);
						CRect rcSrcClip;
						CRect rcSrcClipped;

						// Destination Size
						int nDstWidth =		m_ScreenArray[m_nCurrentDevice]->m_rcMonitor.right - 
											m_ScreenArray[m_nCurrentDevice]->m_rcMonitor.left;
						int nDstHeight =	m_ScreenArray[m_nCurrentDevice]->m_rcMonitor.bottom - 
											m_ScreenArray[m_nCurrentDevice]->m_rcMonitor.top;

						// Ratios
						double dRatioX = (double)m_nSrcWidth / (double)nDstWidth;
						double dRatioY = (double)m_nSrcHeight / (double)nDstHeight;

						// Destination Clip
						if (rc.left < 0)
						{
							rcDstClip.left = -rc.left;
							rcDstClipped.left = 0;
						}
						if (rc.top < 0)
						{
							rcDstClip.top = -rc.top;
							rcDstClipped.top = 0;
						}
						if (rc.right > nDstWidth)
						{
							rcDstClip.right = rc.right - nDstWidth;
							rcDstClipped.right = nDstWidth;
						}
						if (rc.bottom > nDstHeight)
						{
							rcDstClip.bottom = rc.bottom - nDstHeight;
							rcDstClipped.bottom = nDstHeight;
						}

						// Source Clip
						rcSrcClip.left = Round(dRatioX * rcDstClip.left);
						rcSrcClip.right = Round(dRatioX * rcDstClip.right);
						rcSrcClip.top = Round(dRatioY * rcDstClip.top);
						rcSrcClip.bottom = Round(dRatioY * rcDstClip.bottom);
						rcSrcClipped.left = rcSrcClip.left;
						rcSrcClipped.top = rcSrcClip.top;
						rcSrcClipped.right = m_nSrcWidth - rcSrcClip.right;
						rcSrcClipped.bottom = m_nSrcHeight - rcSrcClip.bottom;

						// Blt
						hRet = GetBackSurface()->Blt(rcDstClipped,
													GetOffscreenSurface(),
													rcSrcClipped,
													DDBLT_WAIT,
													NULL);
					}
					else
					{
						hRet = GetBackSurface()->Blt(rc,
													GetOffscreenSurface(),
													NULL,
													DDBLT_WAIT,
													NULL);
					}
				}
				else
					hRet = GetBackSurface()->Blt(NULL,
												GetOffscreenSurface(),
												NULL,
												DDBLT_WAIT,
												NULL);
				if (hRet == DD_OK)
					return TRUE;
				else if (hRet == DDERR_SURFACELOST)
				{
					if (!RestoreSurfaces())
						return FALSE;
				}
				else if (hRet != DDERR_WASSTILLDRAWING)
				{
					Error(hRet, _T("UpdateBackSurface()"));
					return FALSE;
				}
			}
			else
				return FALSE;
			::Sleep(DXDRAW_RETRY_SLEEP);
		}
	}
	return FALSE;
}

BOOL CDxDraw::LoadFontDib(UINT uID)
{
	if (m_FontDib.LoadDibSectionRes(::AfxGetInstanceHandle(), uID))
	{
		m_FontTableSize.cx = m_FontDib.GetWidth();
		m_FontTableSize.cy = m_FontDib.GetHeight();
		m_FontSize.cx = m_FontTableSize.cx / FONT_COLUMNS;
		m_FontSize.cy = m_FontTableSize.cy / FONT_ROWS;
		return TRUE;
	}
	else
	{
		m_FontTableSize.cx = 0;
		m_FontTableSize.cy = 0;
		m_FontSize.cx = 0;
		m_FontSize.cy = 0;
		return FALSE;
	}
}

BOOL CDxDraw::CopyFontDib(BOOL bRestoreSurfaces/*=TRUE*/)
{
	// Get DC
	HDC hDC = GetFontDC(bRestoreSurfaces);
	if (hDC)
	{
		// Copy Bits
		HDC memDC = ::CreateCompatibleDC(hDC);
		HBITMAP hOldBitmap = (HBITMAP)::SelectObject(memDC, (HBITMAP)m_FontDib);
		BOOL res = ::BitBlt(hDC,
							0, 0, 
							m_FontTableSize.cx, m_FontTableSize.cy,
							memDC,
							0, 0, 
							SRCCOPY);
		::SelectObject(memDC, hOldBitmap);
		::DeleteDC(memDC);

		// Release DC
		ReleaseFontDC(hDC, bRestoreSurfaces);

		// Error
		if (!res)
			TRACE(_T("BitBlt failed in CopyFontDib()\n"));

		return res;
	}
	else
		return FALSE; // error traced in GetFontDC()
}

/* uiAlign may be:
DRAWTEXT_BOTTOMLEFT
DRAWTEXT_TOPLEFT
DRAWTEXT_BOTTOMRIGHT
DRAWTEXT_TOPRIGHT
*/
BOOL CDxDraw::DrawText(LPCTSTR sText, int x, int y, UINT uiAlign)
{
	HRESULT hRet;
	RECT rcDst, rcSrc;
	int i;
	BOOL bLeftAlign;
	char c;
	int len = _tcslen(sText);

	// Max. Width & Height
	int nMaxWidth, nMaxHeight;
	if (m_bFullScreen)
	{
		nMaxWidth =		m_ScreenArray[m_nCurrentDevice]->m_rcMonitor.right - 
						m_ScreenArray[m_nCurrentDevice]->m_rcMonitor.left;
		nMaxHeight =	m_ScreenArray[m_nCurrentDevice]->m_rcMonitor.bottom - 
						m_ScreenArray[m_nCurrentDevice]->m_rcMonitor.top;
	}
	else
	{
		nMaxWidth = m_nSrcWidth;
		nMaxHeight = m_nSrcHeight;
	}

	// Align Right?
	if (uiAlign >= DRAWTEXT_BOTTOMRIGHT)
	{
		i = len - 1;
		bLeftAlign = FALSE;
	}
	// Align Left
	else
	{
		i = 0;
		bLeftAlign = TRUE;
	}

	// Init c
#ifdef _UNICODE
	wctomb(&c, sText[i]);
#else
	c = sText[i];
#endif

	// Loop Chars
	while (c != _T('\0') && (i >= 0))
	{
		int nCharOffset = c - FONT_ASCII_OFFSET;
		if (nCharOffset >= 0 && nCharOffset < FONT_COLUMNS * FONT_ROWS)
		{
			int xOffset = nCharOffset % FONT_COLUMNS;
			int yOffset = nCharOffset / FONT_COLUMNS;

			// Destination Rect
			if (uiAlign == DRAWTEXT_TOPLEFT)
			{
				rcDst.left = x + i * m_FontSize.cx;
				rcDst.top = y;
				rcDst.right = x + (i + 1) * m_FontSize.cx;
				rcDst.bottom = y + m_FontSize.cy;
			}
			else if (uiAlign == DRAWTEXT_BOTTOMLEFT)
			{
				rcDst.left = x + i * m_FontSize.cx;
				rcDst.top = y - m_FontSize.cy + 1;
				rcDst.right = x + (i + 1) * m_FontSize.cx;
				rcDst.bottom = y + 1;
			}
			else if (uiAlign == DRAWTEXT_TOPRIGHT)
			{
				rcDst.left = x - (len - i) * m_FontSize.cx + 1;
				rcDst.top = y;
				rcDst.right = x - (len - i - 1) * m_FontSize.cx + 1;
				rcDst.bottom = y + m_FontSize.cy;
			}
			else if (uiAlign == DRAWTEXT_BOTTOMRIGHT)
			{
				rcDst.left = x - (len - i) * m_FontSize.cx + 1;
				rcDst.top = y - m_FontSize.cy + 1;
				rcDst.right = x - (len - i - 1) * m_FontSize.cx + 1;
				rcDst.bottom = y + 1;
			}
			else
				return FALSE;

			// Source Rect
			rcSrc.left = xOffset * m_FontSize.cx;
			rcSrc.top = yOffset  * m_FontSize.cy;
			rcSrc.right = rcSrc.left + m_FontSize.cx;
			rcSrc.bottom = rcSrc.top + m_FontSize.cy;

			// Copy From Offscreen Font Buffer to Back Buffer
			if ((rcDst.left >= 0)				&&
				(rcDst.top >= 0)				&&
				(rcDst.right <= nMaxWidth)		&&
				(rcDst.bottom <= nMaxHeight))
			{
				for (int loop = 0 ; loop < DXDRAW_MAX_RETRY ; loop++)
				{
					if (GetBackSurface())
					{
						hRet = GetBackSurface()->Blt(	&rcDst,
														GetFontSurface(),
														&rcSrc,
														(m_bFontTransparent ? DDBLT_KEYSRC : 0) |
														DDBLT_WAIT,
														NULL);
						if (hRet == DD_OK)
							break;
						else if (hRet == DDERR_SURFACELOST)
						{
							if (!RestoreSurfaces())
								break;
						}
						else if (hRet != DDERR_WASSTILLDRAWING)
						{
							Error(hRet, _T("DrawText()"));
							break;
						}
					}
					else
						break;
					::Sleep(DXDRAW_RETRY_SLEEP);
				}
			}
		}

		// Inc. / Dec.
		if (bLeftAlign)
			++i;
		else
			--i;

		// Update c
#ifdef _UNICODE
		wctomb(&c, sText[i]);
#else
		c = sText[i];
#endif
	}

	return TRUE;
}

// Convert a RGB color to a physical color.
// We do this by letting GDI SetPixel() do the color matching
// then we lock the memory and see what it got mapped to.
DWORD CDxDraw::ColorMatch(IDirectDrawSurface7* pdds, COLORREF rgb)
{
    COLORREF                rgbT = CLR_INVALID;
    HDC                     hdc;
    DWORD                   dw = CLR_INVALID;
    DDSURFACEDESC2          ddsd;
    HRESULT                 hres;

    //  Use GDI SetPixel to color match for us
    if (rgb != CLR_INVALID && pdds->GetDC(&hdc) == DD_OK)
    {
        rgbT = ::GetPixel(hdc, 0, 0);	// Save current pixel value
        ::SetPixel(hdc, 0, 0, rgb);		// Set our value
        pdds->ReleaseDC(hdc);
    }

    // Now lock the surface so we can read back the converted color
    ddsd.dwSize = sizeof(ddsd);
	for (int loop = 0 ; loop < DXDRAW_MAX_RETRY ; loop++)
	{
		if ((hres = pdds->Lock(NULL, &ddsd, 0, NULL)) != DDERR_WASSTILLDRAWING)
			break;
		::Sleep(DXDRAW_RETRY_SLEEP);
	}
    if (hres == DD_OK)
    {
        dw = *(DWORD *) ddsd.lpSurface;                 // Get DWORD
        if (ddsd.ddpfPixelFormat.dwRGBBitCount < 32)
            dw &= (1 << ddsd.ddpfPixelFormat.dwRGBBitCount) - 1;  // Mask it to bpp
        pdds->Unlock(NULL);
    }

    //  Now put the color that was there back.
    if (rgb != CLR_INVALID && pdds->GetDC(&hdc) == DD_OK)
    {
        ::SetPixel(hdc, 0, 0, rgbT);
        pdds->ReleaseDC(hdc);
    }

    return dw;
}

// Set a source color key for a surface, given a RGB.
// If you pass CLR_INVALID as the color key, the pixel
// in the upper-left corner will be used.
HRESULT CDxDraw::SetSrcColorKey(IDirectDrawSurface7* pdds, COLORREF rgb)
{
    DDCOLORKEY ddck;
    ddck.dwColorSpaceLowValue = ColorMatch(pdds, rgb);
    ddck.dwColorSpaceHighValue = ddck.dwColorSpaceLowValue;
    return pdds->SetColorKey(DDCKEY_SRCBLT, &ddck);
}

TCHAR* CDxDraw::ErrorString(HRESULT hRet)
{
	switch (hRet)
	{
		case DDERR_ALREADYINITIALIZED:           return _T("DDERR_ALREADYINITIALIZED");
		case DDERR_CANNOTATTACHSURFACE:          return _T("DDERR_CANNOTATTACHSURFACE");
		case DDERR_CANNOTDETACHSURFACE:          return _T("DDERR_CANNOTDETACHSURFACE");
		case DDERR_CURRENTLYNOTAVAIL:            return _T("DDERR_CURRENTLYNOTAVAIL");
		case DDERR_EXCEPTION:                    return _T("DDERR_EXCEPTION");
		case DDERR_GENERIC:                      return _T("DDERR_GENERIC");
		case DDERR_HEIGHTALIGN:                  return _T("DDERR_HEIGHTALIGN");
		case DDERR_INCOMPATIBLEPRIMARY:          return _T("DDERR_INCOMPATIBLEPRIMARY");
		case DDERR_INVALIDCAPS:                  return _T("DDERR_INVALIDCAPS");
		case DDERR_INVALIDCLIPLIST:              return _T("DDERR_INVALIDCLIPLIST");
		case DDERR_INVALIDMODE:                  return _T("DDERR_INVALIDMODE");
		case DDERR_INVALIDOBJECT:                return _T("DDERR_INVALIDOBJECT");
		case DDERR_INVALIDPARAMS:                return _T("DDERR_INVALIDPARAMS");
		case DDERR_INVALIDPIXELFORMAT:           return _T("DDERR_INVALIDPIXELFORMAT");
		case DDERR_INVALIDRECT:                  return _T("DDERR_INVALIDRECT");
		case DDERR_LOCKEDSURFACES:               return _T("DDERR_LOCKEDSURFACES");
		case DDERR_NO3D:                         return _T("DDERR_NO3D");
		case DDERR_NOALPHAHW:                    return _T("DDERR_NOALPHAHW");
		case DDERR_NOCLIPLIST:                   return _T("DDERR_NOCLIPLIST");
		case DDERR_NOCOLORCONVHW:                return _T("DDERR_NOCOLORCONVHW");
		case DDERR_NOCOOPERATIVELEVELSET:        return _T("DDERR_NOCOOPERATIVELEVELSET");
		case DDERR_NOCOLORKEY:                   return _T("DDERR_NOCOLORKEY");
		case DDERR_NOCOLORKEYHW:                 return _T("DDERR_NOCOLORKEYHW");
		case DDERR_NODIRECTDRAWSUPPORT:          return _T("DDERR_NODIRECTDRAWSUPPORT");
		case DDERR_NOEXCLUSIVEMODE:              return _T("DDERR_NOEXCLUSIVEMODE");
		case DDERR_NOFLIPHW:                     return _T("DDERR_NOFLIPHW");
		case DDERR_NOGDI:                        return _T("DDERR_NOGDI");
		case DDERR_NOMIRRORHW:                   return _T("DDERR_NOMIRRORHW");
		case DDERR_NOTFOUND:                     return _T("DDERR_NOTFOUND");
		case DDERR_NOOVERLAYHW:                  return _T("DDERR_NOOVERLAYHW");
		case DDERR_NORASTEROPHW:                 return _T("DDERR_NORASTEROPHW");
		case DDERR_NOROTATIONHW:                 return _T("DDERR_NOROTATIONHW");
		case DDERR_NOSTRETCHHW:                  return _T("DDERR_NOSTRETCHHW");
		case DDERR_NOT4BITCOLOR:                 return _T("DDERR_NOT4BITCOLOR");
		case DDERR_NOT4BITCOLORINDEX:            return _T("DDERR_NOT4BITCOLORINDEX");
		case DDERR_NOT8BITCOLOR:                 return _T("DDERR_NOT8BITCOLOR");
		case DDERR_NOTEXTUREHW:                  return _T("DDERR_NOTEXTUREHW");
		case DDERR_NOVSYNCHW:                    return _T("DDERR_NOVSYNCHW");
		case DDERR_NOZBUFFERHW:                  return _T("DDERR_NOZBUFFERHW");
		case DDERR_NOZOVERLAYHW:                 return _T("DDERR_NOZOVERLAYHW");
		case DDERR_OUTOFCAPS:                    return _T("DDERR_OUTOFCAPS");
		case DDERR_OUTOFMEMORY:                  return _T("DDERR_OUTOFMEMORY");
		case DDERR_OUTOFVIDEOMEMORY:             return _T("DDERR_OUTOFVIDEOMEMORY");
		case DDERR_OVERLAYCANTCLIP:              return _T("DDERR_OVERLAYCANTCLIP");
		case DDERR_OVERLAYCOLORKEYONLYONEACTIVE: return _T("DDERR_OVERLAYCOLORKEYONLYONEACTIVE");
		case DDERR_PALETTEBUSY:                  return _T("DDERR_PALETTEBUSY");
		case DDERR_COLORKEYNOTSET:               return _T("DDERR_COLORKEYNOTSET");
		case DDERR_SURFACEALREADYATTACHED:       return _T("DDERR_SURFACEALREADYATTACHED");
		case DDERR_SURFACEALREADYDEPENDENT:      return _T("DDERR_SURFACEALREADYDEPENDENT");
		case DDERR_SURFACEBUSY:                  return _T("DDERR_SURFACEBUSY");
		case DDERR_CANTLOCKSURFACE:              return _T("DDERR_CANTLOCKSURFACE");
		case DDERR_SURFACEISOBSCURED:            return _T("DDERR_SURFACEISOBSCURED");
		case DDERR_SURFACELOST:                  return _T("DDERR_SURFACELOST");
		case DDERR_SURFACENOTATTACHED:           return _T("DDERR_SURFACENOTATTACHED");
		case DDERR_TOOBIGHEIGHT:                 return _T("DDERR_TOOBIGHEIGHT");
		case DDERR_TOOBIGSIZE:                   return _T("DDERR_TOOBIGSIZE");
		case DDERR_TOOBIGWIDTH:                  return _T("DDERR_TOOBIGWIDTH");
		case DDERR_UNSUPPORTED:                  return _T("DDERR_UNSUPPORTED");
		case DDERR_UNSUPPORTEDFORMAT:            return _T("DDERR_UNSUPPORTEDFORMAT");
		case DDERR_UNSUPPORTEDMASK:              return _T("DDERR_UNSUPPORTEDMASK");
		case DDERR_VERTICALBLANKINPROGRESS:      return _T("DDERR_VERTICALBLANKINPROGRESS");
		case DDERR_WASSTILLDRAWING:              return _T("DDERR_WASSTILLDRAWING");
		case DDERR_XALIGN:                       return _T("DDERR_XALIGN");
		case DDERR_INVALIDDIRECTDRAWGUID:        return _T("DDERR_INVALIDDIRECTDRAWGUID");
		case DDERR_DIRECTDRAWALREADYCREATED:     return _T("DDERR_DIRECTDRAWALREADYCREATED");
		case DDERR_NODIRECTDRAWHW:               return _T("DDERR_NODIRECTDRAWHW");
		case DDERR_PRIMARYSURFACEALREADYEXISTS:  return _T("DDERR_PRIMARYSURFACEALREADYEXISTS");
		case DDERR_NOEMULATION:                  return _T("DDERR_NOEMULATION");
		case DDERR_REGIONTOOSMALL:               return _T("DDERR_REGIONTOOSMALL");
		case DDERR_CLIPPERISUSINGHWND:           return _T("DDERR_CLIPPERISUSINGHWND");
		case DDERR_NOCLIPPERATTACHED:            return _T("DDERR_NOCLIPPERATTACHED");
		case DDERR_NOHWND:                       return _T("DDERR_NOHWND");
		case DDERR_HWNDSUBCLASSED:               return _T("DDERR_HWNDSUBCLASSED");
		case DDERR_HWNDALREADYSET:               return _T("DDERR_HWNDALREADYSET");
		case DDERR_NOPALETTEATTACHED:            return _T("DDERR_NOPALETTEATTACHED");
		case DDERR_NOPALETTEHW:                  return _T("DDERR_NOPALETTEHW");
		case DDERR_BLTFASTCANTCLIP:              return _T("DDERR_BLTFASTCANTCLIP");
		case DDERR_NOBLTHW:                      return _T("DDERR_NOBLTHW");
		case DDERR_NODDROPSHW:                   return _T("DDERR_NODDROPSHW");
		case DDERR_OVERLAYNOTVISIBLE:            return _T("DDERR_OVERLAYNOTVISIBLE");
		case DDERR_NOOVERLAYDEST:                return _T("DDERR_NOOVERLAYDEST");
		case DDERR_INVALIDPOSITION:              return _T("DDERR_INVALIDPOSITION");
		case DDERR_NOTAOVERLAYSURFACE:           return _T("DDERR_NOTAOVERLAYSURFACE");
		case DDERR_EXCLUSIVEMODEALREADYSET:      return _T("DDERR_EXCLUSIVEMODEALREADYSET");
		case DDERR_NOTFLIPPABLE:                 return _T("DDERR_NOTFLIPPABLE");
		case DDERR_CANTDUPLICATE:                return _T("DDERR_CANTDUPLICATE");
		case DDERR_NOTLOCKED:                    return _T("DDERR_NOTLOCKED");
		case DDERR_CANTCREATEDC:                 return _T("DDERR_CANTCREATEDC");
		case DDERR_NODC:                         return _T("DDERR_NODC");
		case DDERR_WRONGMODE:                    return _T("DDERR_WRONGMODE");
		case DDERR_IMPLICITLYCREATED:            return _T("DDERR_IMPLICITLYCREATED");
		case DDERR_NOTPALETTIZED:                return _T("DDERR_NOTPALETTIZED");
		case DDERR_UNSUPPORTEDMODE:              return _T("DDERR_UNSUPPORTEDMODE");
		case DDERR_NOMIPMAPHW:                   return _T("DDERR_NOMIPMAPHW");
		case DDERR_INVALIDSURFACETYPE:           return _T("DDERR_INVALIDSURFACETYPE");
		case DDERR_DCALREADYCREATED:             return _T("DDERR_DCALREADYCREATED");
		case DDERR_CANTPAGELOCK:                 return _T("DDERR_CANTPAGELOCK");
		case DDERR_CANTPAGEUNLOCK:               return _T("DDERR_CANTPAGEUNLOCK");
		case DDERR_NOTPAGELOCKED:                return _T("DDERR_NOTPAGELOCKED");
		case DDERR_NOTINITIALIZED:               return _T("DDERR_NOTINITIALIZED");
	}
	return _T("Unknown Error");
}

BOOL CDxDraw::Error(HRESULT hRet, LPCTSTR lpszMessage)
{
	if (FAILED(hRet))
	{
		TCHAR buf[1024];
		_sntprintf(buf, 1024, _T("%s (%s)\n"), lpszMessage, ErrorString(hRet));
		TRACE(buf);
		return TRUE;
	}
	else
		return FALSE;
}