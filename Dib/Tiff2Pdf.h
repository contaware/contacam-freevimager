#ifndef _INC_TIFF2PDF
#define _INC_TIFF2PDF

#ifdef SUPPORT_LIBTIFF
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <windows.h>
#include <tchar.h>
extern "C"
{
#include "tiffiop.h"
}

#define TIFF2PDF_MODULE "tiff2pdf"
#define T2P_VERSION "d"

/* This type is of PDF color spaces. */
typedef enum{
	T2P_CS_BILEVEL=0x01, /* Bilevel, black and white */
	T2P_CS_GRAY=0x02, /* Single channel */
	T2P_CS_RGB=0x04, /* Three channel tristimulus RGB */
	T2P_CS_CMYK=0x08, /* Four channel CMYK print inkset */
	T2P_CS_LAB=0x10, /* Three channel L*a*b* color space */
	T2P_CS_PALETTE=0x1000, /* One of the above with a color map */
	T2P_CS_CALGRAY=0x20, /* Calibrated single channel */
	T2P_CS_CALRGB=0x40, /* Calibrated three channel tristimulus RGB */
	T2P_CS_ICCBASED=0x80, /* ICC profile color specification */
} t2p_cs_t;

/* This type is of PDF compression types.  */
typedef enum{
	T2P_COMPRESS_NONE=0x00
	, T2P_COMPRESS_G4=0x01
	, T2P_COMPRESS_JPEG=0x02
	, T2P_COMPRESS_ZIP=0x04
} t2p_compress_t;

/* This type is whether TIFF image data can be used in PDF without transcoding. */
typedef enum{
	T2P_TRANSCODE_RAW=0x01, /* The raw data from the input can be used without recompressing */
	T2P_TRANSCODE_ENCODE=0x02 /* The data from the input is perhaps unencoded and reencoded */
} t2p_transcode_t;

/* This type is of information about the data samples of the input image. */
typedef enum{
	T2P_SAMPLE_NOTHING=0x0000, /* The unencoded samples are normal for the output colorspace */
	T2P_SAMPLE_ABGR_TO_RGB=0x0001, /* The unencoded samples are the result of ReadRGBAImage */
	T2P_SAMPLE_RGBA_TO_RGB=0x0002, /* The unencoded samples are contiguous RGBA */
	T2P_SAMPLE_RGBAA_TO_RGB=0x0004, /* The unencoded samples are RGBA with premultiplied alpha */
	T2P_SAMPLE_YCBCR_TO_RGB=0x0008, 
	T2P_SAMPLE_YCBCR_TO_LAB=0x0010, 
	T2P_SAMPLE_REALIZE_PALETTE=0x0020, /* The unencoded samples are indexes into the color map */
	T2P_SAMPLE_SIGNED_TO_UNSIGNED=0x0040, /* The unencoded samples are signed instead of unsignd */
	T2P_SAMPLE_LAB_SIGNED_TO_UNSIGNED=0x0040, /* The L*a*b* samples have a* and b* signed */
	T2P_SAMPLE_PLANAR_SEPARATE_TO_CONTIG=0x0100 /* The unencoded samples are separate instead of contiguous */
} t2p_sample_t;

/* This type is of error status of the T2P struct. */
typedef enum{
	T2P_ERR_OK = 0, /* This is the value of t2p->t2p_error when there is no error */
	T2P_ERR_ERROR = 1 /* This is the value of t2p->t2p_error when there was an error */
} t2p_err_t;

/* This struct defines a logical page of a TIFF. */
typedef struct {
	tdir_t page_directory;
	uint32 page_number;
	ttile_t page_tilecount;
	uint32 page_extra;
	uint16 page_compression;
} T2P_PAGE;

/* This struct defines a PDF rectangle's coordinates. */
typedef struct {
	float x1;
	float y1;
	float x2;
	float y2;
	float mat[9];
} T2P_BOX;

/* This struct defines a tile of a PDF.  */
typedef struct {
	T2P_BOX tile_box;
} T2P_TILE;

/* This struct defines information about the tiles on a PDF page. */
typedef struct {
	ttile_t tiles_tilecount;
	uint32 tiles_tilewidth;
	uint32 tiles_tilelength;
	uint32 tiles_tilecountx;
	uint32 tiles_tilecounty;
	uint32 tiles_edgetilewidth;
	uint32 tiles_edgetilelength;
	T2P_TILE* tiles_tiles;
} T2P_TILES;

/* This struct is the context of a function to generate PDF from a TIFF. */
typedef struct {
	t2p_err_t t2p_error;
	T2P_PAGE* tiff_pages;
	T2P_TILES* tiff_tiles;
	tdir_t tiff_pagecount;
	uint16 tiff_compression;
	uint16 tiff_photometric;
	uint16 tiff_fillorder;
	uint16 tiff_bitspersample;
	uint16 tiff_samplesperpixel;
	uint16 tiff_planar;
	uint32 tiff_width;
	uint32 tiff_length;
	float tiff_xres;
	float tiff_yres;
	uint16 tiff_orientation;
	toff_t tiff_dataoffset;
	tsize_t tiff_datasize;
	TIFFReadWriteProc tiff_readproc;
	TIFFReadWriteProc tiff_writeproc;
	TIFFSeekProc tiff_seekproc;
	uint16 tiff_resunit;
	uint16 pdf_centimeters;
	uint16 pdf_overrideres;
	uint16 pdf_overridepagesize;
	float pdf_defaultxres;
	float pdf_defaultyres;
	float pdf_xres;
	float pdf_yres;
	float pdf_defaultpagewidth;
	float pdf_defaultpagelength;
	float pdf_pagewidth;
	float pdf_pagelength;
	float pdf_imagewidth;
	float pdf_imagelength;
	T2P_BOX pdf_mediabox;
	T2P_BOX pdf_imagebox;
	uint16 pdf_majorversion;
	uint16 pdf_minorversion;
	uint32 pdf_catalog;
	uint32 pdf_pages;
	uint32 pdf_info;
	uint32 pdf_palettecs;
	uint16 pdf_fitwindow;
	uint32 pdf_startxref;
	unsigned char* pdf_fileid;
	unsigned char* pdf_datetime;
	unsigned char* pdf_creator;
	unsigned char* pdf_author;
	unsigned char* pdf_title;
	unsigned char* pdf_subject;
	unsigned char* pdf_keywords;
	t2p_cs_t pdf_colorspace;
	uint16 pdf_colorspace_invert;
	uint16 pdf_switchdecode;
	uint16 pdf_palettesize;
	unsigned char* pdf_palette;
	int pdf_labrange[4];
	t2p_compress_t pdf_defaultcompression;
	uint16 pdf_defaultcompressionquality;
	t2p_compress_t pdf_compression;
	uint16 pdf_compressionquality;
	uint16 pdf_nopassthrough;
	t2p_transcode_t pdf_transcode;
	t2p_sample_t pdf_sample;
	uint32* pdf_xrefoffsets;
	uint32 pdf_xrefcount;
	tdir_t pdf_page;
	tdata_t pdf_ojpegdata;
	uint32 pdf_ojpegdatalength;
	uint32 pdf_ojpegiflength;
	float tiff_whitechromaticities[2];
	float tiff_primarychromaticities[6];
	float tiff_referenceblackwhite[2];
	float* tiff_transferfunction[3];
	int pdf_image_interpolate;	/* 0 (default) : do not interpolate,
					   1 : interpolate */
	uint16 tiff_transferfunctioncount;
	uint32 pdf_icccs;
	uint32 tiff_iccprofilelength;
	tdata_t tiff_iccprofile;
	int jpeg_defaultcompressionquality; // Oli Added
} T2P;

// sPaper may be "Fit" to fit each page to the given image
// or "A4", "Letter", ... centering each image inside the given constant paper type
//
// Compression conversion:
//
// COMPRESSION_CCITTRLE, COMPRESSION_CCITTFAX3, COMPRESSION_CCITTFAX4
//   -> T2P_COMPRESS_G4
// COMPRESSION_OJPEG, COMPRESSION_JPEG
//   -> T2P_COMPRESS_JPEG
// All Other
//   -> T2P_COMPRESS_ZIP
extern BOOL Tiff2Pdf(	LPCTSTR lpszTiff,
						LPCTSTR lpszPdf,
						const CString& sPaper,
						BOOL bFitWindow,
						BOOL bInterpolate,
						int nJpegCompressionQuality);

#endif
#endif //!_INC_TIFF2PDF