#include "stdafx.h"
#include "YuvToRgb.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static int g_crv_tab[256];
static int g_cbu_tab[256];
static int g_cgu_tab[256];
static int g_cgv_tab[256];
static int g_y_tab[256];
static unsigned char g_clip[1024];

#ifdef YUVTORGB_SPEEDTEST_VERSIONS
static unsigned char g_y_clip[256];
static unsigned char g_c_clip[256];
static unsigned char g_YuvToG[32768];
static unsigned char g_YuToB[1024];
static unsigned char g_YvToR[1024];
#endif

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

#ifdef YUVTORGB_SPEEDTEST_VERSIONS
	// Y Clip
	for (i = 0 ; i <= 16 ; i++)
		g_y_clip[i] = 16;
	for (i = 17 ; i <= 235 ; i++)
		g_y_clip[i] = i;
	for (i = 236 ; i <= 255 ; i++)
		g_y_clip[i] = 235;

	// C Clip
	for (i = 0 ; i <= 16 ; i++)
		g_c_clip[i] = 16;
	for (i = 17 ; i <= 240 ; i++)
		g_c_clip[i] = i;
	for (i = 241 ; i <= 255 ; i++)
		g_c_clip[i] = 240;

	int y, u, v;   
	for (i = 0 ; i < 1024 ; i++)
	{
		y = (i & 0x1F) << 3;
		u = (i & 0x3E0) >> 2;

		y = g_y_tab[y];
		int rv = g_crv_tab[u];
		int bu = g_cbu_tab[u];

		g_YuToB[i] = g_clip[384+((y + bu)>>16)];
		g_YvToR[i] = g_clip[384+((y + rv)>>16)];
	}

	for (i = 0 ; i < 32768 ; i++)
	{
		y = (i & 0x1F) << 3;
		u = (i & 0x3E0) >> 2;
		v = (i & 0x7C00) >> 7;

		y = g_y_tab[y];
		int gu = g_cgu_tab[u];
		int gv = g_cgv_tab[v];

		g_YuvToG[i] = g_clip[384+((y - gu - gv)>>16)];
	}
#endif
}

bool YUVToRGB24(	DWORD dwFourCC,
					unsigned char *src,	// YUV format depending from dwFourCC
					unsigned char *dst,	// RGB24 Dib
					int width,
					int height)
{
	// Check
	if (!src || !dst || (width <= 0) || (height <= 0))
		return false;

	// Select the Right Decoder
	if (dwFourCC == FCC('YV12'))
	{
		YV12ToRGB24(	src,
						dst,
						width,
						height);
	}
	else if (	dwFourCC == FCC('I420') ||
				dwFourCC == FCC('IYUV'))
	{
		I420ToRGB24(	src,
						dst,
						width,
						height);
	}
	else if (dwFourCC == FCC('YVU9'))
	{
		YVU9ToRGB24(	src,
						dst,
						width,
						height);
	}
	else if (dwFourCC == FCC('YUV9'))
	{
		YUV9ToRGB24(	src,
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
		YUY2ToRGB24(	src,
						dst,
						width,
						height);
	}
	else if (	dwFourCC == FCC('UYVY') ||
				dwFourCC == FCC('Y422') ||
				dwFourCC == FCC('UYNV'))
	{
		UYVYToRGB24(	src,
						dst,
						width,
						height);
	}
	else if (dwFourCC == FCC('YVYU'))
	{
		YVYUToRGB24(	src,
						dst,
						width,
						height);
	}
	else if (dwFourCC == FCC('VCR1'))
	{
		VCR1ToRGB24(	src,
						dst,
						width,
						height);
	}
	else if (dwFourCC == FCC('CLJR'))
	{
		CLJRToRGB24(	src,
						dst,
						width,
						height);
	}
	else if (dwFourCC == FCC('cyuv'))
	{
		cyuvToRGB24(	src,
						dst,
						width,
						height);
	}
	else if (	dwFourCC == FCC('VIXL') ||
				dwFourCC == FCC('PIXL'))
	{
		VIXLToRGB24(	src,
						dst,
						width,
						height);
	}
	else if (dwFourCC == FCC('Y41P'))
	{
		Y41PToRGB24(	src,
						dst,
						width,
						height);
	}
	else if (dwFourCC == FCC('YV16'))
	{
		YV16ToRGB24(	src,
						dst,
						width,
						height);
	}
	else if (dwFourCC == FCC('Y42B'))
	{
		Y42BToRGB24(	src,
						dst,
						width,
						height);
	}
	else if (	dwFourCC == FCC('Y800') ||
				dwFourCC == FCC('Y8  ') ||
				dwFourCC == FCC('GREY'))
	{
		Y800ToRGB24(	src,
						dst,
						width,
						height);
	}
	else
		return false;

	return true;
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
		YV12ToRGB32(	src,
						dst,
						width,
						height);
	}
	else if (	dwFourCC == FCC('I420') ||
				dwFourCC == FCC('IYUV'))
	{
		I420ToRGB32(	src,
						dst,
						width,
						height);
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
		YUY2ToRGB32(	src,
						dst,
						width,
						height);
	}
	else if (	dwFourCC == FCC('UYVY') ||
				dwFourCC == FCC('Y422') ||
				dwFourCC == FCC('UYNV'))
	{
		UYVYToRGB32(	src,
						dst,
						width,
						height);
	}
	else if (dwFourCC == FCC('YVYU'))
	{
		YVYUToRGB32(	src,
						dst,
						width,
						height);
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

void YUVToRGB24(unsigned char *src0,	// Y Plane
				unsigned char *src1,	// U Plane
				unsigned char *src2,	// V Plane
				unsigned char *dst,		// RGB24 Dib
				int width,
				int height)
{
	int y1, y2, u, v; 
	unsigned char *py1,*py2;
	int rv, gu, gv, bu;
	unsigned char *d1, *d2;

	py1 = src0;
	py2 = py1+width;

	int nDWAlignedLineSize = DWALIGNEDWIDTHBYTES(width * 24);

 	for (int j = 0 ; j < height ; j += 2)
	{
		d1 = dst + j * nDWAlignedLineSize;
		d2 = d1 + nDWAlignedLineSize;
		for (int i = 0 ; i < width ; i += 2)
		{
			u = *src1++;
			v = *src2++;

			rv = g_crv_tab[v];
			gu = g_cgu_tab[u];
			gv = g_cgv_tab[v];
			bu = g_cbu_tab[u];

			// up-left
            y1 = g_y_tab[*py1++];	
			*d1++ = g_clip[384+((y1 + bu)>>16)];		// Blue 
			*d1++ = g_clip[384+((y1 - gu - gv)>>16)];	// Green
            *d1++ = g_clip[384+((y1 + rv)>>16)];		// Red 

			// down-left
			y2 = g_y_tab[*py2++];
			*d2++ = g_clip[384+((y2 + bu)>>16)];		// Blue
			*d2++ = g_clip[384+((y2 - gu - gv)>>16)];	// Green
            *d2++ = g_clip[384+((y2 + rv)>>16)];		// Red

			// up-right
			y1 = g_y_tab[*py1++];
			*d1++ = g_clip[384+((y1 + bu)>>16)];		// Blue
			*d1++ = g_clip[384+((y1 - gu - gv)>>16)];	// Green
			*d1++ = g_clip[384+((y1 + rv)>>16)];		// Red

			// down-right
			y2 = g_y_tab[*py2++];
			*d2++ = g_clip[384+((y2 + bu)>>16)];		// Blue
			*d2++ = g_clip[384+((y2 - gu - gv)>>16)];	// Green
            *d2++ = g_clip[384+((y2 + rv)>>16)];		// Red
		}
		py1 += width;
		py2 += width;
	}
}

void YUVToRGB32(unsigned char *src0,	// Y Plane
				unsigned char *src1,	// U Plane
				unsigned char *src2,	// V Plane
				unsigned char *dst,		// RGB32 Dib
				int width,
				int height)
{
	int y1, y2, u, v; 
	unsigned char *py1,*py2;
	int rv, gu, gv, bu;
	unsigned char *d1, *d2;

	py1 = src0;
	py2 = py1+width;

	int nDWAlignedLineSize = width << 2;

 	for (int j = 0 ; j < height ; j += 2)
	{
		d1 = dst + j * nDWAlignedLineSize;
		d2 = d1 + nDWAlignedLineSize;
		for (int i = 0 ; i < width ; i += 2)
		{
			u = *src1++;
			v = *src2++;

			rv = g_crv_tab[v];
			gu = g_cgu_tab[u];
			gv = g_cgv_tab[v];
			bu = g_cbu_tab[u];

			// up-left
            y1 = g_y_tab[*py1++];	
			*d1++ = g_clip[384+((y1 + bu)>>16)];		// Blue 
			*d1++ = g_clip[384+((y1 - gu - gv)>>16)];	// Green
            *d1++ = g_clip[384+((y1 + rv)>>16)];		// Red 
			*d1++ = 0;									// Alpha

			// down-left
			y2 = g_y_tab[*py2++];
			*d2++ = g_clip[384+((y2 + bu)>>16)];		// Blue
			*d2++ = g_clip[384+((y2 - gu - gv)>>16)];	// Green
            *d2++ = g_clip[384+((y2 + rv)>>16)];		// Red
			*d2++ = 0;									// Alpha

			// up-right
			y1 = g_y_tab[*py1++];
			*d1++ = g_clip[384+((y1 + bu)>>16)];		// Blue
			*d1++ = g_clip[384+((y1 - gu - gv)>>16)];	// Green
			*d1++ = g_clip[384+((y1 + rv)>>16)];		// Red
			*d1++ = 0;									// Alpha

			// down-right
			y2 = g_y_tab[*py2++];
			*d2++ = g_clip[384+((y2 + bu)>>16)];		// Blue
			*d2++ = g_clip[384+((y2 - gu - gv)>>16)];	// Green
            *d2++ = g_clip[384+((y2 + rv)>>16)];		// Red
			*d2++ = 0;									// Alpha
		}
		py1 += width;
		py2 += width;
	}
}

void YUVToRGB24Flip(unsigned char *src0,	// Y Plane
					unsigned char *src1,	// U Plane
					unsigned char *src2,	// V Plane
					unsigned char *dst,		// RGB24 Dib
					int width,
					int height)
{
	int y1, y2, u, v; 
	unsigned char *py1,*py2;
	int rv, gu, gv, bu;
	unsigned char *d1, *d2;

	py1 = src0;
	py2 = py1+width;

	int nDWAlignedLineSize = DWALIGNEDWIDTHBYTES(width * 24);

 	for (int j = (height - 1) ; j > 0 ; j -= 2)
	{
		d1 = dst + j * nDWAlignedLineSize;
		d2 = d1 - nDWAlignedLineSize;
		for (int i = 0 ; i < width ; i += 2)
		{
			u = *src1++;
			v = *src2++;

			rv = g_crv_tab[v];
			gu = g_cgu_tab[u];
			gv = g_cgv_tab[v];
			bu = g_cbu_tab[u];

			// up-left
            y1 = g_y_tab[*py1++];	
			*d1++ = g_clip[384+((y1 + bu)>>16)];		// Blue 
			*d1++ = g_clip[384+((y1 - gu - gv)>>16)];	// Green
            *d1++ = g_clip[384+((y1 + rv)>>16)];		// Red 

			// down-left
			y2 = g_y_tab[*py2++];
			*d2++ = g_clip[384+((y2 + bu)>>16)];		// Blue
			*d2++ = g_clip[384+((y2 - gu - gv)>>16)];	// Green
            *d2++ = g_clip[384+((y2 + rv)>>16)];		// Red

			// up-right
			y1 = g_y_tab[*py1++];
			*d1++ = g_clip[384+((y1 + bu)>>16)];		// Blue
			*d1++ = g_clip[384+((y1 - gu - gv)>>16)];	// Green
			*d1++ = g_clip[384+((y1 + rv)>>16)];		// Red

			// down-right
			y2 = g_y_tab[*py2++];
			*d2++ = g_clip[384+((y2 + bu)>>16)];		// Blue
			*d2++ = g_clip[384+((y2 - gu - gv)>>16)];	// Green
            *d2++ = g_clip[384+((y2 + rv)>>16)];		// Red
		}
		py1 += width;
		py2 += width;
	}
}

void YUVToRGB32Flip(unsigned char *src0,	// Y Plane
					unsigned char *src1,	// U Plane
					unsigned char *src2,	// V Plane
					unsigned char *dst,		// RGB32 Dib
					int width,
					int height)
{
	int y1, y2, u, v; 
	unsigned char *py1,*py2;
	int rv, gu, gv, bu;
	unsigned char *d1, *d2;

	py1 = src0;
	py2 = py1+width;

	int nDWAlignedLineSize = width << 2;

 	for (int j = (height - 1) ; j > 0 ; j -= 2)
	{
		d1 = dst + j * nDWAlignedLineSize;
		d2 = d1 - nDWAlignedLineSize;
		for (int i = 0 ; i < width ; i += 2)
		{
			u = *src1++;
			v = *src2++;

			rv = g_crv_tab[v];
			gu = g_cgu_tab[u];
			gv = g_cgv_tab[v];
			bu = g_cbu_tab[u];

			// up-left
            y1 = g_y_tab[*py1++];	
			*d1++ = g_clip[384+((y1 + bu)>>16)];		// Blue 
			*d1++ = g_clip[384+((y1 - gu - gv)>>16)];	// Green
            *d1++ = g_clip[384+((y1 + rv)>>16)];		// Red
			*d1++ = 0;									// Alpha

			// down-left
			y2 = g_y_tab[*py2++];
			*d2++ = g_clip[384+((y2 + bu)>>16)];		// Blue
			*d2++ = g_clip[384+((y2 - gu - gv)>>16)];	// Green
            *d2++ = g_clip[384+((y2 + rv)>>16)];		// Red
			*d2++ = 0;									// Alpha

			// up-right
			y1 = g_y_tab[*py1++];
			*d1++ = g_clip[384+((y1 + bu)>>16)];		// Blue
			*d1++ = g_clip[384+((y1 - gu - gv)>>16)];	// Green
			*d1++ = g_clip[384+((y1 + rv)>>16)];		// Red
			*d1++ = 0;									// Alpha

			// down-right
			y2 = g_y_tab[*py2++];
			*d2++ = g_clip[384+((y2 + bu)>>16)];		// Blue
			*d2++ = g_clip[384+((y2 - gu - gv)>>16)];	// Green
            *d2++ = g_clip[384+((y2 + rv)>>16)];		// Red
			*d2++ = 0;									// Alpha
		}
		py1 += width;
		py2 += width;
	}
}

void YV16ToRGB24(	unsigned char *src,	// Y Plane, V Plane and U Plane
					unsigned char *dst,	// RGB24 Dib
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

	int nDWAlignedLineSize = DWALIGNEDWIDTHBYTES(width * 24);
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

			// up-right
			y2 = g_y_tab[*src++];
			*dst++ = g_clip[384+((y2 + bu)>>16)];		// Blue
			*dst++ = g_clip[384+((y2 - gu - gv)>>16)];	// Green
			*dst++ = g_clip[384+((y2 + rv)>>16)];		// Red
		}
		dst = dst - nDWAlignedLineSize - width * 3;
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

void Y42BToRGB24(	unsigned char *src,	// Y Plane, U Plane and V Plane
					unsigned char *dst,	// RGB24 Dib
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

	int nDWAlignedLineSize = DWALIGNEDWIDTHBYTES(width * 24);
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

			// up-right
			y2 = g_y_tab[*src++];
			*dst++ = g_clip[384+((y2 + bu)>>16)];		// Blue
			*dst++ = g_clip[384+((y2 - gu - gv)>>16)];	// Green
			*dst++ = g_clip[384+((y2 + rv)>>16)];		// Red
		}
		dst = dst - nDWAlignedLineSize - width * 3;
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

void Y800ToRGB24(	unsigned char *src,	// Y Plane
					unsigned char *dst,	// RGB24 Dib
					int width,
					int height)
{
	int nDWAlignedLineSize = DWALIGNEDWIDTHBYTES(width * 24);
	dst = dst + (nDWAlignedLineSize * (height - 1));
 	for (int j = 0 ; j < height ; j++)
	{
		for (int i = 0 ; i < width ; i++)
		{
            int y = g_y_tab[*src++];
			y >>= 16;
			unsigned char rgb = g_clip[384+y];
			*dst++ = rgb;	// Blue 
			*dst++ = rgb;	// Green
            *dst++ = rgb;	// Red
		}
		dst = dst - nDWAlignedLineSize - width * 3;
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

void YVU9ToRGB24(	unsigned char *src,	// Y Plane, V Plane and U Plane
					unsigned char *dst,	// RGB24 Dib
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

	int nDWAlignedLineSize = DWALIGNEDWIDTHBYTES(width * 24);

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

			// Pixel (1,0)
			y1 = g_y_tab[*py1++];
			*d1++ = g_clip[384+((y1 + bu)>>16)];		// Blue
			*d1++ = g_clip[384+((y1 - gu - gv)>>16)];	// Green
			*d1++ = g_clip[384+((y1 + rv)>>16)];		// Red

			// Pixel (2,0)
			y1 = g_y_tab[*py1++];
			*d1++ = g_clip[384+((y1 + bu)>>16)];		// Blue
			*d1++ = g_clip[384+((y1 - gu - gv)>>16)];	// Green
			*d1++ = g_clip[384+((y1 + rv)>>16)];		// Red

			// Pixel (3,0)
			y1 = g_y_tab[*py1++];
			*d1++ = g_clip[384+((y1 + bu)>>16)];		// Blue
			*d1++ = g_clip[384+((y1 - gu - gv)>>16)];	// Green
			*d1++ = g_clip[384+((y1 + rv)>>16)];		// Red

			// Pixel (0,1)
			y2 = g_y_tab[*py2++];
			*d2++ = g_clip[384+((y2 + bu)>>16)];		// Blue
			*d2++ = g_clip[384+((y2 - gu - gv)>>16)];	// Green
            *d2++ = g_clip[384+((y2 + rv)>>16)];		// Red

			// Pixel (1,1)
			y2 = g_y_tab[*py2++];
			*d2++ = g_clip[384+((y2 + bu)>>16)];		// Blue
			*d2++ = g_clip[384+((y2 - gu - gv)>>16)];	// Green
            *d2++ = g_clip[384+((y2 + rv)>>16)];		// Red

			// Pixel (2,1)
			y2 = g_y_tab[*py2++];
			*d2++ = g_clip[384+((y2 + bu)>>16)];		// Blue
			*d2++ = g_clip[384+((y2 - gu - gv)>>16)];	// Green
            *d2++ = g_clip[384+((y2 + rv)>>16)];		// Red

			// Pixel (3,1)
			y2 = g_y_tab[*py2++];
			*d2++ = g_clip[384+((y2 + bu)>>16)];		// Blue
			*d2++ = g_clip[384+((y2 - gu - gv)>>16)];	// Green
            *d2++ = g_clip[384+((y2 + rv)>>16)];		// Red

			// Pixel (0,2)
			y3 = g_y_tab[*py3++];
			*d3++ = g_clip[384+((y3 + bu)>>16)];		// Blue
			*d3++ = g_clip[384+((y3 - gu - gv)>>16)];	// Green
            *d3++ = g_clip[384+((y3 + rv)>>16)];		// Red

			// Pixel (1,2)
			y3 = g_y_tab[*py3++];
			*d3++ = g_clip[384+((y3 + bu)>>16)];		// Blue
			*d3++ = g_clip[384+((y3 - gu - gv)>>16)];	// Green
            *d3++ = g_clip[384+((y3 + rv)>>16)];		// Red

			// Pixel (2,2)
			y3 = g_y_tab[*py3++];
			*d3++ = g_clip[384+((y3 + bu)>>16)];		// Blue
			*d3++ = g_clip[384+((y3 - gu - gv)>>16)];	// Green
            *d3++ = g_clip[384+((y3 + rv)>>16)];		// Red

			// Pixel (3,2)
			y3 = g_y_tab[*py3++];
			*d3++ = g_clip[384+((y3 + bu)>>16)];		// Blue
			*d3++ = g_clip[384+((y3 - gu - gv)>>16)];	// Green
            *d3++ = g_clip[384+((y3 + rv)>>16)];		// Red

			// Pixel (0,3)
			y4 = g_y_tab[*py4++];
			*d4++ = g_clip[384+((y4 + bu)>>16)];		// Blue
			*d4++ = g_clip[384+((y4 - gu - gv)>>16)];	// Green
            *d4++ = g_clip[384+((y4 + rv)>>16)];		// Red

			// Pixel (1,3)
			y4 = g_y_tab[*py4++];
			*d4++ = g_clip[384+((y4 + bu)>>16)];		// Blue
			*d4++ = g_clip[384+((y4 - gu - gv)>>16)];	// Green
            *d4++ = g_clip[384+((y4 + rv)>>16)];		// Red

			// Pixel (2,3)
			y4 = g_y_tab[*py4++];
			*d4++ = g_clip[384+((y4 + bu)>>16)];		// Blue
			*d4++ = g_clip[384+((y4 - gu - gv)>>16)];	// Green
            *d4++ = g_clip[384+((y4 + rv)>>16)];		// Red

			// Pixel (3,3)
			y4 = g_y_tab[*py4++];
			*d4++ = g_clip[384+((y4 + bu)>>16)];		// Blue
			*d4++ = g_clip[384+((y4 - gu - gv)>>16)];	// Green
            *d4++ = g_clip[384+((y4 + rv)>>16)];		// Red
		}
		py1 += 3*width;
		py2 += 3*width;
		py3 += 3*width;
		py4 += 3*width;
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

void YUV9ToRGB24(	unsigned char *src,	// Y Plane, U Plane and V Plane
					unsigned char *dst,	// RGB24 Dib
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

	int nDWAlignedLineSize = DWALIGNEDWIDTHBYTES(width * 24);

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

			// Pixel (1,0)
			y1 = g_y_tab[*py1++];
			*d1++ = g_clip[384+((y1 + bu)>>16)];		// Blue
			*d1++ = g_clip[384+((y1 - gu - gv)>>16)];	// Green
			*d1++ = g_clip[384+((y1 + rv)>>16)];		// Red

			// Pixel (2,0)
			y1 = g_y_tab[*py1++];
			*d1++ = g_clip[384+((y1 + bu)>>16)];		// Blue
			*d1++ = g_clip[384+((y1 - gu - gv)>>16)];	// Green
			*d1++ = g_clip[384+((y1 + rv)>>16)];		// Red

			// Pixel (3,0)
			y1 = g_y_tab[*py1++];
			*d1++ = g_clip[384+((y1 + bu)>>16)];		// Blue
			*d1++ = g_clip[384+((y1 - gu - gv)>>16)];	// Green
			*d1++ = g_clip[384+((y1 + rv)>>16)];		// Red

			// Pixel (0,1)
			y2 = g_y_tab[*py2++];
			*d2++ = g_clip[384+((y2 + bu)>>16)];		// Blue
			*d2++ = g_clip[384+((y2 - gu - gv)>>16)];	// Green
            *d2++ = g_clip[384+((y2 + rv)>>16)];		// Red

			// Pixel (1,1)
			y2 = g_y_tab[*py2++];
			*d2++ = g_clip[384+((y2 + bu)>>16)];		// Blue
			*d2++ = g_clip[384+((y2 - gu - gv)>>16)];	// Green
            *d2++ = g_clip[384+((y2 + rv)>>16)];		// Red

			// Pixel (2,1)
			y2 = g_y_tab[*py2++];
			*d2++ = g_clip[384+((y2 + bu)>>16)];		// Blue
			*d2++ = g_clip[384+((y2 - gu - gv)>>16)];	// Green
            *d2++ = g_clip[384+((y2 + rv)>>16)];		// Red

			// Pixel (3,1)
			y2 = g_y_tab[*py2++];
			*d2++ = g_clip[384+((y2 + bu)>>16)];		// Blue
			*d2++ = g_clip[384+((y2 - gu - gv)>>16)];	// Green
            *d2++ = g_clip[384+((y2 + rv)>>16)];		// Red

			// Pixel (0,2)
			y3 = g_y_tab[*py3++];
			*d3++ = g_clip[384+((y3 + bu)>>16)];		// Blue
			*d3++ = g_clip[384+((y3 - gu - gv)>>16)];	// Green
            *d3++ = g_clip[384+((y3 + rv)>>16)];		// Red

			// Pixel (1,2)
			y3 = g_y_tab[*py3++];
			*d3++ = g_clip[384+((y3 + bu)>>16)];		// Blue
			*d3++ = g_clip[384+((y3 - gu - gv)>>16)];	// Green
            *d3++ = g_clip[384+((y3 + rv)>>16)];		// Red

			// Pixel (2,2)
			y3 = g_y_tab[*py3++];
			*d3++ = g_clip[384+((y3 + bu)>>16)];		// Blue
			*d3++ = g_clip[384+((y3 - gu - gv)>>16)];	// Green
            *d3++ = g_clip[384+((y3 + rv)>>16)];		// Red

			// Pixel (3,2)
			y3 = g_y_tab[*py3++];
			*d3++ = g_clip[384+((y3 + bu)>>16)];		// Blue
			*d3++ = g_clip[384+((y3 - gu - gv)>>16)];	// Green
            *d3++ = g_clip[384+((y3 + rv)>>16)];		// Red

			// Pixel (0,3)
			y4 = g_y_tab[*py4++];
			*d4++ = g_clip[384+((y4 + bu)>>16)];		// Blue
			*d4++ = g_clip[384+((y4 - gu - gv)>>16)];	// Green
            *d4++ = g_clip[384+((y4 + rv)>>16)];		// Red

			// Pixel (1,3)
			y4 = g_y_tab[*py4++];
			*d4++ = g_clip[384+((y4 + bu)>>16)];		// Blue
			*d4++ = g_clip[384+((y4 - gu - gv)>>16)];	// Green
            *d4++ = g_clip[384+((y4 + rv)>>16)];		// Red

			// Pixel (2,3)
			y4 = g_y_tab[*py4++];
			*d4++ = g_clip[384+((y4 + bu)>>16)];		// Blue
			*d4++ = g_clip[384+((y4 - gu - gv)>>16)];	// Green
            *d4++ = g_clip[384+((y4 + rv)>>16)];		// Red

			// Pixel (3,3)
			y4 = g_y_tab[*py4++];
			*d4++ = g_clip[384+((y4 + bu)>>16)];		// Blue
			*d4++ = g_clip[384+((y4 - gu - gv)>>16)];	// Green
            *d4++ = g_clip[384+((y4 + rv)>>16)];		// Red
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
void YUY2ToRGB24(	unsigned char *src,	// Y0 U0 Y1 V0, Y2 U2 Y3 V2, ...
					unsigned char *dst,	// RGB24 Dib
					int width,
					int height)
{
	int nDWAlignedLineSize = DWALIGNEDWIDTHBYTES(width * 24);
	dst = dst + (nDWAlignedLineSize * (height - 1));
	for (int j = 0 ; j < height ; j++)
	{
		for (int i = 0 ; i < width ; i += 2)
		{
			int y0 = *src++;
			int u  = *src++;
			int y1 = *src++;
			int v  = *src++;

			int rv = g_crv_tab[v];
			int gu = g_cgu_tab[u];
			int gv = g_cgv_tab[v];
			int bu = g_cbu_tab[u];

			// First Pixel
            y0 = g_y_tab[y0];	
			*dst++ = g_clip[384+((y0 + bu)>>16)];		// Blue 
			*dst++ = g_clip[384+((y0 - gu - gv)>>16)];	// Green
            *dst++ = g_clip[384+((y0 + rv)>>16)];		// Red 

			// Second Pixel
			y1 = g_y_tab[y1];
			*dst++ = g_clip[384+((y1 + bu)>>16)];		// Blue
			*dst++ = g_clip[384+((y1 - gu - gv)>>16)];	// Green
			*dst++ = g_clip[384+((y1 + rv)>>16)];		// Red
		}
		dst = dst - nDWAlignedLineSize - width * 3;
	}
}

// Equivalent FCCs Are: YUNV, VYUY, V422 and YUYV
void YUY2ToRGB32(	unsigned char *src,	// Y0 U0 Y1 V0, Y2 U2 Y3 V2, ...
					unsigned char *dst,	// RGB32 Dib
					int width,
					int height)
{
	int nDWAlignedLineSize = width << 2;
	int n2DWAlignedLineSize = width << 3;
	dst = dst + (nDWAlignedLineSize * (height - 1));
	for (int j = 0 ; j < height ; j++)
	{
		for (int i = 0 ; i < width ; i += 2)
		{
			int y0 = *src++;
			int u  = *src++;
			int y1 = *src++;
			int v  = *src++;

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
		}
		dst -= n2DWAlignedLineSize;
	}
}

// Equivalent FCCs Are: Y422 and UYNV 
void UYVYToRGB24(	unsigned char *src,	// U0 Y0 V0 Y1, U2 Y2 V2 Y3, ...
					unsigned char *dst,	// RGB24 Dib
					int width,
					int height)
{
	int nDWAlignedLineSize = DWALIGNEDWIDTHBYTES(width * 24);
	dst = dst + (nDWAlignedLineSize * (height - 1));
	for (int j = 0 ; j < height ; j++)
	{
		for (int i = 0 ; i < width ; i += 2)
		{
			int u  = *src++;
			int y0 = *src++;
			int v  = *src++;
			int y1 = *src++;

			int rv = g_crv_tab[v];
			int gu = g_cgu_tab[u];
			int gv = g_cgv_tab[v];
			int bu = g_cbu_tab[u];

			// First Pixel
            y0 = g_y_tab[y0];	
			*dst++ = g_clip[384+((y0 + bu)>>16)];		// Blue 
			*dst++ = g_clip[384+((y0 - gu - gv)>>16)];	// Green
            *dst++ = g_clip[384+((y0 + rv)>>16)];		// Red 

			// Second Pixel
			y1 = g_y_tab[y1];
			*dst++ = g_clip[384+((y1 + bu)>>16)];		// Blue
			*dst++ = g_clip[384+((y1 - gu - gv)>>16)];	// Green
			*dst++ = g_clip[384+((y1 + rv)>>16)];		// Red
		}
		dst = dst - nDWAlignedLineSize - width * 3;
	}
}

// Equivalent FCCs Are: Y422 and UYNV 
void UYVYToRGB32(	unsigned char *src,	// U0 Y0 V0 Y1, U2 Y2 V2 Y3, ...
					unsigned char *dst,	// RGB32 Dib
					int width,
					int height)
{
	int nDWAlignedLineSize = width << 2;
	int n2DWAlignedLineSize = width << 3;
	dst = dst + (nDWAlignedLineSize * (height - 1));
	for (int j = 0 ; j < height ; j++)
	{
		for (int i = 0 ; i < width ; i += 2)
		{
			int u  = *src++;
			int y0 = *src++;
			int v  = *src++;
			int y1 = *src++;

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
		}
		dst -= n2DWAlignedLineSize;
	}
}

void YVYUToRGB24(	unsigned char *src,	// Y0 V0 Y1 U0, Y2 V2 Y3 U2, ...
					unsigned char *dst,	// RGB24 Dib
					int width,
					int height)
{
	int nDWAlignedLineSize = DWALIGNEDWIDTHBYTES(width * 24);
	dst = dst + (nDWAlignedLineSize * (height - 1));
	for (int j = 0 ; j < height ; j++)
	{
		for (int i = 0 ; i < width ; i += 2)
		{
			int y0 = *src++;
			int v  = *src++;
			int y1 = *src++;
			int u  = *src++;

			int rv = g_crv_tab[v];
			int gu = g_cgu_tab[u];
			int gv = g_cgv_tab[v];
			int bu = g_cbu_tab[u];

			// First Pixel
            y0 = g_y_tab[y0];	
			*dst++ = g_clip[384+((y0 + bu)>>16)];		// Blue 
			*dst++ = g_clip[384+((y0 - gu - gv)>>16)];	// Green
            *dst++ = g_clip[384+((y0 + rv)>>16)];		// Red 

			// Second Pixel
			y1 = g_y_tab[y1];
			*dst++ = g_clip[384+((y1 + bu)>>16)];		// Blue
			*dst++ = g_clip[384+((y1 - gu - gv)>>16)];	// Green
			*dst++ = g_clip[384+((y1 + rv)>>16)];		// Red
		}
		dst = dst - nDWAlignedLineSize - width * 3;
	}
}

void YVYUToRGB32(	unsigned char *src,	// Y0 V0 Y1 U0, Y2 V2 Y3 U2, ...
					unsigned char *dst,	// RGB32 Dib
					int width,
					int height)
{
	int nDWAlignedLineSize = width << 2;
	int n2DWAlignedLineSize = width << 3;
	dst = dst + (nDWAlignedLineSize * (height - 1));
	for (int j = 0 ; j < height ; j++)
	{
		for (int i = 0 ; i < width ; i += 2)
		{
			int y0 = *src++;
			int v  = *src++;
			int y1 = *src++;
			int u  = *src++;

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
		}
		dst -= n2DWAlignedLineSize;
	}
}

void Y41PToRGB24(	unsigned char *src,	// U0 Y0 V0 Y1 U4 Y2 V4 Y3 Y4 Y5 Y6 Y7
					unsigned char *dst,	// RGB24 Dib
					int width,
					int height)
{
	int nDWAlignedLineSize = DWALIGNEDWIDTHBYTES(width * 24);
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

			// Second Pixel
            y1 = g_y_tab[y1];	
			*dst++ = g_clip[384+((y1 + bu0)>>16)];			// Blue 
			*dst++ = g_clip[384+((y1 - gu0 - gv0)>>16)];	// Green
            *dst++ = g_clip[384+((y1 + rv0)>>16)];			// Red 

			// Third Pixel
            y2 = g_y_tab[y2];	
			*dst++ = g_clip[384+((y2 + bu0)>>16)];			// Blue 
			*dst++ = g_clip[384+((y2 - gu0 - gv0)>>16)];	// Green
            *dst++ = g_clip[384+((y2 + rv0)>>16)];			// Red 

			// Fourth Pixel
			y3 = g_y_tab[y3];
			*dst++ = g_clip[384+((y3 + bu0)>>16)];			// Blue
			*dst++ = g_clip[384+((y3 - gu0 - gv0)>>16)];	// Green
			*dst++ = g_clip[384+((y3 + rv0)>>16)];			// Red

			// Fifth Pixel
            y4 = g_y_tab[y4];	
			*dst++ = g_clip[384+((y4 + bu4)>>16)];			// Blue 
			*dst++ = g_clip[384+((y4 - gu4 - gv4)>>16)];	// Green
            *dst++ = g_clip[384+((y4 + rv4)>>16)];			// Red 

			// Sixth Pixel
            y5 = g_y_tab[y5];	
			*dst++ = g_clip[384+((y5 + bu4)>>16)];			// Blue 
			*dst++ = g_clip[384+((y5 - gu4 - gv4)>>16)];	// Green
            *dst++ = g_clip[384+((y5 + rv4)>>16)];			// Red 

			// Seventh Pixel
            y6 = g_y_tab[y6];	
			*dst++ = g_clip[384+((y6 + bu4)>>16)];			// Blue 
			*dst++ = g_clip[384+((y6 - gu4 - gv4)>>16)];	// Green
            *dst++ = g_clip[384+((y6 + rv4)>>16)];			// Red 

			// Eighth Pixel
			y7 = g_y_tab[y7];
			*dst++ = g_clip[384+((y7 + bu4)>>16)];			// Blue
			*dst++ = g_clip[384+((y7 - gu4 - gv4)>>16)];	// Green
			*dst++ = g_clip[384+((y7 + rv4)>>16)];			// Red
		}
		dst = dst - nDWAlignedLineSize - width * 3;
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
void VCR1ToRGB24(	unsigned char *src,
					unsigned char *dst,	// RGB24 Dib
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

	int nDWAlignedLineSize = DWALIGNEDWIDTHBYTES(width * 24);

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

			// Pixel (1,0)
			y10 = g_y_tab[y10];
			*d0++ = g_clip[384+((y10 + bu)>>16)];		// Blue
			*d0++ = g_clip[384+((y10 - gu - gv)>>16)];	// Green
			*d0++ = g_clip[384+((y10 + rv)>>16)];		// Red

			// Pixel (2,0)
			y20 = g_y_tab[y20];
			*d0++ = g_clip[384+((y20 + bu)>>16)];		// Blue
			*d0++ = g_clip[384+((y20 - gu - gv)>>16)];	// Green
			*d0++ = g_clip[384+((y20 + rv)>>16)];		// Red

			// Pixel (3,0)
			y30 = g_y_tab[y30];
			*d0++ = g_clip[384+((y30 + bu)>>16)];		// Blue
			*d0++ = g_clip[384+((y30 - gu - gv)>>16)];	// Green
			*d0++ = g_clip[384+((y30 + rv)>>16)];		// Red

			// Pixel (0,1)
            y01 = g_y_tab[y01];	
			*d1++ = g_clip[384+((y01 + bu)>>16)];		// Blue 
			*d1++ = g_clip[384+((y01 - gu - gv)>>16)];	// Green
            *d1++ = g_clip[384+((y01 + rv)>>16)];		// Red 

			// Pixel (1,1)
			y11 = g_y_tab[y11];
			*d1++ = g_clip[384+((y11 + bu)>>16)];		// Blue
			*d1++ = g_clip[384+((y11 - gu - gv)>>16)];	// Green
			*d1++ = g_clip[384+((y11 + rv)>>16)];		// Red

			// Pixel (2,1)
			y21 = g_y_tab[y21];
			*d1++ = g_clip[384+((y21 + bu)>>16)];		// Blue
			*d1++ = g_clip[384+((y21 - gu - gv)>>16)];	// Green
			*d1++ = g_clip[384+((y21 + rv)>>16)];		// Red

			// Pixel (3,1)
			y31 = g_y_tab[y31];
			*d1++ = g_clip[384+((y31 + bu)>>16)];		// Blue
			*d1++ = g_clip[384+((y31 - gu - gv)>>16)];	// Green
			*d1++ = g_clip[384+((y31 + rv)>>16)];		// Red

			// Pixel (0,2)
            y02 = g_y_tab[y02];	
			*d2++ = g_clip[384+((y02 + bu)>>16)];		// Blue 
			*d2++ = g_clip[384+((y02 - gu - gv)>>16)];	// Green
            *d2++ = g_clip[384+((y02 + rv)>>16)];		// Red 

			// Pixel (1,2)
			y12 = g_y_tab[y12];
			*d2++ = g_clip[384+((y12 + bu)>>16)];		// Blue
			*d2++ = g_clip[384+((y12 - gu - gv)>>16)];	// Green
			*d2++ = g_clip[384+((y12 + rv)>>16)];		// Red

			// Pixel (2,2)
			y22 = g_y_tab[y22];
			*d2++ = g_clip[384+((y22 + bu)>>16)];		// Blue
			*d2++ = g_clip[384+((y22 - gu - gv)>>16)];	// Green
			*d2++ = g_clip[384+((y22 + rv)>>16)];		// Red

			// Pixel (3,2)
			y32 = g_y_tab[y32];
			*d2++ = g_clip[384+((y32 + bu)>>16)];		// Blue
			*d2++ = g_clip[384+((y32 - gu - gv)>>16)];	// Green
			*d2++ = g_clip[384+((y32 + rv)>>16)];		// Red

			// Pixel (0,3)
            y03 = g_y_tab[y03];	
			*d3++ = g_clip[384+((y03 + bu)>>16)];		// Blue 
			*d3++ = g_clip[384+((y03 - gu - gv)>>16)];	// Green
            *d3++ = g_clip[384+((y03 + rv)>>16)];		// Red 

			// Pixel (1,3)
			y13 = g_y_tab[y13];
			*d3++ = g_clip[384+((y13 + bu)>>16)];		// Blue
			*d3++ = g_clip[384+((y13 - gu - gv)>>16)];	// Green
			*d3++ = g_clip[384+((y13 + rv)>>16)];		// Red

			// Pixel (2,3)
			y23 = g_y_tab[y23];
			*d3++ = g_clip[384+((y23 + bu)>>16)];		// Blue
			*d3++ = g_clip[384+((y23 - gu - gv)>>16)];	// Green
			*d3++ = g_clip[384+((y23 + rv)>>16)];		// Red

			// Pixel (3,3)
			y33 = g_y_tab[y33];
			*d3++ = g_clip[384+((y33 + bu)>>16)];		// Blue
			*d3++ = g_clip[384+((y33 - gu - gv)>>16)];	// Green
			*d3++ = g_clip[384+((y33 + rv)>>16)];		// Red
		}
		s0 += (3*width/2);
		s1 = s0+4+width;
		s2 = s1+width/2;
		s3 = s2+width/2;
	}
}

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
void CLJRToRGB24(	unsigned char *src,
					unsigned char *dst,	// RGB24 Dib
					int width,
					int height)
{
	DWORD dw;
	int nDWAlignedLineSize = DWALIGNEDWIDTHBYTES(width * 24);
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

			// Second Pixel
            y1 = g_y_tab[y1];	
			*dst++ = g_clip[384+((y1 + bu)>>16)];		// Blue 
			*dst++ = g_clip[384+((y1 - gu - gv)>>16)];	// Green
            *dst++ = g_clip[384+((y1 + rv)>>16)];		// Red 

			// Third Pixel
            y2 = g_y_tab[y2];	
			*dst++ = g_clip[384+((y2 + bu)>>16)];		// Blue 
			*dst++ = g_clip[384+((y2 - gu - gv)>>16)];	// Green
            *dst++ = g_clip[384+((y2 + rv)>>16)];		// Red 

			// Fourth Pixel
			y3 = g_y_tab[y3];
			*dst++ = g_clip[384+((y3 + bu)>>16)];		// Blue
			*dst++ = g_clip[384+((y3 - gu - gv)>>16)];	// Green
			*dst++ = g_clip[384+((y3 + rv)>>16)];		// Red
		}
		dst = dst - nDWAlignedLineSize - width * 3;
	}
}

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
void cyuvToRGB24(	unsigned char *src,
					unsigned char *dst,	// RGB24 Dib
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

	int nDWAlignedLineSize = DWALIGNEDWIDTHBYTES(width * 24);
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

			// Second Pixel
            y1 = g_y_tab[y1];	
			*dst++ = g_clip[384+((y1 + bu)>>16)];		// Blue 
			*dst++ = g_clip[384+((y1 - gu - gv)>>16)];	// Green
            *dst++ = g_clip[384+((y1 + rv)>>16)];		// Red 

			// Third Pixel
            y2 = g_y_tab[y2];	
			*dst++ = g_clip[384+((y2 + bu)>>16)];		// Blue 
			*dst++ = g_clip[384+((y2 - gu - gv)>>16)];	// Green
            *dst++ = g_clip[384+((y2 + rv)>>16)];		// Red 

			// Fourth Pixel
			y3 = g_y_tab[y3];
			*dst++ = g_clip[384+((y3 + bu)>>16)];		// Blue
			*dst++ = g_clip[384+((y3 - gu - gv)>>16)];	// Green
			*dst++ = g_clip[384+((y3 + rv)>>16)];		// Red
		}
		dst = dst - nDWAlignedLineSize - width * 3;
	}
}

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
void VIXLToRGB24(	unsigned char *src,
					unsigned char *dst,	// RGB24 Dib
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

	int nDWAlignedLineSize = DWALIGNEDWIDTHBYTES(width * 24);
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

			// Second Pixel
            y1 = g_y_tab[(y1<<1) & 0xFF];	
			*dst++ = g_clip[384+((y1 + bu)>>16)];		// Blue 
			*dst++ = g_clip[384+((y1 - gu - gv)>>16)];	// Green
            *dst++ = g_clip[384+((y1 + rv)>>16)];		// Red 

			// Third Pixel
            y2 = g_y_tab[(y2<<1) & 0xFF];	
			*dst++ = g_clip[384+((y2 + bu)>>16)];		// Blue 
			*dst++ = g_clip[384+((y2 - gu - gv)>>16)];	// Green
            *dst++ = g_clip[384+((y2 + rv)>>16)];		// Red 

			// Fourth Pixel
			y3 = g_y_tab[(y3<<1) & 0xFF];
			*dst++ = g_clip[384+((y3 + bu)>>16)];		// Blue
			*dst++ = g_clip[384+((y3 - gu - gv)>>16)];	// Green
			*dst++ = g_clip[384+((y3 + rv)>>16)];		// Red
		}
		src += width;
		dst = dst - nDWAlignedLineSize - width * 3;
	}
}

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

#ifdef YUVTORGB_SPEEDTEST_VERSIONS

// Slowest!
void YUY2ToRGB24Calc(	unsigned char *src,	// Y0 U0 Y1 V0, Y2 U2 Y3 V2, ...
						unsigned char *dst,	// RGB24 Dib
						int width,
						int height)
{
	int nDWAlignedLineSize = DWALIGNEDWIDTHBYTES(width * 24);
	dst = dst + (nDWAlignedLineSize * (height - 1));
	for (int j = 0 ; j < height ; j++)
	{
		for (int i = 0 ; i < width ; i += 2)
		{
			int y0 = *src++;
			int u  = *src++;
			int y1 = *src++;
			int v  = *src++;

			// First Pixel	
			*dst++ = MAX(0, MIN(((298 * (y0 - 16) + 516 * (u - 128) + 128) >> 8), 255));					// Blue 
			*dst++ = MAX(0, MIN(((298 * (y0 - 16) - 100 * (u - 128) - 208 * (v - 128) + 128) >> 8), 255));	// Green
            *dst++ = MAX(0, MIN(((298 * (y0 - 16) + 409 * (v - 128) + 128) >> 8), 255));					// Red 

			// Second Pixel
			*dst++ = MAX(0, MIN(((298 * (y1 - 16) + 516 * (u - 128) + 128) >> 8), 255));					// Blue 
			*dst++ = MAX(0, MIN(((298 * (y1 - 16) - 100 * (u - 128) - 208 * (v - 128) + 128) >> 8), 255));	// Green
            *dst++ = MAX(0, MIN(((298 * (y1 - 16) + 409 * (v - 128) + 128) >> 8), 255));					// Red 
		}
		dst = dst - nDWAlignedLineSize - width * 3;
	}
}

// This is Fastest but needs a big LUT and it has also a bad Quality!!
void YUY2ToRGB24BigLut(	unsigned char *src,	// Y0 U0 Y1 V0, Y2 U2 Y3 V2, ...
						unsigned char *dst,	// RGB24 Dib
						int width,
						int height)
{
	int nDWAlignedLineSize = DWALIGNEDWIDTHBYTES(width * 24);
	dst = dst + (nDWAlignedLineSize * (height - 1));
	for (int j = 0 ; j < height ; j++)
	{
		for (int i = 0 ; i < width ; i += 2)
		{
			int y0 = *src++;
			int u  = *src++;
			int y1 = *src++;
			int v  = *src++;

			// First Pixel
			*dst++ = g_YuToB[(y0 >> 3) | ((u >> 3) << 5)];						// Blue 
			*dst++ = g_YuvToG[(y0 >> 3) | ((u >> 3) << 5) | ((v >> 3) << 10)];	// Green
			*dst++ = g_YvToR[(y0 >> 3) | ((v >> 3) << 5)];						// Red 

			// Second Pixel
			*dst++ = g_YuToB[(y1 >> 3) | ((u >> 3) << 5)];						// Blue 
			*dst++ = g_YuvToG[(y1 >> 3) | ((u >> 3) << 5) | ((v >> 3) << 10)];	// Green
			*dst++ = g_YvToR[(y1 >> 3) | ((v >> 3) << 5)];						// Red
		}
		dst = dst - nDWAlignedLineSize - width * 3;
	}
}

#endif