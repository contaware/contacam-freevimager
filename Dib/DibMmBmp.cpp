#include "stdafx.h"
#include "dib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BOOL CDib::MapBMP(LPCTSTR lpszPathName, BOOL bReadOnly)
{
	// Free
	m_FileInfo.Clear();
	m_bAlpha = FALSE;
	m_bGrayscale = FALSE;
	Free();

	try
	{
		CString sPathName(lpszPathName);
		if (sPathName.IsEmpty())
			throw (int)MMBMP_E_ZEROPATH;

		// Check for .bmp or .dib Extension
		if ((::GetFileExt(sPathName) != _T(".bmp")) && (::GetFileExt(sPathName) != _T(".dib")))
			throw (int)MMBMP_E_WRONGEXTENTION;

		// Store ReadOnly Flag
		m_bMMReadOnly = bReadOnly;

		// Access Type
		DWORD dwDesiredFileAccess = GENERIC_READ;
		if (!bReadOnly)
			dwDesiredFileAccess |= GENERIC_WRITE;

		// Open the real file on the file system
		m_hMMFile = ::CreateFile(lpszPathName,
								dwDesiredFileAccess,
								FILE_SHARE_READ | FILE_SHARE_WRITE,
								NULL, 
								OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL,
								NULL);
		if (m_hMMFile == INVALID_HANDLE_VALUE)
		{
			ShowLastError(_T("MapBMP()"));
			throw (int)MMBMP_E_CREATEFILE;
		}
		else
		{
			// Get the size of the file we are mapping
			DWORD dwFileSizeHigh = 0;
			m_FileInfo.m_dwFileSize = ::GetFileSize(m_hMMFile, &dwFileSizeHigh);
			if (m_FileInfo.m_dwFileSize == 0xFFFFFFFF)
				throw (int)MMBMP_E_FILESIZE;
			if (m_FileInfo.m_dwFileSize == 0)
				throw (int)MMBMP_E_FILEEMPTY;
		}

		// Create the file mapping object
		DWORD flProtect = (bReadOnly) ? PAGE_WRITECOPY : PAGE_READWRITE;
		
		// Create File Mapping
		m_hMMapping = ::CreateFileMapping(m_hMMFile, NULL, flProtect, 0, m_FileInfo.m_dwFileSize, NULL);
		if (m_hMMapping == NULL)
		{
			ShowLastError(_T("MapBMP()"));
			throw (int)MMBMP_E_CREATEFILEMAPPING;
		}

		// Map the view
		DWORD dwDesiredMapAccess = (bReadOnly) ? FILE_MAP_COPY : FILE_MAP_WRITE;
		m_pMMFile = ::MapViewOfFile(m_hMMapping, dwDesiredMapAccess, 0, 0, m_FileInfo.m_dwFileSize);
		if (m_pMMFile == NULL)
		{
			ShowLastError(_T("MapBMP()"));
			throw (int)MMBMP_E_MAPVIEWOFFILE;
		}

		// Go read the DIB file header and check if it's valid.
		LPBITMAPFILEHEADER lpBmfHeader = (LPBITMAPFILEHEADER)m_pMMFile;
		if (lpBmfHeader->bfType != DIB_HEADER_MARKER)
			throw (int)MMBMP_E_BADBMP;

		// Calculate the BMI, the Colors and the Bits Pointers
		m_pBMI = (LPBITMAPINFO)((LPBYTE)m_pMMFile + sizeof(BITMAPFILEHEADER)); 
		if (m_pBMI->bmiHeader.biBitCount <= 8)
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + (WORD)(m_pBMI->bmiHeader.biSize));
		else
			m_pColors = NULL;
		m_pBits = (LPBYTE)m_pMMFile + lpBmfHeader->bfOffBits;

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

		// Compute Image Size
		ComputeImageSize();

		// Check
		if (!IsCompressed())
		{
			DWORD dwBitsBytes = m_FileInfo.m_dwFileSize - lpBmfHeader->bfOffBits;
			DWORD dwMinImageSize = DWALIGNEDWIDTHBYTES(GetWidth() * GetBitCount()) * GetHeight();
			if (dwBitsBytes < dwMinImageSize)
				throw (int)MMBMP_E_BADBMP;
		}

		// Create Palette from BMI
		CreatePaletteFromBMI();

		return TRUE;
	}
	catch (int error_code)
	{
		UnMapBMP();

		CString str;
#ifdef _DEBUG
		str.Format(_T("MapBMP(%s):\n"), lpszPathName);
#endif
		switch(error_code)
		{
			case MMBMP_E_ZEROPATH :			str += _T("The file name is zero\n");
			break;
			case MMBMP_E_WRONGEXTENTION :	str += _T("The file extention is not .bmp or .dib\n");
			break;
			case MMBMP_E_BADBMP :			str += _T("Corrupted or unsupported DIB\n");
			break;
			case MMBMP_E_FILESIZE :			str += _T("GetFileSize failed\n");
			break;
			case MMBMP_E_FILEEMPTY :		str += _T("File is empty\n");
			break;
			case MMBMP_E_CREATEFILE :		return FALSE;
			case MMBMP_E_CREATEFILEMAPPING :return FALSE;
			case MMBMP_E_MAPVIEWOFFILE :	return FALSE;
			default:						str += _T("Unspecified error\n");
			break;
		}

		TRACE(str);
		if (m_bShowMessageBoxOnError)
			::AfxMessageBox(str, MB_ICONSTOP);
	
		return FALSE;
	}
}

BOOL CDib::MMCreateBMP(LPCTSTR lpszPathName)
{
	DWORD dwLastError = 0;

	// Check
	if (m_pBMI == NULL)
		return FALSE;

	try
	{
		CString sPathName(lpszPathName);
		if (sPathName.IsEmpty())
			throw (int)MMBMP_E_ZEROPATH;

		// Check for .bmp or .dib Extension
		if ((::GetFileExt(sPathName) != _T(".bmp")) && (::GetFileExt(sPathName) != _T(".dib")))
			throw (int)MMBMP_E_WRONGEXTENTION;

		// Free
		m_FileInfo.Clear();
		m_bAlpha = FALSE;
		m_bGrayscale = FALSE;
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

		// Access Type
		DWORD dwDesiredFileAccess = GENERIC_READ | GENERIC_WRITE;

		// Open the real file on the file system
		m_hMMFile = ::CreateFile(lpszPathName,
								dwDesiredFileAccess,
								FILE_SHARE_READ,
								NULL, 
								CREATE_ALWAYS,
								FILE_ATTRIBUTE_NORMAL,
								NULL);
		if (m_hMMFile == INVALID_HANDLE_VALUE)
		{
			ShowLastError(_T("MMCreateBMP()"));
			throw (int)MMBMP_E_CREATEFILE;
		}

		// Create the file mapping object
		DWORD flProtect = PAGE_READWRITE;
		
		// File Size
		m_FileInfo.m_dwFileSize = sizeof(BITMAPFILEHEADER) + GetBMISize() + m_dwImageSize;

		// Create File Mapping
		m_hMMapping = ::CreateFileMapping(m_hMMFile, NULL, flProtect, 0, m_FileInfo.m_dwFileSize, NULL);
		if (m_hMMapping == NULL)
		{
			ShowLastError(_T("MMCreateBMP()"));
			throw (int)MMBMP_E_CREATEFILEMAPPING;
		}

		// Map the view
		DWORD dwDesiredMapAccess = FILE_MAP_WRITE;
		m_pMMFile = ::MapViewOfFile(m_hMMapping, dwDesiredMapAccess, 0, 0, m_FileInfo.m_dwFileSize);
		if (m_pMMFile == NULL)
		{
			ShowLastError(_T("MMCreateBMP()"));
			throw (int)MMBMP_E_MAPVIEWOFFILE;
		}

		// Make the DIB file header
		BITMAPFILEHEADER BmfHdr;
		BmfHdr.bfType = DIB_HEADER_MARKER;
		BmfHdr.bfSize = m_FileInfo.m_dwFileSize;
		BmfHdr.bfReserved1 = 0;
		BmfHdr.bfReserved2 = 0;
		BmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + GetBMISize();
		
		// Write the file header
		memcpy(m_pMMFile, &BmfHdr, sizeof(BITMAPFILEHEADER));
		DWORD dwBytesSaved = sizeof(BITMAPFILEHEADER); 

		// Write the DIB header
		UINT nCount = GetBMISize();
		memcpy((LPBYTE)m_pMMFile + dwBytesSaved, m_pBMI, nCount);
		dwBytesSaved += nCount;

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

		// Free BMI
		delete [] m_pBMI;
		m_pBMI = NULL;

		// Set Pointers
		m_pBMI = (LPBITMAPINFO)((LPBYTE)m_pMMFile + sizeof(BITMAPFILEHEADER)); 
		if (m_pBMI->bmiHeader.biBitCount <= 8)
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + (WORD)(m_pBMI->bmiHeader.biSize));
		else
			m_pColors = NULL;
		m_pBits = (LPBYTE)m_pMMFile + BmfHdr.bfOffBits;

		// Create Palette From BMI
		CreatePaletteFromBMI();

		return TRUE;
	}
	catch (int error_code)
	{
		UnMapBMP();

		CString str;
#ifdef _DEBUG
		str.Format(_T("MMCreateBMP(%s):\n"), lpszPathName);
#endif
		switch(error_code)
		{
			case MMBMP_E_ZEROPATH :			str += _T("The file name is zero\n");
			break;
			case MMBMP_E_WRONGEXTENTION :	str += _T("The file extention is not .bmp or .dib\n");
			break;
			case MMBMP_E_CREATEFILE :		return FALSE;
			case MMBMP_E_CREATEFILEMAPPING :return FALSE;
			case MMBMP_E_MAPVIEWOFFILE :	return FALSE;
			default:						str += _T("Unspecified error\n");
			break;
		}

		TRACE(str);
		if (m_bShowMessageBoxOnError)
			::AfxMessageBox(str, MB_ICONSTOP);
	
		return FALSE;
	}
}

BOOL CDib::MMBMPToBits()
{
	LPBYTE pBits;
	LPBITMAPINFO pBMI;
	LPBITMAPFILEHEADER pBmfHeader;

	if (m_pMMFile == NULL) return FALSE;
	
	// Allocate memory for BMI and the Image Bits
	pBmfHeader = (LPBITMAPFILEHEADER)m_pMMFile;
	pBMI = (LPBITMAPINFO)new BYTE[pBmfHeader->bfOffBits - sizeof(BITMAPFILEHEADER)];
	if (pBMI == NULL)
		return FALSE;
	pBits = (LPBYTE)BIGALLOC(m_FileInfo.m_dwFileSize - pBmfHeader->bfOffBits);
	if (pBits == NULL)
	{
		delete [] pBMI;
		return FALSE;
	}

	// Copy the Image
	memcpy((void*)pBMI, (void*)m_pBMI, pBmfHeader->bfOffBits - sizeof(BITMAPFILEHEADER));
	memcpy((void*)pBits, (void*)m_pBits, m_FileInfo.m_dwFileSize - pBmfHeader->bfOffBits);

	// Unmap the memory mapped file
	UnMapBMP();

	// Update the pointers
	m_pBMI = pBMI;
	m_pBits = pBits;
	if (m_pBMI->bmiHeader.biBitCount <= 8)
		m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + (WORD)(m_pBMI->bmiHeader.biSize));
	else
		m_pColors = NULL;

	return TRUE;
}

BOOL CDib::MMBMPToBMI()
{
	LPBITMAPINFO pBMI;
	LPBITMAPFILEHEADER pBmfHeader;

	if (m_pMMFile == NULL) return FALSE;
	
	// Allocate memory for BMI
	pBmfHeader = (LPBITMAPFILEHEADER)m_pMMFile;
	pBMI = (LPBITMAPINFO)new BYTE[pBmfHeader->bfOffBits - sizeof(BITMAPFILEHEADER)];
	if (pBMI == NULL)
		return FALSE;

	// Copy BMI
	memcpy((void*)pBMI, (void*)m_pBMI, pBmfHeader->bfOffBits - sizeof(BITMAPFILEHEADER));

	// Unmap the memory mapped file
	UnMapBMP();

	// Update the pointer
	m_pBMI = pBMI;
	if (m_pBMI->bmiHeader.biBitCount <= 8)
		m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + (WORD)(m_pBMI->bmiHeader.biSize));
	else
		m_pColors = NULL;

	return TRUE;
}

void CDib::UnMapBMP()
{
	if (m_pMMFile != NULL)
	{
		::FlushViewOfFile(m_pMMFile, 0);
		::UnmapViewOfFile(m_pMMFile);
		m_pMMFile = NULL;
		m_pBMI = NULL;
		m_pBits = NULL;
	}
	if (m_hMMapping != NULL)
	{
		::CloseHandle(m_hMMapping);
		m_hMMapping = NULL;
	}
	if (m_hMMFile != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(m_hMMFile);
		m_hMMFile = INVALID_HANDLE_VALUE;
	}
}