#include "stdafx.h"
#include "dib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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
	if (bForceNearestNeighbor)
	{
		return NearestNeighborResizeBits(	Round(pSrcDib->GetWidth() / dRatioMax),
											Round(pSrcDib->GetHeight() / dRatioMax),
											pSrcDib,
											pProgressWnd,
											bProgressSend,
											pThread);
	}
	else
	{
		return StretchBits(	Round(pSrcDib->GetWidth() / dRatioMax),
							Round(pSrcDib->GetHeight() / dRatioMax),
							pSrcDib,
							pProgressWnd,
							bProgressSend,
							pThread);
	}
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
	if (bForceNearestNeighbor)
	{
		if (!NearestNeighborResizeBits(	Round(pSrcDib->GetWidth() / dRatioMax),
										Round(pSrcDib->GetHeight() / dRatioMax),
										pSrcDib,
										pProgressWnd,
										bProgressSend,
										pThread))
			return FALSE;
	}
	else
	{
		if (!StretchBits(	Round(pSrcDib->GetWidth() / dRatioMax),
							Round(pSrcDib->GetHeight() / dRatioMax),
							pSrcDib,
							pProgressWnd,
							bProgressSend,
							pThread))
			return FALSE;
	}

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

BOOL CDib::StretchBits(	DWORD dwNewWidth,
						DWORD dwNewHeight,
						CDib* pSrcDib/*=NULL*/,
						CWnd* pProgressWnd/*=NULL*/,
						BOOL bProgressSend/*=TRUE*/,
						CWorkerThread* pThread/*=NULL*/)
{
	if (pSrcDib == NULL)
		pSrcDib = this;

	// Calc. Ratio
	double dRatioX = (double)pSrcDib->GetWidth() / (double)dwNewWidth;
	double dRatioY = (double)pSrcDib->GetHeight() / (double)dwNewHeight;
	double dRatioMin = min(dRatioX, dRatioY);

	// Shrink
	if (dRatioMin > 1.0)
	{
		BOOL res = FALSE;

		// Two Pass Shrinking to Speed-Up
		if (dRatioMin >= 7.0)
		{
			if (NearestNeighborResizeBits(	5 * dwNewWidth,
											5 * dwNewHeight,
											pSrcDib,
											pProgressWnd,
											bProgressSend,
											pThread))
			{
				res = ShrinkBits(	dwNewWidth,
									dwNewHeight,
									NULL,
									pProgressWnd,
									bProgressSend,
									pThread);
			}
		}
		else
		{
			res = ShrinkBits(	dwNewWidth,
								dwNewHeight,
								pSrcDib,
								pProgressWnd,
								bProgressSend,
								pThread);
		}

		return res;
	}
	// Enlarge
	else
	{
		return BicubicResampleBits(	dwNewWidth,
									dwNewHeight,
									pSrcDib,
									pProgressWnd,
									bProgressSend,
									pThread);
	}
}

// Good for extreme shrinking because it avoids anti-aliasing
// (Like for thumbnail creation)
//
// Algorithm:
// ----------
//
// Note: Analyze the 32 bpp case which is more readable!
//
// We walk through the source pixels; the source pixel contribution is added
// to the corresponding destination pixel container (floating point array f).
// A destination pixel is calculated from multiple source pixels,
// in the best case the mapping is simple and includes entire source pixels.
// But usually we have to take into account the fact that a source pixel may
// contribute to 2 or 4 destination pixels.
// The source pixel contribution weighting is equal for all pixels. 
// 
// After walking the source array, the destination pixel container elements f[.]
// are copied to the destination array by scaling with the area ratio between
// source and destination images. Each element of f represents a color channel
// area in B, G, R (, A) order.
BOOL CDib::ShrinkBits(	DWORD dwNewWidth,
						DWORD dwNewHeight,
						CDib* pSrcDib/*=NULL*/,
						CWnd* pProgressWnd/*=NULL*/,
						BOOL bProgressSend/*=TRUE*/,
						CWorkerThread* pThread/*=NULL*/)
{
	// Vars
	BYTE R, G, B, index;
	LPBYTE pSrcBits;
	DWORD i, j;
	DWORD x, y;
	DWORD nx;
	DWORD ny = 0;
	double qx[2], qy[2], q[4];
	DWORD dwWidthContainer;
	DWORD dwContainerSize;

	// Check
	if (pSrcDib == NULL || this == pSrcDib)
	{
		// Only Shrinking is Allowed
		if ((dwNewWidth > GetWidth()) || (dwNewHeight > GetHeight()))
			return FALSE;

		// If Same Size Return
		if ((dwNewWidth == GetWidth()) && (dwNewHeight == GetHeight()))
			return TRUE;

		if (IsCompressed())
		{
			if (!Decompress(GetBitCount())) // Decompress
				return FALSE;
		}

		// Init Container Size
		if (HasAlpha() && GetBitCount() == 32)
			dwWidthContainer = 4 * dwNewWidth;
		else
			dwWidthContainer = 3 * dwNewWidth;
		dwContainerSize = dwWidthContainer * dwNewHeight;
	}
	else
	{
		// Only Shrinking is Allowed
		if ((dwNewWidth > pSrcDib->GetWidth()) || (dwNewHeight > pSrcDib->GetHeight()))
			return FALSE;

		// Pointers Check
		if (!pSrcDib->m_pBits || !pSrcDib->m_pBMI)
			return FALSE;

		// No Compression Supported!
		if (pSrcDib->IsCompressed())
			return FALSE;

		// Init Container Size
		if (pSrcDib->HasAlpha() && pSrcDib->GetBitCount() == 32)
			dwWidthContainer = 4 * dwNewWidth;
		else
			dwWidthContainer = 3 * dwNewWidth;
		dwContainerSize = dwWidthContainer * dwNewHeight;
	}

	// Allocate Temp Floating-Point Image
	double* f = (double*)BIGALLOC(dwContainerSize * sizeof(double));
	// If Not Enough Memory Use Bilinear 2x2 Averaging
	if (f == NULL)
	{
		return BilinearResampleBits(dwNewWidth,
									dwNewHeight,
									pSrcDib,
									pProgressWnd,
									bProgressSend,
									pThread);
	}

	// Init Floating-Point Pixel Channels (R, G, B and Alpha if available)
	for (j = 0 ; j < dwContainerSize ; j++)
		f[j] = 0.0;
	j = 0;

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
			{
				BIGFREE(f);
				return FALSE;
			}
		}

		// Pointers Check
		if (!pSrcDib->m_pBits || !pSrcDib->m_pBMI)
		{
			BIGFREE(f);
			return FALSE;
		}
	}
	else
	{
		// Allocate BMI
		if (m_pBMI == NULL)
		{
			// Allocate & Copy BMI
			m_pBMI = (LPBITMAPINFO)new BYTE[pSrcDib->GetBMISize()];
			if (m_pBMI == NULL)
			{
				BIGFREE(f);
				return FALSE;
			}
			memcpy((void*)m_pBMI, (void*)pSrcDib->m_pBMI, pSrcDib->GetBMISize());
		}
		// Need to ReAllocate BMI because they are of differente size
		else if (pSrcDib->GetBMISize() != GetBMISize())
		{
			delete [] m_pBMI;

			// Allocate & Copy BMI
			m_pBMI = (LPBITMAPINFO)new BYTE[pSrcDib->GetBMISize()];
			if (m_pBMI == NULL)
			{
				BIGFREE(f);
				return FALSE;
			}
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

	// Area Ratio
	double dAreaRatio =	(double)(dwNewWidth * dwNewHeight) /
						(double)(dwSrcWidth * dwSrcHeight);

	// Next Src Line Pointer
	LPBYTE pNextSrcLineBits = pSrcDib->GetBits();

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
	{
		BIGFREE(f);
		return FALSE;
	}

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
		BIGFREE(f);
		memcpy((void*)m_pBits, (void*)pSrcDib->m_pBits, m_dwImageSize);
		return TRUE;
	}

	DIB_INIT_PROGRESS;

	// Shrink
	switch (pSrcDib->GetBitCount())
	{
		case 1:
		{
			// Calculate Floating-Point Picture
			for (y = 0 ; y < dwSrcHeight ; y++)
			{
				if (pThread && pThread->DoExit())
				{
					BIGFREE(f);
					DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
					return FALSE;
				}
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, dwSrcHeight);

				pSrcBits = pNextSrcLineBits;
				pNextSrcLineBits += uiDIBSourceScanLineSize;
				nx = 0;
				ny += dwNewHeight;

				if (ny > dwSrcHeight)
				{
					qy[0] = 1.0 - (qy[1] = (ny - dwSrcHeight) / (double)dwNewHeight);

					for (x = 0 ; x < dwSrcWidth ; x++)
					{
						nx += dwNewWidth;
						i = (pSrcBits[x/8] >> (7-(x%8))) & 0x01;
						R = m_pColors[i].rgbRed;
						G = m_pColors[i].rgbGreen;
						B = m_pColors[i].rgbBlue;

						if (nx > dwSrcWidth)
						{
							qx[0] = 1.0 - (qx[1] = (nx - dwSrcWidth) / (double)dwNewWidth);

							for (i = 0 ; i < 4 ; i++)	
								q[i] = qx[i&1] * qy[i>>1];
	
							f[j] += B * q[0];
							f[j+3] += B * q[1];
							f[j+dwWidthContainer] += B * q[2];
							f[(j++)+dwWidthContainer+3] += B * q[3];
							f[j] += G * q[0];
							f[j+3] += G * q[1];
							f[j+dwWidthContainer] += G * q[2];
							f[(j++)+dwWidthContainer+3] += G * q[3];
							f[j] += R * q[0];
							f[j+3] += R * q[1];
							f[j+dwWidthContainer] += R * q[2];
							f[(j++)+dwWidthContainer+3] += R * q[3];
						}
						else
						{
							f[j] += B * qy[0];
							f[j+dwWidthContainer] += B * qy[1];
							f[j+1] += G * qy[0];
							f[j+dwWidthContainer+1] += G * qy[1];
							f[j+2] += R * qy[0];
							f[j+dwWidthContainer+2] += R * qy[1];
						}
			
						if (nx >= dwSrcWidth)
							nx -= dwSrcWidth;
						if (nx == 0)
							j += 3;
					}
				}
				else
				{
					for (x = 0 ; x < dwSrcWidth ; x++)
					{
						nx += dwNewWidth;
						i = (pSrcBits[x/8] >> (7-(x%8))) & 0x01;
						R = m_pColors[i].rgbRed;
						G = m_pColors[i].rgbGreen;
						B = m_pColors[i].rgbBlue;
 
						if (nx > dwSrcWidth)
						{
							qx[0] = 1.0 - (qx[1] = (nx - dwSrcWidth) / (double)dwNewWidth);
							
							f[j] += B * qx[0];
							f[(j++)+3] += B * qx[1];
							f[j] += G * qx[0];
							f[(j++)+3] += G * qx[1];
							f[j] += R * qx[0];
							f[(j++)+3] += R * qx[1];
						}
						else
						{
							f[j] += B;
							f[j+1] += G;
							f[j+2] += R;
						}
 
						if (nx >= dwSrcWidth)
							nx -= dwSrcWidth;
						if (nx == 0)
							j += 3;
					}

					if (ny < dwSrcHeight)
						j -= dwWidthContainer;
				}

				if (ny >= dwSrcHeight)
					ny -= dwSrcHeight;
			}
 
			// Copy From Floating-Point Picture to the new bits
			DWORD dwSrc = 0;
			LPBYTE pDstBits = m_pBits;
			InitGetClosestColorIndex();
			for (y = 0 ; y < dwNewHeight ; y++)
			{
				if (pThread && pThread->DoExit())
				{
					BIGFREE(f);
					DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
					return FALSE;
				}
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, dwNewHeight);

				for (j = 0 ; j < dwNewWidth ; j++)
				{
					B = (BYTE)Round(f[dwSrc++] * dAreaRatio);
					G = (BYTE)Round(f[dwSrc++] * dAreaRatio);
					R = (BYTE)Round(f[dwSrc++] * dAreaRatio);
					i = GetClosestColorIndex(RGB(R,G,B));
					ASSERT(i < 2);
					if (j%8 == 0)
						pDstBits[j/8] = 0;
					pDstBits[j/8] |= i << (7-(j%8));
				}
				pDstBits += uiDIBTargetScanLineSize;
			}

			break;
		}
		case 4:
		{
			// Calculate Floating-Point Picture
			for (y = 0 ; y < dwSrcHeight ; y++)
			{
				if (pThread && pThread->DoExit())
				{
					BIGFREE(f);
					DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
					return FALSE;
				}
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, dwSrcHeight);

				pSrcBits = pNextSrcLineBits;
				pNextSrcLineBits += uiDIBSourceScanLineSize;
				nx = 0;
				ny += dwNewHeight;

				if (ny > dwSrcHeight)
				{
					qy[0] = 1.0 - (qy[1] = (ny - dwSrcHeight) / (double)dwNewHeight);

					for (x = 0 ; x < dwSrcWidth ; x++)
					{
						nx += dwNewWidth;
						if (x%2) // LS Nibble
							i = pSrcBits[x/2] & 0x0F;
						else // MS Nibble
							i = pSrcBits[x/2] >> 4;
						R = m_pColors[i].rgbRed;
						G = m_pColors[i].rgbGreen;
						B = m_pColors[i].rgbBlue;

						if (nx > dwSrcWidth)
						{
							qx[0] = 1.0 - (qx[1] = (nx - dwSrcWidth) / (double)dwNewWidth);

							for (i = 0 ; i < 4 ; i++)	
								q[i] = qx[i&1] * qy[i>>1];
	
							f[j] += B * q[0];
							f[j+3] += B * q[1];
							f[j+dwWidthContainer] += B * q[2];
							f[(j++)+dwWidthContainer+3] += B * q[3];
							f[j] += G * q[0];
							f[j+3] += G * q[1];
							f[j+dwWidthContainer] += G * q[2];
							f[(j++)+dwWidthContainer+3] += G * q[3];
							f[j] += R * q[0];
							f[j+3] += R * q[1];
							f[j+dwWidthContainer] += R * q[2];
							f[(j++)+dwWidthContainer+3] += R * q[3];
						}
						else
						{
							f[j] += B * qy[0];
							f[j+dwWidthContainer] += B * qy[1];
							f[j+1] += G * qy[0];
							f[j+dwWidthContainer+1] += G * qy[1];
							f[j+2] += R * qy[0];
							f[j+dwWidthContainer+2] += R * qy[1];
						}
			
						if (nx >= dwSrcWidth)
							nx -= dwSrcWidth;
						if (nx == 0)
							j += 3;
					}
				}
				else
				{
					for (x = 0 ; x < dwSrcWidth ; x++)
					{
						nx += dwNewWidth;
						if (x%2) // LS Nibble
							i = pSrcBits[x/2] & 0x0F;
						else // MS Nibble
							i = pSrcBits[x/2] >> 4;
						R = m_pColors[i].rgbRed;
						G = m_pColors[i].rgbGreen;
						B = m_pColors[i].rgbBlue;
 
						if (nx > dwSrcWidth)
						{
							qx[0] = 1.0 - (qx[1] = (nx - dwSrcWidth) / (double)dwNewWidth);
							
							f[j] += B * qx[0];
							f[(j++)+3] += B * qx[1];
							f[j] += G * qx[0];
							f[(j++)+3] += G * qx[1];
							f[j] += R * qx[0];
							f[(j++)+3] += R * qx[1];
						}
						else
						{
							f[j] += B;
							f[j+1] += G;
							f[j+2] += R;
						}
 
						if (nx >= dwSrcWidth)
							nx -= dwSrcWidth;
						if (nx == 0)
							j += 3;
					}

					if (ny < dwSrcHeight)
						j -= dwWidthContainer;
				}

				if (ny >= dwSrcHeight)
					ny -= dwSrcHeight;
			}
 
			// Copy From Floating-Point Picture to the new bits
			DWORD dwSrc = 0;
			LPBYTE pDstBits = m_pBits;
			InitGetClosestColorIndex();
			for (y = 0 ; y < dwNewHeight ; y++)
			{
				if (pThread && pThread->DoExit())
				{
					BIGFREE(f);
					DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
					return FALSE;
				}
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, dwNewHeight);

				for (j = 0 ; j < dwNewWidth ; j++)
				{
					B = (BYTE)Round(f[dwSrc++] * dAreaRatio);
					G = (BYTE)Round(f[dwSrc++] * dAreaRatio);
					R = (BYTE)Round(f[dwSrc++] * dAreaRatio);
					index = GetClosestColorIndex(RGB(R,G,B));
					ASSERT(index < 16);
					if (j%2) // LS Nibble
						pDstBits[j/2] |= index;
					else // MS Nibble
						pDstBits[j/2] = index << 4;

				}
				pDstBits += uiDIBTargetScanLineSize;
			}

			break;
		}
		case 8:
		{
			// Calculate Floating-Point Picture
			for (y = 0 ; y < dwSrcHeight ; y++)
			{
				if (pThread && pThread->DoExit())
				{
					BIGFREE(f);
					DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
					return FALSE;
				}
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, dwSrcHeight);

				pSrcBits = pNextSrcLineBits;
				pNextSrcLineBits += uiDIBSourceScanLineSize;
				nx = 0;
				ny += dwNewHeight;

				if (ny > dwSrcHeight)
				{
					qy[0] = 1.0 - (qy[1] = (ny - dwSrcHeight) / (double)dwNewHeight);

					for (x = 0 ; x < dwSrcWidth ; x++)
					{
						nx += dwNewWidth;
						R = m_pColors[*pSrcBits].rgbRed;
						G = m_pColors[*pSrcBits].rgbGreen;
						B = m_pColors[*pSrcBits].rgbBlue;

						if (nx > dwSrcWidth)
						{
							qx[0] = 1.0 - (qx[1] = (nx - dwSrcWidth) / (double)dwNewWidth);

							for (i = 0 ; i < 4 ; i++)	
								q[i] = qx[i&1] * qy[i>>1];
	
							f[j] += B * q[0];
							f[j+3] += B * q[1];
							f[j+dwWidthContainer] += B * q[2];
							f[(j++)+dwWidthContainer+3] += B * q[3];
							f[j] += G * q[0];
							f[j+3] += G * q[1];
							f[j+dwWidthContainer] += G * q[2];
							f[(j++)+dwWidthContainer+3] += G * q[3];
							f[j] += R * q[0];
							f[j+3] += R * q[1];
							f[j+dwWidthContainer] += R * q[2];
							f[(j++)+dwWidthContainer+3] += R * q[3];
						}
						else
						{
							f[j] += B * qy[0];
							f[j+dwWidthContainer] += B * qy[1];
							f[j+1] += G * qy[0];
							f[j+dwWidthContainer+1] += G * qy[1];
							f[j+2] += R * qy[0];
							f[j+dwWidthContainer+2] += R * qy[1];
						}
			
						++pSrcBits;
						if (nx >= dwSrcWidth)
							nx -= dwSrcWidth;
						if (nx == 0)
							j += 3;
					}
				}
				else
				{
					for (x = 0 ; x < dwSrcWidth ; x++)
					{
						nx += dwNewWidth;
						R = m_pColors[*pSrcBits].rgbRed;
						G = m_pColors[*pSrcBits].rgbGreen;
						B = m_pColors[*pSrcBits].rgbBlue;
 
						if (nx > dwSrcWidth)
						{
							qx[0] = 1.0 - (qx[1] = (nx - dwSrcWidth) / (double)dwNewWidth);
							
							f[j] += B * qx[0];
							f[(j++)+3] += B * qx[1];
							f[j] += G * qx[0];
							f[(j++)+3] += G * qx[1];
							f[j] += R * qx[0];
							f[(j++)+3] += R * qx[1];
						}
						else
						{
							f[j] += B;
							f[j+1] += G;
							f[j+2] += R;
						}
 
						++pSrcBits;
						if (nx >= dwSrcWidth)
							nx -= dwSrcWidth;
						if (nx == 0)
							j += 3;
					}

					if (ny < dwSrcHeight)
						j -= dwWidthContainer;
				}

				if (ny >= dwSrcHeight)
					ny -= dwSrcHeight;
			}
 
			// Copy From Floating-Point Picture to the new bits
			DWORD dwSrc = 0;
			LPBYTE pDstBits = m_pBits;
			InitGetClosestColorIndex();
			for (y = 0 ; y < dwNewHeight ; y++)
			{
				if (pThread && pThread->DoExit())
				{
					BIGFREE(f);
					DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
					return FALSE;
				}
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, dwNewHeight);

				for (j = 0 ; j < dwNewWidth ; j++)
				{
					B = (BYTE)Round(f[dwSrc++] * dAreaRatio);
					G = (BYTE)Round(f[dwSrc++] * dAreaRatio);
					R = (BYTE)Round(f[dwSrc++] * dAreaRatio);
					pDstBits[j] = GetClosestColorIndex(RGB(R,G,B));
				}
				pDstBits += uiDIBTargetScanLineSize;
			}

			break;
		}
		case 16:
		{
			// Calculate Floating-Point Picture
			for (y = 0 ; y < dwSrcHeight ; y++)
			{
				if (pThread && pThread->DoExit())
				{
					BIGFREE(f);
					DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
					return FALSE;
				}
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, dwSrcHeight);

				pSrcBits = pNextSrcLineBits;
				pNextSrcLineBits += uiDIBSourceScanLineSize;
				nx = 0;
				ny += dwNewHeight;

				if (ny > dwSrcHeight)
				{
					qy[0] = 1.0 - (qy[1] = (ny - dwSrcHeight) / (double)dwNewHeight);

					for (x = 0 ; x < dwSrcWidth ; x++)
					{
						nx += dwNewWidth;
						DIB16ToRGB(*((WORD*)pSrcBits),&R,&G,&B);

						if (nx > dwSrcWidth)
						{
							qx[0] = 1.0 - (qx[1] = (nx - dwSrcWidth) / (double)dwNewWidth);

							for (i = 0 ; i < 4 ; i++)	
								q[i] = qx[i&1] * qy[i>>1];
	
							f[j] += B * q[0];
							f[j+3] += B * q[1];
							f[j+dwWidthContainer] += B * q[2];
							f[(j++)+dwWidthContainer+3] += B * q[3];
							f[j] += G * q[0];
							f[j+3] += G * q[1];
							f[j+dwWidthContainer] += G * q[2];
							f[(j++)+dwWidthContainer+3] += G * q[3];
							f[j] += R * q[0];
							f[j+3] += R * q[1];
							f[j+dwWidthContainer] += R * q[2];
							f[(j++)+dwWidthContainer+3] += R * q[3];
						}
						else
						{
							f[j] += B * qy[0];
							f[j+dwWidthContainer] += B * qy[1];
							f[j+1] += G * qy[0];
							f[j+dwWidthContainer+1] += G * qy[1];
							f[j+2] += R * qy[0];
							f[j+dwWidthContainer+2] += R * qy[1];
						}
			
						pSrcBits += 2;
						if (nx >= dwSrcWidth)
							nx -= dwSrcWidth;
						if (nx == 0)
							j += 3;
					}
				}
				else
				{
					for (x = 0 ; x < dwSrcWidth ; x++)
					{
						nx += dwNewWidth;
						DIB16ToRGB(*((WORD*)pSrcBits),&R,&G,&B);
 
						if (nx > dwSrcWidth)
						{
							qx[0] = 1.0 - (qx[1] = (nx - dwSrcWidth) / (double)dwNewWidth);
							
							f[j] += B * qx[0];
							f[(j++)+3] += B * qx[1];
							f[j] += G * qx[0];
							f[(j++)+3] += G * qx[1];
							f[j] += R * qx[0];
							f[(j++)+3] += R * qx[1];
						}
						else
						{
							f[j] += B;
							f[j+1] += G;
							f[j+2] += R;
						}
 
						pSrcBits += 2;
						if (nx >= dwSrcWidth)
							nx -= dwSrcWidth;
						if (nx == 0)
							j += 3;
					}

					if (ny < dwSrcHeight)
						j -= dwWidthContainer;
				}

				if (ny >= dwSrcHeight)
					ny -= dwSrcHeight;
			}
 
			// Copy From Floating-Point Picture to the new bits
			DWORD dwSrc = 0;
			LPBYTE pDstBits = m_pBits;
			for (y = 0 ; y < dwNewHeight ; y++)
			{ 
				for (j = 0 ; j < dwNewWidth ; j++)
				{
					B = (BYTE)Round(f[dwSrc++] * dAreaRatio);
					G = (BYTE)Round(f[dwSrc++] * dAreaRatio);
					R = (BYTE)Round(f[dwSrc++] * dAreaRatio);
					((WORD*)pDstBits)[j] = RGBToDIB16(R,G,B);
				}
				pDstBits += uiDIBTargetScanLineSize;
			}

			break;
		}
		case 24:
		{
			// Calculate Floating-Point Picture
			for (y = 0 ; y < dwSrcHeight ; y++)
			{
				if (pThread && pThread->DoExit())
				{
					BIGFREE(f);
					DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
					return FALSE;
				}
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, dwSrcHeight);

				pSrcBits = pNextSrcLineBits;
				pNextSrcLineBits += uiDIBSourceScanLineSize;
				nx = 0;
				ny += dwNewHeight;

				if (ny > dwSrcHeight)
				{
					qy[0] = 1.0 - (qy[1] = (ny - dwSrcHeight) / (double)dwNewHeight);

					for (x = 0 ; x < dwSrcWidth ; x++)
					{
						nx += dwNewWidth;

						if (nx > dwSrcWidth)
						{
							qx[0] = 1.0 - (qx[1] = (nx - dwSrcWidth) / (double)dwNewWidth);

							for (i = 0 ; i < 4 ; i++)	
								q[i] = qx[i&1] * qy[i>>1];

							for (i = 0 ; i < 3 ; i++)
							{
								f[j] += (*pSrcBits) * q[0]; 
								f[j+3] += (*pSrcBits) * q[1];
								f[j+dwWidthContainer] += (*pSrcBits) * q[2];
								f[(j++)+dwWidthContainer+3] += (*(pSrcBits++)) * q[3];
							}
						}
						else 
						{
							for (i = 0 ; i < 3 ; i++)
							{
								f[j+i] += (*pSrcBits) * qy[0];
								f[j+dwWidthContainer+i] += (*(pSrcBits++)) * qy[1];
							}
						}

						if (nx >= dwSrcWidth)
							nx -= dwSrcWidth;
						if (nx == 0)
							j += 3;
					}
				}
				else
				{
					for (x = 0 ; x < dwSrcWidth ; x++)
					{
						nx += dwNewWidth;
 
						if (nx > dwSrcWidth)
						{
							qx[0] = 1.0 - (qx[1] = (nx - dwSrcWidth) / (double)dwNewWidth);
							for (i = 0 ; i < 3 ; i++)
							{
								f[j] += (*pSrcBits) * qx[0];
								f[(j++)+3] += (*(pSrcBits++)) * qx[1];
							}
						}
						else 
						{
							for (i = 0 ; i < 3 ; i++)
								f[j+i] += *(pSrcBits++);
						}
 
						if (nx >= dwSrcWidth)
							nx -= dwSrcWidth;
						if (nx == 0)
							j += 3;
					}

					if (ny < dwSrcHeight)
						j -= dwWidthContainer;
				}

				if (ny >= dwSrcHeight)
					ny -= dwSrcHeight;
			}
 
			// Copy From Floating-Point Picture to the new bits
			DWORD dwSrc = 0;
			LPBYTE pDstBits = m_pBits;
			for (y = 0 ; y < dwNewHeight ; y++)
			{ 
				for (j = 0 ; j < dwWidthContainer ; j++)
					pDstBits[j] = (BYTE)Round(f[dwSrc++] * dAreaRatio);
				pDstBits += uiDIBTargetScanLineSize;
			}

			break;
		}
		case 32:
		{
			if (pSrcDib->HasAlpha())
			{
				// Calculate Floating-Point Picture
				for (y = 0 ; y < dwSrcHeight ; y++)
				{
					if (pThread && pThread->DoExit())
					{
						BIGFREE(f);
						DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
						return FALSE;
					}
					DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, dwSrcHeight);

					pSrcBits = pNextSrcLineBits;
					pNextSrcLineBits += uiDIBSourceScanLineSize;
					nx = 0;
					ny += dwNewHeight;

					if (ny > dwSrcHeight)
					{
						qy[0] = 1.0 - (qy[1] = (ny - dwSrcHeight) / (double)dwNewHeight);

						for (x = 0 ; x < dwSrcWidth ; x++)
						{
							nx += dwNewWidth;

							if (nx > dwSrcWidth)
							{
								qx[0] = 1.0 - (qx[1] = (nx - dwSrcWidth) / (double)dwNewWidth);

								for (i = 0 ; i < 4 ; i++)	
									q[i] = qx[i&1] * qy[i>>1];

								for (i = 0 ; i < 4 ; i++)
								{
									f[j] += (*pSrcBits) * q[0]; 
									f[j+4] += (*pSrcBits) * q[1];
									f[j+dwWidthContainer] += (*pSrcBits) * q[2];
									f[(j++)+dwWidthContainer+4] += (*(pSrcBits++)) * q[3];
								}
							}
							else 
							{
								for (i = 0 ; i < 4 ; i++)
								{
									f[j+i] += (*pSrcBits) * qy[0];
									f[j+dwWidthContainer+i] += (*(pSrcBits++)) * qy[1];
								}
							}

							if (nx >= dwSrcWidth)
								nx -= dwSrcWidth;
							if (nx == 0)
								j += 4;
						}
					}
					else
					{
						for (x = 0 ; x < dwSrcWidth ; x++)
						{
							nx += dwNewWidth;
 
							if (nx > dwSrcWidth)
							{
								qx[0] = 1.0 - (qx[1] = (nx - dwSrcWidth) / (double)dwNewWidth);
								for (i = 0 ; i < 4 ; i++)
								{
									f[j] += (*pSrcBits) * qx[0];
									f[(j++)+4] += (*(pSrcBits++)) * qx[1];
								}
							}
							else 
							{
								for (i = 0 ; i < 4 ; i++)
									f[j+i] += *(pSrcBits++);
							}
 
							if (nx >= dwSrcWidth)
								nx -= dwSrcWidth;
							if (nx == 0)
								j += 4;
						}

						if (ny < dwSrcHeight)
							j -= dwWidthContainer;
					}

					if (ny >= dwSrcHeight)
						ny -= dwSrcHeight;
				}
 
				// Copy From Floating-Point Picture to the new bits
				DWORD dwSrc = 0;
				LPBYTE pDstBits = m_pBits;
				for (y = 0 ; y < dwNewHeight ; y++)
				{ 
					for (j = 0 ; j < dwWidthContainer ; j++)
						pDstBits[j] = (BYTE)Round(f[dwSrc++] * dAreaRatio);
					pDstBits += uiDIBTargetScanLineSize;
				}
			}
			else if (pSrcDib->IsFast32bpp())
			{
				// Calculate Floating-Point Picture
				for (y = 0 ; y < dwSrcHeight ; y++)
				{
					if (pThread && pThread->DoExit())
					{
						BIGFREE(f);
						DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
						return FALSE;
					}
					DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, dwSrcHeight);

					pSrcBits = pNextSrcLineBits;
					pNextSrcLineBits += uiDIBSourceScanLineSize;
					nx = 0;
					ny += dwNewHeight;

					if (ny > dwSrcHeight)
					{
						qy[0] = 1.0 - (qy[1] = (ny - dwSrcHeight) / (double)dwNewHeight);

						for (x = 0 ; x < dwSrcWidth ; x++)
						{
							nx += dwNewWidth;

							if (nx > dwSrcWidth)
							{
								qx[0] = 1.0 - (qx[1] = (nx - dwSrcWidth) / (double)dwNewWidth);

								for (i = 0 ; i < 4 ; i++)	
									q[i] = qx[i&1] * qy[i>>1];

								for (i = 0 ; i < 3 ; i++)
								{
									f[j] += (*pSrcBits) * q[0]; 
									f[j+3] += (*pSrcBits) * q[1];
									f[j+dwWidthContainer] += (*pSrcBits) * q[2];
									f[(j++)+dwWidthContainer+3] += (*(pSrcBits++)) * q[3];
								}
								pSrcBits++; // Skip Alpha
							}
							else 
							{
								for (i = 0 ; i < 3 ; i++)
								{
									f[j+i] += (*pSrcBits) * qy[0];
									f[j+dwWidthContainer+i] += (*(pSrcBits++)) * qy[1];
								}
								pSrcBits++; // Skip Alpha
							}

							if (nx >= dwSrcWidth)
								nx -= dwSrcWidth;
							if (nx == 0)
								j += 3;
						}
					}
					else
					{
						for (x = 0 ; x < dwSrcWidth ; x++)
						{
							nx += dwNewWidth;
 
							if (nx > dwSrcWidth)
							{
								qx[0] = 1.0 - (qx[1] = (nx - dwSrcWidth) / (double)dwNewWidth);
								for (i = 0 ; i < 3 ; i++)
								{
									f[j] += (*pSrcBits) * qx[0];
									f[(j++)+3] += (*(pSrcBits++)) * qx[1];
								}
								pSrcBits++; // Skip Alpha
							}
							else 
							{
								for (i = 0 ; i < 3 ; i++)
									f[j+i] += *(pSrcBits++);
								pSrcBits++; // Skip Alpha
							}
 
							if (nx >= dwSrcWidth)
								nx -= dwSrcWidth;
							if (nx == 0)
								j += 3;
						}

						if (ny < dwSrcHeight)
							j -= dwWidthContainer;
					}

					if (ny >= dwSrcHeight)
						ny -= dwSrcHeight;
				}
 
				// Copy From Floating-Point Picture to the new bits
				DWORD dwSrc = 0;
				LPBYTE pDstBits = m_pBits;
				for (y = 0 ; y < dwNewHeight ; y++)
				{ 
					for (j = 0 ; j < dwNewWidth ; j++)
					{
						pDstBits[4*j]   = (BYTE)Round(f[dwSrc++] * dAreaRatio);
						pDstBits[4*j+1] = (BYTE)Round(f[dwSrc++] * dAreaRatio);
						pDstBits[4*j+2] = (BYTE)Round(f[dwSrc++] * dAreaRatio);
					}
					pDstBits += uiDIBTargetScanLineSize;
				}
			}
			else
			{
				// Calculate Floating-Point Picture
				for (y = 0 ; y < dwSrcHeight ; y++)
				{
					if (pThread && pThread->DoExit())
					{
						BIGFREE(f);
						DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
						return FALSE;
					}
					DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, dwSrcHeight);

					// Src Pixels Pointer
					pSrcBits = pNextSrcLineBits;
					pNextSrcLineBits += uiDIBSourceScanLineSize;

					// Reset nx
					nx = 0;

					// Inc. ny
					ny += dwNewHeight;

					if (ny > dwSrcHeight)
					{
						// Calc. contribution amounts in vertical direction
						qy[1] = (double)(ny - dwSrcHeight) / (double)dwNewHeight;
						qy[0] = 1.0 - qy[1];

						for (x = 0 ; x < dwSrcWidth ; x++)
						{
							// Inc. nx
							nx += dwNewWidth;

							// Get Src Pixel
							DIB32ToRGB(*((DWORD*)pSrcBits),&R,&G,&B);

							// Src Pixel contribution to 4 Dst Pixels (to square region)
							if (nx > dwSrcWidth)
							{
								// Calc. contribution amounts in horizontal direction
								qx[1] = (double)(nx - dwSrcWidth) / (double)dwNewWidth;
								qx[0] = 1.0 - qx[1];

								// Calc. contribution amounts of square region
								// q[0] = qx[0] * qy[0]
								// q[1] = qx[1] * qy[0]
								// q[2] = qx[0] * qy[1]
								// q[3] = qx[1] * qy[1]
								for (i = 0 ; i < 4 ; i++)	
									q[i] = qx[i&1] * qy[i>>1];
		
								f[j]					+= B * q[0];
								f[j+3]					+= B * q[1];
								f[j+dwWidthContainer]	+= B * q[2];
								f[j+dwWidthContainer+3]	+= B * q[3];
								j++;

								f[j]					+= G * q[0];
								f[j+3]					+= G * q[1];
								f[j+dwWidthContainer]	+= G * q[2];
								f[j+dwWidthContainer+3]	+= G * q[3];
								j++;

								f[j]					+= R * q[0];
								f[j+3]					+= R * q[1];
								f[j+dwWidthContainer]	+= R * q[2];
								f[j+dwWidthContainer+3]	+= R * q[3];
								j++;
							}
							// Src Pixel contribution to 2 vertical Dst Pixels
							else
							{
								f[j]						+= B * qy[0];
								f[j+dwWidthContainer]		+= B * qy[1];

								f[j+1]						+= G * qy[0];
								f[j+dwWidthContainer+1]		+= G * qy[1];

								f[j+2]						+= R * qy[0];
								f[j+dwWidthContainer+2]		+= R * qy[1];
							}
				
							// Inc. Src Pixel Pos
							pSrcBits += 4;

							// Adjust nx, time to go to the next Dst Pixel
							if (nx >= dwSrcWidth)
								nx -= dwSrcWidth;

							// Inc. Dst Pixel pos (nx was equal to dwSrcWidth)
							if (nx == 0)
								j += 3;
						}
					}
					else
					{
						for (x = 0 ; x < dwSrcWidth ; x++)
						{
							// Inc. nx
							nx += dwNewWidth;

							// Get Src Pixel
							DIB32ToRGB(*((DWORD*)pSrcBits),&R,&G,&B);
 
							// Src Pixel contribution to 2 horizontal Dst Pixels
							if (nx > dwSrcWidth)
							{
								// Calc. contribution amounts in horizontal direction
								qx[1] = (double)(nx - dwSrcWidth) / (double)dwNewWidth;
								qx[0] = 1.0 - qx[1];
								
								f[j]	+= B * qx[0];
								f[j+3]	+= B * qx[1];
								j++;

								f[j]	+= G * qx[0];
								f[j+3]	+= G * qx[1];
								j++;

								f[j]	+= R * qx[0];
								f[j+3]	+= R * qx[1];
								j++;
							}
							// Src Pixel contribution to 1 Dst Pixel
							else
							{
								f[j]	+= B;
								f[j+1]	+= G;
								f[j+2]	+= R;
							}
 
							// Inc. Src Pixel Pos
							pSrcBits += 4;

							// Adjust nx, time to go to the next Dst Pixel
							if (nx >= dwSrcWidth)
								nx -= dwSrcWidth;

							// Inc. Dst Pixel pos (nx was equal to dwSrcWidth)
							if (nx == 0)
								j += 3;
						}

						// Reset Dst Line (contributions to this line not yet finished!)
						if (ny < dwSrcHeight)
							j -= dwWidthContainer;
					}

					// Adjust ny
					if (ny >= dwSrcHeight)
						ny -= dwSrcHeight;
				}
 
				// Copy From Floating-Point Picture to the new bits
				DWORD dwSrc = 0;
				LPBYTE pDstBits = m_pBits;
				for (y = 0 ; y < dwNewHeight ; y++)
				{ 
					for (j = 0 ; j < dwNewWidth ; j++)
					{
						B = (BYTE)Round(f[dwSrc++] * dAreaRatio);
						G = (BYTE)Round(f[dwSrc++] * dAreaRatio);
						R = (BYTE)Round(f[dwSrc++] * dAreaRatio);
						((DWORD*)pDstBits)[j] = RGBToDIB32(R,G,B);
					}
					pDstBits += uiDIBTargetScanLineSize;
				}
			}
			break;
		}
		default : break;
	}

	// Create Palette
	CreatePaletteFromBMI();

	// Free
	BIGFREE(f);

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
		if (g_bSSE)
		{
			if (!BicubicResample24_SSE(	(int)dwNewWidth,
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
		else
		{
			if (!BicubicResample24_C(	(int)dwNewWidth,
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
		if (g_bSSE)
		{
			if (!BicubicResample32_SSE(	(int)dwNewWidth,
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
		else
		{
			if (!BicubicResample32_C(	(int)dwNewWidth,
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

typedef __declspec(align(16)) union
{
	struct
	{
		float f0;
		float f1;
		float f2;
		float f3;
	} f;
	struct
	{
		DWORD dw0;
		DWORD dw1;
		DWORD dw2;
		DWORD dw3;
	} dw;
} SSETYPE;

BOOL CDib::BicubicResample24_SSE(	int nNewWidth,
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
	int nDIBTargetScanLineSize = DWALIGNEDWIDTHBYTES(nNewWidth * 24);
	const float const_0_5 = 0.5f;
	int i, x, y, i_x, i_y, xx, yy;
	int nFloatSrcScanLineSizeBytes = nFloatSrcScanLineSize << 2;
	DWORD dwPix;
	DWORD kernel_b;
	float* pf;

	SSETYPE absconst;
	ASSERT(((DWORD)&absconst % 16U) == 0);
	absconst.dw.dw0 = 0x7FFFFFFFU;
	absconst.dw.dw1 = 0x7FFFFFFFU;
	absconst.dw.dw2 = 0x7FFFFFFFU;
	absconst.dw.dw3 = 0x7FFFFFFFU;

	SSETYPE kernconst;
	ASSERT(((DWORD)&kernconst % 16U) == 0);
	kernconst.f.f0 = -1.0f;
	kernconst.f.f1 = 0.0f;
	kernconst.f.f2 = 1.0f;
	kernconst.f.f3 = 2.0f;

	SSETYPE kernconst_minus_0_5;
	ASSERT(((DWORD)&kernconst_minus_0_5 % 16U) == 0);
	kernconst_minus_0_5.f.f0 = -0.5f;
	kernconst_minus_0_5.f.f1 = -0.5f;
	kernconst_minus_0_5.f.f2 = -0.5f;
	kernconst_minus_0_5.f.f3 = -0.5f;

	SSETYPE kernconst_1;
	ASSERT(((DWORD)&kernconst_1 % 16U) == 0);
	kernconst_1.f.f0 = 1.0f;
	kernconst_1.f.f1 = 1.0f;
	kernconst_1.f.f2 = 1.0f;
	kernconst_1.f.f3 = 1.0f;

	SSETYPE kernconst_1_5;
	ASSERT(((DWORD)&kernconst_1_5 % 16U) == 0);
	kernconst_1_5.f.f0 = 1.5f;
	kernconst_1_5.f.f1 = 1.5f;
	kernconst_1_5.f.f2 = 1.5f;
	kernconst_1_5.f.f3 = 1.5f;

	SSETYPE kernconst_2;
	ASSERT(((DWORD)&kernconst_2 % 16U) == 0);
	kernconst_2.f.f0 = 2.0f;
	kernconst_2.f.f1 = 2.0f;
	kernconst_2.f.f2 = 2.0f;
	kernconst_2.f.f3 = 2.0f;

	SSETYPE kernconst_minus_2_5;
	ASSERT(((DWORD)&kernconst_minus_2_5 % 16U) == 0);
	kernconst_minus_2_5.f.f0 = -2.5f;
	kernconst_minus_2_5.f.f1 = -2.5f;
	kernconst_minus_2_5.f.f2 = -2.5f;
	kernconst_minus_2_5.f.f3 = -2.5f;

	SSETYPE kernconst_minus_4;
	ASSERT(((DWORD)&kernconst_minus_4 % 16U) == 0);
	kernconst_minus_4.f.f0 = -4.0f;
	kernconst_minus_4.f.f1 = -4.0f;
	kernconst_minus_4.f.f2 = -4.0f;
	kernconst_minus_4.f.f3 = -4.0f;

	float* array_kernel_b = (float*)BIGALLOC(nNewWidth * 4 * sizeof(float));
	if (!array_kernel_b)
		return FALSE;

	for (x = 0 , kernel_b = (DWORD)array_kernel_b ; x < nNewWidth ; x++ , kernel_b += 16U)
	{
		__asm
		{
			//float f_x = (x + 0.5f) * xScale;
			//i_x = RoundF(f_x); // Center X coord. of the 16 source pixels
			//float b   = f_x - (float)i_x + 0.5f;
			cvtsi2ss	xmm0,	x
			addss		xmm0,	const_0_5
			mulss		xmm0,	xScale		; xmm0 is now f_x
			cvtss2si	ecx,	xmm0		; ecx is now i_x
			mov			i_x,	ecx
			cvtsi2ss	xmm1,	ecx			; xmm1 is now (float)i_x
			subss		xmm0,	xmm1
			addss		xmm0,	const_0_5	; xmm0 is b
			shufps		xmm0,	xmm0, 0		; xmm0 is now: b b b b

			/*
			float abs_t = fabs(t);
			if (|t| < 1.0f)
			{
				return (1.0f - (c+3.0f)*(t^2) + (c+2.0f)*(|t|^3));
			}
			if (|t| < 2.0f)
			{
				return (-4.0f*c + (8.0f*c)*(|t|) - (5.0f*c)*(t^2) + c*(|t|^3));
			}
			return 0.0f;

			c = -0.5f
			---------

			float abs_t = fabs(t);
			if (|t| < 1.0f)
			{
				return (1.0f + (-2.5f)*(t^2) + (1.5f)*(|t|^3));
			}
			if (|t| < 2.0f)
			{
				return (2.0f + (-4.0f)*(|t|) + (2.5f)*(t^2) + (-0.5f)*(|t|^3));
			}
			return 0.0f;
			*/
		
			;t
			movaps xmm1, kernconst
			subps  xmm1, xmm0			; xmm1 is now: -1.0f-b -b 1.0f-b 2.0f-b
			
			;|t| = xmm1, xmm2 and xmm7
			andps  xmm1, absconst
			movaps xmm2, xmm1
			movaps xmm7, xmm2

			;t^2 = xmm3
			movaps xmm3, xmm2
			mulps  xmm3, xmm3

			;|t|^3 = xmm5 and xmm4
			movaps xmm5, xmm1
			mulps  xmm5, xmm3
			movaps xmm4, xmm5

			;
			; xmm5 will contain the calculation for |t| < 1.0f 
			;

			; xmm5 = (1.5f)*(|t|^3) 
			mulps  xmm5, kernconst_1_5
			
			; xmm3 = (-2.5f)*(t^2)
			mulps  xmm3, kernconst_minus_2_5

			; xmm5 = (-2.5f)*(t^2) + (1.5f)*(|t|^3)
			addps  xmm5, xmm3

			; xmm5 = 1.0f + (-2.5f)*(t^2) + (1.5f)*(|t|^3)
			addps  xmm5, kernconst_1

			;
			; xmm4 will contain the calculation for |t| < 2.0f 
			;

			; xmm4 = (-0.5f)*(|t|^3)
			mulps  xmm4, kernconst_minus_0_5
			
			; xmm3 = (-2.5f)*(t^2) (from above calculation)
			; xmm4 = (2.5f)*(t^2) + (-0.5f)*(|t|^3)
			subps  xmm4, xmm3

			; xmm2 = (-4.0f)*(|t|)
			mulps  xmm2, kernconst_minus_4

			; xmm4 = (-4.0f)*(|t|) + (2.5f)*(t^2) + (-0.5f)*(|t|^3)
			addps  xmm4, xmm2

			; xmm4 = 2.0f + (-4.0f)*(|t|) + (2.5f)*(t^2) + (-0.5f)*(|t|^3)
			addps  xmm4, kernconst_2

			;
			; Merging the 2 calculations
			;

			; xmm4 contains the result for |t| < 2.0f, otherwise 0.0f
			cmpltps xmm1, kernconst_2
			andps	xmm4, xmm1

			; xmm5 contains the result for |t| < 1.0f, otherwise 0.0f
			; xmm7 contains the result for 1.0f <= |t| < 2.0f, otherwise 0.0f
			cmpltps xmm7, kernconst_1
			andps	xmm5, xmm7
			andnps	xmm7, xmm4
			
			; xmm7 contains the final result
			orps	xmm7, xmm5

			; store into array
			mov		ecx,  kernel_b
			movaps	[ecx], xmm7
		}
	}

	DIB_INIT_PROGRESS;

	for (y = 0 ; y < nNewHeight ; y++)
	{
		__asm emms; // End MMX instructions, prep for possible FP instrs.
		if (pThread && pThread->DoExit())
		{
			BIGFREE(array_kernel_b);
			DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
			return FALSE;
		}
		// This uses floats -> the above emms is necessary
		DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, nNewHeight);

		__asm
		{
			//float f_y = (y + 0.5f) * yScale;
			//i_y = RoundF(f_y); // Center Y coord. of the 16 source pixels
			//float a   = f_y - (float)i_y + 0.5f;
			cvtsi2ss	xmm0,	y
			addss		xmm0,	const_0_5
			mulss		xmm0,	yScale		; xmm0 is now f_y
			cvtss2si	ecx,	xmm0		; ecx is now i_y
			mov			i_y,	ecx
			cvtsi2ss	xmm1,	ecx			; xmm1 is now (float)i_y
			subss		xmm0,	xmm1
			addss		xmm0,	const_0_5	; xmm0 is now a
			shufps		xmm0,	xmm0, 0		; xmm0 is now: a a a a

			/*
			float abs_t = fabs(t);
			if (|t| < 1.0f)
			{
				return (1.0f - (c+3.0f)*(t^2) + (c+2.0f)*(|t|^3));
			}
			if (|t| < 2.0f)
			{
				return (-4.0f*c + (8.0f*c)*(|t|) - (5.0f*c)*(t^2) + c*(|t|^3));
			}
			return 0.0f;

			c = -0.5f
			---------

			float abs_t = fabs(t);
			if (|t| < 1.0f)
			{
				return (1.0f + (-2.5f)*(t^2) + (1.5f)*(|t|^3));
			}
			if (|t| < 2.0f)
			{
				return (2.0f + (-4.0f)*(|t|) + (2.5f)*(t^2) + (-0.5f)*(|t|^3));
			}
			return 0.0f;
			*/

			;t
			movaps xmm1, kernconst
			subps  xmm1, xmm0			; xmm1 is now: -1.0f-a -a 1.0f-a 2.0f-a
			
			;|t| = xmm1, xmm2 and xmm6
			andps  xmm1, absconst
			movaps xmm2, xmm1
			movaps xmm6, xmm2

			;t^2 = xmm3
			movaps xmm3, xmm2
			mulps  xmm3, xmm3

			;|t|^3 = xmm5 and xmm4
			movaps xmm5, xmm1
			mulps  xmm5, xmm3
			movaps xmm4, xmm5

			;
			; xmm5 will contain the calculation for |t| < 1.0f 
			;

			; xmm5 = (1.5f)*(|t|^3) 
			mulps  xmm5, kernconst_1_5
			
			; xmm3 = (-2.5f)*(t^2)
			mulps  xmm3, kernconst_minus_2_5

			; xmm5 = (-2.5f)*(t^2) + (1.5f)*(|t|^3)
			addps  xmm5, xmm3

			; xmm5 = 1.0f + (-2.5f)*(t^2) + (1.5f)*(|t|^3)
			addps  xmm5, kernconst_1

			;
			; xmm4 will contain the calculation for |t| < 2.0f 
			;

			; xmm4 = (-0.5f)*(|t|^3)
			mulps  xmm4, kernconst_minus_0_5

			; xmm3 = (-2.5f)*(t^2) (from above calculation)
			; xmm4 = (2.5f)*(t^2) + (-0.5f)*(|t|^3)
			subps  xmm4, xmm3

			; xmm2 = (-4.0f)*(|t|)
			mulps  xmm2, kernconst_minus_4

			; xmm4 = (-4.0f)*(|t|) + (2.5f)*(t^2) + (-0.5f)*(|t|^3)
			addps  xmm4, xmm2

			; xmm4 = 2.0f + (-4.0f)*(|t|) + (2.5f)*(t^2) + (-0.5f)*(|t|^3)
			addps  xmm4, kernconst_2

			;
			; Merging the 2 calculations
			;

			; xmm4 contains the result for |t| < 2.0f, otherwise 0.0f
			cmpltps xmm1, kernconst_2
			andps	xmm4, xmm1

			; xmm5 contains the result for |t| < 1.0f, otherwise 0.0f
			; xmm6 contains the result for 1.0f <= |t| < 2.0f, otherwise 0.0f
			cmpltps xmm6, kernconst_1
			andps	xmm5, xmm6
			andnps	xmm6, xmm4
			
			; xmm6 contains the final result
			orps	xmm6, xmm5
		}

		for (x = 0 , kernel_b = (DWORD)array_kernel_b ; x < nNewWidth ; x++ , kernel_b += 16U)
		{
			//float f_x = (x + 0.5f) * xScale;
			//i_x = RoundF(f_x); // Center X coord. of the 16 source pixels
			__asm
			{
				cvtsi2ss	xmm0,	x
				addss		xmm0,	const_0_5
				mulss		xmm0,	xScale		; xmm0 is now f_x
				cvtss2si	ecx,	xmm0		; ecx is now i_x
				mov			i_x,	ecx
			}

			xx = i_x - 2;
			yy = i_y - 2;
			yy = (yy + BICUBIC_PIXMARGIN) * nFloatSrcScanLineSize;
			i = yy + ((xx + BICUBIC_PIXMARGIN) << 2);
			pf = f + i;

			__asm
			{	
				mov	   ecx,  kernel_b
				movaps xmm7, [ecx]				; xmm7 is now: r2_1 r2_2 r2_3 r2_4

				; xmm0 is for bb gg rr aa, reset it
				xorps	xmm0, xmm0

				;
				; m = -1
				;
				mov ecx, pf

				movaps xmm2, xmm6				; xmm2 is now: r1_1 r1_2 r1_3 r1_4
				shufps xmm2, xmm2, 0			; xmm2 is now: r1_1 r1_1 r1_1 r1_1
				
				mulps  xmm2, xmm7				; xmm2 is now: r1 r2 r3 r4
				movaps xmm3, xmm2				; xmm3 is now: r1 r2 r3 r4
				movaps xmm4, xmm3				; xmm4 is now: r1 r2 r3 r4
				movaps xmm5, xmm4				; xmm5 is now: r1 r2 r3 r4

				shufps xmm2, xmm2, 0			; xmm2 is now: r1 r1 r1 r1
				shufps xmm3, xmm3, 01010101b	; xmm3 is now: r2 r2 r2 r2
				shufps xmm4, xmm4, 10101010b	; xmm4 is now: r3 r3 r3 r3
				shufps xmm5, xmm5, 11111111b	; xmm5 is now: r4 r4 r4 r4

				movaps xmm1, [ecx]				; xmm1 is now: b g r a
				mulps  xmm1, xmm2				; xmm1 is now: r1*b r1*g r1*r r1*a 
				addps  xmm0, xmm1				; xmm0 is now: bb+r1*b gg+r1*g rr+r1*r aa+r1*a 

				movaps xmm1, [ecx+16]			; xmm1 is now: b g r a
				mulps  xmm1, xmm3				; xmm1 is now: r2*b r2*g r2*r r2*a 
				addps  xmm0, xmm1				; xmm0 is now: bb+r2*b gg+r2*g rr+r2*r aa+r2*a

				movaps xmm1, [ecx+32]			; xmm1 is now: b g r a
				mulps  xmm1, xmm4				; xmm1 is now: r3*b r3*g r3*r r3*a 
				addps  xmm0, xmm1				; xmm0 is now: bb+r3*b gg+r3*g rr+r3*r aa+r3*a

				movaps xmm1, [ecx+48]			; xmm1 is now: b g r a
				mulps  xmm1, xmm5				; xmm1 is now: r4*b r4*g r4*r r4*a 
				addps  xmm0, xmm1				; xmm0 is now: bb+r4*b gg+r4*g rr+r4*r aa+r4*a

				;
				; m = 0
				;
				add	ecx, nFloatSrcScanLineSizeBytes
			
				movaps xmm2, xmm6				; xmm2 is now: r1_1 r1_2 r1_3 r1_4
				shufps xmm2, xmm2, 01010101b	; xmm2 is now: r1_2 r1_2 r1_2 r1_2

				mulps  xmm2, xmm7				; xmm2 is now: r1 r2 r3 r4
				movaps xmm3, xmm2				; xmm3 is now: r1 r2 r3 r4
				movaps xmm4, xmm3				; xmm4 is now: r1 r2 r3 r4
				movaps xmm5, xmm4				; xmm5 is now: r1 r2 r3 r4

				shufps xmm2, xmm2, 0			; xmm2 is now: r1 r1 r1 r1
				shufps xmm3, xmm3, 01010101b	; xmm3 is now: r2 r2 r2 r2
				shufps xmm4, xmm4, 10101010b	; xmm4 is now: r3 r3 r3 r3
				shufps xmm5, xmm5, 11111111b	; xmm5 is now: r4 r4 r4 r4

				movaps xmm1, [ecx]				; xmm1 is now: b g r a
				mulps  xmm1, xmm2				; xmm1 is now: r1*b r1*g r1*r r1*a 
				addps  xmm0, xmm1				; xmm0 is now: bb+r1*b gg+r1*g rr+r1*r aa+r1*a 

				movaps xmm1, [ecx+16]			; xmm1 is now: b g r a
				mulps  xmm1, xmm3				; xmm1 is now: r2*b r2*g r2*r r2*a 
				addps  xmm0, xmm1				; xmm0 is now: bb+r2*b gg+r2*g rr+r2*r aa+r2*a

				movaps xmm1, [ecx+32]			; xmm1 is now: b g r a
				mulps  xmm1, xmm4				; xmm1 is now: r3*b r3*g r3*r r3*a 
				addps  xmm0, xmm1				; xmm0 is now: bb+r3*b gg+r3*g rr+r3*r aa+r3*a

				movaps xmm1, [ecx+48]			; xmm1 is now: b g r a
				mulps  xmm1, xmm5				; xmm1 is now: r4*b r4*g r4*r r4*a 
				addps  xmm0, xmm1				; xmm0 is now: bb+r4*b gg+r4*g rr+r4*r aa+r4*a

				;
				; m = 1
				;
				add	ecx, nFloatSrcScanLineSizeBytes

				movaps xmm2, xmm6				; xmm2 is now: r1_1 r1_2 r1_3 r1_4
				shufps xmm2, xmm2, 10101010b	; xmm2 is now: r1_3 r1_3 r1_3 r1_3

				mulps  xmm2, xmm7				; xmm2 is now: r1 r2 r3 r4
				movaps xmm3, xmm2				; xmm3 is now: r1 r2 r3 r4
				movaps xmm4, xmm3				; xmm4 is now: r1 r2 r3 r4
				movaps xmm5, xmm4				; xmm5 is now: r1 r2 r3 r4

				shufps xmm2, xmm2, 0			; xmm2 is now: r1 r1 r1 r1
				shufps xmm3, xmm3, 01010101b	; xmm3 is now: r2 r2 r2 r2
				shufps xmm4, xmm4, 10101010b	; xmm4 is now: r3 r3 r3 r3
				shufps xmm5, xmm5, 11111111b	; xmm5 is now: r4 r4 r4 r4

				movaps xmm1, [ecx]				; xmm1 is now: b g r a
				mulps  xmm1, xmm2				; xmm1 is now: r1*b r1*g r1*r r1*a 
				addps  xmm0, xmm1				; xmm0 is now: bb+r1*b gg+r1*g rr+r1*r aa+r1*a

				movaps xmm1, [ecx+16]			; xmm1 is now: b g r a
				mulps  xmm1, xmm3				; xmm1 is now: r2*b r2*g r2*r r2*a 
				addps  xmm0, xmm1				; xmm0 is now: bb+r2*b gg+r2*g rr+r2*r aa+r2*a

				movaps xmm1, [ecx+32]			; xmm1 is now: b g r a
				mulps  xmm1, xmm4				; xmm1 is now: r3*b r3*g r3*r r3*a 
				addps  xmm0, xmm1				; xmm0 is now: bb+r3*b gg+r3*g rr+r3*r aa+r3*a

				movaps xmm1, [ecx+48]			; xmm1 is now: b g r a
				mulps  xmm1, xmm5				; xmm1 is now: r4*b r4*g r4*r r4*a 
				addps  xmm0, xmm1				; xmm0 is now: bb+r4*b gg+r4*g rr+r4*r aa+r4*a
				
				;
				; m = 2
				;
				add	ecx, nFloatSrcScanLineSizeBytes

				movaps xmm2, xmm6				; xmm2 is now: r1_1 r1_2 r1_3 r1_4
				shufps xmm2, xmm2, 11111111b	; xmm2 is now: r1_4 r1_4 r1_4 r1_4

				mulps  xmm2, xmm7				; xmm2 is now: r1 r2 r3 r4
				movaps xmm3, xmm2				; xmm3 is now: r1 r2 r3 r4
				movaps xmm4, xmm3				; xmm4 is now: r1 r2 r3 r4
				movaps xmm5, xmm4				; xmm5 is now: r1 r2 r3 r4

				shufps xmm2, xmm2, 0			; xmm2 is now: r1 r1 r1 r1
				shufps xmm3, xmm3, 01010101b	; xmm3 is now: r2 r2 r2 r2
				shufps xmm4, xmm4, 10101010b	; xmm4 is now: r3 r3 r3 r3
				shufps xmm5, xmm5, 11111111b	; xmm5 is now: r4 r4 r4 r4

				movaps xmm1, [ecx]				; xmm1 is now: b g r a
				mulps  xmm1, xmm2				; xmm1 is now: r1*b r1*g r1*r r1*a
				addps  xmm0, xmm1				; xmm0 is now: bb+r1*b gg+r1*g rr+r1*r aa+r1*a 

				movaps xmm1, [ecx+16]			; xmm1 is now: b g r a
				mulps  xmm1, xmm3				; xmm1 is now: r2*b r2*g r2*r r2*a 
				addps  xmm0, xmm1				; xmm0 is now: bb+r2*b gg+r2*g rr+r2*r aa+r2*a

				movaps xmm1, [ecx+32]			; xmm1 is now: b g r a
				mulps  xmm1, xmm4				; xmm1 is now: r3*b r3*g r3*r r3*a 
				addps  xmm0, xmm1				; xmm0 is now: bb+r3*b gg+r3*g rr+r3*r aa+r3*a

				movaps xmm1, [ecx+48]			; xmm1 is now: b g r a
				mulps  xmm1, xmm5				; xmm1 is now: r4*b r4*g r4*r r4*a 
				addps  xmm0, xmm1				; xmm0 is now: bb+r4*b gg+r4*g rr+r4*r aa+r4*a

				; Convert with clamping to 0..255 from xmm0 to dwPix 
				; xmm0 is (0) bb gg rr aa (127)
				cvtps2pi  mm0,   xmm0			; (0) bb gg (63)
				shufps    xmm0,  xmm0, 01001110b
				cvtps2pi  mm1,   xmm0			; (0) rr aa (63)
				packssdw  mm0,   mm1			; (0) bb gg rr aa (63)
				packuswb  mm0,   mm0			; (0) bb gg rr aa bb gg rr aa (63) unsigned saturated!
				movd	  dwPix, mm0			; dwPix = (0) bb gg rr aa (31)
			}

			// Set Output Pixel
			pOutBits[3*x] = (BYTE)(dwPix & 0xFF);			// Blue
			pOutBits[3*x+1] = (BYTE)((dwPix>>8U) & 0xFF);	// Green
			pOutBits[3*x+2] = (BYTE)((dwPix>>16U) & 0xFF);	// Red
		}

		// Inc. Output Pixel Offset
		pOutBits += nDIBTargetScanLineSize;
	}

	__asm emms; // End MMX instructions, prep for possible FP instrs.
	BIGFREE(array_kernel_b);
	DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

	return TRUE;
}

BOOL CDib::BicubicResample32_SSE(	int nNewWidth,
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
	const float const_0_5 = 0.5f;
	int i, x, y, i_x, i_y, xx, yy;
	int nFloatSrcScanLineSizeBytes = nFloatSrcScanLineSize << 2;
	DWORD dwPix;
	DWORD kernel_b;
	float* pf;

	SSETYPE absconst;
	ASSERT(((DWORD)&absconst % 16U) == 0);
	absconst.dw.dw0 = 0x7FFFFFFFU;
	absconst.dw.dw1 = 0x7FFFFFFFU;
	absconst.dw.dw2 = 0x7FFFFFFFU;
	absconst.dw.dw3 = 0x7FFFFFFFU;

	SSETYPE kernconst;
	ASSERT(((DWORD)&kernconst % 16U) == 0);
	kernconst.f.f0 = -1.0f;
	kernconst.f.f1 = 0.0f;
	kernconst.f.f2 = 1.0f;
	kernconst.f.f3 = 2.0f;

	SSETYPE kernconst_minus_0_5;
	ASSERT(((DWORD)&kernconst_minus_0_5 % 16U) == 0);
	kernconst_minus_0_5.f.f0 = -0.5f;
	kernconst_minus_0_5.f.f1 = -0.5f;
	kernconst_minus_0_5.f.f2 = -0.5f;
	kernconst_minus_0_5.f.f3 = -0.5f;

	SSETYPE kernconst_1;
	ASSERT(((DWORD)&kernconst_1 % 16U) == 0);
	kernconst_1.f.f0 = 1.0f;
	kernconst_1.f.f1 = 1.0f;
	kernconst_1.f.f2 = 1.0f;
	kernconst_1.f.f3 = 1.0f;

	SSETYPE kernconst_1_5;
	ASSERT(((DWORD)&kernconst_1_5 % 16U) == 0);
	kernconst_1_5.f.f0 = 1.5f;
	kernconst_1_5.f.f1 = 1.5f;
	kernconst_1_5.f.f2 = 1.5f;
	kernconst_1_5.f.f3 = 1.5f;

	SSETYPE kernconst_2;
	ASSERT(((DWORD)&kernconst_2 % 16U) == 0);
	kernconst_2.f.f0 = 2.0f;
	kernconst_2.f.f1 = 2.0f;
	kernconst_2.f.f2 = 2.0f;
	kernconst_2.f.f3 = 2.0f;

	SSETYPE kernconst_minus_2_5;
	ASSERT(((DWORD)&kernconst_minus_2_5 % 16U) == 0);
	kernconst_minus_2_5.f.f0 = -2.5f;
	kernconst_minus_2_5.f.f1 = -2.5f;
	kernconst_minus_2_5.f.f2 = -2.5f;
	kernconst_minus_2_5.f.f3 = -2.5f;

	SSETYPE kernconst_minus_4;
	ASSERT(((DWORD)&kernconst_minus_4 % 16U) == 0);
	kernconst_minus_4.f.f0 = -4.0f;
	kernconst_minus_4.f.f1 = -4.0f;
	kernconst_minus_4.f.f2 = -4.0f;
	kernconst_minus_4.f.f3 = -4.0f;

	float* array_kernel_b = (float*)BIGALLOC(nNewWidth * 4 * sizeof(float));
	if (!array_kernel_b)
		return FALSE;

	for (x = 0 , kernel_b = (DWORD)array_kernel_b ; x < nNewWidth ; x++ , kernel_b += 16U)
	{
		__asm
		{
			//float f_x = (x + 0.5f) * xScale;
			//i_x = RoundF(f_x); // Center X coord. of the 16 source pixels
			//float b   = f_x - (float)i_x + 0.5f;
			cvtsi2ss	xmm0,	x
			addss		xmm0,	const_0_5
			mulss		xmm0,	xScale		; xmm0 is now f_x
			cvtss2si	ecx,	xmm0		; ecx is now i_x
			mov			i_x,	ecx
			cvtsi2ss	xmm1,	ecx			; xmm1 is now (float)i_x
			subss		xmm0,	xmm1
			addss		xmm0,	const_0_5	; xmm0 is b
			shufps		xmm0,	xmm0, 0		; xmm0 is now: b b b b

			/*
			float abs_t = fabs(t);
			if (|t| < 1.0f)
			{
				return (1.0f - (c+3.0f)*(t^2) + (c+2.0f)*(|t|^3));
			}
			if (|t| < 2.0f)
			{
				return (-4.0f*c + (8.0f*c)*(|t|) - (5.0f*c)*(t^2) + c*(|t|^3));
			}
			return 0.0f;

			c = -0.5f
			---------

			float abs_t = fabs(t);
			if (|t| < 1.0f)
			{
				return (1.0f + (-2.5f)*(t^2) + (1.5f)*(|t|^3));
			}
			if (|t| < 2.0f)
			{
				return (2.0f + (-4.0f)*(|t|) + (2.5f)*(t^2) + (-0.5f)*(|t|^3));
			}
			return 0.0f;
			*/
		
			;t
			movaps xmm1, kernconst
			subps  xmm1, xmm0			; xmm1 is now: -1.0f-b -b 1.0f-b 2.0f-b
			
			;|t| = xmm1, xmm2 and xmm7
			andps  xmm1, absconst
			movaps xmm2, xmm1
			movaps xmm7, xmm2

			;t^2 = xmm3
			movaps xmm3, xmm2
			mulps  xmm3, xmm3

			;|t|^3 = xmm5 and xmm4
			movaps xmm5, xmm1
			mulps  xmm5, xmm3
			movaps xmm4, xmm5

			;
			; xmm5 will contain the calculation for |t| < 1.0f 
			;

			; xmm5 = (1.5f)*(|t|^3) 
			mulps  xmm5, kernconst_1_5
			
			; xmm3 = (-2.5f)*(t^2)
			mulps  xmm3, kernconst_minus_2_5

			; xmm5 = (-2.5f)*(t^2) + (1.5f)*(|t|^3)
			addps  xmm5, xmm3

			; xmm5 = 1.0f + (-2.5f)*(t^2) + (1.5f)*(|t|^3)
			addps  xmm5, kernconst_1

			;
			; xmm4 will contain the calculation for |t| < 2.0f 
			;

			; xmm4 = (-0.5f)*(|t|^3)
			mulps  xmm4, kernconst_minus_0_5
			
			; xmm3 = (-2.5f)*(t^2) (from above calculation)
			; xmm4 = (2.5f)*(t^2) + (-0.5f)*(|t|^3)
			subps  xmm4, xmm3

			; xmm2 = (-4.0f)*(|t|)
			mulps  xmm2, kernconst_minus_4

			; xmm4 = (-4.0f)*(|t|) + (2.5f)*(t^2) + (-0.5f)*(|t|^3)
			addps  xmm4, xmm2

			; xmm4 = 2.0f + (-4.0f)*(|t|) + (2.5f)*(t^2) + (-0.5f)*(|t|^3)
			addps  xmm4, kernconst_2

			;
			; Merging the 2 calculations
			;

			; xmm4 contains the result for |t| < 2.0f, otherwise 0.0f
			cmpltps xmm1, kernconst_2
			andps	xmm4, xmm1

			; xmm5 contains the result for |t| < 1.0f, otherwise 0.0f
			; xmm7 contains the result for 1.0f <= |t| < 2.0f, otherwise 0.0f
			cmpltps xmm7, kernconst_1
			andps	xmm5, xmm7
			andnps	xmm7, xmm4
			
			; xmm7 contains the final result
			orps	xmm7, xmm5

			; store into array
			mov		ecx,  kernel_b
			movaps	[ecx], xmm7
		}
	}

	DIB_INIT_PROGRESS;

	for (y = 0 ; y < nNewHeight ; y++)
	{
		__asm emms; // End MMX instructions, prep for possible FP instrs.
		if (pThread && pThread->DoExit())
		{
			BIGFREE(array_kernel_b);
			DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
			return FALSE;
		}
		// This uses floats -> the above emms is necessary
		DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, nNewHeight);

		__asm
		{
			//float f_y = (y + 0.5f) * yScale;
			//i_y = RoundF(f_y); // Center Y coord. of the 16 source pixels
			//float a   = f_y - (float)i_y + 0.5f;
			cvtsi2ss	xmm0,	y
			addss		xmm0,	const_0_5
			mulss		xmm0,	yScale		; xmm0 is now f_y
			cvtss2si	ecx,	xmm0		; ecx is now i_y
			mov			i_y,	ecx
			cvtsi2ss	xmm1,	ecx			; xmm1 is now (float)i_y
			subss		xmm0,	xmm1
			addss		xmm0,	const_0_5	; xmm0 is now a
			shufps		xmm0,	xmm0, 0		; xmm0 is now: a a a a

			/*
			float abs_t = fabs(t);
			if (|t| < 1.0f)
			{
				return (1.0f - (c+3.0f)*(t^2) + (c+2.0f)*(|t|^3));
			}
			if (|t| < 2.0f)
			{
				return (-4.0f*c + (8.0f*c)*(|t|) - (5.0f*c)*(t^2) + c*(|t|^3));
			}
			return 0.0f;

			c = -0.5f
			---------

			float abs_t = fabs(t);
			if (|t| < 1.0f)
			{
				return (1.0f + (-2.5f)*(t^2) + (1.5f)*(|t|^3));
			}
			if (|t| < 2.0f)
			{
				return (2.0f + (-4.0f)*(|t|) + (2.5f)*(t^2) + (-0.5f)*(|t|^3));
			}
			return 0.0f;
			*/

			;t
			movaps xmm1, kernconst
			subps  xmm1, xmm0			; xmm1 is now: -1.0f-a -a 1.0f-a 2.0f-a
			
			;|t| = xmm1, xmm2 and xmm6
			andps  xmm1, absconst
			movaps xmm2, xmm1
			movaps xmm6, xmm2

			;t^2 = xmm3
			movaps xmm3, xmm2
			mulps  xmm3, xmm3

			;|t|^3 = xmm5 and xmm4
			movaps xmm5, xmm1
			mulps  xmm5, xmm3
			movaps xmm4, xmm5

			;
			; xmm5 will contain the calculation for |t| < 1.0f 
			;

			; xmm5 = (1.5f)*(|t|^3) 
			mulps  xmm5, kernconst_1_5
			
			; xmm3 = (-2.5f)*(t^2)
			mulps  xmm3, kernconst_minus_2_5

			; xmm5 = (-2.5f)*(t^2) + (1.5f)*(|t|^3)
			addps  xmm5, xmm3

			; xmm5 = 1.0f + (-2.5f)*(t^2) + (1.5f)*(|t|^3)
			addps  xmm5, kernconst_1

			;
			; xmm4 will contain the calculation for |t| < 2.0f 
			;

			; xmm4 = (-0.5f)*(|t|^3)
			mulps  xmm4, kernconst_minus_0_5

			; xmm3 = (-2.5f)*(t^2) (from above calculation)
			; xmm4 = (2.5f)*(t^2) + (-0.5f)*(|t|^3)
			subps  xmm4, xmm3

			; xmm2 = (-4.0f)*(|t|)
			mulps  xmm2, kernconst_minus_4

			; xmm4 = (-4.0f)*(|t|) + (2.5f)*(t^2) + (-0.5f)*(|t|^3)
			addps  xmm4, xmm2

			; xmm4 = 2.0f + (-4.0f)*(|t|) + (2.5f)*(t^2) + (-0.5f)*(|t|^3)
			addps  xmm4, kernconst_2

			;
			; Merging the 2 calculations
			;

			; xmm4 contains the result for |t| < 2.0f, otherwise 0.0f
			cmpltps xmm1, kernconst_2
			andps	xmm4, xmm1

			; xmm5 contains the result for |t| < 1.0f, otherwise 0.0f
			; xmm6 contains the result for 1.0f <= |t| < 2.0f, otherwise 0.0f
			cmpltps xmm6, kernconst_1
			andps	xmm5, xmm6
			andnps	xmm6, xmm4
			
			; xmm6 contains the final result
			orps	xmm6, xmm5
		}

		for (x = 0 , kernel_b = (DWORD)array_kernel_b ; x < nNewWidth ; x++ , kernel_b += 16U)
		{
			//float f_x = (x + 0.5f) * xScale;
			//i_x = RoundF(f_x); // Center X coord. of the 16 source pixels
			__asm
			{
				cvtsi2ss	xmm0,	x
				addss		xmm0,	const_0_5
				mulss		xmm0,	xScale		; xmm0 is now f_x
				cvtss2si	ecx,	xmm0		; ecx is now i_x
				mov			i_x,	ecx
			}

			xx = i_x - 2;
			yy = i_y - 2;
			yy = (yy + BICUBIC_PIXMARGIN) * nFloatSrcScanLineSize;
			i = yy + ((xx + BICUBIC_PIXMARGIN) << 2);
			pf = f + i;

			__asm
			{	
				mov	   ecx,  kernel_b
				movaps xmm7, [ecx]				; xmm7 is now: r2_1 r2_2 r2_3 r2_4

				; xmm0 is for bb gg rr aa, reset it
				xorps	xmm0, xmm0

				;
				; m = -1
				;
				mov ecx, pf

				movaps xmm2, xmm6				; xmm2 is now: r1_1 r1_2 r1_3 r1_4
				shufps xmm2, xmm2, 0			; xmm2 is now: r1_1 r1_1 r1_1 r1_1
				
				mulps  xmm2, xmm7				; xmm2 is now: r1 r2 r3 r4
				movaps xmm3, xmm2				; xmm3 is now: r1 r2 r3 r4
				movaps xmm4, xmm3				; xmm4 is now: r1 r2 r3 r4
				movaps xmm5, xmm4				; xmm5 is now: r1 r2 r3 r4

				shufps xmm2, xmm2, 0			; xmm2 is now: r1 r1 r1 r1
				shufps xmm3, xmm3, 01010101b	; xmm3 is now: r2 r2 r2 r2
				shufps xmm4, xmm4, 10101010b	; xmm4 is now: r3 r3 r3 r3
				shufps xmm5, xmm5, 11111111b	; xmm5 is now: r4 r4 r4 r4

				movaps xmm1, [ecx]				; xmm1 is now: b g r a
				mulps  xmm1, xmm2				; xmm1 is now: r1*b r1*g r1*r r1*a 
				addps  xmm0, xmm1				; xmm0 is now: bb+r1*b gg+r1*g rr+r1*r aa+r1*a 

				movaps xmm1, [ecx+16]			; xmm1 is now: b g r a
				mulps  xmm1, xmm3				; xmm1 is now: r2*b r2*g r2*r r2*a 
				addps  xmm0, xmm1				; xmm0 is now: bb+r2*b gg+r2*g rr+r2*r aa+r2*a

				movaps xmm1, [ecx+32]			; xmm1 is now: b g r a
				mulps  xmm1, xmm4				; xmm1 is now: r3*b r3*g r3*r r3*a 
				addps  xmm0, xmm1				; xmm0 is now: bb+r3*b gg+r3*g rr+r3*r aa+r3*a

				movaps xmm1, [ecx+48]			; xmm1 is now: b g r a
				mulps  xmm1, xmm5				; xmm1 is now: r4*b r4*g r4*r r4*a 
				addps  xmm0, xmm1				; xmm0 is now: bb+r4*b gg+r4*g rr+r4*r aa+r4*a

				;
				; m = 0
				;
				add	ecx, nFloatSrcScanLineSizeBytes
			
				movaps xmm2, xmm6				; xmm2 is now: r1_1 r1_2 r1_3 r1_4
				shufps xmm2, xmm2, 01010101b	; xmm2 is now: r1_2 r1_2 r1_2 r1_2

				mulps  xmm2, xmm7				; xmm2 is now: r1 r2 r3 r4
				movaps xmm3, xmm2				; xmm3 is now: r1 r2 r3 r4
				movaps xmm4, xmm3				; xmm4 is now: r1 r2 r3 r4
				movaps xmm5, xmm4				; xmm5 is now: r1 r2 r3 r4

				shufps xmm2, xmm2, 0			; xmm2 is now: r1 r1 r1 r1
				shufps xmm3, xmm3, 01010101b	; xmm3 is now: r2 r2 r2 r2
				shufps xmm4, xmm4, 10101010b	; xmm4 is now: r3 r3 r3 r3
				shufps xmm5, xmm5, 11111111b	; xmm5 is now: r4 r4 r4 r4

				movaps xmm1, [ecx]				; xmm1 is now: b g r a
				mulps  xmm1, xmm2				; xmm1 is now: r1*b r1*g r1*r r1*a 
				addps  xmm0, xmm1				; xmm0 is now: bb+r1*b gg+r1*g rr+r1*r aa+r1*a 

				movaps xmm1, [ecx+16]			; xmm1 is now: b g r a
				mulps  xmm1, xmm3				; xmm1 is now: r2*b r2*g r2*r r2*a 
				addps  xmm0, xmm1				; xmm0 is now: bb+r2*b gg+r2*g rr+r2*r aa+r2*a

				movaps xmm1, [ecx+32]			; xmm1 is now: b g r a
				mulps  xmm1, xmm4				; xmm1 is now: r3*b r3*g r3*r r3*a 
				addps  xmm0, xmm1				; xmm0 is now: bb+r3*b gg+r3*g rr+r3*r aa+r3*a

				movaps xmm1, [ecx+48]			; xmm1 is now: b g r a
				mulps  xmm1, xmm5				; xmm1 is now: r4*b r4*g r4*r r4*a 
				addps  xmm0, xmm1				; xmm0 is now: bb+r4*b gg+r4*g rr+r4*r aa+r4*a

				;
				; m = 1
				;
				add	ecx, nFloatSrcScanLineSizeBytes

				movaps xmm2, xmm6				; xmm2 is now: r1_1 r1_2 r1_3 r1_4
				shufps xmm2, xmm2, 10101010b	; xmm2 is now: r1_3 r1_3 r1_3 r1_3

				mulps  xmm2, xmm7				; xmm2 is now: r1 r2 r3 r4
				movaps xmm3, xmm2				; xmm3 is now: r1 r2 r3 r4
				movaps xmm4, xmm3				; xmm4 is now: r1 r2 r3 r4
				movaps xmm5, xmm4				; xmm5 is now: r1 r2 r3 r4

				shufps xmm2, xmm2, 0			; xmm2 is now: r1 r1 r1 r1
				shufps xmm3, xmm3, 01010101b	; xmm3 is now: r2 r2 r2 r2
				shufps xmm4, xmm4, 10101010b	; xmm4 is now: r3 r3 r3 r3
				shufps xmm5, xmm5, 11111111b	; xmm5 is now: r4 r4 r4 r4

				movaps xmm1, [ecx]				; xmm1 is now: b g r a
				mulps  xmm1, xmm2				; xmm1 is now: r1*b r1*g r1*r r1*a 
				addps  xmm0, xmm1				; xmm0 is now: bb+r1*b gg+r1*g rr+r1*r aa+r1*a

				movaps xmm1, [ecx+16]			; xmm1 is now: b g r a
				mulps  xmm1, xmm3				; xmm1 is now: r2*b r2*g r2*r r2*a 
				addps  xmm0, xmm1				; xmm0 is now: bb+r2*b gg+r2*g rr+r2*r aa+r2*a

				movaps xmm1, [ecx+32]			; xmm1 is now: b g r a
				mulps  xmm1, xmm4				; xmm1 is now: r3*b r3*g r3*r r3*a 
				addps  xmm0, xmm1				; xmm0 is now: bb+r3*b gg+r3*g rr+r3*r aa+r3*a

				movaps xmm1, [ecx+48]			; xmm1 is now: b g r a
				mulps  xmm1, xmm5				; xmm1 is now: r4*b r4*g r4*r r4*a 
				addps  xmm0, xmm1				; xmm0 is now: bb+r4*b gg+r4*g rr+r4*r aa+r4*a
				
				;
				; m = 2
				;
				add	ecx, nFloatSrcScanLineSizeBytes

				movaps xmm2, xmm6				; xmm2 is now: r1_1 r1_2 r1_3 r1_4
				shufps xmm2, xmm2, 11111111b	; xmm2 is now: r1_4 r1_4 r1_4 r1_4

				mulps  xmm2, xmm7				; xmm2 is now: r1 r2 r3 r4
				movaps xmm3, xmm2				; xmm3 is now: r1 r2 r3 r4
				movaps xmm4, xmm3				; xmm4 is now: r1 r2 r3 r4
				movaps xmm5, xmm4				; xmm5 is now: r1 r2 r3 r4

				shufps xmm2, xmm2, 0			; xmm2 is now: r1 r1 r1 r1
				shufps xmm3, xmm3, 01010101b	; xmm3 is now: r2 r2 r2 r2
				shufps xmm4, xmm4, 10101010b	; xmm4 is now: r3 r3 r3 r3
				shufps xmm5, xmm5, 11111111b	; xmm5 is now: r4 r4 r4 r4

				movaps xmm1, [ecx]				; xmm1 is now: b g r a
				mulps  xmm1, xmm2				; xmm1 is now: r1*b r1*g r1*r r1*a
				addps  xmm0, xmm1				; xmm0 is now: bb+r1*b gg+r1*g rr+r1*r aa+r1*a 

				movaps xmm1, [ecx+16]			; xmm1 is now: b g r a
				mulps  xmm1, xmm3				; xmm1 is now: r2*b r2*g r2*r r2*a 
				addps  xmm0, xmm1				; xmm0 is now: bb+r2*b gg+r2*g rr+r2*r aa+r2*a

				movaps xmm1, [ecx+32]			; xmm1 is now: b g r a
				mulps  xmm1, xmm4				; xmm1 is now: r3*b r3*g r3*r r3*a 
				addps  xmm0, xmm1				; xmm0 is now: bb+r3*b gg+r3*g rr+r3*r aa+r3*a

				movaps xmm1, [ecx+48]			; xmm1 is now: b g r a
				mulps  xmm1, xmm5				; xmm1 is now: r4*b r4*g r4*r r4*a 
				addps  xmm0, xmm1				; xmm0 is now: bb+r4*b gg+r4*g rr+r4*r aa+r4*a

				; Convert with clamping to 0..255 from xmm0 to dwPix 
				; xmm0 is (0) bb gg rr aa (127)
				cvtps2pi  mm0,   xmm0			; (0) bb gg (63)
				shufps    xmm0,  xmm0, 01001110b
				cvtps2pi  mm1,   xmm0			; (0) rr aa (63)
				packssdw  mm0,   mm1			; (0) bb gg rr aa (63)
				packuswb  mm0,   mm0			; (0) bb gg rr aa bb gg rr aa (63) unsigned saturated!
				movd	  dwPix, mm0			; dwPix = (0) bb gg rr aa (31)
			}

			// Set Output Pixel
			pOutBits[x] = dwPix;
		}

		// Inc. Output Pixel Offset
		pOutBits += nNewWidth;
	}

	__asm emms; // End MMX instructions, prep for possible FP instrs.
	BIGFREE(array_kernel_b);
	DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

	return TRUE;
}

BOOL CDib::BicubicResample24_C(	int nNewWidth,
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

BOOL CDib::BicubicResample32_C(	int nNewWidth,
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
