#include "stdafx.h"
#include "dib.h"
#include <gdiplus.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BOOL CDib::SetColorUndo()
{
	if (!m_pBits)
	{
		if (!DibSectionToBits())
			return FALSE;
	}

	if (!m_pBits || !m_pBMI)
		return FALSE;

	unsigned int nWidthDWAligned;

	switch (m_pBMI->bmiHeader.biBitCount)
	{
		case 1:
		{
			if (!m_pColors)
				return FALSE;
			WORD wNumColors = GetNumColors();
			if (!m_pOrigColors)
				m_pOrigColors = (RGBQUAD*)new BYTE[wNumColors*sizeof(RGBQUAD)];
			if (!m_pOrigColors)
				return FALSE;

			memcpy((void*)m_pOrigColors, (void*)m_pColors, wNumColors*sizeof(RGBQUAD));

			break;
		}

		case 4:
		{
			if (!m_pColors)
				return FALSE;
			WORD wNumColors = GetNumColors();
			if (!m_pOrigColors)
				m_pOrigColors = (RGBQUAD*)new BYTE[wNumColors*sizeof(RGBQUAD)];
			if (!m_pOrigColors)
				return FALSE;

			memcpy((void*)m_pOrigColors, (void*)m_pColors, wNumColors*sizeof(RGBQUAD));

			break;
		}

		case 8:
		{
			if (!m_pColors)
				return FALSE;
			WORD wNumColors = GetNumColors();
			if (!m_pOrigColors)
				m_pOrigColors = (RGBQUAD*)new BYTE[wNumColors*sizeof(RGBQUAD)];
			if (!m_pOrigColors)
				return FALSE;

			memcpy((void*)m_pOrigColors, (void*)m_pColors, wNumColors*sizeof(RGBQUAD));
			
			break;
		}

		case 16:
			nWidthDWAligned = DWALIGNEDWIDTHBYTES(GetWidth() * 16); // DWORD aligned (in bytes)
			if (!m_pOrigBits)
				m_pOrigBits = (LPBYTE)BIGALLOC(nWidthDWAligned * GetHeight());
			if (!m_pOrigBits)
				return FALSE;

			memcpy((void*)m_pOrigBits, (void*)m_pBits, nWidthDWAligned * GetHeight());
			
			break;

		case 24:
			nWidthDWAligned = DWALIGNEDWIDTHBYTES(GetWidth() * 24); // DWORD aligned (in bytes)
			if (!m_pOrigBits)
				m_pOrigBits = (LPBYTE)BIGALLOC(nWidthDWAligned * GetHeight());
			if (!m_pOrigBits)
				return FALSE;

			memcpy((void*)m_pOrigBits, (void*)m_pBits, nWidthDWAligned * GetHeight());
			
			break;

		case 32:
			nWidthDWAligned = 4*GetWidth();
			if (!m_pOrigBits)
				m_pOrigBits = (LPBYTE)BIGALLOC(nWidthDWAligned * GetHeight());
			if (!m_pOrigBits)
				return FALSE;

			memcpy((void*)m_pOrigBits, (void*)m_pBits, nWidthDWAligned * GetHeight());
			
			break;

		default:
			return FALSE;
	}

	m_bColorUndoSet = TRUE;

	m_wBrightness = 0;
	m_wContrast = 0;
	m_wSaturation = 0;
	m_wHue = 0;

	return TRUE;
}

BOOL CDib::ResetColorUndo()
{
	if (m_pOrigBits)
	{
		BIGFREE(m_pOrigBits);
		m_pOrigBits = NULL;
	}
	if (m_pOrigColors)
	{
		delete [] m_pOrigColors;
		m_pOrigColors = NULL;
	}

	m_bColorUndoSet = FALSE;

	m_wBrightness = 0;
	m_wContrast = 0;
	m_wSaturation = 0;
	m_wHue = 0;
	
	return TRUE;
}

BOOL CDib::UndoColor()
{
	switch (m_pBMI->bmiHeader.biBitCount)
	{
		unsigned int nWidthDWAligned;

		case 1:
			if ( (m_pOrigColors == NULL) || (m_pColors == NULL) )
				break;
			memcpy((void*)m_pColors, (void*)m_pOrigColors, GetNumColors()*sizeof(RGBQUAD));
			CreatePaletteFromBMI();

			nWidthDWAligned = DWALIGNEDWIDTHBYTES(GetWidth()); // DWORD aligned (in bytes)
			if ( (m_pOrigBits == NULL) || (m_pBits == NULL) )
				break;
			memcpy((void*)m_pBits, (void*)m_pOrigBits, nWidthDWAligned * GetHeight());

			break;

		case 4:
			if ( (m_pOrigColors == NULL) || (m_pColors == NULL) )
				break;
			memcpy((void*)m_pColors, (void*)m_pOrigColors, GetNumColors()*sizeof(RGBQUAD));
			CreatePaletteFromBMI();

			nWidthDWAligned = DWALIGNEDWIDTHBYTES(GetWidth() * 4); // DWORD aligned (in bytes)
			if ( (m_pOrigBits == NULL) || (m_pBits == NULL) )
				break;
			memcpy((void*)m_pBits, (void*)m_pOrigBits, nWidthDWAligned * GetHeight());

			break;

		case 8:
			if ( (m_pOrigColors == NULL) || (m_pColors == NULL) )
				break;
			memcpy((void*)m_pColors, (void*)m_pOrigColors, GetNumColors()*sizeof(RGBQUAD));
			CreatePaletteFromBMI();

			nWidthDWAligned = DWALIGNEDWIDTHBYTES(GetWidth() * 8); // DWORD aligned (in bytes)
			if ( (m_pOrigBits == NULL) || (m_pBits == NULL) )
				break;
			memcpy((void*)m_pBits, (void*)m_pOrigBits, nWidthDWAligned * GetHeight());

			break;

		case 16:
			nWidthDWAligned = DWALIGNEDWIDTHBYTES(GetWidth() * 16); // DWORD aligned (in bytes)
			if ( (m_pOrigBits == NULL) || (m_pBits == NULL) )
				break;
			memcpy((void*)m_pBits, (void*)m_pOrigBits, nWidthDWAligned * GetHeight());
			break;

		case 24:
			nWidthDWAligned = DWALIGNEDWIDTHBYTES(GetWidth() * 24); // DWORD aligned (in bytes)
			if ( (m_pOrigBits == NULL) || (m_pBits == NULL) )
				break;
			memcpy((void*)m_pBits, (void*)m_pOrigBits, nWidthDWAligned * GetHeight());
			break;

		case 32:
			nWidthDWAligned = 4*GetWidth();
			if ( (m_pOrigBits == NULL) || (m_pBits == NULL) )
				break;
			memcpy((void*)m_pBits, (void*)m_pOrigBits, nWidthDWAligned * GetHeight());
			break;

		default:
			return FALSE;
	}
	m_wBrightness = 0;
	m_wContrast = 0;
	m_wSaturation = 0;
	m_wHue = 0;
	
	return TRUE;
}

BOOL CDib::AdjustImage(	short brightness,	// -255..255
						short contrast,		// -100..100
						short saturation,	// -100..100
						short hue,			// -180..180
						BOOL bEnableUndo)	// Enable Undo of Adjusted Pixels
{
	// Reset Color Undo
	if (!bEnableUndo)
		ResetColorUndo();

	if ((brightness == m_wBrightness) &&
		(contrast == m_wContrast) &&
		(saturation == m_wSaturation) &&
		(hue == m_wHue))
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

	// Only 24 bpp or 32 bpp
	if (GetBitCount() == 32)
	{
		// Make sure we have a 32 bpp image with alpha or fast (with 0xFF0000, 0xFF00, 0xFF masks)
		if (!HasAlpha() && !IsFast32bpp())
		{
			ConvertTo24bits();
			ConvertTo32bits();
		}
	}
	else if (GetBitCount() < 24)
		ConvertTo24bits();

	// The first time this function is called make a copy of the image
	if (bEnableUndo)
	{
		if (!m_bColorUndoSet)
			SetColorUndo();
		else
			UndoColor(); // Restore the original image
	}

	// GDI plus bitmap
	// GDI plus constructors and functions taking DIBs or
	// pixel buffers as parameter will be referencing them
	// (no copy is made).
	//
	// GDI plus states that if the stride is positive,
	// the bitmap is top-down and scan0 points to the start
	// of first scan line, if the stride is negative, the
	// bitmap is bottom-up and scan0 points to the start of
	// the last scan line.
	// -> to be correct we would have to set the stride to:
	// -uiDIBScanLineSize
	// and supply a pixels pointer of:
	// GetBits() + (GetHeight() - 1) * uiDIBScanLineSize
	// -> but while processing the single pixel colors who
	// cares that CDib is bottom-up!
	Gdiplus::PixelFormat format;
	if (GetBitCount() == 32)
		format = HasAlpha() ? PixelFormat32bppARGB : PixelFormat32bppRGB;
	else
		format = PixelFormat24bppRGB;
	DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * GetBitCount());
	Gdiplus::Bitmap GdiPlusBm(GetWidth(), GetHeight(), uiDIBScanLineSize, format, GetBits());

	// Adjust brightness and contrast
	if (brightness != 0 || contrast != 0)
	{
		Gdiplus::BrightnessContrastParams MyBriConParams;
		MyBriConParams.brightnessLevel = brightness;	// -255 through 255 (a value of 0 specifies no change)
		MyBriConParams.contrastLevel = contrast;		// -100 through 100 (a value of 0 specifies no change)
		Gdiplus::BrightnessContrast MyBriCon;
		MyBriCon.SetParameters(&MyBriConParams);
		GdiPlusBm.ApplyEffect(&MyBriCon, NULL);
	}

	// Adjust saturation and hue
	if (saturation != 0 || hue != 0)
	{
		Gdiplus::HueSaturationLightnessParams MyHSLParams;
		MyHSLParams.hueLevel = hue;						// -180 through 180 (a value of 0 specifies no change)
		MyHSLParams.saturationLevel = saturation;		// -100 through 100 (a value of 0 specifies no change)
		MyHSLParams.lightnessLevel = 0;					// -100 through 100 (a value of 0 specifies no change)
		Gdiplus::HueSaturationLightness MyHSL;
		MyHSL.SetParameters(&MyHSLParams);
		GdiPlusBm.ApplyEffect(&MyHSL, NULL);
	}

	m_wBrightness = brightness;
	m_wContrast = contrast;
	m_wSaturation = saturation;
	m_wHue = hue;

	CreatePaletteFromBMI();

	return TRUE;
}

// Negative
BOOL CDib::Negative(CWnd* pProgressWnd/*=NULL*/, BOOL bProgressSend/*=TRUE*/)
{
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

	WORD wNumColors = GetNumColors();
	unsigned int line, i, nWidthDWAligned;

	DIB_INIT_PROGRESS;

	switch (m_pBMI->bmiHeader.biBitCount)
	{
		case 1:
		case 4:
		case 8:
		{
			for (i = 0; i < (int)wNumColors; i++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, i, wNumColors);

				m_pColors[i].rgbRed = 255 - m_pColors[i].rgbRed;
				m_pColors[i].rgbGreen = 255 - m_pColors[i].rgbGreen;
				m_pColors[i].rgbBlue = 255 - m_pColors[i].rgbBlue;
			}
			CreatePaletteFromBMI();
			break;
		}
		case 16:
		{
			nWidthDWAligned = DWALIGNEDWIDTHBYTES(GetWidth() * 16); // DWORD aligned (in bytes)

			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = (nWidthDWAligned/2) * line ; i < ((nWidthDWAligned/2) * (line+1)) ; i++)
				{
					BYTE R, G, B;
					DIB16ToRGB(((WORD*)m_pBits)[i], &R, &G, &B);
					R = ~R;
					G = ~G;
					B = ~B;
					((WORD*)m_pBits)[i] = RGBToDIB16(R, G, B);
				}
			}
			break;
		}
		case 24:
		{
			nWidthDWAligned = DWALIGNEDWIDTHBYTES(GetWidth() * 24); // DWORD aligned (in bytes)

			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());
			
				for (i = nWidthDWAligned * line ; i < (nWidthDWAligned * (line+1)) ; i++)
					m_pBits[i] = ~m_pBits[i];
			}
			break;
		}
		case 32:
		{
			if (HasAlpha())
			{
				for (line = 0 ; line < GetHeight() ; line++)
				{
					DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

					for (i = GetWidth() * line ; i < (GetWidth() * (line+1)) ; i++)
					{
						BYTE R, G, B, A;
						DIB32ToRGBA(((DWORD*)m_pBits)[i], &R, &G, &B, &A);
						R = ~R;
						G = ~G;
						B = ~B;
						((DWORD*)m_pBits)[i] = RGBAToDIB32(R, G, B, A);
					}
				}
			}
			else
			{
				for (line = 0 ; line < GetHeight() ; line++)
				{
					DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

					for (i = GetWidth() * line ; i < (GetWidth() * (line+1)) ; i++)
					{
						BYTE R, G, B;
						DIB32ToRGB(((DWORD*)m_pBits)[i], &R, &G, &B);
						R = ~R;
						G = ~G;
						B = ~B;
						((DWORD*)m_pBits)[i] = RGBToDIB32(R, G, B);
					}
				}
			}
			break;
		}
		default:
			break;
	}

	DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

	return TRUE;
}

BOOL CDib::NegativeAlpha(CWnd* pProgressWnd/*=NULL*/, BOOL bProgressSend/*=TRUE*/)
{
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

	if (m_pBMI->bmiHeader.biBitCount != 32 || !HasAlpha())
		return FALSE;

	unsigned int line, i;
	DWORD dwPixA, dwPixRGB;

	DIB_INIT_PROGRESS;
	
	for (line = 0 ; line < GetHeight() ; line++)
	{
		DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

		for (i = GetWidth() * line ; i < (GetWidth() * (line+1)) ; i++)
		{
			dwPixRGB = ((DWORD*)m_pBits)[i];
			dwPixA = ~dwPixRGB & 0xFF000000;
			dwPixRGB &= 0x00FFFFFF;
			((DWORD*)m_pBits)[i] = dwPixA | dwPixRGB;
		}
	}

	DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

	return TRUE;
}

// Convert to Grayscale
BOOL CDib::Grayscale(CWnd* pProgressWnd/*=NULL*/, BOOL bProgressSend/*=TRUE*/)
{
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

	WORD wNumColors = GetNumColors();
	unsigned int line, i, nWidthDWAligned;

	DIB_INIT_PROGRESS;

	switch (m_pBMI->bmiHeader.biBitCount)
	{
		case 1:
		case 4:
		case 8:
		{
			for (i = 0; i < (int)wNumColors; i++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, i, wNumColors);

				BYTE Gray = RGBToGray(m_pColors[i].rgbRed, m_pColors[i].rgbGreen, m_pColors[i].rgbBlue);
				m_pColors[i].rgbRed = Gray;
				m_pColors[i].rgbGreen = Gray;
				m_pColors[i].rgbBlue = Gray;
			}
			CreatePaletteFromBMI();
			break;
		}
		case 16:
		{
			nWidthDWAligned = DWALIGNEDWIDTHBYTES(GetWidth() * 16); // DWORD aligned (in bytes)

			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = (nWidthDWAligned/2) * line ; i < ((nWidthDWAligned/2) * (line+1)) ; i++)
				{
					BYTE R, G, B;
					DIB16ToRGB(((WORD*)m_pBits)[i], &R, &G, &B);
					BYTE Gray = RGBToGray(R, G, B);
					((WORD*)m_pBits)[i] = RGBToDIB16(Gray, Gray, Gray);
				}
			}
			break;
		}
		case 24:
		{
			nWidthDWAligned = DWALIGNEDWIDTHBYTES(GetWidth() * 24); // DWORD aligned (in bytes)
		
			for (line = 0 ; line < GetHeight(); line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = nWidthDWAligned * line ; (i+2) < (nWidthDWAligned * (line+1)) ; i = i+3)
				{
					BYTE Gray = RGBToGray(m_pBits[i+2], m_pBits[i+1], m_pBits[i]);
					m_pBits[i] = (BYTE)Gray;
					m_pBits[i+1] = (BYTE)Gray;
					m_pBits[i+2] = (BYTE)Gray;
				}
			}
			break;
		}
		case 32:
		{
			if (HasAlpha())
			{
				for (line = 0 ; line < GetHeight() ; line++)
				{
					DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

					for (i = GetWidth() * line ; i < (GetWidth() * (line+1)) ; i++)
					{
						BYTE R, G, B, A;
						DIB32ToRGBA(((DWORD*)m_pBits)[i], &R, &G, &B, &A);
						BYTE Gray = RGBToGray(R, G, B);
						((DWORD*)m_pBits)[i] = RGBAToDIB32(Gray, Gray, Gray, A);
					}
				}
			}
			else
			{
				for (line = 0 ; line < GetHeight() ; line++)
				{
					DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

					for (i = GetWidth() * line ; i < (GetWidth() * (line+1)) ; i++)
					{
						BYTE R, G, B;
						DIB32ToRGB(((DWORD*)m_pBits)[i], &R, &G, &B);
						BYTE Gray = RGBToGray(R, G, B);
						((DWORD*)m_pBits)[i] = RGBToDIB32(Gray, Gray, Gray);
					}
				}
			}
			break;
		}
		default:
			break;
	}

	m_bGrayscale = TRUE;

	DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

	return TRUE;
}

BOOL CDib::UpdateGrayscaleFlag()
{
	if (!m_pBMI)
		return FALSE;

	if (m_pBMI->bmiHeader.biBitCount > 8)
		return FALSE;

	if (!m_pBits)
	{
		if (!DibSectionToBits())
			return FALSE;
	}

	if (!m_pBits)
		return FALSE;

	if (IsCompressed())
	{
		if (!Decompress(GetBitCount())) // Decompress
			return FALSE;
	}

	if (!m_pColors)
		return FALSE;

    for (int i = 0 ; i < GetNumColors() ; i++)
	{
		if (m_pColors[i].rgbRed != m_pColors[i].rgbGreen ||
			m_pColors[i].rgbGreen != m_pColors[i].rgbBlue)
		{
			m_bGrayscale = FALSE;
			return TRUE;
		}
	}
	
	m_bGrayscale = TRUE;
	return TRUE;
}

BOOL CDib::MakeGrayscaleAscending(CWnd* pProgressWnd/*=NULL*/, BOOL bProgressSend/*=TRUE*/)
{
	if (!m_pBMI || !m_bGrayscale)
		return FALSE;

	if (m_pBMI->bmiHeader.biBitCount > 8)
		return FALSE;

	if (!m_pBits)
	{
		if (!DibSectionToBits())
			return FALSE;
	}

	if (!m_pBits)
		return FALSE;

	if (IsCompressed())
	{
		if (!Decompress(GetBitCount())) // Decompress
			return FALSE;
	}

	DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * GetBitCount());
	int i, j, gray;
	LPBYTE lpBits = m_pBits;
	int nPaletteEntries = 1 << m_pBMI->bmiHeader.biBitCount;

	DIB_INIT_PROGRESS;

	switch (m_pBMI->bmiHeader.biBitCount)
	{
		case 1:
		{
			for (unsigned int y = 0 ; y < GetHeight() ; y++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, GetHeight());

				for (unsigned int x = 0 ; x < GetWidth() ; x++)
				{
					i = (lpBits[x/8] >> (7-x%8)) & 0x01;
					gray = m_pColors[i].rgbRed;
					j = gray * (nPaletteEntries - 1) / 255;
					lpBits[x/8] &= ~(0x01 << (7-x%8));
					lpBits[x/8] |= (j << (7-x%8));
				}

				lpBits += uiDIBScanLineSize;
			}
			break;
		}
		case 4:
		{
			for (unsigned int y = 0 ; y < GetHeight() ; y++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, GetHeight());

				for (unsigned int x = 0 ; x < GetWidth() ; x++)
				{
					i = (lpBits[x/2] >> (4*(1-x%2))) & 0x0F;
					gray = m_pColors[i].rgbRed;
					j = gray * (nPaletteEntries - 1) / 255;
					lpBits[x/2] &= ~(0x0F << (4*(1-x%2)));
					lpBits[x/2] |= (j << (4*(1-x%2)));
				}

				lpBits += uiDIBScanLineSize;
			}
			break;
		}
		case 8:
		{
			for (unsigned int y = 0 ; y < GetHeight() ; y++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, y, GetHeight());

				for (unsigned int x = 0 ; x < GetWidth() ; x++)
				{
					i = lpBits[x];
					gray = m_pColors[i].rgbRed;
					lpBits[x] = gray;
				}

				lpBits += uiDIBScanLineSize;
			}
			break;
		}
	}

	// Create Ascending Grayscale Colors
	FillGrayscaleColors(m_pColors, GetNumColors());

	// Create Palette
	CreatePaletteFromBMI();

	DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

	return TRUE;

}

__forceinline BOOL CDib::FloodFillPop(int &x, int &y)
{
    if (m_nFloodFillStackPos > 1)
    {
		y = m_pFloodFillStack[--m_nFloodFillStackPos];
		x = m_pFloodFillStack[--m_nFloodFillStackPos];
        return TRUE;
    }    
    else
        return FALSE;
}   
 
__forceinline BOOL CDib::FloodFillPush(int x, int y)
{
    if (m_nFloodFillStackPos < m_nFloodFillStackSize - 2)
    {
        m_pFloodFillStack[m_nFloodFillStackPos++] = x;
		m_pFloodFillStack[m_nFloodFillStackPos++] = y;
        return TRUE;
    }    
    else
	{
		int* p = new int[2 * m_nFloodFillStackSize];
		if (p)
		{
			memcpy(p, m_pFloodFillStack, m_nFloodFillStackPos * sizeof(int));
			m_nFloodFillStackSize *= 2;
			delete [] m_pFloodFillStack;
			m_pFloodFillStack = p;
			return FloodFillPush(x, y);
		}
		else
			return FALSE;
	}
}

int CDib::FloodFillColor(int x, int y, COLORREF crNewColor, COLORREF crOldColor)
{
	// Optimized?
	if (GetBitCount() == 32)
	{
		if (m_bAlpha)
			return FloodFill32Alpha(x, y, crNewColor, crOldColor);
		else if (IsFast32bpp())
			return FloodFill32(x, y, crNewColor, crOldColor);
	}
	else if (GetBitCount() == 24)
		return FloodFill24(x, y, crNewColor, crOldColor);
	else if (GetBitCount() == 16)
	{	
		WORD pixel = RGBToDIB16(GetRValue(crOldColor),
								GetGValue(crOldColor),
								GetBValue(crOldColor));
		crOldColor = DIB16ToCOLORREF(pixel);
		pixel = RGBToDIB16(	GetRValue(crNewColor),
							GetGValue(crNewColor),
							GetBValue(crNewColor));
		crNewColor = DIB16ToCOLORREF(pixel);
	}
	else if (GetBitCount() <= 8)
	{	
		return FloodFillIndex(x, y,
			m_pPalette->GetNearestPaletteIndex(crNewColor),
			m_pPalette->GetNearestPaletteIndex(crOldColor));
	}

	// Check
    if (crNewColor == crOldColor)
		return 0;

	// Top-Down to Bottom-Up Coordinates and check
	y = GetHeight() - 1 - y;
	if (x < 0 || x >= (int)GetWidth() || y < 0 || y >= (int)GetHeight())
		return 0;

	// Allocate Stack
	m_nFloodFillStackSize = FLOODFILL_INITSTACKSIZE;
	m_pFloodFillStack = new int[m_nFloodFillStackSize];
	if (!m_pFloodFillStack)
		return -1;
	m_nFloodFillStackPos = 0;
    
    int x1; 
    BOOL spanTop, spanBottom;
	int nCount = 0;

    FloodFillPush(x, y);
    while (FloodFillPop(x, y))
    {    
		// Go to the left most matching color
        x1 = x;
        while (x1 >= 0 && GetPixelColor(x1, y) == crOldColor)
			x1--;
        x1++;

		// Go right as long as we have a color match
        spanTop = spanBottom = FALSE;
        while (x1 < (int)GetWidth() && GetPixelColor(x1, y) == crOldColor)
        {          
			// Set Color & inc. count
            SetPixelColor(x1, y, crNewColor);
			nCount++;
			
			// Check top line and add point to stack
            if (!spanTop && y > 0 && GetPixelColor(x1, y - 1) == crOldColor) 
            {
                FloodFillPush(x1, y - 1);
                spanTop = TRUE;
            }
			// Top line interrupted start new
            else if (spanTop && y > 0 && GetPixelColor(x1, y - 1) != crOldColor)
                spanTop = FALSE;
           
			// Check bottom line and add point to stack
            if (!spanBottom && y < (int)GetHeight() - 1 && GetPixelColor(x1, y + 1) == crOldColor) 
            {
                FloodFillPush(x1, y + 1);
                spanBottom = TRUE;
            }
			// Bottom line interrupted start new
            else if (spanBottom && y < (int)GetHeight() - 1 && GetPixelColor(x1, y + 1) != crOldColor)
                spanBottom = FALSE;

			// Next
            x1++;                    
        }
    }

	// Clean-Up
	delete [] m_pFloodFillStack;
	m_pFloodFillStack = NULL;

	return nCount;
}

int CDib::FloodFill24(int x, int y, COLORREF crNewColor, COLORREF crOldColor)
{
	// Check
    if (crNewColor == crOldColor)
		return 0;

	// Top-Down to Bottom-Up Coordinates and check
	y = GetHeight() - 1 - y;
	if (x < 0 || x >= (int)GetWidth() || y < 0 || y >= (int)GetHeight())
		return 0;

	// Allocate Stack
	m_nFloodFillStackSize = FLOODFILL_INITSTACKSIZE;
	m_pFloodFillStack = new int[m_nFloodFillStackSize];
	if (!m_pFloodFillStack)
		return -1;
	m_nFloodFillStackPos = 0;
    
    int x1;
    BOOL spanTop, spanBottom;
	int nCount = 0;

	// Scan Line Alignment
	DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * GetBitCount());

	// Get Bits
	LPBYTE lpBits = GetBits();

	// Colors
	BYTE oldR = GetRValue(crOldColor);
	BYTE oldG = GetGValue(crOldColor);
	BYTE oldB = GetBValue(crOldColor);
	BYTE newR = GetRValue(crNewColor);
	BYTE newG = GetGValue(crNewColor);
	BYTE newB = GetBValue(crNewColor);

    FloodFillPush(x, y);
    while (FloodFillPop(x, y))
    {    
        x1 = x;

        while (	x1 >= 0											&&
				lpBits[y*uiDIBScanLineSize + 3*x1]		== oldB &&
				lpBits[y*uiDIBScanLineSize + 3*x1 + 1]	== oldG &&
				lpBits[y*uiDIBScanLineSize + 3*x1 + 2]	== oldR)
			x1--;
        x1++;

        spanTop = spanBottom = FALSE;
        while (	x1 < (int)GetWidth()							&&
				lpBits[y*uiDIBScanLineSize + 3*x1]		== oldB &&
				lpBits[y*uiDIBScanLineSize + 3*x1 + 1]	== oldG &&
				lpBits[y*uiDIBScanLineSize + 3*x1 + 2]	== oldR)
        {           
			lpBits[y*uiDIBScanLineSize + 3*x1]     = newB;
			lpBits[y*uiDIBScanLineSize + 3*x1 + 1] = newG;
			lpBits[y*uiDIBScanLineSize + 3*x1 + 2] = newR;
			
			nCount++;

            if (!spanTop	&&
				y > 0		&&
				lpBits[(y-1)*uiDIBScanLineSize + 3*x1]		== oldB &&
				lpBits[(y-1)*uiDIBScanLineSize + 3*x1 + 1]	== oldG &&
				lpBits[(y-1)*uiDIBScanLineSize + 3*x1 + 2]	== oldR)
            {
                FloodFillPush(x1, y - 1);
                spanTop = TRUE;
            }
            else if (spanTop	&&
					y > 0		&&
					(lpBits[(y-1)*uiDIBScanLineSize + 3*x1]		!= oldB ||
					lpBits[(y-1)*uiDIBScanLineSize + 3*x1 + 1]	!= oldG ||
					lpBits[(y-1)*uiDIBScanLineSize + 3*x1 + 2]	!= oldR))
                spanTop = FALSE;
           
            if (!spanBottom					&&
				y < (int)GetHeight() - 1	&&
				lpBits[(y+1)*uiDIBScanLineSize + 3*x1]		== oldB &&
				lpBits[(y+1)*uiDIBScanLineSize + 3*x1 + 1]	== oldG &&
				lpBits[(y+1)*uiDIBScanLineSize + 3*x1 + 2]	== oldR)
            {
                FloodFillPush(x1, y + 1);
                spanBottom = TRUE;
            }
            else if (spanBottom					&&
					y < (int)GetHeight() - 1	&&
					(lpBits[(y+1)*uiDIBScanLineSize + 3*x1]		!= oldB ||
					lpBits[(y+1)*uiDIBScanLineSize + 3*x1 + 1]	!= oldG ||
					lpBits[(y+1)*uiDIBScanLineSize + 3*x1 + 2]	!= oldR))
                spanBottom = FALSE;

            x1++;                    
        }
    }

	// Clean-Up
	delete [] m_pFloodFillStack;
	m_pFloodFillStack = NULL;

	return nCount;
}

int CDib::FloodFill32(int x, int y, COLORREF crNewColor, COLORREF crOldColor)
{
	// Check
    if (crNewColor == crOldColor)
		return 0;

	// Top-Down to Bottom-Up Coordinates and check
	y = GetHeight() - 1 - y;
	if (x < 0 || x >= (int)GetWidth() || y < 0 || y >= (int)GetHeight())
		return 0;

	// Allocate Stack
	m_nFloodFillStackSize = FLOODFILL_INITSTACKSIZE;
	m_pFloodFillStack = new int[m_nFloodFillStackSize];
	if (!m_pFloodFillStack)
		return -1;
	m_nFloodFillStackPos = 0;
    
    int x1;
    BOOL spanTop, spanBottom;
	int nCount = 0;

	// Scan Line Alignment
	DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * GetBitCount());

	// Get Bits
	LPBYTE lpBits = GetBits();

	// Colors
	BYTE oldR = GetRValue(crOldColor);
	BYTE oldG = GetGValue(crOldColor);
	BYTE oldB = GetBValue(crOldColor);
	BYTE newR = GetRValue(crNewColor);
	BYTE newG = GetGValue(crNewColor);
	BYTE newB = GetBValue(crNewColor);

    FloodFillPush(x, y);
    while (FloodFillPop(x, y))
    {    
        x1 = x;

        while (	x1 >= 0											&&
				lpBits[y*uiDIBScanLineSize + 4*x1]		== oldB &&
				lpBits[y*uiDIBScanLineSize + 4*x1 + 1]	== oldG &&
				lpBits[y*uiDIBScanLineSize + 4*x1 + 2]	== oldR)
			x1--;
        x1++;

        spanTop = spanBottom = FALSE;
        while (	x1 < (int)GetWidth()							&&
				lpBits[y*uiDIBScanLineSize + 4*x1]		== oldB &&
				lpBits[y*uiDIBScanLineSize + 4*x1 + 1]	== oldG &&
				lpBits[y*uiDIBScanLineSize + 4*x1 + 2]	== oldR)
        {           
			lpBits[y*uiDIBScanLineSize + 4*x1]     = newB;
			lpBits[y*uiDIBScanLineSize + 4*x1 + 1] = newG;
			lpBits[y*uiDIBScanLineSize + 4*x1 + 2] = newR;
			
			nCount++;

            if (!spanTop	&&
				y > 0		&&
				lpBits[(y-1)*uiDIBScanLineSize + 4*x1]		== oldB &&
				lpBits[(y-1)*uiDIBScanLineSize + 4*x1 + 1]	== oldG &&
				lpBits[(y-1)*uiDIBScanLineSize + 4*x1 + 2]	== oldR)
            {
                FloodFillPush(x1, y - 1);
                spanTop = TRUE;
            }
            else if (spanTop	&&
					y > 0		&&
					(lpBits[(y-1)*uiDIBScanLineSize + 4*x1]		!= oldB ||
					lpBits[(y-1)*uiDIBScanLineSize + 4*x1 + 1]	!= oldG ||
					lpBits[(y-1)*uiDIBScanLineSize + 4*x1 + 2]	!= oldR))
                spanTop = FALSE;
           
            if (!spanBottom					&&
				y < (int)GetHeight() - 1	&&
				lpBits[(y+1)*uiDIBScanLineSize + 4*x1]		== oldB &&
				lpBits[(y+1)*uiDIBScanLineSize + 4*x1 + 1]	== oldG &&
				lpBits[(y+1)*uiDIBScanLineSize + 4*x1 + 2]	== oldR)
            {
                FloodFillPush(x1, y + 1);
                spanBottom = TRUE;
            }
            else if (spanBottom					&&
					y < (int)GetHeight() - 1	&&
					(lpBits[(y+1)*uiDIBScanLineSize + 4*x1]		!= oldB ||
					lpBits[(y+1)*uiDIBScanLineSize + 4*x1 + 1]	!= oldG ||
					lpBits[(y+1)*uiDIBScanLineSize + 4*x1 + 2]	!= oldR))
                spanBottom = FALSE;

            x1++;                    
        }
    }

	// Clean-Up
	delete [] m_pFloodFillStack;
	m_pFloodFillStack = NULL;

	return nCount;
}

int CDib::FloodFill32Alpha(int x, int y, COLORREF crNewColor, COLORREF crOldColor)
{
	// Check
    if (crNewColor == crOldColor)
		return 0;

	// Top-Down to Bottom-Up Coordinates and check
	y = GetHeight() - 1 - y;
	if (x < 0 || x >= (int)GetWidth() || y < 0 || y >= (int)GetHeight())
		return 0;

	// Allocate Stack
	m_nFloodFillStackSize = FLOODFILL_INITSTACKSIZE;
	m_pFloodFillStack = new int[m_nFloodFillStackSize];
	if (!m_pFloodFillStack)
		return -1;
	m_nFloodFillStackPos = 0;
    
    int x1;
    BOOL spanTop, spanBottom;
	int nCount = 0;

	// Scan Line Alignment
	DWORD uiDIBScanLineSize = GetWidth();

	// Get Bits
	LPDWORD lpBits = (LPDWORD)GetBits();

	// RGBQUAD order which is also the order in memory is:	B,G,R,A
	// COLORREF order is:									R,G,B,A
	DWORD oldColor, newColor;
	RGBQUAD* p = (RGBQUAD*)&oldColor;
	p->rgbBlue = GetBValue(crOldColor);
	p->rgbGreen = GetGValue(crOldColor);
	p->rgbRed = GetRValue(crOldColor);
	p->rgbReserved = GetAValue(crOldColor);
	p = (RGBQUAD*)&newColor;
	p->rgbBlue = GetBValue(crNewColor);
	p->rgbGreen = GetGValue(crNewColor);
	p->rgbRed = GetRValue(crNewColor);
	p->rgbReserved = GetAValue(crNewColor);

    FloodFillPush(x, y);
    while (FloodFillPop(x, y))
    {    
        x1 = x;

        while (	x1 >= 0 &&
				lpBits[y*uiDIBScanLineSize + x1] == oldColor)
			x1--;
        x1++;

        spanTop = spanBottom = FALSE;
        while (	x1 < (int)GetWidth() &&
				lpBits[y*uiDIBScanLineSize + x1] == oldColor)
        {           
			lpBits[y*uiDIBScanLineSize + x1] = newColor;
			
			nCount++;

            if (!spanTop	&&
				y > 0		&&
				lpBits[(y-1)*uiDIBScanLineSize + x1] == oldColor)
            {
                FloodFillPush(x1, y - 1);
                spanTop = TRUE;
            }
            else if (spanTop	&&
					y > 0		&&
					lpBits[(y-1)*uiDIBScanLineSize + x1] != oldColor)
                spanTop = FALSE;
           
            if (!spanBottom					&&
				y < (int)GetHeight() - 1	&&
				lpBits[(y+1)*uiDIBScanLineSize + x1] == oldColor)
            {
                FloodFillPush(x1, y + 1);
                spanBottom = TRUE;
            }
            else if (spanBottom					&&
					y < (int)GetHeight() - 1	&&
					lpBits[(y+1)*uiDIBScanLineSize + x1] != oldColor)
                spanBottom = FALSE;

            x1++;                    
        }
    }

	// Clean-Up
	delete [] m_pFloodFillStack;
	m_pFloodFillStack = NULL;

	return nCount;
}

int CDib::FloodFillIndex(int x, int y, int nNewIndex, int nOldIndex)
{
	// Check
	if (GetBitCount() > 8)
		return -1;

	// Check
    if (nNewIndex == nOldIndex)
		return 0;

	// Top-Down to Bottom-Up Coordinates and check
	y = GetHeight() - 1 - y;
	if (x < 0 || x >= (int)GetWidth() || y < 0 || y >= (int)GetHeight())
		return 0;

	// Allocate Stack
	m_nFloodFillStackSize = FLOODFILL_INITSTACKSIZE;
	m_pFloodFillStack = new int[m_nFloodFillStackSize];
	if (!m_pFloodFillStack)
		return -1;
	m_nFloodFillStackPos = 0;
    
    int x1; 
    BOOL spanTop, spanBottom;
	int nCount = 0;

    FloodFillPush(x, y);
    while (FloodFillPop(x, y))
    {    
        x1 = x;

        while (x1 >= 0 && GetPixelIndex(x1, y) == nOldIndex)
			x1--;
        x1++;

        spanTop = spanBottom = FALSE;
        while (x1 < (int)GetWidth() && GetPixelIndex(x1, y) == nOldIndex)
        {           
            SetPixelIndex(x1, y, nNewIndex);

			nCount++;
			
            if (!spanTop && y > 0 && GetPixelIndex(x1, y - 1) == nOldIndex) 
            {
                FloodFillPush(x1, y - 1);
                spanTop = TRUE;
            }
            else if(spanTop && y > 0 && GetPixelIndex(x1, y - 1) != nOldIndex)
                spanTop = FALSE;
           
            if (!spanBottom && y < (int)GetHeight() - 1 && GetPixelIndex(x1, y + 1) == nOldIndex) 
            {
                FloodFillPush(x1, y + 1);
                spanBottom = TRUE;
            }
            else if (spanBottom && y < (int)GetHeight() - 1 && GetPixelIndex(x1, y + 1) != nOldIndex)
                spanBottom = FALSE;

            x1++;                    
        }
    }

	// Clean-Up
	delete [] m_pFloodFillStack;
	m_pFloodFillStack = NULL;

	return nCount;
}