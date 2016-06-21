#ifndef _INC_DIBINLINE
#define _INC_DIBINLINE

#include <math.h>
#include "..\Progress.h"

////////////
// Macros //
////////////

// MAX, MIN and ABS
#ifndef MAX
#  define MAX(a,b)  ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#  define MIN(a,b)  ((a) < (b) ? (a) : (b))
#endif
#ifndef ABS
#define ABS(x)		((x) > 0 ? (x) : (-(x)))
#endif

// The PI Constant For The Rotation Function
#ifndef PI
#define PI  3.14159265358979323846
#endif

// DIB Width and Height
#define RECTWIDTH(lpRect)     ((lpRect)->right - (lpRect)->left)
#define RECTHEIGHT(lpRect)    ((lpRect)->bottom - (lpRect)->top)

// DWALIGNEDWIDTHBYTES performs DWORD-aligning of scanlines.  The "bits"
// parameter is the bit count for the scanline (= biWidth * biBitCount),
// and this macro returns the number of DWORD-aligned bytes needed
// to hold those bits.
#define DWALIGNEDWIDTHBYTES(bits)    (((bits) + 31) / 32 * 4)

// WALIGNEDWIDTHBYTES performs WORD-aligning of scanlines.  The "bits"
// parameter is the bit count for the scanline (= biWidth * biBitCount),
// and this macro returns the number of WORD-aligned bytes needed
// to hold those bits.
#define WALIGNEDWIDTHBYTES(bits)    (((bits) + 15) / 16 * 2)

// Dpi <-> Pixels Per Meter
#define DPI(PixPerMeter)	(Round((PixPerMeter) * 2.54 / 100.0))
#define PIXPERMETER(Dpi)	(Round((Dpi) * 100.0 / 2.54))

// COLORREF Alpha Support
#define RGBA(r,g,b,a)       ( (COLORREF) ( ((DWORD)(BYTE)(r)) | (((DWORD)((BYTE)(g))<<8)) | (((DWORD)(BYTE)(b))<<16) | (((DWORD)(BYTE)(a))<<24) ) )
#define GetAValue(rgba)      ((BYTE)((rgba)>>24))

// RGB to Gray Scale Conversion:
#define RGBTOGRAY(red, green, blue) (BYTE)(((blue)*117 + (green)*601 + (red)*306) >> 10)

// Gray Scale to RGB Conversion:
#define GRAYTORGB(GrayLevel,pRed,pGreen,pBlue)\
{\
	*pBlue = GrayLevel;\
	*pGreen = GrayLevel;\
	*pRed = GrayLevel;\
}\

// COLORREF to Gray Scale Conversion:
#define COLORREFTOGRAY(color) (BYTE)(((GetBValue(color))*117 +\
									(GetGValue(color))*601 +\
									(GetRValue(color))*306) >> 10)

// Gray Scale to COLORREF Conversion:
#define GRAYTOCOLORREF(GrayLevel) (RGB((GrayLevel),(GrayLevel),(GrayLevel)))

#define GETPIXELINDEX1(x_param,pindex_param,pbits_param)\
{\
	*(pindex_param) = ((pbits_param)[(x_param)/8] >> (7-(x_param)%8)) & 0x01;\
}

#define GETPIXELINDEX4(x_param,pindex_param,pbits_param)\
{\
	*(pindex_param) = ((pbits_param)[(x_param)/2] >> (4*(1-(x_param)%2))) & 0x0F;\
}

#define GETPIXELINDEX8(x_param,pindex_param,pbits_param)\
{\
	*(pindex_param) = (pbits_param)[(x_param)];\
}

#define SETPIXELINDEX1(x_param,index_param,pbits_param)\
{\
	(pbits_param)[(x_param)/8] &= ~(0x01 << (7-(x_param)%8));\
	(pbits_param)[(x_param)/8] |= ((index_param) << (7-(x_param)%8));\
}

#define SETPIXELINDEX4(x_param,index_param,pbits_param)\
{\
	(pbits_param)[(x_param)/2] &= ~(0x0F << (4*(1-(x_param)%2)));\
	(pbits_param)[(x_param)/2] |= ((index_param) << (4*(1-(x_param)%2)));\
}

#define SETPIXELINDEX8(x_param,index_param,pbits_param)\
{\
	(pbits_param)[(x_param)] = (index_param);\
}


#define GETPIXELCOLOR1(x_param,pcr_param,pdib_param,pbits_param)\
{\
	int index = ((pbits_param)[(x_param)/8] >> (7-(x_param)%8)) & 0x01;\
	*(pcr_param) = RGB((pdib_param)->m_pColors[index].rgbRed,\
						(pdib_param)->m_pColors[index].rgbGreen,\
						(pdib_param)->m_pColors[index].rgbBlue);\
}

#define GETPIXELCOLOR4(x_param,pcr_param,pdib_param,pbits_param)\
{\
	int index = ((pbits_param)[(x_param)/2] >> (4*(1-(x_param)%2))) & 0x0F;\
	*(pcr_param) = RGB((pdib_param)->m_pColors[index].rgbRed,\
						(pdib_param)->m_pColors[index].rgbGreen,\
						(pdib_param)->m_pColors[index].rgbBlue);\
}

#define GETPIXELCOLOR8(x_param,pcr_param,pdib_param,pbits_param)\
{\
	int index = (pbits_param)[(x_param)];\
	*(pcr_param) = RGB((pdib_param)->m_pColors[index].rgbRed,\
						(pdib_param)->m_pColors[index].rgbGreen,\
						(pdib_param)->m_pColors[index].rgbBlue);\
}

#define GETPIXELCOLOR16(x_param,pcr_param,pdib_param,pbits_param)\
{\
	(pdib_param)->DIB16ToRGB(((WORD*)(pbits_param))[(x_param)], &R, &G, &B);\
	*(pcr_param) = RGB(R, G, B);\
}

#define GETPIXELCOLOR24(x_param,pcr_param,pbits_param)\
{\
	BYTE R, G, B;\
	B = (pbits_param)[3*(x_param)];\
	G = (pbits_param)[3*(x_param)+1];\
	R = (pbits_param)[3*(x_param)+2];\
	*(pcr_param) = RGB(R, G, B);\
}

#define GETPIXELCOLOR32(x_param,pcr_param,pdib_param,pbits_param)\
{\
	(pdib_param)->DIB32ToRGB(((DWORD*)(pbits_param))[(x_param)], &R, &G, &B);\
	*(pcr_param) = RGB(R, G, B);\
}

#define SETPIXELCOLOR1(x_param,cr_param,pdib_param,pbits_param)\
{\
	int index = (pdib_param)->m_pPalette->GetNearestPaletteIndex((cr_param));\
	(pbits_param)[(x_param)/8] &= ~(0x01 << (7-(x_param)%8));\
	(pbits_param)[(x_param)/8] |= (index << (7-(x_param)%8));\
}

#define SETPIXELCOLOR1FAST(x_param,cr_param,pdib_param,pbits_param)\
{\
	int index = (pdib_param)->GetClosestColorIndex(cr_param);\
	(pbits_param)[(x_param)/8] &= ~(0x01 << (7-(x_param)%8));\
	(pbits_param)[(x_param)/8] |= (index << (7-(x_param)%8));\
}

#define SETPIXELCOLOR4(x_param,cr_param,pdib_param,pbits_param)\
{\
	int index = (pdib_param)->m_pPalette->GetNearestPaletteIndex((cr_param));\
	(pbits_param)[(x_param)/2] &= ~(0x0F << (4*(1-(x_param)%2)));\
	(pbits_param)[(x_param)/2] |= (index << (4*(1-(x_param)%2)));\
}

#define SETPIXELCOLOR4FAST(x_param,cr_param,pdib_param,pbits_param)\
{\
	int index = (pdib_param)->GetClosestColorIndex(cr_param);\
	(pbits_param)[(x_param)/2] &= ~(0x0F << (4*(1-(x_param)%2)));\
	(pbits_param)[(x_param)/2] |= (index << (4*(1-(x_param)%2)));\
}

#define SETPIXELCOLOR8(x_param,cr_param,pdib_param,pbits_param)\
{\
	(pbits_param)[(x_param)] = (pdib_param)->m_pPalette->GetNearestPaletteIndex((cr_param));\
}

#define SETPIXELCOLOR8FAST(x_param,cr_param,pdib_param,pbits_param)\
{\
	(pbits_param)[(x_param)] = (pdib_param)->GetClosestColorIndex(cr_param);\
}

#define SETPIXELCOLOR16(x_param,cr_param,pdib_param,pbits_param)\
{\
	((WORD*)(pbits_param))[(x_param)] = (pdib_param)->RGBToDIB16(GetRValue((cr_param)),\
																GetGValue((cr_param)),\
																GetBValue((cr_param)));\
}

#define SETPIXELCOLOR24(x_param,cr_param,pbits_param)\
{\
	(pbits_param)[3*(x_param)] = GetBValue((cr_param));\
	(pbits_param)[3*(x_param)+1] = GetGValue((cr_param));\
	(pbits_param)[3*(x_param)+2] = GetRValue((cr_param));\
}

// Do Not Touch Alpha Channel!
#define SETPIXELCOLOR32(x_param,cr_param,pdib_param,pbits_param)\
{\
	((DWORD*)(pbits_param))[(x_param)] = (((DWORD)(pbits_param)[4*(x_param)+3]) << 24) |\
										(pdib_param)->RGBToDIB32(GetRValue((cr_param)),\
																GetGValue((cr_param)),\
																GetBValue((cr_param)));\
}

////////////////////// 
// Inline Functions //
//////////////////////

// Get Metadata (do allocate it if not yet done)
__forceinline CMetadata* CDib::GetMetadata() {return (m_pMetadata ? m_pMetadata : (m_pMetadata = new CMetadata));};
// Get Exif Info Structure
__forceinline CMetadata::EXIFINFO* CDib::GetExifInfo() {return &(GetMetadata()->m_ExifInfo);};
// Get Exif Info Write Structure
__forceinline CMetadata::EXIFINFOINPLACEWRITE* CDib::GetExifInfoWrite() {return &(GetMetadata()->m_ExifInfoWrite);};

// Get Iptc Legacy Info Structure
__forceinline CMetadata::IPTCINFO* CDib::GetIptcLegacyInfo() {return &(GetMetadata()->m_IptcLegacyInfo);};
// Get Iptc from Xmp Info Structure
__forceinline CMetadata::IPTCINFO* CDib::GetIptcFromXmpInfo() {return &(GetMetadata()->m_IptcFromXmpInfo);};
// Get Xmp Info Structure
__forceinline CMetadata::XMPINFO* CDib::GetXmpInfo() {return &(GetMetadata()->m_XmpInfo);};

// File Info
__forceinline int CDib::CFileInfo::GetXDpi() const {return DPI(m_nXPixsPerMeter);};
__forceinline int CDib::CFileInfo::GetYDpi() const {return DPI(m_nYPixsPerMeter);};
__forceinline CGif* CDib::GetGif() {return &m_Gif;};
__forceinline DWORD CDib::GetImageSize()	const { return m_dwImageSize; };
__forceinline DWORD CDib::GetFileSize()		const { return m_FileInfo.m_dwFileSize; };
__forceinline void CDib::SetImageSize(DWORD dwImageSize) {m_dwImageSize = dwImageSize;}; 
__forceinline void CDib::SetFileSize(DWORD dwFileSize) {m_FileInfo.m_dwFileSize = dwFileSize;};
__forceinline BOOL CDib::IsValid()			const { return ((m_pBMI != NULL && m_pBits != NULL) ||
															(m_pBMI != NULL && m_hDibSection != NULL)); };
__forceinline DWORD CDib::GetUpTime()		const {return m_dwUpTime;};
__forceinline void CDib::SetUpTime(DWORD dwUpTime) {m_dwUpTime = dwUpTime;};
__forceinline DWORD CDib::GetUserFlag()		const {return m_dwUserFlag;};
__forceinline void CDib::SetUserFlag(DWORD dwUserFlag) {m_dwUserFlag = dwUserFlag;};
__forceinline DWORD CDib::GetCompression()		const {return (m_pBMI ? m_pBMI->bmiHeader.biCompression : 0);};
__forceinline CDib* CDib::GetThumbnailDib() {return m_pThumbnailDib;};
__forceinline CDib* CDib::GetPreviewDib() {return m_pPreviewDib;};
__forceinline double CDib::GetThumbnailDibRatio() {return m_dThumbnailDibRatio;};
__forceinline double CDib::GetPreviewDibRatio() {return m_dPreviewDibRatio;};
__forceinline BOOL CDib::UsesPalette(HDC hDC) { return (GetDeviceCaps(hDC, RASTERCAPS) & RC_PALETTE);};
__forceinline RGBQUAD* CDib::GetColors() const {return m_pColors;};
__forceinline short CDib::GetBrightness() const { return m_wBrightness;};
__forceinline short CDib::GetContrast() const { return m_wContrast;};
__forceinline short CDib::GetLightness() const { return m_wLightness;};
__forceinline short CDib::GetSaturation() const { return m_wSaturation;};
__forceinline unsigned short CDib::GetHue() const { return m_uwHue;};
__forceinline double CDib::GetGamma() const { return m_dGamma;};
__forceinline BOOL CDib::IsGrayscale() const {return m_bGrayscale;};
__forceinline BOOL CDib::HasAlpha() const {return m_bAlpha;};
__forceinline void CDib::SetAlpha(BOOL bAlpha) {m_bAlpha = bAlpha;};
__forceinline BOOL CDib::IsFast32bpp() const {return m_bFast32bpp;};
__forceinline COLORREF CDib::GetBackgroundColor() const {return m_crBackgroundColor;};
__forceinline void CDib::SetBackgroundColor(COLORREF crBackgroundColor) {	m_crBackgroundColor = crBackgroundColor;};
__forceinline BOOL CDib::IsMMReadOnly() const {return m_bMMReadOnly;};
__forceinline int CDib::GetXDpi() const {return m_pBMI ? DPI(m_pBMI->bmiHeader.biXPelsPerMeter) : 0;};
__forceinline int CDib::GetYDpi() const {return m_pBMI ? DPI(m_pBMI->bmiHeader.biYPelsPerMeter) : 0;};
__forceinline void CDib::SetXDpi(int dpi) {if (m_pBMI) m_pBMI->bmiHeader.biXPelsPerMeter = PIXPERMETER(dpi);};
__forceinline void CDib::SetYDpi(int dpi) {if (m_pBMI) m_pBMI->bmiHeader.biYPelsPerMeter = PIXPERMETER(dpi);};
__forceinline LPBITMAPINFO CDib::GetBMI() const {return m_pBMI;};
__forceinline LPBITMAPINFOHEADER CDib::GetBMIH() const {return (LPBITMAPINFOHEADER)m_pBMI;};

__forceinline BOOL CDib::IsRgb16_565()
{
	// Check
	if (!m_pBMI)
		return FALSE;

	// 16 Bpp?
	if (m_pBMI->bmiHeader.biBitCount != 16)
		return FALSE;

	if ((m_pBMI->bmiHeader.biCompression == BI_BITFIELDS)	&&
		(m_wBlueMask16 == 0x001F)							&&
		(m_wGreenMask16 == 0x07E0)							&&
		(m_wRedMask16 == 0xF800))
		return TRUE;
	else
		return FALSE;
}

__forceinline BOOL CDib::IsRgb16_555()
{
	// Check
	if (!m_pBMI)
		return FALSE;

	// 16 Bpp?
	if (m_pBMI->bmiHeader.biBitCount != 16)
		return FALSE;

	// 16 Bpp BI_RGB is by definition 555!
	if (m_pBMI->bmiHeader.biCompression == BI_RGB)
		return TRUE;

	if ((m_pBMI->bmiHeader.biCompression == BI_BITFIELDS)	&&
		(m_wBlueMask16 == 0x001F)							&&
		(m_wGreenMask16 == 0x03E0)							&&
		(m_wRedMask16 == 0x7C00))
		return TRUE;
	else
		return FALSE;
}

__forceinline BOOL CDib::IsRgb16_565(LPBITMAPINFO pBMI)
{
	// Check
	if (!pBMI)
		return FALSE;

	// 16 Bpp?
	if (pBMI->bmiHeader.biBitCount != 16)
		return FALSE;

	LPBITMAPINFOBITFIELDS pBmiBf = (LPBITMAPINFOBITFIELDS)pBMI;
	if ((pBMI->bmiHeader.biCompression == BI_BITFIELDS)	&&
		(pBmiBf->biBlueMask == 0x001F)					&&
		(pBmiBf->biGreenMask == 0x07E0)					&&
		(pBmiBf->biRedMask == 0xF800))
		return TRUE;
	else
		return FALSE;
}

__forceinline BOOL CDib::IsRgb16_555(LPBITMAPINFO pBMI)
{
	// Check
	if (!pBMI)
		return FALSE;

	// 16 Bpp?
	if (pBMI->bmiHeader.biBitCount != 16)
		return FALSE;

	// 16 Bpp BI_RGB is by definition 555!
	if (pBMI->bmiHeader.biCompression == BI_RGB)
		return TRUE;

	LPBITMAPINFOBITFIELDS pBmiBf = (LPBITMAPINFOBITFIELDS)pBMI;
	if ((pBMI->bmiHeader.biCompression == BI_BITFIELDS)	&&
		(pBmiBf->biBlueMask == 0x001F)					&&
		(pBmiBf->biGreenMask == 0x03E0)					&&
		(pBmiBf->biRedMask == 0x7C00))
		return TRUE;
	else
		return FALSE;
}

// Flips The Red And Blue Bytes of 32bpp images
__forceinline void CDib::RGBA2BGRA(void* buffer, int pixcount)
{
	__asm
	{
		pushad
		mov ecx, pixcount				// Set Up A Counter
		mov ebx, buffer					// Points ebx To Our Data (b)
		label:							// Label Used For Looping
			mov al,[ebx+0]				// Loads Value At ebx Into al
			mov ah,[ebx+2]				// Loads Value At ebx+2 Into ah
			mov [ebx+2],al				// Stores Value In al At ebx+2
			mov [ebx+0],ah				// Stores Value In ah At ebx
			
			add ebx,4					// Moves Through The Data By 4 Bytes
			dec ecx						// Decreases Our Loop Counter
			jnz label					// If Not Zero Jump Back To Label
		popad
	}
}

// Flips The Red And Blue Bytes of 24bpp images
__forceinline void CDib::RGB2BGR(void* buffer, int pixcount)
{
	__asm
	{
		pushad
		mov ecx, pixcount				// Set Up A Counter
		mov ebx, buffer					// Points ebx To Our Data (b)
		label:							// Label Used For Looping
			mov al,[ebx+0]				// Loads Value At ebx Into al
			mov ah,[ebx+2]				// Loads Value At ebx+2 Into ah
			mov [ebx+2],al				// Stores Value In al At ebx+2
			mov [ebx+0],ah				// Stores Value In ah At ebx
			
			add ebx,3					// Moves Through The Data By 3 Bytes
			dec ecx						// Decreases Our Loop Counter
			jnz label					// If Not Zero Jump Back To Label
		popad
	}
}

/* Bicubic interpolation kernel with a = -1.0:
          /
         | 1 - 2*t^2 + |t|^3          , if |t| < 1
  h(t) = | 4 - 8*|t| + 5*t^2 - |t|^3  , if 1 <= |t| < 2
         | 0                          , otherwise
          \
*/
__forceinline double CDib::KernelCubic_1_0(const double t)
{
	double abs_t = fabs(t);
	double abs_t_sq = abs_t*abs_t;
	if (abs_t < 1.0)
		return (1.0 - 2.0*abs_t_sq + abs_t_sq*abs_t);
	if (abs_t < 2.0)
		return (4.0 - 8.0*abs_t + 5.0*abs_t_sq - abs_t_sq*abs_t);
	return 0.0;
}
__forceinline float CDib::KernelCubic_1_0(const float t)
{
	float abs_t = fabs(t);
	float abs_t_sq = abs_t*abs_t;
	if (abs_t < 1.0f)
		return (1.0f - 2.0f*abs_t_sq + abs_t_sq*abs_t);
	if (abs_t < 2.0f)
		return (4.0f - 8.0f*abs_t + 5.0f*abs_t_sq - abs_t_sq*abs_t);
	return 0.0f;
}

/* Bicubic interpolation kernel with a = -0.5:
          /
         | 1 - 2.5*t^2 + 1.5*|t|^3           , if |t| < 1
  h(t) = | 2 - 4*|t| + 2.5*t^2 - 0.5*|t|^3   , if 1 <= |t| < 2
         | 0                                 , otherwise
          \
*/
__forceinline double CDib::KernelCubic_0_5(const double t)
{
	double abs_t = fabs(t);
	double abs_t_sq = abs_t*abs_t;
	if (abs_t < 1.0)
		return (1.0 - 2.5*abs_t_sq + 1.5*abs_t_sq*abs_t);
	if (abs_t < 2.0)
		return (2.0 - 4.0*abs_t + 2.5*abs_t_sq - 0.5*abs_t_sq*abs_t);
	return 0.0;
}
__forceinline float CDib::KernelCubic_0_5(const float t)
{
	float abs_t = fabs(t);
	float abs_t_sq = abs_t*abs_t;
	if (abs_t < 1.0f)
		return (1.0f - 2.5f*abs_t_sq + 1.5f*abs_t_sq*abs_t);
	if (abs_t < 2.0f)
		return (2.0f - 4.0f*abs_t + 2.5f*abs_t_sq - 0.5f*abs_t_sq*abs_t);
	return 0.0f;
}

/* Generalized Bicubic kernel:
          /
         | 1 - (a+3)*t^2 + (a+2)*|t|^3      , if |t| < 1
  h(t) = | -4a + 8a*|t| - 5a*t^2 + a*|t|^3  , if 1 <= |t| < 2
         | 0                                , otherwise
          \
*/
__forceinline double CDib::KernelGeneralizedCubic(const double t, const double a)
{
	double abs_t = fabs(t);
	if (abs_t < 1.0)
	{
		double abs_t_sq = abs_t * abs_t;
		return (1.0 - (a+3.0)*abs_t_sq + (a+2.0)*abs_t_sq*abs_t);
	}
	if (abs_t < 2.0)
	{
		double abs_t_sq = abs_t * abs_t;
		return (-4.0*a + 8.0*a*abs_t - 5.0*a*abs_t_sq + a*abs_t_sq*abs_t);
	}
	return 0.0;
}
__forceinline float CDib::KernelGeneralizedCubic(const float t, const float a)
{
	float abs_t = fabs(t);
	if (abs_t < 1.0f)
	{
		float abs_t_sq = abs_t * abs_t;
		return (1.0f - (a+3.0f)*abs_t_sq + (a+2.0f)*abs_t_sq*abs_t);
	}
	if (abs_t < 2.0f)
	{
		float abs_t_sq = abs_t * abs_t;
		return (-4.0f*a + 8.0f*a*abs_t - 5.0f*a*abs_t_sq + a*abs_t_sq*abs_t);
	}
	return 0.0f;
}

// Helper functions for the RotateCW and RotateCCW functions
__forceinline double CDib::min4(double a, double b, double c, double d)
{
	if (a < b)
	{
		if (c < a)
		{
			if (d < c)
				return d;
			else
				return c;
		}
		else
		{
			if (d < a)
				return d;
			else
				return a;
		}
	}
	else
	{
		if (c < b)
		{
			if (d < c)
				return d;
			else
				return c;
		}
		else
		{
			if (d < b)
				return d;
			else
				return b;
		}
	}
}

__forceinline double CDib::max4(double a, double b, double c, double d)
{
	if (a > b)
	{
		if (c > a)
		{
			if (d > c)
				return d;
			else
				return c;
		}
		else
		{
			if (d > a)
				return d;
			else
				return a;
		}
	}
	else
	{
		if (c > b)
		{
			if (d > c)
				return d;
			else
				return c;
		}
		else
		{
			if (d > b)
				return d;
			else
				return b;
		}
	}
}

// Returns the image width in pixels	
__forceinline DWORD CDib::GetWidth() const
{
	if (!m_pBMI)
		return 0;

	return m_pBMI->bmiHeader.biWidth;
}

// Returns the image height in pixels (always positive)
__forceinline DWORD CDib::GetHeight() const
{
	if (!m_pBMI)
		return 0;
	
	// return the positive DIB height, 
	// height is negative if we have a top-down bitmap -> use the ABS function!
	return ABS(m_pBMI->bmiHeader.biHeight);
}

// Get the Image Depth
__forceinline int CDib::GetBitCount(LPBITMAPINFO pBMI)
{
	if (!pBMI)
		return -1;

	return pBMI->bmiHeader.biBitCount; 
}

// Get the Image Depth
__forceinline int CDib::GetBitCount() const
{	
	if (!m_pBMI)
		return -1;

	return m_pBMI->bmiHeader.biBitCount; 
}

// Returns the number of used colors, 0 is returned for 16,24 and 32 bit images
__forceinline WORD CDib::GetNumColors(LPBITMAPINFO pBMI)
{
	if (!pBMI)
		return 0;

	WORD wBitCount;  // DIB bit count

	/*  The number of colors in the color table can be less than 
	 *  the number of bits per pixel allows for (i.e. lpbi->biClrUsed
	 *  can be set to some value).
	 *  If this is the case, return the appropriate value.
	 */
	DWORD dwClrUsed;

	dwClrUsed = pBMI->bmiHeader.biClrUsed;
	if (dwClrUsed != 0)
		return (WORD)dwClrUsed;

	/*  Calculate the number of colors in the color table based on
	 *  the number of bits per pixel for the DIB.
	 */
	wBitCount = pBMI->bmiHeader.biBitCount;

	/* return number of colors based on bits per pixel */
	switch (wBitCount)
	{
		case 1:
			return 2;

		case 4:
			return 16;

		case 8:
			return 256;

		default:
			return 0;
	}
}

// Returns the number of used colors, 0 is returned for 16,24 and 32 bit images
__forceinline WORD CDib::GetNumColors() const
{
	if (!m_pBMI)
		return 0;

	WORD wBitCount;  // DIB bit count

	/*  The number of colors in the color table can be less than 
	 *  the number of bits per pixel allows for (i.e. lpbi->biClrUsed
	 *  can be set to some value).
	 *  If this is the case, return the appropriate value.
	 */
	DWORD dwClrUsed;

	dwClrUsed = m_pBMI->bmiHeader.biClrUsed;
	if (dwClrUsed != 0)
		return (WORD)dwClrUsed;

	/*  Calculate the number of colors in the color table based on
	 *  the number of bits per pixel for the DIB.
	 */
	wBitCount = m_pBMI->bmiHeader.biBitCount;

	/* return number of colors based on bits per pixel */
	switch (wBitCount)
	{
		case 1:
			return 2;

		case 4:
			return 16;

		case 8:
			return 256;

		default:
			return 0;
	}
}

// Check whether there is a DibSection Loaded
__forceinline BOOL CDib::HasDibSection() const 
{
	return (m_hDibSection != NULL) ? TRUE : FALSE;
}

// Check whether there are Bits Loaded
__forceinline BOOL CDib::HasBits() const 
{
	return (m_pBits != NULL) ? TRUE : FALSE;
}

__forceinline BOOL CDib::IsCompressed() const 
{
	if (!m_pBMI)
		return FALSE;

	if (m_pBMI->bmiHeader.biCompression == BI_RGB ||
		m_pBMI->bmiHeader.biCompression == BI_BITFIELDS)
		return FALSE;
	else
		return TRUE;
}

__forceinline BOOL CDib::IsDibSection(HBITMAP hDibSection)
{
	DIBSECTION ds;
	DWORD dwSize = ::GetObject(hDibSection, sizeof(DIBSECTION), &ds);
    if (dwSize != sizeof(DIBSECTION))
        return FALSE;
	else
	{
		if (ds.dsBmih.biSize >= sizeof(BITMAPINFOHEADER))
			return TRUE;
		else
			return FALSE;
	}
}

__forceinline void CDib::DoLookUpTable(	LPBYTE pLookUpTable,
										CWnd* pProgressWnd/*=NULL*/,
										BOOL bProgressSend/*=TRUE*/)
{
	WORD wNumColors = GetNumColors();
	unsigned int line, i, nWidthDWAligned;

	DIB_INIT_PROGRESS;

	switch (m_pBMI->bmiHeader.biBitCount)
	{
		case 1:
		case 4:
		case 8:
		{
			for (i = 0; i < (int)wNumColors; i++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, i, wNumColors);

				m_pColors[i].rgbRed = pLookUpTable[m_pColors[i].rgbRed];
				m_pColors[i].rgbGreen = pLookUpTable[m_pColors[i].rgbGreen];
				m_pColors[i].rgbBlue = pLookUpTable[m_pColors[i].rgbBlue];
			}
			break;
		}
		case 16:
		{
			nWidthDWAligned = DWALIGNEDWIDTHBYTES(GetWidth() * 16); // DWORD aligned (in bytes)

			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = (nWidthDWAligned/2) * line ; i < ((nWidthDWAligned/2) * (line+1)) ; i++)
				{
					BYTE R, G, B;
					DIB16ToRGB(((WORD*)m_pBits)[i], &R, &G, &B);
					((WORD*)m_pBits)[i] = RGBToDIB16(	pLookUpTable[R],
														pLookUpTable[G],
														pLookUpTable[B]);
				}
			}
			break;
		}
		case 24:
		{
			nWidthDWAligned = DWALIGNEDWIDTHBYTES(GetWidth() * 24); // DWORD aligned (in bytes)

			for (line = 0 ; line < GetHeight() ; line++)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

				for (i = nWidthDWAligned * line ; i < (nWidthDWAligned * (line+1)) ; i++)
				{
					m_pBits[i] = pLookUpTable[m_pBits[i]];
				}
			}
			break;
		}
		case 32:
		{
			if (HasAlpha())
			{
				for (line = 0 ; line < GetHeight() ; line++)
				{
					DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

					for (i = GetWidth() * line ; i < (GetWidth() * (line+1)) ; i++)
					{
						BYTE R, G, B, A;
						DIB32ToRGBA(((DWORD*)m_pBits)[i], &R, &G, &B, &A);
						((DWORD*)m_pBits)[i] = RGBAToDIB32(	pLookUpTable[R],
															pLookUpTable[G],
															pLookUpTable[B],
															A);
					}
				}
			}
			else
			{
				for (line = 0 ; line < GetHeight() ; line++)
				{
					DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, GetHeight());

					for (i = GetWidth() * line ; i < (GetWidth() * (line+1)) ; i++)
					{
						BYTE R, G, B;
						DIB32ToRGB(((DWORD*)m_pBits)[i], &R, &G, &B);
						((DWORD*)m_pBits)[i] = RGBToDIB32(	pLookUpTable[R],
															pLookUpTable[G],
															pLookUpTable[B]);
					}
				}
			}
			break;
		}
		default:
			break;
	}

	DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
}

#define COLOR2LOOKUPINDEX(cr) ((unsigned int)((((cr)>>8)&(0x1F<<11))|(((cr)>>5)&(0x3F<<5))|(((cr)>>3)&0x1F)))
__forceinline int CDib::GetClosestColorIndex(COLORREF crColor)
{
	int c = -1;
	int dist = INT_MAX;
	unsigned int uiLookUpIndex;

	if (m_GetClosestColorIndexLookUp[uiLookUpIndex = COLOR2LOOKUPINDEX((unsigned int)crColor)] >= 0)
		return m_GetClosestColorIndexLookUp[uiLookUpIndex];
	else
	{
		for (int i = 0 ; i < GetNumColors() ; i++)
		{
			int d;
			d =  3*(unsigned)((GetRValue(crColor))-(m_pColors[i].rgbRed))*(unsigned)((GetRValue(crColor))-(m_pColors[i].rgbRed));
			d += 4*(unsigned)((GetGValue(crColor))-(m_pColors[i].rgbGreen))*(unsigned)((GetGValue(crColor))-(m_pColors[i].rgbGreen));
			d += 2*(unsigned)((GetBValue(crColor))-(m_pColors[i].rgbBlue))*(unsigned)((GetBValue(crColor))-(m_pColors[i].rgbBlue));
			if (d == 0)
			{
				m_GetClosestColorIndexLookUp[uiLookUpIndex] = i;
				return i;
			}
			if (dist > d) 
			{
				c = i;
				dist = d;
			}
		}
	}
	m_GetClosestColorIndexLookUp[uiLookUpIndex] = c;
	return c;
}

__forceinline WORD CDib::GetPaletteSize() const
{
	if (!m_pBMI)
		return 0;
	return (WORD)(GetNumColors() * sizeof(RGBQUAD));
}

// 16 bit DIB pixel to RGB Conversion:
__forceinline void CDib::DIB16ToRGB(WORD pixel, BYTE* pRed, BYTE* pGreen, BYTE* pBlue)
{
	*pBlue  = (BYTE)((pixel & m_wBlueMask16)  << m_nBlueRoundShift16);
	*pGreen = (BYTE)((pixel & m_wGreenMask16) >> m_nGreenDownShift16);
	*pRed   = (BYTE)((pixel & m_wRedMask16)   >> m_nRedDownShift16);	
}

// 16 bit DIB pixel to RGB Conversion:
__forceinline void CDib::DIB16ToRGB(WORD pixel, int* pRed, int* pGreen, int* pBlue)
{
	*pBlue  = (int)((pixel & m_wBlueMask16)  << m_nBlueRoundShift16);
	*pGreen = (int)((pixel & m_wGreenMask16) >> m_nGreenDownShift16);
	*pRed   = (int)((pixel & m_wRedMask16)   >> m_nRedDownShift16);
}

// 16 bit DIB pixel to COLORREF Conversion:
__forceinline COLORREF CDib::DIB16ToCOLORREF(WORD pixel)
{
	return RGB(	(BYTE)((pixel & m_wBlueMask16)  << m_nBlueRoundShift16),
				(BYTE)((pixel & m_wGreenMask16) >> m_nGreenDownShift16),
				(BYTE)((pixel & m_wRedMask16)   >> m_nRedDownShift16));
}

// 32 bit DIB pixel to RGB Conversion:
__forceinline void CDib::DIB32ToRGB(DWORD pixel, BYTE* pRed, BYTE* pGreen, BYTE* pBlue)
{
	*pBlue  = (BYTE)((pixel & m_dwBlueMask32)  << m_nBlueRoundShift32);
	*pGreen = (BYTE)((pixel & m_dwGreenMask32) >> m_nGreenDownShift32);
	*pRed   = (BYTE)((pixel & m_dwRedMask32)   >> m_nRedDownShift32);
}

// 32 bit DIB pixel to RGB Conversion:
__forceinline void CDib::DIB32ToRGB(DWORD pixel, int* pRed, int* pGreen, int* pBlue)
{
	*pBlue  = (int)((pixel & m_dwBlueMask32)  << m_nBlueRoundShift32);
	*pGreen = (int)((pixel & m_dwGreenMask32) >> m_nGreenDownShift32);
	*pRed   = (int)((pixel & m_dwRedMask32)   >> m_nRedDownShift32);
}

// 32 bit DIB pixel to COLORREF Conversion:
__forceinline COLORREF CDib::DIB32ToCOLORREF(DWORD pixel)
{
	return RGB(	(BYTE)((pixel & m_dwBlueMask32)  << m_nBlueRoundShift32),
				(BYTE)((pixel & m_dwGreenMask32) >> m_nGreenDownShift32),
				(BYTE)((pixel & m_dwRedMask32)   >> m_nRedDownShift32));
}

// RGB to 16 bit DIB pixel Conversion:
__forceinline WORD CDib::RGBToDIB16(BYTE red, BYTE green, BYTE blue)
{
	return (WORD)(	((((WORD)red)   >> m_nRedRoundShift16)   << m_nRedShift16)   |
					((((WORD)green) >> m_nGreenRoundShift16) << m_nGreenShift16) |
					((((WORD)blue)  >> m_nBlueRoundShift16)));
}

// RGB to 32 bit DIB pixel Conversion:
__forceinline DWORD CDib::RGBToDIB32(BYTE red, BYTE green, BYTE blue)
{
	return (DWORD)(	((((DWORD)red)   >> m_nRedRoundShift32)   << m_nRedShift32)   |
					((((DWORD)green) >> m_nGreenRoundShift32) << m_nGreenShift32) |
					((((DWORD)blue)  >> m_nBlueRoundShift32)));
}

// 32 bit DIB pixel to RGBA Conversion:
__forceinline void CDib::DIB32ToRGBA(DWORD pixel, BYTE* pRed, BYTE* pGreen, BYTE* pBlue, BYTE* pAlpha)
{
	*pAlpha = (BYTE)(pixel >> 24); 
	*pRed   = (BYTE)(pixel >> 16);
	*pGreen = (BYTE)(pixel >> 8);
	*pBlue  = (BYTE)(pixel);
}

// 32 bit DIB pixel to RGBA Conversion:
__forceinline void CDib::DIB32ToRGBA(DWORD pixel, int* pRed, int* pGreen, int* pBlue, int* pAlpha)
{
	*pAlpha = (int)(pixel >> 24); 
	*pRed   = (int)((pixel >> 16)	& 0xFF);
	*pGreen = (int)((pixel >> 8)	& 0xFF);
	*pBlue  = (int)((pixel)			& 0xFF);
}

// RGBA to 32 bit DIB pixel Conversion:
__forceinline DWORD CDib::RGBAToDIB32(BYTE red, BYTE green, BYTE blue, BYTE alpha)
{
	return (DWORD)(	((DWORD)blue)			|
					(((DWORD)green) << 8)	|
					(((DWORD)red)   << 16)  |
					(((DWORD)alpha) << 24));
}

__forceinline BYTE CDib::RGBToGray(BYTE red, BYTE green, BYTE blue)
{
	return (BYTE)(((blue)*117 + (green)*601 + (red)*306) >> 10);
}

__forceinline void CDib::GrayToRGB(BYTE nGrayLevel, BYTE* pRed, BYTE* pGreen, BYTE* pBlue)
{
	*pBlue = nGrayLevel;
	*pGreen = nGrayLevel;
	*pRed = nGrayLevel;
}

__forceinline BYTE CDib::COLORREFToGray(COLORREF crColor)
{
	return (BYTE)(	((GetBValue(crColor))*117 +
					(GetGValue(crColor))*601 +
					(GetRValue(crColor))*306) >> 10);
}

__forceinline COLORREF CDib::GrayToCOLORREF(BYTE nGrayLevel)
{
	return RGB(nGrayLevel, nGrayLevel, nGrayLevel);
}

__forceinline int CDib::GetPixelIndex(int x, int y)
{
	LPBYTE lpBits;
	if (m_pBits)
		lpBits = m_pBits;
	else
		lpBits = m_pDibSectionBits;

	if (!lpBits || !m_pBMI)
		return FALSE;

	if (GetBitCount() > 8)
		return -1;

	if (GetWidth() == 0 || GetHeight() == 0)
		return -1;
	
	// Clamp
	if (x >= (int)GetWidth())
		x = GetWidth() - 1;
	else if (x < 0)
		x = 0;
	if (y >= (int)GetHeight())
		y = GetHeight() - 1;
	else if (y < 0)
		y = 0;

	DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * GetBitCount());
	lpBits += (y * uiDIBScanLineSize);

	switch (GetBitCount())
	{
		case 1 :
			return (lpBits[x/8] >> (7-x%8)) & 0x01;
		case 4 :
			return (lpBits[x/2] >> (4*(1-x%2))) & 0x0F;
		case 8 :
			return lpBits[x];
		default:
			return -1;
	}
}

__forceinline BOOL CDib::SetPixelIndex(int x, int y, int nIndex)
{
	LPBYTE lpBits;
	if (m_pBits)
		lpBits = m_pBits;
	else
		lpBits = m_pDibSectionBits;

	if (!lpBits || !m_pBMI)
		return FALSE;

	if (GetBitCount() > 8)
		return FALSE;

	if (nIndex < 0 || nIndex > GetNumColors() - 1)
		return FALSE;

	if (x >= (int)GetWidth() ||
		y >= (int)GetHeight() ||
		x < 0 || y < 0)
		return FALSE;

	DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * GetBitCount());
	lpBits += (y * uiDIBScanLineSize);

	switch (GetBitCount())
	{
		case 1 :
		{	
			lpBits[x/8] &= ~(0x01 << (7-x%8));
			lpBits[x/8] |= (nIndex << (7-x%8));
			return TRUE;
		}
		case 4 :
		{		
			lpBits[x/2] &= ~(0x0F << (4*(1-x%2)));
			lpBits[x/2] |= (nIndex << (4*(1-x%2)));
			return TRUE;
		}
		case 8 :
		{
			lpBits[x] = nIndex;
			return TRUE;
		}
		default:
			return FALSE;
	}
}

__forceinline COLORREF CDib::GetPixelColor(int x, int y)
{
	LPBYTE lpBits;
	if (m_pBits)
		lpBits = m_pBits;
	else
		lpBits = m_pDibSectionBits;

	if (!lpBits || !m_pBMI)
		return FALSE;

	if (GetWidth() == 0 || GetHeight() == 0)
		return RGB(0, 0, 0);
	
	// Clamp
	if (x >= (int)GetWidth())
		x = GetWidth() - 1;
	else if (x < 0)
		x = 0;
	if (y >= (int)GetHeight())
		y = GetHeight() - 1;
	else if (y < 0)
		y = 0;

	DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * GetBitCount());
	lpBits += (y * uiDIBScanLineSize);
	int index;
	BYTE R, G, B;

	switch (GetBitCount())
	{
		case 1 :
		{
			index = (lpBits[x/8] >> (7-x%8)) & 0x01;
			return RGB(	m_pColors[index].rgbRed,
						m_pColors[index].rgbGreen,
						m_pColors[index].rgbBlue);
		}
		case 4 :
		{
			index = (lpBits[x/2] >> (4*(1-x%2))) & 0x0F;
			return RGB(	m_pColors[index].rgbRed,
						m_pColors[index].rgbGreen,
						m_pColors[index].rgbBlue);
		}
		case 8 :
		{
			index = lpBits[x];
			return RGB(	m_pColors[index].rgbRed,
						m_pColors[index].rgbGreen,
						m_pColors[index].rgbBlue);
		}
		case 16 :
		{
			DIB16ToRGB(((WORD*)lpBits)[x], &R, &G, &B);
			return RGB(R, G, B);
		}
		case 24 :
		{	
			B = lpBits[3*x];
			G = lpBits[3*x+1];
			R = lpBits[3*x+2];
			return RGB(R, G, B);
		}
		case 32 :
		{
			DIB32ToRGB(((DWORD*)lpBits)[x], &R, &G, &B);
			return RGB(R, G, B);
		}
		default:
			return RGB(0, 0, 0);
	}
}

__forceinline BOOL CDib::SetPixelColor(int x, int y, COLORREF crColor)
{
	LPBYTE lpBits;
	if (m_pBits)
		lpBits = m_pBits;
	else
		lpBits = m_pDibSectionBits;

	if (!lpBits || !m_pBMI)
		return FALSE;

	if (x >= (int)GetWidth() ||
		y >= (int)GetHeight() ||
		x < 0 || y < 0)
		return FALSE;

	DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * GetBitCount());
	lpBits += (y * uiDIBScanLineSize);
	int index;

	switch (GetBitCount())
	{
		case 1 :
		{
			index = m_pPalette->GetNearestPaletteIndex(crColor);
			lpBits[x/8] &= ~(0x01 << (7-x%8));
			lpBits[x/8] |= (index << (7-x%8));
			return TRUE;
		}
		case 4 :
		{		
			index = m_pPalette->GetNearestPaletteIndex(crColor);
			lpBits[x/2] &= ~(0x0F << (4*(1-x%2)));
			lpBits[x/2] |= (index << (4*(1-x%2)));
			return TRUE;
		}
		case 8 :
		{
			index = m_pPalette->GetNearestPaletteIndex(crColor);
			lpBits[x] = index;
			return TRUE;
		}
		case 16 :
		{
			((WORD*)lpBits)[x] = RGBToDIB16(GetRValue(crColor),
											GetGValue(crColor),
											GetBValue(crColor));
			return TRUE;
		}
		case 24 :
		{	
			lpBits[3*x] = GetBValue(crColor);
			lpBits[3*x+1] = GetGValue(crColor);
			lpBits[3*x+2] = GetRValue(crColor);
			return TRUE;
		}
		case 32 :
		{
			// Do Not Touch Alpha Channel!
			((DWORD*)lpBits)[x] =	(((DWORD)lpBits[4*x+3]) << 24) | // Alpha
									RGBToDIB32(GetRValue(crColor),   // RGB
												GetGValue(crColor),
												GetBValue(crColor));
			return TRUE;
		}
		default:
			return FALSE;
	}
}

__forceinline COLORREF CDib::GetPixelColor32Alpha(int x, int y)
{
	LPBYTE lpBits;
	if (m_pBits)
		lpBits = m_pBits;
	else
		lpBits = m_pDibSectionBits;

	if (!lpBits || !m_pBMI)
		return FALSE;

	if (GetWidth() == 0 || GetHeight() == 0)
		return RGBA(0, 0, 0, 0);
	
	// Clamp
	if (x >= (int)GetWidth())
		x = GetWidth() - 1;
	else if (x < 0)
		x = 0;
	if (y >= (int)GetHeight())
		y = GetHeight() - 1;
	else if (y < 0)
		y = 0;

	if (GetBitCount() != 32)
		return RGBA(0, 0, 0, 0);

	DWORD uiDIBScanLineSize = 4 * GetWidth();
	lpBits += (y * uiDIBScanLineSize);
	
	BYTE R, G, B, A;
	DIB32ToRGBA(((DWORD*)lpBits)[x], &R, &G, &B, &A);
	return RGBA(R, G, B, A);
}

__forceinline BOOL CDib::SetPixelColor32Alpha(int x, int y, COLORREF crColor)
{
	LPBYTE lpBits;
	if (m_pBits)
		lpBits = m_pBits;
	else
		lpBits = m_pDibSectionBits;

	if (!lpBits || !m_pBMI)
		return FALSE;

	if (x >= (int)GetWidth() ||
		y >= (int)GetHeight() ||
		x < 0 || y < 0)
		return FALSE;

	if (GetBitCount() != 32)
		return FALSE;

	DWORD uiDIBScanLineSize = 4 * GetWidth();
	lpBits += (y * uiDIBScanLineSize);
	((DWORD*)lpBits)[x] = RGBAToDIB32(	GetRValue(crColor),
										GetGValue(crColor),
										GetBValue(crColor),
										GetAValue(crColor));
	return TRUE;
}

__forceinline BOOL CDib::IsAddSingleLineTextSupported(LPBITMAPINFO pBMI)
{
	if (pBMI												&&

		// RGB
		(pBMI->bmiHeader.biCompression == BI_RGB			||
		pBMI->bmiHeader.biCompression == BI_BITFIELDS		||

		// Planar 422
		pBMI->bmiHeader.biCompression == FCC('YV16')		||
		pBMI->bmiHeader.biCompression == FCC('Y42B')		||
		
		// Planar 420
		pBMI->bmiHeader.biCompression == FCC('YV12')		||
		pBMI->bmiHeader.biCompression == FCC('I420')		||
		pBMI->bmiHeader.biCompression == FCC('IYUV')		||
		
		// Planar 410
		pBMI->bmiHeader.biCompression == FCC('YVU9')		||
		pBMI->bmiHeader.biCompression == FCC('YUV9')		||
		
		// Packed Y0 U0 Y1 V0
		pBMI->bmiHeader.biCompression == FCC('YUY2')		||
		pBMI->bmiHeader.biCompression == FCC('V422')		||
		pBMI->bmiHeader.biCompression == FCC('VYUY')		||
		pBMI->bmiHeader.biCompression == FCC('YUNV')		||
		pBMI->bmiHeader.biCompression == FCC('YUYV')		||
		
		// Packed Y0 V0 Y1 U0
		pBMI->bmiHeader.biCompression == FCC('YVYU')		||

		// Packed U0 Y0 V0 Y1
		pBMI->bmiHeader.biCompression == FCC('UYVY')		||
		pBMI->bmiHeader.biCompression == FCC('Y422')		||
		pBMI->bmiHeader.biCompression == FCC('UYNV')))
		
		return TRUE;
	else
		return FALSE;
}

#endif //!_INC_DIBINLINE