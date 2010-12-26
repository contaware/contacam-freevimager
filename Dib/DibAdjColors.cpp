#include "stdafx.h"
#include "dib.h"

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
				m_pOrigBits = (LPBYTE)BIGALLOC(nWidthDWAligned * GetHeight() + SAFETY_BITALLOC_MARGIN);
			if (!m_pOrigBits)
				return FALSE;

			memcpy((void*)m_pOrigBits, (void*)m_pBits, nWidthDWAligned * GetHeight());
			
			break;

		case 24:
			nWidthDWAligned = DWALIGNEDWIDTHBYTES(GetWidth() * 24); // DWORD aligned (in bytes)
			if (!m_pOrigBits)
				m_pOrigBits = (LPBYTE)BIGALLOC(nWidthDWAligned * GetHeight() + SAFETY_BITALLOC_MARGIN);
			if (!m_pOrigBits)
				return FALSE;

			memcpy((void*)m_pOrigBits, (void*)m_pBits, nWidthDWAligned * GetHeight());
			
			break;

		case 32:
			nWidthDWAligned = 4*GetWidth();
			if (!m_pOrigBits)
				m_pOrigBits = (LPBYTE)BIGALLOC(nWidthDWAligned * GetHeight() + SAFETY_BITALLOC_MARGIN);
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
	m_wLightness = 0;
	m_wSaturation = 0;
	m_uwHue = 0;
	m_dGamma = 1.0;

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
	m_wLightness = 0;
	m_wSaturation = 0;
	m_uwHue = 0;
	m_dGamma = 1.0;
	
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
	m_wLightness = 0;
	m_wSaturation = 0;
	m_uwHue = 0;
	m_dGamma = 1.0;
	
	return TRUE;
}

// color:      The color to adjust
// saturation: -100 .. +100
// hue:           0 .. +360
// lightness:  -100 .. +100
__forceinline void CDib::AdjustColor(CColor &color, short lightness, short saturation, unsigned short hue)
{
	float fNewLightness = 0.0f;
	float fNewSaturation = 0.0f;
	float fNewHue = 0.0f;

	// Adjust Saturation:
	if (saturation != 0)
	{
		fNewSaturation = color.GetSaturation() * ((float)(saturation + 100) / 100.0f);
		if (fNewSaturation > 1.0)
			color.SetSaturation(1.0);
		else if (fNewSaturation < 0.0)
			color.SetSaturation(0.0);
		else
			color.SetSaturation(fNewSaturation);
	}

	// Adjust Lightness:
	if (lightness != 0)
	{
		fNewLightness = color.GetLuminance() + (float)lightness / 100.0f;
		if (fNewLightness > 1.0)
			color.SetLuminance(1.0);
		else if (fNewLightness < 0.0)
			color.SetLuminance(0.0);
		else
			color.SetLuminance(fNewLightness);
	}

	// Adjust Hue:
	if (hue > 0)
	{
		fNewHue = color.GetHue() + (float)hue;
		if (fNewHue > 360.0f)
			fNewHue = fNewHue - 360.0f;
		color.SetHue(fNewHue);
	}
}

BOOL CDib::AdjustImage(	short brightness,
						short contrast,
						short lightness,
						short saturation,
						unsigned short hue,
						double gamma,
						BOOL bFast,	// For Fast And Imprecise Brightness & Contrast Regulation
						BOOL bEnableUndo, // Enable Undo of Adjusted Pixels
						CWnd* pProgressWnd/*=NULL*/,
						BOOL bProgressSend/*=TRUE*/)
{
	// Reset Color Undo
	if (!bEnableUndo)
		ResetColorUndo();

	if ((brightness == m_wBrightness) &&
		(contrast == m_wContrast) &&
		(lightness == m_wLightness) &&
		(saturation == m_wSaturation) &&
		(hue == m_uwHue) &&
		(gamma == m_dGamma))
		return TRUE;

	if (!m_pBits)
	{
		if (!DibSectionToBits())
			return FALSE;
	}

	if (!m_pBits || !m_pBMI)
		return FALSE;

	if (IsCompressed())
#ifndef _WIN32_WCE
		if (!Decompress(GetBitCount())) // Decompress
			return FALSE;
#else
		return FALSE;
#endif

	// The first time this function is called make a copy of the image
	if (bEnableUndo)
	{
		if (!m_bColorUndoSet)
			SetColorUndo();
		else
			UndoColor(); // Restore the original image
	}

	WORD wNumColors = GetNumColors();
	CColor color;
	unsigned int line, i, nWidthDWAligned;

	if ((lightness != 0) || (saturation != 0) || (hue != 0))
	{
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

					color.SetRed(m_pColors[i].rgbRed);
					color.SetGreen(m_pColors[i].rgbGreen);
					color.SetBlue(m_pColors[i].rgbBlue);

					AdjustColor(color, lightness, saturation, hue);

					m_pColors[i].rgbRed = (unsigned char)color.GetRed();
					m_pColors[i].rgbGreen = (unsigned char)color.GetGreen();
					m_pColors[i].rgbBlue = (unsigned char)color.GetBlue();
				}
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
						color.SetBlue ((int)B);
						color.SetGreen((int)G);
						color.SetRed  ((int)R);

						AdjustColor(color, lightness, saturation, hue);

						((WORD*)m_pBits)[i] = RGBToDIB16(color.GetRed(), color.GetGreen(), color.GetBlue());
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
						color.SetBlue((int)m_pBits[i]);
						color.SetGreen((int)m_pBits[i+1]);
						color.SetRed((int)m_pBits[i+2]);

						AdjustColor(color, lightness, saturation, hue);

						m_pBits[i] = (BYTE)color.GetBlue();
						m_pBits[i+1] = (BYTE)color.GetGreen();
						m_pBits[i+2] = (BYTE)color.GetRed();
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
							color.SetBlue ((int)B);
							color.SetGreen((int)G);
							color.SetRed  ((int)R);

							AdjustColor(color, lightness, saturation, hue);

							((DWORD*)m_pBits)[i] = RGBAToDIB32(color.GetRed(), color.GetGreen(), color.GetBlue(), A);
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
							color.SetBlue ((int)B);
							color.SetGreen((int)G);
							color.SetRed  ((int)R);

							AdjustColor(color, lightness, saturation, hue);

							((DWORD*)m_pBits)[i] = RGBToDIB32(color.GetRed(), color.GetGreen(), color.GetBlue());
						}
					}
				}
				break;
			}
			default:
				break;
		}

		DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
	}
	m_wLightness = lightness;
	m_wSaturation = saturation;
	m_uwHue = hue;

	if (bFast)
		AdjustBrightnessContrastFast(brightness, contrast, pProgressWnd, bProgressSend);
	else
	{
		AdjustBrightness(brightness, pProgressWnd, bProgressSend);
		AdjustContrast(contrast, pProgressWnd, bProgressSend);
	}

	AdjustGamma(gamma, pProgressWnd, bProgressSend);

	CreatePaletteFromBMI();

	return TRUE;
}

// brightness: -100 .. +100
// contrast: -100 .. +100
void CDib::AdjustBrightnessContrastFast(short brightness,
										short contrast,
										CWnd* pProgressWnd/*=NULL*/,
										BOOL bProgressSend/*=TRUE*/)
{
	if (brightness == 0 && contrast == 0)
		return;

	if (!m_pBits)
	{
		if (!DibSectionToBits())
			return;
	}

	if (!m_pBits || !m_pBMI)
		return;

	if (IsCompressed())
#ifndef _WIN32_WCE
		if (!Decompress(GetBitCount())) // Decompress
			return;
#else
		return;
#endif

	// Create Brightness & Contrast Lookup Table
	float c = (100 + contrast) / 100.0f;

	BYTE BrightnessContrastLookUpTable[256];
	for (int i = 0 ; i < 256 ; i++)
		BrightnessContrastLookUpTable[i] = (BYTE)MAX( 0, MIN( 255, (int)((i-128)*c) + 128 + (int)(2.55*brightness) ) );

	DoLookUpTable(BrightnessContrastLookUpTable, pProgressWnd, bProgressSend);

	m_wBrightness = brightness;
	m_wContrast = contrast;
}

// brightness: -100 .. +100
void CDib::AdjustBrightness(short brightness,
							CWnd* pProgressWnd/*=NULL*/,
							BOOL bProgressSend/*=TRUE*/)
{
	float fNewBrightness = 0.0f;

	if (brightness == 0)
		return;

	if (!m_pBits)
	{
		if (!DibSectionToBits())
			return;
	}

	if (!m_pBits || !m_pBMI)
		return;

	if (IsCompressed())
#ifndef _WIN32_WCE
		if (!Decompress(GetBitCount())) // Decompress
			return;
#else
		return;
#endif

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

				// Use the YIQ Color Space. Y is the Brightness.
				int R, G, B;
				R = m_pColors[i].rgbRed;
				G = m_pColors[i].rgbGreen;
				B = m_pColors[i].rgbBlue;
				int Y = 30 * R / 100 + 59 * G / 100 + 11 * B / 100;
				int I = 60 * R / 100 - 28 * G / 100 - 32 * B / 100;
				int Q = 21 * R / 100 - 52 * G / 100 + 31 * B / 100;
				fNewBrightness = (float)brightness * 2.55f;
				Y = Y + (int)fNewBrightness;
				R = Y + 2401 * I / 2532 + 395 * Q / 633;
				G = Y - 233 * I / 844 - 135 * Q / 211;
				B = Y - 2799 * I / 2532 + 365 * Q / 211;
				if (R > 255) R = 255;
				else if (R < 0) R = 0;
				if (G > 255) G = 255;
				else if (G < 0) G = 0;
				if (B > 255) B = 255;
				else if (B < 0) B = 0;
				m_pColors[i].rgbRed = (unsigned char)R;
				m_pColors[i].rgbGreen = (unsigned char)G;
				m_pColors[i].rgbBlue = (unsigned char)B;
			}
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
					// Use the YIQ Color Space. Y is the Brightness.
					BYTE R, G, B;
					int r, g, b;
					DIB16ToRGB(((WORD*)m_pBits)[i], &R, &G, &B);
					int Y = 30 * R / 100 + 59 * G / 100 + 11 * B / 100;
					int I = 60 * R / 100 - 28 * G / 100 - 32 * B / 100;
					int Q = 21 * R / 100 - 52 * G / 100 + 31 * B / 100;
					fNewBrightness = (float)brightness * 2.55f;
					Y = Y + (int)fNewBrightness;
					r = Y + 2401 * I / 2532 + 395 * Q / 633;
					g = Y - 233 * I / 844 - 135 * Q / 211;
					b = Y - 2799 * I / 2532 + 365 * Q / 211;
					if (r > 255) R = 255;
					else if (r < 0) R = 0;
					else R = r;
					if (g > 255) G = 255;
					else if (g < 0) G = 0;
					else G = g;
					if (b > 255) B = 255;
					else if (b < 0) B = 0;
					else B = b;
					((WORD*)m_pBits)[i] = RGBToDIB16(R, G, B);
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
					// Use the YIQ Color Space. Y is the Brightness.
					int R, G, B;
					R = (int)m_pBits[i+2];
					G = (int)m_pBits[i+1];
					B = (int)m_pBits[i];
					int Y = 30 * R / 100 + 59 * G / 100 + 11 * B / 100;
					int I = 60 * R / 100 - 28 * G / 100 - 32 * B / 100;
					int Q = 21 * R / 100 - 52 * G / 100 + 31 * B / 100;
					fNewBrightness = (float)brightness * 2.55f;
					Y = Y + (int)fNewBrightness;
					R = Y + 2401 * I / 2532 + 395 * Q / 633;
					G = Y - 233 * I / 844 - 135 * Q / 211;
					B = Y - 2799 * I / 2532 + 365 * Q / 211;
					if (R > 255) R = 255;
					else if (R < 0) R = 0;
					if (G > 255) G = 255;
					else if (G < 0) G = 0;
					if (B > 255) B = 255;
					else if (B < 0) B = 0;
					m_pBits[i] = (BYTE)B;
					m_pBits[i+1] = (BYTE)G;
					m_pBits[i+2] = (BYTE)R;
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
						// Use the YIQ Color Space. Y is the Brightness.
						BYTE R, G, B, A;
						int r, g, b;
						DIB32ToRGBA(((DWORD*)m_pBits)[i], &R, &G, &B, &A);
						int Y = 30 * R / 100 + 59 * G / 100 + 11 * B / 100;
						int I = 60 * R / 100 - 28 * G / 100 - 32 * B / 100;
						int Q = 21 * R / 100 - 52 * G / 100 + 31 * B / 100;
						fNewBrightness = (float)brightness * 2.55f;
						Y = Y + (int)fNewBrightness;
						r = Y + 2401 * I / 2532 + 395 * Q / 633;
						g = Y - 233 * I / 844 - 135 * Q / 211;
						b = Y - 2799 * I / 2532 + 365 * Q / 211;
						if (r > 255) R = 255;
						else if (r < 0) R = 0;
						else R = r;
						if (g > 255) G = 255;
						else if (g < 0) G = 0;
						else G = g;
						if (b > 255) B = 255;
						else if (b < 0) B = 0;
						else B = b;
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
						// Use the YIQ Color Space. Y is the Brightness.
						BYTE R, G, B;
						int r, g, b;
						DIB32ToRGB(((DWORD*)m_pBits)[i], &R, &G, &B);
						int Y = 30 * R / 100 + 59 * G / 100 + 11 * B / 100;
						int I = 60 * R / 100 - 28 * G / 100 - 32 * B / 100;
						int Q = 21 * R / 100 - 52 * G / 100 + 31 * B / 100;
						fNewBrightness = (float)brightness * 2.55f;
						Y = Y + (int)fNewBrightness;
						r = Y + 2401 * I / 2532 + 395 * Q / 633;
						g = Y - 233 * I / 844 - 135 * Q / 211;
						b = Y - 2799 * I / 2532 + 365 * Q / 211;
						if (r > 255) R = 255;
						else if (r < 0) R = 0;
						else R = r;
						if (g > 255) G = 255;
						else if (g < 0) G = 0;
						else G = g;
						if (b > 255) B = 255;
						else if (b < 0) B = 0;
						else B = b;
						((DWORD*)m_pBits)[i] = RGBToDIB32(R, G, B);
					}
				}
			}
			break;
		}
		default:
			break;
	}
	m_wBrightness = brightness;
	DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
}

// contrast: -100 .. +100
void CDib::AdjustContrast(	short contrast,
							CWnd* pProgressWnd/*=NULL*/,
							BOOL bProgressSend/*=TRUE*/)
{
	float fNewContrast = 0.0f;

	if (contrast == 0)
		return;

	if (!m_pBits)
	{
		if (!DibSectionToBits())
			return;
	}

	if (!m_pBits || !m_pBMI)
		return;

	if (IsCompressed())
#ifndef _WIN32_WCE
		if (!Decompress(GetBitCount())) // Decompress
			return;
#else
		return;
#endif

	WORD wNumColors = GetNumColors();
	unsigned int line, i, nWidthDWAligned;
	int AverageImageBrightness = 0;
	int pixcount = 0;
	int linepixcount;

	// For Average Image Brightness Calculation
	int nHeightInc = GetHeight() / REASONABLE_SIZE_CONTRAST_AVGBRIGHT_CALC;
	if (nHeightInc == 0) nHeightInc++;
	int nWidthInc = GetWidth() / REASONABLE_SIZE_CONTRAST_AVGBRIGHT_CALC;
	if (nWidthInc == 0) nWidthInc++;

	DIB_INIT_PROGRESS;

	switch (m_pBMI->bmiHeader.biBitCount)
	{
		case 1:
		{
			nWidthDWAligned = DWALIGNEDWIDTHBYTES(GetWidth()); // DWORD aligned (in bytes)

			for (line = 0 ; line < GetHeight() ; line += nHeightInc)
			{
				linepixcount = 0;
				for (i = nWidthDWAligned * line ; i < (nWidthDWAligned * (line+1)) ; i += nWidthInc)
				{
					// First pixel is the most significant bit of the byte
					AverageImageBrightness += ((30 * (int)m_pColors[(m_pBits[i]>>7)&0x1].rgbRed +
												59 * (int)m_pColors[(m_pBits[i]>>7)&0x1].rgbGreen +
												11 * (int)m_pColors[(m_pBits[i]>>7)&0x1].rgbBlue) / 100);
					pixcount++;
					if (++linepixcount >= (int)GetWidth()) break;
					
					AverageImageBrightness += ((30 * (int)m_pColors[(m_pBits[i]>>6)&0x1].rgbRed +
												59 * (int)m_pColors[(m_pBits[i]>>6)&0x1].rgbGreen +
												11 * (int)m_pColors[(m_pBits[i]>>6)&0x1].rgbBlue) / 100);
					pixcount++;
					if (++linepixcount >= (int)GetWidth()) break;
					
					AverageImageBrightness += ((30 * (int)m_pColors[(m_pBits[i]>>5)&0x1].rgbRed +
												59 * (int)m_pColors[(m_pBits[i]>>5)&0x1].rgbGreen +
												11 * (int)m_pColors[(m_pBits[i]>>5)&0x1].rgbBlue) / 100);
					pixcount++;
					if (++linepixcount >= (int)GetWidth()) break;
					
					AverageImageBrightness += ((30 * (int)m_pColors[(m_pBits[i]>>4)&0x1].rgbRed +
												59 * (int)m_pColors[(m_pBits[i]>>4)&0x1].rgbGreen +
												11 * (int)m_pColors[(m_pBits[i]>>4)&0x1].rgbBlue) / 100);
					pixcount++;
					if (++linepixcount >= (int)GetWidth()) break;
					
					AverageImageBrightness += ((30 * (int)m_pColors[(m_pBits[i]>>3)&0x1].rgbRed +
												59 * (int)m_pColors[(m_pBits[i]>>3)&0x1].rgbGreen +
												11 * (int)m_pColors[(m_pBits[i]>>3)&0x1].rgbBlue) / 100);
					pixcount++;
					if (++linepixcount >= (int)GetWidth()) break;
					
					AverageImageBrightness += ((30 * (int)m_pColors[(m_pBits[i]>>2)&0x1].rgbRed +
												59 * (int)m_pColors[(m_pBits[i]>>2)&0x1].rgbGreen +
												11 * (int)m_pColors[(m_pBits[i]>>2)&0x1].rgbBlue) / 100);
					pixcount++;
					if (++linepixcount >= (int)GetWidth()) break;
					
					AverageImageBrightness += ((30 * (int)m_pColors[(m_pBits[i]>>1)&0x1].rgbRed +
												59 * (int)m_pColors[(m_pBits[i]>>1)&0x1].rgbGreen +
												11 * (int)m_pColors[(m_pBits[i]>>1)&0x1].rgbBlue) / 100);
					pixcount++;
					if (++linepixcount >= (int)GetWidth()) break;
					
					AverageImageBrightness += ((30 * (int)m_pColors[m_pBits[i]&0x1].rgbRed +
												59 * (int)m_pColors[m_pBits[i]&0x1].rgbGreen +
												11 * (int)m_pColors[m_pBits[i]&0x1].rgbBlue) / 100);
					pixcount++;
					if (++linepixcount >= (int)GetWidth()) break;
				}
			}
			AverageImageBrightness /= pixcount;

			for (i = 0; i < (int)wNumColors; i++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, i, wNumColors);

				// Use the YIQ Color Space. Y is the Brightness.
				int R, G, B;
				R = m_pColors[i].rgbRed;
				G = m_pColors[i].rgbGreen;
				B = m_pColors[i].rgbBlue;
				int Y = (30 * R + 59 * G + 11 * B) / 100;
				int I = (60 * R - 28 * G - 32 * B) / 100;
				int Q = (21 * R - 52 * G + 31 * B) / 100;
				if (Y != AverageImageBrightness)
				{
					fNewContrast = (((float)contrast) + 100.0f) / 100.0f;
					Y = (int)(fNewContrast * ((float)(Y - AverageImageBrightness))) + AverageImageBrightness;
					R = Y + 2401 * I / 2532 + 395 * Q / 633;
					G = Y - 233 * I / 844 - 135 * Q / 211;
					B = Y - 2799 * I / 2532 + 365 * Q / 211;
					if (R > 255) R = 255;
					else if (R < 0) R = 0;
					if (G > 255) G = 255;
					else if (G < 0) G = 0;
					if (B > 255) B = 255;
					else if (B < 0) B = 0;
					m_pColors[i].rgbRed = (unsigned char)R;
					m_pColors[i].rgbGreen = (unsigned char)G;
					m_pColors[i].rgbBlue = (unsigned char)B;
				}
			}
			break;
		}
		case 4:
		{
			nWidthDWAligned = DWALIGNEDWIDTHBYTES(GetWidth() * 4); // DWORD aligned (in bytes)

			for (line = 0 ; line < GetHeight() ; line += nHeightInc)
			{
				linepixcount = 0;
				for (i = nWidthDWAligned * line ; i < (nWidthDWAligned * (line+1)) ; i += nWidthInc)
				{
					// First pixel is the most significant nibble of the byte
					AverageImageBrightness += ((30 * (int)m_pColors[(m_pBits[i] & 0xF0)>>4].rgbRed +
												59 * (int)m_pColors[(m_pBits[i] & 0xF0)>>4].rgbGreen +
												11 * (int)m_pColors[(m_pBits[i] & 0xF0)>>4].rgbBlue) / 100);
					pixcount++;
					if (++linepixcount >= (int)GetWidth()) break;
					
					AverageImageBrightness += ((30 * (int)m_pColors[m_pBits[i] & 0x0F].rgbRed +
												59 * (int)m_pColors[m_pBits[i] & 0x0F].rgbGreen +
												11 * (int)m_pColors[m_pBits[i] & 0x0F].rgbBlue) / 100);
					pixcount++;
					if (++linepixcount >= (int)GetWidth()) break;
				}
			}
			AverageImageBrightness /= pixcount;

			for (i = 0; i < (int)wNumColors; i++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, i, wNumColors);

				// Use the YIQ Color Space. Y is the Brightness.
				int R, G, B;
				R = m_pColors[i].rgbRed;
				G = m_pColors[i].rgbGreen;
				B = m_pColors[i].rgbBlue;
				int Y = (30 * R + 59 * G + 11 * B) / 100;
				int I = (60 * R - 28 * G - 32 * B) / 100;
				int Q = (21 * R - 52 * G + 31 * B) / 100;
				if (Y != AverageImageBrightness)
				{
					fNewContrast = (((float)contrast) + 100.0f) / 100.0f;
					Y = (int)(fNewContrast * ((float)(Y - AverageImageBrightness))) + AverageImageBrightness;
					R = Y + 2401 * I / 2532 + 395 * Q / 633;
					G = Y - 233 * I / 844 - 135 * Q / 211;
					B = Y - 2799 * I / 2532 + 365 * Q / 211;
					if (R > 255) R = 255;
					else if (R < 0) R = 0;
					if (G > 255) G = 255;
					else if (G < 0) G = 0;
					if (B > 255) B = 255;
					else if (B < 0) B = 0;
					m_pColors[i].rgbRed = (unsigned char)R;
					m_pColors[i].rgbGreen = (unsigned char)G;
					m_pColors[i].rgbBlue = (unsigned char)B;
				}
			}
			break;
		}
		case 8:
		{
			nWidthDWAligned = DWALIGNEDWIDTHBYTES(GetWidth() * 8); // DWORD aligned (in bytes)

			for (line = 0 ; line < GetHeight() ; line += nHeightInc)
			{
				linepixcount = 0;
				for (i = nWidthDWAligned * line ; i < (nWidthDWAligned * (line+1)) ; i += nWidthInc)
				{
					AverageImageBrightness += ((30 * (int)m_pColors[m_pBits[i]].rgbRed +
												59 * (int)m_pColors[m_pBits[i]].rgbGreen +
												11 * (int)m_pColors[m_pBits[i]].rgbBlue) / 100);
					pixcount++;
					if (++linepixcount >= (int)GetWidth()) break;
				}
			}
			AverageImageBrightness /= pixcount;

			for (i = 0; i < (int)wNumColors; i++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, i, wNumColors);

				// Use the YIQ Color Space. Y is the Brightness.
				int R, G, B;
				R = m_pColors[i].rgbRed;
				G = m_pColors[i].rgbGreen;
				B = m_pColors[i].rgbBlue;
				int Y = (30 * R + 59 * G + 11 * B) / 100;
				int I = (60 * R - 28 * G - 32 * B) / 100;
				int Q = (21 * R - 52 * G + 31 * B) / 100;
				if (Y != AverageImageBrightness)
				{
					fNewContrast = (((float)contrast) + 100.0f) / 100.0f;
					Y = (int)(fNewContrast * ((float)(Y - AverageImageBrightness))) + AverageImageBrightness;
					R = Y + 2401 * I / 2532 + 395 * Q / 633;
					G = Y - 233 * I / 844 - 135 * Q / 211;
					B = Y - 2799 * I / 2532 + 365 * Q / 211;
					if (R > 255) R = 255;
					else if (R < 0) R = 0;
					if (G > 255) G = 255;
					else if (G < 0) G = 0;
					if (B > 255) B = 255;
					else if (B < 0) B = 0;
					m_pColors[i].rgbRed = (unsigned char)R;
					m_pColors[i].rgbGreen = (unsigned char)G;
					m_pColors[i].rgbBlue = (unsigned char)B;
				}
			}
			break;
		}
		case 16:
		{
			nWidthDWAligned = DWALIGNEDWIDTHBYTES(GetWidth() * 16); // DWORD aligned (in bytes)

			for (line = 0 ; line < GetHeight() ; line += nHeightInc)
			{
				linepixcount = 0;
				for (i = (nWidthDWAligned/2) * line ; i < ((nWidthDWAligned/2) * (line+1)) ; i += nWidthInc)
				{
					BYTE R, G, B;
					DIB16ToRGB(((WORD*)m_pBits)[i], &R, &G, &B);
					AverageImageBrightness += ((30 * (int)R + 59 * (int)G + 11 * (int)B) / 100);
					pixcount++;
					if (++linepixcount >= (int)GetWidth()) break;
				}
			}
			AverageImageBrightness /= pixcount;

			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = (nWidthDWAligned/2) * line ; i < ((nWidthDWAligned/2) * (line+1)) ; i++)
				{
					// Use the YIQ Color Space. Y is the Brightness.
					int R, G, B;
					DIB16ToRGB(((WORD*)m_pBits)[i], &R, &G, &B);
					int Y = (30 * R + 59 * G + 11 * B) / 100;
					int I = (60 * R - 28 * G - 32 * B) / 100;
					int Q = (21 * R - 52 * G + 31 * B) / 100;
					if (Y != AverageImageBrightness)
					{
						fNewContrast = (((float)contrast) + 100.0f) / 100.0f;
						Y = (int)(fNewContrast * ((float)(Y - AverageImageBrightness))) + AverageImageBrightness;
						R = Y + 2401 * I / 2532 + 395 * Q / 633;
						G = Y - 233 * I / 844 - 135 * Q / 211;
						B = Y - 2799 * I / 2532 + 365 * Q / 211;
						if (R > 255) R = 255;
						else if (R < 0) R = 0;
						if (G > 255) G = 255;
						else if (G < 0) G = 0;
						if (B > 255) B = 255;
						else if (B < 0) B = 0;
						((WORD*)m_pBits)[i] = RGBToDIB16(R, G, B);
					}
				}
			}
			break;
		}
		case 24:
		{
			nWidthDWAligned = DWALIGNEDWIDTHBYTES(GetWidth() * 24); // DWORD aligned (in bytes)

			for (line = 0 ; line < GetHeight(); line += nHeightInc)
			{
				linepixcount = 0;
				for (i = nWidthDWAligned * line ; (i+2) < (nWidthDWAligned * (line+1)) ; i += 3*nWidthInc)
				{
					AverageImageBrightness += ((30 * (int)m_pBits[i+2] +
												59 * (int)m_pBits[i+1] +
												11 * (int)m_pBits[i]) / 100);
					pixcount++;
					if (++linepixcount >= (int)GetWidth()) break;
				}
			}
			AverageImageBrightness /= pixcount;

			for (line = 0 ; line < GetHeight(); line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = nWidthDWAligned * line ; (i+2) < (nWidthDWAligned * (line+1)) ; i = i+3)
				{
					// Use the YIQ Color Space. Y is the Brightness.
					int R, G, B;
					R = (int)m_pBits[i+2];
					G = (int)m_pBits[i+1];
					B = (int)m_pBits[i];
					int Y = (30 * R + 59 * G + 11 * B) / 100;
					int I = (60 * R - 28 * G - 32 * B) / 100;
					int Q = (21 * R - 52 * G + 31 * B) / 100;
					if (Y != AverageImageBrightness)
					{
						fNewContrast = (((float)contrast) + 100.0f) / 100.0f; // min factor is 1.00f
						Y = (int)(fNewContrast * ((float)(Y - AverageImageBrightness))) + AverageImageBrightness;
						R = Y + 2401 * I / 2532 + 395 * Q / 633;
						G = Y - 233 * I / 844 - 135 * Q / 211;
						B = Y - 2799 * I / 2532 + 365 * Q / 211;
						if (R > 255) R = 255;
						else if (R < 0) R = 0;
						if (G > 255) G = 255;
						else if (G < 0) G = 0;
						if (B > 255) B = 255;
						else if (B < 0) B = 0;
						m_pBits[i] = (BYTE)B;
						m_pBits[i+1] = (BYTE)G;
						m_pBits[i+2] = (BYTE)R;
					}
				}
			}
			break;
		}
		case 32:
		{
			for (line = 0 ; line < GetHeight() ; line += nHeightInc)
			{
				for (i = GetWidth() * line ; i < (GetWidth() * (line+1)) ; i += nWidthInc)
				{
					BYTE R, G, B;
					DIB32ToRGB(((DWORD*)m_pBits)[i], &R, &G, &B);
					AverageImageBrightness += ((30 * (int)R + 59 * (int)G + 11 * (int)B) / 100);
					pixcount++;
				}
			}
			AverageImageBrightness /= pixcount;

			if (HasAlpha())
			{
				for (line = 0 ; line < GetHeight() ; line++)
				{
					DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

					for (i = GetWidth() * line ; i < (GetWidth() * (line+1)) ; i++)
					{
						// Use the YIQ Color Space. Y is the Brightness.
						int R, G, B, A;
						DIB32ToRGBA(((DWORD*)m_pBits)[i], &R, &G, &B, &A);
						int Y = (30 * R + 59 * G + 11 * B) / 100;
						int I = (60 * R - 28 * G - 32 * B) / 100;
						int Q = (21 * R - 52 * G + 31 * B) / 100;
						if (Y != AverageImageBrightness)
						{
							fNewContrast = (((float)contrast) + 100.0f) / 100.0f;
							Y = (int)(fNewContrast * ((float)(Y - AverageImageBrightness))) + AverageImageBrightness;
							R = Y + 2401 * I / 2532 + 395 * Q / 633;
							G = Y - 233 * I / 844 - 135 * Q / 211;
							B = Y - 2799 * I / 2532 + 365 * Q / 211;
							if (R > 255) R = 255;
							else if (R < 0) R = 0;
							if (G > 255) G = 255;
							else if (G < 0) G = 0;
							if (B > 255) B = 255;
							else if (B < 0) B = 0;
							((DWORD*)m_pBits)[i] = RGBAToDIB32(R, G, B, A);
						}
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
						// Use the YIQ Color Space. Y is the Brightness.
						int R, G, B;
						DIB32ToRGB(((DWORD*)m_pBits)[i], &R, &G, &B);
						int Y = (30 * R + 59 * G + 11 * B) / 100;
						int I = (60 * R - 28 * G - 32 * B) / 100;
						int Q = (21 * R - 52 * G + 31 * B) / 100;
						if (Y != AverageImageBrightness)
						{
							fNewContrast = (((float)contrast) + 100.0f) / 100.0f;
							Y = (int)(fNewContrast * ((float)(Y - AverageImageBrightness))) + AverageImageBrightness;
							R = Y + 2401 * I / 2532 + 395 * Q / 633;
							G = Y - 233 * I / 844 - 135 * Q / 211;
							B = Y - 2799 * I / 2532 + 365 * Q / 211;
							if (R > 255) R = 255;
							else if (R < 0) R = 0;
							if (G > 255) G = 255;
							else if (G < 0) G = 0;
							if (B > 255) B = 255;
							else if (B < 0) B = 0;
							((DWORD*)m_pBits)[i] = RGBToDIB32(R, G, B);
						}
					}
				}
			}
			break;
		}
		default:
			break;
	}
	m_wContrast = contrast;
	DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
}

void CDib::AdjustGamma(	double gamma,
						CWnd* pProgressWnd/*=NULL*/,
						BOOL bProgressSend/*=TRUE*/)
{
	if (gamma == 1.0)
		return;

	if (!m_pBits)
	{
		if (!DibSectionToBits())
			return;
	}

	if (!m_pBits || !m_pBMI)
		return;

	if (IsCompressed())
#ifndef _WIN32_WCE
		if (!Decompress(GetBitCount())) // Decompress
			return;
#else
		return;
#endif

	// Create Gamma Lookup Table
	double dInvGamma = 1.0 / gamma;
	double dMax = pow(255.0, dInvGamma) / 255.0;
	BYTE GammaLookUpTable[256];
	for (int i = 0 ; i < 256 ; i++)
		GammaLookUpTable[i] = (BYTE)MAX(0, MIN(255, (int)(pow((double)i, dInvGamma) / dMax)));

	DoLookUpTable(GammaLookUpTable, pProgressWnd, bProgressSend);

	m_dGamma = gamma;
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
#ifndef _WIN32_WCE
		if (!Decompress(GetBitCount())) // Decompress
			return FALSE;
#else
		return FALSE;
#endif

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
#ifndef _WIN32_WCE
		if (!Decompress(GetBitCount())) // Decompress
			return FALSE;
#else
		return FALSE;
#endif

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
#ifndef _WIN32_WCE
		if (!Decompress(GetBitCount())) // Decompress
			return FALSE;
#else
		return FALSE;
#endif

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
#ifndef _WIN32_WCE
		if (!Decompress(GetBitCount())) // Decompress
			return FALSE;
#else
		return FALSE;
#endif

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
#ifndef _WIN32_WCE
		if (!Decompress(GetBitCount())) // Decompress
			return FALSE;
#else
		return FALSE;
#endif

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

int CDib::FloodFill(int x, int y, COLORREF crNewColor, COLORREF crOldColor)
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