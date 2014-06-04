#include "stdafx.h"
#include "YuvToYuv.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Defines
#define SCALEBITS 10
#define ONE_HALF  (1 << (SCALEBITS - 1))
#define FIX(x)    ((int) ((x) * (1<<SCALEBITS) + 0.5))

// JPEG Y range: [0,255]
static __forceinline int Y_ITU601_TO_JPEG(int y)
{
	y = (y * FIX(255.0/219.0) + (ONE_HALF - 16 * FIX(255.0/219.0))) >> SCALEBITS;
	if (y < 0)
		y = 0;
	else if (y > 255)
		y = 255;
	return y;
}

// ITU601 Y range: [16,235]
static __forceinline int Y_JPEG_TO_ITU601(int y)
{
	y = (y * FIX(219.0/255.0) + (ONE_HALF + (16 << SCALEBITS))) >> SCALEBITS;
	if (y < 16)
		y = 16;
	else if (y > 235)
		y = 235;
	return y;
}

// JPEG C range: [0,255]
static __forceinline int C_ITU601_TO_JPEG(int c)
{
	c = ((c - 128) * FIX(127.0/112.0) + (ONE_HALF + (128 << SCALEBITS))) >> SCALEBITS;
	if (c < 0)
		c = 0;
	else if (c > 255)
		c = 255;
	return c;
}

// ITU601 C range: [16,240]
static __forceinline int C_JPEG_TO_ITU601(int c)
{
	c = ((c - 128) * FIX(112.0/127.0) + (ONE_HALF + (128 << SCALEBITS))) >> SCALEBITS;
	if (c < 16)
		c = 16;
	else if (c > 240)
		c = 240;
	return c;
}

// C lut
static uint8_t y_itu601_to_jpeg[256];
static uint8_t y_jpeg_to_itu601[256];
static uint8_t c_itu601_to_jpeg[256];
static uint8_t c_jpeg_to_itu601[256];
#ifdef _DEBUG
static BOOL ITU601JPEGConvertTest(int width, int height);
#endif
void InitYUVToYUVTable()
{
	// Init tables
	for (int i = 0 ; i < 256 ; i++)
	{
		y_itu601_to_jpeg[i] = Y_ITU601_TO_JPEG(i);
		y_jpeg_to_itu601[i] = Y_JPEG_TO_ITU601(i);
		c_itu601_to_jpeg[i] = C_ITU601_TO_JPEG(i);
		c_jpeg_to_itu601[i] = C_JPEG_TO_ITU601(i);
	}

	// In DEBUG mode test all 8 conversion combinations
	ASSERT(ITU601JPEGConvertTest(640, 480));
}

static void YUVApplyTable(	const uint8_t* src, int srcStride,
							uint8_t* dst, int dstStride,
							int dst_plane_width, int dst_plane_height,
							const uint8_t* table)
{
	int w;
	const uint8_t* s;
	uint8_t* d;

	for ( ; dst_plane_height > 0 ; dst_plane_height--)
	{
		// Process line
		s = src;
		d = dst;
		for (w = dst_plane_width ; w >= 4 ; w -= 4)
		{
			d[0] = table[s[0]];
			d[1] = table[s[1]];
			d[2] = table[s[2]];
			d[3] = table[s[3]];
			s += 4;
			d += 4;
		}
		for ( ; w > 0 ; w--)
		{
			d[0] = table[s[0]];
			s++;
			d++;			
		}
		src += srcStride;
		dst += dstStride;
	}
}

static void YUV420PTo422PApplyTable(const uint8_t* src, int srcStride,
								uint8_t* dst, int dstStride,
								int dst_plane_width, int dst_plane_height,
								const uint8_t* table)
{
	int w;
	const uint8_t* s;
	uint8_t* d;

	for ( ; dst_plane_height > 0 ; dst_plane_height -= 2)
	{
		// Process line
		s = src;
		d = dst;
		for (w = dst_plane_width ; w >= 4 ; w -= 4)
		{
			d[0] = table[s[0]];
			d[1] = table[s[1]];
			d[2] = table[s[2]];
			d[3] = table[s[3]];
			s += 4;
			d += 4;
		}
		for ( ; w > 0 ; w--)
		{
			d[0] = table[s[0]];
			s++;
			d++;			
		}
		src += srcStride;
		dst += dstStride;

		// Duplicate the just processed line
		memcpy(dst, dst - dstStride, dst_plane_width);
		dst += dstStride;
	}
}

static void YUV422PTo420PApplyTable(const uint8_t *src, int srcStride,
									uint8_t *dst, int dstStride,
									int dst_plane_width, int dst_plane_height,
									const uint8_t* table)
{
	int w;
	const uint8_t* s1;
	const uint8_t* s2;
	uint8_t* d;

	for ( ; dst_plane_height > 0 ; dst_plane_height--)
	{
		// Process two source lines
		s1 = src;
		s2 = s1 + srcStride;
		d = dst;
		for (w = dst_plane_width ; w >= 4 ; w -= 4)
		{
			d[0] = table[(s1[0] + s2[0]) >> 1];
			d[1] = table[(s1[1] + s2[1]) >> 1];
			d[2] = table[(s1[2] + s2[2]) >> 1];
			d[3] = table[(s1[3] + s2[3]) >> 1];
			s1 += 4;
			s2 += 4;
			d += 4;
		}
		for ( ; w > 0 ; w--)
		{
			d[0] = table[(s1[0] + s2[0]) >> 1];
			s1++;
			s2++;
			d++;
		}
		src += 2 * srcStride;
		dst += dstStride;
	}
}

static __forceinline bool IsJPEGToITU601(enum AVPixelFormat src_pix_fmt, enum AVPixelFormat dst_pix_fmt)
{
	BOOL bSrcJPEG = (	src_pix_fmt == AV_PIX_FMT_YUVJ420P	||	// .log2_chroma_w = 1 , .log2_chroma_h = 1
						src_pix_fmt == AV_PIX_FMT_YUVJ422P);	// .log2_chroma_w = 1 , .log2_chroma_h = 0
						
	BOOL bDstITU601 = (	dst_pix_fmt == AV_PIX_FMT_YUV420P	||	// .log2_chroma_w = 1 , .log2_chroma_h = 1
						dst_pix_fmt == AV_PIX_FMT_YUV422P);		// .log2_chroma_w = 1 , .log2_chroma_h = 0

	return bSrcJPEG && bDstITU601;
}

static __forceinline bool IsITU601ToJPEG(enum AVPixelFormat src_pix_fmt, enum AVPixelFormat dst_pix_fmt)
{
	BOOL bSrcITU601 = (	src_pix_fmt == AV_PIX_FMT_YUV420P	||	// .log2_chroma_w = 1 , .log2_chroma_h = 1
						src_pix_fmt == AV_PIX_FMT_YUV422P);		// .log2_chroma_w = 1 , .log2_chroma_h = 0
						
	BOOL bDstJPEG = (	dst_pix_fmt == AV_PIX_FMT_YUVJ420P	||	// .log2_chroma_w = 1 , .log2_chroma_h = 1
						dst_pix_fmt == AV_PIX_FMT_YUVJ422P);	// .log2_chroma_w = 1 , .log2_chroma_h = 0

	return bSrcITU601 && bDstJPEG;
}

BOOL ITU601JPEGConvert(	enum AVPixelFormat src_pix_fmt,
						enum AVPixelFormat dst_pix_fmt,
						uint8_t* src[], int srcStride[],
						uint8_t* dst[], int dstStride[],
						int width, int height)
{
	int i;
	const AVPixFmtDescriptor* dst_pix_desc = av_pix_fmt_desc_get(dst_pix_fmt);

	// Check
	if (dst_pix_desc			&&
		src && dst				&&
		srcStride && dstStride	&&
		width > 0 && height > 0)
	{
		// JPEG -> ITU601
		if (IsJPEGToITU601(src_pix_fmt, dst_pix_fmt))
		{
			// Y plane
			YUVApplyTable(	src[0], srcStride[0],
							dst[0], dstStride[0],
							width,
							height,
							y_jpeg_to_itu601);

			// Chroma planes
			for (i = 1 ; i <= 2 ; i++)
			{
				// J420P -> 422P
				if (src_pix_fmt == AV_PIX_FMT_YUVJ420P && dst_pix_fmt == AV_PIX_FMT_YUV422P)
				{
					YUV420PTo422PApplyTable(src[i], srcStride[i],
											dst[i], dstStride[i],
											width >> dst_pix_desc->log2_chroma_w,
											height >> dst_pix_desc->log2_chroma_h,
											c_jpeg_to_itu601);
				}
				// J422P -> 420P
				else if (src_pix_fmt == AV_PIX_FMT_YUVJ422P && dst_pix_fmt == AV_PIX_FMT_YUV420P)
				{
					YUV422PTo420PApplyTable(src[i], srcStride[i],
											dst[i], dstStride[i],
											width >> dst_pix_desc->log2_chroma_w,
											height >> dst_pix_desc->log2_chroma_h,
											c_jpeg_to_itu601);
				}
				// J420P -> 420P or J422P -> 422P
				else
				{
					YUVApplyTable(	src[i], srcStride[i],
									dst[i], dstStride[i],
									width >> dst_pix_desc->log2_chroma_w,
									height >> dst_pix_desc->log2_chroma_h,
									c_jpeg_to_itu601);
				}
			}

			return TRUE;
		}
		// ITU601 -> JPEG
		else if (IsITU601ToJPEG(src_pix_fmt, dst_pix_fmt))
		{
			// Y plane
			YUVApplyTable(	src[0], srcStride[0],
							dst[0], dstStride[0],
							width,
							height,
							y_itu601_to_jpeg);

			// Chroma planes
			for (i = 1 ; i <= 2 ; i++)
			{
				// 420P -> J422P
				if (src_pix_fmt == AV_PIX_FMT_YUV420P && dst_pix_fmt == AV_PIX_FMT_YUVJ422P)
				{
					YUV420PTo422PApplyTable(src[i], srcStride[i],
											dst[i], dstStride[i],
											width >> dst_pix_desc->log2_chroma_w,
											height >> dst_pix_desc->log2_chroma_h,
											c_itu601_to_jpeg);
				}
				// 422P -> J420P 
				else if (src_pix_fmt == AV_PIX_FMT_YUV422P && dst_pix_fmt == AV_PIX_FMT_YUVJ420P)
				{
					YUV422PTo420PApplyTable(src[i], srcStride[i],
											dst[i], dstStride[i],
											width >> dst_pix_desc->log2_chroma_w,
											height >> dst_pix_desc->log2_chroma_h,
											c_itu601_to_jpeg);
				}
				// 420P -> J420P or 422P -> J422P
				else
				{
					YUVApplyTable(	src[i], srcStride[i],
									dst[i], dstStride[i],
									width >> dst_pix_desc->log2_chroma_w,
									height >> dst_pix_desc->log2_chroma_h,
									c_itu601_to_jpeg);
				}
			}

			return TRUE;
		}
	}

	return FALSE;
}


#ifdef _DEBUG

static BOOL compare_plane(uint8_t* srcdata, int srclinesize,
						uint8_t* dstdata, int dstlinesize,
						int plane_width, int plane_height)
{
	int x, diff, srcvalue, dstvalue;
	if (!srcdata || !dstdata)
		return FALSE;
	for ( ; plane_height > 0 ; plane_height--)
	{
		for (x = 0 ; x < plane_width ; x++)
		{
			srcvalue = srcdata[x];
			dstvalue = dstdata[x];
			diff = srcvalue - dstvalue;
			if (diff > 1 || diff < -1) // allow +/- 1 rounding error
				return FALSE;
		}
		srcdata += srclinesize;
		dstdata += dstlinesize;
	}
	return TRUE;
}

static BOOL compare_yuv_image(uint8_t* srcdata[4], int srclinesize[4],
							uint8_t* dstdata[4], int dstlinesize[4],
							enum AVPixelFormat pix_fmt,
							int width, int height)
{
	const AVPixFmtDescriptor* desc = av_pix_fmt_desc_get(pix_fmt);
	if (!compare_plane(srcdata[0], srclinesize[0], dstdata[0], dstlinesize[0], width, height))
		return FALSE;
	if (!compare_plane(srcdata[1], srclinesize[1], dstdata[1], dstlinesize[1], width >> desc->log2_chroma_w, height >> desc->log2_chroma_h))
		return FALSE;
	if (!compare_plane(srcdata[2], srclinesize[2], dstdata[2], dstlinesize[2], width >> desc->log2_chroma_w, height >> desc->log2_chroma_h))
		return FALSE;
	return TRUE;
}

static void clear_plane(int value,
						uint8_t* data, int linesize,
						int plane_width, int plane_height)
{
	if (!data)
		return;
	for ( ; plane_height > 0 ; plane_height--)
	{
		memset(data, value, plane_width);
		data += linesize;
	}
}

// Clear to red (becomes blue if U <-> V flipped)
static void clear_yuv_image(uint8_t* data[4], int linesize[4],
							enum AVPixelFormat pix_fmt,
							int width, int height)
{
	const AVPixFmtDescriptor* desc = av_pix_fmt_desc_get(pix_fmt);
	clear_plane(81, data[0], linesize[0], width, height);
	clear_plane(90, data[1], linesize[1], width >> desc->log2_chroma_w, height >> desc->log2_chroma_h);
	clear_plane(240, data[2], linesize[2], width >> desc->log2_chroma_w, height >> desc->log2_chroma_h);
}

// Make a multicolor synthetic image
static void fill_yuv_image(	int frame_index,
							uint8_t* data[4], int linesize[4],
							enum AVPixelFormat pix_fmt,
							int width, int height)
{
	const AVPixFmtDescriptor* desc = av_pix_fmt_desc_get(pix_fmt);
	int x, y;

	// Y plane (ITU601 range [16,235])
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			data[0][y * linesize[0] + x] = x + y + frame_index * 3;
			if (data[0][y * linesize[0] + x] < 16)
				data[0][y * linesize[0] + x] = 16;
			else if (data[0][y * linesize[0] + x] > 235)
				data[0][y * linesize[0] + x] = 235;
		}
	}

	// Cb and Cr planes (ITU601 range [16,240])
	width >>= desc->log2_chroma_w;
	height >>= desc->log2_chroma_h;
	for (y = 0; y < height ; y++)
	{
		for (x = 0; x < width ; x++)
		{
			data[1][y * linesize[1] + x] = 128 + y + frame_index * 2;
			if (data[1][y * linesize[1] + x] < 16)
				data[1][y * linesize[1] + x] = 16;
			else if (data[1][y * linesize[1] + x] > 240)
				data[1][y * linesize[1] + x] = 240;

			data[2][y * linesize[2] + x] = 64 + x + frame_index * 5;
			if (data[2][y * linesize[1] + x] < 16)
				data[2][y * linesize[1] + x] = 16;
			else if (data[2][y * linesize[1] + x] > 240)
				data[2][y * linesize[1] + x] = 240;
		}
	}
}

static BOOL ITU601JPEGConvertTest(int width, int height)
{
	BOOL res = FALSE;

	// Init frames
	// Note: av_frame_get_buffer correctly aligns the planes buffers
	// (no need for FF_INPUT_BUFFER_PADDING_SIZE)

	AVFrame* SrcFrame_420P = av_frame_alloc();
	if (!SrcFrame_420P)
		goto error;
	SrcFrame_420P->width  = width;
	SrcFrame_420P->height = height;
	SrcFrame_420P->format = AV_PIX_FMT_YUV420P;
	if (av_frame_get_buffer(SrcFrame_420P, 32) < 0)
		goto error;
	fill_yuv_image(	0,
					SrcFrame_420P->data, SrcFrame_420P->linesize,
					AV_PIX_FMT_YUV420P,
					width, height);

	AVFrame* Frame_420P = av_frame_alloc();
	if (!Frame_420P)
		goto error;
	Frame_420P->width  = width;
	Frame_420P->height = height;
	Frame_420P->format = AV_PIX_FMT_YUV420P;
	if (av_frame_get_buffer(Frame_420P, 32) < 0)
		goto error;

	AVFrame* Frame_J420P = av_frame_alloc();
	if (!Frame_J420P)
		goto error;
	Frame_J420P->width  = width;
	Frame_J420P->height = height;
	Frame_J420P->format = AV_PIX_FMT_YUVJ420P;
	if (av_frame_get_buffer(Frame_J420P, 32) < 0)
		goto error;
	
	AVFrame* Frame_422P = av_frame_alloc();
	if (!Frame_422P)
		goto error;
	Frame_422P->width  = width;
	Frame_422P->height = height;
	Frame_422P->format = AV_PIX_FMT_YUV422P;
	if (av_frame_get_buffer(Frame_422P, 32) < 0)
		goto error;

	AVFrame* Frame_J422P = av_frame_alloc();
	if (!Frame_J422P)
		goto error;
	Frame_J422P->width  = width;
	Frame_J422P->height = height;
	Frame_J422P->format = AV_PIX_FMT_YUVJ422P;
	if (av_frame_get_buffer(Frame_J422P, 32) < 0)
		goto error;
	
	// Pass source frame through all 8 combinations

	// 420P -> J420P
	clear_yuv_image(Frame_J420P->data, Frame_J420P->linesize,
					AV_PIX_FMT_YUVJ420P, width, height);
	if (!ITU601JPEGConvert(	AV_PIX_FMT_YUV420P,
							AV_PIX_FMT_YUVJ420P,
							SrcFrame_420P->data, SrcFrame_420P->linesize,
							Frame_J420P->data, Frame_J420P->linesize,
							width, height))
		goto error;

	// J420P -> 422P
	clear_yuv_image(Frame_422P->data, Frame_422P->linesize,
					AV_PIX_FMT_YUV422P, width, height);
	if (!ITU601JPEGConvert(	AV_PIX_FMT_YUVJ420P,
							AV_PIX_FMT_YUV422P,
							Frame_J420P->data, Frame_J420P->linesize,
							Frame_422P->data, Frame_422P->linesize,
							width, height))
		goto error;

	// 422P -> J422P
	clear_yuv_image(Frame_J422P->data, Frame_J422P->linesize,
					AV_PIX_FMT_YUVJ422P, width, height);
	if (!ITU601JPEGConvert(	AV_PIX_FMT_YUV422P,
							AV_PIX_FMT_YUVJ422P,
							Frame_422P->data, Frame_422P->linesize,
							Frame_J422P->data, Frame_J422P->linesize,
							width, height))
		goto error;


	// J422P -> 422P
	clear_yuv_image(Frame_422P->data, Frame_422P->linesize,
					AV_PIX_FMT_YUV422P, width, height);
	if (!ITU601JPEGConvert(	AV_PIX_FMT_YUVJ422P,
							AV_PIX_FMT_YUV422P,
							Frame_J422P->data, Frame_J422P->linesize,
							Frame_422P->data, Frame_422P->linesize,
							width, height))
		goto error;


	// 422P -> J420P
	clear_yuv_image(Frame_J420P->data, Frame_J420P->linesize,
					AV_PIX_FMT_YUVJ420P, width, height);
	if (!ITU601JPEGConvert(	AV_PIX_FMT_YUV422P,
							AV_PIX_FMT_YUVJ420P,
							Frame_422P->data, Frame_422P->linesize,
							Frame_J420P->data, Frame_J420P->linesize,
							width, height))
		goto error;
	

	// J420P -> 420P
	clear_yuv_image(Frame_420P->data, Frame_420P->linesize,
					AV_PIX_FMT_YUV420P, width, height);
	if (!ITU601JPEGConvert(	AV_PIX_FMT_YUVJ420P,
							AV_PIX_FMT_YUV420P,
							Frame_J420P->data, Frame_J420P->linesize,
							Frame_420P->data, Frame_420P->linesize,
							width, height))
		goto error;

	// 420P -> J422P
	clear_yuv_image(Frame_J422P->data, Frame_J422P->linesize,
					AV_PIX_FMT_YUVJ422P, width, height);
	if (!ITU601JPEGConvert(	AV_PIX_FMT_YUV420P,
							AV_PIX_FMT_YUVJ422P,
							Frame_420P->data, Frame_420P->linesize,
							Frame_J422P->data, Frame_J422P->linesize,
							width, height))
		goto error;

	// J422P -> 420P
	clear_yuv_image(Frame_420P->data, Frame_420P->linesize,
					AV_PIX_FMT_YUV420P, width, height);
	if (!ITU601JPEGConvert(	AV_PIX_FMT_YUVJ422P,
							AV_PIX_FMT_YUV420P,
							Frame_J422P->data, Frame_J422P->linesize,
							Frame_420P->data, Frame_420P->linesize,
							width, height))
		goto error;

	// Compare source and last one
	res = compare_yuv_image(SrcFrame_420P->data, SrcFrame_420P->linesize,
							Frame_420P->data, Frame_420P->linesize,
							AV_PIX_FMT_YUV420P,
							width, height);

error:
	av_frame_free(&SrcFrame_420P);
	av_frame_free(&Frame_420P);
	av_frame_free(&Frame_J420P);
	av_frame_free(&Frame_422P);
	av_frame_free(&Frame_J422P);

	return res;
}

#endif