#include "stdafx.h"
#include "dib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef SUPPORT_LIBTIFF

/*
#include <tiffio.h>
#include <stdio.h>

#define MYTAG_EXIFVERSION 36864

static const TIFFFieldInfo
myFieldInfo[] = {
   { MYTAG_EXIFVERSION,	4, 4,		TIFF_UNDEFINED,	FIELD_CUSTOM,
	  1,	0,	"ExifVersion" }
};

int
main(int argc, char **argv)
{
	TIFF    *tiff;
	char    szVersion[5];
	long    iOffset;

	tiff = TIFFOpen(argv[1], "r");

	if (!TIFFGetField(tiff, TIFFTAG_EXIFIFD, &iOffset))
		return 1;

	TIFFReadCustomDirectory(tiff, iOffset, myFieldInfo,
							sizeof(myFieldInfo)/sizeof(myFieldInfo[0]));

	if (TIFFGetField(tiff, MYTAG_EXIFVERSION, szVersion)) {
		szVersion[4] = '\0';
		printf("EXIF version is: %.2x%.2x%.2x%.2x\n",
			   szVersion[0], szVersion[1], szVersion[2], szVersion[3]);
	}

	TIFFClose(tiff);
	return 0;
}


// Or Could read Exif like this
uint32 exifoffset;
if (TIFFGetField(tif, TIFFTAG_EXIFIFD, &exifoffset))
{
	if (TIFFReadEXIFDirectory(tif, exifoffset))
	{
		long flags = 0;
		FILE* pf = _tfopen(_T("c:\\tifftags.txt"), _T("wt"));
		if (pf)
		{
			TIFFPrintDirectory(tif, pf, flags);
			fclose(pf);
		}
	}
}
*/

BOOL CDib::LoadTIFF(LPCTSTR lpszPathName,
					int nPageNum/*=0*/,
					BOOL bOnlyHeader/*=FALSE*/,
					CWnd* pProgressWnd/*=NULL*/,
					BOOL bProgressSend/*=TRUE*/,
					CWorkerThread* pThread/*=NULL*/)
{
	TIFF* tif = NULL;
	LPBYTE buf = NULL;
	DIB_INIT_PROGRESS;

	// Free
	m_FileInfo.Clear();
	m_bAlpha = FALSE;
	m_bGrayscale = FALSE;
	Free();

	try
	{
		CString sPathName(lpszPathName);
		if (sPathName.IsEmpty())
			throw (int)TIFF_E_ZEROPATH;

		// Check for tiff filename
		if (!::IsTIFF(sPathName))
			throw (int)TIFF_E_WRONGEXTENTION;

		CFile file(lpszPathName, CFile::modeRead | CFile::shareDenyNone);
		m_FileInfo.m_dwFileSize = (DWORD)file.GetLength();
		file.Close();
		if (m_FileInfo.m_dwFileSize == 0)
			throw (int)TIFF_E_FILEEMPTY;       
   
		tif = TIFFOpenW(lpszPathName, "r");   
		if (!tif)
			throw (int)TIFF_E_READ;

		// Pages
		int nNumFrames = TIFFNumberOfDirectories(tif);
		if (nPageNum < 0 || nPageNum >= nNumFrames)
			throw (int)TIFF_E_WRONGPARAMETERS; 
		if (!TIFFSetDirectory(tif, (uint16)nPageNum))
			throw (int)TIFF_E_LIBTIFF;

		// Parse Metadata of the given Page Number
		m_Metadata.ParseTIFF(nPageNum, tif->tif_base, tif->tif_size);

		uint32 uiImageWidth = 0;
		uint32 uiImageHeight = 0;
		uint16 BitsPerSample = 0;
		uint16 SamplePerPixel = 0;
		uint16 PhotometricInterpretation = -1;
		uint16 inkset;
		uint16 planarconfig;
		uint16 compress;
		uint16 orientation;
		float xres = 0.0f;
		float yres = 0.0f;
		uint16 res_unit;
		uint16* sampleinfo;
		uint16 extrasamples;
		uint16 alpha = EXTRASAMPLE_UNSPECIFIED;
		TIFFGetFieldDefaulted(tif, TIFFTAG_IMAGEWIDTH, &uiImageWidth);
		TIFFGetFieldDefaulted(tif, TIFFTAG_IMAGELENGTH, &uiImageHeight);  
		TIFFGetFieldDefaulted(tif, TIFFTAG_BITSPERSAMPLE, &BitsPerSample);
		TIFFGetFieldDefaulted(tif, TIFFTAG_SAMPLESPERPIXEL, &SamplePerPixel);
		TIFFGetFieldDefaulted(tif, TIFFTAG_PHOTOMETRIC, &PhotometricInterpretation);	
		TIFFGetFieldDefaulted(tif, TIFFTAG_INKSET, &inkset);
		TIFFGetFieldDefaulted(tif, TIFFTAG_PLANARCONFIG, &planarconfig);
		TIFFGetFieldDefaulted(tif, TIFFTAG_COMPRESSION, &compress);
		TIFFGetFieldDefaulted(tif, TIFFTAG_ORIENTATION, &orientation);
		TIFFGetFieldDefaulted(tif, TIFFTAG_RESOLUTIONUNIT, &res_unit);
		TIFFGetFieldDefaulted(tif, TIFFTAG_XRESOLUTION, &xres);
		TIFFGetFieldDefaulted(tif, TIFFTAG_YRESOLUTION, &yres);
		switch (res_unit)
		{
			case RESUNIT_CENTIMETER:
				xres *= 2.54f, yres *= 2.54f;
				break;
			case RESUNIT_INCH:
			case RESUNIT_NONE:
			default:
				break;
		}
		/*	The difference between associated alpha and unassociated alpha
			is not just a matter of taste or a matter of maths.
			Associated alpha is generally interpreted as true transparancy
			information. Indeed, the original color values are lost in the
			case of complete transparency, and rounded in the case of partial
			transparency. Also, associated alpha is only logically possible
			as the single extra channel.

			Unassociated alpha channels, on the other hand, can be used to
			encode a number of independent masks, for example.
			The original color data is preserved without rounding.
			Any number of unassociated alpha channels can accompany an image.

			If an extra sample is used to encode information that has little
			or nothing to do with alpha, ExtraSample=0
			(EXTRASAMPLE_UNSPECIFIED) is recommended.
		*/
		TIFFGetFieldDefaulted(tif, TIFFTAG_EXTRASAMPLES, &extrasamples, &sampleinfo);
		if (extrasamples >= 1)
		{
			switch (sampleinfo[0])
			{
				case EXTRASAMPLE_UNSPECIFIED:	/* Workaround for some images without */
					if (SamplePerPixel > 3)		/* correct info about alpha channel */
						alpha = EXTRASAMPLE_ASSOCALPHA;
					break;
				case EXTRASAMPLE_ASSOCALPHA:	/* data is pre-multiplied */
				case EXTRASAMPLE_UNASSALPHA:	/* data is not pre-multiplied */
					alpha = sampleinfo[0];
					break;
			}
		}
		if (extrasamples == 0	&&
			SamplePerPixel == 4	&&
			PhotometricInterpretation == PHOTOMETRIC_RGB)
		{
			alpha = EXTRASAMPLE_ASSOCALPHA;
			extrasamples = 1;
		}
  
		// Number of byte in one line
		int32 LineSize = TIFFScanlineSize(tif);

		// Allocate memory for target DIB Header
		// and set biBitCount
		if (!TIFFIsTiled(tif))
		{
			if ((BitsPerSample*SamplePerPixel) == 1) // 1bit
			{
				m_pBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER) + (sizeof(RGBQUAD)*2)];
				if (m_pBMI == NULL)
					throw (int)TIFF_E_NOMEM;
				m_pBMI->bmiHeader.biBitCount = 1;
				m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + sizeof(BITMAPINFOHEADER));
			}
			else if ((BitsPerSample*SamplePerPixel) <= 4) // 2bit & 4bit
			{
				m_pBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER) + (sizeof(RGBQUAD)*16)];
				if (m_pBMI == NULL)
					throw (int)TIFF_E_NOMEM;
				m_pBMI->bmiHeader.biBitCount = 4;
				m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + sizeof(BITMAPINFOHEADER));
			}
			else if ((BitsPerSample*SamplePerPixel) == 8) // 8bit
			{ 
				m_pBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER) + (sizeof(RGBQUAD)*256)];
				if (m_pBMI == NULL)
					throw (int)TIFF_E_NOMEM;
				m_pBMI->bmiHeader.biBitCount = 8;
				m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + sizeof(BITMAPINFOHEADER));
			}
			else // 16, 24 or 32bit 
			{
				m_pBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER)];
				if (m_pBMI == NULL)
					throw (int)TIFF_E_NOMEM;
				m_pBMI->bmiHeader.biBitCount = 32;
				m_pColors = NULL;
			}
		}
		else // 16, 24 or 32bit 
		{
			m_pBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER)];
			if (m_pBMI == NULL)
				throw (int)TIFF_E_NOMEM;
			m_pBMI->bmiHeader.biBitCount = 32;
			m_pColors = NULL;
		}
		
		// DWORD aligned target DIB ScanLineSize
		// and decoded image size
		DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES((DWORD)uiImageWidth * m_pBMI->bmiHeader.biBitCount);
		m_dwImageSize = uiDIBScanLineSize * (DWORD)uiImageHeight;

		// Initialize BITMAPINFOHEADER
		m_pBMI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		m_pBMI->bmiHeader.biWidth = (DWORD)uiImageWidth; // fill in width from parameter
		m_pBMI->bmiHeader.biHeight = (DWORD)uiImageHeight; // fill in height from parameter
		m_pBMI->bmiHeader.biPlanes = 1; // must be 1
		m_pBMI->bmiHeader.biCompression = BI_RGB;    
		m_pBMI->bmiHeader.biSizeImage = m_dwImageSize;
		m_pBMI->bmiHeader.biXPelsPerMeter = PIXPERMETER(xres);
		m_pBMI->bmiHeader.biYPelsPerMeter = PIXPERMETER(yres);
		m_pBMI->bmiHeader.biClrUsed = 0;
		m_pBMI->bmiHeader.biClrImportant = 0;

		// Init Masks For 16 and 32 bits Pictures
		InitMasks();

		// Init File Info
		m_FileInfo.m_nType = CFileInfo::TIFF;
		m_FileInfo.m_nWidth = m_pBMI->bmiHeader.biWidth;
		m_FileInfo.m_nHeight = m_pBMI->bmiHeader.biHeight;
		m_FileInfo.m_nBpp = BitsPerSample*SamplePerPixel;
		m_FileInfo.m_nCompression = compress;
		m_FileInfo.m_dwImageSize = m_FileInfo.m_nWidth * m_FileInfo.m_nHeight * m_FileInfo.m_nBpp / 8;
		m_FileInfo.m_nXPixsPerMeter = m_pBMI->bmiHeader.biXPelsPerMeter;
		m_FileInfo.m_nYPixsPerMeter = m_pBMI->bmiHeader.biYPelsPerMeter;
		m_FileInfo.m_nNumColors = m_pBMI->bmiHeader.biClrUsed;
		if (PhotometricInterpretation == PHOTOMETRIC_MINISBLACK ||
			PhotometricInterpretation == PHOTOMETRIC_MINISWHITE ||
			PhotometricInterpretation == PHOTOMETRIC_LOGL)
		{
			m_FileInfo.m_nColorSpace = CFileInfo::COLORSPACE_GRAYSCALE;
			m_bGrayscale = TRUE;
		}
		else if (PhotometricInterpretation == PHOTOMETRIC_SEPARATED && inkset == INKSET_CMYK)
			m_FileInfo.m_nColorSpace = CFileInfo::COLORSPACE_CMYK;
		else
			m_FileInfo.m_nColorSpace = CFileInfo::COLORSPACE_RGB;
		m_FileInfo.m_bAlphaChannel =	((alpha == EXTRASAMPLE_ASSOCALPHA) ||
										(alpha == EXTRASAMPLE_UNASSALPHA));
		m_FileInfo.m_bPalette = (PhotometricInterpretation == PHOTOMETRIC_PALETTE);
		m_FileInfo.m_nImageCount = nNumFrames;
		m_FileInfo.m_nImagePos = nPageNum;

		// If only header wanted return now
		if (bOnlyHeader)
		{
			TIFFClose(tif);
			return TRUE;
		}

		//
		// Set Colors
		//
		// PhotometricInterpretation:
		// PHOTOMETRIC_MINISWHITE(=0)	min value is white (bitonal or grayscale)
		// PHOTOMETRIC_MINISBLACK(=1)	min value is black (bitonal or grayscale)
		// PHOTOMETRIC_RGB(=2)			image is RGB
		// PHOTOMETRIC_PALETTE(=3)		image has a color palette
		// PHOTOMETRIC_MASK(=4)			$holdout mask
		// PHOTOMETRIC_SEPARATED(=5)	!color separations
		// PHOTOMETRIC_YCBCR(=6)		!CCIR 601
		// PHOTOMETRIC_CIELAB(=8)		!1976 CIE L*a*b*
		// PHOTOMETRIC_LOGL(=32844)		CIE Log2(L)
		// PHOTOMETRIC_LOGLUV(=32845)	CIE Log2(L) (u',v')
		if ((SamplePerPixel == 1) &&
			(BitsPerSample == 1 || BitsPerSample == 2 || BitsPerSample == 4 || BitsPerSample == 8) &&
			!(TIFFIsTiled(tif)))
		{
			if (PhotometricInterpretation == PHOTOMETRIC_MINISBLACK)
			{
				int NumColors = 1 << BitsPerSample;
				BYTE step = (BYTE)(255 / (NumColors-1));
				// warning: the following ignores possible halftone hints
				for (int i = 0 ; i < NumColors ; i++)
				{
					m_pColors[i].rgbRed = (BYTE)(i*step);
					m_pColors[i].rgbGreen = (BYTE)(i*step);
					m_pColors[i].rgbBlue = (BYTE)(i*step);
					m_pColors[i].rgbReserved = 0;
				}
			}
			else if (PhotometricInterpretation == PHOTOMETRIC_MINISWHITE)
			{
				int NumColors = 1 << BitsPerSample;
				BYTE step = (BYTE)(255 / (NumColors-1));
				// warning: the following ignores possible halftone hints
				for (int i = 0 ; i < NumColors ; i++)
				{
					m_pColors[i].rgbRed = (BYTE)((NumColors - 1 - i)*step);
					m_pColors[i].rgbGreen = (BYTE)((NumColors - 1 - i)*step);
					m_pColors[i].rgbBlue = (BYTE)((NumColors - 1 - i)*step);
					m_pColors[i].rgbReserved = 0;
				}
			}
			else if (PhotometricInterpretation == PHOTOMETRIC_PALETTE)
			{
				uint16* red;
				uint16* r;
				uint16* green;
				uint16* g;
				uint16* blue;
				uint16* b;
				int i; 
				BOOL Palette16Bits = FALSE;          

				TIFFGetField(tif, TIFFTAG_COLORMAP, &red, &green, &blue); 
				r = red;
				g = green;
				b = blue;
				// Is the palette 16 or 8 bits ?
				int NumColors = 1 << BitsPerSample;
				while (NumColors-- > 0)
				{
					if (*r++ >= 256 || *g++ >= 256 || *b++ >= 256)
					{
						Palette16Bits = TRUE;
						break;
					}
					else
						Palette16Bits = FALSE;   
				}

				// Load the palette into the DIB
				for (i = (1 << BitsPerSample) - 1 ; i >= 0 ; i--) 
				{             
					if (Palette16Bits)
					{
						#define CVT(x) (((x) * 255L) / ((1L<<16)-1))
						m_pColors[i].rgbRed =(BYTE)CVT(red[i]);
						m_pColors[i].rgbGreen = (BYTE)CVT(green[i]);
						m_pColors[i].rgbBlue = (BYTE)CVT(blue[i]); 
						m_pColors[i].rgbReserved = 0;
					}
					else
					{
						m_pColors[i].rgbRed = (BYTE)red[i];
						m_pColors[i].rgbGreen = (BYTE)green[i];
						m_pColors[i].rgbBlue = (BYTE)blue[i];
						m_pColors[i].rgbReserved = 0;
					}
				}
			}
		}

		// Create Palette from BMI
		CreatePaletteFromBMI();

		// Prepare a buffer large enough to hold the target DIB image pixels
		m_pBits = (LPBYTE)BIGALLOC(m_dwImageSize + SAFETY_BITALLOC_MARGIN);
		if (m_pBits == NULL)
			throw (int)TIFF_E_NOMEM;

		// In the tiff file the scan lines are top / down 
		// In a DIB the scan lines must be saved bottom / up
		LPBYTE lpBits = m_pBits;
		lpBits += uiDIBScanLineSize*(uiImageHeight-1);

		// Decode 1bit ... 8bit images
		if ((SamplePerPixel == 1) &&
			(BitsPerSample == 1 || BitsPerSample == 2 || BitsPerSample == 4 || BitsPerSample == 8) &&
			!(TIFFIsTiled(tif)))
		{
			buf = (LPBYTE)new BYTE[LineSize];
			if (!buf)
				throw (int)TIFF_E_NOMEM;

			for (uint32 row = 0 ; row < uiImageHeight ; row++)
			{
				if ((row & 0x7) == 0)
				{
					if (pThread && pThread->DoExit())
						throw (int)TIFF_THREADEXIT;
					DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, row, uiImageHeight);
				}

				uint16 x;
				int status = TIFFReadScanline(tif, buf, row, 0);
				if ((status == -1) && (row < uiImageHeight / 3))
					throw (int)TIFF_E_LIBTIFF;
				if (BitsPerSample == 2) // 2bit image
				{
					// 2bit -> 4bit conversion
					for (unsigned int pixel = 0 ; pixel < uiImageWidth ; pixel++)
					{
						int index = (buf[pixel/4] >> (2*(3-pixel%4))) & 0x03;
						lpBits[pixel/2] &= ~(0x0F << (4*(1-pixel%2)));
						lpBits[pixel/2] |= (index << (4*(1-pixel%2)));
					}
				}
				else if (BitsPerSample == 4) // 4bit image
				{
					for (x=0 ; x < uiImageWidth / 2 ; x++)
						lpBits[x] = buf[x];
					// Odd number of pixels
					if (uiImageWidth & 1)
						lpBits[x] = (BYTE)(buf[x] & 0xf0);
				}
				else // 8bit image or 1bit image
					memcpy(lpBits, buf, LineSize);
				lpBits -= uiDIBScanLineSize;
			}

			// Clean-up
			delete [] buf;
			buf = NULL;

			DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
		}
		// Decode all other image formats to a 32bit image
		else
		{
			char emsg[1024] = "";
			TIFFRGBAImage img;

			// Check whether file is supported
			if (!TIFFRGBAImageOK(tif, emsg))
				throw (int)TIFF_E_LIBTIFF;

			// RGBA Image Read Begin
			if (!TIFFRGBAImageBegin(&img, tif, 1, emsg))
				throw (int)TIFF_E_LIBTIFF;

			// Init img
			img.hProgressWnd = (void*)pProgressWnd->GetSafeHwnd();
			img.bProgressSend = (int)bProgressSend;
			if (pThread)
				img.hKillEvent = (void*)pThread->GetKillEvent();
			else
				img.hKillEvent = (void*)NULL;
			img.bForceVerticalFlip = 1;

			// RGBA Read Image
			if (!TIFFRGBAImageGet(&img, (uint32*)m_pBits, img.width, img.height))
			{
				TIFFRGBAImageEnd(&img);
				if (pThread && pThread->DoExit())
					throw (int)TIFF_THREADEXIT;
				else
					throw (int)TIFF_E_LIBTIFF;
			}

			// Correct the byte ordering
			CDib::RGBA2BGRA(m_pBits, img.width * img.height);

			// Has Alpha?
			if (img.alpha)
				m_bAlpha = TRUE;

			// Clean-up
			TIFFRGBAImageEnd(&img);
		}

		// Free
		TIFFClose(tif);

		return TRUE;
	}
	catch (CFileException* e)
	{
		TCHAR szCause[255];
		CString str(_T("LoadTIFF: "));
		e->GetErrorMessage(szCause, 255);
		str += szCause;
		str += _T("\n");
		TRACE(str);
		if (m_bShowMessageBoxOnError)
			::AfxMessageBox(str, MB_ICONSTOP);
		e->Delete();
		DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
		return FALSE;
	}
	catch (int error_code)
	{
		if (tif)
			TIFFClose(tif);
		if (buf)
			delete [] buf;

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

		DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

		// Just Exit
		if (error_code == TIFF_THREADEXIT)
			return FALSE;

		CString str;
#ifdef _DEBUG
		str.Format(_T("LoadTIFF(%s):\n"), lpszPathName);
#endif
		switch (error_code)
		{
			case TIFF_E_WRONGPARAMETERS :	str += _T("The Function Parameters Are Wrong!\n");
			break;
			case TIFF_E_ZEROPATH :			str += _T("The file name is zero\n");
			break;
			case TIFF_E_WRONGEXTENTION :	str += _T("The file extention is wrong\n");
			break;
			case TIFF_E_NOMEM :				str += _T("Could not alloc memory\n");
			break;
			case TIFF_E_READ :				str += _T("Couldn't read TIFF file\n");
			break;
			case TIFF_E_FILEEMPTY :			str += _T("File is empty\n");
			break;
			case TIFF_E_LIBTIFF :			str += _T("Cannot load file\n");
			break;
			default:						str += _T("Unspecified error\n");
			break;
		}
		
		TRACE(str);
		if (m_bShowMessageBoxOnError)
			::AfxMessageBox(str, MB_ICONSTOP);

		return FALSE;
	}
}

BOOL CDib::SaveTIFF(LPCTSTR lpszPathName,
					int nCompression/*=-1*/,	// -1: automatically select compression
					int nJpegQuality/*=DEFAULT_JPEGCOMPRESSION*/,
					CWnd* pProgressWnd/*=NULL*/,
					BOOL bProgressSend/*=TRUE*/,
					CWorkerThread* pThread/*=NULL*/)
{
	TIFF* tif = NULL;

	// Save TIFF
	BOOL res = SaveFirstTIFF(	lpszPathName,
								&tif,
								1,				// One Page Image
								nCompression,
								nJpegQuality,
								pProgressWnd,
								bProgressSend,
								pThread);

	// Only close if successful, otherwise tif has already
	// been closed by the above function.
	if (res)
		TIFFClose(tif);

	return res;
}

BOOL CDib::SaveFirstTIFF(	LPCTSTR lpszPathName,
							TIFF** ptif,
							int nPageCount/*=1*/,
							int nCompression/*=-1*/,	// -1: automatically select compression
							int nJpegQuality/*=DEFAULT_JPEGCOMPRESSION*/,
							CWnd* pProgressWnd/*=NULL*/,
							BOOL bProgressSend/*=TRUE*/,
							CWorkerThread* pThread/*=NULL*/)
{
	try
	{
		if (!ptif)
			throw (int)TIFF_E_WRONGPARAMETERS;

		CString sPathName(lpszPathName);
		if (sPathName.IsEmpty())
			throw (int)TIFF_E_ZEROPATH;

		DWORD dwAttrib = ::GetFileAttributes(lpszPathName);
		if ((dwAttrib != 0xFFFFFFFF) && (dwAttrib & FILE_ATTRIBUTE_READONLY))
			throw (int)TIFF_E_FILEREADONLY;

		if (!m_pBits)
		{
			if (!DibSectionToBits())
				throw (int)TIFF_E_BADBMP;
		}

		if (!m_pBits || !m_pBMI)
			throw (int)TIFF_E_BADBMP;

		if (IsCompressed())
		{
			if (!Decompress(GetBitCount())) // Decompress
				throw (int)TIFF_E_BADBMP;
		}
   
		*ptif = TIFFOpenW(lpszPathName, "w");   
		if (!(*ptif))
			throw (int)TIFF_E_WRITE;

		if (SaveNextTIFF(	*ptif,
							0,
							nPageCount,
							nCompression,
							nJpegQuality,
							pProgressWnd,
							bProgressSend,
							pThread))
			return TRUE;
		else
		{
			TIFFClose(*ptif);
			*ptif = NULL;
			return FALSE;
		}
	}
	catch (int error_code)
	{
		if (ptif && *ptif)
		{
			TIFFClose(*ptif);
			*ptif = NULL;
		}

		// Just Exit
		if (error_code == TIFF_THREADEXIT)
			return FALSE;

		CString str;
#ifdef _DEBUG
		str.Format(_T("SaveFirstTIFF(%s):\n"), lpszPathName);
#endif
		switch (error_code)
		{
			case TIFF_E_WRONGPARAMETERS :	str += _T("The Function Parameters Are Wrong!\n");
			break;
			case TIFF_E_ZEROPATH :			str += _T("The file name is zero\n");
			break;
			case TIFF_E_FILEREADONLY :		str += _T("The file is read only\n");
			break;
			case TIFF_E_BADBMP :			str += _T("Corrupted or unsupported DIB\n");
			break;
			case TIFF_E_WRITE :				str += _T("Couldn't write TIFF file\n");
			break;
			default:						str += _T("Unspecified error\n");
			break;
		}
		
		TRACE(str);
		if (m_bShowMessageBoxOnError)
			::AfxMessageBox(str, MB_ICONSTOP);

		return FALSE;
	}
}

static const TIFFFieldInfo
gpsFieldInfo[] = {
	{ TAG_GPS_VERSION,		4, 4,			TIFF_BYTE,		FIELD_CUSTOM,
      1,	0,	"GpsVersion" },
	{ TAG_GPS_LAT,			3, 3,			TIFF_RATIONAL,	FIELD_CUSTOM, 
      1,	0,	"GpsLatitude" },
	{ TAG_GPS_LONG,			3, 3,			TIFF_RATIONAL,	FIELD_CUSTOM, 
      1,	0,	"GpsLongitude" },
	{ TAG_GPS_ALT,			1, 1,			TIFF_RATIONAL,	FIELD_CUSTOM, 
      1,	0,	"GpsAltitude" },
	{ TAG_GPS_LAT_REF,		2, 2,			TIFF_ASCII,		FIELD_CUSTOM,
      1,	0,	"GpsLatitudeReference" },
	{ TAG_GPS_LONG_REF,		2, 2,			TIFF_ASCII,		FIELD_CUSTOM,
      1,	0,	"GpsLongitudeReference" },
	{ TAG_GPS_ALT_REF,		1, 1,			TIFF_BYTE,		FIELD_CUSTOM,
      1,	0,	"GpsAltitudeReference" },
	{ TAG_GPS_TIMESTAMP,	3, 3,			TIFF_RATIONAL,	FIELD_CUSTOM, 
      1,	0,	"GpsTimeStamp" },
	{ TAG_GPS_MAPDATUM,		-1, -1,			TIFF_ASCII,		FIELD_CUSTOM,
      1,	0,	"GpsMapDatum" }
};

int TIFFGetGpsField(void* userdata, TIFF* tif, ttag_t tag, ...)
{
	int bSet = 0;
	va_list ap;
	CMetadata::EXIFINFO* pExifInfo = (CMetadata::EXIFINFO*)userdata;
	va_start(ap, tag);

	switch (tag)
	{
		case TAG_GPS_VERSION :
			// Allocate Buffer, do not forget to free it!
			if (pExifInfo->pUserData)
				delete [] pExifInfo->pUserData;
			pExifInfo->pUserData = new BYTE[4]; 
			
			// Copy Data
			if (pExifInfo->pUserData)
			{
				// Min. Version
				pExifInfo->pUserData[0] = DEFAULT_GPS_VERSION_0;
				pExifInfo->pUserData[1] = DEFAULT_GPS_VERSION_1;
				pExifInfo->pUserData[2] = DEFAULT_GPS_VERSION_2;
				pExifInfo->pUserData[3] = DEFAULT_GPS_VERSION_3;
				char sVer[5];
				memcpy(sVer, pExifInfo->GpsVersion, 4);
				sVer[0] += '0'; sVer[1] += '0'; sVer[2] += '0'; sVer[3] += '0';
				sVer[4] = '\0';
				int nVer = atoi(sVer);
				if (nVer > DEFAULT_GPS_VERSION_INT)
					memcpy(pExifInfo->pUserData, pExifInfo->GpsVersion, 4);
				*va_arg(ap, char**) = (char*)(pExifInfo->pUserData);
				bSet = 1;
			}
			break;
		case TAG_GPS_LAT :
			if (pExifInfo->GpsLat[GPS_DEGREE]  >= 0.0f	&&
				pExifInfo->GpsLat[GPS_MINUTES] >= 0.0f	&&
				pExifInfo->GpsLat[GPS_SECONDS] >= 0.0f)
			{
				*va_arg(ap, float**) = pExifInfo->GpsLat;
				bSet = 1;
			}
			break;
		case TAG_GPS_LONG :
			if (pExifInfo->GpsLong[GPS_DEGREE]  >= 0.0f	&&
				pExifInfo->GpsLong[GPS_MINUTES] >= 0.0f	&&
				pExifInfo->GpsLong[GPS_SECONDS] >= 0.0f)
			{
				*va_arg(ap, float**) = pExifInfo->GpsLong;
				bSet = 1;
			}
			break;
		case TAG_GPS_ALT :
			if (pExifInfo->GpsAlt >= 0.0f)
			{
				*va_arg(ap, float*) = pExifInfo->GpsAlt;
				bSet = 1;
			}
			break;
		case TAG_GPS_LAT_REF :
			if (pExifInfo->GpsLatRef[0])
			{
				*va_arg(ap, char**) = pExifInfo->GpsLatRef;
				bSet = 1;
			}
			break;
		case TAG_GPS_LONG_REF :
			if (pExifInfo->GpsLongRef[0])
			{
				*va_arg(ap, char**) = pExifInfo->GpsLongRef;
				bSet = 1;
			}
			break;
		case TAG_GPS_ALT_REF :
			if (pExifInfo->GpsAltRef >= 0)
			{
				*va_arg(ap, uint8*) = (uint8)pExifInfo->GpsAltRef;
				bSet = 1;
			}
			break;
		case TAG_GPS_TIMESTAMP :
			if (pExifInfo->GpsTime[GPS_HOUR]	>= 0.0f	&&
				pExifInfo->GpsTime[GPS_MINUTES] >= 0.0f	&&
				pExifInfo->GpsTime[GPS_SECONDS] >= 0.0f)
			{
				*va_arg(ap, float**) = pExifInfo->GpsTime;
				bSet = 1;
			}
			break;
		case TAG_GPS_MAPDATUM :
			if (pExifInfo->GpsMapDatum[0])
			{
				*va_arg(ap, char**) = pExifInfo->GpsMapDatum;
				bSet = 1;
			}
			break;
		default :
			break;
	}

	va_end(ap);
	
	return bSet;
}

int TIFFGetExifField(void* userdata, TIFF* tif, ttag_t tag, ...)
{
	int bSet = 0;
	va_list ap;
	CMetadata::EXIFINFO* pExifInfo = (CMetadata::EXIFINFO*)userdata;
	va_start(ap, tag);

	switch (tag)
	{
		case EXIFTAG_EXPOSURETIME :
			if (pExifInfo->ExposureTime)
			{
				*va_arg(ap, float*) = pExifInfo->ExposureTime;
				bSet = 1;
			}
			break;
		case EXIFTAG_FNUMBER :
			if (pExifInfo->ApertureFNumber)
			{
				*va_arg(ap, float*) = pExifInfo->ApertureFNumber;
				bSet = 1;
			}
			break;
		case EXIFTAG_EXPOSUREPROGRAM :
			if (pExifInfo->ExposureProgram)
			{
				*va_arg(ap, uint16*) = pExifInfo->ExposureProgram;
				bSet = 1;
			}
			break;
		case EXIFTAG_EXPOSUREMODE :
			if (pExifInfo->ExposureMode)
			{
				*va_arg(ap, uint16*) = pExifInfo->ExposureMode;
				bSet = 1;
			}
			break;
		case EXIFTAG_EXIFVERSION :
			if (pExifInfo->Version[0])
			{
				// Allocate Buffer, do not forget to free it!
				if (pExifInfo->pUserData)
					delete [] pExifInfo->pUserData;
				pExifInfo->pUserData = new BYTE[5]; 
				
				// Copy Data
				if (pExifInfo->pUserData)
				{
					strcpy((char*)pExifInfo->pUserData, DEFAULT_EXIF_VERSION_STR);	// Min. Version
					pExifInfo->Version[4] = '\0';									// Just in case...
					int nVer = atoi(pExifInfo->Version);
					if (nVer > DEFAULT_EXIF_VERSION_INT)
						strcpy((char*)pExifInfo->pUserData, pExifInfo->Version);
					*va_arg(ap, char**) = (char*)(pExifInfo->pUserData);
					bSet = 1;
				}
			}
			break;
		case EXIFTAG_DATETIMEORIGINAL :
		case EXIFTAG_DATETIMEDIGITIZED :
			if (pExifInfo->DateTime[0])
			{
				*va_arg(ap, char**) = pExifInfo->DateTime;
				bSet = 1;
			}
			break;
		case EXIFTAG_FLASH :
			if (pExifInfo->Flash >= 0)
			{
				*va_arg(ap, uint16*) = (uint16)pExifInfo->Flash;
				bSet = 1;
			}
			break;
		case EXIFTAG_ISOSPEEDRATINGS :
			if (pExifInfo->ISOequivalent)
			{
				*va_arg(ap, uint16*) = 1; // Count
				*va_arg(ap, uint16**) = &pExifInfo->ISOequivalent;
				bSet = 1;
			}
			break;
		case EXIFTAG_FOCALLENGTH :
			if (pExifInfo->FocalLength)
			{
				*va_arg(ap, float*) = pExifInfo->FocalLength;
				bSet = 1;
			}
			break;
		case EXIFTAG_SUBJECTDISTANCE :
			if (pExifInfo->Distance)
			{
				*va_arg(ap, float*) = pExifInfo->Distance;
				bSet = 1;
			}
			break;
		case EXIFTAG_EXPOSUREBIASVALUE :
			if (pExifInfo->ExposureBias)
			{
				*va_arg(ap, float*) = pExifInfo->ExposureBias;
				bSet = 1;
			}
			break;
		case EXIFTAG_DIGITALZOOMRATIO :
			if (pExifInfo->DigitalZoomRatio)
			{
				*va_arg(ap, float*) = pExifInfo->DigitalZoomRatio;
				bSet = 1;
			}
			break;
		case EXIFTAG_FOCALLENGTHIN35MMFILM :
			if (pExifInfo->FocalLength35mmEquiv)
			{
				*va_arg(ap, uint16*) = (uint16)pExifInfo->FocalLength35mmEquiv;
				bSet = 1;
			}
			break;
		case EXIFTAG_SUBJECTDISTANCERANGE :
			if (pExifInfo->DistanceRange)
			{
				*va_arg(ap, uint16*) = (uint16)pExifInfo->DistanceRange;
				bSet = 1;
			}
			break;
		case EXIFTAG_WHITEBALANCE :
			if (pExifInfo->WhiteBalance >= 0)
			{
				*va_arg(ap, uint16*) = (uint16)pExifInfo->WhiteBalance;
				bSet = 1;
			}
			break;
		case EXIFTAG_METERINGMODE :
			if (pExifInfo->MeteringMode)
			{
				*va_arg(ap, uint16*) = (uint16)pExifInfo->MeteringMode;
				bSet = 1;
			}
			break;
		case EXIFTAG_LIGHTSOURCE :
			if (pExifInfo->LightSource)
			{
				*va_arg(ap, uint16*) = (uint16)pExifInfo->LightSource;
				bSet = 1;
			}
			break;
		case EXIFTAG_COMPRESSEDBITSPERPIXEL :
			if (pExifInfo->CompressionLevel)
			{
				*va_arg(ap, float*) = pExifInfo->CompressionLevel;
				bSet = 1;
			}
			break;
		case EXIFTAG_FOCALPLANEXRESOLUTION :
			if (pExifInfo->FocalplaneXRes)
			{
				*va_arg(ap, float*) = pExifInfo->FocalplaneXRes;
				bSet = 1;
			}
			break;
		case EXIFTAG_FOCALPLANEYRESOLUTION :
			if (pExifInfo->FocalplaneYRes)
			{
				*va_arg(ap, float*) = pExifInfo->FocalplaneYRes;
				bSet = 1;
			}
			break;
		case EXIFTAG_FOCALPLANERESOLUTIONUNIT :
			if (pExifInfo->FocalplaneUnits)
			{
				uint16 ResolutionUnit;
				if (pExifInfo->FocalplaneUnits == 25.4f)
					ResolutionUnit = 2;
				else if (pExifInfo->FocalplaneUnits == 10.0f)
					ResolutionUnit = 3;
				else if (pExifInfo->FocalplaneUnits == 1.0f)
					ResolutionUnit = 4;
				else if (pExifInfo->FocalplaneUnits == 0.001f)
					ResolutionUnit = 5;
				else
					ResolutionUnit = 1;
				*va_arg(ap, uint16*) = ResolutionUnit;
				bSet = 1;
			}
			break;
		case EXIFTAG_BRIGHTNESSVALUE :
			if (pExifInfo->Brightness)
			{
				*va_arg(ap, float*) = pExifInfo->Brightness;
				bSet = 1;
			}
			break;
		case EXIFTAG_USERCOMMENT :
			if (pExifInfo->UserComment[0])
			{
				// String Len
				int Len = wcslen(pExifInfo->UserComment) + 1;
				
				// Allocate Buffer, do not forget to free it!
				if (pExifInfo->pUserData)
					delete [] pExifInfo->pUserData;
				pExifInfo->pUserData = new BYTE[USER_COMMENT_HEADER_SIZE + 2*Len]; 
				
				// Copy Data
				if (pExifInfo->pUserData)
				{
					memcpy(pExifInfo->pUserData, CMetadata::m_USER_COMMENT_UNICODE, USER_COMMENT_HEADER_SIZE);
					memcpy(pExifInfo->pUserData + USER_COMMENT_HEADER_SIZE, pExifInfo->UserComment, 2*Len);
					*va_arg(ap, uint16*) = USER_COMMENT_HEADER_SIZE + 2*Len; // Count
					*va_arg(ap, char**) = (char*)pExifInfo->pUserData;
					bSet = 1;
				}
			}
			break;
		default :
			break;
	}

	va_end(ap);
	
	return bSet;
}

void CDib::TIFFFreeUserData()
{
	if (GetExifInfo()->pUserData)
	{
		delete [] GetExifInfo()->pUserData;
		GetExifInfo()->pUserData = NULL;
	}
}

BOOL CDib::TIFFSetMetadata(TIFF* tif, BOOL bRemoveIcc/*=FALSE*/)
{
	// Set User Data to Zero
	GetExifInfo()->pUserData = NULL;

	// Write Exif Sub IFD
	size_t exifFieldInfoCount;
	const TIFFFieldInfo* exifFieldInfo = _TIFFGetExifFieldInfo(&exifFieldInfoCount);
	toff_t exifoffset = TIFFWritePrivateDataSubDirectory(	tif,
															exifFieldInfo,
															exifFieldInfoCount,
															TIFFGetExifField,
															(void*)GetExifInfo());
	TIFFFreeUserData(); // Allocated by TIFFGetExifField
	if (exifoffset)
		TIFFSetField(tif, TIFFTAG_EXIFIFD, exifoffset);

	// Write Gps Sub IFD
	if (GetExifInfo()->bGpsInfoPresent)
	{
		GetExifInfo()->pUserData = NULL;
		size_t gpsFieldInfoCount = sizeof(gpsFieldInfo) / sizeof(gpsFieldInfo[0]);
		toff_t gpsoffset = TIFFWritePrivateDataSubDirectory(	tif,
																gpsFieldInfo,
																gpsFieldInfoCount,
																TIFFGetGpsField,
																(void*)GetExifInfo());
		TIFFFreeUserData(); // Allocated by TIFFGetGpsField
		if (gpsoffset)
			TIFFSetField(tif, TIFFTAG_GPSIFD, gpsoffset);
	}

	// Orientation
	if (GetExifInfo()->Orientation)
		TIFFSetField(tif, TIFFTAG_ORIENTATION, GetExifInfo()->Orientation);
	else
		TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);

	// Image Description
	if (GetExifInfo()->ImageDescription[0])
		TIFFSetField(tif, TIFFTAG_IMAGEDESCRIPTION, GetExifInfo()->ImageDescription);

	// Make
	if (GetExifInfo()->CameraMake[0])
		TIFFSetField(tif, TIFFTAG_MAKE, GetExifInfo()->CameraMake);

	// Model
	if (GetExifInfo()->CameraModel[0])
		TIFFSetField(tif, TIFFTAG_MODEL, GetExifInfo()->CameraModel);

	// Software
	if (GetExifInfo()->Software[0])
		TIFFSetField(tif, TIFFTAG_SOFTWARE, GetExifInfo()->Software);
	else
		TIFFSetField(tif, TIFFTAG_SOFTWARE, APPNAME_NOEXT_ASCII);

	// Artist
	if (GetExifInfo()->Artist[0])
		TIFFSetField(tif, TIFFTAG_ARTIST, GetExifInfo()->Artist);

	// Copyright
	if (GetExifInfo()->CopyrightPhotographer[0])
	{
		int LenPhoto = strlen(GetExifInfo()->CopyrightPhotographer) + 1;
		int LenEditor = 0;
		if (GetExifInfo()->CopyrightEditor[0])
			LenEditor = strlen(GetExifInfo()->CopyrightEditor) + 1;

		// Allocate
		GetExifInfo()->pUserData = new BYTE[LenPhoto + LenEditor];
		if (GetExifInfo()->pUserData)
		{
			memcpy(GetExifInfo()->pUserData, GetExifInfo()->CopyrightPhotographer, LenPhoto);
			if (GetExifInfo()->CopyrightEditor[0])
			{
				GetExifInfo()->pUserData[LenPhoto - 1] = ' '; // Replace '\0' with ' '
				memcpy(GetExifInfo()->pUserData + LenPhoto, GetExifInfo()->CopyrightEditor, LenEditor);
			}
			TIFFSetField(tif, TIFFTAG_COPYRIGHT, GetExifInfo()->pUserData);
		}
	}

	// Data & Time
	if (GetExifInfo()->DateTime[0])
		TIFFSetField(tif, TIFFTAG_DATETIME, GetExifInfo()->DateTime);

	// Xmp Packet
	if (m_Metadata.m_pXmpData && m_Metadata.m_dwXmpSize > 0)
	{
		// Call this to change to the image/tiff mime
		// when saving from a jpeg file for example!
		m_Metadata.UpdateXmpData(_T("image/tiff"));

		TIFFSetField(tif, TIFFTAG_XMLPACKET, (uint32)m_Metadata.m_dwXmpSize, (void*)m_Metadata.m_pXmpData);
	}

	// Legacy Iptc
	if (m_Metadata.m_pIptcLegacyData && m_Metadata.m_dwIptcLegacySize > 0)
	{
		unsigned long dwDWordsCount = (m_Metadata.m_dwIptcLegacySize % 4) ? (m_Metadata.m_dwIptcLegacySize / 4) + 1 : m_Metadata.m_dwIptcLegacySize / 4;

		// Swab it here because saving it as big endian on this
		// little endian machine (windows) will swab it again.
		// Iptc data are in reality of type UNDEFINED (just bytes),
		// but a historical bug attributes it type LONG...
		if (tif->tif_header.tiff_magic == TIFF_BIGENDIAN)
			TIFFSwabArrayOfLong((uint32*)(m_Metadata.m_pIptcLegacyData), dwDWordsCount);

		TIFFSetField(tif, TIFFTAG_RICHTIFFIPTC, (uint32)dwDWordsCount, (void*)m_Metadata.m_pIptcLegacyData);
	}

	// ICC Profile
	if (!bRemoveIcc)
	{
		if (m_Metadata.m_pIccData && m_Metadata.m_dwIccSize > 0)
			TIFFSetField(tif, TIFFTAG_ICCPROFILE, (uint32)m_Metadata.m_dwIccSize, (void*)m_Metadata.m_pIccData);
	}

	// Photoshop
	if (m_Metadata.m_pPhotoshopData && m_Metadata.m_dwPhotoshopSize > 0)
		TIFFSetField(tif, TIFFTAG_PHOTOSHOP, (uint32)m_Metadata.m_dwPhotoshopSize, (void*)m_Metadata.m_pPhotoshopData);

	return TRUE;
}

BOOL CDib::SaveNextTIFF(	TIFF* tif,
							int nPageNum/*=0*/,
							int nPageCount/*=1*/,
							int nCompression/*=-1*/,	// -1: automatically select compression
							int nJpegQuality/*=DEFAULT_JPEGCOMPRESSION*/,
							CWnd* pProgressWnd/*=NULL*/,
							BOOL bProgressSend/*=TRUE*/,
							CWorkerThread* pThread/*=NULL*/)
{
	LPBYTE buf = NULL;
	uint16 ui16;
	DIB_INIT_PROGRESS;

	try
	{
		if (!tif)
			throw (int)TIFF_E_WRONGPARAMETERS;

		if (!m_pBits)
		{
			if (!DibSectionToBits())
				throw (int)TIFF_E_BADBMP;
		}

		if (!m_pBits || !m_pBMI)
			throw (int)TIFF_E_BADBMP;

		if (IsCompressed())
		{
			if (!Decompress(GetBitCount())) // Decompress
				throw (int)TIFF_E_BADBMP;
		}

		// Set Metadata,
		// we have to remove the ICC profile when saving a CMYK or YCCK image
		// because we use the RGB color space for saving!
		BOOL bRemoveIcc = (	m_FileInfo.m_nColorSpace == CFileInfo::COLORSPACE_YCCK ||
							m_FileInfo.m_nColorSpace == CFileInfo::COLORSPACE_CMYK);
		TIFFSetMetadata(tif, bRemoveIcc);

		// Multi-paging
		if (nPageCount == 1)
			TIFFSetField(tif, TIFFTAG_SUBFILETYPE, 0);
		else
		{
			TIFFSetField(tif, TIFFTAG_SUBFILETYPE, FILETYPE_PAGE);

			// A Page Count of 0 means that we do not know how many pages!
			if (nPageCount > 1)
			{
				// The first page is numbered 0 (zero).
				// nPageCount is the total number of pages in the document.
				// If nPageCount is 0, the total number of pages in the
				// document is not available
				TIFFSetField(tif, TIFFTAG_PAGENUMBER, nPageNum, nPageCount);
				
				// Page Name
				char sPageNumber[20];
				sprintf(sPageNumber, "Page %d", nPageNum+1);
				TIFFSetField(tif, TIFFTAG_PAGENAME, sPageNumber);
			}
		}	

		// Width & Height
		TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, (uint32)GetWidth());
		TIFFSetField(tif, TIFFTAG_IMAGELENGTH, (uint32)GetHeight());
	
		// Depth
		ui16 = (uint16)GetBitCount();
		if (ui16 > 8) 
			ui16 = 8;
		TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, ui16);
		ui16 = (uint16)GetBitCount();
		ui16 = (uint16)(ui16 <= 8 ? 1 : (m_bAlpha ? 4 : 3));
		TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, ui16);
		
		// Planar Configuration
		ui16 = PLANARCONFIG_CONTIG;
		TIFFSetField(tif, TIFFTAG_PLANARCONFIG, ui16);

		// Rows per strip
		uint32 rowsperstrip = TIFFDefaultStripSize(tif, (uint32)-1);
		TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, rowsperstrip);

		// Physical Resolution
		ui16 = RESUNIT_INCH;
		float xres = (float)GetXDpi();
		float yres = (float)GetYDpi();
		TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT, ui16);
		TIFFSetField(tif, TIFFTAG_XRESOLUTION, xres);
		TIFFSetField(tif, TIFFTAG_YRESOLUTION, yres);

		// Alpha
		if (m_bAlpha)
		{
			uint16 sampleinfo[1];
			sampleinfo[0] = EXTRASAMPLE_ASSOCALPHA;
			TIFFSetField(tif, TIFFTAG_EXTRASAMPLES, 1, sampleinfo);
		}

		// Compression
		uint16 compress;
		if (nCompression == -1)
		{
			switch (GetBitCount())
			{
				case 1 :
					compress = COMPRESSION_CCITTFAX4;
					break;
				case 4 :
				case 8 :
					compress = COMPRESSION_LZW;
					break;
				case 16 :
				case 24 :
				case 32 :
					compress = COMPRESSION_JPEG;
					break;
				default :
					compress = COMPRESSION_NONE;
					break;
			}
		}
		else
			compress = (uint16)nCompression;

		// Check and change compression if necessary
		if (compress == COMPRESSION_OJPEG)
			compress = COMPRESSION_JPEG;
		if (compress == COMPRESSION_SGILOG		||
			compress == COMPRESSION_SGILOG24	||
			compress == COMPRESSION_THUNDERSCAN	||
			compress == COMPRESSION_NEXT)
			compress = COMPRESSION_LZW;
		if (compress == COMPRESSION_JPEG)
		{
			if (GetBitCount() == 1)
				compress = COMPRESSION_CCITTFAX4;
			else if (GetBitCount() == 4)
				compress = COMPRESSION_LZW;
			else if (GetBitCount() == 8)
			{
				UpdateGrayscaleFlag();
				if (IsGrayscale())
					MakeGrayscaleAscending();
				else
					compress = COMPRESSION_LZW;
			}
		}
		if (GetBitCount() != 1 &&
			(compress == COMPRESSION_CCITTRLE	||
			compress == COMPRESSION_CCITTFAX3	||
			compress == COMPRESSION_CCITTFAX4))
			compress = COMPRESSION_LZW;

		// Set Compression
		TIFFSetField(tif, TIFFTAG_COMPRESSION, compress);
		switch (compress)
		{
			case COMPRESSION_LZW :
				if (GetBitCount() >= 8)
					TIFFSetField(tif, TIFFTAG_PREDICTOR, 2);
				break;

			case COMPRESSION_JPEG :
				// The Jpeg quality is a "pseudo tags",
				// which means that the tag is not written to the file!
				TIFFSetField(tif, TIFFTAG_JPEGQUALITY, nJpegQuality);
				TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, ((7+rowsperstrip)>>3)<<3);
				break;

			default :
				break;
		}

		// Scan Line Sizes
		DWORD uiOutputDIBScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * (m_bAlpha ? 32 : 24));
		DWORD uiInputDIBScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * GetBitCount());

		// In the tiff file the scan lines are top / down 
		// In a DIB the scan lines are stored bottom / up
		LPBYTE lpBits = m_pBits;
		lpBits += uiInputDIBScanLineSize*(GetHeight()-1);

		// 8 bpp or less depth
		uint16 PhotometricInterpretation = PHOTOMETRIC_PALETTE;
		if (GetNumColors() != 0)
		{
			// For Jpeg the Grayscale levels have already
			// been made ascending, see some lines above!
			if (compress == COMPRESSION_JPEG)
				PhotometricInterpretation = PHOTOMETRIC_MINISBLACK;
			else
			{
				// Can Store as Grayscale?
				if (GetBitCount() == 1)
				{
					if (m_pColors[0].rgbRed		== 0	&& m_pColors[1].rgbRed		== 255	&&
						m_pColors[0].rgbGreen	== 0	&& m_pColors[1].rgbGreen	== 255	&&
						m_pColors[0].rgbBlue	== 0	&& m_pColors[1].rgbBlue		== 255)
						PhotometricInterpretation = PHOTOMETRIC_MINISBLACK;
					else if (	m_pColors[0].rgbRed		== 255	&& m_pColors[1].rgbRed		== 0	&&
								m_pColors[0].rgbGreen	== 255	&& m_pColors[1].rgbGreen	== 0	&&
								m_pColors[0].rgbBlue	== 255	&& m_pColors[1].rgbBlue		== 0)
						PhotometricInterpretation = PHOTOMETRIC_MINISWHITE;
				}
				else
				{
					UpdateGrayscaleFlag();
					if (IsGrayscale())
					{				
						MakeGrayscaleAscending();
						PhotometricInterpretation = PHOTOMETRIC_MINISBLACK;
					}
				}
				
				// Store as Palettized,
				// 8 bit palette to 16 bit palette conversion
				if (PhotometricInterpretation == PHOTOMETRIC_PALETTE)
				{
					uint16 red[256];
					uint16 green[256];
					uint16 blue[256];
					for (int i = 0 ; i < GetNumColors() ; i++)
					{
						red[i]   = (uint16)(m_pColors[i].rgbRed		<< 8);
						green[i] = (uint16)(m_pColors[i].rgbGreen	<< 8);
						blue[i]  = (uint16)(m_pColors[i].rgbBlue	<< 8);
					}
					TIFFSetField(tif, TIFFTAG_COLORMAP, red, green, blue);
				}
			}
		}

		switch (GetBitCount())
		{
			case 1:
			{
				TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PhotometricInterpretation);

				buf = new BYTE[uiInputDIBScanLineSize];
				if (!buf)
					throw (int)TIFF_E_NOMEM;
				for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
				{
					if ((CurLine & 0x7) == 0)
					{
						if (pThread && pThread->DoExit())
							throw (int)TIFF_THREADEXIT;
						DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, CurLine, GetHeight());
					}

					memcpy(buf, lpBits - CurLine*uiInputDIBScanLineSize, uiInputDIBScanLineSize); 
					int status = TIFFWriteScanline(tif, buf, CurLine, 0); // TIFFWriteScanline changes the buffer!
					if (status == -1)
						throw (int)TIFF_E_LIBTIFF;
				}
				delete [] buf;
				break;
			}
			case 4:
			{
				TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PhotometricInterpretation);
				
				buf = new BYTE[uiInputDIBScanLineSize];
				if (!buf)
					throw (int)TIFF_E_NOMEM;
				for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
				{
					if ((CurLine & 0x7) == 0)
					{
						if (pThread && pThread->DoExit())
							throw (int)TIFF_THREADEXIT;
						DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, CurLine, GetHeight());
					}

					memcpy(buf, lpBits - CurLine*uiInputDIBScanLineSize, uiInputDIBScanLineSize);
					int status = TIFFWriteScanline(tif, buf, CurLine, 0); // TIFFWriteScanline changes the buffer!
					if (status == -1)
						throw (int)TIFF_E_LIBTIFF;
				}
				delete [] buf;
				break;
			}
			case 8:
			{
				TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PhotometricInterpretation);
				
				buf = new BYTE[uiInputDIBScanLineSize];
				if (!buf)
					throw (int)TIFF_E_NOMEM;
				for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
				{
					if ((CurLine & 0x7) == 0)
					{
						if (pThread && pThread->DoExit())
							throw (int)TIFF_THREADEXIT;
						DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, CurLine, GetHeight());
					}

					memcpy(buf, lpBits - CurLine*uiInputDIBScanLineSize, uiInputDIBScanLineSize); 
					int status = TIFFWriteScanline(tif, buf, CurLine, 0); // TIFFWriteScanline changes the buffer!
					if (status == -1)
						throw (int)TIFF_E_LIBTIFF;
				}
				delete [] buf;
				break;
			}
			case 16: // 16bit to 24bit conversion
			{
				PhotometricInterpretation = PHOTOMETRIC_RGB;
				TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PhotometricInterpretation);

				// Swap B <-> R
				buf = new BYTE[uiOutputDIBScanLineSize];
				if (!buf)
					throw (int)TIFF_E_NOMEM;
				int n;
				for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
				{
					if ((CurLine & 0x7) == 0)
					{
						if (pThread && pThread->DoExit())
							throw (int)TIFF_THREADEXIT;
						DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, CurLine, GetHeight());
					}

					n = 0;
					for (unsigned int i = 0 ; i < GetWidth() ; i++)
					{
						DIB16ToRGB(((WORD*)lpBits)[i], &(buf[n]), &(buf[n+1]), &(buf[n+2]));
						n += 3;
					}
					int status = TIFFWriteScanline(tif, buf, CurLine, 0);
					if (status == -1)
						throw (int)TIFF_E_LIBTIFF;
					lpBits -= uiInputDIBScanLineSize;
				}
				delete [] buf;
				break;
			}
			case 24:
			{
				PhotometricInterpretation = PHOTOMETRIC_RGB;
				TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PhotometricInterpretation);

				// Swap B <-> R
				buf = new BYTE[uiOutputDIBScanLineSize];
				if (!buf)
					throw (int)TIFF_E_NOMEM;
				for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
				{
					if ((CurLine & 0x7) == 0)
					{
						if (pThread && pThread->DoExit())
							throw (int)TIFF_THREADEXIT;
						DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, CurLine, GetHeight());
					}

					for (unsigned int i = 0 ; i < GetWidth() ; i++)
					{
						buf[3*i] = (BYTE)lpBits[3*i+2];		// Red
						buf[3*i+1] = (BYTE)lpBits[3*i+1];	// Green
						buf[3*i+2] = (BYTE)lpBits[3*i];		// Blue
					}
					int status = TIFFWriteScanline(tif, buf, CurLine, 0);
					if (status == -1)
						throw (int)TIFF_E_LIBTIFF;
					lpBits -= uiInputDIBScanLineSize;
				}
				delete [] buf;
				break;
			}
			case 32:
			{
				PhotometricInterpretation = PHOTOMETRIC_RGB;
				TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PhotometricInterpretation);
				
				if (m_bAlpha)
				{
					// Swap B <-> R
					buf = new BYTE[uiOutputDIBScanLineSize];
					if (!buf)
						throw (int)TIFF_E_NOMEM;
					for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
					{
						if ((CurLine & 0x7) == 0)
						{
							if (pThread && pThread->DoExit())
								throw (int)TIFF_THREADEXIT;
							DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, CurLine, GetHeight());
						}
						for (unsigned int i = 0 ; i < uiOutputDIBScanLineSize ; i += 4)
						{
							buf[i+3] = lpBits[i+3];
							buf[i+2] = lpBits[i];
							buf[i+1] = lpBits[i+1];
							buf[i]   = lpBits[i+2];
						}
						int status = TIFFWriteScanline(tif, buf, CurLine, 0);
						if (status == -1)
							throw (int)TIFF_E_LIBTIFF;
						lpBits -= uiInputDIBScanLineSize;
					}
					delete [] buf;
				}
				// 32bit to 24bit conversion
				else
				{
					// Swap B <-> R
					buf = new BYTE[uiOutputDIBScanLineSize];
					if (!buf)
						throw (int)TIFF_E_NOMEM;
					int n = 0;
					for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
					{
						if ((CurLine & 0x7) == 0)
						{
							if (pThread && pThread->DoExit())
								throw (int)TIFF_THREADEXIT;
							DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, CurLine, GetHeight());
						}

						n = 0;
						for (unsigned int i = 0 ; i < GetWidth() ; i++)
						{
							DIB32ToRGB(((DWORD*)lpBits)[i], &(buf[n]), &(buf[n+1]), &(buf[n+2]));
							n += 3;
						}
						int status = TIFFWriteScanline(tif, buf, CurLine, 0);
						if (status == -1)
							throw (int)TIFF_E_LIBTIFF;
						lpBits -= uiInputDIBScanLineSize;
					}
					delete [] buf;
				}
				break;
			}
			default:
				throw (int)TIFF_E_BADBMP;
		}

		// Write to the file
		if (!TIFFWriteDirectory(tif))
			throw (int)TIFF_E_LIBTIFF;
		
		// Now that we have written to the file we can
		// clean-up the eventually allocated user buf
		TIFFFreeUserData();

		DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

		return TRUE;
	}
	catch (int error_code)
	{
		if (buf)
			delete [] buf;
		TIFFFreeUserData();

		DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

		// Just Exit
		if (error_code == TIFF_THREADEXIT)
			return FALSE;

		CString str;
#ifdef _DEBUG
		str = _T("SaveNextTIFF():\n");
#endif
		switch (error_code)
		{
			case TIFF_E_WRONGPARAMETERS :	str += _T("The Function Parameters Are Wrong!\n");
			break;
			case TIFF_E_NOMEM :				str += _T("Could not alloc memory\n");
			break;
			case TIFF_E_BADBMP :			str += _T("Corrupted or unsupported DIB\n");
			break;
			case TIFF_E_LIBTIFF :			str += _T("Cannot save file\n");
			break;
			default:						str += _T("Unspecified error\n");
			break;
		}
		
		TRACE(str);
		if (m_bShowMessageBoxOnError)
			::AfxMessageBox(str, MB_ICONSTOP);

		return FALSE;
	}
}

// Static Version
BOOL CDib::SaveMultiPageTIFF(	LPCTSTR lpszPathName,			// Dst File Name
								CDib::ARRAY& a,					// Array of Dibs to save
								CArray<int,int>& Compression,	// Array of Compressions
								int nJpegQuality/*=DEFAULT_JPEGCOMPRESSION*/,
								CWnd* pProgressWnd/*=NULL*/,
								BOOL bProgressSend/*=TRUE*/,
								CWorkerThread* pThread/*=NULL*/)
{
	TIFF* tif = NULL;
	int nPageCount = a.GetSize();
	if (Compression.GetSize() != nPageCount)
		return FALSE;

	if (nPageCount < 1)
		return FALSE;
	else if (nPageCount == 1)
	{
		return a[0] && a[0]->SaveTIFF(	lpszPathName,
										Compression[0],	
										nJpegQuality,
										pProgressWnd,
										bProgressSend,
										pThread);
	}
		
	// Save first TIFF page
	if (!(a[0] && a[0]->SaveFirstTIFF(	lpszPathName,
										&tif,
										nPageCount,
										Compression[0],
										nJpegQuality,
										pProgressWnd,
										bProgressSend,
										pThread)))
		return FALSE;

	// Save remaining TIFF pages
	for (int nPageNum = 1 ; nPageNum < nPageCount ; nPageNum++)
	{
		if (!(a[nPageNum] && a[nPageNum]->SaveNextTIFF(
										tif,
										nPageNum,
										nPageCount,
										Compression[nPageNum],
										nJpegQuality,
										pProgressWnd,
										bProgressSend,
										pThread)))
		{
			TIFFClose(tif);
			return FALSE;
		}
	}

	// Close
	TIFFClose(tif);

	return TRUE;
}

static BOOL cpStrips(TIFF* in, TIFF* out)
{
	tsize_t bufsize  = TIFFStripSize(in);
	unsigned char* buf = (unsigned char*)_TIFFmalloc(bufsize);

	if (buf)
	{
		tstrip_t s, ns = TIFFNumberOfStrips(in);
		uint32* bytecounts;

		TIFFGetField(in, TIFFTAG_STRIPBYTECOUNTS, &bytecounts);
		for (s = 0 ; s < ns ; s++)
		{
			if (bytecounts[s] > (uint32)bufsize)
			{
				buf = (unsigned char*)_TIFFrealloc(buf, bytecounts[s]);
				if (!buf)
					return FALSE;
				bufsize = bytecounts[s];
			}
			if (TIFFReadRawStrip(in, s, buf, bytecounts[s]) < 0 ||
			    TIFFWriteRawStrip(out, s, buf, bytecounts[s]) < 0)
			{
				_TIFFfree(buf);
				return FALSE;
			}
		}
		_TIFFfree(buf);
		return TRUE;
	}
	else
		return FALSE;
}

static BOOL cpTiles(TIFF* in, TIFF* out)
{
	tsize_t bufsize = TIFFTileSize(in);
	unsigned char* buf = (unsigned char*)_TIFFmalloc(bufsize);

	if (buf)
	{
		ttile_t t, nt = TIFFNumberOfTiles(in);
		uint32* bytecounts;

		TIFFGetField(in, TIFFTAG_TILEBYTECOUNTS, &bytecounts);
		for (t = 0 ; t < nt ; t++)
		{
			if (bytecounts[t] > (uint32) bufsize)
			{
				buf = (unsigned char*)_TIFFrealloc(buf, bytecounts[t]);
				if (!buf)
					return FALSE;
				bufsize = bytecounts[t];
			}
			if (TIFFReadRawTile(in, t, buf, bytecounts[t]) < 0 ||
			    TIFFWriteRawTile(out, t, buf, bytecounts[t]) < 0)
			{
				_TIFFfree(buf);
				return FALSE;
			}
		}
		_TIFFfree(buf);
		return TRUE;
	}
	else
		return FALSE;
}

#define	CopyField(tag, v) \
    if (TIFFGetField(in, tag, &v)) TIFFSetField(out, tag, v)
#define	CopyField2(tag, v1, v2) \
    if (TIFFGetField(in, tag, &v1, &v2)) TIFFSetField(out, tag, v1, v2)
#define	CopyField3(tag, v1, v2, v3) \
    if (TIFFGetField(in, tag, &v1, &v2, &v3)) TIFFSetField(out, tag, v1, v2, v3)

static BOOL TiffCp(TIFF* in, TIFF* out)
{
	uint16 bitspersample, samplesperpixel, compression, shortv, shortv2, *shortav;
	uint32 w, l;
	float floatv;
	float* floatav;
	char* stringv;
	double doublev;
	uint32 longv;
	uint16 *red, *green, *blue;

	// No Support for Old Jpegs!
	if (TIFFGetField(in, TIFFTAG_COMPRESSION, &compression) && compression == COMPRESSION_OJPEG)
		return FALSE;

	CopyField(TIFFTAG_TILEWIDTH, w);
	CopyField(TIFFTAG_TILELENGTH, l);
	CopyField(TIFFTAG_IMAGEWIDTH, w);
	CopyField(TIFFTAG_IMAGELENGTH, l);
	CopyField(TIFFTAG_BITSPERSAMPLE, bitspersample);
	CopyField(TIFFTAG_SAMPLESPERPIXEL, samplesperpixel);
	CopyField(TIFFTAG_COMPRESSION, compression);
	if (compression == COMPRESSION_JPEG)
	{
		uint32 count = 0;
		void* table = NULL;
		if (TIFFGetField(in, TIFFTAG_JPEGTABLES, &count, &table) && count > 0 && table)
		    TIFFSetField(out, TIFFTAG_JPEGTABLES, count, table);
	}
    CopyField(TIFFTAG_PHOTOMETRIC, shortv);
	CopyField(TIFFTAG_PREDICTOR, shortv);
	CopyField(TIFFTAG_THRESHHOLDING, shortv);
	CopyField(TIFFTAG_FILLORDER, shortv);
	CopyField(TIFFTAG_MINSAMPLEVALUE, shortv);
	CopyField(TIFFTAG_MAXSAMPLEVALUE, shortv);
	CopyField(TIFFTAG_XRESOLUTION, floatv);
	CopyField(TIFFTAG_YRESOLUTION, floatv);
	CopyField(TIFFTAG_GROUP3OPTIONS, longv);
	CopyField(TIFFTAG_GROUP4OPTIONS, longv);
	CopyField(TIFFTAG_RESOLUTIONUNIT, shortv);
	CopyField(TIFFTAG_PLANARCONFIG, shortv);
	CopyField(TIFFTAG_ROWSPERSTRIP, longv);
	CopyField(TIFFTAG_XPOSITION, floatv);
	CopyField(TIFFTAG_YPOSITION, floatv);
	CopyField(TIFFTAG_IMAGEDEPTH, longv);
	CopyField(TIFFTAG_TILEDEPTH, longv);
	CopyField(TIFFTAG_SAMPLEFORMAT, shortv);
	CopyField2(TIFFTAG_EXTRASAMPLES, shortv, shortav);
	CopyField3(TIFFTAG_COLORMAP, red, green, blue);
	CopyField(TIFFTAG_WHITEPOINT, floatav);
	CopyField(TIFFTAG_PRIMARYCHROMATICITIES, floatav);
	CopyField2(TIFFTAG_HALFTONEHINTS, shortv, shortv2);
	CopyField(TIFFTAG_INKSET, shortv);
	CopyField2(TIFFTAG_DOTRANGE, shortv, shortv2);
	CopyField(TIFFTAG_TARGETPRINTER, stringv);
	CopyField(TIFFTAG_YCBCRCOEFFICIENTS, floatav);
	CopyField2(TIFFTAG_YCBCRSUBSAMPLING, shortv, shortv2);
	CopyField(TIFFTAG_YCBCRPOSITIONING, shortv);
	CopyField(TIFFTAG_REFERENCEBLACKWHITE, floatav);
	CopyField(TIFFTAG_SMINSAMPLEVALUE, doublev);
	CopyField(TIFFTAG_SMAXSAMPLEVALUE, doublev);
	CopyField(TIFFTAG_STONITS, doublev);
	CopyField(TIFFTAG_HOSTCOMPUTER, stringv);
	CopyField(TIFFTAG_DOCUMENTNAME, stringv);
	CopyField(TIFFTAG_BADFAXLINES, longv);
	CopyField(TIFFTAG_CLEANFAXDATA, longv);
	CopyField(TIFFTAG_CONSECUTIVEBADFAXLINES, longv);
	CopyField(TIFFTAG_FAXRECVPARAMS, longv);
	CopyField(TIFFTAG_FAXRECVTIME, longv);
	CopyField(TIFFTAG_FAXSUBADDRESS, stringv);
	CopyField(TIFFTAG_FAXDCS, stringv);
	if (TIFFIsTiled(in))
		return cpTiles(in, out);
	else
		return cpStrips(in, out);
}

BOOL CDib::SaveMultiPageTIFF(	LPCTSTR lpszSavePathName,
								LPCTSTR lpszOrigPathName,
								LPCTSTR szTempDir,
								int nCompression/*=-1*/,	// -1: automatically select compression
								int nJpegQuality/*=DEFAULT_JPEGCOMPRESSION*/,
								CWnd* pProgressWnd/*=NULL*/,
								BOOL bProgressSend/*=TRUE*/,
								CWorkerThread* pThread/*=NULL*/)
{
	// Check File Name
	if (::IsTIFF(lpszOrigPathName))
	{
		// Temporary file if Save & Orig. File Name are the same
		CString sSaveFileName = lpszSavePathName;
		BOOL bRemoveAndRename = FALSE;
		if (sSaveFileName.CompareNoCase(lpszOrigPathName) == 0)
		{
			sSaveFileName = ::MakeTempFileName(szTempDir, lpszSavePathName);
			bRemoveAndRename = TRUE;
		}

		// Check Images Count & Image Pos,
		// make sure that we are dealing with a Multi-Page Tiff
		if (m_FileInfo.m_nImageCount < 1)
			return FALSE;
		if (m_FileInfo.m_nImagePos < 0 || m_FileInfo.m_nImagePos >= m_FileInfo.m_nImageCount)
			return FALSE;
		else if (m_FileInfo.m_nImageCount == 1)
		{
			return SaveTIFF(sSaveFileName,
							nCompression,	
							nJpegQuality,
							pProgressWnd,
							bProgressSend,
							pThread);
		}
   
		// Open Input File
		TIFF* in = TIFFOpenW(lpszOrigPathName, "r");    
		if (!in)
			return FALSE;

		// Open Output File
		TIFF* out = TIFFOpenW(sSaveFileName, "w");   
		if (!out)
		{
			TIFFClose(in);
			return FALSE;
		}

		// Loop Through all Pages
		for (int i = 0 ; i < m_FileInfo.m_nImageCount ; i++)
		{
			// Set Page
			if (!TIFFSetDirectory(in, (uint16)i))
			{
				TIFFClose(in);
				TIFFClose(out);
				return FALSE;
			}

			// Sub File Type
			// Bit 0 is 1 if the image is a reduced-resolution version of another image in this TIFF file; else the bit is 0.
			// Bit 1 is 1 if the image is a single page of a multi-page image (see the PageNumber field description); else the bit is 0.
			// Bit 2 is 1 if the image defines a transparency mask for another image in this TIFF file.
			//               The PhotometricInterpretation value must be 4, designating a transparency mask.
			uint32 SubFileType = 0;
			TIFFGetField(in, TIFFTAG_SUBFILETYPE, &SubFileType);
			
			// Set Bit 1
			TIFFSetField(out, TIFFTAG_SUBFILETYPE, SubFileType | FILETYPE_PAGE);

			// The first page is numbered 0 (zero).
			// nPageCount is the total number of pages in the document.
			// If nPageCount is 0, the total number of pages in the
			// document is not available
			TIFFSetField(out, TIFFTAG_PAGENUMBER, i, m_FileInfo.m_nImageCount);
			
			// Page Name
			char sPageNumber[20];
			sprintf(sPageNumber, "Page %d", m_FileInfo.m_nImagePos+1);
			TIFFSetField(out, TIFFTAG_PAGENAME, sPageNumber);

			// Save Current Page
			if (i == m_FileInfo.m_nImagePos)
			{
				if (!SaveNextTIFF(	out,
									m_FileInfo.m_nImagePos,
									m_FileInfo.m_nImageCount,
									nCompression,
									nJpegQuality,
									pProgressWnd,
									bProgressSend,
									pThread))
				{
					TIFFClose(in);
					TIFFClose(out);
					return FALSE;
				}
			}
			// Copy Other Pages
			else
			{
				CDib Dib;

				// No Support for Old Jpegs -> Convert To New Jpeg (Lossy...)
				uint16 compression;
				if (TIFFGetField(in, TIFFTAG_COMPRESSION, &compression) && compression == COMPRESSION_OJPEG)
				{
					// Fully Load Page i
					if (!Dib.LoadTIFF(lpszOrigPathName, i))
					{
						TIFFClose(in);
						TIFFClose(out);
						return FALSE;
					}

					// Save Page i
					if (!Dib.SaveNextTIFF(	out,
											m_FileInfo.m_nImagePos,
											m_FileInfo.m_nImageCount,
											COMPRESSION_JPEG,
											DEFAULT_JPEGCOMPRESSION,
											pProgressWnd,
											bProgressSend,
											pThread))
					{
						TIFFClose(in);
						TIFFClose(out);
						return FALSE;
					}
				}
				else
				{
					// Load Header of Page i
					if (!Dib.LoadTIFF(lpszOrigPathName, i, TRUE))
					{
						TIFFClose(in);
						TIFFClose(out);
						return FALSE;
					}

					// Set Metadatas
					Dib.TIFFSetMetadata(out);

					// Copy remaining fields and the image bits
					if (!::TiffCp(in, out) || !TIFFWriteDirectory(out))
					{
						TIFFClose(in);
						TIFFClose(out);
						Dib.TIFFFreeUserData();
						return FALSE;
					}

					// Free Eventually Allocated User Data
					Dib.TIFFFreeUserData();
				}
			}
		}

		// Close
		TIFFClose(in);
        TIFFClose(out);
		
		// Remove and Rename Files
		if (bRemoveAndRename)
		{
			try
			{
				CFile::Remove(lpszSavePathName);
				CFile::Rename(sSaveFileName, lpszSavePathName);
			}
			catch (CFileException* e)
			{
				::DeleteFile(sSaveFileName);
				e->Delete();
				return FALSE;
			}
		}
		return TRUE;
	}
	else
		return FALSE;
}

BOOL CDib::TIFFWriteMetadata(LPCTSTR szFileName, LPCTSTR szTempDir)
{
	CString str;

	// Check File Name
	if (::IsTIFF(szFileName))
	{
		// Temporary Dst File
		CString sTempFileName = ::MakeTempFileName(szTempDir, szFileName);
   
		// Check Images Count & Image Pos
		if (m_FileInfo.m_nImageCount < 1)
		{
			str = _T("Image Count is less than 1\n");
			TRACE(str);
			if (m_bShowMessageBoxOnError)
				::AfxMessageBox(str, MB_ICONSTOP);
			return FALSE;
		}
		if (m_FileInfo.m_nImagePos < 0 || m_FileInfo.m_nImagePos >= m_FileInfo.m_nImageCount)
		{
			str = _T("Image Position is wrong\n");
			TRACE(str);
			if (m_bShowMessageBoxOnError)
				::AfxMessageBox(str, MB_ICONSTOP);
			return FALSE;
		}

		// Open Input File
		TIFF* in = ::TIFFOpenW(szFileName, "r");   
		if (!in)
		{
			str = _T("Cannot open the file for reading\n");
			TRACE(str);
			if (m_bShowMessageBoxOnError)
				::AfxMessageBox(str, MB_ICONSTOP);
			return FALSE;
		}

		// Open Output File
		TIFF* out = ::TIFFOpenW(sTempFileName, "w");   
		if (!out)
		{
			::TIFFClose(in);
			str = _T("Cannot open the file for writing\n");
			TRACE(str);
			if (m_bShowMessageBoxOnError)
				::AfxMessageBox(str, MB_ICONSTOP);
			return FALSE;
		}

		// Loop Through all Pages
		for (int i = 0 ; i < m_FileInfo.m_nImageCount ; i++)
		{
			CDib Dib;
			Dib.SetShowMessageBoxOnError(m_bShowMessageBoxOnError);

			// Set Page
			if (!::TIFFSetDirectory(in, (uint16)i))
			{
				::TIFFClose(in);
				::TIFFClose(out);
				str = _T("Cannot set the tiff directory\n");
				TRACE(str);
				if (m_bShowMessageBoxOnError)
					::AfxMessageBox(str, MB_ICONSTOP);
				return FALSE;
			}

			// Sub File Type
			// Bit 0 is 1 if the image is a reduced-resolution version of another image in this TIFF file; else the bit is 0.
			// Bit 1 is 1 if the image is a single page of a multi-page image (see the PageNumber field description); else the bit is 0.
			// Bit 2 is 1 if the image defines a transparency mask for another image in this TIFF file.
			//               The PhotometricInterpretation value must be 4, designating a transparency mask.
			uint32 SubFileType = 0;
			::TIFFGetField(in, TIFFTAG_SUBFILETYPE, &SubFileType);
			
			// Set Page Number
			if (m_FileInfo.m_nImageCount > 1)
			{
				// Set Bit 1
				::TIFFSetField(out, TIFFTAG_SUBFILETYPE, SubFileType | FILETYPE_PAGE);

				// The first page is numbered 0 (zero).
				// nPageCount is the total number of pages in the document.
				// If nPageCount is 0, the total number of pages in the
				// document is not available
				::TIFFSetField(out, TIFFTAG_PAGENUMBER, i, m_FileInfo.m_nImageCount);
				
				// Page Name
				char sPageNumber[20];
				sprintf(sPageNumber, "Page %d", m_FileInfo.m_nImagePos+1);
				::TIFFSetField(out, TIFFTAG_PAGENAME, sPageNumber);
			}
			else
			{
				// Clear Bit 1
				::TIFFSetField(out, TIFFTAG_SUBFILETYPE, SubFileType & ~FILETYPE_PAGE);
			}

			// No Support for Old Jpegs -> Convert To New Jpeg (Lossy...)
			uint16 compression;
			if (::TIFFGetField(in, TIFFTAG_COMPRESSION, &compression) && compression == COMPRESSION_OJPEG)
			{
				if (!Dib.LoadTIFF(szFileName, i)) // Load Full
				{
					::TIFFClose(in);
					::TIFFClose(out);
					return FALSE;
				}
				if (!Dib.SaveNextTIFF(	out,
										m_FileInfo.m_nImagePos,
										m_FileInfo.m_nImageCount,
										COMPRESSION_JPEG,
										DEFAULT_JPEGCOMPRESSION))
				{
					::TIFFClose(in);
					::TIFFClose(out);
					return FALSE;
				}
			}
			else
			{
				// When reaching the wanted page set Metadatas of the current object,
				// otherwise copy the metadatas from the original input file
				if (i == m_FileInfo.m_nImagePos)
					TIFFSetMetadata(out);
				else
				{
					Dib.LoadTIFF(szFileName, i, TRUE); // Load Header only of Page i
					Dib.TIFFSetMetadata(out);
				}

				// Copy remaining fields and the image bits
				if (!::TiffCp(in, out) || !::TIFFWriteDirectory(out))
				{
					::TIFFClose(in);
					::TIFFClose(out);
					TIFFFreeUserData();
					Dib.TIFFFreeUserData();
					str = _T("Failed while copying the tiff metadata\n");
					TRACE(str);
					if (m_bShowMessageBoxOnError)
						::AfxMessageBox(str, MB_ICONSTOP);
					return FALSE;
				}

				// Free Eventually Allocated User Data
				TIFFFreeUserData();
				Dib.TIFFFreeUserData();
			}
		}

		// Close
		::TIFFClose(in);
		::TIFFClose(out);

		// Remove and Rename Files
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
#ifdef _DEBUG
				str.Format(_T("TIFFWriteMetadata(%s):\n"), szFileName);
#endif
				str += _T("The file is read only\n");
				TRACE(str);
				if (m_bShowMessageBoxOnError)
					::AfxMessageBox(str, MB_ICONSTOP);
			}
			else
				ShowError(e->m_lOsError, _T("TIFFWriteMetadata()"));

			e->Delete();
			return FALSE;
		}

		return TRUE;
	}
	else
	{
		str = _T("Wrong tiff extension\n");
		TRACE(str);
		if (m_bShowMessageBoxOnError)
			::AfxMessageBox(str, MB_ICONSTOP);
		return FALSE;
	}
}

BOOL CDib::TIFFCopy(LPCTSTR szInFileName,
					int nInPageNum,
					TIFF* out,
					int nOutPageNum/*=0*/,
					int nOutPageCount/*=0*/,
					int nCompression/*=-1*/,	// -1 do not change compression type, otherwise re-encode
					int nJpegQuality/*=DEFAULT_JPEGCOMPRESSION*/,
					BOOL bFlatten/*=FALSE*/,						// if TRUE flatten the image if it has a alpha channel
					COLORREF crBackgroundColor/*=RGB(0,0,0)*/,		// Flatten Background Color
					BOOL bLimitToStandardBitsPerSample/*=FALSE*/,	// If set -> the images with Bits per Samples other than
																	// 1, 2, 4 or 8 are converted to a standard Bits per Sample
					BOOL bReencodeYCbCrJpegs/*=FALSE*/)				// YCbCr Jpegs inside Tiff are not supported by Tiff2Pdf
{				
	// Check File Name
	if (::IsTIFF(szInFileName))
	{
		// Get Input Metadata
		CDib Dib;
		Dib.LoadTIFF(szInFileName, nInPageNum, TRUE); // Load Header only of nPageNum

		// Check Images Count & Image Pos
		if (Dib.m_FileInfo.m_nImageCount < 1)
			return FALSE;
		if (nInPageNum < 0 || nInPageNum >= Dib.m_FileInfo.m_nImageCount)
			return FALSE;

		// Open Input File
		TIFF* in = ::TIFFOpenW(szInFileName, "r");   
		if (!in)
			return FALSE;

		// Set Page
		if (!::TIFFSetDirectory(in, (uint16)nInPageNum))
		{
			::TIFFClose(in);
			return FALSE;
		}

		// Sub File Type
		// Bit 0 is 1 if the image is a reduced-resolution version of another image in this TIFF file; else the bit is 0.
		// Bit 1 is 1 if the image is a single page of a multi-page image (see the PageNumber field description); else the bit is 0.
		// Bit 2 is 1 if the image defines a transparency mask for another image in this TIFF file.
		//               The PhotometricInterpretation value must be 4, designating a transparency mask.
		uint32 SubFileType = 0;
		::TIFFGetField(in, TIFFTAG_SUBFILETYPE, &SubFileType);

		// Get Compression
		uint16 compression;
		::TIFFGetField(in, TIFFTAG_COMPRESSION, &compression);

		// Get Bits Per Sample
		uint16 BitsPerSample = 0;
		::TIFFGetField(in, TIFFTAG_BITSPERSAMPLE, &BitsPerSample);

		// Get Photometric Interpretation
		uint16 PhotometricInterpretation = -1;
		::TIFFGetField(in, TIFFTAG_PHOTOMETRIC, &PhotometricInterpretation);

		// Re-encode?
		if (nCompression >= 0								||
			(bFlatten && Dib.m_FileInfo.m_bAlphaChannel)	||	// m_bAlpha is not set if loading the Header only!
			(bLimitToStandardBitsPerSample	&&
			BitsPerSample != 1				&&
			BitsPerSample != 2				&&
			BitsPerSample != 4				&&
			BitsPerSample != 8)								||
			(bReencodeYCbCrJpegs			&&
			compression == COMPRESSION_JPEG &&
			PhotometricInterpretation == PHOTOMETRIC_YCBCR))
		{
			// Set Compression if not set
			if (nCompression < 0)
				nCompression = Dib.m_FileInfo.m_nCompression;

			// Load Full
			if (!Dib.LoadTIFF(szInFileName, nInPageNum))
			{
				::TIFFClose(in);
				return FALSE;
			}

			// Flatten?
			if (bFlatten && Dib.HasAlpha() && Dib.GetBitCount() == 32) // m_bAlpha is set now
			{
				Dib.SetBackgroundColor(crBackgroundColor);
				Dib.RenderAlphaWithSrcBackground();
				Dib.SetAlpha(FALSE);
			}

			// Save
			if (!Dib.SaveNextTIFF(	out,
									nOutPageNum,
									nOutPageCount,
									nCompression,
									nJpegQuality))
			{
				::TIFFClose(in);
				return FALSE;
			}
		}
		// No Support for Old Jpegs -> Convert To New Jpeg (Lossy...)
		else if (compression == COMPRESSION_OJPEG)
		{
			if (!Dib.LoadTIFF(szInFileName, nInPageNum)) // Load Full
			{
				::TIFFClose(in);
				return FALSE;
			}
			if (!Dib.SaveNextTIFF(	out,
									nOutPageNum,
									nOutPageCount,
									COMPRESSION_JPEG,
									DEFAULT_JPEGCOMPRESSION))
			{
				::TIFFClose(in);
				return FALSE;
			}
		}
		// Copy
		else
		{
			// Set Page Number
			if (nOutPageCount == 1)
			{
				// Clear Bit 1
				::TIFFSetField(out, TIFFTAG_SUBFILETYPE, SubFileType & ~FILETYPE_PAGE);
			}
			else
			{
				// Set Bit 1
				::TIFFSetField(out, TIFFTAG_SUBFILETYPE, SubFileType | FILETYPE_PAGE);

				// A Page Count of 0 means that we do not know how many pages!
				if (nOutPageCount > 1)
				{
					// The first page is numbered 0 (zero).
					// nPageCount is the total number of pages in the document.
					// If nPageCount is 0, the total number of pages in the
					// document is not available
					::TIFFSetField(out, TIFFTAG_PAGENUMBER, nOutPageNum, nOutPageCount);
					
					// Page Name
					char sPageNumber[20];
					sprintf(sPageNumber, "Page %d", nOutPageNum+1);
					::TIFFSetField(out, TIFFTAG_PAGENAME, sPageNumber);
				}
			}

			// Set Metadata
			Dib.TIFFSetMetadata(out);

			// Copy remaining fields and the image bits
			if (!::TiffCp(in, out) || !::TIFFWriteDirectory(out))
			{
				::TIFFClose(in);
				Dib.TIFFFreeUserData();
				return FALSE;
			}

			// Free Eventually Allocated User Data
			Dib.TIFFFreeUserData();
		}

		// Close
		::TIFFClose(in);

		return TRUE;
	}
	else
		return FALSE;
}

BOOL CDib::TIFFCopyAllPages(LPCTSTR szInFileName,
							TIFF* out,
							int nOutPageNum/*=0*/,
							int nOutPageCount/*=0*/,
							int nCompressionChangePage/*=-1*/,	// -1 change all pages, otherwise re-encode only the given page
							int nCompression/*=-1*/,			// -1 do not change compression type, otherwise re-encode
							int nJpegQuality/*=DEFAULT_JPEGCOMPRESSION*/,
							BOOL bFlatten/*=FALSE*/,						// if TRUE flatten all pages with a alpha channel
							COLORREF crBackgroundColor/*=RGB(0,0,0)*/,		// Flatten Background Color
							BOOL bLimitToStandardBitsPerSample/*=FALSE*/,	// If set -> the images with Bits per Samples other than
																			// 1, 2, 4 or 8 are converted to a standard Bits per Sample
							BOOL bReencodeYCbCrJpegs/*=FALSE*/)				// YCbCr Jpegs inside Tiff are not supported by Tiff2Pdf
{
	// Check File Name
	if (::IsTIFF(szInFileName))
	{
		// Get Input Metadata
		CDib Dib;
		Dib.LoadTIFF(szInFileName, 0, TRUE); // Load Header only

		// Check Images Count
		if (Dib.m_FileInfo.m_nImageCount < 1)
			return FALSE;

		for (int i = 0 ; i < Dib.m_FileInfo.m_nImageCount ; i++)
		{
			if (nCompressionChangePage == -1 || nCompressionChangePage == i)
			{
				if (!TIFFCopy(	szInFileName,
								i,
								out,
								nOutPageNum++,
								nOutPageCount,
								nCompression,
								nJpegQuality,
								bFlatten,
								crBackgroundColor,
								bLimitToStandardBitsPerSample,
								bReencodeYCbCrJpegs))
					return FALSE;
			}
			else
			{
				if (!TIFFCopy(	szInFileName,
								i,
								out,
								nOutPageNum++,
								nOutPageCount,
								-1,
								nJpegQuality,
								bFlatten,
								crBackgroundColor,
								bLimitToStandardBitsPerSample,
								bReencodeYCbCrJpegs))
				return FALSE;
			}
		}

		return TRUE;
	}
	else
		return FALSE;
}

BOOL CDib::TIFFDeletePage(	int nDeletePageNum,
							LPCTSTR szFileName,
							LPCTSTR szTempDir)
{
	// Check File Name
	if (::IsTIFF(szFileName))
	{
		// Temporary Dst File
		CString sTempFileName = ::MakeTempFileName(szTempDir, szFileName);

		// Get Input Metadata
		CDib Dib;
		Dib.LoadTIFF(szFileName, 0, TRUE); // Load Header only

		// Check Images Count
		if (Dib.m_FileInfo.m_nImageCount <= 1)
			return FALSE;

		// Check Page to delete
		if (nDeletePageNum < 0 || (nDeletePageNum >= Dib.m_FileInfo.m_nImageCount))
			return FALSE;

		// Open Output File
		TIFF* out = ::TIFFOpenW(sTempFileName, "w");    
		if (!out)
			return FALSE;

		// Loop Through all Pages
		int nOutPageNum = 0;
		int nOutPageCount = Dib.m_FileInfo.m_nImageCount - 1;
		for (int i = 0 ; i < Dib.m_FileInfo.m_nImageCount ; i++)
		{
			if (nDeletePageNum != i)
			{
				if (!TIFFCopy(	szFileName,
								i,
								out,
								nOutPageNum++,
								nOutPageCount))
				{
					::TIFFClose(out);
					return FALSE;
				}
			}
		}

		// Close
		::TIFFClose(out);

		// Remove and Rename Files
		try
		{
			CFile::Remove(szFileName);
			CFile::Rename(sTempFileName, szFileName);
		}
		catch (CFileException* e)
		{
			::DeleteFile(sTempFileName);
			e->Delete();
			return FALSE;
		}
		
		return TRUE;
	}
	else
		return FALSE;
}

CString CDib::TIFFExtractPages(	LPCTSTR szDstFileName,
								LPCTSTR szSrcFileName,
								CWnd* pProgressWnd/*=NULL*/,
								BOOL bProgressSend/*=TRUE*/)
{
	// Check File Names
	if (::IsTIFF(szDstFileName) && ::IsTIFF(szSrcFileName))
	{
		// Get Input Metadata
		CDib Dib;
		Dib.LoadTIFF(szSrcFileName, 0, TRUE); // Load Header only

		// Check Images Count
		if (Dib.m_FileInfo.m_nImageCount < 1)
			return _T("");

		// Number of Digits for File Names
		int nDigits = (int)log10((double)Dib.m_FileInfo.m_nImageCount) + 1;

		// Loop Through all Pages
		DIB_INIT_PROGRESS;
		CString sFirstFileName;
		BOOL bFirst = TRUE;
		for (int i = 0 ; i < Dib.m_FileInfo.m_nImageCount ; i++)
		{
			// Progress
			DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, i, Dib.m_FileInfo.m_nImageCount);

			// Save TIFF
			CString sFormat;
			CString sCurrentFileName;
			sFormat.Format(_T("%%0%dd"), nDigits);
			sCurrentFileName.Format(_T("%s") + sFormat + _T("%s"),
									::GetFileNameNoExt(szDstFileName),
									i + 1,
									::GetFileExt(szDstFileName));
			int iCopy = 0;
			while (::IsExistingFile(sCurrentFileName))
			{
				sCurrentFileName.Format(_T("%s") + sFormat + _T("(%d)%s"),
									::GetFileNameNoExt(szDstFileName),
									i + 1,
									++iCopy,
									::GetFileExt(szDstFileName));
			}
			TIFF* out = ::TIFFOpenW(sCurrentFileName, "w");   
			if (!out)
			{
				DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
				return _T("");
			}
			if (!TIFFCopy(szSrcFileName, i, out, 0, 1))
			{
				::TIFFClose(out);
				DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
				return _T("");
			}
			::TIFFClose(out);

			// Set First File Name
			if (bFirst)
			{
				sFirstFileName = sCurrentFileName;
				bFirst = FALSE;
			}
		}

		DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

		// OK
		return sFirstFileName;
	}
	else
		return _T("");
}

#endif