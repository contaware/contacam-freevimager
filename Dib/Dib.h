#ifndef _INC_DIB
#define _INC_DIB

extern "C"
{
#ifdef SUPPORT_LIBPNG
#include "png.h"
#endif
#ifdef SUPPORT_LIBTIFF
#include "tiffio.h"
#include "tiffiop.h"
#endif
#ifdef SUPPORT_LIBJPEG
#include "jpeglib.h"
#include "transupp.h"	// Support routines for lossless transformations
#endif
#ifdef SUPPORT_GIFLIB
#include "gif_lib.h"
#endif
}

#include <afxtempl.h>	// For CArray
#include <afxole.h>		// For LoadDibSectionEx()
#include "vfw.h"		// For DrawDib Functions
#include "RgbToYuv.h"
#include "YuvToRgb.h"
#include "..\WorkerThread.h"
#include "..\Helpers.h"
#include "ColorSpace.h"
#include "..\TraceLogFile.h"

// Bitmap info for BI_BITFIELDS
typedef struct {
		DWORD      biSize;
		LONG       biWidth;
		LONG       biHeight;
		WORD       biPlanes;
		WORD       biBitCount;
		DWORD      biCompression;
		DWORD      biSizeImage;
		LONG       biXPelsPerMeter;
		LONG       biYPelsPerMeter;
		DWORD      biClrUsed;
		DWORD      biClrImportant;
		DWORD      biRedMask;
		DWORD      biGreenMask;
		DWORD      biBlueMask;
} BITMAPINFOBITFIELDS, FAR *LPBITMAPINFOBITFIELDS, *PBITMAPINFOBITFIELDS;

// Bitmap info full struct
typedef struct {
		BITMAPINFOHEADER    bmiHeader;
		RGBQUAD             bmiColors[256];
} BITMAPINFOFULL, FAR *LPBITMAPINFOFULL, *PBITMAPINFOFULL;

#if (WINVER < 0x0500)
typedef struct {
        DWORD        bV5Size;
        LONG         bV5Width;
        LONG         bV5Height;
        WORD         bV5Planes;
        WORD         bV5BitCount;
        DWORD        bV5Compression;
        DWORD        bV5SizeImage;
        LONG         bV5XPelsPerMeter;
        LONG         bV5YPelsPerMeter;
        DWORD        bV5ClrUsed;
        DWORD        bV5ClrImportant;
        DWORD        bV5RedMask;
        DWORD        bV5GreenMask;
        DWORD        bV5BlueMask;
        DWORD        bV5AlphaMask;
        DWORD        bV5CSType;
        CIEXYZTRIPLE bV5Endpoints;
        DWORD        bV5GammaRed;
        DWORD        bV5GammaGreen;
        DWORD        bV5GammaBlue;
        DWORD        bV5Intent;
        DWORD        bV5ProfileData;
        DWORD        bV5ProfileSize;
        DWORD        bV5Reserved;
} BITMAPV5HEADER, FAR *LPBITMAPV5HEADER, *PBITMAPV5HEADER;

// Values for bV5CSType
#define PROFILE_LINKED					'LINK'
#define PROFILE_EMBEDDED				'MBED'
#endif

#ifndef BI_RGB
#define BI_RGB							0L
#endif

#ifndef BI_RLE8
#define BI_RLE8							1L
#endif

#ifndef BI_RLE4
#define BI_RLE4							2L
#endif

#ifndef BI_BITFIELDS
#define BI_BITFIELDS					3L
#endif

#ifndef BI_JPEG
#define BI_JPEG							4L
#endif

#ifndef BI_PNG
#define BI_PNG							5L
#endif

#ifndef BI_BGR16
#define BI_BGR16						mmioFOURCC('B','G','R',16)
#endif

#ifndef BI_BGR15
#define BI_BGR15						mmioFOURCC('B','G','R',15)
#endif

#ifndef BI_RGB16
#define BI_RGB16						mmioFOURCC('R','G','B',16)
#endif

#ifndef BI_RGB15
#define BI_RGB15						mmioFOURCC('R','G','B',15)
#endif

// If Dpi Not Available From File, use this default value
// There are two standards for dpi in the world:
// - The typographical standard (72 dpi) used by Macintosh computers.
//   This makes it very easy to convert from pixels to points, because they're the same.
//   This means that if you have the correct monitor size,
//   your screen image will be the same size as what you print.
//   There's 72 points in an inch, there's 72 pixels in an inch. Very nice!
//
// - The Microsoft standard (96 dpi).
#ifndef DEFAULT_DPI
#define DEFAULT_DPI						96
#endif

// Paste enhanced metafile objects rendering them
// with a maximum of the given dpi
#define PASTE_MAX_DPI					600

// Had a conflict with:
// const WORD PALVERSION = 0x300;
// of the directx_includes\winutil.h
// file.
// -> I use the _DEFINE suffix
#define PALVERSION_DEFINE				0x300

// Header Markers
#define DIB_HEADER_MARKER				((WORD) ('M' << 8) | 'B')
#define PCX_HEADER_MARKER				0x0A
#define GIF_HEADER_MARKER				0x38464947

// File Load Checks
#define SECURITY_MAX_ALLOWED_WIDTH		30000
#define SECURITY_MAX_ALLOWED_HEIGHT		30000
#define SECURITY_MAX_PIX_AREA			(20000 * 20000)
#define SECURITY_MAX_RATIO				10000

// Contrast Calculation
#define REASONABLE_SIZE_CONTRAST_AVGBRIGHT_CALC		200

// The following amount is allocated in excess for all m_pBits
// allocated through BIGALLOC.
// I started introducing that because XVID 1.1 converting from RGB24
// to YUV in the MMX routine is reading after the Dib size!
// (found crashing with 640x480 images)
#define SAFETY_BITALLOC_MARGIN			4096 // Bytes

// Flood fill initial stack size
#define FLOODFILL_INITSTACKSIZE			1024 // 1024 Ints = 4096 Bytes

// Default Jpeg Compression Quality
#ifndef DEFAULT_JPEGCOMPRESSION
#define DEFAULT_JPEGCOMPRESSION			80
#endif

// Error Codes
#define BMP_E_ZEROPATH					0   // The file name is zero
#define BMP_E_WRONGEXTENTION			1	// Wrong File Extention
#define BMP_E_FILEREADONLY				2	// The file is read only
#define BMP_E_NOMEM 					3   // Could not alloc memory
#define BMP_E_BADBMP					4	// Corrupted or unsupported DIB
#define BMP_E_READ						5	// Couldn't read BMP file
#define BMP_E_FILEEMPTY					6	// File is empty
#define BMP_THREADEXIT					7	// Thread Exit -> Interrupt the current job!

#define EMF_E_ZEROPATH					0   // The file name is zero
#define EMF_E_WRONGEXTENTION			1	// Wrong File Extention
#define EMF_E_FILEREADONLY				2	// The file is read only
#define EMF_E_NOMEM						3   // Could not alloc memory
#define EMF_E_BADBMP					4	// Corrupted or unsupported DIB
#define EMF_E_BADEMF					5   // Corrupted or unsupported EMF
#define EMF_E_FILEEMPTY					6	// File is empty

#define MMBMP_E_ZEROPATH				0   // The file name is zero
#define MMBMP_E_WRONGEXTENTION			1	// Wrong File Extention
#define MMBMP_E_CREATEFILE				2   // CreateFile failed
#define MMBMP_E_BADBMP					3	// Corrupted or unsupported DIB
#define MMBMP_E_FILESIZE				4	// GetFileSize failed
#define MMBMP_E_FILEEMPTY				5	// File is empty
#define MMBMP_E_CREATEFILEMAPPING		6   // CreateFileMapping failed
#define MMBMP_E_MAPVIEWOFFILE			7   // MapViewOfFile failed

#define DIBSECTIONEX_E_ZEROPATH			0   // The file name is zero
#define DIBSECTIONEX_E_NOMEM			1	// Could not alloc memory
#define DIBSECTIONEX_E_READ				2	// Couldn't read file
#define DIBSECTIONEX_E_ISTREAM			3	// Couldn't create IStream
#define DIBSECTIONEX_E_LOAD				4	// Couldn't load picture
#define DIBSECTIONEX_E_TYPE				5	// Couldn't get right picture type
#define DIBSECTIONEX_E_HANDLE			6	// Couldn't get picture handle
#define DIBSECTIONEX_E_FILEEMPTY		7	// File is empty

#define PCX_E_ZEROPATH					0   // The file name is zero
#define PCX_E_WRONGEXTENTION			1	// Wrong File Extention
#define PCX_E_FILEREADONLY				2	// The file is read only
#define PCX_E_NOMEM						3   // Could not alloc memory
#define PCX_E_BADBMP					4	// Corrupted or unsupported DIB
#define PCX_E_BADPCX					5   // Corrupted or unsupported PCX
#define PCX_E_READ						6   // Couldn't read PCX file
#define PCX_E_WRITE						7   // Couldn't write PCX file
#define PCX_E_FILEEMPTY					8	// File is empty
#define PCX_THREADEXIT					9	// Thread Exit -> Interrupt the current job!

#define TIFF_E_WRONGPARAMETERS			0	// The Function Parameters Are Wrong
#define TIFF_E_ZEROPATH					1   // The file name is zero
#define TIFF_E_WRONGEXTENTION			2	// Wrong File Extention
#define TIFF_E_FILEREADONLY				3	// The file is read only
#define TIFF_E_NOMEM					4   // Could not alloc memory
#define TIFF_E_LIBTIFF					5   // libtiff error
#define TIFF_E_BADBMP					6	// Corrupted or unsupported DIB
#define TIFF_E_READ						7   // Couldn't read TIFF file
#define TIFF_E_WRITE					8   // Couldn't write TIFF file
#define TIFF_E_FILEEMPTY				9	// File is empty
#define TIFF_THREADEXIT					10	// Thread Exit -> Interrupt the current job!

#define JPEG_E_WRONGPARAMETERS			0	// The Function Parameters Are Wrong
#define JPEG_E_ZEROPATH					1   // The file name is zero
#define JPEG_E_FILEREADONLY				2	// The file is read only
#define JPEG_E_WRONGEXTENTION			3	// Wrong File Extention
#define JPEG_E_NOMEM					4   // Could not alloc memory
#define JPEG_E_LIBJPEG_LOAD				5   // libjpeg load error
#define JPEG_E_LIBJPEG_SAVE				6   // libjpeg save error 
#define JPEG_E_BADBMP					7   // Corrupted or unsupported DIB
#define JPEG_E_BADJPEG					8   // Corrupted or unsupported JPEG
#define JPEG_E_SCALEFACTOR				9	// Unsupported Scale Factor, use: 1,2,4 or 8
#define JPEG_E_READ						10   // Couldn't read JPEG file
#define JPEG_E_WRITE					11   // Couldn't write JPEG file
#define JPEG_E_FILEEMPTY				12	// File is empty
#define JPEG_E_QUALITYRANGE				13	// Quality must be 0..100
#define JPEG_E_MEMSRC					14	// Memory Source is not specified
#define JPEG_E_MEMDST					15	// Memory Destination is not specified
#define JPEG_E_MEMDSTSIZE				16	// Memory Destination size pointer is not specified
#define JPEG_THREADEXIT					17	// Thread Exit -> Interrupt the current job!

#define GIF_E_ZEROPATH					0	// The file name is zero
#define GIF_E_WRONGEXTENTION			1	// The file extention is not .gif
#define GIF_E_FILEREADONLY				2	// The file is read only
#define GIF_E_NOMEM						3	// Could not alloc memory
#define GIF_E_GIFLIB					4	// giflib error
#define GIF_E_READ						5	// Couldn't read GIF file
#define GIF_E_FILEEMPTY					6	// File is empty
#define GIF_E_BADBMP					7	// Corrupted or unsupported DIB

#define PNG_E_ZEROPATH					0   // The file name is zero
#define PNG_E_WRONGEXTENTION			1	// Wrong File Extention
#define PNG_E_FILEREADONLY				2	// The file is read only
#define PNG_E_NOMEM						3   // Could not alloc memory
#define PNG_E_UNSUPP					4   // Unsupported image type
#define PNG_E_LIBPNG					5   // libpng error
#define PNG_E_BADBMP					6   // Corrupted or unsupported DIB
#define PNG_E_BADPNG					7   // Corrupted or unsupported PNG
#define PNG_E_READ						8   // Couldn't read PNG file
#define PNG_E_WRITE						9   // Couldn't write PNG file
#define PNG_E_FILEEMPTY					10	// File is empty
#define PNG_THREADEXIT					11	// Thread Exit -> Interrupt the current job!

////////////////////
// Pixels in Memory:
//
// 1 bit:
//
// pix0        pix1        pix2        .. pixn  0 0 0 (0 padding to DWORD align)
// bit7(byte0) bit6(byte0) bit5(byte0)
//
// The bit value is the index into a 2 colors table
//
//
// 4 bits:
//
// pix0             pix1             pix2              .. pixn 0 0 0 (0 padding to DWORD align)
// MS Nibble(byte0) LS Nibble(byte0) MS Nibble (byte1)
//
// The nibble value is the index into a 16 colors table
//
//
// 8 bits:
//
// pix0  pix1  pix2  .. pixn 0 0 0 (0 padding to DWORD align)
// Byte0 Byte1 Byte2
//
// The byte value is the index into a 256 colors table
//
//
// 16 bits: (a word is 16 bits)
//
// pix0  pix1  pix2  .. pixn 0 0 (0 padding to DWORD align)
// word0 word1 word2
//
// R = (word & RedMask) / RedShift * 256 / NumRedLevels
// G = (word & GreenMask) / GreenShift * 256 / NumGreenLevels
// B = (word & BlueMask) / BlueShift * 256 / NumBlueLevels
//
// The masks are found after the BITMAPINFOHEADER (if biCompression == BI_BITFIELDS),
// otherwise standard masks are used:
// RedMask = 0x7C00;
// GreenMask = 0x03E0;
// BlueMask = 0x001F;
//
//
// 24 bits:
//
// pix0                       pix1                       .. pixn  0 0 0 (0 padding to DWORD align)
// B(byte0) G(byte1) R(byte2) B(byte3) G(byte4) R(byte5)
//
//
// 32 bits:
//
// pix0                                  pix1                                  .. pixn
// B(byte0) G(byte1) R(byte2) A(byte3)   B(byte4) G(byte5) R(byte6) A(byte7)
//
// NOTE: this is the order of the RGBQUAD structure, the COLORREF type has B and R swapped:
//       R(byte0) G(byte1) B(byte2) A(byte3) or 0xAABBGGRR (little endian)
//       
//
// Scan Lines:
//
// If biHeight is positive, the bitmap is a bottom-up DIB and its origin is the lower-left corner.
// If biHeight is negative, the bitmap is a top-down DIB and its origin is the upper-left corner.
// 
// All Scan Lines are DWORD aligned with 0 paddings! 
//

// User Buffer
class CUserBuf
{
	public:
		CUserBuf() {m_dwSize = 0; m_pBuf = NULL;};
		virtual ~CUserBuf() {;};
		DWORD m_dwSize;
		LPBYTE m_pBuf;
};

// Gif Support
#ifdef SUPPORT_GIFLIB
#include "DibGif.h"
#endif

// Metadata (Exif, Iptc, Xmp) Support
#ifdef SUPPORT_LIBJPEG
#include "DibMetadata.h"
#endif

// Include Worker Thread
#include "..\WorkerThread.h"

// CDib Class
class CDib
{
public:
	
	// Array
	typedef CArray<CDib*,CDib*> ARRAY;

	// List
	typedef CList<CDib*,CDib*> LIST;

	// Constructors
	CDib(); // Constructs an empty Dib class

	CDib(CBitmap* pBitmap, CPalette* pPal); // Construct with a DDB (Device Dependent Bitmap) CBitmap pointer as parameter
	CDib(HBITMAP hBitmap, HPALETTE hPal);  // Construct with a DDB (Device Dependent Bitmap) handle as parameter
	// OBS: If the palette parameter is null, a halftone palette is created and used

	CDib(HBITMAP hDibSection);

	// Destructor
	virtual ~CDib();

	// Free Dib Array and List
	static void FreeArray(ARRAY& a);
	static void FreeList(LIST& l);

	// Operators
	operator HBITMAP() { return GetSafeHandle(); } // Returns a handle to the Dib Section
	CDib(const CDib& dib); // Copy Constructor
	CDib& operator=(const CDib& dib); // Copy Assignment
	void CopyVars(const CDib& SrcDib);

	// Colors Returned by GetDIBits
	static RGBQUAD ms_GdiColors16[];
	static RGBQUAD ms_GdiColors256[];

	// Halftone Colors
	static RGBQUAD ms_StdColors[];

	// General Purpose User List
	CList<CUserBuf,CUserBuf> m_UserList;

protected:
	int m_nStretchMode;			// Stretch Mode: COLORONCOLOR or HALFTONE
	CDib* m_pThumbnailDib;		// Thumbnail
	CDib* m_pPreviewDib;		// Preview (it is a small version of the Dib, for Preview)
	double m_dPreviewDibRatio;	// Original Dib Size / Preview Dib Size
	double m_dThumbnailDibRatio;// Original Dib Size / Thumbnail Dib Size
	HANDLE m_hMMFile;			// Handle to memory mapped file
	HANDLE m_hMMapping;			// Handle to the mapping of the memory mapped file
	LPVOID m_pMMFile;			// Pointer to View of Memory Mapped File
	BOOL m_bMMReadOnly;			// Memory Mapping Opened in Read Only Mode
	LPBYTE m_pBits;				// Undo() copies OrigBits to Bits
	LPBYTE m_pOrigBits;			// SetUndo() copies Bits to OrigBits
	LPBITMAPINFO m_pBMI;		// The BITMAPINFO
	BYTE m_OldBMI[sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD)];	// The Previous BITMAPINFOHEADER,
																		// used to check whether to call DrawDibBegin()
	RECT m_OldDCRect;			// The Previous DC Rectangle,
								// used to check whether to call DrawDibBegin()
	RECT m_OldDIBRect;			// The Previous DIB Rectangle,
								// used to check whether to call DrawDibBegin()
	RGBQUAD* m_pColors;			// UndoColor() copies OrigColors to Colors
	RGBQUAD* m_pOrigColors;		// SetColorUndo() copies Colors to OrigColors 
	
	COLORREF m_crBackgroundColor;// Background Color
	
	HBITMAP m_hDibSection;		// The Dib Section Handle
	LPBYTE m_pDibSectionBits;	// Pointer to DibSection Bits
	LPPICTURE m_pIPicture;		// The IPicture interface

	HDRAWDIB m_hDrawDib;		// Draw Dib Handle

	BOOL m_bShowMessageBoxOnError;// Display MessageBox on Error	

	DWORD m_dwImageSize;		// Size of image in dword aligned bytes

	short m_wBrightness;		// -100..100 
	short m_wContrast;			// -100..100
	short m_wLightness;			// -100..100
	short m_wSaturation;		// -100..100
	unsigned short m_uwHue;		// 0..360
	double m_dGamma;			// Usually between 0.1 .. 5.0
	BOOL m_bColorUndoSet;		// If TRUE SetColorUndo() was called
	BOOL m_bGrayscale;			// The colorspace is grayscale
	BOOL m_bAlpha;				// Image Has a Alpha Channel
	CPalette* m_pPalette;		// The Palette, for BitCount > 8 a halftone palette is created

	// Special Vars
	DWORD m_dwUpTime;			// Up-time (in milliseconds) when the frame was taken
	BOOL m_bUserFlag;			// General Purpose User Flag

	// 16 bits Masks
	WORD m_wRedMask16;
	WORD m_wGreenMask16;
	WORD m_wBlueMask16;
	int m_nGreenShift16;
	int m_nRedShift16;
	int m_nGreenDownShift16;
	int m_nRedDownShift16;
	int m_nBlueRoundShift16;
	int m_nGreenRoundShift16;
	int m_nRedRoundShift16;

	// 32 bits Masks
	DWORD m_dwRedMask32;
	DWORD m_dwGreenMask32;
	DWORD m_dwBlueMask32;
	int m_nGreenShift32;
	int m_nRedShift32;
	int m_nGreenDownShift32;
	int m_nRedDownShift32;
	int m_nBlueRoundShift32;
	int m_nGreenRoundShift32;
	int m_nRedRoundShift32;
	BOOL m_bFast32bpp;			// If set the 32 bpp masks are 0xFF0000, 0xFF00, 0xFF

	signed short * m_GetClosestColorIndexLookUp;

	// Flood Fill Variables
	int m_nFloodFillStackSize;
	int* m_pFloodFillStack;
	int m_nFloodFillStackPos;

	// The Exif Var
#ifdef SUPPORT_LIBJPEG
	CMetadata m_Metadata;
#endif

	// The Gif Var
#ifdef SUPPORT_GIFLIB
	CGif m_Gif;
#endif

// Member Functions
public:

	// Get Exif
	__forceinline CMetadata* GetMetadata();
	// Get Exif Info Structure
	__forceinline CMetadata::EXIFINFO* GetExifInfo();
	// Get Exif Info Write Structure
	__forceinline CMetadata::EXIFINFOINPLACEWRITE* GetExifInfoWrite();
	// Get Iptc Legacy Info Structure
	__forceinline CMetadata::IPTCINFO* GetIptcLegacyInfo();
	// Get Iptc from Xmp Info Structure
	__forceinline CMetadata::IPTCINFO* GetIptcFromXmpInfo();
	// Get Xmp Info Structure
	__forceinline CMetadata::XMPINFO* GetXmpInfo();

#ifdef SUPPORT_GIFLIB
	__forceinline CGif* GetGif();
#endif

	__forceinline DWORD GetImageSize() const;
	__forceinline DWORD GetFileSize() const;
	__forceinline void SetImageSize(DWORD dwImageSize);
	__forceinline void SetFileSize(DWORD dwFileSize);
	__forceinline BOOL IsValid() const;
	__forceinline DWORD GetUpTime() const;
	__forceinline void SetUpTime(DWORD dwUpTime);
	__forceinline DWORD IsUserFlag()	const;
	__forceinline void SetUserFlag(BOOL bUserFlag = TRUE);
	__forceinline DWORD GetCompression() const;
	__forceinline CDib* GetThumbnailDib();
	__forceinline CDib* GetPreviewDib();
	__forceinline double GetThumbnailDibRatio();
	__forceinline double GetPreviewDibRatio();
	static __forceinline BOOL UsesPalette(HDC hDC);
	__forceinline RGBQUAD* GetColors() const;
	__forceinline BOOL  IsBuiltInColorAdj() const;
	__forceinline void  ForceNoBuiltInColorAdj(BOOL bNoBuiltInColorAdj);
	__forceinline short GetBrightness() const;
	__forceinline short GetContrast() const;
	__forceinline short GetLightness() const;
	__forceinline short GetSaturation() const;
	__forceinline unsigned short GetHue() const;
	__forceinline double GetGamma() const;
	__forceinline BOOL IsGrayscale() const;
	__forceinline BOOL HasAlpha() const;
	__forceinline void SetAlpha(BOOL bAlpha);
	__forceinline BOOL IsFast32bpp() const;
	__forceinline COLORREF GetBackgroundColor() const;
	__forceinline void SetBackgroundColor(COLORREF crBackgroundColor);
	__forceinline BOOL IsMMReadOnly() const;
	__forceinline int GetXDpi() const;
	__forceinline int GetYDpi() const;
	__forceinline void SetXDpi(int dpi);
	__forceinline void SetYDpi(int dpi);
	__forceinline LPBITMAPINFO GetBMI() const;
	__forceinline LPBITMAPINFOHEADER GetBMIH() const;

	// RGB16 Types
	__forceinline BOOL IsRgb16_565();
	__forceinline BOOL IsRgb16_555();
	static __forceinline BOOL IsRgb16_565(LPBITMAPINFO pBMI);
	static __forceinline BOOL IsRgb16_555(LPBITMAPINFO pBMI);

	// Flips The Red And Blue Bytes of 32bpp images
	__forceinline void RGBA2BGRA(void* buffer, int pixcount);

	// Flips The Red And Blue Bytes of 24bpp images
	__forceinline void RGB2BGR(void* buffer, int pixcount);

	// For Resample
	__forceinline double KernelCubic_1_0(const double t);
	__forceinline float  KernelCubic_1_0(const float t);
	__forceinline double KernelCubic_0_5(const double t);
	__forceinline float  KernelCubic_0_5(const float t);
	__forceinline double KernelGeneralizedCubic(const double t, const double a);
	__forceinline float  KernelGeneralizedCubic(const float t, const float a);
	__forceinline double KernelLanczosSinc(const double t, const double r);

	// Helper functions for the RotateCW and RotateCCW functions
	__forceinline double min4(double a, double b, double c, double d);
	__forceinline double max4(double a, double b, double c, double d);

	// Returns the image width in pixels	
	__forceinline DWORD GetWidth() const;

	// Returns the image height in pixels (always positive)
	__forceinline DWORD GetHeight() const;

	// Get the Image Depth
	static __forceinline int GetBitCount(LPBITMAPINFO pBMI);

	// Get the Image Depth
	__forceinline int GetBitCount() const;

	// Returns the number of used colors, 0 is returned for 16,24 and 32 bit images
	static __forceinline WORD GetNumColors(LPBITMAPINFO pBMI);

	// Returns the number of used colors, 0 is returned for 16,24 and 32 bit images
	__forceinline WORD GetNumColors() const;

	// Check whether there is a DibSection Loaded
	__forceinline BOOL HasDibSection() const;

	// Check whether there are Bits Loaded
	__forceinline BOOL HasBits() const;

	__forceinline BOOL IsCompressed() const;

	static __forceinline BOOL IsDibSection(HBITMAP hDibSection);

	__forceinline void DoLookUpTable(	LPBYTE pLookUpTable,
										CWnd* pProgressWnd = NULL,
										BOOL bProgressSend = TRUE);

	__forceinline int GetClosestColorIndex(COLORREF crColor);

	__forceinline WORD GetPaletteSize() const;

	// 16 bit DIB pixel to RGB Conversion:
	__forceinline void DIB16ToRGB(WORD pixel, BYTE* pRed, BYTE* pGreen, BYTE* pBlue);
	__forceinline void DIB16ToRGB(WORD pixel, int* pRed, int* pGreen, int* pBlue);

	// 16 bit DIB pixel to COLORREF Conversion:
	__forceinline COLORREF DIB16ToCOLORREF(WORD pixel);

	// 32 bit DIB pixel to RGB Conversion:
	__forceinline void DIB32ToRGB(DWORD pixel, BYTE* pRed, BYTE* pGreen, BYTE* pBlue);
	__forceinline void DIB32ToRGB(DWORD pixel, int* pRed, int* pGreen, int* pBlue);

	// 32 bit DIB pixel to RGBA Conversion:
	static __forceinline void DIB32ToRGBA(DWORD pixel, BYTE* pRed, BYTE* pGreen, BYTE* pBlue, BYTE* pAlpha);
	static __forceinline void DIB32ToRGBA(DWORD pixel, int* pRed, int* pGreen, int* pBlue, int* pAlpha);

	// 32 bit DIB pixel to COLORREF Conversion:
	__forceinline COLORREF DIB32ToCOLORREF(DWORD pixel);

	// RGB to 16 bit DIB pixel Conversion:
	__forceinline WORD RGBToDIB16(BYTE red, BYTE green, BYTE blue);
	// RGB to 32 bit DIB pixel Conversion:
	__forceinline DWORD RGBToDIB32(BYTE red, BYTE green, BYTE blue);
	// RGBA to 32 bit DIB pixel Conversion:
	static __forceinline DWORD RGBAToDIB32(BYTE red, BYTE green, BYTE blue, BYTE alpha);
	static __forceinline BYTE RGBToGray(BYTE red, BYTE green, BYTE blue);
	static __forceinline void GrayToRGB(BYTE nGrayLevel, BYTE* pRed, BYTE* pGreen, BYTE* pBlue);
	static __forceinline BYTE COLORREFToGray(COLORREF crColor);
	static __forceinline COLORREF GrayToCOLORREF(BYTE nGrayLevel);

	// Coordinates are bottom-up
	__forceinline int GetPixelIndex(int x, int y);
	__forceinline BOOL SetPixelIndex(int x, int y, int nIndex);
	__forceinline COLORREF GetPixelColor(int x, int y);
	__forceinline BOOL SetPixelColor(int x, int y, COLORREF crColor);
	__forceinline COLORREF GetPixelColor32Alpha(int x, int y);
	__forceinline BOOL SetPixelColor32Alpha(int x, int y, COLORREF crColor);

	// Empty the Object
	void Free(	BOOL bLeavePalette = FALSE,
				BOOL bLeaveHeader = FALSE,
				BOOL bLeavePreviewDib = FALSE,
				BOOL bLeaveThumbnailDib = FALSE,
				BOOL bLeaveMetadata = FALSE,
				BOOL bLeaveGIF = FALSE);

	// Init vars
	void Init();

	// Is the given File Name an existing file?
	static BOOL IsFile(LPCTSTR lpszFileName);

	// Image Compression
	CString GetCompressionName();
	static CString GetCompressionName(LPBITMAPINFO pBMI);

	// Get the Num Of Colors as a String
	CString GetNumColorsName();
	static CString GetNumColorsName(LPBITMAPINFO pBMI);

	// Do Show MessageBox on Error?
	BOOL SetShowMessageBoxOnError(BOOL bShow) {	BOOL res = m_bShowMessageBoxOnError; 
												m_bShowMessageBoxOnError = bShow;
												if (m_pThumbnailDib) m_pThumbnailDib->SetShowMessageBoxOnError(bShow);
												if (m_pPreviewDib) m_pPreviewDib->SetShowMessageBoxOnError(bShow);
												return res;};
	BOOL IsShowMessageBoxOnError() const {return m_bShowMessageBoxOnError;};

	// Set Image Size
	void ComputeImageSize();

	// BITMAPINFOHEADER to BITMAPV4HEADER
	BOOL BMIToBITMAPV4HEADER();

	// Creates the Thumbnail Dib from pSrcDib,
	// if pSrcDib is NULL this is used.
	// Aspect ratio is maintained and the
	// effective size is limited by nMaxSizeX or nMaxSizeY
	BOOL CreateThumbnailDib(int nMaxSizeX,
							int nMaxSizeY,
							CDib* pSrcDib = NULL,
							CWnd* pProgressWnd = NULL,
							BOOL bProgressSend = TRUE,
							CWorkerThread* pThread = NULL);	// Optional Worker Thread pointer from which we call DoExit()
															// to check whether interruption of the load is wanted);
	
	// Deletes the Thumbnail Dib
	void DeleteThumbnailDib();

	// Creates the Preview Dib from pSrcDib,
	// if pSrcDib is NULL this is used.
	// Aspect ratio is maintained and the
	// effective size is limited by nMaxSizeX or nMaxSizeY
	BOOL CreatePreviewDib(	int nMaxSizeX,
							int nMaxSizeY,
							CDib* pSrcDib = NULL,
							CWorkerThread* pThread = NULL);	// Optional Worker Thread pointer from which we call DoExit()
															// to check whether interruption of the load is wanted);
	
	// Deletes the Preview Dib
	void DeletePreviewDib();

	// Paint Stretch Mode: COLORONCOLOR or HALFTONE
	void SetStretchMode(int StretchMode) {m_nStretchMode = StretchMode;};
	int GetStretchMode() const {return m_nStretchMode;};

	// Paint
	BOOL Paint(	HDC hDC,							// The Device Context
				const LPRECT lpDCRect,				// Device Context Rectangle
				const LPRECT lpDIBRect,				// Dib Rectangle
				BOOL bForceStretch = FALSE,			// Force Stretch if set
				BOOL bNoDrawDib = FALSE);			// Do not use DrawDib Functions if set

	// Palette Functions
	CPalette* GetPalette();
	static BOOL CreateHalftonePalette(CPalette* pPal, int nNumColors);

	// Color fill functions, pColors must already be allocated!
	static BOOL FillGdiColors(RGBQUAD* pColors, int nNumColors);
	static BOOL FillHalftoneColors(RGBQUAD* pColors, int nNumColors);
	static BOOL FillGrayscaleColors(RGBQUAD* pColors, int nNumColors);

	// Highlight the Color
	static COLORREF HighlightColor(COLORREF rcColor);
	
	// Text
	static __forceinline BOOL IsAddSingleLineTextSupported(LPBITMAPINFO pBMI);
	BOOL AddSingleLineText(	LPCTSTR szText,
							CRect Pos,		// Positive!
							CFont* pFont,	// If NULL, ANSI_VAR_FONT is selected
							UINT Align,		// Align Combination of: DT_LEFT, DT_TOP, DT_BOTTOM, DT_RIGHT, DT_CENTER, DT_VCENTER
							COLORREF crTextColor,
							int nBkgMode,	// TRANSPARENT or OPAQUE
							COLORREF crBkgColor);

	// Flood Fill: returns the number of filled pixels, -1 on error
	// Top-Down Coordinates!
	int FloodFillColor(int x, int y, COLORREF crNewColor, COLORREF crOldColor);
	int FloodFillIndex(int x, int y, int nNewIndex, int nOldIndex); // For 8 bpp or less

	// Color adjustment
	BOOL AdjustImage(short brightness,
					short contrast,
					short lightness,
					short saturation,
					unsigned short hue,
					double gamma,
					BOOL bFast,			// For Fast And Imprecise Brightness & Contrast Regulation
					BOOL bEnableUndo,	// Enable Undo of Adjusted Pixels
					CWnd* pProgressWnd = NULL,
					BOOL bProgressSend = TRUE);
	
	// Negative, alpha channel is not touched!
	BOOL Negative(	CWnd* pProgressWnd = NULL,
					BOOL bProgressSend = TRUE);

	// Invert the alpha channel only 
	BOOL NegativeAlpha(	CWnd* pProgressWnd = NULL,
						BOOL bProgressSend = TRUE);

	// Convert to Grayscale
	BOOL Grayscale(	CWnd* pProgressWnd = NULL,
					BOOL bProgressSend = TRUE);

	// Call this function to update m_bGrayscale flag,
	// necessary if pixels come from a scanner or
	// are pasted or loaded from a file format without
	// grayscale flag.
	// Works only with 1, 4 and 8bpp images
	BOOL UpdateGrayscaleFlag();

	// Give to the 1,4 and 8bpp grayscale image
	// an ascending grayscale palette
	// -> index = grayscale palette entry
	BOOL MakeGrayscaleAscending(CWnd* pProgressWnd = NULL,
								BOOL bProgressSend = TRUE);

	// Undo functions, only for same sized images (used for color adjustment)
	BOOL SetColorUndo();	// SetColorUndo() duplicates the pixels of m_pBits to m_pOrigBits for a BitCount() > 8,
							// and duplicates the colors of m_pColors to m_pOrigColors for a BitCount() <= 8
	BOOL ResetColorUndo();	// Reset the Color Undo System (m_bColorUndoSet = FALSE)
	BOOL UndoColor();		// Does the inverse of SetColorUndo()

	// Alpha, Mix & Difference Bitmaps
	BOOL IsBilevelAlpha();					// Is the alpha channel only composed by 0s and 255s?
	BOOL RenderAlphaWithSrcBackground(CDib* pSrcDib = NULL); // Uses m_crBackgroundColor to render the image
	BOOL GrayToAlphaChannel(COLORREF crColor, CDib* pSrcDib = NULL, CWnd* pProgressWnd = NULL, BOOL bProgressSend = TRUE);
	BOOL AlphaOffset(int nAlphaOffset, CDib* pSrcDib = NULL, CWnd* pProgressWnd = NULL, BOOL bProgressSend = TRUE); // nAlphaOffset: -255 .. +255
	BOOL MixRGB(CDib* pDib);				// Mix this = (this + pDib) / 2, for RGB images only!
	BOOL DiffRGB(CDib* pDib, int nMinDiff);	// for RGB images only!

	// 8 bpp differencing, used to create animated GIFs
	BOOL DiffTransp8(	CDib* pDib,
						int nTransparentIndex);	// Updates current dib's pixel with nTransperentIndex
												// if the indexes of current dib and of pDib are the same.
	BOOL DiffTransp8(	CDib* pDib,
						int nMinDiff,			// Updates current dib's pixel with nTransparentIndex if the
						int nTransparentIndex);	// difference of the gray values is less or equal nMinDiff.

	// Image Croping: Top-Down Orig Coordinates!
	BOOL Crop(	DWORD dwOrigX, DWORD dwOrigY, DWORD dwCropWidth, DWORD dwCropHeight,
				CWnd* pProgressWnd = NULL, BOOL bProgressSend = TRUE);
	BOOL CropDibSection(DWORD dwOrigX, DWORD dwOrigY, DWORD dwCropWidth, DWORD dwCropHeight);
	BOOL CropBits(	DWORD dwOrigX,
					DWORD dwOrigY,
					DWORD dwCropWidth,
					DWORD dwCropHeight,
					CDib* pSrcDib = NULL,
					CWnd* pProgressWnd = NULL,
					BOOL bProgressSend = TRUE);

	// Add A border of the given color (or index for 1,4 and 8 bpp)
	BOOL AddBorders(DWORD dwLeft,
					DWORD dwTop,
					DWORD dwRight,
					DWORD dwBottom,
					COLORREF crBorder = 0,
					CDib* pSrcDib = NULL,
					CWnd* pProgressWnd = NULL,
					BOOL bProgressSend = TRUE,
					CWorkerThread* pThread = NULL);	// Optional Worker Thread pointer from which we call DoExit()
													// to check whether interruption of the load is wanted

	// Soft Borders of Alpha Channel
	BOOL SoftBorders(int nBorder,
					BOOL bBlur,
					CWnd* pProgressWnd = NULL,
					BOOL bProgressSend = TRUE);

	// Fast Image Stretching (Works on Dib Section with StretchBlt())
	BOOL StretchDibSection(	DWORD dwNewWidth,
							DWORD dwNewHeight,
							int nStretchMode = COLORONCOLOR);

	// Selects the Best Method for You (Works on Bitmap Bits)
	BOOL StretchBits(	DWORD dwNewWidth,
						DWORD dwNewHeight,
						CDib* pSrcDib = NULL,
						CWnd* pProgressWnd = NULL,
						BOOL bProgressSend = TRUE,
						CWorkerThread* pThread = NULL);	// Optional Worker Thread pointer from which we call DoExit()
														// to check whether interruption of the load is wanted


	// If bForceNearestNeighbor not set,
	// selects the Best Method for You (Works on Bitmap Bits)
	// Maintains the aspect ratio and stretches to a size
	// that fits inside the given Max Width and Max Height.
	BOOL StretchBitsFitRect(	DWORD dwMaxWidth,
								DWORD dwMaxHeight,
								CDib* pSrcDib = NULL,
								CWnd* pProgressWnd = NULL,
								BOOL bProgressSend = TRUE,
								CWorkerThread* pThread = NULL,	// Optional Worker Thread pointer from which we call DoExit()
																// to check whether interruption of the load is wanted
								BOOL bForceNearestNeighbor = FALSE);

	// If bForceNearestNeighbor not set,
	// selects the Best Method for You (Works on Bitmap Bits).
	// Maintains the aspect ratio by centering the image and
	// by adding a border of the given color (or index for 1,4 and 8 bpp).
	BOOL StretchBitsMaintainAspectRatio(	DWORD dwNewWidth,
											DWORD dwNewHeight,
											COLORREF crBorder = 0,
											CDib* pSrcDib = NULL,
											CWnd* pProgressWnd = NULL,
											BOOL bProgressSend = TRUE,
											CWorkerThread* pThread = NULL,	// Optional Worker Thread pointer from which we call DoExit()
																			// to check whether interruption of the load is wanted
											BOOL bForceNearestNeighbor = FALSE);

	// Area Averaging Shrinking (Works on Bitmap Bits)
	// Good for extreme shrinking (thumbnail creation)
	// because it avoids anti-aliasing
	BOOL ShrinkBits(DWORD dwNewWidth,
					DWORD dwNewHeight,
					CDib* pSrcDib = NULL,
					CWnd* pProgressWnd = NULL,
					BOOL bProgressSend = TRUE,
					CWorkerThread* pThread = NULL);	// Optional Worker Thread pointer from which we call DoExit()
													// to check whether interruption of the load is wanted

	// Nearest Neighbor Resize (Works on Bitmap Bits)
	// Fastest!
	BOOL NearestNeighborResizeBits(	DWORD dwNewWidth,
									DWORD dwNewHeight,
									CDib* pSrcDib = NULL,
									CWnd* pProgressWnd = NULL,
									BOOL bProgressSend = TRUE,
									CWorkerThread* pThread = NULL);	// Optional Worker Thread pointer from which we call DoExit()
																	// to check whether interruption of the load is wanted

	// Bilinear Interpolation (Works on Bitmap Bits)
	// Bilinear is good for decreasing and increasing image size,
	// but not to much decreasing or increasing amount!
	BOOL BilinearResampleBits(	DWORD dwNewWidth,
								DWORD dwNewHeight,
								CDib* pSrcDib = NULL,
								CWnd* pProgressWnd = NULL,
								BOOL bProgressSend = TRUE,
								CWorkerThread* pThread = NULL);	// Optional Worker Thread pointer from which we call DoExit()
																// to check whether interruption of the load is wanted

	// SSE Optimized Bicubic Interpolation (Works on Bitmap Bits)
	// (Bicubic is best for increasing image size)
	BOOL BicubicResampleBits(	DWORD dwNewWidth,
								DWORD dwNewHeight,
								CDib* pSrcDib = NULL,
								CWnd* pProgressWnd = NULL,
								BOOL bProgressSend = TRUE,
								CWorkerThread* pThread = NULL);	// Optional Worker Thread pointer from which we call DoExit()
																// to check whether interruption of the load is wanted
	// Lanczos Interpolation
	BOOL LanczosResampleBits(	DWORD dwNewWidth,
								DWORD dwNewHeight,
								CDib* pSrcDib = NULL,
								CWnd* pProgressWnd = NULL,
								BOOL bProgressSend = TRUE,
								CWorkerThread* pThread = NULL);	// Optional Worker Thread pointer from which we call DoExit()
																// to check whether interruption of the load is wanted

	// Convolution Filter
	BOOL Filter(	int* pKernel,
					int nKernelSize,
					int nKernelFactor,
					int nKernelOffset = 0,
					CDib* pSrcDib = NULL,
					CWnd* pProgressWnd = NULL,
					BOOL bProgressSend = TRUE,
					CWorkerThread* pThread = NULL);		// Optional Worker Thread pointer from which we call DoExit()
														// to check whether interruption of the load is wanted

	// Fast Convolution Filter with a fixed Kernel
	// Size of 3, a Kernel Factor which cannot be 0
	// and a Kernel Offset of 0
	BOOL FilterFast(	int* pKernel,
						int nKernelFactor,
						CDib* pSrcDib = NULL,
						CWnd* pProgressWnd = NULL,
						BOOL bProgressSend = TRUE,
						CWorkerThread* pThread = NULL);	// Optional Worker Thread pointer from which we call DoExit()
														// to check whether interruption of the load is wanted

	// Convolution Filter for Alpha Channel Only
	// Rectangle Coordinates are Top-Down!
	BOOL FilterAlpha(	CRect rc,
						int* pKernel,
						int nKernelSize,
						int nKernelFactor,
						int nKernelOffset = 0,
						CDib* pSrcDib = NULL,
						CWnd* pProgressWnd = NULL,
						BOOL bProgressSend = TRUE,
						CWorkerThread* pThread = NULL);	// Optional Worker Thread pointer from which we call DoExit()
														// to check whether interruption of the load is wanted

	// Fast version for alpha channel only
	// with a fixed Kernel Size of 3,
	// a Kernel Factor which cannot be 0
	// and a Kernel Offset of 0
	BOOL FilterFastAlpha(int* pKernel,
						int nKernelFactor,
						CDib* pSrcDib = NULL,
						CWnd* pProgressWnd = NULL,
						BOOL bProgressSend = TRUE,
						CWorkerThread* pThread = NULL);	// Optional Worker Thread pointer from which we call DoExit()
														// to check whether interruption of the load is wanted


	// Unique Colors Count
	DWORD CountUniqueColors(CWnd* pProgressWnd/*=NULL*/,
							BOOL bProgressSend/*=TRUE*/);

	// Image Bits Convertion

	BOOL ConvertTo(	int nBpp,
					CWnd* pProgressWnd = NULL,
					BOOL bProgressSend = TRUE);

	// Image is first converted to Grayscale, if a value is greater or equal
	// the Threshold it is set to Bright Color, otherwise to Dark
	BOOL ConvertTo1bit(	int nThreshold = 128,
						COLORREF crDarkColor = RGB(0,0,0),
						COLORREF crBrightColor = RGB(255,255,255),
						CWnd* pProgressWnd = NULL,
						BOOL bProgressSend = TRUE);

	// Converts the image to B&W using the desired nMethod :
	// - 0 = Floyd-Steinberg (Default)
	// - 1 = Ordered-Dithering (4x4) 
	// - 2 = Burkes
	// - 3 = Stucki
	// - 4 = Jarvis-Judice-Ninke
	// - 5 = Sierra
	// - 6 = Stevenson-Arce
	// - 7 = Bayer (4x4 ordered dithering) 
	BOOL ConvertTo1bitDitherErrDiff(int nMethod,
									CWnd* pProgressWnd = NULL,
									BOOL bProgressSend = TRUE);

	// Converts to 4 bpp using the given pPalette,
	// if pPalette == NULL use a Halftone Palette
	BOOL ConvertTo4bits(CPalette* pPalette = NULL,
						CWnd* pProgressWnd = NULL,
						BOOL bProgressSend = TRUE);

	// Same as above, precise but slow!
	BOOL ConvertTo4bitsPrecise(	CPalette* pPalette = NULL,
								CWnd* pProgressWnd = NULL,
								BOOL bProgressSend = TRUE);

	// Converts to 4 bpp with error diffusion using the given pPalette,
	// if pPalette == NULL use a Halftone Palette
	BOOL ConvertTo4bitsErrDiff(	CPalette* pPalette = NULL,
								CWnd* pProgressWnd = NULL,
								BOOL bProgressSend = TRUE);

	// Converts to 8 bpp using the given pPalette
	// if pPalette == NULL use a Halftone Palette
	BOOL ConvertTo8bits(CPalette* pPalette = NULL,
						CWnd* pProgressWnd = NULL,
						BOOL bProgressSend = TRUE);

	// Same as above, precise but slow!
	BOOL ConvertTo8bitsPrecise(	CPalette* pPalette = NULL,
								CWnd* pProgressWnd = NULL,
								BOOL bProgressSend = TRUE);

	// Converts to 8 bpp with error diffusion using the given pPalette,
	// if pPalette == NULL use a Halftone Palette
	BOOL ConvertTo8bitsErrDiff(	CPalette* pPalette = NULL,
								CWnd* pProgressWnd = NULL,
								BOOL bProgressSend = TRUE);

	// To 16 bpp Conversion

	// Convert to a 5-5-5 16-bit image with biCompression = BI_RGB,
	// -> blue mask is 0x001F, the green mask is 0x03E0,
	// and the red mask is 0x7C00. Most significant bit is ignored.
	BOOL ConvertTo15bits(	CWnd* pProgressWnd = NULL,
							BOOL bProgressSend = TRUE);

	// Convert to a 16-bit image with biCompression = BI_BITFIELDS
	// and the given masks. Remember that on Win9x only two possible
	// combinations are allowed: 
	// 1. 5-5-5 16-bit image, same as for biCompression = BI_RGB:
	//    blue mask 0x001F, green mask 0x03E0, red mask 0x7C00,
	//    and most significant bit is ignored.
	//
	// 2. 5-6-5 16-bit image with:
	//    blue mask 0x001F, green mask 0x07E0, red mask  0xF800,
	//    and most significant bit is ignored
	BOOL ConvertTo16bitsMasks(	WORD wRedMask = 0xF800,
								WORD wGreenMask = 0x07E0,
								WORD wBlueMask = 0x001F,
								CWnd* pProgressWnd = NULL,
								BOOL bProgressSend = TRUE);

	// To RGB Conversion
	BOOL ConvertTo24bits(	CWnd* pProgressWnd = NULL,
							BOOL bProgressSend = TRUE);
	BOOL ConvertTo32bits(	CWnd* pProgressWnd = NULL,
							BOOL bProgressSend = TRUE);
	BOOL ConvertTo32bitsAlpha(	BYTE Alpha = 255,
								CWnd* pProgressWnd = NULL,
								BOOL bProgressSend = TRUE);

	// Palette Colors Functions
	BOOL FillColorsFromPalette(CPalette* pPalette);
	BOOL CreatePaletteFromColors(DWORD dwNumColors, RGBQUAD* pColors);
	static BOOL CreatePaletteFromColors(CPalette* pPal, DWORD dwNumColors, RGBQUAD* pColors);
	BOOL CreatePaletteFromBMI();
	BOOL CreatePaletteFromDibSection();

	// Image orientation change
	BOOL FlipTopDown(CDib* pSrcDib = NULL);
	BOOL FlipLeftRight(CDib* pSrcDib = NULL);
	BOOL Rotate180(CDib* pSrcDib = NULL);
	BOOL Rotate90CW(CDib* pSrcDib = NULL);
	BOOL Rotate90CCW(CDib* pSrcDib = NULL);
	BOOL SwapWidthHeight();	// Changes Only the Header!

	// Rotate by angle (radiants) counter-clock-wise
	// and set the background color to red, green, blue.
	// If bAntiAliasing is set, anti-aliasing is performed.
	// Note: Good anti-aliasing results are only obtained
	// with color depths > 8 bpp!
	BOOL RotateCCW(	double angle,
					BYTE red, BYTE green, BYTE blue, BYTE alpha,
					BOOL bAntiAliasing,
					CDib* pSrcDib = NULL,
					BOOL bOnlyHeader = FALSE, // To calculate the new Width, Height and Size
					CWnd* pProgressWnd = NULL,
					BOOL bProgressSend = TRUE);

	// Rotate by angle (radiants) clock-wise
	// and set the background to red, green, blue.
	// If bAntiAliasing is set, anti-aliasing is performed.
	// Note: Good anti-aliasing results are only obtained
	// with color depths > 8 bpp!
	BOOL RotateCW(	double angle,
					BYTE red, BYTE green, BYTE blue, BYTE alpha,
					BOOL bAntiAliasing,
					CDib* pSrcDib = NULL,
					BOOL bOnlyHeader = FALSE, // To calculate the new Width, Height and Size
					CWnd* pProgressWnd = NULL,
					BOOL bProgressSend = TRUE);

	// Set / Get Bits

	// If lpBits == NULL, only create the dibsection (if necessary)
	BOOL SetDibSectionBits(LPBYTE lpBits, DWORD dwSize);

	// Gets Size from m_dwImageSize
	// If lpBits == NULL, only create the dibsection (if necessary)
	BOOL SetDibSectionBits(LPBYTE lpBits);

	// Just set the pointers, no allocation:
	// 1. Pay attention to not do operations on the Dib object
	//    which free or re-alloc the passed bits!
	// 2. Reset the pointers before destroying the Dib object
	//    by calling SetDibPointers(NULL, NULL), otherwise the
	//    pointers are freed by the Dib object!
	void SetDibPointers(LPBITMAPINFO pBMI, LPBYTE pBits);

	// If lpBits == NULL, only allocates memory (if necessary)
	BOOL SetBits(LPBYTE lpBits, DWORD dwSize);
	
	// Gets Size from m_dwImageSize
	// If lpBits == NULL, only allocates memory (if necessary)
	BOOL SetBits(LPBYTE lpBits);
	
	// Converts From DibSection if necesssary
	LPBYTE GetBits(BOOL bDeleteDibSection = FALSE);

	// BMI Functions
	BOOL IsSameBMI(LPBITMAPINFO lpBMI) const;
	static BOOL IsSameBMI(LPBITMAPINFO lpBMI1, LPBITMAPINFO lpBMI2);
	BOOL SetBMI(LPBITMAPINFO lpBMI);
	DWORD GetBMISize() const;
	static DWORD GetBMISize(LPBITMAPINFO pBMI);

	// Masks, used by 16 and 32 bpp BI_BITFIELDS Dibs
	DWORD GetRMask() const;
	DWORD GetGMask() const;
	DWORD GetBMask() const;

	// Make an Empty Image:
	//
	// For 1,4 and 8 bpp you may supply a color table.
	//
	// Init Color is a RGB(A) Color for 16, 24 and 32 bpp
	// and an index for 1,4 and 8 bpp.
	// 
	// wCompression may be BI_RGB, BI_RLE4, BI_RLE8,
	// BI_BITFIELDS and some YUV formats.
	//
	// If BI_BITFIELDS then pColorsOrMasks points to the Masks
	// and if pColorsOrMasks is NULL the default mask is set.
	BOOL AllocateBits(	WORD dwBpp,
						DWORD wCompression,
						DWORD dwWidth,
						DWORD dwHeight,
						COLORREF crInitColor = 0,
						RGBQUAD* pColorsOrMasks = NULL,
						DWORD dwNumOfColors = 0);

	// Same as above but without initializing
	// the bits with a specific value
	BOOL AllocateBitsFast(	WORD wBpp,
							DWORD wCompression,
							DWORD dwWidth,
							DWORD dwHeight,
							RGBQUAD* pColorsOrMasks = NULL,
							DWORD dwNumOfColors = 0);

	// Make an Empty DIB Section,
	// For 1,4 and 8 bpp you may
	// supply the color table.
	// Note: For 16, 24 and 32 bpp the pixels are set to black,
	//       for 1,4, 8 bpp the pixels are set to index 0.
	// Init Color is a RGB Color for 16, 24 and 32 bpp
	// and a index for 1,4 and 8 bpp
	BOOL AllocateDibSection(WORD wBpp,
							DWORD dwWidth,
							DWORD dwHeight,
							COLORREF crInitColor = RGB(0,0,0),
							RGBQUAD* pColors = NULL,
							DWORD dwNumOfColors = 0);

	// Set color (or index for 1,4 and 8 bpp)
	BOOL SetBitColors(COLORREF crColor);

	// Set color (or index for 1,4 and 8 bpp)
	BOOL SetDibSectionColors(COLORREF crColor);

	// Check file to find out strange things...
	BOOL FileCheck(LPCTSTR lpszPathName);

	// Try all supported image formats:
	//
	// - If bOnlyHeader is set, only the BITMAPINFO is filled
	//   (width, height, size, bpp, colors if any...)
	//
	// - If nMaxSizeX > 0 and/or nMaxSizeY > 0, the bitmap's bits are loaded to the m_pPreviewDib 
	//   and the bitmap's edges will be limited to nMaxSizeX, nMaxSizeY
	//   (more or less). This is only implemented for JPEG files!
	BOOL LoadImage(	LPCTSTR lpszPathName,				// File Name
					int nMaxSizeX = 0,					// Default Full Size Load
					int nMaxSizeY = 0,					// Default Full Size Load
					int nPageNum = 0,					// Load the given page
					BOOL bDecompressBmp = TRUE,			// If Set Decompress RLE4 and RLE8 to RGB4 and RGB8
					BOOL bOnlyHeader = FALSE,			// Default loads image and not only header
					CWnd* pProgressWnd = NULL,			// Progress Messages Sent to This Window
					BOOL bProgressSend = TRUE,			// Send Or Post Messages to The Progress Window
					CWorkerThread* pThread = NULL);		// Optional Worker Thread pointer from which we call DoExit()
														// to check whether interruption of the load is wante

	// Memory Mapped BMP Support
#ifdef SUPPORT_MMBMP
	BOOL MapBMP(LPCTSTR lpszPathName, BOOL bReadOnly);	// Load a Memory Mapped File,
														// m_pBMI and m_pBits are set
														// to the right place in the file.
	BOOL MMCreateBMP(LPCTSTR lpszPathName);				// Create and Allocate Space in File
														// for a Memory Mapped File, m_pBMI must be set!
	BOOL MMBMPToBits();									// Closes Memory Mapped File and copies the
														// entire image to newly allocated m_pBMI and m_pBits.
	BOOL MMBMPToBMI();									// Closes Memory Mapped File and allocates
														// a Copy of the BMI to m_pBMI.
	void UnMapBMP();									// Closes Memory Mapped File:
														// m_pBits = m_pBMI = NULL;
#endif

	// BMP Support
#ifdef SUPPORT_BMP
	BOOL LoadBMP(	LPCTSTR lpszPathName,
					BOOL bOnlyHeader = FALSE,
					BOOL bDecompress = TRUE,			// If Set Decompress RLE4 and RLE8 to RGB4 and RGB8
					CWnd* pProgressWnd = NULL,			// Progress Messages Sent to This Window
					BOOL bProgressSend = TRUE,			// Send Or Post Messages to The Progress Window
					CWorkerThread* pThread = NULL);		// Optional Worker Thread pointer from which we call DoExit()
														// to check whether interruption of the load is wanted
	BOOL SaveBMP(	LPCTSTR lpszPathName,
					CWnd* pProgressWnd = NULL,			// Progress Messages Sent to This Window
					BOOL bProgressSend = TRUE,			// Send Or Post Messages to The Progress Window
					CWorkerThread* pThread = NULL);		// Optional Worker Thread pointer from which we call DoExit()
														// to check whether interruption of the load is wanted

	BOOL LoadBMPNoFileHeader(CFile& file,
							BOOL bDecompress = TRUE);	// If Set Decompress RLE4 and RLE8 to RGB4 and RGB8
	BOOL SaveBMPNoFileHeader(CFile& file);

	BOOL LoadBMP(	CFile& file,
					BOOL bOnlyHeader = FALSE,
					BOOL bDecompress = TRUE,			// If Set Decompress RLE4 and RLE8 to RGB4 and RGB8
					CWnd* pProgressWnd = NULL,			// Progress Messages Sent to This Window
					BOOL bProgressSend = TRUE,			// Send Or Post Messages to The Progress Window
					CWorkerThread* pThread = NULL);		// Optional Worker Thread pointer from which we call DoExit()
														// to check whether interruption of the load is wanted
	BOOL SaveBMP(	CFile& file,
					CWnd* pProgressWnd = NULL,			// Progress Messages Sent to This Window
					BOOL bProgressSend = TRUE,			// Send Or Post Messages to The Progress Window
					CWorkerThread* pThread = NULL);		// Optional Worker Thread pointer from which we call DoExit()
														// to check whether interruption of the load is wanted
#endif

	// EMF Support
	BOOL LoadEMF(LPCTSTR lpszPathName);
	BOOL SaveEMF(LPCTSTR lpszPathName, HDC hRefDC = NULL);

	// GIF Support
#ifdef SUPPORT_GIFLIB
	static CString GIFGetVersion(	LPCTSTR lpszPathName,
									BOOL bShowMessageBoxOnError);
	static BOOL IsAnimatedGIF(	LPCTSTR lpszPathName,
								BOOL bShowMessageBoxOnError);
	BOOL LoadGIFHeader(LPCTSTR lpszPathName);			// Load the Header Informations
	BOOL LoadGIF(	LPCTSTR lpszPathName,
					BOOL bOnlyHeader = FALSE,
					CWnd* pProgressWnd = NULL,			// Progress Messages Sent to This Window
					BOOL bProgressSend = TRUE,			// Send Or Post Messages to The Progress Window
					CWorkerThread* pThread = NULL);		// Optional Worker Thread pointer from which we call DoExit()
														// to check whether interruption of the load is wanted
	
	// Raw 8 bpp loading without disposal decoding,
	// used to open and directly save an animated gif.
	BOOL LoadFirstGIFRaw(LPCTSTR lpszPathName,
						CWnd* pProgressWnd = NULL,		// Progress Messages Sent to This Window
						BOOL bProgressSend = TRUE,		// Send Or Post Messages to The Progress Window
						CWorkerThread* pThread = NULL);	// Optional Worker Thread pointer from which we call DoExit()
														// to check whether interruption of the load is wanted
	int LoadNextGIFRaw(	CWnd* pProgressWnd = NULL,		// Progress Messages Sent to This Window
						BOOL bProgressSend = TRUE,		// Send Or Post Messages to The Progress Window
						CWorkerThread* pThread = NULL);	// Optional Worker Thread pointer from which we call DoExit()
														// to check whether interruption of the load is wanted
	// Load animated gif to a 8bpp, 4bpp or 1bpp Dib
	BOOL LoadFirstGIF(	LPCTSTR lpszPathName,
						BOOL bOnlyHeader = FALSE,
						CWnd* pProgressWnd = NULL,		// Progress Messages Sent to This Window
						BOOL bProgressSend = TRUE,		// Send Or Post Messages to The Progress Window
						CWorkerThread* pThread = NULL);	// Optional Worker Thread pointer from which we call DoExit()
														// to check whether interruption of the load is wanted
	int LoadNextGIF(CWnd* pProgressWnd = NULL,			// Progress Messages Sent to This Window
					BOOL bProgressSend = TRUE,			// Send Or Post Messages to The Progress Window
					CWorkerThread* pThread = NULL);		// Optional Worker Thread pointer from which we call DoExit()
														// to check whether interruption of the load is wanted

	// Same as above but loads to a 32 bpp Dib with
	// Alpha channel initialized according to the
	// background color.
	// True colors are necessary if using transparency
	// and different image color tables!
	BOOL LoadFirstGIF32(LPCTSTR lpszPathName,
						BOOL bOnlyHeader = FALSE,
						CWnd* pProgressWnd = NULL,		// Progress Messages Sent to This Window
						BOOL bProgressSend = TRUE,		// Send Or Post Messages to The Progress Window
						CWorkerThread* pThread = NULL);	// Optional Worker Thread pointer from which we call DoExit()
														// to check whether interruption of the load is wanted
	int LoadNextGIF32(CWnd* pProgressWnd = NULL,		// Progress Messages Sent to This Window
					BOOL bProgressSend = TRUE,			// Send Or Post Messages to The Progress Window
					CWorkerThread* pThread = NULL);		// Optional Worker Thread pointer from which we call DoExit()
														// to check whether interruption of the load is wanted

	BOOL SaveGIF(	LPCTSTR lpszPathName,
					int nTransparencyColorIndex = GIF_COLORINDEX_NOT_DEFINED,
														// If transparency is defined the image is saved
														// as GIF89a otherwise as GIF87a
					CWnd* pProgressWnd = NULL,			// Progress Messages Sent to This Window
					BOOL bProgressSend = TRUE,			// Send Or Post Messages to The Progress Window
					CWorkerThread* pThread = NULL);		// Optional Worker Thread pointer from which we call DoExit()
														// to check whether interruption of the load is wanted
	BOOL SaveFirstGIF(	LPCTSTR lpszPathName,
						CWnd* pProgressWnd = NULL,		// Progress Messages Sent to This Window
						BOOL bProgressSend = TRUE,		// Send Or Post Messages to The Progress Window
						CWorkerThread* pThread = NULL);	// Optional Worker Thread pointer from which we call DoExit()
														// to check whether interruption of the load is wanted
	BOOL SaveNextGIF(	CDib* pDib = NULL,
						CWnd* pProgressWnd = NULL,		// Progress Messages Sent to This Window
						BOOL bProgressSend = TRUE,		// Send Or Post Messages to The Progress Window
						CWorkerThread* pThread = NULL);	// Optional Worker Thread pointer from which we call DoExit()
														// to check whether interruption of the load is wanted

	// Write Comment inside the gif file,
	// to frame number nFrame
	static BOOL GIFWriteComment(int nFrame,
								LPCTSTR szFileName,
								LPCTSTR szTempDir,
								LPCTSTR szComment,
								BOOL bShowMessageBoxOnError);
#endif

	// PCX Support
#ifdef SUPPORT_PCX
	BOOL LoadPCX(	LPCTSTR lpszPathName,
					BOOL bOnlyHeader = FALSE,
					CWnd* pProgressWnd = NULL,			// Progress Messages Sent to This Window
					BOOL bProgressSend = TRUE,			// Send Or Post Messages to The Progress Window
					CWorkerThread* pThread = NULL);		// Optional Worker Thread pointer from which we call DoExit()
														// to check whether interruption of the load is wanted
	BOOL SavePCX(	LPCTSTR lpszPathName,
					CWnd* pProgressWnd = NULL,			// Progress Messages Sent to This Window
					BOOL bProgressSend = TRUE,			// Send Or Post Messages to The Progress Window
					CWorkerThread* pThread = NULL);		// Optional Worker Thread pointer from which we call DoExit()
														// to check whether interruption of the load is wanted
#endif

	// PNG Support
#ifdef SUPPORT_LIBPNG
	BOOL LoadPNG(	LPCTSTR lpszPathName,
					BOOL bLoadAlpha = TRUE,				// RGB + Alpha are loaded to a 32bpp RGBA Dib
					BOOL bOnlyHeader = FALSE,
					CWnd* pProgressWnd = NULL,			// Progress Messages Sent to This Window
					BOOL bProgressSend = TRUE,			// Send Or Post Messages to The Progress Window
					CWorkerThread* pThread = NULL);		// Optional Worker Thread pointer from which we call DoExit()
														// to check whether interruption of the load is wanted
	BOOL SavePNG(	LPCTSTR lpszPathName,
					BOOL bPaletteTransparency = FALSE,	// With palette images if set use index 0 for transparency
					BOOL bStoreBackgroundColor = FALSE,	// Store Background Color
					CWnd* pProgressWnd = NULL,			// Progress Messages Sent to This Window
					BOOL bProgressSend = TRUE,			// Send Or Post Messages to The Progress Window
					CWorkerThread* pThread = NULL);		// Optional Worker Thread pointer from which we call DoExit()
														// to check whether interruption of the load is wanted
#endif

	// TIFF Support
	//
	// Automatically select compression for save means:
	//
	// 1bpp						-> COMPRESSION_CCITTFAX4
	// 4bpp and 8bpp			-> COMPRESSION_LZW
	// 16bpp, 24bpp and 32bpp	-> COMPRESSION_JPEG
	//
#ifdef SUPPORT_LIBTIFF
	BOOL LoadTIFF(	LPCTSTR lpszPathName,
					int nPageNum = 0,					// Load the given page
					BOOL bOnlyHeader = FALSE,
					CWnd* pProgressWnd = NULL,			// Progress Messages Sent to This Window
					BOOL bProgressSend = TRUE,			// Send Or Post Messages to The Progress Window
					CWorkerThread* pThread = NULL);		// Optional Worker Thread pointer from which we call DoExit()
														// to check whether interruption of the load is wanted
	BOOL SaveTIFF(	LPCTSTR lpszPathName,
					int nCompression = -1,				// -1                    : automatically select compression
														// COMPRESSION_NONE      : uncompressed
														// COMPRESSION_CCITTFAX4 : for 1 bpp images
														// COMPRESSION_LZW       : good for 4bpp and 8bpp
														// COMPRESSION_JPEG      : good for 16bpp, 24bpp and 32 bpp
					int nJpegQuality = DEFAULT_JPEGCOMPRESSION,	// Jpeg compression quality in case of COMPRESSION_JPEG
					CWnd* pProgressWnd = NULL,			// Progress Messages Sent to This Window
					BOOL bProgressSend = TRUE,			// Send Or Post Messages to The Progress Window
					CWorkerThread* pThread = NULL);		// Optional Worker Thread pointer from which we call DoExit()
														// to check whether interruption of the load is wanted

	BOOL SaveFirstTIFF(	LPCTSTR lpszPathName,
						TIFF** ptif,
						int nPageCount = 1,
						int nCompression = -1,			// -1: automatically select compression
						int nJpegQuality = DEFAULT_JPEGCOMPRESSION,
						CWnd* pProgressWnd = NULL,
						BOOL bProgressSend = TRUE,
						CWorkerThread* pThread = NULL);

	BOOL SaveNextTIFF(	TIFF* tif,
						int nPageNum = 0,
						int nPageCount = 1,
						int nCompression = -1,			// -1: automatically select compression
						int nJpegQuality = DEFAULT_JPEGCOMPRESSION,
						CWnd* pProgressWnd = NULL,
						BOOL bProgressSend = TRUE,
						CWorkerThread* pThread = NULL);

	BOOL SaveMultiPageTIFF(	LPCTSTR lpszSavePathName,
							LPCTSTR lpszOrigPathName,
							LPCTSTR szTempDir,
							int nCompression = -1,		// -1: automatically select compression
							int nJpegQuality = DEFAULT_JPEGCOMPRESSION,
							CWnd* pProgressWnd = NULL,
							BOOL bProgressSend = TRUE,
							CWorkerThread* pThread = NULL);

	static BOOL SaveMultiPageTIFF(	LPCTSTR lpszPathName,
									ARRAY& a,						// Array of Dibs to save
									CArray<int,int>& Compression,	// Array of Compressions
									int nJpegQuality = DEFAULT_JPEGCOMPRESSION,
									CWnd* pProgressWnd = NULL,
									BOOL bProgressSend = TRUE,
									CWorkerThread* pThread = NULL);

	// Write the Object's Metadata to the given file name and to the current selected page,
	// if Multi-Page the other pages are copied from szFileName
	BOOL TIFFWriteMetadata(LPCTSTR szFileName, LPCTSTR szTempDir);

	// Set the Object's Metadata to the given tif,
	// if bRemoveIcc is set the Icc is not embedded.
	BOOL TIFFSetMetadata(TIFF* tif, BOOL bRemoveIcc = FALSE);

	// After Calling TIFFWriteDirectory(tif) always call TIFFFreeUserData()
	// because TIFFSetMetadata() may allocate some user data!
	void TIFFFreeUserData();

	// Copy given page of input tiff file to output tif
	static BOOL TIFFCopy(LPCTSTR szInFileName,
						int nInPageNum,
						TIFF* out,
						int nOutPageNum = 0,
						int nOutPageCount = 0,
						int nCompression = -1,							// -1 do not change compression type, otherwise re-encode
						int nJpegQuality = DEFAULT_JPEGCOMPRESSION,
						BOOL bFlatten = FALSE,							// if TRUE flatten the image if it has a alpha channel
						COLORREF crBackgroundColor = RGB(0,0,0),		// Flatten Background Color
						BOOL bLimitToStandardBitsPerSample = FALSE,		// If set images with Bits per Samples other than
																		// 1, 2, 4 or 8 are converted to a standard Bits per Sample
						BOOL bReencodeYCbCrJpegs = FALSE);				// Photoshop produces YCbCr Jpegs inside Tiff with a
																		// CbCr subsampling of 1, this is not supported by Tiff2Pdf!
	// Copy all pages of input tiff file to output tif
	static BOOL TIFFCopyAllPages(LPCTSTR szInFileName,
								TIFF* out,
								int nOutPageNum = 0,
								int nOutPageCount = 0,
								int nCompressionChangePage = -1,			// -1 change all pages, otherwise re-encode only the given page
								int nCompression = -1,						// -1 do not change compression type, otherwise re-encode
								int nJpegQuality = DEFAULT_JPEGCOMPRESSION,
								BOOL bFlatten = FALSE,						// if TRUE flatten all pages with a alpha channel
								COLORREF crBackgroundColor = RGB(0,0,0),	// Flatten Background Color
								BOOL bLimitToStandardBitsPerSample = FALSE,	// If set images with Bits per Samples other than
																			// 1, 2, 4 or 8 are converted to a standard Bits per Sample
								BOOL bReencodeYCbCrJpegs = FALSE);			// Photoshop produces YCbCr Jpegs inside Tiff with a
																			// CbCr subsampling of 1, this is not supported by Tiff2Pdf!

	// Delete the given page number
	static BOOL TIFFDeletePage(	int nDeletePageNum,
								LPCTSTR szFileName,
								LPCTSTR szTempDir);
#endif

	// JPEG Support
#ifdef SUPPORT_LIBJPEG
	BOOL LoadJPEG(LPCTSTR lpszPathName,
					int ScaleFactor = 1,				// ScaleFactor(=making the image smaller) can be 1,2,4 or 8
					BOOL bFast = FALSE,					// bFast=TRUE selects a fast but sloppy decoding
					BOOL bOnlyHeader = FALSE,			// If Set only loader header without decoding image
					CWnd* pProgressWnd = NULL,			// Progress Messages Sent to This Window
					BOOL bProgressSend = TRUE,			// Send Or Post Messages to The Progress Window
					CWorkerThread* pThread = NULL);		// Optional Worker Thread pointer from which we call DoExit()
														// to check whether interruption of the load is wanted
	BOOL LoadJPEG(	LPBYTE pInput,
					int nInputSize,
					int ScaleFactor = 1,				// ScaleFactor(=making the image smaller) can be 1,2,4 or 8
					BOOL bFast = FALSE,					// bFast=TRUE selects a fast but sloppy decoding
					CWorkerThread* pThread = NULL);		// Optional Worker Thread pointer from which we call DoExit()
														// to check whether interruption of the load is wanted
	BOOL SaveJPEG(	LPCTSTR lpszPathName,
					int quality = 80,					// quality ranges from 0 to 100
					BOOL bGrayScale = FALSE,			// Save as grayscale if set	
					LPCTSTR lpszMarkersFrom = _T(""),	// Copy markers from the given jpeg file
					BOOL bSaveMetadata = FALSE,			// Save the metada present in the CMetadata object
					BOOL bDoUpdateExifWidthHeightThumb = FALSE, // Update Exif Width, Height and Thumb
					CWnd* pProgressWnd = NULL,			// Progress Messages Sent to This Window
					BOOL bProgressSend = TRUE,			// Send Or Post Messages to The Progress Window
					CWorkerThread* pThread = NULL);		// Optional Worker Thread pointer from which we call DoExit()
														// to check whether interruption of the save is wanted
	BOOL SaveJPEG(	LPBYTE pOutput,
					int* pOutputSize,
					int quality = 80,					// quality ranges from 0 to 100
					BOOL bGrayScale = FALSE,			// Save as grayscale if set
					LPCTSTR lpszMarkersFrom = _T(""),	// Copy markers from the given jpeg file
					BOOL bSaveMetadata = FALSE,			// Save the metada present in the CMetadata object
					BOOL bDoUpdateExifWidthHeightThumb = FALSE, // Update Exif Width, Height and Thumb
					CWorkerThread* pThread = NULL);		// Optional Worker Thread pointer from which we call DoExit()
														// to check whether interruption of the save is wanted

	// Loss-less Jpeg Transformations
	// Note: If the given comment is _T(""),
	// the original comment is kept!
	static BOOL LossLessJPEGTrans(LPCTSTR lpszInPathName,
								LPCTSTR lpszOutPathName,
								JXFORM_CODE Transform,
								BOOL bTrim=TRUE,
								BOOL bGrayScale=FALSE,
								BOOL bDoUpdateExif = TRUE,
								LPCSTR lpszComment = "",
								BOOL bCrop=FALSE,
								DWORD dwCropOrigX=0,
								DWORD dwCropOrigY=0, 
								DWORD dwCropWidth=0,
								DWORD dwCropHeight=0,
								BOOL bShowMessageBoxOnError = TRUE,
								CWnd* pProgressWnd = NULL,
								BOOL bProgressSend = TRUE);
	static BOOL LossLessJPEGTrans(LPBYTE pJpegInData,
								int nJpegInDataSize,
								LPBYTE pJpegOutData,
								int* pJpegOutDataSize,
								JXFORM_CODE Transform,
								BOOL bTrim=TRUE,
								BOOL bGrayScale=FALSE,
								BOOL bDoUpdateExif = TRUE,
								LPCSTR lpszComment = "",
								BOOL bCrop=FALSE,
								DWORD dwCropOrigX=0,
								DWORD dwCropOrigY=0, 
								DWORD dwCropWidth=0,
								DWORD dwCropHeight=0,
								BOOL bShowMessageBoxOnError = TRUE,
								CWnd* pProgressWnd = NULL,
								BOOL bProgressSend = TRUE);
	
	// Get the Pixel Alignment, usually 8 or 16
	static BOOL JPEGGetPixelAlignment(LPCTSTR lpszPathName,
									int* pXAlignment,
									int* pYAlignment,
									BOOL bShowMessageBoxOnError);

	// Add a section of the given type.
	// Section is appended after JFIF,
	// or after the SOI Marker if no JFIF is present!
	static BOOL JPEGAddSection(	int SectionType,
								int nSectionSize,	// This is the size of the data only (excluded Type and Size fields)
								LPBYTE pSectionData,
								LPCTSTR lpszInPathName,
								LPCTSTR lpszOutPathName,
								BOOL bShowMessageBoxOnError);

	// Replaces the section of the given type.
	// Section is appended after JFIF,
	// or after the SOI Marker if no JFIF is present!
	static BOOL JPEGReplaceSection(	int SectionType,
									const unsigned char* pSectionSubType,
									int nSectionSubTypeSize,
									int nSectionSize,	// This is the size of the data only (excluded Type and Size fields)
									LPBYTE pSectionData,
									LPCTSTR lpszInPathName,
									LPCTSTR lpszOutPathName,
									BOOL bShowMessageBoxOnError);

	// Removes all sections of the given type.
	// If multiple sections of the same type are present,
	// they are all removed!
	static BOOL JPEGRemoveSection(	int SectionType,						// Like: M_JFIF, M_EXIF_XMP or M_COM
									const unsigned char* pSectionSubType,	// NULL if any type, "Exif\0\0" to remove EXIF only
									int nSectionSubTypeSize,				// 0 if any type, otherwise size to check for remove
									LPCTSTR lpszInPathName,
									LPCTSTR lpszOutPathName,
									BOOL bShowMessageBoxOnError);

	// Removes all sections in the given range, start and end included.
	// If multiple sections of the same type are present,
	// they are all removed!
	static BOOL JPEGRemoveSections(	int SectionTypeStart,
									int SectionTypeEnd,
									LPCTSTR lpszInPathName,
									LPCTSTR lpszOutPathName,
									BOOL bShowMessageBoxOnError);

	// Create A Thumbnail of maximum EXIF_THUMB_LONG_EDGE x EXIF_THUMB_SHORT_EDGE
	// (or EXIF_THUMB_SHORT_EDGE x EXIF_THUMB_LONG_EDGE) size.
	// Aspect Ratio is mantained.
	//
	// pDib is resized in any case and a Jpeg Thumb of maximum *pJpegThumbSize
	// size or less is created if ppJpegThumbData != NULL.
	// Memory of *pJpegThumbSize size is allocated (new operator)
	// for *ppJpegThumbData if ppJpegThumbData != NULL.
	// *pJpegThumbSize returns the created thumb size,
	// Note: the allocated size is equal to the passed
	// *pJpegThumbSize size, not the returned one!
	//
	// If pJpegThumbSize == NULL or *pJpegThumbSize == 0
	// a Thumb of Compression Quality EXIF_MAX_THUMB_QUALITY is created.
	// If pJpegThumbSize != NULL, *pJpegThumbSize returns the created thumb size.
	static BOOL MakeEXIFThumbnail(CDib* pDib,
								LPBYTE* ppJpegThumbData,
								int* pJpegThumbSize);

	// Update Exif members (Copy SOFn Width and Height to the Exif Width and Height)
	// and update the Thumbnail in the lossless manner.
	// (Called by LossLessJPEGTrans())
	static BOOL UpdateExifWidthHeightThumbLossLess(	LPCTSTR lpszPathName,
													JXFORM_CODE Transform,
													BOOL bTrim,
													BOOL bGrayScale,
													BOOL bShowMessageBoxOnError);
	static BOOL UpdateExifWidthHeightThumbLossLess(	LPBYTE pJpegData,
													int nJpegDataSize,
													JXFORM_CODE Transform,
													BOOL bTrim,
													BOOL bGrayScale,
													BOOL bShowMessageBoxOnError);

	// Update Exif members (Copy SOFn Width and Height to the Exif Width and Height)
	// and update the Thumbnail which is created from the main image.
	// (Called by SaveJPEG())
	static BOOL UpdateExifWidthHeightThumb(	LPCTSTR lpszPathName,
											BOOL bShowMessageBoxOnError);
	static BOOL UpdateExifWidthHeightThumb(	LPBYTE pJpegData,
											int nJpegDataSize,
											BOOL bShowMessageBoxOnError);

	// Adds an EXIF Thumbnail to the In File and stores the result to the Out File
	static BOOL AddEXIFThumbnail(	LPCTSTR lpszInPathName,
									LPCTSTR lpszOutPathName,
									BOOL bShowMessageBoxOnError);

	// Removes the EXIF Thumbnail from the In File and stores the result to the Out File
	static BOOL RemoveEXIFThumbnail(LPCTSTR lpszInPathName,
									LPCTSTR lpszOutPathName,
									BOOL bShowMessageBoxOnError);

	// Fill the m_Metadata.m_ExifInfo Structure,
	// and loads the Thumbnail if available!
	// (Called by LoadJPEG() and LoadDibSectionEx() for Jpeg Files)
	BOOL JPEGLoadMetadata(LPCTSTR lpszPathName);
	BOOL JPEGLoadMetadata(LPBYTE pJpegData, DWORD dwSize);
	BOOL LoadEXIFThumbnail();

	// Write the m_Metadata.m_ExifInfo member
	// that are marked true inside the m_Metadata.m_ExifInfoWrite
	// to the Jpeg.
	BOOL JPEGWriteEXIFInplace(LPCTSTR lpszPathName);
	BOOL JPEGWriteEXIFInplace(LPBYTE pJpegData, DWORD dwSize);

	// Creates the Thumbnail Dib from a JPEG (used for JPEGs without EXIF Thumbnail)
	// The aspect ratio is maintained and the Edges' size
	// are not exact, they may be bigger or smaller!
	//
	// Note: - The BITMAPINFO of the main dib is initialized
	BOOL CreateThumbnailDibFromJPEG(LPCTSTR lpszPathName,
									int nMaxSizeX,
									int nMaxSizeY,
									CWnd* pProgressWnd = NULL,
									BOOL bProgressSend = TRUE,
									CWorkerThread* pThread = NULL);

	// Creates the Preview Dib from a JPEG (used for Big JPEGs)
	// The aspect ratio is maintained and the Edges' size
	// are not exact, they may be bigger or smaller!
	//
	// Note: - The BITMAPINFO of the main dib is initialized
	//       - CreatePreviewDibFromJPEG returns FALSE and loads nothing
	//         if the Preview Dib would be the same size as the Full Dib
	BOOL CreatePreviewDibFromJPEG(	LPCTSTR lpszPathName,
									int nMaxSizeX,
									int nMaxSizeY,
									CWnd* pProgressWnd = NULL,
									BOOL bProgressSend = TRUE,
									CWorkerThread* pThread = NULL);

	// Write a comment to the jpeg file
	static BOOL JPEGWriteComment(	LPCTSTR szFileName,
									LPCTSTR szTempDir,
									LPCSTR szComment,
									BOOL bShowMessageBoxOnError);

	// Write the given jpeg section
	static BOOL JPEGWriteSection(	int SectionType,						// Like: M_JFIF, M_EXIF_XMP or M_COM
									const unsigned char* pSectionSubType,	// NULL if any type, "Exif\0\0" to remove EXIF only
									int nSectionSubTypeSize,				// 0 if any type, otherwise size to check for remove
									LPCTSTR szFileName,
									LPCTSTR szTempDir,
									int nSize,
									LPBYTE pData,
									BOOL bShowMessageBoxOnError);

	// Checks the m_pDib for the orientation flag
	static BOOL DoAutoOrientate(CDib* pDib);
	
	// Auto-Rotate the Dib, Preview Dib and Thumbnail Dib
	// depending on the set orientation
	static BOOL AutoOrientateDib(CDib* pDib);

	// Auto-Rotate the Thumbnail Dib
	// depending on the set orientation
	static BOOL AutoOrientateThumbnailDib(CDib* pDib);

	// Auto-Rotate the jpeg file depending on the set EXIF orientation
	static BOOL JPEGAutoOrientate(	LPCTSTR szFileName,
									LPCTSTR szTempDir,
									BOOL bShowMessageBoxOnError,
									CWnd* pProgressWnd = NULL,
									BOOL bProgressSend = TRUE);

	// Inplace change the EXIF orientation
	static BOOL JPEGSetOrientationInplace(	LPCTSTR szFileName,
											int nNewOrientation,
											BOOL bShowMessageBoxOnError);

	// Inplace change the EXIF date/time
	static BOOL JPEGSetOleDateTimeInplace(	LPCTSTR szFileName,
											const COleDateTime& Time,
											BOOL bShowMessageBoxOnError);

	// Inplace change the EXIF date/time
	static BOOL JPEGSetDateTimeInplace(	LPCTSTR szFileName,
										const CTime& Time,
										BOOL bShowMessageBoxOnError);

	// Remove the Icc Section of the given file
	static BOOL JPEGRemoveIcc(	LPCTSTR lpszPathName,
								BOOL bShowMessageBoxOnError);
#endif

	// Dib Section Functions
	BOOL LoadDibSection(HBITMAP hDibSection);		// Duplicates a DibSection
	BOOL LoadDibSection(LPCTSTR lpszPathName);		// Loads a Bmp file
	BOOL LoadDibSectionEx(LPCTSTR lpszPathName);	// Loads Bmp, Gif and Jpg files
	BOOL LoadDibSectionRes(HINSTANCE hInst, LPCTSTR lpResourceName); // Loads a resource
	BOOL LoadDibSectionRes(HINSTANCE hInst, UINT uID); // Loads a resource
	BOOL AttachDibSection(HBITMAP hDibSection); // Attaches a Dib Section to the class
	HBITMAP GetSafeHandle(); // Returns a handle to the DIB Section

	// Device dependent bitmap (DDB) functions
	HBITMAP GetDDB(HDC hDC = NULL);	// Creates a DDB Bitmap from the Image Bits and returns a handle to it
									// (you have to call DeleteObject for the HBITMAP!)
	BOOL SetDibSectionFromDDB(CBitmap* pBitmap, CPalette* pPal);	// Initializes the DibSection from a DDB (Device Dependent Bitmap) CBitmap pointer
	BOOL SetDibSectionFromDDB(HBITMAP hBitmap, HPALETTE hPal);		// Initializes the DibSection from a DDB (Device Dependent Bitmap) handle
	BOOL SetBitsFromDDB(CBitmap* pBitmap, CPalette* pPal);			// Initializes the Image Bits from a DDB (Device Dependent Bitmap) CBitmap pointer
	BOOL SetBitsFromDDB(HBITMAP hBitmap, HPALETTE hPal);			// Initializes the Image Bits from a DDB (Device Dependent Bitmap) handle
	// OBS: If the palette parameter is null, a halftone palette is created and used

	// Conversions: Image Bits <--> DIBSECTION
	BOOL BitsToDibSection(BOOL bForceNewDibSection = FALSE, BOOL bDeleteBits = TRUE);	
																// If the Force parameter is set,
																// the DibSection is first freed and
																// then initialized with the Bits.
																// If Force isn't set the DibSection is only initialized
																// if not existing. Leave bDeleteBits = TRUE
																// to avoid inconsistency
																// between bits and DibSection!
	BOOL DibSectionToBits(BOOL bForceNewBits = FALSE, BOOL bDeleteDibSection = TRUE);
																// If the Force parameter is set,
																// the Image Bits are first freed and
																// then initialized with the Dib Section Bits.
																// If Force isn't set the Bits are only initialize
																// if not existing.
																// Leave bDeleteDibSection = TRUE
																// to avoid inconsistency
																// between DibSection and bits!

	// Clipboard support function
	void EditCopy(); 
	void EditPaste(int XDpi = 0, int YDpi = 0);
	HGLOBAL CopyToHandle();
	HGLOBAL CopyFromHandle(HGLOBAL handle);
	
	// Compression / Decompression
	BOOL Compress(DWORD dwFourCC, int stride = 0);
	BOOL Decompress(int nToBitsPerPixel);
	BOOL DecompressRLE(CDib* pBackgroundDib = NULL);

	// Serialize support
	virtual void Serialize(CArchive& ar); // Only BMP file serialization

protected:
	// Pixel operations

	// Init 16 or 32 bits mask
	void InitMasks(LPBITMAPINFO pBMI = NULL);

	// Init BMI From the DibSection
	BOOL DibSectionInitBMI();
	BOOL DibSectionInitBMI(HBITMAP hDibSection);

	// Compression / Decompression
	BOOL CompressRLE(int nCompression);
	__forceinline int MakeRLE8Scanline(	BYTE* UnencodedBuffer,	// Pointer to buffer holding unencoded scan line
										BYTE* EncodedBuffer,	// Pointer to buffer to hold encoded scan line
										int nWidth);			// The length of a scan line in pixels
	__forceinline int MakeRLE4Scanline(	BYTE* UnencodedBuffer,	// Pointer to buffer holding unencoded scan line
										BYTE* EncodedBuffer,	// Pointer to buffer to hold encoded scan line
										int nWidth);			// The length of a scan line in pixels
	BOOL DecompressRLE8(CDib* pBackgroundDib = NULL);
	BOOL DecompressRLE4(CDib* pBackgroundDib = NULL);
										
	// Adjust Colors
	__forceinline void AdjustColor(CColor &color, short lightness, short saturation, unsigned short hue);
	void AdjustBrightness(	short brightness,
							CWnd* pProgressWnd = NULL,
							BOOL bProgressSend = TRUE);
	void AdjustContrast(short contrast,
						CWnd* pProgressWnd = NULL,
						BOOL bProgressSend = TRUE);
	void AdjustBrightnessContrastFast(	short brightness,
										short contrast,
										CWnd* pProgressWnd = NULL,
										BOOL bProgressSend = TRUE);
	void AdjustGamma(	double gamma,
						CWnd* pProgressWnd = NULL,
						BOOL bProgressSend = TRUE);

	// Init Lookup Table for closest color matching
	void InitGetClosestColorIndex();

	// Free Lookup Table for closest color matching
	void FreeGetClosestColorIndex();

	// Bicubic Resample Helpers
#if (_MSC_VER > 1200)
	BOOL BicubicResample24_SSE(	int nNewWidth,
								int nNewHeight,
								float xScale,
								float yScale,
								int nFloatSrcScanLineSize,
								float* f,
								LPBYTE pOutBits,
								CWnd* pProgressWnd = NULL,
								BOOL bProgressSend = TRUE,
								CWorkerThread* pThread = NULL);
	BOOL BicubicResample32_SSE(	int nNewWidth,
								int nNewHeight,
								float xScale,
								float yScale,
								int nFloatSrcScanLineSize,
								float* f,
								LPDWORD pOutBits,
								CWnd* pProgressWnd = NULL,
								BOOL bProgressSend = TRUE,
								CWorkerThread* pThread = NULL);
#endif
	BOOL BicubicResample24_C(	int nNewWidth,
								int nNewHeight,
								float xScale,
								float yScale,
								int nFloatSrcScanLineSize,
								float* f,
								LPBYTE pOutBits,
								CWnd* pProgressWnd = NULL,
								BOOL bProgressSend = TRUE,
								CWorkerThread* pThread = NULL);
	BOOL BicubicResample32_C(	int nNewWidth,
								int nNewHeight,
								float xScale,
								float yScale,
								int nFloatSrcScanLineSize,
								float* f,
								LPDWORD pOutBits,
								CWnd* pProgressWnd = NULL,
								BOOL bProgressSend = TRUE,
								CWorkerThread* pThread = NULL);

	// Show GetLastError() Message
	// In Debug mode display passed function name
	void ShowLastError(const CString& sFunctionName);
	static void ShowLastError(BOOL bShowMessageBoxOnError, const CString& sFunctionName);

	// Show given error code
	// In Debug mode display passed function name
	void ShowError(DWORD dwErrorCode, const CString& sFunctionName);
	static void ShowError(DWORD dwErrorCode, BOOL bShowMessageBoxOnError, const CString& sFunctionName);

	// Check file to find out strange things...
	BOOL FileCheckInternal(LPCTSTR lpszPathName);

	// Returns the Log2 of nNumColors
	static int LogNumColors(int nNumColors);

	// Save Jpeg Helper for Metadata
	void SaveJPEGWriteMetadata(jpeg_compress_struct& cinfo);

	// Get Resolution From Exif and set it to BITMAPINFOHEADER
	BOOL ResFromExif();

	// Flood Fill Helper Functions
	int FloodFill24(int x, int y, COLORREF crNewColor, COLORREF crOldColor);
	int FloodFill32(int x, int y, COLORREF crNewColor, COLORREF crOldColor);
	int FloodFill32Alpha(int x, int y, COLORREF crNewColor, COLORREF crOldColor);
	__forceinline BOOL FloodFillPop(int &x, int &y);
	__forceinline BOOL FloodFillPush(int x, int y);

	// Copy Bits from the source rectangle to the destination rectangle
	//
	// Supported FCCs:
	// BI_RGB
	// BI_BITFIELDS
	// YV16
	// Y42B
	// YV12
	// I420 (and the equivalent : IYUV)
	// YVU9
	// YUV9
	// YUY2 (and the equivalents: VYUY, V422, YUYV, YUNV)
	// YVYU
	// UYVY (and the equivalents: UYNV, Y422)
	//
	// Note: Min. copy size is one byte ->
	//       for 1bpp & 4bpp images you have to
	//       make sure to align and copy multiples
	//       of 8 & 2 pixels!
	//
	// Coordinates are bottom-up for BI_RGB and BI_BITFIELDS
	// and top-down for the YUV formats
	//
	static BOOL CopyBits(	DWORD dwFourCC,
							DWORD dwBitCount,
							DWORD uiDstStartX,
							DWORD uiDstStartY,
							DWORD uiSrcStartX,
							DWORD uiSrcStartY,
							DWORD uiWidthCopy,
							DWORD uiHeightCopy,
							DWORD uiDstHeight,
							DWORD uiSrcHeight,
							LPBYTE pDstBits,
							LPBYTE pSrcBits,
							DWORD uiDIBDstScanLineSize,
							DWORD uiDIBSrcScanLineSize);
public:
	// PCX header
	struct PCXHeader
	{
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
	};
	// pixels ...
	// 12 = init of palette marker
	// 768 bytes palette data

	// Nested File Information Class
	class CFileInfo
	{
	public:
		enum {BMP, EMF, JPEG, PNG, GIF, PCX, TIFF};
		enum {	COLORSPACE_UNKNOWN,		// Unspecified
				COLORSPACE_GRAYSCALE,	// Monochrome
				COLORSPACE_RGB,			// red/green/blue
				COLORSPACE_YCbCr,		// Y/Cb/Cr (also known as YUV)
				COLORSPACE_CMYK,		// C/M/Y/K
				COLORSPACE_YCCK			// Y/Cb/Cr/K
				};
		void Clear(){m_nType					= BMP;
					m_dwFileSize				= 0;
					m_dwImageSize				= 0;
					m_nWidth					= 0;
					m_nHeight					= 0;
					m_nBpp						= 0;
					m_nNumColors				= 0;
					m_nCompression				= BI_RGB;
					m_nColorSpace				= COLORSPACE_RGB;
					m_nXPixsPerMeter			= 0;
					m_nYPixsPerMeter			= 0;
					m_nNumTransparencyIndexes	= 0;
					m_dwBlueMask				= 0;
					m_dwGreenMask				= 0;
					m_dwRedMask					= 0; 
					m_crBackgroundColor			= RGB(0,0,0);
					m_bHasBackgroundColor		= FALSE;
					m_bAlphaChannel				= FALSE;
					m_bPalette					= FALSE;
					m_bBmpOS2Hdr				= FALSE;
					m_nImageCount				= 0;
					m_nImagePos					= 0;
					};
		CFileInfo() {Clear();};
		int GetNumColors() const;
		CString GetDepthName();
		__forceinline int GetXDpi() const;
		__forceinline int GetYDpi() const;

		int m_nType;
		DWORD m_dwFileSize;
		DWORD m_dwImageSize;
		int m_nWidth;
		int m_nHeight;
		int m_nBpp;
		int m_nNumColors;
		int m_nCompression;
		int m_nColorSpace;
		int m_nXPixsPerMeter;
		int m_nYPixsPerMeter;
		int m_nNumTransparencyIndexes;
		DWORD m_dwBlueMask;
		DWORD m_dwGreenMask;
		DWORD m_dwRedMask; 
		COLORREF m_crBackgroundColor;
		BOOL m_bHasBackgroundColor; // Flag indicating whether the image has a defined Background Color
		BOOL m_bAlphaChannel;
		BOOL m_bPalette;
		BOOL m_bBmpOS2Hdr;			// Bmp File Has OS2 Header (Converted to Normal Header by LoadBMP)
		int m_nImageCount;			// Images count of current file
		int m_nImagePos;			// Current image position of a Multi-Page file

	protected:
		CString GetTIFFDepthName();
	};

	// File Information Member
	CFileInfo m_FileInfo;
};

// Inline Functions Implemented here
#include "DibInline.h"

#endif //!_INC_DIB
