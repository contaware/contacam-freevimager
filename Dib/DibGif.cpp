#include "stdafx.h"
#include "dib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef SUPPORT_GIFLIB

CString CDib::GIFGetVersion(LPCTSTR lpszPathName, BOOL bShowMessageBoxOnError)
{
	if (::GetFileExt(lpszPathName) != _T(".gif"))
		return _T("");

	try
	{
		char Ver[GIF_STAMP_LEN];
		CFile file(lpszPathName, CFile::modeRead | CFile::shareDenyNone);
		if (file.Read(Ver, GIF_STAMP_LEN) != GIF_STAMP_LEN)
		{
			file.Close();
			return _T("");
		}
		if (strncmp(GIF87_STAMP, Ver, GIF_STAMP_LEN) == 0)
			return CString(GIF87_STAMP);
		else if (strncmp(GIF89_STAMP, Ver, GIF_STAMP_LEN) == 0)
			return CString(GIF89_STAMP);
		else
			return _T("");
    }
	catch (CFileException* e)
	{
		TCHAR szCause[255];
		CString str(_T("GIFGetVersion: "));
		e->GetErrorMessage(szCause, 255);
		str += szCause;
		str += _T("\n");
		TRACE(str);
		if (bShowMessageBoxOnError)
			::AfxMessageBox(str, MB_ICONSTOP);
		e->Delete();
		return _T("");
	}
}

BOOL CDib::IsAnimatedGIF(LPCTSTR lpszPathName, BOOL bShowMessageBoxOnError)
{	
	if (::GetFileExt(lpszPathName) != _T(".gif"))
		return FALSE;

	CDib Dib;
	Dib.SetShowMessageBoxOnError(bShowMessageBoxOnError);
	if (Dib.LoadGIFHeader(lpszPathName))
	{
		if (Dib.m_Gif.m_nLoadImageCount > 1)
		{
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CDib::LoadGIFHeader(LPCTSTR lpszPathName)
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
			throw (int)GIF_E_ZEROPATH;

		// Check for .gif Extension
		if (::GetFileExt(sPathName) != _T(".gif"))
			throw (int)GIF_E_WRONGEXTENTION;

		CFile file(lpszPathName, CFile::modeRead | CFile::shareDenyWrite);
		m_FileInfo.m_dwFileSize = (DWORD)file.GetLength();
		file.Close();
		if (m_FileInfo.m_dwFileSize == 0)
			throw (int)GIF_E_FILEEMPTY;
	
		// Open gif File
		m_Gif.LoadClose();
		m_Gif.m_pLoadFile = ::DGifOpenFileName(lpszPathName);

		if (m_Gif.m_pLoadFile == NULL)
			throw GIF_E_READ;

		// Allocate BMIs
		m_pBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER) + 256*sizeof(RGBQUAD)];
		m_Gif.m_pScreenBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER) + 256*sizeof(RGBQUAD)];
		if (!m_pBMI || !m_Gif.m_pScreenBMI)
			throw GIF_E_NOMEM;

		// Get Screen Colors
		if (m_Gif.m_pLoadFile->SColorMap)
		{
			m_Gif.m_bHasScreenColorTable = TRUE;
			m_Gif.GetColorMap(m_Gif.m_pLoadFile->SColorMap, m_pBMI);
			m_Gif.GetColorMap(m_Gif.m_pLoadFile->SColorMap, m_Gif.m_pScreenBMI);
			m_Gif.m_nBackgndColorIndex = m_Gif.m_pLoadFile->SBackGroundColor;
			m_FileInfo.m_bHasBackgroundColor = TRUE;
		}
		else
		{
			m_Gif.m_bHasScreenColorTable = FALSE;
			m_pBMI->bmiHeader.biBitCount =      8;
			m_pBMI->bmiHeader.biClrUsed =		256; 
			m_pBMI->bmiHeader.biClrImportant =	0;
			m_Gif.m_nBackgndColorIndex = GIF_COLORINDEX_NOT_DEFINED;
		}

		// DWORD aligned target DIB ScanLineSize 
		DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES(m_Gif.m_pLoadFile->SWidth * m_pBMI->bmiHeader.biBitCount);

		// Init BMIs
		m_pBMI->bmiHeader.biSize =          sizeof(BITMAPINFOHEADER);
		m_pBMI->bmiHeader.biWidth =         m_Gif.m_pLoadFile->SWidth;
		m_pBMI->bmiHeader.biHeight =        m_Gif.m_pLoadFile->SHeight;
		m_pBMI->bmiHeader.biPlanes =        1;
		m_pBMI->bmiHeader.biCompression =   BI_RGB;  // uncompressed
		m_pBMI->bmiHeader.biSizeImage =     m_Gif.m_pLoadFile->SHeight*uiDIBScanLineSize;   
		m_pBMI->bmiHeader.biXPelsPerMeter = 0;
		m_pBMI->bmiHeader.biYPelsPerMeter = 0;
		m_dwImageSize = m_pBMI->bmiHeader.biSizeImage;
		m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + (WORD)(m_pBMI->bmiHeader.biSize));
		memcpy(m_Gif.m_pScreenBMI, m_pBMI, m_pBMI->bmiHeader.biSize);

		// Set Background Color
		if (m_FileInfo.m_bHasBackgroundColor)
		{
			m_FileInfo.m_crBackgroundColor = RGB(	m_pColors[m_Gif.m_nBackgndColorIndex].rgbRed,
													m_pColors[m_Gif.m_nBackgndColorIndex].rgbGreen,
													m_pColors[m_Gif.m_nBackgndColorIndex].rgbBlue);
			m_crBackgroundColor = m_FileInfo.m_crBackgroundColor;
		}

		// Fill-Up a Halftone Color Table if Screen Color Table not available
		if (m_Gif.m_pLoadFile->SColorMap == NULL)
		{
			FillHalftoneColors(m_pColors, 256);
			memcpy(m_Gif.m_pScreenBMI, m_pBMI, m_pBMI->bmiHeader.biSize + GetNumColors()*sizeof(RGBQUAD));
		}

		// Init File Info
		m_FileInfo.m_nType = CFileInfo::GIF;
		m_FileInfo.m_nWidth = m_pBMI->bmiHeader.biWidth;
		m_FileInfo.m_nHeight = m_pBMI->bmiHeader.biHeight;
		m_FileInfo.m_nBpp = m_pBMI->bmiHeader.biBitCount;
		m_FileInfo.m_nCompression = m_pBMI->bmiHeader.biCompression;
		m_FileInfo.m_nColorSpace = CFileInfo::COLORSPACE_RGB;
		m_FileInfo.m_dwImageSize = m_pBMI->bmiHeader.biSizeImage;
		m_FileInfo.m_nXPixsPerMeter = m_pBMI->bmiHeader.biXPelsPerMeter;
		m_FileInfo.m_nYPixsPerMeter = m_pBMI->bmiHeader.biYPelsPerMeter;
		m_FileInfo.m_nNumColors = m_pBMI->bmiHeader.biClrUsed;
		m_FileInfo.m_bPalette = TRUE;
		m_FileInfo.m_nImageCount = 0;
		m_FileInfo.m_nImagePos = 0;

		// Parse
		GifRecordType RecordType;
		GifByteType *pExtension;
		m_Gif.m_nLoadImageCount = 0;
		m_Gif.m_wLoopCount = 0;
		m_Gif.m_bHasLoopExtension = FALSE;
		m_Gif.m_bHasGraphicExtension = FALSE;
		m_Gif.m_nDelay = 10; // Default to 100 msec
		m_Gif.m_nTransparencyColorIndex = GIF_COLORINDEX_NOT_DEFINED;
		m_Gif.m_nFlags = 0;
		if (m_Gif.m_lpszComment)
		{
			delete [] m_Gif.m_lpszComment;
			m_Gif.m_lpszComment = NULL;
		}
		int nCommentLen = 0;
		do
		{
			int ExtCode;

			if (::DGifGetRecordType(m_Gif.m_pLoadFile, &RecordType) == GIF_ERROR) 
				break;

			switch (RecordType)
			{
				case IMAGE_DESC_RECORD_TYPE:
				{
					if (::DGifGetImageDesc(m_Gif.m_pLoadFile) != GIF_ERROR)
					{
						// Inc. Loaded Image Count
						++m_Gif.m_nLoadImageCount;
						++m_FileInfo.m_nImageCount;
						m_FileInfo.m_nImagePos = m_FileInfo.m_nImageCount - 1;
					
						// Skip Data
						GifByteType *Dummy;
						do
						{
							if (::DGifGetCodeNext(m_Gif.m_pLoadFile, &Dummy) == GIF_ERROR)
								break;
						}
						while (Dummy != NULL);
					}

					break;
				}

				case EXTENSION_RECORD_TYPE:
				{
					if (::DGifGetExtension(m_Gif.m_pLoadFile, &ExtCode, &pExtension) == GIF_ERROR)
						throw (int)GIF_E_GIFLIB;
					if (!pExtension)
						break;
					
					BOOL bNetscapeExt = FALSE;
					BOOL bCommentExt = FALSE;
					switch (ExtCode)
					{
						case COMMENT_EXT_FUNC_CODE:
						{
							int nLen = pExtension[0];
							char* pComment = new char[nCommentLen+nLen+1];
							if (m_Gif.m_lpszComment)
								memcpy(pComment, m_Gif.m_lpszComment, nCommentLen);
							memcpy(pComment+nCommentLen, &(pExtension[1]), nLen);
							if (m_Gif.m_lpszComment)
								delete [] m_Gif.m_lpszComment;
							m_Gif.m_lpszComment = pComment;
							nCommentLen += nLen; 
							m_Gif.m_lpszComment[nCommentLen] = '\0';
							bCommentExt = TRUE;
							break;
						}
						case GRAPHICS_EXT_FUNC_CODE:
						{
							m_Gif.m_bHasGraphicExtension = TRUE;
							ASSERT(pExtension[0] == 4);
							m_Gif.m_nFlags = pExtension[1];
							m_Gif.m_nDelay = MAKEWORD(pExtension[2], pExtension[3]);
							m_Gif.m_nTransparencyColorIndex = (m_Gif.m_nFlags & GIF_TRANSPARENT) ? pExtension[4] : GIF_COLORINDEX_NOT_DEFINED;
							if (m_Gif.m_nTransparencyColorIndex != GIF_COLORINDEX_NOT_DEFINED)
								m_FileInfo.m_nNumTransparencyIndexes = 1;
							break;
						}
						case PLAINTEXT_EXT_FUNC_CODE:
						{
							TRACE(_T("GIF89 plaintext\n"));
							break;
						}
						case APPLICATION_EXT_FUNC_CODE:
						{
							if (memcmp(pExtension, CGif::m_szNetscape20ext, CGif::m_szNetscape20ext[0] + 1) == 0)
								bNetscapeExt = TRUE;
							break;
						}
						default:
						{
							TRACE(_T("pExtension record of unknown type\n"));
							break;
						}
					}
					
					do
					{
						if (::DGifGetExtensionNext(m_Gif.m_pLoadFile, &pExtension) == GIF_ERROR)
							throw (int)GIF_E_GIFLIB;
						
						if (pExtension)
						{
							// Process Netscape 2.0 extension (GIF looping)
							if (bNetscapeExt)
							{
								GifByteType bLength = pExtension[0];
								int iSubCode = pExtension[1] & 0x07;
								if (bLength == 3 && iSubCode == GIF_NSEXT_LOOP)
								{
									m_Gif.m_bHasLoopExtension = TRUE;
									m_Gif.m_wLoopCount = MAKEWORD(pExtension[2], pExtension[3]);
								}
							}
							// Process More Comment
							else if (bCommentExt)
							{
								int nLen = pExtension[0];
								char* pComment = new char[nCommentLen+nLen+1];
								memcpy(pComment, m_Gif.m_lpszComment, nCommentLen);
								memcpy(pComment+nCommentLen, &(pExtension[1]), nLen);
								delete [] m_Gif.m_lpszComment;
								m_Gif.m_lpszComment = pComment;
								nCommentLen += nLen; 
								m_Gif.m_lpszComment[nCommentLen] = '\0';
							}
						}
					}
					while (pExtension);
					break;
				}

				case TERMINATE_RECORD_TYPE:
					break;

				default:
					break;
			}
		}
		while (RecordType != TERMINATE_RECORD_TYPE);

		// Close File
		m_Gif.LoadClose();
		
		return TRUE;
	}
	catch (CFileException* e)
	{
		TCHAR szCause[255];
		CString str(_T("LoadFirstGIF: "));
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
		if (m_pBMI)
		{
			delete [] m_pBMI;
			m_pBMI = NULL;
			m_pColors = NULL;
		}
		m_Gif.LoadClose();
		m_Gif.Free();

		CString str;
#ifdef _DEBUG
		str.Format(_T("LoadFirstGIF(%s):\n"), lpszPathName);
#endif
		switch (error_code)
		{
			case GIF_E_ZEROPATH :		str += _T("The file name is zero\n");
			break;
			case GIF_E_WRONGEXTENTION :	str += _T("The file extention is not .gif\n");
			break;
			case GIF_E_NOMEM :			str += _T("Could not alloc memory\n");
			break;
			case GIF_E_GIFLIB :			str += _T("Cannot load file\n");
			break;
			case GIF_E_READ :			str += _T("Couldn't read GIF file\n");
			break;
			case GIF_E_FILEEMPTY :		str += _T("File is empty\n");
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

BOOL CDib::LoadGIF(	LPCTSTR lpszPathName,
					BOOL bOnlyHeader/*=FALSE*/,
					CWnd* pProgressWnd/*=NULL*/,
					BOOL bProgressSend/*=TRUE*/,
					CWorkerThread* pThread/*=NULL*/)
{
	BOOL res = LoadGIFHeader(lpszPathName);
	if (bOnlyHeader)
		return res;

	// Load as RGBA32
	if (m_Gif.HasTransparency())
	{
		if (LoadFirstGIF32(	lpszPathName,
							bOnlyHeader,
							pProgressWnd,
							bProgressSend,
							pThread))
			return m_Gif.LoadClose();
		else
			return FALSE;
	}
	// Load as RGB8, RGB4 or RGB1
	else
	{
		if (LoadFirstGIF(	lpszPathName,
							bOnlyHeader,
							pProgressWnd,
							bProgressSend,
							pThread))
			return m_Gif.LoadClose();
		else
			return FALSE;
	}
}

BOOL CDib::LoadFirstGIFRaw(	LPCTSTR lpszPathName,
							CWnd* pProgressWnd/*=NULL*/,
							BOOL bProgressSend/*=TRUE*/,
							CWorkerThread* pThread/*=NULL*/)
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
			throw (int)GIF_E_ZEROPATH;

		// Check for .gif Extension
		if (::GetFileExt(sPathName) != _T(".gif"))
			throw (int)GIF_E_WRONGEXTENTION;

		CFile file(lpszPathName, CFile::modeRead | CFile::shareDenyWrite);
		m_FileInfo.m_dwFileSize = (DWORD)file.GetLength();
		file.Close();
		if (m_FileInfo.m_dwFileSize == 0)
			throw (int)GIF_E_FILEEMPTY;
	
		// Open gif File
		m_Gif.LoadClose();
		m_Gif.m_pLoadFile = ::DGifOpenFileName(lpszPathName);

		if (m_Gif.m_pLoadFile == NULL)
			throw GIF_E_READ;

		// Allocate BMIs
		m_pBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER) + 256*sizeof(RGBQUAD)];
		m_Gif.m_pScreenBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER) + 256*sizeof(RGBQUAD)];
		if (!m_pBMI || !m_Gif.m_pScreenBMI)
			throw GIF_E_NOMEM;

		// Get Screen Colors
		if (m_Gif.m_pLoadFile->SColorMap)
		{
			m_Gif.m_bHasScreenColorTable = TRUE;
			m_Gif.GetColorMap(m_Gif.m_pLoadFile->SColorMap, m_pBMI);
			m_Gif.GetColorMap(m_Gif.m_pLoadFile->SColorMap, m_Gif.m_pScreenBMI);
			m_Gif.m_nBackgndColorIndex = m_Gif.m_pLoadFile->SBackGroundColor;
			m_FileInfo.m_bHasBackgroundColor = TRUE;
			m_pBMI->bmiHeader.biBitCount =      8;
		}
		else
		{
			m_Gif.m_bHasScreenColorTable = FALSE;
			m_pBMI->bmiHeader.biBitCount =      8;
			m_pBMI->bmiHeader.biClrUsed =		256; 
			m_pBMI->bmiHeader.biClrImportant =	0;
			m_Gif.m_nBackgndColorIndex = GIF_COLORINDEX_NOT_DEFINED;
		}

		// DWORD aligned target DIB ScanLineSize 
		DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES(m_Gif.m_pLoadFile->SWidth * m_pBMI->bmiHeader.biBitCount);

		// Init BMIs
		m_pBMI->bmiHeader.biSize =          sizeof(BITMAPINFOHEADER);
		m_pBMI->bmiHeader.biWidth =         m_Gif.m_pLoadFile->SWidth;
		m_pBMI->bmiHeader.biHeight =        m_Gif.m_pLoadFile->SHeight;
		m_pBMI->bmiHeader.biPlanes =        1;
		m_pBMI->bmiHeader.biCompression =   BI_RGB;  // uncompressed
		m_pBMI->bmiHeader.biSizeImage =     m_Gif.m_pLoadFile->SHeight*uiDIBScanLineSize;   
		m_pBMI->bmiHeader.biXPelsPerMeter = 0;
		m_pBMI->bmiHeader.biYPelsPerMeter = 0;
		m_dwImageSize = m_pBMI->bmiHeader.biSizeImage;
		m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + (WORD)(m_pBMI->bmiHeader.biSize));
		memcpy(m_Gif.m_pScreenBMI, m_pBMI, m_pBMI->bmiHeader.biSize);

		// Set Background Color
		if (m_FileInfo.m_bHasBackgroundColor)
		{
			m_FileInfo.m_crBackgroundColor = RGB(	m_pColors[m_Gif.m_nBackgndColorIndex].rgbRed,
													m_pColors[m_Gif.m_nBackgndColorIndex].rgbGreen,
													m_pColors[m_Gif.m_nBackgndColorIndex].rgbBlue);
			m_crBackgroundColor = m_FileInfo.m_crBackgroundColor;
		}

		// Fill-Up a Halftone Color Table if Screen Color Table not available
		if (m_Gif.m_pLoadFile->SColorMap == NULL)
		{
			FillHalftoneColors(m_pColors, 256);
			memcpy(m_Gif.m_pScreenBMI, m_pBMI, m_pBMI->bmiHeader.biSize + GetNumColors()*sizeof(RGBQUAD));
		}

		// Init File Info
		m_FileInfo.m_nType = CFileInfo::GIF;
		m_FileInfo.m_nWidth = m_pBMI->bmiHeader.biWidth;
		m_FileInfo.m_nHeight = m_pBMI->bmiHeader.biHeight;
		m_FileInfo.m_nBpp = m_pBMI->bmiHeader.biBitCount;
		m_FileInfo.m_nCompression = m_pBMI->bmiHeader.biCompression;
		m_FileInfo.m_nColorSpace = CFileInfo::COLORSPACE_RGB;
		m_FileInfo.m_dwImageSize = m_pBMI->bmiHeader.biSizeImage;
		m_FileInfo.m_nXPixsPerMeter = m_pBMI->bmiHeader.biXPelsPerMeter;
		m_FileInfo.m_nYPixsPerMeter = m_pBMI->bmiHeader.biYPelsPerMeter;
		m_FileInfo.m_nNumColors = m_pBMI->bmiHeader.biClrUsed;
		m_FileInfo.m_bPalette = TRUE;
		m_FileInfo.m_nImageCount = 0;
		m_FileInfo.m_nImagePos = 0;

		// Create Palette from BMI
		CreatePaletteFromBMI();

		// Allocate Space for 1 image + 1 Extra Line For Gif Decompression
		m_pBits = (LPBYTE)BIGALLOC(m_dwImageSize + m_Gif.m_pLoadFile->SWidth + SAFETY_BITALLOC_MARGIN);
		if (!m_pBits)
			throw GIF_E_NOMEM;

		// Load First Gif
		m_Gif.m_nLoadImageCount = 0;
		m_Gif.m_wLoopCount = 0;
		m_Gif.m_bHasLoopExtension = FALSE;
		if (LoadNextGIFRaw(pProgressWnd, bProgressSend, pThread) < 0)
			throw (int)GIF_E_GIFLIB;

		return TRUE;
	}
	catch (CFileException* e)
	{
		TCHAR szCause[255];
		CString str(_T("LoadFirstGIFRaw: "));
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
		if (m_Gif.m_pScreenBMI)
		{
			delete [] m_Gif.m_pScreenBMI;
			m_Gif.m_pScreenBMI = NULL;
		}
		if (m_Gif.m_pLoadFile)
		{
			::DGifCloseFile(m_Gif.m_pLoadFile);
			m_Gif.m_pLoadFile = NULL;
		}

		// Just Exit
		if (pThread && pThread->DoExit())
			return FALSE;

		CString str;
#ifdef _DEBUG
		str.Format(_T("LoadFirstGIFRaw(%s):\n"), lpszPathName);
#endif
		switch (error_code)
		{
			case GIF_E_ZEROPATH :		str += _T("The file name is zero\n");
			break;
			case GIF_E_WRONGEXTENTION :	str += _T("The file extention is not .gif\n");
			break;
			case GIF_E_NOMEM :			str += _T("Could not alloc memory\n");
			break;
			case GIF_E_GIFLIB :			str += _T("Cannot load file\n");
			break;
			case GIF_E_READ :			str += _T("Couldn't read GIF file\n");
			break;
			case GIF_E_FILEEMPTY :		str += _T("File is empty\n");
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

//
//  The Following example illustrates how the rows of an interlaced image are
//  ordered.
//  
//        Row Number                                        Interlace Pass
//  
//   0    -----------------------------------------       1
//   1    -----------------------------------------                         4
//   2    -----------------------------------------                   3
//   3    -----------------------------------------                         4
//   4    -----------------------------------------             2
//   5    -----------------------------------------                         4
//   6    -----------------------------------------                   3
//   7    -----------------------------------------                         4
//   8    -----------------------------------------       1
//   9    -----------------------------------------                         4
//   10   -----------------------------------------                   3
//   11   -----------------------------------------                         4
//   12   -----------------------------------------             2
//   13   -----------------------------------------                         4
//   14   -----------------------------------------                   3
//   15   -----------------------------------------                         4
//   16   -----------------------------------------       1
//   17   -----------------------------------------                         4
//   18   -----------------------------------------                   3
//   19   -----------------------------------------                         4
//
// Fetch next image from GIF file
// Returns 1 ok image loaded, 0 for end-of-file (no image loaded), -1 for error
int CDib::LoadNextGIFRaw(	CWnd* pProgressWnd/*=NULL*/,
							BOOL bProgressSend/*=TRUE*/,
							CWorkerThread* pThread/*=NULL*/)
{
	// Error if no gif file!
	if (!m_Gif.m_pLoadFile)
		return -1;

	//					 ___________
	//		pBits ->	|			|
	//					|	current	|
	//					|	image	|
	//					|___________|
	//		pLine ->	|___________|
	//

	LPBYTE pBits = m_pBits;
	GifPixelType *pLine = pBits + m_dwImageSize;
	GifRecordType RecordType;
	GifByteType *pExtension;

	m_Gif.m_bAlphaUsed = FALSE;
	m_Gif.m_bHasGraphicExtension = FALSE;
	m_Gif.m_nDelay = 10; // Default to 100 msec
	m_Gif.m_nTransparencyColorIndex = GIF_COLORINDEX_NOT_DEFINED;
	m_Gif.m_nFlags = 0;
	if (m_Gif.m_lpszComment)
	{
		delete [] m_Gif.m_lpszComment;
		m_Gif.m_lpszComment = NULL;
	}
	int nCommentLen = 0;

	do
	{
		int i, ExtCode;

		if (::DGifGetRecordType(m_Gif.m_pLoadFile, &RecordType) == GIF_ERROR) 
			break;

		switch (RecordType)
		{
			case IMAGE_DESC_RECORD_TYPE:
			{
				if (::DGifGetImageDesc(m_Gif.m_pLoadFile) != GIF_ERROR)
				{
					// Inc. Loaded Image Count
					++m_Gif.m_nLoadImageCount;
					++m_FileInfo.m_nImageCount;
					m_FileInfo.m_nImagePos = m_FileInfo.m_nImageCount - 1;

					// Allocate (if necessary) & Init Current Image BMI
					if (!m_Gif.m_pCurrentImageBMI)
						m_Gif.m_pCurrentImageBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER) + 256*sizeof(RGBQUAD)];
					if (!m_Gif.m_pCurrentImageBMI)
						return -1;
					int nColorsCount = m_Gif.m_pLoadFile->SColorMap ? m_Gif.m_pLoadFile->SColorMap->ColorCount : 256;
					if (nColorsCount >= 256)
						nColorsCount = 256;
					memcpy(m_pBMI, m_Gif.m_pScreenBMI,
								sizeof(BITMAPINFOHEADER) + nColorsCount*sizeof(RGBQUAD));
					if (m_Gif.m_pLoadFile->Image.ColorMap == NULL)
					{
						m_Gif.m_bHasImageColorTable = FALSE;
					}
					else
					{
						m_Gif.m_bHasImageColorTable = TRUE;
						m_Gif.GetColorMap(m_Gif.m_pLoadFile->Image.ColorMap, m_pBMI);
					}
					DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES(	m_pBMI->bmiHeader.biWidth *
																	m_pBMI->bmiHeader.biBitCount);
					m_pBMI->bmiHeader.biSizeImage = m_pBMI->bmiHeader.biHeight*uiDIBScanLineSize;
					m_dwImageSize = m_pBMI->bmiHeader.biSizeImage;
					m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + (WORD)(m_pBMI->bmiHeader.biSize));
					memcpy(m_Gif.m_pCurrentImageBMI, m_pBMI,
								sizeof(BITMAPINFOHEADER) + 256*sizeof(RGBQUAD));

					m_Gif.m_nLeft = m_Gif.m_pLoadFile->Image.Left;
					m_Gif.m_nTop = m_Gif.m_pLoadFile->Image.Top;
					m_Gif.m_nWidth = m_Gif.m_pLoadFile->Image.Width;
					m_Gif.m_nHeight = m_Gif.m_pLoadFile->Image.Height;
					DIB_INIT_PROGRESS;

					// Interlaced image
					if (m_Gif.m_pLoadFile->Image.Interlace)
					{
						// Need to perform 4 passes on the images:
						for (int pass = 0; pass < 4; pass++)
						{
							for (i = CGif::m_InterlacedOffset[pass] ; i < m_Gif.m_pLoadFile->Image.Height ; i += CGif::m_InterlacedJumps[pass])
							{
								if (::DGifGetLine(m_Gif.m_pLoadFile, pLine, m_Gif.m_pLoadFile->Image.Width) == GIF_ERROR)
								{
									DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
									return -1;
								}
								if ((i & 0x7) == 0)
								{
									if (pThread && pThread->DoExit())
									{
										DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
										return -1;
									}
									DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, i, m_Gif.m_pLoadFile->Image.Height);
								}
								memcpy(	pBits + (m_Gif.m_pLoadFile->Image.Top + i) * uiDIBScanLineSize + m_Gif.m_pLoadFile->Image.Left,
										pLine,
										m_Gif.m_pLoadFile->Image.Width);
							}
						}

						// Top-Down -> Bottom-Up
						LPBYTE pBitsLast = pBits + (GetHeight() - 1) * uiDIBScanLineSize;  
						for (i = 0 ; i < (int)GetHeight() / 2 ; i++)
						{
							memcpy(pLine, pBitsLast - i * uiDIBScanLineSize, uiDIBScanLineSize);
							memcpy(pBitsLast - i * uiDIBScanLineSize, pBits + i * uiDIBScanLineSize, uiDIBScanLineSize);
							memcpy(pBits + i * uiDIBScanLineSize, pLine, uiDIBScanLineSize);
						}
					}
					// Non-interlaced image
					else
					{
						int nImageBottom = GetHeight() - m_Gif.m_pLoadFile->Image.Top - m_Gif.m_pLoadFile->Image.Height;
						int progress = 0;
						for (i = GetHeight() - 1 - m_Gif.m_pLoadFile->Image.Top ; i >= nImageBottom ; i--)
						{
							if (::DGifGetLine(m_Gif.m_pLoadFile, pLine, m_Gif.m_pLoadFile->Image.Width) == GIF_ERROR)
							{
								DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
								return -1;
							}
							if (pThread && pThread->DoExit())
							{
								DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
								return -1;
							}
							DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, progress++, m_Gif.m_pLoadFile->Image.Height);
							memcpy(	pBits + i * uiDIBScanLineSize + m_Gif.m_pLoadFile->Image.Left,
									pLine,
									m_Gif.m_pLoadFile->Image.Width);
						}
					}
					DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

					return 1;
				}
				break;
			}

			case EXTENSION_RECORD_TYPE:
			{
				if (::DGifGetExtension(m_Gif.m_pLoadFile, &ExtCode, &pExtension) == GIF_ERROR)
					return -1;
				if (!pExtension)
					break;
				
				BOOL bNetscapeExt = FALSE;
				BOOL bCommentExt = FALSE;
				switch (ExtCode)
				{
					case COMMENT_EXT_FUNC_CODE:
					{
						int nLen = pExtension[0];
						char* pComment = new char[nCommentLen+nLen+1];
						if (m_Gif.m_lpszComment)
							memcpy(pComment, m_Gif.m_lpszComment, nCommentLen);
						memcpy(pComment+nCommentLen, &(pExtension[1]), nLen);
						if (m_Gif.m_lpszComment)
							delete [] m_Gif.m_lpszComment;
						m_Gif.m_lpszComment = pComment;
						nCommentLen += nLen; 
						m_Gif.m_lpszComment[nCommentLen] = '\0';
						bCommentExt = TRUE;
						break;
					}
					case GRAPHICS_EXT_FUNC_CODE:
					{
						m_Gif.m_bHasGraphicExtension = TRUE;
						ASSERT(pExtension[0] == 4);
						m_Gif.m_nFlags = pExtension[1];
						m_Gif.m_nDelay = MAKEWORD(pExtension[2], pExtension[3]);
						m_Gif.m_nTransparencyColorIndex = (m_Gif.m_nFlags & GIF_TRANSPARENT) ? pExtension[4] : GIF_COLORINDEX_NOT_DEFINED;
						if (m_Gif.m_nTransparencyColorIndex != GIF_COLORINDEX_NOT_DEFINED)
							m_FileInfo.m_nNumTransparencyIndexes = 1;
						break;
					}
					case PLAINTEXT_EXT_FUNC_CODE:
					{
						TRACE(_T("GIF89 plaintext\n"));
						break;
					}
					case APPLICATION_EXT_FUNC_CODE:
					{
						if (memcmp(pExtension, CGif::m_szNetscape20ext, CGif::m_szNetscape20ext[0] + 1) == 0)
							bNetscapeExt = TRUE;
						break;
					}
					default:
					{
						TRACE(_T("pExtension record of unknown type\n"));
						break;
					}
				}
				
				do
				{
					if (::DGifGetExtensionNext(m_Gif.m_pLoadFile, &pExtension) == GIF_ERROR)
						return -1;
					
					if (pExtension)
					{
						// Process Netscape 2.0 extension (GIF looping)
						if (bNetscapeExt)
						{
							GifByteType bLength = pExtension[0];
							int iSubCode = pExtension[1] & 0x07;
							if (bLength == 3 && iSubCode == GIF_NSEXT_LOOP)
							{
								m_Gif.m_bHasLoopExtension = TRUE;
								m_Gif.m_wLoopCount = MAKEWORD(pExtension[2], pExtension[3]);
							}
						}
						// Process More Comment
						else if (bCommentExt)
						{
							int nLen = pExtension[0];
							char* pComment = new char[nCommentLen+nLen+1];
							memcpy(pComment, m_Gif.m_lpszComment, nCommentLen);
							memcpy(pComment+nCommentLen, &(pExtension[1]), nLen);
							delete [] m_Gif.m_lpszComment;
							m_Gif.m_lpszComment = pComment;
							nCommentLen += nLen; 
							m_Gif.m_lpszComment[nCommentLen] = '\0';
						}
					}
				}
				while (pExtension);
				break;
			}

			case TERMINATE_RECORD_TYPE:
				break;

			default:
				break;
		}
	}
	while (RecordType != TERMINATE_RECORD_TYPE);

	return 0;
}

BOOL CDib::LoadFirstGIF(LPCTSTR lpszPathName,
						BOOL bOnlyHeader/*=FALSE*/,
						CWnd* pProgressWnd/*=NULL*/,
						BOOL bProgressSend/*=TRUE*/,
						CWorkerThread* pThread/*=NULL*/)
{
	if (bOnlyHeader)
		return LoadGIFHeader(lpszPathName);

	// Free
	m_FileInfo.Clear();
	m_bAlpha = FALSE;
	m_bGrayscale = FALSE;
	Free();

	try
	{
		CString sPathName(lpszPathName);
		if (sPathName.IsEmpty())
			throw (int)GIF_E_ZEROPATH;

		// Check for .gif Extension
		if (::GetFileExt(sPathName) != _T(".gif"))
			throw (int)GIF_E_WRONGEXTENTION;

		CFile file(lpszPathName, CFile::modeRead | CFile::shareDenyWrite);
		m_FileInfo.m_dwFileSize = (DWORD)file.GetLength();
		file.Close();
		if (m_FileInfo.m_dwFileSize == 0)
			throw (int)GIF_E_FILEEMPTY;
	
		// Open gif File
		m_Gif.LoadClose();
		m_Gif.m_pLoadFile = ::DGifOpenFileName(lpszPathName);

		if (m_Gif.m_pLoadFile == NULL)
			throw GIF_E_READ;

		// Allocate BMIs
		m_pBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER) + 256*sizeof(RGBQUAD)];
		m_Gif.m_pScreenBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER) + 256*sizeof(RGBQUAD)];
		if (!m_pBMI || !m_Gif.m_pScreenBMI)
			throw GIF_E_NOMEM;

		// Get Screen Colors
		if (m_Gif.m_pLoadFile->SColorMap)
		{
			m_Gif.m_bHasScreenColorTable = TRUE;
			m_Gif.GetColorMap(m_Gif.m_pLoadFile->SColorMap, m_pBMI);
			m_Gif.GetColorMap(m_Gif.m_pLoadFile->SColorMap, m_Gif.m_pScreenBMI);
			m_Gif.m_nBackgndColorIndex = m_Gif.m_pLoadFile->SBackGroundColor;
			m_FileInfo.m_bHasBackgroundColor = TRUE;
		}
		else
		{
			m_Gif.m_bHasScreenColorTable = FALSE;
			m_pBMI->bmiHeader.biBitCount =      8;
			m_pBMI->bmiHeader.biClrUsed =		256; 
			m_pBMI->bmiHeader.biClrImportant =	0;
			m_Gif.m_nBackgndColorIndex = GIF_COLORINDEX_NOT_DEFINED;
		}

		// DWORD aligned target DIB ScanLineSize 
		DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES(m_Gif.m_pLoadFile->SWidth * m_pBMI->bmiHeader.biBitCount);

		// Init BMIs
		m_pBMI->bmiHeader.biSize =          sizeof(BITMAPINFOHEADER);
		m_pBMI->bmiHeader.biWidth =         m_Gif.m_pLoadFile->SWidth;
		m_pBMI->bmiHeader.biHeight =        m_Gif.m_pLoadFile->SHeight;
		m_pBMI->bmiHeader.biPlanes =        1;
		m_pBMI->bmiHeader.biCompression =   BI_RGB;  // uncompressed
		m_pBMI->bmiHeader.biSizeImage =     m_Gif.m_pLoadFile->SHeight*uiDIBScanLineSize;   
		m_pBMI->bmiHeader.biXPelsPerMeter = 0;
		m_pBMI->bmiHeader.biYPelsPerMeter = 0;
		m_dwImageSize = m_pBMI->bmiHeader.biSizeImage;
		m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + (WORD)(m_pBMI->bmiHeader.biSize));
		memcpy(m_Gif.m_pScreenBMI, m_pBMI, m_pBMI->bmiHeader.biSize);

		// Set Background Color
		if (m_FileInfo.m_bHasBackgroundColor)
		{
			m_FileInfo.m_crBackgroundColor = RGB(	m_pColors[m_Gif.m_nBackgndColorIndex].rgbRed,
													m_pColors[m_Gif.m_nBackgndColorIndex].rgbGreen,
													m_pColors[m_Gif.m_nBackgndColorIndex].rgbBlue);
			m_crBackgroundColor = m_FileInfo.m_crBackgroundColor;
		}

		// Fill-Up a Halftone Color Table if Screen Color Table not available
		if (m_Gif.m_pLoadFile->SColorMap == NULL)
		{
			FillHalftoneColors(m_pColors, 256);
			memcpy(m_Gif.m_pScreenBMI, m_pBMI, m_pBMI->bmiHeader.biSize + GetNumColors()*sizeof(RGBQUAD));
		}

		// Init File Info
		m_FileInfo.m_nType = CFileInfo::GIF;
		m_FileInfo.m_nWidth = m_pBMI->bmiHeader.biWidth;
		m_FileInfo.m_nHeight = m_pBMI->bmiHeader.biHeight;
		m_FileInfo.m_nBpp = m_pBMI->bmiHeader.biBitCount;
		m_FileInfo.m_nCompression = m_pBMI->bmiHeader.biCompression;
		m_FileInfo.m_nColorSpace = CFileInfo::COLORSPACE_RGB;
		m_FileInfo.m_dwImageSize = m_pBMI->bmiHeader.biSizeImage;
		m_FileInfo.m_nXPixsPerMeter = m_pBMI->bmiHeader.biXPelsPerMeter;
		m_FileInfo.m_nYPixsPerMeter = m_pBMI->bmiHeader.biYPelsPerMeter;
		m_FileInfo.m_nNumColors = m_pBMI->bmiHeader.biClrUsed;
		m_FileInfo.m_bPalette = TRUE;
		m_FileInfo.m_nImageCount = 0;
		m_FileInfo.m_nImagePos = 0;

		// Create Palette from BMI
		CreatePaletteFromBMI();

		// Allocate Space for 2 images + 1 Extra Line For Gif Decompression
		m_pBits = (LPBYTE)BIGALLOC(2 * m_dwImageSize + m_Gif.m_pLoadFile->SWidth + SAFETY_BITALLOC_MARGIN);
		if (!m_pBits)
			throw GIF_E_NOMEM;

		// Fill in current and next image with background color
		// Note: if m_nGIFBackgndColorIndex is GIF_COLORINDEX_NOT_DEFINED
		// nothing is filled!
		for (int y = 0 ; y < m_Gif.m_pLoadFile->SHeight * 2 ; y++)
		{
			m_Gif.FillLine(	GetBitCount(), 
							m_pBits + y * uiDIBScanLineSize,
							m_Gif.m_nBackgndColorIndex,
							m_Gif.m_pLoadFile->SWidth);
		}

		// Load First Gif
		m_Gif.m_nLoadImageCount = 0;
		m_Gif.m_wLoopCount = 0;
		m_Gif.m_bHasLoopExtension = FALSE;
		if (LoadNextGIF(pProgressWnd, bProgressSend, pThread) < 0)
			throw (int)GIF_E_GIFLIB;

		return TRUE;
	}
	catch (CFileException* e)
	{
		TCHAR szCause[255];
		CString str(_T("LoadFirstGIF: "));
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
		if (m_Gif.m_pScreenBMI)
		{
			delete [] m_Gif.m_pScreenBMI;
			m_Gif.m_pScreenBMI = NULL;
		}
		if (m_Gif.m_pLoadFile)
		{
			::DGifCloseFile(m_Gif.m_pLoadFile);
			m_Gif.m_pLoadFile = NULL;
		}

		// Just Exit
		if (pThread && pThread->DoExit())
			return FALSE;

		CString str;
#ifdef _DEBUG
		str.Format(_T("LoadFirstGIF(%s):\n"), lpszPathName);
#endif
		switch (error_code)
		{
			case GIF_E_ZEROPATH :		str += _T("The file name is zero\n");
			break;
			case GIF_E_WRONGEXTENTION :	str += _T("The file extention is not .gif\n");
			break;
			case GIF_E_NOMEM :			str += _T("Could not alloc memory\n");
			break;
			case GIF_E_GIFLIB :			str += _T("Cannot load file\n");
			break;
			case GIF_E_READ :			str += _T("Couldn't read GIF file\n");
			break;
			case GIF_E_FILEEMPTY :		str += _T("File is empty\n");
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

//
//  The Following example illustrates how the rows of an interlaced image are
//  ordered.
//  
//        Row Number                                        Interlace Pass
//  
//   0    -----------------------------------------       1
//   1    -----------------------------------------                         4
//   2    -----------------------------------------                   3
//   3    -----------------------------------------                         4
//   4    -----------------------------------------             2
//   5    -----------------------------------------                         4
//   6    -----------------------------------------                   3
//   7    -----------------------------------------                         4
//   8    -----------------------------------------       1
//   9    -----------------------------------------                         4
//   10   -----------------------------------------                   3
//   11   -----------------------------------------                         4
//   12   -----------------------------------------             2
//   13   -----------------------------------------                         4
//   14   -----------------------------------------                   3
//   15   -----------------------------------------                         4
//   16   -----------------------------------------       1
//   17   -----------------------------------------                         4
//   18   -----------------------------------------                   3
//   19   -----------------------------------------                         4
//
// Fetch next image from GIF file
// Returns 1 ok image loaded, 0 for end-of-file (no image loaded), -1 for error
int CDib::LoadNextGIF(	CWnd* pProgressWnd/*=NULL*/,
						BOOL bProgressSend/*=TRUE*/,
						CWorkerThread* pThread/*=NULL*/)
{
	// Error if no gif file!
	if (!m_Gif.m_pLoadFile)
		return -1;

	//					 ___________
	//		pBits1 ->	|			|
	//					|	current	|
	//					|	image	|
	//					|___________|
	//		pBits2 ->	|			|
	//					|	next	|
	//					|	image	|
	//					|___________|
	//		pLine ->	|___________|
	//

	LPBYTE pBits1 = m_pBits;
	LPBYTE pBits2 = pBits1 + m_dwImageSize;
	GifPixelType *pLine = pBits2 + m_dwImageSize;
	GifRecordType RecordType;
	GifByteType *pExtension;

	m_Gif.m_bAlphaUsed = FALSE;
	m_Gif.m_bHasGraphicExtension = FALSE;
	m_Gif.m_nDelay = 10; // Default to 100 msec
	m_Gif.m_nTransparencyColorIndex = GIF_COLORINDEX_NOT_DEFINED;
	m_Gif.m_nFlags = 0;
	if (m_Gif.m_lpszComment)
	{
		delete [] m_Gif.m_lpszComment;
		m_Gif.m_lpszComment = NULL;
	}
	int nCommentLen = 0;

	do
	{
		int i, ExtCode;

		if (::DGifGetRecordType(m_Gif.m_pLoadFile, &RecordType) == GIF_ERROR) 
			break;

		switch (RecordType)
		{
			case IMAGE_DESC_RECORD_TYPE:
			{
				if (::DGifGetImageDesc(m_Gif.m_pLoadFile) != GIF_ERROR)
				{
					// Inc. Loaded Image Count
					++m_Gif.m_nLoadImageCount;
					++m_FileInfo.m_nImageCount;
					m_FileInfo.m_nImagePos = m_FileInfo.m_nImageCount - 1;

					// Allocate (if necessary) & Init Current Image BMI
					if (!m_Gif.m_pCurrentImageBMI)
						m_Gif.m_pCurrentImageBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER) + 256*sizeof(RGBQUAD)];
					if (!m_Gif.m_pCurrentImageBMI)
						return -1;
					int nColorsCount = m_Gif.m_pLoadFile->SColorMap ? m_Gif.m_pLoadFile->SColorMap->ColorCount : 256;
					if (nColorsCount >= 256)
						nColorsCount = 256;
					memcpy(m_pBMI, m_Gif.m_pScreenBMI,
								sizeof(BITMAPINFOHEADER) + nColorsCount*sizeof(RGBQUAD));
					if (m_Gif.m_pLoadFile->Image.ColorMap == NULL)
					{
						m_Gif.m_bHasImageColorTable = FALSE;
					}
					else
					{
						m_Gif.m_bHasImageColorTable = TRUE;
						m_Gif.GetColorMap(m_Gif.m_pLoadFile->Image.ColorMap, m_pBMI);
					}
					DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES(	m_pBMI->bmiHeader.biWidth *
																	m_pBMI->bmiHeader.biBitCount);
					m_pBMI->bmiHeader.biSizeImage = m_pBMI->bmiHeader.biHeight*uiDIBScanLineSize;
					m_dwImageSize = m_pBMI->bmiHeader.biSizeImage;
					m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + (WORD)(m_pBMI->bmiHeader.biSize));
					memcpy(m_Gif.m_pCurrentImageBMI, m_pBMI,
								sizeof(BITMAPINFOHEADER) + 256*sizeof(RGBQUAD));

					// Always copy next -> current image
					memcpy(pBits1, pBits2, m_dwImageSize);

					m_Gif.m_nLeft = m_Gif.m_pLoadFile->Image.Left;
					m_Gif.m_nTop = m_Gif.m_pLoadFile->Image.Top;
					m_Gif.m_nWidth = m_Gif.m_pLoadFile->Image.Width;
					m_Gif.m_nHeight = m_Gif.m_pLoadFile->Image.Height;
					DIB_INIT_PROGRESS;

					// Interlaced image
					if (m_Gif.m_pLoadFile->Image.Interlace)
					{
						// Need to perform 4 passes on the images:
						for (int pass = 0; pass < 4; pass++)
						{
							for (i = CGif::m_InterlacedOffset[pass] ; i < m_Gif.m_pLoadFile->Image.Height ; i += CGif::m_InterlacedJumps[pass])
							{
								if (::DGifGetLine(m_Gif.m_pLoadFile, pLine, m_Gif.m_pLoadFile->Image.Width) == GIF_ERROR)
								{
									DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
									return -1;
								}
								if ((i & 0x7) == 0)
								{
									if (pThread && pThread->DoExit())
									{
										DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
										return -1;
									}
									DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, i, m_Gif.m_pLoadFile->Image.Height);
								}
								m_Gif.CopyLine(	GetBitCount(),
												m_bAlpha,
												pBits1 + (m_Gif.m_pLoadFile->Image.Top + i) * uiDIBScanLineSize + m_Gif.m_pLoadFile->Image.Left,
												pLine,
												m_Gif.m_pLoadFile->Image.Width,
												m_Gif.m_nTransparencyColorIndex,
												m_Gif.m_bAlphaUsed);
							}
						}

						// Top-Down -> Bottom-Up
						LPBYTE pBits1Last = pBits1 + (GetHeight() - 1) * uiDIBScanLineSize;  
						for (i = 0 ; i < (int)GetHeight() / 2 ; i++)
						{
							memcpy(pLine, pBits1Last - i * uiDIBScanLineSize, uiDIBScanLineSize);
							memcpy(pBits1Last - i * uiDIBScanLineSize, pBits1 + i * uiDIBScanLineSize, uiDIBScanLineSize);
							memcpy(pBits1 + i * uiDIBScanLineSize, pLine, uiDIBScanLineSize);
						}
					}
					// Non-interlaced image
					else
					{
						int nImageBottom = GetHeight() - m_Gif.m_pLoadFile->Image.Top - m_Gif.m_pLoadFile->Image.Height;
						int progress = 0;
						for (i = GetHeight() - 1 - m_Gif.m_pLoadFile->Image.Top ; i >= nImageBottom ; i--)
						{
							if (::DGifGetLine(m_Gif.m_pLoadFile, pLine, m_Gif.m_pLoadFile->Image.Width) == GIF_ERROR)
							{
								DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
								return -1;
							}
							if (pThread && pThread->DoExit())
							{
								DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
								return -1;
							}
							DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, progress++, m_Gif.m_pLoadFile->Image.Height);
							m_Gif.CopyLine(	GetBitCount(),
											m_bAlpha,
											pBits1 + i * uiDIBScanLineSize + m_Gif.m_pLoadFile->Image.Left,
											pLine,
											m_Gif.m_pLoadFile->Image.Width,
											m_Gif.m_nTransparencyColorIndex,
											m_Gif.m_bAlphaUsed);
						}
					}
					DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

					// Prepare second image with next starting
					if (m_Gif.GetDispose() == GIF_DISPOSE_BACKGND)
					{
						// Clear next image to background index
						for (i = 0 ; i < m_Gif.m_pLoadFile->Image.Height ; i++)
						{
							m_Gif.FillLine(	GetBitCount(),
											pBits2 + (m_Gif.m_pLoadFile->Image.Top + i) * uiDIBScanLineSize +  m_Gif.m_pLoadFile->Image.Left,
											m_Gif.m_nBackgndColorIndex,
											m_Gif.m_pLoadFile->Image.Width);
						}
					}
					else if (m_Gif.GetDispose() != GIF_DISPOSE_RESTORE)
					{
						// Copy current -> next (Update)
						memcpy(pBits2, pBits1, m_dwImageSize);
					}

					return 1;
				}
				break;
			}

			case EXTENSION_RECORD_TYPE:
			{
				if (::DGifGetExtension(m_Gif.m_pLoadFile, &ExtCode, &pExtension) == GIF_ERROR)
					return -1;
				if (!pExtension)
					break;
				
				BOOL bNetscapeExt = FALSE;
				BOOL bCommentExt = FALSE;
				switch (ExtCode)
				{
					case COMMENT_EXT_FUNC_CODE:
					{
						int nLen = pExtension[0];
						char* pComment = new char[nCommentLen+nLen+1];
						if (m_Gif.m_lpszComment)
							memcpy(pComment, m_Gif.m_lpszComment, nCommentLen);
						memcpy(pComment+nCommentLen, &(pExtension[1]), nLen);
						if (m_Gif.m_lpszComment)
							delete [] m_Gif.m_lpszComment;
						m_Gif.m_lpszComment = pComment;
						nCommentLen += nLen; 
						m_Gif.m_lpszComment[nCommentLen] = '\0';
						bCommentExt = TRUE;
						break;
					}
					case GRAPHICS_EXT_FUNC_CODE:
					{
						m_Gif.m_bHasGraphicExtension = TRUE;
						ASSERT(pExtension[0] == 4);
						m_Gif.m_nFlags = pExtension[1];
						m_Gif.m_nDelay = MAKEWORD(pExtension[2], pExtension[3]);
						m_Gif.m_nTransparencyColorIndex = (m_Gif.m_nFlags & GIF_TRANSPARENT) ? pExtension[4] : GIF_COLORINDEX_NOT_DEFINED;
						if (m_Gif.m_nTransparencyColorIndex != GIF_COLORINDEX_NOT_DEFINED)
							m_FileInfo.m_nNumTransparencyIndexes = 1;
						break;
					}
					case PLAINTEXT_EXT_FUNC_CODE:
					{
						TRACE(_T("GIF89 plaintext\n"));
						break;
					}
					case APPLICATION_EXT_FUNC_CODE:
					{
						if (memcmp(pExtension, CGif::m_szNetscape20ext, CGif::m_szNetscape20ext[0] + 1) == 0)
							bNetscapeExt = TRUE;
						break;
					}
					default:
					{
						TRACE(_T("pExtension record of unknown type\n"));
						break;
					}
				}
				
				do
				{
					if (::DGifGetExtensionNext(m_Gif.m_pLoadFile, &pExtension) == GIF_ERROR)
						return -1;
					
					if (pExtension)
					{
						// Process Netscape 2.0 extension (GIF looping)
						if (bNetscapeExt)
						{
							GifByteType bLength = pExtension[0];
							int iSubCode = pExtension[1] & 0x07;
							if (bLength == 3 && iSubCode == GIF_NSEXT_LOOP)
							{
								m_Gif.m_bHasLoopExtension = TRUE;
								m_Gif.m_wLoopCount = MAKEWORD(pExtension[2], pExtension[3]);
							}
						}
						// Process More Comment
						else if (bCommentExt)
						{
							int nLen = pExtension[0];
							char* pComment = new char[nCommentLen+nLen+1];
							memcpy(pComment, m_Gif.m_lpszComment, nCommentLen);
							memcpy(pComment+nCommentLen, &(pExtension[1]), nLen);
							delete [] m_Gif.m_lpszComment;
							m_Gif.m_lpszComment = pComment;
							nCommentLen += nLen; 
							m_Gif.m_lpszComment[nCommentLen] = '\0';
						}
					}
				}
				while (pExtension);
				break;
			}

			case TERMINATE_RECORD_TYPE:
				break;

			default:
				break;
		}
	}
	while (RecordType != TERMINATE_RECORD_TYPE);

	return 0;
}

BOOL CDib::LoadFirstGIF32(	LPCTSTR lpszPathName,
							BOOL bOnlyHeader/*=FALSE*/,
							CWnd* pProgressWnd/*=NULL*/,
							BOOL bProgressSend/*=TRUE*/,
							CWorkerThread* pThread/*=NULL*/)
{
	if (bOnlyHeader)
		return LoadGIFHeader(lpszPathName);

	// Free
	m_FileInfo.Clear();
	m_bAlpha = FALSE;
	m_bGrayscale = FALSE;
	Free();

	try
	{
		CString sPathName(lpszPathName);
		if (sPathName.IsEmpty())
			throw (int)GIF_E_ZEROPATH;

		// Check for .gif Extension
		if (::GetFileExt(sPathName) != _T(".gif"))
			throw (int)GIF_E_WRONGEXTENTION;

		CFile file(lpszPathName, CFile::modeRead | CFile::shareDenyWrite);
		m_FileInfo.m_dwFileSize = (DWORD)file.GetLength();
		file.Close();
		if (m_FileInfo.m_dwFileSize == 0)
			throw (int)GIF_E_FILEEMPTY;
	
		// Open gif File
		m_Gif.LoadClose();
		m_Gif.m_pLoadFile = ::DGifOpenFileName(lpszPathName);
		if (m_Gif.m_pLoadFile == NULL)
			throw GIF_E_READ;

		// DWORD aligned target DIB ScanLineSizes32
		DWORD uiDIBScanLineSize32 = DWALIGNEDWIDTHBYTES(m_Gif.m_pLoadFile->SWidth * 32);

		// Allocate & Init the 32 bpp BMI
		m_pBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER)];
		if (!m_pBMI)
			throw GIF_E_NOMEM;
		m_pBMI->bmiHeader.biSize =          sizeof(BITMAPINFOHEADER);
		m_pBMI->bmiHeader.biWidth =         m_Gif.m_pLoadFile->SWidth;
		m_pBMI->bmiHeader.biHeight =        m_Gif.m_pLoadFile->SHeight;
		m_pBMI->bmiHeader.biPlanes =        1;
		m_pBMI->bmiHeader.biBitCount =      32;
		m_pBMI->bmiHeader.biCompression =   BI_RGB;  // uncompressed
		m_pBMI->bmiHeader.biSizeImage =     m_Gif.m_pLoadFile->SHeight*uiDIBScanLineSize32;   
		m_pBMI->bmiHeader.biXPelsPerMeter = 0;
		m_pBMI->bmiHeader.biYPelsPerMeter = 0;
		m_pBMI->bmiHeader.biClrUsed =		0; 
		m_pBMI->bmiHeader.biClrImportant =	0;
		m_dwImageSize = m_Gif.m_pLoadFile->SHeight*uiDIBScanLineSize32;
		m_pColors = NULL;
		InitMasks();

		// Allocate Screen BMI
		m_Gif.m_pScreenBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER) + 256*sizeof(RGBQUAD)];
		if (!m_Gif.m_pScreenBMI)
			throw GIF_E_NOMEM;
		
		// Get Screen Colors
		if (m_Gif.m_pLoadFile->SColorMap)
		{
			m_Gif.m_bHasScreenColorTable = TRUE;
			m_Gif.GetColorMap(m_Gif.m_pLoadFile->SColorMap, m_Gif.m_pScreenBMI);
			m_Gif.m_nBackgndColorIndex = m_Gif.m_pLoadFile->SBackGroundColor;
			m_FileInfo.m_bHasBackgroundColor = TRUE;
		}
		else
		{
			m_Gif.m_bHasScreenColorTable = FALSE;
			m_Gif.m_pScreenBMI->bmiHeader.biBitCount =      8;
			m_Gif.m_pScreenBMI->bmiHeader.biClrUsed =		256; 
			m_Gif.m_pScreenBMI->bmiHeader.biClrImportant =	0;
			m_Gif.m_nBackgndColorIndex = GIF_COLORINDEX_NOT_DEFINED;
		}

		// DWORD aligned target DIB ScanLineSize 
		DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES(m_Gif.m_pLoadFile->SWidth * m_Gif.m_pScreenBMI->bmiHeader.biBitCount);

		// Init Screen BMI
		m_Gif.m_pScreenBMI->bmiHeader.biSize =          sizeof(BITMAPINFOHEADER);
		m_Gif.m_pScreenBMI->bmiHeader.biWidth =         m_Gif.m_pLoadFile->SWidth;
		m_Gif.m_pScreenBMI->bmiHeader.biHeight =        m_Gif.m_pLoadFile->SHeight;
		m_Gif.m_pScreenBMI->bmiHeader.biPlanes =        1;
		m_Gif.m_pScreenBMI->bmiHeader.biCompression =   BI_RGB;  // uncompressed
		m_Gif.m_pScreenBMI->bmiHeader.biSizeImage =     m_Gif.m_pLoadFile->SHeight*uiDIBScanLineSize;   
		m_Gif.m_pScreenBMI->bmiHeader.biXPelsPerMeter = 0;
		m_Gif.m_pScreenBMI->bmiHeader.biYPelsPerMeter = 0;
		RGBQUAD* pScreenColors = (RGBQUAD*)((LPBYTE)m_Gif.m_pScreenBMI + (WORD)(m_Gif.m_pScreenBMI->bmiHeader.biSize));

		// Set Background Color
		if (m_FileInfo.m_bHasBackgroundColor)
		{
			m_FileInfo.m_crBackgroundColor = RGB(	pScreenColors[m_Gif.m_nBackgndColorIndex].rgbRed,
													pScreenColors[m_Gif.m_nBackgndColorIndex].rgbGreen,
													pScreenColors[m_Gif.m_nBackgndColorIndex].rgbBlue);
			m_crBackgroundColor = m_FileInfo.m_crBackgroundColor;
		}

		// Fill-Up a Halftone Color Table if Screen Color Table not available
		if (m_Gif.m_pLoadFile->SColorMap == NULL)
		{
			FillHalftoneColors(pScreenColors, 256);
		}

		// Init File Info
		m_FileInfo.m_nType = CFileInfo::GIF;
		m_FileInfo.m_nWidth = m_pBMI->bmiHeader.biWidth;
		m_FileInfo.m_nHeight = m_pBMI->bmiHeader.biHeight;
		m_FileInfo.m_nBpp = m_Gif.m_pScreenBMI->bmiHeader.biBitCount;
		m_FileInfo.m_nCompression = m_pBMI->bmiHeader.biCompression;
		m_FileInfo.m_nColorSpace = CFileInfo::COLORSPACE_RGB;
		m_FileInfo.m_dwImageSize = m_Gif.m_pScreenBMI->bmiHeader.biSizeImage;
		m_FileInfo.m_nXPixsPerMeter = m_pBMI->bmiHeader.biXPelsPerMeter;
		m_FileInfo.m_nYPixsPerMeter = m_pBMI->bmiHeader.biYPelsPerMeter;
		m_FileInfo.m_nNumColors = m_Gif.m_pScreenBMI->bmiHeader.biClrUsed;
		m_FileInfo.m_bPalette = TRUE;
		m_FileInfo.m_nImageCount = 0;
		m_FileInfo.m_nImagePos = 0;

		// Create Palette from BMI
		CreatePaletteFromBMI();

		// Allocate Space for 2 x 32 bpp images + 1 Extra Line For Gif Decompression
		m_pBits = (LPBYTE)BIGALLOC(2 * m_dwImageSize + uiDIBScanLineSize32 + SAFETY_BITALLOC_MARGIN);
		if (!m_pBits)
			throw GIF_E_NOMEM;

		// Fill in current and next image with background color
		if (m_Gif.m_nBackgndColorIndex >= 0)
		{
			for (int y = 0 ; y < m_Gif.m_pLoadFile->SHeight * 2 ; y++)
			{
				m_Gif.FillLine(	32, 
								m_pBits + y * uiDIBScanLineSize32,
								m_Gif.m_nBackgndColorIndex,
								m_Gif.m_pLoadFile->SWidth,
								pScreenColors);
			}
		}

		// Load First Gif
		m_Gif.m_nLoadImageCount = 0;
		m_Gif.m_wLoopCount = 0;
		m_Gif.m_bHasLoopExtension = FALSE;
		if (LoadNextGIF32(pProgressWnd, bProgressSend, pThread) < 0)
			throw (int)GIF_E_GIFLIB;

		return TRUE;
	}
	catch (CFileException* e)
	{
		TCHAR szCause[255];
		CString str(_T("LoadFirstGIF32: "));
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
		if (m_Gif.m_pScreenBMI)
		{
			delete [] m_Gif.m_pScreenBMI;
			m_Gif.m_pScreenBMI = NULL;
		}
		if (m_Gif.m_pLoadFile)
		{
			::DGifCloseFile(m_Gif.m_pLoadFile);
			m_Gif.m_pLoadFile = NULL;
		}

		// Just Exit
		if (pThread && pThread->DoExit())
			return FALSE;

		CString str;
#ifdef _DEBUG
		str.Format(_T("LoadFirstGIF32(%s):\n"), lpszPathName);
#endif
		switch (error_code)
		{
			case GIF_E_ZEROPATH :		str += _T("The file name is zero\n");
			break;
			case GIF_E_WRONGEXTENTION :	str += _T("The file extention is not .gif\n");
			break;
			case GIF_E_NOMEM :			str += _T("Could not alloc memory\n");
			break;
			case GIF_E_GIFLIB :			str += _T("Cannot load file\n");
			break;
			case GIF_E_READ :			str += _T("Couldn't read GIF file\n");
			break;
			case GIF_E_FILEEMPTY :		str += _T("File is empty\n");
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

//
//  The Following example illustrates how the rows of an interlaced image are
//  ordered.
//  
//        Row Number                                        Interlace Pass
//  
//   0    -----------------------------------------       1
//   1    -----------------------------------------                         4
//   2    -----------------------------------------                   3
//   3    -----------------------------------------                         4
//   4    -----------------------------------------             2
//   5    -----------------------------------------                         4
//   6    -----------------------------------------                   3
//   7    -----------------------------------------                         4
//   8    -----------------------------------------       1
//   9    -----------------------------------------                         4
//   10   -----------------------------------------                   3
//   11   -----------------------------------------                         4
//   12   -----------------------------------------             2
//   13   -----------------------------------------                         4
//   14   -----------------------------------------                   3
//   15   -----------------------------------------                         4
//   16   -----------------------------------------       1
//   17   -----------------------------------------                         4
//   18   -----------------------------------------                   3
//   19   -----------------------------------------                         4
//
// Fetch next image from GIF file
// Returns 1 ok image loaded, 0 for end-of-file (no image loaded), -1 for error
int CDib::LoadNextGIF32(CWnd* pProgressWnd/*=NULL*/,
						BOOL bProgressSend/*=TRUE*/,
						CWorkerThread* pThread/*=NULL*/)
{
	// Error if no gif file!
	if (!m_Gif.m_pLoadFile)
		return -1;

	//					 ___________
	//		pBits1 ->	|			|
	//					|	current	|
	//					|	image	|
	//					|___________|
	//		pBits2 ->	|			|
	//					|	next	|
	//					|	image	|
	//					|___________|
	//		pLine ->	|___________|
	//

	LPBYTE pBits1 = m_pBits;
	LPBYTE pBits2 = pBits1 + m_dwImageSize;
	GifPixelType* pLine = pBits2 + m_dwImageSize;
	GifRecordType RecordType;
	GifByteType* pExtension;

	m_Gif.m_bAlphaUsed = FALSE;
	m_Gif.m_bHasGraphicExtension = FALSE;
	m_Gif.m_nDelay = 10; // Default to 100 msec
	m_Gif.m_nTransparencyColorIndex = GIF_COLORINDEX_NOT_DEFINED;
	m_Gif.m_nFlags = 0;
	if (m_Gif.m_lpszComment)
	{
		delete [] m_Gif.m_lpszComment;
		m_Gif.m_lpszComment = NULL;
	}
	int nCommentLen = 0;

	do
	{
		int i, ExtCode;

		if (::DGifGetRecordType(m_Gif.m_pLoadFile, &RecordType) == GIF_ERROR) 
			break;

		switch (RecordType)
		{
			case IMAGE_DESC_RECORD_TYPE:
			{
				if (::DGifGetImageDesc(m_Gif.m_pLoadFile) != GIF_ERROR)
				{
					// Inc. Loaded Image Count
					++m_Gif.m_nLoadImageCount;
					++m_FileInfo.m_nImageCount;
					m_FileInfo.m_nImagePos = m_FileInfo.m_nImageCount - 1;

					// Allocate (if necessary) & Init Current Image BMI
					if (!m_Gif.m_pCurrentImageBMI)
						m_Gif.m_pCurrentImageBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER) + 256*sizeof(RGBQUAD)];
					if (!m_Gif.m_pCurrentImageBMI)
						return -1;
					int nColorsCount = m_Gif.m_pLoadFile->SColorMap ? m_Gif.m_pLoadFile->SColorMap->ColorCount : 256;
					if (nColorsCount >= 256)
						nColorsCount = 256;
					memcpy(m_Gif.m_pCurrentImageBMI, m_Gif.m_pScreenBMI,
							sizeof(BITMAPINFOHEADER) + nColorsCount*sizeof(RGBQUAD));
					if (m_Gif.m_pLoadFile->Image.ColorMap == NULL)
					{
						m_Gif.m_bHasImageColorTable = FALSE;
					}
					else
					{
						m_Gif.m_bHasImageColorTable = TRUE;
						m_Gif.GetColorMap(m_Gif.m_pLoadFile->Image.ColorMap, m_Gif.m_pCurrentImageBMI);
					}
					DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES(	m_Gif.m_pCurrentImageBMI->bmiHeader.biWidth *
																	m_Gif.m_pCurrentImageBMI->bmiHeader.biBitCount);
					m_Gif.m_pCurrentImageBMI->bmiHeader.biSizeImage = m_Gif.m_pCurrentImageBMI->bmiHeader.biHeight*uiDIBScanLineSize;
					RGBQUAD* pImageColors = (RGBQUAD*)((LPBYTE)m_Gif.m_pCurrentImageBMI + (WORD)(m_Gif.m_pCurrentImageBMI->bmiHeader.biSize));

					// Always copy next -> current image
					memcpy(pBits1, pBits2, m_dwImageSize);

					m_Gif.m_nLeft = m_Gif.m_pLoadFile->Image.Left;
					m_Gif.m_nTop = m_Gif.m_pLoadFile->Image.Top;
					m_Gif.m_nWidth = m_Gif.m_pLoadFile->Image.Width;
					m_Gif.m_nHeight = m_Gif.m_pLoadFile->Image.Height;
					DWORD uiDIBScanLineSize32 = DWALIGNEDWIDTHBYTES(m_pBMI->bmiHeader.biWidth * 32);
					DIB_INIT_PROGRESS;

					// Interlaced image
					if (m_Gif.m_pLoadFile->Image.Interlace)
					{
						// Need to perform 4 passes on the images:
						for (int pass = 0; pass < 4; pass++)
						{
							for (i = CGif::m_InterlacedOffset[pass] ; i < m_Gif.m_pLoadFile->Image.Height ; i += CGif::m_InterlacedJumps[pass])
							{
								if (::DGifGetLine(m_Gif.m_pLoadFile, pLine, m_Gif.m_pLoadFile->Image.Width) == GIF_ERROR)
								{
									DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
									return -1;
								}
								if ((i & 0x7) == 0)
								{
									if (pThread && pThread->DoExit())
									{
										DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
										return -1;
									}
									DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, i, m_Gif.m_pLoadFile->Image.Height);
								}
								m_Gif.CopyLine(	32,
												m_bAlpha,
												pBits1 + (m_Gif.m_pLoadFile->Image.Top + i) * uiDIBScanLineSize32 +
													4 * m_Gif.m_pLoadFile->Image.Left,
												pLine,
												m_Gif.m_pLoadFile->Image.Width,
												m_Gif.m_nTransparencyColorIndex,
												m_Gif.m_bAlphaUsed,
												pImageColors);
							}
						}

						// Top-Down -> Bottom-Up
						LPBYTE pBits1Last = pBits1 + (GetHeight() - 1) * uiDIBScanLineSize32;  
						for (i = 0 ; i < (int)GetHeight() / 2 ; i++)
						{
							memcpy(pLine, pBits1Last - i * uiDIBScanLineSize32, uiDIBScanLineSize32);
							memcpy(pBits1Last - i * uiDIBScanLineSize32, pBits1 + i * uiDIBScanLineSize32, uiDIBScanLineSize32);
							memcpy(pBits1 + i * uiDIBScanLineSize32, pLine, uiDIBScanLineSize32);
						}
					}
					// Non-interlaced image
					else
					{
						int nImageBottom = GetHeight() - m_Gif.m_pLoadFile->Image.Top - m_Gif.m_pLoadFile->Image.Height;
						int progress = 0;
						for (i = GetHeight() - 1 - m_Gif.m_pLoadFile->Image.Top ; i >= nImageBottom ; i--)
						{
							if (::DGifGetLine(m_Gif.m_pLoadFile, pLine, m_Gif.m_pLoadFile->Image.Width) == GIF_ERROR)
							{
								DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
								return -1;
							}
							if (pThread && pThread->DoExit())
							{
								DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
								return -1;
							}
							DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, progress++, m_Gif.m_pLoadFile->Image.Height);
							m_Gif.CopyLine(	32,
											m_bAlpha,
											pBits1 + i * uiDIBScanLineSize32 +
												4 * m_Gif.m_pLoadFile->Image.Left,
											pLine,
											m_Gif.m_pLoadFile->Image.Width,
											m_Gif.m_nTransparencyColorIndex,
											m_Gif.m_bAlphaUsed,
											pImageColors);
						}
					}
					DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

					// Prepare second image with next starting
					if (m_Gif.GetDispose() == GIF_DISPOSE_BACKGND)
					{
						// Clear next image to background index
						if (m_Gif.m_nBackgndColorIndex >= 0)
						{
							for (i = 0 ; i < m_Gif.m_pLoadFile->Image.Height ; i++)
							{
								m_Gif.FillLine(	32,
												pBits2 + (m_Gif.m_pLoadFile->Image.Top + i) * uiDIBScanLineSize32 +
													4 * m_Gif.m_pLoadFile->Image.Left,
												m_Gif.m_nBackgndColorIndex,
												m_Gif.m_pLoadFile->Image.Width,
												pImageColors);
							}
						}
					}
					else if (m_Gif.GetDispose() != GIF_DISPOSE_RESTORE)
					{
						// Copy current -> next (Update)
						memcpy(pBits2, pBits1, m_dwImageSize);
					}

					return 1;
				}
				break;
			}

			case EXTENSION_RECORD_TYPE:
			{
				if (::DGifGetExtension(m_Gif.m_pLoadFile, &ExtCode, &pExtension) == GIF_ERROR)
					return -1;
				if (!pExtension)
					break;
				
				BOOL bNetscapeExt = FALSE;
				BOOL bCommentExt = FALSE;
				switch (ExtCode)
				{
					case COMMENT_EXT_FUNC_CODE:
					{
						int nLen = pExtension[0];
						char* pComment = new char[nCommentLen+nLen+1];
						if (m_Gif.m_lpszComment)
							memcpy(pComment, m_Gif.m_lpszComment, nCommentLen);
						memcpy(pComment+nCommentLen, &(pExtension[1]), nLen);
						if (m_Gif.m_lpszComment)
							delete [] m_Gif.m_lpszComment;
						m_Gif.m_lpszComment = pComment;
						nCommentLen += nLen; 
						m_Gif.m_lpszComment[nCommentLen] = '\0';
						bCommentExt = TRUE;
						break;
					}
					case GRAPHICS_EXT_FUNC_CODE:
					{
						m_Gif.m_bHasGraphicExtension = TRUE;
						ASSERT(pExtension[0] == 4);
						m_Gif.m_nFlags = pExtension[1];
						m_Gif.m_nDelay = MAKEWORD(pExtension[2], pExtension[3]);
						m_Gif.m_nTransparencyColorIndex = (m_Gif.m_nFlags & GIF_TRANSPARENT) ? pExtension[4] : GIF_COLORINDEX_NOT_DEFINED;
						if (m_Gif.m_nTransparencyColorIndex != GIF_COLORINDEX_NOT_DEFINED)
						{
							m_bAlpha = TRUE;
							m_FileInfo.m_nNumTransparencyIndexes = 1;
						}
						break;
					}
					case PLAINTEXT_EXT_FUNC_CODE:
					{
						TRACE(_T("GIF89 plaintext\n"));
						break;
					}
					case APPLICATION_EXT_FUNC_CODE:
					{
						if (memcmp(pExtension, CGif::m_szNetscape20ext, CGif::m_szNetscape20ext[0] + 1) == 0)
							bNetscapeExt = TRUE;
						break;
					}
					default:
					{
						TRACE(_T("pExtension record of unknown type\n"));
						break;
					}
				}
				
				do
				{
					if (::DGifGetExtensionNext(m_Gif.m_pLoadFile, &pExtension) == GIF_ERROR)
						return -1;
					
					if (pExtension)
					{
						// Process Netscape 2.0 extension (GIF looping)
						if (bNetscapeExt)
						{
							GifByteType bLength = pExtension[0];
							int iSubCode = pExtension[1] & 0x07;
							if (bLength == 3 && iSubCode == GIF_NSEXT_LOOP)
							{
								m_Gif.m_bHasLoopExtension = TRUE;
								m_Gif.m_wLoopCount = MAKEWORD(pExtension[2], pExtension[3]);
							}
						}
						// Process More Comment
						else if (bCommentExt)
						{
							int nLen = pExtension[0];
							char* pComment = new char[nCommentLen+nLen+1];
							memcpy(pComment, m_Gif.m_lpszComment, nCommentLen);
							memcpy(pComment+nCommentLen, &(pExtension[1]), nLen);
							delete [] m_Gif.m_lpszComment;
							m_Gif.m_lpszComment = pComment;
							nCommentLen += nLen; 
							m_Gif.m_lpszComment[nCommentLen] = '\0';
						}
					}
				}
				while (pExtension);
				break;
			}

			case TERMINATE_RECORD_TYPE:
				break;

			default:
				break;
		}
	}
	while (RecordType != TERMINATE_RECORD_TYPE);

	return 0;
}

int CDib::LogNumColors(int nNumColors)
{
	if (nNumColors > 128)
		return 8;
	else if (nNumColors > 64)
		return 7;
	else if (nNumColors > 32)
		return 6;
	else if (nNumColors > 16)
		return 5;
	else if (nNumColors > 8)
		return 4;
	else if (nNumColors > 4)
		return 3;
	else if (nNumColors > 2)
		return 2;
	else
		return 1;
}

BOOL CDib::SaveGIF(	LPCTSTR lpszPathName,
					int nTransparencyColorIndex/*=GIF_COLORINDEX_NOT_DEFINED*/,
					CWnd* pProgressWnd/*=NULL*/,
					BOOL bProgressSend/*=TRUE*/,
					CWorkerThread* pThread/*=NULL*/)
{
	GifByteType* OutputBuffer = NULL;
	ColorMapObject* OutputColorMap = NULL;
	GifByteType* RedBuffer = NULL;
	GifByteType* GreenBuffer = NULL;
	GifByteType* BlueBuffer = NULL;
	BYTE R, G, B;
	char Version[] = "87a";

	try
	{
		CString sPathName(lpszPathName);
		if (sPathName.IsEmpty())
			throw (int)GIF_E_ZEROPATH;

		DWORD dwAttrib = ::GetFileAttributes(lpszPathName);
		if ((dwAttrib != 0xFFFFFFFF) && (dwAttrib & FILE_ATTRIBUTE_READONLY))
			throw (int)GIF_E_FILEREADONLY;

		if (!m_pBits)
		{
			if (!DibSectionToBits())
				throw (int)GIF_E_BADBMP;
		}

		if (!m_pBits || !m_pBMI)
			throw (int)GIF_E_BADBMP;

		if (IsCompressed())
#ifndef _WIN32_WCE
			if (!Decompress(GetBitCount())) // Decompress
				throw (int)GIF_E_BADBMP;
#else
			throw (int)GIF_E_BADBMP;
#endif

		// Scan Line Sizes
		DWORD uiInputDIBScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * GetBitCount());

		// In the gif file the scan lines are top / down 
		// In a DIB the scan lines are stored bottom / up
		LPBYTE lpBits = m_pBits;
		lpBits += uiInputDIBScanLineSize*(GetHeight()-1);

		// Allocate Output Buffer
		if ((OutputBuffer = (GifByteType*) new GifByteType[GetWidth() * GetHeight()]) == NULL)
			throw (int)GIF_E_NOMEM;

		// Transparency
		if (nTransparencyColorIndex != GIF_COLORINDEX_NOT_DEFINED)
		{
			m_Gif.m_bHasGraphicExtension = TRUE;
			m_Gif.SetTransparency(TRUE);
			m_Gif.SetTransparencyColorIndex(nTransparencyColorIndex);
			m_Gif.SetDispose(GIF_DISPOSE_NONE);
			m_Gif.SetDelay(100); // Default to 100 msec
			m_Gif.SetUserInput(FALSE);
			strcpy(Version, "89a");
		}
		else
			m_Gif.m_bHasGraphicExtension = FALSE;
		m_Gif.m_bHasScreenColorTable = TRUE;
		m_Gif.m_bHasImageColorTable = FALSE;
		m_Gif.m_bHasLoopExtension = FALSE;
		m_Gif.m_nLeft = 0;
		m_Gif.m_nTop = 0;
		m_Gif.m_nWidth = GetWidth();
		m_Gif.m_nHeight = GetHeight();

		// Do Save
		switch (GetBitCount())
		{
			case 1:
			{
				if ((OutputColorMap = ::MakeMapObject(1 << LogNumColors(GetNumColors())/*has to be a power of two*/, NULL)) == NULL)
					throw (int)GIF_E_NOMEM;
				m_Gif.SetColorMap(OutputColorMap, m_pBMI);

				for (unsigned int line = 0 ; line < GetHeight() ; line++)
				{
					for (unsigned int i = 0 ; i < GetWidth() ; i++)
					{
						OutputBuffer[line * GetWidth() + i] = (lpBits[i/8] >> (7-(i%8))) & 0x01;
					}
					lpBits -= uiInputDIBScanLineSize;
				}

				if (!m_Gif.SavePrepare(	lpszPathName,
										OutputColorMap,
										LogNumColors(GetNumColors()),
										GetWidth(),
										GetHeight(),
										Version))
					throw (int)GIF_E_GIFLIB;
				if (!m_Gif.Save(OutputBuffer,
								GetWidth(),
								NULL, // No Color Map
								LogNumColors(GetNumColors()),
								m_Gif.m_nLeft,
								m_Gif.m_nTop,
								m_Gif.m_nWidth,
								m_Gif.m_nHeight,
								pProgressWnd,
								bProgressSend,
								pThread))
					throw (int)GIF_E_GIFLIB;

				break;
			}
			case 4:
			{
				if ((OutputColorMap = ::MakeMapObject(1 << LogNumColors(GetNumColors())/*has to be a power of two*/, NULL)) == NULL)
					throw (int)GIF_E_NOMEM;
				m_Gif.SetColorMap(OutputColorMap, m_pBMI);

				for (unsigned int line = 0 ; line < GetHeight() ; line++)
				{
					for (unsigned int i = 0 ; i < GetWidth() ; i++)
					{
						if (i%2) // LS Nibble
							OutputBuffer[line * GetWidth() + i] = lpBits[i/2] & 0x0F;
						else // MS Nibble
							OutputBuffer[line * GetWidth() + i] = lpBits[i/2] >> 4;
					}
					lpBits -= uiInputDIBScanLineSize;
				}

				if (!m_Gif.SavePrepare(	lpszPathName,
										OutputColorMap,
										LogNumColors(GetNumColors()),
										GetWidth(),
										GetHeight(),
										Version))
					throw (int)GIF_E_GIFLIB;
				if (!m_Gif.Save(OutputBuffer,
								GetWidth(),
								NULL, // No Color Map
								LogNumColors(GetNumColors()),
								m_Gif.m_nLeft,
								m_Gif.m_nTop,
								m_Gif.m_nWidth,
								m_Gif.m_nHeight,
								pProgressWnd,
								bProgressSend,
								pThread))
					throw (int)GIF_E_GIFLIB;

				break;
			}
			case 8:
			{
				if ((OutputColorMap = ::MakeMapObject(1 << LogNumColors(GetNumColors())/*has to be a power of two*/, NULL)) == NULL)
					throw (int)GIF_E_NOMEM;
				m_Gif.SetColorMap(OutputColorMap, m_pBMI);

				for (unsigned int line = 0 ; line < GetHeight() ; line++)
					memcpy(	OutputBuffer + line*GetWidth(),
							lpBits - line*uiInputDIBScanLineSize, GetWidth());

				if (!m_Gif.SavePrepare(	lpszPathName,
										OutputColorMap,
										LogNumColors(GetNumColors()),
										GetWidth(),
										GetHeight(),
										Version))
					throw (int)GIF_E_GIFLIB;
				if (!m_Gif.Save(OutputBuffer,
								GetWidth(),
								NULL, // No Color Map
								LogNumColors(GetNumColors()),
								m_Gif.m_nLeft,
								m_Gif.m_nTop,
								m_Gif.m_nWidth,
								m_Gif.m_nHeight,
								pProgressWnd,
								bProgressSend,
								pThread))
					throw (int)GIF_E_GIFLIB;

				break;
			}
			case 16: // 16bit to RGB conversion
			{
				int ColorMapSize = 256;
				if ((OutputColorMap = ::MakeMapObject(ColorMapSize/*has to be a power of two*/, NULL)) == NULL)
					throw (int)GIF_E_NOMEM;

				if ((RedBuffer = (GifByteType *) new GifByteType[GetWidth() * GetHeight()]) == NULL ||
					(GreenBuffer = (GifByteType *) new GifByteType[GetWidth() * GetHeight()]) == NULL ||
					(BlueBuffer = (GifByteType *) new GifByteType[GetWidth() * GetHeight()]) == NULL)
					throw (int)GIF_E_NOMEM;

				for (unsigned int line = 0 ; line < GetHeight() ; line++)
				{
					for (unsigned int i = 0 ; i < GetWidth() ; i++)
					{
						DIB16ToRGB(((WORD*)lpBits)[i], &R, &G, &B);
						BlueBuffer[line * GetWidth() + i] = B;
						GreenBuffer[line * GetWidth() + i] = G;
						RedBuffer[line * GetWidth() + i] = R;
					}
					lpBits -= uiInputDIBScanLineSize;
				}

				if (QuantizeBuffer(GetWidth(), GetHeight(), &ColorMapSize,
					RedBuffer, GreenBuffer, BlueBuffer,
					OutputBuffer, OutputColorMap->Colors) == GIF_ERROR)
					throw (int)GIF_E_GIFLIB;

				if (!m_Gif.SavePrepare(	lpszPathName,
										OutputColorMap,
										8,
										GetWidth(),
										GetHeight(),
										Version))
					throw (int)GIF_E_GIFLIB;
				if (!m_Gif.Save(OutputBuffer,
								GetWidth(),
								NULL, // No Color Map
								8,
								m_Gif.m_nLeft,
								m_Gif.m_nTop,
								m_Gif.m_nWidth,
								m_Gif.m_nHeight,
								pProgressWnd,
								bProgressSend,
								pThread))
					throw (int)GIF_E_GIFLIB;

				break;
			}
			case 24:
			{
				int ColorMapSize = 256;
				if ((OutputColorMap = ::MakeMapObject(ColorMapSize/*has to be a power of two*/, NULL)) == NULL)
					throw (int)GIF_E_NOMEM;

				if ((RedBuffer = (GifByteType *) new GifByteType[GetWidth() * GetHeight()]) == NULL ||
					(GreenBuffer = (GifByteType *) new GifByteType[GetWidth() * GetHeight()]) == NULL ||
					(BlueBuffer = (GifByteType *) new GifByteType[GetWidth() * GetHeight()]) == NULL)
					throw (int)GIF_E_NOMEM;

				for (unsigned int line = 0 ; line < GetHeight() ; line++)
				{
					for (unsigned int i = 0 ; i < GetWidth() ; i++)
					{
						BlueBuffer[line * GetWidth() + i] = (BYTE)lpBits[3*i];
						GreenBuffer[line * GetWidth() + i] = (BYTE)lpBits[3*i+1];
						RedBuffer[line * GetWidth() + i] = (BYTE)lpBits[3*i+2];
					}
					lpBits -= uiInputDIBScanLineSize;
				}

				if (QuantizeBuffer(GetWidth(), GetHeight(), &ColorMapSize,
					RedBuffer, GreenBuffer, BlueBuffer,
					OutputBuffer, OutputColorMap->Colors) == GIF_ERROR)
					throw (int)GIF_E_GIFLIB;
				
				if (!m_Gif.SavePrepare(	lpszPathName,
										OutputColorMap,
										8,
										GetWidth(),
										GetHeight(),
										Version))
					throw (int)GIF_E_GIFLIB;
				if (!m_Gif.Save(OutputBuffer,
								GetWidth(),
								NULL, // No Color Map
								8,
								m_Gif.m_nLeft,
								m_Gif.m_nTop,
								m_Gif.m_nWidth,
								m_Gif.m_nHeight,
								pProgressWnd,
								bProgressSend,
								pThread))
					throw (int)GIF_E_GIFLIB;
				
				break;
			}
			case 32: // 32bit to RGB conversion
			{
				int ColorMapSize = 256;
				if ((OutputColorMap = ::MakeMapObject(ColorMapSize/*has to be a power of two*/, NULL)) == NULL)
					throw (int)GIF_E_NOMEM;

				if ((RedBuffer = (GifByteType *) new GifByteType[GetWidth() * GetHeight()]) == NULL ||
					(GreenBuffer = (GifByteType *) new GifByteType[GetWidth() * GetHeight()]) == NULL ||
					(BlueBuffer = (GifByteType *) new GifByteType[GetWidth() * GetHeight()]) == NULL)
					throw (int)GIF_E_NOMEM;

				for (unsigned int line = 0 ; line < GetHeight() ; line++)
				{
					for (unsigned int i = 0 ; i < GetWidth() ; i++)
					{
						DIB32ToRGB(((DWORD*)lpBits)[i], &R, &G, &B);
						BlueBuffer[line * GetWidth() + i] = B;
						GreenBuffer[line * GetWidth() + i] = G;
						RedBuffer[line * GetWidth() + i] = R;
					}
					lpBits -= uiInputDIBScanLineSize;
				}

				if (QuantizeBuffer(GetWidth(), GetHeight(), &ColorMapSize,
					RedBuffer, GreenBuffer, BlueBuffer,
					OutputBuffer, OutputColorMap->Colors) == GIF_ERROR)
					throw (int)GIF_E_GIFLIB;

				if (!m_Gif.SavePrepare(	lpszPathName,
										OutputColorMap,
										8,
										GetWidth(),
										GetHeight(),
										Version))
					throw (int)GIF_E_GIFLIB;
				if (!m_Gif.Save(OutputBuffer,
								GetWidth(),
								NULL, // No Color Map
								8,
								m_Gif.m_nLeft,
								m_Gif.m_nTop,
								m_Gif.m_nWidth,
								m_Gif.m_nHeight,
								pProgressWnd,
								bProgressSend,
								pThread))
					throw (int)GIF_E_GIFLIB;

				break;
			}
			default:
				throw (int)GIF_E_BADBMP;
		}

		if (OutputBuffer)
			delete [] OutputBuffer;
		if (OutputColorMap)
			::FreeMapObject(OutputColorMap);
		if (RedBuffer)
			delete [] RedBuffer;
		if (GreenBuffer)
			delete [] GreenBuffer;
		if (BlueBuffer)
			delete [] BlueBuffer;

		m_Gif.SaveClose();

		return TRUE;
	}
	catch (int error_code)
	{
		if (OutputBuffer)
			delete [] OutputBuffer;
		if (OutputColorMap)
			::FreeMapObject(OutputColorMap);
		if (RedBuffer)
			delete [] RedBuffer;
		if (GreenBuffer)
			delete [] GreenBuffer;
		if (BlueBuffer)
			delete [] BlueBuffer;

		m_Gif.SaveClose();

		// Just Exit
		if (pThread && pThread->DoExit())
			return FALSE;

		CString str;
#ifdef _DEBUG
		str.Format(_T("SaveGIF(%s):\n"), lpszPathName);
#endif
		switch (error_code)
		{
			case GIF_E_ZEROPATH :		str += _T("The file name is zero\n");
			break;
			case GIF_E_FILEREADONLY :	str += _T("The file is read only\n");
			break;
			case GIF_E_NOMEM :			str += _T("Could not alloc memory\n");
			break;
			case GIF_E_BADBMP :			str += _T("Corrupted or unsupported DIB\n");
			break;
			case GIF_E_GIFLIB :			str += _T("Cannot save file\n");
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

BOOL CDib::SaveFirstGIF(LPCTSTR lpszPathName,
						CWnd* pProgressWnd/*=NULL*/,
						BOOL bProgressSend/*=TRUE*/,
						CWorkerThread* pThread/*=NULL*/)
{
	GifByteType* OutputBuffer = NULL;
	ColorMapObject* OutputColorMap = NULL;
	GifByteType* RedBuffer = NULL;
	GifByteType* GreenBuffer = NULL;
	GifByteType* BlueBuffer = NULL;
	BYTE R, G, B;

	try
	{
		CString sPathName(lpszPathName);
		if (sPathName.IsEmpty())
			throw (int)GIF_E_ZEROPATH;

		DWORD dwAttrib = ::GetFileAttributes(lpszPathName);
		if ((dwAttrib != 0xFFFFFFFF) && (dwAttrib & FILE_ATTRIBUTE_READONLY))
			throw (int)GIF_E_FILEREADONLY;

		if (!m_pBits)
		{
			if (!DibSectionToBits())
				throw (int)GIF_E_BADBMP;
		}

		if (!m_pBits || !m_pBMI)
			throw (int)GIF_E_BADBMP;

		if (IsCompressed())
#ifndef _WIN32_WCE
			if (!Decompress(GetBitCount())) // Decompress
				throw (int)GIF_E_BADBMP;
#else
			throw (int)GIF_E_BADBMP;
#endif

		// Scan Line Sizes
		DWORD uiInputDIBScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * GetBitCount());

		// In the gif file the scan lines are top / down 
		// In a DIB the scan lines are stored bottom / up
		LPBYTE lpBits = m_pBits;
		lpBits += uiInputDIBScanLineSize*(GetHeight()-1);

		// Allocate Output Buffer
		if ((OutputBuffer = (GifByteType*) new GifByteType[GetWidth() * GetHeight()]) == NULL)
			throw (int)GIF_E_NOMEM;

		switch (GetBitCount())
		{
			case 1:
			{
				if ((OutputColorMap = ::MakeMapObject(1 << LogNumColors(GetNumColors())/*has to be a power of two*/, NULL)) == NULL)
					throw (int)GIF_E_NOMEM;
				m_Gif.SetColorMap(OutputColorMap, m_pBMI);

				for (unsigned int line = 0 ; line < GetHeight() ; line++)
				{
					for (unsigned int i = 0 ; i < GetWidth() ; i++)
					{
						OutputBuffer[line * GetWidth() + i] = (lpBits[i/8] >> (7-(i%8))) & 0x01;
					}
					lpBits -= uiInputDIBScanLineSize;
				}
				
				if (!m_Gif.SavePrepare(	lpszPathName,
										OutputColorMap,
										LogNumColors(GetNumColors()),
										GetWidth(),
										GetHeight(),
										"89a"))
					throw (int)GIF_E_GIFLIB;
				if (!m_Gif.Save(OutputBuffer,
								GetWidth(),
								OutputColorMap,
								LogNumColors(GetNumColors()),
								m_Gif.m_nLeft,
								m_Gif.m_nTop,
								m_Gif.m_nWidth,
								m_Gif.m_nHeight,
								pProgressWnd,
								bProgressSend,
								pThread))
					throw (int)GIF_E_GIFLIB;

				break;
			}
			case 4:
			{
				if ((OutputColorMap = ::MakeMapObject(1 << LogNumColors(GetNumColors())/*has to be a power of two*/, NULL)) == NULL)
					throw (int)GIF_E_NOMEM;
				m_Gif.SetColorMap(OutputColorMap, m_pBMI);

				for (unsigned int line = 0 ; line < GetHeight() ; line++)
				{
					for (unsigned int i = 0 ; i < GetWidth() ; i++)
					{
						if (i%2) // LS Nibble
							OutputBuffer[line * GetWidth() + i] = lpBits[i/2] & 0x0F;
						else // MS Nibble
							OutputBuffer[line * GetWidth() + i] = lpBits[i/2] >> 4;
					}
					lpBits -= uiInputDIBScanLineSize;
				}
				
				if (!m_Gif.SavePrepare(	lpszPathName,
										OutputColorMap,
										LogNumColors(GetNumColors()),
										GetWidth(),
										GetHeight(),
										"89a"))
					throw (int)GIF_E_GIFLIB;
				if (!m_Gif.Save(OutputBuffer,
								GetWidth(),
								OutputColorMap,
								LogNumColors(GetNumColors()),
								m_Gif.m_nLeft,
								m_Gif.m_nTop,
								m_Gif.m_nWidth,
								m_Gif.m_nHeight,
								pProgressWnd,
								bProgressSend,
								pThread))
					throw (int)GIF_E_GIFLIB;

				break;
			}
			case 8:
			{
				if ((OutputColorMap = ::MakeMapObject(1 << LogNumColors(GetNumColors())/*has to be a power of two*/, NULL)) == NULL)
					throw (int)GIF_E_NOMEM;
				m_Gif.SetColorMap(OutputColorMap, m_pBMI);

				for (unsigned int line = 0 ; line < GetHeight() ; line++)
					memcpy(	OutputBuffer + line*GetWidth(),
							lpBits - line*uiInputDIBScanLineSize, GetWidth());

				if (!m_Gif.SavePrepare(	lpszPathName,
										OutputColorMap,
										LogNumColors(GetNumColors()),
										GetWidth(),
										GetHeight(),
										"89a"))
					throw (int)GIF_E_GIFLIB;
				if (!m_Gif.Save(OutputBuffer,
								GetWidth(),
								OutputColorMap,
								LogNumColors(GetNumColors()),
								m_Gif.m_nLeft,
								m_Gif.m_nTop,
								m_Gif.m_nWidth,
								m_Gif.m_nHeight,
								pProgressWnd,
								bProgressSend,
								pThread))
					throw (int)GIF_E_GIFLIB;

				break;
			}
			case 16: // 16bit to RGB conversion
			{
				int ColorMapSize = 256;
				if ((OutputColorMap = ::MakeMapObject(ColorMapSize/*has to be a power of two*/, NULL)) == NULL)
					throw (int)GIF_E_NOMEM;

				if ((RedBuffer = (GifByteType *) new GifByteType[GetWidth() * GetHeight()]) == NULL ||
					(GreenBuffer = (GifByteType *) new GifByteType[GetWidth() * GetHeight()]) == NULL ||
					(BlueBuffer = (GifByteType *) new GifByteType[GetWidth() * GetHeight()]) == NULL)
					throw (int)GIF_E_NOMEM;

				for (unsigned int line = 0 ; line < GetHeight() ; line++)
				{
					for (unsigned int i = 0 ; i < GetWidth() ; i++)
					{
						DIB16ToRGB(((WORD*)lpBits)[i], &R, &G, &B);
						BlueBuffer[line * GetWidth() + i] = B;
						GreenBuffer[line * GetWidth() + i] = G;
						RedBuffer[line * GetWidth() + i] = R;
					}
					lpBits -= uiInputDIBScanLineSize;
				}

				if (QuantizeBuffer(GetWidth(), GetHeight(), &ColorMapSize,
					RedBuffer, GreenBuffer, BlueBuffer,
					OutputBuffer, OutputColorMap->Colors) == GIF_ERROR)
					throw (int)GIF_E_GIFLIB;

				if (!m_Gif.SavePrepare(	lpszPathName,
										OutputColorMap,
										8,
										GetWidth(),
										GetHeight(),
										"89a"))
					throw (int)GIF_E_GIFLIB;
				if (!m_Gif.Save(OutputBuffer,
								GetWidth(),
								OutputColorMap,
								8,
								m_Gif.m_nLeft,
								m_Gif.m_nTop,
								m_Gif.m_nWidth,
								m_Gif.m_nHeight,
								pProgressWnd,
								bProgressSend,
								pThread))
					throw (int)GIF_E_GIFLIB;

				break;
			}
			case 24:
			{
				int ColorMapSize = 256;
				if ((OutputColorMap = ::MakeMapObject(ColorMapSize/*has to be a power of two*/, NULL)) == NULL)
					throw (int)GIF_E_NOMEM;

				if ((RedBuffer = (GifByteType *) new GifByteType[GetWidth() * GetHeight()]) == NULL ||
					(GreenBuffer = (GifByteType *) new GifByteType[GetWidth() * GetHeight()]) == NULL ||
					(BlueBuffer = (GifByteType *) new GifByteType[GetWidth() * GetHeight()]) == NULL)
					throw (int)GIF_E_NOMEM;

				for (unsigned int line = 0 ; line < GetHeight() ; line++)
				{
					for (unsigned int i = 0 ; i < GetWidth() ; i++)
					{
						BlueBuffer[line * GetWidth() + i] = (BYTE)lpBits[3*i];
						GreenBuffer[line * GetWidth() + i] = (BYTE)lpBits[3*i+1];
						RedBuffer[line * GetWidth() + i] = (BYTE)lpBits[3*i+2];
					}
					lpBits -= uiInputDIBScanLineSize;
				}

				if (QuantizeBuffer(GetWidth(), GetHeight(), &ColorMapSize,
					RedBuffer, GreenBuffer, BlueBuffer,
					OutputBuffer, OutputColorMap->Colors) == GIF_ERROR)
					throw (int)GIF_E_GIFLIB;

				if (!m_Gif.SavePrepare(	lpszPathName,
										OutputColorMap,
										8,
										GetWidth(),
										GetHeight(),
										"89a"))
					throw (int)GIF_E_GIFLIB;
				if (!m_Gif.Save(OutputBuffer,
								GetWidth(),
								OutputColorMap,
								8,
								m_Gif.m_nLeft,
								m_Gif.m_nTop,
								m_Gif.m_nWidth,
								m_Gif.m_nHeight,
								pProgressWnd,
								bProgressSend,
								pThread))
					throw (int)GIF_E_GIFLIB;
				
				break;
			}
			case 32: // 32bit to RGB conversion
			{
				int ColorMapSize = 256;
				if ((OutputColorMap = ::MakeMapObject(ColorMapSize/*has to be a power of two*/, NULL)) == NULL)
					throw (int)GIF_E_NOMEM;

				if ((RedBuffer = (GifByteType *) new GifByteType[GetWidth() * GetHeight()]) == NULL ||
					(GreenBuffer = (GifByteType *) new GifByteType[GetWidth() * GetHeight()]) == NULL ||
					(BlueBuffer = (GifByteType *) new GifByteType[GetWidth() * GetHeight()]) == NULL)
					throw (int)GIF_E_NOMEM;

				for (unsigned int line = 0 ; line < GetHeight() ; line++)
				{
					for (unsigned int i = 0 ; i < GetWidth() ; i++)
					{
						DIB32ToRGB(((DWORD*)lpBits)[i], &R, &G, &B);
						BlueBuffer[line * GetWidth() + i] = B;
						GreenBuffer[line * GetWidth() + i] = G;
						RedBuffer[line * GetWidth() + i] = R;
					}
					lpBits -= uiInputDIBScanLineSize;
				}

				if (QuantizeBuffer(GetWidth(), GetHeight(), &ColorMapSize,
					RedBuffer, GreenBuffer, BlueBuffer,
					OutputBuffer, OutputColorMap->Colors) == GIF_ERROR)
					throw (int)GIF_E_GIFLIB;

				if (!m_Gif.SavePrepare(	lpszPathName,
										OutputColorMap,
										8,
										GetWidth(),
										GetHeight(),
										"89a"))
					throw (int)GIF_E_GIFLIB;
				if (!m_Gif.Save(OutputBuffer,
								GetWidth(),
								OutputColorMap,
								8,
								m_Gif.m_nLeft,
								m_Gif.m_nTop,
								m_Gif.m_nWidth,
								m_Gif.m_nHeight,
								pProgressWnd,
								bProgressSend,
								pThread))
					throw (int)GIF_E_GIFLIB;

				break;
			}
			default:
				throw (int)GIF_E_BADBMP;
		}

		if (OutputBuffer)
			delete [] OutputBuffer;
		if (OutputColorMap)
			::FreeMapObject(OutputColorMap);
		if (RedBuffer)
			delete [] RedBuffer;
		if (GreenBuffer)
			delete [] GreenBuffer;
		if (BlueBuffer)
			delete [] BlueBuffer;

		return TRUE;
	}
	catch (int error_code)
	{
		if (OutputBuffer)
			delete [] OutputBuffer;
		if (OutputColorMap)
			::FreeMapObject(OutputColorMap);
		if (RedBuffer)
			delete [] RedBuffer;
		if (GreenBuffer)
			delete [] GreenBuffer;
		if (BlueBuffer)
			delete [] BlueBuffer;

		// Just Exit
		if (pThread && pThread->DoExit())
			return FALSE;

		CString str;
#ifdef _DEBUG
		str.Format(_T("SaveFirstGIF(%s):\n"), lpszPathName);
#endif
		switch (error_code)
		{
			case GIF_E_ZEROPATH :		str += _T("The file name is zero\n");
			break;
			case GIF_E_FILEREADONLY :	str += _T("The file is read only\n");
			break;
			case GIF_E_NOMEM :			str += _T("Could not alloc memory\n");
			break;
			case GIF_E_BADBMP :			str += _T("Corrupted or unsupported DIB\n");
			break;
			case GIF_E_GIFLIB :			str += _T("Cannot save file\n");
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

BOOL CDib::SaveNextGIF(	CDib* pDib/*=NULL*/,
						CWnd* pProgressWnd/*=NULL*/,
						BOOL bProgressSend/*=TRUE*/,
						CWorkerThread* pThread/*=NULL*/)
{
	GifByteType* OutputBuffer = NULL;
	ColorMapObject* OutputColorMap = NULL;
	GifByteType* RedBuffer = NULL;
	GifByteType* GreenBuffer = NULL;
	GifByteType* BlueBuffer = NULL;
	BYTE R, G, B;

	if (pDib == NULL)
		pDib = this;

	try
	{
		if (!pDib->m_pBits)
		{
			if (!pDib->DibSectionToBits())
				throw (int)GIF_E_BADBMP;
		}

		if (!pDib->m_pBits || !pDib->m_pBMI)
			throw (int)GIF_E_BADBMP;

		if (pDib->IsCompressed())
#ifndef _WIN32_WCE
			if (!pDib->Decompress(pDib->GetBitCount())) // Decompress
				throw (int)GIF_E_BADBMP;
#else
			throw (int)GIF_E_BADBMP;
#endif
   
		// Scan Line Sizes
		DWORD uiInputDIBScanLineSize = DWALIGNEDWIDTHBYTES(pDib->GetWidth() * pDib->GetBitCount());

		// In the gif file the scan lines are top / down 
		// In a DIB the scan lines are stored bottom / up
		LPBYTE lpBits = pDib->m_pBits;
		lpBits += uiInputDIBScanLineSize*(pDib->GetHeight()-1);

		// Allocate Output Buffer
		if ((OutputBuffer = (GifByteType*) new GifByteType[pDib->GetWidth() * pDib->GetHeight()]) == NULL)
			throw (int)GIF_E_NOMEM;

		switch (pDib->GetBitCount())
		{
			case 1:
			{
				if ((OutputColorMap = ::MakeMapObject(1 << LogNumColors(pDib->GetNumColors())/*has to be a power of two*/, NULL)) == NULL)
					throw (int)GIF_E_NOMEM;
				m_Gif.SetColorMap(OutputColorMap, pDib->m_pBMI);

				for (unsigned int line = 0 ; line < pDib->GetHeight() ; line++)
				{
					for (unsigned int i = 0 ; i < pDib->GetWidth() ; i++)
					{
						OutputBuffer[line * pDib->GetWidth() + i] = (lpBits[i/8] >> (7-(i%8))) & 0x01;
					}
					lpBits -= uiInputDIBScanLineSize;
				}
				
				if (!m_Gif.Save(OutputBuffer,
								pDib->GetWidth(),
								OutputColorMap,
								LogNumColors(pDib->GetNumColors()),
								m_Gif.m_nLeft,
								m_Gif.m_nTop,
								m_Gif.m_nWidth,
								m_Gif.m_nHeight,
								pProgressWnd,
								bProgressSend,
								pThread))
					throw (int)GIF_E_GIFLIB;

				break;
			}
			case 4:
			{
				if ((OutputColorMap = ::MakeMapObject(1 << LogNumColors(pDib->GetNumColors())/*has to be a power of two*/, NULL)) == NULL)
					throw (int)GIF_E_NOMEM;
				m_Gif.SetColorMap(OutputColorMap, pDib->m_pBMI);

				for (unsigned int line = 0 ; line < pDib->GetHeight() ; line++)
				{
					for (unsigned int i = 0 ; i < pDib->GetWidth() ; i++)
					{
						if (i%2) // LS Nibble
							OutputBuffer[line * pDib->GetWidth() + i] = lpBits[i/2] & 0x0F;
						else // MS Nibble
							OutputBuffer[line * pDib->GetWidth() + i] = lpBits[i/2] >> 4;
					}
					lpBits -= uiInputDIBScanLineSize;
				}
				
				if (!m_Gif.Save(OutputBuffer,
								pDib->GetWidth(),
								OutputColorMap,
								LogNumColors(pDib->GetNumColors()),
								m_Gif.m_nLeft,
								m_Gif.m_nTop,
								m_Gif.m_nWidth,
								m_Gif.m_nHeight,
								pProgressWnd,
								bProgressSend,
								pThread))
					throw (int)GIF_E_GIFLIB;

				break;
			}
			case 8:
			{
				if ((OutputColorMap = ::MakeMapObject(1 << LogNumColors(pDib->GetNumColors())/*has to be a power of two*/, NULL)) == NULL)
					throw (int)GIF_E_NOMEM;
				m_Gif.SetColorMap(OutputColorMap, pDib->m_pBMI);

				for (unsigned int line = 0 ; line < pDib->GetHeight() ; line++)
					memcpy(	OutputBuffer + line*pDib->GetWidth(),
							lpBits - line*uiInputDIBScanLineSize, pDib->GetWidth());

				if (!m_Gif.Save(OutputBuffer,
								pDib->GetWidth(),
								OutputColorMap,
								LogNumColors(pDib->GetNumColors()),
								m_Gif.m_nLeft,
								m_Gif.m_nTop,
								m_Gif.m_nWidth,
								m_Gif.m_nHeight,
								pProgressWnd,
								bProgressSend,
								pThread))
					throw (int)GIF_E_GIFLIB;

				break;
			}
			case 16: // 16bit to RGB conversion
			{
				int ColorMapSize = 256;
				if ((OutputColorMap = ::MakeMapObject(ColorMapSize/*has to be a power of two*/, NULL)) == NULL)
					throw (int)GIF_E_NOMEM;

				if ((RedBuffer = (GifByteType *) new GifByteType[pDib->GetWidth() * pDib->GetHeight()]) == NULL ||
					(GreenBuffer = (GifByteType *) new GifByteType[pDib->GetWidth() * pDib->GetHeight()]) == NULL ||
					(BlueBuffer = (GifByteType *) new GifByteType[pDib->GetWidth() * pDib->GetHeight()]) == NULL)
					throw (int)GIF_E_NOMEM;

				for (unsigned int line = 0 ; line < pDib->GetHeight() ; line++)
				{
					for (unsigned int i = 0 ; i < pDib->GetWidth() ; i++)
					{
						pDib->DIB16ToRGB(((WORD*)lpBits)[i], &R, &G, &B);
						BlueBuffer[line * GetWidth() + i] = B;
						GreenBuffer[line * GetWidth() + i] = G;
						RedBuffer[line * GetWidth() + i] = R;
					}
					lpBits -= uiInputDIBScanLineSize;
				}

				if (QuantizeBuffer(pDib->GetWidth(), pDib->GetHeight(), &ColorMapSize,
					RedBuffer, GreenBuffer, BlueBuffer,
					OutputBuffer, OutputColorMap->Colors) == GIF_ERROR)
					throw (int)GIF_E_GIFLIB;

				if (!m_Gif.Save(OutputBuffer,
								pDib->GetWidth(),
								OutputColorMap,
								8,
								m_Gif.m_nLeft,
								m_Gif.m_nTop,
								m_Gif.m_nWidth,
								m_Gif.m_nHeight,
								pProgressWnd,
								bProgressSend,
								pThread))
					throw (int)GIF_E_GIFLIB;

				break;
			}
			case 24:
			{
				int ColorMapSize = 256;
				if ((OutputColorMap = ::MakeMapObject(ColorMapSize/*has to be a power of two*/, NULL)) == NULL)
					throw (int)GIF_E_NOMEM;

				if ((RedBuffer = (GifByteType *) new GifByteType[pDib->GetWidth() * pDib->GetHeight()]) == NULL ||
					(GreenBuffer = (GifByteType *) new GifByteType[pDib->GetWidth() * pDib->GetHeight()]) == NULL ||
					(BlueBuffer = (GifByteType *) new GifByteType[pDib->GetWidth() * pDib->GetHeight()]) == NULL)
					throw (int)GIF_E_NOMEM;

				for (unsigned int line = 0 ; line < pDib->GetHeight() ; line++)
				{
					for (unsigned int i = 0 ; i < pDib->GetWidth() ; i++)
					{
						BlueBuffer[line * GetWidth() + i] = (BYTE)lpBits[3*i];
						GreenBuffer[line * GetWidth() + i] = (BYTE)lpBits[3*i+1];
						RedBuffer[line * GetWidth() + i] = (BYTE)lpBits[3*i+2];
					}
					lpBits -= uiInputDIBScanLineSize;
				}

				if (QuantizeBuffer(pDib->GetWidth(), pDib->GetHeight(), &ColorMapSize,
					RedBuffer, GreenBuffer, BlueBuffer,
					OutputBuffer, OutputColorMap->Colors) == GIF_ERROR)
					throw (int)GIF_E_GIFLIB;

				if (!m_Gif.Save(OutputBuffer,
								pDib->GetWidth(),
								OutputColorMap,
								8,
								m_Gif.m_nLeft,
								m_Gif.m_nTop,
								m_Gif.m_nWidth,
								m_Gif.m_nHeight,
								pProgressWnd,
								bProgressSend,
								pThread))
					throw (int)GIF_E_GIFLIB;
				
				break;
			}
			case 32: // 32bit to RGB conversion
			{
				int ColorMapSize = 256;
				if ((OutputColorMap = ::MakeMapObject(ColorMapSize/*has to be a power of two*/, NULL)) == NULL)
					throw (int)GIF_E_NOMEM;

				if ((RedBuffer = (GifByteType *) new GifByteType[pDib->GetWidth() * pDib->GetHeight()]) == NULL ||
					(GreenBuffer = (GifByteType *) new GifByteType[pDib->GetWidth() * pDib->GetHeight()]) == NULL ||
					(BlueBuffer = (GifByteType *) new GifByteType[pDib->GetWidth() * pDib->GetHeight()]) == NULL)
					throw (int)GIF_E_NOMEM;

				for (unsigned int line = 0 ; line < pDib->GetHeight() ; line++)
				{
					for (unsigned int i = 0 ; i < pDib->GetWidth() ; i++)
					{
						pDib->DIB32ToRGB(((DWORD*)lpBits)[i], &R, &G, &B);
						BlueBuffer[line * GetWidth() + i] = B;
						GreenBuffer[line * GetWidth() + i] = G;
						RedBuffer[line * GetWidth() + i] = R;
					}
					lpBits -= uiInputDIBScanLineSize;
				}

				if (QuantizeBuffer(pDib->GetWidth(), pDib->GetHeight(), &ColorMapSize,
					RedBuffer, GreenBuffer, BlueBuffer,
					OutputBuffer, OutputColorMap->Colors) == GIF_ERROR)
					throw (int)GIF_E_GIFLIB;

				if (!m_Gif.Save(OutputBuffer,
								pDib->GetWidth(),
								OutputColorMap,
								8,
								m_Gif.m_nLeft,
								m_Gif.m_nTop,
								m_Gif.m_nWidth,
								m_Gif.m_nHeight,
								pProgressWnd,
								bProgressSend,
								pThread))
					throw (int)GIF_E_GIFLIB;

				break;
			}
			default:
				throw (int)GIF_E_BADBMP;
		}

		if (OutputBuffer)
			delete [] OutputBuffer;
		if (OutputColorMap)
			::FreeMapObject(OutputColorMap);
		if (RedBuffer)
			delete [] RedBuffer;
		if (GreenBuffer)
			delete [] GreenBuffer;
		if (BlueBuffer)
			delete [] BlueBuffer;

		return TRUE;
	}
	catch (int error_code)
	{
		if (OutputBuffer)
			delete [] OutputBuffer;
		if (OutputColorMap)
			::FreeMapObject(OutputColorMap);
		if (RedBuffer)
			delete [] RedBuffer;
		if (GreenBuffer)
			delete [] GreenBuffer;
		if (BlueBuffer)
			delete [] BlueBuffer;

		// Just Exit
		if (pThread && pThread->DoExit())
			return FALSE;

		CString str;
#ifdef _DEBUG
		str = _T("SaveNextGIF: ");
#endif
		switch (error_code)
		{
			case GIF_E_NOMEM :		str += _T("Could not alloc memory\n");
			break;
			case GIF_E_BADBMP :		str += _T("Corrupted or unsupported DIB\n");
			break;
			case GIF_E_GIFLIB :		str += _T("Cannot save file\n");
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

BOOL CDib::GIFWriteComment(	int nFrame,
							LPCTSTR szFileName,
							LPCTSTR szTempDir,
							LPCTSTR szComment,
							BOOL bShowMessageBoxOnError)
{
	if (::GetFileExt(szFileName) == _T(".gif"))
	{
		// Temporary File
		CString sTempFileName = ::MakeTempFileName(szTempDir, szFileName);

		// Load First GIF
		CDib Dib;
		int i = 0;
		BOOL bCommentDone = FALSE;
		if (!Dib.LoadFirstGIFRaw(szFileName))
			return FALSE;
		else
		{	
			if (nFrame == i)
			{
				Dib.GetGif()->SetComment(szComment);
				bCommentDone = TRUE;
			}

			// Save First GIF
			if (!Dib.SaveFirstGIF(sTempFileName))
				return FALSE;
		}

		// Load Next GIFs if Any
		while (Dib.LoadNextGIFRaw() > 0)
		{
			// Inc. Pos
			i++;
			
			if (!bCommentDone && nFrame == i)
			{
				Dib.GetGif()->SetComment(szComment);
				bCommentDone = TRUE;
			}

			// Save Next GIF
			if (!Dib.SaveNextGIF())
				return FALSE;
		}

		// Close
		Dib.GetGif()->LoadClose();
		Dib.GetGif()->SaveClose();

		// Remove and Rename Files
		if (bCommentDone)
		{
			try
			{
				CFile::Remove(szFileName);
				CFile::Rename(sTempFileName, szFileName);
			}
			catch (CFileException* e)
			{
				::DeleteFile(sTempFileName);
				
				DWORD dwAttrib = ::GetFileAttributes(szFileName);
				if ((dwAttrib != 0xFFFFFFFF) && (dwAttrib & FILE_ATTRIBUTE_READONLY))
				{
					CString str(_T("The file is read only\n"));
					TRACE(str);
					if (bShowMessageBoxOnError)
						::AfxMessageBox(str, MB_ICONSTOP);
				}
				else
					ShowError(e->m_lOsError, bShowMessageBoxOnError, _T("GIFWriteComment()"));

				e->Delete();
				return FALSE;
			}
			return TRUE;
		}
		else
		{
			::DeleteFile(sTempFileName);
			return FALSE;
		}
	}
	else
		return FALSE;
}

const GifByteType CGif::m_szNetscape20ext[] = "\x0bNETSCAPE2.0";
const int CGif::m_InterlacedOffset[] = { 0, 4, 2, 1 }; /* The way Interlaced image should. */
const int CGif::m_InterlacedJumps[] = { 8, 8, 4, 2 };    /* be read - offsets and jumps... */

CGif::CGif()
{
	m_nBackgndColorIndex = GIF_COLORINDEX_NOT_DEFINED;
	m_nTransparencyColorIndex = GIF_COLORINDEX_NOT_DEFINED;
	m_nLoadImageCount = 0;
	m_nSaveImageCount = 0;
	m_nLeft = 0;
	m_nTop = 0;
	m_nWidth = 0;
	m_nHeight = 0;
	m_wLoopCount = 0;
	m_bHasLoopExtension = FALSE;
	m_nDelay = 10;
	m_nFlags = 0;
	m_lpszComment = NULL;
	m_bHasGraphicExtension = FALSE;
	m_bHasScreenColorTable = FALSE;
	m_bHasImageColorTable = FALSE;
	m_bAlphaUsed = FALSE;
	m_pLoadFile = NULL;
	m_pSaveFile = NULL;
	m_pScreenBMI = NULL;
	m_pCurrentImageBMI = NULL;
}

CGif::~CGif()
{
	Close();
	Free();
}

CGif::CGif(const CGif& gif) // Copy Constructor (CGif Gif1 = Gif2 or CGif Gif1(Gif2))
{
	// Copy Vars
	m_nBackgndColorIndex = gif.m_nBackgndColorIndex;
	m_nTransparencyColorIndex = gif.m_nTransparencyColorIndex;
	m_nLoadImageCount = gif.m_nLoadImageCount;
	m_nSaveImageCount = gif.m_nSaveImageCount;
	m_nLeft = gif.m_nLeft;
	m_nTop = gif.m_nTop;
	m_nWidth = gif.m_nWidth;
	m_nHeight = gif.m_nHeight;
	m_wLoopCount = gif.m_wLoopCount;
	m_bHasLoopExtension = gif.m_bHasLoopExtension;
	m_nDelay = gif.m_nDelay;
	m_nFlags = gif.m_nFlags;
	m_bHasGraphicExtension = gif.m_bHasGraphicExtension;
	m_bHasScreenColorTable = gif.m_bHasScreenColorTable;
	m_bHasImageColorTable = gif.m_bHasImageColorTable;
	m_bAlphaUsed = gif.m_bAlphaUsed;
	m_pLoadFile = NULL; // Do not Copy!
	m_pSaveFile = NULL; // Do not Copy!
	if (gif.m_pScreenBMI)
	{
		m_pScreenBMI = (LPBITMAPINFO)new BYTE[	gif.m_pScreenBMI->bmiHeader.biSize +
												CDib::GetNumColors(gif.m_pScreenBMI)*sizeof(RGBQUAD)];
		memcpy(m_pScreenBMI, gif.m_pScreenBMI,	gif.m_pScreenBMI->bmiHeader.biSize +
												CDib::GetNumColors(gif.m_pScreenBMI)*sizeof(RGBQUAD));
	}
	else
		m_pScreenBMI = NULL;
	if (gif.m_pCurrentImageBMI)
	{
		m_pCurrentImageBMI = (LPBITMAPINFO)new BYTE[gif.m_pCurrentImageBMI->bmiHeader.biSize +
													CDib::GetNumColors(gif.m_pCurrentImageBMI)*sizeof(RGBQUAD)];
		memcpy(	m_pCurrentImageBMI, gif.m_pCurrentImageBMI,
				gif.m_pCurrentImageBMI->bmiHeader.biSize +
				CDib::GetNumColors(gif.m_pCurrentImageBMI)*sizeof(RGBQUAD));
	}
	else
		m_pCurrentImageBMI = NULL;
	if (gif.m_lpszComment)
	{
		m_lpszComment = new char[strlen(gif.m_lpszComment)+1];
		strcpy(m_lpszComment, gif.m_lpszComment);
	}
	else
		m_lpszComment = NULL;
}

CGif& CGif::operator=(const CGif& gif) // Copy Assignment (CGif Gif3; Gif3 = Gif1)
{
	if (this != &gif) // beware of self-assignment!
	{
		// Clean the object
		Free();

		// Copy Vars
		m_nBackgndColorIndex = gif.m_nBackgndColorIndex;
		m_nTransparencyColorIndex = gif.m_nTransparencyColorIndex;
		m_nLoadImageCount = gif.m_nLoadImageCount;
		m_nSaveImageCount = gif.m_nSaveImageCount;
		m_nLeft = gif.m_nLeft;
		m_nTop = gif.m_nTop;
		m_nWidth = gif.m_nWidth;
		m_nHeight = gif.m_nHeight;
		m_wLoopCount = gif.m_wLoopCount;
		m_bHasLoopExtension = gif.m_bHasLoopExtension;
		m_nDelay = gif.m_nDelay;
		m_nFlags = gif.m_nFlags;
		m_bHasGraphicExtension = gif.m_bHasGraphicExtension;
		m_bHasScreenColorTable = gif.m_bHasScreenColorTable;
		m_bHasImageColorTable = gif.m_bHasImageColorTable;
		m_bAlphaUsed = gif.m_bAlphaUsed;
		m_pLoadFile = NULL; // Do not Copy!
		m_pSaveFile = NULL; // Do not Copy!
		if (gif.m_pScreenBMI)
		{
			m_pScreenBMI = (LPBITMAPINFO)new BYTE[	gif.m_pScreenBMI->bmiHeader.biSize +
													CDib::GetNumColors(gif.m_pScreenBMI)*sizeof(RGBQUAD)];
			memcpy(m_pScreenBMI, gif.m_pScreenBMI,	gif.m_pScreenBMI->bmiHeader.biSize +
													CDib::GetNumColors(gif.m_pScreenBMI)*sizeof(RGBQUAD));
		}
		else
			m_pScreenBMI = NULL;
		if (gif.m_pCurrentImageBMI)
		{
			m_pCurrentImageBMI = (LPBITMAPINFO)new BYTE[gif.m_pCurrentImageBMI->bmiHeader.biSize +
														CDib::GetNumColors(gif.m_pCurrentImageBMI)*sizeof(RGBQUAD)];
			memcpy(	m_pCurrentImageBMI, gif.m_pCurrentImageBMI,
					gif.m_pCurrentImageBMI->bmiHeader.biSize +
					CDib::GetNumColors(gif.m_pCurrentImageBMI)*sizeof(RGBQUAD));
		}
		else
			m_pCurrentImageBMI = NULL;
		if (gif.m_lpszComment)
		{
			m_lpszComment = new char[strlen(gif.m_lpszComment)+1];
			strcpy(m_lpszComment, gif.m_lpszComment);
		}
		else
			m_lpszComment = NULL;
	}
	return *this;
}

void CGif::Free()
{
	if (m_lpszComment)
	{
		delete m_lpszComment;
		m_lpszComment = NULL;
	}
	if (m_pScreenBMI)
	{
		delete [] m_pScreenBMI;
		m_pScreenBMI = NULL;
	}
	if (m_pCurrentImageBMI)
	{
		delete [] m_pCurrentImageBMI;
		m_pCurrentImageBMI = NULL;
	}
}

BOOL CGif::Close()
{
	BOOL bLoadRes = TRUE;
	BOOL bSaveRes = TRUE;

	bLoadRes = LoadClose();
	bSaveRes = SaveClose();

	return (bLoadRes && bSaveRes);
}

BOOL CGif::LoadClose()
{
	BOOL res = TRUE;

	if (m_pLoadFile)
	{
		if (::DGifCloseFile(m_pLoadFile) == GIF_ERROR)
			res = FALSE;
		else
			m_pLoadFile = NULL;
	}

	return res;
}

BOOL CGif::SaveClose()
{
	BOOL res = TRUE;

	if (m_pSaveFile)
	{
		if (::EGifCloseFile(m_pSaveFile) == GIF_ERROR)
			res = FALSE;
		else
			m_pSaveFile = NULL;
	}

	return res;
}

void CGif::SetComment(LPCTSTR szComment)
{
	// Free
	if (m_lpszComment)
	{
		delete [] m_lpszComment;
		m_lpszComment = NULL;
	}

	// Return if given comment is empty
	if (szComment == NULL || szComment[0] == _T('\0'))
		return;

	// Convert to Ansi
	::ToANSI(szComment, &m_lpszComment);
}

// Copy GIF ColorMap into Windows BITMAPINFO
void CGif::GetColorMap(ColorMapObject* pColorMap, LPBITMAPINFO pBMI)
{
	// Check
	if (!pBMI || !pColorMap)
		return;

	// Get Colors
	int nColorsCount = pColorMap->ColorCount;
	if (nColorsCount >= 256)
		nColorsCount = 256;
	for (int i = 0 ; i < nColorsCount ; i++)
	{
		pBMI->bmiColors[i].rgbRed = pColorMap->Colors[i].Red;
		pBMI->bmiColors[i].rgbGreen = pColorMap->Colors[i].Green;
		pBMI->bmiColors[i].rgbBlue = pColorMap->Colors[i].Blue;
		pBMI->bmiColors[i].rgbReserved = 0;
	}
	pBMI->bmiHeader.biClrUsed =       nColorsCount;
	pBMI->bmiHeader.biClrImportant =  0;

	// Get Bit Count
	int nBitsPerPixel;
	if ((pColorMap->BitsPerPixel < 4) && // 2,3
		(pColorMap->BitsPerPixel > 1))
		nBitsPerPixel = 4;
	else if ((pColorMap->BitsPerPixel < 8) && // 5,6,7
		(pColorMap->BitsPerPixel > 4))
		nBitsPerPixel = 8;
	else
		nBitsPerPixel = pColorMap->BitsPerPixel; // 1,4,8
	pBMI->bmiHeader.biBitCount = nBitsPerPixel;
}

// Copy Windows BITMAPINFO Colors to GIF ColorMap 
void CGif::SetColorMap(ColorMapObject* pColorMap, LPBITMAPINFO pBMI)
{
	// Check
	if (!pBMI || !pColorMap)
		return;

	// Set Colors
	int nColorsCount = pBMI->bmiHeader.biClrUsed;
	if (nColorsCount == 0)
		nColorsCount = 1 << pBMI->bmiHeader.biBitCount;
	if (nColorsCount > 256)
		nColorsCount = 256;
	for (int i = 0 ; i < MIN(nColorsCount, pColorMap->ColorCount) ; i++)
	{
		pColorMap->Colors[i].Red = pBMI->bmiColors[i].rgbRed;
		pColorMap->Colors[i].Green = pBMI->bmiColors[i].rgbGreen;
		pColorMap->Colors[i].Blue = pBMI->bmiColors[i].rgbBlue;
	}
}

// Copy bytes from source to destination skipping transparent bytes
void CGif::CopyLine(int nBitCount,
					BOOL bAlpha,
					LPBYTE pDst,
					LPBYTE pSrc,
					int width,
					const int transparent,
					BOOL& bAlphaUsed,
					RGBQUAD* pColors/*=NULL*/)
{
	if (!pDst)
		return;

	if (!pSrc)
		return;

	if (width < 0)
		return;

	if ((nBitCount > 8) && (pColors == NULL))
		return;

	for (int i = 0 ; i < width ; i++)
	{
		BYTE b = *pSrc++;
		if (b != transparent)
		{
			switch (nBitCount)
			{
				case 1:
				{
					ASSERT(b < 2);						
					if (i%8 == 0)
						pDst[i/8] = 0;
					pDst[i/8] |= b << (7-(i%8));
					break;
				}
				case 4:
				{
					ASSERT(b < 16);
					if (i%2) // LS Nibble
						pDst[i/2] |= b;
					else // MS Nibble
						pDst[i/2] = b << 4;
					break;
				}
				case 8:
				{
					pDst[i] = b;
					break;
				}
				case 24:
				{
					pDst[3*i]   = pColors[b].rgbBlue;
					pDst[3*i+1] = pColors[b].rgbGreen;
					pDst[3*i+2] = pColors[b].rgbRed;
					break;
				}
				case 32:
				{
					pDst[4*i]   = pColors[b].rgbBlue;
					pDst[4*i+1] = pColors[b].rgbGreen;
					pDst[4*i+2] = pColors[b].rgbRed;
					pDst[4*i+3] = 255; // Fully Opaque

					break;
				}
				default:
					break;
			}
		}
		else if (!bAlphaUsed && bAlpha && nBitCount == 32)
		{
			if (pDst[4*i+3] != 255)
				bAlphaUsed = TRUE;
		}
	}
}

// Fill pixels with index
void CGif::FillLine(int nBitCount,
					LPBYTE pDst,
					const int index,
					int width,
					RGBQUAD* pColors/*=NULL*/)
{
	if (!pDst)
		return;

	if (index < 0)
		return;

	if (width < 0)
		return;

	for (int i = 0 ; i < width ; i++)
	{
		switch (nBitCount)
		{
			case 1:
			{					
				if (i%8 == 0)
					pDst[i/8] = 0;
				pDst[i/8] |= index << (7-(i%8));
				break;
			}
			case 4:
			{
				if (i%2) // LS Nibble
					pDst[i/2] |= index;
				else // MS Nibble
					pDst[i/2] = index << 4;
				break;
			}
			case 8:
			{
				pDst[i] = index;
				break;
			}
			case 24:
			{
				pDst[3*i]   = pColors[index].rgbBlue;
				pDst[3*i+1] = pColors[index].rgbGreen;
				pDst[3*i+2] = pColors[index].rgbRed;
				break;
			}
			case 32:
			{
				pDst[4*i]   = pColors[index].rgbBlue;
				pDst[4*i+1] = pColors[index].rgbGreen;
				pDst[4*i+2] = pColors[index].rgbRed;
				pDst[4*i+3] = 0; // All transparent
				break;
			}
			default:
				break;
		}
	}
}

BOOL CGif::SavePrepare(	LPCTSTR FileName,
						ColorMapObject *OutputColorMap,
						int ExpColorMapSize,
						int Width, int Height,
						const char* Version/*="87a"*/) // "87a" or "89a"
{
	BYTE pExtension[255];
	m_nSaveImageCount = 0;

	// Clean-up before starting
	SaveClose();

	// Set GIF Version
	::EGifSetGifVersion(Version);

    // Open File for Write
    if ((m_pSaveFile = ::EGifOpenFileName(FileName, FALSE)) == NULL)
		return FALSE;

	// Put Screen Descriptor
	if (::EGifPutScreenDesc(m_pSaveFile,
							Width,
							Height,
							ExpColorMapSize,
							m_nBackgndColorIndex,
							m_bHasScreenColorTable ? OutputColorMap : NULL) == GIF_ERROR)
	{
		::EGifCloseFile(m_pSaveFile);
		m_pSaveFile = NULL;
		return FALSE;
	}

	// Put Netscape Loop
	if (m_bHasLoopExtension)
	{
		// First Block
		memcpy(pExtension, m_szNetscape20ext+1, m_szNetscape20ext[0]);
		if (::EGifPutExtensionFirst(m_pSaveFile,
									APPLICATION_EXT_FUNC_CODE,
									m_szNetscape20ext[0],
									pExtension) == GIF_ERROR)
		{
			::EGifCloseFile(m_pSaveFile);
			m_pSaveFile = NULL;
			return FALSE;
		}

		// Next (=Last) Block
		pExtension[0] = GIF_NSEXT_LOOP;
		pExtension[1] = LOBYTE(m_wLoopCount);
		pExtension[2] = HIBYTE(m_wLoopCount);
		if (::EGifPutExtensionLast(	m_pSaveFile,
									APPLICATION_EXT_FUNC_CODE,
									3,
									pExtension) == GIF_ERROR)
		{
			::EGifCloseFile(m_pSaveFile);
			m_pSaveFile = NULL;
			return FALSE;
		}	
	}

	return TRUE;
}

BOOL CGif::Save(	GifByteType *OutputBuffer,
					int nBufLineLen,
					ColorMapObject *OutputColorMap,
					int ExpColorMapSize,
					int Left, int Top,
					int Width, int Height,
					CWnd* pProgressWnd/*=NULL*/,
					BOOL bProgressSend/*=TRUE*/,
					CWorkerThread* pThread/*=NULL*/)
{
	if (!m_pSaveFile)
		return FALSE;

    GifByteType* Ptr = OutputBuffer + Top * nBufLineLen + Left;
	BYTE pExtension[255];

	// COMMENT BLOCKS may appear anywhere as they are ignored.
	// (Do NOT place them before the Netscape Looping Extension!!)
	if (m_lpszComment)
	{
		if (::EGifPutComment(m_pSaveFile, m_lpszComment) == GIF_ERROR)
		{
			::EGifCloseFile(m_pSaveFile);
			m_pSaveFile = NULL;
			return FALSE;
		}
	}

	// Put Graphic Extension
	if (m_bHasGraphicExtension)
	{
		pExtension[0] = m_nFlags;
		// Byte Ordering - Unless otherwise stated, multi-byte numeric fields are ordered
		// with the Least Significant Byte first.
		pExtension[1] = LOBYTE(m_nDelay);
		pExtension[2] = HIBYTE(m_nDelay);
		pExtension[3] = (m_nFlags & GIF_TRANSPARENT) ? m_nTransparencyColorIndex : 0;
		if (::EGifPutExtension(	m_pSaveFile,
								GRAPHICS_EXT_FUNC_CODE,
								4, // Block Size
								pExtension) == GIF_ERROR)
		{
			::EGifCloseFile(m_pSaveFile);
			m_pSaveFile = NULL;
			return FALSE;
		}
	}

	// Put Image Descriptor
	if (::EGifPutImageDesc(	m_pSaveFile,
							Left,
							Top,
							Width, 
							Height,
							FALSE, // Interlace
							m_bHasImageColorTable ? OutputColorMap : NULL) == GIF_ERROR)
	{
		::EGifCloseFile(m_pSaveFile);
		m_pSaveFile = NULL;
		return FALSE;
	}

	// Put Pixels
	DIB_INIT_PROGRESS;
    for (int i = 0 ; i < Height ; i++)
	{
		if ((i & 0x7) == 0)
		{
			if (pThread && pThread->DoExit())
			{
				DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
				return FALSE;
			}
			DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, i, Height);
		}
		if (::EGifPutLine(m_pSaveFile, Ptr, Width) == GIF_ERROR)
		{
			DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
			::EGifCloseFile(m_pSaveFile);
			m_pSaveFile = NULL;
			return FALSE;
		}
		Ptr += nBufLineLen;
    }
	DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

	++m_nSaveImageCount;

	return TRUE;
}

#endif