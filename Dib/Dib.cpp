#include "stdafx.h"
#include "dib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma comment(lib, "vfw32.lib")

CDib::CDib()
{
	m_GetClosestColorIndexLookUp = NULL;
#ifdef VIDEODEVICEDOC
	m_hBitsSharedMemory = NULL;
	m_dwSharedMemorySize = 0;
#endif
	Init();
}

CDib::CDib(CBitmap* pBitmap, CPalette* pPal)
{
	m_GetClosestColorIndexLookUp = NULL;
#ifdef VIDEODEVICEDOC
	m_hBitsSharedMemory = NULL;
	m_dwSharedMemorySize = 0;
#endif
	Init();
	SetDibSectionFromDDB(pBitmap, pPal);
}

CDib::CDib(HBITMAP hBitmap, HPALETTE hPal)
{
	m_GetClosestColorIndexLookUp = NULL;
#ifdef VIDEODEVICEDOC
	m_hBitsSharedMemory = NULL;
	m_dwSharedMemorySize = 0;
#endif
	Init();
	SetDibSectionFromDDB(hBitmap, hPal);
}

CDib::CDib(HBITMAP hDibSection)
{
	m_GetClosestColorIndexLookUp = NULL;
#ifdef VIDEODEVICEDOC
	m_hBitsSharedMemory = NULL;
	m_dwSharedMemorySize = 0;
#endif
	Init();
	AttachDibSection(hDibSection);
}

void CDib::CopyVars(const CDib& SrcDib)
{
	m_nStretchMode = SrcDib.m_nStretchMode;
	m_bColorUndoSet = SrcDib.m_bColorUndoSet;
	m_dwImageSize = SrcDib.m_dwImageSize;
	m_wHue = SrcDib.m_wHue;
	m_wBrightness = SrcDib.m_wBrightness;
	m_wContrast = SrcDib.m_wContrast;
	m_wSaturation = SrcDib.m_wSaturation;
	m_llPts = SrcDib.m_llPts;
	m_llUpTime = SrcDib.m_llUpTime;
	m_dwUserFlag = SrcDib.m_dwUserFlag;
	m_bShowMessageBoxOnError = SrcDib.m_bShowMessageBoxOnError;
	m_bGrayscale = SrcDib.m_bGrayscale;
	m_bAlpha = SrcDib.m_bAlpha;
	m_wRedMask16 = SrcDib.m_wRedMask16;
	m_wGreenMask16 = SrcDib.m_wGreenMask16;
	m_wBlueMask16 = SrcDib.m_wBlueMask16;
	m_nGreenShift16 = SrcDib.m_nGreenShift16;
	m_nRedShift16 = SrcDib.m_nRedShift16;
	m_nGreenDownShift16 = SrcDib.m_nGreenDownShift16;
	m_nRedDownShift16 = SrcDib.m_nRedDownShift16;
	m_nBlueRoundShift16 = SrcDib.m_nBlueRoundShift16;
	m_nGreenRoundShift16 = SrcDib.m_nGreenRoundShift16;
	m_nRedRoundShift16 = SrcDib.m_nRedRoundShift16;
	m_dwRedMask32 = SrcDib.m_dwRedMask32;
	m_dwGreenMask32 = SrcDib.m_dwGreenMask32;
	m_dwBlueMask32 = SrcDib.m_dwBlueMask32;
	m_nGreenShift32 = SrcDib.m_nGreenShift32;
	m_nRedShift32 = SrcDib.m_nRedShift32;
	m_nGreenDownShift32 = SrcDib.m_nGreenDownShift32;
	m_nRedDownShift32 = SrcDib.m_nRedDownShift32;
	m_nBlueRoundShift32 = SrcDib.m_nBlueRoundShift32;
	m_nGreenRoundShift32 = SrcDib.m_nGreenRoundShift32;
	m_nRedRoundShift32 = SrcDib.m_nRedRoundShift32;
	m_bFast32bpp = SrcDib.m_bFast32bpp;
	m_crBackgroundColor = SrcDib.m_crBackgroundColor;
	m_FileInfo = SrcDib.m_FileInfo;
	if (SrcDib.m_pMetadata)						// if metadata exists in Src
		*GetMetadata() = *SrcDib.m_pMetadata;	// -> allocate Dst if necessary and copy
	else
	{											// if metadata doesn't exist in Src
		if (m_pMetadata)						// -> free Dst if allocated 
		{
			delete m_pMetadata;
			m_pMetadata = NULL;
		}
	}
	m_Gif = SrcDib.m_Gif;
}

// Note: Memory Mapped files are copied, memory is allocated for it!?
CDib::CDib(const CDib& dib) // Copy Constructor (CDib dib1 = dib2 or CDib dib1(dib2))
{
	m_GetClosestColorIndexLookUp = NULL;
#ifdef VIDEODEVICEDOC
	m_hBitsSharedMemory = NULL;
	m_dwSharedMemorySize = 0;
#endif

	// Init the object
	Init();

	// Copy Dib Section
	// Note: Must be First because it calls Free
	//       and inits m_pBMI with wrong number of colors!
	//       The Problem is that GetDIBits() only knows
	//       4, 16 and 256 colors table
	//       for the 1bpp, 4bpp and 8bpp images.
	if (dib.m_hDibSection)
	{
		if (LoadDibSection(dib.m_hDibSection))
		{
			// BMI will be allocated and initialized later
			if (m_pBMI)
			{
				delete [] m_pBMI;
				m_pBMI = NULL;
				m_pColors = NULL;
			}
		}
	}

	// Copy Vars
	CopyVars(dib);

	// Copy BMI
	if (dib.m_pBMI == NULL)
		return;
	m_pBMI = (LPBITMAPINFO)new BYTE[dib.GetBMISize()];
	if (m_pBMI == NULL)
		return;
	memcpy((void*)m_pBMI, (void*)dib.m_pBMI, dib.GetBMISize());
	if (m_pBMI->bmiHeader.biBitCount <= 8)
		m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + (WORD)(m_pBMI->bmiHeader.biSize));
	else
		m_pColors = NULL;

	// Copy Bits
	if (dib.m_pBits)
	{
		m_pBits = (LPBYTE)BIGALLOC(dib.m_dwImageSize);
		if (m_pBits == NULL)
			return;
		memcpy((void*)m_pBits, (void*)dib.m_pBits, dib.m_dwImageSize);
	}

	// Copy Orig Bits
	if (dib.m_pOrigBits)
	{
		m_pOrigBits = (LPBYTE)BIGALLOC(dib.m_dwImageSize);
		if (m_pOrigBits == NULL)
			return;
		memcpy((void*)m_pOrigBits, (void*)dib.m_pOrigBits, dib.m_dwImageSize);
	}

	// Copy Orig Colors
	if (dib.m_pOrigColors)
	{
		m_pOrigColors = (RGBQUAD*)new BYTE[sizeof(RGBQUAD)*dib.GetNumColors()];
		if (m_pOrigColors == NULL)
			return;
		memcpy((void*)m_pOrigColors, (void*)dib.m_pOrigColors, sizeof(RGBQUAD)*dib.GetNumColors());
	}

	// Copy Thumbnail Dib
	if (dib.m_pThumbnailDib)
	{
		m_pThumbnailDib = (CDib*)new CDib;
		if (m_pThumbnailDib == NULL)
			return;
		*m_pThumbnailDib = *(dib.m_pThumbnailDib);
		m_dThumbnailDibRatio = dib.m_dThumbnailDibRatio;
	}

	// Copy Preview Dib
	if (dib.m_pPreviewDib)
	{
		m_pPreviewDib = (CDib*)new CDib;
		if (m_pPreviewDib == NULL)
			return;
		*m_pPreviewDib = *(dib.m_pPreviewDib);
		m_dPreviewDibRatio = dib.m_dPreviewDibRatio;
	}

	// Create Palette
	if (dib.m_pPalette)
		CreatePaletteFromBMI();
}

// Note: Memory Mapped files are copied, memory is allocated for it!?
CDib& CDib::operator=(const CDib& dib) // Copy Assignment (CDib dib3; dib3 = dib1)
{
	if (this != &dib) // beware of self-assignment!
	{
		// Clean & Init the object
		Free();
		Init();

		// Copy Dib Section
		// Note: Must be First because it calls Free
		//       and inits m_pBMI with wrong number of colors!
		//       The Problem is that GetDIBits() only knows
		//       4, 16 and 256 colors table
		//       for the 1bpp, 4bpp and 8bpp images.
		if (dib.m_hDibSection)
		{
			if (LoadDibSection(dib.m_hDibSection))
			{
				// BMI will be allocated and initialized later
				if (m_pBMI)
				{
					delete [] m_pBMI;
					m_pBMI = NULL;
					m_pColors = NULL;
				}
			}
		}

		// Copy Vars
		CopyVars(dib);
		
		// Copy BMI
		if (dib.m_pBMI == NULL)
			return *this;
		m_pBMI = (LPBITMAPINFO)new BYTE[dib.GetBMISize()];
		if (m_pBMI == NULL)
			return *this;
		memcpy((void*)m_pBMI, (void*)dib.m_pBMI, dib.GetBMISize());
		if (m_pBMI->bmiHeader.biBitCount <= 8)
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + (WORD)(m_pBMI->bmiHeader.biSize));
		else
			m_pColors = NULL;

		// Copy Bits
		if (dib.m_pBits)
		{
			m_pBits = (LPBYTE)BIGALLOC(dib.m_dwImageSize);
			if (m_pBits == NULL)
				return *this;
			memcpy((void*)m_pBits, (void*)dib.m_pBits, dib.m_dwImageSize);
		}

		// Copy Orig Bits
		if (dib.m_pOrigBits)
		{
			m_pOrigBits = (LPBYTE)BIGALLOC(dib.m_dwImageSize);
			if (m_pOrigBits == NULL)
				return *this;
			memcpy((void*)m_pOrigBits, (void*)dib.m_pOrigBits, dib.m_dwImageSize);
		}

		// Copy Orig Colors
		if (dib.m_pOrigColors)
		{
			m_pOrigColors = (RGBQUAD*)new BYTE[sizeof(RGBQUAD)*dib.GetNumColors()];
			if (m_pOrigColors == NULL)
				return *this;
			memcpy((void*)m_pOrigColors, (void*)dib.m_pOrigColors, sizeof(RGBQUAD)*dib.GetNumColors());
		}

		// Copy Thumbnail Dib
		if (dib.m_pThumbnailDib && this != dib.m_pThumbnailDib)
		{
			m_pThumbnailDib = (CDib*)new CDib;
			if (m_pThumbnailDib == NULL)
				return *this;
			*m_pThumbnailDib = *(dib.m_pThumbnailDib);
			m_dThumbnailDibRatio = dib.m_dThumbnailDibRatio;
		}

		// Copy Preview Dib
		if (dib.m_pPreviewDib && this != dib.m_pPreviewDib)
		{
			m_pPreviewDib = (CDib*)new CDib;
			if (m_pPreviewDib == NULL)
				return *this;
			*m_pPreviewDib = *(dib.m_pPreviewDib);
			m_dPreviewDibRatio = dib.m_dPreviewDibRatio;
		}

		// Create Palette
		if (dib.m_pPalette)
			CreatePaletteFromBMI();
	}
	return *this;
}

CDib::~CDib()
{
	Free();
	FreeGetClosestColorIndex();
#ifdef VIDEODEVICEDOC
	FreeUserList();
	if (m_hBitsSharedMemory)
	{
		m_llOverallSharedMemoryBytes -= (LONGLONG)m_dwSharedMemorySize;
		::CloseHandle(m_hBitsSharedMemory);
	}
#endif
}

void CDib::FreeArray(CDib::ARRAY& a)
{
	for (int i = 0 ; i < a.GetSize() ; i++)
	{
		if (a[i])
			delete a[i];
	}
	a.RemoveAll();
}

void CDib::FreeList(CDib::LIST& l)
{
	while (!l.IsEmpty())
	{
		if (l.GetTail())
			delete l.GetTail();
		l.RemoveTail();
	}
}

#ifdef VIDEODEVICEDOC
DWORD CDib::BitsToSharedMemory()
{
	// Check
	if (m_hBitsSharedMemory)
		return ERROR_INVALID_PARAMETER;
	if (m_dwSharedMemorySize > 0)
		return ERROR_INVALID_PARAMETER;

	// Calculate Shared Memory Size
	DWORD dwSharedMemorySize = CalcSharedMemorySize();

	// Create file mapping
	//
	// RAM is a limited resource, whereas for most practical purposes, virtual memory is
	// unlimited. There can be many processes, and each 32-bit process has its own 2 GB of private
	// virtual address space. When the memory being used by all the existing processes exceeds
	// the available RAM, the operating system moves pages (4-KB pieces) of one or more virtual
	// address spaces to the computer's hard disk. This frees that RAM frame for other uses.
	//
	// CreateFileMapping creates a file mapping object of a specified size that is also backed
	// by the system paging file, which means that "if the system needs to page this memory out,
	// it will store it in the system paging file, otherwise it remains in RAM". This is
	// exactly what happens also to regular memory allocated by HeapAlloc and VirtualAlloc.
	//
	// By default, page files are system-managed. This means that the page files increase /
	// decrease based on the amount of physical memory installed and when the system
	// commit charge is more / less than 90% of the current commit limit.
	// This continues to occur until the page file reaches three times the size of physical
	// memory or 4 GB, whichever is larger.
	//
	// On manually-managed systems the page files increase / decrease between the set limits
	// depending from the current commit charge.
	//
	// The maximum of physical memory + paging files is 16TB on 32 bit systems with PAE
	// enabled (usually all systems that have DEP enabled) and 16TB on all 64 bit systems.
	//
	// Windows limits the sum of all types of handles to 16x1024x1024 per-process, that
	// consumes 128 MB on 32 bit systems or 256 MB on 64 bit system of Paged Pool memory.
	// -> we have enough handles that we can use and they will not consume too much memory!
	//
	// References
	// https://blogs.msdn.microsoft.com/oldnewthing/20130301-00/?p=5093
	// https://support.microsoft.com/en-us/help/2160852/ram-virtual-memory-pagefile-and-memory-management-in-windows
	// https://support.microsoft.com/en-us/kb/2860880
	// https://blogs.technet.microsoft.com/markrussinovich/2009/09/29/pushing-the-limits-of-windows-handles/
	HANDLE mapping = ::CreateFileMapping(	INVALID_HANDLE_VALUE,	// in shared memory
											NULL,					// default security
											PAGE_READWRITE,			// read/write access
											0,						// maximum object size (high-order DWORD)
											dwSharedMemorySize,		// maximum object size (low-order DWORD)
											NULL);					// no name for mapping object
	if (mapping == NULL)
		return ::GetLastError();

	// Map
	void* region = ::MapViewOfFile(	mapping,				// handle to map object
									FILE_MAP_ALL_ACCESS,	// read/write permission
									0,						// offset high
									0,						// offset low, the offset must be a multiple of the allocation granularity
									dwSharedMemorySize);	// number of bytes to map
	if (region == NULL)
	{
		DWORD dwLastError = ::GetLastError();
		::CloseHandle(mapping);	// free if we cannot map into address space
		return dwLastError;
	}

	// Copy image bits
	LPBYTE p = (LPBYTE)region;
	if (GetImageSize() > 0)
	{
		if (m_pBits)
			memcpy(p, m_pBits, GetImageSize());
		p += GetImageSize();
	}

	// Copy User buffers
	POSITION pos = m_UserList.GetHeadPosition();
	while (pos)
	{
		CUserBuf& UserBuf = m_UserList.GetNext(pos);
		if (UserBuf.m_dwSize > 0)
		{
			if (UserBuf.m_pBuf)
				memcpy(p, UserBuf.m_pBuf, UserBuf.m_dwSize);
			p += UserBuf.m_dwSize;
		}
	}

	// Unmap
	::UnmapViewOfFile(region);

	// Free memory
	if (m_pBits)
	{
		BIGFREE(m_pBits);
		m_pBits = NULL;
	}
	ResetColorUndo();
	pos = m_UserList.GetHeadPosition();
	while (pos)
	{
		CUserBuf& UserBuf = m_UserList.GetNext(pos);
		if (UserBuf.m_pBuf)
		{
			av_free(UserBuf.m_pBuf);
			UserBuf.m_pBuf = NULL;
		}
	}

	// Stats
	m_llOverallSharedMemoryBytes += (LONGLONG)dwSharedMemorySize;

	// Store mapping
	m_hBitsSharedMemory = mapping;
	m_dwSharedMemorySize = dwSharedMemorySize;

    return ERROR_SUCCESS;
}

DWORD CDib::SharedMemoryToBits()
{
	// Check
	if (!m_hBitsSharedMemory)
		return ERROR_INVALID_PARAMETER;
	if (m_dwSharedMemorySize == 0)
		return ERROR_INVALID_PARAMETER;

	// Map
	void* region = ::MapViewOfFile(	m_hBitsSharedMemory,	// handle to map object
									FILE_MAP_ALL_ACCESS,	// read/write permission
									0,						// offset high
									0,						// offset low, the offset must be a multiple of the allocation granularity
									m_dwSharedMemorySize);	// number of bytes to map
	if (region == NULL)
		return ::GetLastError();

	// Copy image bits
	LPBYTE p = (LPBYTE)region;
	if (GetImageSize() > 0)
	{
		// Before allocating make sure nothing was left over
		// from a previously failed SharedMemoryToBits() call
		if (m_pBits)
			BIGFREE(m_pBits);
		m_pBits = (LPBYTE)BIGALLOC(GetImageSize());
		if (!m_pBits)
		{
			::UnmapViewOfFile(region);
			return ERROR_OUTOFMEMORY;
		}
		memcpy(m_pBits, p, GetImageSize());
		p += GetImageSize();
	}

	// Copy User buffers
	POSITION pos = m_UserList.GetHeadPosition();
	while (pos)
	{
		CUserBuf& UserBuf = m_UserList.GetNext(pos);
		if (UserBuf.m_dwSize > 0)
		{
			// Before allocating make sure nothing was left over
			// from a previously failed SharedMemoryToBits() call
			if (UserBuf.m_pBuf)
				av_free(UserBuf.m_pBuf);
			UserBuf.m_pBuf = (LPBYTE)av_malloc(UserBuf.m_dwSize);
			if (!UserBuf.m_pBuf)
			{
				::UnmapViewOfFile(region);
				return ERROR_OUTOFMEMORY;
			}
			memcpy(UserBuf.m_pBuf, p, UserBuf.m_dwSize);
			p += UserBuf.m_dwSize;
		}
	}

	// Unmap
	::UnmapViewOfFile(region);

	// Stats
	m_llOverallSharedMemoryBytes -= (LONGLONG)m_dwSharedMemorySize;

	// Free mapping
	::CloseHandle(m_hBitsSharedMemory);
	m_hBitsSharedMemory = NULL;
	m_dwSharedMemorySize = 0;

	return ERROR_SUCCESS;
}

void CDib::CopyUserList(const USERLIST& UserList)
{
	FreeUserList();
	POSITION pos = UserList.GetHeadPosition();
	while (pos)
	{
		CUserBuf UserBuf = UserList.GetNext(pos);
		if (UserBuf.m_pBuf && UserBuf.m_dwSize > 0)
		{
			LPBYTE p = (LPBYTE)av_malloc(UserBuf.m_dwSize);
			if (p)
				memcpy(p, UserBuf.m_pBuf, UserBuf.m_dwSize);
			UserBuf.m_pBuf = p;
			m_UserList.AddTail(UserBuf);
		}
	}
}

void CDib::MoveUserList(USERLIST& UserList)
{
	FreeUserList();
	while (!UserList.IsEmpty())
		m_UserList.AddTail(UserList.RemoveHead());
}

void CDib::FreeUserList()
{
	while (!m_UserList.IsEmpty())
	{
		CUserBuf UserBuf = m_UserList.RemoveHead();
		if (UserBuf.m_pBuf)
			av_free(UserBuf.m_pBuf);
	}
}
#endif

BOOL CDib::IsFile(LPCTSTR lpszFileName)
{
	DWORD dwAttrib = ::GetFileAttributes(lpszFileName);
	if (dwAttrib != 0xFFFFFFFF)
	{
		// Directory
		if (!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
			return TRUE;
		else
			return FALSE;
	}
	else
		return FALSE;
}

BOOL CDib::IsBilevelAlpha()
{
	// Check
	if (!m_pBMI || !m_pBits)
		return FALSE;

	// Has Alpha Channel?
	if (HasAlpha() && GetBitCount() == 32)
	{
		DWORD uiDIBScanLineSize32 = DWALIGNEDWIDTHBYTES(GetWidth() * 32);
		LPBYTE lpAlphaBits = GetBits();
		DWORD dwOpaqueCount = 0;
		DWORD dwTransparentCount = 0;

		// Count the Number of Alpha Values
		for (unsigned int y = 0 ; y < GetHeight() ; y++)
		{
			for (unsigned int x = 0 ; x < GetWidth() ; x++)
			{
				BYTE A = lpAlphaBits[4*x + 3 + y*uiDIBScanLineSize32];
				if (A == 0)
					dwTransparentCount++;
				else if (A == 255)
					dwOpaqueCount++;
				else
					return FALSE;
			}
		}

		// If it has only fully transparent values
		// and fully opaque values then it is a
		// bilevel alpha channel
		if (dwTransparentCount > 0 && dwOpaqueCount > 0)
			return TRUE;
		else
			return FALSE;
	}
	else
		return FALSE;
}

BOOL CDib::Paint(HDC hDC,
				 const LPRECT lpDCRect,
				 const LPRECT lpDIBRect,
				 BOOL bForceStretch/*=FALSE*/)
{
	BOOL bSuccess = FALSE;
	HPALETTE hPal = NULL;           // Our DIB's palette
	HPALETTE hOldPal = NULL;        // Previous palette

	if (UsesPalette(hDC))
	{
		hPal = (HPALETTE)GetPalette()->GetSafeHandle();

		// Select as background since we have
		// already realized in forground if needed
		hOldPal = ::SelectPalette(hDC, hPal, TRUE);
	}

	if (m_pBits)
	{
		// Determine whether to call StretchDIBits() or SetDIBitsToDevice()
		if ((RECTWIDTH(lpDCRect) == RECTWIDTH(lpDIBRect)) &&
			(RECTHEIGHT(lpDCRect) == RECTHEIGHT(lpDIBRect)) &&
			(RC_DIBTODEV & ::GetDeviceCaps(hDC, RASTERCAPS)) && !bForceStretch)
		{		
			// ATTENTION: Here the Src Point (SrcX, SrcY) is the left-bottom point and
			//            not the left-top point like with the other functions (BitBlt, StretchBlt, DrawDibDraw) !!!
			bSuccess = ::SetDIBitsToDevice(hDC,						// hDC
									lpDCRect->left,					// DestX
									lpDCRect->top,					// DestY
									RECTWIDTH(lpDIBRect),			// nSrcWidth
									RECTHEIGHT(lpDIBRect),			// nSrcHeight
									lpDIBRect->left,				// SrcX
									GetHeight() - lpDIBRect->bottom,// SrcY
									0,								// nStartScan
									GetHeight(),					// nNumScans
									m_pBits,						// lpBits
									m_pBMI,							// lpBitsInfo
									DIB_RGB_COLORS);				// wUsage
		}
		else if ((RC_STRETCHDIB & ::GetDeviceCaps(hDC, RASTERCAPS)))
		{
			// ATTENTION: Here the Src Point (SrcX, SrcY) is the left-bottom point and
			//            not the left-top point like with the other functions (BitBlt, StretchBlt, DrawDibDraw) !!!
			int nOldStretchMode = ::SetStretchBltMode(hDC, m_nStretchMode);
			bSuccess = ::StretchDIBits(hDC,							// hDC
									lpDCRect->left,					// DestX
									lpDCRect->top,					// DestY
									RECTWIDTH(lpDCRect),			// nDestWidth
									RECTHEIGHT(lpDCRect),			// nDestHeight
									lpDIBRect->left,				// SrcX
									GetHeight() - lpDIBRect->bottom,// SrcY
									RECTWIDTH(lpDIBRect),			// wSrcWidth
									RECTHEIGHT(lpDIBRect),			// wSrcHeight
									m_pBits,						// lpBits
									m_pBMI,							// lpBitsInfo
									DIB_RGB_COLORS,					// wUsage
									SRCCOPY);						// dwROP
			::SetStretchBltMode(hDC, nOldStretchMode);
		}
	}
	else if (m_hDibSection)
	{
		HPALETTE hOldPalMemDC = NULL;
		HDC memDC = ::CreateCompatibleDC(hDC);
		HBITMAP hOldBitmap = (HBITMAP)::SelectObject(memDC, m_hDibSection);
		if (hPal)
			hOldPalMemDC = ::SelectPalette(memDC, hPal, TRUE);// Select as background since we have already realized in forground if needed
		
		if ((RECTWIDTH(lpDCRect) == RECTWIDTH(lpDIBRect)) &&
			(RECTHEIGHT(lpDCRect) == RECTHEIGHT(lpDIBRect)) &&
			(RC_BITBLT & ::GetDeviceCaps(hDC, RASTERCAPS)) && !bForceStretch)
		{
			bSuccess = ::BitBlt(hDC, lpDCRect->left, lpDCRect->top, 
							RECTWIDTH(lpDCRect), RECTHEIGHT(lpDCRect), memDC,
							lpDIBRect->left, lpDIBRect->top, 
							SRCCOPY);
		}
		else if ((RC_STRETCHBLT & ::GetDeviceCaps(hDC, RASTERCAPS)))
		{
			int nOldStretchMode = ::SetStretchBltMode(hDC, m_nStretchMode);
			bSuccess = ::StretchBlt(hDC, lpDCRect->left, lpDCRect->top, 
							RECTWIDTH(lpDCRect), RECTHEIGHT(lpDCRect), memDC,
							lpDIBRect->left, lpDIBRect->top, 
							RECTWIDTH(lpDIBRect), RECTHEIGHT(lpDIBRect), SRCCOPY);
			::SetStretchBltMode(hDC, nOldStretchMode);
		}
		
		::SelectObject(memDC, hOldBitmap);
		if (hOldPalMemDC != NULL)
			::SelectPalette(memDC, hOldPalMemDC, TRUE);
		::DeleteDC(memDC);
	}

	// Reselect old palette
	if (hOldPal != NULL)
		::SelectPalette(hDC, hOldPal, TRUE);

	return bSuccess;
}

BOOL CDib::CreateHalftonePalette(CPalette* pPal, int nNumColors)
{
	int i;

    // Sanity check on requested number of colours
    if (nNumColors <= 0 || nNumColors > 256)
        nNumColors = 256;

	if (!pPal)
		return FALSE;

	// Allocate memory block for logical palette
	LPLOGPALETTE lpPal = (LPLOGPALETTE)new BYTE[sizeof(LOGPALETTE) +
												sizeof(PALETTEENTRY)*nNumColors];
	if (!lpPal)
		return FALSE;

	// Set version and number of palette entries
	lpPal->palVersion = PALVERSION_DEFINE;
	lpPal->palNumEntries = (WORD)nNumColors;

	int nCurrentColor = 1;

    if (nNumColors <= 2)
    {
		// B & W
		lpPal->palPalEntry[0].peRed   = ms_StdColors[0].rgbRed;
		lpPal->palPalEntry[0].peGreen = ms_StdColors[0].rgbGreen;
		lpPal->palPalEntry[0].peBlue  = ms_StdColors[0].rgbBlue;
		lpPal->palPalEntry[0].peFlags = 0;
		if (++nCurrentColor > nNumColors)
		{
			BOOL bResult = pPal->CreatePalette(lpPal);
			delete [] lpPal;
			return bResult;
		}

		lpPal->palPalEntry[1].peRed   = ms_StdColors[255].rgbRed;
		lpPal->palPalEntry[1].peGreen = ms_StdColors[255].rgbGreen;
		lpPal->palPalEntry[1].peBlue  = ms_StdColors[255].rgbBlue;
		lpPal->palPalEntry[1].peFlags = 0;
	}
	else if (nNumColors <= 16)
	{
		// Microsoft Windows Standard 16-colors palette
		// Backward compatible palette with the CGA text mode,
		// but with colors arranged in a different order.
		// This is the default palette for 16 color icons and toolbars.
		//
		// Index      RGB Hex    RGB Decimal    Color Name
		// -----      -------    -----------    ----------
		//   0        #000000    0,0,0          black
		//   1        #800000    128,0,0        maroon
		//   2        #008000    0,128,0        green
		//   3        #808000    128,128,0      olive
		//   4        #000080    0,0,128        navy
		//   5        #800080    128,0,128      purple
		//   6        #008080    0,128,128      teal
		//   7        #C0C0C0    192,192,192    silver
		//   8        #808080    128,128,128    gray
		//   9        #FF0000    255,0,0        red
		//  10        #00FF00    0,255,0        lime
		//  11        #FFFF00    255,255,0      yellow
		//  12        #0000FF    0,0,255        blue
		//  13        #FF00FF    255,0,255      fuchsia
		//  14        #00FFFF    0,255,255      aqua
		//  15        #FFFFFF    255,255,255    white
		for (i = 0; i < 8; i++)
		{
			lpPal->palPalEntry[i].peRed   = ms_StdColors[i].rgbRed;
			lpPal->palPalEntry[i].peGreen = ms_StdColors[i].rgbGreen;
			lpPal->palPalEntry[i].peBlue  = ms_StdColors[i].rgbBlue;
			lpPal->palPalEntry[i].peFlags = 0;
			if (++nCurrentColor > nNumColors)
			{
				BOOL bResult = pPal->CreatePalette(lpPal);
				delete [] lpPal;
				return bResult;
			}
		}
		for (i = 8; i < 16; i++)
		{
			lpPal->palPalEntry[i].peRed   = ms_StdColors[240+i].rgbRed;
			lpPal->palPalEntry[i].peGreen = ms_StdColors[240+i].rgbGreen;
			lpPal->palPalEntry[i].peBlue  = ms_StdColors[240+i].rgbBlue;
			lpPal->palPalEntry[i].peFlags = 0;
			if (++nCurrentColor > nNumColors)
			{
				BOOL bResult = pPal->CreatePalette(lpPal);
				delete [] lpPal;
				return bResult;
			}
		}
	}
	else
	{
		// Fill palette with full halftone palette
		for (i = 0; i < 256; i++)
		{
			lpPal->palPalEntry[i].peRed   = ms_StdColors[i].rgbRed;
			lpPal->palPalEntry[i].peGreen = ms_StdColors[i].rgbGreen;
			lpPal->palPalEntry[i].peBlue  = ms_StdColors[i].rgbBlue;
			lpPal->palPalEntry[i].peFlags = 0;
			if (++nCurrentColor > nNumColors)
			{
				BOOL bResult = pPal->CreatePalette(lpPal);
				delete [] lpPal;
				return bResult;
			}
		}
	}

	BOOL bResult = pPal->CreatePalette(lpPal);
	delete [] lpPal;
	return bResult;
}

BOOL CDib::FillHalftoneColors(RGBQUAD* pColors, int nNumColors)
{
	int i;

	if (!pColors)
		return FALSE;

	if (nNumColors <= 0)
		return FALSE;

	int nCurrentColor = 1;

    // Sanity check on requested number of colours
    if ((nNumColors > 256) || (nNumColors <= 0))
		return FALSE;
   
    if (nNumColors <= 2)
    {
		// B & W
		pColors[0].rgbRed = ms_StdColors[0].rgbRed;
		pColors[0].rgbGreen = ms_StdColors[0].rgbGreen;
		pColors[0].rgbBlue  = ms_StdColors[0].rgbBlue;
		pColors[0].rgbReserved = 0;
		if (++nCurrentColor > nNumColors)
			return TRUE; 

		pColors[1].rgbRed = ms_StdColors[255].rgbRed;
		pColors[1].rgbGreen = ms_StdColors[255].rgbGreen;
		pColors[1].rgbBlue  = ms_StdColors[255].rgbBlue;
		pColors[1].rgbReserved = 0;
	}
	else if (nNumColors <= 16)
	{
		// Microsoft Windows Standard 16-colors palette
		// Backward compatible palette with the CGA text mode,
		// but with colors arranged in a different order.
		// This is the default palette for 16 color icons and toolbars.
		//
		// Index      RGB Hex    RGB Decimal    Color Name
		// -----      -------    -----------    ----------
		//   0        #000000    0,0,0          black
		//   1        #800000    128,0,0        maroon
		//   2        #008000    0,128,0        green
		//   3        #808000    128,128,0      olive
		//   4        #000080    0,0,128        navy
		//   5        #800080    128,0,128      purple
		//   6        #008080    0,128,128      teal
		//   7        #C0C0C0    192,192,192    silver
		//   8        #808080    128,128,128    gray
		//   9        #FF0000    255,0,0        red
		//  10        #00FF00    0,255,0        lime
		//  11        #FFFF00    255,255,0      yellow
		//  12        #0000FF    0,0,255        blue
		//  13        #FF00FF    255,0,255      fuchsia
		//  14        #00FFFF    0,255,255      aqua
		//  15        #FFFFFF    255,255,255    white
		for (i = 0 ; i < 8 ; i++)
		{
			pColors[i].rgbRed = ms_StdColors[i].rgbRed;
			pColors[i].rgbGreen = ms_StdColors[i].rgbGreen;
			pColors[i].rgbBlue  = ms_StdColors[i].rgbBlue;
			pColors[i].rgbReserved = 0;
			if (++nCurrentColor > nNumColors)
				return TRUE; 
		}
		for (i = 8 ; i < 16 ; i++)
		{
			pColors[i].rgbRed = ms_StdColors[240+i].rgbRed;
			pColors[i].rgbGreen = ms_StdColors[240+i].rgbGreen;
			pColors[i].rgbBlue  = ms_StdColors[240+i].rgbBlue;
			pColors[i].rgbReserved = 0;
			if (++nCurrentColor > nNumColors)
				return TRUE; 
		}
	}
	else
	{
		// Fill colors with full halftone palette
		for (i = 0 ; i < 256 ; i++)
		{
			pColors[i].rgbRed = ms_StdColors[i].rgbRed;
			pColors[i].rgbGreen = ms_StdColors[i].rgbGreen;
			pColors[i].rgbBlue  = ms_StdColors[i].rgbBlue;
			pColors[i].rgbReserved = 0;
			if (++nCurrentColor > nNumColors)
				return TRUE;
		}
	}	

	return TRUE;
}

BOOL CDib::FillGrayscaleColors(RGBQUAD* pColors, int nNumColors)
{
	if (!pColors)
		return FALSE;

	if (nNumColors <= 0)
		return FALSE;

	int nCurrentColor = 1;

    // Sanity check on requested number of colours
    if ((nNumColors > 256) || (nNumColors == 0))
		return FALSE;
   
    if (nNumColors <= 2)
    {
		pColors[0].rgbRed = 0;
		pColors[0].rgbGreen = 0;
		pColors[0].rgbBlue  = 0;
		pColors[0].rgbReserved = 0;
		if (++nCurrentColor > nNumColors)
			return TRUE; 

		pColors[1].rgbRed = 255;
		pColors[1].rgbGreen = 255;
		pColors[1].rgbBlue  = 255;
		pColors[1].rgbReserved = 0;
	}
	else if (nNumColors <= 16)
	{
		for (int i = 0 ; i < 16 ; i++)
		{
			pColors[i].rgbRed = i * 255 / 15;
			pColors[i].rgbGreen = i * 255 / 15;
			pColors[i].rgbBlue  = i * 255 / 15;
			pColors[i].rgbReserved = 0;
			if (++nCurrentColor > nNumColors)
				return TRUE; 
		}
	}
	else
	{
		for (int i = 0 ; i < 256 ; i++)
		{
			pColors[i].rgbRed = i;
			pColors[i].rgbGreen = i;
			pColors[i].rgbBlue  = i;
			pColors[i].rgbReserved = 0;
			if (++nCurrentColor > nNumColors)
				return TRUE;
		}
	}	

	return TRUE;
}

CString CDib::GetNumColorsName()
{
	if (GetBitCount() <= 8)
	{
		CString s;
		if (IsGrayscale())
			s.Format(_T("%d Gray Levels"), GetNumColors());
		else
			s.Format(_T("%d Colors"), GetNumColors());
		return s;
	}
	else if (GetBitCount() == 16)
	{
		if (m_pBMI->bmiHeader.biCompression == BI_BITFIELDS)
		{
			LPBITMAPINFOBITFIELDS pBmiBf = (LPBITMAPINFOBITFIELDS)m_pBMI;
			if ((pBmiBf->biBlueMask == 0x001F)	&&
				(pBmiBf->biGreenMask == 0x07E0)	&&
				(pBmiBf->biRedMask == 0xF800))
				return _T("65536 Col.");
			else
				return _T("32768 Col.");
		}
		else
			return _T("32768 Col.");
	}
	else
		return _T("True Colors");
}

CString CDib::GetNumColorsName(LPBITMAPINFO pBMI)
{
	if (!pBMI)
		return _T("");

	if (pBMI->bmiHeader.biBitCount <= 8)
	{
		CString s;
		s.Format(_T("%d Colors"), GetNumColors(pBMI));
		return s;
	}
	else if (pBMI->bmiHeader.biBitCount == 16)
	{
		if (pBMI->bmiHeader.biCompression == BI_BITFIELDS)
		{
			LPBITMAPINFOBITFIELDS pBmiBf = (LPBITMAPINFOBITFIELDS)pBMI;
			if ((pBmiBf->biBlueMask == 0x001F)	&&
				(pBmiBf->biGreenMask == 0x07E0)	&&
				(pBmiBf->biRedMask == 0xF800))
				return _T("65536 Col.");
			else
				return _T("32768 Col.");
		}
		else
			return _T("32768 Col.");
	}
	else
		return _T("True Colors");
}

// Set color (or index)
BOOL CDib::SetBitColors(COLORREF crColor)
{
	if (!m_pBits)
	{
		if (!DibSectionToBits())
			return FALSE;
	}

	if (!m_pBits || !m_pBMI)
		return FALSE;

	if ((GetCompression() == BI_RGB) ||
		(GetCompression() == BI_BITFIELDS))
	{
		DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * GetBitCount());
		if (GetBitCount() == 1)
		{
			if (crColor)
				memset(m_pBits, 0xFF, uiDIBScanLineSize * GetHeight());
			else
				memset(m_pBits, 0, uiDIBScanLineSize * GetHeight());
		}
		else if (GetBitCount() == 4)
			memset(m_pBits, (crColor<<4) | crColor, uiDIBScanLineSize * GetHeight());
		else if (GetBitCount() == 8)
			memset(m_pBits, crColor, uiDIBScanLineSize * GetHeight());
		else if (GetBitCount() == 16)
		{
			if ((crColor == 0) && (GetCompression() == BI_RGB))
				memset(m_pBits, 0, uiDIBScanLineSize * GetHeight());
			else
			{
				// Fill One Row
				for (unsigned int x = 0 ; x < GetWidth() ; x++)
					SetPixelColor(x, 0, crColor);

				// memcpy other rows
				for (unsigned int y = 1 ; y < GetHeight() ; y++)
					memcpy(m_pBits + y * uiDIBScanLineSize, m_pBits, uiDIBScanLineSize);
			}
		}
		else if (GetBitCount() == 24)
		{
			if ((GetRValue(crColor) == GetGValue(crColor)) &&
				(GetGValue(crColor) == GetBValue(crColor)))
				memset(m_pBits, GetRValue(crColor), uiDIBScanLineSize * GetHeight());
			else
			{
				// Fill One Row
				for (unsigned int x = 0 ; x < GetWidth() ; x++)
				{
					m_pBits[3*x]	= GetBValue(crColor);
					m_pBits[3*x+1]	= GetGValue(crColor);
					m_pBits[3*x+2]	= GetRValue(crColor);
				}

				// memcpy other rows
				for (unsigned int y = 1 ; y < GetHeight() ; y++)
					memcpy(m_pBits + y * uiDIBScanLineSize, m_pBits, uiDIBScanLineSize);
			}
		}
		else if (GetBitCount() == 32)
		{
			if ((GetRValue(crColor) == GetGValue(crColor)) &&
				(GetGValue(crColor) == GetBValue(crColor)) &&
				(GetBValue(crColor) == GetAValue(crColor)) &&
				(GetCompression() == BI_RGB))
				memset(m_pBits, GetRValue(crColor), uiDIBScanLineSize * GetHeight());
			else
			{
				// Fill One Row
				for (unsigned int x = 0 ; x < GetWidth() ; x++)
					SetPixelColor32Alpha(x, 0, crColor);

				// memcpy other rows
				for (unsigned int y = 1 ; y < GetHeight() ; y++)
					memcpy(m_pBits + y * uiDIBScanLineSize, m_pBits, uiDIBScanLineSize);
			}
		}
		else
			memset(m_pBits, (int)crColor, m_dwImageSize);
	}
	else
		memset(m_pBits, (int)crColor, m_dwImageSize);

	return TRUE;
}

// Set color (or index)
BOOL CDib::SetDibSectionColors(COLORREF crColor)
{
	if (!m_hDibSection)
	{
		if (!BitsToDibSection())
			return FALSE;
	}

	if (!m_pDibSectionBits || !m_pBMI)
		return FALSE;

	DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * GetBitCount());

	if (GetBitCount() == 1)
	{
		if (crColor)
			memset(m_pDibSectionBits, 0xFF, uiDIBScanLineSize * GetHeight());
		else
			memset(m_pDibSectionBits, 0, uiDIBScanLineSize * GetHeight());
	}
	else if (GetBitCount() == 4)
		memset(m_pDibSectionBits, (crColor<<4) | crColor, uiDIBScanLineSize * GetHeight());
	else if (GetBitCount() == 8)
		memset(m_pDibSectionBits, crColor, uiDIBScanLineSize * GetHeight());
	else if (GetBitCount() == 16)
	{
		if ((crColor == 0) && (GetCompression() == BI_RGB))
			memset(m_pDibSectionBits, 0, uiDIBScanLineSize * GetHeight());
		else
		{
			// Fill One Row
			for (unsigned int x = 0 ; x < GetWidth() ; x++)
				SetPixelColor(x, 0, crColor);

			// memcpy other rows
			for (unsigned int y = 1 ; y < GetHeight() ; y++)
				memcpy(m_pDibSectionBits + y * uiDIBScanLineSize, m_pDibSectionBits, uiDIBScanLineSize);
		}
	}
	else if (GetBitCount() == 24)
	{
		if ((GetRValue(crColor) == GetGValue(crColor)) &&
			(GetGValue(crColor) == GetBValue(crColor)))
			memset(m_pDibSectionBits, GetRValue(crColor), uiDIBScanLineSize * GetHeight());
		else
		{
			// Fill One Row
			for (unsigned int x = 0 ; x < GetWidth() ; x++)
			{
				m_pDibSectionBits[3*x]		= GetBValue(crColor);
				m_pDibSectionBits[3*x+1]	= GetGValue(crColor);
				m_pDibSectionBits[3*x+2]	= GetRValue(crColor);
			}

			// memcpy other rows
			for (unsigned int y = 1 ; y < GetHeight() ; y++)
				memcpy(m_pDibSectionBits + y * uiDIBScanLineSize, m_pDibSectionBits, uiDIBScanLineSize);
		}
	}
	else if (GetBitCount() == 32)
	{
		if ((GetRValue(crColor) == GetGValue(crColor)) &&
			(GetGValue(crColor) == GetBValue(crColor)) &&
			(GetCompression() == BI_RGB))
			memset(m_pDibSectionBits, GetRValue(crColor), uiDIBScanLineSize * GetHeight());
		else
		{
			// Fill One Row
			for (unsigned int x = 0 ; x < GetWidth() ; x++)
				SetPixelColor(x, 0, crColor);

			// memcpy other rows
			for (unsigned int y = 1 ; y < GetHeight() ; y++)
				memcpy(m_pDibSectionBits + y * uiDIBScanLineSize, m_pDibSectionBits, uiDIBScanLineSize);
		}
	}
	else
		return FALSE;

	return TRUE;
}

BOOL CDib::AllocateBitsFast(WORD wBpp,
							DWORD wCompression,
							DWORD dwWidth,
							DWORD dwHeight,
							RGBQUAD* pColorsOrMasks/*=NULL*/,
							DWORD dwNumOfColors/*=0*/)
{
	Free();

	// Check
	if (dwWidth == 0 || dwHeight == 0)
		return TRUE; // Ok. Allocate Nothing, just Free!
	
	// Allocate BMI for RGB
	if ((wCompression == BI_RGB)		||
		(wCompression == BI_RLE4)		||
		(wCompression == FCC('RLE4'))	||
		(wCompression == BI_RLE8)		||
		(wCompression == FCC('RLE8'))	||
		(wCompression == BI_RGB15)		||
		(wCompression == BI_RGB16)		||
		(wCompression == BI_BGR15)		||
		(wCompression == BI_BGR16)		||
		(wCompression == BI_BITFIELDS))
	{
		// Correct just in case
		if (wCompression == BI_RGB15	||
			wCompression == BI_RGB16	||
			wCompression == BI_BGR15	||
			wCompression == BI_BGR16)
			wBpp = 16;
		else if (wCompression == BI_RLE4	||
				wCompression == FCC('RLE4'))
		{
			wCompression = BI_RLE4;
			wBpp = 4;
		}
		else if (wCompression == BI_RLE8	||
				wCompression == FCC('RLE8'))
		{
			wCompression = BI_RLE8;
			wBpp = 8;
		}

		DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES(dwWidth * wBpp);
		m_dwImageSize = uiDIBScanLineSize * dwHeight;

		// Allocate memory
		if (wBpp == 24)
		{
			m_pBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER)];
		}
		else if (wBpp == 32)
		{
			if (wCompression == BI_BITFIELDS)
			{
				DWORD dwBMISize = sizeof(BITMAPINFOHEADER) + 3 * sizeof(DWORD);
				m_pBMI = (LPBITMAPINFO)new BYTE[dwBMISize];
				if (pColorsOrMasks == NULL)
				{
					LPBYTE pDstMask = (LPBYTE)m_pBMI + sizeof(BITMAPINFOHEADER);
					*((DWORD*)(pDstMask)) = 0x00FF0000;	// Red Mask
					pDstMask = pDstMask + sizeof(DWORD);
					*((DWORD*)(pDstMask)) = 0x0000FF00;	// Green Mask
					pDstMask = pDstMask + sizeof(DWORD);
					*((DWORD*)(pDstMask)) = 0x000000FF;	// Blue Mask
				}
				else
				{
					memcpy((LPBYTE)m_pBMI + sizeof(BITMAPINFOHEADER),
							pColorsOrMasks,
							3 * sizeof(DWORD));
				}
			}
			else
				m_pBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER)];
		}
		else if (wBpp == 16)
		{
			if (wCompression == BI_BITFIELDS)
			{
				DWORD dwBMISize = sizeof(BITMAPINFOHEADER) + 3 * sizeof(DWORD);
				m_pBMI = (LPBITMAPINFO)new BYTE[dwBMISize];

				if (pColorsOrMasks == NULL)
				{
					LPBYTE pDstMask = (LPBYTE)m_pBMI + sizeof(BITMAPINFOHEADER);
					*((DWORD*)(pDstMask)) = 0x7C00; // Red Mask
					pDstMask = pDstMask + sizeof(DWORD);
					*((DWORD*)(pDstMask)) = 0x03E0; // Green Mask
					pDstMask = pDstMask + sizeof(DWORD);
					*((DWORD*)(pDstMask)) = 0x001F; // Blue Mask
				}
				else
				{
					memcpy((LPBYTE)m_pBMI + sizeof(BITMAPINFOHEADER),
							pColorsOrMasks,
							3 * sizeof(DWORD));
				}
			}
			else if (wCompression == BI_RGB16	||
					wCompression == BI_BGR16)
			{
				wCompression = BI_BITFIELDS;

				DWORD dwBMISize = sizeof(BITMAPINFOHEADER) + 3 * sizeof(DWORD);
				m_pBMI = (LPBITMAPINFO)new BYTE[dwBMISize];

				LPBYTE pDstMask = (LPBYTE)m_pBMI + sizeof(BITMAPINFOHEADER);
				*((DWORD*)(pDstMask)) = 0xF800; // Red Mask
				pDstMask = pDstMask + sizeof(DWORD);
				*((DWORD*)(pDstMask)) = 0x07E0; // Green Mask
				pDstMask = pDstMask + sizeof(DWORD);
				*((DWORD*)(pDstMask)) = 0x001F; // Blue Mask
			}
			else if (wCompression == BI_RGB15	||
					wCompression == BI_BGR15)
			{
				wCompression = BI_RGB;
				m_pBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER)];
			}
			else
				m_pBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER)];
		}
		else if (wBpp == 8)
		{
			if (dwNumOfColors == 0)
				m_pBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD)];
			else
				m_pBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER) + MIN(256, dwNumOfColors) * sizeof(RGBQUAD)];
		}
		else if (wBpp == 4)
		{
			if (dwNumOfColors == 0)
				m_pBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER) + 16 * sizeof(RGBQUAD)];
			else
				m_pBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER) + MIN(16, dwNumOfColors) * sizeof(RGBQUAD)];
		}
		else if (wBpp == 1)
		{
			if (dwNumOfColors == 0)
				m_pBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER) + 2 * sizeof(RGBQUAD)];
			else
				m_pBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER) + MIN(2, dwNumOfColors) * sizeof(RGBQUAD)];
		}
	}
	// Allocate BMI for YUV
	else
	{
		wBpp = ::FourCCToBpp(wCompression);
		int stride = ::CalcYUVStride(wCompression, dwWidth);
		if (stride > 0)
			m_dwImageSize = ::CalcYUVSize(wCompression, stride, dwHeight);
		else
			return FALSE; // No YUV.
		m_pBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER)];
	}
	if (!m_pBMI)
		return FALSE;

	// Allocate Bits
	m_pBits = (LPBYTE)BIGALLOC(m_dwImageSize);
	if (!m_pBits)
		return FALSE;

	// Init BMI
	m_pBMI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_pBMI->bmiHeader.biWidth = dwWidth;
	m_pBMI->bmiHeader.biHeight = dwHeight;
	m_pBMI->bmiHeader.biPlanes = 1;
	m_pBMI->bmiHeader.biBitCount = wBpp;
	m_pBMI->bmiHeader.biCompression = wCompression;
	m_pBMI->bmiHeader.biSizeImage = m_dwImageSize;
	m_pBMI->bmiHeader.biXPelsPerMeter = 0;
	m_pBMI->bmiHeader.biYPelsPerMeter = 0;
	m_pBMI->bmiHeader.biClrUsed = dwNumOfColors;
	m_pBMI->bmiHeader.biClrImportant = 0;

	// Init Colors
	if ((wBpp <= 8) &&
		((wCompression == BI_RGB) || (wCompression == BI_RLE4) || (wCompression == BI_RLE8)))
	{
		m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + (WORD)(m_pBMI->bmiHeader.biSize));
		if (m_pBMI->bmiHeader.biBitCount == 8)
		{
			if (dwNumOfColors == 0)
				dwNumOfColors = 256;
			if (pColorsOrMasks)
				memcpy(m_pColors, pColorsOrMasks, MIN(256, dwNumOfColors) * sizeof(RGBQUAD));
			else
				memset(m_pColors, 0, MIN(256, dwNumOfColors) * sizeof(RGBQUAD));
		}
		else if (m_pBMI->bmiHeader.biBitCount == 4)
		{
			if (dwNumOfColors == 0)
				dwNumOfColors = 16;
			if (pColorsOrMasks)
				memcpy(m_pColors, pColorsOrMasks, MIN(16, dwNumOfColors) * sizeof(RGBQUAD));
			else
				memset(m_pColors, 0, MIN(16, dwNumOfColors) * sizeof(RGBQUAD));
		}
		else
		{
			if (dwNumOfColors == 0)
				dwNumOfColors = 2;
			if (pColorsOrMasks)
				memcpy(m_pColors, pColorsOrMasks, MIN(2, dwNumOfColors) * sizeof(RGBQUAD));
			else
				memset(m_pColors, 0, MIN(2, dwNumOfColors) * sizeof(RGBQUAD));
		}
	}
	else
		m_pColors = NULL;

	// Init Palette
	CreatePaletteFromBMI();

	// Init Masks
	InitMasks();

	return TRUE;
}

BOOL CDib::AllocateBits(WORD wBpp,
						DWORD wCompression,
						DWORD dwWidth,
						DWORD dwHeight,
						COLORREF crInitColor/*=0*/,
						RGBQUAD* pColorsOrMasks/*=NULL*/,
						DWORD dwNumOfColors/*=0*/)
{
	if (AllocateBitsFast(	wBpp,
							wCompression,
							dwWidth,
							dwHeight,
							pColorsOrMasks,
							dwNumOfColors))
	{
		SetBitColors(crInitColor);
		return TRUE;
	}
	else
		return FALSE;
}

BOOL CDib::AllocateDibSection(	WORD wBpp,
								DWORD dwWidth,
								DWORD dwHeight,
								COLORREF crInitColor/*=RGB(0,0,0)*/,
								RGBQUAD* pColors/*=NULL*/,
								DWORD dwNumOfColors/*=0*/)
{
	Free();

	// Check
	if (dwWidth == 0 || dwHeight == 0)
		return TRUE; // Ok. Allocate Nothing, just Free!

	// Scan Line Size
	DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES(dwWidth * wBpp);

	// Allocate memory
	if (wBpp == 32 || wBpp == 24 || wBpp == 16)
		m_pBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER)];
	else if (wBpp == 8)
	{
		if (dwNumOfColors == 0)
			m_pBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD)];
		else
			m_pBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER) + MIN(256, dwNumOfColors) * sizeof(RGBQUAD)];
	}
	else if (wBpp == 4)
	{
		if (dwNumOfColors == 0)
			m_pBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER) + 16 * sizeof(RGBQUAD)];
		else
			m_pBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER) + MIN(16, dwNumOfColors) * sizeof(RGBQUAD)];
	}
	else if (wBpp == 1)
	{
		if (dwNumOfColors == 0)
			m_pBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER) + 2 * sizeof(RGBQUAD)];
		else
			m_pBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER) + MIN(2, dwNumOfColors) * sizeof(RGBQUAD)];
	}
	else
		return FALSE;

	// Check
	if (!m_pBMI)
		return FALSE;

	// BMI
	m_pBMI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_pBMI->bmiHeader.biWidth = dwWidth;
	m_pBMI->bmiHeader.biHeight = dwHeight;
	m_pBMI->bmiHeader.biPlanes = 1;
	m_pBMI->bmiHeader.biBitCount = wBpp;
	m_pBMI->bmiHeader.biCompression = BI_RGB;
	m_pBMI->bmiHeader.biSizeImage = DWALIGNEDWIDTHBYTES(dwWidth * wBpp) * dwHeight;
	m_pBMI->bmiHeader.biXPelsPerMeter = 0;
	m_pBMI->bmiHeader.biYPelsPerMeter = 0;
	m_pBMI->bmiHeader.biClrUsed = dwNumOfColors;
	m_pBMI->bmiHeader.biClrImportant = 0;
	m_dwImageSize = uiDIBScanLineSize * dwHeight;
	if (m_pBMI->bmiHeader.biBitCount <= 8)
	{
		m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + (WORD)(m_pBMI->bmiHeader.biSize));
		if (m_pBMI->bmiHeader.biBitCount == 8)
		{
			if (dwNumOfColors == 0)
				dwNumOfColors = 256;
			if (pColors)
				memcpy(m_pColors, pColors, MIN(256, dwNumOfColors) * sizeof(RGBQUAD));
			else
				memset(m_pColors, 0, MIN(256, dwNumOfColors) * sizeof(RGBQUAD));
		}
		else if (m_pBMI->bmiHeader.biBitCount == 4)
		{
			if (dwNumOfColors == 0)
				dwNumOfColors = 16;
			if (pColors)
				memcpy(m_pColors, pColors, MIN(16, dwNumOfColors) * sizeof(RGBQUAD));
			else
				memset(m_pColors, 0, MIN(16, dwNumOfColors) * sizeof(RGBQUAD));
		}
		else
		{
			if (dwNumOfColors == 0)
				dwNumOfColors = 2;
			if (pColors)
				memcpy(m_pColors, pColors, MIN(2, dwNumOfColors) * sizeof(RGBQUAD));
			else
				memset(m_pColors, 0, MIN(2, dwNumOfColors) * sizeof(RGBQUAD));
		}
	}
	else
		m_pColors = NULL;

	// Create a DC
	HDC hDC = ::GetDC(NULL);
	if (!hDC)
		return FALSE;

	// Create the DibSection
	m_hDibSection = ::CreateDIBSection(	hDC,
										(const BITMAPINFO*)m_pBMI,
										DIB_RGB_COLORS,
										(void**)&m_pDibSectionBits, NULL, 0);
	if (!m_hDibSection)
	{
		ShowLastError(_T("CreateDIBSection()"));
		::ReleaseDC(NULL, hDC);
		m_pDibSectionBits = NULL;
		return FALSE;
	}
	else
		::ReleaseDC(NULL, hDC);

	// Init Palette
	CreatePaletteFromBMI();
	
	// Init Masks
	InitMasks();

	// Set Color
	SetDibSectionColors(crInitColor);

	return TRUE;
}

BOOL CDib::CropDibSection(	DWORD dwOrigX,
							DWORD dwOrigY,
							DWORD dwCropWidth,
							DWORD dwCropHeight)
{
	// Check
	if (((dwOrigX + dwCropWidth) > GetWidth()) || ((dwOrigY + dwCropHeight) > GetHeight()))
		return FALSE;

	if (!m_hDibSection)
	{
		if (!BitsToDibSection())
			return FALSE;
	}

	if (!m_pBMI)
		return FALSE;

	// DCs
	HDC hDC = ::GetDC(NULL);
	if (!hDC)
		return FALSE;
	HDC hMemDCSrc = ::CreateCompatibleDC(hDC);
	if (!hMemDCSrc)
	{
		::ReleaseDC(NULL, hDC);
		return FALSE;
	}
	HDC hMemDCDst = ::CreateCompatibleDC(hDC);
	if (!hMemDCDst)
	{
		::ReleaseDC(NULL, hDC);
		::DeleteDC(hMemDCSrc);
		return FALSE;
	}

	// Create the New DibSection
	m_pBMI->bmiHeader.biWidth = dwCropWidth;
	m_pBMI->bmiHeader.biHeight = dwCropHeight;
	m_dwImageSize = DWALIGNEDWIDTHBYTES(dwCropWidth * GetBitCount()) * dwCropHeight;
	m_pBMI->bmiHeader.biSizeImage = m_dwImageSize;
	HBITMAP hDibSection = ::CreateDIBSection(hDC, (const BITMAPINFO*)m_pBMI,
							DIB_RGB_COLORS, (void**)&m_pDibSectionBits, NULL, 0);
	if (!hDibSection)
	{
		ShowLastError(_T("CreateDIBSection()"));
		::ReleaseDC(NULL, hDC);
		::DeleteDC(hMemDCSrc);
		::DeleteDC(hMemDCDst);
		m_pDibSectionBits = NULL;
		return FALSE;
	}

	// Select Dib Sections into Memory DCs
	HBITMAP hOldBitmapSrc = (HBITMAP)::SelectObject(hMemDCSrc, m_hDibSection);
	HBITMAP hOldBitmapDst = (HBITMAP)::SelectObject(hMemDCDst, hDibSection);

	// Crop
	BOOL res = ::BitBlt(hMemDCDst,
						0, 0,
						dwCropWidth, 
						dwCropHeight,
						hMemDCSrc,
						dwOrigX, dwOrigY,
						SRCCOPY);

	// Clean-up
	::SelectObject(hMemDCSrc, hOldBitmapSrc);
	::SelectObject(hMemDCDst, hOldBitmapDst);
	::DeleteDC(hMemDCSrc);
	::DeleteDC(hMemDCDst);
	::ReleaseDC(NULL, hDC);

	// Set New Dib Section
	::DeleteObject(m_hDibSection);
	m_hDibSection = hDibSection;

	return res;
}

BOOL CDib::CropBits(DWORD dwOrigX,
					DWORD dwOrigY,
					DWORD dwCropWidth,
					DWORD dwCropHeight,
					CDib* pSrcDib/*=NULL*/,
					CWnd* pProgressWnd/*=NULL*/,
					BOOL bProgressSend/*=TRUE*/)
{
	// Make a Copy of this?
	CDib SrcDib;
	BOOL bCopySrcToDst = FALSE;
	if (pSrcDib == NULL || this == pSrcDib)
	{
		// Check
		if (((dwOrigX + dwCropWidth) > GetWidth()) || ((dwOrigY + dwCropHeight) > GetHeight()))
			return FALSE;

		if (IsCompressed())
		{
			if (!Decompress(GetBitCount())) // Decompress
				return FALSE;
		}

		SrcDib = *this;
		pSrcDib = &SrcDib;

		if (!pSrcDib->m_pBits)
		{
			if (!pSrcDib->DibSectionToBits())
				return FALSE;
		}

		// Pointers Check
		if (!pSrcDib->m_pBits || !pSrcDib->m_pBMI)
			return FALSE;
	}
	else
	{
		// Pointers Check
		if (!pSrcDib->m_pBits || !pSrcDib->m_pBMI)
			return FALSE;

		// Check
		if (((dwOrigX + dwCropWidth) > pSrcDib->GetWidth()) || ((dwOrigY + dwCropHeight) > pSrcDib->GetHeight()))
			return FALSE;

		// No Compression Supported!
		if (pSrcDib->IsCompressed())
			return FALSE;

		// Allocate BMI
		if (m_pBMI == NULL)
		{
			// Allocate & Copy BMI
			m_pBMI = (LPBITMAPINFO)new BYTE[pSrcDib->GetBMISize()];
			if (m_pBMI == NULL)
				return FALSE;
			memcpy((void*)m_pBMI, (void*)pSrcDib->m_pBMI, pSrcDib->GetBMISize());
		}
		// Need to ReAllocate BMI because they are of different size
		else if (pSrcDib->GetBMISize() != GetBMISize())
		{
			delete [] m_pBMI;

			// Allocate & Copy BMI
			m_pBMI = (LPBITMAPINFO)new BYTE[pSrcDib->GetBMISize()];
			if (m_pBMI == NULL)
				return FALSE;
			memcpy((void*)m_pBMI, (void*)pSrcDib->m_pBMI, pSrcDib->GetBMISize());
		}

		// Make Sure m_pColors Points to the Right Place
		if (m_pBMI->bmiHeader.biBitCount <= 8)
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + (WORD)(m_pBMI->bmiHeader.biSize));
		else
			m_pColors = NULL;

		// Copy Src To Dst
		bCopySrcToDst = TRUE;
	}

	// Top-Down to Bottom-Up Coordinates
	dwOrigY = pSrcDib->GetHeight() - 1 - dwOrigY;

	// Scan Line Alignments
	DWORD uiDIBSourceScanLineSize = DWALIGNEDWIDTHBYTES(pSrcDib->GetWidth() * pSrcDib->GetBitCount());
	DWORD uiDIBTargetScanLineSize = DWALIGNEDWIDTHBYTES(dwCropWidth * GetBitCount());

	// Allocate Bits
	if (m_pBits == NULL)
	{
		// Allocate memory
		m_pBits = (LPBYTE)BIGALLOC(uiDIBTargetScanLineSize * dwCropHeight);
	}
	// Need to ReAllocate Bits because they are of different size
	else if (m_dwImageSize != uiDIBTargetScanLineSize * dwCropHeight)
	{
		BIGFREE(m_pBits);

		// Allocate memory
		m_pBits = (LPBYTE)BIGALLOC(uiDIBTargetScanLineSize * dwCropHeight);
	}
	if (m_pBits == NULL)
		return FALSE;

	// Copy Vars
	if (bCopySrcToDst)
		CopyVars(*pSrcDib);

	// Change BMI
	m_pBMI->bmiHeader.biWidth = dwCropWidth;
	m_pBMI->bmiHeader.biHeight = dwCropHeight;
	m_dwImageSize = m_pBMI->bmiHeader.biSizeImage = uiDIBTargetScanLineSize * GetHeight();

	// Init Masks For 16 and 32 bits Pictures
	InitMasks();

	// Free
	ResetColorUndo();
	if (m_hDibSection)
	{
		::DeleteObject(m_hDibSection);
		m_hDibSection = NULL;
		m_pDibSectionBits = NULL;
	}

	DIB_INIT_PROGRESS;

	// Crop
	switch (pSrcDib->GetBitCount())
	{
		// Not Optimized
		case 1 :
		case 4 :
		{
			for (int y = dwOrigY - dwCropHeight + 1 ; y <= (int)dwOrigY ; y++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y + dwCropHeight - dwOrigY - 1, dwCropHeight);

				for (int x = dwOrigX ; x < (int)(dwOrigX + dwCropWidth) ; x++)
				{
					SetPixelIndex(	x-dwOrigX, y-(dwOrigY - dwCropHeight + 1),
									pSrcDib->GetPixelIndex(x, y));
				}
			}
			break;
		}

		// Optimized
		case 8 :
		{
			LPBYTE lpSrcBits = pSrcDib->GetBits() + (dwOrigY - dwCropHeight + 1) * uiDIBSourceScanLineSize;
			LPBYTE lpDstBits = GetBits();
			for (int y = 0 ; y < (int)dwCropHeight ; y++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, dwCropHeight);
				memcpy(lpDstBits, lpSrcBits + dwOrigX, dwCropWidth);
				lpSrcBits += uiDIBSourceScanLineSize;
				lpDstBits += uiDIBTargetScanLineSize;
			}
			break;
		}

		// Optimized
		case 16 :
		{
			LPBYTE lpSrcBits = pSrcDib->GetBits() + (dwOrigY - dwCropHeight + 1) * uiDIBSourceScanLineSize;
			LPBYTE lpDstBits = GetBits();
			for (int y = 0 ; y < (int)dwCropHeight ; y++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, dwCropHeight);
				memcpy(lpDstBits, lpSrcBits + (dwOrigX<<1), dwCropWidth<<1);
				lpSrcBits += uiDIBSourceScanLineSize;
				lpDstBits += uiDIBTargetScanLineSize;
			}
			break;
		}

		// Optimized
		case 24 :
		{
			LPBYTE lpSrcBits = pSrcDib->GetBits() + (dwOrigY - dwCropHeight + 1) * uiDIBSourceScanLineSize;
			LPBYTE lpDstBits = GetBits();
			for (int y = 0 ; y < (int)dwCropHeight ; y++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, dwCropHeight);
				memcpy(lpDstBits, lpSrcBits + (3*dwOrigX), 3*dwCropWidth);
				lpSrcBits += uiDIBSourceScanLineSize;
				lpDstBits += uiDIBTargetScanLineSize;
			}
			break;
		}

		// Optimized
		case 32 :
		{
			LPBYTE lpSrcBits = pSrcDib->GetBits() + (dwOrigY - dwCropHeight + 1) * uiDIBSourceScanLineSize;
			LPBYTE lpDstBits = GetBits();
			for (int y = 0 ; y < (int)dwCropHeight ; y++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, dwCropHeight);
				memcpy(lpDstBits, lpSrcBits + (dwOrigX<<2), dwCropWidth<<2);
				lpSrcBits += uiDIBSourceScanLineSize;
				lpDstBits += uiDIBTargetScanLineSize;
			}
			break;
		}

		/* Not Optimized for 16, 24 and 32 bpp
		for (int y = dwOrigY - dwCropHeight + 1 ; y <= (int)dwOrigY; y++)
		{
			DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y + dwCropHeight - dwOrigY - 1, dwCropHeight);

			for (int x = dwOrigX ; x < (int)(dwOrigX + dwCropWidth) ; x++)
			{
				SetPixelColor(	x-dwOrigX, y-(dwOrigY - dwCropHeight + 1),
								pSrcDib->GetPixelColor(x, y));
			}
		}
		*/

		default:
			break;
	}

	// Create Palette
	CreatePaletteFromBMI();

	DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

	return TRUE;
}

BOOL CDib::Crop(DWORD dwOrigX, DWORD dwOrigY,
				DWORD dwCropWidth, DWORD dwCropHeight,
				CWnd* pProgressWnd/*=NULL*/,
				BOOL bProgressSend/*=TRUE*/)
{
	if (m_pBits)
		return CropBits(dwOrigX,
						dwOrigY,
						dwCropWidth,
						dwCropHeight,
						NULL,
						pProgressWnd,
						bProgressSend);
	else
		return CropDibSection(	dwOrigX,
								dwOrigY,
								dwCropWidth,
								dwCropHeight);
}

BOOL CDib::AddBorders(	DWORD dwLeft,
						DWORD dwTop,
						DWORD dwRight,
						DWORD dwBottom,
						COLORREF crBorder/*=0*/,
						CDib* pSrcDib/*=NULL*/,
						CWnd* pProgressWnd/*=NULL*/,
						BOOL bProgressSend/*=TRUE*/,
						CWorkerThread* pThread/*=NULL*/)
{
	// Check
	if ((dwLeft == 0)	&&
		(dwTop == 0)	&&
		(dwRight == 0)	&&
		(dwBottom == 0))
		return TRUE;

	// Make a Copy of this?
	CDib SrcDib;
	BOOL bCopySrcToDst = FALSE;
	if (pSrcDib == NULL || this == pSrcDib)
	{
		if (IsCompressed())
		{
			if (!Decompress(GetBitCount())) // Decompress
				return FALSE;
		}

		SrcDib = *this;
		pSrcDib = &SrcDib;

		if (!pSrcDib->m_pBits)
		{
			if (!pSrcDib->DibSectionToBits())
				return FALSE;
		}

		// Pointers Check
		if (!pSrcDib->m_pBits || !pSrcDib->m_pBMI)
			return FALSE;
	}
	else
	{
		// Pointers Check
		if (!pSrcDib->m_pBits || !pSrcDib->m_pBMI)
			return FALSE;

		// No Compression Supported!
		if (pSrcDib->IsCompressed())
			return FALSE;

		// Allocate BMI
		if (m_pBMI == NULL)
		{
			// Allocate & Copy BMI
			m_pBMI = (LPBITMAPINFO)new BYTE[pSrcDib->GetBMISize()];
			if (m_pBMI == NULL)
				return FALSE;
			memcpy((void*)m_pBMI, (void*)pSrcDib->m_pBMI, pSrcDib->GetBMISize());
		}
		// Need to ReAllocate BMI because they are of different size
		else if (pSrcDib->GetBMISize() != GetBMISize())
		{
			delete [] m_pBMI;

			// Allocate & Copy BMI
			m_pBMI = (LPBITMAPINFO)new BYTE[pSrcDib->GetBMISize()];
			if (m_pBMI == NULL)
				return FALSE;
			memcpy((void*)m_pBMI, (void*)pSrcDib->m_pBMI, pSrcDib->GetBMISize());
		}

		// Make Sure m_pColors Points to the Right Place
		if (m_pBMI->bmiHeader.biBitCount <= 8)
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + (WORD)(m_pBMI->bmiHeader.biSize));
		else
			m_pColors = NULL;

		// Copy Src To Dst
		bCopySrcToDst = TRUE;
	}

	// Top-Down to Bottom-Up Coordinates
	DWORD dwTemp = dwTop;
	dwTop = dwBottom;
	dwBottom = dwTemp;

	// Source Sizes
	DWORD dwSrcWidth  = pSrcDib->GetWidth();
	DWORD dwSrcHeight = pSrcDib->GetHeight();

	// Destination Sizes
	DWORD dwDstWidth  = dwSrcWidth + dwLeft + dwRight;
	DWORD dwDstHeight = dwSrcHeight + dwTop + dwBottom;
		
	// Scan Line Alignments
	DWORD uiDIBSourceScanLineSize = DWALIGNEDWIDTHBYTES(dwSrcWidth * pSrcDib->GetBitCount());
	DWORD uiDIBTargetScanLineSize = DWALIGNEDWIDTHBYTES(dwDstWidth * GetBitCount());

	// Allocate Bits
	if (m_pBits == NULL)
	{
		// Allocate memory
		m_pBits = (LPBYTE)BIGALLOC(uiDIBTargetScanLineSize * dwDstHeight);
	}
	// Need to ReAllocate Bits because they are of different size
	else if (m_dwImageSize != uiDIBTargetScanLineSize * dwDstHeight)
	{
		BIGFREE(m_pBits);

		// Allocate memory
		m_pBits = (LPBYTE)BIGALLOC(uiDIBTargetScanLineSize * dwDstHeight);
	}
	if (m_pBits == NULL)
		return FALSE;

	// Copy Vars
	if (bCopySrcToDst)
		CopyVars(*pSrcDib);

	// Change BMI
	m_pBMI->bmiHeader.biWidth = dwDstWidth;
	m_pBMI->bmiHeader.biHeight = dwDstHeight;
	m_dwImageSize = m_pBMI->bmiHeader.biSizeImage = uiDIBTargetScanLineSize * GetHeight();

	// Init Masks For 16 and 32 bits Pictures
	InitMasks();

	// Free
	ResetColorUndo();
	if (m_hDibSection)
	{
		::DeleteObject(m_hDibSection);
		m_hDibSection = NULL;
		m_pDibSectionBits = NULL;
	}

	// Clip Color Index
	if (pSrcDib->GetBitCount() <= 8)
	{
		if (crBorder >= pSrcDib->GetNumColors())
			crBorder = pSrcDib->GetNumColors() - 1;
	}

	DIB_INIT_PROGRESS;

	// Add Border
	switch (pSrcDib->GetBitCount())
	{
		// Not Optimized
		case 1 :
		case 4 :
		{
			for (int y = 0 ; y < (int)dwDstHeight; y++)
			{
				if (pThread && pThread->DoExit())
				{
					DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
					return FALSE;
				}
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, dwDstHeight);
				for (int x = 0 ; x < (int)dwDstWidth ; x++)
				{
					int nSrcX = x - (int)dwLeft;
					int nSrcY = y - (int)dwTop;
					if ((nSrcX < 0) || (nSrcY < 0) ||
						(nSrcX >= (int)dwSrcWidth) || (nSrcY >= (int)dwSrcHeight))
						SetPixelIndex(x, y, crBorder);
					else
						SetPixelIndex(x, y, pSrcDib->GetPixelIndex(nSrcX , nSrcY));
				}
			}
			break;
		}

		// Optimized
		case 8 :
		{
			SetBitColors(crBorder);
			LPBYTE lpSrcBits = pSrcDib->GetBits();
			LPBYTE lpDstBits = GetBits() + (dwTop * uiDIBTargetScanLineSize);
			for (int y = 0 ; y < (int)dwSrcHeight ; y++)
			{
				if (pThread && pThread->DoExit())
				{
					DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
					return FALSE;
				}
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, dwDstHeight);
				memcpy(lpDstBits + dwLeft, lpSrcBits, dwSrcWidth);
				lpSrcBits += uiDIBSourceScanLineSize;
				lpDstBits += uiDIBTargetScanLineSize;
			}
			break;
		}

		// Optimized
		case 16 :
		{
			SetBitColors(crBorder);
			LPBYTE lpSrcBits = pSrcDib->GetBits();
			LPBYTE lpDstBits = GetBits() + (dwTop * uiDIBTargetScanLineSize);
			for (int y = 0 ; y < (int)dwSrcHeight ; y++)
			{
				if (pThread && pThread->DoExit())
				{
					DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
					return FALSE;
				}
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, dwDstHeight);
				memcpy(lpDstBits + (dwLeft<<1), lpSrcBits, dwSrcWidth<<1);
				lpSrcBits += uiDIBSourceScanLineSize;
				lpDstBits += uiDIBTargetScanLineSize;
			}
			break;
		}

		// Optimized
		case 24 :
		{
			SetBitColors(crBorder);
			LPBYTE lpSrcBits = pSrcDib->GetBits();
			LPBYTE lpDstBits = GetBits() + (dwTop * uiDIBTargetScanLineSize);
			for (int y = 0 ; y < (int)dwSrcHeight ; y++)
			{
				if (pThread && pThread->DoExit())
				{
					DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
					return FALSE;
				}
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, dwDstHeight);
				memcpy(lpDstBits + (3*dwLeft), lpSrcBits, 3*dwSrcWidth);
				lpSrcBits += uiDIBSourceScanLineSize;
				lpDstBits += uiDIBTargetScanLineSize;
			}
			break;
		}

		// Optimized
		case 32 :
		{
			SetBitColors(crBorder);
			LPBYTE lpSrcBits = pSrcDib->GetBits();
			LPBYTE lpDstBits = GetBits() + (dwTop * uiDIBTargetScanLineSize);
			for (int y = 0 ; y < (int)dwSrcHeight ; y++)
			{
				if (pThread && pThread->DoExit())
				{
					DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
					return FALSE;
				}
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, dwDstHeight);
				memcpy(lpDstBits + (dwLeft<<2), lpSrcBits, dwSrcWidth<<2);
				lpSrcBits += uiDIBSourceScanLineSize;
				lpDstBits += uiDIBTargetScanLineSize;
			}
			break;
		}

		/* Not Optimized for 16, 24 and 32 bpp
		for (int y = 0 ; y < (int)dwDstHeight; y++)
		{
			if (pThread && pThread->DoExit())
			{
				DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
				return FALSE;
			}
			DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, dwDstHeight);
			for (int x = 0 ; x < (int)dwDstWidth ; x++)
			{
				int nSrcX = x - (int)dwLeft;
				int nSrcY = y - (int)dwTop;
				if ((nSrcX < 0) || (nSrcY < 0) ||
					(nSrcX >= (int)dwSrcWidth) || (nSrcY >= (int)dwSrcHeight))
					SetPixelColor(x, y, crBorder);
				else
					SetPixelColor(x, y, pSrcDib->GetPixelColor(nSrcX , nSrcY));
			}
		}
		*/

		default:
			break;
	}

	// Create Palette
	CreatePaletteFromBMI();

	DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

	return TRUE;
}

BOOL CDib::SoftBorders(int nBorder,
					   BOOL bBlur,
					   CWnd* pProgressWnd/*=NULL*/,
					   BOOL bProgressSend/*=TRUE*/)
{
	// Check
	if (nBorder <= 0)
		return TRUE;

	if (!m_pBits)
	{
		if (!DibSectionToBits())
			return FALSE;
	}

	if (!m_pBits || !m_pBMI)
		return FALSE;

	if (!Decompress(32)) // Decompress to 32 bpp
		return FALSE;

	int x, y;

	DIB_INIT_PROGRESS;

	// If no Alpha
	if (!HasAlpha())
	{
		// Set All Opaque
		for (y = 0 ; y < (int)GetHeight() ; y++)
		{
			for (x = 0 ; x < (int)GetWidth() ; x++)
			{
				((DWORD*)m_pBits)[x + y*GetWidth()] |= 0xFF000000;
			}
		}

		// Set Alpha Flag
		m_bAlpha = TRUE;
	}

	// Make Blended Borders
	double k = 255.0 / nBorder;
	for (y = 0 ; y < (int)GetHeight() ; y++)
	{
		DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, GetHeight());

		for (x = 0 ; x < (int)GetWidth() ; x++)
		{
			int R, G, B, A;

			// Get Pixel
			DIB32ToRGBA(((DWORD*)m_pBits)[x + y*GetWidth()], &R, &G, &B, &A);
			
			// X Direction
			if (x < nBorder)
				A = MIN((int)((x + 1) * k), A);
			else if (x > ((int)GetWidth() - 1 - nBorder))
				A = MIN((int)(((int)GetWidth() - x) * k), A);

			// Y Direction
			if (y < nBorder)
				A = MIN((int)((y + 1) * k), A);
			else if (y > ((int)GetHeight() - 1 - nBorder))
				A = MIN((int)(((int)GetHeight() - y) * k), A);

			// Set Pixel
			((DWORD*)m_pBits)[x + y*GetWidth()] = RGBAToDIB32(R, G, B, A);
		}
	}

	DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

	// Blur Alpha
	if (bBlur)
	{	
		BOOL res1, res2, res3, res4, res5;

		// Corners Kernel
		int Kernel[] = {1,1,1,1,1,1,1,1,1,
						1,1,1,1,1,1,1,1,1,
						1,1,1,1,1,1,1,1,1,
						1,1,1,1,1,1,1,1,1,
						1,1,1,1,1,1,1,1,1,
						1,1,1,1,1,1,1,1,1,
						1,1,1,1,1,1,1,1,1,
						1,1,1,1,1,1,1,1,1,
						1,1,1,1,1,1,1,1,1};

		// Top-Left Corner
		res1 = FilterAlpha(	CRect(	1,
							1,
							nBorder - 1,
							nBorder - 1),
							Kernel,
							9, 81, 0,
							NULL,
							pProgressWnd,
							bProgressSend);

		// Top-Right Corner
		res2 = FilterAlpha(	CRect(	GetWidth() - nBorder + 1,
							1,
							GetWidth() - 1,
							nBorder - 1),
							Kernel,
							9, 81, 0,
							NULL,
							pProgressWnd,
							bProgressSend);

		// Bottom-Left Corner
		res3 = FilterAlpha(	CRect(	1,
							GetHeight() - nBorder + 1,
							nBorder - 1,
							GetHeight() - 1),
							Kernel,
							9, 81, 0,
							NULL,
							pProgressWnd,
							bProgressSend);

		// Bottom-Right Corner
		res4 = FilterAlpha(	CRect(	GetWidth() - nBorder + 1,
							GetHeight() - nBorder + 1,
							GetWidth() - 1,
							GetHeight() - 1),
							Kernel,
							9, 81, 0,
							NULL,
							pProgressWnd,
							bProgressSend);

		// Overall
		int KernelFast[] = {1,1,1,
							1,1,1,
							1,1,1};
		res5 = FilterFastAlpha(	KernelFast, 9,
								NULL,
								pProgressWnd,
								bProgressSend);

		return (res1 && res2 && res3 && res4 && res5);
	}
	else
		return TRUE;
}

BOOL CDib::CopyBits(	DWORD dwFourCC,
						DWORD dwBitCount,
						DWORD uiDstStartX,
						DWORD uiDstStartY,
						DWORD uiSrcStartX,
						DWORD uiSrcStartY,
						DWORD uiWidthCopy,
						DWORD uiHeightCopy,
						DWORD uiDstHeight,
						DWORD uiSrcHeight,
						LPBYTE pDstBits,
						LPBYTE pSrcBits,
						DWORD uiDIBDstScanLineSize,
						DWORD uiDIBSrcScanLineSize)
{
	if (!pDstBits || !pSrcBits)
		return FALSE;

	if (dwFourCC == BI_RGB ||
		dwFourCC == BI_BITFIELDS)
	{
		DWORD uiBytesPerScanLineToCopy = uiWidthCopy * dwBitCount / 8;
		LPBYTE ps = pSrcBits + uiSrcStartX * dwBitCount / 8 + uiDIBSrcScanLineSize * uiSrcStartY;
		LPBYTE pd = pDstBits + uiDstStartX * dwBitCount / 8 + uiDIBDstScanLineSize * uiDstStartY;
		for (DWORD line = 0 ; line < uiHeightCopy ; line++)
		{
			memcpy(pd, ps, uiBytesPerScanLineToCopy);
			ps += uiDIBSrcScanLineSize;
			pd += uiDIBDstScanLineSize;
		}
		return TRUE;
	}
	else if (	dwFourCC == FCC('YV16')	||
				dwFourCC == FCC('Y42B'))
	{
		DWORD line;
		DWORD uiBytesPerScanLineToCopy = ::CalcYUVStride(dwFourCC, uiWidthCopy);
		LPBYTE ps = pSrcBits + uiSrcStartX + uiDIBSrcScanLineSize * uiSrcStartY;
		LPBYTE pd = pDstBits + uiDstStartX + uiDIBDstScanLineSize * uiDstStartY;

		// Copy Y Plane
		for (line = 0 ; line < uiHeightCopy ; line++)
		{
			memcpy(pd, ps, uiBytesPerScanLineToCopy);
			ps += uiDIBSrcScanLineSize;
			pd += uiDIBDstScanLineSize;
		}
		
		// Copy Chroma Planes
		DWORD uiBytesPerScanLineToCopy2 = (uiBytesPerScanLineToCopy >> 1);
		DWORD uiDstStartX2 = (uiDstStartX >> 1);
		DWORD uiDIBDstScanLineSize2 = (uiDIBDstScanLineSize >> 1);
		DWORD uiSrcStartX2 = (uiSrcStartX >> 1);
		DWORD uiDIBSrcScanLineSize2 = (uiDIBSrcScanLineSize >> 1);
		ps =	pSrcBits										+
				uiSrcStartX2									+
				uiDIBSrcScanLineSize * uiSrcHeight				+
				uiDIBSrcScanLineSize2 * uiSrcStartY;
		pd =	pDstBits										+
				uiDstStartX2									+
				uiDIBDstScanLineSize * uiDstHeight				+
				uiDIBDstScanLineSize2 * uiDstStartY;
		for (line = 0 ; line < uiHeightCopy ; line++)
		{
			memcpy(pd, ps, uiBytesPerScanLineToCopy2);
			ps += uiDIBSrcScanLineSize2;
			pd += uiDIBDstScanLineSize2;
		}
		ps =	pSrcBits										+
				uiSrcStartX2									+
				uiDIBSrcScanLineSize * uiSrcHeight				+
				uiDIBSrcScanLineSize2 * uiSrcHeight				+
				uiDIBSrcScanLineSize2 * uiSrcStartY;
		pd =	pDstBits										+
				uiDstStartX2									+
				uiDIBDstScanLineSize * uiDstHeight				+
				uiDIBDstScanLineSize2 * uiDstHeight				+
				uiDIBDstScanLineSize2 * uiDstStartY;
		for (line = 0 ; line < uiHeightCopy ; line++)
		{
			memcpy(pd, ps, uiBytesPerScanLineToCopy2);
			ps += uiDIBSrcScanLineSize2;
			pd += uiDIBDstScanLineSize2;
		}
		return TRUE;
	}
	else if (	dwFourCC == FCC('YV12')	||
				dwFourCC == FCC('I420')	||
				dwFourCC == FCC('IYUV'))
	{
		DWORD line;
		DWORD uiBytesPerScanLineToCopy = ::CalcYUVStride(dwFourCC, uiWidthCopy);
		LPBYTE ps = pSrcBits + uiSrcStartX + uiDIBSrcScanLineSize * uiSrcStartY;
		LPBYTE pd = pDstBits + uiDstStartX + uiDIBDstScanLineSize * uiDstStartY;

		// Copy Y Plane
		for (line = 0 ; line < uiHeightCopy ; line++)
		{
			memcpy(pd, ps, uiBytesPerScanLineToCopy);
			ps += uiDIBSrcScanLineSize;
			pd += uiDIBDstScanLineSize;
		}
		
		// Copy Chroma Planes
		DWORD uiBytesPerScanLineToCopy2 = (uiBytesPerScanLineToCopy >> 1);
		DWORD uiHeightCopy2 = (uiHeightCopy >> 1);
		DWORD uiDstStartX2 = (uiDstStartX >> 1);
		DWORD uiDstStartY2 = (uiDstStartY >> 1);
		DWORD uiDstHeight2 = (uiDstHeight >> 1);
		DWORD uiDIBDstScanLineSize2 = (uiDIBDstScanLineSize >> 1);
		DWORD uiSrcStartX2 = (uiSrcStartX >> 1);
		DWORD uiSrcStartY2 = (uiSrcStartY >> 1);
		DWORD uiSrcHeight2 = (uiSrcHeight >> 1);
		DWORD uiDIBSrcScanLineSize2 = (uiDIBSrcScanLineSize >> 1);
		ps =	pSrcBits										+
				uiSrcStartX2									+
				uiDIBSrcScanLineSize * uiSrcHeight				+
				uiDIBSrcScanLineSize2 * uiSrcStartY2;
		pd =	pDstBits										+
				uiDstStartX2									+
				uiDIBDstScanLineSize * uiDstHeight				+
				uiDIBDstScanLineSize2 * uiDstStartY2;
		for (line = 0 ; line < uiHeightCopy2 ; line++)
		{
			memcpy(pd, ps, uiBytesPerScanLineToCopy2);
			ps += uiDIBSrcScanLineSize2;
			pd += uiDIBDstScanLineSize2;
		}
		ps =	pSrcBits										+
				uiSrcStartX2									+
				uiDIBSrcScanLineSize * uiSrcHeight				+
				uiDIBSrcScanLineSize2 * uiSrcHeight2			+
				uiDIBSrcScanLineSize2 * uiSrcStartY2;
		pd =	pDstBits										+
				uiDstStartX2									+
				uiDIBDstScanLineSize * uiDstHeight				+
				uiDIBDstScanLineSize2 * uiDstHeight2			+
				uiDIBDstScanLineSize2 * uiDstStartY2;
		for (line = 0 ; line < uiHeightCopy2 ; line++)
		{
			memcpy(pd, ps, uiBytesPerScanLineToCopy2);
			ps += uiDIBSrcScanLineSize2;
			pd += uiDIBDstScanLineSize2;
		}
		return TRUE;
	}
	else if (	dwFourCC == FCC('YVU9')	||
				dwFourCC == FCC('YUV9'))
	{
		DWORD line;
		DWORD uiBytesPerScanLineToCopy = ::CalcYUVStride(dwFourCC, uiWidthCopy);
		LPBYTE ps = pSrcBits + uiSrcStartX + uiDIBSrcScanLineSize * uiSrcStartY;
		LPBYTE pd = pDstBits + uiDstStartX + uiDIBDstScanLineSize * uiDstStartY;

		// Copy Y Plane
		for (line = 0 ; line < uiHeightCopy ; line++)
		{
			memcpy(pd, ps, uiBytesPerScanLineToCopy);
			ps += uiDIBSrcScanLineSize;
			pd += uiDIBDstScanLineSize;
		}
		
		// Copy Chroma Planes
		DWORD uiBytesPerScanLineToCopy4 = (uiBytesPerScanLineToCopy >> 2);
		DWORD uiHeightCopy4 = (uiHeightCopy >> 2);
		DWORD uiDstStartX4 = (uiDstStartX >> 2);
		DWORD uiDstStartY4 = (uiDstStartY >> 2);
		DWORD uiDstHeight4 = (uiDstHeight >> 2);
		DWORD uiDIBDstScanLineSize4 = (uiDIBDstScanLineSize >> 2);
		DWORD uiSrcStartX4 = (uiSrcStartX >> 2);
		DWORD uiSrcStartY4 = (uiSrcStartY >> 2);
		DWORD uiSrcHeight4 = (uiSrcHeight >> 2);
		DWORD uiDIBSrcScanLineSize4 = (uiDIBSrcScanLineSize >> 2);
		ps =	pSrcBits										+
				uiSrcStartX4									+
				uiDIBSrcScanLineSize * uiSrcHeight				+
				uiDIBSrcScanLineSize4 * uiSrcStartY4;
		pd =	pDstBits										+
				uiDstStartX4									+
				uiDIBDstScanLineSize * uiDstHeight				+
				uiDIBDstScanLineSize4 * uiDstStartY4;
		for (line = 0 ; line < uiHeightCopy4 ; line++)
		{
			memcpy(pd, ps, uiBytesPerScanLineToCopy4);
			ps += uiDIBSrcScanLineSize4;
			pd += uiDIBDstScanLineSize4;
		}
		ps =	pSrcBits										+
				uiSrcStartX4									+
				uiDIBSrcScanLineSize * uiSrcHeight				+
				uiDIBSrcScanLineSize4 * uiSrcHeight4			+
				uiDIBSrcScanLineSize4 * uiSrcStartY4;
		pd =	pDstBits										+
				uiDstStartX4									+
				uiDIBDstScanLineSize * uiDstHeight				+
				uiDIBDstScanLineSize4 * uiDstHeight4			+
				uiDIBDstScanLineSize4 * uiDstStartY4;
		for (line = 0 ; line < uiHeightCopy4 ; line++)
		{
			memcpy(pd, ps, uiBytesPerScanLineToCopy4);
			ps += uiDIBSrcScanLineSize4;
			pd += uiDIBDstScanLineSize4;
		}
		return TRUE;
	}
	// Note: If uiSrcStartX is odd/even and uiDstStartX is even/odd
	// U & V are wrongly copied!
	else if (	// YUY2 Family
				dwFourCC == FCC('YUY2')	||	// Packed: Y0 U0 Y1 V0
				dwFourCC == FCC('V422')	||
				dwFourCC == FCC('VYUY')	||
				dwFourCC == FCC('YUNV')	||
				dwFourCC == FCC('YUYV')	||
				// YVYU Family
				dwFourCC == FCC('YVYU')	||	// Packed: Y0 V0 Y1 U0
				// UYVY Family
				dwFourCC == FCC('UYVY')	||	// Packed: U0 Y0 V0 Y1
				dwFourCC == FCC('Y422')	||
				dwFourCC == FCC('UYNV'))
	{
		DWORD uiBytesPerScanLineToCopy = ::CalcYUVStride(dwFourCC, uiWidthCopy);
		LPBYTE ps = pSrcBits + 2 * uiSrcStartX + uiDIBSrcScanLineSize * uiSrcStartY;
		LPBYTE pd = pDstBits + 2 * uiDstStartX + uiDIBDstScanLineSize * uiDstStartY;
		for (DWORD line = 0 ; line < uiHeightCopy ; line++)
		{
			memcpy(pd, ps, uiBytesPerScanLineToCopy);
			ps += uiDIBSrcScanLineSize;
			pd += uiDIBDstScanLineSize;
		}
		return TRUE;
	}
	else
		return FALSE;
}

BOOL CDib::StretchDibSection(DWORD dwNewWidth,
							 DWORD dwNewHeight,
							 int nStretchMode/*=COLORONCOLOR*/)
{
	// Check
	if (dwNewWidth == 0 || dwNewHeight == 0)
		return FALSE;

	// If Same Size Return
	if ((dwNewWidth == GetWidth()) && (dwNewHeight == GetHeight()))
		return TRUE;

	if (!m_hDibSection)
	{
		if (!BitsToDibSection())
			return FALSE;
	}

	if (!m_pBMI)
		return FALSE;

	// DCs
	HDC hDC = ::GetDC(NULL);
	if (!hDC)
		return FALSE;
	HDC hMemDCSrc = ::CreateCompatibleDC(hDC);
	if (!hMemDCSrc)
	{
		::ReleaseDC(NULL, hDC);
		return FALSE;
	}
	HDC hMemDCDst = ::CreateCompatibleDC(hDC);
	if (!hMemDCDst)
	{
		::DeleteDC(hMemDCSrc);
		::ReleaseDC(NULL, hDC);
		return FALSE;
	}

	// Create the New DibSection
	DWORD dwSrcWidth = m_pBMI->bmiHeader.biWidth;
	DWORD dwSrcHeight = m_pBMI->bmiHeader.biHeight;
	m_pBMI->bmiHeader.biWidth = dwNewWidth;
	m_pBMI->bmiHeader.biHeight = dwNewHeight;
	m_dwImageSize = DWALIGNEDWIDTHBYTES(dwNewWidth * GetBitCount()) * dwNewHeight;
	m_pBMI->bmiHeader.biSizeImage = m_dwImageSize;
	LPBYTE pDibSectionBits;
	HBITMAP hDibSection = ::CreateDIBSection(hDC, (const BITMAPINFO*)m_pBMI,
							DIB_RGB_COLORS, (void**)&pDibSectionBits, NULL, 0);
	if (!hDibSection)
	{
		ShowLastError(_T("CreateDIBSection()"));
		::DeleteDC(hMemDCSrc);
		::DeleteDC(hMemDCDst);
		::ReleaseDC(NULL, hDC);
		return FALSE;
	}

	// Select Dib Sections into Memory DCs
	HBITMAP hOldBitmapSrc = (HBITMAP)::SelectObject(hMemDCSrc, m_hDibSection);
	HBITMAP hOldBitmapDst = (HBITMAP)::SelectObject(hMemDCDst, hDibSection);

	// Stretch Mode
	int nOldStretchMode = ::SetStretchBltMode(hMemDCDst, nStretchMode);

	// Stretch
	BOOL res = ::StretchBlt(hMemDCDst,
							0, 0,
							dwNewWidth, 
							dwNewHeight,
							hMemDCSrc,
							0, 0,
							dwSrcWidth,
							dwSrcHeight,
							SRCCOPY);

	// Clean-up
	::SetStretchBltMode(hMemDCDst, nOldStretchMode);
	::SelectObject(hMemDCSrc, hOldBitmapSrc);
	::SelectObject(hMemDCDst, hOldBitmapDst);
	::DeleteDC(hMemDCSrc);
	::DeleteDC(hMemDCDst);
	::ReleaseDC(NULL, hDC);

	// Set New Dib Section
	::DeleteObject(m_hDibSection);
	m_hDibSection = hDibSection;
	m_pDibSectionBits = pDibSectionBits;

	return res;
}

BOOL CDib::SetDibSectionBits(LPBYTE lpBits, DWORD dwSize)
{
	if (!m_pBMI)
		return FALSE;

	if (!m_hDibSection)
	{
		// Create a DC
		HDC hDC = ::GetDC(NULL);
		if (!hDC)
			return FALSE;

		// Create the DibSection
		m_hDibSection = ::CreateDIBSection(hDC, (const BITMAPINFO*)m_pBMI,
								 DIB_RGB_COLORS, (void**)&m_pDibSectionBits, NULL, 0);
		if (!m_hDibSection)
		{
			ShowLastError(_T("CreateDIBSection()"));
			::ReleaseDC(NULL, hDC);
			m_pDibSectionBits = NULL;
			return FALSE;
		}
		else
			::ReleaseDC(NULL, hDC);
	}
	if (lpBits)
		memcpy(m_pDibSectionBits, lpBits, (dwSize >= m_dwImageSize) ? m_dwImageSize : dwSize);

	return TRUE;
}

BOOL CDib::SetDibSectionBits(LPBYTE lpBits)
{
	if (!m_pBMI)
		return FALSE;

	if (!m_hDibSection)
	{
		// Create a DC
		HDC hDC = ::GetDC(NULL);
		if (!hDC)
			return FALSE;

		// Create the DibSection
		m_hDibSection = ::CreateDIBSection(hDC, (const BITMAPINFO*)m_pBMI,
								 DIB_RGB_COLORS, (void**)&m_pDibSectionBits, NULL, 0);
		if (!m_hDibSection)
		{
			ShowLastError(_T("CreateDIBSection()"));
			::ReleaseDC(NULL, hDC);
			m_pDibSectionBits = NULL;
			return FALSE;
		}
		else
			::ReleaseDC(NULL, hDC);
	}
	if (lpBits)
		memcpy(m_pDibSectionBits, lpBits, m_dwImageSize);

	return TRUE;
}

// Just set the pointers, no allocation:
// 1. Pay attention to not do operations on the Dib object
//    which free or re-alloc the passed bits!
// 2. Reset the pointers before destroying the Dib object
//    by calling SetDibPointers(NULL, NULL), otherwise the
//    pointers are freed by the Dib object!
void CDib::SetDibPointers(LPBITMAPINFO pBMI, LPBYTE pBits)
{
	m_pBMI = pBMI;
	m_pBits = pBits;
	if (m_pBMI)
	{
		if (m_pBMI->bmiHeader.biBitCount <= 8)
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + (WORD)(m_pBMI->bmiHeader.biSize));
		else
			m_pColors = NULL;
		if (m_pBMI->bmiHeader.biSize == sizeof(BITMAPV4HEADER) ||
			m_pBMI->bmiHeader.biSize == sizeof(BITMAPV5HEADER))
		{
			LPBITMAPV4HEADER pBV4 = (LPBITMAPV4HEADER)m_pBMI;
			if (pBV4->bV4AlphaMask > 0 && m_pBMI->bmiHeader.biBitCount == 32)
				m_bAlpha = TRUE;
		}
		InitMasks();
		ComputeImageSize();
	}
	else
	{
		m_pColors = NULL;
		m_bAlpha = FALSE;
		m_dwImageSize = 0;
	}
}

BOOL CDib::SetBits(LPBYTE lpBits, DWORD dwSize)
{
	if (!m_pBMI)
		return FALSE;

	if (!m_pBits)
	{
		// Allocate Memory
		m_pBits = (LPBYTE)BIGALLOC(m_dwImageSize);
		if (m_pBits == NULL)
			return FALSE;
	}
	if (lpBits)
		memcpy(m_pBits, lpBits, MIN(m_dwImageSize, dwSize));

	return TRUE;
}

BOOL CDib::SetBits(LPBYTE lpBits)
{
	if (!m_pBMI)
		return FALSE;

	if (!m_pBits)
	{
		// Allocate Memory
		m_pBits = (LPBYTE)BIGALLOC(m_dwImageSize);
		if (m_pBits == NULL)
			return FALSE;
	}
	if (lpBits)
		memcpy(m_pBits, lpBits, m_dwImageSize);

	return TRUE;
}

BOOL CDib::IsSameBMI(LPBITMAPINFO lpBMI) const
{
	return m_pBMI && lpBMI && GetBMISize() == GetBMISize(lpBMI) &&
			memcmp(m_pBMI, lpBMI, GetBMISize()) == 0;
}

BOOL CDib::IsSameBMI(LPBITMAPINFO lpBMI1, LPBITMAPINFO lpBMI2)
{
	return lpBMI1 && lpBMI2 && GetBMISize(lpBMI1) == GetBMISize(lpBMI2) &&
			memcmp(lpBMI1, lpBMI2, GetBMISize(lpBMI1)) == 0;
}

BOOL CDib::SetBMI(LPBITMAPINFO lpBMI)
{
	if (!lpBMI)
		return FALSE;

	// Get BMI size of param
	DWORD dwBMISize = GetBMISize(lpBMI);
	
	// Check whether we can recycle the current m_pBMI
	if (m_pBMI && GetBMISize() == dwBMISize && memcmp(m_pBMI, lpBMI, dwBMISize) == 0)
		return TRUE;

	// Free, also bits if available because we have a new format and/or image size
	Free();

	// Allocate memory for Header
	m_pBMI = (LPBITMAPINFO)new BYTE[dwBMISize];
	if (m_pBMI == NULL)
		return FALSE;

	memcpy(m_pBMI, lpBMI, dwBMISize); 

	if (m_pBMI->bmiHeader.biBitCount <= 8)
		m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + (WORD)(m_pBMI->bmiHeader.biSize));
	else
		m_pColors = NULL;

	ComputeImageSize();

	CreatePaletteFromBMI();

	InitMasks();

	return TRUE;
}

LPBYTE CDib::GetBits(BOOL bDeleteDibSection/*=FALSE*/)
{
	DibSectionToBits(FALSE, bDeleteDibSection);
	return m_pBits;
}

DWORD CDib::GetBMISize(LPBITMAPINFO pBMI)
{
	if (!pBMI)
		return 0;
		
	if ((pBMI->bmiHeader.biCompression == BI_RGB)	||
		(pBMI->bmiHeader.biCompression == BI_RLE4)	||
		(pBMI->bmiHeader.biCompression == BI_RLE8)	||
		(pBMI->bmiHeader.biCompression == BI_BITFIELDS))
	{
		if (pBMI->bmiHeader.biClrUsed != 0)
			return (pBMI->bmiHeader.biSize + pBMI->bmiHeader.biClrUsed*sizeof(RGBQUAD));
		else
		{
			if (pBMI->bmiHeader.biBitCount >= 16)
			{
				if (pBMI->bmiHeader.biCompression == BI_BITFIELDS		&&
					pBMI->bmiHeader.biSize == sizeof(BITMAPINFOHEADER)	&&
					(pBMI->bmiHeader.biBitCount == 16 || pBMI->bmiHeader.biBitCount == 32))
					return (pBMI->bmiHeader.biSize + 3 * sizeof(DWORD)); // Bitfield Masks
				else
					return pBMI->bmiHeader.biSize;
			}
			else
				return (pBMI->bmiHeader.biSize + (1 << pBMI->bmiHeader.biBitCount)*sizeof(RGBQUAD));
		}
	}
	else
		return pBMI->bmiHeader.biSize;
}

DWORD CDib::GetBMISize() const
{
	if (!m_pBMI)
		return 0;
		
	if ((m_pBMI->bmiHeader.biCompression == BI_RGB)		||
		(m_pBMI->bmiHeader.biCompression == BI_RLE4)	||
		(m_pBMI->bmiHeader.biCompression == BI_RLE8)	||
		(m_pBMI->bmiHeader.biCompression == BI_BITFIELDS))
	{
		if (m_pBMI->bmiHeader.biClrUsed != 0)
			return (m_pBMI->bmiHeader.biSize + m_pBMI->bmiHeader.biClrUsed*sizeof(RGBQUAD));
		else
		{
			if (m_pBMI->bmiHeader.biBitCount >= 16)
			{
				if (m_pBMI->bmiHeader.biCompression == BI_BITFIELDS			&&
					m_pBMI->bmiHeader.biSize == sizeof(BITMAPINFOHEADER)	&&
					(m_pBMI->bmiHeader.biBitCount == 16 || m_pBMI->bmiHeader.biBitCount == 32))
					return (m_pBMI->bmiHeader.biSize + 3 * sizeof(DWORD)); // Bitfield Masks
				else
					return m_pBMI->bmiHeader.biSize;
			}
			else
				return (m_pBMI->bmiHeader.biSize + (1 << m_pBMI->bmiHeader.biBitCount)*sizeof(RGBQUAD));
		}
	}
	else
		return m_pBMI->bmiHeader.biSize;
}

CString CDib::FileSignatureToExtension(LPCTSTR lpszPathName)
{
	try
	{
		// Init vars
		BYTE buf[8]; // adapt that if having longer signatures!
		memset(buf, 0, sizeof(buf));
		const BYTE bmp[] = {0x42, 0x4D};
		const BYTE gif[] = {0x47, 0x49, 0x46};
		const BYTE jpg[] = {0xFF, 0xD8, 0xFF};
		const BYTE tiff_le[] = {0x49, 0x49, 0x2A, 0x00};
		const BYTE tiff_be[] = {0x4D, 0x4D, 0x00, 0x2A};
		const BYTE png[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
		const BYTE emf[] = {0x01, 0x00, 0x00, 0x00};

		// Read signature
		CFile file(lpszPathName, CFile::modeRead | CFile::shareDenyNone);
		UINT uiReadBytes = sizeof(buf);
		if (file.GetLength() < (ULONGLONG)sizeof(buf))
			uiReadBytes = (UINT)file.GetLength();
		file.Read(buf, uiReadBytes);
		file.Close();

		// Compare signatures
		if (memcmp(buf, bmp, sizeof(bmp)) == 0)
			return _T(".bmp");
		else if (memcmp(buf, gif, sizeof(gif)) == 0)
			return _T(".gif");
		else if (memcmp(buf, jpg, sizeof(jpg)) == 0)
			return _T(".jpg");
		else if (memcmp(buf, tiff_le, sizeof(tiff_le)) == 0 ||
				memcmp(buf, tiff_be, sizeof(tiff_be)) == 0)
			return _T(".tif");
		else if (memcmp(buf, png, sizeof(png)) == 0)
			return _T(".png");
		else if (buf[0] == 0x0A && buf[2] == 0x01)
			return _T(".pcx");
		else if (memcmp(buf, emf, sizeof(emf)) == 0)
			return _T(".emf");
		else
			return _T("");
	}
	catch (CFileException* e)
	{
		e->Delete();	
		return _T("");
	}
}

BOOL CDib::FileCheck(LPCTSTR lpszPathName)
{
	// Check File Existence
	if (!IsFile(lpszPathName))
		return FALSE;
	else
		return FileCheckInternal(lpszPathName);
}

BOOL CDib::FileCheckInternal(LPCTSTR lpszPathName)
{
	// Check various sizes
	if (LoadImage(lpszPathName, 0, 0, 0, TRUE, TRUE))
	{
		CString s, t;
		if (GetWidth() == 0 || GetHeight() == 0)
		{
#ifdef _DEBUG
			s.Format(_T("FileCheck(%s):\n"), lpszPathName);
#endif
			t = _T("Zero Width and/or Height!");
			s += t;
		
			TRACE(s);
			if (m_bShowMessageBoxOnError)
				::AfxMessageBox(s, MB_ICONSTOP);

			return FALSE;
		}
		else if ((GetWidth() / GetHeight() > SECURITY_MAX_RATIO) ||
				(GetHeight() / GetWidth() > SECURITY_MAX_RATIO))
		{
#ifdef _DEBUG
			s.Format(_T("FileCheck(%s):\n"), lpszPathName);
#endif
			t.Format(_T("A Width of %i with a Height of %i are not Supported!"),
																	GetWidth(),
																	GetHeight());
			s += t;
		
			TRACE(s);
			if (m_bShowMessageBoxOnError)
				::AfxMessageBox(s, MB_ICONSTOP);

			return FALSE;
		}
		else if ((GetWidth() * GetHeight()) > SECURITY_MAX_PIX_AREA)
		{
#ifdef _DEBUG
			s.Format(_T("FileCheck(%s):\n"), lpszPathName);
#endif
			t.Format(_T("Sorry, such big files are not Supported!"));
			s += t;
		
			TRACE(s);
			if (m_bShowMessageBoxOnError)
				::AfxMessageBox(s, MB_ICONSTOP);

			return FALSE;
		}
		else if (GetWidth() > SECURITY_MAX_ALLOWED_WIDTH)
		{
#ifdef _DEBUG
			s.Format(_T("FileCheck(%s):\n"), lpszPathName);
#endif
			t.Format(_T("A Width of %i is not Supported!"), GetWidth());
			s += t;
		
			TRACE(s);
			if (m_bShowMessageBoxOnError)
				::AfxMessageBox(s, MB_ICONSTOP);

			return FALSE;
		}
		else if (GetHeight() > SECURITY_MAX_ALLOWED_HEIGHT)
		{
#ifdef _DEBUG
			s.Format(_T("FileCheck(%s):\n"), lpszPathName);
#endif
			t.Format(_T("A Height of %i is not Supported!"), GetHeight());
			s += t;
		
			TRACE(s);
			if (m_bShowMessageBoxOnError)
				::AfxMessageBox(s, MB_ICONSTOP);

			return FALSE;
		}
		else
			return TRUE;
	}
	else
		return FALSE;
}

BOOL CDib::LoadImage(LPCTSTR lpszPathName,
					 int nMaxSizeX/*=0*/,
					 int nMaxSizeY/*=0*/,
					 int nPageNum/*=0*/,
					 BOOL bDecompressBmp/*=TRUE*/,
					 BOOL bOnlyHeader/*=FALSE*/,
					 CWnd* pProgressWnd/*=NULL*/,
					 BOOL bProgressSend/*=TRUE*/,
					 CWorkerThread* pThread/*=NULL*/)
{
	// Check File Existence
	if (!IsFile(lpszPathName))
		return FALSE;

	// Check File Sanity
	if (!bOnlyHeader)
	{
		if (!FileCheckInternal(lpszPathName))
			return FALSE;
	}

	// Load it
	CString sExt(FileSignatureToExtension(lpszPathName));
	if (sExt == _T(""))
		sExt = ::GetFileExt(lpszPathName);
	if ((sExt == _T(".bmp")) || (sExt == _T(".dib")))
	{
		return LoadBMP(	lpszPathName,					// Loads bmp as BMI + bits
						bOnlyHeader,
						bDecompressBmp,
						pProgressWnd,
						bProgressSend,
						pThread);
	}
	else if (sExt == _T(".emf"))
	{
		return LoadEMF(lpszPathName);					// Loads emf as BMI + bits
	}
	else if (sExt == _T(".png"))
	{
		return LoadPNG(	lpszPathName,					// Loads png as BMI + bits	
						TRUE,							// Load Alpha Channel
						bOnlyHeader,
						pProgressWnd,
						bProgressSend,
						pThread);
	}
	else if (IsJPEGExt(sExt))
	{
		if (bOnlyHeader || (nMaxSizeX <= 0 && nMaxSizeY <= 0))
			return LoadJPEG(lpszPathName,				// Loads jpg as BMI + bits
							1,
							FALSE,
							bOnlyHeader,
							pProgressWnd,
							bProgressSend,
							pThread);
		else
		{
			// Free
			m_FileInfo.Clear();
			m_bAlpha = FALSE;
			m_bGrayscale = FALSE;
			Free();

			// Init The BITMAPINFO of the main Dib and load the bits
			// to m_pPreviewDib. The edges of the Preview Dib 
			// are limited (more or less) around nMaxSizeX and/or nMaxSizeY.
			// Note: CreatePreviewDibFromJPEG returns FALSE and does nothing
			// if the Preview Dib would be the same size as the Full Dib
			if (!CreatePreviewDibFromJPEG(	lpszPathName,
											nMaxSizeX,
											nMaxSizeY,
											pProgressWnd,
											bProgressSend,
											pThread))
			{
				if (pThread && pThread->DoExit())
					return FALSE;
				return LoadJPEG(lpszPathName,			// Loads jpg as BMI + bits
								1,
								FALSE,
								bOnlyHeader,
								pProgressWnd,
								bProgressSend,
								pThread);
			}
			else
				return TRUE;
		}
	}
	else if (IsTIFFExt(sExt))
	{
		return LoadTIFF(lpszPathName,					// Loads tif as BMI + bits
						nPageNum,						// Load the given page
						bOnlyHeader,
						pProgressWnd,
						bProgressSend,
						pThread);
	}
	else if (sExt == _T(".pcx"))
	{
		return LoadPCX(	lpszPathName,					// Loads pcx as BMI + bits
						bOnlyHeader,
						pProgressWnd,
						bProgressSend,
						pThread);
	}
	else if (sExt == _T(".gif"))
	{
		return LoadGIF(	lpszPathName,					// Loads gif as BMI + bits
						bOnlyHeader,
						pProgressWnd,
						bProgressSend,
						pThread);
	}
	else
		return SUCCEEDED(LoadWIC(lpszPathName, bOnlyHeader));
}

/*

Necessary decoders
------------------

.heic
free (since end of 2020 it's hard to find PCs where it's still possible to install it):
https://www.microsoft.com/p/hevc-video-extensions-from-device-manufacturer/9n4wgh0z6vhq
or the payed one:
https://www.microsoft.com/p/hevc-video-extensions/9nmzlz57r3t7

.avif
https://www.microsoft.com/p/av1-video-extension/9mvzqvxjbq9v

Note: both .heic and .avif are based on the HEIF container format and both need the usually already installed:
https://www.microsoft.com/p/heif-image-extensions/9pmmsr1cgpwg

.webp
https://www.microsoft.com/p/webp-image-extensions/9pg2dk419drg


For older Windows versions
--------------------------

.heic
https://www.copytrans.net/copytransheic/

Note: on Windows 10 CopyTrans is uninstalling the HEIF container format (HEIF Image Extensions),
so that .avif stops to work, thus after installing CopyTrans we have to reinstall:
https://www.microsoft.com/p/heif-image-extensions/9pmmsr1cgpwg
 
.webp
https://storage.googleapis.com/downloads.webmproject.org/releases/webp/WebpCodecSetup.exe
or:
http://downloads.webmproject.org/releases/webp/WebpCodecSetup.exe

*/
HRESULT CDib::LoadWIC(LPCTSTR lpszPathName, BOOL bOnlyHeader/*=FALSE*/)
{
	// Free
	m_FileInfo.Clear();
	m_bAlpha = FALSE;
	m_bGrayscale = FALSE;
	Free();

	// Init COM
	HRESULT hr;
	::CoInitialize(NULL);
	{
		CComPtr<IWICImagingFactory> pFactory;
		CComPtr<IWICStream> pStream;
		CComPtr<IWICBitmapDecoder> pDecoder;
		CComPtr<IWICBitmapFrameDecode> pFrame;

		// Check file size
		ULARGE_INTEGER FileSize = ::GetFileSize64(lpszPathName);
		m_FileInfo.m_dwFileSize = (DWORD)FileSize.QuadPart;
		if (FileSize.QuadPart == 0)
		{
			hr = WINCODEC_ERR_UNEXPECTEDSIZE;
			goto exit;
		}

		// Create the COM imaging factory
		hr = ::CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFactory));
		if (FAILED(hr))
			goto exit;

		// Special treatment for .heic
		BOOL bDoCreateStream = FALSE;
		CString sExt(::GetFileExt(lpszPathName));
		if (sExt == _T(".heic"))
		{
			// Higher priority for CopyTrans because the Microsoft Heif may not
			// be able to open the file if "HEVC Video Extensions" is missing
			const GUID GUID_CopyTransContainerFormatHeif = { 0x502a9ac6,0x64f9,0x43cb,{0x82,0x44,0xba,0x76,0x6b,0x0c,0x1b,0x0b} };
			hr = pFactory->CreateDecoder(GUID_CopyTransContainerFormatHeif, NULL, &pDecoder);
			if (SUCCEEDED(hr))
				bDoCreateStream = TRUE;
			else
			{
				const GUID GUID_MicrosoftContainerFormatHeif = { 0xe1e62521,0x6787,0x405b,{0xa3,0x39,0x50,0x07,0x15,0xb5,0x76,0x3f} }; // = GUID_ContainerFormatHeif
				hr = pFactory->CreateDecoder(GUID_MicrosoftContainerFormatHeif, NULL, &pDecoder);
				if (SUCCEEDED(hr))
					bDoCreateStream = TRUE;
			}
		}

		// Create the stream for the above found decoder
		if (bDoCreateStream)
		{
			hr = pFactory->CreateStream(&pStream);
			if (FAILED(hr))
				goto exit;
			hr = pStream->InitializeFromFilename(lpszPathName, GENERIC_READ);
			if (FAILED(hr))
				goto exit;
			hr = pDecoder->Initialize(pStream, WICDecodeMetadataCacheOnDemand);
			if (FAILED(hr))
				goto exit;
		}
		// Normally create decoder from file
		else
		{
			// The above special code for .heic is necessary because passing a preferred decoder
			// vendor here is not working (this probably because the container format GUIDs of
			// CopyTrans and Microsoft Heif are different). I did try it with these Vendor GUIDs:
			// GUID_VendorMicrosoft
			// const GUID GUID_VendorCopyTransUrsaMinorLtd = { 0xe27ae9ae,0xd620,0x4aeb,{0xad,0x02,0xe2,0xae,0x03,0x10,0x42,0x34} };
			hr = pFactory->CreateDecoderFromFilename(lpszPathName, NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pDecoder);
			if (FAILED(hr))
				goto exit;
		}

		// Get container GUID
		GUID containerFormatGUID;
		hr = pDecoder->GetContainerFormat(&containerFormatGUID);
		if (FAILED(hr))
			goto exit;

		// Retrieve the First frame from the image
		UINT uiFramesCount = 0;
		hr = pDecoder->GetFrameCount(&uiFramesCount);
		if (FAILED(hr))
			goto exit;
		if (uiFramesCount == 0)
		{
			hr = WINCODEC_ERR_FRAMEMISSING;
			goto exit;
		}
		hr = pDecoder->GetFrame(0, &pFrame);
		if (FAILED(hr))
			goto exit;
		
		// Get format
		UINT uiWidth;
		UINT uiHeight;
		hr = pFrame->GetSize(&uiWidth, &uiHeight);
		if (FAILED(hr))
			goto exit;
		double dXDpi;
		double dYDpi;
		hr = pFrame->GetResolution(&dXDpi, &dYDpi);
		if (FAILED(hr))
		{
			dXDpi = 0.0;
			dYDpi = 0.0;
		}
		WICPixelFormatGUID pixelFormatGUID;
		hr = pFrame->GetPixelFormat(&pixelFormatGUID);
		if (FAILED(hr))
			goto exit;
		UINT uiBitsPerPixelDst;
		BOOL bConvert32bpp;
		if (pixelFormatGUID == GUID_WICPixelFormat24bppBGR)
		{
			uiBitsPerPixelDst = m_FileInfo.m_nBpp = 24;
			bConvert32bpp = FALSE;
			m_bAlpha = m_FileInfo.m_bAlphaChannel = FALSE;
		}
		else if (pixelFormatGUID == GUID_WICPixelFormat32bppBGR)
		{
			uiBitsPerPixelDst = m_FileInfo.m_nBpp = 32;
			bConvert32bpp = FALSE;
			m_bAlpha = m_FileInfo.m_bAlphaChannel = FALSE;
		}
		else if (pixelFormatGUID == GUID_WICPixelFormat32bppBGRA)
		{
			uiBitsPerPixelDst = m_FileInfo.m_nBpp = 32;
			bConvert32bpp = FALSE;
			m_bAlpha = m_FileInfo.m_bAlphaChannel = TRUE;
		}
		else
		{
			// Get bpp and alpha channel availability from pixelFormatGUID
			UINT uiBitsPerPixelSrc = 32;
			BOOL bSupportsTransparency = FALSE;
			CComPtr<IWICComponentInfo> pComponentInfo;
			hr = pFactory->CreateComponentInfo(pixelFormatGUID, &pComponentInfo);
			if (SUCCEEDED(hr))
			{
				CComPtr<IWICPixelFormatInfo2> pPixelFormatInfo2;
				hr = pComponentInfo->QueryInterface(IID_IWICPixelFormatInfo2, (void**)&pPixelFormatInfo2);
				if (SUCCEEDED(hr))
				{
					pPixelFormatInfo2->GetBitsPerPixel(&uiBitsPerPixelSrc);
					pPixelFormatInfo2->SupportsTransparency(&bSupportsTransparency);
				}
				else
				{
					CComPtr<IWICPixelFormatInfo> pPixelFormatInfo;
					hr = pComponentInfo->QueryInterface(IID_IWICPixelFormatInfo, (void**)&pPixelFormatInfo);
					if (SUCCEEDED(hr))
						pPixelFormatInfo->GetBitsPerPixel(&uiBitsPerPixelSrc);
				}
			}
			m_FileInfo.m_nBpp = uiBitsPerPixelSrc;
			m_FileInfo.m_bAlphaChannel = bSupportsTransparency;
			
			// The SupportsTransparency() functions returns FALSE for indexed transparency formats.
			// Because of that we cannot use bSupportsTransparency to decide whether to convert to
			// 32 bpp with or without alpha -> we always convert to 32 bpp with alpha!
			// https://docs.microsoft.com/en-us/windows/win32/api/wincodec/nf-wincodec-iwicpixelformatinfo2-supportstransparency
			uiBitsPerPixelDst = 32;
			bConvert32bpp = TRUE;
			m_bAlpha = TRUE;
		}

		// Allocate BMI
		m_pBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER)];
		if (!m_pBMI)
		{
			hr = E_OUTOFMEMORY;
			goto exit;
		}

		// DWORD aligned target DIB ScanLineSize 
		DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES(uiWidth * uiBitsPerPixelDst);

		// Fill in the DIB header fields
		m_pBMI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		m_pBMI->bmiHeader.biWidth = uiWidth;
		m_pBMI->bmiHeader.biHeight = uiHeight;
		m_pBMI->bmiHeader.biPlanes = 1;
		m_pBMI->bmiHeader.biBitCount = (WORD)uiBitsPerPixelDst;
		m_pBMI->bmiHeader.biCompression = BI_RGB;
		m_pBMI->bmiHeader.biSizeImage = uiHeight * uiDIBScanLineSize;
		m_pBMI->bmiHeader.biXPelsPerMeter = (LONG)PIXPERMETER(dXDpi);
		m_pBMI->bmiHeader.biYPelsPerMeter = (LONG)PIXPERMETER(dYDpi);
		m_pBMI->bmiHeader.biClrUsed = 0;
		m_pBMI->bmiHeader.biClrImportant = 0;
		m_pColors = NULL;
		m_dwImageSize = m_pBMI->bmiHeader.biSizeImage;

		// Init Masks For 16 and 32 bits Pictures
		InitMasks();

		// Init File Info
		m_FileInfo.m_nType = CFileInfo::WIC;
		m_FileInfo.m_nWidth = m_pBMI->bmiHeader.biWidth;
		m_FileInfo.m_nHeight = m_pBMI->bmiHeader.biHeight;
		m_FileInfo.m_nCompression = m_pBMI->bmiHeader.biCompression;
		m_FileInfo.m_dwImageSize = m_pBMI->bmiHeader.biSizeImage;
		m_FileInfo.m_nXPixsPerMeter = m_pBMI->bmiHeader.biXPelsPerMeter;
		m_FileInfo.m_nYPixsPerMeter = m_pBMI->bmiHeader.biYPelsPerMeter;
		m_FileInfo.m_nImageCount = 1; // just load the first image, do not care about other ones
		m_FileInfo.m_nImagePos = 0;

		// Read Metadata
		ReadMetadataWIC(containerFormatGUID, pFrame);

		// If only header wanted return now
		if (bOnlyHeader)
			goto exit;

		// Get bits
		if (!bConvert32bpp)
		{
			CComPtr<IWICBitmap> pBitmap;
			CComPtr<IWICBitmapLock> pBitmapLock;

			// Lock source bits
			WICRect rcLock = {0, 0, (INT)uiWidth, (INT)uiHeight};
			hr = pFactory->CreateBitmapFromSource(pFrame, WICBitmapCacheOnDemand, &pBitmap);
			if (FAILED(hr))
				goto exit;
			hr = pBitmap->Lock(&rcLock, WICBitmapLockRead, &pBitmapLock);
			if (FAILED(hr))
				goto exit;

			// Get source stride
			UINT uiStrideSrc;
			hr = pBitmapLock->GetStride(&uiStrideSrc);
			if (FAILED(hr))
				goto exit;

			// Get source buffer
			UINT cbBufferSize = 0;
			BYTE* pSrc = NULL;
			hr = pBitmapLock->GetDataPointer(&cbBufferSize, &pSrc);
			if (FAILED(hr))
				goto exit;

			// Allocate Bits
			m_pBits = (LPBYTE)BIGALLOC(m_dwImageSize);
			if (m_pBits == NULL)
			{
				hr = E_OUTOFMEMORY;
				goto exit;
			}

			// Copy bits (WIC image bits are always top-down)
			BYTE* pDst = m_pBits + (uiDIBScanLineSize * (uiHeight - 1));
			UINT uiStrideMin = min(uiStrideSrc, uiDIBScanLineSize);
			for (UINT line = 0; line < uiHeight; line++)
			{
				memcpy(pDst, pSrc, uiStrideMin);
				pSrc += uiStrideSrc;
				pDst -= uiDIBScanLineSize;
			}
		}
		else
		{
			CComPtr<IWICBitmapSource> pBitmap32;
			CComPtr<IWICBitmap> pBitmap;
			CComPtr<IWICBitmapLock> pBitmapLock;

			// Convert to 32bpp
			hr = WICConvertBitmapSource(m_bAlpha ? GUID_WICPixelFormat32bppBGRA : GUID_WICPixelFormat32bppBGR, pFrame, &pBitmap32);
			if (FAILED(hr))
				goto exit;

			// Lock source bits
			WICRect rcLock = {0, 0, (INT)uiWidth, (INT)uiHeight};
			hr = pFactory->CreateBitmapFromSource(pBitmap32, WICBitmapCacheOnDemand, &pBitmap);
			if (FAILED(hr))
				goto exit;
			hr = pBitmap->Lock(&rcLock, WICBitmapLockRead, &pBitmapLock);
			if (FAILED(hr))
				goto exit;

			// Get source stride
			UINT uiStrideSrc;
			hr = pBitmapLock->GetStride(&uiStrideSrc);
			if (FAILED(hr))
				goto exit;

			// Get source buffer
			UINT cbBufferSize = 0;
			BYTE* pSrc = NULL;
			hr = pBitmapLock->GetDataPointer(&cbBufferSize, &pSrc);
			if (FAILED(hr))
				goto exit;

			// Allocate Bits
			m_pBits = (LPBYTE)BIGALLOC(m_dwImageSize);
			if (m_pBits == NULL)
			{
				hr = E_OUTOFMEMORY;
				goto exit;
			}

			// Copy bits (WIC image bits are always top-down)
			BYTE* pDst = m_pBits + (uiDIBScanLineSize * (uiHeight - 1));
			UINT uiStrideMin = min(uiStrideSrc, uiDIBScanLineSize);
			for (UINT line = 0; line < uiHeight; line++)
			{
				memcpy(pDst, pSrc, uiStrideMin);
				pSrc += uiStrideSrc;
				pDst -= uiDIBScanLineSize;
			}
		}

		// Note: COM pointers must be released before 'CoUninitialize()' is called!
	}
	
exit:

	// Error message
	if (!SUCCEEDED(hr))
	{
		CString str;
#ifdef _DEBUG
		str.Format(_T("LoadWIC(%s):\n"), lpszPathName);
#endif
		str += _T("Cannot load file\n");
		TRACE(str);
		if (m_bShowMessageBoxOnError)
		{
			// On missing codec different errors are returned by CreateDecoderFromFilename()
			// (WINCODEC_ERR_COMPONENTNOTFOUND, WINCODEC_ERR_COMPONENTINITIALIZEFAILURE or ...),
			// but also Lock() can fail for example with E_FAIL or with 0xC00D5212. Better to 
			// always return "Codec Missing" if not having one of our assigned error codes:
			if (hr != WINCODEC_ERR_UNEXPECTEDSIZE	&&
				hr != WINCODEC_ERR_FRAMEMISSING		&&
				hr != E_OUTOFMEMORY)
			{
				CTaskDialog dlg(_T("<a href=\"") + CString(CODEC_MISSING_ONLINE_PAGE) + _T("\">Download</a>"),
								_T("Codec Missing"),
								APPNAME_NOEXT,
								TDCBF_OK_BUTTON,
								TDF_ENABLE_HYPERLINKS | TDF_USE_COMMAND_LINKS | TDF_SIZE_TO_CONTENT);
				dlg.SetMainIcon(TD_ERROR_ICON);
				dlg.DoModal();
			}
			else
				::AfxMessageBox(str, MB_ICONSTOP);
		}
	}

	// Uninit COM
	::CoUninitialize();

	return hr;
}

double CDib::ReadMetadataWICRational(PROPVARIANT& value)
{
	// Note: Microsoft doc inverted num & den:
	// https://docs.microsoft.com/en-us/windows/win32/wic/-wic-native-image-format-metadata-queries

	if (value.vt == VT_UI8)
		return (double)value.uhVal.LowPart / (double)value.uhVal.HighPart;
	else if (value.vt == VT_I8) // num & den can both be negative -> cast LowPart to LONG
		return (double)(LONG)value.hVal.LowPart / (double)value.hVal.HighPart;
	else
		return 0.0;
}

HRESULT CDib::ReadMetadataWIC(const GUID& containerFormatGUID, CComPtr<IWICBitmapFrameDecode> pFrame)
{
	// Native Image Format Metadata Queries:
	// https://docs.microsoft.com/en-us/windows/win32/wic/-wic-native-image-format-metadata-queries
	
	CComPtr<IWICMetadataQueryReader> pRootQueryReader;
	CStringW sPath(L"/ifd/");
	if (IsEqualGUID(containerFormatGUID, GUID_ContainerFormatJpeg))
		sPath = L"/app1" + sPath;
	HRESULT hr = pFrame->GetMetadataQueryReader(&pRootQueryReader);
	if (FAILED(hr))
		return hr;

	// Alloc/free metadata
	CMetadata* pMetadata = CDib::GetMetadata(); // this allocates if necessary
	if (!pMetadata)
		return E_OUTOFMEMORY;
	else
		pMetadata->Free(); // clear data

	PROPVARIANT value;
	::PropVariantInit(&value);

	// Macros to make the TAG path
	// Attention: GetMetadataByName() expects decimal values for the TAGs not hex!
	#define TIFFTAG_CONCAT(x) {ushort= ## x ## }
	#define EXIFTAG_CONCAT(x) exif/{ushort= ## x ## }
	#define GPSTAG_CONCAT(x) gps/{ushort= ## x ## }
	#define DO_STRINGIFY_HELPER(x) #x
	#define DO_STRINGIFY(x) DO_STRINGIFY_HELPER(x)
	#define DO_WCHAR_HELPER(x) L ## x
	#define DO_WCHAR(x) DO_WCHAR_HELPER(x)
	#define TIFFTAG_QUERY(x) DO_WCHAR(DO_STRINGIFY(TIFFTAG_CONCAT(x)))
	#define EXIFTAG_QUERY(x) DO_WCHAR(DO_STRINGIFY(EXIFTAG_CONCAT(x)))
	#define GPSTAG_QUERY(x) DO_WCHAR(DO_STRINGIFY(GPSTAG_CONCAT(x)))

	// TIFFTAG_ORIENTATION
	hr = pRootQueryReader->GetMetadataByName(sPath + TIFFTAG_QUERY(TIFFTAG_ORIENTATION), &value);
	if (SUCCEEDED(hr) && (value.vt == VT_UI2))
		m_pMetadata->m_ExifInfo.Orientation = value.uiVal;
	::PropVariantClear(&value);

	// TIFFTAG_XRESOLUTION
	hr = pRootQueryReader->GetMetadataByName(sPath + TIFFTAG_QUERY(TIFFTAG_XRESOLUTION), &value);
	if (SUCCEEDED(hr))
		m_pMetadata->m_ExifInfo.Xresolution = (float)ReadMetadataWICRational(value);
	::PropVariantClear(&value);

	// TIFFTAG_YRESOLUTION
	hr = pRootQueryReader->GetMetadataByName(sPath + TIFFTAG_QUERY(TIFFTAG_YRESOLUTION), &value);
	if (SUCCEEDED(hr))
		m_pMetadata->m_ExifInfo.Yresolution = (float)ReadMetadataWICRational(value);
	::PropVariantClear(&value);

	// TIFFTAG_RESOLUTIONUNIT
	hr = pRootQueryReader->GetMetadataByName(sPath + TIFFTAG_QUERY(TIFFTAG_RESOLUTIONUNIT), &value);
	if (SUCCEEDED(hr) && (value.vt == VT_UI2))
	{
		switch (value.uiVal)
		{
			case 1: m_pMetadata->m_ExifInfo.ResolutionUnit = 1.0f;			break;	// inch
			case 2:	m_pMetadata->m_ExifInfo.ResolutionUnit = 1.0f;			break;	// inch
			case 3: m_pMetadata->m_ExifInfo.ResolutionUnit = 0.393701f;		break;	// centimeter
			case 4: m_pMetadata->m_ExifInfo.ResolutionUnit = 0.0393701f;	break;	// millimeter
			case 5: m_pMetadata->m_ExifInfo.ResolutionUnit = 0.0000393701f;	break;	// micrometer
		}
	}
	::PropVariantClear(&value);

	// TIFFTAG_MAKE
	hr = pRootQueryReader->GetMetadataByName(sPath + TIFFTAG_QUERY(TIFFTAG_MAKE), &value);
	if (SUCCEEDED(hr))
	{
		if (value.vt == VT_LPSTR)
		{
			strncpy(m_pMetadata->m_ExifInfo.CameraMake, value.pszVal, 31);
			m_pMetadata->m_ExifInfo.CameraMake[31] = '\0';
		}
		else if (value.vt == (VT_VECTOR | VT_LPSTR) && value.calpstr.cElems >= 1)
		{
			strncpy(m_pMetadata->m_ExifInfo.CameraMake, value.calpstr.pElems[0], 31);
			m_pMetadata->m_ExifInfo.CameraMake[31] = '\0';
		}
	}
	::PropVariantClear(&value);

	// TIFFTAG_MODEL
	hr = pRootQueryReader->GetMetadataByName(sPath + TIFFTAG_QUERY(TIFFTAG_MODEL), &value);
	if (SUCCEEDED(hr))
	{
		if (value.vt == VT_LPSTR)
		{
			strncpy(m_pMetadata->m_ExifInfo.CameraModel, value.pszVal, 39);
			m_pMetadata->m_ExifInfo.CameraModel[39] = '\0';
		}
		else if (value.vt == (VT_VECTOR | VT_LPSTR) && value.calpstr.cElems >= 1)
		{
			strncpy(m_pMetadata->m_ExifInfo.CameraModel, value.calpstr.pElems[0], 39);
			m_pMetadata->m_ExifInfo.CameraModel[39] = '\0';
		}
	}
	::PropVariantClear(&value);

	// TIFFTAG_IMAGEDESCRIPTION
	hr = pRootQueryReader->GetMetadataByName(sPath + TIFFTAG_QUERY(TIFFTAG_IMAGEDESCRIPTION), &value);
	if (SUCCEEDED(hr))
	{
		if (value.vt == VT_LPSTR)
		{
			strncpy(m_pMetadata->m_ExifInfo.ImageDescription, value.pszVal, MAX_IMAGE_DESC - 1);
			m_pMetadata->m_ExifInfo.ImageDescription[MAX_IMAGE_DESC - 1] = '\0';
		}
		else if (value.vt == (VT_VECTOR | VT_LPSTR) && value.calpstr.cElems >= 1)
		{
			strncpy(m_pMetadata->m_ExifInfo.ImageDescription, value.calpstr.pElems[0], MAX_IMAGE_DESC - 1);
			m_pMetadata->m_ExifInfo.ImageDescription[MAX_IMAGE_DESC - 1] = '\0';
		}
	}
	::PropVariantClear(&value);

	// TIFFTAG_SOFTWARE
	hr = pRootQueryReader->GetMetadataByName(sPath + TIFFTAG_QUERY(TIFFTAG_SOFTWARE), &value);
	if (SUCCEEDED(hr))
	{
		if (value.vt == VT_LPSTR)
		{
			strncpy(m_pMetadata->m_ExifInfo.Software, value.pszVal, MAX_SOFTWARE - 1);
			m_pMetadata->m_ExifInfo.Software[MAX_SOFTWARE - 1] = '\0';
		}
		else if (value.vt == (VT_VECTOR | VT_LPSTR) && value.calpstr.cElems >= 1)
		{
			strncpy(m_pMetadata->m_ExifInfo.Software, value.calpstr.pElems[0], MAX_SOFTWARE - 1);
			m_pMetadata->m_ExifInfo.Software[MAX_SOFTWARE - 1] = '\0';
		}
	}
	::PropVariantClear(&value);

	// TIFFTAG_ARTIST
	hr = pRootQueryReader->GetMetadataByName(sPath + TIFFTAG_QUERY(TIFFTAG_ARTIST), &value);
	if (SUCCEEDED(hr))
	{
		if (value.vt == VT_LPSTR)
		{
			strncpy(m_pMetadata->m_ExifInfo.Artist, value.pszVal, MAX_ARTIST - 1);
			m_pMetadata->m_ExifInfo.Artist[MAX_ARTIST - 1] = '\0';
		}
		else if (value.vt == (VT_VECTOR | VT_LPSTR) && value.calpstr.cElems >= 1)
		{
			strncpy(m_pMetadata->m_ExifInfo.Artist, value.calpstr.pElems[0], MAX_ARTIST - 1);
			m_pMetadata->m_ExifInfo.Artist[MAX_ARTIST - 1] = '\0';
		}
	}
	::PropVariantClear(&value);

	// TIFFTAG_COPYRIGHT
	hr = pRootQueryReader->GetMetadataByName(sPath + TIFFTAG_QUERY(TIFFTAG_COPYRIGHT), &value);
	if (SUCCEEDED(hr))
	{
		// Photographer Copyright
		if (value.vt == VT_LPSTR)
		{
			strncpy(m_pMetadata->m_ExifInfo.CopyrightPhotographer, value.pszVal, MAX_COPYRIGHT - 1);
			m_pMetadata->m_ExifInfo.CopyrightPhotographer[MAX_COPYRIGHT - 1] = '\0';
		}
		// Photographer and Editor Copyright
		else if (value.vt == (VT_VECTOR | VT_LPSTR))
		{
			// Photographer
			if (value.calpstr.cElems >= 1)
			{
				strncpy(m_pMetadata->m_ExifInfo.CopyrightPhotographer, value.calpstr.pElems[0], MAX_COPYRIGHT - 1);
				m_pMetadata->m_ExifInfo.CopyrightPhotographer[MAX_COPYRIGHT - 1] = '\0';
			}

			// Editor
			if (value.calpstr.cElems >= 2)
			{
				strncpy(m_pMetadata->m_ExifInfo.CopyrightEditor, value.calpstr.pElems[1], MAX_COPYRIGHT - 1);
				m_pMetadata->m_ExifInfo.CopyrightEditor[MAX_COPYRIGHT - 1] = '\0';
			}
		}
	}
	::PropVariantClear(&value);

	// TIFFTAG_DATETIME
	hr = pRootQueryReader->GetMetadataByName(sPath + TIFFTAG_QUERY(TIFFTAG_DATETIME), &value);
	if (SUCCEEDED(hr) && (value.vt == VT_LPSTR))
	{
		strncpy(m_pMetadata->m_ExifInfo.DateTime, value.pszVal, 19);
		m_pMetadata->m_ExifInfo.DateTime[19] = '\0';
	}
	::PropVariantClear(&value);

	// -------------
	// Exif specific
	// -------------

	// EXIFTAG_DATETIMEDIGITIZED
	hr = pRootQueryReader->GetMetadataByName(sPath + EXIFTAG_QUERY(EXIFTAG_DATETIMEDIGITIZED), &value);
	if (SUCCEEDED(hr) && (value.vt == VT_LPSTR))
	{
		strncpy(m_pMetadata->m_ExifInfo.DateTime, value.pszVal, 19);
		m_pMetadata->m_ExifInfo.DateTime[19] = '\0';
	}
	::PropVariantClear(&value);
	
	// EXIFTAG_DATETIMEORIGINAL (give highest priority)
	hr = pRootQueryReader->GetMetadataByName(sPath + EXIFTAG_QUERY(EXIFTAG_DATETIMEORIGINAL), &value);
	if (SUCCEEDED(hr) && (value.vt == VT_LPSTR))
	{
		strncpy(m_pMetadata->m_ExifInfo.DateTime, value.pszVal, 19);
		m_pMetadata->m_ExifInfo.DateTime[19] = '\0';
	}
	::PropVariantClear(&value);

	// EXIFTAG_EXIFVERSION
	hr = pRootQueryReader->GetMetadataByName(sPath + EXIFTAG_QUERY(EXIFTAG_EXIFVERSION), &value);
	if (SUCCEEDED(hr) && (value.vt == VT_BLOB))
	{
		memcpy(m_pMetadata->m_ExifInfo.Version, value.blob.pBlobData, min(4, value.blob.cbSize));
		m_pMetadata->m_ExifInfo.Version[4] = '\0';
	}
	::PropVariantClear(&value);

	// EXIFTAG_USERCOMMENT
	hr = pRootQueryReader->GetMetadataByName(sPath + EXIFTAG_QUERY(EXIFTAG_USERCOMMENT), &value);
	if (SUCCEEDED(hr) && (value.vt == VT_LPWSTR))
	{
		wcsncpy(m_pMetadata->m_ExifInfo.UserComment, value.pwszVal, MAX_USER_COMMENT - 1);
		m_pMetadata->m_ExifInfo.UserComment[MAX_USER_COMMENT - 1] = L'\0';
	}
	::PropVariantClear(&value);

	// EXIFTAG_FLASH
	hr = pRootQueryReader->GetMetadataByName(sPath + EXIFTAG_QUERY(EXIFTAG_FLASH), &value);
	if (SUCCEEDED(hr) && (value.vt == VT_UI2))
		m_pMetadata->m_ExifInfo.Flash = value.uiVal;
	::PropVariantClear(&value);

	// EXIFTAG_FOCALLENGTH
	hr = pRootQueryReader->GetMetadataByName(sPath + EXIFTAG_QUERY(EXIFTAG_FOCALLENGTH), &value);
	if (SUCCEEDED(hr))
		m_pMetadata->m_ExifInfo.FocalLength = (float)ReadMetadataWICRational(value);
	::PropVariantClear(&value);

	// EXIFTAG_FOCALLENGTHIN35MMFILM
	hr = pRootQueryReader->GetMetadataByName(sPath + EXIFTAG_QUERY(EXIFTAG_FOCALLENGTHIN35MMFILM), &value);
	if (SUCCEEDED(hr) && (value.vt == VT_UI2))
		m_pMetadata->m_ExifInfo.FocalLength35mmEquiv = value.uiVal;
	::PropVariantClear(&value);

	// EXIFTAG_DIGITALZOOMRATIO
	hr = pRootQueryReader->GetMetadataByName(sPath + EXIFTAG_QUERY(EXIFTAG_DIGITALZOOMRATIO), &value);
	if (SUCCEEDED(hr))
		m_pMetadata->m_ExifInfo.DigitalZoomRatio = (float)ReadMetadataWICRational(value);
	::PropVariantClear(&value);

	// EXIFTAG_EXPOSURETIME
	hr = pRootQueryReader->GetMetadataByName(sPath + EXIFTAG_QUERY(EXIFTAG_EXPOSURETIME), &value);
	if (SUCCEEDED(hr))
		m_pMetadata->m_ExifInfo.ExposureTime = (float)ReadMetadataWICRational(value);
	::PropVariantClear(&value);

	// EXIFTAG_BRIGHTNESSVALUE
	hr = pRootQueryReader->GetMetadataByName(sPath + EXIFTAG_QUERY(EXIFTAG_BRIGHTNESSVALUE), &value);
	if (SUCCEEDED(hr))
		m_pMetadata->m_ExifInfo.Brightness = (float)ReadMetadataWICRational(value);
	::PropVariantClear(&value);

	// EXIFTAG_FNUMBER
	hr = pRootQueryReader->GetMetadataByName(sPath + EXIFTAG_QUERY(EXIFTAG_FNUMBER), &value);
	if (SUCCEEDED(hr))
		m_pMetadata->m_ExifInfo.ApertureFNumber = (float)ReadMetadataWICRational(value);
	::PropVariantClear(&value);

	// EXIFTAG_SUBJECTDISTANCE
	hr = pRootQueryReader->GetMetadataByName(sPath + EXIFTAG_QUERY(EXIFTAG_SUBJECTDISTANCE), &value);
	if (SUCCEEDED(hr))
		m_pMetadata->m_ExifInfo.Distance = (float)ReadMetadataWICRational(value);
	::PropVariantClear(&value);

	// EXIFTAG_ISOSPEEDRATINGS or EXIFTAG_EXPOSUREINDEX
	hr = pRootQueryReader->GetMetadataByName(sPath + EXIFTAG_QUERY(EXIFTAG_ISOSPEEDRATINGS), &value);
	if (SUCCEEDED(hr) && (value.vt == VT_UI2))
		m_pMetadata->m_ExifInfo.ISOequivalent = value.uiVal;
	::PropVariantClear(&value);
	if (m_pMetadata->m_ExifInfo.ISOequivalent == 0)
	{
		hr = pRootQueryReader->GetMetadataByName(sPath + EXIFTAG_QUERY(EXIFTAG_EXPOSUREINDEX), &value);
		if (SUCCEEDED(hr))
			m_pMetadata->m_ExifInfo.ISOequivalent = (unsigned short)ReadMetadataWICRational(value);
		::PropVariantClear(&value);
	}

	// EXIFTAG_FOCALPLANEXRESOLUTION
	hr = pRootQueryReader->GetMetadataByName(sPath + EXIFTAG_QUERY(EXIFTAG_FOCALPLANEXRESOLUTION), &value);
	if (SUCCEEDED(hr))
		m_pMetadata->m_ExifInfo.FocalplaneXRes = (float)ReadMetadataWICRational(value);
	::PropVariantClear(&value);

	// EXIFTAG_FOCALPLANEYRESOLUTION
	hr = pRootQueryReader->GetMetadataByName(sPath + EXIFTAG_QUERY(EXIFTAG_FOCALPLANEYRESOLUTION), &value);
	if (SUCCEEDED(hr))
		m_pMetadata->m_ExifInfo.FocalplaneYRes = (float)ReadMetadataWICRational(value);
	::PropVariantClear(&value);

	// EXIFTAG_FOCALPLANERESOLUTIONUNIT
	hr = pRootQueryReader->GetMetadataByName(sPath + EXIFTAG_QUERY(EXIFTAG_FOCALPLANERESOLUTIONUNIT), &value);
	if (SUCCEEDED(hr) && (value.vt == VT_UI2))
	{
		switch (value.uiVal)
		{
			case 1: m_pMetadata->m_ExifInfo.FocalplaneUnits = 25.4f;	break;	// inch
			case 2: m_pMetadata->m_ExifInfo.FocalplaneUnits = 25.4f;	break;	// inch
			case 3: m_pMetadata->m_ExifInfo.FocalplaneUnits = 10.0f;	break;	// centimeter
			case 4: m_pMetadata->m_ExifInfo.FocalplaneUnits = 1.0f;		break;	// millimeter
			case 5: m_pMetadata->m_ExifInfo.FocalplaneUnits = 0.001f;	break;	// micrometer
		}
	}
	::PropVariantClear(&value);

	// EXIFTAG_EXPOSUREBIASVALUE
	hr = pRootQueryReader->GetMetadataByName(sPath + EXIFTAG_QUERY(EXIFTAG_EXPOSUREBIASVALUE), &value);
	if (SUCCEEDED(hr))
		m_pMetadata->m_ExifInfo.ExposureBias = (float)ReadMetadataWICRational(value);
	::PropVariantClear(&value);

	// EXIFTAG_WHITEBALANCE
	hr = pRootQueryReader->GetMetadataByName(sPath + EXIFTAG_QUERY(EXIFTAG_WHITEBALANCE), &value);
	if (SUCCEEDED(hr) && (value.vt == VT_UI2))
		m_pMetadata->m_ExifInfo.WhiteBalance = value.uiVal;
	::PropVariantClear(&value);
	
	// EXIFTAG_LIGHTSOURCE
	hr = pRootQueryReader->GetMetadataByName(sPath + EXIFTAG_QUERY(EXIFTAG_LIGHTSOURCE), &value);
	if (SUCCEEDED(hr) && (value.vt == VT_UI2))
		m_pMetadata->m_ExifInfo.LightSource = value.uiVal;
	::PropVariantClear(&value);

	// EXIFTAG_METERINGMODE
	hr = pRootQueryReader->GetMetadataByName(sPath + EXIFTAG_QUERY(EXIFTAG_METERINGMODE), &value);
	if (SUCCEEDED(hr) && (value.vt == VT_UI2))
		m_pMetadata->m_ExifInfo.MeteringMode = value.uiVal;
	::PropVariantClear(&value);

	// EXIFTAG_EXPOSUREPROGRAM
	hr = pRootQueryReader->GetMetadataByName(sPath + EXIFTAG_QUERY(EXIFTAG_EXPOSUREPROGRAM), &value);
	if (SUCCEEDED(hr) && (value.vt == VT_UI2))
		m_pMetadata->m_ExifInfo.ExposureProgram = value.uiVal;
	::PropVariantClear(&value);

	// EXIFTAG_EXPOSUREMODE
	hr = pRootQueryReader->GetMetadataByName(sPath + EXIFTAG_QUERY(EXIFTAG_EXPOSUREMODE), &value);
	if (SUCCEEDED(hr) && (value.vt == VT_UI2))
		m_pMetadata->m_ExifInfo.ExposureMode = value.uiVal;
	::PropVariantClear(&value);

	// EXIFTAG_SUBJECTDISTANCERANGE
	hr = pRootQueryReader->GetMetadataByName(sPath + EXIFTAG_QUERY(EXIFTAG_SUBJECTDISTANCERANGE), &value);
	if (SUCCEEDED(hr) && (value.vt == VT_UI2))
		m_pMetadata->m_ExifInfo.DistanceRange = value.uiVal;
	::PropVariantClear(&value);

	// EXIFTAG_COMPRESSEDBITSPERPIXEL
	hr = pRootQueryReader->GetMetadataByName(sPath + EXIFTAG_QUERY(EXIFTAG_COMPRESSEDBITSPERPIXEL), &value);
	if (SUCCEEDED(hr))
		m_pMetadata->m_ExifInfo.CompressionLevel = (float)ReadMetadataWICRational(value);
	::PropVariantClear(&value);
	
	// EXIFTAG_AMBIENT_TEMPERATURE
	hr = pRootQueryReader->GetMetadataByName(sPath + EXIFTAG_QUERY(EXIFTAG_AMBIENT_TEMPERATURE), &value);
	if (SUCCEEDED(hr))
		m_pMetadata->m_ExifInfo.AmbientTemperature = (float)ReadMetadataWICRational(value);
	::PropVariantClear(&value);

	// EXIFTAG_HUMIDITY
	hr = pRootQueryReader->GetMetadataByName(sPath + EXIFTAG_QUERY(EXIFTAG_HUMIDITY), &value);
	if (SUCCEEDED(hr))
		m_pMetadata->m_ExifInfo.Humidity = (float)ReadMetadataWICRational(value);
	::PropVariantClear(&value);

	// EXIFTAG_PRESSURE
	hr = pRootQueryReader->GetMetadataByName(sPath + EXIFTAG_QUERY(EXIFTAG_PRESSURE), &value);
	if (SUCCEEDED(hr))
		m_pMetadata->m_ExifInfo.Pressure = (float)ReadMetadataWICRational(value);
	::PropVariantClear(&value);

	// EXIFTAG_WATERDEPTH
	hr = pRootQueryReader->GetMetadataByName(sPath + EXIFTAG_QUERY(EXIFTAG_WATERDEPTH), &value);
	if (SUCCEEDED(hr))
		m_pMetadata->m_ExifInfo.WaterDepth = (float)ReadMetadataWICRational(value);
	::PropVariantClear(&value);

	// ------------
	// Gps specific
	// ------------

	// TAG_GPS_VERSION
	hr = pRootQueryReader->GetMetadataByName(sPath + GPSTAG_QUERY(TAG_GPS_VERSION), &value);
	if (SUCCEEDED(hr) && (value.vt == (VT_VECTOR | VT_UI1)))
	{
		m_pMetadata->m_ExifInfo.bGpsInfoPresent = true;
		memcpy(m_pMetadata->m_ExifInfo.GpsVersion, value.calpstr.pElems, min(4, value.calpstr.cElems));
	}
	::PropVariantClear(&value);

	// TAG_GPS_LAT_REF
	hr = pRootQueryReader->GetMetadataByName(sPath + GPSTAG_QUERY(TAG_GPS_LAT_REF), &value);
	if (SUCCEEDED(hr) && (value.vt == VT_LPSTR))
	{
		m_pMetadata->m_ExifInfo.bGpsInfoPresent = true;
		m_pMetadata->m_ExifInfo.GpsLatRef[0] = value.pszVal[0];
		m_pMetadata->m_ExifInfo.GpsLatRef[1] = '\0';
	}
	::PropVariantClear(&value);

	// TAG_GPS_LAT
	hr = pRootQueryReader->GetMetadataByName(sPath + GPSTAG_QUERY(TAG_GPS_LAT), &value);
	if (SUCCEEDED(hr) && (value.vt == (VT_VECTOR | VT_UI8)))
	{
		m_pMetadata->m_ExifInfo.bGpsInfoPresent = true;
		if (value.cauh.cElems >= 1)
			m_pMetadata->m_ExifInfo.GpsLat[GPS_DEGREE] = (float)((double)value.cauh.pElems[0].LowPart / (double)value.cauh.pElems[0].HighPart);
		if (value.cauh.cElems >= 2)
			m_pMetadata->m_ExifInfo.GpsLat[GPS_MINUTES] = (float)((double)value.cauh.pElems[1].LowPart / (double)value.cauh.pElems[1].HighPart);
		if (value.cauh.cElems >= 3)
			m_pMetadata->m_ExifInfo.GpsLat[GPS_SECONDS] = (float)((double)value.cauh.pElems[2].LowPart / (double)value.cauh.pElems[2].HighPart);
		CMetadata::GpsNormalizeCoord(m_pMetadata->m_ExifInfo.GpsLat[GPS_DEGREE], m_pMetadata->m_ExifInfo.GpsLat[GPS_MINUTES], m_pMetadata->m_ExifInfo.GpsLat[GPS_SECONDS]);
	}
	::PropVariantClear(&value);

	// TAG_GPS_LONG_REF
	hr = pRootQueryReader->GetMetadataByName(sPath + GPSTAG_QUERY(TAG_GPS_LONG_REF), &value);
	if (SUCCEEDED(hr) && (value.vt == VT_LPSTR))
	{
		m_pMetadata->m_ExifInfo.bGpsInfoPresent = true;
		m_pMetadata->m_ExifInfo.GpsLongRef[0] = value.pszVal[0];
		m_pMetadata->m_ExifInfo.GpsLongRef[1] = '\0';
	}
	::PropVariantClear(&value);

	// TAG_GPS_LONG
	hr = pRootQueryReader->GetMetadataByName(sPath + GPSTAG_QUERY(TAG_GPS_LONG), &value);
	if (SUCCEEDED(hr) && (value.vt == (VT_VECTOR | VT_UI8)))
	{
		m_pMetadata->m_ExifInfo.bGpsInfoPresent = true;
		if (value.cauh.cElems >= 1)
			m_pMetadata->m_ExifInfo.GpsLong[GPS_DEGREE] = (float)((double)value.cauh.pElems[0].LowPart / (double)value.cauh.pElems[0].HighPart);
		if (value.cauh.cElems >= 2)
			m_pMetadata->m_ExifInfo.GpsLong[GPS_MINUTES] = (float)((double)value.cauh.pElems[1].LowPart / (double)value.cauh.pElems[1].HighPart);
		if (value.cauh.cElems >= 3)
			m_pMetadata->m_ExifInfo.GpsLong[GPS_SECONDS] = (float)((double)value.cauh.pElems[2].LowPart / (double)value.cauh.pElems[2].HighPart);
		CMetadata::GpsNormalizeCoord(m_pMetadata->m_ExifInfo.GpsLong[GPS_DEGREE], m_pMetadata->m_ExifInfo.GpsLong[GPS_MINUTES], m_pMetadata->m_ExifInfo.GpsLong[GPS_SECONDS]);
	}
	::PropVariantClear(&value);

	// TAG_GPS_ALT_REF
	hr = pRootQueryReader->GetMetadataByName(sPath + GPSTAG_QUERY(TAG_GPS_ALT_REF), &value);
	if (SUCCEEDED(hr) && (value.vt == VT_UI1))
	{
		m_pMetadata->m_ExifInfo.bGpsInfoPresent = true;
		m_pMetadata->m_ExifInfo.GpsAltRef = value.bVal;
	}
	::PropVariantClear(&value);

	// TAG_GPS_ALT
	hr = pRootQueryReader->GetMetadataByName(sPath + GPSTAG_QUERY(TAG_GPS_ALT), &value);
	if (SUCCEEDED(hr))
	{
		m_pMetadata->m_ExifInfo.bGpsInfoPresent = true;
		m_pMetadata->m_ExifInfo.GpsAlt = (float)ReadMetadataWICRational(value);
	}
	::PropVariantClear(&value);

	// TAG_GPS_TIMESTAMP
	hr = pRootQueryReader->GetMetadataByName(sPath + GPSTAG_QUERY(TAG_GPS_TIMESTAMP), &value);
	if (SUCCEEDED(hr) && (value.vt == (VT_VECTOR | VT_UI8)))
	{
		m_pMetadata->m_ExifInfo.bGpsInfoPresent = true;
		if (value.cauh.cElems >= 1)
			m_pMetadata->m_ExifInfo.GpsTime[GPS_HOUR] = (float)((double)value.cauh.pElems[0].LowPart / (double)value.cauh.pElems[0].HighPart);
		if (value.cauh.cElems >= 2)
			m_pMetadata->m_ExifInfo.GpsTime[GPS_MINUTES] = (float)((double)value.cauh.pElems[1].LowPart / (double)value.cauh.pElems[1].HighPart);
		if (value.cauh.cElems >= 3)
			m_pMetadata->m_ExifInfo.GpsTime[GPS_SECONDS] = (float)((double)value.cauh.pElems[2].LowPart / (double)value.cauh.pElems[2].HighPart);
	}
	::PropVariantClear(&value);

	// TAG_GPS_MAPDATUM
	hr = pRootQueryReader->GetMetadataByName(sPath + GPSTAG_QUERY(TAG_GPS_MAPDATUM), &value);
	if (SUCCEEDED(hr) && (value.vt == VT_LPSTR))
	{
		m_pMetadata->m_ExifInfo.bGpsInfoPresent = true;
		strncpy(m_pMetadata->m_ExifInfo.GpsMapDatum, value.pszVal, 19);
		m_pMetadata->m_ExifInfo.GpsMapDatum[19] = '\0';
	}
	::PropVariantClear(&value);

	return S_OK;
}

BOOL CDib::LoadDibSection(HBITMAP hDibSection)
{
	// Check to see if it is a DibSection
	if (!IsDibSection(hDibSection))
		return FALSE;

	LPBYTE pBits;	

	// Create a DC
	HDC hDC = ::GetDC(NULL);
	if (!hDC)
		return FALSE;

	// Free
	m_FileInfo.Clear();
	m_bAlpha = FALSE;
	m_bGrayscale = FALSE;
	Free();	

	// Get the BMI
	if (!DibSectionInitBMI(hDibSection))
	{
		::ReleaseDC(NULL, hDC);
		return FALSE;
	}

	// Get the bits
	pBits = (LPBYTE)BIGALLOC(m_dwImageSize);
	if (!pBits)
	{
		::ReleaseDC(NULL, hDC);
		return FALSE;
	}
	if (!::GetDIBits(hDC, hDibSection, 0, GetHeight(), pBits, m_pBMI, DIB_RGB_COLORS))
	{
		BIGFREE(pBits);
		::ReleaseDC(NULL, hDC);
		return FALSE;
	}

	// Create the DibSection
	m_hDibSection = ::CreateDIBSection(hDC, (const BITMAPINFO*)m_pBMI,
								 DIB_RGB_COLORS, (void**)&m_pDibSectionBits, NULL, 0);

	if (!m_hDibSection)
	{
		ShowLastError(_T("CreateDIBSection()"));
		::ReleaseDC(NULL, hDC);
		BIGFREE(pBits);
		m_pDibSectionBits = NULL;
		return FALSE;
	}

	memcpy(m_pDibSectionBits, (void*)pBits, GetImageSize());
	::ReleaseDC(NULL, hDC);
	BIGFREE(pBits);

	// Init File Info
	m_FileInfo.m_nType = CFileInfo::BMP;
	m_FileInfo.m_nWidth = m_pBMI->bmiHeader.biWidth;
	m_FileInfo.m_nHeight = m_pBMI->bmiHeader.biHeight;
	m_FileInfo.m_nBpp = m_pBMI->bmiHeader.biBitCount;
	m_FileInfo.m_nCompression = m_pBMI->bmiHeader.biCompression;
	m_FileInfo.m_nColorSpace = CFileInfo::COLORSPACE_RGB;
	m_FileInfo.m_dwImageSize = m_pBMI->bmiHeader.biSizeImage;
	m_FileInfo.m_nXPixsPerMeter = m_pBMI->bmiHeader.biXPelsPerMeter;
	m_FileInfo.m_nYPixsPerMeter = m_pBMI->bmiHeader.biYPelsPerMeter;
	m_FileInfo.m_nNumColors = m_pBMI->bmiHeader.biClrUsed;
	m_FileInfo.m_bPalette = (m_FileInfo.m_nBpp <= 8);
	m_FileInfo.m_nImageCount = 1;
	m_FileInfo.m_nImagePos = 0;
	if (m_FileInfo.m_nCompression == BI_BITFIELDS)
	{
		if (m_FileInfo.m_nBpp == 32)
		{
			m_FileInfo.m_dwRedMask = m_dwRedMask32; 
			m_FileInfo.m_dwGreenMask = m_dwGreenMask32; 
			m_FileInfo.m_dwBlueMask = m_dwBlueMask32;
		}
		else
		{
			m_FileInfo.m_dwRedMask = m_wRedMask16; 
			m_FileInfo.m_dwGreenMask = m_wGreenMask16; 
			m_FileInfo.m_dwBlueMask = m_wBlueMask16;
		}
	}

    return TRUE;
}

BOOL CDib::LoadDibSection(LPCTSTR lpszPathName)
{
	BOOL res;

	CString sPathName(lpszPathName);
	if (sPathName.IsEmpty())
		return FALSE;

	// Clear File Info, Alpha and Grayscale
	m_FileInfo.Clear();
	m_bAlpha = FALSE;
	m_bGrayscale = FALSE;

	// Get the File Size
	CFile file;
	if (!file.Open(lpszPathName, CFile::modeRead | CFile::shareDenyNone))
		return FALSE;
	else
		m_FileInfo.m_dwFileSize = (DWORD)file.GetLength();
	file.Close();
	if (m_FileInfo.m_dwFileSize == 0)
		return FALSE;

	HBITMAP hBitmap = (HBITMAP)::LoadImage(NULL, lpszPathName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	if (hBitmap == NULL)
		return FALSE;

	// WinCE only loads as DDB, Desktop Windows can load it as DDB or DIBSECTION (see last flag of LoadImage)
	if (IsDibSection(hBitmap))
	{	
		res = TRUE;
		AttachDibSection(hBitmap); // Will Also Free eventual old Bitmap
	}
	else
	{
		res = SetDibSectionFromDDB(hBitmap, NULL); // Will Also Free eventual old Bitmap
		::DeleteObject(hBitmap);
	}

	// Init File Info
	m_FileInfo.m_nType = CFileInfo::BMP;
	m_FileInfo.m_nWidth = m_pBMI->bmiHeader.biWidth;
	m_FileInfo.m_nHeight = m_pBMI->bmiHeader.biHeight;
	m_FileInfo.m_nBpp = m_pBMI->bmiHeader.biBitCount;
	m_FileInfo.m_nCompression = m_pBMI->bmiHeader.biCompression;
	m_FileInfo.m_nColorSpace = CFileInfo::COLORSPACE_RGB;
	m_FileInfo.m_dwImageSize = m_pBMI->bmiHeader.biSizeImage;
	m_FileInfo.m_nXPixsPerMeter = m_pBMI->bmiHeader.biXPelsPerMeter;
	m_FileInfo.m_nYPixsPerMeter = m_pBMI->bmiHeader.biYPelsPerMeter;
	m_FileInfo.m_nNumColors = m_pBMI->bmiHeader.biClrUsed;
	m_FileInfo.m_bPalette = (m_FileInfo.m_nBpp <= 8);
	m_FileInfo.m_nImageCount = 1;
	m_FileInfo.m_nImagePos = 0;
	if (m_FileInfo.m_nCompression == BI_BITFIELDS)
	{
		if (m_FileInfo.m_nBpp == 32)
		{
			m_FileInfo.m_dwRedMask = m_dwRedMask32; 
			m_FileInfo.m_dwGreenMask = m_dwGreenMask32; 
			m_FileInfo.m_dwBlueMask = m_dwBlueMask32;
		}
		else
		{
			m_FileInfo.m_dwRedMask = m_wRedMask16; 
			m_FileInfo.m_dwGreenMask = m_wGreenMask16; 
			m_FileInfo.m_dwBlueMask = m_wBlueMask16;
		}
	}

	return res;
}

BOOL CDib::LoadDibSectionRes(HINSTANCE hInst, LPCTSTR lpResourceName)
{
	BOOL res;

	// Clear File Info, Alpha and Grayscale
	m_FileInfo.Clear();
	m_bAlpha = FALSE;
	m_bGrayscale = FALSE;

	HBITMAP hBitmap = (HBITMAP)::LoadImage(hInst, lpResourceName, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_DEFAULTCOLOR);
	if (hBitmap == NULL)
		return FALSE;

	// WinCE only loads as DDB, Desktop Windows can load it as DDB or DIBSECTION (see last flag of LoadImage)
	if (IsDibSection(hBitmap))
	{	
		res = TRUE;
		AttachDibSection(hBitmap); // Will Also Free eventual old Bitmap
	}
	else
	{
		res = SetDibSectionFromDDB(hBitmap, NULL); // Will Also Free eventual old Bitmap
		::DeleteObject(hBitmap);
	}

	// Init File Info
	m_FileInfo.m_nType = CFileInfo::BMP;
	m_FileInfo.m_nWidth = m_pBMI->bmiHeader.biWidth;
	m_FileInfo.m_nHeight = m_pBMI->bmiHeader.biHeight;
	m_FileInfo.m_nBpp = m_pBMI->bmiHeader.biBitCount;
	m_FileInfo.m_nCompression = m_pBMI->bmiHeader.biCompression;
	m_FileInfo.m_nColorSpace = CFileInfo::COLORSPACE_RGB;
	m_FileInfo.m_dwImageSize = m_pBMI->bmiHeader.biSizeImage;
	m_FileInfo.m_nXPixsPerMeter = m_pBMI->bmiHeader.biXPelsPerMeter;
	m_FileInfo.m_nYPixsPerMeter = m_pBMI->bmiHeader.biYPelsPerMeter;
	m_FileInfo.m_nNumColors = m_pBMI->bmiHeader.biClrUsed;
	m_FileInfo.m_bPalette = (m_FileInfo.m_nBpp <= 8);
	m_FileInfo.m_nImageCount = 1;
	m_FileInfo.m_nImagePos = 0;
	if (m_FileInfo.m_nCompression == BI_BITFIELDS)
	{
		if (m_FileInfo.m_nBpp == 32)
		{
			m_FileInfo.m_dwRedMask = m_dwRedMask32; 
			m_FileInfo.m_dwGreenMask = m_dwGreenMask32; 
			m_FileInfo.m_dwBlueMask = m_dwBlueMask32;
		}
		else
		{
			m_FileInfo.m_dwRedMask = m_wRedMask16; 
			m_FileInfo.m_dwGreenMask = m_wGreenMask16; 
			m_FileInfo.m_dwBlueMask = m_wBlueMask16;
		}
	}

	return res;
}

BOOL CDib::LoadEMF(LPCTSTR lpszPathName)
{
	// Free
	m_FileInfo.Clear();
	m_bAlpha = FALSE;
	m_bGrayscale = FALSE;

	try
	{
		CString sPathName(lpszPathName);
		if (sPathName.IsEmpty())
			throw (int)EMF_E_ZEROPATH;

		CFile file(lpszPathName, CFile::modeRead | CFile::shareDenyNone);
		m_FileInfo.m_dwFileSize = (DWORD)file.GetLength();
		if (m_FileInfo.m_dwFileSize == 0)
			throw (int)EMF_E_FILEEMPTY;		

		// Get the Handle from the enhanced metafile
		HENHMETAFILE hemf = ::GetEnhMetaFile(lpszPathName);
			
		// Get the header from the enhanced metafile
		ENHMETAHEADER emh;
		memset(&emh, 0, sizeof(ENHMETAHEADER));
		emh.nSize = sizeof(ENHMETAHEADER);
		if (::GetEnhMetaFileHeader(hemf, sizeof(ENHMETAHEADER), &emh) == 0)
		{
			::DeleteEnhMetaFile(hemf);
			throw (int)EMF_E_BADEMF;
		}
		
		// Get the DC of the Window
		HDC hDC = ::GetDC(NULL);

		// DPI
		double dXDpi = 0.0;
		double dYDpi = 0.0;
		if (emh.szlMillimeters.cx > 0)
			dXDpi = 25.4 * (double)emh.szlDevice.cx / (double)emh.szlMillimeters.cx;
		if (emh.szlMillimeters.cy > 0)
			dYDpi = 25.4 * (double)emh.szlDevice.cy / (double)emh.szlMillimeters.cy;
		if (dXDpi <= 0.0)
			dXDpi = DEFAULT_DPI;
		if (dYDpi <= 0.0)
			dYDpi = DEFAULT_DPI;

		// Note: rclFrame and rclBounds include the right and bottom edges.
		// In EditPaste() I add + 1 to the right and bottom coordinates,
		// here it's not working like this... I add + 1 when creating the Bitmap.

		// Get dimension
		int cx = emh.rclBounds.right - emh.rclBounds.left;
		int cy = emh.rclBounds.bottom - emh.rclBounds.top;

		// rclFrame specifies the dimensions in .01 millimeter units
		// -> convert to device units
		if (cx <= 0)
			cx = Round((emh.rclFrame.right - emh.rclFrame.left) * dXDpi / 2540.0);
		if (cy <= 0)
			cy = Round((emh.rclFrame.bottom - emh.rclFrame.top) * dYDpi / 2540.0);

		// Drawing rectangle
		RECT rc = {0, 0, cx, cy};

		// Create a Memory DC
		HDC hMemDC = ::CreateCompatibleDC(hDC); 
		
		// Create a compatible bitmap
		HBITMAP hBitmap = ::CreateCompatibleBitmap(hDC, cx + 1, cy + 1);
		if (hBitmap == NULL)
		{
			::DeleteEnhMetaFile(hemf);
			::DeleteDC(hMemDC);
			::ReleaseDC(NULL, hDC);
			throw (int)EMF_E_NOMEM;
		}
		
		// Select the bitmap into the Mem DC
		HBITMAP hBitmapOld = (HBITMAP)::SelectObject(hMemDC, hBitmap);
		
		// Paint Background
		COLORREF crOldColor = ::SetBkColor(hMemDC, m_crBackgroundColor);
		::ExtTextOut(hMemDC, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
		::SetBkColor(hMemDC, crOldColor);

		// Get Palette
		HPALETTE hPal = NULL;
		UINT uiPalEntries = ::GetEnhMetaFilePaletteEntries(hemf, 0, NULL);
		if (uiPalEntries > 0)
		{
			LPLOGPALETTE lpPal = (LPLOGPALETTE)new BYTE[sizeof(LOGPALETTE) +
														sizeof(PALETTEENTRY) * uiPalEntries];
			if (lpPal)
			{
				lpPal->palVersion = PALVERSION_DEFINE;
				lpPal->palNumEntries = ::GetEnhMetaFilePaletteEntries(	hemf,
																		uiPalEntries,
																		lpPal->palPalEntry);
				hPal = ::CreatePalette(lpPal);
				delete [] lpPal;
			}
		}

		// Now play the enhanced metafile into the memory DC,
		// ignore its return value it may be FALSE even if successful
		::PlayEnhMetaFile(hMemDC, hemf, &rc);

		// DDB -> CDib bits
		SetBitsFromDDB(hBitmap, hPal);

		// Set DPI
		if (GetBMIH())
		{
			GetBMIH()->biXPelsPerMeter = (LONG)Round(dXDpi * 100.0 / 2.54);
			GetBMIH()->biYPelsPerMeter = (LONG)Round(dYDpi * 100.0 / 2.54);
		}

		// Clean-Up
		::SelectObject(hMemDC, hBitmapOld);
		::DeleteEnhMetaFile(hemf);
		::DeleteObject(hBitmap);
		if (hPal)
			::DeleteObject(hPal);
		::DeleteDC(hMemDC);
		::ReleaseDC(NULL, hDC);

		// Init File Info
		m_FileInfo.m_nType = CFileInfo::EMF;
		m_FileInfo.m_nWidth = m_pBMI->bmiHeader.biWidth;
		m_FileInfo.m_nHeight = m_pBMI->bmiHeader.biHeight;
		m_FileInfo.m_nBpp = m_pBMI->bmiHeader.biBitCount;
		m_FileInfo.m_nCompression = m_pBMI->bmiHeader.biCompression;
		m_FileInfo.m_nColorSpace = CFileInfo::COLORSPACE_RGB;
		m_FileInfo.m_dwImageSize = m_pBMI->bmiHeader.biSizeImage;
		m_FileInfo.m_nXPixsPerMeter = m_pBMI->bmiHeader.biXPelsPerMeter;
		m_FileInfo.m_nYPixsPerMeter = m_pBMI->bmiHeader.biYPelsPerMeter;
		m_FileInfo.m_nNumColors = m_pBMI->bmiHeader.biClrUsed;
		m_FileInfo.m_bPalette = (m_FileInfo.m_nBpp <= 8);
		m_FileInfo.m_nImageCount = 1;
		m_FileInfo.m_nImagePos = 0;
		if (m_FileInfo.m_nCompression == BI_BITFIELDS)
		{
			if (m_FileInfo.m_nBpp == 32)
			{
				m_FileInfo.m_dwRedMask = m_dwRedMask32; 
				m_FileInfo.m_dwGreenMask = m_dwGreenMask32; 
				m_FileInfo.m_dwBlueMask = m_dwBlueMask32;
			}
			else
			{
				m_FileInfo.m_dwRedMask = m_wRedMask16; 
				m_FileInfo.m_dwGreenMask = m_wGreenMask16; 
				m_FileInfo.m_dwBlueMask = m_wBlueMask16;
			}
		}

		return TRUE;
	}
	catch (CFileException* e)
	{
		TCHAR szCause[255];
		CString str;
#ifdef _DEBUG
		str = _T("LoadEMF: ");
#endif
		e->GetErrorMessage(szCause, 255);
		str += szCause;
		str += _T("\n");
		TRACE(str);
		if (m_bShowMessageBoxOnError)
			::AfxMessageBox(str, MB_ICONSTOP);
		e->Delete();
		return FALSE;
	}
	catch (int error_code)
	{
		CString str;
#ifdef _DEBUG
		str.Format(_T("LoadEMF(%s):\n"), lpszPathName);
#endif
		switch(error_code)
		{
			case EMF_E_ZEROPATH :		str += _T("The file name is zero\n");
			break;
			case EMF_E_NOMEM :			str += _T("Could not alloc memory\n");
			break;
			case EMF_E_BADEMF :			str += _T("Corrupted or unsupported EMF\n");
			break;
			case EMF_E_FILEEMPTY :		str += _T("File is empty\n");
			break;
			default:					str += _T("Unspecified error\n");
			break;
		}
		
		TRACE(str);
		if (m_bShowMessageBoxOnError)
			::AfxMessageBox(str, MB_ICONSTOP);

		return FALSE;
	}
}

BOOL CDib::SaveEMF(LPCTSTR lpszPathName, HDC hRefDC/*=NULL*/)
{
	try
	{
		CString sPathName(lpszPathName);
		if (sPathName.IsEmpty())
			throw (int)EMF_E_ZEROPATH;

		DWORD dwAttrib = ::GetFileAttributes(lpszPathName);
		if ((dwAttrib != 0xFFFFFFFF) && (dwAttrib & FILE_ATTRIBUTE_READONLY))
			throw (int)EMF_E_FILEREADONLY;

		if (!m_pBits)
		{
			if (!DibSectionToBits())
				throw (int)EMF_E_BADBMP;
		}

		if (!m_pBits || !m_pBMI)
			throw (int)EMF_E_BADBMP;

		if (IsCompressed())
		{
			if (!Decompress(GetBitCount())) // Decompress
				throw (int)EMF_E_BADBMP;
		}

		// Create MemDC
		HDC hdcMem;
		if (!hRefDC)
		{
			HDC hDC = ::GetDC(NULL);
			hdcMem = ::CreateCompatibleDC(hDC);
			::ReleaseDC(NULL, hDC);
		}
		else
			hdcMem = ::CreateCompatibleDC(hRefDC);
		
		// Update m_hDibSection without deleting the bits!
		BitsToDibSection(TRUE, FALSE);
		HBITMAP hOldBitmap = (HBITMAP)::SelectObject(hdcMem, m_hDibSection);

		// Create EMF and get the emf DC
		HDC emfdc = ::CreateEnhMetaFile(hdcMem, lpszPathName, NULL, NULL);
		
		// Blit
		::BitBlt(emfdc,0, 0, GetWidth(), GetHeight(), hdcMem, 0, 0, SRCCOPY);

		// Clean-Up
		if (m_hDibSection)
		{
			::DeleteObject(m_hDibSection);
			m_hDibSection = NULL;
			m_pDibSectionBits = NULL;
		}
		::SelectObject(hdcMem, hOldBitmap);
		HENHMETAFILE hemf = ::CloseEnhMetaFile(emfdc);
		::DeleteEnhMetaFile(hemf);
		::DeleteDC(hdcMem);
	
		return TRUE;
	}
	catch (CFileException* e)
	{
		TCHAR szCause[255];
		CString str;
#ifdef _DEBUG
		str = _T("SaveEMF: ");
#endif
		e->GetErrorMessage(szCause, 255);
		str += szCause;
		str += _T("\n");
		TRACE(str);
		if (m_bShowMessageBoxOnError)
			::AfxMessageBox(str, MB_ICONSTOP);
		e->Delete();
		return FALSE;
	}
	catch (int error_code)
	{
		CString str;
#ifdef _DEBUG
		str.Format(_T("SaveEMF(%s):\n"), lpszPathName);
#endif
		switch (error_code)
		{
			case EMF_E_ZEROPATH :		str += _T("The file name is zero\n");
			break;
			case EMF_E_FILEREADONLY :	str += _T("The file is read only\n");
			break;
			case EMF_E_BADBMP :			str += _T("Corrupted or unsupported DIB\n");
			break;
			default:					str += _T("Unspecified error\n");
			break;
		}

		TRACE(str);
		if (m_bShowMessageBoxOnError)
			::AfxMessageBox(str, MB_ICONSTOP);
		
		return FALSE;
	}
}

BOOL CDib::LoadDibSectionRes(HINSTANCE hInst, UINT uID)
{
	return LoadDibSectionRes(hInst, MAKEINTRESOURCE(uID)); 
}

BOOL CDib::AttachDibSection(HBITMAP hDibSection)
{
	if (hDibSection == NULL)
		return FALSE;

	if (IsDibSection(hDibSection) == FALSE)
		return FALSE;

	// Free up any resource we may currently have
	Free();

	m_hDibSection = hDibSection;

	// Create the Palette
	CreatePaletteFromDibSection();
	
	return DibSectionInitBMI();
}

HBITMAP CDib::GetSafeHandle()
{
	if (!this)
		return NULL;

	BitsToDibSection();

	return m_hDibSection; 
}

HBITMAP CDib::GetDDB(HDC hDC/*=NULL*/)
{
	if (!DibSectionToBits())
		return NULL;

	BOOL bReleaseDC = FALSE;
	if (hDC == NULL)
	{
		hDC = ::GetDC(NULL);
		bReleaseDC = TRUE;
	}
    HBITMAP hBitmap = ::CreateDIBitmap(	hDC,
										(LPBITMAPINFOHEADER)m_pBMI,
										CBM_INIT,
										m_pBits,
										m_pBMI,
										DIB_RGB_COLORS);
	
	if (bReleaseDC)
		::ReleaseDC(NULL, hDC);

	return hBitmap;
}

BOOL CDib::SetBitsFromDDB(CBitmap* pBitmap, CPalette* pPal)
{
	HBITMAP hBitmap = (HBITMAP)pBitmap->GetSafeHandle();
	if (hBitmap == NULL)
		return FALSE;

	HPALETTE hPal = (HPALETTE)pPal->GetSafeHandle();

	return SetBitsFromDDB(hBitmap, hPal);
}

BOOL CDib::SetBitsFromDDB(HBITMAP hBitmap, HPALETTE hPal)
{
	if (hBitmap == NULL)
		return FALSE;

	// If it is already a DibSection return FALSE!
	if (IsDibSection(hBitmap))
		return FALSE;

	// Free up any resource we may currently have
	Free();

	// Get bitmap information
	BITMAP bm;
	if (!::GetObject(hBitmap, sizeof(bm), (LPVOID)&bm)) return FALSE;

	// Compute the size of the infoheader and the color table
	int nColors;
	if (bm.bmBitsPixel >= 24)
		nColors = 1 << (bm.bmPlanes * 24);
	else
		nColors = 1 << (bm.bmPlanes * bm.bmBitsPixel);

	// If a palette has not been supplied use Halftone Palette
	CPalette Pal;
	if (hPal == NULL)
	{
		if (nColors > 256)
			CreateHalftonePalette(&Pal, 256);
		else
			CreateHalftonePalette(&Pal, nColors);
		hPal = (HPALETTE)Pal.GetSafeHandle();
	}

	// Adjust nColors
	if(nColors > 256) nColors = 0;

	// We need a device context to get the DIB from
	HDC hDC = ::GetDC(NULL);
	HPALETTE hOldPal = ::SelectPalette(hDC, hPal, FALSE);
	::RealizePalette(hDC);

	// Allocate memory for Header and Color Table
	m_pBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER)
									+ nColors * sizeof(RGBQUAD)];
	if (m_pBMI == NULL)
	{
		::SelectPalette(hDC, hPal, FALSE);
		::ReleaseDC(NULL, hDC);
		return FALSE;
	}

	// Initialize the BITMAPINFOHEADER
	m_pBMI->bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);
	m_pBMI->bmiHeader.biWidth			= bm.bmWidth;
	m_pBMI->bmiHeader.biHeight 			= bm.bmHeight;
	m_pBMI->bmiHeader.biPlanes 			= 1;
	m_pBMI->bmiHeader.biBitCount		= (WORD)(bm.bmPlanes * bm.bmBitsPixel);
	m_pBMI->bmiHeader.biCompression		= BI_RGB;
	m_pBMI->bmiHeader.biSizeImage		= 0;
	m_pBMI->bmiHeader.biXPelsPerMeter	= 0;
	m_pBMI->bmiHeader.biYPelsPerMeter	= 0;
	m_pBMI->bmiHeader.biClrUsed			= 0;
	m_pBMI->bmiHeader.biClrImportant	= 0;

	// Call GetDIBits with a NULL lpBits param, so the device driver 
	// will calculate the biSizeImage field 
	GetDIBits(hDC, hBitmap, 0L, GetHeight(), NULL, m_pBMI, DIB_RGB_COLORS);

	ComputeImageSize();

	m_pBits = (LPBYTE)BIGALLOC(m_dwImageSize);
	if (m_pBits == NULL)
	{
		delete [] m_pBMI;
		m_pBMI = NULL;
		m_pColors = NULL;
		::SelectPalette(hDC, hPal, FALSE);
		::ReleaseDC(NULL, hDC);
		return FALSE;
	}

	// FINALLY get the DIB
	BOOL bGotBits = GetDIBits(hDC, hBitmap,
				0L,							// Start scan line
				(DWORD)GetHeight(),			// # of scan lines
				m_pBits, 					// address for bitmap bits
				m_pBMI,						// address of bitmapinfo
				DIB_RGB_COLORS);			// Use RGB for color table

	if (!bGotBits)
	{
		delete [] m_pBMI;
		m_pBMI = NULL;
		m_pColors = NULL;
		BIGFREE(m_pBits);
		m_pBits = NULL;
		::SelectPalette(hDC, hPal, FALSE);
		::ReleaseDC(NULL, hDC);
		return FALSE;
	}

	if (m_pBMI->bmiHeader.biBitCount <= 8)
		m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + (WORD)(m_pBMI->bmiHeader.biSize));
	else
		m_pColors = NULL;
	CreatePaletteFromBMI();

	::SelectPalette(hDC, hOldPal, FALSE);
	::ReleaseDC(NULL, hDC);

	// Init Masks For 16 and 32 bits Pictures
	InitMasks();

	return TRUE;
}

BOOL CDib::SetDibSectionFromDDB(CBitmap* pBitmap, CPalette* pPal)
{
	HBITMAP hBitmap = (HBITMAP)pBitmap->GetSafeHandle();
	if (hBitmap == NULL)
		return FALSE;

	HPALETTE hPal = (HPALETTE)pPal->GetSafeHandle();

	return SetDibSectionFromDDB(hBitmap, hPal);
}

BOOL CDib::SetDibSectionFromDDB(HBITMAP hBitmap, HPALETTE hPal)
{
    if (hBitmap == NULL)
		return FALSE;

	// If it is already a DibSection return FALSE!
	if (IsDibSection(hBitmap))
		return FALSE;

	// Free up any resource we may currently have
	Free();

    // Get bitmap information
    BITMAP bm;
    if (!::GetObject(hBitmap, sizeof(bm), (LPVOID)&bm))
		return FALSE;

	// Compute the size of the infoheader and the color table
	int nColors;
	if (bm.bmBitsPixel >= 24)
		nColors = 1 << (bm.bmPlanes * 24);
	else
		nColors = 1 << (bm.bmPlanes * bm.bmBitsPixel);
	
	// If a palette has not been supplied use Halftone Palette
	CPalette Pal;
	if (hPal == NULL)
	{
		if (nColors > 256)
			CreateHalftonePalette(&Pal, 256);
		else
			CreateHalftonePalette(&Pal, nColors);
		hPal = (HPALETTE)Pal.GetSafeHandle();
	}

	// Adjust nColors
	if (nColors > 256) nColors = 0;

    // We need a device context to get the DIB from
	HDC hDC = ::GetDC(NULL);
	HPALETTE hOldPal = ::SelectPalette(hDC, hPal, FALSE);
	::RealizePalette(hDC);

	// Allocate memory for Header and Color Table
	m_pBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER)
									+ nColors * sizeof(RGBQUAD)];
	if (m_pBMI == NULL)
	{
		::SelectPalette(hDC, hPal, FALSE);
		::ReleaseDC(NULL, hDC);
		return FALSE;
	}

    // Initialize the BITMAPINFOHEADER
	m_pBMI->bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);
	m_pBMI->bmiHeader.biWidth			= bm.bmWidth;
	m_pBMI->bmiHeader.biHeight 			= bm.bmHeight;
	m_pBMI->bmiHeader.biPlanes 			= 1;
	m_pBMI->bmiHeader.biBitCount		= (WORD)(bm.bmPlanes * bm.bmBitsPixel);
	m_pBMI->bmiHeader.biCompression		= BI_RGB;
	m_pBMI->bmiHeader.biSizeImage		= 0;
	m_pBMI->bmiHeader.biXPelsPerMeter	= 0;
	m_pBMI->bmiHeader.biYPelsPerMeter	= 0;
	m_pBMI->bmiHeader.biClrUsed			= 0;
	m_pBMI->bmiHeader.biClrImportant	= 0;

	ComputeImageSize();

    // Create it!
    m_hDibSection = ::CreateDIBSection(hDC, 
                                 (const BITMAPINFO*)m_pBMI,
                                 DIB_RGB_COLORS,
                                 (void**)&m_pDibSectionBits, 
                                 NULL, 0);
	if (m_hDibSection == NULL)
	{
		ShowLastError(_T("CreateDIBSection()"));
		::SelectPalette(hDC, hPal, FALSE);
		::ReleaseDC(NULL, hDC);
		return FALSE;
	}

    // Need to copy the supplied bitmap onto the newly created DIBsection
    HDC hMemDC, hCopyDC;
	hMemDC = ::CreateCompatibleDC(hDC);
	hCopyDC = ::CreateCompatibleDC(hDC);
	HPALETTE hOldMemPal = ::SelectPalette(hMemDC, hPal, FALSE);
	::RealizePalette(hMemDC);
	HPALETTE hOldCopyPal = ::SelectPalette(hCopyDC, hPal, FALSE);
	::RealizePalette(hCopyDC);
    HBITMAP hOldMemBitmap  = (HBITMAP)::SelectObject(hMemDC,  hBitmap);
    HBITMAP hOldCopyBitmap = (HBITMAP)::SelectObject(hCopyDC, m_hDibSection);

	// Copy
	::BitBlt(hCopyDC, 0, 0, bm.bmWidth, bm.bmHeight, hMemDC, 0, 0, SRCCOPY);

	// Get Colors from the DibSection
	if (m_pBMI->bmiHeader.biBitCount <= 8)
	{
		m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + (WORD)(m_pBMI->bmiHeader.biSize));
		::GetDIBColorTable(hMemDC, 0, nColors, m_pColors);
	}
	else
		m_pColors = NULL;
	CreatePaletteFromBMI();

	// Free up
	::SelectObject(hMemDC, hOldMemBitmap);
    ::SelectObject(hCopyDC, hOldCopyBitmap);
	::SelectPalette(hMemDC, hOldMemPal, FALSE);
	::SelectPalette(hCopyDC, hOldCopyPal, FALSE);
    ::SelectPalette(hDC, hOldPal, FALSE);
	::DeleteDC(hMemDC);
	::DeleteDC(hCopyDC);
	::ReleaseDC(NULL, hDC);

    return TRUE;
}

BOOL CDib::BitsToDibSection(BOOL bForceNewDibSection/*=FALSE*/, BOOL bDeleteBits/*=TRUE*/)
{
	if (m_hDibSection && !bForceNewDibSection)
		return TRUE;

	if ((m_pBits == NULL) || (m_pBMI == NULL))
		return FALSE;

	if (m_hDibSection)
	{
		::DeleteObject(m_hDibSection);
		m_hDibSection = NULL;
		m_pDibSectionBits = NULL;
	}

	// Uncompress because CreateDIBSection fails with compressed Bitmaps
	if (IsCompressed())
	{
  		if (!Decompress(GetBitCount())) // Decompress
			return FALSE;
	}

	// Create a DC which will be used to get DIB, then create DIBsection
	HDC hDC = ::GetDC(NULL);
	if (!hDC)
		return FALSE;

	// Create the DibSection
	m_hDibSection = ::CreateDIBSection(	hDC, (const BITMAPINFO*)m_pBMI,
										DIB_RGB_COLORS, (void**)&m_pDibSectionBits,
										NULL, 0);
	if (!m_hDibSection)
	{
		ShowLastError(_T("CreateDIBSection()"));
		::ReleaseDC(NULL, hDC);
		m_pDibSectionBits = NULL;
		return FALSE;
	}
	else
		::ReleaseDC(NULL, hDC);

	// Mem Copy
	memcpy(m_pDibSectionBits, (void*)m_pBits, GetImageSize());

	// Free?
	if (bDeleteBits)
	{
		BIGFREE(m_pBits);
		m_pBits = NULL;
		ResetColorUndo();
	}

    return TRUE;
}

BOOL CDib::DibSectionToBits(BOOL bForceNewBits/*=FALSE*/, BOOL bDeleteDibSection/*=TRUE*/)
{
	if (m_pBits && !bForceNewBits)
		return TRUE;

	if (!m_hDibSection || !m_pBMI)
		return FALSE;

	UnMapBMP();

	if (m_pBits)
	{
		BIGFREE(m_pBits);
		m_pBits = NULL;
	}
	ResetColorUndo();

	HDC hDC = ::GetDC(NULL);

	m_pBits = (LPBYTE)BIGALLOC(m_dwImageSize);
	if (!m_pBits)
	{
		::ReleaseDC(NULL, hDC);
		return FALSE;
	}
	if (m_pDibSectionBits)
		memcpy(m_pBits, m_pDibSectionBits, m_dwImageSize);
	else if (!::GetDIBits(hDC, m_hDibSection, 0, GetHeight(), m_pBits, m_pBMI, DIB_RGB_COLORS))
	{
		BIGFREE(m_pBits);
		m_pBits = NULL;
		::ReleaseDC(NULL, hDC);
		return FALSE;
	}
	
	::ReleaseDC(NULL, hDC);

	if (bDeleteDibSection)
	{
		::DeleteObject(m_hDibSection);
		m_hDibSection = NULL;
		m_pDibSectionBits = NULL;
	}

	return TRUE;
}

// Clipboard support
#ifdef _DEBUG
void CDib::EnumCF()
{
	if (::OpenClipboard(NULL))
	{
		int count = 0;
		int format = 0;
		do
		{
			format = ::EnumClipboardFormats(format);
			if (format)
			{
				++count;
				
				LPCTSTR cfNames[] = {
					_T("CF_TEXT"),
					_T("CF_BITMAP"),
					_T("CF_METAFILEPICT"),
					_T("CF_SYLK"),
					_T("CF_DIF"),
					_T("CF_TIFF"),
					_T("CF_OEMTEXT"),
					_T("CF_DIB"),
					_T("CF_PALETTE"),
					_T("CF_PENDATA"),
					_T("CF_RIFF"),
					_T("CF_WAVE"),
					_T("CF_UNICODETEXT"),
					_T("CF_ENHMETAFILE"),
					_T("CF_HDROP"),
					_T("CF_LOCALE"),
					_T("CF_DIBV5")
				};

				if ((format > 0) && (format <= _countof(cfNames)))
					TRACE(_T("Clipboard offered type %s\n"), cfNames[format - 1]);
				else
				{
					TCHAR buffer[100];
					if (::GetClipboardFormatName(format, buffer, _countof(buffer)))
						TRACE(_T("Clipboard offered type %s\n"), buffer);
					else
						TRACE(_T("Clipboard offered type #%i\n"), format);
				}
			}
		}
		while (format != 0);

		if (!count)
			TRACE(_T("Clipboard is empty!\n"));

		::CloseClipboard();
	}
}
#endif

void CDib::EditCopy() 
{
	if (::OpenClipboard(NULL))
	{
		// Clear clipboard
		::EmptyClipboard();

		// Temp Dib
		CDib DibTemp(*this);

		// CF_DIBV5
		DibTemp.ToBITMAPV5HEADER();
		if (m_pBMI->bmiHeader.biBitCount == 32 && m_bAlpha && m_bFast32bpp)
		{
			// Convert to BI_BITFIELDS (that's needed by some programs like paint.net)
			if (m_pBMI->bmiHeader.biCompression == BI_RGB)
			{
				((LPBITMAPV5HEADER)DibTemp.GetBMI())->bV5Compression = BI_BITFIELDS;
				((LPBITMAPV5HEADER)DibTemp.GetBMI())->bV5RedMask = 0x00FF0000;
				((LPBITMAPV5HEADER)DibTemp.GetBMI())->bV5GreenMask = 0x0000FF00;
				((LPBITMAPV5HEADER)DibTemp.GetBMI())->bV5BlueMask = 0x000000FF;
			}

			// Save to clipboard
			::SetClipboardData(CF_DIBV5, DibTemp.CopyToHandle());

			// Flatten with background, to be used below for CF_DIB and CF_BITMAP
			DibTemp.RenderAlphaWithSrcBackground();
			DibTemp.AlphaOffset(255); // remove alpha information by making it fully opaque
			DibTemp.SetAlpha(FALSE);
			((LPBITMAPV5HEADER)DibTemp.GetBMI())->bV5Compression = BI_RGB;
			((LPBITMAPV5HEADER)DibTemp.GetBMI())->bV5RedMask = 0;
			((LPBITMAPV5HEADER)DibTemp.GetBMI())->bV5GreenMask = 0;
			((LPBITMAPV5HEADER)DibTemp.GetBMI())->bV5BlueMask = 0;
			((LPBITMAPV5HEADER)DibTemp.GetBMI())->bV5AlphaMask = 0;
		}
		else
			::SetClipboardData(CF_DIBV5, DibTemp.CopyToHandle());

		// CF_DIB
		DibTemp.ToBITMAPINFOHEADER();
		::SetClipboardData(CF_DIB, DibTemp.CopyToHandle());

		// CF_BITMAP
		HBITMAP hBitmap = DibTemp.GetDDB();
		if (hBitmap && ::SetClipboardData(CF_BITMAP, hBitmap) == NULL)
		{
			// Only when SetClipboardData() fails it's necessary to delete hBitmap.
			// That fact is discussed here:
			// https://stackoverflow.com/questions/32086618/who-releases-handle-in-setclipboarddatacf-bitmap-hbitmap
			// https://www.codeguru.com/multimedia/copying-a-bitmap-to-clipboard/
			// I made tests with Nirsoft's GDIView and found no leaks. Then I also tried to re-use hBitmap by
			// placing the following code after the below ::CloseClipboard():
			//		if (::OpenClipboard(NULL))
			//		{
			//			::EmptyClipboard();
			//			SetBitsFromDDB(hBitmap, NULL);
			//			::CloseClipboard();
			//		}
			// as expected hBitmap is not loading anymore.
			::DeleteObject(hBitmap);
		}

		// Close clipboard
		::CloseClipboard();
	}
}

void CDib::EditPaste(int XDpi/*=0*/, int YDpi/*=0*/)
{
	BOOL bOk = FALSE;
	if (::OpenClipboard(NULL))
	{
		if (::IsClipboardFormatAvailable(CF_DIBV5))
		{
			HGLOBAL hDib = ::GetClipboardData(CF_DIBV5);
			if (hDib)
				bOk = ((CopyFromHandle(hDib) != NULL) && IsValid());
		}
		if (!bOk && ::IsClipboardFormatAvailable(CF_DIB))
		{
			HGLOBAL hDib = ::GetClipboardData(CF_DIB);
			if (hDib)
				bOk = ((CopyFromHandle(hDib) != NULL) && IsValid());
		}
		if (!bOk && ::IsClipboardFormatAvailable(CF_ENHMETAFILE))
		{
			HANDLE hData = NULL;
			if (hData = ::GetClipboardData(CF_ENHMETAFILE))
			{
				// Get header
				HENHMETAFILE hMeta = (HENHMETAFILE)hData;
				ENHMETAHEADER emh;
				::GetEnhMetaFileHeader(hMeta, sizeof(emh), &emh); 

				// Calc. the bound rectangle which can be smaller than the frame rectangle
				// Note: rclFrame and rclBounds include the right and bottom edges
				double dXSrcDpi, dYSrcDpi, dXDstDpi, dYDstDpi;
				int cx, cy;
				CRect rcBound;
				if (emh.szlMillimeters.cx > 0	&&	emh.szlMillimeters.cy &&
					emh.szlDevice.cx > 0		&&	emh.szlDevice.cy > 0)
				{
					dXSrcDpi = 25.4 * (double)emh.szlDevice.cx / (double)emh.szlMillimeters.cx;
					dYSrcDpi = 25.4 * (double)emh.szlDevice.cy / (double)emh.szlMillimeters.cy;
					dXDstDpi = XDpi;
					dYDstDpi = YDpi;
					if (XDpi <= 0)
						dXSrcDpi = dXDstDpi = MIN(PASTE_MAX_DPI, dXSrcDpi);
					if (YDpi <= 0)
						dYSrcDpi = dYDstDpi = MIN(PASTE_MAX_DPI, dYSrcDpi);

					// rclFrame specifies the dimensions in .01 millimeter units
					// -> convert to wanted dpi in device units
					cx = Round((emh.rclFrame.right - emh.rclFrame.left + 1) * dXDstDpi / 2540.0);
					cy = Round((emh.rclFrame.bottom - emh.rclFrame.top + 1) * dYDstDpi / 2540.0);

					// rclBounds specifies the dimensions in device units
					// -> convert to wanted dpi in device units
					if (dXDstDpi == dXSrcDpi && dYDstDpi == dYSrcDpi)
					{
						rcBound = CRect(emh.rclBounds.left,
										emh.rclBounds.top,
										emh.rclBounds.right + 1,
										emh.rclBounds.bottom + 1);
					}
					else
					{
						rcBound = CRect(Round(emh.rclBounds.left			* dXDstDpi / dXSrcDpi),
										Round(emh.rclBounds.top				* dYDstDpi / dYSrcDpi),
										Round((emh.rclBounds.right + 1)		* dXDstDpi / dXSrcDpi),
										Round((emh.rclBounds.bottom + 1)	* dYDstDpi / dYSrcDpi));
					}
					rcBound.OffsetRect(-rcBound.left, -rcBound.top);
				}
				else
				{
					dXSrcDpi = dXDstDpi = XDpi;
					dYSrcDpi = dYDstDpi = YDpi;
					if (XDpi <= 0)
						dXSrcDpi = dXDstDpi = MIN(PASTE_MAX_DPI, DEFAULT_DPI);
					if (YDpi <= 0)
						dYSrcDpi = dYDstDpi = MIN(PASTE_MAX_DPI, DEFAULT_DPI);

					// rclFrame specifies the dimensions in .01 millimeter units
					// -> convert to wanted dpi in device units
					cx = Round((emh.rclFrame.right - emh.rclFrame.left + 1) * dXDstDpi / 2540.0);
					cy = Round((emh.rclFrame.bottom - emh.rclFrame.top + 1) * dYDstDpi / 2540.0);
					rcBound = CRect(0, 0, cx, cy);
				}

				// MemDC
				HDC hDC = ::GetDC(NULL);				// screen dc
				HDC	hMemDC = ::CreateCompatibleDC(hDC);	// memory dc compatible with screen
				HBITMAP hBitmap = ::CreateCompatibleBitmap(hDC, cx, cy);
				::ReleaseDC(NULL, hDC);					// don't needed anymore

				// Render
				if (hMemDC && hBitmap)
				{
					// Drawing rectangle
					RECT rc = {0, 0, cx, cy};

					// Select bitmap
					HBITMAP hBitmapOld = (HBITMAP)::SelectObject(hMemDC, hBitmap);

					// Paint Background
					COLORREF crOldColor = ::SetBkColor(hMemDC, m_crBackgroundColor);
					::ExtTextOut(hMemDC, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
					::SetBkColor(hMemDC, crOldColor);

					// Play the Metafile into Memory DC
					::PlayEnhMetaFile(hMemDC, hMeta, &rc);

					// Restore old bitmap
					::SelectObject(hMemDC, hBitmapOld);

					// DDB -> CDib bits
					bOk = (SetBitsFromDDB(hBitmap, NULL) && IsValid());

					// Crop
					Crop(rcBound.left, rcBound.top, rcBound.right - rcBound.left, rcBound.bottom - rcBound.top);

					// Set DPI
					if (GetBMIH())
					{
						GetBMIH()->biXPelsPerMeter = (LONG)Round(dXDstDpi * 100.0 / 2.54);
						GetBMIH()->biYPelsPerMeter = (LONG)Round(dYDstDpi * 100.0 / 2.54);
					}
				}

				// Clean-Up
				if (hBitmap)
					::DeleteObject(hBitmap);
				if (hMemDC)
					::DeleteDC(hMemDC);
			}
		}
		if (!bOk && ::IsClipboardFormatAvailable(CF_HDROP))
		{
			HDROP hDropInfo = NULL;
			if (hDropInfo = (HDROP)::GetClipboardData(CF_HDROP))
			{
				// Try to load first file in list
				CString sPath;
				UINT uiSize = ::DragQueryFile(hDropInfo, 0, NULL, 0) + 1;
				LPTSTR p = sPath.GetBuffer(uiSize);
				::DragQueryFile(hDropInfo, 0, p, uiSize);
				sPath.ReleaseBuffer();
				bOk = LoadImage(::GetShortcutTarget(sPath));
			}
		}
		::CloseClipboard();
	}
}

HGLOBAL CDib::CopyToHandle()
{
	CSharedFile file;
	try
	{
		if (SaveBMPNoFileHeader(file) == FALSE)
			return NULL;
	}
	catch (CFileException* e)
	{
		e->Delete();
		return NULL;
	}
	return file.Detach();
}

HGLOBAL CDib::CopyFromHandle(HGLOBAL handle)
{
	CSharedFile file;
	file.SetHandle(handle, FALSE);
	try
	{
		if (LoadBMPNoFileHeader(file) == FALSE)
			return NULL;
	}
	catch (CFileException* e)
	{
		e->Delete();
		return NULL;
	}
	return file.Detach();
}

CString CDib::GetCompressionName()
{
	CString sText;

	if (!m_pBMI)
		return _T("");

	switch (m_pBMI->bmiHeader.biCompression)
	{	
		case BI_RGB :
			if (m_pBMI->bmiHeader.biBitCount == 16)
				sText = _T("RGB16 (555)");
			else
			{
				if (m_bAlpha)
					sText.Format(_T("RGBA%d"), m_pBMI->bmiHeader.biBitCount);
				else
					sText.Format(_T("RGB%d"), m_pBMI->bmiHeader.biBitCount);
			}
			return sText;
		case BI_RLE8 :		
			return _T("RLE8");
		case BI_RLE4 :		
			return _T("RLE4");
		case BI_BITFIELDS :
			if (m_pBMI->bmiHeader.biBitCount == 16)
			{
				LPBITMAPINFOBITFIELDS pBmiBf = (LPBITMAPINFOBITFIELDS)m_pBMI;
				if ((pBmiBf->biBlueMask == 0x001F)	&&
					(pBmiBf->biGreenMask == 0x07E0)	&&
					(pBmiBf->biRedMask == 0xF800))
				{
#ifdef _DEBUG
					sText = _T("BITFIELDS16 (565)");
#else
					sText = _T("RGB16 (565)");
#endif
				}
				else if ((pBmiBf->biBlueMask == 0x001F)&&
						(pBmiBf->biGreenMask == 0x03E0)&&
						(pBmiBf->biRedMask == 0x7C00))
				{
#ifdef _DEBUG
					sText = _T("BITFIELDS16 (555)");
#else
					sText = _T("RGB16 (555)");
#endif
				}
				else
				{
#ifdef _DEBUG
					sText = _T("BITFIELDS16");
#else	
					sText = _T("RGB16");
#endif
				}
			}
			else
			{
				if (m_bAlpha)
				{
#ifdef _DEBUG
					sText.Format(_T("BITFIELDSA%d"), m_pBMI->bmiHeader.biBitCount);
#else
					sText.Format(_T("RGBA%d"), m_pBMI->bmiHeader.biBitCount);
#endif
				}
				else
				{
#ifdef _DEBUG
					sText.Format(_T("BITFIELDS%d"), m_pBMI->bmiHeader.biBitCount);
#else
					sText.Format(_T("RGB%d"), m_pBMI->bmiHeader.biBitCount);
#endif
				}
			}
			return sText;
		case BI_JPEG :
			return _T("JPEG");
		default : 
		{
			char ch0 = (char)(m_pBMI->bmiHeader.biCompression & 0xFF);
			char ch1 = (char)((m_pBMI->bmiHeader.biCompression >> 8) & 0xFF);
			char ch2 = (char)((m_pBMI->bmiHeader.biCompression >> 16) & 0xFF);
			char ch3 = (char)((m_pBMI->bmiHeader.biCompression >> 24) & 0xFF);
			WCHAR wch0, wch1, wch2, wch3;
			mbtowc(&wch0, &ch0, sizeof(WCHAR));
			mbtowc(&wch1, &ch1, sizeof(WCHAR));
			mbtowc(&wch2, &ch2, sizeof(WCHAR));
			mbtowc(&wch3, &ch3, sizeof(WCHAR));
			return (CString(wch0) + CString(wch1) + CString(wch2) + CString(wch3));
		}
	}
}

CString CDib::GetCompressionName(LPBITMAPINFO pBMI)
{
	CString sText;

	if (!pBMI)
		return _T("");

	switch (pBMI->bmiHeader.biCompression)
	{	
		case BI_RGB :
			if (pBMI->bmiHeader.biBitCount == 16)
				sText = _T("RGB16 (555)");
			else
				sText.Format(_T("RGB%d"), pBMI->bmiHeader.biBitCount);
			return sText;
		case BI_RLE8 :		
			return _T("RLE8");
		case BI_RLE4 :		
			return _T("RLE4");
		case BI_BITFIELDS :
			if (pBMI->bmiHeader.biBitCount == 16)
			{
				LPBITMAPINFOBITFIELDS pBmiBf = (LPBITMAPINFOBITFIELDS)pBMI;
				if ((pBmiBf->biBlueMask == 0x001F)	&&
					(pBmiBf->biGreenMask == 0x07E0)	&&
					(pBmiBf->biRedMask == 0xF800))
				{
#ifdef _DEBUG
					sText = _T("BITFIELDS16 (565)");
#else
					sText = _T("RGB16 (565)");
#endif
				}
				else if ((pBmiBf->biBlueMask == 0x001F)&&
						(pBmiBf->biGreenMask == 0x03E0)&&
						(pBmiBf->biRedMask == 0x7C00))
				{
#ifdef _DEBUG
					sText = _T("BITFIELDS16 (555)");
#else
					sText = _T("RGB16 (555)");
#endif
				}
				else
				{
#ifdef _DEBUG
					sText = _T("BITFIELDS16");
#else	
					sText = _T("RGB16");
#endif
				}
			}
			else
			{
#ifdef _DEBUG
				sText.Format(_T("BITFIELDS%d"), pBMI->bmiHeader.biBitCount);
#else
				sText.Format(_T("RGB%d"), pBMI->bmiHeader.biBitCount);
#endif	
			}
			return sText;
		case BI_JPEG :
			return _T("JPEG");
		default : 
		{
			char ch0 = (char)(pBMI->bmiHeader.biCompression & 0xFF);
			char ch1 = (char)((pBMI->bmiHeader.biCompression >> 8) & 0xFF);
			char ch2 = (char)((pBMI->bmiHeader.biCompression >> 16) & 0xFF);
			char ch3 = (char)((pBMI->bmiHeader.biCompression >> 24) & 0xFF);
			WCHAR wch0, wch1, wch2, wch3;
			mbtowc(&wch0, &ch0, sizeof(WCHAR));
			mbtowc(&wch1, &ch1, sizeof(WCHAR));
			mbtowc(&wch2, &ch2, sizeof(WCHAR));
			mbtowc(&wch3, &ch3, sizeof(WCHAR));
			return (CString(wch0) + CString(wch1) + CString(wch2) + CString(wch3));
		}
	}
}

BOOL CDib::Decompress(int nToBitsPerPixel)
{
	if (!m_pBMI)
		return FALSE;

	if (!DibSectionToBits())
		return FALSE;

	if (!m_pBits)
		return FALSE;

	// RLE?
	if ((GetBitCount() <= 8)							&&
		((m_pBMI->bmiHeader.biCompression == BI_RLE4)	||
		(m_pBMI->bmiHeader.biCompression == BI_RLE8)))
	{
		if (DecompressRLE())
		{
			if (GetBitCount() == nToBitsPerPixel)
				return TRUE;
			else
				return ConvertTo(nToBitsPerPixel);
		}
		else
			return FALSE;
	}

	// If not compressed return wished Bpp
	if (m_pBMI->bmiHeader.biCompression == BI_RGB ||
		m_pBMI->bmiHeader.biCompression == BI_BITFIELDS)
	{
		if (GetBitCount() == nToBitsPerPixel)
			return TRUE;
		else
			return ConvertTo(nToBitsPerPixel);
	}

	// YUV Decode?
	if (::IsSupportedYuvToRgbFormat(m_pBMI->bmiHeader.biCompression))	
	{
		// New Image Size
		m_dwImageSize = DWALIGNEDWIDTHBYTES(32 * GetWidth()) * GetHeight();

		// Allocate
		LPBITMAPINFO pBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER)];
		if (!pBMI)
			return FALSE;
		LPBYTE pBits = (LPBYTE)BIGALLOC(m_dwImageSize);
		if (!pBits)
		{
			delete [] pBMI;
			return FALSE;
		}

		// BMIH
		pBMI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		pBMI->bmiHeader.biWidth = GetWidth();
		pBMI->bmiHeader.biHeight = GetHeight();
		pBMI->bmiHeader.biPlanes = 1;
		pBMI->bmiHeader.biBitCount = 32;
		pBMI->bmiHeader.biCompression = BI_RGB;
		pBMI->bmiHeader.biSizeImage = m_dwImageSize;
		pBMI->bmiHeader.biXPelsPerMeter = 0;
		pBMI->bmiHeader.biYPelsPerMeter = 0;
		pBMI->bmiHeader.biClrUsed = 0;
		pBMI->bmiHeader.biClrImportant = 0;
		m_pColors = NULL;

		// Decompress
		::YUVToRGB32(	m_pBMI->bmiHeader.biCompression,
						m_pBits,	// YUV format depending from Compression Type
						pBits,		// RGB32 Dib
						GetWidth(),
						GetHeight());

		// Free
		if (m_pBMI)
		{
			delete [] m_pBMI;
			m_pBMI = NULL;
		}
		if (m_pBits)
		{
			BIGFREE(m_pBits);
			m_pBits = NULL;
		}
		ResetColorUndo();
		if (m_pPalette)
		{
			m_pPalette->DeleteObject();
			delete m_pPalette;
			m_pPalette = NULL;
		}
		if (m_hDibSection)
		{
			::DeleteObject(m_hDibSection);
			m_hDibSection = NULL;
			m_pDibSectionBits = NULL;
		}

		// Set Pointer
		m_pBMI = pBMI;
		m_pBits = pBits;

		// Init Palette
		CreatePaletteFromBMI();

		// Init Masks
		InitMasks();

		// Convert to the wished Bpp
		if (GetBitCount() == nToBitsPerPixel)
			return TRUE;
		else
			return ConvertTo(nToBitsPerPixel);
	}
	// Try to Decode with DrawDib Functions
	else
	{
		// Get Display dc
		HDC hDC = ::GetDC(NULL);
			
		// Create mem dc 
		HDC hTmpDC = ::CreateCompatibleDC(hDC);

		// Create a new (uncompressed) dibsection and select it in to the memory dc
		LPBITMAPINFO pBMI = (LPBITMAPINFO)new BYTE[GetBMISize()];
		if (pBMI == NULL)
		{
			::DeleteDC(hTmpDC);
			::ReleaseDC(NULL, hDC);
			return FALSE;
		}
		memcpy(pBMI, m_pBMI, GetBMISize());
		pBMI->bmiHeader.biBitCount = nToBitsPerPixel;
		pBMI->bmiHeader.biCompression = BI_RGB;
		DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES(pBMI->bmiHeader.biWidth * pBMI->bmiHeader.biBitCount);
		pBMI->bmiHeader.biSizeImage = uiDIBScanLineSize * pBMI->bmiHeader.biHeight;
		LPBYTE pBits = NULL;
		HBITMAP hTmpDib = ::CreateDIBSection(	hTmpDC,
												(const BITMAPINFO*)pBMI,
												DIB_RGB_COLORS,
												(void**)&pBits,
												NULL, 0);
		if (!hTmpDib)
		{
			ShowLastError(_T("CreateDIBSection()"));
			delete [] pBMI;
			::DeleteDC(hTmpDC);
			::ReleaseDC(NULL, hDC);
			return FALSE;
		}

		// Select Bitmap into mem dc
		HGDIOBJ hOldBitmap = ::SelectObject(hTmpDC, hTmpDib);

		// Draw to mem dc (=draw to selected bitmap)
		HDRAWDIB hDrawDib = ::DrawDibOpen();
		BOOL res = ::DrawDibDraw(	hDrawDib,         
									hTmpDC,               
									0,
									0,
									m_pBMI->bmiHeader.biWidth,
									m_pBMI->bmiHeader.biHeight,       
									(LPBITMAPINFOHEADER)m_pBMI,
									m_pBits,
									0,		
									0,
									m_pBMI->bmiHeader.biWidth,
									m_pBMI->bmiHeader.biHeight,               
									0);
		::DrawDibClose(hDrawDib);

		// Clean-up
		::SelectObject(hTmpDC, hOldBitmap);
		::DeleteDC(hTmpDC);
		::ReleaseDC(NULL, hDC);
		
		// Check Result
		if (!res)
		{
			::DeleteObject(hTmpDib);
			delete [] pBMI;
			return FALSE;
		}
		
		// Init New Dibsection
		Free();
		m_pBMI = pBMI;
		if (m_pBMI->bmiHeader.biBitCount <= 8)
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + (WORD)(m_pBMI->bmiHeader.biSize));
		else
			m_pColors = NULL;
		CreatePaletteFromBMI();
		m_hDibSection = hTmpDib;
		m_pDibSectionBits = pBits;
		m_dwImageSize = pBMI->bmiHeader.biSizeImage;
		InitMasks();

		// DibSection To Bits and return wished Bpp
		if (DibSectionToBits(TRUE, TRUE))
		{
			// Convert to the wished Bpp
			if (GetBitCount() == nToBitsPerPixel)
				return TRUE;
			else
				return ConvertTo(nToBitsPerPixel);
		}
		else
			return FALSE;
	}
}

BOOL CDib::Compress(DWORD dwFourCC, int stride/*=0*/)
{
	// Already RGB?
	if (dwFourCC == BI_RGB || dwFourCC == GetCompression())
		return TRUE;
	// Compress to RLE?
	else if (	(dwFourCC == BI_RLE4) ||
				(dwFourCC == BI_RLE8))
		return CompressRLE(dwFourCC);
	else if (!::IsSupportedRgbToYuvFormat(dwFourCC))
		return FALSE;

	if (!m_pBMI)
		return FALSE;

	if (!DibSectionToBits())
		return FALSE;

	if (!m_pBits)
		return FALSE;

	// To RGB
	if (IsCompressed())
	{
  		if (!Decompress(32))
			return FALSE;
	}
	else if (GetBitCount() != 32 && GetBitCount() != 24)
	{
		if (!ConvertTo32bits())
			return FALSE;
	}

	// FourCC to Bpp
	int nBpp = ::FourCCToBpp(dwFourCC);
	
	// Set Stride if not set
	if (stride <= 0)
		stride = ::CalcYUVStride(dwFourCC, GetWidth());

	// Allocate BMI
	LPBITMAPINFO pBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER)];
	if (!pBMI)
		return FALSE;
	pBMI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pBMI->bmiHeader.biWidth = GetWidth();
	pBMI->bmiHeader.biHeight = GetHeight();
	pBMI->bmiHeader.biPlanes = 1;
	pBMI->bmiHeader.biBitCount = nBpp;
	pBMI->bmiHeader.biCompression = dwFourCC;
	pBMI->bmiHeader.biSizeImage = ::CalcYUVSize(dwFourCC, stride, GetHeight());
	pBMI->bmiHeader.biXPelsPerMeter = 0;
	pBMI->bmiHeader.biYPelsPerMeter = 0;
	pBMI->bmiHeader.biClrUsed = 0;
	pBMI->bmiHeader.biClrImportant = 0;

	// Allocate Bits
	LPBYTE pBits = (LPBYTE)BIGALLOC(pBMI->bmiHeader.biSizeImage);
	if (!pBits)
	{
		delete [] pBMI;
		return FALSE;
	}

	// Compress
	if (GetBitCount() == 32)
	{
		if (!::RGB32ToYUV(	dwFourCC,
							m_pBits,	// RGB32 Dib
							pBits,		// YUV format depending from dwFourCC
							GetWidth(),
							GetHeight(),
							stride))
		{
			delete [] pBMI;
			BIGFREE(pBits);
			return FALSE;
		}
	}
	else
	{
		if (!::RGB24ToYUV(	dwFourCC,
							m_pBits,	// RGB24 Dib
							pBits,		// YUV format depending from dwFourCC
							GetWidth(),
							GetHeight(),
							stride))
		{
			delete [] pBMI;
			BIGFREE(pBits);
			return FALSE;
		}
	}

	// Image Size
	m_dwImageSize = pBMI->bmiHeader.biSizeImage;

	// Free
	if (m_pBMI)
	{
		delete [] m_pBMI;
		m_pBMI = NULL;
	}
	if (m_pBits)
	{
		BIGFREE(m_pBits);
		m_pBits = NULL;
	}
	ResetColorUndo();
	if (m_pPalette)
	{
		m_pPalette->DeleteObject();
		delete m_pPalette;
		m_pPalette = NULL;
	}
	if (m_hDibSection)
	{
		::DeleteObject(m_hDibSection);
		m_hDibSection = NULL;
		m_pDibSectionBits = NULL;
	}

	// Set Pointer
	m_pBMI = pBMI;
	m_pBits = pBits;

	return TRUE;
}

__forceinline int CDib::MakeRLE8Scanline(	BYTE* UnencodedBuffer,	// Pointer to buffer holding unencoded scan line
											BYTE* EncodedBuffer,		// Pointer to buffer to hold encoded scan line
											int nWidth)				// The length of a scan line in pixels
{
	BOOL bDoExit;
    int	nUnencRunCount;	// The number of pixels in the current unencoded run
    int	nEncRunCount;	// The number of pixels in the current encoded run
    int	nUnencPos;		// The index of UnencodedBuffer
	int	nEncPos;		// The index of EncodedBuffer

	// Init Vars
    nUnencPos = 0;
	nEncPos = 0;

    for (;;)
    {
		// Check For Encoded Run
        nEncRunCount = 1;
		while (((nUnencPos + nEncRunCount) < nWidth) &&
				UnencodedBuffer[nUnencPos] == UnencodedBuffer[nUnencPos + nEncRunCount])
		{
			nEncRunCount++;
		}
		bDoExit = (nUnencPos + nEncRunCount) >= nWidth;
		
		// Encoded Run(s)
		if (bDoExit || (nEncRunCount > 1)) 
		{
            while (nEncRunCount >= 256)
			{
				EncodedBuffer[nEncPos++] = 255;							// Number of pixels in run
				EncodedBuffer[nEncPos++] = UnencodedBuffer[nUnencPos];	// The Pixel
				nEncRunCount -= 255;
				nUnencPos += 255;
			}
			if (nEncRunCount > 0)
			{
				EncodedBuffer[nEncPos++] = nEncRunCount;				// Number of pixels in run
				EncodedBuffer[nEncPos++] = UnencodedBuffer[nUnencPos];	// The Pixel
				nUnencPos += nEncRunCount;
			}

			if (bDoExit)
			{
				EncodedBuffer[nEncPos++] = 0;          
				EncodedBuffer[nEncPos++] = 0;
				return nEncPos;
			}
		}
		else
		{
			// Check For Unencoded Run
			nUnencRunCount = 2;
			BOOL bCurrentSame;
			BOOL bLastSame = FALSE;
			BOOL bLastLastSame = FALSE;
			BOOL bLastLastLastSame = FALSE;
			while (!(bDoExit = ((nUnencPos + nUnencRunCount) >= nWidth)))
			{
				bCurrentSame = UnencodedBuffer[nUnencPos + nUnencRunCount - 1] ==
											UnencodedBuffer[nUnencPos + nUnencRunCount];
				if (bCurrentSame		&&
					bLastSame			&&
					bLastLastSame		&&
					bLastLastLastSame)
				{
					nUnencRunCount -= 4;
					break;
				}
				bLastLastLastSame = bLastLastSame;
				bLastLastSame = bLastSame;
				bLastSame = bCurrentSame;
				nUnencRunCount++;
			}

			// Unencoded Run(s)
			if (nUnencRunCount >= 3)
			{
				while (nUnencRunCount >= 256)
				{
					int nCountDown = 255;
					EncodedBuffer[nEncPos++] = 0;	// Unencoded run indicator
					EncodedBuffer[nEncPos++] = 255;	// 255 pixels in run
					while (nCountDown--)
						EncodedBuffer[nEncPos++] = UnencodedBuffer[nUnencPos++]; // The Pixel
					EncodedBuffer[nEncPos++] = 0;	// Padding
					nUnencRunCount -= 255;
				}
				if (nUnencRunCount >= 3)
				{
					BOOL bPad = ((nUnencRunCount & 0x1) == 0x1);
					EncodedBuffer[nEncPos++] = 0;				// Unencoded run indicator
					EncodedBuffer[nEncPos++] = nUnencRunCount;	// Number of pixels in run
					while (nUnencRunCount--)
						EncodedBuffer[nEncPos++] = UnencodedBuffer[nUnencPos++]; // The Pixel
					if (bPad)
						EncodedBuffer[nEncPos++] = 0;
				}
				else
				{
					while (nUnencRunCount--)
					{
						EncodedBuffer[nEncPos++] = 1;							// One pixels in run
						EncodedBuffer[nEncPos++] = UnencodedBuffer[nUnencPos++];// The Pixel
					}
				}
			}
			else
			{
				while (nUnencRunCount--)
				{
					EncodedBuffer[nEncPos++] = 1;							// One pixels in run
					EncodedBuffer[nEncPos++] = UnencodedBuffer[nUnencPos++];// The Pixel
				}
			}

			if (bDoExit)
			{
				EncodedBuffer[nEncPos++] = 0;          
				EncodedBuffer[nEncPos++] = 0;
				return nEncPos;
			}
		}
    }

    return -1;
}

__forceinline int CDib::MakeRLE4Scanline(	BYTE* UnencodedBuffer,	// Pointer to buffer holding unencoded scan line
											BYTE* EncodedBuffer,		// Pointer to buffer to hold encoded scan line
											int nWidth)				// The length of a scan line in pixels
{
	BOOL bDoExit;
	BOOL bOddWidth;
    int	nUnencRunCount;	// The number of pixels in the current unencoded run
    int	nEncRunCount;	// The number of pixels in the current encoded run
    int	nUnencPos;		// The index of UnencodedBuffer
	int	nEncPos;		// The index of EncodedBuffer
	int	nScanLineBytes;	// Rounded-up bytes for the scan line
	int nLastPixelCountPos;	// Used to adjust the pixel count for odd sized scan lines

	// Init Vars
    nUnencPos = 0;
	nEncPos = 0;
	bOddWidth = ((nWidth & 0x1) == 0x1);
	nScanLineBytes = nWidth / 2;
	if (bOddWidth)
		nScanLineBytes++;

    for (;;)
    {
		// Check For Encoded Run
        nEncRunCount = 1;
		while (((nUnencPos + nEncRunCount) < nScanLineBytes) &&
				UnencodedBuffer[nUnencPos] == UnencodedBuffer[nUnencPos + nEncRunCount])
		{
			nEncRunCount++;
		}
		bDoExit = (nUnencPos + nEncRunCount) >= nScanLineBytes;
		
		// Encoded Run(s)
		if (bDoExit || (nEncRunCount > 1)) 
		{
            while (nEncRunCount >= 128)
			{
				nLastPixelCountPos = nEncPos;
				EncodedBuffer[nEncPos++] = 254;	// Number of pixels in run
				EncodedBuffer[nEncPos++] = UnencodedBuffer[nUnencPos];	// The Pixel
				nEncRunCount -= 127;
				nUnencPos += 127;
			}
			if (nEncRunCount > 0)
			{
				nLastPixelCountPos = nEncPos;
				EncodedBuffer[nEncPos++] = 2 * nEncRunCount;			// Number of pixels in run
				EncodedBuffer[nEncPos++] = UnencodedBuffer[nUnencPos];	// The Pixel
				nUnencPos += nEncRunCount;
			}

			if (bDoExit)
			{
				// Dec. pixel count by one for odd sized scan lines
				if (bOddWidth)
					EncodedBuffer[nLastPixelCountPos]--;

				EncodedBuffer[nEncPos++] = 0;          
				EncodedBuffer[nEncPos++] = 0;
				return nEncPos;
			}
		}
		else
		{
			// Check For Unencoded Run
			nUnencRunCount = 2;
			BOOL bCurrentSame;
			BOOL bLastSame = FALSE;
			BOOL bLastLastSame = FALSE;
			BOOL bLastLastLastSame = FALSE;
			while (!(bDoExit = ((nUnencPos + nUnencRunCount) >= nScanLineBytes)))
			{
				bCurrentSame = UnencodedBuffer[nUnencPos + nUnencRunCount - 1] ==
											UnencodedBuffer[nUnencPos + nUnencRunCount];
				if (bCurrentSame		&&
					bLastSame			&&
					bLastLastSame		&&
					bLastLastLastSame)
				{
					nUnencRunCount -= 4;
					break;
				}
				bLastLastLastSame = bLastLastSame;
				bLastLastSame = bLastSame;
				bLastSame = bCurrentSame;
				nUnencRunCount++;
			}

			// Unencoded Run(s)
			if (nUnencRunCount >= 3)
			{
				while (nUnencRunCount >= 128)
				{
					int nCountDown = 127;
					EncodedBuffer[nEncPos++] = 0;	// Unencoded run indicator
					nLastPixelCountPos = nEncPos;
					EncodedBuffer[nEncPos++] = 254;	// Number of pixels in run
					while (nCountDown--)
						EncodedBuffer[nEncPos++] = UnencodedBuffer[nUnencPos++]; // The Pixels
					EncodedBuffer[nEncPos++] = 0;	// Padding
					nUnencRunCount -= 127;
				}
				if (nUnencRunCount >= 3)
				{
					BOOL bPad = ((nUnencRunCount & 0x1) == 0x1);
					EncodedBuffer[nEncPos++] = 0;					// Unencoded run indicator
					nLastPixelCountPos = nEncPos;
					EncodedBuffer[nEncPos++] = 2 * nUnencRunCount;	// Number of pixels in run
					while (nUnencRunCount--)
						EncodedBuffer[nEncPos++] = UnencodedBuffer[nUnencPos++]; // The Pixel
					if (bPad)
						EncodedBuffer[nEncPos++] = 0;
				}
				else
				{
					while (nUnencRunCount--)
					{
						nLastPixelCountPos = nEncPos;
						EncodedBuffer[nEncPos++] = 2;							// Two pixels in run
						EncodedBuffer[nEncPos++] = UnencodedBuffer[nUnencPos++];// The Pixel
					}
				}
			}
			else
			{
				while (nUnencRunCount--)
				{
					nLastPixelCountPos = nEncPos;
					EncodedBuffer[nEncPos++] = 2;							// Two pixels in run
					EncodedBuffer[nEncPos++] = UnencodedBuffer[nUnencPos++];// The Pixel
				}
			}

			if (bDoExit)
			{
				// Dec. pixel count by one for odd sized scan lines
				if (bOddWidth)
					EncodedBuffer[nLastPixelCountPos]--;

				EncodedBuffer[nEncPos++] = 0;          
				EncodedBuffer[nEncPos++] = 0;
				return nEncPos;
			}
		}
    }

    return -1;
}

/*
This function encodes raw BMP data into 4-bit or 8-bit BMP RLE data.
Delta escape sequences are not included in the encoding.

Pixels are either 4-bits or 8-bits in size. The nCompression parameter
indicates the size with a value of BI_RLE4 or BI_RLE8.

For 4-bit pixels the MSN (Most Significant Nibble) is the first pixel
value and the LSN (Least Significant Nibble) is the second pixel value.
This particular algorithm encodes 4-bit per pixel data two nibbles at a time.
In other words, if you had the raw run "11 11 15" only first four nibbles
would be encoded in the run.  The fifth nibble would be treated part of
the next run. Not the most efficient scheme, but it simplifies the
algorithm by not needing to tear apart bytes into separate nibble values.
*/
BOOL CDib::CompressRLE(int nCompression)
{
	// Check
	if (!m_pBMI)
		return FALSE;

	if (!DibSectionToBits())
		return FALSE;

	if (IsCompressed())
	{
		if (!Decompress((nCompression == BI_RLE4) ? 4 : 8)) // Decompress
				return FALSE;
	}

	if (!m_pBits)
		return FALSE;

	if ((nCompression != BI_RLE4) && (nCompression != BI_RLE8))
		return FALSE;

	// Make sure we have the right Bpp
	if ((nCompression == BI_RLE4) && (GetBitCount() != 4))
		ConvertTo(4);
	else if ((nCompression == BI_RLE8) && (GetBitCount() != 8))
		ConvertTo(8);

	// Source Scan Line Size
	DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * GetBitCount());

	// Allocate BMI
	LPBITMAPINFO lpBMI;
	if (nCompression == BI_RLE8)
	{
		lpBMI = (LPBITMAPINFO)new BYTE[m_pBMI->bmiHeader.biSize + sizeof(RGBQUAD) * 256];
		if (!lpBMI)
			return FALSE;
		memset(lpBMI, 0, m_pBMI->bmiHeader.biSize + sizeof(RGBQUAD) * 256);
		DWORD dwCopySize = MIN(m_pBMI->bmiHeader.biSize + sizeof(RGBQUAD) * 256, GetBMISize());
		memcpy((void*)lpBMI, (void*)m_pBMI, dwCopySize);
		lpBMI->bmiHeader.biCompression = BI_RLE8;
		lpBMI->bmiHeader.biBitCount = 8;
	}
	else // BI_RLE4
	{
		lpBMI = (LPBITMAPINFO)new BYTE[m_pBMI->bmiHeader.biSize + sizeof(RGBQUAD) * 16];
		if (!lpBMI)
			return FALSE;
		memset(lpBMI, 0, m_pBMI->bmiHeader.biSize + sizeof(RGBQUAD) * 16);
		DWORD dwCopySize = MIN(m_pBMI->bmiHeader.biSize + sizeof(RGBQUAD) * 16, GetBMISize());
		memcpy((void*)lpBMI, (void*)m_pBMI, dwCopySize);
		lpBMI->bmiHeader.biCompression = BI_RLE4;
		lpBMI->bmiHeader.biBitCount = 4;
	}
	
	// Allocate Bits
	LPBYTE lpBits = (LPBYTE)BIGALLOC(2 * GetImageSize()); // Be Safe!
	if (!lpBits)
	{
		delete [] lpBMI;
		return FALSE;
	}

	// Encode
	DWORD dwImageSize = 0;
	BYTE* pSrcBuf = GetBits();
	BYTE* pDstBuf = lpBits;
	if (nCompression == BI_RLE8)
	{
		for (int line = 0 ; line < (int)GetHeight() ; line++)
		{
			int res = MakeRLE8Scanline(	pSrcBuf,			// Pointer to buffer holding unencoded scan line
										pDstBuf,			// Pointer to buffer to hold encoded scan line
										GetWidth());		// The length of a scan line in pixels					
			if (res < 0)
			{
				delete [] lpBMI;
				BIGFREE(lpBits);
				return FALSE;
			}
			else
			{
				pDstBuf += res;
				dwImageSize += res;
				pSrcBuf += uiDIBScanLineSize;
			}
		}
	}
	else
	{
		for (int line = 0 ; line < (int)GetHeight() ; line++)
		{
			int res = MakeRLE4Scanline(	pSrcBuf,			// Pointer to buffer holding unencoded scan line
										pDstBuf,			// Pointer to buffer to hold encoded scan line
										GetWidth());		// The length of a scan line in pixels
			if (res < 0)
			{
				delete [] lpBMI;
				BIGFREE(lpBits);
				return FALSE;
			}
			else
			{
				pDstBuf += res;
				dwImageSize += res;
				pSrcBuf += uiDIBScanLineSize;
			}
		}

	}

	// Write the End of Bitmap Escape Code over the
	// last End of Scanline Escape Code
	pDstBuf -= 2;
	pDstBuf[0] = 0;          
	pDstBuf[1] = 1;
	
	// Free
	Free();

	// Set Image Size
	m_dwImageSize = dwImageSize;
	lpBMI->bmiHeader.biSizeImage = dwImageSize;

	// Set Pointers
	m_pBMI = lpBMI;
	m_pBits = lpBits;
	m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + (WORD)(m_pBMI->bmiHeader.biSize));

	// Create Palette
	CreatePaletteFromBMI();

	return TRUE;
}

const int RLE_COMMAND     = 0;
const int RLE_ENDOFLINE   = 0;
const int RLE_ENDOFBITMAP = 1;
const int RLE_DELTA       = 2;

BOOL CDib::DecompressRLE8(CDib* pBackgroundDib/*=NULL*/)
{
	if (!m_pBMI)
		return FALSE;

	if (!m_pBits)
		return FALSE;

	// Only Supported for BI_RLE8
	if (m_pBMI->bmiHeader.biCompression != BI_RLE8	&&
		m_pBMI->bmiHeader.biCompression != FCC('RLE8'))
		return FALSE;

	// Allocate and prepare new BMI
	LPBITMAPINFO lpBMI = (LPBITMAPINFO)new BYTE[GetBMISize()];
	if (!lpBMI)
		return FALSE;
	memcpy((void*)lpBMI, (void*)m_pBMI, GetBMISize());
	lpBMI->bmiHeader.biCompression = BI_RGB;
	lpBMI->bmiHeader.biBitCount = 8;
	DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES(lpBMI->bmiHeader.biWidth * lpBMI->bmiHeader.biBitCount);
	lpBMI->bmiHeader.biSizeImage = uiDIBScanLineSize * ABS(lpBMI->bmiHeader.biHeight);

	// Allocate Bits
	LPBYTE lpUnencBitsStart = (LPBYTE)BIGALLOC(lpBMI->bmiHeader.biSizeImage);
	if (!lpUnencBitsStart)
	{
		delete [] lpBMI;
		return FALSE;
	}
	LPBYTE lpUnencBitsEnd = lpUnencBitsStart + lpBMI->bmiHeader.biSizeImage;
	if (pBackgroundDib && pBackgroundDib->IsValid())
	{
		memcpy(	lpUnencBitsStart,
				pBackgroundDib->GetBits(),
				MIN(pBackgroundDib->GetImageSize(), lpBMI->bmiHeader.biSizeImage));
	}
	else
		memset(lpUnencBitsStart, 0, lpBMI->bmiHeader.biSizeImage); // In case of Delta Commands background is index 0

	// Decode
	BYTE StatusByte = 0;
	BYTE SecondByte = 0;
	LPBYTE lpUnencBits = lpUnencBitsStart;
	int nEncPos = 0;
	int nUnencPos = 0;
	BOOL bContinue = TRUE;
	while (bContinue)
	{
		if (nEncPos < (int)m_pBMI->bmiHeader.biSizeImage)
			StatusByte = m_pBits[nEncPos++];
		else
			break;
		switch (StatusByte)
		{
			case RLE_COMMAND :
				if (nEncPos < (int)m_pBMI->bmiHeader.biSizeImage)				// Check Input
				{
					StatusByte = m_pBits[nEncPos++];
					switch (StatusByte)
					{
						case RLE_ENDOFLINE :
						{
							nUnencPos = 0;
							lpUnencBits += uiDIBScanLineSize;
							if (lpUnencBits >= lpUnencBitsEnd)					// Check Output
								bContinue = FALSE;
							break;
						}

						case RLE_ENDOFBITMAP :
						{
							bContinue = FALSE;
							break;
						}

						case RLE_DELTA :
						{
							// Read the delta values
							int nDeltaX = 0;
							int nDeltaY = 0;
							if (nEncPos < (int)m_pBMI->bmiHeader.biSizeImage)	// Check Input
								nDeltaX = m_pBits[nEncPos++];
							else
								bContinue = FALSE;
							if (nEncPos < (int)m_pBMI->bmiHeader.biSizeImage)	// Check Input
								nDeltaY = m_pBits[nEncPos++];
							else
								bContinue = FALSE;
							
							// Apply X
							nUnencPos += nDeltaX;
							if (nUnencPos >= lpBMI->bmiHeader.biWidth)			// Check Output
								break;

							// Apply Y
							lpUnencBits += nDeltaY * uiDIBScanLineSize;
							if (lpUnencBits >= lpUnencBitsEnd)					// Check Output
								bContinue = FALSE;

							break;
						}

						// Unencoded Run
						default :
						{
							if (nEncPos + StatusByte <= (int)m_pBMI->bmiHeader.biSizeImage)	// Check Input
							{
								if (nUnencPos + StatusByte <= lpBMI->bmiHeader.biWidth)		// Check Output
								{
									memcpy(&lpUnencBits[nUnencPos], &m_pBits[nEncPos], StatusByte);
									nEncPos += StatusByte;
									nUnencPos += StatusByte;
									
									// Align run length to even number of bytes 
									if ((StatusByte & 1) == 1)
										nEncPos++;
								}
								else
									break;
							}
							else
								bContinue = FALSE;
							
							break;
						}
					}
				}
				else
					bContinue = FALSE;

				break;

			// Encoded Run
			default :
				if (nEncPos < (int)m_pBMI->bmiHeader.biSizeImage)			// Check Input
				{
					SecondByte = m_pBits[nEncPos++];
					if (nUnencPos + StatusByte <= lpBMI->bmiHeader.biWidth)	// Check Output
					{
						memset(&lpUnencBits[nUnencPos], SecondByte, StatusByte);
						nUnencPos += StatusByte;
					}
					else
						break;
				}
				else
					bContinue = FALSE;

				break;
		}
	}

	// Clean-Up
	Free();

	// Set Pointers
	m_pBMI = lpBMI;
	m_pBits = lpUnencBitsStart;
	m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + (WORD)(m_pBMI->bmiHeader.biSize));

	// Set Size
	m_dwImageSize = lpBMI->bmiHeader.biSizeImage;

	// Create Palette
	CreatePaletteFromBMI();

	return TRUE;
}

BOOL CDib::DecompressRLE4(CDib* pBackgroundDib/*=NULL*/)
{
	if (!m_pBMI)
		return FALSE;

	if (!m_pBits)
		return FALSE;

	// Only Supported for BI_RLE4
	if (m_pBMI->bmiHeader.biCompression != BI_RLE4 &&
		m_pBMI->bmiHeader.biCompression != FCC('RLE4'))
		return FALSE;

	// Allocate and prepare new BMI
	LPBITMAPINFO lpBMI = (LPBITMAPINFO)new BYTE[GetBMISize()];
	if (!lpBMI)
		return FALSE;
	memcpy((void*)lpBMI, (void*)m_pBMI, GetBMISize());
	lpBMI->bmiHeader.biCompression = BI_RGB;
	lpBMI->bmiHeader.biBitCount = 4;
	DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES(lpBMI->bmiHeader.biWidth * lpBMI->bmiHeader.biBitCount);
	lpBMI->bmiHeader.biSizeImage = uiDIBScanLineSize * ABS(lpBMI->bmiHeader.biHeight);

	// Allocate Bits
	LPBYTE lpUnencBitsStart = (LPBYTE)BIGALLOC(lpBMI->bmiHeader.biSizeImage);
	if (!lpUnencBitsStart)
	{
		delete [] lpBMI;
		return FALSE;
	}
	LPBYTE lpUnencBitsEnd = lpUnencBitsStart + lpBMI->bmiHeader.biSizeImage;
	if (pBackgroundDib && pBackgroundDib->IsValid())
	{
		memcpy(	lpUnencBitsStart,
				pBackgroundDib->GetBits(),
				MIN(pBackgroundDib->GetImageSize(), lpBMI->bmiHeader.biSizeImage));
	}
	else
		memset(lpUnencBitsStart, 0, lpBMI->bmiHeader.biSizeImage); // In case of Delta Commands background is index 0

	// Decode
	BYTE StatusByte = 0;
	BYTE SecondByte = 0;
	LPBYTE lpUnencBits = lpUnencBitsStart;
	int nEncPos = 0;
	int nUnencPos = 0;
	int nUnencScanLineBytes = GetWidth() / 2;
	if (((GetWidth() & 0x1) == 0x1)) // If odd width
		nUnencScanLineBytes++;
	BOOL bEncLowNibble = FALSE;
	BOOL bUnencLowNibble = FALSE;
	BOOL bContinue = TRUE;
	while (bContinue)
	{
		if (nEncPos < (int)m_pBMI->bmiHeader.biSizeImage)
			StatusByte = m_pBits[nEncPos++];
		else
			break;
		switch (StatusByte)
		{
			case RLE_COMMAND :
				if (nEncPos < (int)m_pBMI->bmiHeader.biSizeImage)				// Check Input
				{
					StatusByte = m_pBits[nEncPos++];
					switch (StatusByte)
					{
						case RLE_ENDOFLINE :
						{
							nUnencPos = 0;
							bEncLowNibble = FALSE;
							bUnencLowNibble = FALSE;
							lpUnencBits += uiDIBScanLineSize;
							if (lpUnencBits >= lpUnencBitsEnd)					// Check Output
								bContinue = FALSE;
							break;
						}

						case RLE_ENDOFBITMAP :
						{
							bContinue = FALSE;
							break;
						}

						case RLE_DELTA :
						{
							// Read the delta values
							int nDeltaX = 0;
							int nDeltaY = 0;
							if (nEncPos < (int)m_pBMI->bmiHeader.biSizeImage)	// Check Input
								nDeltaX = m_pBits[nEncPos++];
							else
								bContinue = FALSE;
							if (nEncPos < (int)m_pBMI->bmiHeader.biSizeImage)	// Check Input
								nDeltaY = m_pBits[nEncPos++];
							else
								bContinue = FALSE;
							
							// Apply X
							nUnencPos += nDeltaX / 2;
							if ((nDeltaX & 0x1) == 0x1) // If Odd DeltaX
							{	
								if (bUnencLowNibble)
									nUnencPos++;
								bUnencLowNibble = !bUnencLowNibble;
							}
							if (nUnencPos >= nUnencScanLineBytes)				// Check Output
								break;

							// Apply Y
							lpUnencBits += nDeltaY * uiDIBScanLineSize;
							if (lpUnencBits >= lpUnencBitsEnd)					// Check Output
								bContinue = FALSE;
							
							break;
						}

						// Unencoded Run
						default :
						{
							if (nEncPos < (int)m_pBMI->bmiHeader.biSizeImage)	// Check Input
							{
								bEncLowNibble = FALSE;
								SecondByte = m_pBits[nEncPos++];
								for (int i = 0 ; i < StatusByte ; i++)
								{
									if (nUnencPos >= nUnencScanLineBytes)	// Check Output
										break;
									if (bEncLowNibble)
									{
										if (bUnencLowNibble)
											lpUnencBits[nUnencPos++] |= (SecondByte & 0x0F);
										else
											lpUnencBits[nUnencPos] = (SecondByte << 4);
										if (nEncPos < (int)m_pBMI->bmiHeader.biSizeImage)// Check Input
										{
											if (i != StatusByte - 1)
												SecondByte = m_pBits[nEncPos++];
										}
										else
										{
											bContinue = FALSE;
											break;
										}
									}
									else
									{
										if (bUnencLowNibble)
											lpUnencBits[nUnencPos++] |= (SecondByte >> 4);
										else
											lpUnencBits[nUnencPos] = (SecondByte & 0xF0);
									}
									bEncLowNibble = !bEncLowNibble;
									bUnencLowNibble = !bUnencLowNibble;
								}

								// Align run length to even number of bytes
								if ((((StatusByte + 1) >> 1) & 1) == 1)
									nEncPos++;
							}
							else
								bContinue = FALSE;
							
							break;
						}
					}
				}
				else
					bContinue = FALSE;

				break;

			// Encoded Run
			default :
				if (nEncPos < (int)m_pBMI->bmiHeader.biSizeImage)				// Check Input
				{
					bEncLowNibble = FALSE;
					SecondByte = m_pBits[nEncPos++];
					for (int i = 0 ; i < StatusByte ; i++)
					{
						if (nUnencPos >= nUnencScanLineBytes)					// Check Output
							break;
						if (bEncLowNibble)
						{
							if (bUnencLowNibble)
								lpUnencBits[nUnencPos++] |= (SecondByte & 0x0F);
							else
								lpUnencBits[nUnencPos] = (SecondByte << 4);
						}
						else
						{
							if (bUnencLowNibble)
								lpUnencBits[nUnencPos++] |= (SecondByte >> 4);
							else
								lpUnencBits[nUnencPos] = (SecondByte & 0xF0);	
						}
						bEncLowNibble = !bEncLowNibble;
						bUnencLowNibble = !bUnencLowNibble;
					}
				}
				else
					bContinue = FALSE;

				break;
		}
	}

	// Clean-Up
	Free();

	// Set Pointers
	m_pBMI = lpBMI;
	m_pBits = lpUnencBitsStart;
	m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + (WORD)(m_pBMI->bmiHeader.biSize));

	// Set Size
	m_dwImageSize = lpBMI->bmiHeader.biSizeImage;

	// Create Palette
	CreatePaletteFromBMI();

	return TRUE;
}

BOOL CDib::DecompressRLE(CDib* pBackgroundDib/*=NULL*/)
{
	if (!m_pBMI)
		return FALSE;

	if (m_pBMI->bmiHeader.biCompression == BI_RLE8 ||
		m_pBMI->bmiHeader.biCompression == FCC('RLE8'))
		return DecompressRLE8(pBackgroundDib);
	else if (	m_pBMI->bmiHeader.biCompression == BI_RLE4	||
				m_pBMI->bmiHeader.biCompression == FCC('RLE4'))
		return DecompressRLE4(pBackgroundDib);
	else
		return FALSE;
}

void CDib::Serialize(CArchive& ar) 
{
	CFile* pFile = ar.GetFile();
	if (pFile)
	{
		if (ar.IsStoring())
		{
			SaveBMP(*pFile);
		}
		else
		{
			LoadBMP(*pFile);
		}
	}
}

void CDib::Init()
{
	m_nStretchMode = COLORONCOLOR;
	m_hMMFile = INVALID_HANDLE_VALUE;
	m_hMMapping = NULL;
	m_pMMFile = NULL;
	m_bMMReadOnly = FALSE;
	m_pBMI = NULL;
	m_pColors = NULL;
	m_pOrigColors = NULL;
	m_pBits = NULL;
	m_pOrigBits = NULL;
	m_hDibSection = NULL;
	m_pDibSectionBits = NULL;
	m_pPalette = NULL;
	m_dwImageSize = 0;
	m_llPts = AV_NOPTS_VALUE;
	m_llUpTime = 0;
	m_dwUserFlag = 0;
	m_pPreviewDib = NULL;
	m_pThumbnailDib = NULL;
	m_dPreviewDibRatio = 0.0;
	m_dThumbnailDibRatio = 0.0;
	m_crBackgroundColor = RGB(0,0,0);
	m_pMetadata = NULL;

	m_wBrightness = 0;
	m_wContrast = 0;
	m_wSaturation = 0;
	m_wHue = 0;

	m_bColorUndoSet = FALSE;
	m_bGrayscale = FALSE;
	m_bAlpha = FALSE;
	
	m_bShowMessageBoxOnError = TRUE;

	m_wRedMask16 = 0;
	m_wGreenMask16 = 0;
	m_wBlueMask16 = 0;
	m_nGreenShift16 = 0;
	m_nRedShift16 = 0;
	m_nGreenDownShift16 = 0;
	m_nRedDownShift16 = 0;
	m_nBlueRoundShift16 = 0;
	m_nGreenRoundShift16 = 0;
	m_nRedRoundShift16 = 0;

	m_dwRedMask32 = 0;
	m_dwGreenMask32 = 0;
	m_dwBlueMask32 = 0;
	m_nGreenShift32 = 0;
	m_nRedShift32 = 0;
	m_nGreenDownShift32 = 0;
	m_nRedDownShift32 = 0;
	m_nBlueRoundShift32 = 0;
	m_nGreenRoundShift32 = 0;
	m_nRedRoundShift32 = 0;
	m_bFast32bpp = FALSE;

	m_nFloodFillStackSize = 0;
	m_pFloodFillStack = NULL;
	m_nFloodFillStackPos = 0;
}

void CDib::Free(BOOL bLeavePalette/*=FALSE*/,
				BOOL bLeaveHeader/*=FALSE*/,
				BOOL bLeavePreviewDib/*=FALSE*/,
				BOOL bLeaveThumbnailDib/*=FALSE*/,
				BOOL bLeaveMetadata/*=FALSE*/,
				BOOL bLeaveGIF/*=FALSE*/)
{
	if (bLeaveHeader)
		MMBMPToBMI();
	else
		UnMapBMP();
	
	if (!bLeaveHeader && m_pBMI)
	{
		delete [] m_pBMI;
		m_pBMI = NULL;
		m_pColors = NULL;
	}
	if (m_pBits)
	{
		BIGFREE(m_pBits);
		m_pBits = NULL;
	}
	ResetColorUndo();
	if (m_hDibSection)
	{
		::DeleteObject(m_hDibSection);
		m_hDibSection = NULL;
		m_pDibSectionBits = NULL;
	}

	if (!bLeavePreviewDib)
		DeletePreviewDib();
	if (!bLeaveThumbnailDib)
		DeleteThumbnailDib();
	if (!bLeaveMetadata)
	{
		if (m_pMetadata)
		{
			delete m_pMetadata;
			m_pMetadata = NULL;
		}
	}
	if (!bLeaveGIF)
		m_Gif.Free();
	if (!bLeavePalette && m_pPalette)
	{
		m_pPalette->DeleteObject();
		delete m_pPalette;
		m_pPalette = NULL;
	}
}

BOOL CDib::CreatePreviewDib(int nMaxSizeX,
							int nMaxSizeY,
							CDib* pSrcDib/*=NULL*/,
							CWnd* pProgressWnd/*=NULL*/,
							BOOL bProgressSend/*=TRUE*/,
							CWorkerThread* pThread/*=NULL*/)
{
	if (nMaxSizeX < 1 && nMaxSizeY < 1)
		return FALSE;

	if (pSrcDib == NULL)
		pSrcDib = this;

	// Free
	DeletePreviewDib();

	// Allocate Preview Dib
	m_pPreviewDib = (CDib*)new CDib;
	if (!m_pPreviewDib)
		return FALSE;

	// Copy Vars
	m_pPreviewDib->CopyVars(*pSrcDib);

	// Calc. Ratio
	double dPreviewDibRatioX = (double)pSrcDib->GetWidth() / (double)nMaxSizeX;
	double dPreviewDibRatioY = (double)pSrcDib->GetHeight() / (double)nMaxSizeY;
	m_dPreviewDibRatio = max(dPreviewDibRatioX, dPreviewDibRatioY);

	// Stretch Bits
	return m_pPreviewDib->StretchBits(	Round(pSrcDib->GetWidth() / m_dPreviewDibRatio),
										Round(pSrcDib->GetHeight() / m_dPreviewDibRatio),
										pSrcDib,
										pProgressWnd,
										bProgressSend,
										pThread);
}

void CDib::DeletePreviewDib()
{
	if (m_pPreviewDib)
	{
		delete m_pPreviewDib;
		m_pPreviewDib = NULL;
	}
}

BOOL CDib::CreateThumbnailDib(	int nMaxSizeX,
								int nMaxSizeY,
								CDib* pSrcDib/*=NULL*/,
								CWnd* pProgressWnd/*=NULL*/,
								BOOL bProgressSend/*=TRUE*/,
								CWorkerThread* pThread/*=NULL*/)
{
	if (nMaxSizeX < 1 && nMaxSizeY < 1)
		return FALSE;

	if (pSrcDib == NULL)
		pSrcDib = this;

	// Free
	DeleteThumbnailDib();

	// Allocate Thumbnail Dib
	m_pThumbnailDib = (CDib*)new CDib;
	if (!m_pThumbnailDib)
		return FALSE;

	// Copy Vars
	m_pThumbnailDib->CopyVars(*pSrcDib);

	// Calc. Ratio
	double dThumbnailDibRatioX = (double)pSrcDib->GetWidth() / (double)nMaxSizeX;
	double dThumbnailDibRatioY = (double)pSrcDib->GetHeight() / (double)nMaxSizeY;
	m_dThumbnailDibRatio = max(dThumbnailDibRatioX, dThumbnailDibRatioY);

	// Stretch Bits
	return m_pThumbnailDib->StretchBits(Round(pSrcDib->GetWidth() / m_dThumbnailDibRatio),
										Round(pSrcDib->GetHeight() / m_dThumbnailDibRatio),
										pSrcDib,
										pProgressWnd,
										bProgressSend,
										pThread);
}

void CDib::DeleteThumbnailDib()
{
	if (m_pThumbnailDib)
	{
		delete m_pThumbnailDib;
		m_pThumbnailDib = NULL;
	}
}

DWORD CDib::GetRMask() const
{
	if (GetBitCount() == 16)
		return m_wRedMask16;
	else if (GetBitCount() == 32)
		return m_dwRedMask32;
	else
		return 0xFF0000;
}

DWORD CDib::GetGMask() const
{
	if (GetBitCount() == 16)
		return m_wGreenMask16;
	else if (GetBitCount() == 32)
		return m_dwGreenMask32;
	else
		return 0x00FF00;
}

DWORD CDib::GetBMask() const
{
	if (GetBitCount() == 16)
		return m_wBlueMask16;
	else if (GetBitCount() == 32)
		return m_dwBlueMask32;
	else
		return 0x0000FF;
}

void CDib::InitMasks(LPBITMAPINFO pBMI/*=NULL*/)
{
	// Check
	if (pBMI == NULL)
		pBMI = m_pBMI;
	if (!pBMI)
		return;

	// Set Default
	m_bFast32bpp = FALSE;

	int nMask;
	if (GetBitCount(pBMI) == 16)
	{
		// Masks
		if (pBMI->bmiHeader.biCompression == BI_BITFIELDS)
		{
			LPBITMAPINFOBITFIELDS pBmiBf = (LPBITMAPINFOBITFIELDS)pBMI;
			m_wRedMask16 = (WORD)pBmiBf->biRedMask;
			m_wGreenMask16 = (WORD)pBmiBf->biGreenMask;
			m_wBlueMask16 = (WORD)pBmiBf->biBlueMask;
		}
		else
		{
			m_wRedMask16 = 0x7C00;		// 5 bits
			m_wGreenMask16 = 0x03E0;	// 5 bits
			m_wBlueMask16 = 0x001F;		// 5 bits
		}

		// Calc. the Blue Shift
		m_nBlueRoundShift16 = 0;
		nMask = m_wBlueMask16;
		while (nMask && ((nMask & 0x80) == 0))
		{
			m_nBlueRoundShift16 += 1;
			nMask <<= 1;
		} 
		
		// Calc. the Green Shifts
		m_nGreenShift16 = m_nGreenRoundShift16 = 0;
		nMask = m_wGreenMask16;
		while (nMask && ((nMask & 0x1) == 0))
		{
			m_nGreenShift16 += 1;
			nMask >>= 1;
		}
		while (nMask && ((nMask & 0x80) == 0))
		{
			m_nGreenRoundShift16 += 1;
			nMask <<= 1;
		}
		m_nGreenDownShift16 = m_nGreenShift16 - m_nGreenRoundShift16;

		// Calc. the Red Shifts
		m_nRedShift16 = m_nRedRoundShift16 = 0;
		nMask = m_wRedMask16;
		while (nMask && ((nMask & 0x1) == 0))
		{
			m_nRedShift16 += 1;
			nMask >>= 1;
		}
		while (nMask && ((nMask & 0x80) == 0))
		{
			m_nRedRoundShift16 += 1;
			nMask <<= 1;
		}
		m_nRedDownShift16 = m_nRedShift16 - m_nRedRoundShift16;
	}
	else if (GetBitCount(pBMI) == 32)
	{	
		// Masks
		if (pBMI->bmiHeader.biCompression == BI_BITFIELDS)
		{
			LPBITMAPINFOBITFIELDS pBmiBf = (LPBITMAPINFOBITFIELDS)pBMI;
			m_dwRedMask32 = pBmiBf->biRedMask;
			m_dwGreenMask32 = pBmiBf->biGreenMask;
			m_dwBlueMask32 = pBmiBf->biBlueMask;
			if (m_dwRedMask32   == 0x00FF0000 &&
				m_dwGreenMask32 == 0x0000FF00 &&
				m_dwBlueMask32  == 0x000000FF)
				m_bFast32bpp = TRUE;
		}
		else
		{
			m_dwRedMask32 = 0x00FF0000;
			m_dwGreenMask32 = 0x0000FF00;
			m_dwBlueMask32 = 0x000000FF;
			m_bFast32bpp = TRUE;
		}

		// Calc. the Blue Shift
		m_nBlueRoundShift32 = 0;
		nMask = m_dwBlueMask32;
		while (nMask && ((nMask & 0x80) == 0))
		{
			m_nBlueRoundShift32 += 1;
			nMask <<= 1;
		}

		// Calc. the Green Shifts
		m_nGreenShift32 = m_nGreenRoundShift32 = 0;
		nMask = m_dwGreenMask32;
		while ((nMask & 0x1) == 0)
		{
			m_nGreenShift32 += 1;
			nMask >>= 1;
		}
		while (nMask && ((nMask & 0x80) == 0))
		{
			m_nGreenRoundShift32 += 1;
			nMask <<= 1;
		}
		m_nGreenDownShift32 = m_nGreenShift32 - m_nGreenRoundShift32;

		// Calc. the Blue Shifts
		m_nRedShift32 = m_nRedRoundShift32 = 0;
		nMask = m_dwRedMask32;
		while ((nMask & 0x1) == 0)
		{
			m_nRedShift32 += 1;
			nMask >>= 1;
		}
		while (nMask && ((nMask & 0x80) == 0))
		{
			m_nRedRoundShift32 += 1;
			nMask <<= 1;
		}
		m_nRedDownShift32 = m_nRedShift32 - m_nRedRoundShift32;
	}
}

BOOL CDib::CreatePaletteFromColors(DWORD dwNumColors, RGBQUAD* pColors)
{
	if (!pColors || (dwNumColors == 0))
		return FALSE;
   
	// Allocate memory block for logical palette
	LPLOGPALETTE lpPal = (LPLOGPALETTE)new BYTE[sizeof(LOGPALETTE) +
												sizeof(PALETTEENTRY)*dwNumColors];
	if (!lpPal)
		return FALSE;

	// Set version and number of palette entries
	lpPal->palVersion = PALVERSION_DEFINE;
	lpPal->palNumEntries = (WORD)dwNumColors;

	for (int i = 0; i < (int)dwNumColors; i++)
	{
		lpPal->palPalEntry[i].peRed = pColors[i].rgbRed;
		lpPal->palPalEntry[i].peGreen = pColors[i].rgbGreen;
		lpPal->palPalEntry[i].peBlue = pColors[i].rgbBlue;
		lpPal->palPalEntry[i].peFlags = 0;
	}

	// Create the palette and get handle to it
	if (m_pPalette)
	{
		m_pPalette->DeleteObject();
		delete m_pPalette;
	}
	BOOL bResult = FALSE;
	m_pPalette = new CPalette;
	if (m_pPalette)
		bResult = m_pPalette->CreatePalette(lpPal);
	delete [] lpPal;

	return bResult;
}

BOOL CDib::CreatePaletteFromColors(CPalette* pPal, DWORD dwNumColors, RGBQUAD* pColors)
{
	if (!pColors || (dwNumColors == 0) || !pPal)
		return FALSE;

	// Allocate memory block for logical palette
	LPLOGPALETTE lpPal = (LPLOGPALETTE)new BYTE[sizeof(LOGPALETTE) +
												sizeof(PALETTEENTRY)*dwNumColors];
	if (!lpPal)
		return FALSE;

	// Set version and number of palette entries
	lpPal->palVersion = PALVERSION_DEFINE;
	lpPal->palNumEntries = (WORD)dwNumColors;

	for (int i = 0; i < (int)dwNumColors; i++)
	{
		lpPal->palPalEntry[i].peRed = pColors[i].rgbRed;
		lpPal->palPalEntry[i].peGreen = pColors[i].rgbGreen;
		lpPal->palPalEntry[i].peBlue = pColors[i].rgbBlue;
		lpPal->palPalEntry[i].peFlags = 0;
	}

	// Create the palette
	BOOL bResult = pPal->CreatePalette(lpPal);
	delete [] lpPal;

	return bResult;
}

CPalette* CDib::GetPalette()
{
	if (m_pPalette == NULL)
	{
		m_pPalette = new CPalette;
		CreateHalftonePalette(m_pPalette, 256);
		return m_pPalette;
	}
	else
		return m_pPalette;
}

BOOL CDib::CreatePaletteFromBMI()
{
	if (!m_pBMI)
		return FALSE;

   // Get the number of colors in the DIB
   WORD wNumColors = GetNumColors();

   if (wNumColors != 0)
   {
	   if (!m_pColors)
		   return FALSE;

		// Allocate memory block for logical palette
		LPLOGPALETTE lpPal = (LPLOGPALETTE)new BYTE[sizeof(LOGPALETTE) +
													sizeof(PALETTEENTRY)*wNumColors];
		if (!lpPal)
			return FALSE;

		// Set version and number of palette entries
		lpPal->palVersion = PALVERSION_DEFINE;
		lpPal->palNumEntries = (WORD)wNumColors;

		for (int i = 0; i < (int)wNumColors; i++)
		{
			lpPal->palPalEntry[i].peRed = m_pColors[i].rgbRed;
			lpPal->palPalEntry[i].peGreen = m_pColors[i].rgbGreen;
			lpPal->palPalEntry[i].peBlue = m_pColors[i].rgbBlue;
			lpPal->palPalEntry[i].peFlags = 0;
		}

		// Create the palette and get handle to it
		if (m_pPalette)
		{
			m_pPalette->DeleteObject();
			delete m_pPalette;
		}
		BOOL bResult = FALSE;
		m_pPalette = new CPalette;
		if (m_pPalette)
			bResult = m_pPalette->CreatePalette(lpPal);
		delete [] lpPal;

		return bResult;
	}
	else
		return FALSE;
}

BOOL CDib::FillColorsFromPalette(CPalette* pPalette)
{
	if (!m_pColors)
		return FALSE;

	// Create HalftonePalette if no palette supplied
	CPalette* pPal;
	if (!pPalette)
	{
		pPal = new CPalette;
        if (!CreateHalftonePalette(pPal, GetNumColors()))
		{
			if (pPal)
			{
				pPal->DeleteObject();
				delete pPal;
			}
			return FALSE;
		}
	}
	else
		pPal = pPalette;

	WORD wNumColors;
	if (GetNumColors() < pPal->GetEntryCount())
		wNumColors = GetNumColors();
	else
		wNumColors = pPal->GetEntryCount();

	// Get Palette Entries
	LPPALETTEENTRY lpPalEntries = (LPPALETTEENTRY)new BYTE[sizeof(PALETTEENTRY) * pPal->GetEntryCount()];
	if (!lpPalEntries)
	{
		if (!pPalette)
		{
			pPal->DeleteObject();
			delete pPal;
		}
		return FALSE;
	}
	pPal->GetPaletteEntries(0, pPal->GetEntryCount(), lpPalEntries);

	// Copy Entries
	for (int i = 0; i < (int)wNumColors; i++)
	{
		m_pColors[i].rgbRed = lpPalEntries[i].peRed;
		m_pColors[i].rgbGreen = lpPalEntries[i].peGreen;
		m_pColors[i].rgbBlue = lpPalEntries[i].peBlue;
	}

	// Free
	if (!pPalette)
	{
		pPal->DeleteObject();
		delete pPal;
	}
	delete [] lpPalEntries;

	return TRUE;
}

BOOL CDib::CreatePaletteFromDibSection()
{
	HPALETTE hPalette = NULL;
	BITMAP bm;
	::GetObject(m_hDibSection, sizeof(BITMAP), &bm);

	// If the DIBSection is 256 color or less, it has a color table
	if ((bm.bmBitsPixel * bm.bmPlanes) <= 8)
	{
		// Create a memory DC and select the DIBSection into it
		HDC hMemDC = ::CreateCompatibleDC(NULL);
		HBITMAP hOldBitmap = (HBITMAP)::SelectObject(hMemDC, m_hDibSection);

		// Get the DIBSection's color table
		RGBQUAD rgb[256];
		UINT nEntries = ::GetDIBColorTable(hMemDC, 0, 256, rgb);

		// Create a palette from the color table
		LOGPALETTE* pLogPal = (LOGPALETTE*) new BYTE[sizeof(LOGPALETTE) + (nEntries*sizeof(PALETTEENTRY))];
		if (!pLogPal)
		{
			::SelectObject(hMemDC, hOldBitmap);
			::DeleteDC(hMemDC);
			return FALSE;
		}
		pLogPal->palVersion = PALVERSION_DEFINE;
		pLogPal->palNumEntries = (WORD)nEntries;
		for (UINT i=0; i<nEntries; i++)
		{
			pLogPal->palPalEntry[i].peRed = rgb[i].rgbRed;
			pLogPal->palPalEntry[i].peGreen = rgb[i].rgbGreen;
			pLogPal->palPalEntry[i].peBlue = rgb[i].rgbBlue;
			pLogPal->palPalEntry[i].peFlags = 0;
		}
		hPalette = ::CreatePalette(pLogPal);

		// Clean up
		::SelectObject(hMemDC, hOldBitmap);
		::DeleteDC(hMemDC);
		delete [] pLogPal;

		// Create the m_pPalette
		if (m_pPalette)
		{
			m_pPalette->DeleteObject();
			delete m_pPalette;
		}
		m_pPalette = new CPalette;
		if (m_pPalette)
			return m_pPalette->Attach(hPalette);
		else
			return FALSE;
	}
	else
		return FALSE;
}

BOOL CDib::ToBITMAPV5HEADER()
{
	if (!m_pBMI)
		return FALSE;

	if (m_pBMI->bmiHeader.biSize == sizeof(BITMAPV5HEADER))
		return TRUE;

	LPBITMAPV5HEADER pBV5;
	if (GetBitCount() <= 8)
	{
		pBV5 = (LPBITMAPV5HEADER)new BYTE[sizeof(BITMAPV5HEADER) + sizeof(RGBQUAD) * GetNumColors()];
		if (!pBV5)
			return FALSE;

		// Init & Copy Header
		memset(pBV5, 0, sizeof(BITMAPV5HEADER));
		memcpy(pBV5, m_pBMI, m_pBMI->bmiHeader.biSize);
		pBV5->bV5Size = sizeof(BITMAPV5HEADER);

		// Copy Colors
		memcpy((LPBYTE)pBV5 + pBV5->bV5Size, (LPBYTE)m_pBMI + m_pBMI->bmiHeader.biSize, sizeof(RGBQUAD) * GetNumColors());
	}
	else if (GetCompression() == BI_BITFIELDS)
	{
		pBV5 = (LPBITMAPV5HEADER)new BYTE[sizeof(BITMAPV5HEADER) + 3 * sizeof(DWORD)];
		if (!pBV5)
			return FALSE;

		// Init & Copy Header
		memset(pBV5, 0, sizeof(BITMAPV5HEADER));
		memcpy(pBV5, m_pBMI, m_pBMI->bmiHeader.biSize);
		pBV5->bV5Size = sizeof(BITMAPV5HEADER);

		// Copy Masks if source header is BITMAPINFOHEADER
		// (if source header is BITMAPV4HEADER they have already been copied)
		if (m_pBMI->bmiHeader.biSize == sizeof(BITMAPINFOHEADER))
		{
			LPBITMAPINFOBITFIELDS pBmiBf = (LPBITMAPINFOBITFIELDS)m_pBMI;
			pBV5->bV5RedMask = pBmiBf->biRedMask;
			pBV5->bV5GreenMask = pBmiBf->biGreenMask;
			pBV5->bV5BlueMask = pBmiBf->biBlueMask;
		}

		// Init also the 3 * sizeof(DWORD) duplicated ending masks
		*((DWORD*)((LPBYTE)pBV5 + pBV5->bV5Size)) = pBV5->bV5RedMask;
		*((DWORD*)((LPBYTE)pBV5 + pBV5->bV5Size + sizeof(DWORD))) = pBV5->bV5GreenMask;
		*((DWORD*)((LPBYTE)pBV5 + pBV5->bV5Size + 2 * sizeof(DWORD))) = pBV5->bV5BlueMask;
	}
	else
	{
		pBV5 = (LPBITMAPV5HEADER)new BYTE[sizeof(BITMAPV5HEADER)];
		if (!pBV5)
			return FALSE;

		// Init & Copy Header
		memset(pBV5, 0, sizeof(BITMAPV5HEADER));
		memcpy(pBV5, m_pBMI, m_pBMI->bmiHeader.biSize);
		pBV5->bV5Size = sizeof(BITMAPV5HEADER);
	}

	// Init alpha if not already set while copying the BITMAPV4HEADER source header
	if (m_bAlpha && GetBitCount() == 32 && pBV5->bV5AlphaMask == 0)
		pBV5->bV5AlphaMask = 0xFF000000;
	
	// Init CS Type if source header is BITMAPINFOHEADER
	// (if source header is BITMAPV4HEADER it has already been copied)
	if (m_pBMI->bmiHeader.biSize == sizeof(BITMAPINFOHEADER))
		pBV5->bV5CSType = LCS_WINDOWS_COLOR_SPACE;

	// Always set Intent as it is a BITMAPV5HEADER only member
	pBV5->bV5Intent = LCS_GM_IMAGES;

	// Free
	delete [] m_pBMI;

	// Change Pointer
	m_pBMI = (LPBITMAPINFO)pBV5;

	// Set Colors Pointer
	if (GetBitCount() <= 8)
		m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + (WORD)(m_pBMI->bmiHeader.biSize));
	else
		m_pColors = NULL;

	return TRUE;
}

BOOL CDib::ToBITMAPINFOHEADER()
{
	if (!m_pBMI)
		return FALSE;

	if (m_pBMI->bmiHeader.biSize == sizeof(BITMAPINFOHEADER))
		return TRUE;

	LPBITMAPINFOHEADER pNewBMI;
	if (GetBitCount() <= 8)
	{
		pNewBMI = (LPBITMAPINFOHEADER)new BYTE[sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * GetNumColors()];
		if (!pNewBMI)
			return FALSE;

		// Init & Copy Header
		memcpy(pNewBMI, m_pBMI, sizeof(BITMAPINFOHEADER));
		pNewBMI->biSize = sizeof(BITMAPINFOHEADER);

		// Copy Colors
		memcpy((LPBYTE)pNewBMI + pNewBMI->biSize, (LPBYTE)m_pBMI + m_pBMI->bmiHeader.biSize, sizeof(RGBQUAD) * GetNumColors());
	}
	else if (GetCompression() == BI_BITFIELDS)
	{
		pNewBMI = (LPBITMAPINFOHEADER)new BYTE[sizeof(BITMAPINFOHEADER) + 3 * sizeof(DWORD)];
		if (!pNewBMI)
			return FALSE;

		// Init & Copy Header + Masks
		// for BITMAPV4HEADER and BITMAPV5HEADER it's not known whether m_pBMI includes
		// the 3 * sizeof(DWORD) duplicated ending masks -> take the masks from the members
		memcpy(pNewBMI, m_pBMI, sizeof(BITMAPINFOHEADER) + 3 * sizeof(DWORD));
		pNewBMI->biSize = sizeof(BITMAPINFOHEADER);
	}
	else
	{
		pNewBMI = (LPBITMAPINFOHEADER)new BYTE[sizeof(BITMAPINFOHEADER)];
		if (!pNewBMI)
			return FALSE;

		// Init & Copy Header
		memcpy(pNewBMI, m_pBMI, sizeof(BITMAPINFOHEADER));
		pNewBMI->biSize = sizeof(BITMAPINFOHEADER);
	}

	// Free
	delete[] m_pBMI;

	// Change Pointer
	m_pBMI = (LPBITMAPINFO)pNewBMI;

	// Set Colors Pointer
	if (GetBitCount() <= 8)
		m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + (WORD)(m_pBMI->bmiHeader.biSize));
	else
		m_pColors = NULL;

	return TRUE;
}

BOOL CDib::DibSectionInitBMI()
{
	if (!m_hDibSection)
		return FALSE;

	return DibSectionInitBMI(m_hDibSection);
}

BOOL CDib::DibSectionInitBMI(HBITMAP hDibSection)
{
	// Make sure it is a DibSection
	if (!IsDibSection(hDibSection))
		return FALSE;

	DIBSECTION dibsection;

	int nRes = ::GetObject(hDibSection, sizeof(DIBSECTION), (LPVOID)&dibsection);
	if (nRes == sizeof(DIBSECTION) && dibsection.dsBmih.biSize >= sizeof(BITMAPINFOHEADER))
	{
		if (m_pBits)
		{
			BIGFREE(m_pBits);
			m_pBits = NULL;
		}
		ResetColorUndo();
		if (m_pBMI)
		{
			delete [] m_pBMI;
			m_pBMI = NULL;
			m_pColors = NULL;
		}

		DWORD dwNumColors;
		DWORD dwClrUsed = dibsection.dsBmih.biClrUsed;
		WORD wBitCount = dibsection.dsBmih.biBitCount;
		if (dwClrUsed != 0)
			dwNumColors = dwClrUsed;
		else
		{
			switch (wBitCount)
			{
				case 1:
					dwNumColors = 2;
				break;
				case 4:
					dwNumColors = 16;
				break;
				case 8:
					dwNumColors = 256;
				break;
				default:
					dwNumColors = 0;
			}
		}

		int nSize;
		if (dibsection.dsBmih.biCompression == BI_BITFIELDS)
		{
			nSize = dibsection.dsBmih.biSize + 3 * sizeof(DWORD);
			m_pBMI = (LPBITMAPINFO)new BYTE[nSize];
			if (!m_pBMI)
				return FALSE;
			memcpy((void*)m_pBMI, (void*)(&(dibsection.dsBmih)), nSize);
			m_pColors = NULL;
		}
		else
		{
			nSize = dibsection.dsBmih.biSize + sizeof(RGBQUAD) * dwNumColors;
			m_pBMI = (LPBITMAPINFO)new BYTE[nSize];
			if (!m_pBMI)
				return FALSE;
			memcpy((void*)m_pBMI, (void*)(&(dibsection.dsBmih)), dibsection.dsBmih.biSize);
			if (dwNumColors != 0)
				m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + (WORD)(m_pBMI->bmiHeader.biSize));
			else
				m_pColors = NULL;
		}

		ComputeImageSize();

		// If the DIBSection is 256 color or less, it has a color table
		if (dwNumColors != 0)
		{
			// Create a memory DC and select the DIBSection into it
			HDC hMemDC = ::CreateCompatibleDC(NULL);
			HBITMAP hOldBitmap = (HBITMAP)::SelectObject(hMemDC, hDibSection);

			// Get the DIBSection's color table
			UINT nEntries = ::GetDIBColorTable(hMemDC, 0, dwNumColors, m_pColors);
			VERIFY(nEntries == dwNumColors);

			// Clean up
			::SelectObject(hMemDC, hOldBitmap);
			::DeleteDC(hMemDC);
		}

		// Init Masks For 16 and 32 bits Pictures
		InitMasks();

		return TRUE;
	}
	else
		return FALSE;
}

COLORREF CDib::HighlightColor(COLORREF rcColor)
{
	BYTE Gray = RGBToGray(	GetRValue(rcColor),
							GetGValue(rcColor),
							GetBValue(rcColor));

	// Invert
	BYTE newGray = ~Gray;

	if (ABS(Gray - newGray) < 128) // If Gray Between 64 - 191
	{
		if (Gray < 128) newGray = 255;
		else newGray = 0;
	}

	return RGB(newGray, newGray, newGray);
}

// biSizeImage can be 0 if the file isn't compressed,
// if a compression is used, biSizeImage must be
// the right value(!=0)
void CDib::ComputeImageSize()
{
	if (!m_pBMI)
		return;

	m_dwImageSize = m_pBMI->bmiHeader.biSizeImage;	// Only compressed DIBs must set this field 
													// (compressed size is written into biSizeImage)
	if (m_dwImageSize == 0)
	{
		// no compression
		m_dwImageSize = DWALIGNEDWIDTHBYTES(GetWidth() * GetBitCount()) * GetHeight();
		m_pBMI->bmiHeader.biSizeImage = m_dwImageSize;
	}
}

BOOL CDib::DoAutoOrientate(CDib* pDib)
{
	if (!pDib)
		return FALSE;

	int nOrientation = pDib->GetExifInfo()->Orientation;
	if ((nOrientation >= 2) && (nOrientation <= 8))
		return TRUE;
	else
		return FALSE;
}

BOOL CDib::AutoOrientateDib(CDib* pDib)
{
	if (!pDib)
		return FALSE;

	int nOrientation = pDib->GetExifInfo()->Orientation;
	if ((nOrientation >= 2) && (nOrientation <= 8))
	{
		switch (nOrientation)
		{
			case 2 :	// Flip Horizonat = Flip Left-Right
						if (pDib->IsValid())
							pDib->FlipLeftRight();
						if (pDib->GetPreviewDib() && pDib->GetPreviewDib()->IsValid())
							pDib->GetPreviewDib()->FlipLeftRight();
						if (pDib->GetThumbnailDib() && pDib->GetThumbnailDib()->IsValid())
							pDib->GetThumbnailDib()->FlipLeftRight();
						break;

			case 3 :	// Rotate 180°
						if (pDib->IsValid())
							pDib->Rotate180();
						if (pDib->GetPreviewDib() && pDib->GetPreviewDib()->IsValid())
							pDib->GetPreviewDib()->Rotate180();
						if (pDib->GetThumbnailDib() && pDib->GetThumbnailDib()->IsValid())
							pDib->GetThumbnailDib()->Rotate180();
						break;

			case 4 :	// Flip Vertical = Flip Top-Down
						if (pDib->IsValid())
							pDib->FlipTopDown();
						if (pDib->GetPreviewDib() && pDib->GetPreviewDib()->IsValid())
							pDib->GetPreviewDib()->FlipTopDown();
						if (pDib->GetThumbnailDib() && pDib->GetThumbnailDib()->IsValid())
							pDib->GetThumbnailDib()->FlipTopDown();
						break;

			case 5 :	// Transpose = 90CW + Flip Left-Right
						//           = Flip Left-Right + 90CCW
						if (pDib->IsValid())
						{
							if (pDib->Rotate90CW())
								pDib->FlipLeftRight();
						}
						else
							pDib->SwapWidthHeight(); // Only Swap Header if any
						if (pDib->GetPreviewDib() && pDib->GetPreviewDib()->IsValid())
						{
							if (pDib->GetPreviewDib()->Rotate90CW())
								pDib->GetPreviewDib()->FlipLeftRight();
						}
						if (pDib->GetThumbnailDib() && pDib->GetThumbnailDib()->IsValid())
						{
							if (pDib->GetThumbnailDib()->Rotate90CW())
								pDib->GetThumbnailDib()->FlipLeftRight();
						}
						break;

			case 6 :	// Rotate 90° Clockwise
						if (pDib->IsValid())
							pDib->Rotate90CW();
						else
							pDib->SwapWidthHeight(); // Only Swap Header if any
						if (pDib->GetPreviewDib() && pDib->GetPreviewDib()->IsValid())
							pDib->GetPreviewDib()->Rotate90CW();
						if (pDib->GetThumbnailDib() && pDib->GetThumbnailDib()->IsValid())
							pDib->GetThumbnailDib()->Rotate90CW();
						break;	

			case 7 :	// Transverse = 90CW + Flip Top-Down
						//            = Flip Top-Down + 90CCW
						if (pDib->IsValid())
						{
							if (pDib->Rotate90CW())
								pDib->FlipTopDown();
						}
						else
							pDib->SwapWidthHeight(); // Only Swap Header if any
						if (pDib->GetPreviewDib() && pDib->GetPreviewDib()->IsValid())
						{
							if (pDib->GetPreviewDib()->Rotate90CW())
								pDib->GetPreviewDib()->FlipTopDown();
						}
						if (pDib->GetThumbnailDib() && pDib->GetThumbnailDib()->IsValid())
						{
							if (pDib->GetThumbnailDib()->Rotate90CW())
								pDib->GetThumbnailDib()->FlipTopDown();
						}
						break;

			case 8 :	// Rotate 90° Counter-Clockwise
						if (pDib->IsValid())
							pDib->Rotate90CCW();
						else
							pDib->SwapWidthHeight(); // Only Swap Header if any
						if (pDib->GetPreviewDib() && pDib->GetPreviewDib()->IsValid())
							pDib->GetPreviewDib()->Rotate90CCW();
						if (pDib->GetThumbnailDib() && pDib->GetThumbnailDib()->IsValid())
							pDib->GetThumbnailDib()->Rotate90CCW();
						break;	

			default:	break;
		}
		
		return TRUE;
	}
	else
		return FALSE;
}

BOOL CDib::AutoOrientateThumbnailDib(CDib* pDib)
{
	if (!pDib)
		return FALSE;

	int nOrientation = pDib->GetExifInfo()->Orientation;
	if ((nOrientation >= 2) && (nOrientation <= 8))
	{
		switch (nOrientation)
		{
			case 2 :	// Flip Horizonat = Flip Left-Right
						if (pDib->GetThumbnailDib() && pDib->GetThumbnailDib()->IsValid())
							pDib->GetThumbnailDib()->FlipLeftRight();
						break;

			case 3 :	// Rotate 180°
						if (pDib->GetThumbnailDib() && pDib->GetThumbnailDib()->IsValid())
							pDib->GetThumbnailDib()->Rotate180();
						break;

			case 4 :	// Flip Vertical = Flip Top-Down
						if (pDib->GetThumbnailDib() && pDib->GetThumbnailDib()->IsValid())
							pDib->GetThumbnailDib()->FlipTopDown();
						break;

			case 5 :	// Transpose = 90CW + Flip Left-Right
						//           = Flip Left-Right + 90CCW
						if (pDib->GetThumbnailDib() && pDib->GetThumbnailDib()->IsValid())
						{
							if (pDib->GetThumbnailDib()->Rotate90CW())
								pDib->GetThumbnailDib()->FlipLeftRight();
						}
						break;

			case 6 :	// Rotate 90° Clockwise
						if (pDib->GetThumbnailDib() && pDib->GetThumbnailDib()->IsValid())
							pDib->GetThumbnailDib()->Rotate90CW();
						break;	

			case 7 :	// Transverse = 90CW + Flip Top-Down
						//            = Flip Top-Down + 90CCW
						if (pDib->GetThumbnailDib() && pDib->GetThumbnailDib()->IsValid())
						{
							if (pDib->GetThumbnailDib()->Rotate90CW())
								pDib->GetThumbnailDib()->FlipTopDown();
						}
						break;

			case 8 :	// Rotate 90° Counter-Clockwise
						if (pDib->GetThumbnailDib() && pDib->GetThumbnailDib()->IsValid())
							pDib->GetThumbnailDib()->Rotate90CCW();
						break;	

			default:	break;
		}
		
		return TRUE;
	}
	else
		return FALSE;
}

void CDib::InitGetClosestColorIndex()
{
	if (!m_GetClosestColorIndexLookUp)
		m_GetClosestColorIndexLookUp = new signed short[65536];
	if (m_GetClosestColorIndexLookUp)
		memset((void*)m_GetClosestColorIndexLookUp, -1, 65536 * sizeof(signed short));	// 2^(5+6+5) = 65536
}

void CDib::FreeGetClosestColorIndex()
{
	if (m_GetClosestColorIndexLookUp)
	{
		delete [] m_GetClosestColorIndexLookUp;
		m_GetClosestColorIndexLookUp = NULL;
	}
}

void CDib::ShowError(DWORD dwErrorCode, BOOL bShowMessageBoxOnError, const CString& sFunctionName)
{
	CString sText;
	LPVOID lpMsgBuf = NULL;

	if (::FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dwErrorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR)&lpMsgBuf,
		0,
		NULL) && lpMsgBuf)
	{
		// Init
		sText = (LPCTSTR)lpMsgBuf;
		
		// Remove terminating CR and LF
		sText.TrimRight(_T("\r\n"));

		// Replace eventual CRs or LFs in the middle of the string with a space
		sText.Replace(_T('\r'), _T(' '));
		sText.Replace(_T('\n'), _T(' '));

		// Format message
#ifdef _DEBUG
		sText = sFunctionName + _T(":\n") + sText;
#endif
	}
	else
	{
#ifdef _DEBUG
		sText.Format(_T("%s Failed"), sFunctionName);
#else
		sText = _T("Failure");
#endif
	}

	// Free buffer
	if (lpMsgBuf)
		::LocalFree(lpMsgBuf);

	// Show Error
	if (g_nLogLevel > 0)
		::LogLine(_T("%s"), sText);
	if (bShowMessageBoxOnError)
		::AfxMessageBox(sText, MB_ICONSTOP);
}

void CDib::ShowError(DWORD dwErrorCode, const CString& sFunctionName)
{
	ShowError(dwErrorCode, m_bShowMessageBoxOnError, sFunctionName);
}

void CDib::ShowLastError(BOOL bShowMessageBoxOnError, const CString& sFunctionName)
{
	ShowError(::GetLastError(), bShowMessageBoxOnError, sFunctionName);
}

void CDib::ShowLastError(const CString& sFunctionName)
{
	ShowLastError(m_bShowMessageBoxOnError, sFunctionName);
}

int CDib::CFileInfo::GetNumColors() const
{
	if (m_nNumColors != 0)
		return m_nNumColors;
	else
	{
		switch (m_nBpp)
		{
			case 1:
				return 2;

			case 2:
				return 4;

			case 4:
				return 16;

			case 6:
				return 64;

			case 8:
				return 256;

			case 10:
				return 1024;

			case 12:
				return 4096;

			case 14:
				return 16384;

			case 16:
				if (m_bAlphaChannel)
					return 256;
				else
					return 65536;

			case 24:
				return 0;

			case 32:
				if (m_bAlphaChannel)
					return 65536;
				else
					return 0;

			default:
				return 0;
		}
	}
}

CString CDib::CFileInfo::GetTIFFDepthName()
{
	CString sText;
	CString sCompression;

	switch (m_nCompression)
	{
		case COMPRESSION_NONE : sCompression = _T("");						// Uncompressed
			break;
		case COMPRESSION_CCITTRLE : sCompression = _T("Huffman RLE");		// CCITT modified Huffman RLE
			break;
		case COMPRESSION_CCITTFAX3 : sCompression = _T("CCITT Fax3");		// CCITT Group 3 fax encoding
			break;
		case COMPRESSION_CCITTFAX4 : sCompression = _T("CCITT Fax4");		// CCITT Group 4 fax encoding
			break;
		case COMPRESSION_LZW : sCompression = _T("LZW");					// Lempel-Ziv  & Welch
			break;
		case COMPRESSION_OJPEG : sCompression = _T("OJPEG");				// !6.0 JPEG
			break;
		case COMPRESSION_JPEG : sCompression = _T("JPEG");					// JPEG DCT compression
			break;
		case COMPRESSION_NEXT : sCompression = _T("Next RLE");				// NeXT 2-bit RLE
			break;
		case COMPRESSION_CCITTRLEW : sCompression = _T("Huffman RLE");		// #1 w/ word alignment
			break;
		case COMPRESSION_PACKBITS : sCompression = _T("Packbits");			// Macintosh RLE
			break;
		case COMPRESSION_THUNDERSCAN : sCompression = _T("ThunderScan RLE");// ThunderScan RLE
			break;
		case COMPRESSION_IT8CTPAD : sCompression = _T("IT8 CT");			// IT8 CT w/padding
			break;
		case COMPRESSION_IT8LW : sCompression = _T("IT8 Linework");			// IT8 Linework RLE
			break;
		case COMPRESSION_IT8MP : sCompression = _T("IT8 Monochrome");		// IT8 Monochrome picture
			break;
		case COMPRESSION_IT8BL : sCompression = _T("IT8 Line Art");			// IT8 Binary line art
			break;
		case COMPRESSION_PIXARFILM : sCompression = _T("Pixar LZW");		// Pixar companded 10bit LZW
			break;
		case COMPRESSION_PIXARLOG : sCompression = _T("Pixar ZIP");			// Pixar companded 11bit ZIP
			break;
		case COMPRESSION_DEFLATE : sCompression = _T("ZIP");				// Deflate compression
			break;
		case COMPRESSION_ADOBE_DEFLATE : sCompression = _T("Adobe ZIP");	// Deflate compression
			break;
		case COMPRESSION_DCS : sCompression = _T("Kodak DCS");				// Kodak DCS encoding
			break;
		case COMPRESSION_JBIG : sCompression = _T("ISO JBIG");				// ISO JBIG
			break;
		case COMPRESSION_SGILOG : sCompression = _T("SGI Log L. RLE");		// SGI Log Luminance RLE
			break;
		case COMPRESSION_SGILOG24 : sCompression = _T("SGI Log 24-bit");	// SGI Log 24-bit packed
			break;
		case COMPRESSION_JP2000 : sCompression = _T("JPEG2000");			// Leadtools JPEG2000
			break;
		default : sCompression = _T("");
			break;
	}
	
	if (sCompression == _T(""))
	{
		if (m_nColorSpace == COLORSPACE_GRAYSCALE)
		{
			if (m_bAlphaChannel)
				sText.Format(_T("GA%d, %d Levels"), m_nBpp, GetNumColors());
			else
			{
				if (m_nBpp == 32)
					sText.Format(_T("G%d, 2^32 Gray Levels"), m_nBpp);
				else if (m_nBpp == 24)
					sText.Format(_T("G%d, 2^24 Gray Levels"), m_nBpp);
				else
					sText.Format(_T("G%d, %d Gray Levels"), m_nBpp, GetNumColors());
			}
		}
		else if (m_nColorSpace == COLORSPACE_CMYK)
		{	
			if (m_bAlphaChannel)
				sText.Format(_T("CMYKA, %d Bpp"), m_nBpp);
			else
				sText.Format(_T("CMYK, %d Bpp"), m_nBpp);
		}
		else
		{
			if (m_bAlphaChannel)
				sText.Format(_T("RGBA%d, True Colors"), m_nBpp);
			else
			{
				if (m_nBpp == 16)
				{
					if (m_bPalette)
						sText = _T("RGB16 (Pal.), 65536 Col.");
					else
						sText = _T("RGB16 (555), 65536 Col.");
				}
				else
				{
					if (m_nBpp <= 8)
						sText.Format(_T("RGB%d, %i Colors"), m_nBpp, GetNumColors());
					else
						sText.Format(_T("RGB%d, True Colors"), m_nBpp);
				}
			}
		}
	}
	else
	{
		if (m_nColorSpace == COLORSPACE_GRAYSCALE)
		{
			if (m_bAlphaChannel)
				sText.Format(_T("GA%d, %d Grays, %s"), m_nBpp, GetNumColors(), sCompression);
			else
				sText.Format(_T("G%d, %s"), m_nBpp, sCompression);
		}
		else if (m_nColorSpace == COLORSPACE_CMYK)
		{	
			if (m_bAlphaChannel)
				sText.Format(_T("CMYKA, %s"), sCompression);
			else
				sText.Format(_T("CMYK, %s"), sCompression);
		}
		else
		{
			if (m_bAlphaChannel)
				sText.Format(_T("RGBA%d, %s"), m_nBpp, sCompression);
			else
				sText.Format(_T("RGB%d, %s"), m_nBpp, sCompression);
		}
	}

	return sText;   
}

CString CDib::CFileInfo::GetDepthName()
{
	// Special handling for TIFF files
	if (m_nType == TIFF)
		return GetTIFFDepthName();

	CString sText;

	switch (m_nCompression)
	{	
		case BI_RGB :
		case BI_JPEG :
			if (m_nColorSpace == COLORSPACE_GRAYSCALE)
			{
				if (m_bAlphaChannel)
					sText.Format(_T("GA%d, %d Levels"), m_nBpp, GetNumColors());
				else
				{
					if (m_nNumTransparencyIndexes >= 1)
						sText.Format(_T("G%d, %d Gray, Transp."), m_nBpp, GetNumColors());
					else
					{
						if (m_nBpp == 32)
							sText.Format(_T("G%d, 2^32 Gray Levels"), m_nBpp);
						else if (m_nBpp == 24)
							sText.Format(_T("G%d, 2^24 Gray Levels"), m_nBpp);
						else
							sText.Format(_T("G%d, %d Gray Levels"), m_nBpp, GetNumColors());
					}
				}
			}
			else if (m_nColorSpace == COLORSPACE_YCbCr)
			{
				sText.Format(_T("YCbCr, %d Bpp"), m_nBpp);
			}
			else if (m_nColorSpace == COLORSPACE_CMYK)
			{	
				sText.Format(_T("CMYK, %d Bpp"), m_nBpp);
			}
			else if (m_nColorSpace == COLORSPACE_YCCK)
			{
				sText.Format(_T("YCCK, %d Bpp"), m_nBpp);
			}
			else
			{
				if (m_bAlphaChannel)
					sText.Format(_T("RGBA%d, True Colors"), m_nBpp);
				else
				{
					if (m_nBpp == 16)
					{
						if (m_bPalette)
							sText = _T("RGB16 (Pal.), 65536 Col.");
						else
							sText = _T("RGB16 (555), 32768 Col.");
					}
					else
					{
						if (m_nBpp <= 8)
						{
							if (m_nNumTransparencyIndexes >= 1)
								sText.Format(_T("RGB%d, %i Col., Transp."), m_nBpp, GetNumColors());
							else
								sText.Format(_T("RGB%d, %i Colors"), m_nBpp, GetNumColors());
						}
						else
						{
							if (m_nNumTransparencyIndexes >= 1)
								sText.Format(_T("RGB%d, True Col., Transp."), m_nBpp);
							else
								sText.Format(_T("RGB%d, True Colors"), m_nBpp);
						}
					}
				}
			}
			return sText;
		case BI_RLE8 :		
			sText.Format(_T("RLE8, %i Colors"), GetNumColors());
			return sText;
		case BI_RLE4 :		
			sText.Format(_T("RLE4, %i Colors"), GetNumColors());
			return sText;
		case BI_BITFIELDS :
			if (m_nBpp == 16)
			{
				if ((m_dwBlueMask == 0x001F)	&&
					(m_dwGreenMask == 0x07E0)	&&
					(m_dwRedMask == 0xF800))
				{
#ifdef _DEBUG
					sText = _T("BITFIELDS16 (565), 65536 Col.");
#else
					sText = _T("RGB16 (565), 65536 Col.");
#endif
				}
				else if ((m_dwBlueMask == 0x001F)&&
						(m_dwGreenMask == 0x03E0)&&
						(m_dwRedMask == 0x7C00))
				{
#ifdef _DEBUG
					sText = _T("BITFIELDS16 (555), 32768 Col.");
#else
					sText = _T("RGB16 (555), 32768 Col.");
#endif
				}
				else
				{
#ifdef _DEBUG
					sText = _T("BITFIELDS16, 65536 Col.");
#else
					sText = _T("RGB16, 65536 Col.");
#endif
				}
			}
			else
			{
				if (m_bAlphaChannel)
				{
#ifdef _DEBUG
					sText.Format(_T("BITFIELDSA%d, True Colors"), m_nBpp);
#else
					sText.Format(_T("RGBA%d, True Colors"), m_nBpp);
#endif
				}
				else
				{
#ifdef _DEBUG
					sText.Format(_T("BITFIELDS%d, True Colors"), m_nBpp);
#else
					sText.Format(_T("RGB%d, True Colors"), m_nBpp);
#endif
				}
			}
			return sText;
		default : 
		{
			char ch0 = (char)(m_nCompression & 0xFF);
			char ch1 = (char)((m_nCompression >> 8) & 0xFF);
			char ch2 = (char)((m_nCompression >> 16) & 0xFF);
			char ch3 = (char)((m_nCompression >> 24) & 0xFF);
			WCHAR wch0, wch1, wch2, wch3;
			mbtowc(&wch0, &ch0, sizeof(WCHAR));
			mbtowc(&wch1, &ch1, sizeof(WCHAR));
			mbtowc(&wch2, &ch2, sizeof(WCHAR));
			mbtowc(&wch3, &ch3, sizeof(WCHAR));
			return (CString(wch0) + CString(wch1) + CString(wch2) + CString(wch3));
		}
	}
}

#ifdef VIDEODEVICEDOC
std::atomic<LONGLONG> CDib::m_llOverallSharedMemoryBytes(0);
#endif

// Windows Standard Colors
// Order: Blue, Green, Red, Reserved
RGBQUAD CDib::ms_StdColors[] = {
	{0x00, 0x00, 0x00, 0x00 },	// System palette - first 10 colors
	{0x00, 0x00, 0x80, 0x00 },
	{0x00, 0x80, 0x00, 0x00 },
	{0x00, 0x80, 0x80, 0x00 },
	{0x80, 0x00, 0x00, 0x00 },
	{0x80, 0x00, 0x80, 0x00 },
	{0x80, 0x80, 0x00, 0x00 },
	{0xC0, 0xC0, 0xC0, 0x00 },
	{0xC0, 0xDC, 0xC0, 0x00 },
	{0xF0, 0xCA, 0xA6, 0x00 },

	{0x00, 0x00, 0x2C, 0x00 },	// System palette - middle 236 colors
	{0x00, 0x00, 0x56, 0x00 },
	{0x00, 0x00, 0x87, 0x00 },
	{0x00, 0x00, 0xC0, 0x00 },
	{0x00, 0x00, 0xFF, 0x00 },
	{0x00, 0x2C, 0x00, 0x00 },
	{0x00, 0x2C, 0x2C, 0x00 },
	{0x00, 0x2C, 0x56, 0x00 },
	{0x00, 0x2C, 0x87, 0x00 },
	{0x00, 0x2C, 0xC0, 0x00 },
	{0x00, 0x2C, 0xFF, 0x00 },
	{0x00, 0x56, 0x00, 0x00 },
	{0x00, 0x56, 0x2C, 0x00 },
	{0x00, 0x56, 0x56, 0x00 },
	{0x00, 0x56, 0x87, 0x00 },
	{0x00, 0x56, 0xC0, 0x00 },
	{0x00, 0x56, 0xFF, 0x00 },
	{0x00, 0x87, 0x00, 0x00 },
	{0x00, 0x87, 0x2C, 0x00 },
	{0x00, 0x87, 0x56, 0x00 },
	{0x00, 0x87, 0x87, 0x00 },
	{0x00, 0x87, 0xC0, 0x00 },
	{0x00, 0x87, 0xFF, 0x00 },
	{0x00, 0xC0, 0x00, 0x00 },
	{0x00, 0xC0, 0x2C, 0x00 },
	{0x00, 0xC0, 0x56, 0x00 },
	{0x00, 0xC0, 0x87, 0x00 },
	{0x00, 0xC0, 0xC0, 0x00 },
	{0x00, 0xC0, 0xFF, 0x00 },
	{0x00, 0xFF, 0x00, 0x00 },
	{0x00, 0xFF, 0x2C, 0x00 },
	{0x00, 0xFF, 0x56, 0x00 },
	{0x00, 0xFF, 0x87, 0x00 },
	{0x00, 0xFF, 0xC0, 0x00 },
	{0x00, 0xFF, 0xFF, 0x00 },
	{0x2C, 0x00, 0x00, 0x00 },
	{0x2C, 0x00, 0x2C, 0x00 },
	{0x2C, 0x00, 0x56, 0x00 },
	{0x2C, 0x00, 0x87, 0x00 },
	{0x2C, 0x00, 0xC0, 0x00 },
	{0x2C, 0x00, 0xFF, 0x00 },
	{0x2C, 0x2C, 0x00, 0x00 },
	{0x2C, 0x2C, 0x2C, 0x00 },
	{0x2C, 0x2C, 0x56, 0x00 },
	{0x2C, 0x2C, 0x87, 0x00 },
	{0x2C, 0x2C, 0xC0, 0x00 },
	{0x2C, 0x2C, 0xFF, 0x00 },
	{0x2C, 0x56, 0x00, 0x00 },
	{0x2C, 0x56, 0x2C, 0x00 },
	{0x2C, 0x56, 0x56, 0x00 },
	{0x2C, 0x56, 0x87, 0x00 },
	{0x2C, 0x56, 0xC0, 0x00 },
	{0x2C, 0x56, 0xFF, 0x00 },
	{0x2C, 0x87, 0x00, 0x00 },
	{0x2C, 0x87, 0x2C, 0x00 },
	{0x2C, 0x87, 0x56, 0x00 },
	{0x2C, 0x87, 0x87, 0x00 },
	{0x2C, 0x87, 0xC0, 0x00 },
	{0x2C, 0x87, 0xFF, 0x00 },
	{0x2C, 0xC0, 0x00, 0x00 },
	{0x2C, 0xC0, 0x2C, 0x00 },
	{0x2C, 0xC0, 0x56, 0x00 },
	{0x2C, 0xC0, 0x87, 0x00 },
	{0x2C, 0xC0, 0xC0, 0x00 },
	{0x2C, 0xC0, 0xFF, 0x00 },
	{0x2C, 0xFF, 0x00, 0x00 },
	{0x2C, 0xFF, 0x2C, 0x00 },
	{0x2C, 0xFF, 0x56, 0x00 },
	{0x2C, 0xFF, 0x87, 0x00 },
	{0x2C, 0xFF, 0xC0, 0x00 },
	{0x2C, 0xFF, 0xFF, 0x00 },
	{0x56, 0x00, 0x00, 0x00 },
	{0x56, 0x00, 0x2C, 0x00 },
	{0x56, 0x00, 0x56, 0x00 },
	{0x56, 0x00, 0x87, 0x00 },
	{0x56, 0x00, 0xC0, 0x00 },
	{0x56, 0x00, 0xFF, 0x00 },
	{0x56, 0x2C, 0x00, 0x00 },
	{0x56, 0x2C, 0x2C, 0x00 },
	{0x56, 0x2C, 0x56, 0x00 },
	{0x56, 0x2C, 0x87, 0x00 },
	{0x56, 0x2C, 0xC0, 0x00 },
	{0x56, 0x2C, 0xFF, 0x00 },
	{0x56, 0x56, 0x00, 0x00 },
	{0x56, 0x56, 0x2C, 0x00 },
	{0x56, 0x56, 0x56, 0x00 },
	{0x56, 0x56, 0x87, 0x00 },
	{0x56, 0x56, 0xC0, 0x00 },
	{0x56, 0x56, 0xFF, 0x00 },
	{0x56, 0x87, 0x00, 0x00 },
	{0x56, 0x87, 0x2C, 0x00 },
	{0x56, 0x87, 0x56, 0x00 },
	{0x56, 0x87, 0x87, 0x00 },
	{0x56, 0x87, 0xC0, 0x00 },
	{0x56, 0x87, 0xFF, 0x00 },
	{0x56, 0xC0, 0x00, 0x00 },
	{0x56, 0xC0, 0x2C, 0x00 },
	{0x56, 0xC0, 0x56, 0x00 },
	{0x56, 0xC0, 0x87, 0x00 },
	{0x56, 0xC0, 0xC0, 0x00 },
	{0x56, 0xC0, 0xFF, 0x00 },
	{0x56, 0xFF, 0x00, 0x00 },
	{0x56, 0xFF, 0x2C, 0x00 },
	{0x56, 0xFF, 0x56, 0x00 },
	{0x56, 0xFF, 0x87, 0x00 },
	{0x56, 0xFF, 0xC0, 0x00 },
	{0x56, 0xFF, 0xFF, 0x00 },
	{0x87, 0x00, 0x00, 0x00 },
	{0x87, 0x00, 0x2C, 0x00 },
	{0x87, 0x00, 0x56, 0x00 },
	{0x87, 0x00, 0x87, 0x00 },
	{0x87, 0x00, 0xC0, 0x00 },
	{0x87, 0x00, 0xFF, 0x00 },
	{0x87, 0x2C, 0x00, 0x00 },
	{0x87, 0x2C, 0x2C, 0x00 },
	{0x87, 0x2C, 0x56, 0x00 },
	{0x87, 0x2C, 0x87, 0x00 },
	{0x87, 0x2C, 0xC0, 0x00 },
	{0x87, 0x2C, 0xFF, 0x00 },
	{0x87, 0x56, 0x00, 0x00 },
	{0x87, 0x56, 0x2C, 0x00 },
	{0x87, 0x56, 0x56, 0x00 },
	{0x87, 0x56, 0x87, 0x00 },
	{0x87, 0x56, 0xC0, 0x00 },
	{0x87, 0x56, 0xFF, 0x00 },
	{0x87, 0x87, 0x00, 0x00 },
	{0x87, 0x87, 0x2C, 0x00 },
	{0x87, 0x87, 0x56, 0x00 },
	{0x87, 0x87, 0x87, 0x00 },
	{0x87, 0x87, 0xC0, 0x00 },
	{0x87, 0x87, 0xFF, 0x00 },
	{0x87, 0xC0, 0x00, 0x00 },
	{0x87, 0xC0, 0x2C, 0x00 },
	{0x87, 0xC0, 0x56, 0x00 },
	{0x87, 0xC0, 0x87, 0x00 },
	{0x87, 0xC0, 0xC0, 0x00 },
	{0x87, 0xC0, 0xFF, 0x00 },
	{0x87, 0xFF, 0x00, 0x00 },
	{0x87, 0xFF, 0x2C, 0x00 },
	{0x87, 0xFF, 0x56, 0x00 },
	{0x87, 0xFF, 0x87, 0x00 },
	{0x87, 0xFF, 0xC0, 0x00 },
	{0x87, 0xFF, 0xFF, 0x00 },
	{0xC0, 0x00, 0x00, 0x00 },
	{0xC0, 0x00, 0x2C, 0x00 },
	{0xC0, 0x00, 0x56, 0x00 },
	{0xC0, 0x00, 0x87, 0x00 },
	{0xC0, 0x00, 0xC0, 0x00 },
	{0xC0, 0x00, 0xFF, 0x00 },
	{0xC0, 0x2C, 0x00, 0x00 },
	{0xC0, 0x2C, 0x2C, 0x00 },
	{0xC0, 0x2C, 0x56, 0x00 },
	{0xC0, 0x2C, 0x87, 0x00 },
	{0xC0, 0x2C, 0xC0, 0x00 },
	{0xC0, 0x2C, 0xFF, 0x00 },
	{0xC0, 0x56, 0x00, 0x00 },
	{0xC0, 0x56, 0x2C, 0x00 },
	{0xC0, 0x56, 0x56, 0x00 },
	{0xC0, 0x56, 0x87, 0x00 },
	{0xC0, 0x56, 0xC0, 0x00 },
	{0xC0, 0x56, 0xFF, 0x00 },
	{0xC0, 0x87, 0x00, 0x00 },
	{0xC0, 0x87, 0x2C, 0x00 },
	{0xC0, 0x87, 0x56, 0x00 },
	{0xC0, 0x87, 0x87, 0x00 },
	{0xC0, 0x87, 0xC0, 0x00 },
	{0xC0, 0x87, 0xFF, 0x00 },
	{0xC0, 0xC0, 0x00, 0x00 },
	{0xC0, 0xC0, 0x2C, 0x00 },
	{0xC0, 0xC0, 0x56, 0x00 },
	{0xC0, 0xC0, 0x87, 0x00 },
	{0xC0, 0xC0, 0xFF, 0x00 },
	{0xC0, 0xFF, 0x00, 0x00 },
	{0xC0, 0xFF, 0x2C, 0x00 },
	{0xC0, 0xFF, 0x56, 0x00 },
	{0xC0, 0xFF, 0x87, 0x00 },
	{0xC0, 0xFF, 0xC0, 0x00 },
	{0xC0, 0xFF, 0xFF, 0x00 },
	{0xFF, 0x00, 0x00, 0x00 },
	{0xFF, 0x00, 0x2C, 0x00 },
	{0xFF, 0x00, 0x56, 0x00 },
	{0xFF, 0x00, 0x87, 0x00 },
	{0xFF, 0x00, 0xC0, 0x00 },
	{0xFF, 0x00, 0xFF, 0x00 },
	{0xFF, 0x2C, 0x00, 0x00 },
	{0xFF, 0x2C, 0x2C, 0x00 },
	{0xFF, 0x2C, 0x56, 0x00 },
	{0xFF, 0x2C, 0x87, 0x00 },
	{0xFF, 0x2C, 0xC0, 0x00 },
	{0xFF, 0x2C, 0xFF, 0x00 },
	{0xFF, 0x56, 0x00, 0x00 },
	{0xFF, 0x56, 0x2C, 0x00 },
	{0xFF, 0x56, 0x56, 0x00 },
	{0xFF, 0x56, 0x87, 0x00 },
	{0xFF, 0x56, 0xC0, 0x00 },
	{0xFF, 0x56, 0xFF, 0x00 },
	{0xFF, 0x87, 0x00, 0x00 },
	{0xFF, 0x87, 0x2C, 0x00 },
	{0xFF, 0x87, 0x56, 0x00 },
	{0xFF, 0x87, 0x87, 0x00 },
	{0xFF, 0x87, 0xC0, 0x00 },
	{0xFF, 0x87, 0xFF, 0x00 },
	{0xFF, 0xC0, 0x00, 0x00 },
	{0xFF, 0xC0, 0x2C, 0x00 },
	{0xFF, 0xC0, 0x56, 0x00 },
	{0xFF, 0xC0, 0x87, 0x00 },
	{0xFF, 0xC0, 0xC0, 0x00 },
	{0xFF, 0xC0, 0xFF, 0x00 },
	{0xFF, 0xFF, 0x2C, 0x00 },
	{0xFF, 0xFF, 0x56, 0x00 },
	{0xFF, 0xFF, 0x87, 0x00 },
	{0xFF, 0xFF, 0xC0, 0x00 },
	{0xFF, 0xFF, 0xFF, 0x00 },
	{0x11, 0x11, 0x11, 0x00 },
	{0x18, 0x18, 0x18, 0x00 },
	{0x1E, 0x1E, 0x1E, 0x00 },
	{0x25, 0x25, 0x25, 0x00 },
	{0x2C, 0x2C, 0x2C, 0x00 },
	{0x34, 0x34, 0x34, 0x00 },
	{0x3C, 0x3C, 0x3C, 0x00 },
	{0x44, 0x44, 0x44, 0x00 },
	{0x4D, 0x4D, 0x4D, 0x00 },
	{0x56, 0x56, 0x56, 0x00 },
	{0x5F, 0x5F, 0x5F, 0x00 },
	{0x69, 0x69, 0x69, 0x00 },
	{0x72, 0x72, 0x72, 0x00 },
	{0x7D, 0x7D, 0x7D, 0x00 },
	{0x92, 0x92, 0x92, 0x00 },
	{0x9D, 0x9D, 0x9D, 0x00 },
	{0xA8, 0xA8, 0xA8, 0x00 },
	{0xB4, 0xB4, 0xB4, 0x00 },
	{0xCC, 0xCC, 0xCC, 0x00 },
	{0xD8, 0xD8, 0xD8, 0x00 },
	{0xE5, 0xE5, 0xE5, 0x00 },
	{0xF2, 0xF2, 0xF2, 0x00 },
	{0xFF, 0xFF, 0xFF, 0x00 },

	{0xF0, 0xFB, 0xFF, 0x00 },	// System palette - last 10 colors
	{0xA4, 0xA0, 0xA0, 0x00 },
	{0x80, 0x80, 0x80, 0x00 },
	{0x00, 0x00, 0xFF, 0x00 },
	{0x00, 0xFF, 0x00, 0x00 },
	{0x00, 0xFF, 0xFF, 0x00 },
	{0xFF, 0x00, 0x00, 0x00 },
	{0xFF, 0x00, 0xFF, 0x00 },
	{0xFF, 0xFF, 0x00, 0x00 },
	{0xFF, 0xFF, 0xFF, 0x00 },
};
