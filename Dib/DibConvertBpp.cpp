#include "stdafx.h"
#include "dib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

DWORD CDib::CountUniqueColors(	CWnd* pProgressWnd/*=NULL*/,
								BOOL bProgressSend/*=TRUE*/)
{
	DWORD dwColorsCount = 0;
	unsigned int line, i, n, j;
	DWORD* pColorsCountTable = NULL;
	LPDWORD* ppColorsCountTable = NULL;
	DWORD dwColorsCountTableSize = 0;
	WORD pix;
	int R, G, B, A;

	if (!m_pBits)
	{
		if (!DibSectionToBits())
			return 0;
	}

	if (!m_pBits || !m_pBMI)
		return 0;

	if (IsCompressed())
	{
		if (!Decompress(GetBitCount())) // Decompress
			return 0;
	}

	// Scan Line Alignment
	DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * GetBitCount());

	// Bits Pointer
	LPBYTE lpBits = m_pBits;

	DIB_INIT_PROGRESS;

	switch (GetBitCount())
	{
		case 1 :
		{
			// Init Lookup Table
			dwColorsCountTableSize = GetNumColors();
			pColorsCountTable = new DWORD[dwColorsCountTableSize];
			if (!pColorsCountTable)
				return 0;
			memset(pColorsCountTable, 0, dwColorsCountTableSize * sizeof(DWORD));

			// Count
			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					pColorsCountTable[(lpBits[i/8] >> (7-(i%8))) & 0x01]++;
				}
				lpBits += uiDIBScanLineSize;
			}

			// Remove duplicates in the table
			if (GetNumColors() == 2)
			{
				if (((DWORD*)m_pColors)[0] == ((DWORD*)m_pColors)[1])
					pColorsCountTable[1] = 0;
			}
			
			break;
		}
		case 4 :
		{
			// Init Lookup Table
			dwColorsCountTableSize = GetNumColors();
			pColorsCountTable = new DWORD[dwColorsCountTableSize];
			if (!pColorsCountTable)
				return 0;
			memset(pColorsCountTable, 0, dwColorsCountTableSize * sizeof(DWORD));

			// Count
			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					if (i%2) // LS Nibble
						pColorsCountTable[lpBits[i/2] & 0x0F]++;
					else // MS Nibble
						pColorsCountTable[lpBits[i/2] >> 4]++;

				}
				lpBits += uiDIBScanLineSize;
			}

			// Remove duplicates in the table
			for (i = 0 ; i < dwColorsCountTableSize ; i++)
			{
				if (pColorsCountTable[i] != 0)
				{
					for (n = i + 1 ; n < dwColorsCountTableSize ; n++)
					{
						if (((DWORD*)m_pColors)[i] == ((DWORD*)m_pColors)[n])
							pColorsCountTable[n] = 0;
					}
				}
			}

			break;
		}
		case 8 :
		{	
			// Init Lookup Table
			dwColorsCountTableSize = GetNumColors();
			pColorsCountTable = new DWORD[dwColorsCountTableSize];
			if (!pColorsCountTable)
				return 0;
			memset(pColorsCountTable, 0, dwColorsCountTableSize * sizeof(DWORD));

			// Count
			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					pColorsCountTable[lpBits[i]]++;
				}
				lpBits += uiDIBScanLineSize;
			}

			// Remove duplicates in the table
			for (i = 0 ; i < dwColorsCountTableSize ; i++)
			{
				if (pColorsCountTable[i] != 0)
				{
					for (n = i + 1 ; n < dwColorsCountTableSize ; n++)
					{
						if (((DWORD*)m_pColors)[i] == ((DWORD*)m_pColors)[n])
							pColorsCountTable[n] = 0;
					}
				}
			}

			break;
		}
		case 16 :
		{
			LPBITMAPINFOBITFIELDS pBmiBf = (LPBITMAPINFOBITFIELDS)m_pBMI;
			
			// 565
			if ((GetCompression() == BI_BITFIELDS)	&&
				(pBmiBf->biBlueMask == 0x001F)		&&
				(pBmiBf->biGreenMask == 0x07E0)		&&
				(pBmiBf->biRedMask == 0xF800))
			{
				// Init Lookup Table
				dwColorsCountTableSize = 65536;
				pColorsCountTable = new DWORD[dwColorsCountTableSize];
				if (!pColorsCountTable)
					return 0;
				memset(pColorsCountTable, 0, dwColorsCountTableSize * sizeof(DWORD));

				// Count
				for (line = 0 ; line < GetHeight() ; line++)
				{
					DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

					for (i = 0 ; i < GetWidth() ; i++)
					{
						pColorsCountTable[((WORD*)lpBits)[i]]++;
					}
					lpBits += uiDIBScanLineSize;
				}
			}
			// 555
			else
			{
				// Init Lookup Table
				dwColorsCountTableSize = 32768;
				pColorsCountTable = new DWORD[dwColorsCountTableSize];
				if (!pColorsCountTable)
					return 0;
				memset(pColorsCountTable, 0, dwColorsCountTableSize * sizeof(DWORD));

				// Count
				for (line = 0 ; line < GetHeight() ; line++)
				{
					DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

					for (i = 0 ; i < GetWidth() ; i++)
					{
						pColorsCountTable[(((WORD*)lpBits)[i]) & 0x7FFF]++;
					}
					lpBits += uiDIBScanLineSize;
				}
			}

			break;
		}
		case 24 :
		{
			// Init Lookup Table
			dwColorsCountTableSize = 65536;
			ppColorsCountTable = new LPDWORD[dwColorsCountTableSize];
			if (!ppColorsCountTable)
				return 0;
			memset(ppColorsCountTable, 0, dwColorsCountTableSize * sizeof(LPDWORD));

			// Set flags bit if color available
			for (line = 0 ; line < GetHeight(); line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					B	= lpBits[3*i];
					pix = *((WORD*)&(lpBits[3*i+1])); // G & R
					if (!ppColorsCountTable[pix])
					{
						// I tested and found out that calloc/free is
						// some percents faster than new(with memset)/delete
						ppColorsCountTable[pix] = (LPDWORD)calloc(32, 1);
					}
					ppColorsCountTable[pix][B >> 5] |= 1 << (B & 0x1F);
				}
				lpBits += uiDIBScanLineSize;
			}

			break;
		}
		case 32 :
		{
			// Init Lookup Table
			dwColorsCountTableSize = 65536;
			ppColorsCountTable = new LPDWORD[dwColorsCountTableSize];
			if (!ppColorsCountTable)
				return 0;
			memset(ppColorsCountTable, 0, dwColorsCountTableSize * sizeof(LPDWORD));

			if (!HasAlpha() && IsFast32bpp())
			{	
				// Set flags bit if color available
				for (line = 0 ; line < GetHeight(); line++)
				{
					DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

					for (i = 0 ; i < GetWidth() ; i++)
					{
						B	= lpBits[4*i];
						pix = *((WORD*)&(lpBits[4*i+1])); // G & R
						if (!ppColorsCountTable[pix])
						{
							// I tested and found out that calloc/free is
							// some percents faster than new(with memset)/delete
							ppColorsCountTable[pix] = (LPDWORD)calloc(32, 1);
						}
						ppColorsCountTable[pix][B >> 5] |= 1 << (B & 0x1F);
					}
					lpBits += uiDIBScanLineSize;
				}
			}
			else if (HasAlpha())
			{
				// Set flags bit if color available
				for (line = 0 ; line < GetHeight(); line++)
				{
					DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

					for (i = 0 ; i < GetWidth() ; i++)
					{
						B	= lpBits[4*i];
						pix = *((WORD*)&(lpBits[4*i+1])); // G & R
						A	= lpBits[4*i+3];
						if (A > 0) // Only Count if not transparent!
						{
							if (!ppColorsCountTable[pix])
							{
								// I tested and found out that calloc/free is
								// some percents faster than new(with memset)/delete
								ppColorsCountTable[pix] = (LPDWORD)calloc(32, 1);
							}
							ppColorsCountTable[pix][B >> 5] |= 1 << (B & 0x1F);
						}
					}
					lpBits += uiDIBScanLineSize;
				}
			}
			else
			{
				// Set flags bit if color available
				for (line = 0 ; line < GetHeight() ; line++)
				{
					DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

					for (i = 0 ; i < GetWidth() ; i++)
					{
						DIB32ToRGB(((DWORD*)lpBits)[i], &R, &G, &B);
						pix = (R << 8) | G; // G & R
						if (!ppColorsCountTable[pix])
						{
							// I tested and found out that calloc/free is
							// some percents faster than new(with memset)/delete
							ppColorsCountTable[pix] = (LPDWORD)calloc(32, 1);
						}
						ppColorsCountTable[pix][B >> 5] |= 1 << (B & 0x1F);
					}
					lpBits += uiDIBScanLineSize;
				}
			}

			break;
		}
		default:
			break;
	}

	// Count & Free
	if (pColorsCountTable)
	{
		for (i = 0 ; i < dwColorsCountTableSize ; i++)
		{
			if (pColorsCountTable[i] != 0)
				dwColorsCount++;
		}

		// Free
		if (pColorsCountTable)
			delete [] pColorsCountTable;
	}
	else
	{
		DWORD dwFlags;
		for (i = 0 ; i < dwColorsCountTableSize ; i++)
		{
			if (ppColorsCountTable[i])
			{
				for (j = 0 ; j < 8 ; j++)
				{
					dwFlags = ppColorsCountTable[i][j];
					for (n = 0 ; n < 32 ; n++)
					{
						if (dwFlags & 0x1)
							dwColorsCount++;
						dwFlags >>= 1;
					}
				}

				// Free
				free(ppColorsCountTable[i]);
			}
		}

		// Free
		if (ppColorsCountTable)
			delete [] ppColorsCountTable;
	}

	DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

	return dwColorsCount;
}

BOOL CDib::ConvertTo(	int nBpp,
						CWnd* pProgressWnd/*=NULL*/,
						BOOL bProgressSend/*=TRUE*/)
{
	switch (nBpp)
	{
		case 1 : return ConvertTo1bit(	128,
										RGB(0,0,0),
										RGB(255,255,255),
										pProgressWnd,
										bProgressSend);
		case 4 : return ConvertTo4bits(	NULL,
										pProgressWnd,
										bProgressSend);
		case 8 : return ConvertTo8bits(	NULL,
										pProgressWnd,
										bProgressSend);
		case 16 : return ConvertTo15bits(pProgressWnd,
										bProgressSend);
		case 24 : return ConvertTo24bits(pProgressWnd,
										bProgressSend);
		case 32 : return ConvertTo32bits(pProgressWnd,
										bProgressSend);
		default : return ConvertTo24bits(pProgressWnd,
										bProgressSend);
	}
}

// Image is first converted to Grayscale, if a value is greater or equal
// the Threshold it is set to Bright Color, otherwise to Dark
BOOL CDib::ConvertTo1bit(	int nThreshold/*=128*/,
							COLORREF crDarkColor/*=RGB(0,0,0)*/,
							COLORREF crBrightColor/*=RGB(255,255,255)*/,
							CWnd* pProgressWnd/*=NULL*/,
							BOOL bProgressSend/*=TRUE*/)
{
	if (GetBitCount() == 1)
		return TRUE;

	unsigned int line, i;

	if (!m_pBits)
	{
		if (!DibSectionToBits())
			return FALSE;
	}

	if (!m_pBits || !m_pBMI)
		return FALSE;

	if (IsCompressed())
	{
		if (!Decompress(GetBitCount())) // Decompress
			return FALSE;
	}

	// Reset Alpha
	m_bAlpha = FALSE;

	// First Convert to Grayscale
	Grayscale();

	// Scan Line Alignments
	DWORD uiDIBSourceScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * GetBitCount());
	DWORD uiDIBTargetScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * 1);

	// Allocate memory
	LPBITMAPINFO lpNewBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER) + 2 * sizeof(RGBQUAD)];
	if (!lpNewBMI)
		return FALSE;
	LPBYTE lpNewBitsStart = (LPBYTE)BIGALLOC(uiDIBTargetScanLineSize * GetHeight());
	if (!lpNewBitsStart)
	{
		delete [] lpNewBMI;
		return FALSE;
	}
	LPBYTE lpNewBits = lpNewBitsStart; 
	LPBYTE lpBits = m_pBits;

	// New BMI
	lpNewBMI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	lpNewBMI->bmiHeader.biWidth = GetWidth();
	lpNewBMI->bmiHeader.biHeight = GetHeight();
	lpNewBMI->bmiHeader.biPlanes = 1;
	lpNewBMI->bmiHeader.biBitCount = 1;
	lpNewBMI->bmiHeader.biCompression = BI_RGB;
	lpNewBMI->bmiHeader.biSizeImage = DWALIGNEDWIDTHBYTES(GetWidth() * 1) * GetHeight();
	lpNewBMI->bmiHeader.biXPelsPerMeter = m_pBMI->bmiHeader.biXPelsPerMeter;
	lpNewBMI->bmiHeader.biYPelsPerMeter = m_pBMI->bmiHeader.biYPelsPerMeter;
	lpNewBMI->bmiHeader.biClrUsed = 0;
	lpNewBMI->bmiHeader.biClrImportant = 0;
	m_dwImageSize = DWALIGNEDWIDTHBYTES(GetWidth() * 1) * GetHeight();

	DIB_INIT_PROGRESS;

	switch (GetBitCount())
	{
		case 4 :
		{
			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					int nIndex4bits;
					if (i%2) // LS Nibble
						nIndex4bits = lpBits[i/2] & 0x0F;
					else // MS Nibble
						nIndex4bits = lpBits[i/2] >> 4;
					int nIndex1bit = (m_pColors[nIndex4bits].rgbRed >= nThreshold) ? 1 : 0;
					if (i%8 == 0)
						lpNewBits[i/8] = 0;
					lpNewBits[i/8] |= nIndex1bit << (7-(i%8));
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}

			delete [] m_pBMI;
			m_pBMI = lpNewBMI;
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + m_pBMI->bmiHeader.biSize);
			m_pColors[0].rgbRed = GetRValue(crDarkColor);
			m_pColors[0].rgbGreen = GetGValue(crDarkColor);
			m_pColors[0].rgbBlue = GetBValue(crDarkColor);
			m_pColors[1].rgbRed = GetRValue(crBrightColor);
			m_pColors[1].rgbGreen = GetGValue(crBrightColor);
			m_pColors[1].rgbBlue = GetBValue(crBrightColor);
			CreatePaletteFromBMI();

			break;
		}
		case 8 :
		{
			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					int nIndex1bit = (m_pColors[lpBits[i]].rgbRed >= nThreshold) ? 1 : 0;
					if (i%8 == 0)
						lpNewBits[i/8] = 0;
					lpNewBits[i/8] |= nIndex1bit << (7-(i%8));
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}

			delete [] m_pBMI;
			m_pBMI = lpNewBMI;
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + m_pBMI->bmiHeader.biSize);
			m_pColors[0].rgbRed = GetRValue(crDarkColor);
			m_pColors[0].rgbGreen = GetGValue(crDarkColor);
			m_pColors[0].rgbBlue = GetBValue(crDarkColor);
			m_pColors[1].rgbRed = GetRValue(crBrightColor);
			m_pColors[1].rgbGreen = GetGValue(crBrightColor);
			m_pColors[1].rgbBlue = GetBValue(crBrightColor);
			CreatePaletteFromBMI();

			break;
		}
		case 16 :
		{
			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					BYTE R = (BYTE)((((WORD*)lpBits)[i] & m_wRedMask16) >> m_nRedDownShift16);
					int nIndex1bit = (R >= nThreshold) ? 1 : 0;
					if (i%8 == 0)
						lpNewBits[i/8] = 0;
					lpNewBits[i/8] |= nIndex1bit << (7-(i%8));
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}

			delete [] m_pBMI;
			m_pBMI = lpNewBMI;
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + m_pBMI->bmiHeader.biSize);
			m_pColors[0].rgbRed = GetRValue(crDarkColor);
			m_pColors[0].rgbGreen = GetGValue(crDarkColor);
			m_pColors[0].rgbBlue = GetBValue(crDarkColor);
			m_pColors[1].rgbRed = GetRValue(crBrightColor);
			m_pColors[1].rgbGreen = GetGValue(crBrightColor);
			m_pColors[1].rgbBlue = GetBValue(crBrightColor);
			CreatePaletteFromBMI();

			break;
		}
		case 24 :
		{
			for (line = 0 ; line < GetHeight(); line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					int nIndex1bit = (lpBits[3*i+2] >= nThreshold) ? 1 : 0;
					if (i%8 == 0)
						lpNewBits[i/8] = 0;
					lpNewBits[i/8] |= nIndex1bit << (7-(i%8));
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}

			delete [] m_pBMI;
			m_pBMI = lpNewBMI;
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + m_pBMI->bmiHeader.biSize);
			m_pColors[0].rgbRed = GetRValue(crDarkColor);
			m_pColors[0].rgbGreen = GetGValue(crDarkColor);
			m_pColors[0].rgbBlue = GetBValue(crDarkColor);
			m_pColors[1].rgbRed = GetRValue(crBrightColor);
			m_pColors[1].rgbGreen = GetGValue(crBrightColor);
			m_pColors[1].rgbBlue = GetBValue(crBrightColor);
			CreatePaletteFromBMI();

			break;
		}
		case 32 :
		{
			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					BYTE R = (BYTE)((((DWORD*)lpBits)[i] & m_dwRedMask32) >> m_nRedDownShift32);
					int nIndex1bit = (R >= nThreshold) ? 1 : 0;
					if (i%8 == 0)
						lpNewBits[i/8] = 0;
					lpNewBits[i/8] |= nIndex1bit << (7-(i%8));
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}

			delete [] m_pBMI;
			m_pBMI = lpNewBMI;
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + m_pBMI->bmiHeader.biSize);
			m_pColors[0].rgbRed = GetRValue(crDarkColor);
			m_pColors[0].rgbGreen = GetGValue(crDarkColor);
			m_pColors[0].rgbBlue = GetBValue(crDarkColor);
			m_pColors[1].rgbRed = GetRValue(crBrightColor);
			m_pColors[1].rgbGreen = GetGValue(crBrightColor);
			m_pColors[1].rgbBlue = GetBValue(crBrightColor);
			CreatePaletteFromBMI();

			break;
		}
		default:
			break;
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
	
	m_pBits = lpNewBitsStart;

	DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

	return TRUE;
}

/**
 * Converts the image to B&W using the desired nMethod :
 * - 0 = Floyd-Steinberg (Default)
 * - 1 = Ordered-Dithering (4x4) 
 * - 2 = Burkes
 * - 3 = Stucki
 * - 4 = Jarvis-Judice-Ninke
 * - 5 = Sierra
 * - 6 = Stevenson-Arce
 * - 7 = Bayer (4x4 ordered dithering) 
 */
BOOL CDib::ConvertTo1bitDitherErrDiff(	int nMethod,
										CWnd* pProgressWnd/*=NULL*/,
										BOOL bProgressSend/*=TRUE*/)
{
	if (GetBitCount() == 1)
		return TRUE;

	if (!m_pBits)
	{
		if (!DibSectionToBits())
			return FALSE;
	}

	if (!m_pBits || !m_pBMI)
		return FALSE;

	if (IsCompressed())
	{
		if (!Decompress(GetBitCount())) // Decompress
			return FALSE;
	}

	// Reset Alpha
	m_bAlpha = FALSE;

	CDib OriginalDib(*this);
	if (OriginalDib.GetBitCount() != 24)
		OriginalDib.ConvertTo24bits();
	OriginalDib.Grayscale();
	
	// Convert to 1 bit:

	// Target Scanline Size
	DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * 1);

	// Allocate memory
	LPBITMAPINFO lpNewBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER) + 2 * sizeof(RGBQUAD)];
	if (!lpNewBMI)
		return FALSE;
	LPBYTE lpNewBitsStart = (LPBYTE)BIGALLOC(uiDIBScanLineSize * GetHeight());
	if (!lpNewBitsStart)
	{
		delete [] lpNewBMI;
		return FALSE;
	}
	LPBYTE lpNewBits = lpNewBitsStart; 
	LPBYTE lpBits = m_pBits;

	// New BMI
	lpNewBMI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	lpNewBMI->bmiHeader.biWidth = GetWidth();
	lpNewBMI->bmiHeader.biHeight = GetHeight();
	lpNewBMI->bmiHeader.biPlanes = 1;
	lpNewBMI->bmiHeader.biBitCount = 1;
	lpNewBMI->bmiHeader.biCompression = BI_RGB;
	lpNewBMI->bmiHeader.biSizeImage = DWALIGNEDWIDTHBYTES(GetWidth() * 1) * GetHeight();
	lpNewBMI->bmiHeader.biXPelsPerMeter = m_pBMI->bmiHeader.biXPelsPerMeter;
	lpNewBMI->bmiHeader.biYPelsPerMeter = m_pBMI->bmiHeader.biYPelsPerMeter;
	lpNewBMI->bmiHeader.biClrUsed = 0;
	lpNewBMI->bmiHeader.biClrImportant = 0;
	m_dwImageSize = DWALIGNEDWIDTHBYTES(GetWidth() * 1) * GetHeight();
	delete [] m_pBMI;
	m_pBMI = lpNewBMI;
	m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + m_pBMI->bmiHeader.biSize);
	m_pColors[0].rgbRed = 0;
	m_pColors[0].rgbGreen = 0;
	m_pColors[0].rgbBlue = 0;
	m_pColors[1].rgbRed = 255;
	m_pColors[1].rgbGreen = 255;
	m_pColors[1].rgbBlue = 255;
	CreatePaletteFromBMI();

	// Free
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

	// New Bits Pointer
	m_pBits = lpNewBitsStart;

	DIB_INIT_PROGRESS;

	switch (nMethod)
	{
		case 1:
		{
			// Multi-Level Ordered-Dithering by Kenny Hoff (Oct. 12, 1995)
			#define dth_NumRows 4
			#define dth_NumCols 4
			#define dth_NumIntensityLevels 2
			#define dth_NumRowsLessOne (dth_NumRows-1)
			#define dth_NumColsLessOne (dth_NumCols-1)
			#define dth_RowsXCols (dth_NumRows*dth_NumCols)
			#define dth_MaxIntensityVal 255
			#define dth_MaxDitherIntensityVal (dth_NumRows*dth_NumCols*(dth_NumIntensityLevels-1))

			int DitherMatrix[dth_NumRows][dth_NumCols] = {{0,8,2,10}, {12,4,14,6}, {3,11,1,9}, {15,7,13,5} };
			unsigned char Intensity[dth_NumIntensityLevels] = { 0,1 };
			int DitherIntensity, DitherMatrixIntensity, Offset, DeviceIntensity;
			unsigned char DitherValue;
  
			for (int y = 0 ; y < (int)GetHeight() ; y++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, GetHeight());

				for (int x = 0 ; x < (int)GetWidth() ; x++)
				{

					DeviceIntensity = COLORREFToGray(OriginalDib.GetPixelColor(x, y));
					DitherIntensity = DeviceIntensity*dth_MaxDitherIntensityVal/dth_MaxIntensityVal;
					DitherMatrixIntensity = DitherIntensity % dth_RowsXCols;
					Offset = DitherIntensity / dth_RowsXCols;
					if (DitherMatrix[y&dth_NumRowsLessOne][x&dth_NumColsLessOne] < DitherMatrixIntensity)
						DitherValue = Intensity[1+Offset];
					else
						DitherValue = Intensity[0+Offset];
					SetPixelIndex(x, y, DitherValue);
				}
			}

			break;
		}
		case 2:
		{
			// Burkes error diffusion (Thanks to Franco Gerevini)
			int TotalCoeffSum = 32;
			int error, nlevel, coeff;
			BYTE level;

			for (int y = 0 ; y < (int)GetHeight() ; y++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, GetHeight());

				for (int x = 0 ; x < (int)GetWidth() ; x++)
				{
					level = COLORREFToGray(OriginalDib.GetPixelColor(x, y));
					if (level > 128)
					{
						SetPixelIndex(x, y, 1);
						error = level - 255;
					}
					else
					{
						SetPixelIndex(x, y, 0);
						error = level;
					}

					nlevel = COLORREFToGray(OriginalDib.GetPixelColor(x + 1, y)) + (error * 8) / TotalCoeffSum;
					level = (BYTE)MIN(255, MAX(0, (int)nlevel));
					OriginalDib.SetPixelColor(x + 1, y, GrayToCOLORREF(level));
					nlevel = COLORREFToGray(OriginalDib.GetPixelColor(x + 2, y)) + (error * 4) / TotalCoeffSum;
					level = (BYTE)MIN(255, MAX(0, (int)nlevel));
					OriginalDib.SetPixelColor(x + 2, y, GrayToCOLORREF(level));
					int i;
					for (i = -2 ; i < 3 ; i++)
					{
						switch (i)
						{
							case -2:
								coeff = 2;
								break;
							case -1:
								coeff = 4;
								break;
							case 0:
								coeff = 8; 
								break;
							case 1:
								coeff = 4; 
								break;
							case 2:
								coeff = 2; 
								break;
						}
						nlevel = COLORREFToGray(OriginalDib.GetPixelColor(x + i, y + 1)) + (error * coeff) / TotalCoeffSum;
						level = (BYTE)MIN(255, MAX(0, (int)nlevel));
						OriginalDib.SetPixelColor(x + i, y + 1, GrayToCOLORREF(level));
					}
				}
			}
			break;
		}
		case 3:
		{
			// Stucki error diffusion (Thanks to Franco Gerevini)
			int TotalCoeffSum = 42;
			int error, nlevel, coeff;
			BYTE level;

			for (int y = 0 ; y < (int)GetHeight() ; y++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, GetHeight());

				for (int x = 0 ; x < (int)GetWidth() ; x++)
				{
					level = COLORREFToGray(OriginalDib.GetPixelColor(x, y));
					if (level > 128)
					{
						SetPixelIndex(x, y, 1);
						error = level - 255;
					}
					else
					{
						SetPixelIndex(x, y, 0);
						error = level;
					}

					nlevel = COLORREFToGray(OriginalDib.GetPixelColor(x + 1, y)) + (error * 8) / TotalCoeffSum;
					level = (BYTE)MIN(255, MAX(0, (int)nlevel));
					OriginalDib.SetPixelColor(x + 1, y, GrayToCOLORREF(level));
					nlevel = COLORREFToGray(OriginalDib.GetPixelColor(x + 2, y)) + (error * 4) / TotalCoeffSum;
					level = (BYTE)MIN(255, MAX(0, (int)nlevel));
					OriginalDib.SetPixelColor(x + 2, y, GrayToCOLORREF(level));
					int i;
					for (i = -2 ; i < 3 ; i++)
					{
						switch (i)
						{
							case -2:
								coeff = 2;
								break;
							case -1:
								coeff = 4;
								break;
							case 0:
								coeff = 8; 
								break;
							case 1:
								coeff = 4; 
								break;
							case 2:
								coeff = 2; 
								break;
						}
						nlevel = COLORREFToGray(OriginalDib.GetPixelColor(x + i, y + 1)) + (error * coeff) / TotalCoeffSum;
						level = (BYTE)MIN(255, MAX(0, (int)nlevel));
						OriginalDib.SetPixelColor(x + i, y + 1, GrayToCOLORREF(level));
					}
					for (i = -2 ; i < 3 ; i++)
					{
						switch (i)
						{
							case -2:
								coeff = 1;
								break;
							case -1:
								coeff = 2;
								break;
							case 0:
								coeff = 4; 
								break;
							case 1:
								coeff = 2; 
								break;
							case 2:
								coeff = 1; 
								break;
						}
						nlevel = COLORREFToGray(OriginalDib.GetPixelColor(x + i, y + 2)) + (error * coeff) / TotalCoeffSum;
						level = (BYTE)MIN(255, MAX(0, (int)nlevel));
						OriginalDib.SetPixelColor(x + i, y + 2, GrayToCOLORREF(level));
					}
				}
			}
			break;
		}
		case 4:
		{
			// Jarvis, Judice and Ninke error diffusion (Thanks to Franco Gerevini)
			int TotalCoeffSum = 48;
			int error, nlevel, coeff;
			BYTE level;

			for (int y = 0 ; y < (int)GetHeight() ; y++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, GetHeight());

				for (int x = 0 ; x < (int)GetWidth() ; x++)
				{
					level = COLORREFToGray(OriginalDib.GetPixelColor(x, y));
					if (level > 128)
					{
						SetPixelIndex(x, y, 1);
						error = level - 255;
					}
					else
					{
						SetPixelIndex(x, y, 0);
						error = level;
					}

					nlevel = COLORREFToGray(OriginalDib.GetPixelColor(x + 1, y)) + (error * 7) / TotalCoeffSum;
					level = (BYTE)MIN(255, MAX(0, (int)nlevel));
					OriginalDib.SetPixelColor(x + 1, y, GrayToCOLORREF(level));
					nlevel = COLORREFToGray(OriginalDib.GetPixelColor(x + 2, y)) + (error * 5) / TotalCoeffSum;
					level = (BYTE)MIN(255, MAX(0, (int)nlevel));
					OriginalDib.SetPixelColor(x + 2, y, GrayToCOLORREF(level));
					int i;
					for (i = -2 ; i < 3 ; i++)
					{
						switch (i)
						{
							case -2:
								coeff = 3;
								break;
							case -1:
								coeff = 5;
								break;
							case 0:
								coeff = 7; 
								break;
							case 1:
								coeff = 5; 
								break;
							case 2:
								coeff = 3; 
								break;
						}
						nlevel = COLORREFToGray(OriginalDib.GetPixelColor(x + i, y + 1)) + (error * coeff) / TotalCoeffSum;
						level = (BYTE)MIN(255, MAX(0, (int)nlevel));
						OriginalDib.SetPixelColor(x + i, y + 1, GrayToCOLORREF(level));
					}
					for (i = -2 ; i < 3 ; i++)
					{
						switch (i)
						{
							case -2:
								coeff = 1;
								break;
							case -1:
								coeff = 3;
								break;
							case 0:
								coeff = 5; 
								break;
							case 1:
								coeff = 3; 
								break;
							case 2:
								coeff = 1; 
								break;
						}
						nlevel = COLORREFToGray(OriginalDib.GetPixelColor(x + i, y + 2)) + (error * coeff) / TotalCoeffSum;
						level = (BYTE)MIN(255, MAX(0, (int)nlevel));
						OriginalDib.SetPixelColor(x + i, y + 2, GrayToCOLORREF(level));
					}
				}
			}
			break;
		}
		case 5:
		{
			// Sierra error diffusion (Thanks to Franco Gerevini)
			int TotalCoeffSum = 32;
			int error, nlevel, coeff;
			BYTE level;

			for (int y = 0 ; y < (int)GetHeight() ; y++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, GetHeight());

				for (int x = 0 ; x < (int)GetWidth() ; x++)
				{
					level = COLORREFToGray(OriginalDib.GetPixelColor(x, y));
					if (level > 128)
					{
						SetPixelIndex(x, y, 1);
						error = level - 255;
					}
					else
					{
						SetPixelIndex(x, y, 0);
						error = level;
					}

					nlevel = COLORREFToGray(OriginalDib.GetPixelColor(x + 1, y)) + (error * 5) / TotalCoeffSum;
					level = (BYTE)MIN(255, MAX(0, (int)nlevel));
					OriginalDib.SetPixelColor(x + 1, y, GrayToCOLORREF(level));
					nlevel = COLORREFToGray(OriginalDib.GetPixelColor(x + 2, y)) + (error * 3) / TotalCoeffSum;
					level = (BYTE)MIN(255, MAX(0, (int)nlevel));
					OriginalDib.SetPixelColor(x + 2, y, GrayToCOLORREF(level));
					int i;
					for (i = -2 ; i < 3 ; i++)
					{
						switch (i)
						{
							case -2:
								coeff = 2;
								break;
							case -1:
								coeff = 4;
								break;
							case 0:
								coeff = 5; 
								break;
							case 1:
								coeff = 4; 
								break;
							case 2:
								coeff = 2; 
								break;
						}
						nlevel = COLORREFToGray(OriginalDib.GetPixelColor(x + i, y + 1)) + (error * coeff) / TotalCoeffSum;
						level = (BYTE)MIN(255, MAX(0, (int)nlevel));
						OriginalDib.SetPixelColor(x + i, y + 1, GrayToCOLORREF(level));
					}
					for (i = -1 ; i < 2 ; i++)
					{
						switch (i)
						{
							case -1:
								coeff = 2;
								break;
							case 0:
								coeff = 3; 
								break;
							case 1:
								coeff = 2; 
								break;
						}
						nlevel = COLORREFToGray(OriginalDib.GetPixelColor(x + i, y + 2)) + (error * coeff) / TotalCoeffSum;
						level = (BYTE)MIN(255, MAX(0, (int)nlevel));
						OriginalDib.SetPixelColor(x + i, y + 2, GrayToCOLORREF(level));
					}
				}
			}
			break;
		}
		case 6:
		{
			// Stevenson and Arce error diffusion (Thanks to Franco Gerevini)
			int TotalCoeffSum = 200;
			int error, nlevel;
			BYTE level;

			for (int y = 0 ; y < (int)GetHeight() ; y++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, GetHeight());

				for (int x = 0 ; x < (int)GetWidth() ; x++)
				{
					level = COLORREFToGray(OriginalDib.GetPixelColor(x, y));
					if (level > 128)
					{
						SetPixelIndex(x, y, 1);
						error = level - 255;
					}
					else
					{
						SetPixelIndex(x, y, 0);
						error = level;
					}

					int tmp_index_x = x + 2;
					int tmp_index_y = y;
					int tmp_coeff = 32;
					nlevel = COLORREFToGray(OriginalDib.GetPixelColor(tmp_index_x, tmp_index_y)) + (error * tmp_coeff) / TotalCoeffSum;
					level = (BYTE)MIN(255, MAX(0, (int)nlevel));
					OriginalDib.SetPixelColor(tmp_index_x, tmp_index_y, GrayToCOLORREF(level));

					tmp_index_x = x - 3;
					tmp_index_y = y + 1;
					tmp_coeff = 12;
					nlevel = COLORREFToGray(OriginalDib.GetPixelColor(tmp_index_x, tmp_index_y)) + (error * tmp_coeff) / TotalCoeffSum;
					level = (BYTE)MIN(255, MAX(0, (int)nlevel));
					OriginalDib.SetPixelColor(tmp_index_x, tmp_index_y, GrayToCOLORREF(level));

					tmp_index_x = x - 1;
					tmp_coeff = 26;
					nlevel = COLORREFToGray(OriginalDib.GetPixelColor(tmp_index_x, tmp_index_y)) + (error * tmp_coeff) / TotalCoeffSum;
					level = (BYTE)MIN(255, MAX(0, (int)nlevel));
					OriginalDib.SetPixelColor(tmp_index_x, tmp_index_y, GrayToCOLORREF(level));

					tmp_index_x = x + 1;
					tmp_coeff = 30;
					nlevel = COLORREFToGray(OriginalDib.GetPixelColor(tmp_index_x, tmp_index_y)) + (error * tmp_coeff) / TotalCoeffSum;
					level = (BYTE)MIN(255, MAX(0, (int)nlevel));
					OriginalDib.SetPixelColor(tmp_index_x, tmp_index_y, GrayToCOLORREF(level));

					tmp_index_x = x + 3;
					tmp_coeff = 16;
					nlevel = COLORREFToGray(OriginalDib.GetPixelColor(tmp_index_x, tmp_index_y)) + (error * tmp_coeff) / TotalCoeffSum;
					level = (BYTE)MIN(255, MAX(0, (int)nlevel));
					OriginalDib.SetPixelColor(tmp_index_x, tmp_index_y, GrayToCOLORREF(level));

					tmp_index_x = x - 2;
					tmp_index_y = y + 2;
					tmp_coeff = 12;
					nlevel = COLORREFToGray(OriginalDib.GetPixelColor(tmp_index_x, tmp_index_y)) + (error * tmp_coeff) / TotalCoeffSum;
					level = (BYTE)MIN(255, MAX(0, (int)nlevel));
					OriginalDib.SetPixelColor(tmp_index_x, tmp_index_y, GrayToCOLORREF(level));

					tmp_index_x = x;
					tmp_coeff = 26;
					nlevel = COLORREFToGray(OriginalDib.GetPixelColor(tmp_index_x, tmp_index_y)) + (error * tmp_coeff) / TotalCoeffSum;
					level = (BYTE)MIN(255, MAX(0, (int)nlevel));
					OriginalDib.SetPixelColor(tmp_index_x, tmp_index_y, GrayToCOLORREF(level));

					tmp_index_x = x + 2;
					tmp_coeff = 12;
					nlevel = COLORREFToGray(OriginalDib.GetPixelColor(tmp_index_x, tmp_index_y)) + (error * tmp_coeff) / TotalCoeffSum;
					level = (BYTE)MIN(255, MAX(0, (int)nlevel));
					OriginalDib.SetPixelColor(tmp_index_x, tmp_index_y, GrayToCOLORREF(level));

					tmp_index_x = x - 3;
					tmp_index_y = y + 2;
					tmp_coeff = 5;
					nlevel = COLORREFToGray(OriginalDib.GetPixelColor(tmp_index_x, tmp_index_y)) + (error * tmp_coeff) / TotalCoeffSum;
					level = (BYTE)MIN(255, MAX(0, (int)nlevel));
					OriginalDib.SetPixelColor(tmp_index_x, tmp_index_y, GrayToCOLORREF(level));

					tmp_index_x = x - 1;
					tmp_coeff = 12;
					nlevel = COLORREFToGray(OriginalDib.GetPixelColor(tmp_index_x, tmp_index_y)) + (error * tmp_coeff) / TotalCoeffSum;
					level = (BYTE)MIN(255, MAX(0, (int)nlevel));
					OriginalDib.SetPixelColor(tmp_index_x, tmp_index_y, GrayToCOLORREF(level));

					tmp_index_x = x + 1;
					tmp_coeff = 12;
					nlevel = COLORREFToGray(OriginalDib.GetPixelColor(tmp_index_x, tmp_index_y)) + (error * tmp_coeff) / TotalCoeffSum;
					level = (BYTE)MIN(255, MAX(0, (int)nlevel));
					OriginalDib.SetPixelColor(tmp_index_x, tmp_index_y, GrayToCOLORREF(level));

					tmp_index_x = x + 3;
					tmp_coeff = 5;
					nlevel = COLORREFToGray(OriginalDib.GetPixelColor(tmp_index_x, tmp_index_y)) + (error * tmp_coeff) / TotalCoeffSum;
					level = (BYTE)MIN(255, MAX(0, (int)nlevel));
					OriginalDib.SetPixelColor(tmp_index_x, tmp_index_y, GrayToCOLORREF(level));
				}
			}
			break;
		}
		case 7:
		{
			// Bayer ordered dither
			int order = 4;

			// Create Bayer matrix
			if (order > 4)
				order = 4;
			int size = (1 << (2*order));
			BYTE* Bmatrix = (BYTE*)new BYTE[size];
			if (!Bmatrix)
				return FALSE;

			for (int i = 0 ; i < size ; i++)
			{
				int n = order;
				int x = i / n;
				int y = i % n;
				int dither = 0;
				while (n-- > 0)
				{
					dither = (((dither<<1)|((x&1) ^ (y&1)))<<1) | (y&1);
					x >>= 1;
					y >>= 1;
				}
				Bmatrix[i] = dither;
			}

			int scale = MAX(0, (8-2*order));
			int level;
			for (int y = 0 ; y < (int)GetHeight() ; y++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, GetHeight());

				for (int x = 0 ; x < (int)GetWidth() ; x++)
				{
					level = COLORREFToGray(OriginalDib.GetPixelColor(x, y)) >> scale;
					if (level > Bmatrix[ (x % order) + order * (y % order) ])
					{
						SetPixelIndex(x, y, 1);
					}
					else
					{
						SetPixelIndex(x, y, 0);
					}
				}
			}

			delete [] Bmatrix;
			
			break;
		}
		default:
		{
			/*
			// Not Optimized Floyd-Steinberg error diffusion (Thanks to Steve McMahon)
			int error, nlevel, coeff;
			BYTE level;

			for (int y = 0 ; y < (int)GetHeight() ; y++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, GetHeight());

				for (int x = 0 ; x < (int)GetWidth() ; x++)
				{
					level = COLORREFToGray(OriginalDib.GetPixelColor(x, y));
					if (level > 128)
					{
						SetPixelIndex(x, y, 1);
						error = level - 255;
					}
					else
					{
						SetPixelIndex(x, y, 0);
						error = level;
					}
					nlevel = COLORREFToGray(OriginalDib.GetPixelColor(x+1, y)) + (error * 7)/16;
					level = (BYTE)MIN(255, MAX(0, (int)nlevel));
					OriginalDib.SetPixelColor(x+1, y, GrayToCOLORREF(level));
					for (int i = -1 ; i < 2 ; i++)
					{
						switch (i)
						{
							case -1:
								coeff=3;
								break;
							case 0:
								coeff=5;
								break;
							case 1:
								coeff=1;
								break;
						}
						nlevel = COLORREFToGray(OriginalDib.GetPixelColor(x+i, y+1)) + (error * coeff)/16;
						level = (BYTE)MIN(255, MAX(0, (int)nlevel));
						OriginalDib.SetPixelColor(x+i, y+1, GrayToCOLORREF(level));
					}
				}
			}
			*/
			
			// Optimized Floyd-Steinberg error diffusion (Thanks to Steve McMahon)
			int error, nlevel, coeff;
			BYTE level;
			COLORREF cr;
			lpBits = m_pBits;
			LPBYTE lpOrigBits = OriginalDib.m_pBits;
			DWORD uiOrigDIBScanLineSize = DWALIGNEDWIDTHBYTES(OriginalDib.GetWidth() * OriginalDib.GetBitCount());
			
			// y = 0 ... y = GetHeight() - 2
			for (int y = 0 ; y < (int)GetHeight() - 1 ; y++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, GetHeight());

				// x = 0
				int x = 0;
				{
					GETPIXELCOLOR24(x,&cr,lpOrigBits);
					level = COLORREFTOGRAY(cr);
					if (level > 128)
					{
						lpBits[x/8] |= (1 << (7-x%8));
						error = level - 255;
					}
					else
					{
						lpBits[x/8] &= ~(1 << (7-x%8));
						error = level;
					}
					GETPIXELCOLOR24(x+1,&cr,lpOrigBits);
					nlevel = COLORREFTOGRAY(cr) + (error * 7)/16;
					level = (BYTE)MIN(255, MAX(0, (int)nlevel));
					SETPIXELCOLOR24(x+1,GRAYTOCOLORREF(level),lpOrigBits);
					for (int i = -1 ; i < 2 ; i++)
					{
						switch (i)
						{
							case -1:
								coeff=3;
								break;
							case 0:
								coeff=5;
								break;
							case 1:
								coeff=1;
								break;
						}
						if (x+i >= 0)
						{
							GETPIXELCOLOR24(x+i,&cr,lpOrigBits + uiOrigDIBScanLineSize);
						}
						else
						{
							cr = RGB(0,0,0);
						}
						nlevel = COLORREFTOGRAY(cr) + (error * coeff)/16;
						level = (BYTE)MIN(255, MAX(0, (int)nlevel));
						if (x+i >= 0)
							SETPIXELCOLOR24(x+i,GRAYTOCOLORREF(level),lpOrigBits + uiOrigDIBScanLineSize);
					}
				}

				// x = 1 ... x = GetWidth() - 2
				for (x = 1 ; x < (int)GetWidth() - 1 ; x++)
				{
					GETPIXELCOLOR24(x,&cr,lpOrigBits);
					level = COLORREFTOGRAY(cr);
					if (level > 128)
					{
						lpBits[x/8] |= (1 << (7-x%8));
						error = level - 255;
					}
					else
					{
						lpBits[x/8] &= ~(1 << (7-x%8));
						error = level;
					}
					GETPIXELCOLOR24(x+1,&cr,lpOrigBits);
					nlevel = COLORREFTOGRAY(cr) + (error * 7)/16;
					level = (BYTE)MIN(255, MAX(0, (int)nlevel));
					SETPIXELCOLOR24(x+1,GRAYTOCOLORREF(level),lpOrigBits);
					for (int i = -1 ; i < 2 ; i++)
					{
						switch (i)
						{
							case -1:
								coeff=3;
								break;
							case 0:
								coeff=5;
								break;
							case 1:
								coeff=1;
								break;
						}
						GETPIXELCOLOR24(x+i,&cr,lpOrigBits + uiOrigDIBScanLineSize);
						nlevel = COLORREFTOGRAY(cr) + (error * coeff)/16;
						level = (BYTE)MIN(255, MAX(0, (int)nlevel));
						SETPIXELCOLOR24(x+i,GRAYTOCOLORREF(level),lpOrigBits + uiOrigDIBScanLineSize);
					}
				}

				// x = GetWidth() - 1
				{
					GETPIXELCOLOR24(x,&cr,lpOrigBits);
					level = COLORREFTOGRAY(cr);
					if (level > 128)
					{
						lpBits[x/8] |= (1 << (7-x%8));
						error = level - 255;
					}
					else
					{
						lpBits[x/8] &= ~(1 << (7-x%8));
						error = level;
					}
					//GETPIXELCOLOR24(x+1,&cr,lpOrigBits);
					cr = RGB(0,0,0);
					nlevel = COLORREFTOGRAY(cr) + (error * 7)/16;
					level = (BYTE)MIN(255, MAX(0, (int)nlevel));
					//SETPIXELCOLOR24(x+1,GRAYTOCOLORREF(level),lpOrigBits);
					for (int i = -1 ; i < 2 ; i++)
					{
						switch (i)
						{
							case -1:
								coeff=3;
								break;
							case 0:
								coeff=5;
								break;
							case 1:
								coeff=1;
								break;
						}
						if (x+i <= ((int)GetWidth() - 1))
						{
							GETPIXELCOLOR24(x+i,&cr,lpOrigBits + uiOrigDIBScanLineSize);
						}
						else
						{
							cr = RGB(0,0,0);
						}
						nlevel = COLORREFTOGRAY(cr) + (error * coeff)/16;
						level = (BYTE)MIN(255, MAX(0, (int)nlevel));
						if (x+i <= ((int)GetWidth() - 1))
							SETPIXELCOLOR24(x+i,GRAYTOCOLORREF(level),lpOrigBits + uiOrigDIBScanLineSize);
					}
				}

				// Next Line
				lpBits += uiDIBScanLineSize;
				lpOrigBits += uiOrigDIBScanLineSize;
			}

			// y = GetHeight() - 1
			{
				// x = 0 ... x = GetWidth() - 2
				int x;
				for (x = 0 ; x < (int)GetWidth() - 1 ; x++)
				{
					GETPIXELCOLOR24(x,&cr,lpOrigBits);
					level = COLORREFTOGRAY(cr);
					if (level > 128)
					{
						lpBits[x/8] |= (1 << (7-x%8));
						error = level - 255;
					}
					else
					{
						lpBits[x/8] &= ~(1 << (7-x%8));
						error = level;
					}
					GETPIXELCOLOR24(x+1,&cr,lpOrigBits);
					nlevel = COLORREFTOGRAY(cr) + (error * 7)/16;
					level = (BYTE)MIN(255, MAX(0, (int)nlevel));
					SETPIXELCOLOR24(x+1,GRAYTOCOLORREF(level),lpOrigBits);
					for (int i = -1 ; i < 2 ; i++)
					{
						switch (i)
						{
							case -1:
								coeff=3;
								break;
							case 0:
								coeff=5;
								break;
							case 1:
								coeff=1;
								break;
						}
						//GETPIXELCOLOR24(x+i,&cr,lpOrigBits + uiOrigDIBScanLineSize);
						cr = RGB(0,0,0);
						nlevel = COLORREFTOGRAY(cr) + (error * coeff)/16;
						level = (BYTE)MIN(255, MAX(0, (int)nlevel));
						//SETPIXELCOLOR24(x+i,GRAYTOCOLORREF(level),lpOrigBits + uiOrigDIBScanLineSize);
					}
				}

				// x = GetWidth() - 1
				{
					GETPIXELCOLOR24(x,&cr,lpOrigBits);
					level = COLORREFTOGRAY(cr);
					if (level > 128)
					{
						lpBits[x/8] |= (1 << (7-x%8));
						error = level - 255;
					}
					else
					{
						lpBits[x/8] &= ~(1 << (7-x%8));
						error = level;
					}
					//GETPIXELCOLOR24(x+1,&cr,lpOrigBits);
					cr = RGB(0,0,0);
					nlevel = COLORREFTOGRAY(cr) + (error * 7)/16;
					level = (BYTE)MIN(255, MAX(0, (int)nlevel));
					//SETPIXELCOLOR24(x+1,GRAYTOCOLORREF(level),lpOrigBits);
					for (int i = -1 ; i < 2 ; i++)
					{
						switch (i)
						{
							case -1:
								coeff=3;
								break;
							case 0:
								coeff=5;
								break;
							case 1:
								coeff=1;
								break;
						}
						//GETPIXELCOLOR24(x+i,&cr,lpOrigBits + uiOrigDIBScanLineSize);
						cr = RGB(0,0,0);
						nlevel = COLORREFTOGRAY(cr) + (error * coeff)/16;
						level = (BYTE)MIN(255, MAX(0, (int)nlevel));
						//SETPIXELCOLOR24(x+i,GRAYTOCOLORREF(level),lpOrigBits + uiOrigDIBScanLineSize);
					}
				}

				// No Next Line
			}

			break;
		}
	}

	DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

	return TRUE;
}

// If pPalette = NULL use a Halftone Palette
BOOL CDib::ConvertTo4bits(	CPalette* pPalette/*=NULL*/,
							CWnd* pProgressWnd/*=NULL*/,
							BOOL bProgressSend/*=TRUE*/)
{
	// Source Colors
	WORD wSourceNumColors = GetNumColors();

	unsigned int line, i;

	if (!m_pBits)
	{
		if (!DibSectionToBits())
			return FALSE;
	}

	if (!m_pBits || !m_pBMI)
		return FALSE;

	if (IsCompressed())
	{
		if (!Decompress(GetBitCount())) // Decompress
			return FALSE;
	}

	// Reset Alpha
	m_bAlpha = FALSE;

	// Palette
	CPalette* pPal;
	if (!pPalette)
	{
		pPal = new CPalette;
		if (!pPal)
			return FALSE;
		if (GetBitCount() <= 8)
		{
			if (wSourceNumColors <= 16)
			{
				RGBQUAD* pColors = new RGBQUAD[16];
				if (!pColors)
				{
					delete pPal;
					return FALSE;
				}
				memset(pColors, 0, 16 * sizeof(RGBQUAD));
				memcpy(pColors, m_pColors, wSourceNumColors * sizeof(RGBQUAD));
				CreatePaletteFromColors(pPal, 16, pColors);
				delete [] pColors;
			}
			else
			{
				RGBQUAD* pColors = new RGBQUAD[16];
				if (!pColors)
				{
					delete pPal;
					return FALSE;
				}
				memcpy(pColors, m_pColors, 16 * sizeof(RGBQUAD));
				CreatePaletteFromColors(pPal, 16, pColors);
				delete [] pColors;
			}
		}
		else
			CreateHalftonePalette(pPal, 16);
	}
	else
		pPal = pPalette;

	// Scan Line Alignments
	DWORD uiDIBSourceScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * GetBitCount());
	DWORD uiDIBTargetScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * 4);

	// Allocate memory
	LPBITMAPINFO lpNewBMI = (LPBITMAPINFO)new BYTE[	sizeof(BITMAPINFOHEADER) +
													pPal->GetEntryCount() * sizeof(RGBQUAD)];
	if (!lpNewBMI)
	{
		if (!pPalette)
		{
			pPal->DeleteObject();
			delete pPal;
		}
		return FALSE;
	}
	LPBYTE lpNewBitsStart = (LPBYTE)BIGALLOC(uiDIBTargetScanLineSize * GetHeight());
	if (!lpNewBitsStart)
	{
		if (!pPalette)
		{
			pPal->DeleteObject();
			delete pPal;
		}
		delete [] lpNewBMI;
		return FALSE;
	}
	LPBYTE lpNewBits = lpNewBitsStart; 
	LPBYTE lpBits = m_pBits;

	// New BMI
	lpNewBMI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	lpNewBMI->bmiHeader.biWidth = GetWidth();
	lpNewBMI->bmiHeader.biHeight = GetHeight();
	lpNewBMI->bmiHeader.biPlanes = 1;
	lpNewBMI->bmiHeader.biBitCount = 4;
	lpNewBMI->bmiHeader.biCompression = BI_RGB;
	lpNewBMI->bmiHeader.biSizeImage = DWALIGNEDWIDTHBYTES(GetWidth() * 4) * GetHeight();
	lpNewBMI->bmiHeader.biXPelsPerMeter = m_pBMI->bmiHeader.biXPelsPerMeter;
	lpNewBMI->bmiHeader.biYPelsPerMeter = m_pBMI->bmiHeader.biYPelsPerMeter;
	lpNewBMI->bmiHeader.biClrUsed = (pPal->GetEntryCount() < 16) ? pPal->GetEntryCount() : 0;
	lpNewBMI->bmiHeader.biClrImportant = 0;
	m_dwImageSize = DWALIGNEDWIDTHBYTES(GetWidth() * 4) * GetHeight();

	DIB_INIT_PROGRESS;

	switch (GetBitCount())
	{
		case 1 :
		{
			// Init Lookup Table for GetClosestColorIndex()
			InitGetClosestColorIndex();
			
			LPBITMAPINFO pOldBMI = m_pBMI;
			RGBQUAD* pOldColors = m_pColors;
			m_pBMI = lpNewBMI;
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + m_pBMI->bmiHeader.biSize);
			FillColorsFromPalette(pPal); // Needed by GetClosestColorIndex()!
			CreatePaletteFromBMI();

			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					int index = GetClosestColorIndex(RGB(pOldColors[(lpBits[i/8] >> (7-(i%8))) & 0x01].rgbRed,
														pOldColors[(lpBits[i/8] >> (7-(i%8))) & 0x01].rgbGreen,
														pOldColors[(lpBits[i/8] >> (7-(i%8))) & 0x01].rgbBlue));
					if (i%2) // LS Nibble
						lpNewBits[i/2] |= index;
					else // MS Nibble
						lpNewBits[i/2] = index << 4;
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}

			delete [] pOldBMI;
			
			break;
		}
		case 4 :
		{
			// Init Lookup Table for GetClosestColorIndex()
			InitGetClosestColorIndex();
			
			LPBITMAPINFO pOldBMI = m_pBMI;
			RGBQUAD* pOldColors = m_pColors;
			m_pBMI = lpNewBMI;
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + m_pBMI->bmiHeader.biSize);
			FillColorsFromPalette(pPal); // Needed by GetClosestColorIndex()!
			CreatePaletteFromBMI();

			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					int index;
					if (i%2) // LS Nibble
					{
						index = GetClosestColorIndex(RGB(pOldColors[lpBits[i/2] & 0x0F].rgbRed,
														pOldColors[lpBits[i/2] & 0x0F].rgbGreen,
														pOldColors[lpBits[i/2] & 0x0F].rgbBlue));
						lpNewBits[i/2] |= index;
					}
					else // MS Nibble
					{
						index = GetClosestColorIndex(RGB(pOldColors[lpBits[i/2] >> 4].rgbRed,
														pOldColors[lpBits[i/2] >> 4].rgbGreen,
														pOldColors[lpBits[i/2] >> 4].rgbBlue));
						lpNewBits[i/2] = index << 4;
					}
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}

			delete [] pOldBMI;

			break;
		}
		case 8 :
		{
			// Init Lookup Table for GetClosestColorIndex()
			InitGetClosestColorIndex();
			
			LPBITMAPINFO pOldBMI = m_pBMI;
			RGBQUAD* pOldColors = m_pColors;
			m_pBMI = lpNewBMI;
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + m_pBMI->bmiHeader.biSize);
			FillColorsFromPalette(pPal); // Needed by GetClosestColorIndex()!
			CreatePaletteFromBMI();

			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					int index = GetClosestColorIndex(RGB(pOldColors[lpBits[i]].rgbRed,
														pOldColors[lpBits[i]].rgbGreen,
														pOldColors[lpBits[i]].rgbBlue));
					if (i%2) // LS Nibble
						lpNewBits[i/2] |= index;
					else // MS Nibble
						lpNewBits[i/2] = index << 4;
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}

			delete [] pOldBMI;

			break;
		}
		case 16 :
		{
			// Init Lookup Table for GetClosestColorIndex()
			InitGetClosestColorIndex();

			delete [] m_pBMI;
			m_pBMI = lpNewBMI;
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + m_pBMI->bmiHeader.biSize);
			FillColorsFromPalette(pPal); // Needed by GetClosestColorIndex()!
			CreatePaletteFromBMI();

			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					BYTE R, G, B;
					DIB16ToRGB(((WORD*)lpBits)[i], &R, &G, &B);
					int index = GetClosestColorIndex(RGB(R, G, B));
					if (i%2) // LS Nibble
						lpNewBits[i/2] |= index;
					else // MS Nibble
						lpNewBits[i/2] = index << 4;
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}

			break;
		}
		case 24 :
		{
			// Init Lookup Table for GetClosestColorIndex()
			InitGetClosestColorIndex();

			delete [] m_pBMI;
			m_pBMI = lpNewBMI;
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + m_pBMI->bmiHeader.biSize);
			FillColorsFromPalette(pPal); // Needed by GetClosestColorIndex()!
			CreatePaletteFromBMI();

			for (line = 0 ; line < GetHeight(); line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					int index = GetClosestColorIndex(RGB(	lpBits[3*i+2],
															lpBits[3*i+1],
															lpBits[3*i]));
					if (i%2) // LS Nibble
						lpNewBits[i/2] |= index;
					else // MS Nibble
						lpNewBits[i/2] = index << 4;
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}

			break;
		}
		case 32 :
		{
			// Init Lookup Table for GetClosestColorIndex()
			InitGetClosestColorIndex();

			delete [] m_pBMI;
			m_pBMI = lpNewBMI;
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + m_pBMI->bmiHeader.biSize);
			FillColorsFromPalette(pPal); // Needed by GetClosestColorIndex()!
			CreatePaletteFromBMI();

			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					BYTE R, G, B;
					DIB32ToRGB(((DWORD*)lpBits)[i], &R, &G, &B);
					int index = GetClosestColorIndex(RGB(R, G, B));
					if (i%2) // LS Nibble
						lpNewBits[i/2] |= index;
					else // MS Nibble
						lpNewBits[i/2] = index << 4;
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}

			break;
		}
		default:
			break;
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
	if (!pPalette)
	{
		pPal->DeleteObject();
		delete pPal;
	}

	m_pBits = lpNewBitsStart;

	DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

	return TRUE;
}

// If pPalette = NULL use a Halftone Palette
BOOL CDib::ConvertTo4bitsPrecise(	CPalette* pPalette/*=NULL*/,
									CWnd* pProgressWnd/*=NULL*/,
									BOOL bProgressSend/*=TRUE*/)
{
	// Source Colors
	WORD wSourceNumColors = GetNumColors();

	unsigned int line, i;

	if (!m_pBits)
	{
		if (!DibSectionToBits())
			return FALSE;
	}

	if (!m_pBits || !m_pBMI)
		return FALSE;

	if (IsCompressed())
	{
		if (!Decompress(GetBitCount())) // Decompress
			return FALSE;
	}

	// Reset Alpha
	m_bAlpha = FALSE;

	// Palette
	CPalette* pPal;
	if (!pPalette)
	{
		pPal = new CPalette;
		if (!pPal)
			return FALSE;
		if (GetBitCount() <= 8)
		{
			if (wSourceNumColors <= 16)
			{
				RGBQUAD* pColors = new RGBQUAD[16];
				if (!pColors)
				{
					delete pPal;
					return FALSE;
				}
				memset(pColors, 0, 16 * sizeof(RGBQUAD));
				memcpy(pColors, m_pColors, wSourceNumColors * sizeof(RGBQUAD));
				CreatePaletteFromColors(pPal, 16, pColors);
				delete [] pColors;
			}
			else
			{
				RGBQUAD* pColors = new RGBQUAD[16];
				if (!pColors)
				{
					delete pPal;
					return FALSE;
				}
				memcpy(pColors, m_pColors, 16 * sizeof(RGBQUAD));
				CreatePaletteFromColors(pPal, 16, pColors);
				delete [] pColors;
			}
		}
		else
			CreateHalftonePalette(pPal, 16);
	}
	else
		pPal = pPalette;

	// Scan Line Alignments
	DWORD uiDIBSourceScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * GetBitCount());
	DWORD uiDIBTargetScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * 4);

	// Allocate memory
	LPBITMAPINFO lpNewBMI = (LPBITMAPINFO)new BYTE[	sizeof(BITMAPINFOHEADER) +
													pPal->GetEntryCount() * sizeof(RGBQUAD)];
	if (!lpNewBMI)
	{
		if (!pPalette)
		{
			pPal->DeleteObject();
			delete pPal;
		}
		return FALSE;
	}
	LPBYTE lpNewBitsStart = (LPBYTE)BIGALLOC(uiDIBTargetScanLineSize * GetHeight());
	if (!lpNewBitsStart)
	{
		if (!pPalette)
		{
			pPal->DeleteObject();
			delete pPal;
		}
		delete [] lpNewBMI;
		return FALSE;
	}
	LPBYTE lpNewBits = lpNewBitsStart; 
	LPBYTE lpBits = m_pBits;

	// New BMI
	lpNewBMI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	lpNewBMI->bmiHeader.biWidth = GetWidth();
	lpNewBMI->bmiHeader.biHeight = GetHeight();
	lpNewBMI->bmiHeader.biPlanes = 1;
	lpNewBMI->bmiHeader.biBitCount = 4;
	lpNewBMI->bmiHeader.biCompression = BI_RGB;
	lpNewBMI->bmiHeader.biSizeImage = DWALIGNEDWIDTHBYTES(GetWidth() * 4) * GetHeight();
	lpNewBMI->bmiHeader.biXPelsPerMeter = m_pBMI->bmiHeader.biXPelsPerMeter;
	lpNewBMI->bmiHeader.biYPelsPerMeter = m_pBMI->bmiHeader.biYPelsPerMeter;
	lpNewBMI->bmiHeader.biClrUsed = (pPal->GetEntryCount() < 16) ? pPal->GetEntryCount() : 0;
	lpNewBMI->bmiHeader.biClrImportant = 0;
	m_dwImageSize = DWALIGNEDWIDTHBYTES(GetWidth() * 4) * GetHeight();

	DIB_INIT_PROGRESS;

	switch (GetBitCount())
	{
		case 1 :
		{	
			LPBITMAPINFO pOldBMI = m_pBMI;
			RGBQUAD* pOldColors = m_pColors;
			m_pBMI = lpNewBMI;
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + m_pBMI->bmiHeader.biSize);
			FillColorsFromPalette(pPal);
			CreatePaletteFromBMI();

			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					int index = m_pPalette->GetNearestPaletteIndex(RGB(	pOldColors[(lpBits[i/8] >> (7-(i%8))) & 0x01].rgbRed,
																		pOldColors[(lpBits[i/8] >> (7-(i%8))) & 0x01].rgbGreen,
																		pOldColors[(lpBits[i/8] >> (7-(i%8))) & 0x01].rgbBlue));
					if (i%2) // LS Nibble
						lpNewBits[i/2] |= index;
					else // MS Nibble
						lpNewBits[i/2] = index << 4;
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}

			delete [] pOldBMI;
			
			break;
		}
		case 4 :
		{	
			LPBITMAPINFO pOldBMI = m_pBMI;
			RGBQUAD* pOldColors = m_pColors;
			m_pBMI = lpNewBMI;
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + m_pBMI->bmiHeader.biSize);
			FillColorsFromPalette(pPal);
			CreatePaletteFromBMI();

			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					int index;
					if (i%2) // LS Nibble
					{
						index = m_pPalette->GetNearestPaletteIndex(RGB(	pOldColors[lpBits[i/2] & 0x0F].rgbRed,
																		pOldColors[lpBits[i/2] & 0x0F].rgbGreen,
																		pOldColors[lpBits[i/2] & 0x0F].rgbBlue));
						lpNewBits[i/2] |= index;
					}
					else // MS Nibble
					{
						index = m_pPalette->GetNearestPaletteIndex(RGB(	pOldColors[lpBits[i/2] >> 4].rgbRed,
																		pOldColors[lpBits[i/2] >> 4].rgbGreen,
																		pOldColors[lpBits[i/2] >> 4].rgbBlue));
						lpNewBits[i/2] = index << 4;
					}
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}

			delete [] pOldBMI;

			break;
		}
		case 8 :
		{	
			LPBITMAPINFO pOldBMI = m_pBMI;
			RGBQUAD* pOldColors = m_pColors;
			m_pBMI = lpNewBMI;
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + m_pBMI->bmiHeader.biSize);
			FillColorsFromPalette(pPal);
			CreatePaletteFromBMI();

			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					int index = m_pPalette->GetNearestPaletteIndex(RGB(	pOldColors[lpBits[i]].rgbRed,
																		pOldColors[lpBits[i]].rgbGreen,
																		pOldColors[lpBits[i]].rgbBlue));
					if (i%2) // LS Nibble
						lpNewBits[i/2] |= index;
					else // MS Nibble
						lpNewBits[i/2] = index << 4;
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}

			delete [] pOldBMI;

			break;
		}
		case 16 :
		{
			delete [] m_pBMI;
			m_pBMI = lpNewBMI;
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + m_pBMI->bmiHeader.biSize);
			FillColorsFromPalette(pPal);
			CreatePaletteFromBMI();

			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					BYTE R, G, B;
					DIB16ToRGB(((WORD*)lpBits)[i], &R, &G, &B);
					int index = m_pPalette->GetNearestPaletteIndex(RGB(R, G, B));
					if (i%2) // LS Nibble
						lpNewBits[i/2] |= index;
					else // MS Nibble
						lpNewBits[i/2] = index << 4;
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}

			break;
		}
		case 24 :
		{
			delete [] m_pBMI;
			m_pBMI = lpNewBMI;
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + m_pBMI->bmiHeader.biSize);
			FillColorsFromPalette(pPal);
			CreatePaletteFromBMI();

			for (line = 0 ; line < GetHeight(); line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					int index = m_pPalette->GetNearestPaletteIndex(RGB(	lpBits[3*i+2],
																		lpBits[3*i+1],
																		lpBits[3*i]));
					if (i%2) // LS Nibble
						lpNewBits[i/2] |= index;
					else // MS Nibble
						lpNewBits[i/2] = index << 4;
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}

			break;
		}
		case 32 :
		{
			delete [] m_pBMI;
			m_pBMI = lpNewBMI;
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + m_pBMI->bmiHeader.biSize);
			FillColorsFromPalette(pPal);
			CreatePaletteFromBMI();

			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					BYTE R, G, B;
					DIB32ToRGB(((DWORD*)lpBits)[i], &R, &G, &B);
					int index = m_pPalette->GetNearestPaletteIndex(RGB(R, G, B));
					if (i%2) // LS Nibble
						lpNewBits[i/2] |= index;
					else // MS Nibble
						lpNewBits[i/2] = index << 4;
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}

			break;
		}
		default:
			break;
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
	if (!pPalette)
	{
		pPal->DeleteObject();
		delete pPal;
	}

	m_pBits = lpNewBitsStart;

	DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

	return TRUE;
}

// If pPalette = NULL use a Halftone Palette
BOOL CDib::ConvertTo4bitsErrDiff(	CPalette* pPalette/*=NULL*/,
									CWnd* pProgressWnd/*=NULL*/,
									BOOL bProgressSend/*=TRUE*/)
{
	// Source Colors
	WORD wSourceNumColors = GetNumColors();

	if (!m_pBits)
	{
		if (!DibSectionToBits())
			return FALSE;
	}

	if (!m_pBits || !m_pBMI)
		return FALSE;

	if (IsCompressed())
	{
		if (!Decompress(GetBitCount())) // Decompress
			return FALSE;
	}

	// Reset Alpha
	m_bAlpha = FALSE;

	// Palette
	CPalette* pPal;
	if (!pPalette)
	{
		pPal = new CPalette;
		if (!pPal)
			return FALSE;
		if (GetBitCount() <= 8)
		{
			if (wSourceNumColors <= 16)
			{
				RGBQUAD* pColors = new RGBQUAD[16];
				if (!pColors)
				{
					delete pPal;
					return FALSE;
				}
				memset(pColors, 0, 16 * sizeof(RGBQUAD));
				memcpy(pColors, m_pColors, wSourceNumColors * sizeof(RGBQUAD));
				CreatePaletteFromColors(pPal, 16, pColors);
				delete [] pColors;
			}
			else
			{
				RGBQUAD* pColors = new RGBQUAD[16];
				if (!pColors)
				{
					delete pPal;
					return FALSE;
				}
				memcpy(pColors, m_pColors, 16 * sizeof(RGBQUAD));
				CreatePaletteFromColors(pPal, 16, pColors);
				delete [] pColors;
			}
		}
		else
			CreateHalftonePalette(pPal, 16);
	}
	else
		pPal = pPalette;

	CDib OriginalDib(*this);
	if (OriginalDib.GetBitCount() != 24)
		OriginalDib.ConvertTo24bits();
	
	// Convert to 4 bits:

	// Target Scanline Size
	DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * 4);

	// Allocate memory
	LPBITMAPINFO lpNewBMI = (LPBITMAPINFO)new BYTE[	sizeof(BITMAPINFOHEADER) +
													pPal->GetEntryCount() * sizeof(RGBQUAD)];
	if (!lpNewBMI)
	{
		if (!pPalette)
		{
			pPal->DeleteObject();
			delete pPal;
		}
		return FALSE;
	}
	LPBYTE lpNewBitsStart = (LPBYTE)BIGALLOC(uiDIBScanLineSize * GetHeight());
	if (!lpNewBitsStart)
	{
		if (!pPalette)
		{
			pPal->DeleteObject();
			delete pPal;
		}
		delete [] lpNewBMI;
		return FALSE;
	}
	LPBYTE lpNewBits = lpNewBitsStart; 
	LPBYTE lpBits = m_pBits;

	// New BMI
	lpNewBMI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	lpNewBMI->bmiHeader.biWidth = GetWidth();
	lpNewBMI->bmiHeader.biHeight = GetHeight();
	lpNewBMI->bmiHeader.biPlanes = 1;
	lpNewBMI->bmiHeader.biBitCount = 4;
	lpNewBMI->bmiHeader.biCompression = BI_RGB;
	lpNewBMI->bmiHeader.biSizeImage = DWALIGNEDWIDTHBYTES(GetWidth() * 4) * GetHeight();
	lpNewBMI->bmiHeader.biXPelsPerMeter = m_pBMI->bmiHeader.biXPelsPerMeter;
	lpNewBMI->bmiHeader.biYPelsPerMeter = m_pBMI->bmiHeader.biYPelsPerMeter;
	lpNewBMI->bmiHeader.biClrUsed = (pPal->GetEntryCount() < 16) ? pPal->GetEntryCount() : 0;
	lpNewBMI->bmiHeader.biClrImportant = 0;
	m_dwImageSize = DWALIGNEDWIDTHBYTES(GetWidth() * 4) * GetHeight();
	delete [] m_pBMI;
	m_pBMI = lpNewBMI;
	m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + m_pBMI->bmiHeader.biSize);
	FillColorsFromPalette(pPal);
	CreatePaletteFromBMI();

	// Free
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

	// New Bits Pointer
	m_pBits = lpNewBitsStart;

	DIB_INIT_PROGRESS;

	/*
	// Not Optimized
	int er, eg, eb;
	COLORREF c, ce;

	for (int y = 0 ; y < (int)GetHeight() ; y++)
	{
		DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, GetHeight());

		for (int x = 0 ; x < (int)GetWidth() ; x++)
		{
			c = OriginalDib.GetPixelColor(x, y);
			SetPixelColor(x, y, c);

			ce = GetPixelColor(x, y);
			er = (int)GetRValue(c) - (int)GetRValue(ce);
			eg = (int)GetGValue(c) - (int)GetGValue(ce);
			eb = (int)GetBValue(c) - (int)GetBValue(ce);

			c = OriginalDib.GetPixelColor(x+1, y);
			c = RGB((BYTE)MIN(255, MAX(0, (int)GetRValue(c) + ((er * 7)/16))),
					(BYTE)MIN(255, MAX(0, (int)GetGValue(c) + ((eg * 7)/16))),
					(BYTE)MIN(255, MAX(0, (int)GetBValue(c) + ((eb * 7)/16))));
			OriginalDib.SetPixelColor(x+1, y, c);
			int coeff;
			for (int i = -1 ; i < 2 ; i++)
			{
				switch (i)
				{
					case -1:
						coeff=2; break;
					case 0:
						coeff=4; break;
					case 1:
						coeff=1; break;
				}
				c = OriginalDib.GetPixelColor(x+i, y+1);
				c = RGB((BYTE)MIN(255, MAX(0, (int)GetRValue(c) + ((er * coeff)/16))),
						(BYTE)MIN(255, MAX(0, (int)GetGValue(c) + ((eg * coeff)/16))),
						(BYTE)MIN(255, MAX(0, (int)GetBValue(c) + ((eb * coeff)/16))));
				OriginalDib.SetPixelColor(x+i, y+1, c);
			}
		}
	}
	*/

	// Optimized
	int er, eg, eb;
	COLORREF c, ce;
	lpBits = m_pBits;
	LPBYTE lpOrigBits = OriginalDib.m_pBits;
	DWORD uiOrigDIBScanLineSize = DWALIGNEDWIDTHBYTES(OriginalDib.GetWidth() * OriginalDib.GetBitCount());

	// Init Lookup Table for SETPIXELCOLOR4FAST
	InitGetClosestColorIndex();

	for (int y = 0 ; y < (int)GetHeight() - 1 ; y++)
	{
		DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, GetHeight());

		// x = 0
		int x = 0;
		{
			GETPIXELCOLOR24(x,&c,lpOrigBits);

			SETPIXELCOLOR4FAST(x,c,this,lpBits);
			GETPIXELCOLOR4(x,&ce,this,lpBits);

			er = (int)GetRValue(c) - (int)GetRValue(ce);
			eg = (int)GetGValue(c) - (int)GetGValue(ce);
			eb = (int)GetBValue(c) - (int)GetBValue(ce);

			GETPIXELCOLOR24(x+1,&c,lpOrigBits);
			c = RGB((BYTE)MIN(255, MAX(0, (int)GetRValue(c) + ((er * 7)/16))),
					(BYTE)MIN(255, MAX(0, (int)GetGValue(c) + ((eg * 7)/16))),
					(BYTE)MIN(255, MAX(0, (int)GetBValue(c) + ((eb * 7)/16))));
			SETPIXELCOLOR24(x+1,c,lpOrigBits);
			
			int coeff;
			for (int i = -1 ; i < 2 ; i++)
			{
				switch (i)
				{
					case -1:
						coeff=2; break;
					case 0:
						coeff=4; break;
					case 1:
						coeff=1; break;
				}
				if (x+i >= 0)
				{
					GETPIXELCOLOR24(x+i,&c,lpOrigBits + uiOrigDIBScanLineSize);
				}
				else
				{
					c = RGB(0,0,0);
				}
				c = RGB((BYTE)MIN(255, MAX(0, (int)GetRValue(c) + ((er * coeff)/16))),
						(BYTE)MIN(255, MAX(0, (int)GetGValue(c) + ((eg * coeff)/16))),
						(BYTE)MIN(255, MAX(0, (int)GetBValue(c) + ((eb * coeff)/16))));
				if (x+i >= 0)
					SETPIXELCOLOR24(x+i,c,lpOrigBits + uiOrigDIBScanLineSize);
			}
		}
		
		// x = 1 ... x = GetWidth() - 2
		for (x = 1 ; x < (int)GetWidth() - 1 ; x++)
		{
			GETPIXELCOLOR24(x,&c,lpOrigBits);

			SETPIXELCOLOR4FAST(x,c,this,lpBits);
			GETPIXELCOLOR4(x,&ce,this,lpBits);

			er = (int)GetRValue(c) - (int)GetRValue(ce);
			eg = (int)GetGValue(c) - (int)GetGValue(ce);
			eb = (int)GetBValue(c) - (int)GetBValue(ce);

			GETPIXELCOLOR24(x+1,&c,lpOrigBits);
			c = RGB((BYTE)MIN(255, MAX(0, (int)GetRValue(c) + ((er * 7)/16))),
					(BYTE)MIN(255, MAX(0, (int)GetGValue(c) + ((eg * 7)/16))),
					(BYTE)MIN(255, MAX(0, (int)GetBValue(c) + ((eb * 7)/16))));
			SETPIXELCOLOR24(x+1,c,lpOrigBits);
			
			int coeff;
			for (int i = -1 ; i < 2 ; i++)
			{
				switch (i)
				{
					case -1:
						coeff=2; break;
					case 0:
						coeff=4; break;
					case 1:
						coeff=1; break;
				}
				GETPIXELCOLOR24(x+i,&c,lpOrigBits + uiOrigDIBScanLineSize);
				c = RGB((BYTE)MIN(255, MAX(0, (int)GetRValue(c) + ((er * coeff)/16))),
						(BYTE)MIN(255, MAX(0, (int)GetGValue(c) + ((eg * coeff)/16))),
						(BYTE)MIN(255, MAX(0, (int)GetBValue(c) + ((eb * coeff)/16))));
				SETPIXELCOLOR24(x+i,c,lpOrigBits + uiOrigDIBScanLineSize);
			}
		}

		// x = GetWidth() - 1
		{
			GETPIXELCOLOR24(x,&c,lpOrigBits);

			SETPIXELCOLOR4FAST(x,c,this,lpBits);
			GETPIXELCOLOR4(x,&ce,this,lpBits);

			er = (int)GetRValue(c) - (int)GetRValue(ce);
			eg = (int)GetGValue(c) - (int)GetGValue(ce);
			eb = (int)GetBValue(c) - (int)GetBValue(ce);

			//GETPIXELCOLOR24(x+1,&c,lpOrigBits);
			c = RGB(0,0,0);
			c = RGB((BYTE)MIN(255, MAX(0, (int)GetRValue(c) + ((er * 7)/16))),
					(BYTE)MIN(255, MAX(0, (int)GetGValue(c) + ((eg * 7)/16))),
					(BYTE)MIN(255, MAX(0, (int)GetBValue(c) + ((eb * 7)/16))));
			//SETPIXELCOLOR24(x+1,c,lpOrigBits);
			
			int coeff;
			for (int i = -1 ; i < 2 ; i++)
			{
				switch (i)
				{
					case -1:
						coeff=2; break;
					case 0:
						coeff=4; break;
					case 1:
						coeff=1; break;
				}
				if (x+i <= ((int)GetWidth() - 1))
				{
					GETPIXELCOLOR24(x+i,&c,lpOrigBits + uiOrigDIBScanLineSize);
				}
				else
				{
					c = RGB(0,0,0);
				}
				c = RGB((BYTE)MIN(255, MAX(0, (int)GetRValue(c) + ((er * coeff)/16))),
						(BYTE)MIN(255, MAX(0, (int)GetGValue(c) + ((eg * coeff)/16))),
						(BYTE)MIN(255, MAX(0, (int)GetBValue(c) + ((eb * coeff)/16))));
				if (x+i <= ((int)GetWidth() - 1))
					SETPIXELCOLOR24(x+i,c,lpOrigBits + uiOrigDIBScanLineSize);
			}
		}

		// Next Line
		lpBits += uiDIBScanLineSize;
		lpOrigBits += uiOrigDIBScanLineSize;
	}

	// y = GetHeight() - 1
	{
		// x = 0 ... x = GetWidth() - 2
		int x;
		for (x = 0 ; x < (int)GetWidth() - 1 ; x++)
		{
			GETPIXELCOLOR24(x,&c,lpOrigBits);

			SETPIXELCOLOR4FAST(x,c,this,lpBits);
			GETPIXELCOLOR4(x,&ce,this,lpBits);

			er = (int)GetRValue(c) - (int)GetRValue(ce);
			eg = (int)GetGValue(c) - (int)GetGValue(ce);
			eb = (int)GetBValue(c) - (int)GetBValue(ce);

			GETPIXELCOLOR24(x+1,&c,lpOrigBits);
			c = RGB((BYTE)MIN(255, MAX(0, (int)GetRValue(c) + ((er * 7)/16))),
					(BYTE)MIN(255, MAX(0, (int)GetGValue(c) + ((eg * 7)/16))),
					(BYTE)MIN(255, MAX(0, (int)GetBValue(c) + ((eb * 7)/16))));
			SETPIXELCOLOR24(x+1,c,lpOrigBits);
			
			int coeff;
			for (int i = -1 ; i < 2 ; i++)
			{
				switch (i)
				{
					case -1:
						coeff=2; break;
					case 0:
						coeff=4; break;
					case 1:
						coeff=1; break;
				}
				//GETPIXELCOLOR24(x+i,&c,lpOrigBits + uiOrigDIBScanLineSize);
				c = RGB(0,0,0);
				c = RGB((BYTE)MIN(255, MAX(0, (int)GetRValue(c) + ((er * coeff)/16))),
						(BYTE)MIN(255, MAX(0, (int)GetGValue(c) + ((eg * coeff)/16))),
						(BYTE)MIN(255, MAX(0, (int)GetBValue(c) + ((eb * coeff)/16))));
				//SETPIXELCOLOR24(x+i,c,lpOrigBits + uiOrigDIBScanLineSize);
			}
		}

		// x = GetWidth() - 1
		{
			GETPIXELCOLOR24(x,&c,lpOrigBits);

			SETPIXELCOLOR4FAST(x,c,this,lpBits);
			GETPIXELCOLOR4(x,&ce,this,lpBits);

			er = (int)GetRValue(c) - (int)GetRValue(ce);
			eg = (int)GetGValue(c) - (int)GetGValue(ce);
			eb = (int)GetBValue(c) - (int)GetBValue(ce);

			//GETPIXELCOLOR24(x+1,&c,lpOrigBits);
			c = RGB(0,0,0);
			c = RGB((BYTE)MIN(255, MAX(0, (int)GetRValue(c) + ((er * 7)/16))),
					(BYTE)MIN(255, MAX(0, (int)GetGValue(c) + ((eg * 7)/16))),
					(BYTE)MIN(255, MAX(0, (int)GetBValue(c) + ((eb * 7)/16))));
			//SETPIXELCOLOR24(x+1,c,lpOrigBits);
			
			int coeff;
			for (int i = -1 ; i < 2 ; i++)
			{
				switch (i)
				{
					case -1:
						coeff=2; break;
					case 0:
						coeff=4; break;
					case 1:
						coeff=1; break;
				}
				//GETPIXELCOLOR24(x+i,&c,lpOrigBits + uiOrigDIBScanLineSize);
				c = RGB(0,0,0);
				c = RGB((BYTE)MIN(255, MAX(0, (int)GetRValue(c) + ((er * coeff)/16))),
						(BYTE)MIN(255, MAX(0, (int)GetGValue(c) + ((eg * coeff)/16))),
						(BYTE)MIN(255, MAX(0, (int)GetBValue(c) + ((eb * coeff)/16))));
				//SETPIXELCOLOR24(x+i,c,lpOrigBits + uiOrigDIBScanLineSize);
			}
		}

		// No Next Line
	}

	if (!pPalette)
	{
		pPal->DeleteObject();
		delete pPal;
	}

	DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

	return TRUE;
}

// If pPalette = NULL use a Halftone Palette
BOOL CDib::ConvertTo8bits(	CPalette* pPalette/*=NULL*/,
							CWnd* pProgressWnd/*=NULL*/,
							BOOL bProgressSend/*=TRUE*/)
{
	// Source Colors
	WORD wSourceNumColors = GetNumColors();

	unsigned int line, i;

	if (!m_pBits)
	{
		if (!DibSectionToBits())
			return FALSE;
	}

	if (!m_pBits || !m_pBMI)
		return FALSE;

	if (IsCompressed())
	{
		if (!Decompress(GetBitCount())) // Decompress
			return FALSE;
	}

	// Reset Alpha
	m_bAlpha = FALSE;

	// Palette
	CPalette* pPal;
	if (!pPalette)
	{
		pPal = new CPalette;
		if (!pPal)
			return FALSE;
		if (GetBitCount() <= 8)
		{
			if (wSourceNumColors <= 256)
			{
				RGBQUAD* pColors = new RGBQUAD[256];
				if (!pColors)
				{
					delete pPal;
					return FALSE;
				}
				memset(pColors, 0, 256 * sizeof(RGBQUAD));
				memcpy(pColors, m_pColors, wSourceNumColors * sizeof(RGBQUAD));
				CreatePaletteFromColors(pPal, 256, pColors);
				delete [] pColors;
			}
			else
			{
				RGBQUAD* pColors = new RGBQUAD[256];
				if (!pColors)
				{
					delete pPal;
					return FALSE;
				}
				memcpy(pColors, m_pColors, 256 * sizeof(RGBQUAD));
				CreatePaletteFromColors(pPal, 256, pColors);
				delete [] pColors;
			}
		}
		else
			CreateHalftonePalette(pPal, 256);
	}
	else
		pPal = pPalette;

	// Scan Line Alignments
	DWORD uiDIBSourceScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * GetBitCount());
	DWORD uiDIBTargetScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * 8);

	// Allocate memory
	LPBITMAPINFO lpNewBMI = (LPBITMAPINFO)new BYTE[	sizeof(BITMAPINFOHEADER) +
													pPal->GetEntryCount() * sizeof(RGBQUAD)];
	if (!lpNewBMI)
	{
		if (!pPalette)
		{
			pPal->DeleteObject();
			delete pPal;
		}
		return FALSE;
	}
	LPBYTE lpNewBitsStart = (LPBYTE)BIGALLOC(uiDIBTargetScanLineSize * GetHeight());
	if (!lpNewBitsStart)
	{
		if (!pPalette)
		{
			pPal->DeleteObject();
			delete pPal;
		}
		delete [] lpNewBMI;
		return FALSE;
	}
	LPBYTE lpNewBits = lpNewBitsStart; 
	LPBYTE lpBits = m_pBits;

	// New BMI
	lpNewBMI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	lpNewBMI->bmiHeader.biWidth = GetWidth();
	lpNewBMI->bmiHeader.biHeight = GetHeight();
	lpNewBMI->bmiHeader.biPlanes = 1;
	lpNewBMI->bmiHeader.biBitCount = 8;
	lpNewBMI->bmiHeader.biCompression = BI_RGB;
	lpNewBMI->bmiHeader.biSizeImage = DWALIGNEDWIDTHBYTES(GetWidth() * 8) * GetHeight();
	lpNewBMI->bmiHeader.biXPelsPerMeter = m_pBMI->bmiHeader.biXPelsPerMeter;
	lpNewBMI->bmiHeader.biYPelsPerMeter = m_pBMI->bmiHeader.biYPelsPerMeter;
	lpNewBMI->bmiHeader.biClrUsed = (pPal->GetEntryCount() < 256) ? pPal->GetEntryCount() : 0;
	lpNewBMI->bmiHeader.biClrImportant = 0;
	m_dwImageSize = DWALIGNEDWIDTHBYTES(GetWidth() * 8) * GetHeight();

	DIB_INIT_PROGRESS;

	switch (GetBitCount())
	{
		case 1 :
		{
			// Init Lookup Table for GetClosestColorIndex()
			InitGetClosestColorIndex();
			
			LPBITMAPINFO pOldBMI = m_pBMI;
			RGBQUAD* pOldColors = m_pColors;
			m_pBMI = lpNewBMI;
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + m_pBMI->bmiHeader.biSize);
			FillColorsFromPalette(pPal); // Needed by GetClosestColorIndex()!
			CreatePaletteFromBMI();

			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					lpNewBits[i] = GetClosestColorIndex(RGB(pOldColors[(lpBits[i/8] >> (7-(i%8))) & 0x01].rgbRed,
															pOldColors[(lpBits[i/8] >> (7-(i%8))) & 0x01].rgbGreen,
															pOldColors[(lpBits[i/8] >> (7-(i%8))) & 0x01].rgbBlue));
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}

			delete [] pOldBMI;
			
			break;
		}
		case 4 :
		{
			// Init Lookup Table for GetClosestColorIndex()
			InitGetClosestColorIndex();
			
			LPBITMAPINFO pOldBMI = m_pBMI;
			RGBQUAD* pOldColors = m_pColors;
			m_pBMI = lpNewBMI;
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + m_pBMI->bmiHeader.biSize);
			FillColorsFromPalette(pPal); // Needed by GetClosestColorIndex()!
			CreatePaletteFromBMI();

			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					if (i%2) // LS Nibble
					{
						lpNewBits[i] = GetClosestColorIndex(RGB(pOldColors[lpBits[i/2] & 0x0F].rgbRed,
																pOldColors[lpBits[i/2] & 0x0F].rgbGreen,
																pOldColors[lpBits[i/2] & 0x0F].rgbBlue));
					}
					else // MS Nibble
					{
						lpNewBits[i] = GetClosestColorIndex(RGB(pOldColors[lpBits[i/2] >> 4].rgbRed,
																pOldColors[lpBits[i/2] >> 4].rgbGreen,
																pOldColors[lpBits[i/2] >> 4].rgbBlue));
					}

				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}

			delete [] pOldBMI;

			break;
		}
		case 8 :
		{
			// Init Lookup Table for GetClosestColorIndex()
			InitGetClosestColorIndex();
			
			LPBITMAPINFO pOldBMI = m_pBMI;
			RGBQUAD* pOldColors = m_pColors;
			m_pBMI = lpNewBMI;
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + m_pBMI->bmiHeader.biSize);
			FillColorsFromPalette(pPal); // Needed by GetClosestColorIndex()!
			CreatePaletteFromBMI();

			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					lpNewBits[i] = GetClosestColorIndex(RGB(pOldColors[lpBits[i]].rgbRed,
															pOldColors[lpBits[i]].rgbGreen,
															pOldColors[lpBits[i]].rgbBlue));
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}

			delete [] pOldBMI;

			break;
		}
		case 16 :
		{
			// Init Lookup Table for GetClosestColorIndex()
			InitGetClosestColorIndex();

			delete [] m_pBMI;
			m_pBMI = lpNewBMI;
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + m_pBMI->bmiHeader.biSize);
			FillColorsFromPalette(pPal); // Needed by GetClosestColorIndex()!
			CreatePaletteFromBMI();

			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					BYTE R, G, B;
					DIB16ToRGB(((WORD*)lpBits)[i], &R, &G, &B);
					lpNewBits[i] = GetClosestColorIndex(RGB(R, G, B));
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}

			break;
		}
		case 24 :
		{
			// Init Lookup Table for GetClosestColorIndex()
			InitGetClosestColorIndex();

			delete [] m_pBMI;
			m_pBMI = lpNewBMI;
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + m_pBMI->bmiHeader.biSize);
			FillColorsFromPalette(pPal); // Needed by GetClosestColorIndex()!
			CreatePaletteFromBMI();

			for (line = 0 ; line < GetHeight(); line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					lpNewBits[i] = GetClosestColorIndex(RGB(lpBits[3*i+2],
															lpBits[3*i+1],
															lpBits[3*i]));
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}

			break;
		}
		case 32 :
		{
			// Init Lookup Table for GetClosestColorIndex()
			InitGetClosestColorIndex();

			delete [] m_pBMI;
			m_pBMI = lpNewBMI;
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + m_pBMI->bmiHeader.biSize);
			FillColorsFromPalette(pPal); // Needed by GetClosestColorIndex()!
			CreatePaletteFromBMI();

			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					BYTE R, G, B;
					DIB32ToRGB(((DWORD*)lpBits)[i], &R, &G, &B);
					lpNewBits[i] = GetClosestColorIndex(RGB(R, G, B));
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}

			break;
		}
		default:
			break;
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
	if (!pPalette)
	{
		pPal->DeleteObject();
		delete pPal;
	}

	m_pBits = lpNewBitsStart;

	DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

	return TRUE;
}

// If pPalette = NULL use a Halftone Palette
BOOL CDib::ConvertTo8bitsPrecise(	CPalette* pPalette/*=NULL*/,
									CWnd* pProgressWnd/*=NULL*/,
									BOOL bProgressSend/*=TRUE*/)
{
	// Source Colors
	WORD wSourceNumColors = GetNumColors();

	unsigned int line, i;

	if (!m_pBits)
	{
		if (!DibSectionToBits())
			return FALSE;
	}

	if (!m_pBits || !m_pBMI)
		return FALSE;

	if (IsCompressed())
	{
		if (!Decompress(GetBitCount())) // Decompress
			return FALSE;
	}

	// Reset Alpha
	m_bAlpha = FALSE;

	// Palette
	CPalette* pPal;
	if (!pPalette)
	{
		pPal = new CPalette;
		if (!pPal)
			return FALSE;
		if (GetBitCount() <= 8)
		{
			if (wSourceNumColors <= 256)
			{
				RGBQUAD* pColors = new RGBQUAD[256];
				if (!pColors)
				{
					delete pPal;
					return FALSE;
				}
				memset(pColors, 0, 256 * sizeof(RGBQUAD));
				memcpy(pColors, m_pColors, wSourceNumColors * sizeof(RGBQUAD));
				CreatePaletteFromColors(pPal, 256, pColors);
				delete [] pColors;
			}
			else
			{
				RGBQUAD* pColors = new RGBQUAD[256];
				if (!pColors)
				{
					delete pPal;
					return FALSE;
				}
				memcpy(pColors, m_pColors, 256 * sizeof(RGBQUAD));
				CreatePaletteFromColors(pPal, 256, pColors);
				delete [] pColors;
			}
		}
		else
			CreateHalftonePalette(pPal, 256);
	}
	else
		pPal = pPalette;

	// Scan Line Alignments
	DWORD uiDIBSourceScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * GetBitCount());
	DWORD uiDIBTargetScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * 8);

	// Allocate memory
	LPBITMAPINFO lpNewBMI = (LPBITMAPINFO)new BYTE[	sizeof(BITMAPINFOHEADER) +
													pPal->GetEntryCount() * sizeof(RGBQUAD)];
	if (!lpNewBMI)
	{
		if (!pPalette)
		{
			pPal->DeleteObject();
			delete pPal;
		}
		return FALSE;
	}
	LPBYTE lpNewBitsStart = (LPBYTE)BIGALLOC(uiDIBTargetScanLineSize * GetHeight());
	if (!lpNewBitsStart)
	{
		if (!pPalette)
		{
			pPal->DeleteObject();
			delete pPal;
		}
		delete [] lpNewBMI;
		return FALSE;
	}
	LPBYTE lpNewBits = lpNewBitsStart; 
	LPBYTE lpBits = m_pBits;

	// New BMI
	lpNewBMI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	lpNewBMI->bmiHeader.biWidth = GetWidth();
	lpNewBMI->bmiHeader.biHeight = GetHeight();
	lpNewBMI->bmiHeader.biPlanes = 1;
	lpNewBMI->bmiHeader.biBitCount = 8;
	lpNewBMI->bmiHeader.biCompression = BI_RGB;
	lpNewBMI->bmiHeader.biSizeImage = DWALIGNEDWIDTHBYTES(GetWidth() * 8) * GetHeight();
	lpNewBMI->bmiHeader.biXPelsPerMeter = m_pBMI->bmiHeader.biXPelsPerMeter;
	lpNewBMI->bmiHeader.biYPelsPerMeter = m_pBMI->bmiHeader.biYPelsPerMeter;
	lpNewBMI->bmiHeader.biClrUsed = (pPal->GetEntryCount() < 256) ? pPal->GetEntryCount() : 0;
	lpNewBMI->bmiHeader.biClrImportant = 0;
	m_dwImageSize = DWALIGNEDWIDTHBYTES(GetWidth() * 8) * GetHeight();

	DIB_INIT_PROGRESS;

	switch (GetBitCount())
	{
		case 1 :
		{	
			LPBITMAPINFO pOldBMI = m_pBMI;
			RGBQUAD* pOldColors = m_pColors;
			m_pBMI = lpNewBMI;
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + m_pBMI->bmiHeader.biSize);
			FillColorsFromPalette(pPal);
			CreatePaletteFromBMI();

			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					lpNewBits[i] = m_pPalette->GetNearestPaletteIndex(RGB(	pOldColors[(lpBits[i/8] >> (7-(i%8))) & 0x01].rgbRed,
																			pOldColors[(lpBits[i/8] >> (7-(i%8))) & 0x01].rgbGreen,
																			pOldColors[(lpBits[i/8] >> (7-(i%8))) & 0x01].rgbBlue));
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}

			delete [] pOldBMI;
			
			break;
		}
		case 4 :
		{	
			LPBITMAPINFO pOldBMI = m_pBMI;
			RGBQUAD* pOldColors = m_pColors;
			m_pBMI = lpNewBMI;
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + m_pBMI->bmiHeader.biSize);
			FillColorsFromPalette(pPal);
			CreatePaletteFromBMI();

			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					if (i%2) // LS Nibble
					{
						lpNewBits[i] = m_pPalette->GetNearestPaletteIndex(RGB(	pOldColors[lpBits[i/2] & 0x0F].rgbRed,
																				pOldColors[lpBits[i/2] & 0x0F].rgbGreen,
																				pOldColors[lpBits[i/2] & 0x0F].rgbBlue));
					}
					else // MS Nibble
					{
						lpNewBits[i] = m_pPalette->GetNearestPaletteIndex(RGB(	pOldColors[lpBits[i/2] >> 4].rgbRed,
																				pOldColors[lpBits[i/2] >> 4].rgbGreen,
																				pOldColors[lpBits[i/2] >> 4].rgbBlue));
					}

				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}

			delete [] pOldBMI;

			break;
		}
		case 8 :
		{	
			LPBITMAPINFO pOldBMI = m_pBMI;
			RGBQUAD* pOldColors = m_pColors;
			m_pBMI = lpNewBMI;
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + m_pBMI->bmiHeader.biSize);
			FillColorsFromPalette(pPal);
			CreatePaletteFromBMI();

			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					lpNewBits[i] = m_pPalette->GetNearestPaletteIndex(RGB(	pOldColors[lpBits[i]].rgbRed,
																			pOldColors[lpBits[i]].rgbGreen,
																			pOldColors[lpBits[i]].rgbBlue));
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}

			delete [] pOldBMI;

			break;
		}
		case 16 :
		{
			delete [] m_pBMI;
			m_pBMI = lpNewBMI;
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + m_pBMI->bmiHeader.biSize);
			FillColorsFromPalette(pPal);
			CreatePaletteFromBMI();

			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					BYTE R, G, B;
					DIB16ToRGB(((WORD*)lpBits)[i], &R, &G, &B);
					lpNewBits[i] = m_pPalette->GetNearestPaletteIndex(RGB(R, G, B));
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}

			break;
		}
		case 24 :
		{
			delete [] m_pBMI;
			m_pBMI = lpNewBMI;
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + m_pBMI->bmiHeader.biSize);
			FillColorsFromPalette(pPal);
			CreatePaletteFromBMI();

			for (line = 0 ; line < GetHeight(); line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					lpNewBits[i] = m_pPalette->GetNearestPaletteIndex(RGB(	lpBits[3*i+2],
																			lpBits[3*i+1],
																			lpBits[3*i]));
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}

			break;
		}
		case 32 :
		{
			delete [] m_pBMI;
			m_pBMI = lpNewBMI;
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + m_pBMI->bmiHeader.biSize);
			FillColorsFromPalette(pPal);
			CreatePaletteFromBMI();

			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					BYTE R, G, B;
					DIB32ToRGB(((DWORD*)lpBits)[i], &R, &G, &B);
					lpNewBits[i] = m_pPalette->GetNearestPaletteIndex(RGB(R, G, B));
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}

			break;
		}
		default:
			break;
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
	if (!pPalette)
	{
		pPal->DeleteObject();
		delete pPal;
	}

	m_pBits = lpNewBitsStart;

	DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

	return TRUE;
}

// If pPalette = NULL use a Halftone Palette
BOOL CDib::ConvertTo8bitsErrDiff(	CPalette* pPalette/*=NULL*/,
									CWnd* pProgressWnd/*=NULL*/,
									BOOL bProgressSend/*=TRUE*/)
{
	// Source Colors
	WORD wSourceNumColors = GetNumColors();

	if (!m_pBits)
	{
		if (!DibSectionToBits())
			return FALSE;
	}

	if (!m_pBits || !m_pBMI)
		return FALSE;

	if (IsCompressed())
	{
		if (!Decompress(GetBitCount())) // Decompress
			return FALSE;
	}

	// Reset Alpha
	m_bAlpha = FALSE;

	// Palette
	CPalette* pPal;
	if (!pPalette)
	{
		pPal = new CPalette;
		if (!pPal)
			return FALSE;
		if (GetBitCount() <= 8)
		{
			if (wSourceNumColors <= 256)
			{
				RGBQUAD* pColors = new RGBQUAD[256];
				if (!pColors)
				{
					delete pPal;
					return FALSE;
				}
				memset(pColors, 0, 256 * sizeof(RGBQUAD));
				memcpy(pColors, m_pColors, wSourceNumColors * sizeof(RGBQUAD));
				CreatePaletteFromColors(pPal, 256, pColors);
				delete [] pColors;
			}
			else
			{
				RGBQUAD* pColors = new RGBQUAD[256];
				if (!pColors)
				{
					delete pPal;
					return FALSE;
				}
				memcpy(pColors, m_pColors, 256 * sizeof(RGBQUAD));
				CreatePaletteFromColors(pPal, 256, pColors);
				delete [] pColors;
			}
		}
		else
			CreateHalftonePalette(pPal, 256);
	}
	else
		pPal = pPalette;

	CDib OriginalDib(*this);
	if (OriginalDib.GetBitCount() != 24)
		OriginalDib.ConvertTo24bits();

	// Convert to 8 bits:

	// Target Scanline Size
	DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * 8);

	// Allocate memory
	LPBITMAPINFO lpNewBMI = (LPBITMAPINFO)new BYTE[	sizeof(BITMAPINFOHEADER) +
													pPal->GetEntryCount() * sizeof(RGBQUAD)];
	if (!lpNewBMI)
	{
		if (!pPalette)
		{
			pPal->DeleteObject();
			delete pPal;
		}
		return FALSE;
	}
	LPBYTE lpNewBitsStart = (LPBYTE)BIGALLOC(uiDIBScanLineSize * GetHeight());
	if (!lpNewBitsStart)
	{
		if (!pPalette)
		{
			pPal->DeleteObject();
			delete pPal;
		}
		delete [] lpNewBMI;
		return FALSE;
	}
	LPBYTE lpNewBits = lpNewBitsStart; 
	LPBYTE lpBits = m_pBits;

	// New BMI
	lpNewBMI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	lpNewBMI->bmiHeader.biWidth = GetWidth();
	lpNewBMI->bmiHeader.biHeight = GetHeight();
	lpNewBMI->bmiHeader.biPlanes = 1;
	lpNewBMI->bmiHeader.biBitCount = 8;
	lpNewBMI->bmiHeader.biCompression = BI_RGB;
	lpNewBMI->bmiHeader.biSizeImage = DWALIGNEDWIDTHBYTES(GetWidth() * 8) * GetHeight();
	lpNewBMI->bmiHeader.biXPelsPerMeter = m_pBMI->bmiHeader.biXPelsPerMeter;
	lpNewBMI->bmiHeader.biYPelsPerMeter = m_pBMI->bmiHeader.biYPelsPerMeter;
	lpNewBMI->bmiHeader.biClrUsed = (pPal->GetEntryCount() < 256) ? pPal->GetEntryCount() : 0;
	lpNewBMI->bmiHeader.biClrImportant = 0;
	m_dwImageSize = DWALIGNEDWIDTHBYTES(GetWidth() * 8) * GetHeight();
	delete [] m_pBMI;
	m_pBMI = lpNewBMI;
	m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + m_pBMI->bmiHeader.biSize);
	FillColorsFromPalette(pPal);
	CreatePaletteFromBMI();

	// Free
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

	// New Bits Pointer
	m_pBits = lpNewBitsStart;

	DIB_INIT_PROGRESS;

	/*
	// Not Optimized
	int er, eg, eb;
	COLORREF c, ce;

	for (int y = 0 ; y < (int)GetHeight() ; y++)
	{
		DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, GetHeight());

		for (int x = 0 ; x < (int)GetWidth() ; x++)
		{
			c = OriginalDib.GetPixelColor(x, y);
			SetPixelColor(x, y, c);

			ce = GetPixelColor(x, y);
			er = (int)GetRValue(c) - (int)GetRValue(ce);
			eg = (int)GetGValue(c) - (int)GetGValue(ce);
			eb = (int)GetBValue(c) - (int)GetBValue(ce);

			c = OriginalDib.GetPixelColor(x+1, y);
			c = RGB((BYTE)MIN(255, MAX(0, (int)GetRValue(c) + ((er * 7)/16))),
					(BYTE)MIN(255, MAX(0, (int)GetGValue(c) + ((eg * 7)/16))),
					(BYTE)MIN(255, MAX(0, (int)GetBValue(c) + ((eb * 7)/16))));
			OriginalDib.SetPixelColor(x+1, y, c);
			int coeff;
			for (int i = -1 ; i < 2 ; i++)
			{
				switch (i)
				{
					case -1:
						coeff=2; break;
					case 0:
						coeff=4; break;
					case 1:
						coeff=1; break;
				}
				c = OriginalDib.GetPixelColor(x+i, y+1);
				c = RGB((BYTE)MIN(255, MAX(0, (int)GetRValue(c) + ((er * coeff)/16))),
						(BYTE)MIN(255, MAX(0, (int)GetGValue(c) + ((eg * coeff)/16))),
						(BYTE)MIN(255, MAX(0, (int)GetBValue(c) + ((eb * coeff)/16))));
				OriginalDib.SetPixelColor(x+i, y+1, c);
			}
		}
	}
	*/

	// Optimized
	int er, eg, eb;
	COLORREF c, ce;
	lpBits = m_pBits;
	LPBYTE lpOrigBits = OriginalDib.m_pBits;
	DWORD uiOrigDIBScanLineSize = DWALIGNEDWIDTHBYTES(OriginalDib.GetWidth() * OriginalDib.GetBitCount());
	
	// Init Lookup Table for SETPIXELCOLOR8FAST
	InitGetClosestColorIndex();

	for (int y = 0 ; y < (int)GetHeight() - 1 ; y++)
	{
		DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, GetHeight());

		// x = 0
		int x = 0;
		{
			GETPIXELCOLOR24(x,&c,lpOrigBits);

			//SETPIXELCOLOR8(x,c,this,lpBits);
			SETPIXELCOLOR8FAST(x,c,this,lpBits);
			GETPIXELCOLOR8(x,&ce,this,lpBits);

			er = (int)GetRValue(c) - (int)GetRValue(ce);
			eg = (int)GetGValue(c) - (int)GetGValue(ce);
			eb = (int)GetBValue(c) - (int)GetBValue(ce);

			GETPIXELCOLOR24(x+1,&c,lpOrigBits);
			c = RGB((BYTE)MIN(255, MAX(0, (int)GetRValue(c) + ((er * 7)/16))),
					(BYTE)MIN(255, MAX(0, (int)GetGValue(c) + ((eg * 7)/16))),
					(BYTE)MIN(255, MAX(0, (int)GetBValue(c) + ((eb * 7)/16))));
			SETPIXELCOLOR24(x+1,c,lpOrigBits);
			
			int coeff;
			for (int i = -1 ; i < 2 ; i++)
			{
				switch (i)
				{
					case -1:
						coeff=2; break;
					case 0:
						coeff=4; break;
					case 1:
						coeff=1; break;
				}
				if (x+i >= 0)
				{
					GETPIXELCOLOR24(x+i,&c,lpOrigBits + uiOrigDIBScanLineSize);
				}
				else
				{
					c = RGB(0,0,0);
				}
				c = RGB((BYTE)MIN(255, MAX(0, (int)GetRValue(c) + ((er * coeff)/16))),
						(BYTE)MIN(255, MAX(0, (int)GetGValue(c) + ((eg * coeff)/16))),
						(BYTE)MIN(255, MAX(0, (int)GetBValue(c) + ((eb * coeff)/16))));
				if (x+i >= 0)
					SETPIXELCOLOR24(x+i,c,lpOrigBits + uiOrigDIBScanLineSize);
			}
		}
		
		// x = 1 ... x = GetWidth() - 2
		for (x = 1 ; x < (int)GetWidth() - 1 ; x++)
		{
			GETPIXELCOLOR24(x,&c,lpOrigBits);

			//SETPIXELCOLOR8(x,c,this,lpBits);
			SETPIXELCOLOR8FAST(x,c,this,lpBits);
			GETPIXELCOLOR8(x,&ce,this,lpBits);

			er = (int)GetRValue(c) - (int)GetRValue(ce);
			eg = (int)GetGValue(c) - (int)GetGValue(ce);
			eb = (int)GetBValue(c) - (int)GetBValue(ce);

			GETPIXELCOLOR24(x+1,&c,lpOrigBits);
			c = RGB((BYTE)MIN(255, MAX(0, (int)GetRValue(c) + ((er * 7)/16))),
					(BYTE)MIN(255, MAX(0, (int)GetGValue(c) + ((eg * 7)/16))),
					(BYTE)MIN(255, MAX(0, (int)GetBValue(c) + ((eb * 7)/16))));
			SETPIXELCOLOR24(x+1,c,lpOrigBits);
			
			int coeff;
			for (int i = -1 ; i < 2 ; i++)
			{
				switch (i)
				{
					case -1:
						coeff=2; break;
					case 0:
						coeff=4; break;
					case 1:
						coeff=1; break;
				}
				GETPIXELCOLOR24(x+i,&c,lpOrigBits + uiOrigDIBScanLineSize);
				c = RGB((BYTE)MIN(255, MAX(0, (int)GetRValue(c) + ((er * coeff)/16))),
						(BYTE)MIN(255, MAX(0, (int)GetGValue(c) + ((eg * coeff)/16))),
						(BYTE)MIN(255, MAX(0, (int)GetBValue(c) + ((eb * coeff)/16))));
				SETPIXELCOLOR24(x+i,c,lpOrigBits + uiOrigDIBScanLineSize);
			}
		}

		// x = GetWidth() - 1
		{
			GETPIXELCOLOR24(x,&c,lpOrigBits);

			//SETPIXELCOLOR8(x,c,this,lpBits);
			SETPIXELCOLOR8FAST(x,c,this,lpBits);
			GETPIXELCOLOR8(x,&ce,this,lpBits);

			er = (int)GetRValue(c) - (int)GetRValue(ce);
			eg = (int)GetGValue(c) - (int)GetGValue(ce);
			eb = (int)GetBValue(c) - (int)GetBValue(ce);

			//GETPIXELCOLOR24(x+1,&c,lpOrigBits);
			c = RGB(0,0,0);
			c = RGB((BYTE)MIN(255, MAX(0, (int)GetRValue(c) + ((er * 7)/16))),
					(BYTE)MIN(255, MAX(0, (int)GetGValue(c) + ((eg * 7)/16))),
					(BYTE)MIN(255, MAX(0, (int)GetBValue(c) + ((eb * 7)/16))));
			//SETPIXELCOLOR24(x+1,c,lpOrigBits);
			
			int coeff;
			for (int i = -1 ; i < 2 ; i++)
			{
				switch (i)
				{
					case -1:
						coeff=2; break;
					case 0:
						coeff=4; break;
					case 1:
						coeff=1; break;
				}
				if (x+i <= ((int)GetWidth() - 1))
				{
					GETPIXELCOLOR24(x+i,&c,lpOrigBits + uiOrigDIBScanLineSize);
				}
				else
				{
					c = RGB(0,0,0);
				}
				c = RGB((BYTE)MIN(255, MAX(0, (int)GetRValue(c) + ((er * coeff)/16))),
						(BYTE)MIN(255, MAX(0, (int)GetGValue(c) + ((eg * coeff)/16))),
						(BYTE)MIN(255, MAX(0, (int)GetBValue(c) + ((eb * coeff)/16))));
				if (x+i <= ((int)GetWidth() - 1))
					SETPIXELCOLOR24(x+i,c,lpOrigBits + uiOrigDIBScanLineSize);
			}
		}

		// Next Line
		lpBits += uiDIBScanLineSize;
		lpOrigBits += uiOrigDIBScanLineSize;
	}

	// y = GetHeight() - 1
	{
		// x = 0 ... x = GetWidth() - 2
		int x;
		for (x = 0 ; x < (int)GetWidth() - 1 ; x++)
		{
			GETPIXELCOLOR24(x,&c,lpOrigBits);

			//SETPIXELCOLOR8(x,c,this,lpBits);
			SETPIXELCOLOR8FAST(x,c,this,lpBits);
			GETPIXELCOLOR8(x,&ce,this,lpBits);

			er = (int)GetRValue(c) - (int)GetRValue(ce);
			eg = (int)GetGValue(c) - (int)GetGValue(ce);
			eb = (int)GetBValue(c) - (int)GetBValue(ce);

			GETPIXELCOLOR24(x+1,&c,lpOrigBits);
			c = RGB((BYTE)MIN(255, MAX(0, (int)GetRValue(c) + ((er * 7)/16))),
					(BYTE)MIN(255, MAX(0, (int)GetGValue(c) + ((eg * 7)/16))),
					(BYTE)MIN(255, MAX(0, (int)GetBValue(c) + ((eb * 7)/16))));
			SETPIXELCOLOR24(x+1,c,lpOrigBits);
			
			int coeff;
			for (int i = -1 ; i < 2 ; i++)
			{
				switch (i)
				{
					case -1:
						coeff=2; break;
					case 0:
						coeff=4; break;
					case 1:
						coeff=1; break;
				}
				//GETPIXELCOLOR24(x+i,&c,lpOrigBits + uiOrigDIBScanLineSize);
				c = RGB(0,0,0);
				c = RGB((BYTE)MIN(255, MAX(0, (int)GetRValue(c) + ((er * coeff)/16))),
						(BYTE)MIN(255, MAX(0, (int)GetGValue(c) + ((eg * coeff)/16))),
						(BYTE)MIN(255, MAX(0, (int)GetBValue(c) + ((eb * coeff)/16))));
				//SETPIXELCOLOR24(x+i,c,lpOrigBits + uiOrigDIBScanLineSize);
			}
		}

		// x = GetWidth() - 1
		{
			GETPIXELCOLOR24(x,&c,lpOrigBits);

			//SETPIXELCOLOR8(x,c,this,lpBits);
			SETPIXELCOLOR8FAST(x,c,this,lpBits);
			GETPIXELCOLOR8(x,&ce,this,lpBits);

			er = (int)GetRValue(c) - (int)GetRValue(ce);
			eg = (int)GetGValue(c) - (int)GetGValue(ce);
			eb = (int)GetBValue(c) - (int)GetBValue(ce);

			//GETPIXELCOLOR24(x+1,&c,lpOrigBits);
			c = RGB(0,0,0);
			c = RGB((BYTE)MIN(255, MAX(0, (int)GetRValue(c) + ((er * 7)/16))),
					(BYTE)MIN(255, MAX(0, (int)GetGValue(c) + ((eg * 7)/16))),
					(BYTE)MIN(255, MAX(0, (int)GetBValue(c) + ((eb * 7)/16))));
			//SETPIXELCOLOR24(x+1,c,lpOrigBits);
			
			int coeff;
			for (int i = -1 ; i < 2 ; i++)
			{
				switch (i)
				{
					case -1:
						coeff=2; break;
					case 0:
						coeff=4; break;
					case 1:
						coeff=1; break;
				}
				//GETPIXELCOLOR24(x+i,&c,lpOrigBits + uiOrigDIBScanLineSize);
				c = RGB(0,0,0);
				c = RGB((BYTE)MIN(255, MAX(0, (int)GetRValue(c) + ((er * coeff)/16))),
						(BYTE)MIN(255, MAX(0, (int)GetGValue(c) + ((eg * coeff)/16))),
						(BYTE)MIN(255, MAX(0, (int)GetBValue(c) + ((eb * coeff)/16))));
				//SETPIXELCOLOR24(x+i,c,lpOrigBits + uiOrigDIBScanLineSize);
			}
		}

		// No Next Line
	}

	if (!pPalette)
	{
		pPal->DeleteObject();
		delete pPal;
	}

	DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

	return TRUE;
}

// Convert to a 5-5-5 16-bit image with biCompression = BI_RGB,
// -> blue mask is 0x001F, the green mask is 0x03E0,
// and the red mask is 0x7C00. Most significant bit is ignored.
BOOL CDib::ConvertTo15bits(	CWnd* pProgressWnd/*=NULL*/,
							BOOL bProgressSend/*=TRUE*/)
{
	// Check
	if (GetBitCount() == 16)
	{
		if (IsRgb16_555())
			return TRUE;
		else
		{
			// First Convert to 24 bpp because
			// otherwise we have a masks conflict!
			if (!ConvertTo24bits(	pProgressWnd,
									bProgressSend))
				return FALSE;
		}
	}

	unsigned int line, i;

	if (!m_pBits)
	{
		if (!DibSectionToBits())
			return FALSE;
	}

	if (!m_pBits || !m_pBMI)
		return FALSE;

	if (IsCompressed())
	{
		if (!Decompress(GetBitCount())) // Decompress
			return FALSE;
	}

	// Reset Alpha
	m_bAlpha = FALSE;

	// Scan Line Alignments
	DWORD uiDIBSourceScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * GetBitCount());
	DWORD uiDIBTargetScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * 16);

	// Allocate memory
	LPBITMAPINFO lpNewBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER)];
	if (!lpNewBMI)
		return FALSE;
	LPBYTE lpNewBitsStart = (LPBYTE)BIGALLOC(uiDIBTargetScanLineSize * GetHeight());
	if (!lpNewBitsStart)
	{
		delete [] lpNewBMI;
		return FALSE;
	}
	LPBYTE lpNewBits = lpNewBitsStart; 
	LPBYTE lpBits = m_pBits;

	// New BMI
	lpNewBMI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	lpNewBMI->bmiHeader.biWidth = GetWidth();
	lpNewBMI->bmiHeader.biHeight = GetHeight();
	lpNewBMI->bmiHeader.biPlanes = 1;
	lpNewBMI->bmiHeader.biBitCount = 16;
	lpNewBMI->bmiHeader.biCompression = BI_RGB;
	lpNewBMI->bmiHeader.biSizeImage = DWALIGNEDWIDTHBYTES(GetWidth() * 16) * GetHeight();
	lpNewBMI->bmiHeader.biXPelsPerMeter = m_pBMI->bmiHeader.biXPelsPerMeter;
	lpNewBMI->bmiHeader.biYPelsPerMeter = m_pBMI->bmiHeader.biYPelsPerMeter;
	lpNewBMI->bmiHeader.biClrUsed = 0;
	lpNewBMI->bmiHeader.biClrImportant = 0;
	m_dwImageSize = DWALIGNEDWIDTHBYTES(GetWidth() * 16) * GetHeight();
	InitMasks(lpNewBMI); // Inits 16 bits masks and leaves 32 bits masks as they are

	DIB_INIT_PROGRESS;

	switch (GetBitCount())
	{
		case 1 :
		{
			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				BYTE R, G, B;
				for (i = 0 ; i < GetWidth() ; i++)
				{
					B = m_pColors[(lpBits[i/8] >> (7-(i%8))) & 0x01].rgbBlue;
					G = m_pColors[(lpBits[i/8] >> (7-(i%8))) & 0x01].rgbGreen;
					R = m_pColors[(lpBits[i/8] >> (7-(i%8))) & 0x01].rgbRed;
					((WORD*)lpNewBits)[i] = RGBToDIB16(R, G, B);
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}
			break;
		}
		case 4 :
		{
			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				BYTE R, G, B;
				for (i = 0 ; i < GetWidth() ; i++)
				{
					if (i%2) // LS Nibble
					{
						B = m_pColors[lpBits[i/2] & 0x0F].rgbBlue;
						G = m_pColors[lpBits[i/2] & 0x0F].rgbGreen;
						R = m_pColors[lpBits[i/2] & 0x0F].rgbRed;
					}
					else // MS Nibble
					{
						B = m_pColors[lpBits[i/2] >> 4].rgbBlue;
						G = m_pColors[lpBits[i/2] >> 4].rgbGreen;
						R = m_pColors[lpBits[i/2] >> 4].rgbRed;
					}
					((WORD*)lpNewBits)[i] = RGBToDIB16(R, G, B);
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}
			break;
		}
		case 8 :
		{
			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				BYTE R, G, B;
				for (i = 0 ; i < GetWidth() ; i++)
				{
					B = m_pColors[lpBits[i]].rgbBlue;
					G = m_pColors[lpBits[i]].rgbGreen;
					R = m_pColors[lpBits[i]].rgbRed;
					((WORD*)lpNewBits)[i] = RGBToDIB16(R, G, B);
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}
			break;
		}
		case 24 :
		{
			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					((WORD*)lpNewBits)[i] = RGBToDIB16(	lpBits[3*i+2],
														lpBits[3*i+1],
														lpBits[3*i]);
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}
			break;
		}
		case 32 :
		{
			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				BYTE R, G, B;
				for (i = 0 ; i < GetWidth() ; i++)
				{
					DIB32ToRGB(((DWORD*)lpBits)[i],	&R, &G, &B);
					((WORD*)lpNewBits)[i] = RGBToDIB16(R, G, B);
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}
			break;
		}
		default:
			break;
	}

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

	m_pBMI = lpNewBMI;
	m_pColors = NULL;
	CreatePaletteFromBMI();

	m_pBits = lpNewBitsStart;

	DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

	return TRUE;
}

// Convert to a 16-bit image with biCompression = BI_BITFIELDS
// and the given masks. Remember that on Win9x only two possible
// combinations are allowed: 
// 1. 5-5-5 16-bit image, same as for biCompression = BI_RGB:
//    blue mask 0x001F, green mask 0x03E0, red mask 0x7C00,
//    and most significant bit is ignored.
//
// 2. 5-6-5 16-bit image with:
//    blue mask 0x001F, green mask 0x07E0, red mask  0xF800,
//    and most significant bit is ignored
BOOL CDib::ConvertTo16bitsMasks(WORD wRedMask/*=0xF800*/,
								WORD wGreenMask/*=0x07E0*/,
								WORD wBlueMask/*=0x001F*/,
								CWnd* pProgressWnd/*=NULL*/,
								BOOL bProgressSend/*=TRUE*/)
{
	// Check
	if (GetBitCount() == 16)
	{
		if (m_wRedMask16 == wRedMask		&&	
			m_wGreenMask16 == wGreenMask	&&
			m_wBlueMask16 == wBlueMask)
			return TRUE;
		else
		{
			// First Convert to 24 bpp because
			// otherwise we have a masks conflict!
			if (!ConvertTo24bits(	pProgressWnd,
									bProgressSend))
				return FALSE;
		}
	}

	unsigned int line, i;

	if (!m_pBits)
	{
		if (!DibSectionToBits())
			return FALSE;
	}

	if (!m_pBits || !m_pBMI)
		return FALSE;

	if (IsCompressed())
	{
		if (!Decompress(GetBitCount())) // Decompress
			return FALSE;
	}

	// Reset Alpha
	m_bAlpha = FALSE;

	// Scan Line Alignments
	DWORD uiDIBSourceScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * GetBitCount());
	DWORD uiDIBTargetScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * 16);

	// Allocate memory
	LPBITMAPINFO lpNewBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER) + 3 * sizeof(DWORD)];
	if (!lpNewBMI)
		return FALSE;
	LPBYTE lpNewBitsStart = (LPBYTE)BIGALLOC(uiDIBTargetScanLineSize * GetHeight());
	if (!lpNewBitsStart)
	{
		delete [] lpNewBMI;
		return FALSE;
	}
	LPBYTE lpNewBits = lpNewBitsStart; 
	LPBYTE lpBits = m_pBits;

	// New BMI
	lpNewBMI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	lpNewBMI->bmiHeader.biWidth = GetWidth();
	lpNewBMI->bmiHeader.biHeight = GetHeight();
	lpNewBMI->bmiHeader.biPlanes = 1;
	lpNewBMI->bmiHeader.biBitCount = 16;
	lpNewBMI->bmiHeader.biCompression = BI_BITFIELDS;
	lpNewBMI->bmiHeader.biSizeImage = DWALIGNEDWIDTHBYTES(GetWidth() * 16) * GetHeight();
	lpNewBMI->bmiHeader.biXPelsPerMeter = m_pBMI->bmiHeader.biXPelsPerMeter;
	lpNewBMI->bmiHeader.biYPelsPerMeter = m_pBMI->bmiHeader.biYPelsPerMeter;
	lpNewBMI->bmiHeader.biClrUsed = 0;
	lpNewBMI->bmiHeader.biClrImportant = 0;
	LPBYTE pDstMask = (LPBYTE)lpNewBMI + sizeof(BITMAPINFOHEADER);
	*((DWORD*)(pDstMask)) = wRedMask;	// Red Mask
	pDstMask += sizeof(DWORD);
	*((DWORD*)(pDstMask)) = wGreenMask; // Green Mask
	pDstMask += sizeof(DWORD);
	*((DWORD*)(pDstMask)) = wBlueMask;	// Blue Mask
	m_dwImageSize = DWALIGNEDWIDTHBYTES(GetWidth() * 16) * GetHeight();
	InitMasks(lpNewBMI); // Inits 16 bits masks and leaves 32 bits masks as they are

	DIB_INIT_PROGRESS;

	switch (GetBitCount())
	{
		case 1 :
		{
			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				BYTE R, G, B;
				for (i = 0 ; i < GetWidth() ; i++)
				{
					B = m_pColors[(lpBits[i/8] >> (7-(i%8))) & 0x01].rgbBlue;
					G = m_pColors[(lpBits[i/8] >> (7-(i%8))) & 0x01].rgbGreen;
					R = m_pColors[(lpBits[i/8] >> (7-(i%8))) & 0x01].rgbRed;
					((WORD*)lpNewBits)[i] = RGBToDIB16(R, G, B);
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}
			break;
		}
		case 4 :
		{
			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				BYTE R, G, B;
				for (i = 0 ; i < GetWidth() ; i++)
				{
					if (i%2) // LS Nibble
					{
						B = m_pColors[lpBits[i/2] & 0x0F].rgbBlue;
						G = m_pColors[lpBits[i/2] & 0x0F].rgbGreen;
						R = m_pColors[lpBits[i/2] & 0x0F].rgbRed;
					}
					else // MS Nibble
					{
						B = m_pColors[lpBits[i/2] >> 4].rgbBlue;
						G = m_pColors[lpBits[i/2] >> 4].rgbGreen;
						R = m_pColors[lpBits[i/2] >> 4].rgbRed;
					}
					((WORD*)lpNewBits)[i] = RGBToDIB16(R, G, B);
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}
			break;
		}
		case 8 :
		{
			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				BYTE R, G, B;
				for (i = 0 ; i < GetWidth() ; i++)
				{
					B = m_pColors[lpBits[i]].rgbBlue;
					G = m_pColors[lpBits[i]].rgbGreen;
					R = m_pColors[lpBits[i]].rgbRed;
					((WORD*)lpNewBits)[i] = RGBToDIB16(R, G, B);
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}
			break;
		}
		case 24 :
		{
			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					((WORD*)lpNewBits)[i] = RGBToDIB16(	lpBits[3*i+2],
														lpBits[3*i+1],
														lpBits[3*i]);
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}
			break;
		}
		case 32 :
		{
			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				BYTE R, G, B;
				for (i = 0 ; i < GetWidth() ; i++)
				{
					DIB32ToRGB(((DWORD*)lpBits)[i],	&R, &G, &B);
					((WORD*)lpNewBits)[i] = RGBToDIB16(R, G, B);
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}
			break;
		}
		default:
			break;
	}

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

	m_pBMI = lpNewBMI;
	m_pColors = NULL;
	CreatePaletteFromBMI();

	m_pBits = lpNewBitsStart;

	DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

	return TRUE;
}

BOOL CDib::ConvertTo24bits(	CWnd* pProgressWnd/*=NULL*/,
							BOOL bProgressSend/*=TRUE*/)
{
	if (GetBitCount() == 24)
		return TRUE;

	unsigned int line, i;

	if (!m_pBits)
	{
		if (!DibSectionToBits())
			return FALSE;
	}

	if (!m_pBits || !m_pBMI)
		return FALSE;

	if (IsCompressed())
	{
		if (!Decompress(GetBitCount())) // Decompress
			return FALSE;
	}

	// Reset Alpha
	m_bAlpha = FALSE;

	// Scan Line Alignments
	DWORD uiDIBSourceScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * GetBitCount());
	DWORD uiDIBTargetScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * 24);

	// Allocate memory
	LPBITMAPINFO lpNewBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER)];
	if (!lpNewBMI)
		return FALSE;
	LPBYTE lpNewBitsStart = (LPBYTE)BIGALLOC(uiDIBTargetScanLineSize * GetHeight());
	if (!lpNewBitsStart)
	{
		delete [] lpNewBMI;
		return FALSE;
	}
	LPBYTE lpNewBits = lpNewBitsStart; 
	LPBYTE lpBits = m_pBits;

	// New BMI
	lpNewBMI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	lpNewBMI->bmiHeader.biWidth = GetWidth();
	lpNewBMI->bmiHeader.biHeight = GetHeight();
	lpNewBMI->bmiHeader.biPlanes = 1;
	lpNewBMI->bmiHeader.biBitCount = 24;
	lpNewBMI->bmiHeader.biCompression = BI_RGB;
	lpNewBMI->bmiHeader.biSizeImage = DWALIGNEDWIDTHBYTES(GetWidth() * 24) * GetHeight();
	lpNewBMI->bmiHeader.biXPelsPerMeter = m_pBMI->bmiHeader.biXPelsPerMeter;
	lpNewBMI->bmiHeader.biYPelsPerMeter = m_pBMI->bmiHeader.biYPelsPerMeter;
	lpNewBMI->bmiHeader.biClrUsed = 0;
	lpNewBMI->bmiHeader.biClrImportant = 0;
	m_dwImageSize = DWALIGNEDWIDTHBYTES(GetWidth() * 24) * GetHeight();

	DIB_INIT_PROGRESS;

	switch (GetBitCount())
	{
		case 1 :
		{
			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					lpNewBits[3*i] = m_pColors[(lpBits[i/8] >> (7-(i%8))) & 0x01].rgbBlue;
					lpNewBits[3*i+1] = m_pColors[(lpBits[i/8] >> (7-(i%8))) & 0x01].rgbGreen;
					lpNewBits[3*i+2] = m_pColors[(lpBits[i/8] >> (7-(i%8))) & 0x01].rgbRed;
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}
			break;
		}
		case 4 :
		{
			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					if (i%2) // LS Nibble
					{
						lpNewBits[3*i] = m_pColors[lpBits[i/2] & 0x0F].rgbBlue;
						lpNewBits[3*i+1] = m_pColors[lpBits[i/2] & 0x0F].rgbGreen;
						lpNewBits[3*i+2] = m_pColors[lpBits[i/2] & 0x0F].rgbRed;
					}
					else // MS Nibble
					{
						lpNewBits[3*i] = m_pColors[lpBits[i/2] >> 4].rgbBlue;
						lpNewBits[3*i+1] = m_pColors[lpBits[i/2] >> 4].rgbGreen;
						lpNewBits[3*i+2] = m_pColors[lpBits[i/2] >> 4].rgbRed;
					}
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}
			break;
		}
		case 8 :
		{
			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					lpNewBits[3*i] =   m_pColors[lpBits[i]].rgbBlue;
					lpNewBits[3*i+1] = m_pColors[lpBits[i]].rgbGreen;
					lpNewBits[3*i+2] = m_pColors[lpBits[i]].rgbRed;
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}
			break;
		}
		case 16 :
		{
			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
					DIB16ToRGB(((WORD*)lpBits)[i],	&(lpNewBits[3*i+2]),
													&(lpNewBits[3*i+1]),
													&(lpNewBits[3*i]));
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}
			break;
		}
		case 32 :
		{
			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
					DIB32ToRGB(((DWORD*)lpBits)[i],	&(lpNewBits[3*i+2]),
													&(lpNewBits[3*i+1]),
													&(lpNewBits[3*i]));	
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}
			break;
		}
		default:
			break;
	}

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

	m_pBMI = lpNewBMI;
	m_pColors = NULL;
	CreatePaletteFromBMI();

	m_pBits = lpNewBitsStart;

	DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

	return TRUE;
}

BOOL CDib::ConvertTo32bits(	CWnd* pProgressWnd/*=NULL*/,
							BOOL bProgressSend/*=TRUE*/)
{
	if (GetBitCount() == 32)
		return TRUE;

	unsigned int line, i;

	if (!m_pBits)
	{
		if (!DibSectionToBits())
			return FALSE;
	}

	if (!m_pBits || !m_pBMI)
		return FALSE;

	if (IsCompressed())
	{
		if (!Decompress(GetBitCount())) // Decompress
			return FALSE;
	}

	// Scan Line Alignments
	DWORD uiDIBSourceScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * GetBitCount());
	DWORD uiDIBTargetScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * 32);

	// Allocate memory
	LPBITMAPINFO lpNewBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER)];
	if (!lpNewBMI)
		return FALSE;
	LPBYTE lpNewBitsStart = (LPBYTE)BIGALLOC(uiDIBTargetScanLineSize * GetHeight());
	if (!lpNewBitsStart)
	{
		delete [] lpNewBMI;
		return FALSE;
	}
	LPBYTE lpNewBits = lpNewBitsStart; 
	LPBYTE lpBits = m_pBits;

	// New BMI
	lpNewBMI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	lpNewBMI->bmiHeader.biWidth = GetWidth();
	lpNewBMI->bmiHeader.biHeight = GetHeight();
	lpNewBMI->bmiHeader.biPlanes = 1;
	lpNewBMI->bmiHeader.biBitCount = 32;
	lpNewBMI->bmiHeader.biCompression = BI_RGB;
	lpNewBMI->bmiHeader.biSizeImage = DWALIGNEDWIDTHBYTES(GetWidth() * 32) * GetHeight();
	lpNewBMI->bmiHeader.biXPelsPerMeter = m_pBMI->bmiHeader.biXPelsPerMeter;
	lpNewBMI->bmiHeader.biYPelsPerMeter = m_pBMI->bmiHeader.biYPelsPerMeter;
	lpNewBMI->bmiHeader.biClrUsed = 0;
	lpNewBMI->bmiHeader.biClrImportant = 0;
	m_dwImageSize = DWALIGNEDWIDTHBYTES(GetWidth() * 32) * GetHeight();
	InitMasks(lpNewBMI); // Inits 32 bits masks and leaves 16 bits masks as they are

	DIB_INIT_PROGRESS;

	switch (GetBitCount())
	{
		case 1 :
		{
			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					lpNewBits[4*i] = m_pColors[(lpBits[i/8] >> (7-(i%8))) & 0x01].rgbBlue;
					lpNewBits[4*i+1] = m_pColors[(lpBits[i/8] >> (7-(i%8))) & 0x01].rgbGreen;
					lpNewBits[4*i+2] = m_pColors[(lpBits[i/8] >> (7-(i%8))) & 0x01].rgbRed;
					lpNewBits[4*i+3] = 0;
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}
			break;
		}
		case 4 :
		{
			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					if (i%2) // LS Nibble
					{
						lpNewBits[4*i] = m_pColors[lpBits[i/2] & 0x0F].rgbBlue;
						lpNewBits[4*i+1] = m_pColors[lpBits[i/2] & 0x0F].rgbGreen;
						lpNewBits[4*i+2] = m_pColors[lpBits[i/2] & 0x0F].rgbRed;
						lpNewBits[4*i+3] = 0;
					}
					else // MS Nibble
					{
						lpNewBits[4*i] = m_pColors[lpBits[i/2] >> 4].rgbBlue;
						lpNewBits[4*i+1] = m_pColors[lpBits[i/2] >> 4].rgbGreen;
						lpNewBits[4*i+2] = m_pColors[lpBits[i/2] >> 4].rgbRed;
						lpNewBits[4*i+3] = 0;
					}
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}
			break;
		}
		case 8 :
		{
			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					lpNewBits[4*i] =   m_pColors[lpBits[i]].rgbBlue;
					lpNewBits[4*i+1] = m_pColors[lpBits[i]].rgbGreen;
					lpNewBits[4*i+2] = m_pColors[lpBits[i]].rgbRed;
					lpNewBits[4*i+3] = 0;
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}
			break;
		}
		case 16 :
		{
			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				BYTE R, G, B;
				for (i = 0 ; i < GetWidth() ; i++)
				{
					DIB16ToRGB(((WORD*)lpBits)[i], &R, &G, &B);
					lpNewBits[4*i] = B;
					lpNewBits[4*i+1] = G;
					lpNewBits[4*i+2] = R;
					lpNewBits[4*i+3] = 0;
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}
			break;
		}
		case 24 :
		{
			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					lpNewBits[4*i] = lpBits[3*i];
					lpNewBits[4*i+1] = lpBits[3*i+1];
					lpNewBits[4*i+2] = lpBits[3*i+2];
					lpNewBits[4*i+3] = 0;
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}
			break;
		}
		default:
			break;
	}

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

	m_pBMI = lpNewBMI;
	m_pColors = NULL;
	CreatePaletteFromBMI();

	m_pBits = lpNewBitsStart;

	DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

	return TRUE;
}

BOOL CDib::ConvertTo32bitsAlpha(BYTE Alpha/*=255*/,
								CWnd* pProgressWnd/*=NULL*/,
								BOOL bProgressSend/*=TRUE*/)
{
	if (GetBitCount() == 32 && m_bAlpha)
		return TRUE;

	unsigned int line, i;

	if (!m_pBits)
	{
		if (!DibSectionToBits())
			return FALSE;
	}

	if (!m_pBits || !m_pBMI)
		return FALSE;

	if (IsCompressed())
	{
		if (!Decompress(GetBitCount())) // Decompress
			return FALSE;
	}

	// Scan Line Alignments
	DWORD uiDIBSourceScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * GetBitCount());
	DWORD uiDIBTargetScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * 32);

	// Allocate memory
	LPBITMAPINFO lpNewBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER)];
	if (!lpNewBMI)
		return FALSE;
	LPBYTE lpNewBitsStart = (LPBYTE)BIGALLOC(uiDIBTargetScanLineSize * GetHeight());
	if (!lpNewBitsStart)
	{
		delete [] lpNewBMI;
		return FALSE;
	}
	LPBYTE lpNewBits = lpNewBitsStart; 
	LPBYTE lpBits = m_pBits;

	// New BMI
	lpNewBMI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	lpNewBMI->bmiHeader.biWidth = GetWidth();
	lpNewBMI->bmiHeader.biHeight = GetHeight();
	lpNewBMI->bmiHeader.biPlanes = 1;
	lpNewBMI->bmiHeader.biBitCount = 32;
	lpNewBMI->bmiHeader.biCompression = BI_RGB;
	lpNewBMI->bmiHeader.biSizeImage = DWALIGNEDWIDTHBYTES(GetWidth() * 32) * GetHeight();
	lpNewBMI->bmiHeader.biXPelsPerMeter = m_pBMI->bmiHeader.biXPelsPerMeter;
	lpNewBMI->bmiHeader.biYPelsPerMeter = m_pBMI->bmiHeader.biYPelsPerMeter;
	lpNewBMI->bmiHeader.biClrUsed = 0;
	lpNewBMI->bmiHeader.biClrImportant = 0;
	m_dwImageSize = DWALIGNEDWIDTHBYTES(GetWidth() * 32) * GetHeight();
	InitMasks(lpNewBMI); // Inits 32 bits masks and leaves 16 bits masks as they are

	DIB_INIT_PROGRESS;

	switch (GetBitCount())
	{
		case 1 :
		{
			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					lpNewBits[4*i] = m_pColors[(lpBits[i/8] >> (7-(i%8))) & 0x01].rgbBlue;
					lpNewBits[4*i+1] = m_pColors[(lpBits[i/8] >> (7-(i%8))) & 0x01].rgbGreen;
					lpNewBits[4*i+2] = m_pColors[(lpBits[i/8] >> (7-(i%8))) & 0x01].rgbRed;
					lpNewBits[4*i+3] = Alpha;
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}
			break;
		}
		case 4 :
		{
			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					if (i%2) // LS Nibble
					{
						lpNewBits[4*i] = m_pColors[lpBits[i/2] & 0x0F].rgbBlue;
						lpNewBits[4*i+1] = m_pColors[lpBits[i/2] & 0x0F].rgbGreen;
						lpNewBits[4*i+2] = m_pColors[lpBits[i/2] & 0x0F].rgbRed;
						lpNewBits[4*i+3] = Alpha;
					}
					else // MS Nibble
					{
						lpNewBits[4*i] = m_pColors[lpBits[i/2] >> 4].rgbBlue;
						lpNewBits[4*i+1] = m_pColors[lpBits[i/2] >> 4].rgbGreen;
						lpNewBits[4*i+2] = m_pColors[lpBits[i/2] >> 4].rgbRed;
						lpNewBits[4*i+3] = Alpha;
					}
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}
			break;
		}
		case 8 :
		{
			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					lpNewBits[4*i] =   m_pColors[lpBits[i]].rgbBlue;
					lpNewBits[4*i+1] = m_pColors[lpBits[i]].rgbGreen;
					lpNewBits[4*i+2] = m_pColors[lpBits[i]].rgbRed;
					lpNewBits[4*i+3] = Alpha;
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}
			break;
		}
		case 16 :
		{
			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				BYTE R, G, B;
				for (i = 0 ; i < GetWidth() ; i++)
				{
					DIB16ToRGB(((WORD*)lpBits)[i], &R, &G, &B);
					lpNewBits[4*i] = B;
					lpNewBits[4*i+1] = G;
					lpNewBits[4*i+2] = R;
					lpNewBits[4*i+3] = Alpha;
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}
			break;
		}
		case 24 :
		{
			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					lpNewBits[4*i] = lpBits[3*i];
					lpNewBits[4*i+1] = lpBits[3*i+1];
					lpNewBits[4*i+2] = lpBits[3*i+2];
					lpNewBits[4*i+3] = Alpha;
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}
			break;
		}
		case 32 :
		{
			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = 0 ; i < GetWidth() ; i++)
				{
					lpNewBits[4*i] = lpBits[4*i];
					lpNewBits[4*i+1] = lpBits[4*i+1];
					lpNewBits[4*i+2] = lpBits[4*i+2];
					lpNewBits[4*i+3] = Alpha;
				}
				lpNewBits += uiDIBTargetScanLineSize;
				lpBits += uiDIBSourceScanLineSize;
			}
			break;
		}
		default:
			break;
	}

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

	m_pBMI = lpNewBMI;
	m_pColors = NULL;
	m_bAlpha = TRUE;
	CreatePaletteFromBMI();

	m_pBits = lpNewBitsStart;

	DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

	return TRUE;
}