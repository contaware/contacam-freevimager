#include "stdafx.h"
#include "YuvToRgb.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// C lut
static int g_crv_tab[256];
static int g_cbu_tab[256];
static int g_cgu_tab[256];
static int g_cgv_tab[256];
static int g_y_tab[256];
static unsigned char g_clip[1024];
void InitYUVToRGBTable()
{
	int i, ind;   
     
	const int crv = 104597;
	const int cbu = 132201;
	const int cgu = 25675;
	const int cgv = 53279;

	for (i = 0 ; i < 256 ; i++)
	{
		g_crv_tab[i] = (i-128) * crv;
		g_cbu_tab[i] = (i-128) * cbu;
		g_cgu_tab[i] = (i-128) * cgu;
		g_cgv_tab[i] = (i-128) * cgv;
		g_y_tab[i] = 76309*(i-16);
	}
 
	for (i = 0 ; i < 384 ; i++)
		g_clip[i] = 0;
	ind=384;
	for (i = 0 ; i < 256 ; i++)
		g_clip[ind++] = i;
	ind=640;
	for (i = 0 ; i < 384 ; i++)
		g_clip[ind++] = 255;
}

bool YUVToRGB32(	DWORD dwFourCC,
					unsigned char *src,	// YUV format depending from dwFourCC
					unsigned char *dst,	// RGB32 Dib
					int width,
					int height)
{
	// Check
	if (!src || !dst || (width <= 0) || (height <= 0))
		return false;

	// Select the Right Decoder
	if (dwFourCC == FCC('YV12'))
	{
		int nChromaOffset = width * height;
		int nChromaSize = width * height / 4;
		if (g_bMMX && (width & 1) == 0 && (height & 1) == 0)
		{
			YUV420ToRGB32Asm(src,								// Y Plane
							src + nChromaOffset + nChromaSize,	// U Plane
							src + nChromaOffset,				// V Plane
							(LPDWORD)dst,						// RGB32 Dib
							width,
							height);
		}
		else
		{
			YUV420ToRGB32(	src,								// Y Plane
							src + nChromaOffset + nChromaSize,	// U Plane
							src + nChromaOffset,				// V Plane
							(LPDWORD)dst,						// RGB32 Dib
							width,
							height);
		}
	}
	else if (	dwFourCC == FCC('I420') ||
				dwFourCC == FCC('IYUV'))
	{
		int nChromaOffset = width * height;
		int nChromaSize = width * height / 4;
		if (g_bMMX && (width & 1) == 0 && (height & 1) == 0)
		{
			YUV420ToRGB32Asm(src,								// Y Plane
							src + nChromaOffset,				// U Plane
							src + nChromaOffset + nChromaSize,	// V Plane
							(LPDWORD)dst,						// RGB32 Dib
							width,
							height);
		}
		else
		{
			YUV420ToRGB32(	src,								// Y Plane
							src + nChromaOffset,				// U Plane
							src + nChromaOffset + nChromaSize,	// V Plane
							(LPDWORD)dst,						// RGB32 Dib
							width,
							height);
		}
	}
	else if (dwFourCC == FCC('YVU9'))
	{
		YVU9ToRGB32(	src,
						dst,
						width,
						height);
	}
	else if (dwFourCC == FCC('YUV9'))
	{
		YUV9ToRGB32(	src,
						dst,
						width,
						height);
	}
	else if (	dwFourCC == FCC('YUY2') ||
				dwFourCC == FCC('V422')	||
				dwFourCC == FCC('VYUY')	||
				dwFourCC == FCC('YUNV') ||
				dwFourCC == FCC('YUYV'))
	{
		if (g_bMMX && (width & 1) == 0)
		{
			YUY2ToRGB32Asm(	src,
							(LPDWORD)dst,
							width,
							height);
		}
		else
		{
			YUY2ToRGB32(src,
						(LPDWORD)dst,
						width,
						height);
		}
	}
	else if (	dwFourCC == FCC('UYVY') ||
				dwFourCC == FCC('Y422') ||
				dwFourCC == FCC('UYNV'))
	{
		if (g_bMMX && (width & 1) == 0)
		{
			UYVYToRGB32Asm(	src,
							(LPDWORD)dst,
							width,
							height);
		}
		else
		{
			UYVYToRGB32(src,
						(LPDWORD)dst,
						width,
						height);
		}
	}
	else if (dwFourCC == FCC('YVYU'))
	{
		if (g_bMMX && (width & 1) == 0)
		{
			YVYUToRGB32Asm(	src,
							(LPDWORD)dst,
							width,
							height);
		}
		else
		{
			YVYUToRGB32(src,
						(LPDWORD)dst,
						width,
						height);
		}
	}
	else if (dwFourCC == FCC('VCR1'))
	{
		VCR1ToRGB32(	src,
						dst,
						width,
						height);
	}
	else if (dwFourCC == FCC('CLJR'))
	{
		CLJRToRGB32(	src,
						dst,
						width,
						height);
	}
	else if (dwFourCC == FCC('cyuv'))
	{
		cyuvToRGB32(	src,
						dst,
						width,
						height);
	}
	else if (	dwFourCC == FCC('VIXL') ||
				dwFourCC == FCC('PIXL'))
	{
		VIXLToRGB32(	src,
						dst,
						width,
						height);
	}
	else if (dwFourCC == FCC('Y41P'))
	{
		Y41PToRGB32(	src,
						dst,
						width,
						height);
	}
	else if (dwFourCC == FCC('YV16'))
	{
		YV16ToRGB32(	src,
						dst,
						width,
						height);
	}
	else if (dwFourCC == FCC('Y42B'))
	{
		Y42BToRGB32(	src,
						dst,
						width,
						height);
	}
	else if (	dwFourCC == FCC('Y800') ||
				dwFourCC == FCC('Y8  ') ||
				dwFourCC == FCC('GREY'))
	{
		Y800ToRGB32(	src,
						dst,
						width,
						height);
	}
	else
		return false;

	return true;
}

void YUV420ToRGB32(	LPBYTE src0,	// Y Plane
					LPBYTE src1,	// U Plane
					LPBYTE src2,	// V Plane
					LPDWORD dst,	// RGB32 Dib
					int width,
					int height)
{
	for (int j = (height - 1) ; j > 0 ; j -= 2)
	{
		LPBYTE py1 = src0 + (height - 1 - j) * width;
		LPBYTE py2 = py1 + width;
		LPDWORD d1 = dst + j * width;
		LPDWORD d2 = d1 - width;
		for (int i = 0 ; i < width ; i += 2)
		{
			int u = *src1++;
			int v = *src2++;
			int rv = g_crv_tab[v];
			int bu = g_cbu_tab[u];
			int gugv = -g_cgu_tab[u] - g_cgv_tab[v];

			// up-left
			int y1 = g_y_tab[*py1++];	
			*d1++ = (DWORD)g_clip[384+((y1 + bu)>>16)]			|	// Blue 
					((DWORD)g_clip[384+((y1 + gugv)>>16)] << 8)	|	// Green
					((DWORD)g_clip[384+((y1 + rv)>>16)] << 16);		// Red

			// up-right
			y1 = g_y_tab[*py1++];
			*d1++ = (DWORD)g_clip[384+((y1 + bu)>>16)]			|	// Blue
					((DWORD)g_clip[384+((y1 + gugv)>>16)] << 8)	|	// Green
					((DWORD)g_clip[384+((y1 + rv)>>16)] << 16);		// Red

			// down-left
			int y2 = g_y_tab[*py2++];
			*d2++ = (DWORD)g_clip[384+((y2 + bu)>>16)]			|	// Blue
					((DWORD)g_clip[384+((y2 + gugv)>>16)] << 8)	|	// Green
					((DWORD)g_clip[384+((y2 + rv)>>16)] << 16);		// Red

			// down-right
			y2 = g_y_tab[*py2++];
			*d2++ = (DWORD)g_clip[384+((y2 + bu)>>16)]			|	// Blue
					((DWORD)g_clip[384+((y2 + gugv)>>16)] << 8)	|	// Green
					((DWORD)g_clip[384+((y2 + rv)>>16)] << 16);		// Red
		}
	}
}

void YV16ToRGB32(	unsigned char *src,	// Y Plane, V Plane and U Plane
					unsigned char *dst,	// RGB32 Dib
					int width,
					int height)
{
	int y1, y2, u, v; 
	unsigned char *pv, *pu;
	int rv, gu, gv, bu;

	int nChromaOffset = width * height;
	int nChromaSize = width * height / 2;

	pv = src + nChromaOffset;
	pu = src + nChromaOffset + nChromaSize;

	int nDWAlignedLineSize = width << 2;
	int n2DWAlignedLineSize = width << 3;
	dst = dst + (nDWAlignedLineSize * (height - 1));
 	for (int j = 0 ; j < height ; j++)
	{
		for (int i = 0 ; i < width ; i += 2)
		{
			u = *pu++;
			v = *pv++;

			rv = g_crv_tab[v];
			gu = g_cgu_tab[u];
			gv = g_cgv_tab[v];
			bu = g_cbu_tab[u];

			// up-left
            y1 = g_y_tab[*src++];	
			*dst++ = g_clip[384+((y1 + bu)>>16)];		// Blue 
			*dst++ = g_clip[384+((y1 - gu - gv)>>16)];	// Green
            *dst++ = g_clip[384+((y1 + rv)>>16)];		// Red
			*dst++ = 0;									// Alpha

			// up-right
			y2 = g_y_tab[*src++];
			*dst++ = g_clip[384+((y2 + bu)>>16)];		// Blue
			*dst++ = g_clip[384+((y2 - gu - gv)>>16)];	// Green
			*dst++ = g_clip[384+((y2 + rv)>>16)];		// Red
			*dst++ = 0;									// Alpha
		}
		dst -= n2DWAlignedLineSize;
	}
}

void Y42BToRGB32(	unsigned char *src,	// Y Plane, U Plane and V Plane
					unsigned char *dst,	// RGB32 Dib
					int width,
					int height)
{
	int y1, y2, u, v; 
	unsigned char *pv, *pu;
	int rv, gu, gv, bu;

	int nChromaOffset = width * height;
	int nChromaSize = width * height / 2;

	pu = src + nChromaOffset;
	pv = src + nChromaOffset + nChromaSize;

	int nDWAlignedLineSize = width << 2;
	int n2DWAlignedLineSize = width << 3;
	dst = dst + (nDWAlignedLineSize * (height - 1));
 	for (int j = 0 ; j < height ; j++)
	{
		for (int i = 0 ; i < width ; i += 2)
		{
			u = *pu++;
			v = *pv++;

			rv = g_crv_tab[v];
			gu = g_cgu_tab[u];
			gv = g_cgv_tab[v];
			bu = g_cbu_tab[u];

			// up-left
            y1 = g_y_tab[*src++];	
			*dst++ = g_clip[384+((y1 + bu)>>16)];		// Blue 
			*dst++ = g_clip[384+((y1 - gu - gv)>>16)];	// Green
            *dst++ = g_clip[384+((y1 + rv)>>16)];		// Red
			*dst++ = 0;									// Alpha

			// up-right
			y2 = g_y_tab[*src++];
			*dst++ = g_clip[384+((y2 + bu)>>16)];		// Blue
			*dst++ = g_clip[384+((y2 - gu - gv)>>16)];	// Green
			*dst++ = g_clip[384+((y2 + rv)>>16)];		// Red
			*dst++ = 0;									// Alpha
		}
		dst -= n2DWAlignedLineSize;
	}
}

void Y800ToRGB32(	unsigned char *src,	// Y Plane
					unsigned char *dst,	// RGB32 Dib
					int width,
					int height)
{
	int n2width = width << 1;
	DWORD* p = (DWORD*)dst;
	p += width * (height - 1);
 	for (int j = 0 ; j < height ; j++)
	{
		for (int i = 0 ; i < width ; i++)
		{
            int y = g_y_tab[*src++];
			y >>= 16;
			DWORD rgb = g_clip[384+y];
			*p++ = (DWORD)(rgb | (rgb<<8) | (rgb<<16));
		}
		p -= n2width;
	}
}

void YVU9ToRGB32(	unsigned char *src,	// Y Plane, V Plane and U Plane
					unsigned char *dst,	// RGB32 Dib
					int width,
					int height)
{
	int y1, y2, y3, y4, u, v; 
	unsigned char *py1, *py2, *py3, *py4, *pv, *pu;
	int rv, gu, gv, bu;
	unsigned char *d1, *d2, *d3, *d4;

	int nChromaOffset = width * height;
	int nChromaSize = width * height / 16;

	py1 = src;
	py2 = py1+width;
	py3 = py2+width;
	py4 = py3+width;

	pv = src + nChromaOffset;
	pu = src + nChromaOffset + nChromaSize;

	int nDWAlignedLineSize = width << 2;

 	for (int j = (height - 1) ; j > 0 ; j -= 4)
	{
		d1 = dst + j * nDWAlignedLineSize;
		d2 = d1 - nDWAlignedLineSize;
		d3 = d2 - nDWAlignedLineSize;
		d4 = d3 - nDWAlignedLineSize;
		for (int i = 0 ; i < width ; i += 4)
		{
			u = *pu++;
			v = *pv++;

			rv = g_crv_tab[v];
			gu = g_cgu_tab[u];
			gv = g_cgv_tab[v];
			bu = g_cbu_tab[u];

			// Pixel (0,0)
            y1 = g_y_tab[*py1++];	
			*d1++ = g_clip[384+((y1 + bu)>>16)];		// Blue 
			*d1++ = g_clip[384+((y1 - gu - gv)>>16)];	// Green
            *d1++ = g_clip[384+((y1 + rv)>>16)];		// Red
			*d1++ = 0;									// Alpha

			// Pixel (1,0)
			y1 = g_y_tab[*py1++];
			*d1++ = g_clip[384+((y1 + bu)>>16)];		// Blue
			*d1++ = g_clip[384+((y1 - gu - gv)>>16)];	// Green
			*d1++ = g_clip[384+((y1 + rv)>>16)];		// Red
			*d1++ = 0;									// Alpha

			// Pixel (2,0)
			y1 = g_y_tab[*py1++];
			*d1++ = g_clip[384+((y1 + bu)>>16)];		// Blue
			*d1++ = g_clip[384+((y1 - gu - gv)>>16)];	// Green
			*d1++ = g_clip[384+((y1 + rv)>>16)];		// Red
			*d1++ = 0;									// Alpha

			// Pixel (3,0)
			y1 = g_y_tab[*py1++];
			*d1++ = g_clip[384+((y1 + bu)>>16)];		// Blue
			*d1++ = g_clip[384+((y1 - gu - gv)>>16)];	// Green
			*d1++ = g_clip[384+((y1 + rv)>>16)];		// Red
			*d1++ = 0;									// Alpha

			// Pixel (0,1)
			y2 = g_y_tab[*py2++];
			*d2++ = g_clip[384+((y2 + bu)>>16)];		// Blue
			*d2++ = g_clip[384+((y2 - gu - gv)>>16)];	// Green
            *d2++ = g_clip[384+((y2 + rv)>>16)];		// Red
			*d2++ = 0;									// Alpha

			// Pixel (1,1)
			y2 = g_y_tab[*py2++];
			*d2++ = g_clip[384+((y2 + bu)>>16)];		// Blue
			*d2++ = g_clip[384+((y2 - gu - gv)>>16)];	// Green
            *d2++ = g_clip[384+((y2 + rv)>>16)];		// Red
			*d2++ = 0;									// Alpha

			// Pixel (2,1)
			y2 = g_y_tab[*py2++];
			*d2++ = g_clip[384+((y2 + bu)>>16)];		// Blue
			*d2++ = g_clip[384+((y2 - gu - gv)>>16)];	// Green
            *d2++ = g_clip[384+((y2 + rv)>>16)];		// Red
			*d2++ = 0;									// Alpha

			// Pixel (3,1)
			y2 = g_y_tab[*py2++];
			*d2++ = g_clip[384+((y2 + bu)>>16)];		// Blue
			*d2++ = g_clip[384+((y2 - gu - gv)>>16)];	// Green
            *d2++ = g_clip[384+((y2 + rv)>>16)];		// Red
			*d2++ = 0;									// Alpha

			// Pixel (0,2)
			y3 = g_y_tab[*py3++];
			*d3++ = g_clip[384+((y3 + bu)>>16)];		// Blue
			*d3++ = g_clip[384+((y3 - gu - gv)>>16)];	// Green
            *d3++ = g_clip[384+((y3 + rv)>>16)];		// Red
			*d3++ = 0;									// Alpha

			// Pixel (1,2)
			y3 = g_y_tab[*py3++];
			*d3++ = g_clip[384+((y3 + bu)>>16)];		// Blue
			*d3++ = g_clip[384+((y3 - gu - gv)>>16)];	// Green
            *d3++ = g_clip[384+((y3 + rv)>>16)];		// Red
			*d3++ = 0;									// Alpha

			// Pixel (2,2)
			y3 = g_y_tab[*py3++];
			*d3++ = g_clip[384+((y3 + bu)>>16)];		// Blue
			*d3++ = g_clip[384+((y3 - gu - gv)>>16)];	// Green
            *d3++ = g_clip[384+((y3 + rv)>>16)];		// Red
			*d3++ = 0;									// Alpha

			// Pixel (3,2)
			y3 = g_y_tab[*py3++];
			*d3++ = g_clip[384+((y3 + bu)>>16)];		// Blue
			*d3++ = g_clip[384+((y3 - gu - gv)>>16)];	// Green
            *d3++ = g_clip[384+((y3 + rv)>>16)];		// Red
			*d3++ = 0;									// Alpha

			// Pixel (0,3)
			y4 = g_y_tab[*py4++];
			*d4++ = g_clip[384+((y4 + bu)>>16)];		// Blue
			*d4++ = g_clip[384+((y4 - gu - gv)>>16)];	// Green
            *d4++ = g_clip[384+((y4 + rv)>>16)];		// Red
			*d4++ = 0;									// Alpha

			// Pixel (1,3)
			y4 = g_y_tab[*py4++];
			*d4++ = g_clip[384+((y4 + bu)>>16)];		// Blue
			*d4++ = g_clip[384+((y4 - gu - gv)>>16)];	// Green
            *d4++ = g_clip[384+((y4 + rv)>>16)];		// Red
			*d4++ = 0;									// Alpha

			// Pixel (2,3)
			y4 = g_y_tab[*py4++];
			*d4++ = g_clip[384+((y4 + bu)>>16)];		// Blue
			*d4++ = g_clip[384+((y4 - gu - gv)>>16)];	// Green
            *d4++ = g_clip[384+((y4 + rv)>>16)];		// Red
			*d4++ = 0;									// Alpha

			// Pixel (3,3)
			y4 = g_y_tab[*py4++];
			*d4++ = g_clip[384+((y4 + bu)>>16)];		// Blue
			*d4++ = g_clip[384+((y4 - gu - gv)>>16)];	// Green
            *d4++ = g_clip[384+((y4 + rv)>>16)];		// Red
			*d4++ = 0;									// Alpha
		}
		py1 += 3*width;
		py2 += 3*width;
		py3 += 3*width;
		py4 += 3*width;
	}
}

void YUV9ToRGB32(	unsigned char *src,	// Y Plane, U Plane and V Plane
					unsigned char *dst,	// RGB32 Dib
					int width,
					int height)
{
	int y1, y2, y3, y4, u, v; 
	unsigned char *py1, *py2, *py3, *py4, *pv, *pu;
	int rv, gu, gv, bu;
	unsigned char *d1, *d2, *d3, *d4;

	int nChromaOffset = width * height;
	int nChromaSize = width * height / 16;

	py1 = src;
	py2 = py1+width;
	py3 = py2+width;
	py4 = py3+width;

	pu = src + nChromaOffset;
	pv = src + nChromaOffset + nChromaSize;

	int nDWAlignedLineSize = width << 2;

 	for (int j = (height - 1) ; j > 0 ; j -= 4)
	{
		d1 = dst + j * nDWAlignedLineSize;
		d2 = d1 - nDWAlignedLineSize;
		d3 = d2 - nDWAlignedLineSize;
		d4 = d3 - nDWAlignedLineSize;
		for (int i = 0 ; i < width ; i += 4)
		{
			u = *pu++;
			v = *pv++;

			rv = g_crv_tab[v];
			gu = g_cgu_tab[u];
			gv = g_cgv_tab[v];
			bu = g_cbu_tab[u];

			// Pixel (0,0)
            y1 = g_y_tab[*py1++];	
			*d1++ = g_clip[384+((y1 + bu)>>16)];		// Blue 
			*d1++ = g_clip[384+((y1 - gu - gv)>>16)];	// Green
            *d1++ = g_clip[384+((y1 + rv)>>16)];		// Red
			*d1++ = 0;									// Alpha

			// Pixel (1,0)
			y1 = g_y_tab[*py1++];
			*d1++ = g_clip[384+((y1 + bu)>>16)];		// Blue
			*d1++ = g_clip[384+((y1 - gu - gv)>>16)];	// Green
			*d1++ = g_clip[384+((y1 + rv)>>16)];		// Red
			*d1++ = 0;									// Alpha

			// Pixel (2,0)
			y1 = g_y_tab[*py1++];
			*d1++ = g_clip[384+((y1 + bu)>>16)];		// Blue
			*d1++ = g_clip[384+((y1 - gu - gv)>>16)];	// Green
			*d1++ = g_clip[384+((y1 + rv)>>16)];		// Red
			*d1++ = 0;									// Alpha

			// Pixel (3,0)
			y1 = g_y_tab[*py1++];
			*d1++ = g_clip[384+((y1 + bu)>>16)];		// Blue
			*d1++ = g_clip[384+((y1 - gu - gv)>>16)];	// Green
			*d1++ = g_clip[384+((y1 + rv)>>16)];		// Red
			*d1++ = 0;									// Alpha

			// Pixel (0,1)
			y2 = g_y_tab[*py2++];
			*d2++ = g_clip[384+((y2 + bu)>>16)];		// Blue
			*d2++ = g_clip[384+((y2 - gu - gv)>>16)];	// Green
            *d2++ = g_clip[384+((y2 + rv)>>16)];		// Red
			*d2++ = 0;									// Alpha

			// Pixel (1,1)
			y2 = g_y_tab[*py2++];
			*d2++ = g_clip[384+((y2 + bu)>>16)];		// Blue
			*d2++ = g_clip[384+((y2 - gu - gv)>>16)];	// Green
            *d2++ = g_clip[384+((y2 + rv)>>16)];		// Red
			*d2++ = 0;									// Alpha

			// Pixel (2,1)
			y2 = g_y_tab[*py2++];
			*d2++ = g_clip[384+((y2 + bu)>>16)];		// Blue
			*d2++ = g_clip[384+((y2 - gu - gv)>>16)];	// Green
            *d2++ = g_clip[384+((y2 + rv)>>16)];		// Red
			*d2++ = 0;									// Alpha

			// Pixel (3,1)
			y2 = g_y_tab[*py2++];
			*d2++ = g_clip[384+((y2 + bu)>>16)];		// Blue
			*d2++ = g_clip[384+((y2 - gu - gv)>>16)];	// Green
            *d2++ = g_clip[384+((y2 + rv)>>16)];		// Red
			*d2++ = 0;									// Alpha

			// Pixel (0,2)
			y3 = g_y_tab[*py3++];
			*d3++ = g_clip[384+((y3 + bu)>>16)];		// Blue
			*d3++ = g_clip[384+((y3 - gu - gv)>>16)];	// Green
            *d3++ = g_clip[384+((y3 + rv)>>16)];		// Red
			*d3++ = 0;									// Alpha

			// Pixel (1,2)
			y3 = g_y_tab[*py3++];
			*d3++ = g_clip[384+((y3 + bu)>>16)];		// Blue
			*d3++ = g_clip[384+((y3 - gu - gv)>>16)];	// Green
            *d3++ = g_clip[384+((y3 + rv)>>16)];		// Red
			*d3++ = 0;									// Alpha

			// Pixel (2,2)
			y3 = g_y_tab[*py3++];
			*d3++ = g_clip[384+((y3 + bu)>>16)];		// Blue
			*d3++ = g_clip[384+((y3 - gu - gv)>>16)];	// Green
            *d3++ = g_clip[384+((y3 + rv)>>16)];		// Red
			*d3++ = 0;									// Alpha

			// Pixel (3,2)
			y3 = g_y_tab[*py3++];
			*d3++ = g_clip[384+((y3 + bu)>>16)];		// Blue
			*d3++ = g_clip[384+((y3 - gu - gv)>>16)];	// Green
            *d3++ = g_clip[384+((y3 + rv)>>16)];		// Red
			*d3++ = 0;									// Alpha

			// Pixel (0,3)
			y4 = g_y_tab[*py4++];
			*d4++ = g_clip[384+((y4 + bu)>>16)];		// Blue
			*d4++ = g_clip[384+((y4 - gu - gv)>>16)];	// Green
            *d4++ = g_clip[384+((y4 + rv)>>16)];		// Red
			*d4++ = 0;									// Alpha

			// Pixel (1,3)
			y4 = g_y_tab[*py4++];
			*d4++ = g_clip[384+((y4 + bu)>>16)];		// Blue
			*d4++ = g_clip[384+((y4 - gu - gv)>>16)];	// Green
            *d4++ = g_clip[384+((y4 + rv)>>16)];		// Red
			*d4++ = 0;									// Alpha

			// Pixel (2,3)
			y4 = g_y_tab[*py4++];
			*d4++ = g_clip[384+((y4 + bu)>>16)];		// Blue
			*d4++ = g_clip[384+((y4 - gu - gv)>>16)];	// Green
            *d4++ = g_clip[384+((y4 + rv)>>16)];		// Red
			*d4++ = 0;									// Alpha

			// Pixel (3,3)
			y4 = g_y_tab[*py4++];
			*d4++ = g_clip[384+((y4 + bu)>>16)];		// Blue
			*d4++ = g_clip[384+((y4 - gu - gv)>>16)];	// Green
            *d4++ = g_clip[384+((y4 + rv)>>16)];		// Red
			*d4++ = 0;									// Alpha
		}
		py1 += 3*width;
		py2 += 3*width;
		py3 += 3*width;
		py4 += 3*width;
	}
}

// Equivalent FCCs Are: YUNV, VYUY, V422 and YUYV
void YUY2ToRGB32(	LPBYTE src,		// Y0 U0 Y1 V0, Y2 U2 Y3 V2, ...
					LPDWORD dst,	// RGB32 Dib
					int width,
					int height)
{
	int n2Width = width << 1;
	dst = dst + (width * (height - 1));
	for (int j = 0 ; j < height ; j++)
	{
		for (int i = 0 ; i < width ; i += 2)
		{
			int y0 = *src++;
			int u  = *src++;
			int y1 = *src++;
			int v  = *src++;
			int rv = g_crv_tab[v];
			int bu = g_cbu_tab[u];
			int gugv = -g_cgu_tab[u] - g_cgv_tab[v];

			// First Pixel
            y0 = g_y_tab[y0];	
			*dst++ = (DWORD)g_clip[384+((y0 + bu)>>16)]			|	// Blue 
					((DWORD)g_clip[384+((y0 + gugv)>>16)] << 8)	|	// Green
					((DWORD)g_clip[384+((y0 + rv)>>16)] << 16);		// Red

			// Second Pixel
			y1 = g_y_tab[y1];
			*dst++ = (DWORD)g_clip[384+((y1 + bu)>>16)]			|	// Blue
					((DWORD)g_clip[384+((y1 + gugv)>>16)] << 8)	|	// Green
					((DWORD)g_clip[384+((y1 + rv)>>16)] << 16);		// Red
			
		}
		dst -= n2Width;
	}
}

// Equivalent FCCs Are: Y422 and UYNV 
void UYVYToRGB32(	LPBYTE src,		// U0 Y0 V0 Y1, U2 Y2 V2 Y3, ...
					LPDWORD dst,	// RGB32 Dib
					int width,
					int height)
{
	int n2Width = width << 1;
	dst = dst + (width * (height - 1));
	for (int j = 0 ; j < height ; j++)
	{
		for (int i = 0 ; i < width ; i += 2)
		{
			int u  = *src++;
			int y0 = *src++;
			int v  = *src++;
			int y1 = *src++;
			int rv = g_crv_tab[v];
			int bu = g_cbu_tab[u];
			int gugv = -g_cgu_tab[u] - g_cgv_tab[v];

			// First Pixel
            y0 = g_y_tab[y0];	
			*dst++ = (DWORD)g_clip[384+((y0 + bu)>>16)]			|	// Blue 
					((DWORD)g_clip[384+((y0 + gugv)>>16)] << 8)	|	// Green
					((DWORD)g_clip[384+((y0 + rv)>>16)] << 16);		// Red

			// Second Pixel
			y1 = g_y_tab[y1];
			*dst++ = (DWORD)g_clip[384+((y1 + bu)>>16)]			|	// Blue
					((DWORD)g_clip[384+((y1 + gugv)>>16)] << 8)	|	// Green
					((DWORD)g_clip[384+((y1 + rv)>>16)] << 16);		// Red
		}
		dst -= n2Width;
	}
}

void YVYUToRGB32(	LPBYTE src,		// Y0 V0 Y1 U0, Y2 V2 Y3 U2, ...
					LPDWORD dst,	// RGB32 Dib
					int width,
					int height)
{
	int n2Width = width << 1;
	dst = dst + (width * (height - 1));
	for (int j = 0 ; j < height ; j++)
	{
		for (int i = 0 ; i < width ; i += 2)
		{
			int y0 = *src++;
			int v  = *src++;
			int y1 = *src++;
			int u  = *src++;
			int rv = g_crv_tab[v];
			int bu = g_cbu_tab[u];
			int gugv = -g_cgu_tab[u] - g_cgv_tab[v];

			// First Pixel
            y0 = g_y_tab[y0];	
			*dst++ = (DWORD)g_clip[384+((y0 + bu)>>16)]			|	// Blue 
					((DWORD)g_clip[384+((y0 + gugv)>>16)] << 8)	|	// Green
					((DWORD)g_clip[384+((y0 + rv)>>16)] << 16);		// Red

			// Second Pixel
			y1 = g_y_tab[y1];
			*dst++ = (DWORD)g_clip[384+((y1 + bu)>>16)]			|	// Blue
					((DWORD)g_clip[384+((y1 + gugv)>>16)] << 8)	|	// Green
					((DWORD)g_clip[384+((y1 + rv)>>16)] << 16);		// Red
		}
		dst -= n2Width;
	}
}

void Y41PToRGB32(	unsigned char *src,	// U0 Y0 V0 Y1 U4 Y2 V4 Y3 Y4 Y5 Y6 Y7
					unsigned char *dst,	// RGB32 Dib
					int width,
					int height)
{
	int nDWAlignedLineSize = width << 2;
	int n2DWAlignedLineSize = width << 3;
	dst = dst + (nDWAlignedLineSize * (height - 1));
	for (int j = 0 ; j < height ; j++)
	{
		for (int i = 0 ; i < width ; i += 8)
		{
			int u0 = *src++;
			int y0 = *src++;
			int v0 = *src++;
			int y1 = *src++;
			int u4 = *src++;
			int y2 = *src++;
			int v4 = *src++;
			int y3 = *src++;
			int y4 = *src++;
			int y5 = *src++;
			int y6 = *src++;
			int y7 = *src++;

			int rv0 = g_crv_tab[v0];
			int gu0 = g_cgu_tab[u0];
			int gv0 = g_cgv_tab[v0];
			int bu0 = g_cbu_tab[u0];

			int rv4 = g_crv_tab[v4];
			int gu4 = g_cgu_tab[u4];
			int gv4 = g_cgv_tab[v4];
			int bu4 = g_cbu_tab[u4];

			// First Pixel
            y0 = g_y_tab[y0];	
			*dst++ = g_clip[384+((y0 + bu0)>>16)];			// Blue 
			*dst++ = g_clip[384+((y0 - gu0 - gv0)>>16)];	// Green
            *dst++ = g_clip[384+((y0 + rv0)>>16)];			// Red
			*dst++ = 0;										// Alpha

			// Second Pixel
            y1 = g_y_tab[y1];	
			*dst++ = g_clip[384+((y1 + bu0)>>16)];			// Blue 
			*dst++ = g_clip[384+((y1 - gu0 - gv0)>>16)];	// Green
            *dst++ = g_clip[384+((y1 + rv0)>>16)];			// Red
			*dst++ = 0;										// Alpha

			// Third Pixel
            y2 = g_y_tab[y2];	
			*dst++ = g_clip[384+((y2 + bu0)>>16)];			// Blue 
			*dst++ = g_clip[384+((y2 - gu0 - gv0)>>16)];	// Green
            *dst++ = g_clip[384+((y2 + rv0)>>16)];			// Red
			*dst++ = 0;										// Alpha

			// Fourth Pixel
			y3 = g_y_tab[y3];
			*dst++ = g_clip[384+((y3 + bu0)>>16)];			// Blue
			*dst++ = g_clip[384+((y3 - gu0 - gv0)>>16)];	// Green
			*dst++ = g_clip[384+((y3 + rv0)>>16)];			// Red
			*dst++ = 0;										// Alpha

			// Fifth Pixel
            y4 = g_y_tab[y4];	
			*dst++ = g_clip[384+((y4 + bu4)>>16)];			// Blue 
			*dst++ = g_clip[384+((y4 - gu4 - gv4)>>16)];	// Green
            *dst++ = g_clip[384+((y4 + rv4)>>16)];			// Red
			*dst++ = 0;										// Alpha

			// Sixth Pixel
            y5 = g_y_tab[y5];	
			*dst++ = g_clip[384+((y5 + bu4)>>16)];			// Blue 
			*dst++ = g_clip[384+((y5 - gu4 - gv4)>>16)];	// Green
            *dst++ = g_clip[384+((y5 + rv4)>>16)];			// Red
			*dst++ = 0;										// Alpha

			// Seventh Pixel
            y6 = g_y_tab[y6];	
			*dst++ = g_clip[384+((y6 + bu4)>>16)];			// Blue 
			*dst++ = g_clip[384+((y6 - gu4 - gv4)>>16)];	// Green
            *dst++ = g_clip[384+((y6 + rv4)>>16)];			// Red
			*dst++ = 0;										// Alpha

			// Eighth Pixel
			y7 = g_y_tab[y7];
			*dst++ = g_clip[384+((y7 + bu4)>>16)];			// Blue
			*dst++ = g_clip[384+((y7 - gu4 - gv4)>>16)];	// Green
			*dst++ = g_clip[384+((y7 + rv4)>>16)];			// Red
			*dst++ = 0;										// Alpha
		}
		dst -= n2DWAlignedLineSize;
	}
}

/*
ATI VCR1
--------
The ATI VCR1 codec, identified by the fourcc VCR1, uses differential 
coding to pack Y samples. C samples are left alone. VCR1 is based on a YUV 
4:1:0 colorspace. This means that for each block of 4x4 pixels each pixel 
has a Y sample and the entire block shares both C samples.

The format of a VCR1-encoded video chunk is as follows:

  bytes 0-31    16 16-bit, signed, little-endian deltas used in this frame
  bytes 32..    encoded YUV data

The deltas are apparently 16 bits in width which is somewhat irrelevant
since the Y samples to which they are applied are only 8-bit numbers.

The YUV data is coded after the initial deltas. The data is coded as:

  luminance/chrominance line
  luminance line
  luminance line
  luminance line
  [...]

Every fourth line, starting with line 0, contains both luminance (Y) and 
chrominance (C) data. The other lines only contain Y data. 

Each Y/C line begins with 4 offsets to be used when decoding the Y data 
for the next 4 lines:

  byte 0    offset for this line's Y data
  byte 1    offset for second line's Y data
  byte 2    offset for third line's Y data
  byte 3    offset for fourth line's Y data
  bytes 4.. Y/C data

For the remainder of the data on a Y/C line, these 6 pieces of data:

  Y0 Y1 Y2 Y3 U V

are encoded within groups of 4 bytes of the bytestream. Y0..Y3 are the
next 4 Y samples in the line while U and V are the C samples for the 4 Y
samples as well as the 4 Y samples on each of the next 3 lines (since
this is a YUV 4:1:0 colorspace). The 4 bytes in the group have the
following meaning:

   byte0     byte1    byte2    byte3
  Y3i Y2i      V     Y1i Y0i     U

Bytes 1 and 3 correspond to the V and U samples, respectively. Bytes 0
and 2 break down into 4 4-bit nibbles which do not actually represent
the Y samples. Instead, they index into the delta table from the start
of the frame. The indexed signed delta is added to the previous Y to
get the current Y. The first Y is initialized with the offset itself.
(Note that the first Y index of each line is not used).

For the other lines that only contain Y data, each group of 4 bytes 
decodes to 8 Y samples in a similar manner as on the Y/C lines:

   byte0    byte1    byte2    byte3
  Y5i Y4i  Y7i Y6i  Y1i Y0i  Y3i Y2i
*/
void VCR1ToRGB32(	unsigned char *src,
					unsigned char *dst,	// RGB32 Dib
					int width,
					int height)
{
	short TableY[16];

	// Init Y Table
	memcpy(TableY, src, 32);
	src += 32;

	int u, v;
	int y00, y10, y20, y30;
	int y01, y11, y21, y31;
	int y02, y12, y22, y32;
	int y03, y13, y23, y33;
	int offset0, offset1, offset2, offset3;
	unsigned char *s, *s0, *s1, *s2, *s3;
	int rv, gu, gv, bu;
	unsigned char *d0, *d1, *d2, *d3;

	s0 = src;
	s1 = s0+4+width;
	s2 = s1+width/2;
	s3 = s2+width/2;

	int nDWAlignedLineSize = width << 2;

 	for (int j = (height - 1) ; j > 0 ; j -= 4)
	{
		d0 = dst + j * nDWAlignedLineSize;
		d1 = d0 - nDWAlignedLineSize;
		d2 = d1 - nDWAlignedLineSize;
		d3 = d2 - nDWAlignedLineSize;
		for (int i = 0 ; i < width ; i += 4)
		{
			// Init offsets for this 4 lines
			if (i == 0)
			{
				// Read offsets
				offset0 = *s0++;
				offset1 = *s0++;
				offset2 = *s0++;
				offset3 = *s0++;

				// First y value is the offset itself!
				// -> Subtract the TableY value which 
				// will be added again below:
				offset0 -= TableY[s0[2] & 0x0F];
				offset1 -= TableY[s1[2] & 0x0F];
				offset2 -= TableY[s2[2] & 0x0F];
				offset3 -= TableY[s3[2] & 0x0F];
			}

			// Line0
			y00 = (offset0 += TableY[s0[2] & 0x0F]);
			y10 = (offset0 += TableY[s0[2] >> 4]);
			y20 = (offset0 += TableY[s0[0] & 0x0F]);
			y30 = (offset0 += TableY[s0[0] >> 4]);
			u = s0[3];
			v = s0[1];
			s0 += 4;

			// Line1
			s = s1 + ((i / 8) * 4);
			if ((i % 8) == 0)
			{
				y01 = (offset1 += TableY[s[2] & 0x0F]);
				y11 = (offset1 += TableY[s[2] >> 4]);
				y21 = (offset1 += TableY[s[3] & 0x0F]);
				y31 = (offset1 += TableY[s[3] >> 4]);
			}
			else
			{
				y01 = (offset1 += TableY[s[0] & 0x0F]);
				y11 = (offset1 += TableY[s[0] >> 4]);
				y21 = (offset1 += TableY[s[1] & 0x0F]);
				y31 = (offset1 += TableY[s[1] >> 4]);
			}

			// Line2
			s = s2 + ((i / 8) * 4);
			if ((i % 8) == 0)
			{
				y02 = (offset2 += TableY[s[2] & 0x0F]);
				y12 = (offset2 += TableY[s[2] >> 4]);
				y22 = (offset2 += TableY[s[3] & 0x0F]);
				y32 = (offset2 += TableY[s[3] >> 4]);
			}
			else
			{
				y02 = (offset2 += TableY[s[0] & 0x0F]);
				y12 = (offset2 += TableY[s[0] >> 4]);
				y22 = (offset2 += TableY[s[1] & 0x0F]);
				y32 = (offset2 += TableY[s[1] >> 4]);
			}

			// Line3
			s = s3 + ((i / 8) * 4);
			if ((i % 8) == 0)
			{
				y03 = (offset3 += TableY[s[2] & 0x0F]);
				y13 = (offset3 += TableY[s[2] >> 4]);
				y23 = (offset3 += TableY[s[3] & 0x0F]);
				y33 = (offset3 += TableY[s[3] >> 4]);
			}
			else
			{
				y03 = (offset3 += TableY[s[0] & 0x0F]);
				y13 = (offset3 += TableY[s[0] >> 4]);
				y23 = (offset3 += TableY[s[1] & 0x0F]);
				y33 = (offset3 += TableY[s[1] >> 4]);
			}

			// Chrominance Coeff.
			rv = g_crv_tab[v];
			gu = g_cgu_tab[u];
			gv = g_cgv_tab[v];
			bu = g_cbu_tab[u];

			// Pixel (0,0)
            y00 = g_y_tab[y00];	
			*d0++ = g_clip[384+((y00 + bu)>>16)];		// Blue 
			*d0++ = g_clip[384+((y00 - gu - gv)>>16)];	// Green
            *d0++ = g_clip[384+((y00 + rv)>>16)];		// Red
			*d0++ = 0;									// Alpha

			// Pixel (1,0)
			y10 = g_y_tab[y10];
			*d0++ = g_clip[384+((y10 + bu)>>16)];		// Blue
			*d0++ = g_clip[384+((y10 - gu - gv)>>16)];	// Green
			*d0++ = g_clip[384+((y10 + rv)>>16)];		// Red
			*d0++ = 0;									// Alpha

			// Pixel (2,0)
			y20 = g_y_tab[y20];
			*d0++ = g_clip[384+((y20 + bu)>>16)];		// Blue
			*d0++ = g_clip[384+((y20 - gu - gv)>>16)];	// Green
			*d0++ = g_clip[384+((y20 + rv)>>16)];		// Red
			*d0++ = 0;									// Alpha

			// Pixel (3,0)
			y30 = g_y_tab[y30];
			*d0++ = g_clip[384+((y30 + bu)>>16)];		// Blue
			*d0++ = g_clip[384+((y30 - gu - gv)>>16)];	// Green
			*d0++ = g_clip[384+((y30 + rv)>>16)];		// Red
			*d0++ = 0;									// Alpha

			// Pixel (0,1)
            y01 = g_y_tab[y01];	
			*d1++ = g_clip[384+((y01 + bu)>>16)];		// Blue 
			*d1++ = g_clip[384+((y01 - gu - gv)>>16)];	// Green
            *d1++ = g_clip[384+((y01 + rv)>>16)];		// Red
			*d1++ = 0;									// Alpha

			// Pixel (1,1)
			y11 = g_y_tab[y11];
			*d1++ = g_clip[384+((y11 + bu)>>16)];		// Blue
			*d1++ = g_clip[384+((y11 - gu - gv)>>16)];	// Green
			*d1++ = g_clip[384+((y11 + rv)>>16)];		// Red
			*d1++ = 0;									// Alpha

			// Pixel (2,1)
			y21 = g_y_tab[y21];
			*d1++ = g_clip[384+((y21 + bu)>>16)];		// Blue
			*d1++ = g_clip[384+((y21 - gu - gv)>>16)];	// Green
			*d1++ = g_clip[384+((y21 + rv)>>16)];		// Red
			*d1++ = 0;									// Alpha

			// Pixel (3,1)
			y31 = g_y_tab[y31];
			*d1++ = g_clip[384+((y31 + bu)>>16)];		// Blue
			*d1++ = g_clip[384+((y31 - gu - gv)>>16)];	// Green
			*d1++ = g_clip[384+((y31 + rv)>>16)];		// Red
			*d1++ = 0;									// Alpha

			// Pixel (0,2)
            y02 = g_y_tab[y02];	
			*d2++ = g_clip[384+((y02 + bu)>>16)];		// Blue 
			*d2++ = g_clip[384+((y02 - gu - gv)>>16)];	// Green
            *d2++ = g_clip[384+((y02 + rv)>>16)];		// Red
			*d2++ = 0;									// Alpha

			// Pixel (1,2)
			y12 = g_y_tab[y12];
			*d2++ = g_clip[384+((y12 + bu)>>16)];		// Blue
			*d2++ = g_clip[384+((y12 - gu - gv)>>16)];	// Green
			*d2++ = g_clip[384+((y12 + rv)>>16)];		// Red
			*d2++ = 0;									// Alpha

			// Pixel (2,2)
			y22 = g_y_tab[y22];
			*d2++ = g_clip[384+((y22 + bu)>>16)];		// Blue
			*d2++ = g_clip[384+((y22 - gu - gv)>>16)];	// Green
			*d2++ = g_clip[384+((y22 + rv)>>16)];		// Red
			*d2++ = 0;									// Alpha

			// Pixel (3,2)
			y32 = g_y_tab[y32];
			*d2++ = g_clip[384+((y32 + bu)>>16)];		// Blue
			*d2++ = g_clip[384+((y32 - gu - gv)>>16)];	// Green
			*d2++ = g_clip[384+((y32 + rv)>>16)];		// Red
			*d2++ = 0;									// Alpha

			// Pixel (0,3)
            y03 = g_y_tab[y03];	
			*d3++ = g_clip[384+((y03 + bu)>>16)];		// Blue 
			*d3++ = g_clip[384+((y03 - gu - gv)>>16)];	// Green
            *d3++ = g_clip[384+((y03 + rv)>>16)];		// Red
			*d3++ = 0;									// Alpha

			// Pixel (1,3)
			y13 = g_y_tab[y13];
			*d3++ = g_clip[384+((y13 + bu)>>16)];		// Blue
			*d3++ = g_clip[384+((y13 - gu - gv)>>16)];	// Green
			*d3++ = g_clip[384+((y13 + rv)>>16)];		// Red
			*d3++ = 0;									// Alpha

			// Pixel (2,3)
			y23 = g_y_tab[y23];
			*d3++ = g_clip[384+((y23 + bu)>>16)];		// Blue
			*d3++ = g_clip[384+((y23 - gu - gv)>>16)];	// Green
			*d3++ = g_clip[384+((y23 + rv)>>16)];		// Red
			*d3++ = 0;									// Alpha

			// Pixel (3,3)
			y33 = g_y_tab[y33];
			*d3++ = g_clip[384+((y33 + bu)>>16)];		// Blue
			*d3++ = g_clip[384+((y33 - gu - gv)>>16)];	// Green
			*d3++ = g_clip[384+((y33 + rv)>>16)];		// Red
			*d3++ = 0;									// Alpha
		}
		s0 += (3*width/2);
		s1 = s0+4+width;
		s2 = s1+width/2;
		s3 = s2+width/2;
	}
}

/*
Cirrus Logic AccuPak (CLJR)
---------------------------
The Cirrus Logic AccuPak codec, identified by the fourcc CLJR, packs 4 Y 
samples and 2 C samples into 32 bits by representing each Y sample with 5 
bits and each C sample with 6 bits. It is essentially a scaled-down method 
of coding YUV 4:1:1, where each group of 4 pixels on a line is represented 
by a luminance sample each but share C samples.

Each set of 32 bits represents 4 pixels on a line,
(the 32 bits set is stored in Big Endian format):

  p0 p1 p2 p3


For each set of 32 bits:

bit31 - 27: p3.Y (5 bits)
bit26 - 22: p2.Y (5 bits)
bit21 - 17: p1.Y (5 bits)
bit16 - 12: p0.Y (5 bits)
bit11 - 6 : Cb/U (6 bits)
bit5  - 0 : Cr/V (6 bits)

Thus, the first 5 bits represent the Y sample for the last pixel in the 
group of 4 pixels.
*/
void CLJRToRGB32(	unsigned char *src,
					unsigned char *dst,	// RGB32 Dib
					int width,
					int height)
{
	DWORD dw;
	int nDWAlignedLineSize = width << 2;
	int n2DWAlignedLineSize = width << 3;
	dst = dst + (nDWAlignedLineSize * (height - 1));
	for (int j = 0 ; j < height ; j++)
	{
		for (int i = 0 ; i < width ; i += 4)
		{
			// Read Big Endian DWORD
			dw =	(((DWORD)(src[0]) << 24)) | 
					(((DWORD)(src[1]) << 16)) |
					(((DWORD)(src[2]) <<  8)) |
					((DWORD)(src[3]));
			src += 4;

			// Parse the 4 Packed Pixels 
			int v = (dw & 0x3F) << 2;
			dw >>= 6;
			int u = (dw & 0x3F) << 2;
			dw >>= 6;
			int y0 = (dw & 0x1F) << 3;
			dw >>= 5;
			int y1 = (dw & 0x1F) << 3;
			dw >>= 5;
			int y2 = (dw & 0x1F) << 3;
			dw >>= 5;
			int y3 = (dw & 0x1F) << 3;
			
			// Chrominance Coeff.
			int rv = g_crv_tab[v];
			int gu = g_cgu_tab[u];
			int gv = g_cgv_tab[v];
			int bu = g_cbu_tab[u];

			// First Pixel
            y0 = g_y_tab[y0];	
			*dst++ = g_clip[384+((y0 + bu)>>16)];		// Blue 
			*dst++ = g_clip[384+((y0 - gu - gv)>>16)];	// Green
            *dst++ = g_clip[384+((y0 + rv)>>16)];		// Red
			*dst++ = 0;									// Alpha

			// Second Pixel
            y1 = g_y_tab[y1];	
			*dst++ = g_clip[384+((y1 + bu)>>16)];		// Blue 
			*dst++ = g_clip[384+((y1 - gu - gv)>>16)];	// Green
            *dst++ = g_clip[384+((y1 + rv)>>16)];		// Red
			*dst++ = 0;									// Alpha

			// Third Pixel
            y2 = g_y_tab[y2];	
			*dst++ = g_clip[384+((y2 + bu)>>16)];		// Blue 
			*dst++ = g_clip[384+((y2 - gu - gv)>>16)];	// Green
            *dst++ = g_clip[384+((y2 + rv)>>16)];		// Red
			*dst++ = 0;									// Alpha

			// Fourth Pixel
			y3 = g_y_tab[y3];
			*dst++ = g_clip[384+((y3 + bu)>>16)];		// Blue
			*dst++ = g_clip[384+((y3 - gu - gv)>>16)];	// Green
			*dst++ = g_clip[384+((y3 + rv)>>16)];		// Red
			*dst++ = 0;									// Alpha
		}
		dst -= n2DWAlignedLineSize;
	}
}

/*
Creative YUV (CYUV)
-------------------
Creative YUV, identified by the fourcc CYUV, uses differential coding to 
effectively compress each Y, U, and V sample to 4 bits with some overhead 
at the start of each line. The codec operates on a YUV 4:1:1 colorspace 
which means that each group of 4 pixels on a line has 1 Y sample per 
pixel, but only 1 of each C sample for the entire group.

A chunk of CYUV-encoded data is laid out as:

  bytes 0-15    signed Y predictor byte values
  bytes 16-31   signed U predictor byte values
  bytes 32-47   signed V predictor byte values
  bytes 48..    lines of CYUV-encoded data

The format of each line is as follows:

  byte 0
    bits 7-4  initial U sample and predictor for line
    bits 3-0  initial Y sample and predictor for line
  byte 1
    bits 7-4  initial V sample and predictor for line
    bits 3-0  next Y predictor index
  byte 2
    bits 7-4  next Y predictor index
    bits 3-0  next Y predictor index
  bytes 3..   remaining predictor indices for line

The first 3 bytes contain the setup information for the line. Each initial 
sample (Y, U, and V) actually represents the top 4 bits of the initial 
8-bit sample. The initial sample also serves as the initial predictor. For 
each of the 3 Y predictor indices, use the 4-bit value to index into the 
table of 16 Y predictors, encoded at the start of the frame. Apply each 
predictor to the previous Y value.

At this point, the first group of 4 pixels will be decoded. For each group 
of 4 pixels remaining on the line

  byte 0
    bits 7-4  next U predictor index
    bits 3-0  next Y predictor index
  byte 1
    bits 7-4  next V predictor index
    bits 3-0  next Y predictor index
  byte 2
    bits 7-4  next Y predictor index
    bits 3-0  next Y predictor index

For each predictor index, use the 4 bits to index into the appropriate 
predictor table and apply the predictor to the previous sample of the same 
type (Y, U, or V) and output the sample.
*/
void cyuvToRGB32(	unsigned char *src,
					unsigned char *dst,	// RGB32 Dib
					int width,
					int height)
{
	int n;
	int u, v;
	int y0, y1, y2, y3, y_last;
	char TableY[16];
	char TableU[16];
	char TableV[16];

	// Init Predictor Tables
	memcpy(TableY, src, 16);
	src += 16;
	memcpy(TableU, src, 16);
	src += 16;
	memcpy(TableV, src, 16);
	src += 16;

	int nDWAlignedLineSize = width << 2;
	int n2DWAlignedLineSize = width << 3;
	dst = dst + (nDWAlignedLineSize * (height - 1));
	for (int j = 0 ; j < height ; j++)
	{
		for (int i = 0 ; i < width / 4 ; i++)
		{
			// Init values for scanline
			if (i == 0)
			{
				n = *src++;
				y0 = (n & 0x0F) << 4;
				u = n & 0xF0;

				n = *src++;
				y1 = y0 + TableY[n & 0x0F];
				v = n & 0xF0;

				n = *src++;
				y2 = y1 + TableY[n & 0x0F];
				y_last = y3 = y2 + TableY[n >> 4];
			}
			else
			{
				n = *src++;
				y0 = y_last + TableY[n & 0x0F];
				u = u + TableU[n >> 4];

				n = *src++;
				y1 = y0 + TableY[n & 0x0F];
				v = v + TableV[n >> 4];

				n = *src++;
				y2 = y1 + TableY[n & 0x0F];
				y_last = y3 = y2 + TableY[n >> 4];
			}

			// Chrominance Coeff.
			int rv = g_crv_tab[v];
			int gu = g_cgu_tab[u];
			int gv = g_cgv_tab[v];
			int bu = g_cbu_tab[u];

			// First Pixel
            y0 = g_y_tab[y0];	
			*dst++ = g_clip[384+((y0 + bu)>>16)];		// Blue 
			*dst++ = g_clip[384+((y0 - gu - gv)>>16)];	// Green
            *dst++ = g_clip[384+((y0 + rv)>>16)];		// Red
			*dst++ = 0;									// Alpha

			// Second Pixel
            y1 = g_y_tab[y1];	
			*dst++ = g_clip[384+((y1 + bu)>>16)];		// Blue 
			*dst++ = g_clip[384+((y1 - gu - gv)>>16)];	// Green
            *dst++ = g_clip[384+((y1 + rv)>>16)];		// Red
			*dst++ = 0;									// Alpha

			// Third Pixel
            y2 = g_y_tab[y2];	
			*dst++ = g_clip[384+((y2 + bu)>>16)];		// Blue 
			*dst++ = g_clip[384+((y2 - gu - gv)>>16)];	// Green
            *dst++ = g_clip[384+((y2 + rv)>>16)];		// Red
			*dst++ = 0;									// Alpha

			// Fourth Pixel
			y3 = g_y_tab[y3];
			*dst++ = g_clip[384+((y3 + bu)>>16)];		// Blue
			*dst++ = g_clip[384+((y3 - gu - gv)>>16)];	// Green
			*dst++ = g_clip[384+((y3 + rv)>>16)];		// Red
			*dst++ = 0;									// Alpha
		}
		dst -= n2DWAlignedLineSize;
	}
}

/*
Miro/Pinnacle Video XL (VIXL/PIXL)
----------------------------------
The Miro Video XL codec, identified by the fourcc VIXL, uses
differential coding on a reduced-precision YUV 4:1:1 colorspace image.
Each Y, U, or V component is only 7 bits (where 8 is more typical). Each
group of 32 bits in the bitstream represents 6 5-bit delta table indices
(with 2 unused bits). There is one index for each of the next 4 Y
samples on the line and one index for each of the color samples.

The Pinnacle Video XL codec, indentified by the fourcc PIXL, is
apparently the same algorithm as the Miro codec except that the frames
are 8 bytes longer. However, the same decoding process applies.

For each block of 4 pixels on a line, fetch the next 32 bits as a little
endian number and then swap the 16 bit words to achieve the correct bit
orientation for decoding. To illustrate more clearly, this is the
arrangement of the next 4 8-bit bytes (A, B, C, and D) on disk:

  aaaaaaaa bbbbbbbb cccccccc dddddddd

Load the 4 bytes into a program variable so that the bytes are in this
order:

 31                                 0
  dddddddd cccccccc bbbbbbbb aaaaaaaa

Then, swap the upper and lower 16-bit words to achieve this order:

 31                                 0
  bbbbbbbb aaaaaaaa dddddddd cccccccc

Further, the 32-bit blocks are stored in reverse order. So, for example,
if an image is 16 pixels wide, it would have 4 pixel groups per line.
Each pixel group would be represented by a 32-bit doubleword, swapped
and mangled as described previously. The doublewords would be stored in
the bytestream as:

  D3 D2 D1 D0
  
D0 represents the first 4 pixels on the line and D3 represents the final
4 pixels on the line. Thus, a decoder must jump forward in the
bytestream and work backwards through the bytestream while decoding in
the forward direction on a particular line, then jump forward again in 
the bytestream when decoding the next line.

The 32 bits of the doubleword represent the following values:

  bit 31:     unused
  bits 30-26: V delta index
  bits 25-21: U delta index
  bits 20-16: Y3 delta index
  bit 15:     unused
  bits 14-10: Y2 delta index
  bits 9-5:   Y1 delta index
  bits 4-0:   Y0 delta index

Each delta index value is used to index into this table and the
referenced value is added to the previous element on the same plane,
either Y, U, or V:

const int xl_delta_table[32] = {
   0,   1,   2,   3,   4,   5,   6,   7,
   8,   9,  12,  15,  20,  25,  34,  46,
  64,  82,  94, 103, 108, 113, 116, 119,
 120, 121, 122, 123, 124, 125, 126, 127
};

Remember that the YUV components only have 7 bits of precision. Thus,
the second half of the table values all count as negative values.

At the beginning of a line, the Y0, U, and V delta indices actually
represent the top 5 bits of the absolute 7-bit component value.

The final, concise decoding algorithm operates as follows:

  foreach line in image
    foreach 32-bit doubleword, working from right -> left in bytestream
      load doubleword as little-endian number, swap 16-bit words
      if this is the first pixel group in line
        next Y value = (Y0 delta index) << 2
        next U value = (U delta index) << 2
        next V value = (V delta index) << 2
      else
        next Y value = last Y value + xl_delta_table[Y0 delta index]
        next U value = last U value + xl_delta_table[U delta index]
        next V value = last V value + xl_delta_table[V delta index]
      next Y value = last Y value + xl_delta_table[Y1 delta index]
      next Y value = last Y value + xl_delta_table[Y2 delta index]
      next Y value = last Y value + xl_delta_table[Y3 delta index]

Since the components only have 7 bits of meaningful precision, it will
likely be necessary to shift each of the components left once more to
achieve 8 bits of output precision.
*/
void VIXLToRGB32(	unsigned char *src,
					unsigned char *dst,	// RGB32 Dib
					int width,
					int height)
{
	DWORD dw;
	int u, v;
	int y0, y1, y2, y3, y_last;

	/*
	const int t[32] =
	{
	   0,   1,   2,   3,   4,   5,   6,   7,
	   8,   9,  12,  15,  20,  25,  34,  46,
	  64,  82,  94, 103, 108, 113, 116, 119,
	 120, 121, 122, 123, 124, 125, 126, 127
	};
	*/
	// Equivalent table, just more understandable
	const int t[32] =
	{
	   0,   1,   2,   3,   4,   5,   6,   7,
	   8,   9,  12,  15,  20,  25,  34,  46,
	  -64,  -46,  -34, -25, -20, -15, -12, -9,
	 -8, -7, -6, -5, -4, -3, -2, -1
	};

	int nDWAlignedLineSize = width << 2;
	int n2DWAlignedLineSize = width << 3;
	dst = dst + (nDWAlignedLineSize * (height - 1));
	for (int j = 0 ; j < height ; j++)
	{
		for (int i = 0 ; i < width / 4 ; i++)
		{
			// Get 4 Packet Pixels
			dw = ((DWORD*)src)[width/4 - i - 1];
			dw = (dw << 16) | (dw >> 16);

			// Init values for scanline
			if (i == 0)
			{
				y0 = (dw & 0x1F) << 2;
				dw >>= 5;
				y1 = y0 + t[dw & 0x1F];
				dw >>= 5;
				y2 = y1 + t[dw & 0x1F];
				dw >>= 6;
				y_last = y3 = y2 + t[dw & 0x1F];
				dw >>= 5;
				u = (dw & 0x1F) << 2;
				dw >>= 5;
				v = (dw & 0x1F) << 2;
			}
			else
			{
				y0 = y_last + t[dw & 0x1F];
				dw >>= 5;
				y1 = y0 + t[dw & 0x1F];
				dw >>= 5;
				y2 = y1 + t[dw & 0x1F];
				dw >>= 6;
				y_last = y3 = y2 + t[dw & 0x1F];
				dw >>= 5;
				u += t[dw & 0x1F];
				dw >>= 5;
				v += t[dw & 0x1F];
			}

			// Chrominance Coeff.
			int rv = g_crv_tab[(v<<1) & 0xFF];
			int gu = g_cgu_tab[(u<<1) & 0xFF];
			int gv = g_cgv_tab[(v<<1) & 0xFF];
			int bu = g_cbu_tab[(u<<1) & 0xFF];

			// First Pixel
            y0 = g_y_tab[(y0<<1) & 0xFF];	
			*dst++ = g_clip[384+((y0 + bu)>>16)];		// Blue 
			*dst++ = g_clip[384+((y0 - gu - gv)>>16)];	// Green
            *dst++ = g_clip[384+((y0 + rv)>>16)];		// Red
			*dst++ = 0;									// Alpha

			// Second Pixel
            y1 = g_y_tab[(y1<<1) & 0xFF];	
			*dst++ = g_clip[384+((y1 + bu)>>16)];		// Blue 
			*dst++ = g_clip[384+((y1 - gu - gv)>>16)];	// Green
            *dst++ = g_clip[384+((y1 + rv)>>16)];		// Red
			*dst++ = 0;									// Alpha

			// Third Pixel
            y2 = g_y_tab[(y2<<1) & 0xFF];	
			*dst++ = g_clip[384+((y2 + bu)>>16)];		// Blue 
			*dst++ = g_clip[384+((y2 - gu - gv)>>16)];	// Green
            *dst++ = g_clip[384+((y2 + rv)>>16)];		// Red
			*dst++ = 0;									// Alpha

			// Fourth Pixel
			y3 = g_y_tab[(y3<<1) & 0xFF];
			*dst++ = g_clip[384+((y3 + bu)>>16)];		// Blue
			*dst++ = g_clip[384+((y3 - gu - gv)>>16)];	// Green
			*dst++ = g_clip[384+((y3 + rv)>>16)];		// Red
			*dst++ = 0;									// Alpha
		}
		src += width;
		dst -= n2DWAlignedLineSize;
	}
}


// Assembly lut (in memory: B1 G1 R1 0 B2 G2 R2 0 ...)

/*
Old table, this one correctly clips the Y,U,V values, but gives results which
visually differ from the one of the directdraw blt and of the C implementation.

// 0.5 for rounding from double to short, 32 for rounding before dividing by 64:
#define RGBY(i) {\
(short)(1.164 * 64 * (i - 16) + 0.5) + 32,\
(short)(1.164 * 64 * (i - 16) + 0.5) + 32,\
(short)(1.164 * 64 * (i - 16) + 0.5) + 32,\
0x00}
#define RGBU(i) {\
(short)(2.018 * 64 * (i - 128) + 0.5),\
(short)(-0.391 * 64 * (i - 128) + 0.5),\
0x00,\
0x00}
#define RGBV(i) {\
0x00,\
(short)(-0.813 * 64 * (i - 128) + 0.5),\
(short)(1.596 * 64 * (i - 128) + 0.5),\
0x00}

static const YUV2RGBASM Yuv2RgbAsm = {{{
	RGBY(0x10), RGBY(0x10), RGBY(0x10), RGBY(0x10), RGBY(0x10), RGBY(0x10), RGBY(0x10), RGBY(0x10),
	RGBY(0x10), RGBY(0x10), RGBY(0x10), RGBY(0x10), RGBY(0x10), RGBY(0x10), RGBY(0x10), RGBY(0x10),
	RGBY(0x10), RGBY(0x11), RGBY(0x12), RGBY(0x13), RGBY(0x14), RGBY(0x15), RGBY(0x16), RGBY(0x17),
	RGBY(0x18), RGBY(0x19), RGBY(0x1A), RGBY(0x1B), RGBY(0x1C), RGBY(0x1D), RGBY(0x1E), RGBY(0x1F),
	RGBY(0x20), RGBY(0x21), RGBY(0x22), RGBY(0x23), RGBY(0x24), RGBY(0x25), RGBY(0x26), RGBY(0x27),
	RGBY(0x28), RGBY(0x29), RGBY(0x2A), RGBY(0x2B), RGBY(0x2C), RGBY(0x2D), RGBY(0x2E), RGBY(0x2F),
	RGBY(0x30), RGBY(0x31), RGBY(0x32), RGBY(0x33), RGBY(0x34), RGBY(0x35), RGBY(0x36), RGBY(0x37),
	RGBY(0x38), RGBY(0x39), RGBY(0x3A), RGBY(0x3B), RGBY(0x3C), RGBY(0x3D), RGBY(0x3E), RGBY(0x3F),
	RGBY(0x40), RGBY(0x41), RGBY(0x42), RGBY(0x43), RGBY(0x44), RGBY(0x45), RGBY(0x46), RGBY(0x47),
	RGBY(0x48), RGBY(0x49), RGBY(0x4A), RGBY(0x4B), RGBY(0x4C), RGBY(0x4D), RGBY(0x4E), RGBY(0x4F),
	RGBY(0x50), RGBY(0x51), RGBY(0x52), RGBY(0x53), RGBY(0x54), RGBY(0x55), RGBY(0x56), RGBY(0x57),
	RGBY(0x58), RGBY(0x59), RGBY(0x5A), RGBY(0x5B), RGBY(0x5C), RGBY(0x5D), RGBY(0x5E), RGBY(0x5F),
	RGBY(0x60), RGBY(0x61), RGBY(0x62), RGBY(0x63), RGBY(0x64), RGBY(0x65), RGBY(0x66), RGBY(0x67),
	RGBY(0x68), RGBY(0x69), RGBY(0x6A), RGBY(0x6B), RGBY(0x6C), RGBY(0x6D), RGBY(0x6E), RGBY(0x6F),
	RGBY(0x70), RGBY(0x71), RGBY(0x72), RGBY(0x73), RGBY(0x74), RGBY(0x75), RGBY(0x76), RGBY(0x77),
	RGBY(0x78), RGBY(0x79), RGBY(0x7A), RGBY(0x7B), RGBY(0x7C), RGBY(0x7D), RGBY(0x7E), RGBY(0x7F),
	RGBY(0x80), RGBY(0x81), RGBY(0x82), RGBY(0x83), RGBY(0x84), RGBY(0x85), RGBY(0x86), RGBY(0x87),
	RGBY(0x88), RGBY(0x89), RGBY(0x8A), RGBY(0x8B), RGBY(0x8C), RGBY(0x8D), RGBY(0x8E), RGBY(0x8F),
	RGBY(0x90), RGBY(0x91), RGBY(0x92), RGBY(0x93), RGBY(0x94), RGBY(0x95), RGBY(0x96), RGBY(0x97),
	RGBY(0x98), RGBY(0x99), RGBY(0x9A), RGBY(0x9B), RGBY(0x9C), RGBY(0x9D), RGBY(0x9E), RGBY(0x9F),
	RGBY(0xA0), RGBY(0xA1), RGBY(0xA2), RGBY(0xA3), RGBY(0xA4), RGBY(0xA5), RGBY(0xA6), RGBY(0xA7),
	RGBY(0xA8), RGBY(0xA9), RGBY(0xAA), RGBY(0xAB), RGBY(0xAC), RGBY(0xAD), RGBY(0xAE), RGBY(0xAF),
	RGBY(0xB0), RGBY(0xB1), RGBY(0xB2), RGBY(0xB3), RGBY(0xB4), RGBY(0xB5), RGBY(0xB6), RGBY(0xB7),
	RGBY(0xB8), RGBY(0xB9), RGBY(0xBA), RGBY(0xBB), RGBY(0xBC), RGBY(0xBD), RGBY(0xBE), RGBY(0xBF),
	RGBY(0xC0), RGBY(0xC1), RGBY(0xC2), RGBY(0xC3), RGBY(0xC4), RGBY(0xC5), RGBY(0xC6), RGBY(0xC7),
	RGBY(0xC8), RGBY(0xC9), RGBY(0xCA), RGBY(0xCB), RGBY(0xCC), RGBY(0xCD), RGBY(0xCE), RGBY(0xCF),
	RGBY(0xD0), RGBY(0xD1), RGBY(0xD2), RGBY(0xD3), RGBY(0xD4), RGBY(0xD5), RGBY(0xD6), RGBY(0xD7),
	RGBY(0xD8), RGBY(0xD9), RGBY(0xDA), RGBY(0xDB), RGBY(0xDC), RGBY(0xDD), RGBY(0xDE), RGBY(0xDF),
	RGBY(0xE0), RGBY(0xE1), RGBY(0xE2), RGBY(0xE3), RGBY(0xE4), RGBY(0xE5), RGBY(0xE6), RGBY(0xE7),
	RGBY(0xE8), RGBY(0xE9), RGBY(0xEA), RGBY(0xEB), RGBY(0xEB), RGBY(0xEB), RGBY(0xEB), RGBY(0xEB),
	RGBY(0xEB), RGBY(0xEB), RGBY(0xEB), RGBY(0xEB), RGBY(0xEB), RGBY(0xEB), RGBY(0xEB), RGBY(0xEB),
	RGBY(0xEB), RGBY(0xEB), RGBY(0xEB), RGBY(0xEB), RGBY(0xEB), RGBY(0xEB), RGBY(0xEB), RGBY(0xEB)},{

	RGBU(0x10), RGBU(0x10), RGBU(0x10), RGBU(0x10), RGBU(0x10), RGBU(0x10), RGBU(0x10), RGBU(0x10),
	RGBU(0x10), RGBU(0x10), RGBU(0x10), RGBU(0x10), RGBU(0x10), RGBU(0x10), RGBU(0x10), RGBU(0x10),
	RGBU(0x10), RGBU(0x11), RGBU(0x12), RGBU(0x13), RGBU(0x14), RGBU(0x15), RGBU(0x16), RGBU(0x17),
	RGBU(0x18), RGBU(0x19), RGBU(0x1A), RGBU(0x1B), RGBU(0x1C), RGBU(0x1D), RGBU(0x1E), RGBU(0x1F),
	RGBU(0x20), RGBU(0x21), RGBU(0x22), RGBU(0x23), RGBU(0x24), RGBU(0x25), RGBU(0x26), RGBU(0x27),
	RGBU(0x28), RGBU(0x29), RGBU(0x2A), RGBU(0x2B), RGBU(0x2C), RGBU(0x2D), RGBU(0x2E), RGBU(0x2F),
	RGBU(0x30), RGBU(0x31), RGBU(0x32), RGBU(0x33), RGBU(0x34), RGBU(0x35), RGBU(0x36), RGBU(0x37),
	RGBU(0x38), RGBU(0x39), RGBU(0x3A), RGBU(0x3B), RGBU(0x3C), RGBU(0x3D), RGBU(0x3E), RGBU(0x3F),
	RGBU(0x40), RGBU(0x41), RGBU(0x42), RGBU(0x43), RGBU(0x44), RGBU(0x45), RGBU(0x46), RGBU(0x47),
	RGBU(0x48), RGBU(0x49), RGBU(0x4A), RGBU(0x4B), RGBU(0x4C), RGBU(0x4D), RGBU(0x4E), RGBU(0x4F),
	RGBU(0x50), RGBU(0x51), RGBU(0x52), RGBU(0x53), RGBU(0x54), RGBU(0x55), RGBU(0x56), RGBU(0x57),
	RGBU(0x58), RGBU(0x59), RGBU(0x5A), RGBU(0x5B), RGBU(0x5C), RGBU(0x5D), RGBU(0x5E), RGBU(0x5F),
	RGBU(0x60), RGBU(0x61), RGBU(0x62), RGBU(0x63), RGBU(0x64), RGBU(0x65), RGBU(0x66), RGBU(0x67),
	RGBU(0x68), RGBU(0x69), RGBU(0x6A), RGBU(0x6B), RGBU(0x6C), RGBU(0x6D), RGBU(0x6E), RGBU(0x6F),
	RGBU(0x70), RGBU(0x71), RGBU(0x72), RGBU(0x73), RGBU(0x74), RGBU(0x75), RGBU(0x76), RGBU(0x77),
	RGBU(0x78), RGBU(0x79), RGBU(0x7A), RGBU(0x7B), RGBU(0x7C), RGBU(0x7D), RGBU(0x7E), RGBU(0x7F),
	RGBU(0x80), RGBU(0x81), RGBU(0x82), RGBU(0x83), RGBU(0x84), RGBU(0x85), RGBU(0x86), RGBU(0x87),
	RGBU(0x88), RGBU(0x89), RGBU(0x8A), RGBU(0x8B), RGBU(0x8C), RGBU(0x8D), RGBU(0x8E), RGBU(0x8F),
	RGBU(0x90), RGBU(0x91), RGBU(0x92), RGBU(0x93), RGBU(0x94), RGBU(0x95), RGBU(0x96), RGBU(0x97),
	RGBU(0x98), RGBU(0x99), RGBU(0x9A), RGBU(0x9B), RGBU(0x9C), RGBU(0x9D), RGBU(0x9E), RGBU(0x9F),
	RGBU(0xA0), RGBU(0xA1), RGBU(0xA2), RGBU(0xA3), RGBU(0xA4), RGBU(0xA5), RGBU(0xA6), RGBU(0xA7),
	RGBU(0xA8), RGBU(0xA9), RGBU(0xAA), RGBU(0xAB), RGBU(0xAC), RGBU(0xAD), RGBU(0xAE), RGBU(0xAF),
	RGBU(0xB0), RGBU(0xB1), RGBU(0xB2), RGBU(0xB3), RGBU(0xB4), RGBU(0xB5), RGBU(0xB6), RGBU(0xB7),
	RGBU(0xB8), RGBU(0xB9), RGBU(0xBA), RGBU(0xBB), RGBU(0xBC), RGBU(0xBD), RGBU(0xBE), RGBU(0xBF),
	RGBU(0xC0), RGBU(0xC1), RGBU(0xC2), RGBU(0xC3), RGBU(0xC4), RGBU(0xC5), RGBU(0xC6), RGBU(0xC7),
	RGBU(0xC8), RGBU(0xC9), RGBU(0xCA), RGBU(0xCB), RGBU(0xCC), RGBU(0xCD), RGBU(0xCE), RGBU(0xCF),
	RGBU(0xD0), RGBU(0xD1), RGBU(0xD2), RGBU(0xD3), RGBU(0xD4), RGBU(0xD5), RGBU(0xD6), RGBU(0xD7),
	RGBU(0xD8), RGBU(0xD9), RGBU(0xDA), RGBU(0xDB), RGBU(0xDC), RGBU(0xDD), RGBU(0xDE), RGBU(0xDF),
	RGBU(0xE0), RGBU(0xE1), RGBU(0xE2), RGBU(0xE3), RGBU(0xE4), RGBU(0xE5), RGBU(0xE6), RGBU(0xE7),
	RGBU(0xE8), RGBU(0xE9), RGBU(0xEA), RGBU(0xEB), RGBU(0xEC), RGBU(0xED), RGBU(0xEE), RGBU(0xEF),
	RGBU(0xF0), RGBU(0xF0), RGBU(0xF0), RGBU(0xF0), RGBU(0xF0), RGBU(0xF0), RGBU(0xF0), RGBU(0xF0),
	RGBU(0xF0), RGBU(0xF0), RGBU(0xF0), RGBU(0xF0), RGBU(0xF0), RGBU(0xF0), RGBU(0xF0), RGBU(0xF0)},{

	RGBV(0x10), RGBV(0x10), RGBV(0x10), RGBV(0x10), RGBV(0x10), RGBV(0x10), RGBV(0x10), RGBV(0x10),
	RGBV(0x10), RGBV(0x10), RGBV(0x10), RGBV(0x10), RGBV(0x10), RGBV(0x10), RGBV(0x10), RGBV(0x10),
	RGBV(0x10), RGBV(0x11), RGBV(0x12), RGBV(0x13), RGBV(0x14), RGBV(0x15), RGBV(0x16), RGBV(0x17),
	RGBV(0x18), RGBV(0x19), RGBV(0x1A), RGBV(0x1B), RGBV(0x1C), RGBV(0x1D), RGBV(0x1E), RGBV(0x1F),
	RGBV(0x20), RGBV(0x21), RGBV(0x22), RGBV(0x23), RGBV(0x24), RGBV(0x25), RGBV(0x26), RGBV(0x27),
	RGBV(0x28), RGBV(0x29), RGBV(0x2A), RGBV(0x2B), RGBV(0x2C), RGBV(0x2D), RGBV(0x2E), RGBV(0x2F),
	RGBV(0x30), RGBV(0x31), RGBV(0x32), RGBV(0x33), RGBV(0x34), RGBV(0x35), RGBV(0x36), RGBV(0x37),
	RGBV(0x38), RGBV(0x39), RGBV(0x3A), RGBV(0x3B), RGBV(0x3C), RGBV(0x3D), RGBV(0x3E), RGBV(0x3F),
	RGBV(0x40), RGBV(0x41), RGBV(0x42), RGBV(0x43), RGBV(0x44), RGBV(0x45), RGBV(0x46), RGBV(0x47),
	RGBV(0x48), RGBV(0x49), RGBV(0x4A), RGBV(0x4B), RGBV(0x4C), RGBV(0x4D), RGBV(0x4E), RGBV(0x4F),
	RGBV(0x50), RGBV(0x51), RGBV(0x52), RGBV(0x53), RGBV(0x54), RGBV(0x55), RGBV(0x56), RGBV(0x57),
	RGBV(0x58), RGBV(0x59), RGBV(0x5A), RGBV(0x5B), RGBV(0x5C), RGBV(0x5D), RGBV(0x5E), RGBV(0x5F),
	RGBV(0x60), RGBV(0x61), RGBV(0x62), RGBV(0x63), RGBV(0x64), RGBV(0x65), RGBV(0x66), RGBV(0x67),
	RGBV(0x68), RGBV(0x69), RGBV(0x6A), RGBV(0x6B), RGBV(0x6C), RGBV(0x6D), RGBV(0x6E), RGBV(0x6F),
	RGBV(0x70), RGBV(0x71), RGBV(0x72), RGBV(0x73), RGBV(0x74), RGBV(0x75), RGBV(0x76), RGBV(0x77),
	RGBV(0x78), RGBV(0x79), RGBV(0x7A), RGBV(0x7B), RGBV(0x7C), RGBV(0x7D), RGBV(0x7E), RGBV(0x7F),
	RGBV(0x80), RGBV(0x81), RGBV(0x82), RGBV(0x83), RGBV(0x84), RGBV(0x85), RGBV(0x86), RGBV(0x87),
	RGBV(0x88), RGBV(0x89), RGBV(0x8A), RGBV(0x8B), RGBV(0x8C), RGBV(0x8D), RGBV(0x8E), RGBV(0x8F),
	RGBV(0x90), RGBV(0x91), RGBV(0x92), RGBV(0x93), RGBV(0x94), RGBV(0x95), RGBV(0x96), RGBV(0x97),
	RGBV(0x98), RGBV(0x99), RGBV(0x9A), RGBV(0x9B), RGBV(0x9C), RGBV(0x9D), RGBV(0x9E), RGBV(0x9F),
	RGBV(0xA0), RGBV(0xA1), RGBV(0xA2), RGBV(0xA3), RGBV(0xA4), RGBV(0xA5), RGBV(0xA6), RGBV(0xA7),
	RGBV(0xA8), RGBV(0xA9), RGBV(0xAA), RGBV(0xAB), RGBV(0xAC), RGBV(0xAD), RGBV(0xAE), RGBV(0xAF),
	RGBV(0xB0), RGBV(0xB1), RGBV(0xB2), RGBV(0xB3), RGBV(0xB4), RGBV(0xB5), RGBV(0xB6), RGBV(0xB7),
	RGBV(0xB8), RGBV(0xB9), RGBV(0xBA), RGBV(0xBB), RGBV(0xBC), RGBV(0xBD), RGBV(0xBE), RGBV(0xBF),
	RGBV(0xC0), RGBV(0xC1), RGBV(0xC2), RGBV(0xC3), RGBV(0xC4), RGBV(0xC5), RGBV(0xC6), RGBV(0xC7),
	RGBV(0xC8), RGBV(0xC9), RGBV(0xCA), RGBV(0xCB), RGBV(0xCC), RGBV(0xCD), RGBV(0xCE), RGBV(0xCF),
	RGBV(0xD0), RGBV(0xD1), RGBV(0xD2), RGBV(0xD3), RGBV(0xD4), RGBV(0xD5), RGBV(0xD6), RGBV(0xD7),
	RGBV(0xD8), RGBV(0xD9), RGBV(0xDA), RGBV(0xDB), RGBV(0xDC), RGBV(0xDD), RGBV(0xDE), RGBV(0xDF),
	RGBV(0xE0), RGBV(0xE1), RGBV(0xE2), RGBV(0xE3), RGBV(0xE4), RGBV(0xE5), RGBV(0xE6), RGBV(0xE7),
	RGBV(0xE8), RGBV(0xE9), RGBV(0xEA), RGBV(0xEB), RGBV(0xEC), RGBV(0xED), RGBV(0xEE), RGBV(0xEF),
	RGBV(0xF0), RGBV(0xF0), RGBV(0xF0), RGBV(0xF0), RGBV(0xF0), RGBV(0xF0), RGBV(0xF0), RGBV(0xF0),
	RGBV(0xF0), RGBV(0xF0), RGBV(0xF0), RGBV(0xF0), RGBV(0xF0), RGBV(0xF0), RGBV(0xF0), RGBV(0xF0)
}}};
*/

// I decided to allow the full 0..255 range for Y,U,V (somehow the capture devices return values
// which are outside the allowed range for Y,U,V). Doing so we have to lower the precision
// from 64 to 32 (that doesn't result in perceptible changes) but we get visual consistency for
// all type of conversions!

// 0.5 for rounding from double to short, 16 for rounding before dividing by 32:
#define RGBY(i) {\
(short)(1.164 * 32 * (i - 16) + 0.5) + 16,\
(short)(1.164 * 32 * (i - 16) + 0.5) + 16,\
(short)(1.164 * 32 * (i - 16) + 0.5) + 16,\
0x00}
#define RGBU(i) {\
(short)(2.018 * 32 * (i - 128) + 0.5),\
(short)(-0.391 * 32 * (i - 128) + 0.5),\
0x00,\
0x00}
#define RGBV(i) {\
0x00,\
(short)(-0.813 * 32 * (i - 128) + 0.5),\
(short)(1.596 * 32 * (i - 128) + 0.5),\
0x00}

static const YUV2RGBASM Yuv2RgbAsm = {{{
	RGBY(0x00), RGBY(0x01), RGBY(0x02), RGBY(0x03), RGBY(0x04), RGBY(0x05), RGBY(0x06), RGBY(0x07),
	RGBY(0x08), RGBY(0x09), RGBY(0x0A), RGBY(0x0B), RGBY(0x0C), RGBY(0x0D), RGBY(0x0E), RGBY(0x0F),
	RGBY(0x10), RGBY(0x11), RGBY(0x12), RGBY(0x13), RGBY(0x14), RGBY(0x15), RGBY(0x16), RGBY(0x17),
	RGBY(0x18), RGBY(0x19), RGBY(0x1A), RGBY(0x1B), RGBY(0x1C), RGBY(0x1D), RGBY(0x1E), RGBY(0x1F),
	RGBY(0x20), RGBY(0x21), RGBY(0x22), RGBY(0x23), RGBY(0x24), RGBY(0x25), RGBY(0x26), RGBY(0x27),
	RGBY(0x28), RGBY(0x29), RGBY(0x2A), RGBY(0x2B), RGBY(0x2C), RGBY(0x2D), RGBY(0x2E), RGBY(0x2F),
	RGBY(0x30), RGBY(0x31), RGBY(0x32), RGBY(0x33), RGBY(0x34), RGBY(0x35), RGBY(0x36), RGBY(0x37),
	RGBY(0x38), RGBY(0x39), RGBY(0x3A), RGBY(0x3B), RGBY(0x3C), RGBY(0x3D), RGBY(0x3E), RGBY(0x3F),
	RGBY(0x40), RGBY(0x41), RGBY(0x42), RGBY(0x43), RGBY(0x44), RGBY(0x45), RGBY(0x46), RGBY(0x47),
	RGBY(0x48), RGBY(0x49), RGBY(0x4A), RGBY(0x4B), RGBY(0x4C), RGBY(0x4D), RGBY(0x4E), RGBY(0x4F),
	RGBY(0x50), RGBY(0x51), RGBY(0x52), RGBY(0x53), RGBY(0x54), RGBY(0x55), RGBY(0x56), RGBY(0x57),
	RGBY(0x58), RGBY(0x59), RGBY(0x5A), RGBY(0x5B), RGBY(0x5C), RGBY(0x5D), RGBY(0x5E), RGBY(0x5F),
	RGBY(0x60), RGBY(0x61), RGBY(0x62), RGBY(0x63), RGBY(0x64), RGBY(0x65), RGBY(0x66), RGBY(0x67),
	RGBY(0x68), RGBY(0x69), RGBY(0x6A), RGBY(0x6B), RGBY(0x6C), RGBY(0x6D), RGBY(0x6E), RGBY(0x6F),
	RGBY(0x70), RGBY(0x71), RGBY(0x72), RGBY(0x73), RGBY(0x74), RGBY(0x75), RGBY(0x76), RGBY(0x77),
	RGBY(0x78), RGBY(0x79), RGBY(0x7A), RGBY(0x7B), RGBY(0x7C), RGBY(0x7D), RGBY(0x7E), RGBY(0x7F),
	RGBY(0x80), RGBY(0x81), RGBY(0x82), RGBY(0x83), RGBY(0x84), RGBY(0x85), RGBY(0x86), RGBY(0x87),
	RGBY(0x88), RGBY(0x89), RGBY(0x8A), RGBY(0x8B), RGBY(0x8C), RGBY(0x8D), RGBY(0x8E), RGBY(0x8F),
	RGBY(0x90), RGBY(0x91), RGBY(0x92), RGBY(0x93), RGBY(0x94), RGBY(0x95), RGBY(0x96), RGBY(0x97),
	RGBY(0x98), RGBY(0x99), RGBY(0x9A), RGBY(0x9B), RGBY(0x9C), RGBY(0x9D), RGBY(0x9E), RGBY(0x9F),
	RGBY(0xA0), RGBY(0xA1), RGBY(0xA2), RGBY(0xA3), RGBY(0xA4), RGBY(0xA5), RGBY(0xA6), RGBY(0xA7),
	RGBY(0xA8), RGBY(0xA9), RGBY(0xAA), RGBY(0xAB), RGBY(0xAC), RGBY(0xAD), RGBY(0xAE), RGBY(0xAF),
	RGBY(0xB0), RGBY(0xB1), RGBY(0xB2), RGBY(0xB3), RGBY(0xB4), RGBY(0xB5), RGBY(0xB6), RGBY(0xB7),
	RGBY(0xB8), RGBY(0xB9), RGBY(0xBA), RGBY(0xBB), RGBY(0xBC), RGBY(0xBD), RGBY(0xBE), RGBY(0xBF),
	RGBY(0xC0), RGBY(0xC1), RGBY(0xC2), RGBY(0xC3), RGBY(0xC4), RGBY(0xC5), RGBY(0xC6), RGBY(0xC7),
	RGBY(0xC8), RGBY(0xC9), RGBY(0xCA), RGBY(0xCB), RGBY(0xCC), RGBY(0xCD), RGBY(0xCE), RGBY(0xCF),
	RGBY(0xD0), RGBY(0xD1), RGBY(0xD2), RGBY(0xD3), RGBY(0xD4), RGBY(0xD5), RGBY(0xD6), RGBY(0xD7),
	RGBY(0xD8), RGBY(0xD9), RGBY(0xDA), RGBY(0xDB), RGBY(0xDC), RGBY(0xDD), RGBY(0xDE), RGBY(0xDF),
	RGBY(0xE0), RGBY(0xE1), RGBY(0xE2), RGBY(0xE3), RGBY(0xE4), RGBY(0xE5), RGBY(0xE6), RGBY(0xE7),
	RGBY(0xE8), RGBY(0xE9), RGBY(0xEA), RGBY(0xEB), RGBY(0xEC), RGBY(0xED), RGBY(0xEE), RGBY(0xEF),
	RGBY(0xF0), RGBY(0xF1), RGBY(0xF2), RGBY(0xF3), RGBY(0xF4), RGBY(0xF5), RGBY(0xF6), RGBY(0xF7),
	RGBY(0xF8), RGBY(0xF9), RGBY(0xFA), RGBY(0xEB), RGBY(0xFC), RGBY(0xFD), RGBY(0xFE), RGBY(0xFF)},{

	RGBU(0x00), RGBU(0x01), RGBU(0x02), RGBU(0x03), RGBU(0x04), RGBU(0x05), RGBU(0x06), RGBU(0x07),
	RGBU(0x08), RGBU(0x09), RGBU(0x0A), RGBU(0x0B), RGBU(0x0C), RGBU(0x0D), RGBU(0x0E), RGBU(0x0F),
	RGBU(0x10), RGBU(0x11), RGBU(0x12), RGBU(0x13), RGBU(0x14), RGBU(0x15), RGBU(0x16), RGBU(0x17),
	RGBU(0x18), RGBU(0x19), RGBU(0x1A), RGBU(0x1B), RGBU(0x1C), RGBU(0x1D), RGBU(0x1E), RGBU(0x1F),
	RGBU(0x20), RGBU(0x21), RGBU(0x22), RGBU(0x23), RGBU(0x24), RGBU(0x25), RGBU(0x26), RGBU(0x27),
	RGBU(0x28), RGBU(0x29), RGBU(0x2A), RGBU(0x2B), RGBU(0x2C), RGBU(0x2D), RGBU(0x2E), RGBU(0x2F),
	RGBU(0x30), RGBU(0x31), RGBU(0x32), RGBU(0x33), RGBU(0x34), RGBU(0x35), RGBU(0x36), RGBU(0x37),
	RGBU(0x38), RGBU(0x39), RGBU(0x3A), RGBU(0x3B), RGBU(0x3C), RGBU(0x3D), RGBU(0x3E), RGBU(0x3F),
	RGBU(0x40), RGBU(0x41), RGBU(0x42), RGBU(0x43), RGBU(0x44), RGBU(0x45), RGBU(0x46), RGBU(0x47),
	RGBU(0x48), RGBU(0x49), RGBU(0x4A), RGBU(0x4B), RGBU(0x4C), RGBU(0x4D), RGBU(0x4E), RGBU(0x4F),
	RGBU(0x50), RGBU(0x51), RGBU(0x52), RGBU(0x53), RGBU(0x54), RGBU(0x55), RGBU(0x56), RGBU(0x57),
	RGBU(0x58), RGBU(0x59), RGBU(0x5A), RGBU(0x5B), RGBU(0x5C), RGBU(0x5D), RGBU(0x5E), RGBU(0x5F),
	RGBU(0x60), RGBU(0x61), RGBU(0x62), RGBU(0x63), RGBU(0x64), RGBU(0x65), RGBU(0x66), RGBU(0x67),
	RGBU(0x68), RGBU(0x69), RGBU(0x6A), RGBU(0x6B), RGBU(0x6C), RGBU(0x6D), RGBU(0x6E), RGBU(0x6F),
	RGBU(0x70), RGBU(0x71), RGBU(0x72), RGBU(0x73), RGBU(0x74), RGBU(0x75), RGBU(0x76), RGBU(0x77),
	RGBU(0x78), RGBU(0x79), RGBU(0x7A), RGBU(0x7B), RGBU(0x7C), RGBU(0x7D), RGBU(0x7E), RGBU(0x7F),
	RGBU(0x80), RGBU(0x81), RGBU(0x82), RGBU(0x83), RGBU(0x84), RGBU(0x85), RGBU(0x86), RGBU(0x87),
	RGBU(0x88), RGBU(0x89), RGBU(0x8A), RGBU(0x8B), RGBU(0x8C), RGBU(0x8D), RGBU(0x8E), RGBU(0x8F),
	RGBU(0x90), RGBU(0x91), RGBU(0x92), RGBU(0x93), RGBU(0x94), RGBU(0x95), RGBU(0x96), RGBU(0x97),
	RGBU(0x98), RGBU(0x99), RGBU(0x9A), RGBU(0x9B), RGBU(0x9C), RGBU(0x9D), RGBU(0x9E), RGBU(0x9F),
	RGBU(0xA0), RGBU(0xA1), RGBU(0xA2), RGBU(0xA3), RGBU(0xA4), RGBU(0xA5), RGBU(0xA6), RGBU(0xA7),
	RGBU(0xA8), RGBU(0xA9), RGBU(0xAA), RGBU(0xAB), RGBU(0xAC), RGBU(0xAD), RGBU(0xAE), RGBU(0xAF),
	RGBU(0xB0), RGBU(0xB1), RGBU(0xB2), RGBU(0xB3), RGBU(0xB4), RGBU(0xB5), RGBU(0xB6), RGBU(0xB7),
	RGBU(0xB8), RGBU(0xB9), RGBU(0xBA), RGBU(0xBB), RGBU(0xBC), RGBU(0xBD), RGBU(0xBE), RGBU(0xBF),
	RGBU(0xC0), RGBU(0xC1), RGBU(0xC2), RGBU(0xC3), RGBU(0xC4), RGBU(0xC5), RGBU(0xC6), RGBU(0xC7),
	RGBU(0xC8), RGBU(0xC9), RGBU(0xCA), RGBU(0xCB), RGBU(0xCC), RGBU(0xCD), RGBU(0xCE), RGBU(0xCF),
	RGBU(0xD0), RGBU(0xD1), RGBU(0xD2), RGBU(0xD3), RGBU(0xD4), RGBU(0xD5), RGBU(0xD6), RGBU(0xD7),
	RGBU(0xD8), RGBU(0xD9), RGBU(0xDA), RGBU(0xDB), RGBU(0xDC), RGBU(0xDD), RGBU(0xDE), RGBU(0xDF),
	RGBU(0xE0), RGBU(0xE1), RGBU(0xE2), RGBU(0xE3), RGBU(0xE4), RGBU(0xE5), RGBU(0xE6), RGBU(0xE7),
	RGBU(0xE8), RGBU(0xE9), RGBU(0xEA), RGBU(0xEB), RGBU(0xEC), RGBU(0xED), RGBU(0xEE), RGBU(0xEF),
	RGBU(0xF0), RGBU(0xF1), RGBU(0xF2), RGBU(0xF3), RGBU(0xF4), RGBU(0xF5), RGBU(0xF6), RGBU(0xF7),
	RGBU(0xF8), RGBU(0xF9), RGBU(0xFA), RGBU(0xFB), RGBU(0xFC), RGBU(0xFD), RGBU(0xFE), RGBU(0xFF)},{

	RGBV(0x00), RGBV(0x01), RGBV(0x02), RGBV(0x03), RGBV(0x04), RGBV(0x05), RGBV(0x06), RGBV(0x07),
	RGBV(0x08), RGBV(0x09), RGBV(0x0A), RGBV(0x0B), RGBV(0x0C), RGBV(0x0D), RGBV(0x0E), RGBV(0x0F),
	RGBV(0x10), RGBV(0x11), RGBV(0x12), RGBV(0x13), RGBV(0x14), RGBV(0x15), RGBV(0x16), RGBV(0x17),
	RGBV(0x18), RGBV(0x19), RGBV(0x1A), RGBV(0x1B), RGBV(0x1C), RGBV(0x1D), RGBV(0x1E), RGBV(0x1F),
	RGBV(0x20), RGBV(0x21), RGBV(0x22), RGBV(0x23), RGBV(0x24), RGBV(0x25), RGBV(0x26), RGBV(0x27),
	RGBV(0x28), RGBV(0x29), RGBV(0x2A), RGBV(0x2B), RGBV(0x2C), RGBV(0x2D), RGBV(0x2E), RGBV(0x2F),
	RGBV(0x30), RGBV(0x31), RGBV(0x32), RGBV(0x33), RGBV(0x34), RGBV(0x35), RGBV(0x36), RGBV(0x37),
	RGBV(0x38), RGBV(0x39), RGBV(0x3A), RGBV(0x3B), RGBV(0x3C), RGBV(0x3D), RGBV(0x3E), RGBV(0x3F),
	RGBV(0x40), RGBV(0x41), RGBV(0x42), RGBV(0x43), RGBV(0x44), RGBV(0x45), RGBV(0x46), RGBV(0x47),
	RGBV(0x48), RGBV(0x49), RGBV(0x4A), RGBV(0x4B), RGBV(0x4C), RGBV(0x4D), RGBV(0x4E), RGBV(0x4F),
	RGBV(0x50), RGBV(0x51), RGBV(0x52), RGBV(0x53), RGBV(0x54), RGBV(0x55), RGBV(0x56), RGBV(0x57),
	RGBV(0x58), RGBV(0x59), RGBV(0x5A), RGBV(0x5B), RGBV(0x5C), RGBV(0x5D), RGBV(0x5E), RGBV(0x5F),
	RGBV(0x60), RGBV(0x61), RGBV(0x62), RGBV(0x63), RGBV(0x64), RGBV(0x65), RGBV(0x66), RGBV(0x67),
	RGBV(0x68), RGBV(0x69), RGBV(0x6A), RGBV(0x6B), RGBV(0x6C), RGBV(0x6D), RGBV(0x6E), RGBV(0x6F),
	RGBV(0x70), RGBV(0x71), RGBV(0x72), RGBV(0x73), RGBV(0x74), RGBV(0x75), RGBV(0x76), RGBV(0x77),
	RGBV(0x78), RGBV(0x79), RGBV(0x7A), RGBV(0x7B), RGBV(0x7C), RGBV(0x7D), RGBV(0x7E), RGBV(0x7F),
	RGBV(0x80), RGBV(0x81), RGBV(0x82), RGBV(0x83), RGBV(0x84), RGBV(0x85), RGBV(0x86), RGBV(0x87),
	RGBV(0x88), RGBV(0x89), RGBV(0x8A), RGBV(0x8B), RGBV(0x8C), RGBV(0x8D), RGBV(0x8E), RGBV(0x8F),
	RGBV(0x90), RGBV(0x91), RGBV(0x92), RGBV(0x93), RGBV(0x94), RGBV(0x95), RGBV(0x96), RGBV(0x97),
	RGBV(0x98), RGBV(0x99), RGBV(0x9A), RGBV(0x9B), RGBV(0x9C), RGBV(0x9D), RGBV(0x9E), RGBV(0x9F),
	RGBV(0xA0), RGBV(0xA1), RGBV(0xA2), RGBV(0xA3), RGBV(0xA4), RGBV(0xA5), RGBV(0xA6), RGBV(0xA7),
	RGBV(0xA8), RGBV(0xA9), RGBV(0xAA), RGBV(0xAB), RGBV(0xAC), RGBV(0xAD), RGBV(0xAE), RGBV(0xAF),
	RGBV(0xB0), RGBV(0xB1), RGBV(0xB2), RGBV(0xB3), RGBV(0xB4), RGBV(0xB5), RGBV(0xB6), RGBV(0xB7),
	RGBV(0xB8), RGBV(0xB9), RGBV(0xBA), RGBV(0xBB), RGBV(0xBC), RGBV(0xBD), RGBV(0xBE), RGBV(0xBF),
	RGBV(0xC0), RGBV(0xC1), RGBV(0xC2), RGBV(0xC3), RGBV(0xC4), RGBV(0xC5), RGBV(0xC6), RGBV(0xC7),
	RGBV(0xC8), RGBV(0xC9), RGBV(0xCA), RGBV(0xCB), RGBV(0xCC), RGBV(0xCD), RGBV(0xCE), RGBV(0xCF),
	RGBV(0xD0), RGBV(0xD1), RGBV(0xD2), RGBV(0xD3), RGBV(0xD4), RGBV(0xD5), RGBV(0xD6), RGBV(0xD7),
	RGBV(0xD8), RGBV(0xD9), RGBV(0xDA), RGBV(0xDB), RGBV(0xDC), RGBV(0xDD), RGBV(0xDE), RGBV(0xDF),
	RGBV(0xE0), RGBV(0xE1), RGBV(0xE2), RGBV(0xE3), RGBV(0xE4), RGBV(0xE5), RGBV(0xE6), RGBV(0xE7),
	RGBV(0xE8), RGBV(0xE9), RGBV(0xEA), RGBV(0xEB), RGBV(0xEC), RGBV(0xED), RGBV(0xEE), RGBV(0xEF),
	RGBV(0xF0), RGBV(0xF1), RGBV(0xF2), RGBV(0xF3), RGBV(0xF4), RGBV(0xF5), RGBV(0xF6), RGBV(0xF7),
	RGBV(0xF8), RGBV(0xF9), RGBV(0xFA), RGBV(0xFB), RGBV(0xFC), RGBV(0xFD), RGBV(0xFE), RGBV(0xFF)
}}};


/*
#define RGB_Y_Prec(value,prec) ((int)(1.164 * prec * (value - 16) + 0.5) + (prec/2))
#define RGB_BU_Prec(value,prec) (int)(2.018 * prec * (value - 128) + 0.5)
#define RGB_GU_Prec(value,prec) (int)(-0.391 * prec * (value - 128) + 0.5)
#define RGB_GV_Prec(value,prec) (int)(-0.813 * prec * (value - 128) + 0.5)
#define RGB_RV_Prec(value,prec) (int)(1.596 * prec * (value - 128) + 0.5)
void YUV420ToRGB32TestPrec(	LPBYTE src0,	// Y Plane
							LPBYTE src1,	// U Plane
							LPBYTE src2,	// V Plane
							LPDWORD dst,	// RGB32 Dib
							int width,
							int height)	
{
	int precision = 64;
	for (int j = (height - 1) ; j > 0 ; j -= 2)
	{
		LPBYTE py1 = src0 + (height - 1 - j) * width;
		LPBYTE py2 = py1 + width;
		LPDWORD d1 = dst + j * width;
		LPDWORD d2 = d1 - width;
		for (int i = 0 ; i < width ; i += 2)
		{
			int u = *src1++;
			int v = *src2++;
			int rv = RGB_RV_Prec(v, precision);
			int bu = RGB_BU_Prec(u, precision);
			int gugv = RGB_GU_Prec(u, precision) + RGB_GV_Prec(v, precision);

			// up-left
			int y1 = RGB_Y_Prec(*py1++, precision);	
			*d1++ = (DWORD)g_clip[384+((y1 + bu)/precision)]			|	// Blue 
					((DWORD)g_clip[384+((y1 + gugv)/precision)] << 8)	|	// Green
					((DWORD)g_clip[384+((y1 + rv)/precision)] << 16);		// Red

			// up-right
			y1 = RGB_Y_Prec(*py1++, precision);
			*d1++ = (DWORD)g_clip[384+((y1 + bu)/precision)]			|	// Blue
					((DWORD)g_clip[384+((y1 + gugv)/precision)] << 8)	|	// Green
					((DWORD)g_clip[384+((y1 + rv)/precision)] << 16);		// Red

			// down-left
			int y2 = RGB_Y_Prec(*py2++, precision);
			*d2++ = (DWORD)g_clip[384+((y2 + bu)/precision)]			|	// Blue
					((DWORD)g_clip[384+((y2 + gugv)/precision)] << 8)	|	// Green
					((DWORD)g_clip[384+((y2 + rv)/precision)] << 16);		// Red

			// down-right
			y2 = RGB_Y_Prec(*py2++, precision);
			*d2++ = (DWORD)g_clip[384+((y2 + bu)/precision)]			|	// Blue
					((DWORD)g_clip[384+((y2 + gugv)/precision)] << 8)	|	// Green
					((DWORD)g_clip[384+((y2 + rv)/precision)] << 16);		// Red
		}
	}
}

void YUV420ToRGB32EmuAsm(	LPBYTE src0,	// Y Plane
							LPBYTE src1,	// U Plane
							LPBYTE src2,	// V Plane
							LPDWORD dst,	// RGB32 Dib
							int width,
							int height)	
{
	for (int j = (height - 1) ; j > 0 ; j -= 2)
	{
		LPBYTE py1 = src0 + (height - 1 - j) * width;
		LPBYTE py2 = py1 + width;
		LPDWORD d1 = dst + j * width;
		LPDWORD d2 = d1 - width;
		for (int i = 0 ; i < width ; i += 2)
		{
			int u = *src1++;
			int v = *src2++;
			int rv = Yuv2RgbAsm.lut.CoefficientsRGBV[v][2];
			int bu = Yuv2RgbAsm.lut.CoefficientsRGBU[u][0];
			int gugv = Yuv2RgbAsm.lut.CoefficientsRGBU[u][1] + Yuv2RgbAsm.lut.CoefficientsRGBV[v][1];

			// up-left
			int y1 = Yuv2RgbAsm.lut.CoefficientsRGBY[*py1++][0];
			*d1++ = (DWORD)g_clip[384+((y1 + bu)>>5)]			|	// Blue 
					((DWORD)g_clip[384+((y1 + gugv)>>5)] << 8)	|	// Green
					((DWORD)g_clip[384+((y1 + rv)>>5)] << 16);		// Red

			// up-right
			y1 = Yuv2RgbAsm.lut.CoefficientsRGBY[*py1++][0];
			*d1++ = (DWORD)g_clip[384+((y1 + bu)>>5)]			|	// Blue
					((DWORD)g_clip[384+((y1 + gugv)>>5)] << 8)	|	// Green
					((DWORD)g_clip[384+((y1 + rv)>>5)] << 16);		// Red

			// down-left
			int y2 = Yuv2RgbAsm.lut.CoefficientsRGBY[*py2++][0];
			*d2++ = (DWORD)g_clip[384+((y2 + bu)>>5)]			|	// Blue
					((DWORD)g_clip[384+((y2 + gugv)>>5)] << 8)	|	// Green
					((DWORD)g_clip[384+((y2 + rv)>>5)] << 16);		// Red

			// down-right
			y2 = Yuv2RgbAsm.lut.CoefficientsRGBY[*py2++][0];
			*d2++ = (DWORD)g_clip[384+((y2 + bu)>>5)]			|	// Blue
					((DWORD)g_clip[384+((y2 + gugv)>>5)] << 8)	|	// Green
					((DWORD)g_clip[384+((y2 + rv)>>5)] << 16);		// Red
		}
	}
}
*/

#pragma warning(disable: 4100)

__declspec(naked)
void __cdecl YUV420ToRGB32Asm(	LPBYTE src0,	// Y Plane
								LPBYTE src1,	// U Plane
								LPBYTE src2,	// V Plane
								LPDWORD dst,	// RGB32 Dib
								int width,
								int height)	
{
	__asm {		
		pushad

		mov       edx, [esp + 32 + 4]	; src0
		mov       edi, [esp + 32 + 8]	; src1
		mov       esi, [esp + 32 + 12]	; src2
		mov       ebp, [esp + 32 + 16]	; dst
		mov       ecx, [esp + 32 + 20]	; width
		mov       ebx, [esp + 32 + 24]	; height

		// Adjust ebp to point to the second last line,
		// this because we convert from top-down yuv to bottom-up rgb 
		sub       ebx, 2				; height - 2
		lea       eax, [4 * ecx]		; eax is now 4 * width
		imul      eax, ebx				; (4 * width) * (height - 2)
		add       ebx, 2				; restore height
		add       ebp, eax				; ebp = dst + (4 * width) * (height - 2)

		// Reset eax
		xor       eax, eax

// Height loop
hloop : 
		push      ebx		; store height countdown
		mov       ebx, ecx	; init ebx with width

// Width loop
wloop :	
		push      ebx		; store width countdown
		xor       ebx, ebx	; reset ebx

		// Load U, V
		mov       al, [edi]	; U
		mov       bl, [esi]	; V
		movq      mm0, [Yuv2RgbAsm.lut.CoefficientsRGBU + 8 * eax] ; | BU |   GU    | 0  | 0 |
		paddw     mm0, [Yuv2RgbAsm.lut.CoefficientsRGBV + 8 * ebx] ; | BU | GU + GV | RV | 0 | = | BU | GU | 0 | 0 |  +  | 0 | GV | RV | 0 |

		// Load Y1 and Y2 of first line
		mov       al, [edx]				; Y1 
		mov       bl, [edx + 1]			; Y2    
		movq      mm1, [Yuv2RgbAsm.lut.CoefficientsRGBY + 8 * eax] ; | BY1 | GY1 | RY1 | 0 |
		movq      mm2, [Yuv2RgbAsm.lut.CoefficientsRGBY + 8 * ebx] ; | BY2 | GY2 | RY2 | 0 |

		// Load Y2 and Y3 of next line
		mov       al, [edx + ecx]		; Y3
		mov       bl, [edx + ecx + 1]	; Y4
		movq      mm3, [Yuv2RgbAsm.lut.CoefficientsRGBY + 8 * eax] ; | BY3 | GY3 | RY3 | 0 |
		movq      mm4, [Yuv2RgbAsm.lut.CoefficientsRGBY + 8 * ebx] ; | BY4 | GY4 | RY4 | 0 |

		// Add together
		paddw     mm1, mm0	; | BY1 + BU | GY1 + GU + GV | RY1 + RV | 0 |
		paddw     mm2, mm0	; | BY2 + BU | GY2 + GU + GV | RY2 + RV | 0 |
		paddw     mm3, mm0	; | BY3 + BU | GY3 + GU + GV | RY3 + RV | 0 |
		paddw     mm4, mm0	; | BY4 + BU | GY4 + GU + GV | RY4 + RV | 0 |

		// Divide by 32 (look-up table is pre-multiplied by 32,
		// that's the max possible factor to still fit a signed word)
		psraw     mm1, 5
		psraw     mm2, 5
		psraw     mm3, 5
		psraw     mm4, 5
		
		// Pack signed word to unsigned byte clipping to 0 .. 255
		packuswb  mm1, mm2	; | B1 | G1 | R1 | 0 | B2 | G2 | R2 | 0 |
		packuswb  mm3, mm4	; | B3 | G3 | R3 | 0 | B4 | G4 | R4 | 0 |

		// Move to destination RGB, mm3 first because we are storing bottom-up
		movq      [ebp], mm3			; ^ | B3 | G3 | R3 | 0 | B4 | G4 | R4 | 0 |
		movq      [ebp + 4 * ecx], mm1	; | | B1 | G1 | R1 | 0 | B2 | G2 | R2 | 0 |

		// Increment pointers
		add       ebp, 8	; go to the next two destination RGB pixels
		add       edx, 2	; increment Y source
		add       edi, 1	; increment U source
		add       esi, 1	; increment V source

// End width loop
		pop       ebx		; pop width countdown
		sub       ebx, 2	; decrement it by 2 because we just processed two horizontal pixels
		jnz       wloop		; 0 -> we reached the end of line
		
		// Increment 1x line for Y and decrement by 1+2=3x lines for RGB
		add       edx, ecx
		imul      ebx, ecx, 12
		sub       ebp, ebx

// End height loop
		pop       ebx		; pop height countdown
		sub       ebx, 2	; decrement it by 2 because we just processed two lines
		jnz       hloop		; 0 -> we reached the end of image
		
		// Clean-up
		emms
		popad
		ret
	}
}

__declspec(naked)
void __cdecl YUY2ToRGB32Asm(	LPBYTE src,		// Y0 U0 Y1 V0, Y2 U2 Y3 V2, ...
								LPDWORD dst,	// RGB32 Dib
								int width,
								int height)
{
	__asm {		
		pushad

		mov       edx, [esp + 32 + 4]	; src
		mov       ebp, [esp + 32 + 8]	; dst
		mov       ecx, [esp + 32 + 12]	; width
		mov       ebx, [esp + 32 + 16]	; height

		// Adjust ebp to point to the last line,
		// this because we convert from top-down yuv to bottom-up rgb 
		sub       ebx, 1				; height - 1
		lea       eax, [4 * ecx]		; eax is now 4 * width
		imul      eax, ebx				; (4 * width) * (height - 1)
		add       ebx, 1				; restore height
		add       ebp, eax				; ebp = dst + (4 * width) * (height - 1)

		// Reset eax
		xor       eax, eax

// Height loop
hloop : 
		push      ebx		; store height countdown
		mov       ebx, ecx	; init ebx with width

// Width loop
wloop :	
		push      ebx		; store width countdown
		xor       ebx, ebx	; reset ebx

		// Load U, V
		mov       al, [edx + 1]	; U
		mov       bl, [edx + 3]	; V
		movq      mm0, [Yuv2RgbAsm.lut.CoefficientsRGBU + 8 * eax] ; | BU |   GU    | 0  | 0 |
		paddw     mm0, [Yuv2RgbAsm.lut.CoefficientsRGBV + 8 * ebx] ; | BU | GU + GV | RV | 0 | = | BU | GU | 0 | 0 |  +  | 0 | GV | RV | 0 |

		// Load Y0, Y1
		mov       al, [edx]		; Y0 
		mov       bl, [edx + 2]	; Y1    
		movq      mm1, [Yuv2RgbAsm.lut.CoefficientsRGBY + 8 * eax] ; | BY0 | GY0 | RY0 | 0 |
		movq      mm2, [Yuv2RgbAsm.lut.CoefficientsRGBY + 8 * ebx] ; | BY1 | GY1 | RY1 | 0 |

		// Add together
		paddw     mm1, mm0	; | BY0 + BU | GY0 + GU + GV | RY0 + RV | 0 |
		paddw     mm2, mm0	; | BY1 + BU | GY1 + GU + GV | RY1 + RV | 0 |

		// Divide by 32 (look-up table is pre-multiplied by 32,
		// that's the max possible factor to still fit a signed word)
		psraw     mm1, 5
		psraw     mm2, 5
		
		// Pack signed word to unsigned byte clipping to 0 .. 255
		packuswb  mm1, mm2	; | B0 | G0 | R0 | 0 | B1 | G1 | R1 | 0 |

		// Move to destination RGB
		movq      [ebp], mm1; | B0 | G0 | R0 | 0 | B1 | G1 | R1 | 0 |

		// Increment pointers
		add       ebp, 8	; go to the next two destination RGB pixels
		add       edx, 4	; increment source

// End width loop
		pop       ebx		; pop width countdown
		sub       ebx, 2	; decrement it by 2 because we just processed two horizontal pixels
		jnz       wloop		; 0 -> we reached the end of line
		
		// Decrement by 2x lines for RGB
		lea       ebx, [8 * ecx] ; ebx is now 8 * width
		sub       ebp, ebx

// End height loop
		pop       ebx		; pop height countdown
		sub       ebx, 1	; decrement it
		jnz       hloop		; 0 -> we reached the end of image
		
		// Clean-up
		emms
		popad
		ret
	}
}

__declspec(naked)
void __cdecl UYVYToRGB32Asm(LPBYTE src,		// U0 Y0 V0 Y1, U2 Y2 V2 Y3, ...
							LPDWORD dst,	// RGB32 Dib
							int width,
							int height)
{
	__asm {		
		pushad

		mov       edx, [esp + 32 + 4]	; src
		mov       ebp, [esp + 32 + 8]	; dst
		mov       ecx, [esp + 32 + 12]	; width
		mov       ebx, [esp + 32 + 16]	; height

		// Adjust ebp to point to the last line,
		// this because we convert from top-down yuv to bottom-up rgb 
		sub       ebx, 1				; height - 1
		lea       eax, [4 * ecx]		; eax is now 4 * width
		imul      eax, ebx				; (4 * width) * (height - 1)
		add       ebx, 1				; restore height
		add       ebp, eax				; ebp = dst + (4 * width) * (height - 1)

		// Reset eax
		xor       eax, eax

// Height loop
hloop : 
		push      ebx		; store height countdown
		mov       ebx, ecx	; init ebx with width

// Width loop
wloop :	
		push      ebx		; store width countdown
		xor       ebx, ebx	; reset ebx

		// Load U, V
		mov       al, [edx]		; U
		mov       bl, [edx + 2]	; V
		movq      mm0, [Yuv2RgbAsm.lut.CoefficientsRGBU + 8 * eax] ; | BU |   GU    | 0  | 0 |
		paddw     mm0, [Yuv2RgbAsm.lut.CoefficientsRGBV + 8 * ebx] ; | BU | GU + GV | RV | 0 | = | BU | GU | 0 | 0 |  +  | 0 | GV | RV | 0 |

		// Load Y0, Y1
		mov       al, [edx + 1]	; Y0 
		mov       bl, [edx + 3]	; Y1    
		movq      mm1, [Yuv2RgbAsm.lut.CoefficientsRGBY + 8 * eax] ; | BY0 | GY0 | RY0 | 0 |
		movq      mm2, [Yuv2RgbAsm.lut.CoefficientsRGBY + 8 * ebx] ; | BY1 | GY1 | RY1 | 0 |

		// Add together
		paddw     mm1, mm0	; | BY0 + BU | GY0 + GU + GV | RY0 + RV | 0 |
		paddw     mm2, mm0	; | BY1 + BU | GY1 + GU + GV | RY1 + RV | 0 |

		// Divide by 32 (look-up table is pre-multiplied by 32,
		// that's the max possible factor to still fit a signed word)
		psraw     mm1, 5
		psraw     mm2, 5
		
		// Pack signed word to unsigned byte clipping to 0 .. 255
		packuswb  mm1, mm2	; | B0 | G0 | R0 | 0 | B1 | G1 | R1 | 0 |

		// Move to destination RGB
		movq      [ebp], mm1; | B0 | G0 | R0 | 0 | B1 | G1 | R1 | 0 |

		// Increment pointers
		add       ebp, 8	; go to the next two destination RGB pixels
		add       edx, 4	; increment source

// End width loop
		pop       ebx		; pop width countdown
		sub       ebx, 2	; decrement it by 2 because we just processed two horizontal pixels
		jnz       wloop		; 0 -> we reached the end of line
		
		// Decrement by 2x lines for RGB
		lea       ebx, [8 * ecx] ; ebx is now 8 * width
		sub       ebp, ebx

// End height loop
		pop       ebx		; pop height countdown
		sub       ebx, 1	; decrement it
		jnz       hloop		; 0 -> we reached the end of image
		
		// Clean-up
		emms
		popad
		ret
	}
}

__declspec(naked)
void __cdecl YVYUToRGB32Asm(LPBYTE src,		// Y0 V0 Y1 U0, Y2 V2 Y3 U2, ...
							LPDWORD dst,	// RGB32 Dib
							int width,
							int height)
{
	__asm {		
		pushad

		mov       edx, [esp + 32 + 4]	; src
		mov       ebp, [esp + 32 + 8]	; dst
		mov       ecx, [esp + 32 + 12]	; width
		mov       ebx, [esp + 32 + 16]	; height

		// Adjust ebp to point to the last line,
		// this because we convert from top-down yuv to bottom-up rgb 
		sub       ebx, 1				; height - 1
		lea       eax, [4 * ecx]		; eax is now 4 * width
		imul      eax, ebx				; (4 * width) * (height - 1)
		add       ebx, 1				; restore height
		add       ebp, eax				; ebp = dst + (4 * width) * (height - 1)

		// Reset eax
		xor       eax, eax

// Height loop
hloop : 
		push      ebx		; store height countdown
		mov       ebx, ecx	; init ebx with width

// Width loop
wloop :	
		push      ebx		; store width countdown
		xor       ebx, ebx	; reset ebx

		// Load U, V
		mov       al, [edx + 3]	; U
		mov       bl, [edx + 1]	; V
		movq      mm0, [Yuv2RgbAsm.lut.CoefficientsRGBU + 8 * eax] ; | BU |   GU    | 0  | 0 |
		paddw     mm0, [Yuv2RgbAsm.lut.CoefficientsRGBV + 8 * ebx] ; | BU | GU + GV | RV | 0 | = | BU | GU | 0 | 0 |  +  | 0 | GV | RV | 0 |

		// Load Y0, Y1
		mov       al, [edx]		; Y0 
		mov       bl, [edx + 2]	; Y1    
		movq      mm1, [Yuv2RgbAsm.lut.CoefficientsRGBY + 8 * eax] ; | BY0 | GY0 | RY0 | 0 |
		movq      mm2, [Yuv2RgbAsm.lut.CoefficientsRGBY + 8 * ebx] ; | BY1 | GY1 | RY1 | 0 |

		// Add together
		paddw     mm1, mm0	; | BY0 + BU | GY0 + GU + GV | RY0 + RV | 0 |
		paddw     mm2, mm0	; | BY1 + BU | GY1 + GU + GV | RY1 + RV | 0 |

		// Divide by 32 (look-up table is pre-multiplied by 32,
		// that's the max possible factor to still fit a signed word)
		psraw     mm1, 5
		psraw     mm2, 5
		
		// Pack signed word to unsigned byte clipping to 0 .. 255
		packuswb  mm1, mm2	; | B0 | G0 | R0 | 0 | B1 | G1 | R1 | 0 |

		// Move to destination RGB
		movq      [ebp], mm1; | B0 | G0 | R0 | 0 | B1 | G1 | R1 | 0 |

		// Increment pointers
		add       ebp, 8	; go to the next two destination RGB pixels
		add       edx, 4	; increment source

// End width loop
		pop       ebx		; pop width countdown
		sub       ebx, 2	; decrement it by 2 because we just processed two horizontal pixels
		jnz       wloop		; 0 -> we reached the end of line
		
		// Decrement by 2x lines for RGB
		lea       ebx, [8 * ecx] ; ebx is now 8 * width
		sub       ebp, ebx

// End height loop
		pop       ebx		; pop height countdown
		sub       ebx, 1	; decrement it
		jnz       hloop		; 0 -> we reached the end of image
		
		// Clean-up
		emms
		popad
		ret
	}
}