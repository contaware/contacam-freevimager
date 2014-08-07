#ifndef _INC_RGBTOYUV
#define _INC_RGBTOYUV

/*
ITU-R BT.601 (formerly called CCIR 601)
Y Range:     [16,235] (220 steps)
Cb,Cr Range: [16,240] (225 steps) 

RGB to YUV Conversion:

    Y  =      0.257 * R + 0.504 * G + 0.098 * B + 16

    Cb = U = -0.148 * R - 0.291 * G + 0.439 * B + 128

	Cr = V =  0.439 * R - 0.368 * G - 0.071 * B + 128
*/


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

// Vars

extern int g_RGB2YUV_YR[256];
extern int g_RGB2YUV_YG[256];
extern int g_RGB2YUV_YB[256];
extern int g_RGB2YUV_UR[256];
extern int g_RGB2YUV_UG[256];
extern int g_RGB2YUV_UBVR[256];
extern int g_RGB2YUV_VG[256];
extern int g_RGB2YUV_VB[256];

// Functions

extern void InitRGBToYUVTable();

extern bool RGB24ToYUV(	DWORD dwFourCC,
						unsigned char *src,	// RGB24 Dib
						unsigned char *dst,	// YUV format depending from dwFourCC
						int width,
						int height,
						int stride = 0);
extern bool RGB32ToYUV(	DWORD dwFourCC,
						unsigned char *src,	// RGB24 Dib
						unsigned char *dst,	// YUV format depending from dwFourCC
						int width,
						int height,
						int stride = 0);

extern bool RGB24ToYV12(unsigned char *src,	// RGB24 Dib
						unsigned char *dst,	// Y Plane, V Plane and U Plane
						int width,
						int height,
						int stride = 0);
extern bool RGB32ToYV12(unsigned char *src,	// RGB32 Dib
						unsigned char *dst,	// Y Plane, V Plane and U Plane
						int width,
						int height,
						int stride = 0);

// Equivalent FCC Is: IYUV
extern bool RGB24ToI420(unsigned char *src,	// RGB24 Dib
						unsigned char *dst,	// Y Plane, U Plane and V Plane
						int width,
						int height,
						int stride = 0);
extern bool RGB32ToI420(unsigned char *src,	// RGB32 Dib
						unsigned char *dst,	// Y Plane, U Plane and V Plane
						int width,
						int height,
						int stride = 0);

extern bool RGB24ToYV16(unsigned char *src,	// RGB24 Dib
						unsigned char *dst,	// Y Plane, V Plane and U Plane
						int width,
						int height,
						int stride = 0);
extern bool RGB32ToYV16(unsigned char *src,	// RGB32 Dib
						unsigned char *dst,	// Y Plane, V Plane and U Plane
						int width,
						int height,
						int stride = 0);

extern bool RGB24ToY42B(unsigned char *src,	// RGB24 Dib
						unsigned char *dst,	// Y Plane, U Plane and V Plane
						int width,
						int height,
						int stride = 0);
extern bool RGB32ToY42B(unsigned char *src,	// RGB32 Dib
						unsigned char *dst,	// Y Plane, U Plane and V Plane
						int width,
						int height,
						int stride = 0);

extern bool RGB24ToYVU9(unsigned char *src,	// RGB24 Dib
						unsigned char *dst,	// Y Plane, V Plane and U Plane
						int width,
						int height,
						int stride = 0);
extern bool RGB32ToYVU9(unsigned char *src,	// RGB32 Dib
						unsigned char *dst,	// Y Plane, V Plane and U Plane
						int width,
						int height,
						int stride = 0);

extern bool RGB24ToYUV9(unsigned char *src,	// RGB24 Dib
						unsigned char *dst,	// Y Plane, U Plane and V Plane
						int width,
						int height,
						int stride = 0);
extern bool RGB32ToYUV9(unsigned char *src,	// RGB32 Dib
						unsigned char *dst,	// Y Plane, U Plane and V Plane
						int width,
						int height,
						int stride = 0);

// Equivalent FCCs Are: YUNV, VYUY, V422 and YUYV
extern bool RGB24ToYUY2(unsigned char *src,	// RGB24 Dib
						unsigned char *dst,	// Y0 U0 Y1 V0, Y2 U2 Y3 V2, ...
						int width,
						int height,
						int stride = 0);
extern bool RGB32ToYUY2(unsigned char *src,	// RGB32 Dib
						unsigned char *dst,	// Y0 U0 Y1 V0, Y2 U2 Y3 V2, ...
						int width,
						int height,
						int stride = 0);

// Equivalent FCCs Are: Y422 and UYNV
extern bool RGB24ToUYVY(unsigned char *src,	// RGB24 Dib
						unsigned char *dst,	// U0 Y0 V0 Y1, U2 Y2 V2 Y3, ...
						int width,
						int height,
						int stride = 0);
extern bool RGB32ToUYVY(unsigned char *src,	// RGB32 Dib
						unsigned char *dst,	// U0 Y0 V0 Y1, U2 Y2 V2 Y3, ...
						int width,
						int height,
						int stride = 0);

extern bool RGB24ToYVYU(unsigned char *src,	// RGB24 Dib
						unsigned char *dst,	// Y0 V0 Y1 U0, Y2 V2 Y3 U2, ...
						int width,
						int height,
						int stride = 0);
extern bool RGB32ToYVYU(unsigned char *src,	// RGB32 Dib
						unsigned char *dst,	// Y0 V0 Y1 U0, Y2 V2 Y3 U2, ...
						int width,
						int height,
						int stride = 0);

extern bool RGB24ToY800(unsigned char *src,	// RGB24 Dib
						unsigned char *dst,	// Y Plane
						int width,
						int height,
						int stride = 0);
extern bool RGB32ToY800(unsigned char *src,	// RGB32 Dib
						unsigned char *dst,	// Y Plane
						int width,
						int height,
						int stride = 0);
 
extern bool RGB24ToY41P(unsigned char *src,	// RGB24 Dib
						unsigned char *dst,	// U0 Y0 V0 Y1 U4 Y2 V4 Y3 Y4 Y5 Y6 Y7
						int width,
						int height,
						int stride = 0);
extern bool RGB32ToY41P(unsigned char *src,	// RGB32 Dib
						unsigned char *dst,	// U0 Y0 V0 Y1 U4 Y2 V4 Y3 Y4 Y5 Y6 Y7
						int width,
						int height,
						int stride = 0);

extern bool RGB24ToYUV420(	unsigned char *src,	// RGB24 Dib
							unsigned int *dst,	// Y Plane, U Plane and V Plane, unsigned int size for Y, U, V!
							int width,
							int height);
extern bool RGB32ToYUV420(	unsigned char *src,	// RGB32 Dib
							unsigned int *dst,	// Y Plane, U Plane and V Plane, unsigned int size for Y, U, V!
							int width,
							int height);

__forceinline bool IsSupportedRgbToYuvFormat(DWORD dwFourCC)
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

__forceinline int FourCCToBpp(DWORD dwFourCC)
{
	if (dwFourCC == FCC('YV12'))
		return 12;
	else if (	dwFourCC == FCC('I420') ||
				dwFourCC == FCC('IYUV'))
		return 12;
	else if (dwFourCC == FCC('YVU9'))
		return 9;
	else if (dwFourCC == FCC('YUV9'))
		return 9;
	else if (	dwFourCC == FCC('YUY2') ||
				dwFourCC == FCC('V422')	||
				dwFourCC == FCC('VYUY')	||
				dwFourCC == FCC('YUNV') ||
				dwFourCC == FCC('YUYV'))
		return 16;
	else if (	dwFourCC == FCC('UYVY') ||
				dwFourCC == FCC('Y422') ||
				dwFourCC == FCC('UYNV'))
		return 16;
	else if (dwFourCC == FCC('YVYU'))
		return 16;
	else if (dwFourCC == FCC('Y41P'))
		return 12;
	else if (dwFourCC == FCC('YV16'))
		return 16;
	else if (dwFourCC == FCC('Y42B'))
		return 16;
	else if (	dwFourCC == FCC('Y800') ||
				dwFourCC == FCC('  Y8') ||
				dwFourCC == FCC('Y8  ') ||
				dwFourCC == FCC('GREY'))
		return 8;
	else
		return 0;
}

__forceinline int CalcYUVStride(DWORD dwFourCC, int width)
{
	if (dwFourCC == FCC('YV12'))
		return width;
	else if (	dwFourCC == FCC('I420') ||
				dwFourCC == FCC('IYUV'))
		return width;
	else if (dwFourCC == FCC('YVU9'))
		return width;
	else if (dwFourCC == FCC('YUV9'))
		return width;
	else if (	dwFourCC == FCC('YUY2') ||
				dwFourCC == FCC('V422')	||
				dwFourCC == FCC('VYUY')	||
				dwFourCC == FCC('YUNV') ||
				dwFourCC == FCC('YUYV'))
		return width * 2;
	else if (	dwFourCC == FCC('UYVY') ||
				dwFourCC == FCC('Y422') ||
				dwFourCC == FCC('UYNV'))
		return width * 2;
	else if (dwFourCC == FCC('YVYU'))
		return width * 2;
	else if (dwFourCC == FCC('Y41P'))
		return width * 3 / 2;
	else if (dwFourCC == FCC('YV16'))
		return width;
	else if (dwFourCC == FCC('Y42B'))
		return width;
	else if (	dwFourCC == FCC('Y800') ||
				dwFourCC == FCC('  Y8') ||
				dwFourCC == FCC('Y8  ') ||
				dwFourCC == FCC('GREY'))
		return width;
	else
		return 0;
}

__forceinline int CalcYUVSize(DWORD dwFourCC, int stride, int height)
{
	if (dwFourCC == FCC('YV12'))
		return stride * height * 3 / 2;
	else if (	dwFourCC == FCC('I420') ||
				dwFourCC == FCC('IYUV'))
		return stride * height * 3 / 2;
	else if (dwFourCC == FCC('YVU9'))
		return stride * height * 9 / 8;
	else if (dwFourCC == FCC('YUV9'))
		return stride * height * 9 / 8;
	else if (	dwFourCC == FCC('YUY2') ||
				dwFourCC == FCC('V422')	||
				dwFourCC == FCC('VYUY')	||
				dwFourCC == FCC('YUNV') ||
				dwFourCC == FCC('YUYV'))
		return stride * height;
	else if (	dwFourCC == FCC('UYVY') ||
				dwFourCC == FCC('Y422') ||
				dwFourCC == FCC('UYNV'))
		return  stride * height;
	else if (dwFourCC == FCC('YVYU'))
		return stride * height;
	else if (dwFourCC == FCC('Y41P'))
		return stride * height;
	else if (dwFourCC == FCC('YV16'))
		return stride * height * 2;
	else if (dwFourCC == FCC('Y42B'))
		return stride * height * 2;
	else if (	dwFourCC == FCC('Y800') ||
				dwFourCC == FCC('  Y8') ||
				dwFourCC == FCC('Y8  ') ||
				dwFourCC == FCC('GREY'))
		return stride * height;
	else
		return 0;
}

#endif
