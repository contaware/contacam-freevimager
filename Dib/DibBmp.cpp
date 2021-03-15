#include "stdafx.h"
#include "dib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BOOL CDib::LoadBMP(	LPCTSTR lpszPathName,
					BOOL bOnlyHeader/*=FALSE*/,
					BOOL bDecompress/*=TRUE*/,
					CWnd* pProgressWnd/*=NULL*/,
					BOOL bProgressSend/*=TRUE*/,
					CWorkerThread* pThread/*=NULL*/)
{
	try
	{	
		CString sPathName(lpszPathName);
		if (sPathName.IsEmpty())
			throw (int)BMP_E_ZEROPATH;
			
		CFile file(lpszPathName, CFile::modeRead | CFile::shareDenyNone);
		if (file.GetLength() == 0)
			throw (int)BMP_E_FILEEMPTY;

		BOOL res = LoadBMP(	file,
							bOnlyHeader,
							bDecompress,
							pProgressWnd,
							bProgressSend,
							pThread);

		file.Close();

		return res;
	}
	catch (CFileException* e)
	{
		TCHAR szCause[255];
		CString str;
#ifdef _DEBUG
		str = _T("LoadBMP: ");
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
		str.Format(_T("LoadBMP(%s):\n"), lpszPathName);
#endif
		switch (error_code)
		{
			case BMP_E_ZEROPATH :		str += _T("The file name is zero\n");
			break;
			case BMP_E_FILEEMPTY :		str += _T("File is empty\n");
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

BOOL CDib::SaveBMP(	LPCTSTR lpszPathName,
					CWnd* pProgressWnd/*=NULL*/,
					BOOL bProgressSend/*=TRUE*/,
					CWorkerThread* pThread/*=NULL*/)
{
	try
	{
		CString sPathName(lpszPathName);
		if (sPathName.IsEmpty())
			throw (int)BMP_E_ZEROPATH;

		DWORD dwAttrib = ::GetFileAttributes(lpszPathName);
		if ((dwAttrib != 0xFFFFFFFF) && (dwAttrib & FILE_ATTRIBUTE_READONLY))
			throw (int)BMP_E_FILEREADONLY;

		if (!m_pBits)
		{
			if (!DibSectionToBits())
				throw (int)BMP_E_BADBMP;
		}

		if (!m_pBits || !m_pBMI)
			throw (int)BMP_E_BADBMP;

		CFile file(lpszPathName,	CFile::modeCreate |
									CFile::modeWrite  |
									CFile::shareExclusive);

		BOOL res = SaveBMP(	file,
							pProgressWnd,
							bProgressSend,
							pThread);

		file.Close();

		return res;
	}
	catch (CFileException* e)
	{
		TCHAR szCause[255];
		CString str;
#ifdef _DEBUG
		str = _T("SaveBMP: ");
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
		str.Format(_T("SaveBMP(%s):\n"), lpszPathName);
#endif
		switch (error_code)
		{
			case BMP_E_ZEROPATH :		str += _T("The file name is zero\n");
			break;
			case BMP_E_FILEREADONLY :	str += _T("The file is read only\n");
			break;
			case BMP_E_BADBMP :			str += _T("Corrupted or unsupported DIB\n");
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

BOOL CDib::LoadBMPNoFileHeader(CFile& file, BOOL bDecompress/*=TRUE*/)
{	
	try
	{
		BITMAPINFOHEADER bmiheader;

		// Check File Size
		if (file.GetLength() == 0)
			throw (int)BMP_E_FILEEMPTY;

		// Read Bit Map Info Header
		if (file.Read(&bmiheader, sizeof(BITMAPINFOHEADER)) != (UINT)(sizeof(BITMAPINFOHEADER)))
			throw (int)BMP_E_READ;
		DWORD dwReadBytes = sizeof(BITMAPINFOHEADER);

		// Free
		m_FileInfo.Clear();
		m_bAlpha = FALSE;
		m_bGrayscale = FALSE;
		ResetColorUndo();
		if (m_hDibSection)
		{
			::DeleteObject(m_hDibSection);
			m_hDibSection = NULL;
			m_pDibSectionBits = NULL;
		}

		// Calculate the Image Bits Offset,
		// we cannot trust the file size because it is the size in memory
		// which is usual a bit bigger (because of page alignment)!
		int OffBits;
		if (bmiheader.biClrUsed != 0)
			OffBits = bmiheader.biSize + bmiheader.biClrUsed*sizeof(RGBQUAD);
		else
		{
			if (bmiheader.biBitCount >= 16)
			{
				if ((bmiheader.biCompression == BI_BITFIELDS) && ((bmiheader.biBitCount == 16) || (bmiheader.biBitCount == 32)))
					OffBits = bmiheader.biSize + 3 * sizeof(RGBQUAD); // Bitfield Masks
				else
					OffBits = bmiheader.biSize;
			}
			else
				OffBits = bmiheader.biSize + (1 << bmiheader.biBitCount)*sizeof(RGBQUAD);
		}
		
		// Allocate BMI
		if (m_pBMI == NULL)
		{
			// Allocate
			m_pBMI = (LPBITMAPINFO)new BYTE[OffBits];
			if (m_pBMI == NULL)
				throw (int)BMP_E_NOMEM;
		}
		// Need to ReAllocate BMI because they are of differente size
		else if (OffBits != (int)GetBMISize())
		{
			delete [] m_pBMI;

			// Allocate
			m_pBMI = (LPBITMAPINFO)new BYTE[OffBits];
			if (m_pBMI == NULL)
				throw (int)BMP_E_NOMEM;
		}

		// Copy the Bit Map Info Header
		memcpy(m_pBMI, &bmiheader, sizeof(BITMAPINFOHEADER));
		if (m_pBMI->bmiHeader.biSizeImage == 0)
			m_pBMI->bmiHeader.biSizeImage = DWALIGNEDWIDTHBYTES(GetWidth() * GetBitCount()) * GetHeight();

		// Read the remaining of the Header
		if (file.Read(((LPBYTE)m_pBMI+sizeof(BITMAPINFOHEADER)), OffBits-sizeof(BITMAPINFOHEADER)) != (UINT)(OffBits-sizeof(BITMAPINFOHEADER)))
			throw (int)BMP_E_READ;
		dwReadBytes += (OffBits-sizeof(BITMAPINFOHEADER));

		// Set Color Pointer
		if (m_pBMI->bmiHeader.biBitCount <= 8)
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + (WORD)(m_pBMI->bmiHeader.biSize));
		else
			m_pColors = NULL;

		// Allocate Bits
		if (m_pBits == NULL)
		{
			// Allocate memory
			m_pBits = (LPBYTE)BIGALLOC(m_pBMI->bmiHeader.biSizeImage);
			if (m_pBits == NULL)
				throw (int)BMP_E_NOMEM;
		}
		// Need to ReAllocate Bits because they are of different size
		else if (m_dwImageSize != m_pBMI->bmiHeader.biSizeImage)
		{
			BIGFREE(m_pBits);

			// Allocate memory
			m_pBits = (LPBYTE)BIGALLOC(m_pBMI->bmiHeader.biSizeImage);
			if (m_pBits == NULL)
				throw (int)BMP_E_NOMEM;
		}

		// Set the image size
		m_dwImageSize = m_pBMI->bmiHeader.biSizeImage;

		// Go read the bits
		if (file.Read(m_pBits, m_dwImageSize) != m_dwImageSize)
			throw (int)BMP_E_READ;
		dwReadBytes += m_dwImageSize;

		// Has Alpha?
		if (m_pBMI->bmiHeader.biSize == sizeof(BITMAPV4HEADER) ||
			m_pBMI->bmiHeader.biSize == sizeof(BITMAPV5HEADER))
		{
			LPBITMAPV4HEADER pBV4 = (LPBITMAPV4HEADER)m_pBMI;
			if (pBV4->bV4AlphaMask > 0 &&
				m_pBMI->bmiHeader.biBitCount == 32)
			{
				m_FileInfo.m_bAlphaChannel = TRUE;
				m_bAlpha = TRUE;
			}
		}

		// Init Masks For 16 and 32 bits Pictures
		InitMasks();

		// Init File Info
		m_FileInfo.m_nType = CFileInfo::BMP;
		m_FileInfo.m_nWidth = m_pBMI->bmiHeader.biWidth;
		m_FileInfo.m_nHeight = m_pBMI->bmiHeader.biHeight;
		m_FileInfo.m_nBpp = m_pBMI->bmiHeader.biBitCount;
		m_FileInfo.m_nCompression = m_pBMI->bmiHeader.biCompression;
		m_FileInfo.m_nColorSpace = CFileInfo::COLORSPACE_RGB;
		m_FileInfo.m_dwImageSize = m_pBMI->bmiHeader.biSizeImage;
		m_FileInfo.m_dwFileSize = dwReadBytes + sizeof(BITMAPFILEHEADER);
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

		// Create Palette From BMI
		CreatePaletteFromBMI();

		// Decompress
		if (IsCompressed() && bDecompress)
		{
			if (!Decompress(GetBitCount())) // Decompress
				return FALSE;
		}

		// Flip Top-Down?
		if (m_pBMI->bmiHeader.biHeight < 0)
		{
			m_pBMI->bmiHeader.biHeight = -m_pBMI->bmiHeader.biHeight;
			FlipTopDown();
		}

		return TRUE;
	}
	catch (int error_code)
	{
		if (m_pBMI)
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
		m_pColors = NULL;

		CString str;
#ifdef _DEBUG
		str = _T("LoadBMPNoFileHeader: ");
#endif
		switch (error_code)
		{
			case BMP_E_NOMEM :			str += _T("Could not alloc memory\n");
			break;
			case BMP_E_BADBMP :			str += _T("Corrupted or unsupported DIB\n");
			break;
			case BMP_E_READ :			str += _T("Couldn't read BMP file\n");
			break;
			case BMP_E_FILEEMPTY :		str += _T("File is empty\n");
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

BOOL CDib::LoadBMP(	CFile& file,
					BOOL bOnlyHeader/*=FALSE*/,
					BOOL bDecompress/*=TRUE*/,
					CWnd* pProgressWnd/*=NULL*/,
					BOOL bProgressSend/*=TRUE*/,
					CWorkerThread* pThread/*=NULL*/)
{
	BITMAPFILEHEADER bmfHeader;
	DIB_INIT_PROGRESS;

	// Free
	m_FileInfo.Clear();
	m_bAlpha = FALSE;
	m_bGrayscale = FALSE;
	Free();
	
	try
	{
		// Get File Size
		m_FileInfo.m_dwFileSize = (DWORD)file.GetLength();
		if (m_FileInfo.m_dwFileSize == 0)
			throw (int)BMP_E_FILEEMPTY;

		// Go read the DIB file header and check if it's valid.
		if (file.Read((LPBYTE)&bmfHeader, sizeof(bmfHeader)) != sizeof(bmfHeader))
			throw (int)BMP_E_READ;
		if (bmfHeader.bfType != DIB_HEADER_MARKER)
			throw (int)BMP_E_BADBMP;

		DWORD dwReadBytes = sizeof(bmfHeader);

		// Allocate memory for Header
		m_pBMI = (LPBITMAPINFO)new BYTE[bmfHeader.bfOffBits-sizeof(BITMAPFILEHEADER)];
		if (m_pBMI == NULL)
			throw (int)BMP_E_NOMEM;

		// Read Header
		if (file.Read(m_pBMI, bmfHeader.bfOffBits-sizeof(BITMAPFILEHEADER)) != (UINT)(bmfHeader.bfOffBits-sizeof(BITMAPFILEHEADER)))
			throw (int)BMP_E_READ;
		dwReadBytes += bmfHeader.bfOffBits-sizeof(BITMAPFILEHEADER);

		// Convert to BITMAPINFOHEADER if Old Version 1.x OS/2 Bmp Format
		if (m_pBMI->bmiHeader.biSize == sizeof(BITMAPCOREHEADER))
		{
			m_FileInfo.m_bBmpOS2Hdr = TRUE;
			int nNumOfColors = (bmfHeader.bfOffBits - sizeof(BITMAPFILEHEADER) - sizeof(BITMAPCOREHEADER)) / sizeof(RGBTRIPLE);
			LPBITMAPINFO pBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER) + nNumOfColors * sizeof(RGBQUAD)];
			if (pBMI == NULL)
				throw (int)BMP_E_NOMEM;
			
			pBMI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER); 
			pBMI->bmiHeader.biWidth =  ((LPBITMAPCOREHEADER)m_pBMI)->bcWidth; 
			pBMI->bmiHeader.biHeight = ((LPBITMAPCOREHEADER)m_pBMI)->bcHeight; 
			pBMI->bmiHeader.biPlanes = ((LPBITMAPCOREHEADER)m_pBMI)->bcPlanes; 
			pBMI->bmiHeader.biBitCount = ((LPBITMAPCOREHEADER)m_pBMI)->bcBitCount;
			pBMI->bmiHeader.biCompression = BI_RGB; 
			pBMI->bmiHeader.biSizeImage = DWALIGNEDWIDTHBYTES(pBMI->bmiHeader.biWidth * pBMI->bmiHeader.biBitCount) * ABS(pBMI->bmiHeader.biHeight); 
			pBMI->bmiHeader.biXPelsPerMeter = 0; 
			pBMI->bmiHeader.biYPelsPerMeter = 0; 
			pBMI->bmiHeader.biClrUsed = 0; 
			pBMI->bmiHeader.biClrImportant = 0;

			RGBTRIPLE* pTripleColors = (RGBTRIPLE*)((LPBYTE)m_pBMI + (WORD)(((LPBITMAPCOREHEADER)m_pBMI)->bcSize));
			RGBQUAD* pQuadColors = (RGBQUAD*)((LPBYTE)pBMI + (WORD)(pBMI->bmiHeader.biSize));
			for (int i = 0 ; i < nNumOfColors ; i++)
			{
				pQuadColors[i].rgbBlue = pTripleColors[i].rgbtBlue;
				pQuadColors[i].rgbGreen = pTripleColors[i].rgbtGreen;
				pQuadColors[i].rgbRed = pTripleColors[i].rgbtRed;
				pQuadColors[i].rgbReserved = 0;
			}

			delete [] m_pBMI;
			m_pBMI = pBMI;
		}
		if (m_pBMI->bmiHeader.biBitCount <= 8)
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + (WORD)(m_pBMI->bmiHeader.biSize));
		else
			m_pColors = NULL;

		// Has Alpha?
		if (m_pBMI->bmiHeader.biSize == sizeof(BITMAPV4HEADER) ||
			m_pBMI->bmiHeader.biSize == sizeof(BITMAPV5HEADER))
		{
			LPBITMAPV4HEADER pBV4 = (LPBITMAPV4HEADER)m_pBMI;
			if (pBV4->bV4AlphaMask > 0 &&
				m_pBMI->bmiHeader.biBitCount == 32)
			{
				m_FileInfo.m_bAlphaChannel = TRUE;
				m_bAlpha = TRUE;
			}
		}

		// Init Masks For 16 and 32 bits Pictures
		InitMasks();

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

		// Compute image size
		ComputeImageSize();

		// If only header wanted return now
		if (bOnlyHeader)
			return TRUE;

		// Check and allocate bits
		if (!IsCompressed())
		{
			DWORD dwBitsBytes = m_FileInfo.m_dwFileSize - bmfHeader.bfOffBits;
			DWORD dwMinImageSize = DWALIGNEDWIDTHBYTES(GetWidth() * GetBitCount()) * GetHeight();
			m_pBits = (LPBYTE)BIGALLOC(MAX(dwBitsBytes, dwMinImageSize));
		}
		else
		{
			m_pBits = (LPBYTE)BIGALLOC(m_FileInfo.m_dwFileSize - bmfHeader.bfOffBits);
		}
		if (m_pBits == NULL)
			throw (int)BMP_E_NOMEM;

		// Go read the bits
		DWORD dwToRead = m_FileInfo.m_dwFileSize - bmfHeader.bfOffBits;
		DWORD dwChunkSize = dwToRead / 10;
		if (dwChunkSize == 0)
			dwChunkSize = 1;
		DWORD dwReadCount;
		for (dwReadCount = 0 ; dwReadCount < (dwToRead - dwChunkSize) ; dwReadCount += dwChunkSize)
		{
			if (pThread && pThread->DoExit())
				throw (int)BMP_THREADEXIT;
			DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, dwReadCount, dwToRead);
			if (file.Read(m_pBits + dwReadCount, dwChunkSize) != dwChunkSize)
				throw (int)BMP_E_READ;
		}
		DWORD dwLeftToRead = dwToRead - dwReadCount;
		DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, dwReadCount, dwToRead);
		if (file.Read(m_pBits + dwReadCount, dwLeftToRead) != dwLeftToRead)
			throw (int)BMP_E_READ;
		dwReadCount += dwLeftToRead;
		DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
		ASSERT(dwReadCount == dwToRead);
		dwReadBytes += dwReadCount;

		// Create Palette from BMI
		CreatePaletteFromBMI();

		// Decompress
		if (IsCompressed() && bDecompress)
		{
			if (!Decompress(GetBitCount())) // Decompress
				return FALSE;
		}

		// Flip Top-Down?
		if (m_pBMI->bmiHeader.biHeight < 0)
		{
			m_pBMI->bmiHeader.biHeight = -m_pBMI->bmiHeader.biHeight;
			FlipTopDown();
		}

		return TRUE;
	}
	catch (CFileException* e)
	{
		DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
		throw e;
	}
	catch (int error_code)
	{
		if (m_pBMI)
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
		m_pColors = NULL;

		DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

		// Just Exit
		if (error_code == BMP_THREADEXIT)
			return FALSE;

		CString str;
#ifdef _DEBUG
		str = _T("LoadBMP: ");
#endif
		switch (error_code)
		{
			case BMP_E_NOMEM :			str += _T("Could not alloc memory\n");
			break;
			case BMP_E_BADBMP :			str += _T("Corrupted or unsupported DIB\n");
			break;
			case BMP_E_READ :			str += _T("Couldn't read BMP file\n");
			break;
			case BMP_E_FILEEMPTY :		str += _T("File is empty\n");
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

BOOL CDib::SaveBMPNoFileHeader(CFile& file)
{
	DWORD dwDIBSize;

	try
	{
		if (!m_pBits)
		{
			if (!DibSectionToBits())
				throw (int)BMP_E_BADBMP;
		}

		if (!m_pBits || !m_pBMI)
			throw (int)BMP_E_BADBMP;

		int nSizePaletteOrMasks;
		if (m_pBMI->bmiHeader.biCompression == BI_BITFIELDS)
			nSizePaletteOrMasks = sizeof(DWORD) * 3;
		else
			nSizePaletteOrMasks = GetPaletteSize();

		// First, find size of header plus size of color table or masks.
		dwDIBSize = m_pBMI->bmiHeader.biSize + nSizePaletteOrMasks;

		// Now add the size of the image
		dwDIBSize += m_dwImageSize;

		// Write the DIB header
		UINT nCount = m_pBMI->bmiHeader.biSize + nSizePaletteOrMasks;
		file.Write(m_pBMI, nCount);
		
		// Write the DIB bits
		nCount += m_dwImageSize; 
		file.Write(m_pBits, m_dwImageSize);
 
		return TRUE;
	}
	catch (int error_code)
	{
		CString str;
#ifdef _DEBUG
		str = _T("SaveBMP: ");
#endif
		switch (error_code)
		{
			case BMP_E_BADBMP :		str += _T("Corrupted or unsupported DIB\n");
			break;
			default:				str += _T("Unspecified error\n");
			break;
		}

		TRACE(str);
		if (m_bShowMessageBoxOnError)
			::AfxMessageBox(str, MB_ICONSTOP);
		
		return FALSE;
	}
}

BOOL CDib::SaveBMP(	CFile& file,
					CWnd* pProgressWnd/*=NULL*/,
					BOOL bProgressSend/*=TRUE*/,
					CWorkerThread* pThread/*=NULL*/)
{
	BITMAPFILEHEADER bmfHdr; // Header for Bitmap file
	DWORD dwDIBSize;
	DIB_INIT_PROGRESS;

	try
	{
		if (!m_pBits)
		{
			if (!DibSectionToBits())
				throw (int)BMP_E_BADBMP;
		}

		if (!m_pBits || !m_pBMI)
			throw (int)BMP_E_BADBMP;

		int nSizePaletteOrMasks;
		if (m_pBMI->bmiHeader.biCompression == BI_BITFIELDS)
			nSizePaletteOrMasks = sizeof(DWORD) * 3;
		else
			nSizePaletteOrMasks = GetPaletteSize();

		// Fill in file type (first 2 bytes must be "BM" for a bitmap)
		bmfHdr.bfType = DIB_HEADER_MARKER;

		// First, find size of header plus size of color table or masks.
		dwDIBSize = m_pBMI->bmiHeader.biSize + nSizePaletteOrMasks;

		// Now add the size of the image
		dwDIBSize += m_dwImageSize;

		// Calculate the file size by adding the DIB size to sizeof(BITMAPFILEHEADER)
		bmfHdr.bfSize = dwDIBSize + sizeof(BITMAPFILEHEADER);
		bmfHdr.bfReserved1 = 0;
		bmfHdr.bfReserved2 = 0;

		// Now, calculate the offset the actual bitmap bits will be in
		// the file -- It's the Bitmap file header plus the DIB header,
		// plus the size of the color table.
		bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + m_pBMI->bmiHeader.biSize + nSizePaletteOrMasks;

		// Write the file header
		file.Write((LPBYTE)&bmfHdr, sizeof(BITMAPFILEHEADER));
		DWORD dwBytesSaved = sizeof(BITMAPFILEHEADER); 

		// Write the DIB header
		UINT nCount = m_pBMI->bmiHeader.biSize + nSizePaletteOrMasks;
		dwBytesSaved += nCount;
		file.Write(m_pBMI, nCount);
		
		// Write the DIB bits
		DWORD dwToWrite = m_dwImageSize;
		DWORD dwChunkSize = dwToWrite / 10;
		if (dwChunkSize == 0)
			dwChunkSize = 1;
		DWORD dwWriteCount;
		for (dwWriteCount = 0 ; dwWriteCount < (dwToWrite - dwChunkSize) ; dwWriteCount += dwChunkSize)
		{
			if (pThread && pThread->DoExit())
				throw (int)BMP_THREADEXIT;
			DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, dwWriteCount, dwToWrite);
			file.Write(m_pBits + dwWriteCount, dwChunkSize);
		}
		DWORD dwLeftToWrite = dwToWrite - dwWriteCount;
		DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, dwWriteCount, dwToWrite);
		file.Write(m_pBits + dwWriteCount, dwLeftToWrite);
		dwWriteCount += dwLeftToWrite;
		DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
		ASSERT(dwWriteCount == dwToWrite);
		dwBytesSaved += m_dwImageSize;

		return TRUE;
	}
	catch (CFileException* e)
	{
		DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
		throw e;
	}
	catch (int error_code)
	{
		DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

		// Just Exit
		if (error_code == BMP_THREADEXIT)
			return FALSE;

		CString str;
#ifdef _DEBUG
		str = (_T("SaveBMP: "));
#endif
		switch (error_code)
		{
		
			case BMP_E_BADBMP :		str += _T("Corrupted or unsupported DIB\n");
			break;
			default:				str += _T("Unspecified error\n");
			break;
		}

		TRACE(str);
		if (m_bShowMessageBoxOnError)
			::AfxMessageBox(str, MB_ICONSTOP);
		
		return FALSE;
	}
}