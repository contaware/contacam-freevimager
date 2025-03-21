#ifndef _INC_YUVTORGB
#define _INC_YUVTORGB

/*
ITU-R BT.601 (formerly called CCIR 601)
Y Range:     [16,235] (220 steps)
Cb,Cr Range: [16,240] (225 steps) 

YUV to RGB Conversion:

	R = 1.164 * (Y - 16) + 0                  + 1.596 * (V - 128)

	G = 1.164 * (Y - 16) - 0.391 * (U - 128)  - 0.813 * (V - 128)  

    B = 1.164 * (Y - 16) + 2.018 * (U - 128)  + 0
*/


// Used global var

extern BOOL g_bMMX;


// Macros

#ifndef MAX
#define MAX(a,b)  ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b)  ((a) < (b) ? (a) : (b))
#endif

// Multiple character literals like 'abcd' are of type int with the
// first character 'a' put into the most significant byte position
// and 'd' in the least significant position. The FCC macro reverts
// the order so that 'a' is the least significant byte and 'd' the
// most significant one (like in a string)
#ifndef FCC
#define FCC(ch4) ((((DWORD)(ch4) & 0xFF) << 24) |     \
				  (((DWORD)(ch4) & 0xFF00) << 8) |    \
				  (((DWORD)(ch4) & 0xFF0000) >> 8) |  \
				  (((DWORD)(ch4) & 0xFF000000) >> 24))
#endif

#ifndef DWALIGNEDWIDTHBYTES
#define DWALIGNEDWIDTHBYTES(bits)    (((bits) + 31) / 32 * 4)
#endif


// Structures

// Usually we would write a definition in the .cpp file:
// __declspec(align(8)) static const short CoefficientsRGBY[256][4] = {..
// but __declspec() does not support align() in VC6.
// It only works on VC.NET or higher, the following is a hack to make
// sure the data is aligned:
#pragma pack(push, 8)
typedef struct {
	short CoefficientsRGBY[256][4];
	short CoefficientsRGBU[256][4];
	short CoefficientsRGBV[256][4];
} YUV2RGBASMLUT;
typedef union {
	YUV2RGBASMLUT lut;
	ULONGLONG dummy[768];
} YUV2RGBASM;
#pragma pack(pop)


// Functions

extern void InitYUVToRGBTable();

// Main YUV to RGB Functions
extern bool YUVToRGB32(	DWORD dwFourCC,		// FourCC
						unsigned char *src,	// YUV format depending from dwFourCC
						unsigned char *dst,	// RGB32 Dib
						int width,
						int height);

// YUV420 Planes to RGB
extern void YUV420ToRGB32(	LPBYTE src0,	// Y Plane
							LPBYTE src1,	// U Plane
							LPBYTE src2,	// V Plane
							LPDWORD dst,	// RGB32 Dib
							int width,
							int height);
extern void YUV420ToRGB32Asm(	LPBYTE src0,// Y Plane
								LPBYTE src1,// U Plane
								LPBYTE src2,// V Plane
								LPDWORD dst,// RGB32 Dib
								int width,
								int height);

// YV16 to RGB
extern void YV16ToRGB32(unsigned char *src,	// Y Plane, V Plane and U Plane
						unsigned char *dst,	// RGB32 Dib
						int width,
						int height);

// Y42B to RGB
extern void Y42BToRGB32(unsigned char *src,	// Y Plane, U Plane and V Plane
						unsigned char *dst,	// RGB32 Dib
						int width,
						int height);

// Y800 to RGB
extern void Y800ToRGB32(unsigned char *src,	// Y Plane
						unsigned char *dst,	// RGB32 Dib
						int width,
						int height);

// YVU9 to RGB
extern void YVU9ToRGB32(unsigned char *src,	// Y Plane, V Plane and U Plane
						unsigned char *dst,	// RGB32 Dib
						int width,
						int height);

// YUV9 to RGB
extern void YUV9ToRGB32(unsigned char *src,	// Y Plane, U Plane and V Plane
						unsigned char *dst,	// RGB32 Dib
						int width,
						int height);

// YUY2 to RGB (Equivalent FCCs Are: YUNV, VYUY, V422 and YUYV)
extern void YUY2ToRGB32(LPBYTE src,			// Y0 U0 Y1 V0, Y2 U2 Y3 V2, ...
						LPDWORD dst,		// RGB32 Dib
						int width,
						int height);
extern void YUY2ToRGB32Asm(	LPBYTE src,		// Y0 U0 Y1 V0, Y2 U2 Y3 V2, ...
							LPDWORD dst,	// RGB32 Dib
							int width,
							int height);

// UYVY to RGB (Equivalent FCCs Are: Y422 and UYNV)
extern void UYVYToRGB32(LPBYTE src,			// U0 Y0 V0 Y1, U2 Y2 V2 Y3, ...
						LPDWORD dst,		// RGB32 Dib
						int width,
						int height);
extern void UYVYToRGB32Asm(	LPBYTE src,		// U0 Y0 V0 Y1, U2 Y2 V2 Y3, ...
							LPDWORD dst,	// RGB32 Dib
							int width,
							int height);

// YVYU to RGB
extern void YVYUToRGB32(LPBYTE src,			// Y0 V0 Y1 U0, Y2 V2 Y3 U2, ...
						LPDWORD dst,		// RGB32 Dib
						int width,
						int height);
extern void YVYUToRGB32Asm(	LPBYTE src,		// Y0 V0 Y1 U0, Y2 V2 Y3 U2, ...
							LPDWORD dst,	// RGB32 Dib
							int width,
							int height);

// VCR1 to RGB
extern void VCR1ToRGB32(unsigned char *src,
						unsigned char *dst,	// RGB32 Dib
						int width,
						int height);

// CLJR to RGB
extern void CLJRToRGB32(unsigned char *src,
						unsigned char *dst,	// RGB32 Dib
						int width,
						int height);

// cyuv to RGB
extern void cyuvToRGB32(unsigned char *src,
						unsigned char *dst,	// RGB32 Dib
						int width,
						int height);

// VIXL to RGB
extern void VIXLToRGB32(unsigned char *src,
						unsigned char *dst,	// RGB32 Dib
						int width,
						int height);

// Y41P to RGB
extern void Y41PToRGB32(unsigned char *src,	// U0 Y0 V0 Y1 U4 Y2 V4 Y3 Y4 Y5 Y6 Y7
						unsigned char *dst,	// RGB32 Dib
						int width,
						int height);

__forceinline bool IsSupportedYuvToRgbFormat(DWORD dwFourCC)
{
	if (dwFourCC == FCC('YV12'))
		return true;
	else if (	dwFourCC == FCC('I420') ||
				dwFourCC == FCC('IYUV'))
		return true;
	else if (dwFourCC == FCC('YVU9'))
		return true;
	else if (dwFourCC == FCC('YUV9'))
		return true;
	else if (	dwFourCC == FCC('YUY2') ||
				dwFourCC == FCC('V422')	||
				dwFourCC == FCC('VYUY')	||
				dwFourCC == FCC('YUNV') ||
				dwFourCC == FCC('YUYV'))
		return true;
	else if (	dwFourCC == FCC('UYVY') ||
				dwFourCC == FCC('Y422') ||
				dwFourCC == FCC('UYNV'))
		return true;
	else if (dwFourCC == FCC('YVYU'))
		return true;
	else if (dwFourCC == FCC('VCR1'))
		return true;
	else if (dwFourCC == FCC('CLJR'))
		return true;
	else if (dwFourCC == FCC('cyuv'))
		return true;
	else if (	dwFourCC == FCC('VIXL') ||
				dwFourCC == FCC('PIXL'))
		return true;
	else if (dwFourCC == FCC('Y41P'))
		return true;
	else if (dwFourCC == FCC('YV16'))
		return true;
	else if (dwFourCC == FCC('Y42B'))
		return true;
	else if (	dwFourCC == FCC('Y800') ||
				dwFourCC == FCC('  Y8') ||
				dwFourCC == FCC('Y8  ') ||
				dwFourCC == FCC('GREY'))
		return true;
	else
		return false;
}

#endif
