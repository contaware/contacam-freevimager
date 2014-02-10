#include "stdafx.h"
#include "dib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BOOL CDib::FlipTopDown(CDib* pSrcDib/*=NULL*/)
{
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

	DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * GetBitCount());

	// Allocate Bits
	if (m_pBits == NULL)
	{
		// Allocate memory
		m_pBits = (LPBYTE)BIGALLOC(uiDIBScanLineSize * GetHeight());
	}
	// Need to ReAllocate Bits because they are of differente size
	else if (m_dwImageSize != uiDIBScanLineSize * GetHeight())
	{
		BIGFREE(m_pBits);

		// Allocate memory
		m_pBits = (LPBYTE)BIGALLOC(uiDIBScanLineSize * GetHeight());
	}
	if (m_pBits == NULL)
		return FALSE;

	// Copy Vars
	if (bCopySrcToDst)
		CopyVars(*pSrcDib);

	// Set image size
	m_dwImageSize = m_pBMI->bmiHeader.biSizeImage = uiDIBScanLineSize * GetHeight();

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

	// Set Pointer
	LPBYTE lpDstBits = m_pBits + (GetHeight() - 1) * uiDIBScanLineSize; // Points to last Scan Line

	// Flip
	for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
	{
		memcpy((void*)lpDstBits, (void*)(pSrcDib->GetBits() + CurLine*uiDIBScanLineSize), uiDIBScanLineSize); 
		lpDstBits -= uiDIBScanLineSize;
	}

	// Create Palette
	CreatePaletteFromBMI();

	return TRUE;
}

BOOL CDib::FlipLeftRight(CDib* pSrcDib/*=NULL*/)
{
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

	DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * GetBitCount());

	// Allocate Bits
	if (m_pBits == NULL)
	{
		// Allocate memory
		m_pBits = (LPBYTE)BIGALLOC(uiDIBScanLineSize * GetHeight());
	}
	// Need to ReAllocate Bits because they are of differente size
	else if (m_dwImageSize != uiDIBScanLineSize * GetHeight())
	{
		BIGFREE(m_pBits);

		// Allocate memory
		m_pBits = (LPBYTE)BIGALLOC(uiDIBScanLineSize * GetHeight());
	}
	if (m_pBits == NULL)
		return FALSE;

	// Copy Vars
	if (bCopySrcToDst)
		CopyVars(*pSrcDib);

	// Set image size
	m_dwImageSize = m_pBMI->bmiHeader.biSizeImage = uiDIBScanLineSize * GetHeight();

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

	// Set Pointers
	LPBYTE lpSrcBits = pSrcDib->GetBits();
	LPBYTE lpDstBits = m_pBits;
	
	// Flip
	switch (pSrcDib->GetBitCount())
	{
		case 1:
		{
			for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
			{
				for (unsigned int i = 0 ; i < GetWidth() ; i++)
				{
					unsigned int ByteNum = i / 8;
					unsigned int DstByteNum = (GetWidth() - i - 1) / 8;
					BYTE BitNum = (BYTE)(7 - (i % 8)); // 7=MSBit .. 0=LSBit
					BYTE DstBitNum = (BYTE)(7 - ((GetWidth() - i - 1) % 8)); // 7=MSBit .. 0=LSBit
					
					BYTE PixelIndex = (BYTE)(1 & (lpSrcBits[ByteNum] >> BitNum));
					
					if (PixelIndex) // Set the bit
						lpDstBits[DstByteNum] = (BYTE)(lpDstBits[DstByteNum] | (1 << DstBitNum)); 
					else // Clear the bit
						lpDstBits[DstByteNum] = (BYTE)(lpDstBits[DstByteNum] & ~(1 << DstBitNum));
				}	

				lpDstBits += uiDIBScanLineSize;
				lpSrcBits += uiDIBScanLineSize;
			}
			break;
		}
		case 4:
		{
			for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
			{
				for (unsigned int i = 0 ; i < GetWidth() ; i++)
				{
					unsigned int ByteNum = i / 2;
					unsigned int DstByteNum = (GetWidth() - i - 1) / 2;
					BYTE NibbleNum = (BYTE)(1 - (i % 2)); // 1=MSNibble .. 0=LSNibble
					BYTE DstNibbleNum = (BYTE)(1 - ((GetWidth() - i - 1) % 2)); // 1=MSNibble .. 0=LSNibble

					if (NibbleNum == 1) // source pixel is MSNibble
					{
						if (DstNibbleNum == 1) // destination pixel is MSNibble
						{
							lpDstBits[DstByteNum] = (BYTE)((lpDstBits[DstByteNum] & 0x0F) | (lpSrcBits[ByteNum] & 0xF0));
						}
						else // destination pixel is LSNibble
						{
							lpDstBits[DstByteNum] = (BYTE)((lpDstBits[DstByteNum] & 0xF0) | ((lpSrcBits[ByteNum] & 0xF0) >> 4));
						}
					}
					else // source pixel is LSNibble
					{
						if (DstNibbleNum == 1) // destination pixel is MSNibble
						{
							lpDstBits[DstByteNum] = (BYTE)((lpDstBits[DstByteNum] & 0x0F) | ((lpSrcBits[ByteNum] & 0x0F) << 4));
						}
						else // destination pixel is LSNibble
						{
							lpDstBits[DstByteNum] = (BYTE)((lpDstBits[DstByteNum] & 0xF0) | (lpSrcBits[ByteNum] & 0x0F));
						}
					}
				}

				lpDstBits += uiDIBScanLineSize;
				lpSrcBits += uiDIBScanLineSize;
			}
			break;
		}
		case 8:
		{
			for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
			{
				for (unsigned int i = 0 ; i < GetWidth() ; i++)
					lpDstBits[GetWidth() - i - 1] = lpSrcBits[i];		

				lpDstBits += uiDIBScanLineSize;
				lpSrcBits += uiDIBScanLineSize;
			}
			break;
		}
		case 16:
		{
			for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
			{
				for (unsigned int i = 0 ; i < GetWidth() ; i++)
					((WORD*)lpDstBits)[GetWidth() - i - 1] = ((WORD*)lpSrcBits)[i];		

				lpDstBits += uiDIBScanLineSize;
				lpSrcBits += uiDIBScanLineSize;
			}
			break;
		}
		case 24:
		{
			for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
			{
				for (unsigned int i = 0 ; i < GetWidth() ; i++)
				{
					lpDstBits[3*GetWidth() - 3*i - 3] = lpSrcBits[3*i];
					lpDstBits[3*GetWidth() - 3*i - 2] = lpSrcBits[3*i+1];
					lpDstBits[3*GetWidth() - 3*i - 1] = lpSrcBits[3*i+2];
				}

				lpDstBits += uiDIBScanLineSize;
				lpSrcBits += uiDIBScanLineSize;
			}
			break;
		}
		case 32:
		{
			for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
			{
				for (unsigned int i = 0 ; i < GetWidth() ; i++)
					((DWORD*)lpDstBits)[GetWidth() - i - 1] = ((DWORD*)lpSrcBits)[i];		

				lpDstBits += uiDIBScanLineSize;
				lpSrcBits += uiDIBScanLineSize;
			}
			break;
		}
		default:
			
			break;
	}

	// Create Palette
	CreatePaletteFromBMI();

	return TRUE;
}

BOOL CDib::Rotate180(CDib* pSrcDib/*=NULL*/)
{
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

	DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * GetBitCount());

	// Allocate Bits
	if (m_pBits == NULL)
	{
		// Allocate memory
		m_pBits = (LPBYTE)BIGALLOC(uiDIBScanLineSize * GetHeight());
	}
	// Need to ReAllocate Bits because they are of differente size
	else if (m_dwImageSize != uiDIBScanLineSize * GetHeight())
	{
		BIGFREE(m_pBits);

		// Allocate memory
		m_pBits = (LPBYTE)BIGALLOC(uiDIBScanLineSize * GetHeight());
	}
	if (m_pBits == NULL)
		return FALSE;

	// Copy Vars
	if (bCopySrcToDst)
		CopyVars(*pSrcDib);

	// Set image size
	m_dwImageSize = m_pBMI->bmiHeader.biSizeImage = uiDIBScanLineSize * GetHeight();

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

	// Set Pointers
	LPBYTE lpSrcBits = pSrcDib->GetBits();
	LPBYTE lpDstBits = m_pBits + (GetHeight() - 1) * uiDIBScanLineSize; // Points to last Scan Line 
	
	switch (pSrcDib->GetBitCount())
	{
		case 1:
		{
			for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
			{
				for (unsigned int i = 0 ; i < GetWidth() ; i++)
				{
					unsigned int ByteNum = i / 8;
					unsigned int DstByteNum = (GetWidth() - i - 1) / 8;
					BYTE BitNum = (BYTE)(7 - (i % 8)); // 7=MSBit .. 0=LSBit
					BYTE DstBitNum = (BYTE)(7 - ((GetWidth() - i - 1) % 8)); // 7=MSBit .. 0=LSBit
					
					BYTE PixelIndex = (BYTE)(1 & (lpSrcBits[ByteNum] >> BitNum));
					
					if (PixelIndex) // Set the bit
						lpDstBits[DstByteNum] = (BYTE)(lpDstBits[DstByteNum] | (1 << DstBitNum)); 
					else // Clear the bit
						lpDstBits[DstByteNum] = (BYTE)(lpDstBits[DstByteNum] & ~(1 << DstBitNum));
				}	

				lpDstBits -= uiDIBScanLineSize;
				lpSrcBits += uiDIBScanLineSize;
			}
			break;
		}
		case 4:
		{
			for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
			{
				for (unsigned int i = 0 ; i < GetWidth() ; i++)
				{
					unsigned int ByteNum = i / 2;
					unsigned int DstByteNum = (GetWidth() - i - 1) / 2;
					BYTE NibbleNum = (BYTE)(1 - (i % 2)); // 1=MSNibble .. 0=LSNibble
					BYTE DstNibbleNum = (BYTE)(1 - ((GetWidth() - i - 1) % 2)); // 1=MSNibble .. 0=LSNibble

					if (NibbleNum == 1) // source pixel is MSNibble
					{
						if (DstNibbleNum == 1) // destination pixel is MSNibble
						{
							lpDstBits[DstByteNum] = (BYTE)((lpDstBits[DstByteNum] & 0x0F) | (lpSrcBits[ByteNum] & 0xF0));
						}
						else // destination pixel is LSNibble
						{
							lpDstBits[DstByteNum] = (BYTE)((lpDstBits[DstByteNum] & 0xF0) | ((lpSrcBits[ByteNum] & 0xF0) >> 4));
						}
					}
					else // source pixel is LSNibble
					{
						if (DstNibbleNum == 1) // destination pixel is MSNibble
						{
							lpDstBits[DstByteNum] = (BYTE)((lpDstBits[DstByteNum] & 0x0F) | ((lpSrcBits[ByteNum] & 0x0F) << 4));
						}
						else // destination pixel is LSNibble
						{
							lpDstBits[DstByteNum] = (BYTE)((lpDstBits[DstByteNum] & 0xF0) | (lpSrcBits[ByteNum] & 0x0F));
						}
					}
				}

				lpDstBits -= uiDIBScanLineSize;
				lpSrcBits += uiDIBScanLineSize;
			}
			break;
		}
		case 8:
		{
			for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
			{
				for (unsigned int i = 0 ; i < GetWidth() ; i++)
					lpDstBits[GetWidth() - i - 1] = lpSrcBits[i];		

				lpDstBits -= uiDIBScanLineSize;
				lpSrcBits += uiDIBScanLineSize;
			}
			break;
		}
		case 16:
		{
			for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
			{
				for (unsigned int i = 0 ; i < GetWidth() ; i++)
					((WORD*)lpDstBits)[GetWidth() - i - 1] = ((WORD*)lpSrcBits)[i];		

				lpDstBits -= uiDIBScanLineSize;
				lpSrcBits += uiDIBScanLineSize;
			}
			break;
		}
		case 24:
		{
			for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
			{
				for (unsigned int i = 0 ; i < GetWidth() ; i++)
				{
					lpDstBits[3*GetWidth() - 3*i - 3] = lpSrcBits[3*i];
					lpDstBits[3*GetWidth() - 3*i - 2] = lpSrcBits[3*i+1];
					lpDstBits[3*GetWidth() - 3*i - 1] = lpSrcBits[3*i+2];
				}

				lpDstBits -= uiDIBScanLineSize;
				lpSrcBits += uiDIBScanLineSize;
			}
			break;
		}
		case 32:
		{
			for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
			{
				for(unsigned int i = 0 ; i < GetWidth() ; i++)
					((DWORD*)lpDstBits)[GetWidth() - i - 1] = ((DWORD*)lpSrcBits)[i];		

				lpDstBits -= uiDIBScanLineSize;
				lpSrcBits += uiDIBScanLineSize;
			}
			break;
		}
		default:
			
			break;
	}

	// Create Palette
	CreatePaletteFromBMI();

	return TRUE;
}

BOOL CDib::SwapWidthHeight()
{
	if (!m_pBMI)
		return FALSE;

	// Swap Width <-> Height
	LONG nWidth = m_pBMI->bmiHeader.biWidth;
	LONG nHeight = m_pBMI->bmiHeader.biHeight;
	m_pBMI->bmiHeader.biHeight = nWidth;
	m_pBMI->bmiHeader.biWidth = nHeight;
	
	// Because of padding the image size may change!
	DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * GetBitCount());
	m_dwImageSize = m_pBMI->bmiHeader.biSizeImage = uiDIBScanLineSize * GetHeight();

	// Swap Dpm
	LONG lXDpm = m_pBMI->bmiHeader.biXPelsPerMeter;
	LONG lYDpm = m_pBMI->bmiHeader.biYPelsPerMeter;
	m_pBMI->bmiHeader.biXPelsPerMeter = lYDpm;
	m_pBMI->bmiHeader.biYPelsPerMeter = lXDpm;

	return TRUE;
}

// Bitmaps are stored Bottom-up!
BOOL CDib::Rotate90CW(CDib* pSrcDib/*=NULL*/)
{
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

	DWORD nSrcWidth = pSrcDib->GetWidth();	// = Destination Height
	DWORD nSrcHeight = pSrcDib->GetHeight();// = Destination Width

	DWORD uiDIBSourceScanLineSize = DWALIGNEDWIDTHBYTES(nSrcWidth * pSrcDib->GetBitCount());
	DWORD uiDIBTargetScanLineSize = DWALIGNEDWIDTHBYTES(nSrcHeight * GetBitCount());

	// Allocate Bits
	if (m_pBits == NULL)
	{
		// Allocate memory
		m_pBits = (LPBYTE)BIGALLOC(uiDIBTargetScanLineSize * nSrcWidth);
	}
	// Need to ReAllocate Bits because they are of differente size
	else if (m_dwImageSize != uiDIBTargetScanLineSize * nSrcWidth)
	{
		BIGFREE(m_pBits);

		// Allocate memory
		m_pBits = (LPBYTE)BIGALLOC(uiDIBTargetScanLineSize * nSrcWidth);
	}
	if (m_pBits == NULL)
		return FALSE;

	// Copy Vars
	if (bCopySrcToDst)
		CopyVars(*pSrcDib);

	// Swap
	m_pBMI->bmiHeader.biWidth = nSrcWidth;
	m_pBMI->bmiHeader.biHeight = nSrcHeight;
	m_pBMI->bmiHeader.biXPelsPerMeter = pSrcDib->m_pBMI->bmiHeader.biXPelsPerMeter;
	m_pBMI->bmiHeader.biYPelsPerMeter = pSrcDib->m_pBMI->bmiHeader.biXPelsPerMeter;
	SwapWidthHeight();

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

	// Set Pointers
	LPBYTE lpSrcBits = pSrcDib->GetBits();
	LPBYTE lpDstBits;
	
	switch (pSrcDib->GetBitCount())
	{
		case 1:
		{
			for (unsigned int CurLine = 0 ; CurLine < nSrcHeight ; CurLine++)
			{
				lpDstBits = m_pBits + (GetHeight() - 1)*uiDIBTargetScanLineSize; 
				for (unsigned int i = 0 ; i < nSrcWidth ; i++)
				{
					unsigned int ByteNum = i / 8;
					unsigned int DstByteNum = CurLine / 8;
					BYTE BitNum = (BYTE)(7 - (i % 8)); // 7=MSBit .. 0=LSBit
					BYTE DstBitNum = (BYTE)(7 - (CurLine % 8)); // 7=MSBit .. 0=LSBit
					
					BYTE PixelIndex = (BYTE)(1 & (lpSrcBits[ByteNum] >> BitNum));
					
					if (PixelIndex) // Set the bit
						lpDstBits[DstByteNum] = (BYTE)(lpDstBits[DstByteNum] | (1 << DstBitNum)); 
					else // Clear the bit
						lpDstBits[DstByteNum] = (BYTE)(lpDstBits[DstByteNum] & ~(1 << DstBitNum));

					lpDstBits -= uiDIBTargetScanLineSize;
				}	
				lpSrcBits += uiDIBSourceScanLineSize;
			}
			break;
		}
		case 4:
		{
			for (unsigned int CurLine = 0 ; CurLine < nSrcHeight ; CurLine++)
			{
				lpDstBits = m_pBits + (GetHeight() - 1)*uiDIBTargetScanLineSize; 
				for (unsigned int i = 0 ; i < nSrcWidth ; i++)
				{
					unsigned int ByteNum = i / 2;
					unsigned int DstByteNum = CurLine / 2;
					BYTE NibbleNum = (BYTE)(1 - (i % 2)); // 1=MSNibble .. 0=LSNibble
					BYTE DstNibbleNum = (BYTE)(1 - (CurLine % 2)); // 1=MSNibble .. 0=LSNibble

					if (NibbleNum == 1) // source pixel is MSNibble
					{
						if (DstNibbleNum == 1) // destination pixel is MSNibble
						{
							lpDstBits[DstByteNum] = (BYTE)((lpDstBits[DstByteNum] & 0x0F) | (lpSrcBits[ByteNum] & 0xF0));
						}
						else // destination pixel is LSNibble
						{
							lpDstBits[DstByteNum] = (BYTE)((lpDstBits[DstByteNum] & 0xF0) | ((lpSrcBits[ByteNum] & 0xF0) >> 4));
						}
					}
					else // source pixel is LSNibble
					{
						if (DstNibbleNum == 1) // destination pixel is MSNibble
						{
							lpDstBits[DstByteNum] = (BYTE)((lpDstBits[DstByteNum] & 0x0F) | ((lpSrcBits[ByteNum] & 0x0F) << 4));
						}
						else // destination pixel is LSNibble
						{
							lpDstBits[DstByteNum] = (BYTE)((lpDstBits[DstByteNum] & 0xF0) | (lpSrcBits[ByteNum] & 0x0F));
						}
					}

					lpDstBits -= uiDIBTargetScanLineSize;
				}
				lpSrcBits += uiDIBSourceScanLineSize;
			}
			break;
		}
		case 8:
		{
			for (unsigned int CurLine = 0 ; CurLine < nSrcHeight ; CurLine++)
			{
				lpDstBits = m_pBits + (GetHeight() - 1)*uiDIBTargetScanLineSize; 
				for (unsigned int i = 0 ; i < nSrcWidth ; i++)
				{
					lpDstBits[CurLine] = lpSrcBits[i];
					lpDstBits -= uiDIBTargetScanLineSize;
				}
				lpSrcBits += uiDIBSourceScanLineSize;
			}
			break;
		}
		case 16:
		{
			for (unsigned int CurLine = 0 ; CurLine < nSrcHeight ; CurLine++)
			{
				lpDstBits = m_pBits + (GetHeight() - 1)*uiDIBTargetScanLineSize;
				for (unsigned int i = 0 ; i < nSrcWidth ; i++)
				{
					((WORD*)lpDstBits)[CurLine] = ((WORD*)lpSrcBits)[i];
					lpDstBits -= uiDIBTargetScanLineSize;
				}
				lpSrcBits += uiDIBSourceScanLineSize;
			}
			break;
		}
		case 24:
		{
			for (unsigned int CurLine = 0 ; CurLine < nSrcHeight ; CurLine++)
			{
				lpDstBits = m_pBits + (GetHeight() - 1)*uiDIBTargetScanLineSize;
				for (unsigned int i = 0 ; i < nSrcWidth ; i++)
				{
					lpDstBits[3*CurLine] = lpSrcBits[3*i];
					lpDstBits[3*CurLine + 1] = lpSrcBits[3*i+1];
					lpDstBits[3*CurLine + 2] = lpSrcBits[3*i+2];
					lpDstBits -= uiDIBTargetScanLineSize;
				}
				lpSrcBits += uiDIBSourceScanLineSize;
			}
			break;
		}
		case 32:
		{
			for (unsigned int CurLine = 0 ; CurLine < nSrcHeight ; CurLine++)
			{
				lpDstBits = m_pBits + (GetHeight() - 1)*uiDIBTargetScanLineSize;
				for (unsigned int i = 0 ; i < nSrcWidth ; i++)
				{
					((DWORD*)lpDstBits)[CurLine] = ((DWORD*)lpSrcBits)[i];
					lpDstBits -= uiDIBTargetScanLineSize;
				}
				lpSrcBits += uiDIBSourceScanLineSize;
			}
			break;
		}
		default:
			
			break;
	}

	// Create Palette
	CreatePaletteFromBMI();

	return TRUE;
}

// Bitmaps are stored Bottom-up!
BOOL CDib::Rotate90CCW(CDib* pSrcDib/*=NULL*/)
{
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

	DWORD nSrcWidth = pSrcDib->GetWidth();	// = Destination Height
	DWORD nSrcHeight = pSrcDib->GetHeight();// = Destination Width

	DWORD uiDIBSourceScanLineSize = DWALIGNEDWIDTHBYTES(nSrcWidth * pSrcDib->GetBitCount());
	DWORD uiDIBTargetScanLineSize = DWALIGNEDWIDTHBYTES(nSrcHeight * GetBitCount());

	// Allocate Bits
	if (m_pBits == NULL)
	{
		// Allocate memory
		m_pBits = (LPBYTE)BIGALLOC(uiDIBTargetScanLineSize * nSrcWidth);
	}
	// Need to ReAllocate Bits because they are of differente size
	else if (m_dwImageSize != uiDIBTargetScanLineSize * nSrcWidth)
	{
		BIGFREE(m_pBits);

		// Allocate memory
		m_pBits = (LPBYTE)BIGALLOC(uiDIBTargetScanLineSize * nSrcWidth);
	}
	if (m_pBits == NULL)
		return FALSE;

	// Copy Vars
	if (bCopySrcToDst)
		CopyVars(*pSrcDib);

	// Swap
	m_pBMI->bmiHeader.biWidth = nSrcWidth;
	m_pBMI->bmiHeader.biHeight = nSrcHeight;
	m_pBMI->bmiHeader.biXPelsPerMeter = pSrcDib->m_pBMI->bmiHeader.biXPelsPerMeter;
	m_pBMI->bmiHeader.biYPelsPerMeter = pSrcDib->m_pBMI->bmiHeader.biXPelsPerMeter;
	SwapWidthHeight();

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

	// Set Pointers
	LPBYTE lpSrcBits = pSrcDib->GetBits();
	LPBYTE lpDstBits; 

	switch (pSrcDib->GetBitCount())
	{
		case 1:
		{
			for (unsigned int CurLine = 0 ; CurLine < nSrcHeight ; CurLine++)
			{
				lpDstBits = m_pBits; 
				for (unsigned int i = 0 ; i < nSrcWidth ; i++)
				{
					unsigned int ByteNum = i / 8;
					unsigned int DstByteNum = (nSrcHeight - CurLine - 1) / 8;
					BYTE BitNum = (BYTE)(7 - (i % 8)); // 7=MSBit .. 0=LSBit
					BYTE DstBitNum = (BYTE)(7 - ((nSrcHeight - CurLine - 1) % 8)); // 7=MSBit .. 0=LSBit
					
					BYTE PixelIndex = (BYTE)(1 & (lpSrcBits[ByteNum] >> BitNum));
					
					if (PixelIndex) // Set the bit
						lpDstBits[DstByteNum] = (BYTE)(lpDstBits[DstByteNum] | (1 << DstBitNum)); 
					else // Clear the bit
						lpDstBits[DstByteNum] = (BYTE)(lpDstBits[DstByteNum] & ~(1 << DstBitNum));

					lpDstBits += uiDIBTargetScanLineSize;
				}	
				lpSrcBits += uiDIBSourceScanLineSize;
			}
			break;
		}
		case 4:
		{
			for (unsigned int CurLine = 0 ; CurLine < nSrcHeight ; CurLine++)
			{
				lpDstBits = m_pBits; 
				for (unsigned int i = 0 ; i < nSrcWidth ; i++)
				{
					unsigned int ByteNum = i / 2;
					unsigned int DstByteNum = (nSrcHeight - CurLine - 1) / 2;
					BYTE NibbleNum = (BYTE)(1 - (i % 2)); // 1=MSNibble .. 0=LSNibble
					BYTE DstNibbleNum = (BYTE)(1 - ((nSrcHeight - CurLine - 1) % 2)); // 1=MSNibble .. 0=LSNibble

					if (NibbleNum == 1) // source pixel is MSNibble
					{
						if (DstNibbleNum == 1) // destination pixel is MSNibble
						{
							lpDstBits[DstByteNum] = (BYTE)((lpDstBits[DstByteNum] & 0x0F) | (lpSrcBits[ByteNum] & 0xF0));
						}
						else // destination pixel is LSNibble
						{
							lpDstBits[DstByteNum] = (BYTE)((lpDstBits[DstByteNum] & 0xF0) | ((lpSrcBits[ByteNum] & 0xF0) >> 4));
						}
					}
					else // source pixel is LSNibble
					{
						if (DstNibbleNum == 1) // destination pixel is MSNibble
						{
							lpDstBits[DstByteNum] = (BYTE)((lpDstBits[DstByteNum] & 0x0F) | ((lpSrcBits[ByteNum] & 0x0F) << 4));
						}
						else // destination pixel is LSNibble
						{
							lpDstBits[DstByteNum] = (BYTE)((lpDstBits[DstByteNum] & 0xF0) | (lpSrcBits[ByteNum] & 0x0F));
						}
					}

					lpDstBits += uiDIBTargetScanLineSize;
				}
				lpSrcBits += uiDIBSourceScanLineSize;
			}
			break;
		}
		case 8:
		{
			for (unsigned int CurLine = 0 ; CurLine < nSrcHeight ; CurLine++)
			{
				lpDstBits = m_pBits; 
				for (unsigned int i = 0 ; i < nSrcWidth ; i++)
				{
					lpDstBits[nSrcHeight - CurLine - 1] = lpSrcBits[i];
					lpDstBits += uiDIBTargetScanLineSize;
				}
				lpSrcBits += uiDIBSourceScanLineSize;
			}
			break;
		}
		case 16:
		{
			for (unsigned int CurLine = 0 ; CurLine < nSrcHeight ; CurLine++)
			{
				lpDstBits = m_pBits;
				for (unsigned int i = 0 ; i < nSrcWidth ; i++)
				{
					((WORD*)lpDstBits)[nSrcHeight - CurLine - 1] = ((WORD*)lpSrcBits)[i];
					lpDstBits += uiDIBTargetScanLineSize;
				}
				lpSrcBits += uiDIBSourceScanLineSize;
			}
			break;
		}
		case 24:
		{
			for (unsigned int CurLine = 0 ; CurLine < nSrcHeight ; CurLine++)
			{
				lpDstBits = m_pBits;
				for (unsigned int i = 0 ; i < nSrcWidth ; i++)
				{
					lpDstBits[3*nSrcHeight - 3*CurLine - 3] = lpSrcBits[3*i];
					lpDstBits[3*nSrcHeight - 3*CurLine - 2] = lpSrcBits[3*i+1];
					lpDstBits[3*nSrcHeight - 3*CurLine - 1] = lpSrcBits[3*i+2];
					lpDstBits += uiDIBTargetScanLineSize;
				}
				lpSrcBits += uiDIBSourceScanLineSize;
			}
			break;
		}
		case 32:
		{
			for (unsigned int CurLine = 0 ; CurLine < nSrcHeight ; CurLine++)
			{
				lpDstBits = m_pBits;
				for (unsigned int i = 0 ; i < nSrcWidth ; i++)
				{
					((DWORD*)lpDstBits)[nSrcHeight - CurLine - 1] = ((DWORD*)lpSrcBits)[i];
					lpDstBits += uiDIBTargetScanLineSize;
				}
				lpSrcBits += uiDIBSourceScanLineSize;
			}
			break;
		}
		default:
			
			break;
	}

	// Create Palette
	CreatePaletteFromBMI();

	return TRUE;
}

// Rotate by angle (radiants) clock-wise
// and set the background to red, green, blue
BOOL CDib::RotateCW(double angle,
					BYTE red,
					BYTE green,
					BYTE blue,
					BYTE alpha,
					BOOL bAntiAliasing,
					CDib* pSrcDib/*=NULL*/,
					BOOL bOnlyHeader/*=FALSE*/,
					CWnd* pProgressWnd/*=NULL*/,
					BOOL bProgressSend/*=TRUE*/)
{
	return RotateCCW(	2.0*PI - angle,
						red, green, blue, alpha,
						bAntiAliasing,
						pSrcDib,
						bOnlyHeader,
						pProgressWnd,
						bProgressSend);
}

// Rotate by angle (radiants) counter-clock-wise
// and set the background to red, green, blue
//
// Rotation Formula (1):
// xrot = x*cos(angle) - y*sin(angle)
// yrot = y*cos(angle) + x*sin(angle)
//
// Inverse Rotation Formula (2):
// x = xrot*cos(angle) + yrot*sin(angle)
// y = yrot*cos(angle) - xrot*sin(angle)
BOOL CDib::RotateCCW(	double angle,
						BYTE red,
						BYTE green,
						BYTE blue,
						BYTE alpha,
						BOOL bAntiAliasing,
						CDib* pSrcDib/*=NULL*/,
						BOOL bOnlyHeader/*=FALSE*/,
						CWnd* pProgressWnd/*=NULL*/,
						BOOL bProgressSend/*=TRUE*/)
{
	if (!m_pBMI)
		return FALSE;

	double cA, sA, orgX, orgY, errX, errY;
	double x0y0_weight, x1y0_weight, x0y1_weight, x1y1_weight;
	int stepX, stepY;
	int iorgX, iorgY, iorgX1, iorgY1;
	RGBQUAD rgb;
	BYTE R, G, B;
	BYTE x0y0[4];
	BYTE x1y0[4];
	BYTE x0y1[4];
	BYTE x1y1[4];

	// Cosinus and Sinus of the Angle
	cA = cos(angle);
	sA = sin(angle);

	// The Source Rectangle
	int nSrcWidth, nSrcHeight;
	if (pSrcDib == NULL)
	{
		nSrcWidth = (int)GetWidth();
		nSrcHeight = (int)GetHeight();
	}
	else
	{
		nSrcWidth = (int)pSrcDib->GetWidth();
		nSrcHeight = (int)pSrcDib->GetHeight();
	}

	// The Source Offset
	// (Rotate the source bitmap around the center -> center it!)
	int nSrcOffsetX = nSrcWidth  / 2;
	int nSrcOffsetY = nSrcHeight / 2;
	double dSrcOffsetX = ((double)nSrcWidth)  / 2.0;
	double dSrcOffsetY = ((double)nSrcHeight) / 2.0;

	// The size of the destination rectangle is
	int nDstWidth  = Round(ABS(nSrcHeight * sA) + ABS(nSrcWidth * cA));
	int nDstHeight = Round(ABS(nSrcHeight * cA) + ABS(nSrcWidth * sA));

	// The Destination Offset
	int nDstOffsetX = nDstWidth / 2;
	int nDstOffsetY = nDstHeight / 2;
	double dDstOffsetX = ((double)nDstWidth) / 2.0;
	double dDstOffsetY = ((double)nDstHeight) / 2.0;

	// If Only Header
	if (bOnlyHeader)
	{
		m_pBMI->bmiHeader.biHeight = nDstHeight;
		m_pBMI->bmiHeader.biWidth = nDstWidth;
		if (!IsCompressed())
			m_pBMI->bmiHeader.biSizeImage = DWALIGNEDWIDTHBYTES(GetWidth() * GetBitCount()) * GetHeight();
		else
			m_pBMI->bmiHeader.biSizeImage = 0;
		return TRUE;
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

	// Scan Line Alignments
	DWORD uiDIBSourceScanLineSize = DWALIGNEDWIDTHBYTES(nSrcWidth * pSrcDib->GetBitCount());
	DWORD uiDIBTargetScanLineSize = DWALIGNEDWIDTHBYTES(nDstWidth * GetBitCount());

	// Allocate Bits
	if (m_pBits == NULL)
	{
		// Allocate memory
		m_pBits = (LPBYTE)BIGALLOC(uiDIBTargetScanLineSize * nDstHeight);
	}
	// Need to ReAllocate Bits because they are of differente size
	else if (m_dwImageSize != uiDIBTargetScanLineSize * nDstHeight)
	{
		BIGFREE(m_pBits);

		// Allocate memory
		m_pBits = (LPBYTE)BIGALLOC(uiDIBTargetScanLineSize * nDstHeight);
	}
	if (m_pBits == NULL)
		return FALSE;

	// Copy Vars
	if (bCopySrcToDst)
		CopyVars(*pSrcDib);

	// Set Width & Height
	m_pBMI->bmiHeader.biHeight = nDstHeight;
	m_pBMI->bmiHeader.biWidth = nDstWidth;
	
	// Set image size
	m_dwImageSize = m_pBMI->bmiHeader.biSizeImage = uiDIBTargetScanLineSize * nDstHeight;

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

	// Set Pointers
	LPBYTE lpSrcBits = pSrcDib->GetBits();
	LPBYTE lpDstBits = m_pBits;
	
	DIB_INIT_PROGRESS;

	switch (pSrcDib->GetBitCount())
	{
		case 1:
		{
			InitGetClosestColorIndex();

			// Step through the destination bitmap
			for (stepY = 0; stepY < nDstHeight; stepY++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, stepY, nDstHeight);

				for (stepX = 0; stepX < nDstWidth; stepX++)
				{
					if (!bAntiAliasing)
					{
						// Transform the destination point back
						// see inverse rotation formula (2)
						orgX = (stepX - nDstOffsetX)*cA + (stepY - nDstOffsetY)*sA;
						orgY = (stepY - nDstOffsetY)*cA - (stepX - nDstOffsetX)*sA;

						// Translate to positive coordinates
						iorgX = (int)(orgX + dSrcOffsetX);
						iorgY = (int)(orgY + dSrcOffsetY);

						BYTE BitNum = (BYTE)(7 - ((iorgX + 8*iorgY*uiDIBSourceScanLineSize) % 8)); // 7=MSBit .. 0=LSBit
						BYTE DstBitNum = (BYTE)(7 - (stepX % 8)); // 7=MSBit .. 0=LSBit

						// Check if it is inside the original source rectangle
						if ((iorgX >= 0) && (iorgY >= 0) && (iorgX < (int)nSrcWidth) && (iorgY < (int)nSrcHeight))
						{
							// Inside the source bitmap -> copy the bits
							BYTE PixelIndex = (BYTE)(1 & (lpSrcBits[iorgX/8 + iorgY*uiDIBSourceScanLineSize] >> BitNum));

							if (PixelIndex) // Set the bit
								lpDstBits[stepX/8] = (BYTE)(lpDstBits[stepX/8] | (1 << DstBitNum));
							else // Clear the bit
								lpDstBits[stepX/8] = (BYTE)(lpDstBits[stepX/8] & ~(1 << DstBitNum));
						}
						else
						{
							// Outside the source -> set to the color passed by parameter
							if (GetClosestColorIndex(RGB(red, green, blue)))
								lpDstBits[stepX/8] = (BYTE)(lpDstBits[stepX/8] | (1 << DstBitNum)); // Set the bit
							else
								lpDstBits[stepX/8] = (BYTE)(lpDstBits[stepX/8] & ~(1 << DstBitNum));// Clear the bit
						}
					}
					else
					{
						// Transform the destination point back
						// see inverse rotation formula (2)
						orgX = (stepX + 0.5 - dDstOffsetX)*cA + (stepY + 0.5 - dDstOffsetY)*sA;
						orgY = (stepY + 0.5 - dDstOffsetY)*cA - (stepX + 0.5 - dDstOffsetX)*sA;

						// Translate to positive coordinates
						iorgX1 = Round(orgX + dSrcOffsetX); // Center X coord.of the 4 source pixels
						iorgY1 = Round(orgY + dSrcOffsetY); // Center Y coord.of the 4 source pixels
						iorgX = iorgX1 - 1;
						iorgY = iorgY1 - 1;

						// Error
						errX = orgX + dSrcOffsetX - iorgX1 + 0.5;
						errY = orgY + dSrcOffsetY - iorgY1 + 0.5;

						// Weights
						x0y0_weight = (1.0 - errX) * (1.0 - errY);
						x0y1_weight = (1.0 - errX) * (errY);
						x1y0_weight = (errX) * (1.0 - errY);
						x1y1_weight = (errX) * (errY);

						BYTE BitNum = (BYTE)(7 - ((iorgX + 8*iorgY*uiDIBSourceScanLineSize) % 8)); // 7=MSBit .. 0=LSBit
						BYTE DstBitNum = (BYTE)(7 - (stepX % 8)); // 7=MSBit .. 0=LSBit

						// x0y0
						if ((iorgX >= 0) && (iorgY >= 0) && (iorgX < (int)nSrcWidth) && (iorgY < (int)nSrcHeight))
						{
							// Inside the source bitmap -> copy the bits
							rgb = m_pColors[1 & (lpSrcBits[iorgX/8 + iorgY*uiDIBSourceScanLineSize] >> BitNum)];
							x0y0[2] = rgb.rgbRed;
							x0y0[1] = rgb.rgbGreen;
							x0y0[0] = rgb.rgbBlue;
						}
						else
						{
							// Outside the source -> set to the color passed by parameter
							x0y0[2] = red;
							x0y0[1] = green;
							x0y0[0] = blue;
						}

						// x1y0
						if ((iorgX1 >= 0) && (iorgY >= 0) && (iorgX1 < (int)nSrcWidth) && (iorgY < (int)nSrcHeight))
						{
							// Inside the source bitmap -> copy the bits
							rgb = m_pColors[1 & (lpSrcBits[iorgX1/8 + iorgY*uiDIBSourceScanLineSize] >> BitNum)];
							x1y0[2] = rgb.rgbRed;
							x1y0[1] = rgb.rgbGreen;
							x1y0[0] = rgb.rgbBlue;
						}
						else
						{
							// Outside the source -> set to the color passed by parameter
							x1y0[2] = red;
							x1y0[1] = green;
							x1y0[0] = blue;
						}

						// x0y1
						if ((iorgX >= 0) && (iorgY1 >= 0) && (iorgX < (int)nSrcWidth) && (iorgY1 < (int)nSrcHeight))
						{
							// Inside the source bitmap -> copy the bits
							rgb = m_pColors[1 & (lpSrcBits[iorgX/8 + iorgY1*uiDIBSourceScanLineSize] >> BitNum)];
							x0y1[2] = rgb.rgbRed;
							x0y1[1] = rgb.rgbGreen;
							x0y1[0] = rgb.rgbBlue;
						}
						else
						{
							// Outside the source -> set to the color passed by parameter
							x0y1[2] = red;
							x0y1[1] = green;
							x0y1[0] = blue;
						}

						// x1y1
						if ((iorgX1 >= 0) && (iorgY1 >= 0) && (iorgX1 < (int)nSrcWidth) && (iorgY1 < (int)nSrcHeight))
						{
							// Inside the source bitmap -> copy the bits
							rgb = m_pColors[1 & (lpSrcBits[iorgX1/8 + iorgY1*uiDIBSourceScanLineSize] >> BitNum)];
							x1y1[2] = rgb.rgbRed;
							x1y1[1] = rgb.rgbGreen;
							x1y1[0] = rgb.rgbBlue;
						}
						else
						{
							// Outside the source -> set to the color passed by parameter
							x1y1[2] = red;
							x1y1[1] = green;
							x1y1[0] = blue;
						}
						
						R = Round(	x0y0_weight * (double)x0y0[2] + 
									x1y0_weight * (double)x1y0[2] +
									x0y1_weight * (double)x0y1[2] +
									x1y1_weight * (double)x1y1[2]);
						G = Round(	x0y0_weight * (double)x0y0[1] + 
									x1y0_weight * (double)x1y0[1] +
									x0y1_weight * (double)x0y1[1] +
									x1y1_weight * (double)x1y1[1]);
						B = Round(	x0y0_weight * (double)x0y0[0] + 
									x1y0_weight * (double)x1y0[0] +
									x0y1_weight * (double)x0y1[0] +
									x1y1_weight * (double)x1y1[0]);
						int index = GetClosestColorIndex(RGB(R, G, B));
						if (index) // Set the bit
							lpDstBits[stepX/8] = (BYTE)(lpDstBits[stepX/8] | (1 << DstBitNum));
						else // Clear the bit
							lpDstBits[stepX/8] = (BYTE)(lpDstBits[stepX/8] & ~(1 << DstBitNum));
					}
				}
				lpDstBits += uiDIBTargetScanLineSize;		
			}
			break;
		}
		case 4:
		{
			InitGetClosestColorIndex();

			// Step through the destination bitmap
			for (stepY = 0; stepY < nDstHeight; stepY++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, stepY, nDstHeight);

				for (stepX = 0; stepX < nDstWidth; stepX++)
				{
					if (!bAntiAliasing)
					{
						// Transform the destination point back
						// see inverse rotation formula (2)
						orgX = (stepX - nDstOffsetX)*cA + (stepY - nDstOffsetY)*sA;
						orgY = (stepY - nDstOffsetY)*cA - (stepX - nDstOffsetX)*sA;

						// Translate to positive coordinates
						iorgX = (int)(orgX + dSrcOffsetX);
						iorgY = (int)(orgY + dSrcOffsetY);
						
						BYTE NibbleNum = (BYTE)(1 - ((iorgX + 2*iorgY*uiDIBSourceScanLineSize) % 2)); // 1=MSNibble .. 0=LSNibble
						BYTE DstNibbleNum = (BYTE)(1 - (stepX % 2)); // 1=MSNibble .. 0=LSNibble

						// Check if it is inside the original source rectangle
						if ((iorgX >= 0) && (iorgY >= 0) && (iorgX < (int)nSrcWidth) && (iorgY < (int)nSrcHeight))
						{
							// Inside the source bitmap -> copy the bits
							
							if (NibbleNum == 1) // source pixel is MSNibble
							{
								if (DstNibbleNum == 1) // destination pixel is MSNibble
									lpDstBits[stepX/2] = (BYTE)((lpDstBits[stepX/2] & 0x0F) | (lpSrcBits[iorgX/2 + iorgY*uiDIBSourceScanLineSize] & 0xF0));
								else // destination pixel is LSNibble
									lpDstBits[stepX/2] = (BYTE)((lpDstBits[stepX/2] & 0xF0) | ((lpSrcBits[iorgX/2 + iorgY*uiDIBSourceScanLineSize] & 0xF0) >> 4));
							}
							else // source pixel is LSNibble
							{
								if (DstNibbleNum == 1) // destination pixel is MSNibble
									lpDstBits[stepX/2] = (BYTE)((lpDstBits[stepX/2] & 0x0F) | ((lpSrcBits[iorgX/2 + iorgY*uiDIBSourceScanLineSize] & 0x0F) << 4));
								else // destination pixel is LSNibble
									lpDstBits[stepX/2] = (BYTE)((lpDstBits[stepX/2] & 0xF0) | (lpSrcBits[iorgX/2 + iorgY*uiDIBSourceScanLineSize] & 0x0F));
							}
						}
						else
						{
							// Outside the source -> set to the color passed by parameter
							
							int index = GetClosestColorIndex(RGB(red, green, blue));
							if (DstNibbleNum == 1) // destination pixel is MSNibble
								lpDstBits[stepX/2] = (BYTE)((lpDstBits[stepX/2] & 0x0F) | ((index & 0x0F) << 4));
							else // destination pixel is LSNibble
								lpDstBits[stepX/2] = (BYTE)((lpDstBits[stepX/2] & 0xF0) | (index & 0x0F));
						}
					}
					else
					{
						// Transform the destination point back
						// see inverse rotation formula (2)
						orgX = (stepX + 0.5 - dDstOffsetX)*cA + (stepY + 0.5 - dDstOffsetY)*sA;
						orgY = (stepY + 0.5 - dDstOffsetY)*cA - (stepX + 0.5 - dDstOffsetX)*sA;

						// Translate to positive coordinates
						iorgX1 = Round(orgX + dSrcOffsetX); // Center X coord.of the 4 source pixels
						iorgY1 = Round(orgY + dSrcOffsetY); // Center Y coord.of the 4 source pixels
						iorgX = iorgX1 - 1;
						iorgY = iorgY1 - 1;

						// Error
						errX = orgX + dSrcOffsetX - iorgX1 + 0.5;
						errY = orgY + dSrcOffsetY - iorgY1 + 0.5;

						// Weights
						x0y0_weight = (1.0 - errX) * (1.0 - errY);
						x0y1_weight = (1.0 - errX) * (errY);
						x1y0_weight = (errX) * (1.0 - errY);
						x1y1_weight = (errX) * (errY);

						BYTE NibbleNum = (BYTE)(1 - ((iorgX + 2*iorgY*uiDIBSourceScanLineSize) % 2)); // 1=MSNibble .. 0=LSNibble
						BYTE DstNibbleNum = (BYTE)(1 - (stepX % 2)); // 1=MSNibble .. 0=LSNibble

						// x0y0
						if ((iorgX >= 0) && (iorgY >= 0) && (iorgX < (int)nSrcWidth) && (iorgY < (int)nSrcHeight))
						{
							// Inside the source bitmap -> copy the bits
							if (NibbleNum == 1) // source pixel is MSNibble
								rgb = m_pColors[(lpSrcBits[iorgX/2 + iorgY*uiDIBSourceScanLineSize] & 0xF0) >> 4];
							else				// source pixel is LSNibble
								rgb = m_pColors[lpSrcBits[iorgX/2 + iorgY*uiDIBSourceScanLineSize] & 0x0F];
							x0y0[2] = rgb.rgbRed;
							x0y0[1] = rgb.rgbGreen;
							x0y0[0] = rgb.rgbBlue;
						}
						else
						{
							// Outside the source -> set to the color passed by parameter
							x0y0[2] = red;
							x0y0[1] = green;
							x0y0[0] = blue;
						}

						// x1y0
						if ((iorgX1 >= 0) && (iorgY >= 0) && (iorgX1 < (int)nSrcWidth) && (iorgY < (int)nSrcHeight))
						{
							// Inside the source bitmap -> copy the bits
							if (NibbleNum == 1) // source pixel is MSNibble
								rgb = m_pColors[(lpSrcBits[iorgX1/2 + iorgY*uiDIBSourceScanLineSize] & 0xF0) >> 4];
							else				// source pixel is LSNibble
								rgb = m_pColors[lpSrcBits[iorgX1/2 + iorgY*uiDIBSourceScanLineSize] & 0x0F];
							x1y0[2] = rgb.rgbRed;
							x1y0[1] = rgb.rgbGreen;
							x1y0[0] = rgb.rgbBlue;
						}
						else
						{
							// Outside the source -> set to the color passed by parameter
							x1y0[2] = red;
							x1y0[1] = green;
							x1y0[0] = blue;
						}

						// x0y1
						if ((iorgX >= 0) && (iorgY1 >= 0) && (iorgX < (int)nSrcWidth) && (iorgY1 < (int)nSrcHeight))
						{
							// Inside the source bitmap -> copy the bits
							if (NibbleNum == 1) // source pixel is MSNibble
								rgb = m_pColors[(lpSrcBits[iorgX/2 + iorgY1*uiDIBSourceScanLineSize] & 0xF0) >> 4];
							else				// source pixel is LSNibble
								rgb = m_pColors[lpSrcBits[iorgX/2 + iorgY1*uiDIBSourceScanLineSize] & 0x0F];
							x0y1[2] = rgb.rgbRed;
							x0y1[1] = rgb.rgbGreen;
							x0y1[0] = rgb.rgbBlue;
						}
						else
						{
							// Outside the source -> set to the color passed by parameter
							x0y1[2] = red;
							x0y1[1] = green;
							x0y1[0] = blue;
						}

						// x1y1
						if ((iorgX1 >= 0) && (iorgY1 >= 0) && (iorgX1 < (int)nSrcWidth) && (iorgY1 < (int)nSrcHeight))
						{
							// Inside the source bitmap -> copy the bits
							if (NibbleNum == 1) // source pixel is MSNibble
								rgb = m_pColors[(lpSrcBits[iorgX1/2 + iorgY1*uiDIBSourceScanLineSize] & 0xF0) >> 4];
							else				// source pixel is LSNibble
								rgb = m_pColors[lpSrcBits[iorgX1/2 + iorgY1*uiDIBSourceScanLineSize] & 0x0F];
							x1y1[2] = rgb.rgbRed;
							x1y1[1] = rgb.rgbGreen;
							x1y1[0] = rgb.rgbBlue;
						}
						else
						{
							// Outside the source -> set to the color passed by parameter
							x1y1[2] = red;
							x1y1[1] = green;
							x1y1[0] = blue;
						}
						
						R = Round(	x0y0_weight * (double)x0y0[2] + 
									x1y0_weight * (double)x1y0[2] +
									x0y1_weight * (double)x0y1[2] +
									x1y1_weight * (double)x1y1[2]);
						G = Round(	x0y0_weight * (double)x0y0[1] + 
									x1y0_weight * (double)x1y0[1] +
									x0y1_weight * (double)x0y1[1] +
									x1y1_weight * (double)x1y1[1]);
						B = Round(	x0y0_weight * (double)x0y0[0] + 
									x1y0_weight * (double)x1y0[0] +
									x0y1_weight * (double)x0y1[0] +
									x1y1_weight * (double)x1y1[0]);
						int index = GetClosestColorIndex(RGB(R, G, B));
						if (DstNibbleNum == 1) // destination pixel is MSNibble
							lpDstBits[stepX/2] = (BYTE)((lpDstBits[stepX/2] & 0x0F) | ((index & 0x0F) << 4));
						else // destination pixel is LSNibble
							lpDstBits[stepX/2] = (BYTE)((lpDstBits[stepX/2] & 0xF0) | (index & 0x0F));
					}
				}
				lpDstBits += uiDIBTargetScanLineSize;
			}

			break;
		}
		case 8:
		{
			InitGetClosestColorIndex();

			// Step through the destination bitmap
			for (stepY = 0; stepY < nDstHeight; stepY++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, stepY, nDstHeight);

				for (stepX = 0; stepX < nDstWidth; stepX++)
				{
					if (!bAntiAliasing)
					{
						// Transform the destination point back
						// see inverse rotation formula (2)
						orgX = (stepX - nDstOffsetX)*cA + (stepY - nDstOffsetY)*sA;
						orgY = (stepY - nDstOffsetY)*cA - (stepX - nDstOffsetX)*sA;

						// Translate to positive coordinates
						iorgX = (int)(orgX + dSrcOffsetX);
						iorgY = (int)(orgY + dSrcOffsetY);
						
						// Check if it is inside the original source rectangle
						if ((iorgX >= 0) && (iorgY >= 0) && (iorgX < (int)nSrcWidth) && (iorgY < (int)nSrcHeight))
						{
							// Inside the source bitmap -> copy the bits
							lpDstBits[stepX] = lpSrcBits[iorgX + iorgY*uiDIBSourceScanLineSize];
						}
						else
						{
							// Outside the source -> set to the first color in color table
							lpDstBits[stepX] = GetClosestColorIndex(RGB(red, green, blue));
						}
					}
					else
					{
						// Transform the destination point back
						// see inverse rotation formula (2)
						orgX = (stepX + 0.5 - dDstOffsetX)*cA + (stepY + 0.5 - dDstOffsetY)*sA;
						orgY = (stepY + 0.5 - dDstOffsetY)*cA - (stepX + 0.5 - dDstOffsetX)*sA;

						// Translate to positive coordinates
						iorgX1 = Round(orgX + dSrcOffsetX); // Center X coord.of the 4 source pixels
						iorgY1 = Round(orgY + dSrcOffsetY); // Center Y coord.of the 4 source pixels
						iorgX = iorgX1 - 1;
						iorgY = iorgY1 - 1;

						// Error
						errX = orgX + dSrcOffsetX - iorgX1 + 0.5;
						errY = orgY + dSrcOffsetY - iorgY1 + 0.5;

						// Weights
						x0y0_weight = (1.0 - errX) * (1.0 - errY);
						x0y1_weight = (1.0 - errX) * (errY);
						x1y0_weight = (errX) * (1.0 - errY);
						x1y1_weight = (errX) * (errY);

						// x0y0
						if ((iorgX >= 0) && (iorgY >= 0) && (iorgX < (int)nSrcWidth) && (iorgY < (int)nSrcHeight))
						{
							// Inside the source bitmap -> copy the bits
							rgb = m_pColors[lpSrcBits[iorgX + iorgY*uiDIBSourceScanLineSize]];
							x0y0[2] = rgb.rgbRed;
							x0y0[1] = rgb.rgbGreen;
							x0y0[0] = rgb.rgbBlue;
						}
						else
						{
							// Outside the source -> set to the color passed by parameter
							x0y0[2] = red;
							x0y0[1] = green;
							x0y0[0] = blue;
						}

						// x1y0
						if ((iorgX1 >= 0) && (iorgY >= 0) && (iorgX1 < (int)nSrcWidth) && (iorgY < (int)nSrcHeight))
						{
							// Inside the source bitmap -> copy the bits
							rgb = m_pColors[lpSrcBits[iorgX1 + iorgY*uiDIBSourceScanLineSize]];
							x1y0[2] = rgb.rgbRed;
							x1y0[1] = rgb.rgbGreen;
							x1y0[0] = rgb.rgbBlue;
						}
						else
						{
							// Outside the source -> set to the color passed by parameter
							x1y0[2] = red;
							x1y0[1] = green;
							x1y0[0] = blue;
						}

						// x0y1
						if ((iorgX >= 0) && (iorgY1 >= 0) && (iorgX < (int)nSrcWidth) && (iorgY1 < (int)nSrcHeight))
						{
							// Inside the source bitmap -> copy the bits
							rgb = m_pColors[lpSrcBits[iorgX + iorgY1*uiDIBSourceScanLineSize]];
							x0y1[2] = rgb.rgbRed;
							x0y1[1] = rgb.rgbGreen;
							x0y1[0] = rgb.rgbBlue;
						}
						else
						{
							// Outside the source -> set to the color passed by parameter
							x0y1[2] = red;
							x0y1[1] = green;
							x0y1[0] = blue;
						}

						// x1y1
						if ((iorgX1 >= 0) && (iorgY1 >= 0) && (iorgX1 < (int)nSrcWidth) && (iorgY1 < (int)nSrcHeight))
						{
							// Inside the source bitmap -> copy the bits
							rgb = m_pColors[lpSrcBits[iorgX1 + iorgY1*uiDIBSourceScanLineSize]];
							x1y1[2] = rgb.rgbRed;
							x1y1[1] = rgb.rgbGreen;
							x1y1[0] = rgb.rgbBlue;
						}
						else
						{
							// Outside the source -> set to the color passed by parameter
							x1y1[2] = red;
							x1y1[1] = green;
							x1y1[0] = blue;
						}
						
						R = Round(	x0y0_weight * (double)x0y0[2] + 
									x1y0_weight * (double)x1y0[2] +
									x0y1_weight * (double)x0y1[2] +
									x1y1_weight * (double)x1y1[2]);
						G = Round(	x0y0_weight * (double)x0y0[1] + 
									x1y0_weight * (double)x1y0[1] +
									x0y1_weight * (double)x0y1[1] +
									x1y1_weight * (double)x1y1[1]);
						B = Round(	x0y0_weight * (double)x0y0[0] + 
									x1y0_weight * (double)x1y0[0] +
									x0y1_weight * (double)x0y1[0] +
									x1y1_weight * (double)x1y1[0]);
						lpDstBits[stepX] = GetClosestColorIndex(RGB(R, G, B));
					}
				}
				lpDstBits += uiDIBTargetScanLineSize;
			}
			break;
		}
		case 16:
		{
			// Step through the destination bitmap
			for (stepY = 0; stepY < nDstHeight; stepY++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, stepY, nDstHeight);

				for (stepX = 0; stepX < nDstWidth; stepX++)
				{
					if (!bAntiAliasing)
					{
						// Transform the destination point back
						// see inverse rotation formula (2)
						orgX = (stepX - nDstOffsetX)*cA + (stepY - nDstOffsetY)*sA;
						orgY = (stepY - nDstOffsetY)*cA - (stepX - nDstOffsetX)*sA;

						// Translate to positive coordinates
						iorgX = (int)(orgX + dSrcOffsetX);
						iorgY = (int)(orgY + dSrcOffsetY);
						
						// Check if it is inside the original source rectangle
						if ((iorgX >= 0) && (iorgY >= 0) && (iorgX < (int)nSrcWidth) && (iorgY < (int)nSrcHeight))
						{
							// Inside the source bitmap -> copy the bits
							lpDstBits[2*stepX + 1] = lpSrcBits[2*iorgX + iorgY*uiDIBSourceScanLineSize + 1];
							lpDstBits[2*stepX] = lpSrcBits[2*iorgX + iorgY*uiDIBSourceScanLineSize];
						}
						else
						{
							// Outside the source -> set to the color passed by parameter
							((WORD*)lpDstBits)[stepX] = RGBToDIB16(red, green, blue);
						}
					}
					else
					{
						// Transform the destination point back
						// see inverse rotation formula (2)
						orgX = (stepX + 0.5 - dDstOffsetX)*cA + (stepY + 0.5 - dDstOffsetY)*sA;
						orgY = (stepY + 0.5 - dDstOffsetY)*cA - (stepX + 0.5 - dDstOffsetX)*sA;

						// Translate to positive coordinates
						iorgX1 = Round(orgX + dSrcOffsetX); // Center X coord.of the 4 source pixels
						iorgY1 = Round(orgY + dSrcOffsetY); // Center Y coord.of the 4 source pixels
						iorgX = iorgX1 - 1;
						iorgY = iorgY1 - 1;

						// Error
						errX = orgX + dSrcOffsetX - iorgX1 + 0.5;
						errY = orgY + dSrcOffsetY - iorgY1 + 0.5;

						// Weights
						x0y0_weight = (1.0 - errX) * (1.0 - errY);
						x0y1_weight = (1.0 - errX) * (errY);
						x1y0_weight = (errX) * (1.0 - errY);
						x1y1_weight = (errX) * (errY);

						// x0y0
						if ((iorgX >= 0) && (iorgY >= 0) && (iorgX < (int)nSrcWidth) && (iorgY < (int)nSrcHeight))
						{
							// Inside the source bitmap -> copy the bits
							DIB16ToRGB(((WORD*)lpSrcBits)[iorgX + iorgY * (uiDIBSourceScanLineSize / 2)], &R, &G, &B);
							x0y0[2] = R;
							x0y0[1] = G;
							x0y0[0] = B;
						}
						else
						{
							// Outside the source -> set to the color passed by parameter
							x0y0[2] = red;
							x0y0[1] = green;
							x0y0[0] = blue;
						}

						// x1y0
						if ((iorgX1 >= 0) && (iorgY >= 0) && (iorgX1 < (int)nSrcWidth) && (iorgY < (int)nSrcHeight))
						{
							// Inside the source bitmap -> copy the bits
							DIB16ToRGB(((WORD*)lpSrcBits)[iorgX1 + iorgY * (uiDIBSourceScanLineSize / 2)], &R, &G, &B);
							x1y0[2] = R;
							x1y0[1] = G;
							x1y0[0] = B;
						}
						else
						{
							// Outside the source -> set to the color passed by parameter
							x1y0[2] = red;
							x1y0[1] = green;
							x1y0[0] = blue;
						}

						// x0y1
						if ((iorgX >= 0) && (iorgY1 >= 0) && (iorgX < (int)nSrcWidth) && (iorgY1 < (int)nSrcHeight))
						{
							// Inside the source bitmap -> copy the bits
							DIB16ToRGB(((WORD*)lpSrcBits)[iorgX + iorgY1 * (uiDIBSourceScanLineSize / 2)], &R, &G, &B);
							x0y1[2] = R;
							x0y1[1] = G;
							x0y1[0] = B;
						}
						else
						{
							// Outside the source -> set to the color passed by parameter
							x0y1[2] = red;
							x0y1[1] = green;
							x0y1[0] = blue;
						}

						// x1y1
						if ((iorgX1 >= 0) && (iorgY1 >= 0) && (iorgX1 < (int)nSrcWidth) && (iorgY1 < (int)nSrcHeight))
						{
							// Inside the source bitmap -> copy the bits
							DIB16ToRGB(((WORD*)lpSrcBits)[iorgX1 + iorgY1 * (uiDIBSourceScanLineSize / 2)], &R, &G, &B);
							x1y1[2] = R;
							x1y1[1] = G;
							x1y1[0] = B;
						}
						else
						{
							// Outside the source -> set to the color passed by parameter
							x1y1[2] = red;
							x1y1[1] = green;
							x1y1[0] = blue;
						}
						
						R = Round(	x0y0_weight * (double)x0y0[2] + 
									x1y0_weight * (double)x1y0[2] +
									x0y1_weight * (double)x0y1[2] +
									x1y1_weight * (double)x1y1[2]);
						G = Round(	x0y0_weight * (double)x0y0[1] + 
									x1y0_weight * (double)x1y0[1] +
									x0y1_weight * (double)x0y1[1] +
									x1y1_weight * (double)x1y1[1]);
						B = Round(	x0y0_weight * (double)x0y0[0] + 
									x1y0_weight * (double)x1y0[0] +
									x0y1_weight * (double)x0y1[0] +
									x1y1_weight * (double)x1y1[0]);
						((WORD*)lpDstBits)[stepX] = RGBToDIB16(R, G, B);
					}
				}
				lpDstBits += uiDIBTargetScanLineSize;
			}
			break;
		}
		case 24:
		{
			// Step through the destination rectangle
			for (stepY = 0; stepY < nDstHeight; stepY++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, stepY, nDstHeight);

				for (stepX = 0; stepX < nDstWidth; stepX++)
				{
					if (!bAntiAliasing)
					{
						// Transform the destination point back
						// see inverse rotation formula (2)
						orgX = (stepX - nDstOffsetX)*cA + (stepY - nDstOffsetY)*sA;
						orgY = (stepY - nDstOffsetY)*cA - (stepX - nDstOffsetX)*sA;

						// Translate to positive coordinates
						iorgX = (int)(orgX + dSrcOffsetX);
						iorgY = (int)(orgY + dSrcOffsetY);
						
						// Check if it is inside the original source rectangle
						if ((iorgX >= 0) && (iorgY >= 0) && (iorgX < (int)nSrcWidth) && (iorgY < (int)nSrcHeight))
						{
							// Inside the source bitmap -> copy the bits
							lpDstBits[3*stepX + 2] = lpSrcBits[3*iorgX + iorgY*uiDIBSourceScanLineSize + 2];
							lpDstBits[3*stepX + 1] = lpSrcBits[3*iorgX + iorgY*uiDIBSourceScanLineSize + 1];
							lpDstBits[3*stepX] = lpSrcBits[3*iorgX + iorgY*uiDIBSourceScanLineSize];
						}
						else
						{
							// Outside the source -> set to the color passed by parameter
							lpDstBits[3*stepX + 2] = red;
							lpDstBits[3*stepX + 1] = green;
							lpDstBits[3*stepX] = blue;
						}
					}
					else
					{
						// Transform the destination point back
						// see inverse rotation formula (2)
						orgX = (stepX + 0.5 - dDstOffsetX)*cA + (stepY + 0.5 - dDstOffsetY)*sA;
						orgY = (stepY + 0.5 - dDstOffsetY)*cA - (stepX + 0.5 - dDstOffsetX)*sA;

						// Translate to positive coordinates
						iorgX1 = Round(orgX + dSrcOffsetX); // Center X coord.of the 4 source pixels
						iorgY1 = Round(orgY + dSrcOffsetY); // Center Y coord.of the 4 source pixels
						iorgX = iorgX1 - 1;
						iorgY = iorgY1 - 1;

						// Error
						errX = orgX + dSrcOffsetX - iorgX1 + 0.5;
						errY = orgY + dSrcOffsetY - iorgY1 + 0.5;

						// Weights
						x0y0_weight = (1.0 - errX) * (1.0 - errY);
						x0y1_weight = (1.0 - errX) * (errY);
						x1y0_weight = (errX) * (1.0 - errY);
						x1y1_weight = (errX) * (errY);
						
						// x0y0
						if ((iorgX >= 0) && (iorgY >= 0) && (iorgX < (int)nSrcWidth) && (iorgY < (int)nSrcHeight))
						{
							// Inside the source bitmap -> copy the bits
							x0y0[2] = lpSrcBits[3*iorgX + iorgY*uiDIBSourceScanLineSize + 2];
							x0y0[1] = lpSrcBits[3*iorgX + iorgY*uiDIBSourceScanLineSize + 1];
							x0y0[0] = lpSrcBits[3*iorgX + iorgY*uiDIBSourceScanLineSize];
						}
						else
						{
							// Outside the source -> set to the color passed by parameter
							x0y0[2] = red;
							x0y0[1] = green;
							x0y0[0] = blue;
						}

						// x1y0
						if ((iorgX1 >= 0) && (iorgY >= 0) && (iorgX1 < (int)nSrcWidth) && (iorgY < (int)nSrcHeight))
						{
							// Inside the source bitmap -> copy the bits
							x1y0[2] = lpSrcBits[3*iorgX1 + iorgY*uiDIBSourceScanLineSize + 2];
							x1y0[1] = lpSrcBits[3*iorgX1 + iorgY*uiDIBSourceScanLineSize + 1];
							x1y0[0] = lpSrcBits[3*iorgX1 + iorgY*uiDIBSourceScanLineSize];
						}
						else
						{
							// Outside the source -> set to the color passed by parameter
							x1y0[2] = red;
							x1y0[1] = green;
							x1y0[0] = blue;
						}

						// x0y1
						if ((iorgX >= 0) && (iorgY1 >= 0) && (iorgX < (int)nSrcWidth) && (iorgY1 < (int)nSrcHeight))
						{
							// Inside the source bitmap -> copy the bits
							x0y1[2] = lpSrcBits[3*iorgX + iorgY1*uiDIBSourceScanLineSize + 2];
							x0y1[1] = lpSrcBits[3*iorgX + iorgY1*uiDIBSourceScanLineSize + 1];
							x0y1[0] = lpSrcBits[3*iorgX + iorgY1*uiDIBSourceScanLineSize];
						}
						else
						{
							// Outside the source -> set to the color passed by parameter
							x0y1[2] = red;
							x0y1[1] = green;
							x0y1[0] = blue;
						}

						// x1y1
						if ((iorgX1 >= 0) && (iorgY1 >= 0) && (iorgX1 < (int)nSrcWidth) && (iorgY1 < (int)nSrcHeight))
						{
							// Inside the source bitmap -> copy the bits
							x1y1[2] = lpSrcBits[3*iorgX1 + iorgY1*uiDIBSourceScanLineSize + 2];
							x1y1[1] = lpSrcBits[3*iorgX1 + iorgY1*uiDIBSourceScanLineSize + 1];
							x1y1[0] = lpSrcBits[3*iorgX1 + iorgY1*uiDIBSourceScanLineSize];
						}
						else
						{
							// Outside the source -> set to the color passed by parameter
							x1y1[2] = red;
							x1y1[1] = green;
							x1y1[0] = blue;
						}

						lpDstBits[3*stepX + 2] = Round(	x0y0_weight * (double)x0y0[2] + 
														x1y0_weight * (double)x1y0[2] +
														x0y1_weight * (double)x0y1[2] +
														x1y1_weight * (double)x1y1[2]);
						lpDstBits[3*stepX + 1] = Round(	x0y0_weight * (double)x0y0[1] + 
														x1y0_weight * (double)x1y0[1] +
														x0y1_weight * (double)x0y1[1] +
														x1y1_weight * (double)x1y1[1]);
						lpDstBits[3*stepX]     = Round(	x0y0_weight * (double)x0y0[0] + 
														x1y0_weight * (double)x1y0[0] +
														x0y1_weight * (double)x0y1[0] +
														x1y1_weight * (double)x1y1[0]);
					}
				}
				lpDstBits += uiDIBTargetScanLineSize;
			}
			break;
		}
		case 32:
		{
			if (pSrcDib->HasAlpha())
			{
				// Step through the destination rectangle
				for (stepY = 0; stepY < nDstHeight; stepY++)
				{
					DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, stepY, nDstHeight);

					for (stepX = 0; stepX < nDstWidth; stepX++)
					{
						if (!bAntiAliasing)
						{
							// Transform the destination point back
							// see inverse rotation formula (2)
							orgX = (stepX - nDstOffsetX)*cA + (stepY - nDstOffsetY)*sA;
							orgY = (stepY - nDstOffsetY)*cA - (stepX - nDstOffsetX)*sA;

							// Translate to positive coordinates
							iorgX = (int)(orgX + dSrcOffsetX);
							iorgY = (int)(orgY + dSrcOffsetY);
							
							// Check if it is inside the original source rectangle
							if ((iorgX >= 0) && (iorgY >= 0) && (iorgX < (int)nSrcWidth) && (iorgY < (int)nSrcHeight))
							{
								// Inside the source bitmap -> copy the bits
								lpDstBits[4*stepX + 3] = lpSrcBits[4*iorgX + iorgY*uiDIBSourceScanLineSize + 3];
								lpDstBits[4*stepX + 2] = lpSrcBits[4*iorgX + iorgY*uiDIBSourceScanLineSize + 2];
								lpDstBits[4*stepX + 1] = lpSrcBits[4*iorgX + iorgY*uiDIBSourceScanLineSize + 1];
								lpDstBits[4*stepX] = lpSrcBits[4*iorgX + iorgY*uiDIBSourceScanLineSize];
							}
							else
							{
								// Outside the source -> set to the color passed by parameter
								lpDstBits[4*stepX + 3] = alpha;
								lpDstBits[4*stepX + 2] = red;
								lpDstBits[4*stepX + 1] = green;
								lpDstBits[4*stepX] = blue;
							}
						}
						else
						{
							// Transform the destination point back
							// see inverse rotation formula (2)
							orgX = (stepX + 0.5 - dDstOffsetX)*cA + (stepY + 0.5 - dDstOffsetY)*sA;
							orgY = (stepY + 0.5 - dDstOffsetY)*cA - (stepX + 0.5 - dDstOffsetX)*sA;

							// Translate to positive coordinates
							iorgX1 = Round(orgX + dSrcOffsetX); // Center X coord.of the 4 source pixels
							iorgY1 = Round(orgY + dSrcOffsetY); // Center Y coord.of the 4 source pixels
							iorgX = iorgX1 - 1;
							iorgY = iorgY1 - 1;

							// Error
							errX = orgX + dSrcOffsetX - iorgX1 + 0.5;
							errY = orgY + dSrcOffsetY - iorgY1 + 0.5;

							// Weights
							x0y0_weight = (1.0 - errX) * (1.0 - errY);
							x0y1_weight = (1.0 - errX) * (errY);
							x1y0_weight = (errX) * (1.0 - errY);
							x1y1_weight = (errX) * (errY);
							
							// x0y0
							if ((iorgX >= 0) && (iorgY >= 0) && (iorgX < (int)nSrcWidth) && (iorgY < (int)nSrcHeight))
							{
								// Inside the source bitmap -> copy the bits
								x0y0[3] = lpSrcBits[4*iorgX + iorgY*uiDIBSourceScanLineSize + 3];
								x0y0[2] = lpSrcBits[4*iorgX + iorgY*uiDIBSourceScanLineSize + 2];
								x0y0[1] = lpSrcBits[4*iorgX + iorgY*uiDIBSourceScanLineSize + 1];
								x0y0[0] = lpSrcBits[4*iorgX + iorgY*uiDIBSourceScanLineSize];
							}
							else
							{
								// Outside the source -> set to the color passed by parameter
								x0y0[3] = alpha;
								x0y0[2] = red;
								x0y0[1] = green;
								x0y0[0] = blue;
							}

							// x1y0
							if ((iorgX1 >= 0) && (iorgY >= 0) && (iorgX1 < (int)nSrcWidth) && (iorgY < (int)nSrcHeight))
							{
								// Inside the source bitmap -> copy the bits
								x1y0[3] = lpSrcBits[4*iorgX1 + iorgY*uiDIBSourceScanLineSize + 3];
								x1y0[2] = lpSrcBits[4*iorgX1 + iorgY*uiDIBSourceScanLineSize + 2];
								x1y0[1] = lpSrcBits[4*iorgX1 + iorgY*uiDIBSourceScanLineSize + 1];
								x1y0[0] = lpSrcBits[4*iorgX1 + iorgY*uiDIBSourceScanLineSize];
							}
							else
							{
								// Outside the source -> set to the color passed by parameter
								x1y0[3] = alpha;
								x1y0[2] = red;
								x1y0[1] = green;
								x1y0[0] = blue;
							}

							// x0y1
							if ((iorgX >= 0) && (iorgY1 >= 0) && (iorgX < (int)nSrcWidth) && (iorgY1 < (int)nSrcHeight))
							{
								// Inside the source bitmap -> copy the bits							
								x0y1[3] = lpSrcBits[4*iorgX + iorgY1*uiDIBSourceScanLineSize + 3];
								x0y1[2] = lpSrcBits[4*iorgX + iorgY1*uiDIBSourceScanLineSize + 2];
								x0y1[1] = lpSrcBits[4*iorgX + iorgY1*uiDIBSourceScanLineSize + 1];
								x0y1[0] = lpSrcBits[4*iorgX + iorgY1*uiDIBSourceScanLineSize];
							}
							else
							{
								// Outside the source -> set to the color passed by parameter
								x0y1[3] = alpha;
								x0y1[2] = red;
								x0y1[1] = green;
								x0y1[0] = blue;
							}

							// x1y1
							if ((iorgX1 >= 0) && (iorgY1 >= 0) && (iorgX1 < (int)nSrcWidth) && (iorgY1 < (int)nSrcHeight))
							{
								// Inside the source bitmap -> copy the bits
								x1y1[3] = lpSrcBits[4*iorgX1 + iorgY1*uiDIBSourceScanLineSize + 3];
								x1y1[2] = lpSrcBits[4*iorgX1 + iorgY1*uiDIBSourceScanLineSize + 2];
								x1y1[1] = lpSrcBits[4*iorgX1 + iorgY1*uiDIBSourceScanLineSize + 1];
								x1y1[0] = lpSrcBits[4*iorgX1 + iorgY1*uiDIBSourceScanLineSize];
							}
							else
							{
								// Outside the source -> set to the color passed by parameter
								x1y1[3] = alpha;
								x1y1[2] = red;
								x1y1[1] = green;
								x1y1[0] = blue;
							}

							lpDstBits[4*stepX + 3] = Round(	x0y0_weight * (double)x0y0[3] + 
															x1y0_weight * (double)x1y0[3] +
															x0y1_weight * (double)x0y1[3] +
															x1y1_weight * (double)x1y1[3]);
							lpDstBits[4*stepX + 2] = Round(	x0y0_weight * (double)x0y0[2] + 
															x1y0_weight * (double)x1y0[2] +
															x0y1_weight * (double)x0y1[2] +
															x1y1_weight * (double)x1y1[2]);
							lpDstBits[4*stepX + 1] = Round(	x0y0_weight * (double)x0y0[1] + 
															x1y0_weight * (double)x1y0[1] +
															x0y1_weight * (double)x0y1[1] +
															x1y1_weight * (double)x1y1[1]);
							lpDstBits[4*stepX]     = Round(	x0y0_weight * (double)x0y0[0] + 
															x1y0_weight * (double)x1y0[0] +
															x0y1_weight * (double)x0y1[0] +
															x1y1_weight * (double)x1y1[0]);
						}
					}
					lpDstBits += uiDIBTargetScanLineSize;
				}
			}
			else if (pSrcDib->IsFast32bpp())
			{
				// Step through the destination rectangle
				for (stepY = 0; stepY < nDstHeight; stepY++)
				{
					DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, stepY, nDstHeight);

					for (stepX = 0; stepX < nDstWidth; stepX++)
					{
						if (!bAntiAliasing)
						{
							// Transform the destination point back
							// see inverse rotation formula (2)
							orgX = (stepX - nDstOffsetX)*cA + (stepY - nDstOffsetY)*sA;
							orgY = (stepY - nDstOffsetY)*cA - (stepX - nDstOffsetX)*sA;

							// Translate to positive coordinates
							iorgX = (int)(orgX + dSrcOffsetX);
							iorgY = (int)(orgY + dSrcOffsetY);
							
							// Check if it is inside the original source rectangle
							if ((iorgX >= 0) && (iorgY >= 0) && (iorgX < (int)nSrcWidth) && (iorgY < (int)nSrcHeight))
							{
								// Inside the source bitmap -> copy the bits
								lpDstBits[4*stepX + 2] = lpSrcBits[4*iorgX + iorgY*uiDIBSourceScanLineSize + 2];
								lpDstBits[4*stepX + 1] = lpSrcBits[4*iorgX + iorgY*uiDIBSourceScanLineSize + 1];
								lpDstBits[4*stepX] = lpSrcBits[4*iorgX + iorgY*uiDIBSourceScanLineSize];
							}
							else
							{
								// Outside the source -> set to the color passed by parameter
								lpDstBits[4*stepX + 2] = red;
								lpDstBits[4*stepX + 1] = green;
								lpDstBits[4*stepX] = blue;
							}
						}
						else
						{
							// Transform the destination point back
							// see inverse rotation formula (2)
							orgX = (stepX + 0.5 - dDstOffsetX)*cA + (stepY + 0.5 - dDstOffsetY)*sA;
							orgY = (stepY + 0.5 - dDstOffsetY)*cA - (stepX + 0.5 - dDstOffsetX)*sA;

							// Translate to positive coordinates
							iorgX1 = Round(orgX + dSrcOffsetX); // Center X coord.of the 4 source pixels
							iorgY1 = Round(orgY + dSrcOffsetY); // Center Y coord.of the 4 source pixels
							iorgX = iorgX1 - 1;
							iorgY = iorgY1 - 1;

							// Error
							errX = orgX + dSrcOffsetX - iorgX1 + 0.5;
							errY = orgY + dSrcOffsetY - iorgY1 + 0.5;

							// Weights
							x0y0_weight = (1.0 - errX) * (1.0 - errY);
							x0y1_weight = (1.0 - errX) * (errY);
							x1y0_weight = (errX) * (1.0 - errY);
							x1y1_weight = (errX) * (errY);
							
							// x0y0
							if ((iorgX >= 0) && (iorgY >= 0) && (iorgX < (int)nSrcWidth) && (iorgY < (int)nSrcHeight))
							{
								// Inside the source bitmap -> copy the bits
								x0y0[2] = lpSrcBits[4*iorgX + iorgY*uiDIBSourceScanLineSize + 2];
								x0y0[1] = lpSrcBits[4*iorgX + iorgY*uiDIBSourceScanLineSize + 1];
								x0y0[0] = lpSrcBits[4*iorgX + iorgY*uiDIBSourceScanLineSize];
							}
							else
							{
								// Outside the source -> set to the color passed by parameter
								x0y0[2] = red;
								x0y0[1] = green;
								x0y0[0] = blue;
							}

							// x1y0
							if ((iorgX1 >= 0) && (iorgY >= 0) && (iorgX1 < (int)nSrcWidth) && (iorgY < (int)nSrcHeight))
							{
								// Inside the source bitmap -> copy the bits
								x1y0[2] = lpSrcBits[4*iorgX1 + iorgY*uiDIBSourceScanLineSize + 2];
								x1y0[1] = lpSrcBits[4*iorgX1 + iorgY*uiDIBSourceScanLineSize + 1];
								x1y0[0] = lpSrcBits[4*iorgX1 + iorgY*uiDIBSourceScanLineSize];
							}
							else
							{
								// Outside the source -> set to the color passed by parameter
								x1y0[2] = red;
								x1y0[1] = green;
								x1y0[0] = blue;
							}

							// x0y1
							if ((iorgX >= 0) && (iorgY1 >= 0) && (iorgX < (int)nSrcWidth) && (iorgY1 < (int)nSrcHeight))
							{
								// Inside the source bitmap -> copy the bits							
								x0y1[2] = lpSrcBits[4*iorgX + iorgY1*uiDIBSourceScanLineSize + 2];
								x0y1[1] = lpSrcBits[4*iorgX + iorgY1*uiDIBSourceScanLineSize + 1];
								x0y1[0] = lpSrcBits[4*iorgX + iorgY1*uiDIBSourceScanLineSize];
							}
							else
							{
								// Outside the source -> set to the color passed by parameter
								x0y1[2] = red;
								x0y1[1] = green;
								x0y1[0] = blue;
							}

							// x1y1
							if ((iorgX1 >= 0) && (iorgY1 >= 0) && (iorgX1 < (int)nSrcWidth) && (iorgY1 < (int)nSrcHeight))
							{
								// Inside the source bitmap -> copy the bits
								x1y1[2] = lpSrcBits[4*iorgX1 + iorgY1*uiDIBSourceScanLineSize + 2];
								x1y1[1] = lpSrcBits[4*iorgX1 + iorgY1*uiDIBSourceScanLineSize + 1];
								x1y1[0] = lpSrcBits[4*iorgX1 + iorgY1*uiDIBSourceScanLineSize];
							}
							else
							{
								// Outside the source -> set to the color passed by parameter
								x1y1[2] = red;
								x1y1[1] = green;
								x1y1[0] = blue;
							}

							lpDstBits[4*stepX + 2] = Round(	x0y0_weight * (double)x0y0[2] + 
															x1y0_weight * (double)x1y0[2] +
															x0y1_weight * (double)x0y1[2] +
															x1y1_weight * (double)x1y1[2]);
							lpDstBits[4*stepX + 1] = Round(	x0y0_weight * (double)x0y0[1] + 
															x1y0_weight * (double)x1y0[1] +
															x0y1_weight * (double)x0y1[1] +
															x1y1_weight * (double)x1y1[1]);
							lpDstBits[4*stepX]     = Round(	x0y0_weight * (double)x0y0[0] + 
															x1y0_weight * (double)x1y0[0] +
															x0y1_weight * (double)x0y1[0] +
															x1y1_weight * (double)x1y1[0]);
						}
					}
					lpDstBits += uiDIBTargetScanLineSize;
				}
			}
			else
			{
				// Step through the destination rectangle
				for (stepY = 0; stepY < nDstHeight; stepY++)
				{
					DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, stepY, nDstHeight);

					for (stepX = 0; stepX < nDstWidth; stepX++)
					{
						if (!bAntiAliasing)
						{
							// Transform the destination point back
							// see inverse rotation formula (2)
							orgX = (stepX - nDstOffsetX)*cA + (stepY - nDstOffsetY)*sA;
							orgY = (stepY - nDstOffsetY)*cA - (stepX - nDstOffsetX)*sA;

							// Translate to positive coordinates
							iorgX = (int)(orgX + dSrcOffsetX);
							iorgY = (int)(orgY + dSrcOffsetY);
							
							// Check if it is inside the original source rectangle
							if ((iorgX >= 0) && (iorgY >= 0) && (iorgX < (int)nSrcWidth) && (iorgY < (int)nSrcHeight))
							{
								// Inside the source bitmap -> copy the bits
								((DWORD*)lpDstBits)[stepX] = ((DWORD*)lpSrcBits)[iorgX + iorgY * nSrcWidth];
							}
							else
							{
								// Outside the source -> set to the color passed by parameter
								((DWORD*)lpDstBits)[stepX] = RGBToDIB32(red, green, blue);
							}
						}
						else
						{
							// Transform the destination point back
							// see inverse rotation formula (2)
							orgX = (stepX + 0.5 - dDstOffsetX)*cA + (stepY + 0.5 - dDstOffsetY)*sA;
							orgY = (stepY + 0.5 - dDstOffsetY)*cA - (stepX + 0.5 - dDstOffsetX)*sA;

							// Translate to positive coordinates
							iorgX1 = Round(orgX + dSrcOffsetX); // Center X coord.of the 4 source pixels
							iorgY1 = Round(orgY + dSrcOffsetY); // Center Y coord.of the 4 source pixels
							iorgX = iorgX1 - 1;
							iorgY = iorgY1 - 1;

							// Error
							errX = orgX + dSrcOffsetX - iorgX1 + 0.5;
							errY = orgY + dSrcOffsetY - iorgY1 + 0.5;

							// Weights
							x0y0_weight = (1.0 - errX) * (1.0 - errY);
							x0y1_weight = (1.0 - errX) * (errY);
							x1y0_weight = (errX) * (1.0 - errY);
							x1y1_weight = (errX) * (errY);

							// x0y0
							if ((iorgX >= 0) && (iorgY >= 0) && (iorgX < (int)nSrcWidth) && (iorgY < (int)nSrcHeight))
							{
								// Inside the source bitmap -> copy the bits
								DIB32ToRGB(((DWORD*)lpSrcBits)[iorgX + iorgY * nSrcWidth], &R, &G, &B);
								x0y0[2] = R;
								x0y0[1] = G;
								x0y0[0] = B;
							}
							else
							{
								// Outside the source -> set to the color passed by parameter
								x0y0[2] = red;
								x0y0[1] = green;
								x0y0[0] = blue;
							}

							// x1y0
							if ((iorgX1 >= 0) && (iorgY >= 0) && (iorgX1 < (int)nSrcWidth) && (iorgY < (int)nSrcHeight))
							{
								// Inside the source bitmap -> copy the bits
								DIB32ToRGB(((DWORD*)lpSrcBits)[iorgX1 + iorgY * nSrcWidth], &R, &G, &B);
								x1y0[2] = R;
								x1y0[1] = G;
								x1y0[0] = B;
							}
							else
							{
								// Outside the source -> set to the color passed by parameter
								x1y0[2] = red;
								x1y0[1] = green;
								x1y0[0] = blue;
							}

							// x0y1
							if ((iorgX >= 0) && (iorgY1 >= 0) && (iorgX < (int)nSrcWidth) && (iorgY1 < (int)nSrcHeight))
							{
								// Inside the source bitmap -> copy the bits
								DIB32ToRGB(((DWORD*)lpSrcBits)[iorgX + iorgY1 * nSrcWidth], &R, &G, &B);
								x0y1[2] = R;
								x0y1[1] = G;
								x0y1[0] = B;
							}
							else
							{
								// Outside the source -> set to the color passed by parameter
								x0y1[2] = red;
								x0y1[1] = green;
								x0y1[0] = blue;
							}

							// x1y1
							if ((iorgX1 >= 0) && (iorgY1 >= 0) && (iorgX1 < (int)nSrcWidth) && (iorgY1 < (int)nSrcHeight))
							{
								// Inside the source bitmap -> copy the bits
								DIB32ToRGB(((DWORD*)lpSrcBits)[iorgX1 + iorgY1 * nSrcWidth], &R, &G, &B);
								x1y1[2] = R;
								x1y1[1] = G;
								x1y1[0] = B;
							}
							else
							{
								// Outside the source -> set to the color passed by parameter
								x1y1[2] = red;
								x1y1[1] = green;
								x1y1[0] = blue;
							}
							
							R = Round(	x0y0_weight * (double)x0y0[2] + 
										x1y0_weight * (double)x1y0[2] +
										x0y1_weight * (double)x0y1[2] +
										x1y1_weight * (double)x1y1[2]);
							G = Round(	x0y0_weight * (double)x0y0[1] + 
										x1y0_weight * (double)x1y0[1] +
										x0y1_weight * (double)x0y1[1] +
										x1y1_weight * (double)x1y1[1]);
							B = Round(	x0y0_weight * (double)x0y0[0] + 
										x1y0_weight * (double)x1y0[0] +
										x0y1_weight * (double)x0y1[0] +
										x1y1_weight * (double)x1y1[0]);
							((DWORD*)lpDstBits)[stepX] = RGBToDIB32(R, G, B);
						}
					}
					lpDstBits += uiDIBTargetScanLineSize;
				}
			}
			break;
		}
		default:
			
			break;
	}

	// Create Palette
	CreatePaletteFromBMI();

	DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

	return TRUE;
}