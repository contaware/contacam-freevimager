#ifndef _INC_YUVTOYUV
#define _INC_YUVTOYUV

#ifdef VIDEODEVICEDOC

extern "C"
{
#include "libavutil/opt.h"
#include "libavutil/mathematics.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/pixdesc.h"
}

/*
ITU-R BT.601 (formerly called CCIR 601)
Y Range:     [16,235] (220 steps)
Cb,Cr Range: [16,240] (225 steps) 

RGB to YUV Conversion:

    Y  =      0.257 * R + 0.504 * G + 0.098 * B + 16

    Cb = U = -0.148 * R - 0.291 * G + 0.439 * B + 128

	Cr = V =  0.439 * R - 0.368 * G - 0.071 * B + 128

YUV to RGB Conversion:

	R = 1.164 * (Y - 16) + 0                  + 1.596 * (V - 128)

	G = 1.164 * (Y - 16) - 0.391 * (U - 128)  - 0.813 * (V - 128)  

    B = 1.164 * (Y - 16) + 2.018 * (U - 128)  + 0



JPEG
Y Range:     [0,255]
Cb,Cr Range: [0,255] 

RGB to YUV Conversion:

	Y =       0.299 * R + 0.587 * G + 0.114 * B

	Cb = U = -0.169 * R - 0.331 * G + 0.500 * B + 128

    Cr = V =  0.500 * R - 0.419 * G - 0.081 * B + 128

YUV to RGB Conversion:

	R = Y + 0                 + 1.400 * (V - 128)

	G = Y - 0.343 * (U - 128) - 0.711 * (V - 128) 

    B = Y + 1.765 * (U - 128) + 0



Converting between JPEG <-> ITU601 is just a scaling:
Y_ITU601 = Y_JPEG * 219 / 255 + 16
Y_JPEG = (Y_ITU601 - 16) * 255 / 219 

C_ITU601 = C_JPEG * 224 / 255 + 16
C_JPEG = (C_ITU601 - 16) * 255 / 224
*/


// Inizialization function
extern void InitYUVToYUVTable();

// Convert between:	AV_PIX_FMT_YUV420P, AV_PIX_FMT_YUV422P
//                                    ^
//                                    |
//                                    v
//                  AV_PIX_FMT_YUVJ420P, AV_PIX_FMT_YUVJ422P
// Attention:       Src and Dst images must have same width and height
// returns:         TRUE on success, FALSE in case of unsupported
//                  pixel formats or NULL parameters
extern BOOL ITU601JPEGConvert(	enum AVPixelFormat src_pix_fmt,
								enum AVPixelFormat dst_pix_fmt,
								uint8_t* src[], int srcStride[],
								uint8_t* dst[], int dstStride[],
								int width, int height);

#endif
#endif //!_INC_YUVTOYUV
