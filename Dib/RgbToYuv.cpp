#include "stdafx.h"
#include "..\Round.h"
#include "RgbToYuv.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int g_RGB2YUV_YR[256];
int g_RGB2YUV_YG[256];
int g_RGB2YUV_YB[256];
int g_RGB2YUV_UR[256];
int g_RGB2YUV_UG[256];
int g_RGB2YUV_UBVR[256];
int g_RGB2YUV_VG[256];
int g_RGB2YUV_VB[256];

void InitRGBToYUVTable()
{
	int i;
	for (i = 0; i < 256; i++)
		g_RGB2YUV_YR[i] = Round((65.481 * (double)(i<<8)));
	for (i = 0; i < 256; i++)
		g_RGB2YUV_YG[i] = Round((128.553 * (double)(i<<8)));
	for (i = 0; i < 256; i++)
		g_RGB2YUV_YB[i] = Round((24.966 * (double)(i<<8)));
	for (i = 0; i < 256; i++)
		g_RGB2YUV_UR[i] = Round((37.797 * (double)(i<<8)));
	for (i = 0; i < 256; i++)
		g_RGB2YUV_UG[i] = Round((74.203 * (double)(i<<8)));
	for (i = 0; i < 256; i++)
		g_RGB2YUV_VG[i] = Round((93.786 * (double)(i<<8)));
	for (i = 0; i < 256; i++)
		g_RGB2YUV_VB[i] = Round((18.214 * (double)(i<<8)));
	for (i = 0; i < 256; i++)
		g_RGB2YUV_UBVR[i] = (int)(112 * (i<<8));
}

bool RGB24ToYUV(	DWORD dwFourCC,
					unsigned char *src,	// RGB24 Dib
					unsigned char *dst,	// YUV format depending from dwFourCC
					int width,
					int height,
					int stride/*=0*/)
{
	// Check
	if (!src || !dst || (width <= 0) || (height <= 0))
		return false;

	// Select the Right Encoder
	if (dwFourCC == FCC('YV12'))
	{
		return RGB24ToYV12(	src,
							dst,
							width,
							height,
							stride);
	}
	else if (	dwFourCC == FCC('I420') ||
				dwFourCC == FCC('IYUV'))
	{
		return RGB24ToI420(	src,
							dst,
							width,
							height,
							stride);
	}
	else if (dwFourCC == FCC('YVU9'))
	{
		return RGB24ToYVU9(	src,
							dst,
							width,
							height,
							stride);
	}
	else if (dwFourCC == FCC('YUV9'))
	{
		return RGB24ToYUV9(	src,
							dst,
							width,
							height,
							stride);
	}
	else if (	dwFourCC == FCC('YUY2') ||
				dwFourCC == FCC('V422')	||
				dwFourCC == FCC('VYUY')	||
				dwFourCC == FCC('YUNV') ||
				dwFourCC == FCC('YUYV'))
	{
		return RGB24ToYUY2(	src,
							dst,
							width,
							height,
							stride);
	}
	else if (	dwFourCC == FCC('UYVY') ||
				dwFourCC == FCC('Y422') ||
				dwFourCC == FCC('UYNV'))
	{
		return RGB24ToUYVY(	src,
							dst,
							width,
							height,
							stride);
	}
	else if (dwFourCC == FCC('YVYU'))
	{
		return RGB24ToYVYU(	src,
							dst,
							width,
							height,
							stride);
	}
	else if (dwFourCC == FCC('Y41P'))
	{
		return RGB24ToY41P(	src,
							dst,
							width,
							height,
							stride);
	}
	else if (dwFourCC == FCC('YV16'))
	{
		return RGB24ToYV16(	src,
							dst,
							width,
							height,
							stride);
	}
	else if (dwFourCC == FCC('Y42B'))
	{
		return RGB24ToY42B(	src,
							dst,
							width,
							height,
							stride);
	}
	else if (	dwFourCC == FCC('Y800') ||
				dwFourCC == FCC('  Y8') ||
				dwFourCC == FCC('Y8  ') ||
				dwFourCC == FCC('GREY'))
	{
		return RGB24ToY800(	src,
							dst,
							width,
							height,
							stride);
	}
	
	return false;
}

bool RGB32ToYUV(	DWORD dwFourCC,
					unsigned char *src,	// RGB32 Dib
					unsigned char *dst,	// YUV format depending from dwFourCC
					int width,
					int height,
					int stride/*=0*/)
{
	// Check
	if (!src || !dst || (width <= 0) || (height <= 0))
		return false;

	// Select the Right Encoder
	if (dwFourCC == FCC('YV12'))
	{
		return RGB32ToYV12(	src,
							dst,
							width,
							height,
							stride);
	}
	else if (	dwFourCC == FCC('I420') ||
				dwFourCC == FCC('IYUV'))
	{
		return RGB32ToI420(	src,
							dst,
							width,
							height,
							stride);
	}
	else if (dwFourCC == FCC('YVU9'))
	{
		return RGB32ToYVU9(	src,
							dst,
							width,
							height,
							stride);
	}
	else if (dwFourCC == FCC('YUV9'))
	{
		return RGB32ToYUV9(	src,
							dst,
							width,
							height,
							stride);
	}
	else if (	dwFourCC == FCC('YUY2') ||
				dwFourCC == FCC('V422')	||
				dwFourCC == FCC('VYUY')	||
				dwFourCC == FCC('YUNV') ||
				dwFourCC == FCC('YUYV'))
	{
		return RGB32ToYUY2(	src,
							dst,
							width,
							height,
							stride);
	}
	else if (	dwFourCC == FCC('UYVY') ||
				dwFourCC == FCC('Y422') ||
				dwFourCC == FCC('UYNV'))
	{
		return RGB32ToUYVY(	src,
							dst,
							width,
							height,
							stride);
	}
	else if (dwFourCC == FCC('YVYU'))
	{
		return RGB32ToYVYU(	src,
							dst,
							width,
							height,
							stride);
	}
	else if (dwFourCC == FCC('Y41P'))
	{
		return RGB32ToY41P(	src,
							dst,
							width,
							height,
							stride);
	}
	else if (dwFourCC == FCC('YV16'))
	{
		return RGB32ToYV16(	src,
							dst,
							width,
							height,
							stride);
	}
	else if (dwFourCC == FCC('Y42B'))
	{
		return RGB32ToY42B(	src,
							dst,
							width,
							height,
							stride);
	}
	else if (	dwFourCC == FCC('Y800') ||
				dwFourCC == FCC('  Y8') ||
				dwFourCC == FCC('Y8  ') ||
				dwFourCC == FCC('GREY'))
	{
		return RGB32ToY800(	src,
							dst,
							width,
							height,
							stride);
	}
	
	return false;
}

// V and U Planes have half the stride of the Y Plane!
bool RGB24ToYV12(	unsigned char *src,	// RGB24 Dib
					unsigned char *dst,	// Y Plane, V Plane and U Plane
					int width,
					int height,
					int stride/*=0*/)
{
	unsigned char *y, *u, *v;
	int *u_int,*v_int,*uu_int,*vv_int;
	int *pu1,*pu2,*pu3,*pu4;
	int *pv1,*pv2,*pv3,*pv4;
	unsigned char *r,*g,*b;
	int i,j;

	// Check
	if (width%2 || height%2)
		return false;

	// Set Stride if not set
	if (stride <= 0)
		stride = width;

	// Use int and not unsigned char because it is faster,
	// but uses more memory... 
	uu_int = new int[width*height];
	if (uu_int == NULL)
		return false;
	vv_int = new int[width*height];
	if (vv_int == NULL)
	{
		delete [] uu_int;
		return false;
	}

	y=dst;
	u_int=uu_int;
	v_int=vv_int;

	int nDWAlignedLineSize = DWALIGNEDWIDTHBYTES(width * 24);

	// Get YUV values from RGB values
	for (i = height - 1 ; i >= 0 ; i--)
	{
		b = src + i*nDWAlignedLineSize;
		g = src + i*nDWAlignedLineSize + 1;
		r = src + i*nDWAlignedLineSize + 2;
		for (j = 0 ; j < width ; j++)
		{
			*y++=(unsigned char)(( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			*u_int++=(-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			*v_int++=( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=3;
			g+=3;
			b+=3;
		}
		y += stride - width;
	}

	// Now sample the U & V to obtain YUV 4:2:0 format

	// Sampling mechanism...
/*	  @  ->  Y
	  #  ->  U or V
	  
	  @   @   @   @
		#       #
	  @   @   @   @
	
	  @   @   @   @
		#       #
	  @   @   @   @

*/

	// Get the right pointers
	v=dst+stride*height;
	u=v+(stride*height)/4;

	// For U
	pu1=uu_int;
	pu2=pu1+1;
	pu3=pu1+width;
	pu4=pu3+1;

	// For V
	pv1=vv_int;
	pv2=pv1+1;
	pv3=pv1+width;
	pv4=pv3+1;

	// Do sampling
	for (i = 0 ; i < height ; i += 2)
	{
		for (j = 0 ; j < width ; j += 2)
		{
			*u++=(unsigned char)((*pu1+*pu2+*pu3+*pu4)>>2);
			*v++=(unsigned char)((*pv1+*pv2+*pv3+*pv4)>>2);

			pu1+=2;
			pu2+=2;
			pu3+=2;
			pu4+=2;

			pv1+=2;
			pv2+=2;
			pv3+=2;
			pv4+=2;
		}
	
		pu1+=width;
		pu2+=width;
		pu3+=width;
		pu4+=width;

		pv1+=width;
		pv2+=width;
		pv3+=width;
		pv4+=width;

		u += (stride - width) >> 1;
		v += (stride - width) >> 1;
	}

	delete [] uu_int;
	delete [] vv_int;

	return true;
}

// V and U Planes have half the stride of the Y Plane!
bool RGB32ToYV12(	unsigned char *src,	// RGB32 Dib
					unsigned char *dst,	// Y Plane, V Plane and U Plane
					int width,
					int height,
					int stride/*=0*/)
{
	unsigned char *y, *u, *v;
	int *u_int,*v_int,*uu_int,*vv_int;
	int *pu1,*pu2,*pu3,*pu4;
	int *pv1,*pv2,*pv3,*pv4;
	unsigned char *r,*g,*b;
	int i,j;

	// Check
	if (width%2 || height%2)
		return false;

	// Set Stride if not set
	if (stride <= 0)
		stride = width;

	// Use int and not unsigned char because it is faster,
	// but uses more memory... 
	uu_int = new int[width*height];
	if (uu_int == NULL)
		return false;
	vv_int = new int[width*height];
	if (vv_int == NULL)
	{
		delete [] uu_int;
		return false;
	}

	y=dst;
	u_int=uu_int;
	v_int=vv_int;

	int nDWAlignedLineSize = width << 2;

	// Get YUV values from RGB values
	for (i = height - 1 ; i >= 0 ; i--)
	{
		b = src + i*nDWAlignedLineSize;
		g = src + i*nDWAlignedLineSize + 1;
		r = src + i*nDWAlignedLineSize + 2;
		for (j = 0 ; j < width ; j++)
		{
			*y++=(unsigned char)(( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			*u_int++=(-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			*v_int++=( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=4;
			g+=4;
			b+=4;
		}
		y += stride - width;
	}

	// Now sample the U & V to obtain YUV 4:2:0 format

	// Sampling mechanism...
/*	  @  ->  Y
	  #  ->  U or V
	  
	  @   @   @   @
		#       #
	  @   @   @   @
	
	  @   @   @   @
		#       #
	  @   @   @   @

*/

	// Get the right pointers
	v=dst+stride*height;
	u=v+(stride*height)/4;

	// For U
	pu1=uu_int;
	pu2=pu1+1;
	pu3=pu1+width;
	pu4=pu3+1;

	// For V
	pv1=vv_int;
	pv2=pv1+1;
	pv3=pv1+width;
	pv4=pv3+1;

	// Do sampling
	for (i = 0 ; i < height ; i += 2)
	{
		for (j = 0 ; j < width ; j += 2)
		{
			*u++=(unsigned char)((*pu1+*pu2+*pu3+*pu4)>>2);
			*v++=(unsigned char)((*pv1+*pv2+*pv3+*pv4)>>2);

			pu1+=2;
			pu2+=2;
			pu3+=2;
			pu4+=2;

			pv1+=2;
			pv2+=2;
			pv3+=2;
			pv4+=2;
		}
	
		pu1+=width;
		pu2+=width;
		pu3+=width;
		pu4+=width;

		pv1+=width;
		pv2+=width;
		pv3+=width;
		pv4+=width;

		u += (stride - width) >> 1;
		v += (stride - width) >> 1;
	}

	delete [] uu_int;
	delete [] vv_int;

	return true;
}

// Equivalent FCC Is: IYUV
// U and V Planes have half the stride of the Y Plane!
bool RGB24ToI420(	unsigned char *src,	// RGB24 Dib
					unsigned char *dst,	// Y Plane, U Plane and V Plane
					int width,
					int height,
					int stride/*=0*/)
{
	unsigned char *y, *u, *v;
	int *u_int,*v_int,*uu_int,*vv_int;
	int *pu1,*pu2,*pu3,*pu4;
	int *pv1,*pv2,*pv3,*pv4;
	unsigned char *r,*g,*b;
	int i,j;

	// Check
	if (width%2 || height%2)
		return false;

	// Set Stride if not set
	if (stride <= 0)
		stride = width;

	// Use int and not unsigned char because it is faster,
	// but uses more memory... 
	uu_int = new int[width*height];
	if (uu_int == NULL)
		return false;
	vv_int = new int[width*height];
	if (vv_int == NULL)
	{
		delete [] uu_int;
		return false;
	}

	y=dst;
	u_int=uu_int;
	v_int=vv_int;

	int nDWAlignedLineSize = DWALIGNEDWIDTHBYTES(width * 24);

	// Get YUV values from RGB values
	for (i = height - 1 ; i >= 0 ; i--)
	{
		b = src + i*nDWAlignedLineSize;
		g = src + i*nDWAlignedLineSize + 1;
		r = src + i*nDWAlignedLineSize + 2;
		for (j = 0 ; j < width ; j++)
		{
			*y++=(unsigned char)(( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			*u_int++=(-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			*v_int++=( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=3;
			g+=3;
			b+=3;
		}
		y += stride - width;
	}

	// Now sample the U & V to obtain YUV 4:2:0 format

	// Sampling mechanism...
/*	  @  ->  Y
	  #  ->  U or V
	  
	  @   @   @   @
		#       #
	  @   @   @   @
	
	  @   @   @   @
		#       #
	  @   @   @   @

*/

	// Get the right pointers
	u=dst+stride*height;
	v=u+(stride*height)/4;

	// For U
	pu1=uu_int;
	pu2=pu1+1;
	pu3=pu1+width;
	pu4=pu3+1;

	// For V
	pv1=vv_int;
	pv2=pv1+1;
	pv3=pv1+width;
	pv4=pv3+1;

	// Do sampling
	for (i = 0 ; i < height ; i += 2)
	{
		for (j = 0 ; j < width ; j += 2)
		{
			*u++=(unsigned char)((*pu1+*pu2+*pu3+*pu4)>>2);
			*v++=(unsigned char)((*pv1+*pv2+*pv3+*pv4)>>2);

			pu1+=2;
			pu2+=2;
			pu3+=2;
			pu4+=2;

			pv1+=2;
			pv2+=2;
			pv3+=2;
			pv4+=2;
		}
	
		pu1+=width;
		pu2+=width;
		pu3+=width;
		pu4+=width;

		pv1+=width;
		pv2+=width;
		pv3+=width;
		pv4+=width;

		u += (stride - width) >> 1;
		v += (stride - width) >> 1;
	}

	delete [] uu_int;
	delete [] vv_int;

	return true;
}

// Equivalent FCC Is: IYUV
// U and V Planes have half the stride of the Y Plane!
bool RGB32ToI420(	unsigned char *src,	// RGB32 Dib
					unsigned char *dst,	// Y Plane, U Plane and V Plane
					int width,
					int height,
					int stride/*=0*/)
{
	unsigned char *y, *u, *v;
	int *u_int,*v_int,*uu_int,*vv_int;
	int *pu1,*pu2,*pu3,*pu4;
	int *pv1,*pv2,*pv3,*pv4;
	unsigned char *r,*g,*b;
	int i,j;

	// Check
	if (width%2 || height%2)
		return false;

	// Set Stride if not set
	if (stride <= 0)
		stride = width;

	// Use int and not unsigned char because it is faster,
	// but uses more memory... 
	uu_int = new int[width*height];
	if (uu_int == NULL)
		return false;
	vv_int = new int[width*height];
	if (vv_int == NULL)
	{
		delete [] uu_int;
		return false;
	}

	y=dst;
	u_int=uu_int;
	v_int=vv_int;

	int nDWAlignedLineSize = width << 2;

	// Get YUV values from RGB values
	for (i = height - 1 ; i >= 0 ; i--)
	{
		b = src + i*nDWAlignedLineSize;
		g = src + i*nDWAlignedLineSize + 1;
		r = src + i*nDWAlignedLineSize + 2;
		for (j = 0 ; j < width ; j++)
		{
			*y++=(unsigned char)(( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			*u_int++=(-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			*v_int++=( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=4;
			g+=4;
			b+=4;
		}
		y += stride - width;
	}

	// Now sample the U & V to obtain YUV 4:2:0 format

	// Sampling mechanism...
/*	  @  ->  Y
	  #  ->  U or V
	  
	  @   @   @   @
		#       #
	  @   @   @   @
	
	  @   @   @   @
		#       #
	  @   @   @   @

*/

	// Get the right pointers
	u=dst+stride*height;
	v=u+(stride*height)/4;

	// For U
	pu1=uu_int;
	pu2=pu1+1;
	pu3=pu1+width;
	pu4=pu3+1;

	// For V
	pv1=vv_int;
	pv2=pv1+1;
	pv3=pv1+width;
	pv4=pv3+1;

	// Do sampling
	for (i = 0 ; i < height ; i += 2)
	{
		for (j = 0 ; j < width ; j += 2)
		{
			*u++=(unsigned char)((*pu1+*pu2+*pu3+*pu4)>>2);
			*v++=(unsigned char)((*pv1+*pv2+*pv3+*pv4)>>2);

			pu1+=2;
			pu2+=2;
			pu3+=2;
			pu4+=2;

			pv1+=2;
			pv2+=2;
			pv3+=2;
			pv4+=2;
		}
	
		pu1+=width;
		pu2+=width;
		pu3+=width;
		pu4+=width;

		pv1+=width;
		pv2+=width;
		pv3+=width;
		pv4+=width;

		u += (stride - width) >> 1;
		v += (stride - width) >> 1;
	}

	delete [] uu_int;
	delete [] vv_int;

	return true;
}

bool RGB24ToYV16(	unsigned char *src,	// RGB24 Dib
					unsigned char *dst,	// Y Plane, V Plane and U Plane
					int width,
					int height,
					int stride/*=0*/)
{
	int u0, u1, v0, v1;
	unsigned char y0, y1;
	unsigned char *r, *g, *b;
	int i, j;

	// Check
	if (width%2)
		return false;

	// Set Stride if not set
	if (stride <= 0)
		stride = width;

	int nChromaOffset = stride * height;
	int nChromaSize = stride * height / 2;
	unsigned char* pv = dst + nChromaOffset;
	unsigned char* pu = dst + nChromaOffset + nChromaSize;

	int nDWAlignedLineSize = DWALIGNEDWIDTHBYTES(width * 24);

	// Get YUV values from RGB values
	for (i = height - 1 ; i >= 0 ; i--)
	{
		b = src + i*nDWAlignedLineSize;
		g = src + i*nDWAlignedLineSize + 1;
		r = src + i*nDWAlignedLineSize + 2;
		for (j = 0 ; j < width ; j += 2)
		{
			y0 = (unsigned char)(( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			u0 = (-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			v0 = ( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=3;
			g+=3;
			b+=3;
			y1 = (unsigned char)(( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			u1 = (-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			v1 = ( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=3;
			g+=3;
			b+=3;
			*dst++ = y0;
			*dst++ = y1;
			*pu++ = (unsigned char)((u0+u1)>>1);
			*pv++ = (unsigned char)((v0+v1)>>1);
		}
		dst += stride - width;
		pu  += (stride - width) >> 1;
		pv  += (stride - width) >> 1;
	}

	return true;
}

bool RGB32ToYV16(	unsigned char *src,	// RGB32 Dib
					unsigned char *dst,	// Y Plane, V Plane and U Plane
					int width,
					int height,
					int stride/*=0*/)
{
	int u0, u1, v0, v1;
	unsigned char y0, y1;
	unsigned char *r, *g, *b;
	int i, j;

	// Check
	if (width%2)
		return false;

	// Set Stride if not set
	if (stride <= 0)
		stride = width;

	int nChromaOffset = stride * height;
	int nChromaSize = stride * height / 2;
	unsigned char* pv = dst + nChromaOffset;
	unsigned char* pu = dst + nChromaOffset + nChromaSize;

	int nDWAlignedLineSize = width << 2;

	// Get YUV values from RGB values
	for (i = height - 1 ; i >= 0 ; i--)
	{
		b = src + i*nDWAlignedLineSize;
		g = src + i*nDWAlignedLineSize + 1;
		r = src + i*nDWAlignedLineSize + 2;
		for (j = 0 ; j < width ; j += 2)
		{
			y0 = (unsigned char)(( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			u0 = (-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			v0 = ( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=4;
			g+=4;
			b+=4;
			y1 = (unsigned char)(( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			u1 = (-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			v1 = ( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=4;
			g+=4;
			b+=4;
			*dst++ = y0;
			*dst++ = y1;
			*pu++ = (unsigned char)((u0+u1)>>1);
			*pv++ = (unsigned char)((v0+v1)>>1);
		}
		dst += stride - width;
		pu  += (stride - width) >> 1;
		pv  += (stride - width) >> 1;
	}

	return true;
}

bool RGB24ToY42B(	unsigned char *src,	// RGB24 Dib
					unsigned char *dst,	// Y Plane, U Plane and V Plane
					int width,
					int height,
					int stride/*=0*/)
{
	int u0, u1, v0, v1;
	unsigned char y0, y1;
	unsigned char *r, *g, *b;
	int i, j;

	// Check
	if (width%2)
		return false;

	// Set Stride if not set
	if (stride <= 0)
		stride = width;

	int nChromaOffset = stride * height;
	int nChromaSize = stride * height / 2;
	unsigned char* pu = dst + nChromaOffset;
	unsigned char* pv = dst + nChromaOffset + nChromaSize;

	int nDWAlignedLineSize = DWALIGNEDWIDTHBYTES(width * 24);

	// Get YUV values from RGB values
	for (i = height - 1 ; i >= 0 ; i--)
	{
		b = src + i*nDWAlignedLineSize;
		g = src + i*nDWAlignedLineSize + 1;
		r = src + i*nDWAlignedLineSize + 2;
		for (j = 0 ; j < width ; j += 2)
		{
			y0 = (unsigned char)(( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			u0 = (-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			v0 = ( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=3;
			g+=3;
			b+=3;
			y1 = (unsigned char)(( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			u1 = (-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			v1 = ( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=3;
			g+=3;
			b+=3;
			*dst++ = y0;
			*dst++ = y1;
			*pu++ = (unsigned char)((u0+u1)>>1);
			*pv++ = (unsigned char)((v0+v1)>>1);
		}
		dst += stride - width;
		pu  += (stride - width) >> 1;
		pv  += (stride - width) >> 1;
	}

	return true;
}

bool RGB32ToY42B(	unsigned char *src,	// RGB32 Dib
					unsigned char *dst,	// Y Plane, U Plane and V Plane
					int width,
					int height,
					int stride/*=0*/)
{
	int u0, u1, v0, v1;
	unsigned char y0, y1;
	unsigned char *r, *g, *b;
	int i, j;

	// Check
	if (width%2)
		return false;

	// Set Stride if not set
	if (stride <= 0)
		stride = width;

	int nChromaOffset = stride * height;
	int nChromaSize = stride * height / 2;
	unsigned char* pu = dst + nChromaOffset;
	unsigned char* pv = dst + nChromaOffset + nChromaSize;

	int nDWAlignedLineSize = width << 2;

	// Get YUV values from RGB values
	for (i = height - 1 ; i >= 0 ; i--)
	{
		b = src + i*nDWAlignedLineSize;
		g = src + i*nDWAlignedLineSize + 1;
		r = src + i*nDWAlignedLineSize + 2;
		for (j = 0 ; j < width ; j += 2)
		{
			y0 = (unsigned char)(( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			u0 = (-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			v0 = ( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=4;
			g+=4;
			b+=4;
			y1 = (unsigned char)(( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			u1 = (-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			v1 = ( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=4;
			g+=4;
			b+=4;
			*dst++ = y0;
			*dst++ = y1;
			*pu++ = (unsigned char)((u0+u1)>>1);
			*pv++ = (unsigned char)((v0+v1)>>1);
		}
		dst += stride - width;
		pu  += (stride - width) >> 1;
		pv  += (stride - width) >> 1;
	}

	return true;
}

bool RGB24ToYVU9(	unsigned char *src,	// RGB24 Dib
					unsigned char *dst,	// Y Plane, V Plane and U Plane
					int width,
					int height,
					int stride/*=0*/)
{
	unsigned char *y, *u, *v;
	int *u_int,*v_int,*uu_int,*vv_int;
	int *pu1,*pu2,*pu3,*pu4,*pu5,*pu6,*pu7,*pu8,*pu9,*pu10,*pu11,*pu12,*pu13,*pu14,*pu15,*pu16;
	int *pv1,*pv2,*pv3,*pv4,*pv5,*pv6,*pv7,*pv8,*pv9,*pv10,*pv11,*pv12,*pv13,*pv14,*pv15,*pv16;
	unsigned char *r,*g,*b;
	int i,j;

	// Check
	if (width%4 || height%4)
		return false;

	// Set Stride if not set
	if (stride <= 0)
		stride = width;

	// Use int and not unsigned char because it is faster,
	// but uses more memory... 
	uu_int = new int[width*height];
	if (uu_int == NULL)
		return false;
	vv_int = new int[width*height];
	if (vv_int == NULL)
	{
		delete [] uu_int;
		return false;
	}

	y=dst;
	u_int=uu_int;
	v_int=vv_int;

	int nDWAlignedLineSize = DWALIGNEDWIDTHBYTES(width * 24);

	// Get YUV values from RGB values
	for (i = height - 1 ; i >= 0 ; i--)
	{
		b = src + i*nDWAlignedLineSize;
		g = src + i*nDWAlignedLineSize + 1;
		r = src + i*nDWAlignedLineSize + 2;
		for (j = 0 ; j < width ; j++)
		{
			*y++=(unsigned char)(( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			*u_int++=(-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			*v_int++=( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=3;
			g+=3;
			b+=3;
		}
		y += stride - width;
	}

	// Now sample the U & V to obtain YUV 4:1:0 format

	// Sampling mechanism...
/*	  @  ->  Y
	  #  ->  U or V
	  
	  @   @   @   @   @   @   @   @
	
	  @   @   @   @   @   @   @   @
	        #               #
	  @   @   @   @   @   @   @   @

	  @   @   @   @   @   @   @   @

      @   @   @   @   @   @   @   @
	
	  @   @   @   @   @   @   @   @
	        #               #
	  @   @   @   @   @   @   @   @

	  @   @   @   @   @   @   @   @

*/

	// Get the right pointers
	v=dst+stride*height;
	u=v+(stride*height)/16;

	// For U
	pu1=uu_int;
	pu2=pu1+1;
	pu3=pu2+1;
	pu4=pu3+1;
	pu5=pu1+width;
	pu6=pu5+1;
	pu7=pu6+1;
	pu8=pu7+1;
	pu9=pu5+width;
	pu10=pu9+1;
	pu11=pu10+1;
	pu12=pu11+1;
	pu13=pu9+width;
	pu14=pu13+1;
	pu15=pu14+1;
	pu16=pu15+1;

	// For V
	pv1=vv_int;
	pv2=pv1+1;
	pv3=pv2+1;
	pv4=pv3+1;
	pv5=pv1+width;
	pv6=pv5+1;
	pv7=pv6+1;
	pv8=pv7+1;
	pv9=pv5+width;
	pv10=pv9+1;
	pv11=pv10+1;
	pv12=pv11+1;
	pv13=pv9+width;
	pv14=pv13+1;
	pv15=pv14+1;
	pv16=pv15+1;

	// Do sampling
	int n3Width = 3*width;
	for (i = 0 ; i < height ; i += 4)
	{
		for (j = 0 ; j < width ; j += 4)
		{
			*u++=(unsigned char)((*pu1+*pu2+*pu3+*pu4+*pu5+*pu6+*pu7+*pu8+*pu9+*pu10+*pu11+*pu12+*pu13+*pu14+*pu15+*pu16)>>4);
			*v++=(unsigned char)((*pv1+*pv2+*pv3+*pv4+*pv5+*pv6+*pv7+*pv8+*pv9+*pv10+*pv11+*pv12+*pv13+*pv14+*pv15+*pv16)>>4);

			pu1+=4;
			pu2+=4;
			pu3+=4;
			pu4+=4;
			pu5+=4;
			pu6+=4;
			pu7+=4;
			pu8+=4;
			pu9+=4;
			pu10+=4;
			pu11+=4;
			pu12+=4;
			pu13+=4;
			pu14+=4;
			pu15+=4;
			pu16+=4;

			pv1+=4;
			pv2+=4;
			pv3+=4;
			pv4+=4;
			pv5+=4;
			pv6+=4;
			pv7+=4;
			pv8+=4;
			pv9+=4;
			pv10+=4;
			pv11+=4;
			pv12+=4;
			pv13+=4;
			pv14+=4;
			pv15+=4;
			pv16+=4;
		}
	
		pu1+=n3Width;
		pu2+=n3Width;
		pu3+=n3Width;
		pu4+=n3Width;
		pu5+=n3Width;
		pu6+=n3Width;
		pu7+=n3Width;
		pu8+=n3Width;
		pu9+=n3Width;
		pu10+=n3Width;
		pu11+=n3Width;
		pu12+=n3Width;
		pu13+=n3Width;
		pu14+=n3Width;
		pu15+=n3Width;
		pu16+=n3Width;

		pv1+=n3Width;
		pv2+=n3Width;
		pv3+=n3Width;
		pv4+=n3Width;
		pv5+=n3Width;
		pv6+=n3Width;
		pv7+=n3Width;
		pv8+=n3Width;
		pv9+=n3Width;
		pv10+=n3Width;
		pv11+=n3Width;
		pv12+=n3Width;
		pv13+=n3Width;
		pv14+=n3Width;
		pv15+=n3Width;
		pv16+=n3Width;

		u += (stride - width) >> 2;
		v += (stride - width) >> 2;
	}

	delete [] uu_int;
	delete [] vv_int;

	return true;
}

bool RGB32ToYVU9(	unsigned char *src,	// RGB32 Dib
					unsigned char *dst,	// Y Plane, V Plane and U Plane
					int width,
					int height,
					int stride/*=0*/)
{
	unsigned char *y, *u, *v;
	int *u_int,*v_int,*uu_int,*vv_int;
	int *pu1,*pu2,*pu3,*pu4,*pu5,*pu6,*pu7,*pu8,*pu9,*pu10,*pu11,*pu12,*pu13,*pu14,*pu15,*pu16;
	int *pv1,*pv2,*pv3,*pv4,*pv5,*pv6,*pv7,*pv8,*pv9,*pv10,*pv11,*pv12,*pv13,*pv14,*pv15,*pv16;
	unsigned char *r,*g,*b;
	int i,j;

	// Check
	if (width%4 || height%4)
		return false;

	// Set Stride if not set
	if (stride <= 0)
		stride = width;

	// Use int and not unsigned char because it is faster,
	// but uses more memory... 
	uu_int = new int[width*height];
	if (uu_int == NULL)
		return false;
	vv_int = new int[width*height];
	if (vv_int == NULL)
	{
		delete [] uu_int;
		return false;
	}

	y=dst;
	u_int=uu_int;
	v_int=vv_int;

	int nDWAlignedLineSize = width << 2;

	// Get YUV values from RGB values
	for (i = height - 1 ; i >= 0 ; i--)
	{
		b = src + i*nDWAlignedLineSize;
		g = src + i*nDWAlignedLineSize + 1;
		r = src + i*nDWAlignedLineSize + 2;
		for (j = 0 ; j < width ; j++)
		{
			*y++=(unsigned char)(( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			*u_int++=(-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			*v_int++=( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=4;
			g+=4;
			b+=4;
		}
		y += stride - width;
	}

	// Now sample the U & V to obtain YUV 4:1:0 format

	// Sampling mechanism...
/*	  @  ->  Y
	  #  ->  U or V
	  
	  @   @   @   @   @   @   @   @
	
	  @   @   @   @   @   @   @   @
	        #               #
	  @   @   @   @   @   @   @   @

	  @   @   @   @   @   @   @   @

      @   @   @   @   @   @   @   @
	
	  @   @   @   @   @   @   @   @
	        #               #
	  @   @   @   @   @   @   @   @

	  @   @   @   @   @   @   @   @

*/

	// Get the right pointers
	v=dst+stride*height;
	u=v+(stride*height)/16;

	// For U
	pu1=uu_int;
	pu2=pu1+1;
	pu3=pu2+1;
	pu4=pu3+1;
	pu5=pu1+width;
	pu6=pu5+1;
	pu7=pu6+1;
	pu8=pu7+1;
	pu9=pu5+width;
	pu10=pu9+1;
	pu11=pu10+1;
	pu12=pu11+1;
	pu13=pu9+width;
	pu14=pu13+1;
	pu15=pu14+1;
	pu16=pu15+1;

	// For V
	pv1=vv_int;
	pv2=pv1+1;
	pv3=pv2+1;
	pv4=pv3+1;
	pv5=pv1+width;
	pv6=pv5+1;
	pv7=pv6+1;
	pv8=pv7+1;
	pv9=pv5+width;
	pv10=pv9+1;
	pv11=pv10+1;
	pv12=pv11+1;
	pv13=pv9+width;
	pv14=pv13+1;
	pv15=pv14+1;
	pv16=pv15+1;

	// Do sampling
	int n3Width = 3*width;
	for (i = 0 ; i < height ; i += 4)
	{
		for (j = 0 ; j < width ; j += 4)
		{
			*u++=(unsigned char)((*pu1+*pu2+*pu3+*pu4+*pu5+*pu6+*pu7+*pu8+*pu9+*pu10+*pu11+*pu12+*pu13+*pu14+*pu15+*pu16)>>4);
			*v++=(unsigned char)((*pv1+*pv2+*pv3+*pv4+*pv5+*pv6+*pv7+*pv8+*pv9+*pv10+*pv11+*pv12+*pv13+*pv14+*pv15+*pv16)>>4);

			pu1+=4;
			pu2+=4;
			pu3+=4;
			pu4+=4;
			pu5+=4;
			pu6+=4;
			pu7+=4;
			pu8+=4;
			pu9+=4;
			pu10+=4;
			pu11+=4;
			pu12+=4;
			pu13+=4;
			pu14+=4;
			pu15+=4;
			pu16+=4;

			pv1+=4;
			pv2+=4;
			pv3+=4;
			pv4+=4;
			pv5+=4;
			pv6+=4;
			pv7+=4;
			pv8+=4;
			pv9+=4;
			pv10+=4;
			pv11+=4;
			pv12+=4;
			pv13+=4;
			pv14+=4;
			pv15+=4;
			pv16+=4;
		}
	
		pu1+=n3Width;
		pu2+=n3Width;
		pu3+=n3Width;
		pu4+=n3Width;
		pu5+=n3Width;
		pu6+=n3Width;
		pu7+=n3Width;
		pu8+=n3Width;
		pu9+=n3Width;
		pu10+=n3Width;
		pu11+=n3Width;
		pu12+=n3Width;
		pu13+=n3Width;
		pu14+=n3Width;
		pu15+=n3Width;
		pu16+=n3Width;

		pv1+=n3Width;
		pv2+=n3Width;
		pv3+=n3Width;
		pv4+=n3Width;
		pv5+=n3Width;
		pv6+=n3Width;
		pv7+=n3Width;
		pv8+=n3Width;
		pv9+=n3Width;
		pv10+=n3Width;
		pv11+=n3Width;
		pv12+=n3Width;
		pv13+=n3Width;
		pv14+=n3Width;
		pv15+=n3Width;
		pv16+=n3Width;

		u += (stride - width) >> 2;
		v += (stride - width) >> 2;
	}

	delete [] uu_int;
	delete [] vv_int;

	return true;
}

bool RGB24ToYUV9(	unsigned char *src,	// RGB24 Dib
					unsigned char *dst,	// Y Plane, U Plane and V Plane
					int width,
					int height,
					int stride/*=0*/)
{
	unsigned char *y, *u, *v;
	int *u_int,*v_int,*uu_int,*vv_int;
	int *pu1,*pu2,*pu3,*pu4,*pu5,*pu6,*pu7,*pu8,*pu9,*pu10,*pu11,*pu12,*pu13,*pu14,*pu15,*pu16;
	int *pv1,*pv2,*pv3,*pv4,*pv5,*pv6,*pv7,*pv8,*pv9,*pv10,*pv11,*pv12,*pv13,*pv14,*pv15,*pv16;
	unsigned char *r,*g,*b;
	int i,j;

	// Check
	if (width%4 || height%4)
		return false;

	// Set Stride if not set
	if (stride <= 0)
		stride = width;

	// Use int and not unsigned char because it is faster,
	// but uses more memory... 
	uu_int = new int[width*height];
	if (uu_int == NULL)
		return false;
	vv_int = new int[width*height];
	if (vv_int == NULL)
	{
		delete [] uu_int;
		return false;
	}

	y=dst;
	u_int=uu_int;
	v_int=vv_int;

	int nDWAlignedLineSize = DWALIGNEDWIDTHBYTES(width * 24);

	// Get YUV values from RGB values
	for (i = height - 1 ; i >= 0 ; i--)
	{
		b = src + i*nDWAlignedLineSize;
		g = src + i*nDWAlignedLineSize + 1;
		r = src + i*nDWAlignedLineSize + 2;
		for (j = 0 ; j < width ; j++)
		{
			*y++=(unsigned char)(( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			*u_int++=(-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			*v_int++=( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=3;
			g+=3;
			b+=3;
		}
		y += stride - width;
	}

	// Now sample the U & V to obtain YUV 4:1:0 format

	// Sampling mechanism...
/*	  @  ->  Y
	  #  ->  U or V
	  
	  @   @   @   @   @   @   @   @
	
	  @   @   @   @   @   @   @   @
	        #               #
	  @   @   @   @   @   @   @   @

	  @   @   @   @   @   @   @   @

      @   @   @   @   @   @   @   @
	
	  @   @   @   @   @   @   @   @
	        #               #
	  @   @   @   @   @   @   @   @

	  @   @   @   @   @   @   @   @

*/

	// Get the right pointers
	u=dst+stride*height;
	v=u+(stride*height)/16;

	// For U
	pu1=uu_int;
	pu2=pu1+1;
	pu3=pu2+1;
	pu4=pu3+1;
	pu5=pu1+width;
	pu6=pu5+1;
	pu7=pu6+1;
	pu8=pu7+1;
	pu9=pu5+width;
	pu10=pu9+1;
	pu11=pu10+1;
	pu12=pu11+1;
	pu13=pu9+width;
	pu14=pu13+1;
	pu15=pu14+1;
	pu16=pu15+1;

	// For V
	pv1=vv_int;
	pv2=pv1+1;
	pv3=pv2+1;
	pv4=pv3+1;
	pv5=pv1+width;
	pv6=pv5+1;
	pv7=pv6+1;
	pv8=pv7+1;
	pv9=pv5+width;
	pv10=pv9+1;
	pv11=pv10+1;
	pv12=pv11+1;
	pv13=pv9+width;
	pv14=pv13+1;
	pv15=pv14+1;
	pv16=pv15+1;

	// Do sampling
	int n3Width = 3*width;
	for (i = 0 ; i < height ; i += 4)
	{
		for (j = 0 ; j < width ; j += 4)
		{
			*u++=(unsigned char)((*pu1+*pu2+*pu3+*pu4+*pu5+*pu6+*pu7+*pu8+*pu9+*pu10+*pu11+*pu12+*pu13+*pu14+*pu15+*pu16)>>4);
			*v++=(unsigned char)((*pv1+*pv2+*pv3+*pv4+*pv5+*pv6+*pv7+*pv8+*pv9+*pv10+*pv11+*pv12+*pv13+*pv14+*pv15+*pv16)>>4);

			pu1+=4;
			pu2+=4;
			pu3+=4;
			pu4+=4;
			pu5+=4;
			pu6+=4;
			pu7+=4;
			pu8+=4;
			pu9+=4;
			pu10+=4;
			pu11+=4;
			pu12+=4;
			pu13+=4;
			pu14+=4;
			pu15+=4;
			pu16+=4;

			pv1+=4;
			pv2+=4;
			pv3+=4;
			pv4+=4;
			pv5+=4;
			pv6+=4;
			pv7+=4;
			pv8+=4;
			pv9+=4;
			pv10+=4;
			pv11+=4;
			pv12+=4;
			pv13+=4;
			pv14+=4;
			pv15+=4;
			pv16+=4;
		}
	
		pu1+=n3Width;
		pu2+=n3Width;
		pu3+=n3Width;
		pu4+=n3Width;
		pu5+=n3Width;
		pu6+=n3Width;
		pu7+=n3Width;
		pu8+=n3Width;
		pu9+=n3Width;
		pu10+=n3Width;
		pu11+=n3Width;
		pu12+=n3Width;
		pu13+=n3Width;
		pu14+=n3Width;
		pu15+=n3Width;
		pu16+=n3Width;

		pv1+=n3Width;
		pv2+=n3Width;
		pv3+=n3Width;
		pv4+=n3Width;
		pv5+=n3Width;
		pv6+=n3Width;
		pv7+=n3Width;
		pv8+=n3Width;
		pv9+=n3Width;
		pv10+=n3Width;
		pv11+=n3Width;
		pv12+=n3Width;
		pv13+=n3Width;
		pv14+=n3Width;
		pv15+=n3Width;
		pv16+=n3Width;

		u += (stride - width) >> 2;
		v += (stride - width) >> 2;
	}

	delete [] uu_int;
	delete [] vv_int;

	return true;
}

bool RGB32ToYUV9(	unsigned char *src,	// RGB32 Dib
					unsigned char *dst,	// Y Plane, U Plane and V Plane
					int width,
					int height,
					int stride/*=0*/)
{
	unsigned char *y, *u, *v;
	int *u_int,*v_int,*uu_int,*vv_int;
	int *pu1,*pu2,*pu3,*pu4,*pu5,*pu6,*pu7,*pu8,*pu9,*pu10,*pu11,*pu12,*pu13,*pu14,*pu15,*pu16;
	int *pv1,*pv2,*pv3,*pv4,*pv5,*pv6,*pv7,*pv8,*pv9,*pv10,*pv11,*pv12,*pv13,*pv14,*pv15,*pv16;
	unsigned char *r,*g,*b;
	int i,j;

	// Check
	if (width%4 || height%4)
		return false;

	// Set Stride if not set
	if (stride <= 0)
		stride = width;

	// Use int and not unsigned char because it is faster,
	// but uses more memory... 
	uu_int = new int[width*height];
	if (uu_int == NULL)
		return false;
	vv_int = new int[width*height];
	if (vv_int == NULL)
	{
		delete [] uu_int;
		return false;
	}

	y=dst;
	u_int=uu_int;
	v_int=vv_int;

	int nDWAlignedLineSize = width << 2;

	// Get YUV values from RGB values
	for (i = height - 1 ; i >= 0 ; i--)
	{
		b = src + i*nDWAlignedLineSize;
		g = src + i*nDWAlignedLineSize + 1;
		r = src + i*nDWAlignedLineSize + 2;
		for (j = 0 ; j < width ; j++)
		{
			*y++=(unsigned char)(( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			*u_int++=(-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			*v_int++=( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=4;
			g+=4;
			b+=4;
		}
		y += stride - width;
	}

	// Now sample the U & V to obtain YUV 4:1:0 format

	// Sampling mechanism...
/*	  @  ->  Y
	  #  ->  U or V
	  
	  @   @   @   @   @   @   @   @
	
	  @   @   @   @   @   @   @   @
	        #               #
	  @   @   @   @   @   @   @   @

	  @   @   @   @   @   @   @   @

      @   @   @   @   @   @   @   @
	
	  @   @   @   @   @   @   @   @
	        #               #
	  @   @   @   @   @   @   @   @

	  @   @   @   @   @   @   @   @

*/

	// Get the right pointers
	u=dst+stride*height;
	v=u+(stride*height)/16;

	// For U
	pu1=uu_int;
	pu2=pu1+1;
	pu3=pu2+1;
	pu4=pu3+1;
	pu5=pu1+width;
	pu6=pu5+1;
	pu7=pu6+1;
	pu8=pu7+1;
	pu9=pu5+width;
	pu10=pu9+1;
	pu11=pu10+1;
	pu12=pu11+1;
	pu13=pu9+width;
	pu14=pu13+1;
	pu15=pu14+1;
	pu16=pu15+1;

	// For V
	pv1=vv_int;
	pv2=pv1+1;
	pv3=pv2+1;
	pv4=pv3+1;
	pv5=pv1+width;
	pv6=pv5+1;
	pv7=pv6+1;
	pv8=pv7+1;
	pv9=pv5+width;
	pv10=pv9+1;
	pv11=pv10+1;
	pv12=pv11+1;
	pv13=pv9+width;
	pv14=pv13+1;
	pv15=pv14+1;
	pv16=pv15+1;

	// Do sampling
	int n3Width = 3*width;
	for (i = 0 ; i < height ; i += 4)
	{
		for (j = 0 ; j < width ; j += 4)
		{
			*u++=(unsigned char)((*pu1+*pu2+*pu3+*pu4+*pu5+*pu6+*pu7+*pu8+*pu9+*pu10+*pu11+*pu12+*pu13+*pu14+*pu15+*pu16)>>4);
			*v++=(unsigned char)((*pv1+*pv2+*pv3+*pv4+*pv5+*pv6+*pv7+*pv8+*pv9+*pv10+*pv11+*pv12+*pv13+*pv14+*pv15+*pv16)>>4);

			pu1+=4;
			pu2+=4;
			pu3+=4;
			pu4+=4;
			pu5+=4;
			pu6+=4;
			pu7+=4;
			pu8+=4;
			pu9+=4;
			pu10+=4;
			pu11+=4;
			pu12+=4;
			pu13+=4;
			pu14+=4;
			pu15+=4;
			pu16+=4;

			pv1+=4;
			pv2+=4;
			pv3+=4;
			pv4+=4;
			pv5+=4;
			pv6+=4;
			pv7+=4;
			pv8+=4;
			pv9+=4;
			pv10+=4;
			pv11+=4;
			pv12+=4;
			pv13+=4;
			pv14+=4;
			pv15+=4;
			pv16+=4;
		}
	
		pu1+=n3Width;
		pu2+=n3Width;
		pu3+=n3Width;
		pu4+=n3Width;
		pu5+=n3Width;
		pu6+=n3Width;
		pu7+=n3Width;
		pu8+=n3Width;
		pu9+=n3Width;
		pu10+=n3Width;
		pu11+=n3Width;
		pu12+=n3Width;
		pu13+=n3Width;
		pu14+=n3Width;
		pu15+=n3Width;
		pu16+=n3Width;

		pv1+=n3Width;
		pv2+=n3Width;
		pv3+=n3Width;
		pv4+=n3Width;
		pv5+=n3Width;
		pv6+=n3Width;
		pv7+=n3Width;
		pv8+=n3Width;
		pv9+=n3Width;
		pv10+=n3Width;
		pv11+=n3Width;
		pv12+=n3Width;
		pv13+=n3Width;
		pv14+=n3Width;
		pv15+=n3Width;
		pv16+=n3Width;

		u += (stride - width) >> 2;
		v += (stride - width) >> 2;
	}

	delete [] uu_int;
	delete [] vv_int;

	return true;
}

// Equivalent FCCs Are: YUNV and YUYV
bool RGB24ToYUY2(	unsigned char *src,	// RGB24 Dib
					unsigned char *dst,	// Y0 U0 Y1 V0, Y2 U2 Y3 V2, ...
					int width,
					int height,
					int stride/*=0*/)
{
	int u0, u1, v0, v1;
	unsigned char y0, y1;
	unsigned char *r, *g, *b;
	int i, j;

	// Check
	if (width%2)
		return false;

	// Set Stride if not set
	if (stride <= 0)
		stride = 2 * width;

	int nDWAlignedLineSize = DWALIGNEDWIDTHBYTES(width * 24);

	// Get YUV values from RGB values
	for (i = height - 1 ; i >= 0 ; i--)
	{
		b = src + i*nDWAlignedLineSize;
		g = src + i*nDWAlignedLineSize + 1;
		r = src + i*nDWAlignedLineSize + 2;
		for (j = 0 ; j < width ; j += 2)
		{
			y0 = (unsigned char)(( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			u0 = (-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			v0 = ( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=3;
			g+=3;
			b+=3;
			y1 = (unsigned char)(( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			u1 = (-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			v1 = ( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=3;
			g+=3;
			b+=3;
			*dst++ = y0;
			*dst++ = (unsigned char)((u0+u1)>>1);
			*dst++ = y1;
			*dst++ = (unsigned char)((v0+v1)>>1);
		}
		dst += stride - 2 * width;
	}

	return true;
}

// Equivalent FCCs Are: YUNV and YUYV
bool RGB32ToYUY2(	unsigned char *src,	// RGB32 Dib
					unsigned char *dst,	// Y0 U0 Y1 V0, Y2 U2 Y3 V2, ...
					int width,
					int height,
					int stride/*=0*/)
{
	int u0, u1, v0, v1;
	unsigned char y0, y1;
	unsigned char *r, *g, *b;
	int i, j;

	// Check
	if (width%2)
		return false;

	// Set Stride if not set
	if (stride <= 0)
		stride = 2 * width;

	int nDWAlignedLineSize = width << 2;

	// Get YUV values from RGB values
	for (i = height - 1 ; i >= 0 ; i--)
	{
		b = src + i*nDWAlignedLineSize;
		g = src + i*nDWAlignedLineSize + 1;
		r = src + i*nDWAlignedLineSize + 2;
		for (j = 0 ; j < width ; j += 2)
		{
			y0 = (unsigned char)(( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			u0 = (-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			v0 = ( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=4;
			g+=4;
			b+=4;
			y1 = (unsigned char)(( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			u1 = (-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			v1 = ( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=4;
			g+=4;
			b+=4;
			*dst++ = y0;
			*dst++ = (unsigned char)((u0+u1)>>1);
			*dst++ = y1;
			*dst++ = (unsigned char)((v0+v1)>>1);
		}
		dst += stride - 2 * width;
	}

	return true;
}

// Equivalent FCCs Are: Y422 and UYNV
bool RGB24ToUYVY(	unsigned char *src,	// RGB24 Dib
					unsigned char *dst,	// U0 Y0 V0 Y1, U2 Y2 V2 Y3, ...
					int width,
					int height,
					int stride/*=0*/)
{
	int u0, u1, v0, v1;
	unsigned char y0, y1;
	unsigned char *r, *g, *b;
	int i, j;

	// Check
	if (width%2)
		return false;

	// Set Stride if not set
	if (stride <= 0)
		stride = 2 * width;

	int nDWAlignedLineSize = DWALIGNEDWIDTHBYTES(width * 24);

	// Get YUV values from RGB values
	for (i = height - 1 ; i >= 0 ; i--)
	{
		b = src + i*nDWAlignedLineSize;
		g = src + i*nDWAlignedLineSize + 1;
		r = src + i*nDWAlignedLineSize + 2;
		for (j = 0 ; j < width ; j += 2)
		{
			y0 = (unsigned char)(( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			u0 = (-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			v0 = ( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=3;
			g+=3;
			b+=3;
			y1 = (unsigned char)(( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			u1 = (-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			v1 = ( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=3;
			g+=3;
			b+=3;
			*dst++ = (unsigned char)((u0+u1)>>1);
			*dst++ = y0;
			*dst++ = (unsigned char)((v0+v1)>>1);
			*dst++ = y1;
		}
		dst += stride - 2 * width;
	}

	return true;
}

// Equivalent FCCs Are: Y422 and UYNV
bool RGB32ToUYVY(	unsigned char *src,	// RGB32 Dib
					unsigned char *dst,	// U0 Y0 V0 Y1, U2 Y2 V2 Y3, ...
					int width,
					int height,
					int stride/*=0*/)
{
	int u0, u1, v0, v1;
	unsigned char y0, y1;
	unsigned char *r, *g, *b;
	int i, j;

	// Check
	if (width%2)
		return false;

	// Set Stride if not set
	if (stride <= 0)
		stride = 2 * width;

	int nDWAlignedLineSize = width << 2;

	// Get YUV values from RGB values
	for (i = height - 1 ; i >= 0 ; i--)
	{
		b = src + i*nDWAlignedLineSize;
		g = src + i*nDWAlignedLineSize + 1;
		r = src + i*nDWAlignedLineSize + 2;
		for (j = 0 ; j < width ; j += 2)
		{
			y0 = (unsigned char)(( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			u0 = (-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			v0 = ( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=4;
			g+=4;
			b+=4;
			y1 = (unsigned char)(( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			u1 = (-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			v1 = ( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=4;
			g+=4;
			b+=4;
			*dst++ = (unsigned char)((u0+u1)>>1);
			*dst++ = y0;
			*dst++ = (unsigned char)((v0+v1)>>1);
			*dst++ = y1;
		}
		dst += stride - 2 * width;
	}

	return true;
}

bool RGB24ToYVYU(	unsigned char *src,	// RGB24 Dib
					unsigned char *dst,	// Y0 V0 Y1 U0, Y2 V2 Y3 U2, ...
					int width,
					int height,
					int stride/*=0*/)
{
	int u0, u1, v0, v1;
	unsigned char y0, y1;
	unsigned char *r, *g, *b;
	int i, j;

	// Check
	if (width%2)
		return false;

	// Set Stride if not set
	if (stride <= 0)
		stride = 2 * width;

	int nDWAlignedLineSize = DWALIGNEDWIDTHBYTES(width * 24);

	// Get YUV values from RGB values
	for (i = height - 1 ; i >= 0 ; i--)
	{
		b = src + i*nDWAlignedLineSize;
		g = src + i*nDWAlignedLineSize + 1;
		r = src + i*nDWAlignedLineSize + 2;
		for (j = 0 ; j < width ; j += 2)
		{
			y0 = (unsigned char)(( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			u0 = (-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			v0 = ( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=3;
			g+=3;
			b+=3;
			y1 = (unsigned char)(( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			u1 = (-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			v1 = ( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=3;
			g+=3;
			b+=3;
			*dst++ = y0;
			*dst++ = (unsigned char)((v0+v1)>>1);
			*dst++ = y1;
			*dst++ = (unsigned char)((u0+u1)>>1);
		}
		dst += stride - 2 * width;
	}

	return true;
}

bool RGB32ToYVYU(	unsigned char *src,	// RGB32 Dib
					unsigned char *dst,	// Y0 V0 Y1 U0, Y2 V2 Y3 U2, ...
					int width,
					int height,
					int stride/*=0*/)
{
	int u0, u1, v0, v1;
	unsigned char y0, y1;
	unsigned char *r, *g, *b;
	int i, j;

	// Check
	if (width%2)
		return false;

	// Set Stride if not set
	if (stride <= 0)
		stride = 2 * width;

	int nDWAlignedLineSize = width << 2;

	// Get YUV values from RGB values
	for (i = height - 1 ; i >= 0 ; i--)
	{
		b = src + i*nDWAlignedLineSize;
		g = src + i*nDWAlignedLineSize + 1;
		r = src + i*nDWAlignedLineSize + 2;
		for (j = 0 ; j < width ; j += 2)
		{
			y0 = (unsigned char)(( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			u0 = (-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			v0 = ( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=4;
			g+=4;
			b+=4;
			y1 = (unsigned char)(( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			u1 = (-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			v1 = ( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=4;
			g+=4;
			b+=4;
			*dst++ = y0;
			*dst++ = (unsigned char)((v0+v1)>>1);
			*dst++ = y1;
			*dst++ = (unsigned char)((u0+u1)>>1);
		}
		dst += stride - 2 * width;
	}

	return true;
}

bool RGB24ToY41P(	unsigned char *src,	// RGB24 Dib
					unsigned char *dst,	// U0 Y0 V0 Y1 U4 Y2 V4 Y3 Y4 Y5 Y6 Y7
					int width,
					int height,
					int stride/*=0*/)
{
	int u0, u4, v0, v4;
	unsigned char y0, y1, y2, y3, y4, y5, y6, y7;
	unsigned char *r, *g, *b;
	int i, j;

	// Check
	if (width%8)
		return false;

	// Set Stride if not set
	if (stride <= 0)
		stride = width * 3 / 2;

	int nDWAlignedLineSize = DWALIGNEDWIDTHBYTES(width * 24);

	// Get YUV values from RGB values
	for (i = height - 1 ; i >= 0 ; i--)
	{
		b = src + i*nDWAlignedLineSize;
		g = src + i*nDWAlignedLineSize + 1;
		r = src + i*nDWAlignedLineSize + 2;
		for (j = 0 ; j < width ; j += 8)
		{
			y0 = (unsigned char)(( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			u0 = (-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			v0 = ( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=3;
			g+=3;
			b+=3;
			y1 = (unsigned char)(( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			u0 += (-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			v0 += ( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=3;
			g+=3;
			b+=3;
			y2 = (unsigned char)(( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			u0 += (-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			v0 += ( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=3;
			g+=3;
			b+=3;
			y3 = (unsigned char)(( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			u0 += (-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			v0 += ( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=3;
			g+=3;
			b+=3;
			y4 = (unsigned char)(( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			u4 = (-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			v4 = ( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=3;
			g+=3;
			b+=3;
			y5 = (unsigned char)(( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			u4 += (-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			v4 += ( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=3;
			g+=3;
			b+=3;
			y6 = (unsigned char)(( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			u4 += (-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			v4 += ( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=3;
			g+=3;
			b+=3;
			y7 = (unsigned char)(( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			u4 += (-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			v4 += ( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=3;
			g+=3;
			b+=3;
			*dst++ = (unsigned char)(u0>>2);
			*dst++ = y0;
			*dst++ = (unsigned char)(v0>>2);
			*dst++ = y1;
			*dst++ = (unsigned char)(u4>>2);
			*dst++ = y2;
			*dst++ = (unsigned char)(v4>>2);
			*dst++ = y3;
			*dst++ = y4;
			*dst++ = y5;
			*dst++ = y6;
			*dst++ = y7;
		}
		dst += stride - width * 3 / 2;
	}

	return true;
}

bool RGB32ToY41P(	unsigned char *src,	// RGB32 Dib
					unsigned char *dst,	// U0 Y0 V0 Y1 U4 Y2 V4 Y3 Y4 Y5 Y6 Y7
					int width,
					int height,
					int stride/*=0*/)
{
	int u0, u4, v0, v4;
	unsigned char y0, y1, y2, y3, y4, y5, y6, y7;
	unsigned char *r, *g, *b;
	int i, j;

	// Check
	if (width%8)
		return false;

	// Set Stride if not set
	if (stride <= 0)
		stride = width * 3 / 2;

	int nDWAlignedLineSize = width << 2;

	// Get YUV values from RGB values
	for (i = height - 1 ; i >= 0 ; i--)
	{
		b = src + i*nDWAlignedLineSize;
		g = src + i*nDWAlignedLineSize + 1;
		r = src + i*nDWAlignedLineSize + 2;
		for (j = 0 ; j < width ; j += 8)
		{
			y0 = (unsigned char)(( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			u0 = (-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			v0 = ( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=4;
			g+=4;
			b+=4;
			y1 = (unsigned char)(( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			u0 += (-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			v0 += ( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=4;
			g+=4;
			b+=4;
			y2 = (unsigned char)(( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			u0 += (-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			v0 += ( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=4;
			g+=4;
			b+=4;
			y3 = (unsigned char)(( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			u0 += (-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			v0 += ( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=4;
			g+=4;
			b+=4;
			y4 = (unsigned char)(( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			u4 = (-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			v4 = ( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=4;
			g+=4;
			b+=4;
			y5 = (unsigned char)(( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			u4 += (-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			v4 += ( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=4;
			g+=4;
			b+=4;
			y6 = (unsigned char)(( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			u4 += (-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			v4 += ( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=4;
			g+=4;
			b+=4;
			y7 = (unsigned char)(( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			u4 += (-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			v4 += ( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=4;
			g+=4;
			b+=4;
			*dst++ = (unsigned char)(u0>>2);
			*dst++ = y0;
			*dst++ = (unsigned char)(v0>>2);
			*dst++ = y1;
			*dst++ = (unsigned char)(u4>>2);
			*dst++ = y2;
			*dst++ = (unsigned char)(v4>>2);
			*dst++ = y3;
			*dst++ = y4;
			*dst++ = y5;
			*dst++ = y6;
			*dst++ = y7;
		}
		dst += stride - width * 3 / 2;
	}

	return true;
}

bool RGB24ToY800(	unsigned char *src,	// RGB24 Dib
					unsigned char *dst,	// Y Plane
					int width,
					int height,
					int stride/*=0*/)
{
	unsigned char *r,*g,*b;
	int i,j;

	// Set Stride if not set
	if (stride <= 0)
		stride = width;

	int nDWAlignedLineSize = DWALIGNEDWIDTHBYTES(width * 24);
	for (i = height - 1 ; i >= 0 ; i--)
	{
		b = src + i*nDWAlignedLineSize;
		g = src + i*nDWAlignedLineSize + 1;
		r = src + i*nDWAlignedLineSize + 2;
		for (j = 0 ; j < width ; j++)
		{
			*dst++=(unsigned char)((g_RGB2YUV_YR[*r]+g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			r+=3;
			g+=3;
			b+=3;
		}
		dst += stride - width;
	}

	return true;
}

bool RGB32ToY800(	unsigned char *src,	// RGB32 Dib
					unsigned char *dst,	// Y Plane
					int width,
					int height,
					int stride/*=0*/)
{
	unsigned char *r,*g,*b;
	int i,j;

	// Set Stride if not set
	if (stride <= 0)
		stride = width;

	int nDWAlignedLineSize = width << 2;
	for (i = height - 1 ; i >= 0 ; i--)
	{
		b = src + i*nDWAlignedLineSize;
		g = src + i*nDWAlignedLineSize + 1;
		r = src + i*nDWAlignedLineSize + 2;
		for (j = 0 ; j < width ; j++)
		{
			*dst++=(unsigned char)((g_RGB2YUV_YR[*r]+g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16);
			r+=4;
			g+=4;
			b+=4;
		}
		dst += stride - width;
	}

	return true;
}

bool RGB24ToYUV420(	unsigned char *src,	// RGB24 Dib
					unsigned int *dst,	// Y Plane, U Plane and V Plane, unsigned int size for Y, U, V!
					int width,
					int height)
{
	unsigned int *u,*v,*y,*uu,*vv;
	unsigned int *pu1,*pu2,*pu3,*pu4;
	unsigned int *pv1,*pv2,*pv3,*pv4;
	unsigned char *r,*g,*b;
	int i,j;

	// Check
	if (width%2 || height%2)
		return false;

	uu = new unsigned int[width*height];
	if (uu == NULL)
		return false;
	vv = new unsigned int[width*height];
	if (vv == NULL)
	{
		delete [] uu;
		return false;
	}

	y=dst;
	u=uu;
	v=vv;

	int nDWAlignedLineSize = DWALIGNEDWIDTHBYTES(width * 24);

	// Get YUV values from RGB values
	for (i = 0 ; i < height ; i++)
	{
		b = src + i*nDWAlignedLineSize;
		g = src + i*nDWAlignedLineSize + 1;
		r = src + i*nDWAlignedLineSize + 2;
		for (j = 0 ; j < width ; j++)
		{
			*y++=( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16;
			*u++=(-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			*v++=( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=3;
			g+=3;
			b+=3;
		}
	}

	// Now sample the U & V to obtain YUV 4:2:0 format

	// Sampling mechanism...
/*	  @  ->  Y
	  #  ->  U or V
	  
	  @   @   @   @
		#       #
	  @   @   @   @
	
	  @   @   @   @
		#       #
	  @   @   @   @

*/

	// Get the right pointers
	u=dst+width*height;
	v=u+(width*height)/4;

	// For U
	pu1=uu;
	pu2=pu1+1;
	pu3=pu1+width;
	pu4=pu3+1;

	// For V
	pv1=vv;
	pv2=pv1+1;
	pv3=pv1+width;
	pv4=pv3+1;

	// Do sampling
	for (i = 0 ; i < height ; i += 2)
	{
		for (j = 0 ; j < width ; j += 2)
		{
			*u++=(*pu1+*pu2+*pu3+*pu4)>>2;
			*v++=(*pv1+*pv2+*pv3+*pv4)>>2;

			pu1+=2;
			pu2+=2;
			pu3+=2;
			pu4+=2;

			pv1+=2;
			pv2+=2;
			pv3+=2;
			pv4+=2;
		}
	
		pu1+=width;
		pu2+=width;
		pu3+=width;
		pu4+=width;

		pv1+=width;
		pv2+=width;
		pv3+=width;
		pv4+=width;
	}

	delete [] uu;
	delete [] vv;

	return true;
}

bool RGB32ToYUV420(	unsigned char *src,	// RGB32 Dib
					unsigned int *dst,	// Y Plane, U Plane and V Plane, unsigned int size for Y, U, V!
					int width,
					int height)
{
	unsigned int *u,*v,*y,*uu,*vv;
	unsigned int *pu1,*pu2,*pu3,*pu4;
	unsigned int *pv1,*pv2,*pv3,*pv4;
	unsigned char *r,*g,*b;
	int i,j;

	// Check
	if (width%2 || height%2)
		return false;

	uu = new unsigned int[width*height];
	if (uu == NULL)
		return false;
	vv = new unsigned int[width*height];
	if (vv == NULL)
	{
		delete [] uu;
		return false;
	}

	y=dst;
	u=uu;
	v=vv;

	int nDWAlignedLineSize = width << 2;

	// Get YUV values from RGB values
	for (i = 0 ; i < height ; i++)
	{
		b = src + i*nDWAlignedLineSize;
		g = src + i*nDWAlignedLineSize + 1;
		r = src + i*nDWAlignedLineSize + 2;
		for (j = 0 ; j < width ; j++)
		{
			*y++=( g_RGB2YUV_YR[*r]  +g_RGB2YUV_YG[*g]+g_RGB2YUV_YB[*b]+1048576)>>16;
			*u++=(-g_RGB2YUV_UR[*r]  -g_RGB2YUV_UG[*g]+g_RGB2YUV_UBVR[*b]+8388608)>>16;
			*v++=( g_RGB2YUV_UBVR[*r]-g_RGB2YUV_VG[*g]-g_RGB2YUV_VB[*b]+8388608)>>16;
			r+=4;
			g+=4;
			b+=4;
		}
	}

	// Now sample the U & V to obtain YUV 4:2:0 format

	// Sampling mechanism...
/*	  @  ->  Y
	  #  ->  U or V
	  
	  @   @   @   @
		#       #
	  @   @   @   @
	
	  @   @   @   @
		#       #
	  @   @   @   @

*/

	// Get the right pointers
	u=dst+width*height;
	v=u+(width*height)/4;

	// For U
	pu1=uu;
	pu2=pu1+1;
	pu3=pu1+width;
	pu4=pu3+1;

	// For V
	pv1=vv;
	pv2=pv1+1;
	pv3=pv1+width;
	pv4=pv3+1;

	// Do sampling
	for (i = 0 ; i < height ; i += 2)
	{
		for (j = 0 ; j < width ; j += 2)
		{
			*u++=(*pu1+*pu2+*pu3+*pu4)>>2;
			*v++=(*pv1+*pv2+*pv3+*pv4)>>2;

			pu1+=2;
			pu2+=2;
			pu3+=2;
			pu4+=2;

			pv1+=2;
			pv2+=2;
			pv3+=2;
			pv4+=2;
		}
	
		pu1+=width;
		pu2+=width;
		pu3+=width;
		pu4+=width;

		pv1+=width;
		pv2+=width;
		pv3+=width;
		pv4+=width;
	}

	delete [] uu;
	delete [] vv;

	return true;
}
