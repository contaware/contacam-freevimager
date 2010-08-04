#if !defined(AFX_DXDRAW_H__1E152EB4_ED1D_4079_BDD4_773383DD98C8__INCLUDED_)
#define AFX_DXDRAW_H__1E152EB4_ED1D_4079_BDD4_773383DD98C8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <ddraw.h>
#include "Dib.h"
#include "TryEnterCriticalSection.h"

// Window Message
#define WM_RESTORE_FRAME						WM_USER + 900
#define RESTORE_FRAME_RETRY_DELAY				300

// Max Device Description Size
#define MAX_DD7_DEVICE_DESCRIPTION_SIZE			256

// Max operation retry
#define DXDRAW_MAX_RETRY						4
#define DXDRAW_RETRY_SLEEP						5U

// rgb to 16 bpp macros
#define RGBTO15BITS(r,g,b) (((r&0xF8)<<7) + ((g&0xF8)<<2) + (b>>3))
#define RGBTO16BITS(r,g,b) (((r&0xF8)<<8) + ((g&0xFC)<<3) + (b>>3))

#ifndef FCC
#define FCC(ch4) ((((DWORD)(ch4) & 0xFF) << 24) |     \
				  (((DWORD)(ch4) & 0xFF00) << 8) |    \
				  (((DWORD)(ch4) & 0xFF0000) >> 8) |  \
				  (((DWORD)(ch4) & 0xFF000000) >> 24))
#endif

// Text Drawing Define
#define DRAWTEXT_BOTTOMLEFT		0
#define DRAWTEXT_TOPLEFT		1
#define DRAWTEXT_BOTTOMRIGHT	2
#define DRAWTEXT_TOPRIGHT		3
#define FONT_COLUMNS			16
#define FONT_ROWS				6
#define FONT_ASCII_OFFSET		32

// Forward Declaration
class CDib;

class CDxDraw  
{
public:

	// EnterCSTimeout() Timeout
	enum {CS_TIMEOUT = 50}; // ms

	class CScreen
	{
	public:
		CScreen() {	::ZeroMemory(&m_guidNull, sizeof(GUID));
					::ZeroMemory(&m_rcMonitor, sizeof(RECT));
					m_guid = m_guidNull;
					m_szDesc[0] = _T('\0');
					m_hMonitor = NULL;
					m_pDD = NULL;
					m_pFrontBuffer = NULL;
					m_pBackBuffer = NULL;
					m_pOffscreenBuffer = NULL;
					m_pFontBuffer = NULL;
					m_pClipper = NULL;
					m_bFirstBlt = TRUE;
					m_nSrcBpp = 0;
					m_bFlipUV = FALSE;
					m_bRgb15 = FALSE;
					m_nSrcPitch = 0;
					m_nSrcMinPitch = 0;
					m_dwSrcFourCC = 0;
					m_bSameSrcPitch = FALSE;
					m_nMonitorBpp = 0;}
		virtual ~CScreen() {;}
		GUID m_guid;
		GUID m_guidNull;
		TCHAR m_szDesc[MAX_DD7_DEVICE_DESCRIPTION_SIZE];
		HMONITOR m_hMonitor;
		RECT m_rcMonitor;
		LPDIRECTDRAW7 m_pDD;
		LPDIRECTDRAWSURFACE7 m_pFrontBuffer;
		LPDIRECTDRAWSURFACE7 m_pBackBuffer;
		LPDIRECTDRAWSURFACE7 m_pOffscreenBuffer;
		LPDIRECTDRAWSURFACE7 m_pFontBuffer;
		LPDIRECTDRAWCLIPPER m_pClipper;
		BOOL m_bFirstBlt;
		int m_nSrcBpp;
		BOOL m_bFlipUV;	// Flip the U <-> V Plains of a YV12 surface when rendering the dib
		BOOL m_bRgb15;	// Set for RGB16 with 555 format, cleared for 565 format
		int m_nSrcPitch;
		int m_nSrcMinPitch;
		BOOL m_bSameSrcPitch;
		DWORD m_dwSrcFourCC;
		int m_nMonitorBpp;
	};

	typedef HRESULT (WINAPI * LPDIRECTDRAWCREATEEX)(GUID FAR * lpGuid, LPVOID* lplpDD, REFIID iid, IUnknown FAR * pUnkOuter);
	typedef HRESULT (WINAPI * LPDIRECTDRAWENUMERATEA)(LPDDENUMCALLBACKA lpCallback, LPVOID lpContext);
	typedef CArray<CScreen*,CScreen*> SCREENARRAY;

public:
	CDxDraw();
	virtual ~CDxDraw();
	BOOL EnumerateScreens();
	BOOL Init(	HWND hWnd,
				int nSrcWidth,
				int nSrcHeight,
				DWORD dwSrcFourCC,
				UINT uiFontTableID);
	BOOL InitFullScreen(HWND hWnd,
						int nSrcWidth,
						int nSrcHeight,
						BOOL bExclusiveMode,
						DWORD dwSrcFourCC,
						UINT uiFontTableID);
	__forceinline BOOL IsFullScreen() const {return m_bFullScreen;}
	__forceinline BOOL IsFullScreenExclusive() const {return m_bFullScreenExclusive;}
	__forceinline BOOL IsTripleBuffering() const {return (m_bFullScreen && m_bTripleBuffering);}
	__forceinline BOOL HasDxDraw() const {return m_bDx7;}
	__forceinline BOOL IsInit() const {return m_bInit;}
	__forceinline void EnterCS() {m_cs.EnterCriticalSection();}
	__forceinline void LeaveCS() {m_cs.LeaveCriticalSection();}
	__forceinline BOOL EnterCSTimeout() {return m_cs.EnterCriticalSection(CS_TIMEOUT);}
	void Free();
	BOOL GetCurrentVideoMem(DWORD& dwTotal, DWORD& dwFree);
	void ClearFront(const RECT* prc = NULL);
	void ClearBack(const RECT* prc = NULL);
	void ClearOffscreen(const RECT* prc = NULL);
	int GetCurrentDevice() const {return m_nCurrentDevice;}
	int GetSrcWidth() const {return m_nSrcWidth;}
	int GetSrcHeight() const {return m_nSrcHeight;}
	CSize GetFontSize() const {return CSize(m_FontSize);}
	__forceinline BOOL FullScreenCreateOffscreen(	int nSrcWidth,
													int nSrcHeight,
													DWORD dwSrcFourCC)
	{
		BOOL bFlipUV = FALSE;

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

		return FullScreenCreateOffscreen(	nSrcWidth,
											nSrcHeight,
											dwSrcFourCC,
											bFlipUV);
	}
	__forceinline int GetCurrentSrcBpp() const	{		if (m_ScreenArray.GetSize() > 0)
															return m_ScreenArray[m_nCurrentDevice]->m_nSrcBpp;
														else
															return 0;}
	__forceinline BOOL IsCurrentSrcRgb15() const	{		if (m_ScreenArray.GetSize() > 0)
															return m_ScreenArray[m_nCurrentDevice]->m_bRgb15;
														else
															return FALSE;}
	__forceinline int GetCurrentSrcPitch() const{		if (m_ScreenArray.GetSize() > 0)
															return m_ScreenArray[m_nCurrentDevice]->m_nSrcPitch;
														else
															return 0;}
	__forceinline int GetCurrentSrcMinPitch() const{	if (m_ScreenArray.GetSize() > 0)
															return m_ScreenArray[m_nCurrentDevice]->m_nSrcMinPitch;
														else
															return 0;}
	__forceinline DWORD GetCurrentSrcFourCC() const	{	if (m_ScreenArray.GetSize() > 0)
															return m_ScreenArray[m_nCurrentDevice]->m_dwSrcFourCC;
														else
															return 0;}
	__forceinline BOOL GetCurrentSrcFlipUV() const	{	if (m_ScreenArray.GetSize() > 0)
															return m_ScreenArray[m_nCurrentDevice]->m_bFlipUV;
														else
															return FALSE;}
	__forceinline int GetCurrentMonitorBpp() const	{	if (m_ScreenArray.GetSize() > 0)
															return m_ScreenArray[m_nCurrentDevice]->m_nMonitorBpp;
														else
															return 0;}
	__forceinline BOOL HasSameSrcPitch() const	{		if (m_ScreenArray.GetSize() > 0)
															return m_ScreenArray[m_nCurrentDevice]->m_bSameSrcPitch;
														else
															return FALSE;}

	void FlipCurrentToGDISurface() {if (m_ScreenArray.GetSize() > 0)
										m_ScreenArray[m_nCurrentDevice]->m_pDD->FlipToGDISurface();}
	LPDIRECTDRAWSURFACE7 GetBackSurface() {	return ((m_ScreenArray.GetSize() > 0) ?
											m_ScreenArray[m_nCurrentDevice]->m_pBackBuffer : NULL);}
	LPDIRECTDRAWSURFACE7 GetOffscreenSurface() {return ((m_ScreenArray.GetSize() > 0) ?
												m_ScreenArray[m_nCurrentDevice]->m_pOffscreenBuffer : NULL);}
	LPDIRECTDRAWSURFACE7 GetFontSurface() {	return ((m_ScreenArray.GetSize() > 0) ?
											m_ScreenArray[m_nCurrentDevice]->m_pFontBuffer : NULL);}
	void UpdateCurrentDevice();
	BOOL Blt(RECT rcDest, RECT rcSrc);
	BOOL Flip(BOOL bDoBlt = FALSE);

	// Attention:	- Never Attach a CDC Object, it will not work!
	//				- YUV Surface does not support DC!

	__forceinline HDC GetSrcDC() 
						{if (m_ScreenArray.GetSize() > 0)
						{
							if (m_ScreenArray[m_nCurrentDevice]->m_pOffscreenBuffer)
								return GetOffscreenDC();
							else
								return GetBackDC();
						}
						else
							return NULL;
						}
	__forceinline BOOL ReleaseSrcDC(HDC hDC)
						{if (m_ScreenArray.GetSize() > 0)
						{
							if (m_ScreenArray[m_nCurrentDevice]->m_pOffscreenBuffer)
								return ReleaseOffscreenDC(hDC);
							else
								return ReleaseBackDC(hDC);
						}
						else
							return FALSE;
						}
	__forceinline HDC GetBackDC()
						{if ((m_ScreenArray.GetSize() > 0) && m_ScreenArray[m_nCurrentDevice]->m_pBackBuffer)
						{
							HDC hDC;
							for (int loop = 0 ; loop < DXDRAW_MAX_RETRY ; loop++)
							{
								HRESULT hRet = m_ScreenArray[m_nCurrentDevice]->m_pBackBuffer->GetDC(&hDC);
								if (hRet == DD_OK)
									return hDC;
								else if (hRet == DDERR_SURFACELOST)
								{
									if (!RestoreSurfaces())
										return NULL;
								}
								else if (hRet != DDERR_WASSTILLDRAWING)
								{
									Error(hRet, _T("GetBackDC()"));
									return NULL;
								}
								::Sleep(DXDRAW_RETRY_SLEEP);
							}
						}
						return NULL;
						}
	__forceinline BOOL ReleaseBackDC(HDC hDC) 
						{if ((m_ScreenArray.GetSize() > 0) && m_ScreenArray[m_nCurrentDevice]->m_pBackBuffer)
						{
							if (hDC)
							{
								for (int loop = 0 ; loop < DXDRAW_MAX_RETRY ; loop++)
								{
									HRESULT hRet = m_ScreenArray[m_nCurrentDevice]->m_pBackBuffer->ReleaseDC(hDC);
									if (hRet == DD_OK)
										return TRUE;
									else if (hRet == DDERR_SURFACELOST)
									{
										if (!RestoreSurfaces())
											return FALSE;
									}
									else if (hRet != DDERR_WASSTILLDRAWING)
									{
										Error(hRet, _T("ReleaseBackDC()"));
										return FALSE;
									}
									::Sleep(DXDRAW_RETRY_SLEEP);
								}
							}
						}
						return FALSE;
						}
	__forceinline HDC GetOffscreenDC()
						{if ((m_ScreenArray.GetSize() > 0) && m_ScreenArray[m_nCurrentDevice]->m_pOffscreenBuffer)
						{
							HDC hDC;
							for (int loop = 0 ; loop < DXDRAW_MAX_RETRY ; loop++)
							{
								HRESULT hRet = m_ScreenArray[m_nCurrentDevice]->m_pOffscreenBuffer->GetDC(&hDC);
								if (hRet == DD_OK)
									return hDC;
								else if (hRet == DDERR_SURFACELOST)
								{
									if (!RestoreSurfaces())
										return NULL;
								}
								else if (hRet != DDERR_WASSTILLDRAWING)
								{
									Error(hRet, _T("GetOffscreenDC()"));
									return NULL;
								}
								::Sleep(DXDRAW_RETRY_SLEEP);
							}
						}
						return NULL;
						}
	__forceinline BOOL ReleaseOffscreenDC(HDC hDC) 
						{if ((m_ScreenArray.GetSize() > 0) && m_ScreenArray[m_nCurrentDevice]->m_pOffscreenBuffer)
						{
							if (hDC)
							{
								for (int loop = 0 ; loop < DXDRAW_MAX_RETRY ; loop++)
								{
									HRESULT hRet = m_ScreenArray[m_nCurrentDevice]->m_pOffscreenBuffer->ReleaseDC(hDC);
									if (hRet == DD_OK)
										return TRUE;
									else if (hRet == DDERR_SURFACELOST)
									{
										if (!RestoreSurfaces())
											return FALSE;
									}
									else if (hRet != DDERR_WASSTILLDRAWING)
									{
										Error(hRet, _T("ReleaseOffscreenDC()"));
										return FALSE;
									}
									::Sleep(DXDRAW_RETRY_SLEEP);
								}
							}
						}
						return FALSE;
						}
	__forceinline HDC GetFontDC(BOOL bRestoreSurfaces = TRUE)
						{if ((m_ScreenArray.GetSize() > 0) && m_ScreenArray[m_nCurrentDevice]->m_pFontBuffer)
						{
							HDC hDC;
							for (int loop = 0 ; loop < DXDRAW_MAX_RETRY ; loop++)
							{
								// Note: this fails with DDERR_CANTCREATEDC,
								// if switching user under XP or higher
								// (no logout, ContaCam continues to run)
								HRESULT hRet = m_ScreenArray[m_nCurrentDevice]->m_pFontBuffer->GetDC(&hDC);
								if (hRet == DD_OK)
									return hDC;
								else if (hRet == DDERR_SURFACELOST)
								{
									if (bRestoreSurfaces)
									{
										if (!RestoreSurfaces())
											return NULL;
									}
									else
										return NULL;
								}
								else if (hRet != DDERR_WASSTILLDRAWING)
								{
									Error(hRet, _T("GetFontDC()"));
									return NULL;
								}
								::Sleep(DXDRAW_RETRY_SLEEP);
							}
						}
						return NULL;
						}
	__forceinline BOOL ReleaseFontDC(HDC hDC, BOOL bRestoreSurfaces = TRUE) 
						{if ((m_ScreenArray.GetSize() > 0) && m_ScreenArray[m_nCurrentDevice]->m_pFontBuffer)
						{
							if (hDC)
							{
								for (int loop = 0 ; loop < DXDRAW_MAX_RETRY ; loop++)
								{
									HRESULT hRet = m_ScreenArray[m_nCurrentDevice]->m_pFontBuffer->ReleaseDC(hDC);
									if (hRet == DD_OK)
										return TRUE;
									else if (hRet == DDERR_SURFACELOST)
									{
										if (bRestoreSurfaces)
										{
											if (!RestoreSurfaces())
												return FALSE;
										}
										else
											return FALSE;
									}
									else if (hRet != DDERR_WASSTILLDRAWING)
									{
										Error(hRet, _T("ReleaseFontDC()"));
										return FALSE;
									}
									::Sleep(DXDRAW_RETRY_SLEEP);
								}
							}
						}
						return FALSE;
						}
	__forceinline BOOL LockSrc(LPDDSURFACEDESC2 pddsd) 
						{if (m_ScreenArray.GetSize() > 0)
						{
							if (m_ScreenArray[m_nCurrentDevice]->m_pOffscreenBuffer)
								return LockOffscreen(pddsd);
							else
								return LockBack(pddsd);
						}
						else
							return FALSE;
						}
	__forceinline BOOL UnlockSrc()
						{if (m_ScreenArray.GetSize() > 0)
						{
							if (m_ScreenArray[m_nCurrentDevice]->m_pOffscreenBuffer)
								return UnlockOffscreen();
							else
								return UnlockBack();
						}
						else
							return FALSE;
						}
	__forceinline BOOL LockBack(LPDDSURFACEDESC2 pddsd) 
						{if ((pddsd != 0) && (m_ScreenArray.GetSize() > 0) && m_ScreenArray[m_nCurrentDevice]->m_pBackBuffer)
						{
							::ZeroMemory(pddsd, sizeof(DDSURFACEDESC2));
							pddsd->dwSize = sizeof(DDSURFACEDESC2);
							for (int loop = 0 ; loop < DXDRAW_MAX_RETRY ; loop++)
							{
								HRESULT hRet = m_ScreenArray[m_nCurrentDevice]->m_pBackBuffer->Lock(NULL, pddsd, DDLOCK_WAIT, 0);
								if (hRet == DD_OK)
									return TRUE;
								else if (hRet == DDERR_SURFACELOST)
								{
									if (!RestoreSurfaces())
										return FALSE;
								}
								else if (hRet != DDERR_WASSTILLDRAWING)
								{
									Error(hRet, _T("LockBack()"));
									return FALSE;
								}
								::Sleep(DXDRAW_RETRY_SLEEP);
							}
						}
						return FALSE;
						}
	__forceinline BOOL UnlockBack() 
						{if ((m_ScreenArray.GetSize() > 0) && m_ScreenArray[m_nCurrentDevice]->m_pBackBuffer)
						{
							for (int loop = 0 ; loop < DXDRAW_MAX_RETRY ; loop++)
							{
								HRESULT hRet = m_ScreenArray[m_nCurrentDevice]->m_pBackBuffer->Unlock(NULL);
								if (hRet == DD_OK)
									return TRUE;
								else if (hRet == DDERR_SURFACELOST)
								{
									if (!RestoreSurfaces())
										return FALSE;
								}
								else if (hRet != DDERR_WASSTILLDRAWING)
								{
									Error(hRet, _T("UnlockBack()"));
									return FALSE;
								}
								::Sleep(DXDRAW_RETRY_SLEEP);
							}
						}
						return FALSE;
						}
	__forceinline BOOL LockOffscreen(LPDDSURFACEDESC2 pddsd) 
						{if ((pddsd != 0) && (m_ScreenArray.GetSize() > 0) && m_ScreenArray[m_nCurrentDevice]->m_pOffscreenBuffer)
						{
							::ZeroMemory(pddsd, sizeof(DDSURFACEDESC2));
							pddsd->dwSize = sizeof(DDSURFACEDESC2);
							for (int loop = 0 ; loop < DXDRAW_MAX_RETRY ; loop++)
							{
								HRESULT hRet = m_ScreenArray[m_nCurrentDevice]->m_pOffscreenBuffer->Lock(NULL, pddsd, DDLOCK_WAIT, 0);
								if (hRet == DD_OK)
									return TRUE;
								else if (hRet == DDERR_SURFACELOST)
								{
									if (!RestoreSurfaces())
										return FALSE;
								}
								else if (hRet != DDERR_WASSTILLDRAWING)
								{
									Error(hRet, _T("LockOffscreen()"));
									return FALSE;
								}
								::Sleep(DXDRAW_RETRY_SLEEP);
							}
						}
						return FALSE;
						}
	__forceinline BOOL UnlockOffscreen() 
						{if ((m_ScreenArray.GetSize() > 0) && m_ScreenArray[m_nCurrentDevice]->m_pOffscreenBuffer)
						{
							for (int loop = 0 ; loop < DXDRAW_MAX_RETRY ; loop++)
							{
								HRESULT hRet = m_ScreenArray[m_nCurrentDevice]->m_pOffscreenBuffer->Unlock(NULL);
								if (hRet == DD_OK)
									return TRUE;
								else if (hRet == DDERR_SURFACELOST)
								{
									if (!RestoreSurfaces())
										return FALSE;
								}
								else if (hRet != DDERR_WASSTILLDRAWING)
								{
									Error(hRet, _T("UnlockOffscreen()"));
									return FALSE;
								}
								::Sleep(DXDRAW_RETRY_SLEEP);
							}
						}
						return FALSE;
						}
	__forceinline BOOL LockFont(LPDDSURFACEDESC2 pddsd) 
						{if ((pddsd != 0) && (m_ScreenArray.GetSize() > 0) && m_ScreenArray[m_nCurrentDevice]->m_pFontBuffer)
						{
							::ZeroMemory(pddsd, sizeof(DDSURFACEDESC2));
							pddsd->dwSize = sizeof(DDSURFACEDESC2);
							for (int loop = 0 ; loop < DXDRAW_MAX_RETRY ; loop++)
							{
								HRESULT hRet = m_ScreenArray[m_nCurrentDevice]->m_pFontBuffer->Lock(NULL, pddsd, DDLOCK_WAIT, 0);
								if (hRet == DD_OK)
									return TRUE;
								else if (hRet == DDERR_SURFACELOST)
								{
									if (!RestoreSurfaces())
										return FALSE;
								}
								else if (hRet != DDERR_WASSTILLDRAWING)
								{
									Error(hRet, _T("LockFont()"));
									return FALSE;
								}
								::Sleep(DXDRAW_RETRY_SLEEP);
							}
						}
						return FALSE;
						}
	__forceinline BOOL UnlockFont() 
						{if ((m_ScreenArray.GetSize() > 0) && m_ScreenArray[m_nCurrentDevice]->m_pFontBuffer)
						{
							for (int loop = 0 ; loop < DXDRAW_MAX_RETRY ; loop++)
							{
								HRESULT hRet = m_ScreenArray[m_nCurrentDevice]->m_pFontBuffer->Unlock(NULL);
								if (hRet == DD_OK)
									return TRUE;
								else if (hRet == DDERR_SURFACELOST)
								{
									if (!RestoreSurfaces())
										return FALSE;
								}
								else if (hRet != DDERR_WASSTILLDRAWING)
								{
									Error(hRet, _T("UnlockFont()"));
									return FALSE;
								}
								::Sleep(DXDRAW_RETRY_SLEEP);
							}
						}
						return FALSE;
						}
	__forceinline void ClientToMonitor(RECT* prc)
						{if (prc && (m_ScreenArray.GetSize() > 0))
						{
							POINT p = {0, 0};
							::ClientToScreen(m_hWnd, &p);
							::OffsetRect(prc, p.x, p.y);
							if (m_nCurrentDevice > 0)
							{
								OffsetRect(	prc,
											-m_ScreenArray[m_nCurrentDevice]->m_rcMonitor.left,
											-m_ScreenArray[m_nCurrentDevice]->m_rcMonitor.top);
							}
						}
						}
	__forceinline BOOL IsClippingRect(const CRect& rc)
	{
		int nWidth =	m_ScreenArray[m_nCurrentDevice]->m_rcMonitor.right - 
						m_ScreenArray[m_nCurrentDevice]->m_rcMonitor.left;
		int nHeight =	m_ScreenArray[m_nCurrentDevice]->m_rcMonitor.bottom - 
						m_ScreenArray[m_nCurrentDevice]->m_rcMonitor.top;
		if (rc.left < 0)
			return TRUE;
		if (rc.top < 0)
			return TRUE;
		if (rc.right > nWidth)
			return TRUE;
		if (rc.bottom > nHeight)
			return TRUE;
		return FALSE;
	}
	BOOL RenderDib(LPBITMAPINFO pBmi, LPBYTE pSrc, CRect rc);
	__forceinline BOOL RenderDib(CDib* pDib, CRect rc)
	{
		if (!pDib)
			return FALSE;
		else
			return RenderDib(pDib->GetBMI(), pDib->GetBits(), rc);
	}
	BOOL UpdateBackSurface(CRect rc);
	BOOL DrawText(LPCTSTR sText, int x, int y, UINT uiAlign);
	static DWORD ColorMatch(IDirectDrawSurface7* pdds, COLORREF rgb);
	static HRESULT SetSrcColorKey(IDirectDrawSurface7* pdds, COLORREF rgb);
	BOOL Error(HRESULT hRet, LPCTSTR lpszMessage);
	BOOL RestoreSurfaces();

	GUID m_guidNull;
	SCREENARRAY m_ScreenArray;

protected:
	static TCHAR* ErrorString(HRESULT hRet);
	void DeleteScreenArray();
	BOOL LoadFontDib(UINT uID);
	BOOL CopyFontDib(BOOL bRestoreSurfaces = TRUE);
	__forceinline BOOL IsCurrentSrcSameRgbFormat(LPBITMAPINFO pBmi);
	BOOL FullScreenCreateBack(DWORD dwWidth, DWORD dwHeight);
	BOOL FullScreenCreateOffscreen(	int nSrcWidth,
									int nSrcHeight,
									DWORD dwSrcFourCC,
									BOOL bFlipUV);

	volatile BOOL m_bDx7;	// DirectDraw7 or higher installed
	volatile BOOL m_bInit;
	HINSTANCE m_hDirectDraw;
	HWND m_hWnd;
	volatile int m_nSrcWidth;
	volatile int m_nSrcHeight;
	CDib m_FontDib;
	volatile BOOL m_bFontTransparent;
	SIZE m_FontTableSize;
	SIZE m_FontSize;
	volatile int m_nCurrentDevice;
	volatile BOOL m_bFullScreen;
	volatile BOOL m_bFullScreenExclusive;
	volatile BOOL m_bTripleBuffering;
	CTryEnterCriticalSection m_cs;
};

#endif // !defined(AFX_DXDRAW_H__1E152EB4_ED1D_4079_BDD4_773383DD98C8__INCLUDED_)
