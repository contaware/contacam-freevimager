#include "stdafx.h"
#include "dib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static void png_error_load(png_structp png_ptr, const char *err_msg)
{
	png_ptr;
	err_msg;
	throw (int)PNG_E_LIBPNG;
}
static void png_warning_load(png_structp png_ptr, const char *warn_msg)
{
	png_ptr;
	warn_msg;
}

BOOL CDib::LoadPNG(	LPCTSTR lpszPathName,
					BOOL bLoadAlpha/*=TRUE*/,
					BOOL bOnlyHeader/*=FALSE*/,
					CWnd* pProgressWnd/*=NULL*/,
					BOOL bProgressSend/*=TRUE*/,
					CWorkerThread* pThread/*=NULL*/)
{
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	png_uint_32 width = 0;
	png_uint_32 height = 0;
	int png_bit_depth = 0;
	int color_type = 0;
	int interlace_type = 0;
	png_colorp png_palette = NULL;
	png_uint_32 res_x = 0;
	png_uint_32 res_y = 0;
	int res_unit_type = PNG_RESOLUTION_UNKNOWN;
	FILE* fp = NULL;
	int palette_entries = 0;
	LPBYTE* row_pointers = NULL;
	LPBYTE tmprow = NULL;
	int dib_bpp = 0;
	int j;
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
			throw (int)PNG_E_ZEROPATH;

		CFile file(lpszPathName, CFile::modeRead | CFile::shareDenyNone);
		m_FileInfo.m_dwFileSize = (DWORD)file.GetLength();
		file.Close();
		if (m_FileInfo.m_dwFileSize == 0)
			throw (int)PNG_E_FILEEMPTY;
	
		// Set the user-defined pointer to point to our jmp_buf. This will
		// hopefully protect against potentially different sized jmp_buf's in
		// libpng, while still allowing this library to be threadsafe.
		png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, (void*)NULL, png_error_load, png_warning_load);
		if (!png_ptr)
			throw (int)PNG_E_NOMEM;

		// Create Info Structure
		info_ptr = png_create_info_struct(png_ptr);
		if (!info_ptr) 
		{
			png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
			throw (int)PNG_E_NOMEM;
		}

		// Open the input file,
		// if only header wanted use 'R' for
		// random access optimization.
		// With network drives it is faster!
		CString sOpenMode = bOnlyHeader ? _T("rbR") : _T("rb");
		if ((fp = _tfopen(lpszPathName, sOpenMode)) == NULL)
			throw (int)PNG_E_READ;

		// Init IO
		png_init_io(png_ptr, fp);

		// Read Info
		png_read_info(png_ptr, info_ptr);

		// Get Header
		png_get_IHDR(	png_ptr,
						info_ptr,
						&width,
						&height,
						&png_bit_depth,
						&color_type,
						&interlace_type,
						NULL,
						NULL);

		// Handle Color Type
		//
		// Note on Simple Transparency:
		//
		// The tRNS chunk supplies transparency data for paletted images and
		// other image types that don't need a full alpha channel. There are
		// "num_trans" transparency values for a paletted image, stored in the
		// same order as the palette colors, starting from index 0. Values
		// for the data are in the range [0, 255], ranging from fully transparent
		// to fully opaque, respectively.
		// For non-paletted images, there is a single color specified that
		// should be treated as fully transparent.
		// Data is valid if (valid & PNG_INFO_tRNS) is non-zero.
		//
		// num_trans:		number of transparent palette colors
		// trans:			transparent values for paletted image
		// trans_values:	transparent color for non-palette image
		switch (color_type) 
		{
			case PNG_COLOR_TYPE_RGB:
				png_set_bgr(png_ptr);
				if (bLoadAlpha && 
					png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS) &&
					info_ptr->num_trans != 0)
				{
					png_set_tRNS_to_alpha(png_ptr);
					dib_bpp = 32;
				}
				else
					dib_bpp = 24;
				if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS) &&
					info_ptr->num_trans != 0)
				{
					m_FileInfo.m_nNumTransparencyIndexes = info_ptr->num_trans;
					m_bAlpha = TRUE;
				}
				palette_entries = 0;
				m_FileInfo.m_nColorSpace = CFileInfo::COLORSPACE_RGB;
				m_FileInfo.m_nBpp = png_bit_depth * 3;
				m_FileInfo.m_nNumColors = 0;
				m_FileInfo.m_bPalette = FALSE;
				break;

			case PNG_COLOR_TYPE_RGB_ALPHA:
				png_set_bgr(png_ptr);
				if (bLoadAlpha)
					dib_bpp = 32;
				else
					dib_bpp = 24;
				m_FileInfo.m_bAlphaChannel = TRUE;
				m_bAlpha = TRUE;
				palette_entries = 0;
				m_FileInfo.m_nColorSpace = CFileInfo::COLORSPACE_RGB;
				m_FileInfo.m_nBpp = png_bit_depth * 4;
				m_FileInfo.m_nNumColors = 0;
				m_FileInfo.m_bPalette = FALSE;
				break;

			case PNG_COLOR_TYPE_PALETTE:
				png_get_PLTE(png_ptr, info_ptr, &png_palette, &m_FileInfo.m_nNumColors);
				if (bLoadAlpha &&
					png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS) &&
					info_ptr->num_trans != 0)
				{
					png_set_bgr(png_ptr);
					png_set_tRNS_to_alpha(png_ptr);
					dib_bpp = 32;
					palette_entries = 0;
				}
				else
				{
					dib_bpp = png_bit_depth;
					palette_entries = m_FileInfo.m_nNumColors;
				}
				if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS) &&
					info_ptr->num_trans != 0)
				{
					m_FileInfo.m_nNumTransparencyIndexes = info_ptr->num_trans;
					m_bAlpha = TRUE;
				}
				m_FileInfo.m_nColorSpace = CFileInfo::COLORSPACE_RGB;
				m_FileInfo.m_nBpp = png_bit_depth;
				m_FileInfo.m_bPalette = TRUE;
				break;

			case PNG_COLOR_TYPE_GRAY:
				if (bLoadAlpha &&
					png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS) &&
					info_ptr->num_trans != 0)
				{
					png_set_tRNS_to_alpha(png_ptr);
					png_set_gray_to_rgb(png_ptr);
					dib_bpp = 32;
					palette_entries = 0;
				}
				else
				{
					if (png_bit_depth > 8) 
						dib_bpp = 8;
					else
						dib_bpp = png_bit_depth;
					palette_entries = 1 << dib_bpp;
				}
				if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS) &&
					info_ptr->num_trans != 0)
				{
					m_FileInfo.m_nNumTransparencyIndexes = info_ptr->num_trans;
					m_bAlpha = TRUE;
				}
				m_FileInfo.m_nColorSpace = CFileInfo::COLORSPACE_GRAYSCALE;
				m_bGrayscale = TRUE;
				m_FileInfo.m_nBpp = png_bit_depth;
				m_FileInfo.m_nNumColors = 0;
				m_FileInfo.m_bPalette = FALSE;
				break;

			case PNG_COLOR_TYPE_GRAY_ALPHA:
				if (bLoadAlpha)
				{
					png_set_gray_to_rgb(png_ptr);
					dib_bpp = 32;
					palette_entries = 0;
				}
				else
				{
					if (png_bit_depth > 8) 
						dib_bpp = 8;
					else
						dib_bpp = png_bit_depth;
					palette_entries = 1 << dib_bpp;
				}
				m_FileInfo.m_bAlphaChannel = TRUE;
				m_bAlpha = TRUE;
				m_FileInfo.m_nColorSpace = CFileInfo::COLORSPACE_GRAYSCALE;
				m_bGrayscale = TRUE;
				m_FileInfo.m_nBpp = png_bit_depth * 2;
				m_FileInfo.m_nNumColors = 0;
				m_FileInfo.m_bPalette = FALSE;
				break;

			default:
				throw (int)PNG_E_BADPNG;
		}

		// Windows supports only 1,4,8,16,24 and 32 bpp
		if (dib_bpp == 2) 
			dib_bpp = 4;

		// DIBs support physical resolution
		if (png_get_valid(png_ptr, info_ptr, PNG_INFO_pHYs)) 
			png_get_pHYs(png_ptr, info_ptr, &res_x, &res_y, &res_unit_type);

		// Convert 16 bit per channel to 8 bits per channel
		if (info_ptr->bit_depth == 16)	
			png_set_strip_16(png_ptr);

		// Set the background color to draw transparent and alpha images over
		// if bLoadAlpha is not set.
		// It is possible to set the red, green, and blue components directly
		// for paletted images instead of supplying a palette index.
		png_color_16 my_background, *image_background;
		my_background.red = GetRValue(m_crBackgroundColor);
		my_background.green = GetGValue(m_crBackgroundColor);
		my_background.blue = GetBValue(m_crBackgroundColor);
		my_background.gray = RGBTOGRAY(	GetRValue(m_crBackgroundColor),
										GetGValue(m_crBackgroundColor),
										GetBValue(m_crBackgroundColor));
		my_background.index = 0;
		if (png_get_bKGD(png_ptr, info_ptr, &image_background))
		{
			if (!bLoadAlpha)
				png_set_background(	png_ptr, image_background,
									PNG_BACKGROUND_GAMMA_SCREEN, 1, 1.0);
			m_FileInfo.m_bHasBackgroundColor = TRUE;
			m_FileInfo.m_crBackgroundColor = RGB(image_background->red,
												image_background->green,
												image_background->blue);
			m_crBackgroundColor = m_FileInfo.m_crBackgroundColor;
		}
		else 
		{
			if (!bLoadAlpha)
				png_set_background(	png_ptr, &my_background,
									PNG_BACKGROUND_GAMMA_SCREEN, 1, 1.0);
		}

		// Allocate BMI
		m_pBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER) + palette_entries*sizeof(RGBQUAD)];
		if (!m_pBMI)
			throw (int)PNG_E_NOMEM;

		// DWORD aligned target DIB ScanLineSize 
		DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES(width * dib_bpp);

		// Fill in the DIB header fields
		m_pBMI->bmiHeader.biSize =          sizeof(BITMAPINFOHEADER);
		m_pBMI->bmiHeader.biWidth =         width;
		m_pBMI->bmiHeader.biHeight =        height;
		m_pBMI->bmiHeader.biPlanes =        1;
		m_pBMI->bmiHeader.biBitCount =      (WORD)dib_bpp;
		m_pBMI->bmiHeader.biCompression =   BI_RGB;  // uncompressed
		m_pBMI->bmiHeader.biSizeImage =     height*uiDIBScanLineSize;   
		// PNG_RESOLUTION_UNKNOWN = 0
        // PNG_RESOLUTION_METER = 1 (pixels/meter) 
		if (res_unit_type == PNG_RESOLUTION_METER) 
		{
			m_pBMI->bmiHeader.biXPelsPerMeter = res_x;
			m_pBMI->bmiHeader.biYPelsPerMeter = res_y;
		}
		else
		{
			m_pBMI->bmiHeader.biXPelsPerMeter = 0;
			m_pBMI->bmiHeader.biYPelsPerMeter = 0;
		}
		m_pBMI->bmiHeader.biClrUsed =       palette_entries;
		m_pBMI->bmiHeader.biClrImportant =  0;
		if (m_pBMI->bmiHeader.biBitCount <= 8)
			m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + m_pBMI->bmiHeader.biSize);
		else
			m_pColors = NULL;
		m_dwImageSize = m_pBMI->bmiHeader.biSizeImage;		

		// Init Masks For 16 and 32 bits Pictures
		InitMasks();

		// Init File Info
		m_FileInfo.m_nType = CFileInfo::PNG;
		m_FileInfo.m_nWidth = m_pBMI->bmiHeader.biWidth;
		m_FileInfo.m_nHeight = m_pBMI->bmiHeader.biHeight;
		m_FileInfo.m_nCompression = m_pBMI->bmiHeader.biCompression;
		m_FileInfo.m_dwImageSize = m_FileInfo.m_nWidth * m_FileInfo.m_nHeight * m_FileInfo.m_nBpp / 8;
		m_FileInfo.m_nXPixsPerMeter = m_pBMI->bmiHeader.biXPelsPerMeter;
		m_FileInfo.m_nYPixsPerMeter = m_pBMI->bmiHeader.biYPelsPerMeter;
		m_FileInfo.m_nImageCount = 1;
		m_FileInfo.m_nImagePos = 0;

		// If only header wanted return now
		if (bOnlyHeader)
		{
			png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
			fclose(fp);
			return TRUE;
		}

		// Set Colors
		switch (color_type) 
		{
			case PNG_COLOR_TYPE_PALETTE:
			{
				for (int i = 0 ; i < palette_entries ; i++) 
				{
					m_pColors[i].rgbRed   = png_palette[i].red;
					m_pColors[i].rgbGreen = png_palette[i].green;
					m_pColors[i].rgbBlue  = png_palette[i].blue;
				}
				break;
			}
			case PNG_COLOR_TYPE_GRAY:
			case PNG_COLOR_TYPE_GRAY_ALPHA:
			{
				for (int i = 0 ; i < palette_entries ; i++) 
				{
					m_pColors[i].rgbRed   =
					m_pColors[i].rgbGreen =
					m_pColors[i].rgbBlue  = (BYTE)((i*255)/(palette_entries-1));
				}
				break;
			}
			default:
				break;
		}

		// Create Palette from BMI
		CreatePaletteFromBMI();

		// Allocate Bits
		m_pBits = (LPBYTE)BIGALLOC(m_dwImageSize);
		if (!m_pBits)
			throw (int)PNG_E_NOMEM;

		// Init Row Pointers
		row_pointers = (LPBYTE*)new BYTE[height*sizeof(LPBYTE)];
		if (!row_pointers)
			throw (int)PNG_E_NOMEM;
		for (j = 0 ; j < (int)height ; j++)
			row_pointers[height-1-j] = &m_pBits[j*uiDIBScanLineSize];

		// Decode
		int pass;
#ifdef PNG_READ_INTERLACING_SUPPORTED
		pass = png_set_interlace_handling(png_ptr);
#else
		if (png_ptr->interlaced)
			png_error(png_ptr, "Cannot read interlaced image -- interlace handler disabled.");
		pass = 1;
#endif
		png_ptr->num_rows = height; // Make sure this is set correctly

		for (int passnum = 0 ; passnum < pass ; passnum++)
		{
			for (unsigned int rownum = 0 ; rownum < height ; rownum++)
			{
				if ((rownum & 0x7) == 0)
				{
					if (pThread && pThread->DoExit())
						throw (int)PNG_THREADEXIT;
					DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, rownum, height);
				}
				png_read_row(png_ptr, row_pointers[rownum], png_bytep_NULL);
			}
		}

		// Special handling for this bit depth, since it doesn't exist in DIBs
		// expand 2bpp to 4bpp
		if (png_bit_depth == 2 && dib_bpp == 4) 
		{
			tmprow = (LPBYTE)new BYTE[(width+3)/4];
			if (!tmprow)
				throw (int)PNG_E_NOMEM;

			for (j = 0 ; j < (int)height ; j++) 
			{
				memcpy(tmprow, row_pointers[j], (width+3)/4);
				memset(row_pointers[j], 0, (width+1)/2 );

				for(int i = 0 ; i < (int)width ; i++) 
				{
					row_pointers[j][i/2] |= 
						( ((tmprow[i/4] >> (2*(3-i%4)) ) & 0x03)<< (4*(1-i%2)) );
				}
			}
			delete [] tmprow;
			tmprow = NULL;
		}

		DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

		// Free
		delete [] row_pointers;
		row_pointers = NULL;

		// Close
		png_read_end(png_ptr, info_ptr);
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		png_ptr = NULL;
		fclose(fp);
		fp = NULL;

		return TRUE;
	}
	catch (CFileException* e)
	{
		DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
		TCHAR szCause[255];
		CString str;
#ifdef _DEBUG
		str = _T("LoadPNG: ");
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
		if (png_ptr)
		{
			png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
			png_ptr = NULL;
		}
		if (fp)
		{
			fclose(fp);
			fp = NULL;
		}
		if (tmprow)
		{
			delete [] tmprow;
			tmprow = NULL;
		}
		if (row_pointers)
		{
			delete [] row_pointers;
			row_pointers = NULL;
		}
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
		if (error_code == PNG_THREADEXIT)
			return FALSE;
		
		CString str;
#ifdef _DEBUG
		str.Format(_T("LoadPNG(%s):\n"), lpszPathName);
#endif
		switch (error_code)
		{
			case PNG_E_ZEROPATH :		str += _T("The file name is zero\n");
			break;
			case PNG_E_NOMEM :			str += _T("Could not alloc memory\n");
			break;
			case PNG_E_LIBPNG :			str += _T("Cannot load file\n");
			break;
			case PNG_E_BADPNG :			str += _T("Corrupted or unsupported PNG\n");
			break;
			case PNG_E_READ :			str += _T("Couldn't read PNG file\n");
			break;
			case PNG_E_FILEEMPTY :		str += _T("File is empty\n");
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

static void png_error_save(png_structp png_ptr, const char *err_msg)
{
	png_ptr;
	err_msg;
	throw (int)PNG_E_LIBPNG;
}

static void png_warning_save(png_structp png_ptr, const char *warn_msg)
{
	png_ptr;
	warn_msg;
}

BOOL CDib::SavePNG(	LPCTSTR lpszPathName,
					BOOL bPaletteTransparency/*=FALSE*/,
					BOOL bStoreBackgroundColor/*=FALSE*/,
					CWnd* pProgressWnd/*=NULL*/,
					BOOL bProgressSend/*=TRUE*/,
					CWorkerThread* pThread/*=NULL*/)
{
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	LPBYTE newimage = NULL;
	int dib_bpp = 0;
	png_uint_32 res_x, res_y;
	png_color_8 pngc8;
	int height,width;
	int palette_entries = 0;
	int topdown;
	int compression;
	FILE *fp = NULL;
	png_color png_palette[256];
	LPBYTE* row_pointers = NULL;
	DWORD *bitfields = NULL;
	unsigned int v;
	int bf_format;// bitfields format identifier
	DIB_INIT_PROGRESS;

	try
	{
		CString sPathName(lpszPathName);
		if (sPathName.IsEmpty())
			throw (int)PNG_E_ZEROPATH;

		DWORD dwAttrib = ::GetFileAttributes(lpszPathName);
		if ((dwAttrib != 0xFFFFFFFF) && (dwAttrib & FILE_ATTRIBUTE_READONLY))
			throw (int)PNG_E_FILEREADONLY;

		if (!m_pBits)
		{
			if (!DibSectionToBits())
				throw (int)PNG_E_BADBMP;
		}

		if (!m_pBits || !m_pBMI)
			throw (int)PNG_E_BADBMP;

		if (IsCompressed())
		{
			if (!Decompress(GetBitCount())) // Decompress
				throw (int)PNG_E_BADBMP;
		}

		// Init Vars
		width = m_pBMI->bmiHeader.biWidth;
		height = m_pBMI->bmiHeader.biHeight;
		dib_bpp = m_pBMI->bmiHeader.biBitCount;
		compression = m_pBMI->bmiHeader.biCompression;
		res_x = m_pBMI->bmiHeader.biXPelsPerMeter;
		res_y = m_pBMI->bmiHeader.biYPelsPerMeter;
		palette_entries = m_pBMI->bmiHeader.biClrUsed;

		// If the height is negative, the top scanline is stored first
		topdown = 0;
		if (height < 0) 
		{
			height = -height;
			topdown = 1;
		}

		// Only certain combinations of compression and bpp are allowed
		switch (compression) 
		{
			case BI_RGB: 
				if (dib_bpp!=1 && dib_bpp!=4 && dib_bpp!=8 && dib_bpp!=16
					&& dib_bpp!=24 && dib_bpp!=32)
					throw (int)PNG_E_UNSUPP;
				break;
			case BI_BITFIELDS:
				if (dib_bpp!=16 && dib_bpp!=32)
					throw (int)PNG_E_UNSUPP;
				break;
			default:
				throw (int)PNG_E_UNSUPP;
		}

		// DWORD aligned Source DIB ScanLineSize 
		DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES(width * dib_bpp);

		// Set Number of Palette Entries
		if (dib_bpp > 8) 
			palette_entries = 0;
		else
		{
			if (palette_entries == 0) // 0 means it has a full palette 
				palette_entries = 1 << dib_bpp;
		}

		// Bitfields Handling
		bitfields = (DWORD*)((LPBYTE)m_pBMI + sizeof(BITMAPINFOHEADER));	// that's also correct for BITMAPV4HEADER and BITMAPV5HEADER
																			// because they have bV4RedMask, bV4GreenMask, bV4BlueMask and
																			// bV5RedMask, bV5GreenMask, bV5BlueMask members at this offset
		bf_format = 0;
		if (compression == BI_BITFIELDS) 
		{
			if (dib_bpp == 16) 
			{
				if     (bitfields[0]==0x00007c00 && bitfields[1]==0x000003e0 && 
						bitfields[2]==0x0000001f)
						bf_format=11;  // 555
				else if (bitfields[0]==0x0000f800 && bitfields[1]==0x000007e0 &&
						bitfields[2]==0x0000001f)
						bf_format=12;  // 565
				else
					throw (int)PNG_E_UNSUPP;
			}
			if (dib_bpp == 32) 
			{
				if     (bitfields[0]==0x00ff0000 &&
						bitfields[1]==0x0000ff00 &&
						bitfields[2]==0x000000ff)
						bf_format=21;
				else
					throw (int)PNG_E_UNSUPP;
			}
		}
		if (bf_format==0 && dib_bpp==16)
			bf_format=10;
		if (bf_format==0 && dib_bpp==32 && !m_bAlpha)
			bf_format=20;

		// Done analyzing the DIB, now time to convert it to PNG
		png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, (void*)NULL, png_error_save, png_warning_save);
		if (!png_ptr) 
			throw (int)PNG_E_NOMEM;

		// Create Info Structure
		info_ptr = png_create_info_struct(png_ptr);
		if (!info_ptr) 
			throw (int)PNG_E_NOMEM;

		// Open File
		fp = _tfopen(lpszPathName, _T("wb"));
		if (!fp)
			throw (int)PNG_E_WRITE;

		// Init IO
		png_init_io(png_ptr, fp);

		// Set Color Type
		int color_type;
		if (dib_bpp > 8)
		{
			if (m_bAlpha && dib_bpp == 32)
			{
				if (m_bGrayscale)
					color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
				else
					color_type = PNG_COLOR_TYPE_RGB_ALPHA;
			}
			else
				color_type = PNG_COLOR_TYPE_RGB;
		}
		else if (dib_bpp == 8 && m_bGrayscale)
		{
			MakeGrayscaleAscending();
			color_type = PNG_COLOR_TYPE_GRAY;
			palette_entries = 0;
		}
		else
			color_type = PNG_COLOR_TYPE_PALETTE;

		// Write Header
		png_set_IHDR(	png_ptr,
						info_ptr,
						width,
						height,
						(dib_bpp > 8) ? 8 : dib_bpp, // bits per channel
						color_type,
						PNG_INTERLACE_NONE,
						PNG_COMPRESSION_TYPE_BASE,
						PNG_FILTER_TYPE_BASE);

		// Set compression level
		// Note that tests have
		// shown that zlib compression levels 3-6 usually perform as well as level 9
		// for PNG images, and do considerably fewer caclulations.
		// -> leave default
		//png_set_compression_level(png_ptr, 9);

		// For 16-bit DIBs, we get to write an sBIT chunk
		if ((bf_format == 10) || (bf_format == 11)) 
		{
			pngc8.red = 5;  pngc8.green = 5;  pngc8.blue = 5;
			png_set_sBIT(png_ptr, info_ptr, &pngc8);
		}
		if (bf_format == 12) 
		{
			pngc8.red = 5;  pngc8.green = 6;  pngc8.blue = 5;
			png_set_sBIT(png_ptr, info_ptr, &pngc8);
		}

		// Set DPI
		if ((res_x > 0) && (res_y > 0))
			png_set_pHYs(png_ptr, info_ptr, res_x, res_y, PNG_RESOLUTION_METER);

		// Store Background Color
		if (bStoreBackgroundColor)
		{
			png_color_16 image_background;
			if (color_type == PNG_COLOR_TYPE_RGB ||
				color_type == PNG_COLOR_TYPE_RGB_ALPHA)
			{
				image_background.red = GetRValue(m_crBackgroundColor);
				image_background.green = GetGValue(m_crBackgroundColor);
				image_background.blue = GetBValue(m_crBackgroundColor);
			}
			else if (color_type == PNG_COLOR_TYPE_PALETTE)
			{
				if (bPaletteTransparency)
					image_background.index = 0;
				else
					image_background.index = m_pPalette->GetNearestPaletteIndex(m_crBackgroundColor);
			}
			else
			{
				image_background.gray = RGBTOGRAY(	GetRValue(m_crBackgroundColor),
													GetGValue(m_crBackgroundColor),
													GetBValue(m_crBackgroundColor));
			}
			png_set_bKGD(png_ptr, info_ptr, &image_background);
		}

		// Set Palette
		if (palette_entries > 0) 
		{
			for (int i = 0 ; i < palette_entries ; i++) 
			{
				png_palette[i].red   = m_pColors[i].rgbRed;
				png_palette[i].green = m_pColors[i].rgbGreen;
				png_palette[i].blue  = m_pColors[i].rgbBlue;
			}
			png_set_PLTE(png_ptr, info_ptr, png_palette, palette_entries);
		}

		// Set Transparency
		if (bPaletteTransparency &&
			color_type == PNG_COLOR_TYPE_PALETTE)
		{
			png_byte trans[1];
			trans[0] = 0; // Index 0 is fully transparent
			png_color_16 trans_values; // Not used
			memset(&trans_values, 0, sizeof(png_color_16));
			png_set_tRNS(png_ptr, info_ptr, trans, 1, &trans_values);
		}

		// BGR -> RGB
		if (color_type == PNG_COLOR_TYPE_RGB ||
			color_type == PNG_COLOR_TYPE_RGB_ALPHA)
			png_set_bgr(png_ptr);

		// Write Info
		png_write_info(png_ptr, info_ptr);

		// Allocate Row Pointers
		row_pointers = (LPBYTE*)new BYTE[height*sizeof(LPBYTE)];
		if (!row_pointers)
			throw (int)PNG_E_NOMEM;

		// Special handling for these bit depths:
		if (dib_bpp == 16 || (dib_bpp == 32 && !m_bAlpha)) 
		{
			newimage = (LPBYTE)BIGALLOC(height*width*3);
			if (!newimage)
				throw (int)PNG_E_NOMEM;

			for (int y = 0 ; y < height ; y++) 
			{
				for (int x = 0 ; x < width ; x++) 
				{
					switch (bf_format) 
					{
						case 10:  case 11:  // 16-bit, format 555 (xRRRRRGG GGGBBBBB)
							v = m_pBits[y*uiDIBScanLineSize+x*2+0] | (m_pBits[y*uiDIBScanLineSize+x*2+1]<<8);
							newimage[(y*width+x)*3+0]= (unsigned char)((v & 0x0000001f)<<3); // blue
							newimage[(y*width+x)*3+1]= (unsigned char)((v & 0x000003e0)>>2); // green
							newimage[(y*width+x)*3+2]= (unsigned char)((v & 0x00007c00)>>7); // red
							break;
						case 12:            // 16-bit, format 565 (RRRRRGGG GGGBBBBB)
							v = m_pBits[y*uiDIBScanLineSize+x*2+0] | (m_pBits[y*uiDIBScanLineSize+x*2+1]<<8);
							newimage[(y*width+x)*3+0]= (unsigned char)((v & 0x0000001f)<<3); // blue
							newimage[(y*width+x)*3+1]= (unsigned char)((v & 0x000007e0)>>3); // green
							newimage[(y*width+x)*3+2]= (unsigned char)((v & 0x0000f800)>>8); // red
							break;
						case 20:  case 21:  // 32-bit, every 4th byte wasted (b g r x)
							newimage[(y*width+x)*3+0]= m_pBits[y*uiDIBScanLineSize+x*4+0]; // blue
							newimage[(y*width+x)*3+1]= m_pBits[y*uiDIBScanLineSize+x*4+1]; // green
							newimage[(y*width+x)*3+2]= m_pBits[y*uiDIBScanLineSize+x*4+2]; // red
							break;
					}
				}
			}

			for (int i = 0; i < height ; i++) 
			{
				if (topdown) 
					row_pointers[i]= &newimage[i*width*3];
				else
					row_pointers[height-1-i]= &newimage[i*width*3];
			}
			int pass;
#ifdef PNG_WRITE_INTERLACING_SUPPORTED
			pass = png_set_interlace_handling(png_ptr);
#else
			pass = 1;
#endif
			for (int passnum = 0; passnum < pass; passnum++)
			{
				for (int rownum = 0 ; rownum < height ; rownum++)
				{
					if ((rownum & 0x7) == 0)
					{
						if (pThread && pThread->DoExit())
							throw (int)PNG_THREADEXIT;
						DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, rownum, height);
					}
					png_write_row(png_ptr, row_pointers[rownum]);
				}
			}

			BIGFREE(newimage);
			newimage = NULL;
		}
		else 
		{
			// Bottom-Up Top-Down Conversion
			for (int i = 0 ; i < height ; i++) 
			{
				if (topdown) 
					row_pointers[i] = &m_pBits[i*uiDIBScanLineSize];
				else
					row_pointers[height-1-i] = &m_pBits[i*uiDIBScanLineSize];
			}

			// Convert From BGRA -> GA
			if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
			{
				// Allocate
				LPBYTE tmp = (LPBYTE)new BYTE[uiDIBScanLineSize];
				
				// Write Rows
				int pass;
	#ifdef PNG_WRITE_INTERLACING_SUPPORTED
				pass = png_set_interlace_handling(png_ptr);
	#else
				pass = 1;
	#endif
				for (int passnum = 0; passnum < pass; passnum++)
				{
					for (int rownum = 0 ; rownum < height ; rownum++)
					{
						if ((rownum & 0x7) == 0)
						{
							if (pThread && pThread->DoExit())
								throw (int)PNG_THREADEXIT;
							DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, rownum, height);
						}
						for (int x = 0 ; x < width ; x++)
						{
							tmp[2*x]   = row_pointers[rownum][4*x+2]; // Gray
							tmp[2*x+1] = row_pointers[rownum][4*x+3]; // Alpha
						}
						png_write_row(png_ptr, tmp);
					}
				}

				// Free
				delete [] tmp;
			}
			else
			{
				int pass;
	#ifdef PNG_WRITE_INTERLACING_SUPPORTED
				pass = png_set_interlace_handling(png_ptr);
	#else
				pass = 1;
	#endif
				for (int passnum = 0; passnum < pass; passnum++)
				{
					for (int rownum = 0 ; rownum < height ; rownum++)
					{
						if ((rownum & 0x7) == 0)
						{
							if (pThread && pThread->DoExit())
								throw (int)PNG_THREADEXIT;
							DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, rownum, height);
						}
						png_write_row(png_ptr, row_pointers[rownum]);
					}
				}
			}
		}

		DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

		// Free
		delete [] row_pointers;
		row_pointers = NULL;

		// Set Text
		/*
		png_text text_ptr[1];
		text_ptr[0].key = "Library";
		text_ptr[0].text = "CDib";
		text_ptr[0].compression = PNG_TEXT_COMPRESSION_NONE;
		png_set_text(png_ptr, info_ptr, text_ptr, 1);
		*/

		// Close
		png_write_end(png_ptr, info_ptr);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		png_ptr = NULL;
		fclose(fp);
		fp = NULL;

		return TRUE;
	}
	catch (int error_code)
	{
		if (png_ptr) 
		{
			png_destroy_write_struct(&png_ptr, &info_ptr);
			png_ptr = NULL;
		}
		if (fp) 
		{
			fclose(fp);
			fp = NULL;
		}
		if (row_pointers)
		{
			delete [] row_pointers;
			row_pointers = NULL;
		}
		if (newimage) 
		{
			BIGFREE(newimage);
			newimage = NULL;
		}
		
		DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

		// Just Exit
		if (error_code == PNG_THREADEXIT)
			return FALSE;

		CString str;
#ifdef _DEBUG
		str.Format(_T("SavePNG(%s):\n"), lpszPathName);
#endif
		switch (error_code)
		{
			case PNG_E_ZEROPATH :		str += _T("The file name is zero\n");
			break;
			case PNG_E_FILEREADONLY :	str += _T("The file is read only\n");
			break;
			case PNG_E_NOMEM :			str += _T("Could not alloc memory\n");
			break;
			case PNG_E_UNSUPP :			str += _T("Unsupported image type\n");
			break;
			case PNG_E_LIBPNG :			str += _T("Cannot save file\n");
			break;
			case PNG_E_BADBMP :			str += _T("Corrupted or unsupported DIB\n");
			break;
			case PNG_E_WRITE :			str += _T("Couldn't write PNG file\n");
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