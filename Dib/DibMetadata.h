#ifndef _INC_DIBMETADATA
#define _INC_DIBMETADATA

#include "..\XMLite.h"

// Xmp, Exif & Icc Header Sizes
#define EXIF_HEADER_SIZE			6
#define XMP_HEADER_SIZE				29
#define ICC_HEADER_SIZE				12

// Exif
#define	MAX_IMAGE_DESC				1024
#define	MAX_USER_COMMENT			1024
#define MAX_ARTIST					128
#define MAX_SOFTWARE				128
#define MAX_COPYRIGHT				128
#define MAX_JPEG_SECTIONS			96
#define EXIF_THUMB_LONG_EDGE		160
#define EXIF_THUMB_SHORT_EDGE		120
#define	EXIF_ADDTHUMB_MAXSIZE		5000
#define	EXIF_MAX_THUMB_QUALITY		100
#define USER_COMMENT_HEADER_SIZE	8
#define DEFAULT_EXIF_VERSION_STR	"0220"
#define DEFAULT_EXIF_VERSION_INT	220
#define MAX_TIFFDIR_DEPTH			16

// Iptc
#define MAX_IPTC_FIELD_SIZE			2048
#define UTF8_INVOCATION_SIZE		3
#define IPTC_VERSION				4

// Gps
#define GPS_DEGREE					0
#define GPS_HOUR					0
#define GPS_MINUTES					1
#define GPS_SECONDS					2
#define MAX_GPS_TAG					0x1E
#define TAG_GPS_VERSION				0
#define TAG_GPS_LAT_REF				1
#define TAG_GPS_LAT					2
#define TAG_GPS_LONG_REF			3
#define TAG_GPS_LONG				4
#define TAG_GPS_ALT_REF				5
#define TAG_GPS_ALT					6
#define TAG_GPS_TIMESTAMP			7
#define TAG_GPS_MAPDATUM			0x12
#define DEFAULT_GPS_VERSION_0		2
#define DEFAULT_GPS_VERSION_1		2
#define DEFAULT_GPS_VERSION_2		0
#define DEFAULT_GPS_VERSION_3		0
#define DEFAULT_GPS_VERSION_INT		2200

// Output Linebreak for Iptc Legacy and Xmp
//
// Photoshop 6-7 use CR, Photoshop CS and CS2 use LF
// Photoshop 6-7 don't understand LF
// -> use CR which is understood by all!
#define OUT_LINEBREAK				_T("\r")

//--------------------------------------------
// Here are the jpeg marker codes
// (See also jdmarker.c)
//
// Simplified Jpeg File Format:
//
// SOI
// JFIF and/or EXIF
// COM
// SOFn
// SOS
// Data
// EOI
//
// Each Marker consists of 16 bits:
// 0xFF + Marker
//
// Any Marker may be preceded by 0xFF paddings
//
// There are standalone markers like SOI or EOI,
// there are also marker segments: the size of
// the segment is limited to 16 bits. Size is
// following the marker in big endian format.
// The size includes the two size bytes, but
// excludes the 2 marker bytes.
//--------------------------------------------
#define M_SOF0  0xC0            // Start Of Frame N
#define M_SOF1  0xC1            // N indicates which compression process
#define M_SOF2  0xC2            // Only SOF0-SOF2 are now in common use
#define M_SOF3  0xC3
#define M_SOF5  0xC5
#define M_SOF6  0xC6
#define M_SOF7  0xC7
#define M_JPG   0xC8
#define M_SOF9  0xC9
#define M_DHT   0xC4
#define M_SOF10 0xCA
#define M_SOF11 0xCB
#define M_DAC   0xCC
#define M_SOF13 0xCD
#define M_SOF14 0xCE
#define M_SOF15 0xCF
#define M_RST0	0xD0
#define M_RST1	0xD1
#define M_RST2	0xD2
#define M_RST3	0xD3
#define M_RST4	0xD4
#define M_RST5	0xD5
#define M_RST6	0xD6
#define M_RST7	0xD7
#define M_SOI   0xD8            // Start Of Image (beginning of datastream)
#define M_EOI   0xD9            // End Of Image (end of datastream)
#define M_SOS   0xDA            // Start Of Scan (begins compressed data)
#define M_DQT	0xDB
#define M_DNL	0xDC
#define M_DRI	0xDD
#define M_DHP	0xDE
#define M_EXP	0xDF
#define M_JFIF  0xE0            // = M_APP0, Jfif marker
#define M_EXIF_XMP  0xE1        // = M_APP1, Exif or Xmp marker
#define M_ICC	0xE2			// = M_APP2, Icc Color Profile marker
#define M_IPTC  0xED            // = M_APP13, Iptc marker
#define M_APP0	0xE0
#define M_APP1	0xE1
#define M_APP2	0xE2
#define M_APP3	0xE3
#define M_APP4	0xE4
#define M_APP5	0xE5
#define M_APP6	0xE6
#define M_APP7	0xE7
#define M_APP8	0xE8
#define M_APP9	0xE9
#define M_APP10	0xEA
#define M_APP11	0xEB
#define M_APP12	0xEC
#define M_APP13	0xED
#define M_APP14	0xEE
#define M_APP15	0xEF
#define M_JPG0	0xF0
#define M_JPG13	0xFD
#define M_COM   0xFE            // COMment

// Formats
#define NUM_FORMATS		12
#define FMT_BYTE		1 
#define FMT_STRING		2
#define FMT_USHORT		3
#define FMT_ULONG		4
#define FMT_URATIONAL	5
#define FMT_SBYTE		6
#define FMT_UNDEFINED	7
#define FMT_SSHORT		8
#define FMT_SLONG		9
#define FMT_SRATIONAL	10
#define FMT_SINGLE		11
#define FMT_DOUBLE		12

// Describes tag values

// Original Tiff Tags describing the image in IFD0
#define TAG_IMAGE_DESC				0x010E
#define TAG_MAKE					0x010F
#define TAG_MODEL					0x0110
#define TAG_ORIENTATION				0x0112
#define TAG_XRESOLUTION				0x011A
#define TAG_YRESOLUTION				0x011B
#define TAG_RESOLUTIONUNIT			0x0128
#define TAG_SOFTWARE				0x0131
#define TAG_DATETIME				0x0132
#define TAG_ARTIST					0x013B

// Original Tiff Tags used for the Thumbnail in IFD1
#define TAG_WIDTH					0x0100
#define TAG_HEIGHT					0x0101
#define TAG_BITSPERSAMPLE			0x0102
#define TAG_COMPRESSION				0x0103
#define TAG_PHOTOMETRIC				0x0106
#define TAG_SAMPLESPERPIX			0x0115
#define TAG_ROWSPERSTRIP			0x0116
#define TAG_JPEGIFOFFSET			0x0201
#define TAG_JPEGIFBYTECOUNT			0x0202
#define TAG_COEFFICIENTS			0x0211
#define TAG_YCBCRSUBSAMPL			0x0212
#define TAG_YCBCRPOS				0x0213
#define TAG_REFERENCEBW				0x0214
#define TAG_PLANARCONFIG			0x011C
#define TAG_STRIPOFFSETS			0x0111
#define TAG_STRIPBYTECOUNTS			0x0117

// Offsets to Sub IFD
#define TAG_EXIF_SUBIFD				0x8769
#define TAG_INTEROP_SUBIFD			0xA005
#define TAG_GPS_SUBIFD				0x8825

// Metadata Tags
#define TAG_XMLPACKET				700
#define TAG_RICHTIFFIPTC			33723
#define TAG_ICCPROFILE				34675
#define TAG_PHOTOSHOP				34377

// Exif Specific Tags
#define TAG_EXIF_VERSION			0x9000
#define TAG_COPYRIGHT				0x8298
#define TAG_MAKER_NOTE				0x927C
#define TAG_USER_COMMENT			0x9286
#define TAG_EXPOSURETIME			0x829A
#define TAG_FNUMBER					0x829D
#define TAG_SHUTTERSPEED			0x9201
#define TAG_APERTURE				0x9202
#define TAG_BRIGHTNESS				0x9203
#define TAG_MAXAPERTURE				0x9205
#define TAG_FOCALLENGTH				0x920A
#define TAG_DATETIME_ORIGINAL		0x9003
#define TAG_DATETIME_DIGITIZED		0x9004
#define TAG_SUBJECT_DISTANCE		0x9206
#define TAG_FLASH					0x9209
#define TAG_FOCALPLANEXRES			0xA20E
#define TAG_FOCALPLANEYRES			0xA20F
#define TAG_FOCALPLANEUNITS			0xA210
#define TAG_EXIF_IMAGEWIDTH			0xA002
#define TAG_EXIF_IMAGEHEIGHT		0xA003
#define TAG_EXPOSURE_PROGRAM		0x8822
#define TAG_EXPOSURE_BIAS			0x9204
#define TAG_EXPOSURE_INDEX			0xA215
#define TAG_EXPOSURE_MODE			0xA402
#define TAG_WHITEBALANCE			0xA403
#define TAG_DIGITALZOOMRATIO		0xA404
#define TAG_FOCALLENGTH_35MM		0xA405
#define TAG_DISTANCE_RANGE			0xA40C
#define TAG_METERING_MODE			0x9207
#define TAG_ISO_EQUIVALENT			0x8827 // = EXIFTAG_ISOSPEEDRATINGS
#define TAG_COMPRESSION_LEVEL		0x9102
#define TAG_LIGHT_SOURCE			0x9208
#define TAG_AMBIENT_TEMPERATURE		0x9400
#define TAG_HUMIDITY				0x9401
#define TAG_PRESSURE				0x9402
#define TAG_WATERDEPTH				0x9403

/////////////////////////////////////////////////
// The Exif Format in Jpeg
// Jpeg Exif Marker = 0xFF + M_APP1 (2 bytes)
// Jpeg Exif Length					(2 bytes)
// Exif Id Code = 'Exif\0\0'		(6 bytes)
// Tiff Header	= 'II' or 'MM'		(2 bytes)
//                0x002A			(2 bytes)
//                Offset to 0th IFD	(4 bytes)
//   - II Intel byte ordering,
//     MM Motorola byte ordering
//   - The Offset is from Tiff Header Begin	
// 0th IFD = Count					(2 bytes) 
//           Entry[0]				(12 bytes)
//           Entry[1]				(12 bytes)
//           ..
//           Entry[Count-1]			(12 bytes)
//           Offset to 1st IFD      (4 bytes)
//   - The Offset is from Tiff Header Begin
//   - Entry = Tag			(2 bytes)
//             Type			(2 bytes)
//             Count		(4 bytes)
//             Value Offset	(4 bytes)
//   - The Value Offset is from Tiff Header Begin.
//     If the Type is less or equal 4 bytes, the
//     Value Offset containes the Value itself!
//     Less than 4 bytes values start at the
//     lower address.
//   - The 0th IFD contains the EXIF IFD SubDir
//     and may also contain the Interoperability
//     IFD SubDir.
//
// 1st IFD = Like 0th IFD but for Thumbnail
//   - There is an Entry that points to the
//     Thumbnail Data (Offset is from Tiff
//     Header Begin) and another which gives
//     the Thumbnail Size.
/////////////////////////////////////////////////

/////////////////////////////////////////////////
// IPTC problem in LibTiff
/*
The structure is the following, and the same as the first few bytes of every
IPTC tag in the structure:
- offset 0: signature byte 0x1C
- offset 1: record ID byte
- offset 2: tag ID byte
- offset 3: datalength motorola-order word

If the most significant bit of the datalength word is set, then the datalength
is not actually the datalength, but the most significant bit should be cleared,
and the value should be taken to be the number of bytes that follow and the
actually do make the datalength. For example, if it is (32768 or 4), then 4
bytes follow that together or the datalength motorola-order long.

Next follows the actual IPTC tag data. The length of that data is not restricted
to multiples of two bytes or anything, it can be 3 or it could be 4.

Next follow no pad bytes, and if there's more tags to follow, the IPTC tag
structure is repeated.

What happens now if the data is byteswapped as if it were an array of long, is
that any IPTC tag reader does not find the data in the expected place. The 0x1C
byte, for example, is not at offset 0 but at offset 3, the record ID byte is not
at offset 1 (expected), nor at offset 4 (expected if the reader skips to 0x1C
for robustness), but at offset 2. Thus, the only way the reader can make sense
of it, is by again byteswapping as if it were an array of long. Thus, if an
intel machine writes a motorola-order TIFF, and feeds LibTiff proper data, the
data is byteswapped, and the reader has to byteswap again, even if it runs on a
motorola machine.

A reader that knows about LibTiff's wrong datatype and incorrect long array
byteswapping corruption, could compensate by checking the offset of the 0x1C
value. If it is not found at offset 0, but is found at offset 3, a reader might
assume this particular corruption, and long array byteswap again. This is what I
mean by 'compensate for long-swapped corrupted data'. Does Photoshop do that?

A writer that writes out the data with datatype long, can choose to either not
byteswap even though when the incorrect datatype long implies swapping, or
corrupt the data. Which of these two options does Photoshop go for? (That is,
assuming Photoshop has the ability to write TIFF byteorder that is non-native to
the machine, otherwise this is not an issue.)
*/

/////////////////////////////////////////////////
// Add a new IFD0 Exif SubDir entry
/*
1. Add a hex define under "Exif Specific Tags" (DibMetadata.h)
2. Add a var to EXIFINFO (DibMetadata.h)
3. Unset values other than 0 must be init in CMetadata::CMetadata() and CMetadata::Free() (DibMetadata.cpp)
4. Add two if entries in CMetadata::MakeExifSection() (DibMetadata.cpp)
5. Add a switch entry to CMetadata::ParseTIFFDir() (DibMetadata.cpp)
6. Add a decimal define under "EXIF tags" (tiff.h)
7. Add a exifFieldInfo[] entry (tif_dirinfo.c)
8. Add a switch entry to TIFFGetExifField() (DibTiff.cpp)
*/
class CMetadata
{
public:
	// Jpeg Section
	struct JPEGSECTION
	{
		unsigned char* Data;
		int Type;
		unsigned int Size;
	};

	// Exif
	struct EXIFINFO
	{
		// Flags
		bool bHasExif;					// Is set if the TAG_EXIF_OFFSET tag is found,
										// for simple Tiff this is not set!
		bool bHasExifSection;			// Has EXIF Section in Jpeg APP1,
										// this is set even if EXIF is corrupted!

		// From Jpeg File
		int   Width;					// Always Set (even if bHasExif == false)
		int   Height;					// Always Set (even if bHasExif == false)
		int   JpegSOFnMarker;			// Always Set (even if bHasExif == false)

		// From Tags
		char  Version				[5];
		char  CameraMake			[32];
		char  CameraModel			[40];
		char  Artist				[MAX_ARTIST];
		char  Software				[MAX_SOFTWARE];
		char  CopyrightPhotographer	[MAX_COPYRIGHT];
		char  CopyrightEditor		[MAX_COPYRIGHT];
		char  ImageDescription		[MAX_IMAGE_DESC];
		WCHAR UserComment			[MAX_USER_COMMENT];
		char  DateTime				[20];
		int   Orientation;
		int   Flash;					// -1 if not set
		float FocalLength;
		float ExposureTime;
		float ApertureFNumber;
		float Distance;
		float ExposureBias;
		float DigitalZoomRatio;
		int   FocalLength35mmEquiv;		// Exif 2.2 tag (usually not present)
		int   WhiteBalance;				// -1 if not set
		int   MeteringMode;
		int   ExposureProgram;
		int   ExposureMode;
		unsigned short ISOequivalent;	// See DibTiff.cpp to understand the use of the unsigned short
		int   LightSource;
		int   DistanceRange;
		float CompressionLevel;			// Rough estimate of the average compression
										// ratio of JPEG in Compressed Bits / Pixel
		float FocalplaneXRes;
		float FocalplaneYRes;
		float FocalplaneUnits;
		float Xresolution;
		float Yresolution;
		float ResolutionUnit;
		float Brightness;
		float AmbientTemperature;		// ambient temperature in °C, NAN if not set
		float Humidity;					// ambient relative humidity in percent, NAN if not set
		float Pressure;					// air pressure in hPa or mbar, NAN if not set
		float WaterDepth;				// depth under water in meters, negative for above water, NAN if not set

		// Calculated, it's not from a Tag!
		float CCDWidth;

		// Thumbnail
		bool bFreeThumbnailPointer;
		unsigned char* ThumbnailPointer;	// Pointer at the thumbnail
		unsigned int ThumbnailSize;			// Size of thumbnail
		unsigned int ThumbnailOffset;
		int ThumbnailWidth;
		int ThumbnailHeight;
		int ThumbnailOrientation;
		int ThumbnailCompression;
		int ThumbnailPhotometricInterpretation;
		double dThumbnailYCbCrCoeff[3];		// Conversion Coefficients From YCbCr to RGB
		double dThumbnailRefBW[6];			// BW Reference
		int ThumbnailYCbCrSubSampling[2];	// 2,1 = YCbCr4:2:2
											// 2,2 = YCbCr4:2:0

		// GPS
		bool bGpsInfoPresent;
		unsigned char GpsVersion[4];
		float GpsLat[3];					// index 0: degree, index 1: minutes, index 2: seconds, value -1.0 if not set
		float GpsLong[3];					// index 0: degree, index 1: minutes, index 2: seconds, value -1.0 if not set
		float GpsAlt;						// -1.0 if not set
		char GpsLatRef[2];					// "N" or "S"
		char GpsLongRef[2];					// "E" or "W"
		char GpsAltRef;						// -1 if not set
											// If the reference is sea level and the altitude is above sea level,
											// 0 is given. If the altitude is below sea level, a value of 1 is
											// given and the altitude is indicated as an absolute value in
											// the GPSAltitude tag
		float GpsTime[3];					// index 0: hour, index 1: minutes, index 2: seconds, value -1.0 if not set
		char GpsMapDatum[20];				// Usually "WGS-84"

		// May be used for any purpose
		LPBYTE pUserData;

		// Not Stored Tags:
		// - Maker Notes
	};

	struct EXIFINFOINPLACEWRITE
	{
		bool bWidth;
		bool bHeight;
		bool bOrientation;
		bool bThumbnail;
		bool bThumbnailWidth;
		bool bThumbnailHeight;
		bool bThumbnailOrientation;
		bool bThumbnailCompression;
		bool bThumbnailPhotometricInterpretation;
		// Note: The Photometric Interpretation
		// is not used for Jpeg Thumbs,
		// but should be changed to 6 (YCbCr Format)
		// when converting from Tiff RGB Thumbs
		// to Jpeg Thumbs!
		bool bDateTime;
	};

	class IPTCINFO  
	{
	public:
		// Iptc Encoding
		enum {	IPTC_ENCODING_UNKNOWN = 0,
				IPTC_ENCODING_UNSPECIFIED = 1,
				IPTC_ENCODING_UTF8 = 2};

		// Member Functions
		IPTCINFO(){Clear();};
		virtual ~IPTCINFO(){;};
		IPTCINFO(const IPTCINFO& iptcinfo) // Copy Constructor (IPTCINFO iptcinfo1 = iptcinfo2 or IPTCINFO iptcinfo1(iptcinfo2))
		{
			// Clear
			Clear();

			// Copy
			CopyVars(iptcinfo);
		};
		IPTCINFO& operator=(const IPTCINFO& iptcinfo) // Copy Assignment (IPTCINFO iptcinfo3; iptcinfo3 = iptcinfo1)
		{
			if (this != &iptcinfo) // beware of self-assignment!
			{
				// Clear
				Clear();

				// Copy
				CopyVars(iptcinfo);
			}
			return *this;
		};
		BOOL operator==(const IPTCINFO& iptcinfo) {return IsEqual(iptcinfo);};
		BOOL operator!=(const IPTCINFO& iptcinfo) {return !IsEqual(iptcinfo);};
		BOOL IsEqual(const IPTCINFO& iptcinfo);
		BOOL IsANSIConvertible();
		BOOL IsEmpty();
		void CopyVars(const IPTCINFO& iptcinfo);
		void Clear();
		void MakeCRLF();
		static __forceinline BOOL IsArrayEqual(const CStringArray& array1, const CStringArray& array2)
		{
			if (array1.GetSize() != array2.GetSize())
				return FALSE;
			for (int i = 0 ; i < array1.GetSize() ; i++)
			{
				if (array1[i] != array2[i])
					return FALSE;
			}
			return TRUE;
		};
		static __forceinline BOOL IsInArray(const CString& string, const CStringArray& array)
		{
			for (int i = 0 ; i < array.GetSize() ; i++)
			{
				if (array[i] == string)
					return TRUE;
			}
			return FALSE;
		};
		
		// Encoding
		int Encoding;

		// Version
		unsigned short Version;

		// Caption
		CString Caption;
		CString CaptionWriter;
		CString Headline;
		CString SpecialInstructions;

		// Credits
		CString Byline;	
		CString BylineTitle;
		CString Credits;
		CString Source;
		
		// Origin
		CString ObjectName;
		CString DateCreated;
		CString City;
		CString ProvinceState;
		CString Country;
		CString OriginalTransmissionReference;
		
		// Categories
		CString Category;
		CStringArray SupplementalCategories;
		CString Urgency;
		
		// Keywords
		CStringArray Keywords;
		
		// Copyright
		CString CopyrightNotice;
	};

	class XMPINFO  
	{
	public:
		// Member Functions
		XMPINFO(){Clear();};
		virtual ~XMPINFO(){;};
		XMPINFO(const XMPINFO& xmpinfo) // Copy Constructor (XMPINFO xmpinfo1 = xmpinfo2 or XMPINFO xmpinfo1(xmpinfo2))
		{
			// Clear
			Clear();

			// Copy
			CopyVars(xmpinfo);
		};
		XMPINFO& operator=(const XMPINFO& xmpinfo) // Copy Assignment (XMPINFO xmpinfo3; xmpinfo3 = xmpinfo1)
		{
			if (this != &xmpinfo) // beware of self-assignment!
			{
				// Clear
				Clear();

				// Copy
				CopyVars(xmpinfo);
			}
			return *this;
		};
		BOOL operator==(const XMPINFO& xmpinfo) {return IsEqual(xmpinfo);};
		BOOL operator!=(const XMPINFO& xmpinfo) {return !IsEqual(xmpinfo);};
		BOOL IsEqual(const XMPINFO& xmpinfo);
		BOOL IsEmpty();
		void CopyVars(const XMPINFO& xmpinfo);
		void Clear();
		void MakeCRLF();
		static __forceinline BOOL IsArrayEqual(const CStringArray& array1, const CStringArray& array2)
		{
			if (array1.GetSize() != array2.GetSize())
				return FALSE;
			for (int i = 0 ; i < array1.GetSize() ; i++)
			{
				if (array1[i] != array2[i])
					return FALSE;
			}
			return TRUE;
		};
		static __forceinline BOOL IsInArray(const CString& string, const CStringArray& array)
		{
			for (int i = 0 ; i < array.GetSize() ; i++)
			{
				if (array[i] == string)
					return TRUE;
			}
			return FALSE;
		};
		
		// Copyright
		CString CopyrightUrl;		// The location of a web page describing the owner
									// and/or rights statement for this resource.
		CString CopyrightMarked;	// _T("")      : Unknown, Unmarked
									// _T("True")  : Copyrighted
									// _T("False") : Public Domain

		// Creator Contact Info (IPTC4XMP Core)
		//
		// Byline from IPTCINFO is the Creator
		// BylineTitle form IPTCINFO is Creator's Job Title
		CString CiAdrExtadr;		// Address
		CString CiAdrCity;			// City
		CString CiAdrRegion;		// State/Province
		CString CiAdrPcode;			// Postal Code
		CString CiAdrCtry;			// Country
		CString CiTelWork;			// Phone(s)
		CString CiEmailWork;		// E-Mail(s)
		CString CiUrlWork;			// Website(s)

		// Content
		//
		// Headline from IPTCINFO
		// Caption from IPTCINFO is the Description
		// Keywords from IPTCINFO
		CStringArray SubjectCode;	// IPTC Subject Codes
		// CaptionWriter from IPTCINFO is the Description Writer

		// Image
		//
		// DateCreated from IPTCINFO
		CString IntellectualGenre;	// Intellectual Genre
		CStringArray Scene;			// IPTC Scenes
		CString Location;			// Location
		// City from IPTCINFO
		// ProvinceState from IPTCINFO
		// Country from IPTCINFO
		CString CountryCode;		// ISO 3166 Country Code, 2 or 3 letter code

		// Status
		//
		// ObjectName from IPTCINFO is the Title
		// OriginalTransmissionReference from IPTCINFO is the JobID
		// SpecialInstructions from IPTCINFO is the Instructions
		// Credits from IPTCINFO is the Provider
		// Source from IPTCINFO
		// CopyrightNotice from IPTCINFO
		CString UsageTerms;			// Rights Usage Terms
	};

	// Jpeg ICC Marker(s) parsing
	class CIccSectionEntry
	{
		public:
			CIccSectionEntry() {data = NULL; size = 0;};
			virtual ~CIccSectionEntry() {if (data) delete [] data;};
			LPBYTE data;
			int size;
	};
	typedef CArray<CIccSectionEntry*,CIccSectionEntry*> ICCARRAY;

	// Member Functions
	CMetadata();
	CMetadata(const CMetadata& metadata); // Copy Constructor
	CMetadata& operator=(const CMetadata& metadata); // Copy Assignment
	~CMetadata();
	void Free();
	bool HasJfif() {return m_bHasJfif;};
	bool HasCom() {return m_bHasCom;};
	bool HasOtherAppSections() {return m_bHasOtherAppSections;};
	bool HasIcc() {return m_pIccData && m_dwIccSize > 0;};
	bool HasIptcLegacy() {return m_pIptcLegacyData && m_dwIptcLegacySize > 0;};
	bool HasXmp() {return m_pXmpData && m_dwXmpSize > 0;};
	static COleDateTime GetOleDateTimeFromExifString(const CString& sExifDateTime);
	static CTime GetDateTimeFromExifString(const CString& sExifDateTime);
	static void FillExifOleDateTimeString(const COleDateTime& Time, char* DateTime);
	static void FillExifDateTimeString(const CTime& Time, char* DateTime);
	static CTime GetDateFromIptcLegacyString(const CString& sIptcDate);
	static CTime GetDateFromXmpString(const CString& sXmpDate);
	bool ParseProcessJPEG(unsigned char* pData, int nLength, bool bDoWrite = false);
	bool ParseTIFF(int nStartIFD, unsigned char* pData, int nLength);
	void ParseIPTCLegacy(const unsigned char* pData, int nLength);
	BOOL ParseXMP(const unsigned char* pData, int nLength);
	JPEGSECTION* GetSection(int nSection);
	int GetSectionCount() const {return m_nSectionsRead;};
	
	// Removes the Exif Thumbnail from the given Section.
	// Returns the new Section size, -1 on error.
	int RemoveExifThumbnail(LPBYTE Section,			// Section data
							int nSectionSize);		// Section size

	// Appends a Thumbnail to the given Section.
	// Returns the new Section size, -1 on error.
	int AppendExifThumbnail(LPBYTE Section,			// Must be big enough to hold the additional Thumbnail!
							int nSectionSize,		// Current Section size
							int nMaxSectionSize,	// The maximum allowed size -> the Section buffer size
							LPBYTE pJpegThumbData,	// Jpeg Thumbnail Data, may be NULL
							DWORD dwJpegThumbSize);	// Jpeg Thumbnail Size, may be 0

	// Makes a Exif Section using the EXIFINFO struct
	// and the given Thumbnail.
	// Returns the created Section size, -1 on error.
	int MakeExifSection(LPBYTE Section,				// Must be big enough to hold all the data!
						int nMaxSectionSize,		// The maximum allowed size -> the Section buffer size
						DWORD dwImageWidth,			// Image Width
						DWORD dwImageHeight,		// Image Height
						LPBYTE pJpegThumbData,		// Jpeg Thumbnail Data, may be NULL
						DWORD dwJpegThumbSize);		// Jpeg Thumbnail Size, may be 0

	// Allocates m_pIptcLegacyData using the m_IptcLegacyInfo class.
	// Returns the created Data (as parameter) and the Size,
	// -1 on error.
	int MakeIptcLegacyData(LPBYTE* ppData);

	// Allocates a Iptc Legacy Jpeg Section merging
	// m_pIptcLegacyPreData, m_pIptcLegacyData and m_pIptcLegacyPostData.
	// MakeIptcLegacyData() is called to create the m_pIptcLegacyData.
	// Returns the created Data (as parameter) and the Size,
	// -1 on error.
	int MakeIptcLegacySection(LPBYTE* ppData);

	// Update the m_pXmpData and m_dwXmpSize from the m_IptcFromXmpInfo and the m_XmpInfo classes.
	BOOL UpdateXmpData(const CString& sMime); // sMime: image/jpeg, image/tiff, ...

	// Allocates a Xmp Section using m_pXmpData.
	// UpdateXmpData() is called to create the m_pXmpData.
	// Returns the created Data (as parameter) and the Size,
	// -1 on error.
	int MakeXmpSection(LPBYTE* ppData);

	// Jpeg File SOS Data
	LPBYTE GetAfterSOSData() const {return m_pAfterSOSData;};
	int GetAfterSOSSize() const {return m_dwAfterSOSSize;};
	unsigned int GetMaxOffset() const {return m_uiMaxOffset;};

	// Public Member Vars
	static const int m_BytesPerFormat[13];
	static const TCHAR * m_OrientationTab[9];
	bool m_bDoWrite;

	// Public Metadata Member Vars
	bool m_bHasJfif;				// Jpeg Has JFIF Section in APP0
	bool m_bHasCom;					// Jpeg Has COM Section
	CStringA m_sJpegComment;		// The encoding for the COM segment is not specified (like many software we use UTF-8)
	bool m_bHasOtherAppSections;	// Has at least one of the remaining APP Sections
	EXIFINFO m_ExifInfo;
	EXIFINFOINPLACEWRITE m_ExifInfoWrite;
	IPTCINFO m_IptcLegacyInfo;
	IPTCINFO m_IptcFromXmpInfo;
	XMPINFO m_XmpInfo;
	LPBYTE m_pXmpData;
	DWORD m_dwXmpSize;
	LPBYTE m_pIptcLegacyData;
	DWORD m_dwIptcLegacySize;
	LPBYTE m_pIptcLegacyPreData;
	DWORD m_dwIptcLegacyPreSize;
	LPBYTE m_pIptcLegacyPostData;
	DWORD m_dwIptcLegacyPostSize;
	LPBYTE m_pIccData;
	DWORD m_dwIccSize;
	LPBYTE m_pPhotoshopData;
	DWORD m_dwPhotoshopSize;
	static const char* m_GpsTags[MAX_GPS_TAG + 1];
	static const unsigned char m_ExifHeader[EXIF_HEADER_SIZE];
	static const unsigned char m_XmpHeader[XMP_HEADER_SIZE];
	static const unsigned char m_IccHeader[ICC_HEADER_SIZE];
	static const char m_USER_COMMENT_ASCII[USER_COMMENT_HEADER_SIZE];
	static const char m_USER_COMMENT_JIS[USER_COMMENT_HEADER_SIZE];
	static const char m_USER_COMMENT_UNICODE[USER_COMMENT_HEADER_SIZE];
	static const char m_USER_COMMENT_UNDEFINED[USER_COMMENT_HEADER_SIZE];
	static const unsigned char m_UTF8Invocation[UTF8_INVOCATION_SIZE];

protected:
	// Protected Member Functions
	int GetJpegIptcOffsetAndSize(unsigned char* pData, int& nLength);
	int WriteExifString(	int nTag,	
							const char* szTag,
							LPBYTE Section,
							int nMaxSize,
							int Base,
							int& Pos,
							int& PosData);
	void AppendIptcLegacyVersionAndEncoding(LPBYTE pData, int& Pos);
	void AppendIptcLegacyShort(BYTE Type, unsigned short Value, LPBYTE pData, int& Pos);
	void AppendIptcLegacyString(BYTE Type, CString sEntry, LPBYTE pData, int& Pos);
	CString ReadXmpString(LPXNode pXNode, const CString& name);
	void ReadXmpStringArray(LPXNode pXNode,
							const CString& name,
							CStringArray& array);
	void WriteXmpString(LPXNode pXNode, const CString& name, CString value);
	void WriteXmpStringArray(	LPXNode pXNode,
								const CString& name,
								const CStringArray& array,
								const CString& arraytype,
								const CString& attrname = _T(""),
								const CString& attrvalue = _T(""));
	bool ParseTIFFData(	int nStartIFD,
						bool bOnlyParseGivenIFD,
						bool bExifSection,
						unsigned char* pData,
						int nLength);
	bool ParseMakerNote(unsigned char* pDirStart, 
						unsigned char* pOffsetBase,
						int nLength,
						CMetadata::EXIFINFO* pExifInfo);
	bool ParseCanonMakerNote(	unsigned char* pDirStart,
								unsigned char* pOffsetBase, 
								int nLength,
								CMetadata::EXIFINFO* pExifInfo);
	void GpsNormalizeCoord(float& fDegree, float& fMinutes, float& fSeconds);
	void ParseGpsInfo(	unsigned char* pDirStart,
						unsigned char* pOffsetBase,
						int nLength,
						CMetadata::EXIFINFO* pExifInfo);
	void ParseCOMSection(const unsigned char* pData, int nLength);
	void ParseSOFnSection(int nMarker, const unsigned char* pData, int nLength);
	int GetNextIptcLegacyField(const unsigned char* pData, char& Record, char& Type, CString& Content);
	unsigned short Get16u(void* Short);
	short Get16s(void* Short);
	void Put16u(void* Short, unsigned short PutValue);
	void Put16s(void* Short, short PutValue);
	int Get16m(void* Short);
	void Put16m(void* Short, unsigned short PutValue);
	long Get32s(void* Long);
	unsigned long Get32u(void* Long);
	void Put32s(void* Long, long PutValue);
	void Put32u(void* Long, unsigned long PutValue);
	void SwabArrayOfDword(register DWORD* lp, register DWORD n);
	double ConvertFromAnyFormat(void* ValuePtr, int Format);
	bool ConvertToValue(void* ValuePtr, double Value, int Format);
	unsigned char* GetNextDirPointer(unsigned char* pData, unsigned char* pOffsetBase, int nLength);
	bool ParseTIFFDir(	int nIFD,
						bool bOnlyParseGivenIFD,
						bool bExifSection,	// true: parsing Jpeg file's exif section, false: parsing Tiff file
						unsigned char* pData,
						unsigned char* pOffsetBase,
						int nLength,
						EXIFINFO* pExifInfo,
						int nRecursionDepth);

	// Protected Member Vars
	int m_nExifImageWidth;
	bool m_bMotorolaOrder;
	JPEGSECTION m_Sections[MAX_JPEG_SECTIONS];
	LPBYTE m_pAfterSOSData;
	int m_dwAfterSOSSize;
	int m_nSectionsRead;
	LPBYTE m_pAllocatedThumbnailPointer;
	unsigned int m_uiMaxOffset;
};

#endif //!_INC_DIBMETADATA