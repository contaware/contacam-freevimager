#include "stdafx.h"
#include "dib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern "C"
{
#include "jerror.h"
}

//
// CMYK -> RGB
//
// RGB:  [0,1]^3
// CMY:  [0,1]^3
// CMYK: [0,1]^4
//
// CMYK -> C'M'Y' = {C*(1-K) + K, M*(1-K) + K, Y*(1-K) + K}
//
// C'M'Y' -> RGB = {1-C', 1-M', 1-Y'}
//               = {(1-C)*(1-K), (1-M)*(1-K), (1-Y)*(1-K)}
//

struct my_jpeg_error_mgr {
	struct jpeg_error_mgr pub;		// "public" fields
	char szLastLibJpegError[256];	// error message
};
typedef my_jpeg_error_mgr *my_jpeg_error_ptr;

// This procedure is called by the IJPEG library when an error occurs
static void jpeg_error_load(j_common_ptr pcinfo)
{
	// Get My Error Pointer
	my_jpeg_error_ptr myerr = (my_jpeg_error_ptr)pcinfo->err;
	
	// Be a bit error tolerant and ignore the following
	if (myerr->pub.msg_code == JERR_SOF_DUPLICATE	||
		myerr->pub.msg_code == JERR_SOI_DUPLICATE	||
		myerr->pub.msg_code == JERR_EOI_EXPECTED)
	    return;

	// Create the message
	myerr->pub.format_message(pcinfo, myerr->szLastLibJpegError);

	throw (int)JPEG_E_LIBJPEG_LOAD;
}

BOOL CDib::IsJPEGExt(CString sExt)
{
	sExt.TrimLeft(_T('.'));
	return ((sExt.CompareNoCase(_T("jpg")) == 0)	||
			(sExt.CompareNoCase(_T("jpeg")) == 0)	||
			(sExt.CompareNoCase(_T("jpe")) == 0)	||
			(sExt.CompareNoCase(_T("jfif")) == 0)	||
			(sExt.CompareNoCase(_T("thm")) == 0));
}

BOOL CDib::IsJPEG(const CString& sFileName)
{
	return IsJPEGExt(::GetFileExt(sFileName));
}

// Possible values for ScaleFactore are : 1,2,4,8
// (example: 4 means that the image width and height are divided by 4)
// This scaling feature is nice to display Thumbnails
BOOL CDib::LoadJPEG(LPCTSTR lpszPathName,
					int ScaleFactor/*=1*/,
					BOOL bFast/*=FALSE*/,
					BOOL bOnlyHeader/*=FALSE*/,
					CWnd* pProgressWnd/*=NULL*/,
					BOOL bProgressSend/*=TRUE*/,
					CWorkerThread* pThread/*=NULL*/)
{
	FILE * infile = NULL;
	jpeg_decompress_struct cinfo;	// IJPEG decoder state
	my_jpeg_error_mgr jerr;			// Custom error manager
	jerr.szLastLibJpegError[0] = '\0';
	LPBYTE buf = NULL;
	DIB_INIT_PROGRESS;

	// Free
	m_FileInfo.Clear();
	m_bAlpha = FALSE;
	m_bGrayscale = FALSE;
	Free();

	try
	{
		// Setup Error Handling
		cinfo.err = jpeg_std_error(&jerr.pub);
		jerr.pub.error_exit = jpeg_error_load;

		// Initialize the JPEG decompression object
		jpeg_create_decompress(&cinfo);

		// Check Path
		CString sPathName(lpszPathName);
		if (sPathName.IsEmpty())
			throw (int)JPEG_E_ZEROPATH;

		// Check whether file is empty
		CFile file(lpszPathName, CFile::modeRead | CFile::shareDenyWrite);
		m_FileInfo.m_dwFileSize = (DWORD)file.GetLength();
		file.Close();
		if (m_FileInfo.m_dwFileSize == 0)
			throw (int)JPEG_E_FILEEMPTY;

		// Check scale factor param
		if ((ScaleFactor != 1) && (ScaleFactor != 2) && (ScaleFactor != 4) && (ScaleFactor != 8))
			throw (int)JPEG_E_SCALEFACTOR;

		// Open the input file,
		// if only header wanted use 'R' for
		// random access optimization.
		// With network drives it is faster!
		CString sOpenMode = bOnlyHeader ? _T("rbR") : _T("rb");
		if ((infile = _tfopen(lpszPathName, sOpenMode)) == NULL)
			throw (int)JPEG_E_READ;
		
		// Specify data source 
		jpeg_stdio_src(&cinfo, infile);

		// Read file parameters
		if(!jpeg_read_header(&cinfo, TRUE))
			throw (int)JPEG_E_BADJPEG;		

		// Fast decompression?
		if (bFast)
		{
			cinfo.dct_method = JDCT_IFAST;
			cinfo.do_fancy_upsampling = FALSE;
			cinfo.do_block_smoothing = FALSE;
			cinfo.dither_mode = JDITHER_NONE;
		}
		else
			cinfo.dct_method = JDCT_FLOAT;
		
		// Scaled load
		cinfo.scale_num = 1;
		cinfo.scale_denom = ScaleFactor; 

		// Start decompressor
		jpeg_start_decompress(&cinfo);
		if (cinfo.out_color_space == JCS_GRAYSCALE)
		{
			m_bGrayscale = TRUE;
			m_pBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER) + (sizeof(RGBQUAD)*256)];
			if (m_pBMI == NULL)
			{
				jpeg_finish_decompress(&cinfo);
				throw (int)JPEG_E_NOMEM;
			}

			// DWORD aligned target DIB ScanLineSize
			DWORD uiDIBTargetScanLineSize = DWALIGNEDWIDTHBYTES((DWORD)cinfo.output_width * 8);
			DWORD uiDIBImageSize = uiDIBTargetScanLineSize * (DWORD)cinfo.output_height;

			// Initialize BITMAPINFOHEADER
			if (cinfo.saw_JFIF_marker)
			{
				switch (cinfo.density_unit)
				{
					// unknown density unit
					case 0 :
						m_pBMI->bmiHeader.biXPelsPerMeter = 0;
						m_pBMI->bmiHeader.biYPelsPerMeter = 0;
						break;

					// dots/inch
					case 1 :
						// Convert to dots/meter
						m_pBMI->bmiHeader.biXPelsPerMeter = (LONG)Round(cinfo.X_density * 100.0 / 2.54);
						m_pBMI->bmiHeader.biYPelsPerMeter = (LONG)Round(cinfo.Y_density * 100.0 / 2.54);
						break;

					// dots/cm
					case 2 :
						// Convert to dots/meter
						m_pBMI->bmiHeader.biXPelsPerMeter = cinfo.X_density * 100;
						m_pBMI->bmiHeader.biYPelsPerMeter = cinfo.Y_density * 100;
						break;

					default:
						m_pBMI->bmiHeader.biXPelsPerMeter = 0;
						m_pBMI->bmiHeader.biYPelsPerMeter = 0;
						break;
				}
			}
			else
			{
				m_pBMI->bmiHeader.biXPelsPerMeter = 0;
				m_pBMI->bmiHeader.biYPelsPerMeter = 0;
			}
			m_pBMI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			m_pBMI->bmiHeader.biWidth = (DWORD)cinfo.output_width;
			m_pBMI->bmiHeader.biHeight = (DWORD)cinfo.output_height;
			m_pBMI->bmiHeader.biPlanes = 1; // must be 1
			m_pBMI->bmiHeader.biBitCount = 8;
			m_pBMI->bmiHeader.biCompression = BI_RGB;    
			m_pBMI->bmiHeader.biSizeImage = uiDIBImageSize;
			m_pBMI->bmiHeader.biClrUsed = 0;
			m_pBMI->bmiHeader.biClrImportant = 0;
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + (WORD)(m_pBMI->bmiHeader.biSize));
			m_dwImageSize = m_pBMI->bmiHeader.biSizeImage;

			// Init File Info
			m_FileInfo.m_nType = CFileInfo::JPEG;
			m_FileInfo.m_nWidth = m_pBMI->bmiHeader.biWidth;
			m_FileInfo.m_nHeight = m_pBMI->bmiHeader.biHeight;
			m_FileInfo.m_nBpp = cinfo.output_components * 8;
			m_FileInfo.m_nCompression = BI_JPEG;
			m_FileInfo.m_nColorSpace = cinfo.jpeg_color_space;
			m_FileInfo.m_dwImageSize = m_pBMI->bmiHeader.biSizeImage;
			m_FileInfo.m_nXPixsPerMeter = m_pBMI->bmiHeader.biXPelsPerMeter;
			m_FileInfo.m_nYPixsPerMeter = m_pBMI->bmiHeader.biYPelsPerMeter;
			m_FileInfo.m_nNumColors = m_pBMI->bmiHeader.biClrUsed;
			m_FileInfo.m_bPalette = FALSE;
			m_FileInfo.m_nImageCount = 1;
			m_FileInfo.m_nImagePos = 0;

			// If only header wanted return now
			if (bOnlyHeader)
			{
				jpeg_destroy_decompress(&cinfo);
				fclose(infile);
				JPEGLoadMetadata(lpszPathName);
				ResFromExif();
				return TRUE;
			}

			// Set Colors
			for (unsigned int i = 0 ; i <= 255 ; i++)
			{
				m_pColors[i].rgbRed = (BYTE)i;
				m_pColors[i].rgbGreen = (BYTE)i;
				m_pColors[i].rgbBlue = (BYTE)i;
				m_pColors[i].rgbReserved = 0;
			}

			// Create Palette from BMI
			CreatePaletteFromBMI();

			// Prepare a buffer large enough to hold the target DIB image pixels
			m_pBits = (LPBYTE)BIGALLOC(uiDIBImageSize);
			if (m_pBits == NULL)
			{
				jpeg_finish_decompress(&cinfo);
				throw (int)JPEG_E_NOMEM;
			}

			// In the jpeg file the scan lines are top / down 
			// In a DIB the scan lines must be saved bottom / up
			LPBYTE lpBits = m_pBits;
			lpBits += uiDIBTargetScanLineSize*(cinfo.output_height-1);

			unsigned int CurLine = 0;
			while (CurLine < cinfo.output_height)
			{
				if ((CurLine & 0x7) == 0)
				{
					if (pThread && pThread->DoExit())
						throw (int)JPEG_THREADEXIT;
					DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, CurLine, cinfo.output_height);
				}
				jpeg_read_scanlines(&cinfo, &lpBits, 1);
				lpBits -= uiDIBTargetScanLineSize;
				CurLine++;
			}
		}
		else
		{
			m_pBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER)];
			if (m_pBMI == NULL)
			{
				jpeg_finish_decompress(&cinfo);
				throw (int)JPEG_E_NOMEM;
			}

			// Scan Line Sizes
			DWORD uiDIBSourceScanLineSize = cinfo.output_width * cinfo.output_components;
			DWORD uiDIBTargetScanLineSize = DWALIGNEDWIDTHBYTES((DWORD)cinfo.output_width * 24);
			DWORD uiDIBImageSize = uiDIBTargetScanLineSize * (DWORD)cinfo.output_height;

			// Initialize BITMAPINFOHEADER
			if (cinfo.saw_JFIF_marker)
			{
				switch (cinfo.density_unit)
				{
					// unknown density unit
					case 0 :
						m_pBMI->bmiHeader.biXPelsPerMeter = 0;
						m_pBMI->bmiHeader.biYPelsPerMeter = 0;
						break;

					// dots/inch
					case 1 :
						// Convert to dots/meter
						m_pBMI->bmiHeader.biXPelsPerMeter = (LONG)Round(cinfo.X_density * 100.0 / 2.54);
						m_pBMI->bmiHeader.biYPelsPerMeter = (LONG)Round(cinfo.Y_density * 100.0 / 2.54);
						break;

					// dots/cm
					case 2 :
						// Convert to dots/meter
						m_pBMI->bmiHeader.biXPelsPerMeter = cinfo.X_density * 100;
						m_pBMI->bmiHeader.biYPelsPerMeter = cinfo.Y_density * 100;
						break;

					default:
						m_pBMI->bmiHeader.biXPelsPerMeter = 0;
						m_pBMI->bmiHeader.biYPelsPerMeter = 0;
						break;
				}
			}
			else
			{
				m_pBMI->bmiHeader.biXPelsPerMeter = 0;
				m_pBMI->bmiHeader.biYPelsPerMeter = 0;
			}
			m_pBMI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			m_pBMI->bmiHeader.biWidth = (DWORD)cinfo.output_width;
			m_pBMI->bmiHeader.biHeight = (DWORD)cinfo.output_height;
			m_pBMI->bmiHeader.biPlanes = 1; // must be 1
			m_pBMI->bmiHeader.biBitCount = 24;
			m_pBMI->bmiHeader.biCompression = BI_RGB;    
			m_pBMI->bmiHeader.biSizeImage = uiDIBImageSize;
			m_pBMI->bmiHeader.biClrUsed = 0;
			m_pBMI->bmiHeader.biClrImportant = 0;
			m_pColors = NULL;
			m_dwImageSize = m_pBMI->bmiHeader.biSizeImage;

			// Init File Info
			m_FileInfo.m_nType = CFileInfo::JPEG;
			m_FileInfo.m_nWidth = m_pBMI->bmiHeader.biWidth;
			m_FileInfo.m_nHeight = m_pBMI->bmiHeader.biHeight;
			m_FileInfo.m_nBpp = cinfo.output_components * 8;
			m_FileInfo.m_nCompression = BI_JPEG;
			m_FileInfo.m_nColorSpace = cinfo.jpeg_color_space;
			m_FileInfo.m_dwImageSize = m_pBMI->bmiHeader.biSizeImage;
			m_FileInfo.m_nXPixsPerMeter = m_pBMI->bmiHeader.biXPelsPerMeter;
			m_FileInfo.m_nYPixsPerMeter = m_pBMI->bmiHeader.biYPelsPerMeter;
			m_FileInfo.m_nNumColors = m_pBMI->bmiHeader.biClrUsed;
			m_FileInfo.m_bPalette = FALSE;
			m_FileInfo.m_nImageCount = 1;
			m_FileInfo.m_nImagePos = 0;

			// If only header wanted return now
			if (bOnlyHeader)
			{
				jpeg_destroy_decompress(&cinfo);
				fclose(infile);
				JPEGLoadMetadata(lpszPathName);
				ResFromExif();
				return TRUE;
			}

			// Create Palette from BMI
			CreatePaletteFromBMI();

			// Prepare a buffer large enough to hold the target DIB image pixels
			m_pBits = (LPBYTE)BIGALLOC(uiDIBImageSize);
			if (m_pBits == NULL)
			{
				jpeg_finish_decompress(&cinfo);
				throw (int)JPEG_E_NOMEM;
			}

			// In the jpeg file the scan lines are top / down 
			// In a DIB the scan lines must be saved bottom / up
			LPBYTE lpBits = m_pBits;
			lpBits += uiDIBTargetScanLineSize*(cinfo.output_height-1);

			// CMYK->RGB
			if ((cinfo.num_components == 4) && (cinfo.quantize_colors == FALSE))
			{
				// Allocate source scan line buffer
				buf = (LPBYTE)new BYTE[uiDIBSourceScanLineSize];
				if (!buf)
					throw (int)JPEG_E_NOMEM;

				while (cinfo.output_scanline < cinfo.output_height)
				{
					if ((cinfo.output_scanline & 0x7) == 0)
					{
						if (pThread && pThread->DoExit())
							throw (int)JPEG_THREADEXIT;
						DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, cinfo.output_scanline, cinfo.output_height);
					}
					jpeg_read_scanlines(&cinfo, &buf, 1);
					for (int x3 = 0, x4 = 0 ; x3 < (int)uiDIBTargetScanLineSize && x4 < (int)uiDIBSourceScanLineSize ; x3 += 3 , x4 += 4)
					{
						// R = (255-C) * (255-K) / 255
						// G = (255-M) * (255-K) / 255
						// B = (255-Y) * (255-K) / 255
						int k = (int)buf[x4+3]; // This is (255-K)
						lpBits[x3]   = (BYTE)((k * (int)buf[x4+2]) / 255); // buf[x4+2] is (255-Y)
						lpBits[x3+1] = (BYTE)((k * (int)buf[x4+1]) / 255); // buf[x4+1] is (255-M)
						lpBits[x3+2] = (BYTE)((k * (int)buf[x4+0]) / 255); // buf[x4+0] is (255-C)
					}
					lpBits -= uiDIBTargetScanLineSize;
				}

				// Clean-up
				delete [] buf;
				buf = NULL;
			} 
			else
			{
				while (cinfo.output_scanline < cinfo.output_height)
				{
					if ((cinfo.output_scanline & 0x7) == 0)
					{
						if (pThread && pThread->DoExit())
							throw (int)JPEG_THREADEXIT;
						DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, cinfo.output_scanline, cinfo.output_height);
					}
					jpeg_read_scanlines(&cinfo, &lpBits, 1);
					RGB2BGR(lpBits, cinfo.output_width);
					lpBits -= uiDIBTargetScanLineSize;
				}
			}
		}

		// Finish decompression
		jpeg_finish_decompress(&cinfo);

		// Release JPEG decompression object
		jpeg_destroy_decompress(&cinfo);
		
		// Close file
		fclose(infile);

		// Load Metadata
		JPEGLoadMetadata(lpszPathName);

		// Set Resolution if not Set Yet
		ResFromExif();

		DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

		return TRUE;
	}
	catch (CFileException* e)
	{
		TCHAR szCause[255];
		CString str(_T("LoadJPEG: "));
		e->GetErrorMessage(szCause, 255);
		str += szCause;
		str += _T("\n");
		TRACE(str);
		if (m_bShowMessageBoxOnError)
			::AfxMessageBox(str, MB_ICONSTOP);
		e->Delete();

		jpeg_destroy_decompress(&cinfo);
		if (infile)
			fclose(infile);

		DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

		return FALSE;
	}
	catch (int error_code)
	{
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
		jpeg_destroy_decompress(&cinfo);
		if (infile)
			fclose(infile);

		DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

		// Just Exit
		if (error_code == JPEG_THREADEXIT)
			return FALSE;

		CString str;
#ifdef _DEBUG
		str.Format(_T("LoadJPEG(%s):\n"), lpszPathName);
#endif
		switch (error_code)
		{
			case JPEG_E_ZEROPATH :		str += _T("The file name is zero\n");
			break;
			case JPEG_E_NOMEM :			str += _T("Could not alloc memory\n");
			break;
			case JPEG_E_LIBJPEG_LOAD :	str += CString(jerr.szLastLibJpegError) + _T("\n");
			break;
			case JPEG_E_BADJPEG :		str += _T("Corrupted or unsupported JPEG\n");
			break;
			case JPEG_E_SCALEFACTOR:	str += _T("Unsupported Scale Factor, use: 1,2,4 or 8\n");
			break;
			case JPEG_E_READ :			str += _T("Couldn't read JPEG file\n");
			break;
			case JPEG_E_FILEEMPTY :		str += _T("File is empty\n");
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

BOOL CDib::ResFromExif()
{
	// Set Resolution if not Set Yet
	if (m_pBMI->bmiHeader.biXPelsPerMeter == 0	&&
		m_pBMI->bmiHeader.biYPelsPerMeter == 0	&&
		GetExifInfo()->ResolutionUnit > 0.0f	&&
		GetExifInfo()->Xresolution > 0.0f		&&
		GetExifInfo()->Yresolution > 0.0f)
	{
		double Xresolution = (double)GetExifInfo()->Xresolution/(double)GetExifInfo()->ResolutionUnit;
		double Yresolution = (double)GetExifInfo()->Yresolution/(double)GetExifInfo()->ResolutionUnit;
		m_pBMI->bmiHeader.biXPelsPerMeter = PIXPERMETER(Xresolution);
		m_pBMI->bmiHeader.biYPelsPerMeter = PIXPERMETER(Yresolution);
		return TRUE;
	}
	else
		return FALSE;
}

// Possible values for ScaleFactore are : 1,2,4,8
// (example: 4 means that the image width and height are divided by 4)
// This scaling feature is nice to display Thumbnails
BOOL CDib::LoadJPEG(LPBYTE pInput,
					int nInputSize,
					int ScaleFactor/*=1*/,
					BOOL bFast/*=FALSE*/,
					CWorkerThread* pThread/*=NULL*/)
{
	jpeg_decompress_struct cinfo;	// IJPEG decoder state.
	my_jpeg_error_mgr jerr;			// Custom error manager
	jerr.szLastLibJpegError[0] = '\0';
	LPBYTE buf = NULL;

	// Free
	m_FileInfo.Clear();
	m_bAlpha = FALSE;
	m_bGrayscale = FALSE;
	Free();

	try
	{
		// Setup Error Handling
		cinfo.err = jpeg_std_error(&jerr.pub);
		jerr.pub.error_exit = jpeg_error_load;

		// Initialize the JPEG decompression object
		jpeg_create_decompress(&cinfo);

		// Check input params
		if ((pInput == NULL) || (nInputSize == 0))
			throw (int)JPEG_E_MEMSRC;

		// Check scale factor param
		if ((ScaleFactor != 1) && (ScaleFactor != 2) && (ScaleFactor != 4) && (ScaleFactor != 8))
			throw (int)JPEG_E_SCALEFACTOR;

		// Specify data source
		jpeg_mem_src(&cinfo, pInput, nInputSize);

		// Read file parameters
		if(!jpeg_read_header(&cinfo, TRUE))
			throw (int)JPEG_E_BADJPEG;		

		// Fast decompression?
		if (bFast)
		{
			cinfo.dct_method = JDCT_IFAST;
			cinfo.do_fancy_upsampling = FALSE;
			cinfo.do_block_smoothing = FALSE;
			cinfo.dither_mode = JDITHER_NONE;
		}
		else
			cinfo.dct_method = JDCT_FLOAT;
		
		// Scaled load
		cinfo.scale_num = 1;
		cinfo.scale_denom = ScaleFactor; 

		// Start decompressor
		jpeg_start_decompress(&cinfo);
		if (cinfo.out_color_space == JCS_GRAYSCALE)
		{
			m_bGrayscale = TRUE;
			m_pBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER) + (sizeof(RGBQUAD)*256)];
			if (m_pBMI == NULL)
			{
				jpeg_finish_decompress(&cinfo);
				throw (int)JPEG_E_NOMEM;
			}

			// DWORD aligned target DIB ScanLineSize
			DWORD uiDIBTargetScanLineSize = DWALIGNEDWIDTHBYTES((DWORD)cinfo.output_width * 8);
			DWORD uiDIBImageSize = uiDIBTargetScanLineSize * (DWORD)cinfo.output_height;

			// Initialize BITMAPINFOHEADER
			if (cinfo.saw_JFIF_marker)
			{
				switch (cinfo.density_unit)
				{
					// unknown density unit
					case 0 :
						m_pBMI->bmiHeader.biXPelsPerMeter = 0;
						m_pBMI->bmiHeader.biYPelsPerMeter = 0;
						break;

					// dots/inch
					case 1 :
						// Convert to dots/meter
						m_pBMI->bmiHeader.biXPelsPerMeter = (LONG)Round(cinfo.X_density * 100.0 / 2.54);
						m_pBMI->bmiHeader.biYPelsPerMeter = (LONG)Round(cinfo.Y_density * 100.0 / 2.54);
						break;

					// dots/cm
					case 2 :
						// Convert to dots/meter
						m_pBMI->bmiHeader.biXPelsPerMeter = cinfo.X_density * 100;
						m_pBMI->bmiHeader.biYPelsPerMeter = cinfo.Y_density * 100;
						break;

					default:
						m_pBMI->bmiHeader.biXPelsPerMeter = 0;
						m_pBMI->bmiHeader.biYPelsPerMeter = 0;
						break;
				}
			}
			else
			{
				m_pBMI->bmiHeader.biXPelsPerMeter = 0;
				m_pBMI->bmiHeader.biYPelsPerMeter = 0;
			}
			m_pBMI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			m_pBMI->bmiHeader.biWidth = (DWORD)cinfo.output_width;
			m_pBMI->bmiHeader.biHeight = (DWORD)cinfo.output_height;
			m_pBMI->bmiHeader.biPlanes = 1; // must be 1
			m_pBMI->bmiHeader.biBitCount = 8;
			m_pBMI->bmiHeader.biCompression = BI_RGB;    
			m_pBMI->bmiHeader.biSizeImage = uiDIBImageSize;
			m_pBMI->bmiHeader.biClrUsed = 0;
			m_pBMI->bmiHeader.biClrImportant = 0;
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + (WORD)(m_pBMI->bmiHeader.biSize));
			m_dwImageSize = m_pBMI->bmiHeader.biSizeImage;

			// Set Colors
			for (unsigned int i = 0 ; i <= 255 ; i++)
			{
				m_pColors[i].rgbRed = (BYTE)i;
				m_pColors[i].rgbGreen = (BYTE)i;
				m_pColors[i].rgbBlue = (BYTE)i;
				m_pColors[i].rgbReserved = 0;
			}

			// Init File Info
			m_FileInfo.m_nType = CFileInfo::JPEG;
			m_FileInfo.m_nWidth = m_pBMI->bmiHeader.biWidth;
			m_FileInfo.m_nHeight = m_pBMI->bmiHeader.biHeight;
			m_FileInfo.m_nBpp = cinfo.output_components * 8;
			m_FileInfo.m_nCompression = BI_JPEG;
			m_FileInfo.m_nColorSpace = cinfo.jpeg_color_space;
			m_FileInfo.m_dwImageSize = m_pBMI->bmiHeader.biSizeImage;
			m_FileInfo.m_nXPixsPerMeter = m_pBMI->bmiHeader.biXPelsPerMeter;
			m_FileInfo.m_nYPixsPerMeter = m_pBMI->bmiHeader.biYPelsPerMeter;
			m_FileInfo.m_nNumColors = m_pBMI->bmiHeader.biClrUsed;
			m_FileInfo.m_bPalette = FALSE;
			m_FileInfo.m_nImageCount = 1;
			m_FileInfo.m_nImagePos = 0;

			// Create Palette from BMI
			CreatePaletteFromBMI();

			// Prepare a buffer large enough to hold the target DIB image pixels
			m_pBits = (LPBYTE)BIGALLOC(uiDIBImageSize);
			if (m_pBits == NULL)
			{
				jpeg_finish_decompress(&cinfo);
				throw (int)JPEG_E_NOMEM;
			}

			// In the jpeg file the scan lines are top / down 
			// In a DIB the scan lines must be saved bottom / up
			LPBYTE lpBits = m_pBits;
			lpBits += uiDIBTargetScanLineSize*(cinfo.output_height-1);

			unsigned int CurLine = 0;
			if (pThread)
			{
				while (CurLine < cinfo.output_height)
				{
					if (((CurLine & 0x7) == 0) && pThread->DoExit())
						throw (int)JPEG_THREADEXIT;
					jpeg_read_scanlines (&cinfo, &lpBits, 1);
					lpBits -= uiDIBTargetScanLineSize;
					CurLine++;
				}

			}
			else
			{
				while (CurLine < cinfo.output_height)
				{
					jpeg_read_scanlines (&cinfo, &lpBits, 1);
					lpBits -= uiDIBTargetScanLineSize;
					CurLine++;
				}
			}
		}
		else
		{
			m_pBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER)];
			if (m_pBMI == NULL)
			{
				jpeg_finish_decompress(&cinfo);
				throw (int)JPEG_E_NOMEM;
			}

			// Scan Line Sizes
			DWORD uiDIBSourceScanLineSize = cinfo.output_width * cinfo.output_components;
			DWORD uiDIBTargetScanLineSize = DWALIGNEDWIDTHBYTES((DWORD)cinfo.output_width * 24);
			DWORD uiDIBImageSize = uiDIBTargetScanLineSize * (DWORD)cinfo.output_height;

			// Initialize BITMAPINFOHEADER
			if (cinfo.saw_JFIF_marker)
			{
				switch (cinfo.density_unit)
				{
					// unknown density unit
					case 0 :
						m_pBMI->bmiHeader.biXPelsPerMeter = 0;
						m_pBMI->bmiHeader.biYPelsPerMeter = 0;
						break;

					// dots/inch
					case 1 :
						// Convert to dots/meter
						m_pBMI->bmiHeader.biXPelsPerMeter = (LONG)Round(cinfo.X_density * 100.0 / 2.54);
						m_pBMI->bmiHeader.biYPelsPerMeter = (LONG)Round(cinfo.Y_density * 100.0 / 2.54);
						break;

					// dots/cm
					case 2 :
						// Convert to dots/meter
						m_pBMI->bmiHeader.biXPelsPerMeter = cinfo.X_density * 100;
						m_pBMI->bmiHeader.biYPelsPerMeter = cinfo.Y_density * 100;
						break;

					default:
						m_pBMI->bmiHeader.biXPelsPerMeter = 0;
						m_pBMI->bmiHeader.biYPelsPerMeter = 0;
						break;
				}
			}
			else
			{
				m_pBMI->bmiHeader.biXPelsPerMeter = 0;
				m_pBMI->bmiHeader.biYPelsPerMeter = 0;
			}
			m_pBMI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			m_pBMI->bmiHeader.biWidth = (DWORD)cinfo.output_width ;
			m_pBMI->bmiHeader.biHeight = (DWORD)cinfo.output_height;
			m_pBMI->bmiHeader.biPlanes = 1; // must be 1
			m_pBMI->bmiHeader.biBitCount = 24;
			m_pBMI->bmiHeader.biCompression = BI_RGB;    
			m_pBMI->bmiHeader.biSizeImage = uiDIBImageSize;
			m_pBMI->bmiHeader.biClrUsed = 0;
			m_pBMI->bmiHeader.biClrImportant = 0;
			m_pColors = NULL;
			m_dwImageSize = m_pBMI->bmiHeader.biSizeImage;

			// Init File Info
			m_FileInfo.m_nType = CFileInfo::JPEG;
			m_FileInfo.m_nWidth = m_pBMI->bmiHeader.biWidth;
			m_FileInfo.m_nHeight = m_pBMI->bmiHeader.biHeight;
			m_FileInfo.m_nBpp = cinfo.output_components * 8;
			m_FileInfo.m_nCompression = BI_JPEG;
			m_FileInfo.m_nColorSpace = cinfo.jpeg_color_space;
			m_FileInfo.m_dwImageSize = m_pBMI->bmiHeader.biSizeImage;
			m_FileInfo.m_nXPixsPerMeter = m_pBMI->bmiHeader.biXPelsPerMeter;
			m_FileInfo.m_nYPixsPerMeter = m_pBMI->bmiHeader.biYPelsPerMeter;
			m_FileInfo.m_nNumColors = m_pBMI->bmiHeader.biClrUsed;
			m_FileInfo.m_bPalette = FALSE;
			m_FileInfo.m_nImageCount = 1;
			m_FileInfo.m_nImagePos = 0;

			// Create Palette from BMI
			CreatePaletteFromBMI();

			// Prepare a buffer large enough to hold the target DIB image pixels
			m_pBits = (LPBYTE)BIGALLOC(uiDIBImageSize);
			if (m_pBits == NULL)
			{
				jpeg_finish_decompress(&cinfo);
				throw (int)JPEG_E_NOMEM;
			}

			// In the jpeg file the scan lines are top / down 
			// In a DIB the scan lines must be saved bottom / up
			LPBYTE lpBits = m_pBits;
			lpBits += uiDIBTargetScanLineSize*(cinfo.output_height-1);

			if (pThread)
			{
				// CMYK->RGB
				if ((cinfo.num_components == 4) && (cinfo.quantize_colors == FALSE))
				{
					// Allocate source scan line buffer
					buf = (LPBYTE)new BYTE[uiDIBSourceScanLineSize];
					if (!buf)
						throw (int)JPEG_E_NOMEM;

					while (cinfo.output_scanline < cinfo.output_height)
					{
						if (((cinfo.output_scanline & 0x7) == 0) && pThread->DoExit())
							throw (int)JPEG_THREADEXIT;
						jpeg_read_scanlines(&cinfo, &buf, 1);
						for (int x3 = 0, x4 = 0 ; x3 < (int)uiDIBTargetScanLineSize && x4 < (int)uiDIBSourceScanLineSize ; x3 += 3 , x4 += 4)
						{
							// R = (255-C) * (255-K) / 255
							// G = (255-M) * (255-K) / 255
							// B = (255-Y) * (255-K) / 255
							int k = (int)buf[x4+3]; // This is (255-K)
							lpBits[x3]   = (BYTE)((k * (int)buf[x4+2]) / 255); // buf[x4+2] is (255-Y)
							lpBits[x3+1] = (BYTE)((k * (int)buf[x4+1]) / 255); // buf[x4+1] is (255-M)
							lpBits[x3+2] = (BYTE)((k * (int)buf[x4+0]) / 255); // buf[x4+0] is (255-C)
						}
						lpBits -= uiDIBTargetScanLineSize;
					}

					// Clean-up
					delete [] buf;
					buf = NULL;
				} 
				else
				{
					while (cinfo.output_scanline < cinfo.output_height)
					{
						if (((cinfo.output_scanline & 0x7) == 0) && pThread->DoExit())
							throw (int)JPEG_THREADEXIT;
						jpeg_read_scanlines(&cinfo, &lpBits, 1);
						RGB2BGR(lpBits, cinfo.output_width);
						lpBits -= uiDIBTargetScanLineSize;
					}
				}
			}
			else
			{
				// CMYK->RGB
				if ((cinfo.num_components == 4) && (cinfo.quantize_colors == FALSE))
				{
					// Allocate source scan line buffer
					buf = (LPBYTE)new BYTE[uiDIBSourceScanLineSize];
					if (!buf)
						throw (int)JPEG_E_NOMEM;

					while (cinfo.output_scanline < cinfo.output_height)
					{
						jpeg_read_scanlines(&cinfo, &buf, 1);
						for (int x3 = 0, x4 = 0 ; x3 < (int)uiDIBTargetScanLineSize && x4 < (int)uiDIBSourceScanLineSize ; x3 += 3 , x4 += 4)
						{
							// R = (255-C) * (255-K) / 255
							// G = (255-M) * (255-K) / 255
							// B = (255-Y) * (255-K) / 255
							int k = (int)buf[x4+3]; // This is (255-K)
							lpBits[x3]   = (BYTE)((k * (int)buf[x4+2]) / 255); // buf[x4+2] is (255-Y)
							lpBits[x3+1] = (BYTE)((k * (int)buf[x4+1]) / 255); // buf[x4+1] is (255-M)
							lpBits[x3+2] = (BYTE)((k * (int)buf[x4+0]) / 255); // buf[x4+0] is (255-C)
						}
						lpBits -= uiDIBTargetScanLineSize;
					}

					// Clean-up
					delete [] buf;
					buf = NULL;
				} 
				else
				{
					while (cinfo.output_scanline < cinfo.output_height)
					{
						jpeg_read_scanlines(&cinfo, &lpBits, 1);
						RGB2BGR(lpBits, cinfo.output_width);
						lpBits -= uiDIBTargetScanLineSize;
					}
				}
			}
		}

		// Finish decompression
		jpeg_finish_decompress(&cinfo);

		// Release JPEG decompression object
		jpeg_destroy_decompress(&cinfo);

		return TRUE;
	}
	catch (int error_code)
	{
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
		jpeg_destroy_decompress(&cinfo);

		// Just Exit
		if (error_code == JPEG_THREADEXIT)
			return FALSE;

		CString str;
#ifdef _DEBUG
		str = _T("LoadJPEG: ");
#endif
		switch(error_code)
		{
			case JPEG_E_NOMEM :			str += _T("Could not alloc memory\n");
			break;
			case JPEG_E_LIBJPEG_LOAD :	str += CString(jerr.szLastLibJpegError) + _T("\n");
			break;
			case JPEG_E_BADJPEG :		str += _T("Corrupted or unsupported JPEG\n");
			break;
			case JPEG_E_SCALEFACTOR:	str += _T("Unsupported Scale Factor, use: 1,2,4 or 8\n");
			break;
			case JPEG_E_READ :			str += _T("Couldn't read JPEG file\n");
			break;
			case JPEG_E_MEMSRC :		str += _T("Memory Source is not specified\n");
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

void CDib::SaveJPEGWriteMetadata(jpeg_compress_struct& cinfo)
{
	// Max. Section Size
	const int nMaxSectionSize = 65533;

	// Exif Data
	CDib Dib(*this);

	// Create the Jpeg Thumbnail
	LPBYTE pJpegThumbData = NULL;
	int nJpegThumbDataSize = EXIF_ADDTHUMB_MAXSIZE;
	MakeEXIFThumbnail(	&Dib,
						&pJpegThumbData,
						&nJpegThumbDataSize);

	// Allocate
	LPBYTE data = new BYTE[nMaxSectionSize];
	
	// Create Exif Section
	int size = GetMetadata()->MakeExifSection(	data,					// Exif Start
												nMaxSectionSize,		// Max. Allowed Size
												GetWidth(),				// Main Image Width
												GetHeight(),			// Main Image Height
												pJpegThumbData,			// Jpeg Thumbnail Pointer
												nJpegThumbDataSize);	// Jpeg Thumbnail Size

	// Write Marker
	if (size > 0)
	{
		jpeg_write_marker(	&cinfo,
							M_EXIF_XMP,
							data,
							size);
	}

	// Clean-Up
	if (pJpegThumbData)
		delete [] pJpegThumbData;

	// Xmp Packet
	if (GetMetadata()->m_pXmpData		&&
		GetMetadata()->m_dwXmpSize > 0	&&
		XMP_HEADER_SIZE + GetMetadata()->m_dwXmpSize <= nMaxSectionSize)
	{
		// Call this to change to the image/jpeg mime
		// when saving from a tiff file for example!
		GetMetadata()->UpdateXmpData(_T("image/jpeg")); 

		// Header
		memcpy(data, CMetadata::m_XmpHeader, XMP_HEADER_SIZE);
		
		// Copy Xmp Data
		memcpy(data + XMP_HEADER_SIZE, GetMetadata()->m_pXmpData, GetMetadata()->m_dwXmpSize);

		// Write Marker
		jpeg_write_marker(	&cinfo,
							M_EXIF_XMP,
							data,
							XMP_HEADER_SIZE + GetMetadata()->m_dwXmpSize);
	}

	// Iptc Legacy Data
	if (GetMetadata()->m_pIptcLegacyData &&
		GetMetadata()->m_dwIptcLegacySize > 0)
	{
		int nTotalSize = 26 + GetMetadata()->m_dwIptcLegacySize;

		// Pad to Even Length
		if (nTotalSize & 0x1)
		{
			nTotalSize++;
			data[nTotalSize - 1] = 0;
		}

		if (nTotalSize <= nMaxSectionSize)
		{
			// Header
			memcpy(data, "Photoshop 3.0", 14);
			memcpy(data+14, "8BIM", 4);
			data[18] = 0x04;
			data[19] = 0x04;
			data[20] = 0;
			data[21] = 0;

			// Iptc Size is big endian and 32 bit
			int iptcsize = GetMetadata()->m_dwIptcLegacySize;
			BYTE t;
			LPBYTE cp = (LPBYTE)&iptcsize;
			t = cp[3]; cp[3] = cp[0]; cp[0] = t;
			t = cp[2]; cp[2] = cp[1]; cp[1] = t;
			*((DWORD*)(&data[22])) = iptcsize;

			// Copy Iptc Data
			memcpy(data + 26, GetMetadata()->m_pIptcLegacyData, GetMetadata()->m_dwIptcLegacySize);

			// Write Marker
			jpeg_write_marker(	&cinfo,
								M_IPTC,
								data,
								nTotalSize);
		}
	}

	// Icc Color Profile
	if (GetMetadata()->m_pIccData && GetMetadata()->m_dwIccSize > 0)
	{
		// Max Size for a jpeg section is: 65535 - 2 = 65533
		//
		// 12 Bytes: ICC header
		// Byte:     Chunk id
		// Byte:     Chunks count
		// Max Data: 65533 - 12 - 2 = 65519 bytes
		const int nMaxIccChunkSize = 32000; // Photoshop uses 32000
		int nSectionsCount = GetMetadata()->m_dwIccSize / nMaxIccChunkSize;
		if (GetMetadata()->m_dwIccSize % nMaxIccChunkSize)
			nSectionsCount++;
		
		int datapos;
		int iccpos = 0;
		int iccsize = nMaxIccChunkSize;
		for (int i = 0 ; i < nSectionsCount ; i++)
		{
			// Reset data pos
			datapos = 0;

			// Last?
			if (i == nSectionsCount - 1)
				iccsize = GetMetadata()->m_dwIccSize - i * nMaxIccChunkSize;

			// Icc Header
			memcpy(data, CMetadata::m_IccHeader, ICC_HEADER_SIZE);
			datapos += ICC_HEADER_SIZE;

			// Chunk Id and Count
			data[datapos++] = i+1;				// Chunk id: 1..255
			data[datapos++] = nSectionsCount;	// Chunks count: 1..255
			
			// Copy Icc Data
			memcpy(data + datapos, GetMetadata()->m_pIccData + iccpos, iccsize);
			datapos += iccsize; 
			iccpos += iccsize;

			// Write Marker
			jpeg_write_marker(	&cinfo,
								M_ICC,
								data,
								datapos);
		}
	}

	// Clean-Up
	delete [] data;
}

// This procedure is called by the IJPEG library when an error occurs
static void jpeg_error_save(j_common_ptr pcinfo)
{
	// Get My Error Pointer
	my_jpeg_error_ptr myerr = (my_jpeg_error_ptr)pcinfo->err;
	
	// Create the message
	myerr->pub.format_message(pcinfo, myerr->szLastLibJpegError);
	
	throw (int)JPEG_E_LIBJPEG_SAVE;
}

BOOL CDib::SaveJPEG(LPCTSTR lpszPathName,
					int quality/*=80*/,
					BOOL bGrayScale/*=FALSE*/,
					LPCTSTR lpszMarkersFrom/*=_T("")*/,
					BOOL bSaveMetadata/*=FALSE*/,
					BOOL bDoUpdateExifWidthHeightThumb/*=FALSE*/,
					CWnd* pProgressWnd/*=NULL*/,
					BOOL bProgressSend/*=TRUE*/,
					CWorkerThread* pThread/*=NULL*/)
{
	// Markers Copy Option
	// Do not Forget To Define SAVE_MARKERS_SUPPORTED !!!
	JCOPY_OPTION copyoption = JCOPYOPT_ALL;
	DIB_INIT_PROGRESS;
	LPBYTE buf = NULL;
	FILE* infile = NULL;
	FILE* outfile = NULL;
	jpeg_decompress_struct srcinfo;
	jpeg_compress_struct cinfo;			// IJPEG decoder state
	my_jpeg_error_mgr jsrcerr, jerr;	// Custom error manager
	jsrcerr.szLastLibJpegError[0] = '\0';
	jerr.szLastLibJpegError[0] = '\0';
	BOOL bCopySrcMarkers;
	if ((lpszMarkersFrom == NULL) || (lpszMarkersFrom[0] == _T('\0')) ||
		FileSignatureToExtension(lpszMarkersFrom) != _T(".jpg"))
		bCopySrcMarkers = FALSE;
	else
		bCopySrcMarkers = TRUE;

	try
	{
		// Setup Error Handling
		srcinfo.err = jpeg_std_error(&jsrcerr.pub);
		jsrcerr.pub.error_exit = jpeg_error_load;
		cinfo.err = jpeg_std_error(&jerr.pub);
		jerr.pub.error_exit = jpeg_error_save;

		// Initialize the JPEG decompression object
		jpeg_create_decompress(&srcinfo);

		// Initialize the JPEG compression object
		jpeg_create_compress(&cinfo);

		CString sPathName(lpszPathName);
		if (sPathName.IsEmpty())
			throw (int)JPEG_E_ZEROPATH;

		DWORD dwAttrib = ::GetFileAttributes(lpszPathName);
		if ((dwAttrib != 0xFFFFFFFF) && (dwAttrib & FILE_ATTRIBUTE_READONLY))
			throw (int)JPEG_E_FILEREADONLY;

		if (!m_pBits)
		{
			if (!DibSectionToBits())
				throw (int)JPEG_E_BADBMP;
		}

		if (!m_pBits || !m_pBMI)
			throw (int)JPEG_E_BADBMP;

		if (IsCompressed())
		{
			if (!Decompress(GetBitCount())) // Decompress
				throw (int)JPEG_E_BADBMP;
		}

		if ((quality < 0) || (quality > 100))
			quality = DEFAULT_JPEGCOMPRESSION;
	
		// Open the input file
		if (bCopySrcMarkers)
		{
			if ((infile = _tfopen(lpszMarkersFrom, _T("rb"))) == NULL)
				bCopySrcMarkers = FALSE;
			else
			{	
				// Specify data source for decompression
				jpeg_stdio_src(&srcinfo, infile);

				// Enable saving of extra markers that we want to copy
				jcopy_markers_setup(&srcinfo, copyoption);

				// Read file header
				if(!jpeg_read_header(&srcinfo, TRUE))
					bCopySrcMarkers = FALSE;
			}
		}

		// Open the output file
		if ((outfile = _tfopen(lpszPathName, _T("wb"))) == NULL)
			throw (int)JPEG_E_WRITE;
		jpeg_stdio_dest(&cinfo, outfile);

		// Set Header Fields
		cinfo.image_width = GetWidth();
		cinfo.image_height = GetHeight();

		if (bGrayScale == FALSE)
		{
			cinfo.input_components = 3;
			cinfo.in_color_space = JCS_RGB;
			
			jpeg_set_defaults(&cinfo);
			jpeg_set_quality(&cinfo, quality, TRUE); // Quality between 0..100
			// density_unit:
			// 0 for unknown
			// 1 for dots/inch
			// 2 for dots/cm
			cinfo.write_JFIF_header = TRUE;
			cinfo.density_unit = 1;
			cinfo.X_density = (UINT16)GetXDpi();
			cinfo.Y_density = (UINT16)GetYDpi();
			cinfo.dct_method = JDCT_FLOAT;
			jpeg_start_compress(&cinfo, TRUE);

			// Copy to the output file any extra markers that we want to preserve
			if (bCopySrcMarkers)
				jcopy_markers_execute(&srcinfo, &cinfo, copyoption);
			else if (bSaveMetadata)
				SaveJPEGWriteMetadata(cinfo);

			DWORD uiOutputDIBScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * 24);
			DWORD uiInputDIBScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * GetBitCount());

			// In the jpeg file the scan lines are top / down 
			// In a DIB the scan lines are stored bottom / up
			LPBYTE lpBits = m_pBits;
			lpBits += uiInputDIBScanLineSize*(GetHeight()-1);

			switch (GetBitCount())
			{
				case 1: // 1bit to 24bit conversion
				{
					buf = new BYTE[uiOutputDIBScanLineSize];
					int n;
					BYTE PixelIndex = 0;
					unsigned int ByteNum = 0;
					BYTE BitNum = 0;
					for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
					{
						if ((CurLine & 0x7) == 0)
						{
							if (pThread && pThread->DoExit())
								throw (int)JPEG_THREADEXIT;
							DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, CurLine, GetHeight());
						}

						n = 0;
						for (unsigned int i = 0 ; i < GetWidth(); i++)
						{
							ByteNum = i / 8;
							BitNum = (BYTE)(7 - (i % 8)); // 7=MSBit .. 0=LSBit
							// First pixel is the most significant bit of the byte:
							PixelIndex = (BYTE)(1 & (lpBits[ByteNum] >> BitNum));
							buf[n++] = m_pColors[PixelIndex].rgbRed;
							buf[n++] = m_pColors[PixelIndex].rgbGreen;
							buf[n++] = m_pColors[PixelIndex].rgbBlue;
						}
						jpeg_write_scanlines(&cinfo, &buf, 1);
						lpBits -= uiInputDIBScanLineSize;
					}
					delete [] buf;
					break;
				}
				case 4: // 4bit to 24bit conversion
				{
					buf = new BYTE[uiOutputDIBScanLineSize];
					int n;
					BYTE LowPixelIndex = 0;
					BYTE HighPixelIndex = 0;
					for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
					{
						if ((CurLine & 0x7) == 0)
						{
							if (pThread && pThread->DoExit())
								throw (int)JPEG_THREADEXIT;
							DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, CurLine, GetHeight());
						}

						n = 0;
						unsigned int i;
						for (i = 0 ; i < GetWidth()/2; i++)
						{
							LowPixelIndex = (BYTE)(0x0F & lpBits[i]);
							HighPixelIndex = (BYTE)((0xF0 & lpBits[i]) >> 4);
							buf[n++] = m_pColors[HighPixelIndex].rgbRed;
							buf[n++] = m_pColors[HighPixelIndex].rgbGreen;
							buf[n++] = m_pColors[HighPixelIndex].rgbBlue;
							buf[n++] = m_pColors[LowPixelIndex].rgbRed;
							buf[n++] = m_pColors[LowPixelIndex].rgbGreen;
							buf[n++] = m_pColors[LowPixelIndex].rgbBlue;
						}
						// odd number of pixels
						if (GetWidth() & 1)
						{
							HighPixelIndex = (BYTE)((0xF0 & lpBits[i]) >> 4);
							buf[n++] = m_pColors[HighPixelIndex].rgbRed;
							buf[n++] = m_pColors[HighPixelIndex].rgbGreen;
							buf[n++] = m_pColors[HighPixelIndex].rgbBlue;
						}
						jpeg_write_scanlines(&cinfo, &buf, 1);
						lpBits -= uiInputDIBScanLineSize;
					}
					delete [] buf;
					break;
				}
				case 8: // 8bit to 24bit conversion
				{
					buf = new BYTE[uiOutputDIBScanLineSize];
					for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
					{
						if ((CurLine & 0x7) == 0)
						{
							if (pThread && pThread->DoExit())
								throw (int)JPEG_THREADEXIT;
							DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, CurLine, GetHeight());
						}

						for (unsigned int i = 0 ; i < GetWidth() ; i++)
						{
							buf[3*i] =   m_pColors[lpBits[i]].rgbRed;
							buf[3*i+1] = m_pColors[lpBits[i]].rgbGreen;
							buf[3*i+2] = m_pColors[lpBits[i]].rgbBlue;
						}
						jpeg_write_scanlines(&cinfo, &buf, 1);
						lpBits -= uiInputDIBScanLineSize;
					}
					delete [] buf;
					break;
				}
				case 16: // 16bit to 24bit conversion
				{
					buf = new BYTE[uiOutputDIBScanLineSize];
					int n;
					for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
					{
						if ((CurLine & 0x7) == 0)
						{
							if (pThread && pThread->DoExit())
								throw (int)JPEG_THREADEXIT;
							DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, CurLine, GetHeight());
						}

						n = 0;
						for (unsigned int i = 0 ; i < GetWidth() ; i++)
						{
							DIB16ToRGB(((WORD*)lpBits)[i], &(buf[n]), &(buf[n+1]), &(buf[n+2]));
							n += 3;
						}
						jpeg_write_scanlines(&cinfo, &buf, 1);
						lpBits -= uiInputDIBScanLineSize;
					}
					delete [] buf;
					break;
				}
				case 24:
				{
					buf = new BYTE[uiOutputDIBScanLineSize];
					int n;
					for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
					{
						if ((CurLine & 0x7) == 0)
						{
							if (pThread && pThread->DoExit())
								throw (int)JPEG_THREADEXIT;
							DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, CurLine, GetHeight());
						}

						n = 0;
						for (unsigned int i = 0 ; i < GetWidth() ; i++)
						{
							buf[n]   = lpBits[n+2];	// Red
							buf[n+1] = lpBits[n+1];	// Green
							buf[n+2] = lpBits[n];	// Blue
							n += 3;
						}
						jpeg_write_scanlines(&cinfo, &buf, 1);
						lpBits -= uiInputDIBScanLineSize;
					}
					delete [] buf;
					break;
				}
				case 32: // 32bit to 24bit conversion
				{
					buf = new BYTE[uiOutputDIBScanLineSize];
					int n;
					for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
					{
						if ((CurLine & 0x7) == 0)
						{
							if (pThread && pThread->DoExit())
								throw (int)JPEG_THREADEXIT;
							DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, CurLine, GetHeight());
						}

						n = 0;
						for (unsigned int i = 0 ; i < GetWidth() ; i++)
						{
							DIB32ToRGB(((DWORD*)lpBits)[i], &(buf[n]), &(buf[n+1]), &(buf[n+2]));
							n += 3;
						}
						jpeg_write_scanlines(&cinfo, &buf, 1);
						lpBits -= uiInputDIBScanLineSize;
					}
					delete [] buf;
					break;
				}
				default:
					break;
			}
		}
		else // Gray Scale Save
		{
			cinfo.input_components = 1;
			cinfo.in_color_space = JCS_GRAYSCALE;
			
			jpeg_set_defaults(&cinfo);
			jpeg_set_quality(&cinfo, quality, TRUE); // Quality between 0..100
			// density_unit:
			// 0 for unknown
			// 1 for dots/inch
			// 2 for dots/cm
			cinfo.write_JFIF_header = TRUE;
			cinfo.density_unit = 1;
			cinfo.X_density = (UINT16)GetXDpi();
			cinfo.Y_density = (UINT16)GetYDpi();
			cinfo.dct_method = JDCT_FLOAT;
			jpeg_start_compress(&cinfo, TRUE);

			// Copy to the output file any extra markers that we want to preserve
			if (bCopySrcMarkers)
				jcopy_markers_execute(&srcinfo, &cinfo, copyoption);
			else if (bSaveMetadata)
				SaveJPEGWriteMetadata(cinfo);

			DWORD uiOutputDIBScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * 8);
			DWORD uiInputDIBScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * GetBitCount());

			// In the jpeg file the scan lines are top / down 
			// In a DIB the scan lines are stored bottom / up
			LPBYTE lpBits = m_pBits;
			lpBits += uiInputDIBScanLineSize*(GetHeight()-1);

			switch (GetBitCount())
			{
				case 1: // 1bit to 8bit gray scale conversion
				{
					buf = new BYTE[uiOutputDIBScanLineSize];
					BYTE PixelIndex = 0;
					unsigned int ByteNum = 0;
					BYTE BitNum = 0;
					for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
					{
						if ((CurLine & 0x7) == 0)
						{
							if (pThread && pThread->DoExit())
								throw (int)JPEG_THREADEXIT;
							DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, CurLine, GetHeight());
						}

						for (unsigned int i = 0 ; i < GetWidth(); i++)
						{
							ByteNum = i / 8;
							BitNum = (BYTE)(7 - (i % 8)); // 7=MSBit .. 0=LSBit
							// First pixel is the most significant bit of the byte:
							PixelIndex = (BYTE)(1 & (lpBits[ByteNum] >> BitNum));
							buf[i] = RGBToGray(	m_pColors[PixelIndex].rgbRed,
												m_pColors[PixelIndex].rgbGreen,
												m_pColors[PixelIndex].rgbBlue);
						}
						jpeg_write_scanlines(&cinfo, &buf, 1);
						lpBits -= uiInputDIBScanLineSize;
					}
					delete [] buf;
					break;
				}
				case 4: // 4bit to 8bit gray scale conversion
				{
					buf = new BYTE[uiOutputDIBScanLineSize];
					BYTE LowPixelIndex = 0;
					BYTE HighPixelIndex = 0;
					int n;
					for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
					{
						if ((CurLine & 0x7) == 0)
						{
							if (pThread && pThread->DoExit())
								throw (int)JPEG_THREADEXIT;
							DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, CurLine, GetHeight());
						}

						n = 0;
						unsigned int i;
						for (i = 0 ; i < GetWidth()/2; i++)
						{
							LowPixelIndex = (BYTE)(0x0F & lpBits[i]);
							HighPixelIndex = (BYTE)((0xF0 & lpBits[i]) >> 4);
							buf[n++] = RGBToGray(	m_pColors[HighPixelIndex].rgbRed,
													m_pColors[HighPixelIndex].rgbGreen,
													m_pColors[HighPixelIndex].rgbBlue);
							buf[n++] = RGBToGray(	m_pColors[LowPixelIndex].rgbRed,
													m_pColors[LowPixelIndex].rgbGreen,
													m_pColors[LowPixelIndex].rgbBlue);
						}
						// odd number of pixels
						if (GetWidth() & 1)
						{
							HighPixelIndex = (BYTE)((0xF0 & lpBits[i]) >> 4);
							buf[n++] = RGBToGray(	m_pColors[HighPixelIndex].rgbRed,
													m_pColors[HighPixelIndex].rgbGreen,
													m_pColors[HighPixelIndex].rgbBlue);
						}
						jpeg_write_scanlines(&cinfo, &buf, 1);
						lpBits -= uiInputDIBScanLineSize;
					}
					delete [] buf;
					break;
				}
				case 8: // 8bit to 8bit gray scale conversion
				{
					buf = new BYTE[uiOutputDIBScanLineSize];
					for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
					{
						if ((CurLine & 0x7) == 0)
						{
							if (pThread && pThread->DoExit())
								throw (int)JPEG_THREADEXIT;
							DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, CurLine, GetHeight());
						}

						for (unsigned int i = 0 ; i < GetWidth() ; i++)
						{
							buf[i] = RGBToGray(	m_pColors[lpBits[i]].rgbRed,
												m_pColors[lpBits[i]].rgbGreen,
												m_pColors[lpBits[i]].rgbBlue);
						}
						jpeg_write_scanlines(&cinfo, &buf, 1);
						lpBits -= uiInputDIBScanLineSize;
					}
					delete [] buf;
					break;
				}
				case 16: // 16bit to 8bit gray scale conversion
				{
					buf = new BYTE[uiOutputDIBScanLineSize];
					for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
					{
						if ((CurLine & 0x7) == 0)
						{
							if (pThread && pThread->DoExit())
								throw (int)JPEG_THREADEXIT;
							DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, CurLine, GetHeight());
						}

						for (unsigned int i = 0 ; i < GetWidth() ; i++)
						{
							BYTE red, green, blue;
							DIB16ToRGB(((WORD*)lpBits)[i], &red, &green, &blue);
							buf[i] = RGBToGray(red, green, blue);
						}
						jpeg_write_scanlines(&cinfo, &buf, 1);
						lpBits -= uiInputDIBScanLineSize;
					}
					delete [] buf;
					break;
				}
				case 24: // 24bit to 8bit gray scale conversion
				{
					buf = new BYTE[uiOutputDIBScanLineSize];
					for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
					{
						if ((CurLine & 0x7) == 0)
						{
							if (pThread && pThread->DoExit())
								throw (int)JPEG_THREADEXIT;
							DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, CurLine, GetHeight());
						}

						for (unsigned int i = 0 ; i < GetWidth() ; i++)
						{
							buf[i] = RGBToGray(lpBits[3*i+2], lpBits[3*i+1], lpBits[3*i]);
						}
						jpeg_write_scanlines(&cinfo, &buf, 1);
						lpBits -= uiInputDIBScanLineSize;
					}
					delete [] buf;
					break;
				}
				case 32: // 32bit to 8bit gray scale conversion
				{
					buf = new BYTE[uiOutputDIBScanLineSize];
					for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
					{
						if ((CurLine & 0x7) == 0)
						{
							if (pThread && pThread->DoExit())
								throw (int)JPEG_THREADEXIT;
							DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, CurLine, GetHeight());
						}

						for (unsigned int i = 0 ; i < GetWidth() ; i++)
						{
							BYTE red, green, blue;
							DIB32ToRGB(((DWORD*)lpBits)[i], &red, &green, &blue);
							buf[i] = RGBToGray(red, green, blue);
						}
						jpeg_write_scanlines(&cinfo, &buf, 1);
						lpBits -= uiInputDIBScanLineSize;
					}
					delete [] buf;
					break;
				}
				default:
					break;
			}
		}

		jpeg_destroy_decompress(&srcinfo);
		if (infile)
			fclose(infile);

		jpeg_finish_compress(&cinfo);
		jpeg_destroy_compress(&cinfo);
		fclose(outfile);

		if (bDoUpdateExifWidthHeightThumb)
			UpdateExifWidthHeightThumb(lpszPathName, m_bShowMessageBoxOnError);

		DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

		// We have to remove the ICC profile when saving a YCCK or CMYK image,
		// because we use the YCbCr color space for saving!
		BOOL bRemoveIcc = (	m_FileInfo.m_nColorSpace == CFileInfo::COLORSPACE_YCCK ||
							m_FileInfo.m_nColorSpace == CFileInfo::COLORSPACE_CMYK);
		if (bRemoveIcc)
			return JPEGRemoveIcc(lpszPathName, m_bShowMessageBoxOnError);
		else
			return TRUE;
	}
	catch (int error_code)
	{
		// Free & Close
		if (buf)
			delete [] buf;
		jpeg_destroy_decompress(&srcinfo);
		jpeg_destroy_compress(&cinfo);
		if (infile)
			fclose(infile);
		if (outfile)
			fclose(outfile);

		DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

		// Just Exit
		if (error_code == JPEG_THREADEXIT)
			return FALSE;

		CString str;
#ifdef _DEBUG
		str.Format(_T("SaveJPEG(%s):\n"), lpszPathName);
#endif
		switch (error_code)
		{
			case JPEG_E_ZEROPATH :		str += _T("The file name is zero\n");
			break;
			case JPEG_E_FILEREADONLY :	str += _T("The file is read only\n");
			break;
			case JPEG_E_LIBJPEG_LOAD :	str += CString(jsrcerr.szLastLibJpegError) + _T("\n");
			break;
			case JPEG_E_LIBJPEG_SAVE :	str += CString(jerr.szLastLibJpegError) + _T("\n");
			break;
			case JPEG_E_BADBMP :		str += _T("Corrupted or unsupported DIB\n");
			break;
			case JPEG_E_WRITE :			str += _T("Couldn't write JPEG file\n");
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

BOOL CDib::JPEGRemoveIcc(LPCTSTR lpszPathName, BOOL bShowMessageBoxOnError)
{
	CString sTempFileName = ::MakeTempFileName(::GetDriveAndDirName(lpszPathName), lpszPathName);

	// Remove ICC Section
	if (JPEGRemoveSection(	M_ICC,
							CMetadata::m_IccHeader,
							ICC_HEADER_SIZE,
							lpszPathName,
							sTempFileName,
							bShowMessageBoxOnError))
	{
		// Remove and Rename Files
		try
		{
			CFile::Remove(lpszPathName);
			CFile::Rename(sTempFileName, lpszPathName);
			return TRUE;
		}
		catch (CFileException* e)
		{
			::DeleteFile(sTempFileName);

			DWORD dwAttrib = ::GetFileAttributes(lpszPathName);
			if ((dwAttrib != 0xFFFFFFFF) && (dwAttrib & FILE_ATTRIBUTE_READONLY))
			{
				CString str(_T("The file is read only\n"));
				TRACE(str);
				if (bShowMessageBoxOnError)
					::AfxMessageBox(str, MB_ICONSTOP);
			}
			else
				ShowError(e->m_lOsError, bShowMessageBoxOnError, _T("JPEGRemoveIcc()"));

			e->Delete();
			return FALSE;
		}
	}
	else
		return TRUE; // No Icc Section Found
}

BOOL CDib::SaveJPEG(LPBYTE pOutput,
					int* pOutputSize,
					int quality/*=80*/,
					BOOL bGrayScale/*=FALSE*/,
					LPCTSTR lpszMarkersFrom/*=_T("")*/,
					BOOL bSaveMetadata/*=FALSE*/,
					BOOL bDoUpdateExifWidthHeightThumb/*=FALSE*/,
					CWorkerThread* pThread/*=NULL*/)
{
	// Markers Copy Option
	// Do not Forget To Define SAVE_MARKERS_SUPPORTED !!!
	JCOPY_OPTION copyoption = JCOPYOPT_ALL;

	LPBYTE buf = NULL;
	FILE* infile = NULL;
	jpeg_decompress_struct srcinfo;
	jpeg_compress_struct cinfo;			// IJPEG decoder state
	my_jpeg_error_mgr jsrcerr, jerr;	// Custom error manager
	jsrcerr.szLastLibJpegError[0] = '\0';
	jerr.szLastLibJpegError[0] = '\0';
	BOOL bCopySrcMarkers;
	if ((lpszMarkersFrom == NULL) || (lpszMarkersFrom[0] == _T('\0')) ||
		FileSignatureToExtension(lpszMarkersFrom) != _T(".jpg"))
		bCopySrcMarkers = FALSE;
	else
		bCopySrcMarkers = TRUE;

	try
	{
		// Setup Error Handling
		srcinfo.err = jpeg_std_error(&jsrcerr.pub);
		jsrcerr.pub.error_exit = jpeg_error_load;
		cinfo.err = jpeg_std_error(&jerr.pub);
		jerr.pub.error_exit = jpeg_error_save;

		// Initialize the JPEG decompression object
		jpeg_create_decompress(&srcinfo);

		// Initialize the JPEG compression object
		jpeg_create_compress(&cinfo);

		if (pOutputSize == NULL)
			throw (int)JPEG_E_MEMDSTSIZE;

		if (!m_pBits)
		{
			if (!DibSectionToBits())
				throw (int)JPEG_E_BADBMP;
		}

		if (!m_pBits || !m_pBMI)
			throw (int)JPEG_E_BADBMP;

		if (IsCompressed())
		{
			if (!Decompress(GetBitCount())) // Decompress
				throw (int)JPEG_E_BADBMP;
		}

		if ((quality < 0) || (quality > 100))
			quality = DEFAULT_JPEGCOMPRESSION;
	
		// Open the input file
		if (bCopySrcMarkers)
		{
			if ((infile = _tfopen(lpszMarkersFrom, _T("rb"))) == NULL)
				bCopySrcMarkers = FALSE;
			else
			{	
				// Specify data source for decompression
				jpeg_stdio_src(&srcinfo, infile);

				// Enable saving of extra markers that we want to copy
				jcopy_markers_setup(&srcinfo, copyoption);

				// Read file header
				if (!jpeg_read_header(&srcinfo, TRUE))
					bCopySrcMarkers = FALSE;
			}
		}

		jpeg_mem_dest(&cinfo, pOutput, pOutputSize);

		// Set Header Fields
		cinfo.image_width = GetWidth();
		cinfo.image_height = GetHeight();

		if (bGrayScale == FALSE)
		{
			cinfo.input_components = 3;
			cinfo.in_color_space = JCS_RGB;
			
			jpeg_set_defaults(&cinfo);
			jpeg_set_quality(&cinfo, quality, TRUE); // Quality between 0..100
			// density_unit:
			// 0 for unknown
			// 1 for dots/inch
			// 2 for dots/cm
			cinfo.write_JFIF_header = TRUE;
			cinfo.density_unit = 1;
			cinfo.X_density = (UINT16)GetXDpi();
			cinfo.Y_density = (UINT16)GetYDpi();
			cinfo.dct_method = JDCT_FLOAT;
			jpeg_start_compress(&cinfo, TRUE);

			// Copy to the output file any extra markers that we want to preserve
			if (bCopySrcMarkers)
				jcopy_markers_execute(&srcinfo, &cinfo, copyoption);
			else if (bSaveMetadata)
				SaveJPEGWriteMetadata(cinfo);

			DWORD uiOutputDIBScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * 24);
			DWORD uiInputDIBScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * GetBitCount());

			// In the jpeg file the scan lines are top / down 
			// In a DIB the scan lines are stored bottom / up
			LPBYTE lpBits = m_pBits;
			lpBits += uiInputDIBScanLineSize*(GetHeight()-1);

			switch(GetBitCount())
			{
				case 1: // 1bit to 24bit conversion
				{
					buf = new BYTE[uiOutputDIBScanLineSize];
					int n;
					BYTE PixelIndex = 0;
					unsigned int ByteNum = 0;
					BYTE BitNum = 0;
					for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
					{
						if (((CurLine & 0x7) == 0) && pThread && pThread->DoExit())
							throw (int)JPEG_THREADEXIT;

						n = 0;
						for(unsigned int i = 0 ; i < GetWidth(); i++)
						{
							ByteNum = i / 8;
							BitNum = (BYTE)(7 - (i % 8)); // 7=MSBit .. 0=LSBit
							// First pixel is the most significant bit of the byte:
							PixelIndex = (BYTE)(1 & (lpBits[ByteNum] >> BitNum));
							buf[n++] = m_pColors[PixelIndex].rgbRed;
							buf[n++] = m_pColors[PixelIndex].rgbGreen;
							buf[n++] = m_pColors[PixelIndex].rgbBlue;
						}
						jpeg_write_scanlines(&cinfo, &buf, 1);
						lpBits -= uiInputDIBScanLineSize;
					}
					delete [] buf;
					break;
				}
				case 4: // 4bit to 24bit conversion
				{
					buf = new BYTE[uiOutputDIBScanLineSize];
					int n;
					BYTE LowPixelIndex = 0;
					BYTE HighPixelIndex = 0;
					for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
					{
						if (((CurLine & 0x7) == 0) && pThread && pThread->DoExit())
							throw (int)JPEG_THREADEXIT;

						n = 0;
						unsigned int i;
						for (i = 0 ; i < GetWidth()/2; i++)
						{
							LowPixelIndex = (BYTE)(0x0F & lpBits[i]);
							HighPixelIndex = (BYTE)((0xF0 & lpBits[i]) >> 4);
							buf[n++] = m_pColors[HighPixelIndex].rgbRed;
							buf[n++] = m_pColors[HighPixelIndex].rgbGreen;
							buf[n++] = m_pColors[HighPixelIndex].rgbBlue;
							buf[n++] = m_pColors[LowPixelIndex].rgbRed;
							buf[n++] = m_pColors[LowPixelIndex].rgbGreen;
							buf[n++] = m_pColors[LowPixelIndex].rgbBlue;
						}
						// odd number of pixels
						if (GetWidth() & 1)
						{
							HighPixelIndex = (BYTE)((0xF0 & lpBits[i]) >> 4);
							buf[n++] = m_pColors[HighPixelIndex].rgbRed;
							buf[n++] = m_pColors[HighPixelIndex].rgbGreen;
							buf[n++] = m_pColors[HighPixelIndex].rgbBlue;
						}
						jpeg_write_scanlines(&cinfo, &buf, 1);
						lpBits -= uiInputDIBScanLineSize;
					}
					delete [] buf;
					break;
				}
				case 8: // 8bit to 24bit conversion
				{
					buf = new BYTE[uiOutputDIBScanLineSize];
					for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
					{
						if (((CurLine & 0x7) == 0) && pThread && pThread->DoExit())
							throw (int)JPEG_THREADEXIT;

						for(unsigned int i = 0 ; i < GetWidth() ; i++)
						{
							buf[3*i] =   m_pColors[lpBits[i]].rgbRed;
							buf[3*i+1] = m_pColors[lpBits[i]].rgbGreen;
							buf[3*i+2] = m_pColors[lpBits[i]].rgbBlue;
						}
						jpeg_write_scanlines(&cinfo, &buf, 1);
						lpBits -= uiInputDIBScanLineSize;
					}
					delete [] buf;
					break;
				}
				case 16: // 16bit to 24bit conversion
				{
					buf = new BYTE[uiOutputDIBScanLineSize];
					int n;
					for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
					{
						if (((CurLine & 0x7) == 0) && pThread && pThread->DoExit())
							throw (int)JPEG_THREADEXIT;

						n = 0;
						for (unsigned int i = 0 ; i < GetWidth() ; i++)
						{
							DIB16ToRGB(((WORD*)lpBits)[i], &(buf[n]), &(buf[n+1]), &(buf[n+2]));
							n += 3;
						}
						jpeg_write_scanlines(&cinfo, &buf, 1);
						lpBits -= uiInputDIBScanLineSize;
					}
					delete [] buf;
					break;
				}
				case 24:
				{
					buf = new BYTE[uiOutputDIBScanLineSize];
					int n;
					for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
					{
						if (((CurLine & 0x7) == 0) && pThread && pThread->DoExit())
							throw (int)JPEG_THREADEXIT;

						n = 0;
						for (unsigned int i = 0 ; i < GetWidth() ; i++)
						{
							buf[n]   = lpBits[n+2];	// Red
							buf[n+1] = lpBits[n+1];	// Green
							buf[n+2] = lpBits[n];	// Blue
							n += 3;
						}
						jpeg_write_scanlines(&cinfo, &buf, 1);
						lpBits -= uiInputDIBScanLineSize;
					}
					delete [] buf;
					break;
				}
				case 32: // 32bit to 24bit conversion
				{
					buf = new BYTE[uiOutputDIBScanLineSize];
					int n;
					for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
					{
						if (((CurLine & 0x7) == 0) && pThread && pThread->DoExit())
							throw (int)JPEG_THREADEXIT;

						n = 0;
						for (unsigned int i = 0 ; i < GetWidth() ; i++)
						{
							DIB32ToRGB(((DWORD*)lpBits)[i], &(buf[n]), &(buf[n+1]), &(buf[n+2]));
							n += 3;
						}
						jpeg_write_scanlines(&cinfo, &buf, 1);
						lpBits -= uiInputDIBScanLineSize;
					}
					delete [] buf;
					break;
				}
				default:
					break;
			}
		}
		else // Gray Scale Save
		{
			cinfo.input_components = 1;
			cinfo.in_color_space = JCS_GRAYSCALE;
			
			jpeg_set_defaults(&cinfo);
			jpeg_set_quality(&cinfo, quality, TRUE); // Quality between 0..100
			// density_unit:
			// 0 for unknown
			// 1 for dots/inch
			// 2 for dots/cm
			cinfo.write_JFIF_header = TRUE;
			cinfo.density_unit = 1;
			cinfo.X_density = (UINT16)GetXDpi();
			cinfo.Y_density = (UINT16)GetYDpi();
			cinfo.dct_method = JDCT_FLOAT;
			jpeg_start_compress(&cinfo, TRUE);

			// Copy to the output file any extra markers that we want to preserve
			if (bCopySrcMarkers)
				jcopy_markers_execute(&srcinfo, &cinfo, copyoption);
			else if (bSaveMetadata)
				SaveJPEGWriteMetadata(cinfo);

			DWORD uiOutputDIBScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * 8);
			DWORD uiInputDIBScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * GetBitCount());

			// In the jpeg file the scan lines are top / down 
			// In a DIB the scan lines are stored bottom / up
			LPBYTE lpBits = m_pBits;
			lpBits += uiInputDIBScanLineSize*(GetHeight()-1);

			switch (GetBitCount())
			{
				case 1: // 1bit to 8bit gray scale conversion
				{
					buf = new BYTE[uiOutputDIBScanLineSize];
					BYTE PixelIndex = 0;
					unsigned int ByteNum = 0;
					BYTE BitNum = 0;
					for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
					{
						if (((CurLine & 0x7) == 0) && pThread && pThread->DoExit())
							throw (int)JPEG_THREADEXIT;

						for (unsigned int i = 0 ; i < GetWidth(); i++)
						{
							ByteNum = i / 8;
							BitNum = (BYTE)(7 - (i % 8)); // 7=MSBit .. 0=LSBit
							// First pixel is the most significant bit of the byte:
							PixelIndex = (BYTE)(1 & (lpBits[ByteNum] >> BitNum));
							buf[i] = RGBToGray(	m_pColors[PixelIndex].rgbRed,
												m_pColors[PixelIndex].rgbGreen,
												m_pColors[PixelIndex].rgbBlue);
						}
						jpeg_write_scanlines(&cinfo, &buf, 1);
						lpBits -= uiInputDIBScanLineSize;
					}
					delete [] buf;
					break;
				}
				case 4: // 4bit to 8bit gray scale conversion
				{
					buf = new BYTE[uiOutputDIBScanLineSize];
					BYTE LowPixelIndex = 0;
					BYTE HighPixelIndex = 0;
					int n;
					for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
					{
						if (((CurLine & 0x7) == 0) && pThread && pThread->DoExit())
							throw (int)JPEG_THREADEXIT;

						n = 0;
						unsigned int i;
						for (i = 0 ; i < GetWidth()/2; i++)
						{
							LowPixelIndex = (BYTE)(0x0F & lpBits[i]);
							HighPixelIndex = (BYTE)((0xF0 & lpBits[i]) >> 4);
							buf[n++] = RGBToGray(	m_pColors[HighPixelIndex].rgbRed,
													m_pColors[HighPixelIndex].rgbGreen,
													m_pColors[HighPixelIndex].rgbBlue);
							buf[n++] = RGBToGray(	m_pColors[LowPixelIndex].rgbRed,
													m_pColors[LowPixelIndex].rgbGreen,
													m_pColors[LowPixelIndex].rgbBlue);
						}
						// odd number of pixels
						if (GetWidth() & 1)
						{
							HighPixelIndex = (BYTE)((0xF0 & lpBits[i]) >> 4);
							buf[n++] = RGBToGray(	m_pColors[HighPixelIndex].rgbRed,
													m_pColors[HighPixelIndex].rgbGreen,
													m_pColors[HighPixelIndex].rgbBlue);
						}
						jpeg_write_scanlines(&cinfo, &buf, 1);
						lpBits -= uiInputDIBScanLineSize;
					}
					delete [] buf;
					break;
				}
				case 8: // 8bit to 8bit gray scale conversion
				{
					buf = new BYTE[uiOutputDIBScanLineSize];
					for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
					{
						if (((CurLine & 0x7) == 0) && pThread && pThread->DoExit())
							throw (int)JPEG_THREADEXIT;

						for (unsigned int i = 0 ; i < GetWidth() ; i++)
						{
							buf[i] = RGBToGray(	m_pColors[lpBits[i]].rgbRed,
												m_pColors[lpBits[i]].rgbGreen,
												m_pColors[lpBits[i]].rgbBlue);
						}
						jpeg_write_scanlines(&cinfo, &buf, 1);
						lpBits -= uiInputDIBScanLineSize;
					}
					delete [] buf;
					break;
				}
				case 16: // 16bit to 8bit gray scale conversion
				{
					buf = new BYTE[uiOutputDIBScanLineSize];
					for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
					{
						if (((CurLine & 0x7) == 0) && pThread && pThread->DoExit())
							throw (int)JPEG_THREADEXIT;

						for (unsigned int i = 0 ; i < GetWidth() ; i++)
						{
							BYTE red, green, blue;
							DIB16ToRGB(((WORD*)lpBits)[i], &red, &green, &blue);
							buf[i] = RGBToGray(red, green, blue);
						}
						jpeg_write_scanlines(&cinfo, &buf, 1);
						lpBits -= uiInputDIBScanLineSize;
					}
					delete [] buf;
					break;
				}
				case 24: // 24bit to 8bit gray scale conversion
				{
					buf = new BYTE[uiOutputDIBScanLineSize];
					for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
					{
						if (((CurLine & 0x7) == 0) && pThread && pThread->DoExit())
							throw (int)JPEG_THREADEXIT;

						for (unsigned int i = 0 ; i < GetWidth() ; i++)
						{
							buf[i] = RGBToGray(lpBits[3*i+2], lpBits[3*i+1], lpBits[3*i]);
						}
						jpeg_write_scanlines(&cinfo, &buf, 1);
						lpBits -= uiInputDIBScanLineSize;
					}
					delete [] buf;
					break;
				}
				case 32: // 32bit to 8bit gray scale conversion
				{
					buf = new BYTE[uiOutputDIBScanLineSize];
					for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
					{
						if (((CurLine & 0x7) == 0) && pThread && pThread->DoExit())
							throw (int)JPEG_THREADEXIT;

						for (unsigned int i = 0 ; i < GetWidth() ; i++)
						{
							BYTE red, green, blue;
							DIB32ToRGB(((DWORD*)lpBits)[i], &red, &green, &blue);
							buf[i] = RGBToGray(red, green, blue);
						}
						jpeg_write_scanlines(&cinfo, &buf, 1);
						lpBits -= uiInputDIBScanLineSize;
					}
					delete [] buf;
					break;
				}
				default:
					break;
			}
		}

		jpeg_destroy_decompress(&srcinfo);
		if (infile)
			fclose(infile);

		jpeg_finish_compress(&cinfo);
		jpeg_destroy_compress(&cinfo);

		if (bDoUpdateExifWidthHeightThumb)
			UpdateExifWidthHeightThumb(pOutput, *pOutputSize, m_bShowMessageBoxOnError);

		return TRUE;
	}
	catch (int error_code)
	{
		// Free & Close
		if (buf)
			delete [] buf;
		jpeg_destroy_decompress(&srcinfo);
		jpeg_destroy_compress(&cinfo);
		if (infile)
			fclose(infile);

		// Just Exit
		if (error_code == JPEG_THREADEXIT)
			return FALSE;

		CString str;
#ifdef _DEBUG
		str = _T("SaveJPEG: ");
#endif
		switch (error_code)
		{
			case JPEG_E_LIBJPEG_LOAD :	str += CString(jsrcerr.szLastLibJpegError) + _T("\n");
			break;
			case JPEG_E_LIBJPEG_SAVE :	str += CString(jerr.szLastLibJpegError) + _T("\n");
			break;
			case JPEG_E_BADBMP :		str += _T("Corrupted or unsupported DIB\n");
			break;
			case JPEG_E_MEMDSTSIZE :	str += _T("Memory Destination size pointer is not specified\n");
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

// Transform:
// JXFORM_NONE,			no transformation
// JXFORM_FLIP_H,		horizontal flip
// JXFORM_FLIP_V,		vertical flip
// JXFORM_TRANSPOSE,	flip across TopLeft-to-BottomRight axis
// JXFORM_TRANSVERSE,	flip across TopRight-to-BottomLeft axis
// JXFORM_ROT_90,		90-degree clockwise rotation
// JXFORM_ROT_180,		180-degree rotation
// JXFORM_ROT_270,		270-degree clockwise (or 90 ccw)
//
// The bTrim option causes untransformable partial iMCUs to be dropped;
// this is not strictly lossless, but it usually gives the best-looking
// result for odd-size images.
BOOL CDib::LossLessJPEGTrans(	LPCTSTR lpszInPathName, LPCTSTR lpszOutPathName,
								JXFORM_CODE Transform, BOOL bTrim/*=TRUE*/,
								BOOL bGrayScale/*=FALSE*/,
								BOOL bDoUpdateExif/*=TRUE*/,
								LPCSTR lpszComment/*=""*/,
								BOOL bCrop/*=FALSE*/,
								DWORD dwCropOrigX/*=0*/, DWORD dwCropOrigY/*=0*/,
								DWORD dwCropWidth/*=0*/, DWORD dwCropHeight/*=0*/,
								BOOL bShowMessageBoxOnError/*=TRUE*/,
								CWnd* pProgressWnd/*=NULL*/,
								BOOL bProgressSend/*=TRUE*/)
{
	// Markers Copy Option
	// Do not Forget To Define SAVE_MARKERS_SUPPORTED !!!
	JCOPY_OPTION copyoption = JCOPYOPT_ALL;
	
	// Image transformation options
	jpeg_transform_info transformoption;
	transformoption.transform = Transform;
	transformoption.trim = bTrim;
	transformoption.force_grayscale = bGrayScale;
	transformoption.crop = bCrop;
	transformoption.crop_width = dwCropWidth;	// Width of selected region
	transformoption.crop_width_set = bCrop ? JCROP_POS : JCROP_UNSET;
	transformoption.crop_height = dwCropHeight;	// Height of selected region
	transformoption.crop_height_set = bCrop ? JCROP_POS : JCROP_UNSET;
	transformoption.crop_xoffset = dwCropOrigX;	// X offset of selected region
	transformoption.crop_xoffset_set = bCrop ? JCROP_POS : JCROP_UNSET;
	transformoption.crop_yoffset = dwCropOrigY;	// Y offset of selected region
	transformoption.crop_yoffset_set = bCrop ? JCROP_POS : JCROP_UNSET;

	struct jpeg_decompress_struct srcinfo;
	struct jpeg_compress_struct dstinfo;
	struct my_jpeg_error_mgr jsrcerr;
	struct my_jpeg_error_mgr jdsterr;
	jsrcerr.szLastLibJpegError[0] = '\0';
	jdsterr.szLastLibJpegError[0] = '\0';
	
	jvirt_barray_ptr * src_coef_arrays;
	jvirt_barray_ptr * dst_coef_arrays;
	FILE * infile = NULL;
	FILE * outfile = NULL;

	try
	{
		// Initialize the JPEG decompression object
		srcinfo.err = jpeg_std_error(&jsrcerr.pub);
		jsrcerr.pub.error_exit = jpeg_error_load;
		jpeg_create_decompress(&srcinfo);

		// Initialize the JPEG compression object
		dstinfo.err = jpeg_std_error(&jdsterr.pub);
		jdsterr.pub.error_exit = jpeg_error_save;
		jpeg_create_compress(&dstinfo);

		CString sInPathName(lpszInPathName);
		if (sInPathName.IsEmpty())
			throw (int)JPEG_E_ZEROPATH;

		CString sOutPathName(lpszOutPathName);
		if (sOutPathName.IsEmpty())
			throw (int)JPEG_E_ZEROPATH;

		// Open the input file
		if ((infile = _tfopen(lpszInPathName, _T("rb"))) == NULL)
			throw (int)JPEG_E_READ;

		// Open the output file
		if ((outfile = _tfopen(lpszOutPathName, _T("wb"))) == NULL)
			throw (int)JPEG_E_WRITE;

		// Specify data source for decompression
		jpeg_stdio_src(&srcinfo, infile);

		// Enable saving of extra markers that we want to copy
		jcopy_markers_setup(&srcinfo, copyoption);

		// Read file header
		if(!jpeg_read_header(&srcinfo, TRUE))
			throw (int)JPEG_E_BADJPEG;	

		// Any space needed by a transform option must be requested before
		// jpeg_read_coefficients so that memory allocation will be done right
		jtransform_request_workspace(&srcinfo, &transformoption);

		// Read source file as DCT coefficients
		src_coef_arrays = jpeg_read_coefficients(&srcinfo);

		// Initialize destination compression parameters from source values
		jpeg_copy_critical_parameters(&srcinfo, &dstinfo);

		// Adjust destination parameters if required by transform options;
		// also find out which set of coefficient arrays will hold the output.
		dst_coef_arrays = jtransform_adjust_parameters(&srcinfo, &dstinfo,
						 src_coef_arrays,
						 &transformoption);

		// Specify data destination for compression
		jpeg_stdio_dest(&dstinfo, outfile);

		// Swap Dpi
		if (Transform == JXFORM_TRANSPOSE	||
			Transform == JXFORM_TRANSVERSE	||
			Transform == JXFORM_ROT_90		||
			Transform == JXFORM_ROT_270)
		{
			dstinfo.X_density = srcinfo.Y_density;
			dstinfo.Y_density = srcinfo.X_density;
		}

		// Start compressor and write JFIF marker
		// (note no image data is actually written here)
		jpeg_write_coefficients(&dstinfo, dst_coef_arrays);

		// Copy to the output file any extra markers that we want to preserve
		// and add comment if given
		if ((lpszComment == NULL) || (lpszComment[0] == _T('\0')))
			jcopy_markers_execute(&srcinfo, &dstinfo, copyoption);
		else
		{
			jcopy_markers_execute_addmarker (	&srcinfo,
												&dstinfo, 
												JPEG_COM,
												(JOCTET*)lpszComment,
												(INT32)((strlen(lpszComment) + 1)),
												copyoption);
		}

		// Execute image transformation
		jtransform_execute_transformation(	&srcinfo,
											&dstinfo,
											src_coef_arrays,
											&transformoption,
											(void*)pProgressWnd->GetSafeHwnd(),
											(int)(bProgressSend ? 1 : 0));

		// Finish compression and release memory
		jpeg_finish_compress(&dstinfo);
		jpeg_destroy_compress(&dstinfo);
		jpeg_finish_decompress(&srcinfo);
		jpeg_destroy_decompress(&srcinfo);

		// Close files
		fclose(infile);
		fclose(outfile);

		// Copy SOFn Width and Height to the Exif Width and Height
		// This because the lossless jpeg transformation library is a bit buggy:
		// If trim is set and happens (odd sized jpegs), the Exif width and height are not trimmed...
		// (To be fixed...)
		// The Thumbnail is Also Adjusted
		if (bDoUpdateExif)
		{
			if (bCrop)
				UpdateExifWidthHeightThumb(lpszOutPathName, bShowMessageBoxOnError);
			else
				UpdateExifWidthHeightThumbLossLess(	lpszOutPathName,
													Transform,
													bTrim,
													bGrayScale,
													bShowMessageBoxOnError);
		}

		return TRUE;
	}
	catch (CFileException* e)
	{
		TCHAR szCause[255];
		CString str(_T("LossLessJPEGTrans: "));
		e->GetErrorMessage(szCause, 255);
		str += szCause;
		str += _T("\n");
		TRACE(str);
		if (bShowMessageBoxOnError)
			::AfxMessageBox(str, MB_ICONSTOP);
		e->Delete();

		return FALSE;
	}
	catch (int error_code)
	{
		jpeg_destroy_compress(&dstinfo);
		jpeg_destroy_decompress(&srcinfo);
		if (outfile)
			fclose(outfile);
		if (infile)
			fclose(infile);

		CString str;
#ifdef _DEBUG
		str.Format(_T("LossLessJPEGTrans(%s ->\n%s):\n"), lpszInPathName, lpszOutPathName);
#endif
		switch (error_code)
		{
			case JPEG_E_ZEROPATH :		str += _T("The file name is zero\n");
			break;
			case JPEG_E_LIBJPEG_LOAD :	str += CString(jsrcerr.szLastLibJpegError) + _T("\n");
			break;
			case JPEG_E_LIBJPEG_SAVE :	str += CString(jdsterr.szLastLibJpegError) + _T("\n");
			break;
			case JPEG_E_BADJPEG :		str += _T("Corrupted or unsupported JPEG\n");
			break;
			case JPEG_E_READ :			str += _T("Couldn't read JPEG file\n");
			break;
			case JPEG_E_WRITE :			str += _T("Couldn't write JPEG file\n");
			break;
			default:					str += _T("Unspecified error\n");
			break;
		}
		
		TRACE(str);
		if (bShowMessageBoxOnError)
			::AfxMessageBox(str, MB_ICONSTOP);

		return FALSE;
	}
}

// Transform:
// JXFORM_NONE,			no transformation
// JXFORM_FLIP_H,		horizontal flip
// JXFORM_FLIP_V,		vertical flip
// JXFORM_TRANSPOSE,	flip across TopLeft-to-BottomRight axis
// JXFORM_TRANSVERSE,	flip across TopRight-to-BottomLeft axis
// JXFORM_ROT_90,		90-degree clockwise rotation
// JXFORM_ROT_180,		180-degree rotation
// JXFORM_ROT_270,		270-degree clockwise (or 90 ccw)
//
// The bTrim option causes untransformable partial iMCUs to be dropped;
// this is not strictly lossless, but it usually gives the best-looking
// result for odd-size images.
BOOL CDib::LossLessJPEGTrans(	LPBYTE pJpegInData, int nJpegInDataSize,
								LPBYTE pJpegOutData, int* pJpegOutDataSize,
								JXFORM_CODE Transform, BOOL bTrim/*=TRUE*/,
								BOOL bGrayScale/*=FALSE*/,
								BOOL bDoUpdateExif/*=TRUE*/,
								LPCSTR lpszComment/*=""*/,
								BOOL bCrop/*=FALSE*/,
								DWORD dwCropOrigX/*=0*/, DWORD dwCropOrigY/*=0*/,
								DWORD dwCropWidth/*=0*/, DWORD dwCropHeight/*=0*/,
								BOOL bShowMessageBoxOnError/*=TRUE*/,
								CWnd* pProgressWnd/*=NULL*/,
								BOOL bProgressSend/*=TRUE*/)
{
	// Markers Copy Option
	// Do not Forget To Define SAVE_MARKERS_SUPPORTED !!!
	JCOPY_OPTION copyoption = JCOPYOPT_ALL;
	
	// Image transformation options
	jpeg_transform_info transformoption;
	transformoption.transform = Transform;
	transformoption.trim = bTrim;
	transformoption.force_grayscale = bGrayScale;
	transformoption.crop = bCrop;
	transformoption.crop_width = dwCropWidth;	// Width of selected region
	transformoption.crop_width_set = bCrop ? JCROP_POS : JCROP_UNSET;
	transformoption.crop_height = dwCropHeight;	// Height of selected region
	transformoption.crop_height_set = bCrop ? JCROP_POS : JCROP_UNSET;
	transformoption.crop_xoffset = dwCropOrigX;	// X offset of selected region
	transformoption.crop_xoffset_set = bCrop ? JCROP_POS : JCROP_UNSET;
	transformoption.crop_yoffset = dwCropOrigY;	// Y offset of selected region
	transformoption.crop_yoffset_set = bCrop ? JCROP_POS : JCROP_UNSET;

	struct jpeg_decompress_struct srcinfo;
	struct jpeg_compress_struct dstinfo;
	struct my_jpeg_error_mgr jsrcerr;
	struct my_jpeg_error_mgr jdsterr;
	jsrcerr.szLastLibJpegError[0] = '\0';
	jdsterr.szLastLibJpegError[0] = '\0';
	
	jvirt_barray_ptr * src_coef_arrays;
	jvirt_barray_ptr * dst_coef_arrays;

	try
	{
		// Initialize the JPEG decompression object
		srcinfo.err = jpeg_std_error(&jsrcerr.pub);
		jsrcerr.pub.error_exit = jpeg_error_load;
		jpeg_create_decompress(&srcinfo);

		// Initialize the JPEG compression object
		dstinfo.err = jpeg_std_error(&jdsterr.pub);
		jdsterr.pub.error_exit = jpeg_error_save;
		jpeg_create_compress(&dstinfo);

		// Check Pointer
		if (pJpegInData == NULL)
			throw (int)JPEG_E_MEMSRC;

		// Specify data source for decompression
		jpeg_mem_src(&srcinfo, pJpegInData, nJpegInDataSize);

		// Enable saving of extra markers that we want to copy
		jcopy_markers_setup(&srcinfo, copyoption);

		// Read file header
		if(!jpeg_read_header(&srcinfo, TRUE))
			throw (int)JPEG_E_BADJPEG;	

		// Any space needed by a transform option must be requested before
		// jpeg_read_coefficients so that memory allocation will be done right
		jtransform_request_workspace(&srcinfo, &transformoption);

		// Read source file as DCT coefficients
		src_coef_arrays = jpeg_read_coefficients(&srcinfo);

		// Initialize destination compression parameters from source values
		jpeg_copy_critical_parameters(&srcinfo, &dstinfo);

		// Adjust destination parameters if required by transform options;
		// also find out which set of coefficient arrays will hold the output.
		dst_coef_arrays = jtransform_adjust_parameters(&srcinfo, &dstinfo,
						 src_coef_arrays,
						 &transformoption);

		// Specify data destination for compression
		jpeg_mem_dest(&dstinfo, pJpegOutData, pJpegOutDataSize);

		// Swap Dpi
		if (Transform == JXFORM_TRANSPOSE	||
			Transform == JXFORM_TRANSVERSE	||
			Transform == JXFORM_ROT_90		||
			Transform == JXFORM_ROT_270)
		{
			dstinfo.X_density = srcinfo.Y_density;
			dstinfo.Y_density = srcinfo.X_density;
		}

		// Start compressor and write JFIF marker
		// (note no image data is actually written here)
		jpeg_write_coefficients(&dstinfo, dst_coef_arrays);

		// Copy to the output file any extra markers that we want to preserve
		// and add comment if given
		if ((lpszComment == NULL) || (lpszComment[0] == _T('\0')))
			jcopy_markers_execute(&srcinfo, &dstinfo, copyoption);
		else
		{
			jcopy_markers_execute_addmarker (	&srcinfo,
												&dstinfo,
												JPEG_COM,
												(JOCTET*)lpszComment,
												(INT32)((strlen(lpszComment) + 1)),
												copyoption);
		}

		// Execute image transformation
		jtransform_execute_transformation(	&srcinfo,
											&dstinfo,
											src_coef_arrays,
											&transformoption,
											(void*)pProgressWnd->GetSafeHwnd(),
											(int)(bProgressSend ? 1 : 0));

		// Finish compression and release memory
		jpeg_finish_compress(&dstinfo);
		jpeg_destroy_compress(&dstinfo);
		jpeg_finish_decompress(&srcinfo);
		jpeg_destroy_decompress(&srcinfo);

		// Copy SOFn Width and Height to the Exif Width and Height
		// This because the lossless jpeg transformation library is a bit buggy:
		// If trim is set and happens (odd sized jpegs), the Exif width and height are not trimmed...
		// (To be fixed...)
		// The Thumbnail is Also Adjusted
		if (bDoUpdateExif)
		{
			if (bCrop)
				UpdateExifWidthHeightThumb(pJpegOutData, *pJpegOutDataSize, bShowMessageBoxOnError);
			else
				UpdateExifWidthHeightThumbLossLess(	pJpegOutData,
													*pJpegOutDataSize,
													Transform,
													bTrim,
													bGrayScale,
													bShowMessageBoxOnError);
		}

		return TRUE;
	}
	catch (CFileException* e)
	{
		TCHAR szCause[255];
		CString str(_T("LossLessJPEGTrans: "));
		e->GetErrorMessage(szCause, 255);
		str += szCause;
		str += _T("\n");
		TRACE(str);
		if (bShowMessageBoxOnError)
			::AfxMessageBox(str, MB_ICONSTOP);
		e->Delete();

		return FALSE;
	}
	catch (int error_code)
	{
		jpeg_destroy_compress(&dstinfo);
		jpeg_destroy_decompress(&srcinfo);

		CString str;
#ifdef _DEBUG
		(_T("LossLessJPEGTrans: "));
#endif
		switch (error_code)
		{
			case JPEG_E_MEMDST :		str += _T("Memory Destination is not specified\n");
			break;
			case JPEG_E_LIBJPEG_LOAD :	str += CString(jsrcerr.szLastLibJpegError) + _T("\n");
			break;
			case JPEG_E_LIBJPEG_SAVE :	str += CString(jdsterr.szLastLibJpegError) + _T("\n");
			break;
			case JPEG_E_BADJPEG :		str += _T("Corrupted or unsupported JPEG\n");
			break;
			default:					str += _T("Unspecified error\n");
			break;
		}
		
		TRACE(str);
		if (bShowMessageBoxOnError)
			::AfxMessageBox(str, MB_ICONSTOP);

		return FALSE;
	}
}

BOOL CDib::UpdateExifWidthHeightThumbLossLess(	LPCTSTR lpszPathName,
												JXFORM_CODE Transform,
												BOOL bTrim,
												BOOL bGrayScale,
												BOOL bShowMessageBoxOnError)
{
	// Note:
	// When Loading m_ExifInfo.Width and m_ExifInfo.Height are taken
	// from SOFn markers (not from Exif section!),
	// when writing the m_ExifInfo.Width and m_ExifInfo.Height
	// values are written to the Exif section (not to the SOFn markers!).
	// This because SOFn are more reliable for the image size.

	CDib Dib;
	Dib.SetShowMessageBoxOnError(bShowMessageBoxOnError);
	if (Dib.JPEGLoadMetadata(lpszPathName) && Dib.GetMetadata()->m_ExifInfo.bHasExifSubIFD)
	{
		// If the thumbnail is not jpeg 
		// -> do a lossy thumbnail transformation
		if (Dib.GetMetadata()->m_ExifInfo.ThumbnailCompression != 6)
			return UpdateExifWidthHeightThumb(lpszPathName, bShowMessageBoxOnError);

		memset(&Dib.GetMetadata()->m_ExifInfoWrite, 0, sizeof(CMetadata::EXIFINFOINPLACEWRITE));
		LPBYTE pJpegOutThumbData = NULL;
		if (Dib.GetMetadata()->m_ExifInfo.ThumbnailPointer && Dib.GetMetadata()->m_ExifInfo.ThumbnailSize)
		{
			// Calc Output Size, it should be the same, but in some cases it is bigger -> Lossy Exif Update
			// (All the Exif Stores are in place -> the size must be the same or smaller!)
			int nJpegOutThumbDataSize = 0;
			if (!LossLessJPEGTrans(Dib.GetMetadata()->m_ExifInfo.ThumbnailPointer, Dib.GetMetadata()->m_ExifInfo.ThumbnailSize,
							 NULL, &nJpegOutThumbDataSize,
							 Transform, bTrim,
							 bGrayScale, FALSE, 
							 "",
							 FALSE, 0, 0, 0, 0, bShowMessageBoxOnError))
				return FALSE;
			if (nJpegOutThumbDataSize > (int)Dib.GetMetadata()->m_ExifInfo.ThumbnailSize)
				return UpdateExifWidthHeightThumb(lpszPathName, bShowMessageBoxOnError);
			else
				pJpegOutThumbData = new BYTE[Dib.GetMetadata()->m_ExifInfo.ThumbnailSize];

			// Do Lossless Transformation On Thumbnail
			Dib.GetMetadata()->m_ExifInfoWrite.bThumbnail =
			LossLessJPEGTrans(Dib.GetMetadata()->m_ExifInfo.ThumbnailPointer, Dib.GetMetadata()->m_ExifInfo.ThumbnailSize,
							 pJpegOutThumbData, &nJpegOutThumbDataSize,
							 Transform, bTrim,
							 bGrayScale, FALSE, 
							 "",
							 FALSE, 0, 0, 0, 0, bShowMessageBoxOnError) ? true : false;
			Dib.GetMetadata()->m_ExifInfo.ThumbnailPointer = pJpegOutThumbData;
			// Leave the Same Size to avoid a continuos shrinking if editing (lossy) many times!
			// Dib.GetMetadata()->m_ExifInfo.ThumbnailSize = nJpegOutThumbDataSize;
			CDib ThumbDib;
			ThumbDib.SetShowMessageBoxOnError(bShowMessageBoxOnError);
			ThumbDib.JPEGLoadMetadata(pJpegOutThumbData, nJpegOutThumbDataSize); // It has no Exif, but Width and Height are returned from SOFn
			Dib.GetMetadata()->m_ExifInfo.ThumbnailWidth = ThumbDib.GetExifInfo()->Width;
			Dib.GetMetadata()->m_ExifInfo.ThumbnailHeight = ThumbDib.GetExifInfo()->Height;
			Dib.GetMetadata()->m_ExifInfoWrite.bThumbnailWidth = true;
			Dib.GetMetadata()->m_ExifInfoWrite.bThumbnailHeight = true;
		}
		Dib.GetMetadata()->m_ExifInfoWrite.bWidth = true;
		Dib.GetMetadata()->m_ExifInfoWrite.bHeight = true;
		BOOL res = Dib.JPEGWriteEXIFInplace(lpszPathName);
		if (pJpegOutThumbData)
			delete [] pJpegOutThumbData;
		return res;
	}
	else
		return FALSE;
}

BOOL CDib::UpdateExifWidthHeightThumb(LPCTSTR lpszPathName, BOOL bShowMessageBoxOnError)
{
	// Note:
	// When Loading m_ExifInfo.Width and m_ExifInfo.Height are taken
	// from SOFn markers (not from Exif section!),
	// when writing the m_ExifInfo.Width and m_ExifInfo.Height
	// values are written to the Exif section (not to the SOFn markers!).
	// This because SOFn are more reliable for the image size.

	CDib Dib;
	Dib.SetShowMessageBoxOnError(bShowMessageBoxOnError);
	if (Dib.JPEGLoadMetadata(lpszPathName) && Dib.GetMetadata()->m_ExifInfo.bHasExifSubIFD)
	{
		memset(&Dib.GetMetadata()->m_ExifInfoWrite, 0, sizeof(CMetadata::EXIFINFOINPLACEWRITE));
		LPBYTE pJpegOutThumbData = NULL;
		int nJpegOutThumbDataSize = Dib.GetMetadata()->m_ExifInfo.ThumbnailSize;
		if (nJpegOutThumbDataSize)
		{
			Dib.LoadJPEG(lpszPathName);
			Dib.GetMetadata()->m_ExifInfoWrite.bThumbnail = MakeEXIFThumbnail(
											&Dib,
											&pJpegOutThumbData,
											&nJpegOutThumbDataSize) ? true : false;
			Dib.GetMetadata()->m_ExifInfo.ThumbnailPointer = pJpegOutThumbData;
			// Leave the Same Size to avoid a continuos shrinking if editing (lossy) many times!
			// Dib.GetMetadata()->m_ExifInfo.ThumbnailSize = nJpegOutThumbDataSize;
			Dib.GetMetadata()->m_ExifInfo.ThumbnailWidth = Dib.GetWidth();
			Dib.GetMetadata()->m_ExifInfo.ThumbnailHeight = Dib.GetHeight();
			Dib.GetMetadata()->m_ExifInfo.ThumbnailCompression = 6; // Jpeg
			Dib.GetMetadata()->m_ExifInfo.ThumbnailPhotometricInterpretation = 6; // Jpeg is YCbCr
			Dib.GetMetadata()->m_ExifInfoWrite.bThumbnailWidth = true;
			Dib.GetMetadata()->m_ExifInfoWrite.bThumbnailHeight = true;
			Dib.GetMetadata()->m_ExifInfoWrite.bThumbnailCompression = true;
			Dib.GetMetadata()->m_ExifInfoWrite.bThumbnailPhotometricInterpretation = true;
		}
		Dib.GetMetadata()->m_ExifInfoWrite.bWidth = true;
		Dib.GetMetadata()->m_ExifInfoWrite.bHeight = true;
		BOOL res = Dib.JPEGWriteEXIFInplace(lpszPathName);
		if (pJpegOutThumbData)
			delete [] pJpegOutThumbData;
		return res;
	}
	else
		return FALSE;
}

BOOL CDib::UpdateExifWidthHeightThumbLossLess(	LPBYTE pJpegData,
												int nJpegDataSize,
												JXFORM_CODE Transform,
												BOOL bTrim,
												BOOL bGrayScale,
												BOOL bShowMessageBoxOnError)
{
	nJpegDataSize;

	// Note:
	// When Loading m_ExifInfo.Width and m_ExifInfo.Height are taken
	// from SOFn markers (not from Exif section!),
	// when writing the m_ExifInfo.Width and m_ExifInfo.Height
	// values are written to the Exif section (not to the SOFn markers!).
	// This because SOFn are more reliable for the image size.

	CDib Dib;
	Dib.SetShowMessageBoxOnError(bShowMessageBoxOnError);
	if (Dib.JPEGLoadMetadata(pJpegData, nJpegDataSize) && Dib.GetMetadata()->m_ExifInfo.bHasExifSubIFD)
	{
		// If the thumbnail is not jpeg 
		// -> do a lossy thumbnail transformation
		if (Dib.GetMetadata()->m_ExifInfo.ThumbnailCompression != 6)
			return UpdateExifWidthHeightThumb(pJpegData, nJpegDataSize, bShowMessageBoxOnError);

		memset(&Dib.GetMetadata()->m_ExifInfoWrite, 0, sizeof(CMetadata::EXIFINFOINPLACEWRITE));
		LPBYTE pJpegOutThumbData = NULL;
		if (Dib.GetMetadata()->m_ExifInfo.ThumbnailPointer && Dib.GetMetadata()->m_ExifInfo.ThumbnailSize)
		{
			// Calc Output Size, it should be the same, but in some cases it is bigger -> Lossy Exif Update
			// (All the Exif Stores are in place -> the size must be the same or smaller!)
			int nJpegOutThumbDataSize = 0;
			if (!LossLessJPEGTrans(Dib.GetMetadata()->m_ExifInfo.ThumbnailPointer, Dib.GetMetadata()->m_ExifInfo.ThumbnailSize,
							 NULL, &nJpegOutThumbDataSize,
							 Transform, bTrim,
							 bGrayScale, FALSE, 
							 "",
							 FALSE, 0, 0, 0, 0, bShowMessageBoxOnError))
				return FALSE;
			if (nJpegOutThumbDataSize > (int)Dib.GetMetadata()->m_ExifInfo.ThumbnailSize)
				return UpdateExifWidthHeightThumb(pJpegData, nJpegDataSize, bShowMessageBoxOnError);
			else
				pJpegOutThumbData = new BYTE[Dib.GetMetadata()->m_ExifInfo.ThumbnailSize];

			// Do Lossless Transformation On Thumbnail
			Dib.GetMetadata()->m_ExifInfoWrite.bThumbnail =
			LossLessJPEGTrans(Dib.GetMetadata()->m_ExifInfo.ThumbnailPointer, Dib.GetMetadata()->m_ExifInfo.ThumbnailSize,
							 pJpegOutThumbData, &nJpegOutThumbDataSize,
							 Transform, bTrim,
							 bGrayScale, FALSE, 
							 "",
							 FALSE, 0, 0, 0, 0, bShowMessageBoxOnError) ? true : false;
			Dib.GetMetadata()->m_ExifInfo.ThumbnailPointer = pJpegOutThumbData;
			// Leave the Same Size to avoid a continuos shrinking if editing (lossy) many times!
			// Dib.GetMetadata()->m_ExifInfo.ThumbnailSize = nJpegOutThumbDataSize;
			CDib ThumbDib;
			ThumbDib.SetShowMessageBoxOnError(bShowMessageBoxOnError);
			ThumbDib.JPEGLoadMetadata(pJpegOutThumbData, nJpegOutThumbDataSize); // It has no Exif, but Width and Height are returned from SOFn
			Dib.GetMetadata()->m_ExifInfo.ThumbnailWidth = ThumbDib.GetExifInfo()->Width;
			Dib.GetMetadata()->m_ExifInfo.ThumbnailHeight = ThumbDib.GetExifInfo()->Height;
			Dib.GetMetadata()->m_ExifInfoWrite.bThumbnailWidth = true;
			Dib.GetMetadata()->m_ExifInfoWrite.bThumbnailHeight = true;
		}
		Dib.GetMetadata()->m_ExifInfoWrite.bWidth = true;
		Dib.GetMetadata()->m_ExifInfoWrite.bHeight = true;
		BOOL res = Dib.JPEGWriteEXIFInplace(pJpegData, nJpegDataSize);
		if (pJpegOutThumbData)
			delete [] pJpegOutThumbData;
		return res;
	}
	else
		return FALSE;
}

BOOL CDib::UpdateExifWidthHeightThumb(LPBYTE pJpegData, int nJpegDataSize, BOOL bShowMessageBoxOnError)
{
	// Note:
	// When Loading m_ExifInfo.Width and m_ExifInfo.Height are taken
	// from SOFn markers (not from Exif section!),
	// when writing the m_ExifInfo.Width and m_ExifInfo.Height
	// values are written to the Exif section (not to the SOFn markers!).
	// This because SOFn are more reliable for the image size.

	CDib Dib;
	Dib.SetShowMessageBoxOnError(bShowMessageBoxOnError);
	if (Dib.JPEGLoadMetadata(pJpegData, nJpegDataSize) && Dib.GetMetadata()->m_ExifInfo.bHasExifSubIFD)
	{
		memset(&Dib.GetMetadata()->m_ExifInfoWrite, 0, sizeof(CMetadata::EXIFINFOINPLACEWRITE));
		LPBYTE pJpegOutThumbData = NULL;
		int nJpegOutThumbDataSize = Dib.GetMetadata()->m_ExifInfo.ThumbnailSize;
		if (nJpegOutThumbDataSize)
		{
			Dib.LoadJPEG(pJpegData, nJpegDataSize);
			Dib.GetMetadata()->m_ExifInfoWrite.bThumbnail = MakeEXIFThumbnail(
											&Dib,
											&pJpegOutThumbData,
											&nJpegOutThumbDataSize) ? true : false;
			Dib.GetMetadata()->m_ExifInfo.ThumbnailPointer = pJpegOutThumbData;
			// Leave the Same Size to avoid a continuos shrinking if editing (lossy) many times!
			// Dib.GetMetadata()->m_ExifInfo.ThumbnailSize = nJpegOutThumbDataSize;
			Dib.GetMetadata()->m_ExifInfo.ThumbnailWidth = Dib.GetWidth();
			Dib.GetMetadata()->m_ExifInfo.ThumbnailHeight = Dib.GetHeight();
			Dib.GetMetadata()->m_ExifInfo.ThumbnailCompression = 6; // Jpeg
			Dib.GetMetadata()->m_ExifInfo.ThumbnailPhotometricInterpretation = 6; // Jpeg is YCbCr
			Dib.GetMetadata()->m_ExifInfoWrite.bThumbnailWidth = true;
			Dib.GetMetadata()->m_ExifInfoWrite.bThumbnailHeight = true;
			Dib.GetMetadata()->m_ExifInfoWrite.bThumbnailCompression = true;
			Dib.GetMetadata()->m_ExifInfoWrite.bThumbnailPhotometricInterpretation = true;
		}
		Dib.GetMetadata()->m_ExifInfoWrite.bWidth = true;
		Dib.GetMetadata()->m_ExifInfoWrite.bHeight = true;
		BOOL res = Dib.JPEGWriteEXIFInplace(pJpegData, nJpegDataSize);
		if (pJpegOutThumbData)
			delete [] pJpegOutThumbData;
		return res;
	}
	else
		return FALSE;
}

BOOL CDib::MakeEXIFThumbnail(CDib* pDib, LPBYTE* ppJpegThumbData, int* pJpegThumbSize)
{
	if (pDib == NULL)
		return FALSE;
	
	// Shrink Thumb To the Right Size (Keep Aspect Ratio)
	double dLongRatio, dShortRatio, dFinalRatio;
	if (pDib->GetWidth() >= pDib->GetHeight())
	{
		 dLongRatio = (double)pDib->GetWidth() / (double)EXIF_THUMB_LONG_EDGE;
		 dShortRatio = (double)pDib->GetHeight() / (double)EXIF_THUMB_SHORT_EDGE;
	}
	else
	{
		dLongRatio = (double)pDib->GetHeight() / (double)EXIF_THUMB_LONG_EDGE;
		dShortRatio = (double)pDib->GetWidth() / (double)EXIF_THUMB_SHORT_EDGE;
	}
	if (dLongRatio > dShortRatio)
		dFinalRatio = dLongRatio;
	else
		dFinalRatio = dShortRatio;
	pDib->StretchBits(	Round(pDib->GetWidth() / dFinalRatio),
						Round(pDib->GetHeight() / dFinalRatio));

	// Add Black Borders
	DWORD dwLeft, dwTop, dwRight, dwBottom;
	if (pDib->GetWidth() >= pDib->GetHeight())
	{
		dwLeft = (EXIF_THUMB_LONG_EDGE - pDib->GetWidth()) / 2;
		dwRight = EXIF_THUMB_LONG_EDGE - pDib->GetWidth() - dwLeft;
		dwTop = (EXIF_THUMB_SHORT_EDGE - pDib->GetHeight()) / 2;
		dwBottom = EXIF_THUMB_SHORT_EDGE - pDib->GetHeight() - dwTop;
	}
	else
	{
		dwTop = (EXIF_THUMB_LONG_EDGE - pDib->GetHeight()) / 2;
		dwBottom = EXIF_THUMB_LONG_EDGE - pDib->GetHeight() - dwTop;
		dwLeft = (EXIF_THUMB_SHORT_EDGE - pDib->GetWidth()) / 2;
		dwRight = EXIF_THUMB_SHORT_EDGE - pDib->GetWidth() - dwLeft;
	}
	pDib->AddBorders(dwLeft, dwTop, dwRight, dwBottom, RGB(0,0,0));

	// Jpeg Thum Creation
	if (ppJpegThumbData)
	{
		int nJpegThumbSize;

		// Calculate the Quality to fit the Thumbnail
		// for the given max size *pJpegThumbSize and create it
		if (pJpegThumbSize && (*pJpegThumbSize > 0))
		{
			int nQuality = EXIF_MAX_THUMB_QUALITY + 5;
			do
			{
				nQuality -= 5; // Start With EXIF_MAX_THUMB_QUALITY down to 5
				pDib->SaveJPEG(	NULL,
								&nJpegThumbSize,
								nQuality,
								pDib->IsGrayscale(),
								_T(""),
								FALSE,
								FALSE,
								NULL);
			}
			while ((nJpegThumbSize > *pJpegThumbSize) && (nQuality > 5));

			// Save Jpeg Thumbnail to Buffer
			if (nJpegThumbSize <= *pJpegThumbSize)
			{
				*ppJpegThumbData = new BYTE[*pJpegThumbSize];
				memset(*ppJpegThumbData, 0, *pJpegThumbSize);
				pDib->SaveJPEG(	*ppJpegThumbData,
								&nJpegThumbSize,
								nQuality,
								pDib->IsGrayscale(),
								_T(""),
								FALSE,
								FALSE,
								NULL);
				*pJpegThumbSize = nJpegThumbSize;
			}
			else
			{
				*ppJpegThumbData = NULL;
				*pJpegThumbSize = 0;
				return FALSE;
			}
		}
		// Create a Jpeg Thumbnail of Quality EXIF_MAX_THUMB_QUALITY,
		// (return size if pJpegThumbSize != NULL)
		else
		{
			// nJpegThumbSize for the given quality
			pDib->SaveJPEG(	NULL,
							&nJpegThumbSize,
							EXIF_MAX_THUMB_QUALITY,
							pDib->IsGrayscale(),
							_T(""),
							FALSE,
							FALSE,
							NULL);
			*ppJpegThumbData = new BYTE[nJpegThumbSize];
			pDib->SaveJPEG(	*ppJpegThumbData,
							&nJpegThumbSize,
							EXIF_MAX_THUMB_QUALITY,
							pDib->IsGrayscale(),
							_T(""),
							FALSE,
							FALSE,
							NULL);
			if (pJpegThumbSize)
				*pJpegThumbSize = nJpegThumbSize;
		}
	}

	return TRUE;
}

BOOL CDib::JPEGGetPixelAlignment(LPCTSTR lpszPathName,
								int* pXAlignment,
								int* pYAlignment,
								BOOL bShowMessageBoxOnError)
{
	struct jpeg_decompress_struct srcinfo;
	struct my_jpeg_error_mgr jsrcerr;
	jsrcerr.szLastLibJpegError[0] = '\0';
	FILE * infile = NULL;

	try
	{
		// Check Pointers
		if (!pXAlignment && !pYAlignment)
			throw (int)JPEG_E_WRONGPARAMETERS;

		// Initialize the JPEG decompression object
		srcinfo.err = jpeg_std_error(&jsrcerr.pub);
		jsrcerr.pub.error_exit = jpeg_error_load;
		jpeg_create_decompress(&srcinfo);

		// Empty Path?
		CString sPathName(lpszPathName);
		if (sPathName.IsEmpty())
			throw (int)JPEG_E_ZEROPATH;

		// Get File Size
		CFile file(lpszPathName, CFile::modeRead | CFile::shareDenyWrite);
		DWORD dwFileSize = (DWORD)file.GetLength();
		file.Close();
		if (dwFileSize == 0)
			throw (int)JPEG_E_FILEEMPTY;

		// Open the input file,
		// use 'R' for random access optimization.
		// With network drives it is faster!
		if ((infile = _tfopen(lpszPathName, _T("rbR"))) == NULL)
			throw (int)JPEG_E_READ;

		// Specify data source for decompression
		jpeg_stdio_src(&srcinfo, infile);

		// Read file header
		if(!jpeg_read_header(&srcinfo, TRUE))
			throw (int)JPEG_E_BADJPEG;

		// Set Return Values
		if (pXAlignment)
			*pXAlignment = srcinfo.max_h_samp_factor * DCTSIZE;
		if (pYAlignment)
			*pYAlignment = srcinfo.max_v_samp_factor * DCTSIZE;

		// Clean-up
		jpeg_destroy_decompress(&srcinfo);

		// Close files
		fclose(infile);

		return TRUE;
	}
	catch (CFileException* e)
	{
		TCHAR szCause[255];
		CString str(_T("GetJPEGPixelAlignment: "));
		e->GetErrorMessage(szCause, 255);
		str += szCause;
		str += _T("\n");
		TRACE(str);
		if (bShowMessageBoxOnError)
			::AfxMessageBox(str, MB_ICONSTOP);
		e->Delete();
		return FALSE;
	}
	catch (int error_code)
	{
		jpeg_destroy_decompress(&srcinfo);
		if (infile)
			fclose(infile);

		CString str;
		str.Format(_T("GetJPEGPixelAlignment(%s):\n"), lpszPathName);
		switch (error_code)
		{
			case JPEG_E_WRONGPARAMETERS :	str += _T("The Function Parameters Are Wrong!\n");
			break;
			case JPEG_E_ZEROPATH :			str += _T("The file name is zero\n");
			break;
			case JPEG_E_LIBJPEG_LOAD :		str += CString(jsrcerr.szLastLibJpegError) + _T("\n");
			break;
			case JPEG_E_BADJPEG :			str += _T("Corrupted or unsupported JPEG\n");
			break;
			case JPEG_E_READ :				str += _T("Couldn't read JPEG file\n");
			break;
			case JPEG_E_FILEEMPTY :			str += _T("File is empty\n");
			break;
			default:						str += _T("Unspecified error\n");
			break;
		}
		
		TRACE(str);
		if (bShowMessageBoxOnError)
			::AfxMessageBox(str, MB_ICONSTOP);

		return FALSE;
	}
}

BOOL CDib::JPEGLoadMetadata(LPCTSTR lpszPathName)
{
	if (!IsJPEG(lpszPathName))
		return FALSE;

	// Open File
	DWORD dwDesiredAccess = GENERIC_READ;
	HANDLE hFile = ::CreateFile(lpszPathName, dwDesiredAccess,
								FILE_SHARE_READ, NULL, OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	// Get the size of the file
	DWORD dwFileSizeHigh = 0;
	m_FileInfo.m_dwFileSize = ::GetFileSize(hFile, &dwFileSizeHigh);
	if (m_FileInfo.m_dwFileSize == 0xFFFFFFFF || m_FileInfo.m_dwFileSize == 0)
	{
		::CloseHandle(hFile);
		return FALSE;
	}
	
	// Create the file mapping object
	DWORD flProtect = PAGE_WRITECOPY;
	HANDLE hMMapping = ::CreateFileMapping(hFile, NULL, flProtect, 0, 0, NULL);
	if (hMMapping == NULL)
	{
		::CloseHandle(hFile);
		return FALSE;
	}

	// Map the view
	DWORD dwDesiredMapAccess = FILE_MAP_COPY;
	LPBYTE pMMFile = (LPBYTE)::MapViewOfFile(hMMapping, dwDesiredMapAccess, 0, 0, 0);
	if (pMMFile == NULL)
	{
		::CloseHandle(hMMapping);
		::CloseHandle(hFile);
		return FALSE;
	}

	// Process It
	BOOL res = JPEGLoadMetadata(pMMFile, ::GetFileSize(hFile, NULL));

	// Clean-Up
	if (pMMFile != NULL)
	{
		::FlushViewOfFile(pMMFile, 0);
		::UnmapViewOfFile(pMMFile);
		pMMFile = NULL;
	}
	if (hMMapping != NULL)
	{
		::CloseHandle(hMMapping);
		hMMapping = NULL;
	}
	::CloseHandle(hFile);

	return res;
}

BOOL CDib::JPEGLoadMetadata(LPBYTE pJpegData, DWORD dwSize)
{
	if (pJpegData == NULL)
		return FALSE;

	// Parse Exif Data
	bool res = GetMetadata()->ParseProcessJPEG(pJpegData, dwSize, false);

	if (res)
	{
		// Load Thumbnail
		LoadEXIFThumbnail();
	}

	return (res ? TRUE : FALSE);
}

BOOL CDib::LoadEXIFThumbnail()
{
	if (GetMetadata()->m_ExifInfo.bHasExifSubIFD	&&
		GetMetadata()->m_ExifInfo.ThumbnailPointer	&&
		GetMetadata()->m_ExifInfo.ThumbnailSize)
	{
		BOOL res = FALSE;
		if (GetMetadata()->m_ExifInfo.ThumbnailCompression == 6) // Jpeg
		{
			if (GetMetadata()->m_ExifInfo.ThumbnailSize >= 2			&&
				GetMetadata()->m_ExifInfo.ThumbnailPointer[0] == 0xFF	&&
				GetMetadata()->m_ExifInfo.ThumbnailPointer[1] == 0xD8)
			{
				if (m_pThumbnailDib == NULL)
					m_pThumbnailDib = (CDib*)new CDib;
				res = m_pThumbnailDib->LoadJPEG(GetMetadata()->m_ExifInfo.ThumbnailPointer,
												GetMetadata()->m_ExifInfo.ThumbnailSize);
			}
		}
		else if (GetMetadata()->m_ExifInfo.ThumbnailCompression == 1) // TIFF
		{
			if (GetMetadata()->m_ExifInfo.ThumbnailPhotometricInterpretation == 2) // RGB Format
			{
				// Kodak DC-210/220/260 And Sony D700 use this format

				if (m_pThumbnailDib == NULL)
					m_pThumbnailDib = (CDib*)new CDib;
				else
					m_pThumbnailDib->Free();
			
				// Header
				BITMAPINFO Bmi;
				memset(&Bmi, 0, sizeof(BITMAPINFO));
				DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES(GetMetadata()->m_ExifInfo.ThumbnailWidth * 24);
				Bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
				Bmi.bmiHeader.biWidth = (DWORD)GetMetadata()->m_ExifInfo.ThumbnailWidth;
				Bmi.bmiHeader.biHeight = (DWORD)GetMetadata()->m_ExifInfo.ThumbnailHeight;
				Bmi.bmiHeader.biPlanes = 1;
				Bmi.bmiHeader.biBitCount = 24;
				Bmi.bmiHeader.biCompression = BI_RGB;    
				Bmi.bmiHeader.biSizeImage = uiDIBScanLineSize * GetMetadata()->m_ExifInfo.ThumbnailHeight;
				Bmi.bmiHeader.biClrUsed = 0;
				Bmi.bmiHeader.biClrImportant = 0;
				m_pThumbnailDib->SetBMI(&Bmi);

				// Pixels
				m_pThumbnailDib->m_pBits = (LPBYTE)BIGALLOC(uiDIBScanLineSize * GetMetadata()->m_ExifInfo.ThumbnailHeight);
				if (m_pThumbnailDib->m_pBits)
				{
					// Start Writing To Last Line (TIFF Bitmap is top-down, windows is bottom-up)
					LPBYTE p = m_pThumbnailDib->m_pBits + (uiDIBScanLineSize * (GetMetadata()->m_ExifInfo.ThumbnailHeight - 1));
					for (int SrcLine = 0 ; SrcLine < GetMetadata()->m_ExifInfo.ThumbnailHeight ; SrcLine++)
					{
						for (int pix = 0 ; pix < GetMetadata()->m_ExifInfo.ThumbnailWidth ; pix++)
						{
							// Invert Ordering: TIFF uses RGB ordering, windows uses BGR
							unsigned char R,G,B;
							R = GetMetadata()->m_ExifInfo.ThumbnailPointer[3*(SrcLine*GetMetadata()->m_ExifInfo.ThumbnailWidth + pix)];
							G = GetMetadata()->m_ExifInfo.ThumbnailPointer[3*(SrcLine*GetMetadata()->m_ExifInfo.ThumbnailWidth + pix) + 1];
							B = GetMetadata()->m_ExifInfo.ThumbnailPointer[3*(SrcLine*GetMetadata()->m_ExifInfo.ThumbnailWidth + pix) + 2];
							p[3*pix] = B;
							p[3*pix + 1] = G;
							p[3*pix + 2] = R;
						}
						p -= uiDIBScanLineSize;
					}
					res = TRUE;
				}
				else
				{
					delete m_pThumbnailDib;
					m_pThumbnailDib = NULL;
				}
			}
			else if (GetMetadata()->m_ExifInfo.ThumbnailPhotometricInterpretation == 6) // YCbCr Format
			{
				/* Ricoh RDC4200/4300, Fuji DS-7/300 and DX-5/7/9 use this format

				YCbCrSubsampling(0x0212) has values of '2,1', PlanarConfiguration(0x011c) has a value '1'.
				The data align of this image is:
				Y(0,0),Y(1,0),Cb(0,0),Cr(0,0), Y(2,0),Y(3,0),Cb(2,0),Cr(2,0), Y(4,0),Y(5,0),Cb(4,0),Cr(4,0). . . .
				The numerics in parenthesis are pixel coordinates.

				For Example DX series' YCbCrCoefficients(0x0211) have values '0.299/0.587/0.114',
				ReferenceBlackWhite(0x0214) has values '0,255,128,255,128,255'.
				Therefore to convert from Y/Cb/Cr to RGB use:
				B=(Cb-128)*(2-0.114*2)+Y(0,0)

				R=(Cr-128)*(2-0.299*2)+Y(0,0)

				G=(Y(0,0)-0.114*B(0,0)-0.299*R(0,0))/0.587

				Horizontal subsampling has a value of '2', you can calculate B(1,0)/R(1,0)/G(1,0)
				by using the Y(1,0) and Cr(0,0)/Cb(0,0).
				Repeat this conversion by value of ImageWidth(0x0100) and ImageLength(0x0101).
				*/
				/*	With Coeff.:	k0=0.299, k1=0.587, K2=0.114
									k0+k1+k2=1
					and All Values From 0..255:
					Y  = k0*R + k1*G + k2*B;
					Cb = (k0+k1)*B - (k1*G + k0*R);
					Cr = (k1+k2)*R - (k1*G + k2*B);
					R = Y + Cr;
					B = Y + Cb;
					G = (Y - k0*R - k2*B) / k1;
				*/

				if (m_pThumbnailDib == NULL)
					m_pThumbnailDib = (CDib*)new CDib;
				else
					m_pThumbnailDib->Free();
			
				// Header
				double Y0,Y1,Cb,Cr;
				int R0,G0,B0,R1,G1,B1;
				BITMAPINFO Bmi;
				memset(&Bmi, 0, sizeof(BITMAPINFO));
				DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES(GetMetadata()->m_ExifInfo.ThumbnailWidth * 24);
				Bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
				Bmi.bmiHeader.biWidth = (DWORD)GetMetadata()->m_ExifInfo.ThumbnailWidth;
				Bmi.bmiHeader.biHeight = (DWORD)GetMetadata()->m_ExifInfo.ThumbnailHeight;
				Bmi.bmiHeader.biPlanes = 1;
				Bmi.bmiHeader.biBitCount = 24;
				Bmi.bmiHeader.biCompression = BI_RGB;    
				Bmi.bmiHeader.biSizeImage = uiDIBScanLineSize * GetMetadata()->m_ExifInfo.ThumbnailHeight;
				Bmi.bmiHeader.biClrUsed = 0;
				Bmi.bmiHeader.biClrImportant = 0;
				m_pThumbnailDib->SetBMI(&Bmi);

				// Pixels
				m_pThumbnailDib->m_pBits = (LPBYTE)BIGALLOC(uiDIBScanLineSize * GetMetadata()->m_ExifInfo.ThumbnailHeight);
				if (m_pThumbnailDib->m_pBits)
				{
					// Start Writing To Last Line (TIFF Bitmap is top-down, windows is bottom-up)
					LPBYTE p = m_pThumbnailDib->m_pBits + (uiDIBScanLineSize * (GetMetadata()->m_ExifInfo.ThumbnailHeight - 1));
					
					// 2,1 = YCbCr4:2:2
					if (GetMetadata()->m_ExifInfo.ThumbnailYCbCrSubSampling[0] == 2 &&
						GetMetadata()->m_ExifInfo.ThumbnailYCbCrSubSampling[1] == 1)
					{
						for (int SrcLine = 0 ; SrcLine < GetMetadata()->m_ExifInfo.ThumbnailHeight ; SrcLine++)
						{
							for (int pix = 0 ; pix < GetMetadata()->m_ExifInfo.ThumbnailWidth / 2 ; pix++)
							{
								// Get Y0Y1CbCr
								Y0 = GetMetadata()->m_ExifInfo.ThumbnailPointer[4*(SrcLine*GetMetadata()->m_ExifInfo.ThumbnailWidth/2 + pix)];
								Y1 = GetMetadata()->m_ExifInfo.ThumbnailPointer[4*(SrcLine*GetMetadata()->m_ExifInfo.ThumbnailWidth/2 + pix) + 1];
								Cb = GetMetadata()->m_ExifInfo.ThumbnailPointer[4*(SrcLine*GetMetadata()->m_ExifInfo.ThumbnailWidth/2 + pix) + 2];
								Cr = GetMetadata()->m_ExifInfo.ThumbnailPointer[4*(SrcLine*GetMetadata()->m_ExifInfo.ThumbnailWidth/2 + pix) + 3];
								
								// Convert All to The Range: 0..255
								Y0 = Round((double)(Y0 - GetMetadata()->m_ExifInfo.dThumbnailRefBW[0]) *
									255.0 / (double)(GetMetadata()->m_ExifInfo.dThumbnailRefBW[1] - GetMetadata()->m_ExifInfo.dThumbnailRefBW[0]));
								Y1 = Round((double)(Y1 - GetMetadata()->m_ExifInfo.dThumbnailRefBW[0]) *
									255.0 / (double)(GetMetadata()->m_ExifInfo.dThumbnailRefBW[1] - GetMetadata()->m_ExifInfo.dThumbnailRefBW[0]));
								Cb = Round((double)(Cb - GetMetadata()->m_ExifInfo.dThumbnailRefBW[2])	*
									255.0 / (double)(GetMetadata()->m_ExifInfo.dThumbnailRefBW[3] - GetMetadata()->m_ExifInfo.dThumbnailRefBW[2]));
								Cr = Round((double)(Cr - GetMetadata()->m_ExifInfo.dThumbnailRefBW[4])	*
									255.0 / (double)(GetMetadata()->m_ExifInfo.dThumbnailRefBW[5] - GetMetadata()->m_ExifInfo.dThumbnailRefBW[4]));
								
								// Convert From YCbCr To RGB
								R0 = Round(Y0 + Cr);
								B0 = Round(Y0 + Cb);
								G0 = Round((Y0 -
										GetMetadata()->m_ExifInfo.dThumbnailYCbCrCoeff[0]*R0 -
										GetMetadata()->m_ExifInfo.dThumbnailYCbCrCoeff[2]*B0) /
										GetMetadata()->m_ExifInfo.dThumbnailYCbCrCoeff[1]);
								R1 = Round(Y1 + Cr);
								B1 = Round(Y1 + Cb);
								G1 = Round((Y1 -
										GetMetadata()->m_ExifInfo.dThumbnailYCbCrCoeff[0]*R1 -
										GetMetadata()->m_ExifInfo.dThumbnailYCbCrCoeff[2]*B1) /
										GetMetadata()->m_ExifInfo.dThumbnailYCbCrCoeff[1]);			

								// Clip
								if (B0 < 0) B0 = 0;
								else if (B0 > 255) B0 = 255;
								if (G0 < 0) G0 = 0;
								else if (G0 > 255) G0 = 255;
								if (R0 < 0) R0 = 0;
								else if (R0 > 255) R0 = 255;
								if (B1 < 0) B1 = 0;
								else if (B1 > 255) B1 = 255;
								if (G1 < 0) G1 = 0;
								else if (G1 > 255) G1 = 255;
								if (R1 < 0) R1 = 0;
								else if (R1 > 255) R1 = 255;

								// Invert Ordering: TIFF uses RGB ordering, windows uses BGR
								p[6*pix] = B0;
								p[6*pix + 1] = G0;
								p[6*pix + 2] = R0;
								p[6*pix + 3] = B1;
								p[6*pix + 4] = G1;
								p[6*pix + 5] = R1;
							}
							p -= uiDIBScanLineSize;
						}
						res = TRUE;
					}
					// 2,2 = YCbCr4:2:0
					else if (GetMetadata()->m_ExifInfo.ThumbnailYCbCrSubSampling[0] == 2 &&
							GetMetadata()->m_ExifInfo.ThumbnailYCbCrSubSampling[1] == 2)
					{
						// TODO
						//res = TRUE;
					}
				}
				else
				{
					delete m_pThumbnailDib;
					m_pThumbnailDib = NULL;
				}
			}
		}

		return res;
	}
	else
	{
		DeleteThumbnailDib();
		return FALSE;
	}
}

BOOL CDib::JPEGWriteSection(int SectionType,						// Like: M_JFIF, M_EXIF_XMP or M_COM
							const unsigned char* pSectionSubType,	// NULL if any type, "Exif\0\0" to remove EXIF only
							int nSectionSubTypeSize,				// 0 if any type, otherwise size to check for remove
							LPCTSTR szFileName,
							LPCTSTR szTempDir,
							int nSize,
							LPBYTE pData,
							BOOL bShowMessageBoxOnError)
{
	if (IsJPEG(szFileName))
	{
		// Temporary File
		CString sTempFileName = ::MakeTempFileName(szTempDir, szFileName);

		// Replace the given section
		if (CDib::JPEGReplaceSection(	SectionType,
										pSectionSubType,
										nSectionSubTypeSize,
										nSize,
										pData,
										szFileName,
										sTempFileName,
										bShowMessageBoxOnError))
		{
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
					CString str(_T("The file is read only\n"));
					TRACE(str);
					if (bShowMessageBoxOnError)
						::AfxMessageBox(str, MB_ICONSTOP);
				}
				else
					ShowError(e->m_lOsError, bShowMessageBoxOnError, _T("JPEGWriteSection()"));

				e->Delete();
				return FALSE;
			}
			return TRUE;
		}
		else
			return FALSE;
	}
	else
		return FALSE;
}

BOOL CDib::AddEXIFThumbnail(LPCTSTR lpszInPathName,
							LPCTSTR lpszOutPathName,
							BOOL bShowMessageBoxOnError)
{
	bool bDoWrite;
	HANDLE hInFile = NULL;
	HANDLE hInMMapping = NULL;
	LPBYTE pInMMFile = NULL;
	HANDLE hOutFile = NULL;
	LPBYTE pJpegThumbData = NULL;
	LPBYTE Section = NULL;
	int nJpegThumbDataSize;
	DWORD dwImageWidth, dwImageHeight;
	DWORD dwImageLongEdge, dwImageShortEdge;
	DWORD dwNumberOfBytesWritten;
	CDib Dib;
	Dib.SetShowMessageBoxOnError(bShowMessageBoxOnError);

	try
	{
		// Check filenames
		if (!IsJPEG(lpszInPathName) || !IsJPEG(lpszOutPathName))
			throw 0;

		// Is Out File Read-Only?
		DWORD dwAttrib = ::GetFileAttributes(lpszOutPathName);
		if ((dwAttrib != 0xFFFFFFFF) && (dwAttrib & FILE_ATTRIBUTE_READONLY))
		{
			CString str;
#ifdef _DEBUG
			str.Format(_T("AddEXIFThumbnail(%s):\n"), lpszOutPathName);
#endif
			str += _T("The file is read only\n");
			TRACE(str);
			if (bShowMessageBoxOnError)
				::AfxMessageBox(str, MB_ICONSTOP);
			throw 0;
		}

		// Load Jpeg to Create a Thumbnail From It
		Dib.LoadJPEG(lpszInPathName, 1, FALSE, TRUE); // Only Header For Size
		dwImageWidth = Dib.GetWidth();
		dwImageHeight = Dib.GetHeight();
		dwImageLongEdge = MAX(dwImageWidth, dwImageHeight);
		dwImageShortEdge = MIN(dwImageWidth, dwImageHeight);
		int i;
		for (i = 0 ; i < 4 ; i++)
		{
			if ((dwImageLongEdge >> i) <= EXIF_THUMB_LONG_EDGE ||
				(dwImageShortEdge >> i) <= EXIF_THUMB_SHORT_EDGE)
				break;
		}
		if (i > 0)
			i--;
		Dib.LoadJPEG(lpszInPathName, 1 << i, FALSE, FALSE);

		// Open In File for Reading
		bDoWrite = false;
		DWORD dwInDesiredAccess = bDoWrite ? (GENERIC_READ | GENERIC_WRITE) : GENERIC_READ;
		hInFile = ::CreateFile(	lpszInPathName, dwInDesiredAccess,
								FILE_SHARE_READ, NULL, OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL, NULL);
		if (hInFile == INVALID_HANDLE_VALUE)
			throw 0;
		
		// Create the file mapping object
		DWORD flInProtect = (bDoWrite) ? PAGE_READWRITE : PAGE_WRITECOPY;
		hInMMapping = ::CreateFileMapping(hInFile, NULL, flInProtect, 0, 0, NULL);
		if (hInMMapping == NULL)
			throw 0;

		// Map the view
		DWORD dwInDesiredMapAccess = (bDoWrite) ? FILE_MAP_WRITE : FILE_MAP_COPY;
		pInMMFile = (LPBYTE)::MapViewOfFile(hInMMapping, dwInDesiredMapAccess, 0, 0, 0);
		if (pInMMFile == NULL)
			throw 0;

		// Create Out File for Writing
		bDoWrite = true;
		DWORD dwOutDesiredAccess = bDoWrite ? (GENERIC_READ | GENERIC_WRITE) : GENERIC_READ;
		hOutFile = ::CreateFile(lpszOutPathName, dwOutDesiredAccess,
								FILE_SHARE_READ, NULL, CREATE_ALWAYS,
								FILE_ATTRIBUTE_NORMAL, NULL);
		if (hOutFile == INVALID_HANDLE_VALUE)
			throw 0;
		
		// Load Metadata
		if (!Dib.JPEGLoadMetadata(pInMMFile, ::GetFileSize(hInFile, NULL)))
			throw 0;

		// Create the Jpeg Thumbnail
		nJpegThumbDataSize = EXIF_ADDTHUMB_MAXSIZE;
		if (!MakeEXIFThumbnail(	&Dib,
								&pJpegThumbData,
								&nJpegThumbDataSize))
			throw 0;

		// If No EXIF -> Add Simple EXIF Section with Thumbnail
		if (!Dib.GetMetadata()->m_ExifInfo.bHasExifSubIFD)
		{
			int section;
			int startsection = 0;

			// Check Whether JFIF Section Available
			for (section = 0 ; section < Dib.GetMetadata()->GetSectionCount() ; section++)
			{
				CMetadata::JPEGSECTION* p = Dib.GetMetadata()->GetSection(section);
				if (p->Type == M_JFIF)
					startsection = section;
			}

			// Add EXIF Section
			for (section = 0 ; section < Dib.GetMetadata()->GetSectionCount() ; section++)
			{
				const unsigned char FF = 0xFF;
				unsigned char marker;
				CMetadata::JPEGSECTION* p = Dib.GetMetadata()->GetSection(section);
				if (!p)
					throw 0;
				::WriteFile(hOutFile, &FF, 1, &dwNumberOfBytesWritten, NULL);
				marker = p->Type;
				::WriteFile(hOutFile, &marker, 1, &dwNumberOfBytesWritten, NULL);
				if (p->Data && p->Size > 0)
					::WriteFile(hOutFile, p->Data, p->Size, &dwNumberOfBytesWritten, NULL);

				// Put Exif either after JFIF or after the SOI Marker
				if (section == startsection)
				{
					Section = new BYTE[65537];
					if (!Section)
						throw 0;
					Section[0] = 0xFF;
					Section[1] = M_EXIF_XMP;
					Section[2] = 0;	// High Order Byte of 16 bits Size
					Section[3] = 0; // Low Order Byte of 16 bits Size
			
					// Test
					/*
					strcpy(Dib.GetExifInfo()->ImageDescription, "Image Description Line0\nImage Description Line1");
					wcscpy(Dib.GetExifInfo()->UserComment, L"User Comment Line0\rUser Comment Line1");
					strcpy(Dib.GetExifInfo()->CameraMake, "Contaware.com");
					strcpy(Dib.GetExifInfo()->CameraModel, "SX3");
					Dib.GetExifInfo()->Orientation = 2;
					Dib.GetExifInfo()->Xresolution = 300.0f / 2.54f;
					Dib.GetExifInfo()->Yresolution = 200.0f / 2.54f;
					Dib.GetExifInfo()->ResolutionUnit = 0.393701f;
					strcpy(Dib.GetExifInfo()->Software, "FreeVimager\r\nCool Soft!\nFor everybody!");
					strcpy(Dib.GetExifInfo()->Artist, "OP");
					strcpy(Dib.GetExifInfo()->CopyrightPhotographer, "Photographer");
					strcpy(Dib.GetExifInfo()->CopyrightEditor, "Editor");

					Dib.GetExifInfo()->ExposureTime = 0.12345f;
					Dib.GetExifInfo()->ApertureFNumber = 22.0f;
					strcpy(Dib.GetExifInfo()->DateTime, "2001:09:11 08:30:00");
					Dib.GetExifInfo()->Flash = 0x5F;
					Dib.GetExifInfo()->ISOequivalent = 400;
					Dib.GetExifInfo()->FocalLength = 43.1f;
					Dib.GetExifInfo()->Distance = 10.3f;
					Dib.GetExifInfo()->ExposureBias = 51.01f;
					Dib.GetExifInfo()->DigitalZoomRatio = 2.5f;
					Dib.GetExifInfo()->FocalLength35mmEquiv = 200;
					Dib.GetExifInfo()->DistanceRange = 1;
					Dib.GetExifInfo()->WhiteBalance = 0;
					Dib.GetExifInfo()->MeteringMode = 3;
					Dib.GetExifInfo()->ExposureProgram = 1;
					Dib.GetExifInfo()->ExposureMode = 1;
					Dib.GetExifInfo()->LightSource = 2;
					Dib.GetExifInfo()->CompressionLevel = 4.0f;
					Dib.GetExifInfo()->FocalplaneXRes = 2000.0f;
					Dib.GetExifInfo()->FocalplaneYRes = 1500.0f;
					Dib.GetExifInfo()->FocalplaneUnits = 0.001f;
					Dib.GetExifInfo()->Brightness = 5.0f;
					Dib.GetExifInfo()->AmbientTemperature = 25.0f;
					Dib.GetExifInfo()->Humidity = 80.0f;
					Dib.GetExifInfo()->Pressure = 985.0f;
					Dib.GetExifInfo()->WaterDepth = 10.0f;
					*/

					// Create Simple Exif Section with Thumbnail
					Dib.GetExifInfo()->Orientation = 1;	// Orientation Normal
					strcpy(Dib.GetExifInfo()->Software, APPNAME_NOEXT_ASCII);
					int Size = Dib.GetMetadata()->MakeExifSection(	Section + 4,					// Exif Start
																	65533,							// Max. Allowed Size
																	dwImageWidth,					// Main Image Width
																	dwImageHeight,					// Main Image Height
																	pJpegThumbData,					// Jpeg Thumbnail Pointer
																	nJpegThumbDataSize);			// Jpeg Thumbnail Size
					if (Size <= 0)
						throw 0;
					Size += 2;	// Size Includes Size, excludes Marker

					// Little Endian To Big Endian
					int lh = (Size>>8) & 0xFF;
					int ll = Size & 0xFF;
					Section[2] = lh;
					Section[3] = ll;
					Size += 2;	// Total Section Size

					// Write Section to the Out File
					::WriteFile(hOutFile, Section, Size, &dwNumberOfBytesWritten, NULL);
				}
			}

			// Write the Remaining of Jpeg File
			if (Dib.GetMetadata()->GetAfterSOSData())
				::WriteFile(hOutFile,
							Dib.GetMetadata()->GetAfterSOSData(),
							Dib.GetMetadata()->GetAfterSOSSize(),
							&dwNumberOfBytesWritten,
							NULL);
		}
		// Has EXIF, but no Thumbnail -> Append Thumbnail
		else if (Dib.GetExifInfo()->ThumbnailPointer == NULL &&
				Dib.GetExifInfo()->ThumbnailSize == 0)
		{
			// Append EXIF Thumbnail
			for (int section = 0 ; section < Dib.GetMetadata()->GetSectionCount() ; section++)
			{
				const unsigned char FF = 0xFF;
				unsigned char marker;
				CMetadata::JPEGSECTION* p = Dib.GetMetadata()->GetSection(section);
				if (!p)
					throw 0;

				// Append EXIF Thumbnail
				if (p->Type == M_EXIF_XMP)
				{
					if (Section)
						delete [] Section;
					Section = new BYTE[65537];
					if (!Section)
						throw 0;
					Section[0] = 0xFF;
					Section[1] = M_EXIF_XMP;

					// Get Section
					memcpy(&Section[2], p->Data, p->Size);
			
					// Append
					int Size = Dib.GetMetadata()->AppendExifThumbnail(	Section + 4,		// Exif Start
																		p->Size - 2,		// Current Size
																		65533,				// Max. Allowed Size
																		pJpegThumbData,		// Jpeg Thumbnail Pointer
																		nJpegThumbDataSize);// Jpeg Thumbnail Size
					if (Size <= ((int)p->Size - 2)) // Found a APP1 Section which is not a EXIF Section!
					{
						::WriteFile(hOutFile, &FF, 1, &dwNumberOfBytesWritten, NULL);
						marker = p->Type;
						::WriteFile(hOutFile, &marker, 1, &dwNumberOfBytesWritten, NULL);
						if (p->Data && p->Size > 0)
							::WriteFile(hOutFile, p->Data, p->Size, &dwNumberOfBytesWritten, NULL);
					}
					else
					{
						Size += 2;	// Size Includes Size, excludes Marker

						// Little Endian To Big Endian
						int lh = (Size>>8) & 0xFF;
						int ll = Size & 0xFF;
						Section[2] = lh;
						Section[3] = ll;
						Size += 2;	// Total Section Size

						// Write Section to the Out File
						::WriteFile(hOutFile, Section, Size, &dwNumberOfBytesWritten, NULL);
					}
				}
				else
				{
					::WriteFile(hOutFile, &FF, 1, &dwNumberOfBytesWritten, NULL);
					marker = p->Type;
					::WriteFile(hOutFile, &marker, 1, &dwNumberOfBytesWritten, NULL);
					if (p->Data && p->Size > 0)
						::WriteFile(hOutFile, p->Data, p->Size, &dwNumberOfBytesWritten, NULL);
				}
			}

			// Write the Remaining of Jpeg File
			if (Dib.GetMetadata()->GetAfterSOSData())
				::WriteFile(hOutFile,
							Dib.GetMetadata()->GetAfterSOSData(),
							Dib.GetMetadata()->GetAfterSOSSize(),
							&dwNumberOfBytesWritten,
							NULL);
		}
		else
			throw 0;

		// Clean-Up
		if (pJpegThumbData)
			delete [] pJpegThumbData;
		if (Section)
			delete [] Section;
		::CloseHandle(hOutFile);
		if (pInMMFile != NULL)
		{
			::FlushViewOfFile(pInMMFile, 0);
			::UnmapViewOfFile(pInMMFile);
			pInMMFile = NULL;
		}
		if (hInMMapping != NULL)
		{
			::CloseHandle(hInMMapping);
			hInMMapping = NULL;
		}
		::CloseHandle(hInFile);

		return TRUE;
	}
	catch (int)
	{
		// Clean-Up
		if (pJpegThumbData)
			delete [] pJpegThumbData;
		if (Section)
			delete [] Section;
		if (hOutFile)
			::CloseHandle(hOutFile);
		::DeleteFile(lpszOutPathName);
		if (pInMMFile != NULL)
		{
			::FlushViewOfFile(pInMMFile, 0);
			::UnmapViewOfFile(pInMMFile);
		}
		if (hInMMapping != NULL)
			::CloseHandle(hInMMapping);
		if (hInFile)
			::CloseHandle(hInFile);

		return FALSE;
	}
}

BOOL CDib::RemoveEXIFThumbnail(	LPCTSTR lpszInPathName,
								LPCTSTR lpszOutPathName,
								BOOL bShowMessageBoxOnError)
{
	bool bDoWrite;
	HANDLE hInFile = NULL;
	HANDLE hInMMapping = NULL;
	LPBYTE pInMMFile = NULL;
	HANDLE hOutFile = NULL;
	LPBYTE Section = NULL;
	DWORD dwNumberOfBytesWritten;
	CDib Dib;
	Dib.SetShowMessageBoxOnError(bShowMessageBoxOnError);

	try
	{
		// Check filenames
		if (!IsJPEG(lpszInPathName) || !IsJPEG(lpszOutPathName))
			throw 0;

		// Is Out File Read-Only?
		DWORD dwAttrib = ::GetFileAttributes(lpszOutPathName);
		if ((dwAttrib != 0xFFFFFFFF) && (dwAttrib & FILE_ATTRIBUTE_READONLY))
		{
			CString str;
#ifdef _DEBUG
			str.Format(_T("RemoveEXIFThumbnail(%s):\n"), lpszOutPathName);
#endif
			str += _T("The file is read only\n");
			TRACE(str);
			if (bShowMessageBoxOnError)
				::AfxMessageBox(str, MB_ICONSTOP);
			throw 0;
		}

		// Open In File for Reading
		bDoWrite = false;
		DWORD dwInDesiredAccess = bDoWrite ? (GENERIC_READ | GENERIC_WRITE) : GENERIC_READ;
		hInFile = ::CreateFile(	lpszInPathName, dwInDesiredAccess,
								FILE_SHARE_READ, NULL, OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL, NULL);
		if (hInFile == INVALID_HANDLE_VALUE)
			throw 0;
		
		// Create the file mapping object
		DWORD flInProtect = (bDoWrite) ? PAGE_READWRITE : PAGE_WRITECOPY;
		hInMMapping = ::CreateFileMapping(hInFile, NULL, flInProtect, 0, 0, NULL);
		if (hInMMapping == NULL)
			throw 0;

		// Map the view
		DWORD dwInDesiredMapAccess = (bDoWrite) ? FILE_MAP_WRITE : FILE_MAP_COPY;
		pInMMFile = (LPBYTE)::MapViewOfFile(hInMMapping, dwInDesiredMapAccess, 0, 0, 0);
		if (pInMMFile == NULL)
			throw 0;

		// Create Out File for Writing
		bDoWrite = true;
		DWORD dwOutDesiredAccess = bDoWrite ? (GENERIC_READ | GENERIC_WRITE) : GENERIC_READ;
		hOutFile = ::CreateFile(lpszOutPathName, dwOutDesiredAccess,
								FILE_SHARE_READ, NULL, CREATE_ALWAYS,
								FILE_ATTRIBUTE_NORMAL, NULL);
		if (hOutFile == INVALID_HANDLE_VALUE)
			throw 0;
		
		// Load Metadata
		if (!Dib.JPEGLoadMetadata(pInMMFile, ::GetFileSize(hInFile, NULL)))
			throw 0;

		// Make Sure That Thumbnail Position is Last!
		if (Dib.GetExifInfo()->ThumbnailOffset < Dib.GetMetadata()->GetMaxOffset())
			throw 0;

		// Has EXIF and Thumbnail -> Remove Thumbnail
		if (Dib.GetMetadata()->m_ExifInfo.bHasExifSubIFD	&&
			Dib.GetExifInfo()->ThumbnailPointer != NULL		&&
			Dib.GetExifInfo()->ThumbnailSize > 0)
		{
			// Remove EXIF Thumbnail
			for (int section = 0 ; section < Dib.GetMetadata()->GetSectionCount() ; section++)
			{
				const unsigned char FF = 0xFF;
				unsigned char marker;
				CMetadata::JPEGSECTION* p = Dib.GetMetadata()->GetSection(section);
				if (!p)
					throw 0;

				// Remove EXIF Thumbnail
				if (p->Type == M_EXIF_XMP)
				{
					if (Section)
						delete [] Section;
					Section = new BYTE[2 + p->Size];
					if (!Section)
						throw 0;
					Section[0] = 0xFF;
					Section[1] = M_EXIF_XMP;

					// Get Section
					memcpy(&Section[2], p->Data, p->Size);
			
					// Remove
					int Size = Dib.GetMetadata()->RemoveExifThumbnail(	Section + 4,	// Exif Start
																		p->Size - 2);	// Current Size
					if (Size <= 0) // Found a APP1 Section which is not a EXIF Section!
					{
						::WriteFile(hOutFile, &FF, 1, &dwNumberOfBytesWritten, NULL);
						marker = p->Type;
						::WriteFile(hOutFile, &marker, 1, &dwNumberOfBytesWritten, NULL);
						if (p->Data && p->Size > 0)
							::WriteFile(hOutFile, p->Data, p->Size, &dwNumberOfBytesWritten, NULL);
					}
					else
					{
						Size += 2;	// Size Includes Size, excludes Marker

						// Little Endian To Big Endian
						int lh = (Size>>8) & 0xFF;
						int ll = Size & 0xFF;
						Section[2] = lh;
						Section[3] = ll;
						Size += 2;	// Total Section Size

						// Write Section to the Out File
						::WriteFile(hOutFile, Section, Size, &dwNumberOfBytesWritten, NULL);
					}
				}
				else
				{
					::WriteFile(hOutFile, &FF, 1, &dwNumberOfBytesWritten, NULL);
					marker = p->Type;
					::WriteFile(hOutFile, &marker, 1, &dwNumberOfBytesWritten, NULL);
					if (p->Data && p->Size > 0)
						::WriteFile(hOutFile, p->Data, p->Size, &dwNumberOfBytesWritten, NULL);
				}
			}

			// Write the Remaining of Jpeg File
			if (Dib.GetMetadata()->GetAfterSOSData())
				::WriteFile(hOutFile,
							Dib.GetMetadata()->GetAfterSOSData(),
							Dib.GetMetadata()->GetAfterSOSSize(),
							&dwNumberOfBytesWritten,
							NULL);
		}
		else
			throw 0;

		// Clean-Up
		if (Section)
			delete [] Section;
		::CloseHandle(hOutFile);
		if (pInMMFile != NULL)
		{
			::FlushViewOfFile(pInMMFile, 0);
			::UnmapViewOfFile(pInMMFile);
			pInMMFile = NULL;
		}
		if (hInMMapping != NULL)
		{
			::CloseHandle(hInMMapping);
			hInMMapping = NULL;
		}
		::CloseHandle(hInFile);

		return TRUE;
	}
	catch (int)
	{
		// Clean-Up
		if (Section)
			delete [] Section;
		if (hOutFile)
			::CloseHandle(hOutFile);
		::DeleteFile(lpszOutPathName);
		if (pInMMFile != NULL)
		{
			::FlushViewOfFile(pInMMFile, 0);
			::UnmapViewOfFile(pInMMFile);
		}
		if (hInMMapping != NULL)
			::CloseHandle(hInMMapping);
		if (hInFile)
			::CloseHandle(hInFile);

		return FALSE;
	}
}

BOOL CDib::JPEGAddSection(	int SectionType,
							int nSectionSize,	// This is the size of the data only (excluded Type and Size fields)
							LPBYTE pSectionData,
							LPCTSTR lpszInPathName,
							LPCTSTR lpszOutPathName,
							BOOL bShowMessageBoxOnError)
{
	BOOL bSectionAdded = FALSE;
	bool bDoWrite;
	HANDLE hInFile = NULL;
	HANDLE hInMMapping = NULL;
	LPBYTE pInMMFile = NULL;
	HANDLE hOutFile = NULL;
	LPBYTE Section = NULL;
	DWORD dwNumberOfBytesWritten;
	CDib Dib;
	Dib.SetShowMessageBoxOnError(bShowMessageBoxOnError);

	try
	{
		// Check filenames
		if (!IsJPEG(lpszInPathName) || !IsJPEG(lpszOutPathName))
			throw 0;

		// Is Out File Read-Only?
		DWORD dwAttrib = ::GetFileAttributes(lpszOutPathName);
		if ((dwAttrib != 0xFFFFFFFF) && (dwAttrib & FILE_ATTRIBUTE_READONLY))
		{
			CString str;
#ifdef _DEBUG
			str.Format(_T("JPEGAddSection(%s):\n"), lpszOutPathName);
#endif
			str += _T("The file is read only\n");
			TRACE(str);
			if (bShowMessageBoxOnError)
				::AfxMessageBox(str, MB_ICONSTOP);
			throw 0;
		}

		// Open In File for Reading
		bDoWrite = false;
		DWORD dwInDesiredAccess = bDoWrite ? (GENERIC_READ | GENERIC_WRITE) : GENERIC_READ;
		hInFile = ::CreateFile(	lpszInPathName, dwInDesiredAccess,
								FILE_SHARE_READ, NULL, OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL, NULL);
		if (hInFile == INVALID_HANDLE_VALUE)
			throw 0;
		
		// Create the file mapping object
		DWORD flInProtect = (bDoWrite) ? PAGE_READWRITE : PAGE_WRITECOPY;
		hInMMapping = ::CreateFileMapping(hInFile, NULL, flInProtect, 0, 0, NULL);
		if (hInMMapping == NULL)
			throw 0;

		// Map the view
		DWORD dwInDesiredMapAccess = (bDoWrite) ? FILE_MAP_WRITE : FILE_MAP_COPY;
		pInMMFile = (LPBYTE)::MapViewOfFile(hInMMapping, dwInDesiredMapAccess, 0, 0, 0);
		if (pInMMFile == NULL)
			throw 0;

		// Create Out File for Writing
		bDoWrite = true;
		DWORD dwOutDesiredAccess = bDoWrite ? (GENERIC_READ | GENERIC_WRITE) : GENERIC_READ;
		hOutFile = ::CreateFile(lpszOutPathName, dwOutDesiredAccess,
								FILE_SHARE_READ, NULL, CREATE_ALWAYS,
								FILE_ATTRIBUTE_NORMAL, NULL);
		if (hOutFile == INVALID_HANDLE_VALUE)
			throw 0;
		
		// Load All Sections
		if (!Dib.JPEGLoadMetadata(pInMMFile, ::GetFileSize(hInFile, NULL)))
			throw 0;
		
		// Section Vars
		int section;
		int startsection = 0;

		// Check Whether JFIF Section Available
		for (section = 0 ; section < Dib.GetMetadata()->GetSectionCount() ; section++)
		{
			CMetadata::JPEGSECTION* p = Dib.GetMetadata()->GetSection(section);
			if (p->Type == M_JFIF)
				startsection = section;
		}

		// Loop Sections
		for (section = 0 ; section < Dib.GetMetadata()->GetSectionCount() ; section++)
		{
			const unsigned char FF = 0xFF;
			unsigned char marker;
			CMetadata::JPEGSECTION* p = Dib.GetMetadata()->GetSection(section);
			if (!p)
				throw 0;

			// Write original section
			::WriteFile(hOutFile, &FF, 1, &dwNumberOfBytesWritten, NULL);
			marker = p->Type;
			::WriteFile(hOutFile, &marker, 1, &dwNumberOfBytesWritten, NULL);
			if (p->Data && p->Size > 0)
				::WriteFile(hOutFile, p->Data, p->Size, &dwNumberOfBytesWritten, NULL);

			// Append new section either after JFIF or after the SOI Marker
			if (!bSectionAdded && section == startsection)
			{
				Section = new BYTE[nSectionSize + 4];
				if (!Section)
					throw 0;
				Section[0] = 0xFF;
				Section[1] = SectionType;

				// Little Endian To Big Endian
				int nSectionSizeHdr = nSectionSize + 2;
				int lh = (nSectionSizeHdr>>8) & 0xFF;
				int ll = nSectionSizeHdr & 0xFF;
				Section[2] = lh;
				Section[3] = ll;

				// Copy Data
				if (pSectionData && nSectionSize > 0)
					memcpy(&Section[4], pSectionData, nSectionSize);

				// Write Out Section
				::WriteFile(hOutFile, Section, nSectionSize + 4, &dwNumberOfBytesWritten, NULL);

				// Set Flag
				bSectionAdded = TRUE;
			}
		}

		// Write the Remaining of Jpeg File
		if (Dib.GetMetadata()->GetAfterSOSData())
			::WriteFile(hOutFile,
						Dib.GetMetadata()->GetAfterSOSData(),
						Dib.GetMetadata()->GetAfterSOSSize(),
						&dwNumberOfBytesWritten,
						NULL);

		// Clean-Up
		if (Section)
			delete [] Section;
		::CloseHandle(hOutFile);
		if (pInMMFile != NULL)
		{
			::FlushViewOfFile(pInMMFile, 0);
			::UnmapViewOfFile(pInMMFile);
			pInMMFile = NULL;
		}
		if (hInMMapping != NULL)
		{
			::CloseHandle(hInMMapping);
			hInMMapping = NULL;
		}
		::CloseHandle(hInFile);

		if (!bSectionAdded)
		{
			::DeleteFile(lpszOutPathName);
			return FALSE;
		}
		else
			return TRUE;
	}
	catch (int)
	{
		// Clean-Up
		if (Section)
			delete [] Section;
		if (hOutFile)
			::CloseHandle(hOutFile);
		::DeleteFile(lpszOutPathName);
		if (pInMMFile != NULL)
		{
			::FlushViewOfFile(pInMMFile, 0);
			::UnmapViewOfFile(pInMMFile);
		}
		if (hInMMapping != NULL)
			::CloseHandle(hInMMapping);
		if (hInFile)
			::CloseHandle(hInFile);

		return FALSE;
	}
}

BOOL CDib::JPEGReplaceSection(	int SectionType,
								const unsigned char* pSectionSubType,
								int nSectionSubTypeSize,
								int nSectionSize,	// This is the size of the data only (excluded Type and Size fields)
								LPBYTE pSectionData,
								LPCTSTR lpszInPathName,
								LPCTSTR lpszOutPathName,
								BOOL bShowMessageBoxOnError)
{
	BOOL bSectionAdded = FALSE;
	bool bDoWrite;
	HANDLE hInFile = NULL;
	HANDLE hInMMapping = NULL;
	LPBYTE pInMMFile = NULL;
	HANDLE hOutFile = NULL;
	LPBYTE Section = NULL;
	DWORD dwNumberOfBytesWritten;
	CDib Dib;
	Dib.SetShowMessageBoxOnError(bShowMessageBoxOnError);

	try
	{
		// Check filenames
		if (!IsJPEG(lpszInPathName) || !IsJPEG(lpszOutPathName))
			throw 0;

		// Is Out File Read-Only?
		DWORD dwAttrib = ::GetFileAttributes(lpszOutPathName);
		if ((dwAttrib != 0xFFFFFFFF) && (dwAttrib & FILE_ATTRIBUTE_READONLY))
		{
			CString str;
#ifdef _DEBUG
			str.Format(_T("JPEGReplaceSection(%s):\n"), lpszOutPathName);
#endif
			str += _T("The file is read only\n");
			TRACE(str);
			if (bShowMessageBoxOnError)
				::AfxMessageBox(str, MB_ICONSTOP);
			throw 0;
		}

		// Open In File for Reading
		bDoWrite = false;
		DWORD dwInDesiredAccess = bDoWrite ? (GENERIC_READ | GENERIC_WRITE) : GENERIC_READ;
		hInFile = ::CreateFile(	lpszInPathName, dwInDesiredAccess,
								FILE_SHARE_READ, NULL, OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL, NULL);
		if (hInFile == INVALID_HANDLE_VALUE)
			throw 0;
		
		// Create the file mapping object
		DWORD flInProtect = (bDoWrite) ? PAGE_READWRITE : PAGE_WRITECOPY;
		hInMMapping = ::CreateFileMapping(hInFile, NULL, flInProtect, 0, 0, NULL);
		if (hInMMapping == NULL)
			throw 0;

		// Map the view
		DWORD dwInDesiredMapAccess = (bDoWrite) ? FILE_MAP_WRITE : FILE_MAP_COPY;
		pInMMFile = (LPBYTE)::MapViewOfFile(hInMMapping, dwInDesiredMapAccess, 0, 0, 0);
		if (pInMMFile == NULL)
			throw 0;

		// Create Out File for Writing
		bDoWrite = true;
		DWORD dwOutDesiredAccess = bDoWrite ? (GENERIC_READ | GENERIC_WRITE) : GENERIC_READ;
		hOutFile = ::CreateFile(lpszOutPathName, dwOutDesiredAccess,
								FILE_SHARE_READ, NULL, CREATE_ALWAYS,
								FILE_ATTRIBUTE_NORMAL, NULL);
		if (hOutFile == INVALID_HANDLE_VALUE)
			throw 0;
		
		// Load All Sections
		if (!Dib.JPEGLoadMetadata(pInMMFile, ::GetFileSize(hInFile, NULL)))
			throw 0;
		
		// Section Vars
		int section;
		int startsection = 0;

		// Check Whether JFIF Section Available
		for (section = 0 ; section < Dib.GetMetadata()->GetSectionCount() ; section++)
		{
			CMetadata::JPEGSECTION* p = Dib.GetMetadata()->GetSection(section);
			if (p->Type == M_JFIF)
				startsection = section;
		}

		// Loop Sections
		for (section = 0 ; section < Dib.GetMetadata()->GetSectionCount() ; section++)
		{
			const unsigned char FF = 0xFF;
			unsigned char marker;
			CMetadata::JPEGSECTION* p = Dib.GetMetadata()->GetSection(section);
			if (!p)
				throw 0;

			// Write Section
			if (pSectionSubType == NULL ||
				nSectionSubTypeSize == 0)
			{
				if (p->Type != SectionType)
				{
					::WriteFile(hOutFile, &FF, 1, &dwNumberOfBytesWritten, NULL);
					marker = p->Type;
					::WriteFile(hOutFile, &marker, 1, &dwNumberOfBytesWritten, NULL);
					if (p->Data && p->Size > 0)
						::WriteFile(hOutFile, p->Data, p->Size, &dwNumberOfBytesWritten, NULL);
				}
			}
			else
			{
				if ((p->Type != SectionType)					||
					((int)p->Size < (nSectionSubTypeSize + 2))	||
					(memcmp(p->Data + 2, pSectionSubType, nSectionSubTypeSize) != 0))
				{
					::WriteFile(hOutFile, &FF, 1, &dwNumberOfBytesWritten, NULL);
					marker = p->Type;
					::WriteFile(hOutFile, &marker, 1, &dwNumberOfBytesWritten, NULL);
					if (p->Data && p->Size > 0)
						::WriteFile(hOutFile, p->Data, p->Size, &dwNumberOfBytesWritten, NULL);
				}
			}

			// Append new section either after JFIF or after the SOI Marker
			if (!bSectionAdded && section == startsection)
			{
				Section = new BYTE[nSectionSize + 4];
				if (!Section)
					throw 0;
				Section[0] = 0xFF;
				Section[1] = SectionType;

				// Little Endian To Big Endian
				int nSectionSizeHdr = nSectionSize + 2;
				int lh = (nSectionSizeHdr>>8) & 0xFF;
				int ll = nSectionSizeHdr & 0xFF;
				Section[2] = lh;
				Section[3] = ll;

				// Copy Data
				if (pSectionData && nSectionSize > 0)
					memcpy(&Section[4], pSectionData, nSectionSize);

				// Write Out Section
				::WriteFile(hOutFile, Section, nSectionSize + 4, &dwNumberOfBytesWritten, NULL);

				// Set Flag
				bSectionAdded = TRUE;
			}
		}

		// Write the Remaining of Jpeg File
		if (Dib.GetMetadata()->GetAfterSOSData())
			::WriteFile(hOutFile,
						Dib.GetMetadata()->GetAfterSOSData(),
						Dib.GetMetadata()->GetAfterSOSSize(),
						&dwNumberOfBytesWritten,
						NULL);

		// Clean-Up
		if (Section)
			delete [] Section;
		::CloseHandle(hOutFile);
		if (pInMMFile != NULL)
		{
			::FlushViewOfFile(pInMMFile, 0);
			::UnmapViewOfFile(pInMMFile);
			pInMMFile = NULL;
		}
		if (hInMMapping != NULL)
		{
			::CloseHandle(hInMMapping);
			hInMMapping = NULL;
		}
		::CloseHandle(hInFile);

		if (!bSectionAdded)
		{
			::DeleteFile(lpszOutPathName);
			return FALSE;
		}
		else
			return TRUE;
	}
	catch (int)
	{
		// Clean-Up
		if (Section)
			delete [] Section;
		if (hOutFile)
			::CloseHandle(hOutFile);
		::DeleteFile(lpszOutPathName);
		if (pInMMFile != NULL)
		{
			::FlushViewOfFile(pInMMFile, 0);
			::UnmapViewOfFile(pInMMFile);
		}
		if (hInMMapping != NULL)
			::CloseHandle(hInMMapping);
		if (hInFile)
			::CloseHandle(hInFile);

		return FALSE;
	}
}

BOOL CDib::JPEGRemoveSection(	int SectionType,
								const unsigned char* pSectionSubType,
								int nSectionSubTypeSize,
								LPCTSTR lpszInPathName,
								LPCTSTR lpszOutPathName,
								BOOL bShowMessageBoxOnError)
{
	BOOL bSectionRemoved = FALSE;
	bool bDoWrite;
	HANDLE hInFile = NULL;
	HANDLE hInMMapping = NULL;
	LPBYTE pInMMFile = NULL;
	HANDLE hOutFile = NULL;
	DWORD dwNumberOfBytesWritten;
	CDib Dib;
	Dib.SetShowMessageBoxOnError(bShowMessageBoxOnError);

	try
	{
		// Check filenames
		if (!IsJPEG(lpszInPathName) || !IsJPEG(lpszOutPathName))
			throw 0;

		// Is Out File Read-Only?
		DWORD dwAttrib = ::GetFileAttributes(lpszOutPathName);
		if ((dwAttrib != 0xFFFFFFFF) && (dwAttrib & FILE_ATTRIBUTE_READONLY))
		{
			CString str;
#ifdef _DEBUG
			str.Format(_T("JPEGRemoveSection(%s):\n"), lpszOutPathName);
#endif
			str += _T("The file is read only\n");
			TRACE(str);
			if (bShowMessageBoxOnError)
				::AfxMessageBox(str, MB_ICONSTOP);
			throw 0;
		}

		// Open In File for Reading
		bDoWrite = false;
		DWORD dwInDesiredAccess = bDoWrite ? (GENERIC_READ | GENERIC_WRITE) : GENERIC_READ;
		hInFile = ::CreateFile(	lpszInPathName, dwInDesiredAccess,
								FILE_SHARE_READ, NULL, OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL, NULL);
		if (hInFile == INVALID_HANDLE_VALUE)
			throw 0;
		
		// Create the file mapping object
		DWORD flInProtect = (bDoWrite) ? PAGE_READWRITE : PAGE_WRITECOPY;
		hInMMapping = ::CreateFileMapping(hInFile, NULL, flInProtect, 0, 0, NULL);
		if (hInMMapping == NULL)
			throw 0;

		// Map the view
		DWORD dwInDesiredMapAccess = (bDoWrite) ? FILE_MAP_WRITE : FILE_MAP_COPY;
		pInMMFile = (LPBYTE)::MapViewOfFile(hInMMapping, dwInDesiredMapAccess, 0, 0, 0);
		if (pInMMFile == NULL)
			throw 0;

		// Create Out File for Writing
		bDoWrite = true;
		DWORD dwOutDesiredAccess = bDoWrite ? (GENERIC_READ | GENERIC_WRITE) : GENERIC_READ;
		hOutFile = ::CreateFile(lpszOutPathName, dwOutDesiredAccess,
								FILE_SHARE_READ, NULL, CREATE_ALWAYS,
								FILE_ATTRIBUTE_NORMAL, NULL);
		if (hOutFile == INVALID_HANDLE_VALUE)
			throw 0;
		
		// Load All Sections
		if (!Dib.JPEGLoadMetadata(pInMMFile, ::GetFileSize(hInFile, NULL)))
			throw 0;
		
		// Remove Section
		for (int section = 0 ; section < Dib.GetMetadata()->GetSectionCount() ; section++)
		{
			const unsigned char FF = 0xFF;
			unsigned char marker;
			CMetadata::JPEGSECTION* p = Dib.GetMetadata()->GetSection(section);
			if (!p)
				throw 0;

			// Write Section
			if (pSectionSubType == NULL ||
				nSectionSubTypeSize == 0)
			{
				if (p->Type == SectionType)
					bSectionRemoved = TRUE;
				else
				{
					::WriteFile(hOutFile, &FF, 1, &dwNumberOfBytesWritten, NULL);
					marker = p->Type;
					::WriteFile(hOutFile, &marker, 1, &dwNumberOfBytesWritten, NULL);
					if (p->Data && p->Size > 0)
						::WriteFile(hOutFile, p->Data, p->Size, &dwNumberOfBytesWritten, NULL);
				}
			}
			else
			{
				if ((p->Type == SectionType)					&&
					((int)p->Size >= (nSectionSubTypeSize + 2))	&&
					(memcmp(p->Data + 2, pSectionSubType, nSectionSubTypeSize) == 0))
					bSectionRemoved = TRUE;
				else
				{
					::WriteFile(hOutFile, &FF, 1, &dwNumberOfBytesWritten, NULL);
					marker = p->Type;
					::WriteFile(hOutFile, &marker, 1, &dwNumberOfBytesWritten, NULL);
					if (p->Data && p->Size > 0)
						::WriteFile(hOutFile, p->Data, p->Size, &dwNumberOfBytesWritten, NULL);
				}
			}
		}

		// Write the Remaining of Jpeg File
		if (Dib.GetMetadata()->GetAfterSOSData())
			::WriteFile(hOutFile,
						Dib.GetMetadata()->GetAfterSOSData(),
						Dib.GetMetadata()->GetAfterSOSSize(),
						&dwNumberOfBytesWritten,
						NULL);

		// Clean-Up
		::CloseHandle(hOutFile);
		if (pInMMFile != NULL)
		{
			::FlushViewOfFile(pInMMFile, 0);
			::UnmapViewOfFile(pInMMFile);
			pInMMFile = NULL;
		}
		if (hInMMapping != NULL)
		{
			::CloseHandle(hInMMapping);
			hInMMapping = NULL;
		}
		::CloseHandle(hInFile);

		if (!bSectionRemoved)
		{
			::DeleteFile(lpszOutPathName);
			return FALSE;
		}
		else
			return TRUE;
	}
	catch (int)
	{
		// Clean-Up
		if (hOutFile)
			::CloseHandle(hOutFile);
		::DeleteFile(lpszOutPathName);
		if (pInMMFile != NULL)
		{
			::FlushViewOfFile(pInMMFile, 0);
			::UnmapViewOfFile(pInMMFile);
		}
		if (hInMMapping != NULL)
			::CloseHandle(hInMMapping);
		if (hInFile)
			::CloseHandle(hInFile);

		return FALSE;
	}
}

BOOL CDib::JPEGRemoveSections(	int SectionTypeStart,
								int SectionTypeEnd,
								LPCTSTR lpszInPathName,
								LPCTSTR lpszOutPathName,
								BOOL bShowMessageBoxOnError)
{
	BOOL bSectionRemoved = FALSE;
	bool bDoWrite;
	HANDLE hInFile = NULL;
	HANDLE hInMMapping = NULL;
	LPBYTE pInMMFile = NULL;
	HANDLE hOutFile = NULL;
	DWORD dwNumberOfBytesWritten;
	CDib Dib;
	Dib.SetShowMessageBoxOnError(bShowMessageBoxOnError);

	try
	{
		// Check filenames
		if (!IsJPEG(lpszInPathName) || !IsJPEG(lpszOutPathName))
			throw 0;

		// Is Out File Read-Only?
		DWORD dwAttrib = ::GetFileAttributes(lpszOutPathName);
		if ((dwAttrib != 0xFFFFFFFF) && (dwAttrib & FILE_ATTRIBUTE_READONLY))
		{
			CString str;
#ifdef _DEBUG
			str.Format(_T("JPEGRemoveSections(%s):\n"), lpszOutPathName);
#endif
			str += _T("The file is read only\n");
			TRACE(str);
			if (bShowMessageBoxOnError)
				::AfxMessageBox(str, MB_ICONSTOP);
			throw 0;
		}

		// Open In File for Reading
		bDoWrite = false;
		DWORD dwInDesiredAccess = bDoWrite ? (GENERIC_READ | GENERIC_WRITE) : GENERIC_READ;
		hInFile = ::CreateFile(	lpszInPathName, dwInDesiredAccess,
								FILE_SHARE_READ, NULL, OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL, NULL);
		if (hInFile == INVALID_HANDLE_VALUE)
			throw 0;
		
		// Create the file mapping object
		DWORD flInProtect = (bDoWrite) ? PAGE_READWRITE : PAGE_WRITECOPY;
		hInMMapping = ::CreateFileMapping(hInFile, NULL, flInProtect, 0, 0, NULL);
		if (hInMMapping == NULL)
			throw 0;

		// Map the view
		DWORD dwInDesiredMapAccess = (bDoWrite) ? FILE_MAP_WRITE : FILE_MAP_COPY;
		pInMMFile = (LPBYTE)::MapViewOfFile(hInMMapping, dwInDesiredMapAccess, 0, 0, 0);
		if (pInMMFile == NULL)
			throw 0;

		// Create Out File for Writing
		bDoWrite = true;
		DWORD dwOutDesiredAccess = bDoWrite ? (GENERIC_READ | GENERIC_WRITE) : GENERIC_READ;
		hOutFile = ::CreateFile(lpszOutPathName, dwOutDesiredAccess,
								FILE_SHARE_READ, NULL, CREATE_ALWAYS,
								FILE_ATTRIBUTE_NORMAL, NULL);
		if (hOutFile == INVALID_HANDLE_VALUE)
			throw 0;
		
		// Load All Sections
		if (!Dib.JPEGLoadMetadata(pInMMFile, ::GetFileSize(hInFile, NULL)))
			throw 0;
		
		// Remove Section
		for (int section = 0 ; section < Dib.GetMetadata()->GetSectionCount() ; section++)
		{
			const unsigned char FF = 0xFF;
			unsigned char marker;
			CMetadata::JPEGSECTION* p = Dib.GetMetadata()->GetSection(section);
			if (!p)
				throw 0;

			// Write Section
			if (p->Type < SectionTypeStart ||
				p->Type > SectionTypeEnd)
			{
				::WriteFile(hOutFile, &FF, 1, &dwNumberOfBytesWritten, NULL);
				marker = p->Type;
				::WriteFile(hOutFile, &marker, 1, &dwNumberOfBytesWritten, NULL);
				if (p->Data && p->Size > 0)
					::WriteFile(hOutFile, p->Data, p->Size, &dwNumberOfBytesWritten, NULL);
			}
			else
				bSectionRemoved = TRUE;
		}

		// Write the Remaining of Jpeg File
		if (Dib.GetMetadata()->GetAfterSOSData())
			::WriteFile(hOutFile,
						Dib.GetMetadata()->GetAfterSOSData(),
						Dib.GetMetadata()->GetAfterSOSSize(),
						&dwNumberOfBytesWritten,
						NULL);

		// Clean-Up
		::CloseHandle(hOutFile);
		if (pInMMFile != NULL)
		{
			::FlushViewOfFile(pInMMFile, 0);
			::UnmapViewOfFile(pInMMFile);
			pInMMFile = NULL;
		}
		if (hInMMapping != NULL)
		{
			::CloseHandle(hInMMapping);
			hInMMapping = NULL;
		}
		::CloseHandle(hInFile);

		if (!bSectionRemoved)
		{
			::DeleteFile(lpszOutPathName);
			return FALSE;
		}
		else
			return TRUE;
	}
	catch (int)
	{
		// Clean-Up
		if (hOutFile)
			::CloseHandle(hOutFile);
		::DeleteFile(lpszOutPathName);
		if (pInMMFile != NULL)
		{
			::FlushViewOfFile(pInMMFile, 0);
			::UnmapViewOfFile(pInMMFile);
		}
		if (hInMMapping != NULL)
			::CloseHandle(hInMMapping);
		if (hInFile)
			::CloseHandle(hInFile);

		return FALSE;
	}
}

BOOL CDib::JPEGWriteEXIFInplace(LPCTSTR lpszPathName)
{
	const bool bDoWrite = true;

	if (!IsJPEG(lpszPathName))
		return FALSE;

	DWORD dwAttrib = ::GetFileAttributes(lpszPathName);
	if ((dwAttrib != 0xFFFFFFFF) && (dwAttrib & FILE_ATTRIBUTE_READONLY))
	{
		CString str;
#ifdef _DEBUG
		str.Format(_T("JPEGWriteEXIFInplace(%s):\n"), lpszPathName);
#endif
		str += _T("The file is read only\n");
		TRACE(str);
		if (m_bShowMessageBoxOnError)
			::AfxMessageBox(str, MB_ICONSTOP);
		return FALSE;
	}

	// Open File
	DWORD dwDesiredAccess = bDoWrite ? (GENERIC_READ | GENERIC_WRITE) : GENERIC_READ;
	HANDLE hFile = ::CreateFile(lpszPathName, dwDesiredAccess,
								FILE_SHARE_READ, NULL, OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		ShowLastError(_T("JPEGWriteEXIFInplace()"));
		return FALSE;
	}
	
	// Create the file mapping object
	DWORD flProtect = (bDoWrite) ? PAGE_READWRITE : PAGE_WRITECOPY;
	HANDLE hMMapping = ::CreateFileMapping(hFile, NULL, flProtect, 0, 0, NULL);
	if (hMMapping == NULL)
	{
		ShowLastError(_T("JPEGWriteEXIFInplace()"));
		::CloseHandle(hFile);
		return FALSE;
	}

	// Map the view
	DWORD dwDesiredMapAccess = (bDoWrite) ? FILE_MAP_WRITE : FILE_MAP_COPY;
	LPBYTE pMMFile = (LPBYTE)::MapViewOfFile(hMMapping, dwDesiredMapAccess, 0, 0, 0);
	if (pMMFile == NULL)
	{
		ShowLastError(_T("JPEGWriteEXIFInplace()"));
		::CloseHandle(hMMapping);
		::CloseHandle(hFile);
		return FALSE;
	}

	// Process It
	JPEGWriteEXIFInplace(pMMFile, ::GetFileSize(hFile, NULL));

	// Clean-Up
	if (pMMFile != NULL)
	{
		::FlushViewOfFile(pMMFile, 0);
		::UnmapViewOfFile(pMMFile);
		pMMFile = NULL;
	}
	if (hMMapping != NULL)
	{
		::CloseHandle(hMMapping);
		hMMapping = NULL;
	}
	::CloseHandle(hFile);

	return (GetMetadata()->m_ExifInfo.bHasExifSubIFD);
}

BOOL CDib::JPEGWriteEXIFInplace(LPBYTE pJpegData, DWORD dwSize)
{
	if (pJpegData == NULL)
		return FALSE;

	// Parse & Process Exif Data
	GetMetadata()->ParseProcessJPEG(pJpegData, dwSize, true);

	return (GetMetadata()->m_ExifInfo.bHasExifSubIFD);
}

BOOL CDib::CreatePreviewDibFromJPEG(	LPCTSTR lpszPathName,
										int nMaxSizeX,
										int nMaxSizeY,
										CWnd* pProgressWnd/*=NULL*/,
										BOOL bProgressSend/*=TRUE*/,
										CWorkerThread* pThread/*=NULL*/)
{
	BOOL res = FALSE;

	// Check Sizes
	if (nMaxSizeX < 1 && nMaxSizeY < 1)
		return FALSE;

	// Check for jpg filename
	if (!IsJPEG(lpszPathName))
		return FALSE;

	// Load header to get the picture size and orientation
	if (!LoadJPEG(	lpszPathName,
					1,
					TRUE,
					TRUE))
		return FALSE;

	// Invert Width & Height if image is rotated
	int nOrientation = GetExifInfo()->Orientation;
	int nWidth;
	int nHeight;
	if ((nOrientation >= 5) && (nOrientation <= 8))
	{
		nWidth = GetHeight();
		nHeight = GetWidth();
	}
	else
	{
		nWidth = GetWidth();
		nHeight = GetHeight();
	}
	
	// The Scale Factors
	double dScaleFactorX = 0.0;
	double dScaleFactorY = 0.0;
	double dScaleFactor = 0.0;
	if (nMaxSizeX >= 1)
		dScaleFactorX = (double)nWidth / (double)nMaxSizeX;
	if (nMaxSizeY >= 1)
		dScaleFactorY = (double)nHeight / (double)nMaxSizeY;
	if (nMaxSizeX >= 1 && nMaxSizeY >= 1)
		dScaleFactor = max(dScaleFactorX, dScaleFactorY);
	else if (nMaxSizeX >= 1)
		dScaleFactor = dScaleFactorX;
	else if (nMaxSizeY >= 1)
		dScaleFactor = dScaleFactorY;

	// Do nothing and return FALSE if preview dib
	// would be the same size as main dib
	if (dScaleFactor <= 1.5)
		return FALSE;

	// Free
	DeletePreviewDib();

	// Allocate Preview Dib
	m_pPreviewDib = (CDib*)new CDib;
	if (!m_pPreviewDib)
		return FALSE;

	// Load Scaled JPEG
	int nScaleFactor;
	if (dScaleFactor <= 3.0)
	{
		res = m_pPreviewDib->LoadJPEG(	lpszPathName,
										nScaleFactor = 2,
										TRUE,
										FALSE,
										pProgressWnd,
										bProgressSend,
										pThread);
	}
	else if (dScaleFactor <= 6.0)
	{
		res = m_pPreviewDib->LoadJPEG(	lpszPathName,
										nScaleFactor = 4,
										TRUE,
										FALSE,
										pProgressWnd,
										bProgressSend,
										pThread);
	}	
	else
	{
		res = m_pPreviewDib->LoadJPEG(	lpszPathName,
										nScaleFactor = 8,
										TRUE,
										FALSE,
										pProgressWnd,
										bProgressSend,
										pThread);
	}

	// Update ratio
	if (res)
		m_dPreviewDibRatio = (double)nScaleFactor;

	return res;
}

BOOL CDib::CreateThumbnailDibFromJPEG(	LPCTSTR lpszPathName,
										int nMaxSizeX,
										int nMaxSizeY,
										CWnd* pProgressWnd/*=NULL*/,
										BOOL bProgressSend/*=TRUE*/,
										CWorkerThread* pThread/*=NULL*/)
{
	BOOL res = FALSE;

	// Check Sizes
	if (nMaxSizeX < 1 && nMaxSizeY < 1)
		return FALSE;

	// Check for jpg filename
	if (!IsJPEG(lpszPathName))
		return FALSE;

	// Load header to get the picture size and orientation
	if (!LoadJPEG(	lpszPathName,
					1,
					TRUE,
					TRUE))
		return FALSE;

	// Invert Width & Height if image is rotated
	int nOrientation = GetExifInfo()->Orientation;
	int nWidth;
	int nHeight;
	if ((nOrientation >= 5) && (nOrientation <= 8))
	{
		nWidth = GetHeight();
		nHeight = GetWidth();
	}
	else
	{
		nWidth = GetWidth();
		nHeight = GetHeight();
	}

	// The Scale Factors
	double dScaleFactorX = 0.0;
	double dScaleFactorY = 0.0;
	double dScaleFactor = 0.0;
	if (nMaxSizeX >= 1)
		dScaleFactorX = (double)nWidth / (double)nMaxSizeX;
	if (nMaxSizeY >= 1)
		dScaleFactorY = (double)nHeight / (double)nMaxSizeY;
	if (nMaxSizeX >= 1 && nMaxSizeY >= 1)
		dScaleFactor = max(dScaleFactorX, dScaleFactorY);
	else if (nMaxSizeX >= 1)
		dScaleFactor = dScaleFactorX;
	else if (nMaxSizeY >= 1)
		dScaleFactor = dScaleFactorY;

	// Free
	DeleteThumbnailDib();

	// Allocate Thumbnail Dib
	m_pThumbnailDib = (CDib*)new CDib;
	if (!m_pThumbnailDib)
		return FALSE;

	// Load Scaled JPEG
	int nScaleFactor;
	if (dScaleFactor <= 1.5)
	{
		res = m_pThumbnailDib->LoadJPEG(lpszPathName,
										nScaleFactor = 1,
										TRUE,
										FALSE,
										pProgressWnd,
										bProgressSend,
										pThread);
	}
	else if (dScaleFactor <= 3.0)
	{
		res = m_pThumbnailDib->LoadJPEG(lpszPathName,
										nScaleFactor = 2,
										TRUE,
										FALSE,
										pProgressWnd,
										bProgressSend,
										pThread);
	}
	else if (dScaleFactor <= 6.0)
	{
		res = m_pThumbnailDib->LoadJPEG(lpszPathName,
										nScaleFactor = 4,
										TRUE,
										FALSE,
										pProgressWnd,
										bProgressSend,
										pThread);
	}
	else
	{
		res = m_pThumbnailDib->LoadJPEG(lpszPathName,
										nScaleFactor = 8,
										TRUE,
										FALSE,
										pProgressWnd,
										bProgressSend,
										pThread);
	}

	// Update ratio
	if (res)
		m_dThumbnailDibRatio = (double)nScaleFactor;

	return res;
}

BOOL CDib::JPEGSetOrientationInplace(LPCTSTR szFileName,
									int nNewOrientation,
									BOOL bShowMessageBoxOnError)
{
	if (IsJPEG(szFileName))
	{
		CDib Dib;
		Dib.SetShowMessageBoxOnError(bShowMessageBoxOnError);
		if (!Dib.JPEGLoadMetadata(szFileName))
			return FALSE;
		
		memset(Dib.GetExifInfoWrite(), 0, sizeof(CMetadata::EXIFINFOINPLACEWRITE));
		Dib.GetExifInfoWrite()->bOrientation = true;
		Dib.GetExifInfoWrite()->bThumbnailOrientation = true;
		Dib.GetExifInfo()->Orientation = nNewOrientation;
		Dib.GetExifInfo()->ThumbnailOrientation = nNewOrientation;
		return Dib.JPEGWriteEXIFInplace(szFileName);
	}
	else
		return TRUE;
}

BOOL CDib::JPEGSetOleDateTimeInplace(	LPCTSTR szFileName,
										const COleDateTime& Time,
										BOOL bShowMessageBoxOnError)
{
	if (IsJPEG(szFileName))
	{
		CDib Dib;
		Dib.SetShowMessageBoxOnError(bShowMessageBoxOnError);
		if (!Dib.JPEGLoadMetadata(szFileName))
			return FALSE;
		
		memset(Dib.GetExifInfoWrite(), 0, sizeof(CMetadata::EXIFINFOINPLACEWRITE));
		Dib.GetExifInfoWrite()->bDateTime = true;
		CMetadata::FillExifOleDateTimeString(Time, Dib.GetExifInfo()->DateTime);
		return Dib.JPEGWriteEXIFInplace(szFileName);
	}
	else
		return TRUE;
}

BOOL CDib::JPEGSetDateTimeInplace(	LPCTSTR szFileName,
									const CTime& Time,
									BOOL bShowMessageBoxOnError)
{
	if (IsJPEG(szFileName))
	{
		CDib Dib;
		Dib.SetShowMessageBoxOnError(bShowMessageBoxOnError);
		if (!Dib.JPEGLoadMetadata(szFileName))
			return FALSE;
		
		memset(Dib.GetExifInfoWrite(), 0, sizeof(CMetadata::EXIFINFOINPLACEWRITE));
		Dib.GetExifInfoWrite()->bDateTime = true;
		CMetadata::FillExifDateTimeString(Time, Dib.GetExifInfo()->DateTime);
		return Dib.JPEGWriteEXIFInplace(szFileName);
	}
	else
		return TRUE;
}

BOOL CDib::JPEGAutoOrientate(	LPCTSTR szFileName,
								LPCTSTR szTempDir,
								BOOL bShowMessageBoxOnError,
								CWnd* pProgressWnd/*=NULL*/,
								BOOL bProgressSend/*=TRUE*/)
{
	if (IsJPEG(szFileName))
	{
		CDib Dib;
		Dib.SetShowMessageBoxOnError(bShowMessageBoxOnError);
		if (!Dib.JPEGLoadMetadata(szFileName))
			return FALSE;
		
		int nOrientation = Dib.GetExifInfo()->Orientation;
		if ((nOrientation >= 2) && (nOrientation <= 8))
		{
			// Clear Orientation to 1
			// (Clear also Thumbnail orientation to 1)
			memset(Dib.GetExifInfoWrite(), 0, sizeof(CMetadata::EXIFINFOINPLACEWRITE));
			Dib.GetExifInfoWrite()->bOrientation = true;
			Dib.GetExifInfoWrite()->bThumbnailOrientation = true;
			Dib.GetExifInfo()->Orientation = 1;
			Dib.GetExifInfo()->ThumbnailOrientation = 1;
			if (!Dib.JPEGWriteEXIFInplace(szFileName))
				return FALSE;

			// Temporary File
			CString sTempFileName = ::MakeTempFileName(szTempDir, szFileName);

			// Do Lossless Transformation
			// JXFORM_NONE,			no transformation
			// JXFORM_FLIP_H,		horizontal flip
			// JXFORM_FLIP_V,		vertical flip
			// JXFORM_TRANSPOSE,	flip across TopLeft-to-BottomRight axis
			// JXFORM_TRANSVERSE,	flip across TopRight-to-BottomLeft axis
			// JXFORM_ROT_90,		90-degree clockwise rotation
			// JXFORM_ROT_180,		180-degree rotation
			// JXFORM_ROT_270,		270-degree clockwise (or 90 ccw)
			JXFORM_CODE Transform;
			switch (nOrientation)
			{
				case 2 : Transform = JXFORM_FLIP_H; break;
				case 3 : Transform = JXFORM_ROT_180; break;
				case 4 : Transform = JXFORM_FLIP_V; break;
				case 5 : Transform = JXFORM_TRANSPOSE; break;
				case 6 : Transform = JXFORM_ROT_90; break;
				case 7 : Transform = JXFORM_TRANSVERSE; break;
				case 8 : Transform = JXFORM_ROT_270; break;
				default: Transform = JXFORM_NONE; break;
			}
			BOOL res = Dib.LossLessJPEGTrans(	szFileName, 
												sTempFileName,
												Transform,
												TRUE,
												FALSE,
												TRUE,
												"",
												FALSE,
												0, 0, 0, 0,												
												bShowMessageBoxOnError,
												pProgressWnd,
												bProgressSend);
			if (res)
			{
				// Remove and Rename Files
				try
				{
					// Get Last Write File Time
					FILETIME LastWriteTime;
					::GetFileTime(szFileName, NULL, NULL, &LastWriteTime);

					CFile::Remove(szFileName);
					CFile::Rename(sTempFileName, szFileName);
					
					// Set Last Write File Time
					::SetFileTime(szFileName, NULL, NULL, &LastWriteTime);

					return TRUE;
				}
				catch (CFileException* e)
				{
					::DeleteFile(sTempFileName);
					e->Delete();
					return FALSE;
				}
			}
			else
				return FALSE;
		}
		else
			return TRUE;
	}
	else
		return TRUE;
}

BOOL CDib::JPEGWriteComment(LPCTSTR szFileName,
							LPCTSTR szTempDir,
							LPCSTR szComment,
							BOOL bShowMessageBoxOnError)
{
	if (IsJPEG(szFileName))
	{
		// Temporary File
		CString sTempFileName = ::MakeTempFileName(szTempDir, szFileName);

		CDib Dib;
		BOOL res;

		if (szComment == NULL || szComment[0] == _T('\0'))
		{
			res = CDib::JPEGRemoveSection(	M_COM,
											NULL,
											0,
											szFileName,
											sTempFileName,
											bShowMessageBoxOnError);
		}
		else
		{
			res = CDib::LossLessJPEGTrans(	szFileName,		// Src File
											sTempFileName,	// Dst File
											JXFORM_NONE,	// No Transformation
											TRUE,			// Do Trim (default value)
											FALSE,			// Do not force grayscale
											FALSE,			// Do not update Exif (nothing to update)
											szComment,		// The Comment!
											FALSE,			// Do not crop
											0,0,0,0,		// Crop Sizes
											bShowMessageBoxOnError);// Do Show a MessageBox in case of error?
		}
		if (res)
		{
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
					CString str(_T("The file is read only\n"));
					TRACE(str);
					if (bShowMessageBoxOnError)
						::AfxMessageBox(str, MB_ICONSTOP);
				}
				else
					ShowError(e->m_lOsError, bShowMessageBoxOnError, _T("JPEGWriteComment()"));

				e->Delete();
				return FALSE;
			}
			return TRUE;
		}
		else
			return FALSE;
	}
	else
		return FALSE;
}