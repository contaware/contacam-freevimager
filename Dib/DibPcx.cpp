#include "stdafx.h"
#include "dib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef SUPPORT_PCX

/*******************************************************************************************************
PCX files can hold the following Images: 2-color mono, 4-color palettized CGA, 16-color palettized EGA,
256-color palettized VGA, and 24-bit truecolor.

PCX uses a raster data encoding scheme
The raster data immediately follows the 128byte header.
It uses a run length encoding for compression purposes.
Data is stored top-down, left-to-right.


**The encoding is dependent on the number of bits/pixel** 

1. Raster Data encoding for 1,2,4 and 8bit images:

| n Byte 1 | c Byte 2 | Result                                                           |
| >=192    |   any    | (n - 192) consecutive pixels of color index c                    |
| <192     |    -     | a pixel with color index n. (c is already part of the next code) |
(Pixel 192 is encode as 193 192)

Every scan line plane with an odd number of bytes will have a zero appended, to match a 16 bit alignment.
Only pixels on the same scan line may be combined to form a 2-byte sequence.
When more than one color plane is stored in the file, each line of the
image is stored by color plane (generally ordered red, green, blue,
intensity), as shown below.

Scan line 0: 	RRR...
		GGG...
		BBB...
		III...
Scan line 1: 	RRR...
		GGG...
		BBB...
		III...
(etc.)

2. Raster Data encoding for 24-bit truecolor images:
Every row of pixels is written as three or four 8bit single color component scan lines,
each having the same scheme as the 8bit version.
These scan lines have the following order: Red, Green, Blue and optional Alpha.
Example:
For NumOfplanes=4 the fifth scan line in the file will be the Red component of pixel row 2.


**Decoding .PCX Files**

First, find the pixel dimensions of the image by calculating [XSIZE =
Xmax - Xmin + 1] and [YSIZE = Ymax - Ymin + 1].  Then calculate how
many bytes are required to hold one complete uncompressed scan line:
TotalBytes = NPlanes * BytesPerLine Note that since there are always
an integral number of bytes, there will probably be unused data at the
end of each scan line.  TotalBytes shows how much storage must be
available to decode each scan line, including any blank area on the
right side of the image.  You can now begin decoding the first scan
line - read the first byte of data from the file.  If the top two bits
are set, the remaining six bits in the byte show how many times to
duplicate the next byte in the file.  If the top bits are not set, the
first byte is the data itself, with a count of one.  Continue decoding
the rest of the line.  Keep a running subtotal of how many bytes are
moved and duplicated into the output buffer.  When the subtotal equals
TotalBytes, the scan line is complete.  There will always be a
decoding break at the end of each scan line.  But there will not be a
decoding break at the end of each plane within each scan line.  When
the scan line is completed, there may be extra blank data at the end
of each plane within the scan line.  Use the XSIZE and YSIM values to
find where the valid image data is.  If the data is multi-plane
BytesPerLine shows where each plane ends within the scan line.
Continue decoding the remainder of the scan lines.  There may be extra
scan lines at the bottom of the image, to round to 8 or 16 scan lines.


**Palette Information Description**

1. CGA Color Map

Header Byte #16 
Background color is determined in the upper four bits.
Header Byte #19
Only upper 3 bits are used, lower 5 bits are ignored.  The first three
bits that are used are ordered C, P, I.  These bits are interpreted as
follows:

c: color burst enable - 0 = color; 1 = monochrome
p: palette - 0 = yellow; 1 = white
i: intensity - 0 = dim; 1 = bright$

2. EGA/VGA 16 Color Palette Information

The palette information is stored in one of two different formats. In
standard RGB format (IBM EGA, IBM VGA) the data is stored as 16
triples.  Each triple is a 3 byte quantity of Red, Green, Blue values.
The values can range from 0-255 so some interpretation into the base
card format is necessary. On an IBM EGA, for example, there are 4
possible levels of RGB for each color. Since 256/4 = 64, the
following is a list of the settings and levels:

Setting		Level
0-63		0
64-127		1
128-192		2
193-254		3


3. VGA 256 Color Palette Information

ZSoft has recently added the capability to store palettes containing
more than 16 colors in the .PCX image file.  The 256 color palette is
formatted and treated the same as the 16 color palette, except that it
is substantially longer.  The palette (number of colors x 3 bytes in
length) is appended to the end of the .PCX file, and is preceded by a
12 decimal.  To determine the VGA BIOS palette you need only divide
the values read in the palette by 4. To access a 256 color palette:
First, check the version number in the header, if it contains a 5
there is a palette.  Second, read to the end of the file and count
back 769 bytes.  The value you find should be a 12 decimal, showing
the presence of a 256 color palette.  CGA Color Palette Information
For a standard IBM CGA board, the palette settings are a bit more
complex.  Only the first byte of the triple is used.  The first triple
has a valid first byte which represents the background color.  To find
the background, take the (unsigned) byte value and divide by 16.  This
will give a result between 0-15, hence the background color.  The
second triple has a valid first byte, which represents the foreground
palette.  PC Paintbrush supports 8 possible CGA palettes, so when the
foreground setting i ded between 0 and 255, there are 8 ranges of
numbers and the divisor is 32.
*/
BOOL CDib::LoadPCX(	LPCTSTR lpszPathName,
					BOOL bOnlyHeader/*=FALSE*/,
					CWnd* pProgressWnd/*=NULL*/,
					BOOL bProgressSend/*=TRUE*/,
					CWorkerThread* pThread/*=NULL*/)
{

	LPBYTE pabFileData = NULL;
	LPBYTE pPCXDecompr = NULL;
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
			throw (int)PCX_E_ZEROPATH;

		// Check for .pcx Extension
		if (::GetFileExt(sPathName) != _T(".pcx"))
			throw (int)PCX_E_WRONGEXTENTION;

		CFile file(lpszPathName, CFile::modeRead | CFile::shareDenyNone);
		m_FileInfo.m_dwFileSize = (DWORD)file.GetLength();
		if (m_FileInfo.m_dwFileSize == 0)
			throw (int)PCX_E_FILEEMPTY;

		// PCX header:
		struct PCXHead {
			BYTE   ID; // must be 0x0A   
			BYTE   Version; // 0 = Version 2.5 (=fixed EGA-Palette) 
							// 2 = Version 2.8 with Palette (modified EGA-Palette)
							// 3 = Version 2.8 without Palette   
							// 4 = PC Paintbrush for Windows
							// 5 = Version 3.0 (=24bit support)
			BYTE   Encoding; // 1 = RLE compressed (always set)
			BYTE   BitPerPixel; // 1, 2, 4, or 8
			WORD   X1;
			WORD   Y1;
			WORD   X2; // XSize = X2 - X1 + 1
			WORD   Y2; // YSize = Y2 - Y1 + 1
			WORD   HRes; // Xdpi
			WORD   VRes; // Ydpi
			BYTE   ClrMap[16*3]; // ColorTable for 4color and 16color images: RGB, RGB, ...
			BYTE   Reserved1; // = 0
			BYTE   NumPlanes;	// 1 = Monochrome / Palettized    
								// 3 = RGB
								// 4 = RGBI (Red, Green, Blue, Intensity)
			WORD   BPL; // Bytes Per Scanline Plane uncompressed (is an even number=WORD aligned!)
			WORD   Pal_t;	// 1 = Color or B&W   
							// 2 = Grayscale
			BYTE   Filler[58]; // makes the header a total of 128 bytes
		} sHeader;
		// pixels ...
		// 12 = init of palette marker
		// 768 bytes palette data

		if (file.Read(&(sHeader.ID), 1) != 1)
			throw (int)PCX_E_READ;
			
		if (file.Read(&(sHeader.Version), 1) != 1)
			throw (int)PCX_E_READ;

		if (file.Read(&(sHeader.Encoding), 1) != 1)
			throw (int)PCX_E_READ;

		if ((sHeader.ID != PCX_HEADER_MARKER) ||
			(sHeader.Encoding != 0x01) ||
			(sHeader.Version > 5))
		{
			throw (int)PCX_E_BADPCX;
		}

		// Reset the value of the file pointer to the beginning of the file
		file.SeekToBegin( );

		// Allocate memory to hold the file data
		pabFileData = (LPBYTE)::VirtualAlloc(	NULL,
												m_FileInfo.m_dwFileSize,
												MEM_COMMIT,
												PAGE_READWRITE);
		if (pabFileData == NULL)
			throw (int)PCX_E_NOMEM;

		// Read the PCX file
		if (bOnlyHeader)
		{
			if (file.Read(pabFileData, sizeof(sHeader)) != (UINT)(sizeof(sHeader)))
				throw (int)PCX_E_READ;
		}
		else
		{
			if (file.Read(pabFileData, m_FileInfo.m_dwFileSize) != (UINT)(m_FileInfo.m_dwFileSize))
				throw (int)PCX_E_READ;
		}

		// Close the file
		file.Close();

		// Get the PCX header
		memcpy(&sHeader, pabFileData, sizeof(sHeader));
		
		// WORD aligned PCX ScanLineSize
		DWORD uiPCXScanLineSize = sHeader.NumPlanes * sHeader.BPL;

		// DWORD aligned target DIB ScanLineSize 
		DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES((sHeader.X2 - sHeader.X1 + 1) * sHeader.NumPlanes * sHeader.BitPerPixel);
		DWORD uiDIBImageSize = uiDIBScanLineSize * (sHeader.Y2-sHeader.Y1+1);

		// Allocate memory for target DIB Header
		if ((sHeader.BitPerPixel == 1) && (sHeader.NumPlanes == 1)) // 1bit
		{
			m_pBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER) + (sizeof(RGBQUAD)*2)];
			if (m_pBMI == NULL)
				throw (int)PCX_E_NOMEM;
		}
		else if ((sHeader.BitPerPixel == 1) && (sHeader.NumPlanes == 4)) // 4bit
		{
			m_pBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER) + (sizeof(RGBQUAD)*16)];
			if (m_pBMI == NULL)
				throw (int)PCX_E_NOMEM;
		}
		else if ((sHeader.BitPerPixel == 8) && (sHeader.NumPlanes == 1)) // 8bit
		{ 
			m_pBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER) + (sizeof(RGBQUAD)*256)];
			if (m_pBMI == NULL)
				throw (int)PCX_E_NOMEM;
		}
		else if ((sHeader.BitPerPixel == 8) && (sHeader.NumPlanes == 3)) // 24bit
		{
			m_pBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER)];
			if (m_pBMI == NULL)
				throw (int)PCX_E_NOMEM;
		}
		else // other formats not supported
			throw (int)PCX_E_BADPCX;

		// Set the target DIB header information
		m_pBMI->bmiHeader.biSize           = sizeof(BITMAPINFOHEADER);
		m_pBMI->bmiHeader.biWidth          = sHeader.X2-sHeader.X1+1;
		m_pBMI->bmiHeader.biHeight         = sHeader.Y2-sHeader.Y1+1;
		m_pBMI->bmiHeader.biPlanes         = 1;
		m_pBMI->bmiHeader.biBitCount       = (WORD)(sHeader.BitPerPixel * sHeader.NumPlanes);
		m_pBMI->bmiHeader.biCompression    = BI_RGB;
		m_pBMI->bmiHeader.biSizeImage      = uiDIBImageSize;
		m_pBMI->bmiHeader.biXPelsPerMeter  = PIXPERMETER(sHeader.HRes);
		m_pBMI->bmiHeader.biYPelsPerMeter  = PIXPERMETER(sHeader.VRes);
		m_pBMI->bmiHeader.biClrUsed        = 0; // All colors are used
		m_pBMI->bmiHeader.biClrImportant   = 0; // All colors are important
		m_pColors = (RGBQUAD*)((LPBYTE)m_pBMI + (WORD)(m_pBMI->bmiHeader.biSize));
		m_dwImageSize = m_pBMI->bmiHeader.biSizeImage;

		// Init File Info
		m_FileInfo.m_nType = CFileInfo::PCX;
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

		// If only header wanted return now
		if (bOnlyHeader)
		{
			::VirtualFree((LPVOID)pabFileData, 0, MEM_RELEASE);
			return TRUE;
		}

		// Prepare a buffer large enough to hold the target DIB image pixels
		m_pBits = (LPBYTE)::VirtualAlloc(	NULL,
											uiDIBImageSize + SAFETY_BITALLOC_MARGIN,
											MEM_COMMIT,
											PAGE_READWRITE);
		if (m_pBits == NULL)
			throw (int)PCX_E_NOMEM;

		// Start Values
		long lDataPosStart = 0;
		long lDataPos = 0;
		long lPos = 128; // That's where the data begins
		long lEndOfPixels = m_FileInfo.m_dwFileSize - 1;
		if ((sHeader.BitPerPixel == 8) && (sHeader.NumPlanes == 1))
		{
			if (pabFileData[m_FileInfo.m_dwFileSize - 769] == 12) // Has Palette
				lEndOfPixels -= 769;
		}

		// Note:
		// I changed throw (int)PCX_E_BADPCX to break because many pcx files are corrupted,
		// better to see some scanlines than nothing!

		//////////////////
		// 1 bit Image  //
		//////////////////
		if ((sHeader.BitPerPixel == 1) && (sHeader.NumPlanes == 1))
		{
			for (int iY = 0 ; iY < (sHeader.Y2-sHeader.Y1+1) ; iY++)
			{
				if ((iY & 0x7) == 0)
				{
					if (pThread && pThread->DoExit())
						throw (int)PCX_THREADEXIT;
					DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, iY, sHeader.Y2-sHeader.Y1+1);
				}

				// Decompress the scan line	
				unsigned int iX;
				for (iX = 0 ; iX < sHeader.BPL ; )
				{
					if (lPos > lEndOfPixels)
						break;//throw (int)PCX_E_BADPCX;
					DWORD uiValue = pabFileData[lPos++];
					if ( uiValue >= 192 ) // 0xC0 = 192, two high bits are set => repeat
					{
						uiValue -= 192;                   // Repeat how many times?
						if (lPos > lEndOfPixels)
							break;//throw (int)PCX_E_BADPCX;
						BYTE Color = pabFileData[lPos++]; // What color?
						for (BYTE bRepeat=0 ; bRepeat < uiValue ; bRepeat++)
						{
							m_pBits[lDataPos++] = Color;	
							iX++;
						}
					}
					else
					{
						m_pBits[lDataPos++] = (BYTE)uiValue;
						iX++;
					}
				}
				// Pad the rest with zeros
				while(iX < uiDIBScanLineSize)
				{
					m_pBits[lDataPos++] = 0;
					iX++;
				}
			}

			DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
		}
		//////////////////
		// 4 bit Image  //
		//////////////////
		else if ((sHeader.BitPerPixel == 1) && (sHeader.NumPlanes == 4))
		{
			BYTE Color;
			// Prepare a buffer large enough to hold the decompressed PCX
			pPCXDecompr = (LPBYTE)::VirtualAlloc(NULL,
												(sHeader.Y2-sHeader.Y1+1) * uiPCXScanLineSize,
												MEM_COMMIT,
												PAGE_READWRITE);
			if (pPCXDecompr == NULL)
				throw (int)PCX_E_NOMEM;

			// 1. Decompress the PCX file
			while (lDataPos < (int)((sHeader.Y2-sHeader.Y1+1) * uiPCXScanLineSize))
			{
				if ((lDataPos % uiPCXScanLineSize) == 0)
				{
					if (pThread && pThread->DoExit())
						throw (int)PCX_THREADEXIT;
					DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, lDataPos, (int)((sHeader.Y2-sHeader.Y1+1) * uiPCXScanLineSize));
				}

				if (lPos > lEndOfPixels)
					break;//throw (int)PCX_E_BADPCX;
				DWORD uiValue = pabFileData[lPos++];
				if ( uiValue >= 192 ) // 0xC0 = 192, two high bits are set => repeat
				{
					uiValue -= 192;              // Repeat how many times?
					if (lPos > lEndOfPixels)
						break;//throw (int)PCX_E_BADPCX;
					Color = pabFileData[lPos++]; // What color?
					for (BYTE bRepeat=0 ; bRepeat < uiValue ; bRepeat++)
						pPCXDecompr[lDataPos++] = Color;	
				}
				else
					pPCXDecompr[lDataPos++] = (BYTE)uiValue;
			}

			// 2. Decode the Planes
			for (int iY = 0 ; iY < (sHeader.Y2-sHeader.Y1+1) ; iY++)
			{
				if ((iY & 0x7) == 0)
				{
					if (pThread && pThread->DoExit())
						throw (int)PCX_THREADEXIT;
					DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, iY, sHeader.Y2-sHeader.Y1+1);
				}

				lDataPos = 0;
				for (int iX = 0 ; iX < sHeader.BPL ; iX++)
				{
					for (int i = 0 ; i < 8 ; i++)
					{
						Color = (BYTE)((((pPCXDecompr[4*iY*sHeader.BPL + iX]  >> (7-i)) & 1) << 0) | // Red
									(((pPCXDecompr[(4*iY+1)*sHeader.BPL + iX] >> (7-i)) & 1) << 1) | // Green
									(((pPCXDecompr[(4*iY+2)*sHeader.BPL + iX] >> (7-i)) & 1) << 2) | // Blue
									(((pPCXDecompr[(4*iY+3)*sHeader.BPL + iX] >> (7-i)) & 1) << 3)); // Intensity
						if ((iX * 8 + i) < (sHeader.X2-sHeader.X1+1)) // Pixel check
						{
							if ((i % 2) == 0) // even nibble (pixel)
							{
								m_pBits[iY*uiDIBScanLineSize + lDataPos] = (BYTE)(Color << 4);
								lDataPos++;
							}
							else // odd nibble (pixel)
							{
								m_pBits[iY*uiDIBScanLineSize + lDataPos-1] = (BYTE)(m_pBits[iY*uiDIBScanLineSize + lDataPos-1] | Color);
							}
						}
					}
				}
				// Pad the rest with zeros
				while(lDataPos < (int)uiDIBScanLineSize)
				{
					m_pBits[iY*uiDIBScanLineSize + lDataPos] = 0;
					lDataPos++;
				}
			}

			// 3. Free
			::VirtualFree((LPVOID)pPCXDecompr, 0, MEM_RELEASE);
			pPCXDecompr = NULL;

			DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
		}
		/////////////////////////////
		// 8 bit and 24 bit Image  //
		/////////////////////////////
		else if(sHeader.BitPerPixel == 8)
		{
			for (int iY = 0 ; iY < (sHeader.Y2-sHeader.Y1+1) ; iY++)
			{	
				if ((iY & 0x7) == 0)
				{
					if (pThread && pThread->DoExit())
						throw (int)PCX_THREADEXIT;
					DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, iY, sHeader.Y2-sHeader.Y1+1);
				}

				int iX = 0;
				lDataPosStart = iY * uiDIBScanLineSize;

				// Decompress the scan line
				for (int iPlain = 0 ; iPlain < sHeader.NumPlanes ; iPlain++)
				{
					lDataPos = lDataPosStart + (sHeader.NumPlanes - iPlain - 1); 
					for (iX = 0 ; iX < sHeader.BPL ; )
					{
						if (lPos > lEndOfPixels)
							break;//throw (int)PCX_E_BADPCX;
						UINT uiValue = pabFileData[lPos++];
						if (uiValue >= 192) // 0xC0 = 192, two high bits are set => repeat
						{
							uiValue -= 192;                   // Repeat how many times?
							if (lPos > lEndOfPixels)
								break;//throw (int)PCX_E_BADPCX;
							BYTE Color = pabFileData[lPos++]; // What color?
							
							for (BYTE bRepeat=0 ; (bRepeat < uiValue) ; bRepeat++)
							{
								if (iX == sHeader.BPL) // The encoding may span multiple color planes of a single scan line!!!
								{
									uiValue = uiValue - bRepeat;
									uiValue += 192;
									lPos -= 2;
									pabFileData[lPos] = (BYTE)uiValue;
									break;
								}
								else
								{
									if (iX < (sHeader.X2-sHeader.X1+1))
									{
										m_pBits[lDataPos] = Color;
										lDataPos += sHeader.NumPlanes;
									}
									iX++;
								}
							}
						}
						else
						{
							if (iX < (sHeader.X2-sHeader.X1+1))
							{
							   m_pBits[lDataPos] = (BYTE)uiValue;
							   lDataPos += sHeader.NumPlanes;
							}
							iX++;
						}
					}
					ASSERT(lDataPos <= lDataPosStart + (sHeader.NumPlanes - iPlain - 1) + sHeader.NumPlanes * (sHeader.X2-sHeader.X1+1));
				}
				// Pad the rest with zeros
				for (unsigned int i = 0 ; (i + sHeader.NumPlanes * (sHeader.X2-sHeader.X1+1)) < uiDIBScanLineSize ; i++)
					m_pBits[lDataPos++] = 0;
			}

			DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
		}

		ASSERT(lDataPos <= (int)uiDIBImageSize);

		/////////////////////
		// Get the palette //
		/////////////////////
		if ((sHeader.BitPerPixel == 1) && (sHeader.NumPlanes == 1)) // 1bit
		{
			m_pColors[0].rgbRed       = 0;
			m_pColors[0].rgbGreen     = 0;
			m_pColors[0].rgbBlue      = 0;
			m_pColors[0].rgbReserved  = 0;
			m_pColors[1].rgbRed       = 255;
			m_pColors[1].rgbGreen     = 255;
			m_pColors[1].rgbBlue      = 255;
			m_pColors[1].rgbReserved  = 0;
		}
		else if ((sHeader.BitPerPixel == 1) && (sHeader.NumPlanes == 4)) // 4bit
		{
			int i = 0;
			for (short Entry=0 ; Entry < 16 ; Entry++)
			{
				m_pColors[Entry].rgbRed       = sHeader.ClrMap[i++];
				m_pColors[Entry].rgbGreen     = sHeader.ClrMap[i++];
				m_pColors[Entry].rgbBlue      = sHeader.ClrMap[i++];
				m_pColors[Entry].rgbReserved  = 0;
			}
		}
		else if ((sHeader.BitPerPixel == 8) && (sHeader.NumPlanes == 1)) // 8bit
		{
			// Now some empty scan line may follow -> count back from end of file 768+1 bytes
			// to reach the beginning of palette
			lPos = m_FileInfo.m_dwFileSize - 769;

			if ((sHeader.Version >= 5) && (pabFileData[lPos++] == 12))
			{
				for (short Entry=0 ; Entry < 256 ; Entry++)
				{
					m_pColors[Entry].rgbRed       = pabFileData[lPos++];
					m_pColors[Entry].rgbGreen     = pabFileData[lPos++];
					m_pColors[Entry].rgbBlue      = pabFileData[lPos++];
					m_pColors[Entry].rgbReserved  = 0;
				}
			}
			else
				FillHalftoneColors(m_pColors, GetNumColors());
		}

		// Create Palette from BMI
		CreatePaletteFromBMI();

		// Free
		::VirtualFree((LPVOID)pabFileData, 0, MEM_RELEASE);
		pabFileData = NULL;

		// Make it Top-Down:
		FlipTopDown();

		return TRUE;
	}
	catch (CFileException* e)
	{
		TCHAR szCause[255];
		CString str(_T("LoadPCX: "));
		e->GetErrorMessage(szCause, 255);
		str += szCause;
		str += _T("\n");
		TRACE(str);
		if (m_bShowMessageBoxOnError)
			::AfxMessageBox(str, MB_ICONSTOP);
		e->Delete();
		if (pabFileData)
			::VirtualFree((LPVOID)pabFileData, 0, MEM_RELEASE);
		if (m_pBMI)
		{
			delete [] m_pBMI;
			m_pBMI = NULL;
			m_pColors = NULL;
		}
		if (m_pBits)
		{
			::VirtualFree((LPVOID)m_pBits, 0, MEM_RELEASE);
			m_pBits = NULL;
		}
		if (pPCXDecompr)
			::VirtualFree((LPVOID)pPCXDecompr, 0, MEM_RELEASE);
		DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
		return FALSE;
	}
	catch (int error_code)
	{
		if (pabFileData)
			::VirtualFree((LPVOID)pabFileData, 0, MEM_RELEASE);
		if (m_pBMI)
		{
			delete [] m_pBMI;
			m_pBMI = NULL;
			m_pColors = NULL;
		}
		if (m_pBits)
		{
			::VirtualFree((LPVOID)m_pBits, 0, MEM_RELEASE);
			m_pBits = NULL;
		}
		if (pPCXDecompr)
			::VirtualFree((LPVOID)pPCXDecompr, 0, MEM_RELEASE);

		DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

		// Just Exit
		if (error_code == PCX_THREADEXIT)
			return FALSE;

		CString str;
#ifdef _DEBUG
		str.Format(_T("LoadPCX(%s):\n"), lpszPathName);
#endif
		switch (error_code)
		{
			case PCX_E_ZEROPATH :		str += _T("The file name is zero\n");
			break;
			case PCX_E_WRONGEXTENTION :	str += _T("The file extention is not .pcx or .dcx\n");
			break;
			case PCX_E_NOMEM :			str += _T("Could not alloc memory\n");
			break;
			case PCX_E_BADPCX :			str += _T("Corrupted or unsupported PCX\n");
			break;
			case PCX_E_READ :			str += _T("Couldn't read PCX file\n");
			break;
			case PCX_E_FILEEMPTY :		str += _T("File is empty\n");
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

BOOL CDib::SavePCX(	LPCTSTR lpszPathName,
					CWnd* pProgressWnd/*=NULL*/,
					BOOL bProgressSend/*=TRUE*/,
					CWorkerThread* pThread/*=NULL*/)
{
	// PCX header
	PCXHeader sHeader;
	DIB_INIT_PROGRESS;

	// Buffers
	LPBYTE buf =					NULL;
	LPBYTE palette =				NULL;
	LPBYTE redPlaneScanLine =		NULL;
	LPBYTE greenPlaneScanLine =		NULL;
	LPBYTE bluePlaneScanLine =		NULL;
	LPBYTE UncompressedScanLine =	NULL;
	LPBYTE CompressedScanLine =		NULL;

	try
	{
		CString sPathName(lpszPathName);
		if (sPathName.IsEmpty())
			throw (int)PCX_E_ZEROPATH;

		DWORD dwAttrib = ::GetFileAttributes(lpszPathName);
		if ((dwAttrib != 0xFFFFFFFF) && (dwAttrib & FILE_ATTRIBUTE_READONLY))
			throw (int)PCX_E_FILEREADONLY;

		if (!m_pBits)
		{
			if (!DibSectionToBits())
				throw (int)PCX_E_BADBMP;
		}

		if (!m_pBits || !m_pBMI)
			throw (int)PCX_E_BADBMP;

		if (IsCompressed())
#ifndef _WIN32_WCE
			if (!Decompress(GetBitCount())) // Decompress
				throw (int)PCX_E_BADBMP;
#else
			throw (int)PCX_E_BADBMP;
#endif

		CFile file(lpszPathName,	CFile::modeCreate |
									CFile::modeWrite  |
									CFile::shareExclusive);

		// Create Header:
		sHeader.ID = PCX_HEADER_MARKER;
		sHeader.Version = 5; // Only 24 bit supported
		sHeader.Encoding = 1;
		sHeader.X1 = 0;
		sHeader.Y1 = 0;
		sHeader.X2 = (WORD)(GetWidth() - 1);	// X2 = XSize - 1
		sHeader.Y2 = (WORD)(GetHeight() - 1);	// Y2 = YSize - 1
		sHeader.HRes = DPI(m_pBMI->bmiHeader.biXPelsPerMeter);
		sHeader.VRes = DPI(m_pBMI->bmiHeader.biYPelsPerMeter);
		sHeader.Reserved1 = 0;
		sHeader.Pal_t = 1;

		DWORD uiOutputDIBScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * 24);
		DWORD uiInputDIBScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * GetBitCount());

		// In the pcx file the scan lines are top / down 
		// In a DIB the scan lines are stored bottom / up
		LPBYTE lpBits = m_pBits;
		lpBits += uiInputDIBScanLineSize*(GetHeight()-1);

		switch(GetBitCount())
		{
		case 1:
		{
			sHeader.BitPerPixel = 1;
			sHeader.NumPlanes = 1;
			sHeader.BPL = (WORD)(WALIGNEDWIDTHBYTES(GetWidth() * sHeader.BitPerPixel)); // Bytes Per Scanline Plane uncompressed (is an even number=WORD aligned!)
			file.Write((LPBYTE)&sHeader, sizeof(sHeader));
			// WORD aligned PCX ScanLineSize
			DWORD uiUncompressedPCXScanLineSize = sHeader.NumPlanes * sHeader.BPL;

			UncompressedScanLine =	new BYTE[uiUncompressedPCXScanLineSize];
			if (!UncompressedScanLine)
				throw (int)PCX_E_NOMEM;
			CompressedScanLine =	new BYTE[2*uiUncompressedPCXScanLineSize]; // Compressed may be 2xUncompressed in the worst case!
			if (!CompressedScanLine)
				throw (int)PCX_E_NOMEM;
			for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
			{
				if ((CurLine & 0x7) == 0)
				{
					if (pThread && pThread->DoExit())
						throw (int)PCX_THREADEXIT;
					DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, CurLine, GetHeight());
				}

				if (m_pColors[0].rgbRed		< m_pColors[1].rgbRed	&&
					m_pColors[0].rgbGreen	< m_pColors[1].rgbGreen	&&
					m_pColors[0].rgbBlue	< m_pColors[1].rgbBlue)
				{ // Min is Black
					memcpy((void*)UncompressedScanLine, (void*)lpBits, uiUncompressedPCXScanLineSize);
				}
				else // Min is White
				{
					memcpy((void*)UncompressedScanLine, (void*)lpBits, uiUncompressedPCXScanLineSize);
					for(unsigned int i = 0 ; i < uiUncompressedPCXScanLineSize ; i++)
						UncompressedScanLine[i] = (BYTE)(~(UncompressedScanLine[i])); // Complement the bits
				}

				// RLE encode UncompressedScanLine:
				BYTE uiValue;
				unsigned int repeat;
				unsigned int UncomprIndex = 0;
				unsigned int ComprIndex = 0;

				while (UncomprIndex < uiUncompressedPCXScanLineSize)
				{
					repeat = 1;
					uiValue = UncompressedScanLine[UncomprIndex];
					while (((UncomprIndex + repeat) < uiUncompressedPCXScanLineSize) &&
							(UncompressedScanLine[UncomprIndex + repeat] == uiValue))
						repeat++;

					UncomprIndex += repeat;
					
					while ((repeat + 192) > 255) // Max repeat is 63, if it's more split!
					{
						CompressedScanLine[ComprIndex++] = (BYTE)(63 + 192); // =255
						CompressedScanLine[ComprIndex++] = uiValue;
						repeat -= 63;
					}
					if (repeat > 1)
					{
						CompressedScanLine[ComprIndex++] = (BYTE)(repeat + 192);
						CompressedScanLine[ComprIndex++] = uiValue;
					}
					else if (repeat == 1)
					{
						if (uiValue < 192)
						{
							CompressedScanLine[ComprIndex++] = uiValue;
						}
						else
						{
							CompressedScanLine[ComprIndex++] = (BYTE)(repeat + 192); // = 193
							CompressedScanLine[ComprIndex++] = uiValue;
						}
					}
				}
				file.Write(CompressedScanLine, ComprIndex);
				lpBits -= uiInputDIBScanLineSize;
			}
			delete [] UncompressedScanLine;
			UncompressedScanLine = NULL;
			delete [] CompressedScanLine;
			CompressedScanLine = NULL;
			break;
		}
		case 4: // 4bit to 8bit conversion
		{
			sHeader.BitPerPixel = 8;
			sHeader.NumPlanes = 1;
			sHeader.BPL = (WORD)(WALIGNEDWIDTHBYTES(GetWidth() * sHeader.BitPerPixel)); // Bytes Per Scanline Plane uncompressed (is an even number=WORD aligned!)
			file.Write((LPBYTE)&sHeader, sizeof(sHeader));
			// WORD aligned PCX ScanLineSize
			DWORD uiUncompressedPCXScanLineSize = sHeader.NumPlanes * sHeader.BPL;

			UncompressedScanLine =	new BYTE[uiUncompressedPCXScanLineSize];
			if (!UncompressedScanLine)
				throw (int)PCX_E_NOMEM;
			CompressedScanLine =	new BYTE[2*uiUncompressedPCXScanLineSize]; // Compressed may be 2xUncompressed in the worst case!
			if (!CompressedScanLine)
				throw (int)PCX_E_NOMEM;
			int n;
			for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
			{
				if ((CurLine & 0x7) == 0)
				{
					if (pThread && pThread->DoExit())
						throw (int)PCX_THREADEXIT;
					DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, CurLine, GetHeight());
				}

				n = 0;
				unsigned int i;
				for(i = 0 ; i < GetWidth()/2; i++)
				{
					UncompressedScanLine[n++] = (BYTE)((0xF0 & lpBits[i]) >> 4);
					UncompressedScanLine[n++] = (BYTE)(0x0F & lpBits[i]);
				}
				// odd number of pixels
				if (GetWidth() & 1)
					UncompressedScanLine[n++] = (BYTE)((0xF0 & lpBits[i]) >> 4);

				// RLE encode UncompressedScanLine:
				BYTE uiValue;
				unsigned int repeat;
				unsigned int UncomprIndex = 0;
				unsigned int ComprIndex = 0;

				while(UncomprIndex < uiUncompressedPCXScanLineSize)
				{
					repeat = 1;
					uiValue = UncompressedScanLine[UncomprIndex];
					while ((UncompressedScanLine[UncomprIndex + repeat] == uiValue) &&
							((UncomprIndex + repeat) < uiUncompressedPCXScanLineSize))
						repeat++;

					UncomprIndex += repeat;
					
					while ((repeat + 192) > 255) // Max repeat is 63, if it's more split!
					{
						CompressedScanLine[ComprIndex++] = (BYTE)(63 + 192); // =255
						CompressedScanLine[ComprIndex++] = uiValue;
						repeat -= 63;
					}
					if (repeat > 1)
					{
						CompressedScanLine[ComprIndex++] = (BYTE)(repeat + 192);
						CompressedScanLine[ComprIndex++] = uiValue;
					}
					else if (repeat == 1)
					{
						if (uiValue < 192)
						{
							CompressedScanLine[ComprIndex++] = uiValue;
						}
						else
						{
							CompressedScanLine[ComprIndex++] = (BYTE)(repeat + 192); // = 193
							CompressedScanLine[ComprIndex++] = uiValue;
						}
					}
				}
				file.Write(CompressedScanLine, ComprIndex);
				lpBits -= uiInputDIBScanLineSize;
			}
			delete [] UncompressedScanLine;
			UncompressedScanLine = NULL;
			delete [] CompressedScanLine;
			CompressedScanLine = NULL;

			palette = new BYTE[769];
			if (!palette)
				throw (int)PCX_E_NOMEM;
			palette[0] = 12; // Palette Marker
			for (WORD i = 0 ; i < GetNumColors() ; i++)
			{
				palette[3*i+1] = m_pColors[i].rgbRed;
				palette[3*i+2] = m_pColors[i].rgbGreen;
				palette[3*i+3] = m_pColors[i].rgbBlue;
			}
			file.Write(palette, 769);
			delete [] palette;
			palette = NULL;

			break;		
		}
		case 8:
		{
			sHeader.BitPerPixel = 8;
			sHeader.NumPlanes = 1;
			sHeader.BPL = (WORD)(WALIGNEDWIDTHBYTES(GetWidth() * sHeader.BitPerPixel)); // Bytes Per Scanline Plane uncompressed (is an even number=WORD aligned!)
			file.Write((LPBYTE)&sHeader, sizeof(sHeader));
			// WORD aligned PCX ScanLineSize
			DWORD uiUncompressedPCXScanLineSize = sHeader.NumPlanes * sHeader.BPL;

			UncompressedScanLine =	new BYTE[uiUncompressedPCXScanLineSize];
			if (!UncompressedScanLine)
				throw (int)PCX_E_NOMEM;
			CompressedScanLine =	new BYTE[2*uiUncompressedPCXScanLineSize]; // Compressed may be 2xUncompressed in the worst case!
			if (!CompressedScanLine)
				throw (int)PCX_E_NOMEM;
			for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
			{
				if ((CurLine & 0x7) == 0)
				{
					if (pThread && pThread->DoExit())
						throw (int)PCX_THREADEXIT;
					DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, CurLine, GetHeight());
				}

				memcpy((void*)UncompressedScanLine, (void*)lpBits, uiUncompressedPCXScanLineSize);

				// RLE encode UncompressedScanLine:
				BYTE uiValue;
				unsigned int repeat;
				unsigned int UncomprIndex = 0;
				unsigned int ComprIndex = 0;

				while(UncomprIndex < uiUncompressedPCXScanLineSize)
				{
					repeat = 1;
					uiValue = UncompressedScanLine[UncomprIndex];
					while ((UncompressedScanLine[UncomprIndex + repeat] == uiValue) &&
							((UncomprIndex + repeat) < uiUncompressedPCXScanLineSize))
						repeat++;

					UncomprIndex += repeat;
					
					while ((repeat + 192) > 255) // Max repeat is 63, if it's more split!
					{
						CompressedScanLine[ComprIndex++] = (BYTE)(63 + 192); // =255
						CompressedScanLine[ComprIndex++] = uiValue;
						repeat -= 63;
					}
					if (repeat > 1)
					{
						CompressedScanLine[ComprIndex++] = (BYTE)(repeat + 192);
						CompressedScanLine[ComprIndex++] = uiValue;
					}
					else if (repeat == 1)
					{
						if (uiValue < 192)
						{
							CompressedScanLine[ComprIndex++] = uiValue;
						}
						else
						{
							CompressedScanLine[ComprIndex++] = (BYTE)(repeat + 192); // = 193
							CompressedScanLine[ComprIndex++] = uiValue;
						}
					}
				}
				file.Write(CompressedScanLine, ComprIndex);
				lpBits -= uiInputDIBScanLineSize;
			}
			delete [] UncompressedScanLine;
			UncompressedScanLine = NULL;
			delete [] CompressedScanLine;
			CompressedScanLine = NULL;

			palette = new BYTE[769];
			if (!palette)
				throw (int)PCX_E_NOMEM;
			palette[0] = 12; // Palette Marker
			for (WORD i = 0 ; i < GetNumColors() ; i++)
			{
				palette[3*i+1] = m_pColors[i].rgbRed;
				palette[3*i+2] = m_pColors[i].rgbGreen;
				palette[3*i+3] = m_pColors[i].rgbBlue;
			}
			file.Write(palette, 769);
			delete [] palette;
			palette = NULL;

			break;
		}
		case 16: // 16bit to 24bit conversion
		{
			sHeader.BitPerPixel = 8;
			sHeader.NumPlanes = 3;
			sHeader.BPL = (WORD)(WALIGNEDWIDTHBYTES(GetWidth() * sHeader.BitPerPixel)); // Bytes Per Scanline Plane uncompressed (is an even number=WORD aligned!)
			file.Write((LPBYTE)&sHeader, sizeof(sHeader));
			// WORD aligned PCX ScanLineSize
			DWORD uiUncompressedPCXScanLineSize = sHeader.NumPlanes * sHeader.BPL;

			buf =					new BYTE[uiOutputDIBScanLineSize];
			if (!buf)
				throw (int)PCX_E_NOMEM;
			redPlaneScanLine =		new BYTE[sHeader.BPL];
			if (!redPlaneScanLine)
				throw (int)PCX_E_NOMEM;
			greenPlaneScanLine =	new BYTE[sHeader.BPL];
			if (!greenPlaneScanLine)
				throw (int)PCX_E_NOMEM;
			bluePlaneScanLine =		new BYTE[sHeader.BPL];
			if (!bluePlaneScanLine)
				throw (int)PCX_E_NOMEM;
			UncompressedScanLine =	new BYTE[uiUncompressedPCXScanLineSize];
			if (!UncompressedScanLine)
				throw (int)PCX_E_NOMEM;
			CompressedScanLine =	new BYTE[2*uiUncompressedPCXScanLineSize]; // Compressed may be 2xUncompressed in the worst case!
			if (!CompressedScanLine)
				throw (int)PCX_E_NOMEM;
			int n;
			for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
			{
				if ((CurLine & 0x7) == 0)
				{
					if (pThread && pThread->DoExit())
						throw (int)PCX_THREADEXIT;
					DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, CurLine, GetHeight());
				}

				n = 0;
				unsigned int i;
				for(i = 0 ; i < GetWidth() ; i++)
				{
					DIB16ToRGB(((WORD*)lpBits)[i], &(buf[n+2]), &(buf[n+1]), &(buf[n]));
					n += 3;
				}

				for (i = 0 ; i < GetWidth() ; i++)
				{
					redPlaneScanLine[i]		= (BYTE)buf[3*i+2];	// Red
					greenPlaneScanLine[i]	= (BYTE)buf[3*i+1];	// Green
					bluePlaneScanLine[i]	= (BYTE)buf[3*i];	// Blue
				}
				if (GetWidth() < sHeader.BPL) // WORD Aligned -> Pad with zero
				{
					redPlaneScanLine[sHeader.BPL-1]		= (BYTE)0;	// Red
					greenPlaneScanLine[sHeader.BPL-1]	= (BYTE)0;	// Green
					bluePlaneScanLine[sHeader.BPL-1]	= (BYTE)0;	// Blue
				}
			
				LPBYTE lpBuf = UncompressedScanLine; 
				memcpy((void*)lpBuf, (void*)redPlaneScanLine, sHeader.BPL);
				lpBuf += sHeader.BPL;
				memcpy((void*)lpBuf, (void*)greenPlaneScanLine, sHeader.BPL);
				lpBuf += sHeader.BPL;
				memcpy((void*)lpBuf, (void*)bluePlaneScanLine, sHeader.BPL);

				// RLE encode UncompressedScanLine:
				BYTE uiValue;
				unsigned int repeat;
				unsigned int UncomprIndex = 0;
				unsigned int ComprIndex = 0;

				while(UncomprIndex < uiUncompressedPCXScanLineSize)
				{
					repeat = 1;
					uiValue = UncompressedScanLine[UncomprIndex];
					while ((UncompressedScanLine[UncomprIndex + repeat] == uiValue) &&
							((UncomprIndex + repeat) < uiUncompressedPCXScanLineSize))
						repeat++;

					UncomprIndex += repeat;
					
					while ((repeat + 192) > 255) // Max repeat is 63, if it's more split!
					{
						CompressedScanLine[ComprIndex++] = (BYTE)(63 + 192); // =255
						CompressedScanLine[ComprIndex++] = uiValue;
						repeat -= 63;
					}
					if (repeat > 1)
					{
						CompressedScanLine[ComprIndex++] = (BYTE)(repeat + 192);
						CompressedScanLine[ComprIndex++] = uiValue;
					}
					else if (repeat == 1)
					{
						if (uiValue < 192)
						{
							CompressedScanLine[ComprIndex++] = uiValue;
						}
						else
						{
							CompressedScanLine[ComprIndex++] = (BYTE)(repeat + 192); // = 193
							CompressedScanLine[ComprIndex++] = uiValue;
						}
					}
				}
				file.Write(CompressedScanLine, ComprIndex);
				lpBits -= uiInputDIBScanLineSize;
			}
			delete [] buf;
			buf = NULL;
			delete [] redPlaneScanLine;
			redPlaneScanLine = NULL;
			delete [] greenPlaneScanLine;
			greenPlaneScanLine = NULL;
			delete [] bluePlaneScanLine;
			bluePlaneScanLine = NULL;
			delete [] UncompressedScanLine;
			UncompressedScanLine = NULL;
			delete [] CompressedScanLine;
			CompressedScanLine = NULL;
			break;	
		}
		case 24:
		{
			sHeader.BitPerPixel = 8;
			sHeader.NumPlanes = 3;
			sHeader.BPL = (WORD)(WALIGNEDWIDTHBYTES(GetWidth() * sHeader.BitPerPixel)); // Bytes Per Scanline Plane uncompressed (is an even number=WORD aligned!)
			file.Write((LPBYTE)&sHeader, sizeof(sHeader));
			// WORD aligned PCX ScanLineSize
			DWORD uiUncompressedPCXScanLineSize = sHeader.NumPlanes * sHeader.BPL;

			redPlaneScanLine =		new BYTE[sHeader.BPL];
			if (!redPlaneScanLine)
				throw (int)PCX_E_NOMEM;
			greenPlaneScanLine =	new BYTE[sHeader.BPL];
			if (!greenPlaneScanLine)
				throw (int)PCX_E_NOMEM;
			bluePlaneScanLine =		new BYTE[sHeader.BPL];
			if (!bluePlaneScanLine)
				throw (int)PCX_E_NOMEM;
			UncompressedScanLine =	new BYTE[uiUncompressedPCXScanLineSize];
			if (!UncompressedScanLine)
				throw (int)PCX_E_NOMEM;
			CompressedScanLine =	new BYTE[2*uiUncompressedPCXScanLineSize]; // Compressed may be 2xUncompressed in the worst case!
			if (!CompressedScanLine)
				throw (int)PCX_E_NOMEM;
			for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
			{
				if ((CurLine & 0x7) == 0)
				{
					if (pThread && pThread->DoExit())
						throw (int)PCX_THREADEXIT;
					DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, CurLine, GetHeight());
				}

				for (unsigned int i = 0 ; i < GetWidth() ; i++)
				{
					redPlaneScanLine[i]		= (BYTE)lpBits[3*i+2];	// Red
					greenPlaneScanLine[i]	= (BYTE)lpBits[3*i+1];	// Green
					bluePlaneScanLine[i]	= (BYTE)lpBits[3*i];	// Blue
				}
				if (GetWidth() < sHeader.BPL) // WORD Aligned -> Pad with zero
				{
					redPlaneScanLine[sHeader.BPL-1]		= (BYTE)0;	// Red
					greenPlaneScanLine[sHeader.BPL-1]	= (BYTE)0;	// Green
					bluePlaneScanLine[sHeader.BPL-1]	= (BYTE)0;	// Blue
				}
			
				LPBYTE lpBuf = UncompressedScanLine; 
				memcpy((void*)lpBuf, (void*)redPlaneScanLine, sHeader.BPL);
				lpBuf += sHeader.BPL;
				memcpy((void*)lpBuf, (void*)greenPlaneScanLine, sHeader.BPL);
				lpBuf += sHeader.BPL;
				memcpy((void*)lpBuf, (void*)bluePlaneScanLine, sHeader.BPL);

				// RLE encode UncompressedScanLine:
				BYTE uiValue;
				unsigned int repeat;
				unsigned int UncomprIndex = 0;
				unsigned int ComprIndex = 0;

				while(UncomprIndex < uiUncompressedPCXScanLineSize)
				{
					repeat = 1;
					uiValue = UncompressedScanLine[UncomprIndex];
					while ((UncompressedScanLine[UncomprIndex + repeat] == uiValue) &&
							((UncomprIndex + repeat) < uiUncompressedPCXScanLineSize))
						repeat++;

					UncomprIndex += repeat;
					
					while ((repeat + 192) > 255) // Max repeat is 63, if it's more split!
					{
						CompressedScanLine[ComprIndex++] = (BYTE)(63 + 192); // =255
						CompressedScanLine[ComprIndex++] = uiValue;
						repeat -= 63;
					}
					if (repeat > 1)
					{
						CompressedScanLine[ComprIndex++] = (BYTE)(repeat + 192);
						CompressedScanLine[ComprIndex++] = uiValue;
					}
					else if (repeat == 1)
					{
						if (uiValue < 192)
						{
							CompressedScanLine[ComprIndex++] = uiValue;
						}
						else
						{
							CompressedScanLine[ComprIndex++] = (BYTE)(repeat + 192); // = 193
							CompressedScanLine[ComprIndex++] = uiValue;
						}
					}
				}
				file.Write(CompressedScanLine, ComprIndex);
				lpBits -= uiInputDIBScanLineSize;
			}
			delete [] redPlaneScanLine;
			redPlaneScanLine = NULL;
			delete [] greenPlaneScanLine;
			greenPlaneScanLine = NULL;
			delete [] bluePlaneScanLine;
			bluePlaneScanLine = NULL;
			delete [] UncompressedScanLine;
			UncompressedScanLine = NULL;
			delete [] CompressedScanLine;
			CompressedScanLine = NULL;
			break;
		}
		case 32: // 32bit to 24bit conversion
		{
			sHeader.BitPerPixel = 8;
			sHeader.NumPlanes = 3;
			sHeader.BPL = (WORD)(WALIGNEDWIDTHBYTES(GetWidth() * sHeader.BitPerPixel)); // Bytes Per Scanline Plane uncompressed (is an even number=WORD aligned!)
			file.Write((LPBYTE)&sHeader, sizeof(sHeader));
			// WORD aligned PCX ScanLineSize
			DWORD uiUncompressedPCXScanLineSize = sHeader.NumPlanes * sHeader.BPL;

			buf =					new BYTE[uiOutputDIBScanLineSize];
			if (!buf)
				throw (int)PCX_E_NOMEM;
			redPlaneScanLine =		new BYTE[sHeader.BPL];
			if (!redPlaneScanLine)
				throw (int)PCX_E_NOMEM;
			greenPlaneScanLine =	new BYTE[sHeader.BPL];
			if (!greenPlaneScanLine)
				throw (int)PCX_E_NOMEM;
			bluePlaneScanLine =		new BYTE[sHeader.BPL];
			if (!bluePlaneScanLine)
				throw (int)PCX_E_NOMEM;
			UncompressedScanLine =	new BYTE[uiUncompressedPCXScanLineSize];
			if (!UncompressedScanLine)
				throw (int)PCX_E_NOMEM;
			CompressedScanLine =	new BYTE[2*uiUncompressedPCXScanLineSize]; // Compressed may be 2xUncompressed in the worst case!
			if (!CompressedScanLine)
				throw (int)PCX_E_NOMEM;
			int n;
			for (unsigned int CurLine = 0 ; CurLine < GetHeight() ; CurLine++)
			{
				if ((CurLine & 0x7) == 0)
				{
					if (pThread && pThread->DoExit())
						throw (int)PCX_THREADEXIT;
					DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, CurLine, GetHeight());
				}

				n = 0;
				unsigned int i;
				for(i = 0 ; i < GetWidth() ; i++)
				{
					DIB32ToRGB(((DWORD*)lpBits)[i], &(buf[n+2]), &(buf[n+1]), &(buf[n]));
					n += 3;
				}

				for (i = 0 ; i < GetWidth() ; i++)
				{
					redPlaneScanLine[i]		= (BYTE)buf[3*i+2];	// Red
					greenPlaneScanLine[i]	= (BYTE)buf[3*i+1];	// Green
					bluePlaneScanLine[i]	= (BYTE)buf[3*i];	// Blue
				}
				if (GetWidth() < sHeader.BPL) // WORD Aligned -> Pad with zero
				{
					redPlaneScanLine[sHeader.BPL-1]		= (BYTE)0;	// Red
					greenPlaneScanLine[sHeader.BPL-1]	= (BYTE)0;	// Green
					bluePlaneScanLine[sHeader.BPL-1]	= (BYTE)0;	// Blue
				}
			
				LPBYTE lpBuf = UncompressedScanLine; 
				memcpy((void*)lpBuf, (void*)redPlaneScanLine, sHeader.BPL);
				lpBuf += sHeader.BPL;
				memcpy((void*)lpBuf, (void*)greenPlaneScanLine, sHeader.BPL);
				lpBuf += sHeader.BPL;
				memcpy((void*)lpBuf, (void*)bluePlaneScanLine, sHeader.BPL);

				// RLE encode UncompressedScanLine:
				BYTE uiValue;
				unsigned int repeat;
				unsigned int UncomprIndex = 0;
				unsigned int ComprIndex = 0;

				while(UncomprIndex < uiUncompressedPCXScanLineSize)
				{
					repeat = 1;
					uiValue = UncompressedScanLine[UncomprIndex];
					while ((UncompressedScanLine[UncomprIndex + repeat] == uiValue) &&
							((UncomprIndex + repeat) < uiUncompressedPCXScanLineSize))
						repeat++;

					UncomprIndex += repeat;
					
					while ((repeat + 192) > 255) // Max repeat is 63, if it's more split!
					{
						CompressedScanLine[ComprIndex++] = (BYTE)(63 + 192); // =255
						CompressedScanLine[ComprIndex++] = uiValue;
						repeat -= 63;
					}
					if (repeat > 1)
					{
						CompressedScanLine[ComprIndex++] = (BYTE)(repeat + 192);
						CompressedScanLine[ComprIndex++] = uiValue;
					}
					else if (repeat == 1)
					{
						if (uiValue < 192)
						{
							CompressedScanLine[ComprIndex++] = uiValue;
						}
						else
						{
							CompressedScanLine[ComprIndex++] = (BYTE)(repeat + 192); // = 193
							CompressedScanLine[ComprIndex++] = uiValue;
						}
					}
				}
				file.Write(CompressedScanLine, ComprIndex);
				lpBits -= uiInputDIBScanLineSize;
			}
			delete [] buf;
			buf = NULL;
			delete [] redPlaneScanLine;
			redPlaneScanLine = NULL;
			delete [] greenPlaneScanLine;
			greenPlaneScanLine = NULL;
			delete [] bluePlaneScanLine;
			bluePlaneScanLine = NULL;
			delete [] UncompressedScanLine;
			UncompressedScanLine = NULL;
			delete [] CompressedScanLine;
			CompressedScanLine = NULL;
			break;
		}
		default:
			break;
		}

		DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

		file.Close();
		return TRUE;
	}
	catch (CFileException* e)
	{
		// Free
		if (buf)
			delete [] buf;
		if (palette)
			delete [] palette;
		if (redPlaneScanLine)
			delete [] redPlaneScanLine;
		if (greenPlaneScanLine)	
			delete [] greenPlaneScanLine;
		if (bluePlaneScanLine)	
			delete [] bluePlaneScanLine;
		if (UncompressedScanLine)	
			delete [] UncompressedScanLine;
		if (CompressedScanLine)
			delete [] CompressedScanLine;

		TCHAR szCause[255];
		CString str(_T("SavePCX: "));
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
		// Free
		if (buf)
			delete [] buf;
		if (palette)
			delete [] palette;
		if (redPlaneScanLine)
			delete [] redPlaneScanLine;
		if (greenPlaneScanLine)	
			delete [] greenPlaneScanLine;
		if (bluePlaneScanLine)	
			delete [] bluePlaneScanLine;
		if (UncompressedScanLine)	
			delete [] UncompressedScanLine;
		if (CompressedScanLine)
			delete [] CompressedScanLine;

		DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

		// Just Exit
		if (error_code == PCX_THREADEXIT)
			return FALSE;

		CString str;
#ifdef _DEBUG
		str.Format(_T("SavePCX(%s):\n"), lpszPathName);
#endif
		switch (error_code)
		{
			case PCX_E_ZEROPATH :		str += _T("The file name is zero\n");
			break;
			case PCX_E_FILEREADONLY :	str += _T("The file is read only\n");
			break;
			case PCX_E_NOMEM :			str += _T("Could not alloc memory\n");
			break;
			case PCX_E_BADBMP :			str += _T("Corrupted or unsupported DIB\n");
			break;
			case PCX_E_WRITE :			str += _T("Couldn't write PCX file\n");
			break;
			case PCX_E_FILEEMPTY :		str += _T("File is empty\n");
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

#endif