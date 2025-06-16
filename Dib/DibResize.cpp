#include "stdafx.h"
#include "dib.h"

// Unfortunately avir.h defines one of its member functions as free(), 
// but that conflicts with the free debug macro defined in crtdbg.h,
// thus we have to undef the free macro during the inclusion of avir.h.
#pragma push_macro("free")
#undef free
#include "avir.h"
#pragma pop_macro("free")

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Pixels count that can be allocated safely on the IA32 heap
// (consider that AVIR allocates floats and additionally 2X upsizes the
// original image before performing the resample)
// A safe value is 18 Megapixels:
#define SAFE_PIX_COUNT_FOR_HEAP32		18000000

BOOL CDib::StretchBits(	DWORD dwNewWidth,
						DWORD dwNewHeight,
						CDib* pSrcDib/*=NULL*/,
						CWnd* pProgressWnd/*=NULL*/,
						BOOL bProgressSend/*=TRUE*/,
						CWorkerThread* pThread/*=NULL*/,
						BOOL bForceNearestNeighbor/*=FALSE*/)
{
	if (pSrcDib == NULL)
		pSrcDib = this;

	// Force nearest neighbor resize
	if (bForceNearestNeighbor)
	{
		return NearestNeighborResizeBits(	dwNewWidth,
											dwNewHeight,
											pSrcDib,
											pProgressWnd,
											bProgressSend,
											pThread);
	}
	// Only nearest neighbor and bilinear resample do not allocate floats on the heap,
	// so both cosume little memory, but the latter gives better results
	else if (pSrcDib->GetWidth() * pSrcDib->GetHeight() > SAFE_PIX_COUNT_FOR_HEAP32 &&
			dwNewWidth * dwNewHeight > SAFE_PIX_COUNT_FOR_HEAP32)
	{
		return BilinearResampleBits(dwNewWidth,
									dwNewHeight,
									pSrcDib,
									pProgressWnd,
									bProgressSend,
									pThread);
	}
	else
	{
		// Calc. Ratio
		double dRatioX = (double)pSrcDib->GetWidth() / (double)dwNewWidth;
		double dRatioY = (double)pSrcDib->GetHeight() / (double)dwNewHeight;
		double dRatioMin = min(dRatioX, dRatioY);

		// Shrink
		if (dRatioMin > 1.0)
		{
			// If source image too big start to shrink it to a safe size with bilinear resample
			// (BilinearResampleBits() doesn't allocate floats on the heap)
			if (pSrcDib->GetWidth() * pSrcDib->GetHeight() > SAFE_PIX_COUNT_FOR_HEAP32)
			{
				// In here we know that the source is big, but we also know that the destination is not, thus:
				// dwNewWidth * dwNewHeight <= SAFE_PIX_COUNT_FOR_HEAP32
				// (dSafeRatio * dwNewWidth) * (dSafeRatio * dwNewHeight) = SAFE_PIX_COUNT_FOR_HEAP32
				// -> dSafeRatio >= 1.0
				double dSafeRatio = sqrt((double)SAFE_PIX_COUNT_FOR_HEAP32 / (double)(dwNewWidth * dwNewHeight));

				// Bilinear resample
				if (BilinearResampleBits(	Round(dSafeRatio * dwNewWidth),
											Round(dSafeRatio * dwNewHeight),
											pSrcDib,
											pProgressWnd,
											bProgressSend,
											pThread))
					pSrcDib = this; // update source dib pointer
				else
					return FALSE;
			}

			// Try AVIR (fails if not 24 bpp or 32 bpp)
			if (AvirResizeBits(	dwNewWidth,
								dwNewHeight,
								pSrcDib,
								pProgressWnd,
								bProgressSend,
								pThread))
				return TRUE;
			else
				return BicubicResampleBits(	dwNewWidth,
											dwNewHeight,
											pSrcDib,
											pProgressWnd,
											bProgressSend,
											pThread);
		}
		// Enlarge
		else
		{
			// If destination image too big use bicubic resample
			// (BicubicResampleBits() only allocates floats on heap for the source pixels)
			if (dwNewWidth * dwNewHeight > SAFE_PIX_COUNT_FOR_HEAP32)
			{
				return BicubicResampleBits(	dwNewWidth,
											dwNewHeight,
											pSrcDib,
											pProgressWnd,
											bProgressSend,
											pThread);
			}
			else
			{
				// Try AVIR (fails if not 24 bpp or 32 bpp)
				if (AvirResizeBits(	dwNewWidth,
									dwNewHeight,
									pSrcDib,
									pProgressWnd,
									bProgressSend,
									pThread))
					return TRUE;
				else
					return BicubicResampleBits(	dwNewWidth,
												dwNewHeight,
												pSrcDib,
												pProgressWnd,
												bProgressSend,
												pThread);
			}
		}
	}
}

BOOL CDib::StretchBitsFitRect(	DWORD dwMaxWidth,
								DWORD dwMaxHeight,
								CDib* pSrcDib/*=NULL*/,
								CWnd* pProgressWnd/*=NULL*/,
								BOOL bProgressSend/*=TRUE*/,
								CWorkerThread* pThread/*=NULL*/,
								BOOL bForceNearestNeighbor/*=FALSE*/)
{
	if (pSrcDib == NULL)
		pSrcDib = this;

	// Calc. Ratio
	double dRatioX = (double)pSrcDib->GetWidth() / (double)dwMaxWidth;
	double dRatioY = (double)pSrcDib->GetHeight() / (double)dwMaxHeight;
	double dRatioMax = max(dRatioX, dRatioY);

	// Stretch
	return StretchBits(	Round(pSrcDib->GetWidth() / dRatioMax),
						Round(pSrcDib->GetHeight() / dRatioMax),
						pSrcDib,
						pProgressWnd,
						bProgressSend,
						pThread,
						bForceNearestNeighbor);
}

BOOL CDib::StretchBitsMaintainAspectRatio(	DWORD dwNewWidth,
											DWORD dwNewHeight,
											COLORREF crBorder/*=0*/,
											CDib* pSrcDib/*=NULL*/,
											CWnd* pProgressWnd/*=NULL*/,
											BOOL bProgressSend/*=TRUE*/,
											CWorkerThread* pThread/*=NULL*/,
											BOOL bForceNearestNeighbor/*=FALSE*/)
{
	if (pSrcDib == NULL)
		pSrcDib = this;

	// Calc. Ratio
	double dRatioX = (double)pSrcDib->GetWidth() / (double)dwNewWidth;
	double dRatioY = (double)pSrcDib->GetHeight() / (double)dwNewHeight;
	double dRatioMax = max(dRatioX, dRatioY);

	// Stretch
	if (!StretchBits(	Round(pSrcDib->GetWidth() / dRatioMax),
						Round(pSrcDib->GetHeight() / dRatioMax),
						pSrcDib,
						pProgressWnd,
						bProgressSend,
						pThread,
						bForceNearestNeighbor))
		return FALSE;

	// Add Borders
	DWORD dwLeft = (dwNewWidth - GetWidth()) / 2;
	DWORD dwRight = dwNewWidth - GetWidth() - dwLeft;
	DWORD dwTop = (dwNewHeight - GetHeight()) / 2;
	DWORD dwBottom = dwNewHeight - GetHeight() - dwTop;
	return AddBorders(	dwLeft, dwTop, dwRight, dwBottom,
						crBorder,
						NULL,
						pProgressWnd,
						bProgressSend,
						pThread);
}

// AVIR image resizing algorithm designed by Aleksey Vaneev
BOOL CDib::AvirResizeBits(	DWORD dwNewWidth,
							DWORD dwNewHeight,
							CDib* pSrcDib/*=NULL*/,
							CWnd* pProgressWnd/*=NULL*/,
							BOOL bProgressSend/*=TRUE*/,
							CWorkerThread* pThread/*=NULL*/)
{
	if (pSrcDib == NULL)
		pSrcDib = this;

	// Check
	if (dwNewWidth == 0 || dwNewHeight == 0	||
		pSrcDib->GetBitCount() <= 16		||
		(pSrcDib->GetBitCount() == 32 && !pSrcDib->HasAlpha() && !pSrcDib->IsFast32bpp()))
		return FALSE;

	// Make a Copy of this?
	CDib SrcDib;
	BOOL bCopySrcToDst = FALSE;
	if (pSrcDib == NULL || this == pSrcDib)
	{
		// If Same Size Return
		if ((dwNewWidth == GetWidth()) && (dwNewHeight == GetHeight()))
			return TRUE;

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
			delete[] m_pBMI;

			// Allocate & Copy BMI
			m_pBMI = (LPBITMAPINFO)new BYTE[pSrcDib->GetBMISize()];
			if (m_pBMI == NULL)
				return FALSE;
			memcpy((void*)m_pBMI, (void*)pSrcDib->m_pBMI, pSrcDib->GetBMISize());
		}

		// Make Sure m_pColors Points to the Right Place
		if (m_pBMI->bmiHeader.biBitCount <= 8)
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + m_pBMI->bmiHeader.biSize);
		else
			m_pColors = NULL;

		// Copy Src To Dst
		bCopySrcToDst = TRUE;
	}

	// Src Width & Height
	DWORD dwSrcWidth = pSrcDib->GetWidth();
	DWORD dwSrcHeight = pSrcDib->GetHeight();

	// Scan Line Alignments
	DWORD uiDIBSourceScanLineSize = DWALIGNEDWIDTHBYTES(dwSrcWidth * pSrcDib->GetBitCount());
	DWORD uiDIBTargetScanLineSize = DWALIGNEDWIDTHBYTES(dwNewWidth * GetBitCount());

	// Allocate Bits
	if (m_pBits == NULL)
	{
		// Allocate memory
		m_pBits = (LPBYTE)BIGALLOC(uiDIBTargetScanLineSize * dwNewHeight);
	}
	// Need to ReAllocate Bits because they are of different size
	else if (m_dwImageSize != uiDIBTargetScanLineSize * dwNewHeight)
	{
		BIGFREE(m_pBits);

		// Allocate memory
		m_pBits = (LPBYTE)BIGALLOC(uiDIBTargetScanLineSize * dwNewHeight);
	}
	if (m_pBits == NULL)
		return FALSE;

	// Copy Vars
	if (bCopySrcToDst)
		CopyVars(*pSrcDib);

	// Change BMI
	m_pBMI->bmiHeader.biWidth = dwNewWidth;
	m_pBMI->bmiHeader.biHeight = dwNewHeight;
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

	// If Same Size Copy
	if (bCopySrcToDst && (dwNewWidth == pSrcDib->GetWidth()) && (dwNewHeight == pSrcDib->GetHeight()))
	{
		memcpy((void*)m_pBits, (void*)pSrcDib->m_pBits, m_dwImageSize);
		return TRUE;
	}

	DIB_INIT_PROGRESS;

	// Progress Guess of 30%
	DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, 30, 100);

	// Do Exit?
	if (pThread && pThread->DoExit())
	{
		DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
		return FALSE;
	}

	// AVIR Resize (no progress functionality in AVIR library...)
	avir::CImageResizer<> ImageResizer(8);
	ImageResizer.resizeImage(	pSrcDib->GetBits(),
								pSrcDib->GetWidth(),
								pSrcDib->GetHeight(),
								uiDIBSourceScanLineSize,
								m_pBits,
								GetWidth(),
								GetHeight(),
								GetBitCount() == 32 ? 4 : 3,
								0.0);

	// Progress Guess of 80%
	DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, 80, 100);

	// Do Exit?
	if (pThread && pThread->DoExit())
	{
		DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
		return FALSE;
	}

	// Adjust the DWORD alignment for 24 bpp images
	if (GetBitCount() == 24)
	{
		DWORD uiAvirScanLineSize = 3 * GetWidth();
		for (int line = (int)GetHeight() - 1; line > 0; line--)
		{
			memmove(m_pBits + line * uiDIBTargetScanLineSize,
					m_pBits + line * uiAvirScanLineSize,
					uiAvirScanLineSize);
		}
	}

	// Create Palette
	CreatePaletteFromBMI();

	DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

	return TRUE;
}

// Nearest Neighbor Resize
BOOL CDib::NearestNeighborResizeBits(	DWORD dwNewWidth,
										DWORD dwNewHeight,
										CDib* pSrcDib/*=NULL*/,
										CWnd* pProgressWnd/*=NULL*/,
										BOOL bProgressSend/*=TRUE*/,
										CWorkerThread* pThread/*=NULL*/)
{
	// Check New Sizes
	if ((dwNewWidth <= 0) || (dwNewHeight <= 0))
		return FALSE;

	// Make a Copy of this?
	CDib SrcDib;
	BOOL bCopySrcToDst = FALSE;
	if (pSrcDib == NULL || this == pSrcDib)
	{
		// If Same Size Return
		if ((dwNewWidth == GetWidth()) && (dwNewHeight == GetHeight()))
			return TRUE;

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
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + m_pBMI->bmiHeader.biSize);
		else
			m_pColors = NULL;

		// Copy Src To Dst
		bCopySrcToDst = TRUE;
	}

	// Src Width & Height
	DWORD dwSrcWidth = pSrcDib->GetWidth();
	DWORD dwSrcHeight = pSrcDib->GetHeight();

	// Scan Line Alignments
	DWORD uiDIBSourceScanLineSize = DWALIGNEDWIDTHBYTES(dwSrcWidth * pSrcDib->GetBitCount());
	DWORD uiDIBTargetScanLineSize = DWALIGNEDWIDTHBYTES(dwNewWidth * GetBitCount());

	// Allocate Bits
	if (m_pBits == NULL)
	{
		// Allocate memory
		m_pBits = (LPBYTE)BIGALLOC(uiDIBTargetScanLineSize * dwNewHeight);
	}
	// Need to ReAllocate Bits because they are of different size
	else if (m_dwImageSize != uiDIBTargetScanLineSize * dwNewHeight)
	{
		BIGFREE(m_pBits);

		// Allocate memory
		m_pBits = (LPBYTE)BIGALLOC(uiDIBTargetScanLineSize * dwNewHeight);
	}
	if (m_pBits == NULL)
		return FALSE;

	// Copy Vars
	if (bCopySrcToDst)
		CopyVars(*pSrcDib);

	// Change BMI
	m_pBMI->bmiHeader.biWidth = dwNewWidth;
	m_pBMI->bmiHeader.biHeight = dwNewHeight;
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

	// If Same Size Copy
	if (bCopySrcToDst && (dwNewWidth == pSrcDib->GetWidth()) && (dwNewHeight == pSrcDib->GetHeight()))
	{
		memcpy((void*)m_pBits, (void*)pSrcDib->m_pBits, m_dwImageSize);
		return TRUE;
	}

	DIB_INIT_PROGRESS;

	// Resize
	double dXScale, dYScale, dX, dY;
	dXScale = (double)dwSrcWidth  / (double)dwNewWidth;
	dYScale = (double)dwSrcHeight / (double)dwNewHeight;
	if (pSrcDib->GetBitCount() <= 8)
	{
		for (unsigned int y = 0 ; y < dwNewHeight ; y++)
		{
			if (pThread && pThread->DoExit())
			{
				DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
				return FALSE;
			}
			DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, dwNewHeight);

			dY = y * dYScale;
			for (unsigned int x = 0 ; x < dwNewWidth ; x++)
			{
				dX = x * dXScale;
				SetPixelIndex(x, y, pSrcDib->GetPixelIndex((int)dX, (int)dY)); // Truncate Do Not Use Round!
			}
		}
	}
	// Optimized
	else if (pSrcDib->GetBitCount() == 24)
	{
		int nX, nY;
		LPBYTE lpSrcBits = pSrcDib->GetBits();
		LPBYTE lpDstBits = GetBits();

		for (unsigned int y = 0 ; y < dwNewHeight ; y++)
		{
			if (pThread && pThread->DoExit())
			{
				DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
				return FALSE;
			}
			DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, dwNewHeight);

			dY = y * dYScale;
			nY = (int)dY; // Truncate Do Not Use Round!

			for (unsigned int x = 0 ; x < dwNewWidth ; x++)
			{
				dX = x * dXScale;
				nX = (int)dX; // Truncate Do Not Use Round!

				lpDstBits[3*x]   = lpSrcBits[3*nX   + nY * uiDIBSourceScanLineSize];
				lpDstBits[3*x+1] = lpSrcBits[3*nX+1 + nY * uiDIBSourceScanLineSize];
				lpDstBits[3*x+2] = lpSrcBits[3*nX+2 + nY * uiDIBSourceScanLineSize];
			}
			lpDstBits += uiDIBTargetScanLineSize;
		}
	}
	// Optimized
	else if (!pSrcDib->HasAlpha() && pSrcDib->IsFast32bpp() && pSrcDib->GetBitCount() == 32)
	{
		int nX, nY;
		LPBYTE lpSrcBits = pSrcDib->GetBits();
		LPBYTE lpDstBits = GetBits();

		for (unsigned int y = 0 ; y < dwNewHeight ; y++)
		{
			if (pThread && pThread->DoExit())
			{
				DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
				return FALSE;
			}
			DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, dwNewHeight);

			dY = y * dYScale;
			nY = (int)dY; // Truncate Do Not Use Round!

			for (unsigned int x = 0 ; x < dwNewWidth ; x++)
			{
				dX = x * dXScale;
				nX = (int)dX; // Truncate Do Not Use Round!

				lpDstBits[4*x]   = lpSrcBits[4*nX   + nY * uiDIBSourceScanLineSize];
				lpDstBits[4*x+1] = lpSrcBits[4*nX+1 + nY * uiDIBSourceScanLineSize];
				lpDstBits[4*x+2] = lpSrcBits[4*nX+2 + nY * uiDIBSourceScanLineSize];
			}
			lpDstBits += uiDIBTargetScanLineSize;
		}
	}
	// Optimized
	else if (pSrcDib->HasAlpha() && pSrcDib->GetBitCount() == 32)
	{
		int nX, nY;
		LPBYTE lpSrcBits = pSrcDib->GetBits();
		LPBYTE lpDstBits = GetBits();

		for (unsigned int y = 0 ; y < dwNewHeight ; y++)
		{
			if (pThread && pThread->DoExit())
			{
				DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
				return FALSE;
			}
			DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, dwNewHeight);

			dY = y * dYScale;
			nY = (int)dY; // Truncate Do Not Use Round!

			for (unsigned int x = 0 ; x < dwNewWidth ; x++)
			{
				dX = x * dXScale;
				nX = (int)dX; // Truncate Do Not Use Round!

				lpDstBits[4*x]   = lpSrcBits[4*nX   + nY * uiDIBSourceScanLineSize];
				lpDstBits[4*x+1] = lpSrcBits[4*nX+1 + nY * uiDIBSourceScanLineSize];
				lpDstBits[4*x+2] = lpSrcBits[4*nX+2 + nY * uiDIBSourceScanLineSize];
				lpDstBits[4*x+3] = lpSrcBits[4*nX+3 + nY * uiDIBSourceScanLineSize];
			}
			lpDstBits += uiDIBTargetScanLineSize;
		}
	}
	else
	{
		for (unsigned int y = 0 ; y < dwNewHeight ; y++)
		{
			if (pThread && pThread->DoExit())
			{
				DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
				return FALSE;
			}
			DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, dwNewHeight);

			dY = y * dYScale;
			for (unsigned int x = 0 ; x < dwNewWidth ; x++)
			{
				dX = x * dXScale;
				SetPixelColor(x, y, pSrcDib->GetPixelColor((int)dX, (int)dY));	// Truncate Do Not Use Round!
			}
		}
	}

	// Create Palette
	CreatePaletteFromBMI();

	DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

	return TRUE;
}

// Bilinear Interpolation
BOOL CDib::BilinearResampleBits(DWORD dwNewWidth,
								DWORD dwNewHeight,
								CDib* pSrcDib/*=NULL*/,
								CWnd* pProgressWnd/*=NULL*/,
								BOOL bProgressSend/*=TRUE*/,
								CWorkerThread* pThread/*=NULL*/)
{
	// Check New Sizes
	if ((dwNewWidth <= 0) || (dwNewHeight <= 0))
		return FALSE;

	// Make a Copy of this?
	CDib SrcDib;
	BOOL bCopySrcToDst = FALSE;
	if (pSrcDib == NULL || this == pSrcDib)
	{
		// If Same Size Return
		if ((dwNewWidth == GetWidth()) && (dwNewHeight == GetHeight()))
			return TRUE;

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
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + m_pBMI->bmiHeader.biSize);
		else
			m_pColors = NULL;

		// Copy Src To Dst
		bCopySrcToDst = TRUE;
	}

	// Src Width & Height
	DWORD dwSrcWidth = pSrcDib->GetWidth();
	DWORD dwSrcHeight = pSrcDib->GetHeight();

	// Scan Line Alignments
	DWORD uiDIBSourceScanLineSize = DWALIGNEDWIDTHBYTES(dwSrcWidth * pSrcDib->GetBitCount());
	DWORD uiDIBTargetScanLineSize = DWALIGNEDWIDTHBYTES(dwNewWidth * GetBitCount());

	// Allocate Bits
	if (m_pBits == NULL)
	{
		// Allocate memory
		m_pBits = (LPBYTE)BIGALLOC(uiDIBTargetScanLineSize * dwNewHeight);
	}
	// Need to ReAllocate Bits because they are of different size
	else if (m_dwImageSize != uiDIBTargetScanLineSize * dwNewHeight)
	{
		BIGFREE(m_pBits);

		// Allocate memory
		m_pBits = (LPBYTE)BIGALLOC(uiDIBTargetScanLineSize * dwNewHeight);
	}
	if (m_pBits == NULL)
		return FALSE;

	// Copy Vars
	if (bCopySrcToDst)
		CopyVars(*pSrcDib);

	// Change BMI
	m_pBMI->bmiHeader.biWidth = dwNewWidth;
	m_pBMI->bmiHeader.biHeight = dwNewHeight;
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

	// If Same Size Copy
	if (bCopySrcToDst && (dwNewWidth == pSrcDib->GetWidth()) && (dwNewHeight == pSrcDib->GetHeight()))
	{
		memcpy((void*)m_pBits, (void*)pSrcDib->m_pBits, m_dwImageSize);
		return TRUE;
	}

	DIB_INIT_PROGRESS;

	// Resample
	double dXScale, dYScale, dXOld, dYOld;
	dXScale = (double)dwSrcWidth  / (double)dwNewWidth;
	dYScale = (double)dwSrcHeight / (double)dwNewHeight;
	double dr13, dr24, dg13, dg24, db13, db24, da13, da24;
	double dDeltaXOld, dDeltaYOld;
	int nXOld, nYOld, nXOld1, nYOld1, nXOldMax, nYOldMax;
	BYTE r, g, b, a;
	BYTE r1, g1, b1, a1;
	BYTE r2, g2, b2, a2;
	BYTE r3, g3, b3, a3;
	BYTE r4, g4, b4, a4;
	COLORREF crColor1, crColor2, crColor3, crColor4;
	nXOldMax = (int)dwSrcWidth - 1;
	nYOldMax = (int)dwSrcHeight - 1;
	if (pSrcDib->GetBitCount() <= 8)
	{
		InitGetClosestColorIndex();
		for (unsigned int y = 0 ; y < dwNewHeight ; y++)
		{
			if (pThread && pThread->DoExit())
			{
				DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
				return FALSE;
			}
			DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, dwNewHeight);

			// Map Destination Pixel Center Back
			dYOld = (y + 0.5) * dYScale;
			nYOld1 = Round(dYOld); // Center Y coord. of the 4 source pixels
			nYOld = nYOld1 - 1;			
			dDeltaYOld = dYOld - nYOld1 + 0.5;

			for (unsigned int x = 0 ; x < dwNewWidth ; x++)
			{
				// Map Destination Pixel Center Back
				dXOld = (x + 0.5) * dXScale;
				nXOld1 = Round(dXOld); // Center X coord.of the 4 source pixels
				nXOld = nXOld1 - 1;			
				dDeltaXOld = dXOld - nXOld1 + 0.5;

				// Interpolate using the four nearest pixels in the source,
				// Clamp done in GetPixelColor()
				crColor1 = pSrcDib->GetPixelColor(nXOld,  nYOld);
				crColor2 = pSrcDib->GetPixelColor(nXOld1, nYOld);
				crColor3 = pSrcDib->GetPixelColor(nXOld,  nYOld1);
				crColor4 = pSrcDib->GetPixelColor(nXOld1, nYOld1);

				// Interpolate in y direction:
				dr13 = GetRValue(crColor1)	* (1.0 - dDeltaYOld) + GetRValue(crColor3)	* dDeltaYOld;
				dg13 = GetGValue(crColor1)	* (1.0 - dDeltaYOld) + GetGValue(crColor3)	* dDeltaYOld;
				db13 = GetBValue(crColor1)	* (1.0 - dDeltaYOld) + GetBValue(crColor3)	* dDeltaYOld;
				dr24 = GetRValue(crColor2)	* (1.0 - dDeltaYOld) + GetRValue(crColor4)	* dDeltaYOld;
				dg24 = GetGValue(crColor2)	* (1.0 - dDeltaYOld) + GetGValue(crColor4)	* dDeltaYOld;
				db24 = GetBValue(crColor2)	* (1.0 - dDeltaYOld) + GetBValue(crColor4)	* dDeltaYOld;
				
				// Interpolate in x direction:
				r = Round(dr13 * (1.0 - dDeltaXOld) + dr24 * dDeltaXOld);
				g = Round(dg13 * (1.0 - dDeltaXOld) + dg24 * dDeltaXOld);
				b = Round(db13 * (1.0 - dDeltaXOld) + db24 * dDeltaXOld);
				
				// Set Output Index
				SetPixelIndex(x, y, GetClosestColorIndex(RGB(r, g, b)));
			}
		}
	}
	// Optimized
	else if (pSrcDib->GetBitCount() == 24)
	{
		LPBYTE lpSrcBits = pSrcDib->GetBits();
		LPBYTE lpDstBits = GetBits();

		for (unsigned int y = 0 ; y < dwNewHeight ; y++)
		{
			if (pThread && pThread->DoExit())
			{
				DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
				return FALSE;
			}
			DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, dwNewHeight);

			// Map Destination Pixel Center Back
			dYOld = (y + 0.5) * dYScale;
			nYOld1 = Round(dYOld); // Center Y coord. of the 4 source pixels
			nYOld = nYOld1 - 1;			
			dDeltaYOld = dYOld - nYOld1 + 0.5;

			// Clamp
			nYOld = MAX(0, nYOld);
			nYOld1 = MIN(nYOldMax, nYOld1);

			if (nYOld1 <= nYOldMax)
			{
				for (unsigned int x = 0 ; x < dwNewWidth ; x++)
				{
					// Map Destination Pixel Center Back
					dXOld = (x + 0.5) * dXScale;
					nXOld1 = Round(dXOld); // Center X coord.of the 4 source pixels
					nXOld = nXOld1 - 1;			
					dDeltaXOld = dXOld - nXOld1 + 0.5;

					// Clamp
					nXOld = MAX(0, nXOld);
					nXOld1 = MIN(nXOldMax, nXOld1);

					// Interpolate using the four nearest pixels in the source
					b1 = lpSrcBits[3*nXOld    + nYOld * uiDIBSourceScanLineSize];
					g1 = lpSrcBits[3*nXOld+1  + nYOld * uiDIBSourceScanLineSize];
					r1 = lpSrcBits[3*nXOld+2  + nYOld * uiDIBSourceScanLineSize];
					b2 = lpSrcBits[3*nXOld1   + nYOld * uiDIBSourceScanLineSize];
					g2 = lpSrcBits[3*nXOld1+1 + nYOld * uiDIBSourceScanLineSize];
					r2 = lpSrcBits[3*nXOld1+2 + nYOld * uiDIBSourceScanLineSize];
					b3 = lpSrcBits[3*nXOld    + nYOld1 * uiDIBSourceScanLineSize];
					g3 = lpSrcBits[3*nXOld+1  + nYOld1 * uiDIBSourceScanLineSize];
					r3 = lpSrcBits[3*nXOld+2  + nYOld1 * uiDIBSourceScanLineSize];
					b4 = lpSrcBits[3*nXOld1   + nYOld1 * uiDIBSourceScanLineSize];
					g4 = lpSrcBits[3*nXOld1+1 + nYOld1 * uiDIBSourceScanLineSize];
					r4 = lpSrcBits[3*nXOld1+2 + nYOld1 * uiDIBSourceScanLineSize];

					// Interpolate in y direction:
					dr13 = r1	* (1.0 - dDeltaYOld) + r3	* dDeltaYOld;
					dg13 = g1	* (1.0 - dDeltaYOld) + g3	* dDeltaYOld;
					db13 = b1	* (1.0 - dDeltaYOld) + b3	* dDeltaYOld;
					dr24 = r2	* (1.0 - dDeltaYOld) + r4	* dDeltaYOld;
					dg24 = g2	* (1.0 - dDeltaYOld) + g4	* dDeltaYOld;
					db24 = b2	* (1.0 - dDeltaYOld) + b4	* dDeltaYOld;
					
					// Interpolate in x direction:
					r = Round(dr13 * (1.0 - dDeltaXOld) + dr24 * dDeltaXOld);
					g = Round(dg13 * (1.0 - dDeltaXOld) + dg24 * dDeltaXOld);
					b = Round(db13 * (1.0 - dDeltaXOld) + db24 * dDeltaXOld);
					
					// Set Output Pixel
					lpDstBits[3*x] = b;
					lpDstBits[3*x+1] = g;
					lpDstBits[3*x+2] = r;
				}
			}
			else
			{
				for (unsigned int x = 0 ; x < dwNewWidth ; x++)
				{
					// Map Destination Pixel Center Back
					dXOld = (x + 0.5) * dXScale;
					nXOld1 = Round(dXOld); // Center X coord.of the 4 source pixels
					nXOld = nXOld1 - 1;			
					dDeltaXOld = dXOld - nXOld1 + 0.5;

					// Clamp
					nXOld = MAX(0, nXOld);
					nXOld1 = MIN(nXOldMax, nXOld1);

					// Interpolate using the four nearest pixels in the source
					b1 = lpSrcBits[3*nXOld    + nYOld * uiDIBSourceScanLineSize];
					g1 = lpSrcBits[3*nXOld+1  + nYOld * uiDIBSourceScanLineSize];
					r1 = lpSrcBits[3*nXOld+2  + nYOld * uiDIBSourceScanLineSize];
					b2 = lpSrcBits[3*nXOld1   + nYOld * uiDIBSourceScanLineSize];
					g2 = lpSrcBits[3*nXOld1+1 + nYOld * uiDIBSourceScanLineSize];
					r2 = lpSrcBits[3*nXOld1+2 + nYOld * uiDIBSourceScanLineSize];

					// No Interpolate in y direction:
					dr13 = r1;
					dg13 = g1;
					db13 = b1;
					dr24 = r2;
					dg24 = g2;
					db24 = b2;
					
					// Interpolate in x direction:
					r = Round(dr13 * (1.0 - dDeltaXOld) + dr24 * dDeltaXOld);
					g = Round(dg13 * (1.0 - dDeltaXOld) + dg24 * dDeltaXOld);
					b = Round(db13 * (1.0 - dDeltaXOld) + db24 * dDeltaXOld);
					
					// Set Output Pixel
					lpDstBits[3*x] = b;
					lpDstBits[3*x+1] = g;
					lpDstBits[3*x+2] = r;
				}
			}
			lpDstBits += uiDIBTargetScanLineSize;
		}
	}
	// Optimized
	else if (!pSrcDib->HasAlpha() && pSrcDib->IsFast32bpp() && pSrcDib->GetBitCount() == 32)
	{
		LPBYTE lpSrcBits = pSrcDib->GetBits();
		LPBYTE lpDstBits = GetBits();

		for (unsigned int y = 0 ; y < dwNewHeight ; y++)
		{
			if (pThread && pThread->DoExit())
			{
				DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
				return FALSE;
			}
			DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, dwNewHeight);

			// Map Destination Pixel Center Back
			dYOld = (y + 0.5) * dYScale;
			nYOld1 = Round(dYOld); // Center Y coord. of the 4 source pixels
			nYOld = nYOld1 - 1;			
			dDeltaYOld = dYOld - nYOld1 + 0.5;

			// Clamp
			nYOld = MAX(0, nYOld);
			nYOld1 = MIN(nYOldMax, nYOld1);

			if (nYOld1 <= nYOldMax)
			{
				for (unsigned int x = 0 ; x < dwNewWidth ; x++)
				{
					// Map Destination Pixel Center Back
					dXOld = (x + 0.5) * dXScale;
					nXOld1 = Round(dXOld); // Center X coord.of the 4 source pixels
					nXOld = nXOld1 - 1;			
					dDeltaXOld = dXOld - nXOld1 + 0.5;

					// Clamp
					nXOld = MAX(0, nXOld);
					nXOld1 = MIN(nXOldMax, nXOld1);

					// Interpolate using the four nearest pixels in the source
					b1 = lpSrcBits[4*nXOld    + nYOld * uiDIBSourceScanLineSize];
					g1 = lpSrcBits[4*nXOld+1  + nYOld * uiDIBSourceScanLineSize];
					r1 = lpSrcBits[4*nXOld+2  + nYOld * uiDIBSourceScanLineSize];
					b2 = lpSrcBits[4*nXOld1   + nYOld * uiDIBSourceScanLineSize];
					g2 = lpSrcBits[4*nXOld1+1 + nYOld * uiDIBSourceScanLineSize];
					r2 = lpSrcBits[4*nXOld1+2 + nYOld * uiDIBSourceScanLineSize];
					b3 = lpSrcBits[4*nXOld    + nYOld1 * uiDIBSourceScanLineSize];
					g3 = lpSrcBits[4*nXOld+1  + nYOld1 * uiDIBSourceScanLineSize];
					r3 = lpSrcBits[4*nXOld+2  + nYOld1 * uiDIBSourceScanLineSize];
					b4 = lpSrcBits[4*nXOld1   + nYOld1 * uiDIBSourceScanLineSize];
					g4 = lpSrcBits[4*nXOld1+1 + nYOld1 * uiDIBSourceScanLineSize];
					r4 = lpSrcBits[4*nXOld1+2 + nYOld1 * uiDIBSourceScanLineSize];

					// Interpolate in y direction:
					dr13 = r1	* (1.0 - dDeltaYOld) + r3	* dDeltaYOld;
					dg13 = g1	* (1.0 - dDeltaYOld) + g3	* dDeltaYOld;
					db13 = b1	* (1.0 - dDeltaYOld) + b3	* dDeltaYOld;
					dr24 = r2	* (1.0 - dDeltaYOld) + r4	* dDeltaYOld;
					dg24 = g2	* (1.0 - dDeltaYOld) + g4	* dDeltaYOld;
					db24 = b2	* (1.0 - dDeltaYOld) + b4	* dDeltaYOld;

					// Interpolate in x direction:
					r = Round(dr13 * (1.0 - dDeltaXOld) + dr24 * dDeltaXOld);
					g = Round(dg13 * (1.0 - dDeltaXOld) + dg24 * dDeltaXOld);
					b = Round(db13 * (1.0 - dDeltaXOld) + db24 * dDeltaXOld);

					// Set Output Pixel
					lpDstBits[4*x] = b;
					lpDstBits[4*x+1] = g;
					lpDstBits[4*x+2] = r;
				}
			}
			else
			{
				for (unsigned int x = 0 ; x < dwNewWidth ; x++)
				{
					// Map Destination Pixel Center Back
					dXOld = (x + 0.5) * dXScale;
					nXOld1 = Round(dXOld); // Center X coord.of the 4 source pixels
					nXOld = nXOld1 - 1;			
					dDeltaXOld = dXOld - nXOld1 + 0.5;

					// Clamp
					nXOld = MAX(0, nXOld);
					nXOld1 = MIN(nXOldMax, nXOld1);

					// Interpolate using the four nearest pixels in the source
					b1 = lpSrcBits[4*nXOld    + nYOld * uiDIBSourceScanLineSize];
					g1 = lpSrcBits[4*nXOld+1  + nYOld * uiDIBSourceScanLineSize];
					r1 = lpSrcBits[4*nXOld+2  + nYOld * uiDIBSourceScanLineSize];
					b2 = lpSrcBits[4*nXOld1   + nYOld * uiDIBSourceScanLineSize];
					g2 = lpSrcBits[4*nXOld1+1 + nYOld * uiDIBSourceScanLineSize];
					r2 = lpSrcBits[4*nXOld1+2 + nYOld * uiDIBSourceScanLineSize];

					// No Interpolate in y direction:
					dr13 = r1;
					dg13 = g1;
					db13 = b1;
					dr24 = r2;
					dg24 = g2;
					db24 = b2;
					
					// Interpolate in x direction:
					r = Round(dr13 * (1.0 - dDeltaXOld) + dr24 * dDeltaXOld);
					g = Round(dg13 * (1.0 - dDeltaXOld) + dg24 * dDeltaXOld);
					b = Round(db13 * (1.0 - dDeltaXOld) + db24 * dDeltaXOld);

					// Set Output Pixel
					lpDstBits[4*x] = b;
					lpDstBits[4*x+1] = g;
					lpDstBits[4*x+2] = r;
				}
			}
			lpDstBits += uiDIBTargetScanLineSize;
		}
	}
	// Optimized
	else if (pSrcDib->HasAlpha() && pSrcDib->GetBitCount() == 32)
	{
		LPBYTE lpSrcBits = pSrcDib->GetBits();
		LPBYTE lpDstBits = GetBits();

		for (unsigned int y = 0 ; y < dwNewHeight ; y++)
		{
			if (pThread && pThread->DoExit())
			{
				DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
				return FALSE;
			}
			DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, dwNewHeight);

			// Map Destination Pixel Center Back
			dYOld = (y + 0.5) * dYScale;
			nYOld1 = Round(dYOld); // Center Y coord. of the 4 source pixels
			nYOld = nYOld1 - 1;			
			dDeltaYOld = dYOld - nYOld1 + 0.5;

			// Clamp
			nYOld = MAX(0, nYOld);
			nYOld1 = MIN(nYOldMax, nYOld1);

			if (nYOld1 <= nYOldMax)
			{
				for (unsigned int x = 0 ; x < dwNewWidth ; x++)
				{
					// Map Destination Pixel Center Back
					dXOld = (x + 0.5) * dXScale;
					nXOld1 = Round(dXOld); // Center X coord.of the 4 source pixels
					nXOld = nXOld1 - 1;			
					dDeltaXOld = dXOld - nXOld1 + 0.5;

					// Clamp
					nXOld = MAX(0, nXOld);
					nXOld1 = MIN(nXOldMax, nXOld1);

					// Interpolate using the four nearest pixels in the source
					b1 = lpSrcBits[4*nXOld    + nYOld * uiDIBSourceScanLineSize];
					g1 = lpSrcBits[4*nXOld+1  + nYOld * uiDIBSourceScanLineSize];
					r1 = lpSrcBits[4*nXOld+2  + nYOld * uiDIBSourceScanLineSize];
					a1 = lpSrcBits[4*nXOld+3  + nYOld * uiDIBSourceScanLineSize];
					b2 = lpSrcBits[4*nXOld1   + nYOld * uiDIBSourceScanLineSize];
					g2 = lpSrcBits[4*nXOld1+1 + nYOld * uiDIBSourceScanLineSize];
					r2 = lpSrcBits[4*nXOld1+2 + nYOld * uiDIBSourceScanLineSize];
					a2 = lpSrcBits[4*nXOld1+3 + nYOld * uiDIBSourceScanLineSize];
					b3 = lpSrcBits[4*nXOld    + nYOld1 * uiDIBSourceScanLineSize];
					g3 = lpSrcBits[4*nXOld+1  + nYOld1 * uiDIBSourceScanLineSize];
					r3 = lpSrcBits[4*nXOld+2  + nYOld1 * uiDIBSourceScanLineSize];
					a3 = lpSrcBits[4*nXOld+3  + nYOld1 * uiDIBSourceScanLineSize];
					b4 = lpSrcBits[4*nXOld1   + nYOld1 * uiDIBSourceScanLineSize];
					g4 = lpSrcBits[4*nXOld1+1 + nYOld1 * uiDIBSourceScanLineSize];
					r4 = lpSrcBits[4*nXOld1+2 + nYOld1 * uiDIBSourceScanLineSize];
					a4 = lpSrcBits[4*nXOld1+3 + nYOld1 * uiDIBSourceScanLineSize];

					// Interpolate in y direction:
					dr13 = r1	* (1.0 - dDeltaYOld) + r3	* dDeltaYOld;
					dg13 = g1	* (1.0 - dDeltaYOld) + g3	* dDeltaYOld;
					db13 = b1	* (1.0 - dDeltaYOld) + b3	* dDeltaYOld;
					da13 = a1	* (1.0 - dDeltaYOld) + a3	* dDeltaYOld;
					dr24 = r2	* (1.0 - dDeltaYOld) + r4	* dDeltaYOld;
					dg24 = g2	* (1.0 - dDeltaYOld) + g4	* dDeltaYOld;
					db24 = b2	* (1.0 - dDeltaYOld) + b4	* dDeltaYOld;
					da24 = a2	* (1.0 - dDeltaYOld) + a4	* dDeltaYOld;

					// Interpolate in x direction:
					r = Round(dr13 * (1.0 - dDeltaXOld) + dr24 * dDeltaXOld);
					g = Round(dg13 * (1.0 - dDeltaXOld) + dg24 * dDeltaXOld);
					b = Round(db13 * (1.0 - dDeltaXOld) + db24 * dDeltaXOld);
					a = Round(da13 * (1.0 - dDeltaXOld) + da24 * dDeltaXOld);

					// Set Output Pixel
					lpDstBits[4*x] = b;
					lpDstBits[4*x+1] = g;
					lpDstBits[4*x+2] = r;
					lpDstBits[4*x+3] = a;
				}
			}
			else
			{
				for (unsigned int x = 0 ; x < dwNewWidth ; x++)
				{
					// Map Destination Pixel Center Back
					dXOld = (x + 0.5) * dXScale;
					nXOld1 = Round(dXOld); // Center X coord.of the 4 source pixels
					nXOld = nXOld1 - 1;			
					dDeltaXOld = dXOld - nXOld1 + 0.5;

					// Clamp
					nXOld = MAX(0, nXOld);
					nXOld1 = MIN(nXOldMax, nXOld1);

					// Interpolate using the four nearest pixels in the source
					b1 = lpSrcBits[4*nXOld    + nYOld * uiDIBSourceScanLineSize];
					g1 = lpSrcBits[4*nXOld+1  + nYOld * uiDIBSourceScanLineSize];
					r1 = lpSrcBits[4*nXOld+2  + nYOld * uiDIBSourceScanLineSize];
					a1 = lpSrcBits[4*nXOld+3  + nYOld * uiDIBSourceScanLineSize];
					b2 = lpSrcBits[4*nXOld1   + nYOld * uiDIBSourceScanLineSize];
					g2 = lpSrcBits[4*nXOld1+1 + nYOld * uiDIBSourceScanLineSize];
					r2 = lpSrcBits[4*nXOld1+2 + nYOld * uiDIBSourceScanLineSize];
					a2 = lpSrcBits[4*nXOld1+3 + nYOld * uiDIBSourceScanLineSize];

					// No Interpolate in y direction:
					dr13 = r1;
					dg13 = g1;
					db13 = b1;
					da13 = a1;
					dr24 = r2;
					dg24 = g2;
					db24 = b2;
					da24 = a2;
					
					// Interpolate in x direction:
					r = Round(dr13 * (1.0 - dDeltaXOld) + dr24 * dDeltaXOld);
					g = Round(dg13 * (1.0 - dDeltaXOld) + dg24 * dDeltaXOld);
					b = Round(db13 * (1.0 - dDeltaXOld) + db24 * dDeltaXOld);
					a = Round(da13 * (1.0 - dDeltaXOld) + da24 * dDeltaXOld);

					// Set Output Pixel
					lpDstBits[4*x] = b;
					lpDstBits[4*x+1] = g;
					lpDstBits[4*x+2] = r;
					lpDstBits[4*x+3] = a;
				}
			}
			lpDstBits += uiDIBTargetScanLineSize;
		}
	}
	// Not Optimized
	else
	{
		for (unsigned int y = 0 ; y < dwNewHeight ; y++)
		{
			if (pThread && pThread->DoExit())
			{
				DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
				return FALSE;
			}
			DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, dwNewHeight);

			// Map Destination Pixel Center Back
			dYOld = (y + 0.5) * dYScale;
			nYOld1 = Round(dYOld); // Center Y coord. of the 4 source pixels
			nYOld = nYOld1 - 1;			
			dDeltaYOld = dYOld - nYOld1 + 0.5;

			for (unsigned int x = 0 ; x < dwNewWidth ; x++)
			{
				// Map Destination Pixel Center Back
				dXOld = (x + 0.5) * dXScale;
				nXOld1 = Round(dXOld); // Center X coord.of the 4 source pixels
				nXOld = nXOld1 - 1;			
				dDeltaXOld = dXOld - nXOld1 + 0.5;

				// Interpolate using the four nearest pixels in the source,
				// Clamp done in GetPixelColor()
				crColor1 = pSrcDib->GetPixelColor(nXOld,  nYOld);
				crColor2 = pSrcDib->GetPixelColor(nXOld1, nYOld);
				crColor3 = pSrcDib->GetPixelColor(nXOld,  nYOld1);
				crColor4 = pSrcDib->GetPixelColor(nXOld1, nYOld1);

				// Interpolate in y direction:
				dr13 = GetRValue(crColor1)	* (1.0 - dDeltaYOld) + GetRValue(crColor3)	* dDeltaYOld;
				dg13 = GetGValue(crColor1)	* (1.0 - dDeltaYOld) + GetGValue(crColor3)	* dDeltaYOld;
				db13 = GetBValue(crColor1)	* (1.0 - dDeltaYOld) + GetBValue(crColor3)	* dDeltaYOld;
				dr24 = GetRValue(crColor2)	* (1.0 - dDeltaYOld) + GetRValue(crColor4)	* dDeltaYOld;
				dg24 = GetGValue(crColor2)	* (1.0 - dDeltaYOld) + GetGValue(crColor4)	* dDeltaYOld;
				db24 = GetBValue(crColor2)	* (1.0 - dDeltaYOld) + GetBValue(crColor4)	* dDeltaYOld;
				
				// Interpolate in x direction:
				r = Round(dr13 * (1.0 - dDeltaXOld) + dr24 * dDeltaXOld);
				g = Round(dg13 * (1.0 - dDeltaXOld) + dg24 * dDeltaXOld);
				b = Round(db13 * (1.0 - dDeltaXOld) + db24 * dDeltaXOld);
				
				// Set Output Pixel
				SetPixelColor(x, y, RGB(r, g, b));
			}
		}
	}

	// Create Palette
	CreatePaletteFromBMI();

	DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

	return TRUE;
}

// Bicubic Interpolation
#define BICUBIC_PIXMARGIN	4
BOOL CDib::BicubicResampleBits(	DWORD dwNewWidth,
								DWORD dwNewHeight,
								CDib* pSrcDib/*=NULL*/,
								CWnd* pProgressWnd/*=NULL*/,
								BOOL bProgressSend/*=TRUE*/,
								CWorkerThread* pThread/*=NULL*/)
{
	// Check New Sizes
	if ((dwNewWidth <= 0) || (dwNewHeight <= 0))
		return FALSE;

	// Make a Copy of this?
	CDib SrcDib;
	BOOL bCopySrcToDst = FALSE;
	if (pSrcDib == NULL || this == pSrcDib)
	{
		// If Same Size Return
		if ((dwNewWidth == GetWidth()) && (dwNewHeight == GetHeight()))
			return TRUE;

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
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + m_pBMI->bmiHeader.biSize);
		else
			m_pColors = NULL;

		// Copy Src To Dst
		bCopySrcToDst = TRUE;
	}

	// Src Width & Height
	DWORD dwSrcWidth = pSrcDib->GetWidth();
	DWORD dwSrcHeight = pSrcDib->GetHeight();

	// Scan Line Alignments
	DWORD uiDIBSourceScanLineSize = DWALIGNEDWIDTHBYTES(dwSrcWidth * pSrcDib->GetBitCount());
	DWORD uiDIBTargetScanLineSize = DWALIGNEDWIDTHBYTES(dwNewWidth * GetBitCount());

	// Allocate Bits
	if (m_pBits == NULL)
	{
		// Allocate memory
		m_pBits = (LPBYTE)BIGALLOC(uiDIBTargetScanLineSize * dwNewHeight);
	}
	// Need to ReAllocate Bits because they are of different size
	else if (m_dwImageSize != uiDIBTargetScanLineSize * dwNewHeight)
	{
		BIGFREE(m_pBits);

		// Allocate memory
		m_pBits = (LPBYTE)BIGALLOC(uiDIBTargetScanLineSize * dwNewHeight);
	}
	if (m_pBits == NULL)
		return FALSE;

	// Copy Vars
	if (bCopySrcToDst)
		CopyVars(*pSrcDib);

	// Change BMI
	m_pBMI->bmiHeader.biWidth = dwNewWidth;
	m_pBMI->bmiHeader.biHeight = dwNewHeight;
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

	// If Same Size Copy
	if (bCopySrcToDst && (dwNewWidth == pSrcDib->GetWidth()) && (dwNewHeight == pSrcDib->GetHeight()))
	{
		memcpy((void*)m_pBits, (void*)pSrcDib->m_pBits, m_dwImageSize);
		return TRUE;
	}

	// Resample
	if (pSrcDib->GetBitCount() <= 8)
	{
		double xScale = (double)dwSrcWidth  / (double)dwNewWidth;
		double yScale = (double)dwSrcHeight / (double)dwNewHeight;
		double f_x, f_y, a, b, rr, gg, bb, r1, r2;
		int i_x, i_y, xx, yy;
		COLORREF crColor;

		DIB_INIT_PROGRESS;

		InitGetClosestColorIndex();
		for (unsigned int y = 0 ; y < dwNewHeight ; y++)
		{
			if (pThread && pThread->DoExit())
			{
				DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
				return FALSE;
			}
			DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, dwNewHeight);

			f_y = (y + 0.5) * yScale;
			i_y = Round(f_y); // Center Y coord. of the 16 source pixels
			a   = f_y - i_y + 0.5;

			for (unsigned int x = 0 ; x < dwNewWidth ; x++)
			{
				f_x = (x + 0.5) * xScale;
				i_x = Round(f_x); // Center X coord. of the 16 source pixels
				b   = f_x - i_x + 0.5;

				rr = gg = bb = 0.0;
				for (int m = -1 ; m < 3 ; m++)
				{
					r1 = KernelCubic_0_5((double)m - a);
					yy = i_y + m - 1; // Clamp done by GetPixelColor()
					for (int n = -1 ; n < 3 ; n++)
					{
						r2 = r1 * KernelCubic_0_5((double)n - b);
						xx = i_x + n - 1; // Clamp done by GetPixelColor()
						crColor = pSrcDib->GetPixelColor(xx, yy);
						rr += GetRValue(crColor) * r2;
						gg += GetGValue(crColor) * r2;
						bb += GetBValue(crColor) * r2;
					}
				}

				// Set Output Index
				if (rr < 0.0) rr = 0.0;
				else if (rr > 255.0) rr = 255.0;
				if (gg < 0.0) gg = 0.0;
				else if (gg > 255.0) gg = 255.0;
				if (bb < 0.0) bb = 0.0;
				else if (bb > 255.0) bb = 255.0;
				SetPixelIndex(x, y, GetClosestColorIndex(RGB(Round(rr), Round(gg), Round(bb))));
			}
		}

		DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
	}
	// Optimized 24 bpp
	else if (pSrcDib->GetBitCount() == 24)
	{
		float xScale = (float)dwSrcWidth  / (float)dwNewWidth;
		float yScale = (float)dwSrcHeight / (float)dwNewHeight;
		int i, x, y, l;
		const int BYTESPERPIX = 3;
		float* f;
		float f_B, f_G, f_R;

		// Allocate and init Temp Floating-Point Image: b g r a (alpha not used)
		int nFloatSrcScanLineSize = 4 * ((int)pSrcDib->GetWidth() + 2*BICUBIC_PIXMARGIN);
		f = (float*)BIGALLOC(nFloatSrcScanLineSize * ((int)pSrcDib->GetHeight() + 2*BICUBIC_PIXMARGIN) * sizeof(float));
		if (!f)
			return FALSE;
		LPBYTE p = pSrcDib->GetBits();
		i = 0;

		for (y = 0 ; y < BICUBIC_PIXMARGIN ; y++)
		{
			// Top-Left
			l = 0;
			f_B = (float)p[l++];	// Blue
			f_G = (float)p[l++];	// Green
			f_R = (float)p[l];		// Red
			for (x = 0 ; x < BICUBIC_PIXMARGIN ; x++)
			{
				f[i++] = f_B;
				f[i++] = f_G;
				f[i++] = f_R;
				f[i++] = 0.0f;
			}

			// Center
			l = 0;
			for (x = 0 ; x < (int)pSrcDib->GetWidth() ; x++)
			{
				f[i++] = (float)p[l++];	// Blue
				f[i++] = (float)p[l++];	// Green
				f[i++] = (float)p[l++];	// Red
				f[i++] = 0.0f;			// Alpha
			}

			// Top-Right
			l = BYTESPERPIX * ((int)pSrcDib->GetWidth() - 1);
			f_B = (float)p[l++];	// Blue
			f_G = (float)p[l++];	// Green
			f_R = (float)p[l];		// Red
			for (x = 0 ; x < BICUBIC_PIXMARGIN ; x++)
			{
				f[i++] = f_B;
				f[i++] = f_G;
				f[i++] = f_R;
				f[i++] = 0.0f;
			}
		}
		for (y = 0 ; y < (int)pSrcDib->GetHeight() ; y++)
		{
			// Left
			l = y * uiDIBSourceScanLineSize;
			f_B = (float)p[l++];	// Blue
			f_G = (float)p[l++];	// Green
			f_R = (float)p[l];		// Red
			for (x = 0 ; x < BICUBIC_PIXMARGIN ; x++)
			{
				f[i++] = f_B;
				f[i++] = f_G;
				f[i++] = f_R;
				f[i++] = 0.0f;
			}

			// Center
			l = y * uiDIBSourceScanLineSize;
			for (x = 0 ; x < (int)pSrcDib->GetWidth() ; x++)
			{
				f[i++] = (float)p[l++];	// Blue
				f[i++] = (float)p[l++];	// Green
				f[i++] = (float)p[l++];	// Red
				f[i++] = 0.0f;			// Alpha
			}

			// Right
			l = y * uiDIBSourceScanLineSize + BYTESPERPIX*(x - 1);
			f_B = (float)p[l++];	// Blue
			f_G = (float)p[l++];	// Green
			f_R = (float)p[l];		// Red
			for (x = 0 ; x < BICUBIC_PIXMARGIN ; x++)
			{
				f[i++] = f_B;
				f[i++] = f_G;
				f[i++] = f_R;
				f[i++] = 0.0f;
			}
		}
		for (y = 0 ; y < BICUBIC_PIXMARGIN ; y++)
		{
			// Bottom-Left
			l = ((int)pSrcDib->GetHeight() - 1) * uiDIBSourceScanLineSize;
			f_B = (float)p[l++];	// Blue
			f_G = (float)p[l++];	// Green
			f_R = (float)p[l];		// Red
			for (x = 0 ; x < BICUBIC_PIXMARGIN ; x++)
			{
				f[i++] = f_B;
				f[i++] = f_G;
				f[i++] = f_R;
				f[i++] = 0.0f;
			}

			// Center
			l = ((int)pSrcDib->GetHeight() - 1) * uiDIBSourceScanLineSize;
			for (x = 0 ; x < (int)pSrcDib->GetWidth() ; x++)
			{
				
				f[i++] = (float)p[l++];	// Blue
				f[i++] = (float)p[l++];	// Green
				f[i++] = (float)p[l++];	// Red
				f[i++] = 0.0f;			// Alpha
			}

			// Bottom-Right
			l = ((int)pSrcDib->GetHeight() - 1) * uiDIBSourceScanLineSize + BYTESPERPIX*(x - 1);
			f_B = (float)p[l++];	// Blue
			f_G = (float)p[l++];	// Green
			f_R = (float)p[l];		// Red
			for (x = 0 ; x < BICUBIC_PIXMARGIN ; x++)
			{
				f[i++] = f_B;
				f[i++] = f_G;
				f[i++] = f_R;
				f[i++] = 0.0f;
			}
		}

		// Interpolate
		if (!BicubicResample24(	(int)dwNewWidth,
								(int)dwNewHeight,
								xScale,
								yScale,
								nFloatSrcScanLineSize,
								f,
								GetBits(),
								pProgressWnd,
								bProgressSend,
								pThread))
		{
			BIGFREE(f);
			return FALSE;
		}
		else
			BIGFREE(f);
	}
	// Optimized 32 bpp
	else if ((pSrcDib->HasAlpha() || pSrcDib->IsFast32bpp()) && pSrcDib->GetBitCount() == 32)
	{
		float xScale = (float)dwSrcWidth  / (float)dwNewWidth;
		float yScale = (float)dwSrcHeight / (float)dwNewHeight;
		int i, x, y, l;
		const int BYTESPERPIX = 4;
		float* f;
		float f_B, f_G, f_R, f_A;

		// Allocate and init Temp Floating-Point Image: b g r a
		int nFloatSrcScanLineSize = 4 * ((int)pSrcDib->GetWidth() + 2*BICUBIC_PIXMARGIN);
		f = (float*)BIGALLOC(nFloatSrcScanLineSize * ((int)pSrcDib->GetHeight() + 2*BICUBIC_PIXMARGIN) * sizeof(float));
		if (!f)
			return FALSE;
		LPBYTE p = pSrcDib->GetBits();
		i = 0;

		for (y = 0 ; y < BICUBIC_PIXMARGIN ; y++)
		{
			// Top-Left
			l = 0;
			f_B = (float)p[l++];	// Blue
			f_G = (float)p[l++];	// Green
			f_R = (float)p[l++];	// Red
			f_A = (float)p[l];		// Alpha
			for (x = 0 ; x < BICUBIC_PIXMARGIN ; x++)
			{
				f[i++] = f_B;
				f[i++] = f_G;
				f[i++] = f_R;
				f[i++] = f_A;
			}

			// Center
			l = 0;
			for (x = 0 ; x < (int)pSrcDib->GetWidth() ; x++)
			{
				f[i++] = (float)p[l++];	// Blue
				f[i++] = (float)p[l++];	// Green
				f[i++] = (float)p[l++];	// Red
				f[i++] = (float)p[l++];	// Alpha
			}

			// Top-Right
			l = BYTESPERPIX * ((int)pSrcDib->GetWidth() - 1);
			f_B = (float)p[l++];	// Blue
			f_G = (float)p[l++];	// Green
			f_R = (float)p[l++];	// Red
			f_A = (float)p[l];		// Alpha
			for (x = 0 ; x < BICUBIC_PIXMARGIN ; x++)
			{
				f[i++] = f_B;
				f[i++] = f_G;
				f[i++] = f_R;
				f[i++] = f_A;
			}
		}
		for (y = 0 ; y < (int)pSrcDib->GetHeight() ; y++)
		{
			// Left
			l = y * uiDIBSourceScanLineSize;
			f_B = (float)p[l++];	// Blue
			f_G = (float)p[l++];	// Green
			f_R = (float)p[l++];	// Red
			f_A = (float)p[l];		// Alpha
			for (x = 0 ; x < BICUBIC_PIXMARGIN ; x++)
			{
				f[i++] = f_B;
				f[i++] = f_G;
				f[i++] = f_R;
				f[i++] = f_A;
			}

			// Center
			l = y * uiDIBSourceScanLineSize;
			for (x = 0 ; x < (int)pSrcDib->GetWidth() ; x++)
			{
				f[i++] = (float)p[l++];	// Blue
				f[i++] = (float)p[l++];	// Green
				f[i++] = (float)p[l++];	// Red
				f[i++] = (float)p[l++];	// Alpha
			}

			// Right
			l = y * uiDIBSourceScanLineSize + BYTESPERPIX*(x - 1);
			f_B = (float)p[l++];	// Blue
			f_G = (float)p[l++];	// Green
			f_R = (float)p[l++];	// Red
			f_A = (float)p[l];		// Alpha
			for (x = 0 ; x < BICUBIC_PIXMARGIN ; x++)
			{
				f[i++] = f_B;
				f[i++] = f_G;
				f[i++] = f_R;
				f[i++] = f_A;
			}
		}
		for (y = 0 ; y < BICUBIC_PIXMARGIN ; y++)
		{
			// Bottom-Left
			l = ((int)pSrcDib->GetHeight() - 1) * uiDIBSourceScanLineSize;
			f_B = (float)p[l++];	// Blue
			f_G = (float)p[l++];	// Green
			f_R = (float)p[l++];	// Red
			f_A = (float)p[l];		// Alpha
			for (x = 0 ; x < BICUBIC_PIXMARGIN ; x++)
			{
				f[i++] = f_B;
				f[i++] = f_G;
				f[i++] = f_R;
				f[i++] = f_A;
			}

			// Center
			l = ((int)pSrcDib->GetHeight() - 1) * uiDIBSourceScanLineSize;
			for (x = 0 ; x < (int)pSrcDib->GetWidth() ; x++)
			{
				f[i++] = (float)p[l++];	// Blue
				f[i++] = (float)p[l++];	// Green
				f[i++] = (float)p[l++];	// Red
				f[i++] = (float)p[l++];	// Alpha
			}

			// Bottom-Right
			l = ((int)pSrcDib->GetHeight() - 1) * uiDIBSourceScanLineSize + BYTESPERPIX*(x - 1);
			f_B = (float)p[l++];	// Blue
			f_G = (float)p[l++];	// Green
			f_R = (float)p[l++];	// Red
			f_A = (float)p[l];		// Alpha
			for (x = 0 ; x < BICUBIC_PIXMARGIN ; x++)
			{
				f[i++] = f_B;
				f[i++] = f_G;
				f[i++] = f_R;
				f[i++] = f_A;
			}
		}

		// Interpolate
		if (!BicubicResample32(	(int)dwNewWidth,
								(int)dwNewHeight,
								xScale,
								yScale,
								nFloatSrcScanLineSize,
								f,
								(LPDWORD)GetBits(),
								pProgressWnd,
								bProgressSend,
								pThread))
		{
			BIGFREE(f);
			return FALSE;
		}
		else
			BIGFREE(f);
	}
	// Not Optimized
	else
	{
		double xScale = (double)dwSrcWidth  / (double)dwNewWidth;
		double yScale = (double)dwSrcHeight / (double)dwNewHeight;
		double f_x, f_y, a, b, rr, gg, bb, r1, r2;
		int i_x, i_y, xx, yy;
		COLORREF crColor;

		DIB_INIT_PROGRESS;

		for (unsigned int y = 0 ; y < dwNewHeight ; y++)
		{
			if (pThread && pThread->DoExit())
			{
				DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
				return FALSE;
			}
			DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, dwNewHeight);

			f_y = (y + 0.5) * yScale;
			i_y = Round(f_y); // Center Y coord. of the 16 source pixels
			a   = f_y - i_y + 0.5;

			for (unsigned int x = 0 ; x < dwNewWidth ; x++)
			{
				f_x = (x + 0.5) * xScale;
				i_x = Round(f_x); // Center X coord. of the 16 source pixels
				b   = f_x - i_x + 0.5;

				rr = gg = bb = 0.0;
				for (int m = -1 ; m < 3 ; m++)
				{
					r1 = KernelCubic_0_5((double)m - a);
					yy = i_y + m - 1; // Clamp done by GetPixelColor()
					for (int n = -1 ; n < 3 ; n++)
					{
						r2 = r1 * KernelCubic_0_5((double)n - b);
						xx = i_x + n - 1; // Clamp done by GetPixelColor()
						crColor = pSrcDib->GetPixelColor(xx, yy);
						rr += (GetRValue(crColor) * r2);
						gg += (GetGValue(crColor) * r2);
						bb += (GetBValue(crColor) * r2);
					}
				}

				// Set Output Pixel
				if (rr < 0.0) rr = 0.0;
				else if (rr > 255.0) rr = 255.0;
				if (gg < 0.0) gg = 0.0;
				else if (gg > 255.0) gg = 255.0;
				if (bb < 0.0) bb = 0.0;
				else if (bb > 255.0) bb = 255.0;
				SetPixelColor(x, y, RGB(Round(rr), Round(gg), Round(bb)));
			}
		}

		DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
	}

	// Create Palette
	CreatePaletteFromBMI();

	return TRUE;
}

BOOL CDib::BicubicResample24(	int nNewWidth,
								int nNewHeight,
								float xScale,
								float yScale,
								int nFloatSrcScanLineSize,
								float* f,
								LPBYTE pOutBits,
								CWnd* pProgressWnd/*=NULL*/,
								BOOL bProgressSend/*=TRUE*/,
								CWorkerThread* pThread/*=NULL*/)
{	
	int i, x, y, i_x, i_y, xx, yy;
	int nDIBTargetScanLineSize = DWALIGNEDWIDTHBYTES(nNewWidth * 24);
	float f_x, f_y, a, b, r1_1, r1_2, r1_3, r1_4, r2_1, r2_2, r2_3, r2_4, bb, gg, rr, r1, r2, r3, r4;
	float* kernel_b;
	float* array_kernel_b = (float*)BIGALLOC(nNewWidth * 4 * sizeof(float));
	if (!array_kernel_b)
		return FALSE;

	for (x = 0 , kernel_b = array_kernel_b ; x < nNewWidth ; x++ , kernel_b += 4)
	{
		f_x = (x + 0.5f) * xScale;
		i_x = RoundF(f_x); // Center X coord. of the 16 source pixels
		b   = f_x - (float)i_x + 0.5f;
		kernel_b[0] = KernelCubic_0_5(-1.0f - b);	// r2_1
		kernel_b[1] = KernelCubic_0_5(-b);			// r2_2
		kernel_b[2] = KernelCubic_0_5(1.0f - b);	// r2_3
		kernel_b[3] = KernelCubic_0_5(2.0f - b);	// r2_4
	}

	DIB_INIT_PROGRESS;

	for (y = 0 ; y < nNewHeight ; y++)
	{
		if (pThread && pThread->DoExit())
		{
			BIGFREE(array_kernel_b);
			DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
			return FALSE;
		}
		DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, nNewHeight);

		f_y = (y + 0.5f) * yScale;
		i_y = RoundF(f_y); // Center Y coord. of the 16 source pixels
		a = f_y - (float)i_y + 0.5f;
		r1_1 = KernelCubic_0_5(-1.0f - a);
		r1_2 = KernelCubic_0_5(-a);
		r1_3 = KernelCubic_0_5(1.0f - a);
		r1_4 = KernelCubic_0_5(2.0f - a);

		for (x = 0 , kernel_b = array_kernel_b ; x < nNewWidth ; x++ , kernel_b += 4)
		{
			bb = 0.0f;
			gg = 0.0f;
			rr = 0.0f;
			f_x = (x + 0.5f) * xScale;
			i_x = RoundF(f_x); // Center X coord. of the 16 source pixels
			
			xx = i_x - 2;
			yy = i_y - 2;
			yy = (yy + BICUBIC_PIXMARGIN) * nFloatSrcScanLineSize;
			r2_1 = kernel_b[0];
			r2_2 = kernel_b[1];
			r2_3 = kernel_b[2];
			r2_4 = kernel_b[3];
			
			// m = -1
			i = yy + ((xx + BICUBIC_PIXMARGIN) << 2);

			// n = -1
			r1 = r1_1 * r2_1;
			bb += (f[i++] * r1);
			gg += (f[i++] * r1);
			rr += (f[i++] * r1);
			i++;

			// n = 0
			r2 = r1_1 * r2_2;
			bb += (f[i++] * r2);
			gg += (f[i++] * r2);
			rr += (f[i++] * r2);
			i++;

			// n = 1
			r3 = r1_1 * r2_3;
			bb += (f[i++] * r3);
			gg += (f[i++] * r3);
			rr += (f[i++] * r3);
			i++;

			// n = 2
			r4 = r1_1 * r2_4;
			bb += (f[i++] * r4);
			gg += (f[i++] * r4);
			rr += (f[i] * r4);

			// m = 0
			i += (nFloatSrcScanLineSize - 14);
	
			// n = -1
			r1 = r1_2 * r2_1;
			bb += (f[i++] * r1);
			gg += (f[i++] * r1);
			rr += (f[i++] * r1);
			i++;

			// n = 0
			r2 = r1_2 * r2_2;
			bb += (f[i++] * r2);
			gg += (f[i++] * r2);
			rr += (f[i++] * r2);
			i++;

			// n = 1
			r3 = r1_2 * r2_3;
			bb += (f[i++] * r3);
			gg += (f[i++] * r3);
			rr += (f[i++] * r3);
			i++;

			// n = 2
			r4 = r1_2 * r2_4;
			bb += (f[i++] * r4);
			gg += (f[i++] * r4);
			rr += (f[i] * r4);

			// m = 1
			i += (nFloatSrcScanLineSize - 14);

			// n = -1
			r1 = r1_3 * r2_1;
			bb += (f[i++] * r1);
			gg += (f[i++] * r1);
			rr += (f[i++] * r1);
			i++;

			// n = 0
			r2 = r1_3 * r2_2;
			bb += (f[i++] * r2);
			gg += (f[i++] * r2);
			rr += (f[i++] * r2);
			i++;

			// n = 1
			r3 = r1_3 * r2_3;
			bb += (f[i++] * r3);
			gg += (f[i++] * r3);
			rr += (f[i++] * r3);
			i++;

			// n = 2
			r4 = r1_3 * r2_4;
			bb += (f[i++] * r4);
			gg += (f[i++] * r4);
			rr += (f[i] * r4);

			// m = 2
			i += (nFloatSrcScanLineSize - 14);

			// n = -1
			r1 = r1_4 * r2_1;
			bb += (f[i++] * r1);
			gg += (f[i++] * r1);
			rr += (f[i++] * r1);
			i++;

			// n = 0
			r2 = r1_4 * r2_2;
			bb += (f[i++] * r2);
			gg += (f[i++] * r2);
			rr += (f[i++] * r2);
			i++;

			// n = 1
			r3 = r1_4 * r2_3;
			bb += (f[i++] * r3);
			gg += (f[i++] * r3);
			rr += (f[i++] * r3);
			i++;

			// n = 2
			r4 = r1_4 * r2_4;
			bb += (f[i++] * r4);
			gg += (f[i++] * r4);
			rr += (f[i] * r4);

			// Set Output Pixel
			if (rr < 0.0f) rr = 0.0f;
			else if (rr > 255.0f) rr = 255.0f;
			if (gg < 0.0f) gg = 0.0f;
			else if (gg > 255.0f) gg = 255.0f;
			if (bb < 0.0f) bb = 0.0f;
			else if (bb > 255.0f) bb = 255.0f;
			pOutBits[3*x] = (BYTE)(RoundF(bb) & 0xFF);		// Blue
			pOutBits[3*x+1] = (BYTE)(RoundF(gg) & 0xFF);	// Green
			pOutBits[3*x+2] = (BYTE)(RoundF(rr) & 0xFF);	// Red
		}

		// Inc. Output Pixel Offset
		pOutBits += nDIBTargetScanLineSize;
	}

	BIGFREE(array_kernel_b);
	DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

	return TRUE;
}

BOOL CDib::BicubicResample32(	int nNewWidth,
								int nNewHeight,
								float xScale,
								float yScale,
								int nFloatSrcScanLineSize,
								float* f,
								LPDWORD pOutBits,
								CWnd* pProgressWnd/*=NULL*/,
								BOOL bProgressSend/*=TRUE*/,
								CWorkerThread* pThread/*=NULL*/)
{	
	int i, x, y, i_x, i_y, xx, yy;
	float f_x, f_y, a, b, r1_1, r1_2, r1_3, r1_4, r2_1, r2_2, r2_3, r2_4, bb, gg, rr, aa, r1, r2, r3, r4;
	float* kernel_b;
	float* array_kernel_b = (float*)BIGALLOC(nNewWidth * 4 * sizeof(float));
	if (!array_kernel_b)
		return FALSE;

	for (x = 0 , kernel_b = array_kernel_b ; x < nNewWidth ; x++ , kernel_b += 4)
	{
		f_x = (x + 0.5f) * xScale;
		i_x = RoundF(f_x); // Center X coord. of the 16 source pixels
		b   = f_x - (float)i_x + 0.5f;
		kernel_b[0] = KernelCubic_0_5(-1.0f - b);	// r2_1
		kernel_b[1] = KernelCubic_0_5(-b);			// r2_2
		kernel_b[2] = KernelCubic_0_5(1.0f - b);	// r2_3
		kernel_b[3] = KernelCubic_0_5(2.0f - b);	// r2_4
	}

	DIB_INIT_PROGRESS;

	for (y = 0 ; y < nNewHeight ; y++)
	{
		if (pThread && pThread->DoExit())
		{
			BIGFREE(array_kernel_b);
			DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
			return FALSE;
		}
		DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, nNewHeight);

		f_y = (y + 0.5f) * yScale;
		i_y = RoundF(f_y); // Center Y coord. of the 16 source pixels
		a = f_y - (float)i_y + 0.5f;
		r1_1 = KernelCubic_0_5(-1.0f - a);
		r1_2 = KernelCubic_0_5(-a);
		r1_3 = KernelCubic_0_5(1.0f - a);
		r1_4 = KernelCubic_0_5(2.0f - a);

		for (x = 0 , kernel_b = array_kernel_b ; x < nNewWidth ; x++ , kernel_b += 4)
		{
			bb = 0.0f;
			gg = 0.0f;
			rr = 0.0f;
			aa = 0.0f;
			f_x = (x + 0.5f) * xScale;
			i_x = RoundF(f_x); // Center X coord. of the 16 source pixels
			
			xx = i_x - 2;
			yy = i_y - 2;
			yy = (yy + BICUBIC_PIXMARGIN) * nFloatSrcScanLineSize;
			r2_1 = kernel_b[0];
			r2_2 = kernel_b[1];
			r2_3 = kernel_b[2];
			r2_4 = kernel_b[3];
			
			// m = -1
			i = yy + ((xx + BICUBIC_PIXMARGIN) << 2);

			// n = -1
			r1 = r1_1 * r2_1;
			bb += (f[i++] * r1);
			gg += (f[i++] * r1);
			rr += (f[i++] * r1);
			aa += (f[i++] * r1);

			// n = 0
			r2 = r1_1 * r2_2;
			bb += (f[i++] * r2);
			gg += (f[i++] * r2);
			rr += (f[i++] * r2);
			aa += (f[i++] * r2);

			// n = 1
			r3 = r1_1 * r2_3;
			bb += (f[i++] * r3);
			gg += (f[i++] * r3);
			rr += (f[i++] * r3);
			aa += (f[i++] * r3);

			// n = 2
			r4 = r1_1 * r2_4;
			bb += (f[i++] * r4);
			gg += (f[i++] * r4);
			rr += (f[i++] * r4);
			aa += (f[i] * r4);

			// m = 0
			i += (nFloatSrcScanLineSize - 15);
	
			// n = -1
			r1 = r1_2 * r2_1;
			bb += (f[i++] * r1);
			gg += (f[i++] * r1);
			rr += (f[i++] * r1);
			aa += (f[i++] * r1);

			// n = 0
			r2 = r1_2 * r2_2;
			bb += (f[i++] * r2);
			gg += (f[i++] * r2);
			rr += (f[i++] * r2);
			aa += (f[i++] * r2);

			// n = 1
			r3 = r1_2 * r2_3;
			bb += (f[i++] * r3);
			gg += (f[i++] * r3);
			rr += (f[i++] * r3);
			aa += (f[i++] * r3);

			// n = 2
			r4 = r1_2 * r2_4;
			bb += (f[i++] * r4);
			gg += (f[i++] * r4);
			rr += (f[i++] * r4);
			aa += (f[i] * r4);

			// m = 1
			i += (nFloatSrcScanLineSize - 15);

			// n = -1
			r1 = r1_3 * r2_1;
			bb += (f[i++] * r1);
			gg += (f[i++] * r1);
			rr += (f[i++] * r1);
			aa += (f[i++] * r1);

			// n = 0
			r2 = r1_3 * r2_2;
			bb += (f[i++] * r2);
			gg += (f[i++] * r2);
			rr += (f[i++] * r2);
			aa += (f[i++] * r2);

			// n = 1
			r3 = r1_3 * r2_3;
			bb += (f[i++] * r3);
			gg += (f[i++] * r3);
			rr += (f[i++] * r3);
			aa += (f[i++] * r3);

			// n = 2
			r4 = r1_3 * r2_4;
			bb += (f[i++] * r4);
			gg += (f[i++] * r4);
			rr += (f[i++] * r4);
			aa += (f[i] * r4);

			// m = 2
			i += (nFloatSrcScanLineSize - 15);

			// n = -1
			r1 = r1_4 * r2_1;
			bb += (f[i++] * r1);
			gg += (f[i++] * r1);
			rr += (f[i++] * r1);
			aa += (f[i++] * r1);

			// n = 0
			r2 = r1_4 * r2_2;
			bb += (f[i++] * r2);
			gg += (f[i++] * r2);
			rr += (f[i++] * r2);
			aa += (f[i++] * r2);

			// n = 1
			r3 = r1_4 * r2_3;
			bb += (f[i++] * r3);
			gg += (f[i++] * r3);
			rr += (f[i++] * r3);
			aa += (f[i++] * r3);

			// n = 2
			r4 = r1_4 * r2_4;
			bb += (f[i++] * r4);
			gg += (f[i++] * r4);
			rr += (f[i++] * r4);
			aa += (f[i] * r4);

			// Set Output Pixel
			if (rr < 0.0f) rr = 0.0f;
			else if (rr > 255.0f) rr = 255.0f;
			if (gg < 0.0f) gg = 0.0f;
			else if (gg > 255.0f) gg = 255.0f;
			if (bb < 0.0f) bb = 0.0f;
			else if (bb > 255.0f) bb = 255.0f;
			if (aa < 0.0f) aa = 0.0f;
			else if (aa > 255.0f) aa = 255.0f;
			pOutBits[x] = (DWORD)(	RoundF(bb)			|
									(RoundF(gg) << 8)	|
									(RoundF(rr) << 16)	|
									(RoundF(aa) << 24));
		}

		// Inc. Output Pixel Offset
		pOutBits += nNewWidth;
	}

	BIGFREE(array_kernel_b);
	DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

	return TRUE;
}
