#include "stdafx.h"
#include "dib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BOOL CDib::RenderAlphaWithSrcBackground(CDib* pSrcDib/*=NULL*/)
{
	int r, g, b, a;

	// Make a Copy of this?
	CDib SrcDib;
	BOOL bCopySrcToDst = FALSE;
	if (pSrcDib == NULL || this == pSrcDib)
	{
		// Alpha Check
		if (!m_bAlpha || GetBitCount() != 32)
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
		// Alpha Check
		if (!pSrcDib->m_bAlpha || pSrcDib->GetBitCount() != 32)
			return FALSE;

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
		}
		// Need to ReAllocate BMI because they are of differente size
		else if (pSrcDib->GetBMISize() != GetBMISize())
		{
			delete [] m_pBMI;

			// Allocate & Copy BMI
			m_pBMI = (LPBITMAPINFO)new BYTE[pSrcDib->GetBMISize()];
			if (m_pBMI == NULL)
				return FALSE;
		}
		memcpy((void*)m_pBMI, (void*)pSrcDib->m_pBMI, pSrcDib->GetBMISize());

		// Make Sure m_pColors Points to the Right Place
		m_pColors = NULL;

		// Copy Src To Dst
		bCopySrcToDst = TRUE;
	}

	// Scan Line Alignment
	DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES(pSrcDib->GetWidth() * pSrcDib->GetBitCount());

	// Allocate Bits
	if (m_pBits == NULL)
	{
		// Allocate memory
		m_pBits = (LPBYTE)BIGALLOC(uiDIBScanLineSize * pSrcDib->GetHeight() + SAFETY_BITALLOC_MARGIN);
	}
	// Need to ReAllocate Bits because they are of differente size
	else if (m_dwImageSize != uiDIBScanLineSize * pSrcDib->GetHeight())
	{
		BIGFREE(m_pBits);

		// Allocate memory
		m_pBits = (LPBYTE)BIGALLOC(uiDIBScanLineSize * pSrcDib->GetHeight() + SAFETY_BITALLOC_MARGIN);
	}
	if (m_pBits == NULL)
		return FALSE;

	// Copy Vars
	if (bCopySrcToDst)
		CopyVars(*pSrcDib);

	// Image Size
	m_dwImageSize = m_pBMI->bmiHeader.biSizeImage = uiDIBScanLineSize * GetHeight();

	// Init Mask
	InitMasks();

	// Free
	ResetColorUndo();
	if (m_hDibSection)
	{
		::DeleteObject(m_hDibSection);
		m_hDibSection = NULL;
		m_pDibSectionBits = NULL;
	}
	DeletePreviewDib();
	DeleteThumbnailDib();

	// Render
	int rbck = GetRValue(pSrcDib->m_crBackgroundColor);
	int gbck = GetGValue(pSrcDib->m_crBackgroundColor);
	int bbck = GetBValue(pSrcDib->m_crBackgroundColor);
	for (int i = 0 ; i < (int)GetImageSize() / 4; i++)
	{
		int pix = ((DWORD*)(pSrcDib->GetBits()))[i];
		b = pix         & 0xFF;
		g = (pix >> 8)  & 0xFF;
		r = (pix >> 16) & 0xFF;
		a = (pix >> 24) & 0xFF;
		b = (b * a + bbck * (255 - a)) / 255;
		g = (g * a + gbck * (255 - a)) / 255;
		r = (r * a + rbck * (255 - a)) / 255;
		pix &= ~0x00FFFFFF; // Keep Alpha
		pix |= (r << 16);
		pix |= (g << 8);
		pix |= b;
		((DWORD*)m_pBits)[i] = pix;
	}
		
	return TRUE;
}

BOOL CDib::GrayToAlphaChannel(COLORREF crColor, CDib* pSrcDib/*=NULL*/, CWnd* pProgressWnd/*=NULL*/, BOOL bProgressSend/*=TRUE*/)
{
	int r, g, b, gray;

	// Make a Copy of this?
	CDib SrcDib;
	BOOL bCopySrcToDst = FALSE;
	if (pSrcDib == NULL || this == pSrcDib)
	{
		// Alpha Check
		if (GetBitCount() != 32)
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
		// Alpha Check
		if (pSrcDib->GetBitCount() != 32)
			return FALSE;

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
		}
		// Need to ReAllocate BMI because they are of differente size
		else if (pSrcDib->GetBMISize() != GetBMISize())
		{
			delete [] m_pBMI;

			// Allocate & Copy BMI
			m_pBMI = (LPBITMAPINFO)new BYTE[pSrcDib->GetBMISize()];
			if (m_pBMI == NULL)
				return FALSE;
		}
		memcpy((void*)m_pBMI, (void*)pSrcDib->m_pBMI, pSrcDib->GetBMISize());

		// Make Sure m_pColors Points to the Right Place
		m_pColors = NULL;

		// Copy Src To Dst
		bCopySrcToDst = TRUE;
	}

	// Scan Line Alignment
	DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES(pSrcDib->GetWidth() * pSrcDib->GetBitCount());

	// Allocate Bits
	if (m_pBits == NULL)
	{
		// Allocate memory
		m_pBits = (LPBYTE)BIGALLOC(uiDIBScanLineSize * pSrcDib->GetHeight() + SAFETY_BITALLOC_MARGIN);
	}
	// Need to ReAllocate Bits because they are of differente size
	else if (m_dwImageSize != uiDIBScanLineSize * pSrcDib->GetHeight())
	{
		BIGFREE(m_pBits);

		// Allocate memory
		m_pBits = (LPBYTE)BIGALLOC(uiDIBScanLineSize * pSrcDib->GetHeight() + SAFETY_BITALLOC_MARGIN);
	}
	if (m_pBits == NULL)
		return FALSE;

	// Copy Vars
	if (bCopySrcToDst)
		CopyVars(*pSrcDib);

	// Image Size
	m_dwImageSize = m_pBMI->bmiHeader.biSizeImage = uiDIBScanLineSize * GetHeight();

	// Init Mask
	InitMasks();

	// Free
	ResetColorUndo();
	if (m_hDibSection)
	{
		::DeleteObject(m_hDibSection);
		m_hDibSection = NULL;
		m_pDibSectionBits = NULL;
	}
	DeletePreviewDib();
	DeleteThumbnailDib();

	DIB_INIT_PROGRESS;

	// Gray value to alpha
	int nUpperBound = GetImageSize() / 4;
	r = GetRValue(crColor);
	g = GetGValue(crColor);
	b = GetBValue(crColor);
	int imagepix = b | (g << 8) | (r << 16);
	for (int i = 0 ; i < nUpperBound ; i++)
	{
		if ((i & 0xFFFF) == 0)
			DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, i, nUpperBound);

		int pix = ((DWORD*)(pSrcDib->GetBits()))[i];
		b = pix         & 0xFF;
		g = (pix >> 8)  & 0xFF;
		r = (pix >> 16) & 0xFF;
		gray = (((b)*117 + (g)*601 + (r)*306) >> 10) & 0xFF;
		pix = (gray << 24) | imagepix; // set alpha
		((DWORD*)m_pBits)[i] = pix;
	}

	// Set alpha
	m_bAlpha = TRUE;
		
	DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

	return TRUE;
}

BOOL CDib::AlphaOffset(int nAlphaOffset, CDib* pSrcDib/*=NULL*/, CWnd* pProgressWnd/*=NULL*/, BOOL bProgressSend/*=TRUE*/)
{
	// Make a Copy of this?
	CDib SrcDib;
	BOOL bCopySrcToDst = FALSE;
	if (pSrcDib == NULL || this == pSrcDib)
	{
		// Alpha Check
		if (!m_bAlpha || GetBitCount() != 32)
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
		// Alpha Check
		if (!pSrcDib->m_bAlpha || pSrcDib->GetBitCount() != 32)
			return FALSE;

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
		}
		// Need to ReAllocate BMI because they are of differente size
		else if (pSrcDib->GetBMISize() != GetBMISize())
		{
			delete [] m_pBMI;

			// Allocate & Copy BMI
			m_pBMI = (LPBITMAPINFO)new BYTE[pSrcDib->GetBMISize()];
			if (m_pBMI == NULL)
				return FALSE;
		}
		memcpy((void*)m_pBMI, (void*)pSrcDib->m_pBMI, pSrcDib->GetBMISize());

		// Make Sure m_pColors Points to the Right Place
		m_pColors = NULL;

		// Copy Src To Dst
		bCopySrcToDst = TRUE;
	}

	// Scan Line Alignment
	DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES(pSrcDib->GetWidth() * pSrcDib->GetBitCount());

	// Allocate Bits
	if (m_pBits == NULL)
	{
		// Allocate memory
		m_pBits = (LPBYTE)BIGALLOC(uiDIBScanLineSize * pSrcDib->GetHeight() + SAFETY_BITALLOC_MARGIN);
	}
	// Need to ReAllocate Bits because they are of differente size
	else if (m_dwImageSize != uiDIBScanLineSize * pSrcDib->GetHeight())
	{
		BIGFREE(m_pBits);

		// Allocate memory
		m_pBits = (LPBYTE)BIGALLOC(uiDIBScanLineSize * pSrcDib->GetHeight() + SAFETY_BITALLOC_MARGIN);
	}
	if (m_pBits == NULL)
		return FALSE;

	// Copy Vars
	if (bCopySrcToDst)
		CopyVars(*pSrcDib);

	// Image Size
	m_dwImageSize = m_pBMI->bmiHeader.biSizeImage = uiDIBScanLineSize * GetHeight();

	// Init Mask
	InitMasks();

	// Free
	ResetColorUndo();
	if (m_hDibSection)
	{
		::DeleteObject(m_hDibSection);
		m_hDibSection = NULL;
		m_pDibSectionBits = NULL;
	}
	DeletePreviewDib();
	DeleteThumbnailDib();

	DIB_INIT_PROGRESS;

	// Set new alpha
	int nUpperBound = GetImageSize() / 4;
	for (int i = 0 ; i < nUpperBound ; i++)
	{
		if ((i & 0xFFFF) == 0)
			DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, i, nUpperBound);

		int pix = ((DWORD*)(pSrcDib->GetBits()))[i];
		int a = (pix >> 24) & 0xFF;
		a += nAlphaOffset;
		a = MAX(0, MIN(255, a));
		pix &= 0x00FFFFFF;
		a <<= 24;
		pix |= a;
		((DWORD*)m_pBits)[i] = pix;
	}
		
	DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

	return TRUE;
}

BOOL CDib::MixRGB(CDib* pDib)
{
	WORD wNumColors = GetNumColors();
	unsigned int line, i;

	if (!pDib || !pDib->IsValid())
		return FALSE;

	if (!m_pBits)
	{
		if (!DibSectionToBits())
			return FALSE;
	}

	if (!pDib->m_pBits)
	{
		if (!pDib->DibSectionToBits())
			return FALSE;
	}

	if (!m_pBits || !m_pBMI || !pDib->m_pBits || !pDib->m_pBMI)
		return FALSE;

	if (IsCompressed())
	{
		if (!Decompress(GetBitCount())) // Decompress
			return FALSE;
	}

	if (pDib->IsCompressed())
	{
		if (!pDib->Decompress(pDib->GetBitCount())) // Decompress
			return FALSE;
	}

	if ((GetBitCount() != pDib->GetBitCount()) ||
		(GetCompression() != pDib->GetCompression()) ||
		(GetWidth() != pDib->GetWidth()) ||
		(GetHeight() != pDib->GetHeight()))
		return FALSE;

	int nWidthDWAligned = DWALIGNEDWIDTHBYTES(GetWidth() * GetBitCount()); // DWORD aligned (in bytes)

	switch (GetBitCount())
	{
		case 16:
		{
			for (line = 0 ; line < GetHeight() ; line++)
			{
				for (i = (nWidthDWAligned/2) * line ; i < ((nWidthDWAligned/2) * (line+1)) ; i++)
				{
					BYTE R, G, B;
					BYTE R1, G1, B1;
					DIB16ToRGB(((WORD*)m_pBits)[i], &R, &G, &B);
					pDib->DIB16ToRGB(((WORD*)(pDib->m_pBits))[i], &R1, &G1, &B1);
					((WORD*)m_pBits)[i] = RGBToDIB16(	(BYTE)MIN(255, ((int)R + (int)R1) >> 1),
														(BYTE)MIN(255, ((int)G + (int)G1) >> 1),
														(BYTE)MIN(255, ((int)B + (int)B1) >> 1));
				}
			}
			break;
		}
		case 24:
		{
			for (i = 0 ; i < GetImageSize() ; i++)
				m_pBits[i] = (BYTE)MIN(255, ((int)m_pBits[i] + (int)pDib->m_pBits[i]) >> 1);
			break;
		}
		case 32:
		{
			for (line = 0 ; line < GetHeight() ; line++)
			{
				for (i = GetWidth() * line ; i < (GetWidth() * (line+1)) ; i++)
				{
					BYTE R, G, B;
					BYTE R1, G1, B1;
					DIB32ToRGB(((DWORD*)m_pBits)[i], &R, &G, &B);
					pDib->DIB32ToRGB(((DWORD*)(pDib->m_pBits))[i], &R1, &G1, &B1);
					((DWORD*)m_pBits)[i] = RGBToDIB32(	(BYTE)MIN(255, ((int)R + (int)R1) >> 1),
														(BYTE)MIN(255, ((int)G + (int)G1) >> 1),
														(BYTE)MIN(255, ((int)B + (int)B1) >> 1));
				}
			}
			break;
		}
		default:
			return FALSE;
	}

	return TRUE;
}

BOOL CDib::DiffRGB(CDib* pDib, int nMinDiff)
{
	WORD wNumColors = GetNumColors();
	unsigned int line, i;

	if (!pDib || !pDib->IsValid())
		return FALSE;

	if (!m_pBits)
	{
		if (!DibSectionToBits())
			return FALSE;
	}

	if (!pDib->m_pBits)
	{
		if (!pDib->DibSectionToBits())
			return FALSE;
	}

	if (!m_pBits || !m_pBMI || !pDib->m_pBits || !pDib->m_pBMI)
		return FALSE;

	if (IsCompressed())
	{
		if (!Decompress(GetBitCount())) // Decompress
			return FALSE;
	}

	if (pDib->IsCompressed())
	{
		if (!pDib->Decompress(pDib->GetBitCount())) // Decompress
			return FALSE;
	}

	if ((GetBitCount() != pDib->GetBitCount()) ||
		(GetCompression() != pDib->GetCompression()) ||
		(GetWidth() != pDib->GetWidth()) ||
		(GetHeight() != pDib->GetHeight()))
		return FALSE;

	int nWidthDWAligned = DWALIGNEDWIDTHBYTES(GetWidth() * GetBitCount()); // DWORD aligned (in bytes)

	switch (GetBitCount())
	{
		case 16:
		{
			for (line = 0 ; line < GetHeight() ; line++)
			{
				for (i = (nWidthDWAligned/2) * line ; i < ((nWidthDWAligned/2) * (line+1)) ; i++)
				{
					BYTE R, G, B;
					BYTE R1, G1, B1;
					DIB16ToRGB(((WORD*)m_pBits)[i], &R, &G, &B);
					pDib->DIB16ToRGB(((WORD*)(pDib->m_pBits))[i], &R1, &G1, &B1);
					R = (BYTE)ABS((int)R - (int)R1);
					G = (BYTE)ABS((int)G - (int)G1);
					B = (BYTE)ABS((int)B - (int)B1);
					if (RGBTOGRAY(R, G, B) < nMinDiff)
						((WORD*)m_pBits)[i] = 0;
					else
						((WORD*)m_pBits)[i] = RGBToDIB16(R, G, B);
				}
			}
			break;
		}
		case 24:
		{
			for (line = 0 ; line < GetHeight() ; line++)
			{
				for (i = (nWidthDWAligned) * line ; i < ((nWidthDWAligned) * (line+1)) ; i += 3)
				{
					BYTE R, G, B;
					B = (BYTE)ABS((int)m_pBits[i] - (int)pDib->m_pBits[i]);
					G = (BYTE)ABS((int)m_pBits[i+1] - (int)pDib->m_pBits[i+1]);
					R = (BYTE)ABS((int)m_pBits[i+2] - (int)pDib->m_pBits[i+2]);
					if (RGBTOGRAY(R, G, B) < nMinDiff)
					{
						m_pBits[i] = 0;
						m_pBits[i+1] = 0;
						m_pBits[i+2] = 0;
					}
					else
					{
						m_pBits[i] = B;
						m_pBits[i+1] = G;
						m_pBits[i+2] = R;
					}
				}
			}
			break;
		}
		case 32:
		{
			for (line = 0 ; line < GetHeight() ; line++)
			{
				for (i = GetWidth() * line ; i < (GetWidth() * (line+1)) ; i++)
				{
					BYTE R, G, B;
					BYTE R1, G1, B1;
					DIB32ToRGB(((DWORD*)m_pBits)[i], &R, &G, &B);
					pDib->DIB32ToRGB(((DWORD*)(pDib->m_pBits))[i], &R1, &G1, &B1);
					R = (BYTE)ABS((int)R - (int)R1);
					G = (BYTE)ABS((int)G - (int)G1);
					B = (BYTE)ABS((int)B - (int)B1);
					if (RGBTOGRAY(R, G, B) < nMinDiff)
						((DWORD*)m_pBits)[i] = 0;
					else
						((DWORD*)m_pBits)[i] = RGBToDIB32(R, G, B);
				}
			}
			break;
		}
		default:
			return FALSE;
	}

	return TRUE;
}

BOOL CDib::DiffTransp8(CDib* pDib, int nTransparentIndex)
{
	unsigned int line, i, index, index1;

	if (nTransparentIndex < 0 || nTransparentIndex > 255)
		return FALSE;

	if (!pDib || !pDib->IsValid())
		return FALSE;

	if (!m_pBits)
	{
		if (!DibSectionToBits())
			return FALSE;
	}

	if (!pDib->m_pBits)
	{
		if (!pDib->DibSectionToBits())
			return FALSE;
	}

	if (!m_pBits || !m_pBMI || !pDib->m_pBits || !pDib->m_pBMI)
		return FALSE;

	if (IsCompressed())
	{
		if (!Decompress(8)) // Decompress
			return FALSE;
	}

	if (pDib->IsCompressed())
	{
		if (!pDib->Decompress(pDib->GetBitCount())) // Decompress
			return FALSE;
	}

	if ((GetBitCount() != 8)							||
		(pDib->GetBitCount() != 8)						||
		(GetCompression() != BI_RGB)					||
		(pDib->GetCompression() != BI_RGB)				||
		(GetWidth() != pDib->GetWidth())				||
		(GetHeight() != pDib->GetHeight()))
		return FALSE;

	int nWidthDWAligned = DWALIGNEDWIDTHBYTES(GetWidth() * GetBitCount()); // DWORD aligned (in bytes)

	for (line = 0 ; line < GetHeight() ; line++)
	{
		for (i = (nWidthDWAligned) * line ; i < ((nWidthDWAligned) * (line+1)) ; i++)
		{
			index = m_pBits[i];
			index1 = pDib->m_pBits[i];
			if ((m_pColors[index].rgbRed == pDib->m_pColors[index1].rgbRed)		&&
				(m_pColors[index].rgbGreen == pDib->m_pColors[index1].rgbGreen)	&&
				(m_pColors[index].rgbBlue == pDib->m_pColors[index1].rgbBlue))
				m_pBits[i] = nTransparentIndex;
		}
	}

	return TRUE;
}

BOOL CDib::DiffTransp8(CDib* pDib, int nMinDiff, int nTransparentIndex)
{
	unsigned int line, i, index, index1;

	if (nTransparentIndex < 0 || nTransparentIndex > 255)
		return FALSE;

	if (!pDib || !pDib->IsValid())
		return FALSE;

	if (!m_pBits)
	{
		if (!DibSectionToBits())
			return FALSE;
	}

	if (!pDib->m_pBits)
	{
		if (!pDib->DibSectionToBits())
			return FALSE;
	}

	if (!m_pBits || !m_pBMI || !pDib->m_pBits || !pDib->m_pBMI)
		return FALSE;

	if (IsCompressed())
	{
		if (!Decompress(8)) // Decompress
			return FALSE;
	}

	if (pDib->IsCompressed())
	{
		if (!pDib->Decompress(pDib->GetBitCount())) // Decompress
			return FALSE;
	}

	if ((GetBitCount() != 8)							||
		(pDib->GetBitCount() != 8)						||
		(GetCompression() != BI_RGB)					||
		(pDib->GetCompression() != BI_RGB)				||
		(GetWidth() != pDib->GetWidth())				||
		(GetHeight() != pDib->GetHeight()))
		return FALSE;

	int nWidthDWAligned = DWALIGNEDWIDTHBYTES(GetWidth() * GetBitCount()); // DWORD aligned (in bytes)

	for (line = 0 ; line < GetHeight() ; line++)
	{
		for (i = (nWidthDWAligned) * line ; i < ((nWidthDWAligned) * (line+1)) ; i++)
		{
			index = m_pBits[i];
			index1 = pDib->m_pBits[i];
			BYTE R, G, B;
			R = (BYTE)ABS((int)m_pColors[index].rgbRed - (int)pDib->m_pColors[index1].rgbRed);
			G = (BYTE)ABS((int)m_pColors[index].rgbGreen - (int)pDib->m_pColors[index1].rgbGreen);
			B = (BYTE)ABS((int)m_pColors[index].rgbBlue - (int)pDib->m_pColors[index1].rgbBlue);
			if (RGBTOGRAY(R, G, B) <= nMinDiff)
				m_pBits[i] = nTransparentIndex;
		}
	}

	return TRUE;
}
