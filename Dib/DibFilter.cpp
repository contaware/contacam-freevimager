#include "stdafx.h"
#include "dib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BOOL CDib::Filter(	int* pKernel,
					int nKernelSize,
					int nKernelFactor,
					int nKernelOffset/*=0*/,
					CDib* pSrcDib/*=NULL*/,
					CWnd* pProgressWnd/*=NULL*/,
					BOOL bProgressSend/*=TRUE*/,
					CWorkerThread* pThread/*=NULL*/)
{
	// Check Pointer
	if (!pKernel)
		return FALSE;

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
		// Need to ReAllocate BMI because they are of differente size
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

	// Scan Line Alignments
	DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES(pSrcDib->GetWidth() * pSrcDib->GetBitCount());

	// Allocate Bits
	if (m_pBits == NULL)
	{
		// Allocate memory
		m_pBits = (LPBYTE)BIGALLOC(uiDIBScanLineSize * pSrcDib->GetHeight());
	}
	// Need to ReAllocate Bits because they are of different size
	else if (m_dwImageSize != uiDIBScanLineSize * pSrcDib->GetHeight())
	{
		BIGFREE(m_pBits);

		// Allocate memory
		m_pBits = (LPBYTE)BIGALLOC(uiDIBScanLineSize * pSrcDib->GetHeight());
	}
	if (m_pBits == NULL)
		return FALSE;

	// Copy Vars
	if (bCopySrcToDst)
		CopyVars(*pSrcDib);

	// Change BMI
	m_pBMI->bmiHeader.biWidth = pSrcDib->GetWidth();
	m_pBMI->bmiHeader.biHeight = pSrcDib->GetHeight();
	m_dwImageSize = m_pBMI->bmiHeader.biSizeImage = uiDIBScanLineSize * pSrcDib->GetHeight();

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

	// Filter
	int k2 = nKernelSize / 2;
	int kmax = nKernelSize - k2;
	int r, g, b, a;
	COLORREF crColor;
	if (pSrcDib->GetBitCount() <= 8)
	{
		InitGetClosestColorIndex();

		if (nKernelFactor != 0)
		{
			for (unsigned int y = 0 ; y < pSrcDib->GetHeight() ; y++)
			{
				if ((y & 0x7) == 0)
				{
					if (pThread && pThread->DoExit())
					{
						DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
						return FALSE;
					}
					DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, pSrcDib->GetHeight());
				}

				for (unsigned int x = 0 ; x < pSrcDib->GetWidth() ; x++)
				{
					r = b = g = 0;
					for (int j = -k2 ; j < kmax ; j++)
					{
						for (int k = -k2 ; k < kmax ; k++)
						{
							crColor = pSrcDib->GetPixelColor(x + j, y + k);
							int i = pKernel[(j+k2) + nKernelSize * (k+k2)];
							r += GetRValue(crColor) * i;
							g += GetGValue(crColor) * i;
							b += GetBValue(crColor) * i;
						}
					}

					r = MIN(255, MAX(0, (int)(r / nKernelFactor + nKernelOffset)));
					g = MIN(255, MAX(0, (int)(g / nKernelFactor + nKernelOffset)));
					b = MIN(255, MAX(0, (int)(b / nKernelFactor + nKernelOffset)));
					SetPixelIndex(x, y, GetClosestColorIndex(RGB(r, g, b)));
				}
			}
		}
		else
		{
			for (unsigned int y = 0 ; y < pSrcDib->GetHeight() ; y++)
			{
				if ((y & 0x7) == 0)
				{
					if (pThread && pThread->DoExit())
					{
						DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
						return FALSE;
					}
					DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, pSrcDib->GetHeight());
				}

				for (unsigned int x = 0 ; x < pSrcDib->GetWidth() ; x++)
				{
					r = b = g = 0;
					for (int j = -k2 ; j < kmax ; j++)
					{
						for (int k = -k2 ; k < kmax ; k++)
						{
							crColor = pSrcDib->GetPixelColor(x + j, y + k);
							int i = pKernel[(j+k2) + nKernelSize * (k+k2)];
							r += GetRValue(crColor) * i;
							g += GetGValue(crColor) * i;
							b += GetBValue(crColor) * i;
						}
					}

					r = MIN(255, MAX(0, (int)(r + nKernelOffset)));
					g = MIN(255, MAX(0, (int)(g + nKernelOffset)));
					b = MIN(255, MAX(0, (int)(b + nKernelOffset)));
					SetPixelIndex(x, y, GetClosestColorIndex(RGB(r, g, b)));
				}
			}
		}
	}
	else if (pSrcDib->HasAlpha() && pSrcDib->GetBitCount() == 32)
	{
		if (nKernelFactor != 0)
		{
			for (unsigned int y = 0 ; y < pSrcDib->GetHeight() ; y++)
			{
				if ((y & 0x7) == 0)
				{
					if (pThread && pThread->DoExit())
					{
						DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
						return FALSE;
					}
					DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, pSrcDib->GetHeight());
				}

				for (unsigned int x = 0 ; x < pSrcDib->GetWidth() ; x++)
				{
					r = b = g = a = 0;
					for (int j = -k2 ; j < kmax ; j++)
					{
						for (int k = -k2 ; k < kmax ; k++)
						{
							crColor = pSrcDib->GetPixelColor32Alpha(x + j, y + k);
							int i = pKernel[(j+k2) + nKernelSize * (k+k2)];
							r += GetRValue(crColor) * i;
							g += GetGValue(crColor) * i;
							b += GetBValue(crColor) * i;
							a += GetAValue(crColor) * i;
						}
					}

					r = MIN(255, MAX(0, (int)(r / nKernelFactor + nKernelOffset)));
					g = MIN(255, MAX(0, (int)(g / nKernelFactor + nKernelOffset)));
					b = MIN(255, MAX(0, (int)(b / nKernelFactor + nKernelOffset)));
					a = MIN(255, MAX(0, (int)(a / nKernelFactor + nKernelOffset)));
					SetPixelColor32Alpha(x, y, RGBA(r, g, b, a));
				}
			}
		}
		else
		{
			for (unsigned int y = 0 ; y < pSrcDib->GetHeight() ; y++)
			{
				if ((y & 0x7) == 0)
				{
					if (pThread && pThread->DoExit())
					{
						DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
						return FALSE;
					}
					DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, pSrcDib->GetHeight());
				}

				for (unsigned int x = 0 ; x < pSrcDib->GetWidth() ; x++)
				{
					r = b = g = a = 0;
					for (int j = -k2 ; j < kmax ; j++)
					{
						for (int k = -k2 ; k < kmax ; k++)
						{
							crColor = pSrcDib->GetPixelColor32Alpha(x + j, y + k);
							int i = pKernel[(j+k2) + nKernelSize * (k+k2)];
							r += GetRValue(crColor) * i;
							g += GetGValue(crColor) * i;
							b += GetBValue(crColor) * i;
							a += GetAValue(crColor) * i;
						}
					}

					r = MIN(255, MAX(0, (int)(r + nKernelOffset)));
					g = MIN(255, MAX(0, (int)(g + nKernelOffset)));
					b = MIN(255, MAX(0, (int)(b + nKernelOffset)));
					a = MIN(255, MAX(0, (int)(a + nKernelOffset)));
					SetPixelColor32Alpha(x, y, RGBA(r, g, b, a));
				}
			}
		}
	}
	else
	{
		if (nKernelFactor != 0)
		{
			for (unsigned int y = 0 ; y < pSrcDib->GetHeight() ; y++)
			{
				if ((y & 0x7) == 0)
				{
					if (pThread && pThread->DoExit())
					{
						DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
						return FALSE;
					}
					DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, pSrcDib->GetHeight());
				}

				for (unsigned int x = 0 ; x < pSrcDib->GetWidth() ; x++)
				{
					r = b = g = 0;
					for (int j = -k2 ; j < kmax ; j++)
					{
						for (int k = -k2 ; k < kmax ; k++)
						{
							crColor = pSrcDib->GetPixelColor(x + j, y + k);
							int i = pKernel[(j+k2) + nKernelSize * (k+k2)];
							r += GetRValue(crColor) * i;
							g += GetGValue(crColor) * i;
							b += GetBValue(crColor) * i;
						}
					}

					r = MIN(255, MAX(0, (int)(r / nKernelFactor + nKernelOffset)));
					g = MIN(255, MAX(0, (int)(g / nKernelFactor + nKernelOffset)));
					b = MIN(255, MAX(0, (int)(b / nKernelFactor + nKernelOffset)));
					SetPixelColor(x, y, RGB(r, g, b));
				}
			}
		}
		else
		{
			for (unsigned int y = 0 ; y < pSrcDib->GetHeight() ; y++)
			{
				if ((y & 0x7) == 0)
				{
					if (pThread && pThread->DoExit())
					{
						DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
						return FALSE;
					}
					DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, pSrcDib->GetHeight());
				}

				for (unsigned int x = 0 ; x < pSrcDib->GetWidth() ; x++)
				{
					r = b = g = 0;
					for (int j = -k2 ; j < kmax ; j++)
					{
						for (int k = -k2 ; k < kmax ; k++)
						{
							crColor = pSrcDib->GetPixelColor(x + j, y + k);
							int i = pKernel[(j+k2) + nKernelSize * (k+k2)];
							r += GetRValue(crColor) * i;
							g += GetGValue(crColor) * i;
							b += GetBValue(crColor) * i;
						}
					}

					r = MIN(255, MAX(0, (int)(r + nKernelOffset)));
					g = MIN(255, MAX(0, (int)(g + nKernelOffset)));
					b = MIN(255, MAX(0, (int)(b + nKernelOffset)));
					SetPixelColor(x, y, RGB(r, g, b));
				}
			}
		}
	}

	// Create Palette
	CreatePaletteFromBMI();

	DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

	return TRUE;
}

// Fast version with a fixed Kernel
// Size of 3, a Kernel Factor which cannot be 0
// and a Kernel Offset of 0
BOOL CDib::FilterFast(	int* pKernel,
						int nKernelFactor,
						CDib* pSrcDib/*=NULL*/,
						CWnd* pProgressWnd/*=NULL*/,
						BOOL bProgressSend/*=TRUE*/,
						CWorkerThread* pThread/*=NULL*/)
{
	

	// Check Pointer
	if (!pKernel)
		return FALSE;

	// Check Kernel Factor
	if (!nKernelFactor)
		return FALSE;

	// Slow Version?
	if (pSrcDib)
	{
		if ((pSrcDib->GetBitCount() < 24) ||
			(pSrcDib->GetBitCount() == 32 && !pSrcDib->HasAlpha() && !pSrcDib->IsFast32bpp()))
			return Filter(	pKernel,
							3,
							nKernelFactor,
							0,
							pSrcDib,
							pProgressWnd,
							bProgressSend,
							pThread);
	}
	else 
	{
		if ((GetBitCount() < 24) ||
			(GetBitCount() == 32 && !HasAlpha() && !IsFast32bpp()))
			return Filter(	pKernel,
							3,
							nKernelFactor,
							0,
							pSrcDib,
							pProgressWnd,
							bProgressSend,
							pThread);
	}

	// Make a Copy of this?
	CDib SrcDib;
	BOOL bCopySrcToDst = FALSE;
	if (pSrcDib == NULL || this == pSrcDib)
	{
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

		// Allocate BMI
		if (m_pBMI == NULL)
		{
			// Allocate & Copy BMI
			m_pBMI = (LPBITMAPINFO)new BYTE[pSrcDib->GetBMISize()];
			if (m_pBMI == NULL)
				return FALSE;
			memcpy((void*)m_pBMI, (void*)pSrcDib->m_pBMI, pSrcDib->GetBMISize());
		}
		// Need to ReAllocate BMI because they are of differente size
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

	// Scan Line Alignments
	DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES(pSrcDib->GetWidth() * pSrcDib->GetBitCount());

	// Allocate Bits
	if (m_pBits == NULL)
	{
		// Allocate memory
		m_pBits = (LPBYTE)BIGALLOC(uiDIBScanLineSize * pSrcDib->GetHeight());
	}
	// Need to ReAllocate Bits because they are of different size
	else if (m_dwImageSize != uiDIBScanLineSize * pSrcDib->GetHeight())
	{
		BIGFREE(m_pBits);

		// Allocate memory
		m_pBits = (LPBYTE)BIGALLOC(uiDIBScanLineSize * pSrcDib->GetHeight());
	}
	if (m_pBits == NULL)
		return FALSE;

	// Copy Vars
	if (bCopySrcToDst)
		CopyVars(*pSrcDib);

	// Change BMI
	m_pBMI->bmiHeader.biWidth = pSrcDib->GetWidth();
	m_pBMI->bmiHeader.biHeight = pSrcDib->GetHeight();
	m_dwImageSize = m_pBMI->bmiHeader.biSizeImage = uiDIBScanLineSize * pSrcDib->GetHeight();

	// Init Masks For 32 bits Pictures
	InitMasks();

	// Free
	ResetColorUndo();
	if (m_hDibSection)
	{
		::DeleteObject(m_hDibSection);
		m_hDibSection = NULL;
		m_pDibSectionBits = NULL;
	}

	// Pointers
	LPBYTE lpSrcBits = pSrcDib->GetBits();
	LPBYTE lpDstBits = GetBits();

	DIB_INIT_PROGRESS;

	// Filter
	int r, g, b, a, x, y, X, Y, j, k, i;
	if (pSrcDib->GetBitCount() == 24)
	{
		// Line y = 0:
		for (x = 0 ; x < (int)pSrcDib->GetWidth() ; x++)
		{
			r = b = g = 0;
			for (j = -1 ; j < 2 ; j++)
			{
				for (k = -1 ; k < 2 ; k++)
				{
					i = pKernel[(j+1) + 3 * (k+1)];

					// Src Point
					X = x + j;
					Y = k;

					// Clamp
					X = MIN((int)pSrcDib->GetWidth() - 1, MAX(0, X));
					if (Y == -1) Y++;
					
					b += lpSrcBits[3*X   + Y * uiDIBScanLineSize] * i;
					g += lpSrcBits[3*X+1 + Y * uiDIBScanLineSize] * i;
					r += lpSrcBits[3*X+2 + Y * uiDIBScanLineSize] * i;
				}
			}
			r = MIN(255, MAX(0, (r / nKernelFactor)));
			g = MIN(255, MAX(0, (g / nKernelFactor)));
			b = MIN(255, MAX(0, (b / nKernelFactor)));
			lpDstBits[3*x]   = b;
			lpDstBits[3*x+1] = g;
			lpDstBits[3*x+2] = r;
		}

		// Line y = GetHeight - 1:
		y = pSrcDib->GetHeight() - 1;
		for (x = 0 ; x < (int)pSrcDib->GetWidth() ; x++)
		{
			r = b = g = 0;
			for (j = -1 ; j < 2 ; j++)
			{
				for (k = -1 ; k < 2 ; k++)
				{
					i = pKernel[(j+1) + 3 * (k+1)];

					// Src Point
					X = x + j;
					Y = y + k;
					
					// Clamp
					X = MIN((int)pSrcDib->GetWidth() - 1, MAX(0, X));
					if (Y == (int)pSrcDib->GetHeight()) Y--;
					
					b += lpSrcBits[3*X   + Y * uiDIBScanLineSize] * i;
					g += lpSrcBits[3*X+1 + Y * uiDIBScanLineSize] * i;
					r += lpSrcBits[3*X+2 + Y * uiDIBScanLineSize] * i;
				}
			}
			r = MIN(255, MAX(0, (r / nKernelFactor)));
			g = MIN(255, MAX(0, (g / nKernelFactor)));
			b = MIN(255, MAX(0, (b / nKernelFactor)));
			lpDstBits[3*x   + y * uiDIBScanLineSize] = b;
			lpDstBits[3*x+1 + y * uiDIBScanLineSize] = g;
			lpDstBits[3*x+2 + y * uiDIBScanLineSize] = r;
		}

		// Line x = 0:
		for (y = 1 ; y < ((int)pSrcDib->GetHeight() - 1) ; y++)
		{
			r = b = g = 0;
			for (j = -1 ; j < 2 ; j++)
			{
				for (k = -1 ; k < 2 ; k++)
				{
					i = pKernel[(j+1) + 3 * (k+1)];

					// Src Point
					X = j;
					Y = y + k;

					// Clamp
					if (X == -1) X++;

					b += lpSrcBits[3*X   + Y * uiDIBScanLineSize] * i;
					g += lpSrcBits[3*X+1 + Y * uiDIBScanLineSize] * i;
					r += lpSrcBits[3*X+2 + Y * uiDIBScanLineSize] * i;
				}
			}
			r = MIN(255, MAX(0, (r / nKernelFactor)));
			g = MIN(255, MAX(0, (g / nKernelFactor)));
			b = MIN(255, MAX(0, (b / nKernelFactor)));
			lpDstBits[y * uiDIBScanLineSize]   = b;
			lpDstBits[y * uiDIBScanLineSize+1] = g;
			lpDstBits[y * uiDIBScanLineSize+2] = r;
		}

		// Line x = pSrcDib->GetWidth() - 1:
		x = pSrcDib->GetWidth() - 1;
		for (y = 1 ; y < ((int)pSrcDib->GetHeight() - 1) ; y++)
		{
			r = b = g = 0;
			for (j = -1 ; j < 2 ; j++)
			{
				for (k = -1 ; k < 2 ; k++)
				{
					i = pKernel[(j+1) + 3 * (k+1)];

					// Src Point
					X = x + j;
					Y = y + k;
					
					// Clamp
					if (X == (int)pSrcDib->GetWidth()) X--;
					
					b += lpSrcBits[3*X   + Y * uiDIBScanLineSize] * i;
					g += lpSrcBits[3*X+1 + Y * uiDIBScanLineSize] * i;
					r += lpSrcBits[3*X+2 + Y * uiDIBScanLineSize] * i;
				}
			}
			r = MIN(255, MAX(0, (r / nKernelFactor)));
			g = MIN(255, MAX(0, (g / nKernelFactor)));
			b = MIN(255, MAX(0, (b / nKernelFactor)));
			lpDstBits[3*x   + y * uiDIBScanLineSize] = b;
			lpDstBits[3*x+1 + y * uiDIBScanLineSize] = g;
			lpDstBits[3*x+2 + y * uiDIBScanLineSize] = r;
		}

		// Internal Area
		for (y = 1 ; y < ((int)pSrcDib->GetHeight() - 1) ; y++)
		{
			if ((y & 0x7) == 0)
			{
				if (pThread && pThread->DoExit())
				{
					DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
					return FALSE;
				}
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, pSrcDib->GetHeight());
			}

			for (x = 1 ; x < ((int)pSrcDib->GetWidth() - 1) ; x++)
			{
				r = b = g = 0;
				for (j = -1 ; j < 2 ; j++)
				{
					for (k = -1 ; k < 2 ; k++)
					{
						i = pKernel[(j+1) + 3 * (k+1)];
						b += lpSrcBits[3*(x + j)     + (y + k) * uiDIBScanLineSize] * i;
						g += lpSrcBits[3*(x + j) + 1 + (y + k) * uiDIBScanLineSize] * i;
						r += lpSrcBits[3*(x + j) + 2 + (y + k) * uiDIBScanLineSize] * i;
					}
				}
				r = MIN(255, MAX(0, (r / nKernelFactor)));
				g = MIN(255, MAX(0, (g / nKernelFactor)));
				b = MIN(255, MAX(0, (b / nKernelFactor)));
				lpDstBits[3*x   + y * uiDIBScanLineSize] = b;
				lpDstBits[3*x+1 + y * uiDIBScanLineSize] = g;
				lpDstBits[3*x+2 + y * uiDIBScanLineSize] = r;
			}
		}
	}
	else if (pSrcDib->GetBitCount() == 32)
	{
		if (pSrcDib->HasAlpha())
		{
			// Line y = 0:
			for (x = 0 ; x < (int)pSrcDib->GetWidth() ; x++)
			{
				r = b = g = a = 0;
				for (j = -1 ; j < 2 ; j++)
				{
					for (k = -1 ; k < 2 ; k++)
					{
						i = pKernel[(j+1) + 3 * (k+1)];

						// Src Point
						X = x + j;
						Y = k;

						// Clamp
						X = MIN((int)pSrcDib->GetWidth() - 1, MAX(0, X));
						if (Y == -1) Y++;
						
						b += lpSrcBits[4*X   + Y * uiDIBScanLineSize] * i;
						g += lpSrcBits[4*X+1 + Y * uiDIBScanLineSize] * i;
						r += lpSrcBits[4*X+2 + Y * uiDIBScanLineSize] * i;
						a += lpSrcBits[4*X+3 + Y * uiDIBScanLineSize] * i;
					}
				}
				r = MIN(255, MAX(0, (r / nKernelFactor)));
				g = MIN(255, MAX(0, (g / nKernelFactor)));
				b = MIN(255, MAX(0, (b / nKernelFactor)));
				a = MIN(255, MAX(0, (a / nKernelFactor)));
				lpDstBits[4*x]   = b;
				lpDstBits[4*x+1] = g;
				lpDstBits[4*x+2] = r;
				lpDstBits[4*x+3] = a;
			}

			// Line y = GetHeight - 1:
			y = pSrcDib->GetHeight() - 1;
			for (x = 0 ; x < (int)pSrcDib->GetWidth() ; x++)
			{
				r = b = g = a = 0;
				for (j = -1 ; j < 2 ; j++)
				{
					for (k = -1 ; k < 2 ; k++)
					{
						i = pKernel[(j+1) + 3 * (k+1)];

						// Src Point
						X = x + j;
						Y = y + k;
						
						// Clamp
						X = MIN((int)pSrcDib->GetWidth() - 1, MAX(0, X));
						if (Y == (int)pSrcDib->GetHeight()) Y--;
						
						b += lpSrcBits[4*X   + Y * uiDIBScanLineSize] * i;
						g += lpSrcBits[4*X+1 + Y * uiDIBScanLineSize] * i;
						r += lpSrcBits[4*X+2 + Y * uiDIBScanLineSize] * i;
						a += lpSrcBits[4*X+3 + Y * uiDIBScanLineSize] * i;
					}
				}
				r = MIN(255, MAX(0, (r / nKernelFactor)));
				g = MIN(255, MAX(0, (g / nKernelFactor)));
				b = MIN(255, MAX(0, (b / nKernelFactor)));
				a = MIN(255, MAX(0, (a / nKernelFactor)));
				lpDstBits[4*x   + y * uiDIBScanLineSize] = b;
				lpDstBits[4*x+1 + y * uiDIBScanLineSize] = g;
				lpDstBits[4*x+2 + y * uiDIBScanLineSize] = r;
				lpDstBits[4*x+3 + y * uiDIBScanLineSize] = a;
			}

			// Line x = 0:
			for (y = 1 ; y < ((int)pSrcDib->GetHeight() - 1) ; y++)
			{
				r = b = g = a = 0;
				for (j = -1 ; j < 2 ; j++)
				{
					for (k = -1 ; k < 2 ; k++)
					{
						i = pKernel[(j+1) + 3 * (k+1)];

						// Src Point
						X = j;
						Y = y + k;

						// Clamp
						if (X == -1) X++;

						b += lpSrcBits[4*X   + Y * uiDIBScanLineSize] * i;
						g += lpSrcBits[4*X+1 + Y * uiDIBScanLineSize] * i;
						r += lpSrcBits[4*X+2 + Y * uiDIBScanLineSize] * i;
						a += lpSrcBits[4*X+3 + Y * uiDIBScanLineSize] * i;
					}
				}
				r = MIN(255, MAX(0, (r / nKernelFactor)));
				g = MIN(255, MAX(0, (g / nKernelFactor)));
				b = MIN(255, MAX(0, (b / nKernelFactor)));
				a = MIN(255, MAX(0, (a / nKernelFactor)));
				lpDstBits[y * uiDIBScanLineSize]   = b;
				lpDstBits[y * uiDIBScanLineSize+1] = g;
				lpDstBits[y * uiDIBScanLineSize+2] = r;
				lpDstBits[y * uiDIBScanLineSize+3] = a;
			}

			// Line x = pSrcDib->GetWidth() - 1:
			x = pSrcDib->GetWidth() - 1;
			for (y = 1 ; y < ((int)pSrcDib->GetHeight() - 1) ; y++)
			{
				r = b = g = a = 0;
				for (j = -1 ; j < 2 ; j++)
				{
					for (k = -1 ; k < 2 ; k++)
					{
						i = pKernel[(j+1) + 3 * (k+1)];

						// Src Point
						X = x + j;
						Y = y + k;
						
						// Clamp
						if (X == (int)pSrcDib->GetWidth()) X--;
						
						b += lpSrcBits[4*X   + Y * uiDIBScanLineSize] * i;
						g += lpSrcBits[4*X+1 + Y * uiDIBScanLineSize] * i;
						r += lpSrcBits[4*X+2 + Y * uiDIBScanLineSize] * i;
						a += lpSrcBits[4*X+3 + Y * uiDIBScanLineSize] * i;
					}
				}
				r = MIN(255, MAX(0, (r / nKernelFactor)));
				g = MIN(255, MAX(0, (g / nKernelFactor)));
				b = MIN(255, MAX(0, (b / nKernelFactor)));
				a = MIN(255, MAX(0, (a / nKernelFactor)));
				lpDstBits[4*x   + y * uiDIBScanLineSize] = b;
				lpDstBits[4*x+1 + y * uiDIBScanLineSize] = g;
				lpDstBits[4*x+2 + y * uiDIBScanLineSize] = r;
				lpDstBits[4*x+3 + y * uiDIBScanLineSize] = a;
			}

			// Internal Area
			for (y = 1 ; y < ((int)pSrcDib->GetHeight() - 1) ; y++)
			{
				if ((y & 0x7) == 0)
				{
					if (pThread && pThread->DoExit())
					{
						DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
						return FALSE;
					}
					DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, pSrcDib->GetHeight());
				}

				for (x = 1 ; x < ((int)pSrcDib->GetWidth() - 1) ; x++)
				{
					r = b = g = a = 0;
					for (j = -1 ; j < 2 ; j++)
					{
						for (k = -1 ; k < 2 ; k++)
						{
							i = pKernel[(j+1) + 3 * (k+1)];
							b += lpSrcBits[4*(x + j)     + (y + k) * uiDIBScanLineSize] * i;
							g += lpSrcBits[4*(x + j) + 1 + (y + k) * uiDIBScanLineSize] * i;
							r += lpSrcBits[4*(x + j) + 2 + (y + k) * uiDIBScanLineSize] * i;
							a += lpSrcBits[4*(x + j) + 3 + (y + k) * uiDIBScanLineSize] * i;
						}
					}
					r = MIN(255, MAX(0, (r / nKernelFactor)));
					g = MIN(255, MAX(0, (g / nKernelFactor)));
					b = MIN(255, MAX(0, (b / nKernelFactor)));
					a = MIN(255, MAX(0, (a / nKernelFactor)));
					lpDstBits[4*x   + y * uiDIBScanLineSize] = b;
					lpDstBits[4*x+1 + y * uiDIBScanLineSize] = g;
					lpDstBits[4*x+2 + y * uiDIBScanLineSize] = r;
					lpDstBits[4*x+3 + y * uiDIBScanLineSize] = a;
				}
			}
		}
		else
		{
			// Line y = 0:
			for (x = 0 ; x < (int)pSrcDib->GetWidth() ; x++)
			{
				r = b = g = 0;
				for (j = -1 ; j < 2 ; j++)
				{
					for (k = -1 ; k < 2 ; k++)
					{
						i = pKernel[(j+1) + 3 * (k+1)];

						// Src Point
						X = x + j;
						Y = k;

						// Clamp
						X = MIN((int)pSrcDib->GetWidth() - 1, MAX(0, X));
						if (Y == -1) Y++;
						
						b += lpSrcBits[4*X   + Y * uiDIBScanLineSize] * i;
						g += lpSrcBits[4*X+1 + Y * uiDIBScanLineSize] * i;
						r += lpSrcBits[4*X+2 + Y * uiDIBScanLineSize] * i;
					}
				}
				r = MIN(255, MAX(0, (r / nKernelFactor)));
				g = MIN(255, MAX(0, (g / nKernelFactor)));
				b = MIN(255, MAX(0, (b / nKernelFactor)));
				lpDstBits[4*x]   = b;
				lpDstBits[4*x+1] = g;
				lpDstBits[4*x+2] = r;
			}

			// Line y = GetHeight - 1:
			y = pSrcDib->GetHeight() - 1;
			for (x = 0 ; x < (int)pSrcDib->GetWidth() ; x++)
			{
				r = b = g = 0;
				for (j = -1 ; j < 2 ; j++)
				{
					for (k = -1 ; k < 2 ; k++)
					{
						i = pKernel[(j+1) + 3 * (k+1)];

						// Src Point
						X = x + j;
						Y = y + k;
						
						// Clamp
						X = MIN((int)pSrcDib->GetWidth() - 1, MAX(0, X));
						if (Y == (int)pSrcDib->GetHeight()) Y--;
						
						b += lpSrcBits[4*X   + Y * uiDIBScanLineSize] * i;
						g += lpSrcBits[4*X+1 + Y * uiDIBScanLineSize] * i;
						r += lpSrcBits[4*X+2 + Y * uiDIBScanLineSize] * i;
					}
				}
				r = MIN(255, MAX(0, (r / nKernelFactor)));
				g = MIN(255, MAX(0, (g / nKernelFactor)));
				b = MIN(255, MAX(0, (b / nKernelFactor)));
				lpDstBits[4*x   + y * uiDIBScanLineSize] = b;
				lpDstBits[4*x+1 + y * uiDIBScanLineSize] = g;
				lpDstBits[4*x+2 + y * uiDIBScanLineSize] = r;
			}

			// Line x = 0:
			for (y = 1 ; y < ((int)pSrcDib->GetHeight() - 1) ; y++)
			{
				r = b = g = 0;
				for (j = -1 ; j < 2 ; j++)
				{
					for (k = -1 ; k < 2 ; k++)
					{
						i = pKernel[(j+1) + 3 * (k+1)];

						// Src Point
						X = j;
						Y = y + k;

						// Clamp
						if (X == -1) X++;

						b += lpSrcBits[4*X   + Y * uiDIBScanLineSize] * i;
						g += lpSrcBits[4*X+1 + Y * uiDIBScanLineSize] * i;
						r += lpSrcBits[4*X+2 + Y * uiDIBScanLineSize] * i;
					}
				}
				r = MIN(255, MAX(0, (r / nKernelFactor)));
				g = MIN(255, MAX(0, (g / nKernelFactor)));
				b = MIN(255, MAX(0, (b / nKernelFactor)));
				lpDstBits[y * uiDIBScanLineSize]   = b;
				lpDstBits[y * uiDIBScanLineSize+1] = g;
				lpDstBits[y * uiDIBScanLineSize+2] = r;
			}

			// Line x = pSrcDib->GetWidth() - 1:
			x = pSrcDib->GetWidth() - 1;
			for (y = 1 ; y < ((int)pSrcDib->GetHeight() - 1) ; y++)
			{
				r = b = g = 0;
				for (j = -1 ; j < 2 ; j++)
				{
					for (k = -1 ; k < 2 ; k++)
					{
						i = pKernel[(j+1) + 3 * (k+1)];

						// Src Point
						X = x + j;
						Y = y + k;
						
						// Clamp
						if (X == (int)pSrcDib->GetWidth()) X--;
						
						b += lpSrcBits[4*X   + Y * uiDIBScanLineSize] * i;
						g += lpSrcBits[4*X+1 + Y * uiDIBScanLineSize] * i;
						r += lpSrcBits[4*X+2 + Y * uiDIBScanLineSize] * i;
					}
				}
				r = MIN(255, MAX(0, (r / nKernelFactor)));
				g = MIN(255, MAX(0, (g / nKernelFactor)));
				b = MIN(255, MAX(0, (b / nKernelFactor)));
				lpDstBits[4*x   + y * uiDIBScanLineSize] = b;
				lpDstBits[4*x+1 + y * uiDIBScanLineSize] = g;
				lpDstBits[4*x+2 + y * uiDIBScanLineSize] = r;
			}

			// Internal Area
			for (y = 1 ; y < ((int)pSrcDib->GetHeight() - 1) ; y++)
			{
				if ((y & 0x7) == 0)
				{
					if (pThread && pThread->DoExit())
					{
						DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
						return FALSE;
					}
					DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, pSrcDib->GetHeight());
				}

				for (x = 1 ; x < ((int)pSrcDib->GetWidth() - 1) ; x++)
				{
					r = b = g = 0;
					for (j = -1 ; j < 2 ; j++)
					{
						for (k = -1 ; k < 2 ; k++)
						{
							i = pKernel[(j+1) + 3 * (k+1)];
							b += lpSrcBits[4*(x + j)     + (y + k) * uiDIBScanLineSize] * i;
							g += lpSrcBits[4*(x + j) + 1 + (y + k) * uiDIBScanLineSize] * i;
							r += lpSrcBits[4*(x + j) + 2 + (y + k) * uiDIBScanLineSize] * i;
						}
					}
					r = MIN(255, MAX(0, (r / nKernelFactor)));
					g = MIN(255, MAX(0, (g / nKernelFactor)));
					b = MIN(255, MAX(0, (b / nKernelFactor)));
					lpDstBits[4*x   + y * uiDIBScanLineSize] = b;
					lpDstBits[4*x+1 + y * uiDIBScanLineSize] = g;
					lpDstBits[4*x+2 + y * uiDIBScanLineSize] = r;
				}
			}
		}
	}
	
	// Create Palette
	CreatePaletteFromBMI();

	DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

	return TRUE;
}

// Filter alpha channel only
BOOL CDib::FilterAlpha(	CRect rc,
						int* pKernel,
						int nKernelSize,
						int nKernelFactor,
						int nKernelOffset/*=0*/,
						CDib* pSrcDib/*=NULL*/,
						CWnd* pProgressWnd/*=NULL*/,
						BOOL bProgressSend/*=TRUE*/,
						CWorkerThread* pThread/*=NULL*/)
{
	// Check Pointer
	if (!pKernel)
		return FALSE;

	// Check whether it is a Alpha Image
	if (pSrcDib)
	{
		if (pSrcDib->GetBitCount() != 32 || !pSrcDib->HasAlpha())
			return FALSE;
	}
	else 
	{
		if (GetBitCount() != 32 || !HasAlpha())
			return FALSE;
	}

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
		// Need to ReAllocate BMI because they are of differente size
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

	// Width & Scan Line Alignment
	int nWidth = (int)pSrcDib->GetWidth();
	int nHeight = (int)pSrcDib->GetHeight();
	DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES(nWidth * pSrcDib->GetBitCount());

	// Clip Rectangle
	if (rc.left < 0)
		rc.left = 0;
	if (rc.top < 0)
		rc.top = 0;
	if (rc.right > nWidth)
		rc.right = nWidth;
	if (rc.bottom > nHeight)
		rc.bottom = nHeight;

	// Allocate Bits
	if (m_pBits == NULL)
	{
		// Allocate memory
		m_pBits = (LPBYTE)BIGALLOC(uiDIBScanLineSize * pSrcDib->GetHeight());
	}
	// Need to ReAllocate Bits because they are of different size
	else if (m_dwImageSize != uiDIBScanLineSize * pSrcDib->GetHeight())
	{
		BIGFREE(m_pBits);

		// Allocate memory
		m_pBits = (LPBYTE)BIGALLOC(uiDIBScanLineSize * pSrcDib->GetHeight());
	}
	if (m_pBits == NULL)
		return FALSE;

	// Copy Vars
	if (bCopySrcToDst)
		CopyVars(*pSrcDib);

	// Change BMI
	m_pBMI->bmiHeader.biWidth = pSrcDib->GetWidth();
	m_pBMI->bmiHeader.biHeight = pSrcDib->GetHeight();
	m_dwImageSize = m_pBMI->bmiHeader.biSizeImage = uiDIBScanLineSize * pSrcDib->GetHeight();

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

	// Copy Src To Dst?
	if (bCopySrcToDst && rc != CRect(0, 0, nWidth, nHeight))
		memcpy((void*)m_pBits, (void*)pSrcDib->m_pBits, m_dwImageSize);

	DIB_INIT_PROGRESS;

	// Filter
	LPBYTE lpBits;
	int nYPos = nHeight - rc.top - rc.Height();
	LPBYTE lpSrcBits = pSrcDib->GetBits() + nYPos * uiDIBScanLineSize;
	LPBYTE lpDstBits = GetBits() + nYPos * uiDIBScanLineSize;
	int k2 = nKernelSize / 2;
	int kmax = nKernelSize - k2;
	int i, a, A, X, Y;
	if (nKernelFactor != 0)
	{
		for (int y = 0 ; y < rc.Height() ; y++)
		{
			if ((y & 0x7) == 0)
			{
				if (pThread && pThread->DoExit())
				{
					DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
					return FALSE;
				}
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, rc.Height());
			}

			for (int x = rc.left ; x < rc.right ; x++)
			{
				a = 0;
				for (int j = -k2 ; j < kmax ; j++)
				{
					for (int k = -k2 ; k < kmax ; k++)
					{
						// Src Point
						X = x + j;
						Y = nYPos + k;

						// Clamp
						if (X < 0)
							A = 0;
						else if (Y < 0)
							A = 0;
						else if (X >= nWidth)
							A = 0;
						else if (Y >= nHeight)
							A = 0;
						else
						{
							lpBits = lpSrcBits + (k * (int)uiDIBScanLineSize);
							A = lpBits[4*X+3];
						}

						// Calc. Alpha
						i = pKernel[(j+k2) + nKernelSize * (k+k2)];
						a += A * i;
					}
				}
				a = MIN(255, MAX(0, (a / nKernelFactor + nKernelOffset)));
				((DWORD*)lpDstBits)[x] = ((DWORD)(a << 24)) | (((DWORD*)lpSrcBits)[x] & 0x00FFFFFF);
			}
			nYPos++;
			lpSrcBits += uiDIBScanLineSize;
			lpDstBits += uiDIBScanLineSize;
		}
	}
	else
	{
		for (int y = 0 ; y < rc.Height() ; y++)
		{
			if ((y & 0x7) == 0)
			{
				if (pThread && pThread->DoExit())
				{
					DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
					return FALSE;
				}
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, rc.Height());
			}

			for (int x = rc.left ; x < rc.right ; x++)
			{
				a = 0;
				for (int j = -k2 ; j < kmax ; j++)
				{
					for (int k = -k2 ; k < kmax ; k++)
					{
						// Src Point
						X = x + j;
						Y = nYPos + k;

						// Clamp
						if (X < 0)
							A = 0;
						else if (Y < 0)
							A = 0;
						else if (X >= nWidth)
							A = 0;
						else if (Y >= nHeight)
							A = 0;
						else
						{
							lpBits = lpSrcBits + (k * (int)uiDIBScanLineSize);
							A = lpBits[4*X+3];
						}

						// Calc. Alpha
						i = pKernel[(j+k2) + nKernelSize * (k+k2)];
						a += A * i;
					}
				}
				a = MIN(255, MAX(0, (a + nKernelOffset)));
				((DWORD*)lpDstBits)[x] = ((DWORD)(a << 24)) | (((DWORD*)lpSrcBits)[x] & 0x00FFFFFF);
			}
			nYPos++;
			lpSrcBits += uiDIBScanLineSize;
			lpDstBits += uiDIBScanLineSize;
		}
	}

	// Create Palette
	CreatePaletteFromBMI();

	DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

	return TRUE;
}

// Fast version for alpha channel only
// with a fixed Kernel Size of 3,
// a Kernel Factor which cannot be 0
// and a Kernel Offset of 0
BOOL CDib::FilterFastAlpha(	int* pKernel,
							int nKernelFactor,
							CDib* pSrcDib/*=NULL*/,
							CWnd* pProgressWnd/*=NULL*/,
							BOOL bProgressSend/*=TRUE*/,
							CWorkerThread* pThread/*=NULL*/)
{
	

	// Check Pointer
	if (!pKernel)
		return FALSE;

	// Check Kernel Factor
	if (!nKernelFactor)
		return FALSE;

	// Check whether it is a Alpha Image
	if (pSrcDib)
	{
		if (pSrcDib->GetBitCount() != 32 || !pSrcDib->HasAlpha())
			return FALSE;
	}
	else 
	{
		if (GetBitCount() != 32 || !HasAlpha())
			return FALSE;
	}

	// Make a Copy of this?
	CDib SrcDib;
	BOOL bCopySrcToDst = FALSE;
	if (pSrcDib == NULL || this == pSrcDib)
	{
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

		// Allocate BMI
		if (m_pBMI == NULL)
		{
			// Allocate & Copy BMI
			m_pBMI = (LPBITMAPINFO)new BYTE[pSrcDib->GetBMISize()];
			if (m_pBMI == NULL)
				return FALSE;
			memcpy((void*)m_pBMI, (void*)pSrcDib->m_pBMI, pSrcDib->GetBMISize());
		}
		// Need to ReAllocate BMI because they are of differente size
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

	// Width & Scan Line Alignment
	int nWidth = (int)pSrcDib->GetWidth();
	int nHeight = (int)pSrcDib->GetHeight();
	DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES(nWidth * pSrcDib->GetBitCount());

	// Allocate Bits
	if (m_pBits == NULL)
	{
		// Allocate memory
		m_pBits = (LPBYTE)BIGALLOC(uiDIBScanLineSize * pSrcDib->GetHeight());
	}
	// Need to ReAllocate Bits because they are of different size
	else if (m_dwImageSize != uiDIBScanLineSize * pSrcDib->GetHeight())
	{
		BIGFREE(m_pBits);

		// Allocate memory
		m_pBits = (LPBYTE)BIGALLOC(uiDIBScanLineSize * pSrcDib->GetHeight());
	}
	if (m_pBits == NULL)
		return FALSE;

	// Copy Vars
	if (bCopySrcToDst)
		CopyVars(*pSrcDib);

	// Change BMI
	m_pBMI->bmiHeader.biWidth = pSrcDib->GetWidth();
	m_pBMI->bmiHeader.biHeight = pSrcDib->GetHeight();
	m_dwImageSize = m_pBMI->bmiHeader.biSizeImage = uiDIBScanLineSize * pSrcDib->GetHeight();

	// Init Masks For 32 bits Pictures
	InitMasks();

	// Free
	ResetColorUndo();
	if (m_hDibSection)
	{
		::DeleteObject(m_hDibSection);
		m_hDibSection = NULL;
		m_pDibSectionBits = NULL;
	}

	// Pointers
	LPBYTE lpSrcBits = pSrcDib->GetBits();
	LPBYTE lpDstBits = GetBits();

	DIB_INIT_PROGRESS;

	// Filter
	int a, x, y, A, X, Y, j, k, i;
	
	// Line y = 0:
	for (x = 0 ; x < nWidth ; x++)
	{
		a = 0;
		for (j = -1 ; j < 2 ; j++)
		{
			for (k = -1 ; k < 2 ; k++)
			{
				// Src Point
				X = x + j;
				Y = k;

				// Clamp
				if (X < 0)
					A = 0;
				else if (Y < 0)
					A = 0;
				else if (X >= nWidth)
					A = 0;
				else if (Y >= nHeight)
					A = 0;
				else
					A = lpSrcBits[4*X+3 + Y * uiDIBScanLineSize];
				
				// Calc. Alpha
				i = pKernel[(j+1) + 3 * (k+1)];
				a += A * i;
			}
		}
		((DWORD*)lpDstBits)[x] = ((DWORD*)lpSrcBits)[x];
		lpDstBits[4*x+3] = MIN(255, MAX(0, (a / nKernelFactor)));
	}

	// Line y = nHeight - 1:s
	y = nHeight - 1;
	for (x = 0 ; x < nWidth ; x++)
	{
		a = 0;
		for (j = -1 ; j < 2 ; j++)
		{
			for (k = -1 ; k < 2 ; k++)
			{
				// Src Point
				X = x + j;
				Y = y + k;
				
				// Clamp
				if (X < 0)
					A = 0;
				else if (Y < 0)
					A = 0;
				else if (X >= nWidth)
					A = 0;
				else if (Y >= nHeight)
					A = 0;
				else
					A = lpSrcBits[4*X+3 + Y * uiDIBScanLineSize];
				
				// Calc. Alpha
				i = pKernel[(j+1) + 3 * (k+1)];
				a += A * i;
			}
		}
		((DWORD*)lpDstBits)[x + y * nWidth] = ((DWORD*)lpSrcBits)[x + y * nWidth];
		lpDstBits[4*x+3 + y * uiDIBScanLineSize] = MIN(255, MAX(0, (a / nKernelFactor)));
	}

	// Line x = 0:
	for (y = 1 ; y < (nHeight - 1) ; y++)
	{
		a = 0;
		for (j = -1 ; j < 2 ; j++)
		{
			for (k = -1 ; k < 2 ; k++)
			{
				// Src Point
				X = j;
				Y = y + k;

				// Clamp
				if (X < 0)
					A = 0;
				else if (Y < 0)
					A = 0;
				else if (X >= nWidth)
					A = 0;
				else if (Y >= nHeight)
					A = 0;
				else
					A = lpSrcBits[4*X+3 + Y * uiDIBScanLineSize];

				// Calc. Alpha
				i = pKernel[(j+1) + 3 * (k+1)];
				a += A * i;
			}
		}
		((DWORD*)lpDstBits)[y * nWidth] = ((DWORD*)lpSrcBits)[y * nWidth];
		lpDstBits[y * uiDIBScanLineSize+3] = MIN(255, MAX(0, (a / nKernelFactor)));
	}

	// Line x = nWidth - 1:
	x = nWidth - 1;
	for (y = 1 ; y < (nHeight - 1) ; y++)
	{
		a = 0;
		for (j = -1 ; j < 2 ; j++)
		{
			for (k = -1 ; k < 2 ; k++)
			{
				// Src Point
				X = x + j;
				Y = y + k;
				
				// Clamp
				if (X < 0)
					A = 0;
				else if (Y < 0)
					A = 0;
				else if (X >= nWidth)
					A = 0;
				else if (Y >= nHeight)
					A = 0;
				else
					A = lpSrcBits[4*X+3 + Y * uiDIBScanLineSize];
				
				// Calc. Alpha
				i = pKernel[(j+1) + 3 * (k+1)];
				a += A * i;
			}
		}
		((DWORD*)lpDstBits)[x + y * nWidth] = ((DWORD*)lpSrcBits)[x + y * nWidth];
		lpDstBits[4*x+3 + y * uiDIBScanLineSize] = MIN(255, MAX(0, (a / nKernelFactor)));
	}

	// Internal Area
	for (y = 1 ; y < (nHeight - 1) ; y++)
	{
		if ((y & 0x7) == 0)
		{
			if (pThread && pThread->DoExit())
			{
				DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
				return FALSE;
			}
			DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, nHeight);
		}

		for (x = 1 ; x < (nWidth - 1) ; x++)
		{
			a = 0;
			for (j = -1 ; j < 2 ; j++)
			{
				for (k = -1 ; k < 2 ; k++)
				{
					// Calc. Alpha
					i = pKernel[(j+1) + 3 * (k+1)];
					a += lpSrcBits[4*(x + j) + 3 + (y + k) * uiDIBScanLineSize] * i;
				}
			}
			((DWORD*)lpDstBits)[x + y * nWidth] = ((DWORD*)lpSrcBits)[x + y * nWidth];
			lpDstBits[4*x+3 + y * uiDIBScanLineSize] = MIN(255, MAX(0, (a / nKernelFactor)));
		}
	}
	
	// Create Palette
	CreatePaletteFromBMI();

	DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

	return TRUE;
}