#include "stdafx.h"
#include "..\Helpers.h"
#include "dib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Bytes Per Format
const int CMetadata::m_BytesPerFormat[13] = {0,1,1,2,4,8,1,1,2,4,8,4,8};

// Orientation
const TCHAR* CMetadata::m_OrientationTab[9] =
{
    _T("undefined"),		// 0
    _T("normal"),           // 1
    _T("flip horizontal"),  // 2: left right reversed mirror
    _T("rotate 180°"),      // 3
    _T("flip vertical"),    // 4: upside down mirror
    _T("transpose"),        // 5: Flipped about top-left <--> bottom-right axis.
    _T("rotated 90° ccw"),  // 6: rotate 90° cw to right it.
    _T("transverse"),       // 7: flipped about top-right <--> bottom-left axis
    _T("rotated 90° cw"),   // 8: rotate 90° ccw to right it.
};

// Gps Tags
const char* CMetadata::m_GpsTags[MAX_GPS_TAG + 1] = 
{
    "VersionID       ",//0x00  
    "LatitudeRef     ",//0x01  
    "Latitude        ",//0x02  
    "LongitudeRef    ",//0x03  
    "Longitude       ",//0x04  
    "AltitudeRef     ",//0x05  
    "Altitude        ",//0x06  
    "TimeStamp       ",//0x07  
    "Satellites      ",//0x08  
    "Status          ",//0x09  
    "MeasureMode     ",//0x0A  
    "DOP             ",//0x0B  
    "SpeedRef        ",//0x0C  
    "Speed           ",//0x0D  
    "TrackRef        ",//0x0E  
    "Track           ",//0x0F  
    "ImgDirectionRef ",//0x10  
    "ImgDirection    ",//0x11  
    "MapDatum        ",//0x12  
    "DestLatitudeRef ",//0x13  
    "DestLatitude    ",//0x14  
    "DestLongitudeRef",//0x15  
    "DestLongitude   ",//0x16  
    "DestBearingRef  ",//0x17  
    "DestBearing     ",//0x18  
    "DestDistanceRef ",//0x19  
    "DestDistance    ",//0x1A  
    "ProcessingMethod",//0x1B  
    "AreaInformation ",//0x1C  
    "DateStamp       ",//0x1D  
    "Differential    ",//0x1E
};

// Exif Header
const unsigned char CMetadata::m_ExifHeader[EXIF_HEADER_SIZE] = {'E','x','i','f','\0','\0'};
const unsigned char CMetadata::m_XmpHeader[XMP_HEADER_SIZE] = "http://ns.adobe.com/xap/1.0/";
const unsigned char CMetadata::m_IccHeader[ICC_HEADER_SIZE] = "ICC_PROFILE";

// Exif User Comment Signature
const char CMetadata::m_USER_COMMENT_ASCII[USER_COMMENT_HEADER_SIZE] = {'A','S','C','I','I',0,0,0};
const char CMetadata::m_USER_COMMENT_JIS[USER_COMMENT_HEADER_SIZE] = {0x4A,0x49,0x53,0x00,0x00,0x00,0x00,0x00};
const char CMetadata::m_USER_COMMENT_UNICODE[USER_COMMENT_HEADER_SIZE] = {0x55,0x4E,0x49,0x43,0x4F,0x44,0x45,0x00};
const char CMetadata::m_USER_COMMENT_UNDEFINED[USER_COMMENT_HEADER_SIZE] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

// Iptc Legacy UTF-8 Invocation
const unsigned char CMetadata::m_UTF8Invocation[UTF8_INVOCATION_SIZE] = {'\x1b','\x25','\x47'};

#define DIR_ENTRY_ADDR(Start, Entry) (Start+2+12*(Entry))

CMetadata::CMetadata()
{
	// Init
	m_bMotorolaOrder = false;
	m_pAllocatedThumbnailPointer = NULL;
	m_uiMaxOffset = 0;

	// Jpeg Sections 
	m_nSectionsRead = 0;
	memset(&m_Sections, 0, MAX_JPEG_SECTIONS * sizeof(JPEGSECTION));
	m_pAfterSOSData = NULL;
	m_dwAfterSOSSize = 0;

	// Exif Metadata
	m_nExifImageWidth = 0;
	memset(&m_ExifInfo, 0, sizeof(EXIFINFO));
	m_ExifInfo.WhiteBalance = -1;
	m_ExifInfo.Flash = -1;
	m_ExifInfo.AmbientTemperature = NAN;
	m_ExifInfo.GpsLat[GPS_DEGREE] = -1.0f;
	m_ExifInfo.GpsLat[GPS_MINUTES] = -1.0f;
	m_ExifInfo.GpsLat[GPS_SECONDS] = -1.0f;
	m_ExifInfo.GpsLong[GPS_DEGREE] = -1.0f;
	m_ExifInfo.GpsLong[GPS_MINUTES] = -1.0f;
	m_ExifInfo.GpsLong[GPS_SECONDS] = -1.0f;
	m_ExifInfo.GpsAlt = -1.0f;
	m_ExifInfo.GpsAltRef = -1;
	m_ExifInfo.GpsTime[GPS_HOUR] = -1.0f;
	m_ExifInfo.GpsTime[GPS_MINUTES] = -1.0f;
	m_ExifInfo.GpsTime[GPS_SECONDS] = -1.0f;
	memset(&m_ExifInfoWrite, 0, sizeof(EXIFINFOINPLACEWRITE));
	m_bDoWrite = false;

	// Other Metadata
	m_bHasJfif = false;
	m_bHasCom = false;
	m_sJpegComment = "";
	m_bHasOtherAppSections = false;
	m_pXmpData = NULL;
	m_dwXmpSize = 0;
	m_pIptcLegacyData = NULL;
	m_dwIptcLegacySize = 0;
	m_pIptcLegacyPreData = NULL;
	m_dwIptcLegacyPreSize = 0;
	m_pIptcLegacyPostData = NULL;
	m_dwIptcLegacyPostSize = 0;
	m_pIccData = NULL;
	m_dwIccSize = 0;
	m_pPhotoshopData = NULL;
	m_dwPhotoshopSize = 0;
}

CMetadata::CMetadata(const CMetadata& metadata) // Copy Constructor (CMetadata Metadata1 = Metadata2 or CMetadata Metadata1(Metadata2))
{
	// Copy Vars
	m_bDoWrite = metadata.m_bDoWrite;
	m_bMotorolaOrder = metadata.m_bMotorolaOrder;
	m_bHasJfif = metadata.m_bHasJfif;
	m_bHasCom = metadata.m_bHasCom;
	m_sJpegComment = metadata.m_sJpegComment;
	m_bHasOtherAppSections = metadata.m_bHasOtherAppSections;
	memcpy(&m_ExifInfo, &metadata.m_ExifInfo, sizeof(EXIFINFO));
	memcpy(&m_ExifInfoWrite, &metadata.m_ExifInfoWrite, sizeof(EXIFINFOINPLACEWRITE));
	m_nExifImageWidth = metadata.m_nExifImageWidth;
	m_IptcLegacyInfo = metadata.m_IptcLegacyInfo;
	m_IptcFromXmpInfo = metadata.m_IptcFromXmpInfo;
	m_XmpInfo = metadata.m_XmpInfo;
	m_nSectionsRead = metadata.m_nSectionsRead;
	memcpy(&m_Sections, &metadata.m_Sections, MAX_JPEG_SECTIONS * sizeof(JPEGSECTION));
	if (metadata.m_pAllocatedThumbnailPointer)
	{
		m_pAllocatedThumbnailPointer = new BYTE[metadata.m_ExifInfo.ThumbnailSize];
		memcpy(	m_pAllocatedThumbnailPointer,
				metadata.m_pAllocatedThumbnailPointer,
				metadata.m_ExifInfo.ThumbnailSize); 
	}
	else
		m_pAllocatedThumbnailPointer = NULL;

	// Copy Other Metadata
	if (metadata.m_pXmpData && metadata.m_dwXmpSize > 0)
	{
		m_pXmpData = new BYTE[metadata.m_dwXmpSize];
		if (m_pXmpData == NULL)
			return;
		memcpy(m_pXmpData, metadata.m_pXmpData, metadata.m_dwXmpSize);
		m_dwXmpSize = metadata.m_dwXmpSize;
	}
	else
	{
		m_pXmpData = NULL;
		m_dwXmpSize = 0;
	}
	if (metadata.m_pIptcLegacyData && metadata.m_dwIptcLegacySize > 0)
	{
		DWORD dwDWordsCount = (metadata.m_dwIptcLegacySize % 4) ? (metadata.m_dwIptcLegacySize / 4) + 1 : metadata.m_dwIptcLegacySize / 4;
		m_pIptcLegacyData = (LPBYTE)(new DWORD[dwDWordsCount]);
		if (m_pIptcLegacyData == NULL)
			return;
		memset(m_pIptcLegacyData, 0, 4 * dwDWordsCount);
		memcpy(m_pIptcLegacyData, metadata.m_pIptcLegacyData, metadata.m_dwIptcLegacySize);
		m_dwIptcLegacySize = metadata.m_dwIptcLegacySize;
	}
	else
	{
		m_pIptcLegacyData = NULL;
		m_dwIptcLegacySize = 0;
	}
	if (metadata.m_pIptcLegacyPreData && metadata.m_dwIptcLegacyPreSize > 0)
	{
		m_pIptcLegacyPreData = new BYTE[metadata.m_dwIptcLegacyPreSize];
		if (m_pIptcLegacyPreData == NULL)
			return;
		memcpy(m_pIptcLegacyPreData, metadata.m_pIptcLegacyPreData, metadata.m_dwIptcLegacyPreSize);
		m_dwIptcLegacyPreSize = metadata.m_dwIptcLegacyPreSize;
	}
	else
	{
		m_pIptcLegacyPreData = NULL;
		m_dwIptcLegacyPreSize = 0;
	}
	if (metadata.m_pIptcLegacyPostData && metadata.m_dwIptcLegacyPostSize > 0)
	{
		m_pIptcLegacyPostData = new BYTE[metadata.m_dwIptcLegacyPostSize];
		if (m_pIptcLegacyPostData == NULL)
			return;
		memcpy(m_pIptcLegacyPostData, metadata.m_pIptcLegacyPostData, metadata.m_dwIptcLegacyPostSize);
		m_dwIptcLegacyPostSize = metadata.m_dwIptcLegacyPostSize;
	}
	else
	{
		m_pIptcLegacyPostData = NULL;
		m_dwIptcLegacyPostSize = 0;
	}
	if (metadata.m_pIccData && metadata.m_dwIccSize > 0)
	{
		m_pIccData = new BYTE[metadata.m_dwIccSize];
		if (m_pIccData == NULL)
			return;
		memcpy(m_pIccData, metadata.m_pIccData, metadata.m_dwIccSize);
		m_dwIccSize = metadata.m_dwIccSize;
	}
	else
	{
		m_pIccData = NULL;
		m_dwIccSize = 0;
	}
	if (metadata.m_pPhotoshopData && metadata.m_dwPhotoshopSize > 0)
	{
		m_pPhotoshopData = new BYTE[metadata.m_dwPhotoshopSize];
		if (m_pPhotoshopData == NULL)
			return;
		memcpy(m_pPhotoshopData, metadata.m_pPhotoshopData, metadata.m_dwPhotoshopSize);
		m_dwPhotoshopSize = metadata.m_dwPhotoshopSize;
	}
	else
	{
		m_pPhotoshopData = NULL;
		m_dwPhotoshopSize = 0;
	}
}

CMetadata& CMetadata::operator=(const CMetadata& metadata) // Copy Assignment (CMetadata Metadata3; Metadata3 = Metadata1)
{
	if (this != &metadata) // beware of self-assignment!
	{
		// Clean the object
		Free();

		// Copy Vars
		m_bDoWrite = metadata.m_bDoWrite;
		m_bMotorolaOrder = metadata.m_bMotorolaOrder;
		m_bHasJfif = metadata.m_bHasJfif;
		m_bHasCom = metadata.m_bHasCom;
		m_sJpegComment = metadata.m_sJpegComment;
		m_bHasOtherAppSections = metadata.m_bHasOtherAppSections;
		memcpy(&m_ExifInfo, &metadata.m_ExifInfo, sizeof(EXIFINFO));
		memcpy(&m_ExifInfoWrite, &metadata.m_ExifInfoWrite, sizeof(EXIFINFOINPLACEWRITE));
		m_nExifImageWidth = metadata.m_nExifImageWidth;
		m_IptcLegacyInfo = metadata.m_IptcLegacyInfo;
		m_IptcFromXmpInfo = metadata.m_IptcFromXmpInfo;
		m_XmpInfo = metadata.m_XmpInfo;
		m_nSectionsRead = metadata.m_nSectionsRead;
		memcpy(&m_Sections, &metadata.m_Sections, MAX_JPEG_SECTIONS * sizeof(JPEGSECTION));
		if (metadata.m_pAllocatedThumbnailPointer)
		{
			m_pAllocatedThumbnailPointer = new BYTE[metadata.m_ExifInfo.ThumbnailSize];
			memcpy(	m_pAllocatedThumbnailPointer,
					metadata.m_pAllocatedThumbnailPointer,
					metadata.m_ExifInfo.ThumbnailSize); 
		}
		else
			m_pAllocatedThumbnailPointer = NULL;

		// Copy Other Metadata
		if (metadata.m_pXmpData && metadata.m_dwXmpSize > 0)
		{
			m_pXmpData = new BYTE[metadata.m_dwXmpSize];
			if (m_pXmpData == NULL)
				return *this;
			memcpy(m_pXmpData, metadata.m_pXmpData, metadata.m_dwXmpSize);
			m_dwXmpSize = metadata.m_dwXmpSize;
		}
		else
		{
			m_pXmpData = NULL;
			m_dwXmpSize = 0;
		}
		if (metadata.m_pIptcLegacyData && metadata.m_dwIptcLegacySize > 0)
		{
			DWORD dwDWordsCount = (metadata.m_dwIptcLegacySize % 4) ? (metadata.m_dwIptcLegacySize / 4) + 1 : metadata.m_dwIptcLegacySize / 4;
			m_pIptcLegacyData = (LPBYTE)(new DWORD[dwDWordsCount]);
			if (m_pIptcLegacyData == NULL)
				return *this;
			memset(m_pIptcLegacyData, 0, 4 * dwDWordsCount);
			memcpy(m_pIptcLegacyData, metadata.m_pIptcLegacyData, metadata.m_dwIptcLegacySize);
			m_dwIptcLegacySize = metadata.m_dwIptcLegacySize;
		}
		else
		{
			m_pIptcLegacyData = NULL;
			m_dwIptcLegacySize = 0;
		}
		if (metadata.m_pIptcLegacyPreData && metadata.m_dwIptcLegacyPreSize > 0)
		{
			m_pIptcLegacyPreData = new BYTE[metadata.m_dwIptcLegacyPreSize];
			if (m_pIptcLegacyPreData == NULL)
				return *this;
			memcpy(m_pIptcLegacyPreData, metadata.m_pIptcLegacyPreData, metadata.m_dwIptcLegacyPreSize);
			m_dwIptcLegacyPreSize = metadata.m_dwIptcLegacyPreSize;
		}
		else
		{
			m_pIptcLegacyPreData = NULL;
			m_dwIptcLegacyPreSize = 0;
		}
		if (metadata.m_pIptcLegacyPostData && metadata.m_dwIptcLegacyPostSize > 0)
		{
			m_pIptcLegacyPostData = new BYTE[metadata.m_dwIptcLegacyPostSize];
			if (m_pIptcLegacyPostData == NULL)
				return *this;
			memcpy(m_pIptcLegacyPostData, metadata.m_pIptcLegacyPostData, metadata.m_dwIptcLegacyPostSize);
			m_dwIptcLegacyPostSize = metadata.m_dwIptcLegacyPostSize;
		}
		else
		{
			m_pIptcLegacyPostData = NULL;
			m_dwIptcLegacyPostSize = 0;
		}
		if (metadata.m_pIccData && metadata.m_dwIccSize > 0)
		{
			m_pIccData = new BYTE[metadata.m_dwIccSize];
			if (m_pIccData == NULL)
				return *this;
			memcpy(m_pIccData, metadata.m_pIccData, metadata.m_dwIccSize);
			m_dwIccSize = metadata.m_dwIccSize;
		}
		else
		{
			m_pIccData = NULL;
			m_dwIccSize = 0;
		}
		if (metadata.m_pPhotoshopData && metadata.m_dwPhotoshopSize > 0)
		{
			m_pPhotoshopData = new BYTE[metadata.m_dwPhotoshopSize];
			if (m_pPhotoshopData == NULL)
				return *this;
			memcpy(m_pPhotoshopData, metadata.m_pPhotoshopData, metadata.m_dwPhotoshopSize);
			m_dwPhotoshopSize = metadata.m_dwPhotoshopSize;
		}
		else
		{
			m_pPhotoshopData = NULL;
			m_dwPhotoshopSize = 0;
		}
	}
	return *this;
}

CMetadata::~CMetadata()
{
	Free();
}

void CMetadata::Free()
{
	if (m_pAllocatedThumbnailPointer)
	{
		delete [] m_pAllocatedThumbnailPointer;
		m_pAllocatedThumbnailPointer = NULL;
	}
	m_bHasJfif = false;
	m_bHasCom = false;
	m_sJpegComment = "";
	m_bHasOtherAppSections = false;
	m_nExifImageWidth = 0;
	m_bMotorolaOrder = false;
	m_nSectionsRead = 0;
	memset(&m_Sections, 0, MAX_JPEG_SECTIONS * sizeof(JPEGSECTION));
	memset(&m_ExifInfo, 0, sizeof(EXIFINFO));
	m_ExifInfo.WhiteBalance = -1;
	m_ExifInfo.Flash = -1;
	m_ExifInfo.AmbientTemperature = NAN;
	m_ExifInfo.GpsLat[GPS_DEGREE] = -1.0f;
	m_ExifInfo.GpsLat[GPS_MINUTES] = -1.0f;
	m_ExifInfo.GpsLat[GPS_SECONDS] = -1.0f;
	m_ExifInfo.GpsLong[GPS_DEGREE] = -1.0f;
	m_ExifInfo.GpsLong[GPS_MINUTES] = -1.0f;
	m_ExifInfo.GpsLong[GPS_SECONDS] = -1.0f;
	m_ExifInfo.GpsAlt = -1.0f;
	m_ExifInfo.GpsAltRef = -1;
	m_ExifInfo.GpsTime[GPS_HOUR] = -1.0f;
	m_ExifInfo.GpsTime[GPS_MINUTES] = -1.0f;
	m_ExifInfo.GpsTime[GPS_SECONDS] = -1.0f;
	memset(&m_ExifInfoWrite, 0, sizeof(EXIFINFOINPLACEWRITE));
	m_IptcLegacyInfo.Clear();
	m_IptcFromXmpInfo.Clear();
	m_XmpInfo.Clear();
	m_bDoWrite = false;
	m_uiMaxOffset = 0;

	// Free Other Metadata
	if (m_pXmpData)
	{
		delete [] m_pXmpData;
		m_pXmpData = NULL;
	}
	m_dwXmpSize = 0;
	if (m_pIptcLegacyData)
	{
		delete [] m_pIptcLegacyData;
		m_pIptcLegacyData = NULL;
	}
	m_dwIptcLegacySize = 0;
	if (m_pIptcLegacyPreData)
	{
		delete [] m_pIptcLegacyPreData;
		m_pIptcLegacyPreData = NULL;
	}
	m_dwIptcLegacyPreSize = 0;
	if (m_pIptcLegacyPostData)
	{
		delete [] m_pIptcLegacyPostData;
		m_pIptcLegacyPostData = NULL;
	}
	m_dwIptcLegacyPostSize = 0;
	if (m_pIccData)
	{
		delete [] m_pIccData;
		m_pIccData = NULL;
	}
	m_dwIccSize = 0;
	if (m_pPhotoshopData)
	{
		delete [] m_pPhotoshopData;
		m_pPhotoshopData = NULL;
	}
	m_dwPhotoshopSize = 0;
}

COleDateTime CMetadata::GetOleDateTimeFromExifString(const CString& sExifDateTime)
{
	int nPrevPos = -1;
	int nPos = 0;
	CString t;
	int nDate[6] = {1971,1,1,12,0,0};
	for (int i = 0 ; i < 6 ; i++)
	{
		if (i == 5)
			nPos = sExifDateTime.GetLength();
		else if ((nPos = sExifDateTime.Find((i == 2) ? _T(' ') : _T(':'), nPrevPos + 1)) < 0)
			break;
		t = sExifDateTime.Mid(nPrevPos + 1, nPos - nPrevPos - 1);
		nDate[i] = _ttoi(t);
		nPrevPos = nPos;
	}

	// Validate
	if (nDate[0] < 100 || nDate[0] > 9999)
		nDate[0] = 100;
	if (nDate[1] < 1 || nDate[1] > 12)
		nDate[1] = 1;
	if (nDate[2] < 1 || nDate[2] > 31)
		nDate[2] = 1;

	return COleDateTime(nDate[0], nDate[1], nDate[2], nDate[3], nDate[4], nDate[5]);
}

CTime CMetadata::GetDateTimeFromExifString(const CString& sExifDateTime)
{
	int nPrevPos = -1;
	int nPos = 0;
	CString t;
	int nDate[6] = {1971,1,1,12,0,0};
	for (int i = 0 ; i < 6 ; i++)
	{
		if (i == 5)
			nPos = sExifDateTime.GetLength();
		else if ((nPos = sExifDateTime.Find((i == 2) ? _T(' ') : _T(':'), nPrevPos + 1)) < 0)
			break;
		t = sExifDateTime.Mid(nPrevPos + 1, nPos - nPrevPos - 1);
		nDate[i] = _ttoi(t);
		nPrevPos = nPos;
	}

	// Validate
	if (nDate[0] < 1971 || nDate[0] > 3000) // MFC CTime Limitation (January 1, 1970 00:00:00 crashes also...)
		nDate[0] = 1971;
	if (nDate[1] < 1 || nDate[1] > 12)
		nDate[1] = 1;
	if (nDate[2] < 1 || nDate[2] > 31)
		nDate[2] = 1;

	return CTime(nDate[0], nDate[1], nDate[2], nDate[3], nDate[4], nDate[5]);
}

void CMetadata::FillExifOleDateTimeString(const COleDateTime& Time, char* DateTime)
{
	int nYear = Time.GetYear();
	if (nYear < 0)
		nYear = 0;
	else if (nYear > 9999)
		nYear = 9999;
	int nMonth = Time.GetMonth();
	if (nMonth < 1)
		nMonth = 1;
	else if (nMonth > 12)
		nMonth = 12;
	int nDay = Time.GetDay();
	if (nDay < 1)
		nDay = 1;
	else if (nDay > 31)
		nDay = 31;
	int nHour = Time.GetHour();
	if (nHour < 0)
		nHour = 0;
	else if (nHour > 23)
		nHour = 23;
	int nMinute = Time.GetMinute();
	if (nMinute < 0)
		nMinute = 0;
	else if (nMinute > 59)
		nMinute = 59;
	int nSecond = Time.GetSecond();
	if (nSecond < 0)
		nSecond = 0;
	else if (nSecond > 59)
		nSecond = 59;
	_snprintf(	DateTime, 19, "%04d:%02d:%02d %02d:%02d:%02d",
				nYear, nMonth, nDay,
				nHour, nMinute, nSecond);
	DateTime[19] = '\0';
}

void CMetadata::FillExifDateTimeString(const CTime& Time, char* DateTime)
{
	int nYear = Time.GetYear();
	if (nYear < 0)
		nYear = 0;
	else if (nYear > 9999)
		nYear = 9999;
	int nMonth = Time.GetMonth();
	if (nMonth < 1)
		nMonth = 1;
	else if (nMonth > 12)
		nMonth = 12;
	int nDay = Time.GetDay();
	if (nDay < 1)
		nDay = 1;
	else if (nDay > 31)
		nDay = 31;
	int nHour = Time.GetHour();
	if (nHour < 0)
		nHour = 0;
	else if (nHour > 23)
		nHour = 23;
	int nMinute = Time.GetMinute();
	if (nMinute < 0)
		nMinute = 0;
	else if (nMinute > 59)
		nMinute = 59;
	int nSecond = Time.GetSecond();
	if (nSecond < 0)
		nSecond = 0;
	else if (nSecond > 59)
		nSecond = 59;
	_snprintf(	DateTime, 19, "%04d:%02d:%02d %02d:%02d:%02d",
				nYear, nMonth, nDay,
				nHour, nMinute, nSecond);
	DateTime[19] = '\0';
}

CTime CMetadata::GetDateFromIptcLegacyString(const CString& sIptcDate)
{
	CString t;
	int i;
	int nDate[6] = {1971,1,1,12,0,0};

	if (sIptcDate.GetLength() >= 8)
	{
		// Year
		t = sIptcDate.Left(4);
		i = _ttoi(t);
		if (i >= 1971 && i <= 3000) // MFC CTime Limitation (January 1, 1970 00:00:00 crashes also...)
			nDate[0] = i;

		// Month
		t = sIptcDate.Mid(4, 2);
		i = _ttoi(t);
		if (i >= 1 && i <= 12)
			nDate[1] = i;

		// Day
		t = sIptcDate.Mid(6, 2);
		i = _ttoi(t);
		if (i >= 1 && i <= 31)
			nDate[2] = i;
	}

	CTime Time(nDate[0], nDate[1], nDate[2], nDate[3], nDate[4], nDate[5]);
	return Time;
}

CTime CMetadata::GetDateFromXmpString(const CString& sXmpDate)
{
	CString t;
	int i;
	int nDate[6] = {1971,1,1,12,0,0};

	if (sXmpDate.GetLength() >= 10)
	{
		// Year
		t = sXmpDate.Left(4);
		i = _ttoi(t);
		if (i >= 1971 && i <= 3000) // MFC CTime Limitation (January 1, 1970 00:00:00 crashes also...)
			nDate[0] = i;

		// Month
		t = sXmpDate.Mid(5, 2);
		i = _ttoi(t);
		if (i >= 1 && i <= 12)
			nDate[1] = i;

		// Day
		t = sXmpDate.Mid(8, 2);
		i = _ttoi(t);
		if (i >= 1 && i <= 31)
			nDate[2] = i;
	}

	CTime Time(nDate[0], nDate[1], nDate[2], nDate[3], nDate[4], nDate[5]);
	return Time;
}

CMetadata::JPEGSECTION* CMetadata::GetSection(int nSection)
{
	return ((nSection < m_nSectionsRead && nSection >= 0) ?
			&m_Sections[nSection] :
			NULL);
}

int CMetadata::RemoveExifThumbnail(	LPBYTE Section,
									int nSectionSize)
{
	// Check
	if (!Section || nSectionSize < 14)
		return -1;

	// All Offset are relative to the Tiff Header Start
	DWORD Base = 6;

	// Check the EXIF header component
    if (memcmp(Section, m_ExifHeader, EXIF_HEADER_SIZE))
		return -1;

	// Get the Byte Ordering
    if (memcmp(Section+6,"II",2) == 0)
        m_bMotorolaOrder = false;
	else
	{
        if (memcmp(Section+6,"MM",2) == 0)
            m_bMotorolaOrder = true;
		else
			return -1;
    }

    // Check the next two values for correctness
    if (Get16u(Section+8) != 0x2a)
		return -1;

	// IFD0 Offset
	DWORD dwIFD0Offset = Get32u(Section+10);
    if (dwIFD0Offset < 8 || dwIFD0Offset > 16)
		return -1;

	// Get Number Of Entries
	WORD wNumDirEntries = Get16u(Section + Base + dwIFD0Offset);

	// IFD1 Offset Position
	DWORD dwIFD1OffsetPos = dwIFD0Offset + 2 + wNumDirEntries*12;

	// IFD1 Offset
	DWORD dwIFD1Offset = Get32u(Section + Base + dwIFD1OffsetPos);
	if (dwIFD1Offset == 0)
		return -1;

	// Set IFD1 Offset to 0
	Put32u(Section + Base + dwIFD1OffsetPos, 0);

	
	//////////
	// IFD1 //
	//////////

	// Get Number Of Entries
	wNumDirEntries = Get16u(Section + Base + dwIFD1Offset);

	// Find Thumbnail Offset
	unsigned int ThumbnailOffset = 0;
	for (WORD de = 0 ; de < wNumDirEntries ; de++)
	{
        int Tag, Format, Components;
        unsigned char* DirEntry = Section + Base + dwIFD1Offset + 2 + 12*de;
        Tag = Get16u(DirEntry);
        Format = Get16u(DirEntry+2);
        Components = Get32u(DirEntry+4);
		if (Tag == TAG_JPEGIFOFFSET ||	// Jpeg Thumb
			Tag == TAG_STRIPOFFSETS)	// Tiff Thumb
		{
			ThumbnailOffset = (unsigned int)ConvertFromAnyFormat(DirEntry+8, Format);
			break;
		}
	}

	// IFD1 Size
	DWORD dwIFD1Size = 2 + wNumDirEntries*12 + 4;

	// Clear-It
	memset(Section + Base + dwIFD1Offset, 0, dwIFD1Size);

	// If IFD1 is just before Thumbnail Data, remove it also!
	if (dwIFD1Offset + dwIFD1Size == ThumbnailOffset)
		return (Base + dwIFD1Offset);
	// Just Clear IFD1 Entry and remove Thumbnail Data
	else
		return (Base + ThumbnailOffset);
}

#define POSCHECK(pos) if ((pos) >= nMaxSectionSize) return -1;

int CMetadata::AppendExifThumbnail(	LPBYTE Section,
									int nSectionSize,
									int nMaxSectionSize,
									LPBYTE pJpegThumbData,
									DWORD dwJpegThumbSize)
{
	// Check
	if (!Section || nSectionSize < 14)
		return -1;

	// All Offset are relative to the Tiff Header Start
	DWORD Base = 6;

	// Check the EXIF header component
    if (memcmp(Section, m_ExifHeader, EXIF_HEADER_SIZE))
		return -1;

	// Get the Byte Ordering
    if (memcmp(Section+6,"II",2) == 0)
        m_bMotorolaOrder = false;
	else
	{
        if (memcmp(Section+6,"MM",2) == 0)
            m_bMotorolaOrder = true;
		else
			return -1;
    }

    // Check the next two values for correctness
    if (Get16u(Section+8) != 0x2a)
		return -1;

	// IFD0 Offset
	DWORD dwIFD0Offset = Get32u(Section+10);
    if (dwIFD0Offset < 8 || dwIFD0Offset > 16)
		return -1;

	// Get Number Of Entries
	WORD wNumDirEntries = Get16u(Section + Base + dwIFD0Offset);

	// IFD1 Offset Position
	DWORD dwIFD1OffsetPos = dwIFD0Offset + 2 + wNumDirEntries*12;

	// IFD1 Offset
	DWORD dwIFD1Offset = Get32u(Section + Base + dwIFD1OffsetPos);
	if (dwIFD1Offset != 0)
		return -1; // Has already a Thumbnail!

	// Set IFD1 Offset after the end of the passed section
	Put32u(Section + Base + dwIFD1OffsetPos, nSectionSize - Base);

	
	//////////
	// IFD1 //
	//////////

	// Directory Entry Count
	const int IFD1Entries = 3;

	// Init Pos
	int Pos = nSectionSize;

	// Total Size Check
	int nTotalSize = nSectionSize + 2 + 12 + 12 + 12 + 4 + dwJpegThumbSize;
	POSCHECK(nTotalSize-1);

	// Count
	Put16u(&Section[Pos], IFD1Entries);		// 3 Entries
	Pos += 2;

	// TAG_COMPRESSION Entry
	Put16u(&Section[Pos], TAG_COMPRESSION);// Tag
	Pos += 2;
	Put16u(&Section[Pos], FMT_USHORT);		// Type
	Pos += 2;
	Put32u(&Section[Pos], 1);				// Components Count
	Pos += 4;
	Put16u(&Section[Pos], 6);				// Value: Jpeg Compression
	Pos += 4;
	
	// TAG_JPEGIFBYTECOUNT Entry
	Put16u(&Section[Pos], TAG_JPEGIFBYTECOUNT);// Tag
	Pos += 2;
	Put16u(&Section[Pos], FMT_ULONG);		// Type
	Pos += 2;
	Put32u(&Section[Pos], 1);				// Components Count
	Pos += 4;
	Put32u(&Section[Pos], dwJpegThumbSize);	// Value
	Pos += 4;

	// TAG_JPEGIFOFFSET Entry
	Put16u(&Section[Pos], TAG_JPEGIFOFFSET);// Tag
	Pos += 2;
	Put16u(&Section[Pos], FMT_ULONG);		// Type
	Pos += 2;
	Put32u(&Section[Pos], 1);				// Components Count
	Pos += 4;
	Put32u(&Section[Pos], Pos + 8 - Base);	// Value
	Pos += 4;

	// No Offset
	Put32u(&Section[Pos], 0x00000000);		// Value
	Pos += 4;

	// Copy Thumbnail Data
	memcpy(&Section[Pos], pJpegThumbData, dwJpegThumbSize);
	Pos += dwJpegThumbSize;

	// Check
	ASSERT(Pos == nTotalSize);
	
	return Pos;
}

// Returns the written string length
// Returns 0 if string is empty
// Returns -1 if the Section buffer is to small
int CMetadata::WriteExifString(	int nTag,	
								const char* szTag,
								LPBYTE Section,
								int nMaxSectionSize,
								int Base,
								int& Pos,
								int& PosData)
{
	int Len = 1;
	if (szTag[0])
	{
		Len = strlen(szTag) + 1;
		Put16u(&Section[Pos], nTag);				// Tag
		Pos += 2;
		Put16u(&Section[Pos], FMT_STRING);			// Type
		Pos += 2;
		Put32u(&Section[Pos], Len);					// Components Count
		Pos += 4;
		if (Len > 4)
		{
			// Value Ptr
			Put32u(&Section[Pos], PosData - Base);
			Pos += 4;

			// Copy Data
			POSCHECK(PosData+Len-1);
			memcpy(&Section[PosData], szTag, Len);
			PosData += Len;
		}
		else
		{
			// Copy Data
			memset(&Section[Pos], 0, 4);
			memcpy(&Section[Pos], szTag, Len);
			Pos += 4;
		}
	}
	return Len - 1;
}

int CMetadata::MakeExifSection(LPBYTE Section,
							   int nMaxSectionSize,
							   DWORD dwImageWidth,
							   DWORD dwImageHeight,
							   LPBYTE pJpegThumbData,
							   DWORD dwJpegThumbSize)
{
	// Check
	if (!Section)
		return -1;

	// Current Position
	int Pos = 0;

	// All Offset are relative to the Tiff Header Start
	int Base = 6;

	// Exif Header
	POSCHECK(EXIF_HEADER_SIZE - 1);
	memcpy(Section, m_ExifHeader, EXIF_HEADER_SIZE);
	Pos += EXIF_HEADER_SIZE;

	// Tiff Header
	if (m_bMotorolaOrder)
	{
		POSCHECK(Pos);
		Section[Pos++] = 'M';
		POSCHECK(Pos);
		Section[Pos++] = 'M';
	}
	else
	{
		POSCHECK(Pos);
		Section[Pos++] = 'I';
		POSCHECK(Pos);
		Section[Pos++] = 'I';
	}
	POSCHECK(Pos+1);
	Put16u(&Section[Pos], 0x002A);
	Pos += 2;

	// Offset to IFD0
	POSCHECK(Pos+3);
	Put32u(&Section[Pos], 0x00000008);
	Pos += 4;

	// Minimal Directory Entries Count
	int IFD0Entries				= 2;
	int IFD0ExifSubDirEntries	= 3;
	int IFD0GpsSubDirEntries	= 0;
	int IFD1Entries				= (pJpegThumbData && dwJpegThumbSize > 0) ? 3 : 0;

	// Additional IFD0 entries
	if (m_ExifInfo.ImageDescription[0])
		IFD0Entries++;
	if (m_ExifInfo.CameraMake[0])
		IFD0Entries++;
	if (m_ExifInfo.CameraModel[0])
		IFD0Entries++;
	if (m_ExifInfo.Xresolution)
		IFD0Entries++;
	if (m_ExifInfo.Yresolution)
		IFD0Entries++;
	if (m_ExifInfo.ResolutionUnit)
		IFD0Entries++;
	if (m_ExifInfo.Software[0])
		IFD0Entries++;
	if (m_ExifInfo.Artist[0])
		IFD0Entries++;
	if (m_ExifInfo.CopyrightPhotographer[0])
		IFD0Entries++;

	// Additional EXIF SubDir entries
	if (m_ExifInfo.ExposureTime)
		IFD0ExifSubDirEntries++;
	if (m_ExifInfo.ApertureFNumber)
		IFD0ExifSubDirEntries++;
	if (m_ExifInfo.ExposureProgram)
		IFD0ExifSubDirEntries++;
	if (m_ExifInfo.ExposureMode)
		IFD0ExifSubDirEntries++;
	if (m_ExifInfo.DateTime[0])
		IFD0ExifSubDirEntries++;
	if (m_ExifInfo.Flash >= 0)
		IFD0ExifSubDirEntries++;
	if (m_ExifInfo.ISOequivalent)
		IFD0ExifSubDirEntries++;
	if (m_ExifInfo.FocalLength)
		IFD0ExifSubDirEntries++;
	if (m_ExifInfo.Distance)
		IFD0ExifSubDirEntries++;
	if (m_ExifInfo.ExposureBias)
		IFD0ExifSubDirEntries++;
	if (m_ExifInfo.DigitalZoomRatio)
		IFD0ExifSubDirEntries++;
	if (m_ExifInfo.FocalLength35mmEquiv)
		IFD0ExifSubDirEntries++;
	if (m_ExifInfo.DistanceRange)
		IFD0ExifSubDirEntries++;
	if (m_ExifInfo.WhiteBalance >= 0)
		IFD0ExifSubDirEntries++;
	if (m_ExifInfo.MeteringMode)
		IFD0ExifSubDirEntries++;
	if (m_ExifInfo.LightSource)
		IFD0ExifSubDirEntries++;
	if (m_ExifInfo.CompressionLevel)
		IFD0ExifSubDirEntries++;
	if (m_ExifInfo.FocalplaneXRes)
		IFD0ExifSubDirEntries++;
	if (m_ExifInfo.FocalplaneYRes)
		IFD0ExifSubDirEntries++;
	if (m_ExifInfo.FocalplaneUnits)
		IFD0ExifSubDirEntries++;
	if (m_ExifInfo.Brightness)
		IFD0ExifSubDirEntries++;
	if (!isnan(m_ExifInfo.AmbientTemperature))
		IFD0ExifSubDirEntries++;
	if (m_ExifInfo.UserComment[0])
		IFD0ExifSubDirEntries++;

	// Additional GPS SubDir entries
	if (m_ExifInfo.bGpsInfoPresent)
	{
		// One entry more for pointer to Gps SubDir
		IFD0Entries++;

		// For Gps Version
		IFD0GpsSubDirEntries++;

		if (m_ExifInfo.GpsLat[GPS_DEGREE]  >= 0.0f &&
			m_ExifInfo.GpsLat[GPS_MINUTES] >= 0.0f &&
			m_ExifInfo.GpsLat[GPS_SECONDS] >= 0.0f)
			IFD0GpsSubDirEntries++;
		if (m_ExifInfo.GpsLong[GPS_DEGREE]  >= 0.0f &&
			m_ExifInfo.GpsLong[GPS_MINUTES] >= 0.0f &&
			m_ExifInfo.GpsLong[GPS_SECONDS] >= 0.0f)
			IFD0GpsSubDirEntries++;
		if (m_ExifInfo.GpsAlt >= 0.0f)
			IFD0GpsSubDirEntries++;
		if (m_ExifInfo.GpsLatRef[0])
			IFD0GpsSubDirEntries++;
		if (m_ExifInfo.GpsLongRef[0])
			IFD0GpsSubDirEntries++;
		if (m_ExifInfo.GpsAltRef >= 0)
			IFD0GpsSubDirEntries++;
		if (m_ExifInfo.GpsTime[GPS_HOUR]    >= 0.0f &&
			m_ExifInfo.GpsTime[GPS_MINUTES] >= 0.0f &&
			m_ExifInfo.GpsTime[GPS_SECONDS] >= 0.0f)
			IFD0GpsSubDirEntries++;
		if (m_ExifInfo.GpsMapDatum[0])
			IFD0GpsSubDirEntries++;
	}

	// EXIF Dirs & SubDirs Sizes
	int IFD0DirSize =			2 + IFD0Entries * 12 + 4;
	int IFD0ExifSubDirSize =	2 + IFD0ExifSubDirEntries * 12 + 4;
	int IFD0GpsSubDirSize =		m_ExifInfo.bGpsInfoPresent ? 2 + IFD0GpsSubDirEntries * 12 + 4 : 0;
	int IFD1DirSize =			IFD1Entries > 0 ? 2 + IFD1Entries * 12 + 4 : 0;

	// Total Size of Directories
	int TotalSize =		IFD0DirSize	+
						IFD0ExifSubDirSize +
						IFD0GpsSubDirSize +
						IFD1DirSize;

	// Value Data Ptr
	int PosData = Pos + TotalSize;

	// Check Data Position
	POSCHECK(PosData);

	// Used to calc. rationals
	int nNum;
	int nDen;
	DWORD dwNum;
	DWORD dwDen;

	////////// 
	// IFD0 //
	//////////

	// Count
	Put16u(&Section[Pos], IFD0Entries);				// 2 Entries
	Pos += 2;

	// TAG_IMAGE_DESC
	if (WriteExifString(TAG_IMAGE_DESC,
						m_ExifInfo.ImageDescription,
						Section,
						nMaxSectionSize,
						Base,
						Pos,
						PosData) < 0)
		return -1;

	// TAG_MAKE
	if (WriteExifString(TAG_MAKE,
						m_ExifInfo.CameraMake,
						Section,
						nMaxSectionSize,
						Base,
						Pos,
						PosData) < 0)
		return -1;

	// TAG_MODEL
	if (WriteExifString(TAG_MODEL,
						m_ExifInfo.CameraModel,
						Section,
						nMaxSectionSize,
						Base,
						Pos,
						PosData) < 0)
		return -1;

	// TAG_ORIENTATION Entry
	Put16u(&Section[Pos], TAG_ORIENTATION);			// Tag
	Pos += 2;
	Put16u(&Section[Pos], FMT_USHORT);				// Type
	Pos += 2;
	Put32u(&Section[Pos], 1);						// Components Count
	Pos += 4;
	Put16u(&Section[Pos], m_ExifInfo.Orientation);	// Value
	Pos += 4;

	// TAG_XRESOLUTION
	if (m_ExifInfo.Xresolution)
	{
		Put16u(&Section[Pos], TAG_XRESOLUTION);		// Tag
		Pos += 2;
		Put16u(&Section[Pos], FMT_URATIONAL);		// Type
		Pos += 2;
		Put32u(&Section[Pos], 1);					// Components Count
		Pos += 4;
		Put32u(&Section[Pos], PosData - Base);		// Value Ptr
		Pos += 4;

		// Copy Data
		POSCHECK(PosData+7);
		dwNum = (DWORD)(m_ExifInfo.Xresolution * 10000.0f);
		dwDen = 10000;
		Put32u((void*)(&Section[PosData]), dwNum);
		PosData += 4;
		Put32u((void*)(&Section[PosData]), dwDen);
		PosData += 4;
	}

	// TAG_YRESOLUTION
	if (m_ExifInfo.Yresolution)
	{
		Put16u(&Section[Pos], TAG_YRESOLUTION);		// Tag
		Pos += 2;
		Put16u(&Section[Pos], FMT_URATIONAL);		// Type
		Pos += 2;
		Put32u(&Section[Pos], 1);					// Components Count
		Pos += 4;
		Put32u(&Section[Pos], PosData - Base);		// Value Ptr
		Pos += 4;

		// Copy Data
		POSCHECK(PosData+7);
		dwNum = (DWORD)(m_ExifInfo.Yresolution * 10000.0f);
		dwDen = 10000;
		Put32u((void*)(&Section[PosData]), dwNum);
		PosData += 4;
		Put32u((void*)(&Section[PosData]), dwDen);
		PosData += 4;
	}

	// TAG_RESOLUTIONUNIT
	if (m_ExifInfo.ResolutionUnit)
	{
		int ResolutionUnit;
		if (m_ExifInfo.ResolutionUnit == 1.0f)
			ResolutionUnit = 2;
		else if (m_ExifInfo.ResolutionUnit == 0.393701f)
			ResolutionUnit = 3;
		else if (m_ExifInfo.ResolutionUnit == 0.0393701f)
			ResolutionUnit = 4;
		else if (m_ExifInfo.ResolutionUnit == 0.0000393701f)
			ResolutionUnit = 5;
		else
			ResolutionUnit = 1;

		Put16u(&Section[Pos], TAG_RESOLUTIONUNIT);	// Tag
		Pos += 2;
		Put16u(&Section[Pos], FMT_USHORT);			// Type
		Pos += 2;
		Put32u(&Section[Pos], 1);					// Components Count
		Pos += 4;
		Put16u(&Section[Pos], ResolutionUnit);		// Value
		Pos += 4;
	}

	// TAG_SOFTWARE
	if (WriteExifString(TAG_SOFTWARE,
						m_ExifInfo.Software,
						Section,
						nMaxSectionSize,
						Base,
						Pos,
						PosData) < 0)
		return -1;

	// TAG_ARTIST
	if (WriteExifString(TAG_ARTIST,
						m_ExifInfo.Artist,
						Section,
						nMaxSectionSize,
						Base,
						Pos,
						PosData) < 0)
		return -1;

	// TAG_COPYRIGHT
	if (m_ExifInfo.CopyrightPhotographer[0])
	{
		int LenPhoto = strlen(m_ExifInfo.CopyrightPhotographer) + 1;
		int LenEditor = 0;
		if (m_ExifInfo.CopyrightEditor[0])
			LenEditor = strlen(m_ExifInfo.CopyrightEditor) + 1;
		int Len = LenPhoto + LenEditor;
		Put16u(&Section[Pos], TAG_COPYRIGHT);		// Tag
		Pos += 2;
		Put16u(&Section[Pos], FMT_STRING);			// Type
		Pos += 2;
		Put32u(&Section[Pos], Len);					// Components Count
		Pos += 4;
		if (Len > 4)
		{
			// Value Ptr
			Put32u(&Section[Pos], PosData - Base);
			Pos += 4;

			// Copy Data
			POSCHECK(PosData+LenPhoto-1);
			memcpy(&Section[PosData], m_ExifInfo.CopyrightPhotographer, LenPhoto);
			PosData += LenPhoto;
			if (m_ExifInfo.CopyrightEditor[0])
			{
				POSCHECK(PosData+LenEditor-1);
				memcpy(&Section[PosData], m_ExifInfo.CopyrightEditor, LenEditor);
				PosData += LenEditor;
			}
		}
		else
		{
			// Copy Data
			memset(&Section[Pos], 0, 4);
			memcpy(&Section[Pos], m_ExifInfo.CopyrightPhotographer, LenPhoto);
			Pos += LenPhoto;
			if (m_ExifInfo.CopyrightEditor[0])
			{
				memcpy(&Section[Pos], m_ExifInfo.CopyrightEditor, LenEditor);
				Pos += LenEditor;
			}
			Pos += 4 - Len;
		}
	}

	// TAG_GPS_SUBIFD Entry (Offset to IFD0 Gps SubDir)
	if (IFD0GpsSubDirEntries > 0)
	{
		Put16u(&Section[Pos], TAG_GPS_SUBIFD);			// Tag
		Pos += 2;
		Put16u(&Section[Pos], FMT_ULONG);				// Type
		Pos += 2;
		Put32u(&Section[Pos], 1);						// Components Count
		Pos += 4;
		Put32u(&Section[Pos], Pos + 20 + IFD0ExifSubDirSize - Base); // Value
		Pos += 4;
	}

	// TAG_EXIF_SUBIFD Entry (Offset to IFD0 Exif SubDir)
	Put16u(&Section[Pos], TAG_EXIF_SUBIFD);			// Tag
	Pos += 2;
	Put16u(&Section[Pos], FMT_ULONG);				// Type
	Pos += 2;
	Put32u(&Section[Pos], 1);						// Components Count
	Pos += 4;
	Put32u(&Section[Pos], Pos + 8 - Base);			// Value
	Pos += 4;

	// Offset to 1st IFD
	if (IFD1Entries > 0)
		Put32u(&Section[Pos], Pos + 4 + IFD0ExifSubDirSize + IFD0GpsSubDirSize - Base);	// Value
	else
		Put32u(&Section[Pos], 0x00000000);			// Value
	Pos += 4;
	

	//////////////////////
	// IFD0 Exif SubDir //
	//////////////////////

	// Count
	Put16u(&Section[Pos], IFD0ExifSubDirEntries);
	Pos += 2;

	// TAG_EXPOSURETIME
	if (m_ExifInfo.ExposureTime)
	{
		Put16u(&Section[Pos], TAG_EXPOSURETIME);	// Tag
		Pos += 2;
		Put16u(&Section[Pos], FMT_URATIONAL);		// Type
		Pos += 2;
		Put32u(&Section[Pos], 1);					// Components Count
		Pos += 4;
		Put32u(&Section[Pos], PosData - Base);		// Value Ptr
		Pos += 4;

		// Copy Data
		POSCHECK(PosData+7);
		if (m_ExifInfo.ExposureTime < 0.1f)
		{
			dwNum = (DWORD)(m_ExifInfo.ExposureTime * 10000000.0f);
			dwDen = 10000000;
		}
		else if (m_ExifInfo.ExposureTime < 1.0f)
		{
			dwNum = (DWORD)(m_ExifInfo.ExposureTime * 1000000.0f);
			dwDen = 1000000;
		}
		else if (m_ExifInfo.ExposureTime < 10.0f)
		{
			dwNum = (DWORD)(m_ExifInfo.ExposureTime * 100000.0f);
			dwDen = 100000;
		}
		else
		{
			dwNum = (DWORD)(m_ExifInfo.ExposureTime * 10000.0f);
			dwDen = 10000;
		}
		Put32u((void*)(&Section[PosData]), dwNum);
		PosData += 4;
		Put32u((void*)(&Section[PosData]), dwDen);
		PosData += 4;
	}

	// TAG_FNUMBER
	if (m_ExifInfo.ApertureFNumber)
	{
		Put16u(&Section[Pos], TAG_FNUMBER);			// Tag
		Pos += 2;
		Put16u(&Section[Pos], FMT_URATIONAL);		// Type
		Pos += 2;
		Put32u(&Section[Pos], 1);					// Components Count
		Pos += 4;
		Put32u(&Section[Pos], PosData - Base);		// Value Ptr
		Pos += 4;

		// Copy Data
		POSCHECK(PosData+7);
		dwNum = (DWORD)(m_ExifInfo.ApertureFNumber * 10000.0f);
		dwDen = 10000;
		Put32u((void*)(&Section[PosData]), dwNum);
		PosData += 4;
		Put32u((void*)(&Section[PosData]), dwDen);
		PosData += 4;
	}

	// TAG_EXPOSURE_PROGRAM
	if (m_ExifInfo.ExposureProgram)
	{
		Put16u(&Section[Pos], TAG_EXPOSURE_PROGRAM);	// Tag
		Pos += 2;
		Put16u(&Section[Pos], FMT_USHORT);				// Type
		Pos += 2;
		Put32u(&Section[Pos], 1);						// Components Count
		Pos += 4;
		Put16u(&Section[Pos], m_ExifInfo.ExposureProgram);	// Value
		Pos += 4;
	}
	
	// TAG_EXPOSURE_MODE
	if (m_ExifInfo.ExposureMode)
	{
		Put16u(&Section[Pos], TAG_EXPOSURE_MODE);		// Tag
		Pos += 2;
		Put16u(&Section[Pos], FMT_USHORT);				// Type
		Pos += 2;
		Put32u(&Section[Pos], 1);						// Components Count
		Pos += 4;
		Put16u(&Section[Pos], m_ExifInfo.ExposureMode);	// Value
		Pos += 4;
	}

	// TAG_EXIF_VERSION Entry
	char sVer[5] = DEFAULT_EXIF_VERSION_STR;			// Min. Version
	if (m_ExifInfo.Version[0])
	{
		m_ExifInfo.Version[4] = '\0';					// Just in case...
		int nVer = atoi(m_ExifInfo.Version);
		if (nVer > DEFAULT_EXIF_VERSION_INT)
			strcpy(sVer, m_ExifInfo.Version);
	}
	Put16u(&Section[Pos], TAG_EXIF_VERSION);			// Tag
	Pos += 2;
	Put16u(&Section[Pos], FMT_UNDEFINED);				// Type
	Pos += 2;
	Put32u(&Section[Pos], 4);							// Components Count
	Pos += 4;		
	Section[Pos++] = sVer[0];							// Value
	Section[Pos++] = sVer[1];
	Section[Pos++] = sVer[2];
	Section[Pos++] = sVer[3];

	// TAG_EXIF_IMAGEWIDTH Entry
	Put16u(&Section[Pos], TAG_EXIF_IMAGEWIDTH);			// Tag
	Pos += 2;
	Put16u(&Section[Pos], FMT_ULONG);					// Type
	Pos += 2;
	Put32u(&Section[Pos], 1);							// Components Count
	Pos += 4;
	Put32u(&Section[Pos], dwImageWidth);				// Value
	Pos += 4;

	// TAG_EXIF_IMAGEHEIGHT Entry
	Put16u(&Section[Pos], TAG_EXIF_IMAGEHEIGHT);		// Tag
	Pos += 2;
	Put16u(&Section[Pos], FMT_ULONG);					// Type
	Pos += 2;
	Put32u(&Section[Pos], 1);							// Components Count
	Pos += 4;
	Put32u(&Section[Pos], dwImageHeight);				// Value
	Pos += 4;

	// TAG_DATETIME_ORIGINAL
	if (m_ExifInfo.DateTime[0])
	{
		Put16u(&Section[Pos], TAG_DATETIME_ORIGINAL);	// Tag
		Pos += 2;
		Put16u(&Section[Pos], FMT_STRING);				// Type
		Pos += 2;
		Put32u(&Section[Pos], 20);						// Components Count
		Pos += 4;
		Put32u(&Section[Pos], PosData - Base);			// Value Ptr
		Pos += 4;

		// Copy Data
		POSCHECK(PosData+19);
		strcpy((char*)&Section[PosData], m_ExifInfo.DateTime);
		PosData += 20;
	}

	// TAG_FLASH Entry
	if (m_ExifInfo.Flash >= 0)
	{
		Put16u(&Section[Pos], TAG_FLASH);				// Tag
		Pos += 2;
		Put16u(&Section[Pos], FMT_USHORT);				// Type
		Pos += 2;
		Put32u(&Section[Pos], 1);						// Components Count
		Pos += 4;
		Put16u(&Section[Pos], m_ExifInfo.Flash);		// Value
		Section[Pos+2] = 0; Section[Pos+3] = 0;
		Pos += 4;
	}

	// TAG_ISO_EQUIVALENT Entry
	if (m_ExifInfo.ISOequivalent)
	{
		Put16u(&Section[Pos], TAG_ISO_EQUIVALENT);		// Tag
		Pos += 2;
		Put16u(&Section[Pos], FMT_USHORT);				// Type
		Pos += 2;
		Put32u(&Section[Pos], 1);						// Components Count
		Pos += 4;
		Put16u(&Section[Pos], m_ExifInfo.ISOequivalent);// Value
		Section[Pos+2] = 0; Section[Pos+3] = 0;
		Pos += 4;
	}

	// TAG_FOCALLENGTH
	if (m_ExifInfo.FocalLength)
	{
		Put16u(&Section[Pos], TAG_FOCALLENGTH);			// Tag
		Pos += 2;
		Put16u(&Section[Pos], FMT_URATIONAL);			// Type
		Pos += 2;
		Put32u(&Section[Pos], 1);						// Components Count
		Pos += 4;
		Put32u(&Section[Pos], PosData - Base);			// Value Ptr
		Pos += 4;

		// Copy Data
		POSCHECK(PosData+7);
		dwNum = (DWORD)(m_ExifInfo.FocalLength * 10000.0f);
		dwDen = 10000;
		Put32u((void*)(&Section[PosData]), dwNum);
		PosData += 4;
		Put32u((void*)(&Section[PosData]), dwDen);
		PosData += 4;
	}

	// TAG_SUBJECT_DISTANCE
	if (m_ExifInfo.Distance)
	{
		Put16u(&Section[Pos], TAG_SUBJECT_DISTANCE);	// Tag
		Pos += 2;
		Put16u(&Section[Pos], FMT_SRATIONAL);			// Type
		Pos += 2;
		Put32u(&Section[Pos], 1);						// Components Count
		Pos += 4;
		Put32u(&Section[Pos], PosData - Base);			// Value Ptr
		Pos += 4;

		// Copy Data
		POSCHECK(PosData+7);
		nNum = (int)(m_ExifInfo.Distance * 10000.0f);
		nDen = 10000;
		Put32s((void*)(&Section[PosData]), nNum);
		PosData += 4;
		Put32s((void*)(&Section[PosData]), nDen);
		PosData += 4;
	}

	// TAG_EXPOSURE_BIAS
	if (m_ExifInfo.ExposureBias)
	{
		Put16u(&Section[Pos], TAG_EXPOSURE_BIAS);		// Tag
		Pos += 2;
		Put16u(&Section[Pos], FMT_SRATIONAL);			// Type
		Pos += 2;
		Put32u(&Section[Pos], 1);						// Components Count
		Pos += 4;
		Put32u(&Section[Pos], PosData - Base);			// Value Ptr
		Pos += 4;

		// Copy Data
		POSCHECK(PosData+7);
		nNum = (int)(m_ExifInfo.ExposureBias * 10000.0f);
		nDen = 10000;
		Put32s((void*)(&Section[PosData]), nNum);
		PosData += 4;
		Put32s((void*)(&Section[PosData]), nDen);
		PosData += 4;
	}

	// TAG_DIGITALZOOMRATIO
	if (m_ExifInfo.DigitalZoomRatio)
	{
		Put16u(&Section[Pos], TAG_DIGITALZOOMRATIO);	// Tag
		Pos += 2;
		Put16u(&Section[Pos], FMT_URATIONAL);			// Type
		Pos += 2;
		Put32u(&Section[Pos], 1);						// Components Count
		Pos += 4;
		Put32u(&Section[Pos], PosData - Base);			// Value Ptr
		Pos += 4;

		// Copy Data
		POSCHECK(PosData+7);
		dwNum = (DWORD)(m_ExifInfo.DigitalZoomRatio * 10000.0f);
		dwDen = 10000;
		Put32u((void*)(&Section[PosData]), dwNum);
		PosData += 4;
		Put32u((void*)(&Section[PosData]), dwDen);
		PosData += 4;
	}

	// TAG_FOCALLENGTH_35MM
	if (m_ExifInfo.FocalLength35mmEquiv)
	{
		Put16u(&Section[Pos], TAG_FOCALLENGTH_35MM);	// Tag
		Pos += 2;
		Put16u(&Section[Pos], FMT_USHORT);				// Type
		Pos += 2;
		Put32u(&Section[Pos], 1);						// Components Count
		Pos += 4;
		Put16u(&Section[Pos], m_ExifInfo.FocalLength35mmEquiv);// Value
		Section[Pos+2] = 0; Section[Pos+3] = 0;
		Pos += 4;
	}

	// TAG_DISTANCE_RANGE
	if (m_ExifInfo.DistanceRange)
	{
		Put16u(&Section[Pos], TAG_DISTANCE_RANGE);		// Tag
		Pos += 2;
		Put16u(&Section[Pos], FMT_USHORT);				// Type
		Pos += 2;
		Put32u(&Section[Pos], 1);						// Components Count
		Pos += 4;
		Put16u(&Section[Pos], m_ExifInfo.DistanceRange);// Value
		Section[Pos+2] = 0; Section[Pos+3] = 0;
		Pos += 4;
	}

	// TAG_WHITEBALANCE
	if (m_ExifInfo.WhiteBalance >= 0)
	{
		Put16u(&Section[Pos], TAG_WHITEBALANCE);		// Tag
		Pos += 2;
		Put16u(&Section[Pos], FMT_USHORT);				// Type
		Pos += 2;
		Put32u(&Section[Pos], 1);						// Components Count
		Pos += 4;
		Put16u(&Section[Pos], m_ExifInfo.WhiteBalance);	// Value
		Section[Pos+2] = 0; Section[Pos+3] = 0;
		Pos += 4;
	}

	// TAG_METERING_MODE
	if (m_ExifInfo.MeteringMode)
	{
		Put16u(&Section[Pos], TAG_METERING_MODE);		// Tag
		Pos += 2;
		Put16u(&Section[Pos], FMT_USHORT);				// Type
		Pos += 2;
		Put32u(&Section[Pos], 1);						// Components Count
		Pos += 4;
		Put16u(&Section[Pos], m_ExifInfo.MeteringMode);	// Value
		Section[Pos+2] = 0; Section[Pos+3] = 0;
		Pos += 4;
	}

	// TAG_LIGHT_SOURCE
	if (m_ExifInfo.LightSource)
	{
		Put16u(&Section[Pos], TAG_LIGHT_SOURCE);		// Tag
		Pos += 2;
		Put16u(&Section[Pos], FMT_USHORT);				// Type
		Pos += 2;
		Put32u(&Section[Pos], 1);						// Components Count
		Pos += 4;
		Put16u(&Section[Pos], m_ExifInfo.LightSource);	// Value
		Section[Pos+2] = 0; Section[Pos+3] = 0;
		Pos += 4;
	}

	// TAG_COMPRESSION_LEVEL
	if (m_ExifInfo.CompressionLevel)
	{
		Put16u(&Section[Pos], TAG_COMPRESSION_LEVEL);	// Tag
		Pos += 2;
		Put16u(&Section[Pos], FMT_URATIONAL);			// Type
		Pos += 2;
		Put32u(&Section[Pos], 1);						// Components Count
		Pos += 4;
		Put32u(&Section[Pos], PosData - Base);			// Value Ptr
		Pos += 4;

		// Copy Data
		POSCHECK(PosData+7);
		dwNum = (DWORD)(m_ExifInfo.CompressionLevel * 1000.0f);
		dwDen = 1000;
		Put32u((void*)(&Section[PosData]), dwNum);
		PosData += 4;
		Put32u((void*)(&Section[PosData]), dwDen);
		PosData += 4;
	}

	// TAG_FOCALPLANEXRES
	if (m_ExifInfo.FocalplaneXRes)
	{
		Put16u(&Section[Pos], TAG_FOCALPLANEXRES);		// Tag
		Pos += 2;
		Put16u(&Section[Pos], FMT_URATIONAL);			// Type
		Pos += 2;
		Put32u(&Section[Pos], 1);						// Components Count
		Pos += 4;
		Put32u(&Section[Pos], PosData - Base);			// Value Ptr
		Pos += 4;

		// Copy Data
		POSCHECK(PosData+7);
		dwNum = (DWORD)(m_ExifInfo.FocalplaneXRes * 10000.0f);
		dwDen = 10000;
		Put32u((void*)(&Section[PosData]), dwNum);
		PosData += 4;
		Put32u((void*)(&Section[PosData]), dwDen);
		PosData += 4;
	}

	// TAG_FOCALPLANEYRES
	if (m_ExifInfo.FocalplaneYRes)
	{
		Put16u(&Section[Pos], TAG_FOCALPLANEYRES);		// Tag
		Pos += 2;
		Put16u(&Section[Pos], FMT_URATIONAL);			// Type
		Pos += 2;
		Put32u(&Section[Pos], 1);						// Components Count
		Pos += 4;
		Put32u(&Section[Pos], PosData - Base);			// Value Ptr
		Pos += 4;

		// Copy Data
		POSCHECK(PosData+7);
		dwNum = (DWORD)(m_ExifInfo.FocalplaneYRes * 10000.0f);
		dwDen = 10000;
		Put32u((void*)(&Section[PosData]), dwNum);
		PosData += 4;
		Put32u((void*)(&Section[PosData]), dwDen);
		PosData += 4;
	}

	// TAG_FOCALPLANEUNITS
	if (m_ExifInfo.FocalplaneUnits)
	{
		int ResolutionUnit;
		if (m_ExifInfo.FocalplaneUnits == 25.4f)
			ResolutionUnit = 2;
		else if (m_ExifInfo.FocalplaneUnits == 10.0f)
			ResolutionUnit = 3;
		else if (m_ExifInfo.FocalplaneUnits == 1.0f)
			ResolutionUnit = 4;
		else if (m_ExifInfo.FocalplaneUnits == 0.001f)
			ResolutionUnit = 5;
		else
			ResolutionUnit = 1;

		Put16u(&Section[Pos], TAG_FOCALPLANEUNITS);		// Tag
		Pos += 2;
		Put16u(&Section[Pos], FMT_USHORT);				// Type
		Pos += 2;
		Put32u(&Section[Pos], 1);						// Components Count
		Pos += 4;
		Put16u(&Section[Pos], ResolutionUnit);			// Value
		Pos += 4;
	}

	// TAG_BRIGHTNESS
	if (m_ExifInfo.Brightness)
	{
		Put16u(&Section[Pos], TAG_BRIGHTNESS);			// Tag
		Pos += 2;
		Put16u(&Section[Pos], FMT_SRATIONAL);			// Type
		Pos += 2;
		Put32u(&Section[Pos], 1);						// Components Count
		Pos += 4;
		Put32u(&Section[Pos], PosData - Base);			// Value Ptr
		Pos += 4;

		// Copy Data
		POSCHECK(PosData+7);
		nNum = (int)(m_ExifInfo.Brightness * 10000.0f);
		nDen = 10000;
		Put32s((void*)(&Section[PosData]), nNum);
		PosData += 4;
		Put32s((void*)(&Section[PosData]), nDen);
		PosData += 4;
	}

	// TAG_AMBIENT_TEMPERATURE
	if (!isnan(m_ExifInfo.AmbientTemperature))
	{
		Put16u(&Section[Pos], TAG_AMBIENT_TEMPERATURE);	// Tag
		Pos += 2;
		Put16u(&Section[Pos], FMT_SRATIONAL);			// Type
		Pos += 2;
		Put32u(&Section[Pos], 1);						// Components Count
		Pos += 4;
		Put32u(&Section[Pos], PosData - Base);			// Value Ptr
		Pos += 4;

		// Copy Data
		POSCHECK(PosData + 7);
		nNum = (int)(m_ExifInfo.AmbientTemperature * 10000.0f);
		nDen = 10000;
		Put32s((void*)(&Section[PosData]), nNum);
		PosData += 4;
		Put32s((void*)(&Section[PosData]), nDen);
		PosData += 4;
	}

	// TAG_USER_COMMENT
	if (m_ExifInfo.UserComment[0])
	{
		int Len = wcslen(m_ExifInfo.UserComment) + 1;
		Put16u(&Section[Pos], TAG_USER_COMMENT);		// Tag
		Pos += 2;
		Put16u(&Section[Pos], FMT_UNDEFINED);			// Type
		Pos += 2;
		Put32u(&Section[Pos], USER_COMMENT_HEADER_SIZE + 2*Len);// Components Count
		Pos += 4;	
		Put32u(&Section[Pos], PosData - Base);			// Value Ptr
		Pos += 4;

		// Copy Data
		POSCHECK(PosData+USER_COMMENT_HEADER_SIZE+2*Len-1);
		memcpy(&Section[PosData], m_USER_COMMENT_UNICODE, USER_COMMENT_HEADER_SIZE);
		PosData += USER_COMMENT_HEADER_SIZE;
		memcpy(&Section[PosData], m_ExifInfo.UserComment, 2*Len);
		PosData += 2*Len;
	}

	// No Offset
	Put32u(&Section[Pos], 0x00000000);					// Value
	Pos += 4;


	/////////////////////
	// IFD0 Gps SubDir //
	/////////////////////

	if (IFD0GpsSubDirEntries > 0)
	{
		// Count
		Put16u(&Section[Pos], IFD0GpsSubDirEntries);
		Pos += 2;

		// TAG_GPS_VERSION Entry
		char GpsVer[4]; // Min. Version
		GpsVer[0] = DEFAULT_GPS_VERSION_0;
		GpsVer[1] = DEFAULT_GPS_VERSION_1;
		GpsVer[2] = DEFAULT_GPS_VERSION_2;
		GpsVer[3] = DEFAULT_GPS_VERSION_3;
		char sCurrentGpsVer[5];
		memcpy(sCurrentGpsVer, m_ExifInfo.GpsVersion, 4);
		sCurrentGpsVer[0] += '0';
		sCurrentGpsVer[1] += '0';
		sCurrentGpsVer[2] += '0';
		sCurrentGpsVer[3] += '0';
		sCurrentGpsVer[4] = '\0';
		int nCurrentGpsVer = atoi(sCurrentGpsVer);
		if (nCurrentGpsVer > DEFAULT_GPS_VERSION_INT)
			memcpy(GpsVer, m_ExifInfo.GpsVersion, 4);
		Put16u(&Section[Pos], TAG_GPS_VERSION);				// Tag
		Pos += 2;
		Put16u(&Section[Pos], FMT_BYTE);					// Type
		Pos += 2;
		Put32u(&Section[Pos], 4);							// Components Count
		Pos += 4;		
		Section[Pos++] = GpsVer[0];							// Value
		Section[Pos++] = GpsVer[1];
		Section[Pos++] = GpsVer[2];
		Section[Pos++] = GpsVer[3];

		// TAG_GPS_LAT
		if (m_ExifInfo.GpsLat[GPS_DEGREE]  >= 0.0f &&
			m_ExifInfo.GpsLat[GPS_MINUTES] >= 0.0f &&
			m_ExifInfo.GpsLat[GPS_SECONDS] >= 0.0f)
		{
			Put16u(&Section[Pos], TAG_GPS_LAT);			// Tag
			Pos += 2;
			Put16u(&Section[Pos], FMT_URATIONAL);		// Type
			Pos += 2;
			Put32u(&Section[Pos], 3);					// Components Count
			Pos += 4;
			Put32u(&Section[Pos], PosData - Base);		// Value Ptr
			Pos += 4;

			// Copy Data
			POSCHECK(PosData+23);
			dwNum = (DWORD)(m_ExifInfo.GpsLat[GPS_DEGREE]);
			dwDen = 1;
			Put32u((void*)(&Section[PosData]), dwNum);
			PosData += 4;
			Put32u((void*)(&Section[PosData]), dwDen);
			PosData += 4;
			dwNum = (DWORD)(m_ExifInfo.GpsLat[GPS_MINUTES]);
			dwDen = 1;
			Put32u((void*)(&Section[PosData]), dwNum);
			PosData += 4;
			Put32u((void*)(&Section[PosData]), dwDen);
			PosData += 4;
			dwNum = (DWORD)(m_ExifInfo.GpsLat[GPS_SECONDS] * 10000.0f);
			dwDen = 10000;
			Put32u((void*)(&Section[PosData]), dwNum);
			PosData += 4;
			Put32u((void*)(&Section[PosData]), dwDen);
			PosData += 4;
		}

		// TAG_GPS_LONG
		if (m_ExifInfo.GpsLong[GPS_DEGREE]  >= 0.0f &&
			m_ExifInfo.GpsLong[GPS_MINUTES] >= 0.0f &&
			m_ExifInfo.GpsLong[GPS_SECONDS] >= 0.0f)
		{
			Put16u(&Section[Pos], TAG_GPS_LONG);		// Tag
			Pos += 2;
			Put16u(&Section[Pos], FMT_URATIONAL);		// Type
			Pos += 2;
			Put32u(&Section[Pos], 3);					// Components Count
			Pos += 4;
			Put32u(&Section[Pos], PosData - Base);		// Value Ptr
			Pos += 4;

			// Copy Data
			POSCHECK(PosData+23);
			dwNum = (DWORD)(m_ExifInfo.GpsLong[GPS_DEGREE]);
			dwDen = 1;
			Put32u((void*)(&Section[PosData]), dwNum);
			PosData += 4;
			Put32u((void*)(&Section[PosData]), dwDen);
			PosData += 4;
			dwNum = (DWORD)(m_ExifInfo.GpsLong[GPS_MINUTES]);
			dwDen = 1;
			Put32u((void*)(&Section[PosData]), dwNum);
			PosData += 4;
			Put32u((void*)(&Section[PosData]), dwDen);
			PosData += 4;
			dwNum = (DWORD)(m_ExifInfo.GpsLong[GPS_SECONDS] * 10000.0f);
			dwDen = 10000;
			Put32u((void*)(&Section[PosData]), dwNum);
			PosData += 4;
			Put32u((void*)(&Section[PosData]), dwDen);
			PosData += 4;
		}

		// TAG_GPS_ALT
		if (m_ExifInfo.GpsAlt >= 0.0f)
		{
			Put16u(&Section[Pos], TAG_GPS_ALT);			// Tag
			Pos += 2;
			Put16u(&Section[Pos], FMT_URATIONAL);		// Type
			Pos += 2;
			Put32u(&Section[Pos], 1);					// Components Count
			Pos += 4;
			Put32u(&Section[Pos], PosData - Base);		// Value Ptr
			Pos += 4;

			// Copy Data
			POSCHECK(PosData+7);
			dwNum = (DWORD)(m_ExifInfo.GpsAlt * 100.0f);
			dwDen = 100;
			Put32u((void*)(&Section[PosData]), dwNum);
			PosData += 4;
			Put32u((void*)(&Section[PosData]), dwDen);
			PosData += 4;
		}

		// TAG_GPS_LAT_REF
		if (m_ExifInfo.GpsLatRef[0])
		{
			Put16u(&Section[Pos], TAG_GPS_LAT_REF);		// Tag
			Pos += 2;
			Put16u(&Section[Pos], FMT_STRING);			// Type
			Pos += 2;
			Put32u(&Section[Pos], 2);					// Components Count
			Pos += 4;
			Section[Pos++] = m_ExifInfo.GpsLatRef[0];	// Value
			Section[Pos++] = '\0';
			Section[Pos++] = 0;
			Section[Pos++] = 0;
		}

		// TAG_GPS_LONG_REF
		if (m_ExifInfo.GpsLongRef[0])
		{
			Put16u(&Section[Pos], TAG_GPS_LONG_REF);	// Tag
			Pos += 2;
			Put16u(&Section[Pos], FMT_STRING);			// Type
			Pos += 2;
			Put32u(&Section[Pos], 2);					// Components Count
			Pos += 4;
			Section[Pos++] = m_ExifInfo.GpsLongRef[0];	// Value
			Section[Pos++] = '\0';
			Section[Pos++] = 0;
			Section[Pos++] = 0;
		}

		// TAG_GPS_ALT_REF
		if (m_ExifInfo.GpsAltRef >= 0)
		{
			Put16u(&Section[Pos], TAG_GPS_ALT_REF);				// Tag
			Pos += 2;
			Put16u(&Section[Pos], FMT_BYTE);					// Type
			Pos += 2;
			Put32u(&Section[Pos], 1);							// Components Count
			Pos += 4;		
			Section[Pos++] = m_ExifInfo.GpsAltRef;				// Value
			Section[Pos++] = 0;
			Section[Pos++] = 0;
			Section[Pos++] = 0;
		}

		// TAG_GPS_TIMESTAMP
		if (m_ExifInfo.GpsTime[GPS_HOUR]    >= 0.0f &&
			m_ExifInfo.GpsTime[GPS_MINUTES] >= 0.0f &&
			m_ExifInfo.GpsTime[GPS_SECONDS] >= 0.0f)
		{
			Put16u(&Section[Pos], TAG_GPS_TIMESTAMP);	// Tag
			Pos += 2;
			Put16u(&Section[Pos], FMT_URATIONAL);		// Type
			Pos += 2;
			Put32u(&Section[Pos], 3);					// Components Count
			Pos += 4;
			Put32u(&Section[Pos], PosData - Base);		// Value Ptr
			Pos += 4;

			// Copy Data
			POSCHECK(PosData+23);
			dwNum = (DWORD)(m_ExifInfo.GpsTime[GPS_HOUR]);
			dwDen = 1;
			Put32u((void*)(&Section[PosData]), dwNum);
			PosData += 4;
			Put32u((void*)(&Section[PosData]), dwDen);
			PosData += 4;
			dwNum = (DWORD)(m_ExifInfo.GpsTime[GPS_MINUTES]);
			dwDen = 1;
			Put32u((void*)(&Section[PosData]), dwNum);
			PosData += 4;
			Put32u((void*)(&Section[PosData]), dwDen);
			PosData += 4;
			if ((float)((DWORD)m_ExifInfo.GpsTime[GPS_SECONDS]) == m_ExifInfo.GpsTime[GPS_SECONDS])
			{
				dwNum = (DWORD)(m_ExifInfo.GpsTime[GPS_SECONDS]);
				dwDen = 1;
			}
			else
			{	
				dwNum = (DWORD)(m_ExifInfo.GpsTime[GPS_SECONDS] * 1000.0f);
				dwDen = 1000;
			}
			Put32u((void*)(&Section[PosData]), dwNum);
			PosData += 4;
			Put32u((void*)(&Section[PosData]), dwDen);
			PosData += 4;
		}

		// TAG_GPS_MAPDATUM
		if (WriteExifString(TAG_GPS_MAPDATUM,
							m_ExifInfo.GpsMapDatum,
							Section,
							nMaxSectionSize,
							Base,
							Pos,
							PosData) < 0)
			return -1;

		// No Offset
		Put32u(&Section[Pos], 0x00000000);					// Value
		Pos += 4;
	}


	//////////
	// IFD1 //
	//////////

	if (IFD1Entries > 0)
	{
		// Count
		Put16u(&Section[Pos], IFD1Entries);				// 3 Entries
		Pos += 2;

		// TAG_COMPRESSION Entry
		Put16u(&Section[Pos], TAG_COMPRESSION);// Tag
		Pos += 2;
		Put16u(&Section[Pos], FMT_USHORT);				// Type
		Pos += 2;
		Put32u(&Section[Pos], 1);						// Components Count
		Pos += 4;
		Put16u(&Section[Pos], 6);						// Value: Jpeg Compression
		Pos += 4;
		
		// TAG_JPEGIFBYTECOUNT Entry
		Put16u(&Section[Pos], TAG_JPEGIFBYTECOUNT);		// Tag
		Pos += 2;
		Put16u(&Section[Pos], FMT_ULONG);				// Type
		Pos += 2;
		Put32u(&Section[Pos], 1);						// Components Count
		Pos += 4;
		Put32u(&Section[Pos], dwJpegThumbSize);			// Value
		Pos += 4;

		// TAG_JPEGIFOFFSET Entry
		Put16u(&Section[Pos], TAG_JPEGIFOFFSET);		// Tag
		Pos += 2;
		Put16u(&Section[Pos], FMT_ULONG);				// Type
		Pos += 2;
		Put32u(&Section[Pos], 1);						// Components Count
		Pos += 4;
		Put32u(&Section[Pos], PosData - Base);			// Value Ptr
		Pos += 4;

		// Copy Thumbnail Data
		POSCHECK(PosData + (int)dwJpegThumbSize - 1);
		memcpy(&Section[PosData], pJpegThumbData, dwJpegThumbSize);
		PosData += dwJpegThumbSize;

		// No Offset
		Put32u(&Section[Pos], 0x00000000);				// Value
		Pos += 4;
	}
	
	return PosData;
}

bool CMetadata::ParseProcessJPEG(unsigned char* pData, int nLength, bool bDoWrite/*=false*/)
{
	bool res;
    int a, i;
	m_bDoWrite = bDoWrite;
	ICCARRAY IccArray;

	// Clean-up
	if (!bDoWrite)
		Free();
	else
	{
		if (m_pAllocatedThumbnailPointer)
		{
			delete [] m_pAllocatedThumbnailPointer;
			m_pAllocatedThumbnailPointer = NULL;
		}
	}

	// Start
	DWORD dwPos = 0;
	m_nSectionsRead = 0;
	if (((int)dwPos+1) >= nLength)
		goto error;
    if (pData[dwPos++] != 0xff || pData[dwPos++] != M_SOI)
	{
		TRACE(_T("Invalid File (SOI Marker Missing)\n"));
        goto error;
    }
	else
	{
		//TRACE("M_SOI\n");
		m_Sections[m_nSectionsRead].Type = M_SOI;
		m_Sections[m_nSectionsRead].Size = 0;
		m_Sections[m_nSectionsRead].Data = NULL;
		m_nSectionsRead += 1;
	}

	// Loop Through All Sections
    for(;;)
	{
        int itemlen;
        int marker = 0;
        int ll, lh;

		// Limit of Sections
        if (m_nSectionsRead >= MAX_JPEG_SECTIONS)
		{
			TRACE(_T("Too many sections in jpg file\n"));
			goto error;
        }

		// Padding
        for (a = 0 ; a < 7 ; a++)
		{
			if ((int)dwPos >= nLength)
				goto error;
            marker = pData[dwPos++];
            if (marker != 0xff)
				break;

            if (a >= 6)
			{
				// 0xff is legal padding, but if we get that many, something's wrong
                TRACE(_T("Too many unsigned char paddings\n"));
                goto error;
            }
        }
        if (marker == 0xff)
		{
            // 0xff is legal padding, but if we get that many, something's wrong
            TRACE(_T("Too many unsigned char paddings\n"));
			goto error;
        }

		// Store Section Marker
        m_Sections[m_nSectionsRead].Type = marker;

		// M_EOI is a Standalone Section like M_SOI!
		if (marker == M_EOI)
		{
			m_Sections[m_nSectionsRead].Size = 0;
			m_Sections[m_nSectionsRead].Data = NULL;
			m_nSectionsRead += 1;
		}
		else
		{
			// Read the length of the Section
			// Note: the length parameter includes the two bytes length,
			// but excludes the two bytes marker!
			if (((int)dwPos+1) >= nLength)
				goto error;
			lh = pData[dwPos];
			ll = pData[dwPos+1];
			itemlen = (lh << 8) | ll;
			if (itemlen < 2)
			{
				TRACE(_T("Invalid marker\n"));
				goto error;
			}

			// Store Section's Size and Data Pointer
			if (((int)dwPos+itemlen-1) >= nLength)
				goto error;
			m_Sections[m_nSectionsRead].Size = itemlen;
			m_Sections[m_nSectionsRead].Data = &(pData[dwPos]);
			m_nSectionsRead += 1;
			dwPos += itemlen;
		}

		// Parse Sections
        switch (marker)
		{
            case M_SOS:		// Start of compressed image data
				if ((int)dwPos >= nLength)
					goto error;
				m_pAfterSOSData = &(pData[dwPos]);
				m_dwAfterSOSSize = nLength - dwPos;
                goto ok;

            case M_EOI:		// If no M_SOS encountered
				//TRACE("M_EOI\n");
				m_pAfterSOSData = NULL;
				m_dwAfterSOSSize = 0;
                goto ok;

            case M_COM:		// Comment section
				//TRACE("M_COM\n");
				// Parse Only First Comment Section
                if (!m_bHasCom)
				{
                    ParseCOMSection(	m_Sections[m_nSectionsRead-1].Data + 2,
										m_Sections[m_nSectionsRead-1].Size - 2);
                    m_bHasCom = true;
                }
                break;

            case M_JFIF:
				//TRACE("M_JFIF\n");
                // Regular jpegs always have this tag, exif images have the exif
                // marker instead, althogh some apps will write images with both markers.
				m_bHasJfif = true;
                break;

			case M_IPTC:
			{
				//TRACE("M_IPTC\n");
				int nSectionLength = m_Sections[m_nSectionsRead-1].Size - 2;
				int offset = GetJpegIptcOffsetAndSize(	m_Sections[m_nSectionsRead-1].Data + 2,
														nSectionLength);
				if (offset >= 0)
				{
					m_dwIptcLegacyPreSize = 0;
					m_dwIptcLegacySize = 0;
					m_dwIptcLegacyPostSize = 0;
					if (m_pIptcLegacyPreData)
					{	
						delete [] m_pIptcLegacyPreData;
						m_pIptcLegacyPreData = NULL;
					}
					if (m_pIptcLegacyData)
					{	
						delete [] m_pIptcLegacyData;
						m_pIptcLegacyData = NULL;
					}
					if (m_pIptcLegacyPostData)
					{	
						delete [] m_pIptcLegacyPostData;
						m_pIptcLegacyPostData = NULL;
					}
					if (offset > 0)
					{
						m_pIptcLegacyPreData = new BYTE[offset];
						if (m_pIptcLegacyPreData)
						{
							m_dwIptcLegacyPreSize = (DWORD)offset;
							memcpy(m_pIptcLegacyPreData, m_Sections[m_nSectionsRead-1].Data + 2, m_dwIptcLegacyPreSize);
						}
					}
					DWORD dwDWordsCount = (nSectionLength % 4) ? (nSectionLength / 4) + 1 : nSectionLength / 4;
					DWORD dwWordsCount = (nSectionLength % 2) ? (nSectionLength / 2) + 1 : nSectionLength / 2;
					m_pIptcLegacyData = (LPBYTE)(new DWORD[dwDWordsCount]);
					if (m_pIptcLegacyData)
					{
						m_dwIptcLegacySize = (DWORD)nSectionLength;
						memset(m_pIptcLegacyData, 0, 4 * dwDWordsCount);
						memcpy(m_pIptcLegacyData, m_Sections[m_nSectionsRead-1].Data + 2 + offset, m_dwIptcLegacySize);
						ParseIPTCLegacy(m_pIptcLegacyData, (int)m_dwIptcLegacySize);
					}
					if ((m_Sections[m_nSectionsRead-1].Size - 2 - (2*dwWordsCount) - offset) > 0)
					{
						m_pIptcLegacyPostData = new BYTE[(m_Sections[m_nSectionsRead-1].Size - 2) - (2*dwWordsCount) - offset];
						if (m_pIptcLegacyPostData)
						{
							m_dwIptcLegacyPostSize = (DWORD)((m_Sections[m_nSectionsRead-1].Size - 2) - (2*dwWordsCount) - offset);
							memcpy(m_pIptcLegacyPostData, m_Sections[m_nSectionsRead-1].Data + 2 + offset + (2*dwWordsCount), m_dwIptcLegacyPostSize);
						}
					}
				}
                break;
			}

            case M_EXIF_XMP:
			{
				//TRACE("M_EXIF_XMP\n");
                // Make sure it says 'Exif\0\0' in the section
				// before treating it as exif
                if ((m_Sections[m_nSectionsRead-1].Size >= (EXIF_HEADER_SIZE + 2)) &&
					(memcmp(m_Sections[m_nSectionsRead-1].Data + 2, m_ExifHeader, EXIF_HEADER_SIZE) == 0))
				{
					ParseTIFFData(	0,
									false,
									true,
									m_Sections[m_nSectionsRead-1].Data+8,
									m_Sections[m_nSectionsRead-1].Size-8);
					m_ExifInfo.bHasExifSection = true;
				}
				else if ((m_Sections[m_nSectionsRead-1].Size >= (XMP_HEADER_SIZE + 2)) &&
						(memcmp(m_Sections[m_nSectionsRead-1].Data + 2, m_XmpHeader, XMP_HEADER_SIZE) == 0))
				{
					if (m_pXmpData)
					{	
						delete [] m_pXmpData;
						m_pXmpData = NULL;
					}
					m_pXmpData = new BYTE[m_Sections[m_nSectionsRead-1].Size - (XMP_HEADER_SIZE + 2)];
					if (m_pXmpData)
					{
						m_dwXmpSize = m_Sections[m_nSectionsRead-1].Size - (XMP_HEADER_SIZE + 2);
						memcpy(m_pXmpData, m_Sections[m_nSectionsRead-1].Data + (XMP_HEADER_SIZE + 2), m_dwXmpSize);
						ParseXMP(m_pXmpData, (int)m_dwXmpSize);
					}
				}
                break;
			}

			case M_ICC:
			{
				//TRACE("M_ICC\n");
                if ((m_Sections[m_nSectionsRead-1].Size >= (ICC_HEADER_SIZE + 2 + 2)) &&
					(memcmp(m_Sections[m_nSectionsRead-1].Data + 2, m_IccHeader, ICC_HEADER_SIZE) == 0))
				{
					// If the profile doesn't fit into one marker we have multiple chunks
					BYTE chunkid = *(m_Sections[m_nSectionsRead-1].Data + 2 + ICC_HEADER_SIZE);		// 1..255
					BYTE chunks = *(m_Sections[m_nSectionsRead-1].Data + 2 + ICC_HEADER_SIZE + 1);	// 1..255
					
					// Set array size
					if (IccArray.GetSize() == 0)
						IccArray.SetSize(chunks);

					// New entry
					CIccSectionEntry* pIccSectionEntry = new CIccSectionEntry;
					if (!pIccSectionEntry)
						goto error;
					IccArray.SetAt(chunkid - 1, pIccSectionEntry); 

					// Allocate entry memory
					pIccSectionEntry->size = m_Sections[m_nSectionsRead-1].Size - (ICC_HEADER_SIZE + 2 + 2);
					pIccSectionEntry->data = new BYTE[pIccSectionEntry->size];
					if (!pIccSectionEntry->data)
					{
						pIccSectionEntry->size = 0;
						goto error;
					}

					// Copy data
					memcpy(pIccSectionEntry->data, m_Sections[m_nSectionsRead-1].Data + (ICC_HEADER_SIZE + 2 + 2), pIccSectionEntry->size);
				}
                break;
			}

			case M_APP3:
			case M_APP4:
			case M_APP5:
			case M_APP6:
			case M_APP7:
			case M_APP8:
			case M_APP9:
			case M_APP10:
			case M_APP11:
			case M_APP12:
			case M_APP14:
			case M_APP15:
				m_bHasOtherAppSections = true;
				break;

            case M_SOF0: 
            case M_SOF1: 
            case M_SOF2: 
            case M_SOF3: 
            case M_SOF5: 
            case M_SOF6: 
            case M_SOF7: 
            case M_SOF9: 
            case M_SOF10:
            case M_SOF11:
            case M_SOF13:
            case M_SOF14:
            case M_SOF15:
				//TRACE("M_SOFn\n");
                ParseSOFnSection(	marker,
									m_Sections[m_nSectionsRead-1].Data + 2,
									m_Sections[m_nSectionsRead-1].Size - 2);
                break;

            default:
                // Skip any other sections
				//TRACE("M_?\n");
                break;
        }
    }

ok:
	res = true;
	goto iccjoin;

error:
	res = false;

iccjoin:

	// Join IccArray entries to one linear memory buffer
	if (IccArray.GetSize() > 0)
	{
		int nTotalSize = 0;
		for (i = 0 ; i < IccArray.GetSize() ; i++)
		{
			if (IccArray[i])
				nTotalSize += IccArray[i]->size;
		}
		if (m_pIccData)
		{	
			delete [] m_pIccData;
			m_pIccData = NULL;
		}
		m_pIccData = new BYTE[nTotalSize];
		if (m_pIccData)
		{
			m_dwIccSize = nTotalSize;
			int iccpos = 0;
			for (i = 0 ; i < IccArray.GetSize() ; i++)
			{
				memcpy(m_pIccData + iccpos, IccArray[i]->data, IccArray[i]->size);
				iccpos += IccArray[i]->size;
			}
		}
		for (i = 0 ; i < IccArray.GetSize() ; i++)
			delete IccArray[i];
	}

	return res;
}

/* IPTC in JPEG:
BYTE Version[]	// Null terminated Software Version, usually: "Photoshop 3.0"
BYTE Type[4];	// Always '8BIM'
WORD ID;		// Usually IPTC-IIM which is 0x0404
BYTE Name[];	// Even-length Pascal-format string, 2 bytes or longer
LONG Size;		// Big endian length of resource data following, in bytes
BYTE Data[];	// Resource data, padded to even length

Data[] is a list of:
0x1C			// Iptc Header
BYTE RecordID	// 0x01 Envelope Record, 0x02 Application Record, ... 0x08
BYTE TagID;		// Field id
WORD Size;		// Big endian size of data following
BYTE IptcEntry[Size];
*/
int CMetadata::GetJpegIptcOffsetAndSize(unsigned char* pData, int& nLength)
{
	int pos = 0;
	BOOL b8BIMFound = FALSE;

	// Search for '8BIM'
	while (pos < (nLength - 4))
	{
		if (memcmp(pData + pos, "8BIM", 4) == 0)
		{
			b8BIMFound = TRUE;
			break;
		}
		else
			pos++;
	}
	if (!b8BIMFound)
		return -1;

	// Skip '8BIM' and IPTC-IIM Marker '\x0x4' '\x0x4'
	pos += 6;

	// Skip Caption Pascal Style String 
	if (pos >= nLength)
		return -1;
	int pascalstrlen = pData[pos];
	pascalstrlen++;			// Add length itself
	if (pascalstrlen & 0x1)	// string is always padded to even size
		pascalstrlen++;
	pos += pascalstrlen;

	// Iptc Size is big endian and 32 bit
	if (pos >= (nLength - 4))
		return -1;
	int iptcsize = *((DWORD*)(&pData[pos]));
	BYTE t;
	LPBYTE cp = (LPBYTE)&iptcsize;
	t = cp[3]; cp[3] = cp[0]; cp[0] = t;
	t = cp[2]; cp[2] = cp[1]; cp[1] = t;
	pos += 4;
	if (iptcsize >= 5				&&	// Min. size
		pos + iptcsize <= nLength	&&	// Check length correctness
		pData[pos] == 0x1C)				// Check IPTC begin marker
	{
		nLength = iptcsize;
		return pos;
	}
	else
		return -1;
}

bool CMetadata::ParseTIFF(int nStartIFD, unsigned char* pData, int nLength)
{
	// Clean-up
	Free();
	
	// Parse TIFF Data
	// Note: Thumbnails are treated as a second page
	//       and not as a Thumbnail like for
	//       Exif Sections!
    return ParseTIFFData(	nStartIFD,
							true,	// Parse only given nStartIFD
							false,	// Tiff File, not Exif Section
							pData,
							nLength);			
}

/*--------------------------------------------------------------------------
   Parse the TIFF data
--------------------------------------------------------------------------*/
bool CMetadata::ParseTIFFData(	int nStartIFD,
								bool bOnlyParseGivenIFD,
								bool bExifSection,
								unsigned char* pData,
								int nLength)
{
	// Init Values
	m_ExifInfo.dThumbnailYCbCrCoeff[0] = 0.299;
	m_ExifInfo.dThumbnailYCbCrCoeff[1] = 0.587;
	m_ExifInfo.dThumbnailYCbCrCoeff[2] = 0.114;
	m_ExifInfo.dThumbnailRefBW[0] = 0.0;
	m_ExifInfo.dThumbnailRefBW[1] = 255.0;
	m_ExifInfo.dThumbnailRefBW[2] = 128.0;
	m_ExifInfo.dThumbnailRefBW[3] = 255.0;
	m_ExifInfo.dThumbnailRefBW[4] = 128.0;
	m_ExifInfo.dThumbnailRefBW[5] = 255.0;
	m_ExifInfo.ThumbnailYCbCrSubSampling[0] = 2;
	m_ExifInfo.ThumbnailYCbCrSubSampling[1] = 1;

	// Check The TIFF header component
    if (memcmp(pData,"II",2) == 0)
	{
        m_bMotorolaOrder = false;
    }
	else
	{
        if (memcmp(pData,"MM",2) == 0)
		{
            m_bMotorolaOrder = true;
        }
		else
		{
            TRACE(_T("Invalid Exif alignment marker.\n"));
			return false;
        }
    }

    // Check the next two values for correctness
    if (Get16u(pData+2) != 0x2a)
	{
        TRACE(_T("Invalid Exif start (1)\n"));
		return false;
    }

	// Get the Offset of first IFD with respect to the TIFF Header start
	int FirstOffset = Get32u(pData+4);
    if (FirstOffset < 8)
	{
        TRACE(_T("Suspicious offset of first IFD value\n"));
		return false;
    }

	// Jump to wanted start directory
	unsigned char* pNextDirStart = pData+FirstOffset;
	for (int i = 0 ; i < nStartIFD ; i++)
		pNextDirStart = GetNextDirPointer(pNextDirStart, pData, nLength);

    // Parse directory
    if (!ParseTIFFDir(nStartIFD, bOnlyParseGivenIFD, bExifSection, pNextDirStart, pData, nLength, &m_ExifInfo))
		return false;

	// Compute the CCD width, in millimeters
    if (m_ExifInfo.FocalplaneXRes != 0)
	{
        // Note: With some cameras, its not possible to compute this correctly because
        // they don't adjust the indicated focal plane resolution units when using less
        // than maximum resolution, so the CCDWidth value comes out too small.  Nothing
        // that we can do about it - its a camera problem.
		m_ExifInfo.CCDWidth = (float)(m_nExifImageWidth * m_ExifInfo.FocalplaneUnits / m_ExifInfo.FocalplaneXRes);

        if (m_ExifInfo.CCDWidth > 0.0		&&
			m_ExifInfo.FocalLength > 0.0	&&
			m_ExifInfo.FocalLength35mmEquiv == 0)
		{
            // Compute 35 mm equivalent focal length based on sensor geometry if we haven't
            // already got it explicitly from a tag.
            m_ExifInfo.FocalLength35mmEquiv = Round((double)m_ExifInfo.FocalLength / (double)m_ExifInfo.CCDWidth * 36.0);
        }
    }

	return true;
}

unsigned char* CMetadata::GetNextDirPointer(unsigned char* pData, unsigned char* pOffsetBase, int nLength)
{
	// Check
	if (!pData)
		return NULL;

	// Calc. Number of Entries
	int NumDirEntries = Get16u(pData);
	if ((pData+2+NumDirEntries*12) > (pOffsetBase+nLength))
	{
		TRACE(_T("Illegally sized directory\n"));
		return NULL;
	}

	// Get Next Dir Offset
	unsigned int NextDirOffset = Get32u(pData+2+NumDirEntries*12);
	if (NextDirOffset)
	{
		// Calc. Next Dir Start
		unsigned char* pNextDirStart = pOffsetBase + NextDirOffset;
		if (pNextDirStart < pOffsetBase ||
			pNextDirStart > pOffsetBase+nLength)
		{
			TRACE(_T("Illegal directory link\n"));
			return NULL;
		}
		else
			return pNextDirStart;
	}
	else
		return NULL;
}

//--------------------------------------------------------------------------
// Get 16 bits motorola order (always) for jpeg header stuff.
//--------------------------------------------------------------------------
int CMetadata::Get16m(void* Short)
{
    return (((unsigned char *)Short)[0] << 8) | ((unsigned char *)Short)[1];
}

//--------------------------------------------------------------------------
// Set 16 bits motorola order (always) for jpeg header stuff.
//--------------------------------------------------------------------------
void CMetadata::Put16m(void* Short, unsigned short PutValue)
{
	((unsigned char *)Short)[0] = (unsigned char)(PutValue>>8);
	((unsigned char *)Short)[1] = (unsigned char)PutValue;
}

unsigned short CMetadata::Get16u(void* Short)
{
     return (unsigned short)Get16s(Short);
}

short CMetadata::Get16s(void* Short)
{
    if (m_bMotorolaOrder)
	{
        return (((unsigned char *)Short)[0] << 8) | ((unsigned char *)Short)[1];
    }
	else
	{
        return (((unsigned char *)Short)[1] << 8) | ((unsigned char *)Short)[0];
    }
}

void CMetadata::Put16s(void* Short, short PutValue)
{
    if (m_bMotorolaOrder)
	{
        ((unsigned char *)Short)[0] = (unsigned char)(PutValue>>8);
        ((unsigned char *)Short)[1] = (unsigned char)PutValue;
    }
	else
	{
        ((unsigned char *)Short)[0] = (unsigned char)PutValue;
        ((unsigned char *)Short)[1] = (unsigned char)(PutValue>>8);
    }
}

void CMetadata::Put16u(void* Short, unsigned short PutValue)
{
    if (m_bMotorolaOrder)
	{
        ((unsigned char *)Short)[0] = (unsigned char)(PutValue>>8);
        ((unsigned char *)Short)[1] = (unsigned char)PutValue;
    }
	else
	{
        ((unsigned char *)Short)[0] = (unsigned char)PutValue;
        ((unsigned char *)Short)[1] = (unsigned char)(PutValue>>8);
    }
}

long CMetadata::Get32s(void* Long)
{
    if (m_bMotorolaOrder)
	{
        return  (((unsigned char *)Long)[0] << 24) | (((unsigned char *)Long)[1] << 16)
              | (((unsigned char *)Long)[2] << 8 ) | (((unsigned char *)Long)[3] << 0 );
    }
	else
	{
        return  (((unsigned char *)Long)[3] << 24) | (((unsigned char *)Long)[2] << 16)
              | (((unsigned char *)Long)[1] << 8 ) | (((unsigned char *)Long)[0] << 0 );
    }
}

unsigned long CMetadata::Get32u(void* Long)
{
    return (unsigned long)Get32s(Long);
}

void CMetadata::Put32s(void* Long, long PutValue)
{
    if (m_bMotorolaOrder)
	{
        ((unsigned char *)Long)[0] = (unsigned char)(PutValue>>24);
		((unsigned char *)Long)[1] = (unsigned char)(PutValue>>16);
		((unsigned char *)Long)[2] = (unsigned char)(PutValue>>8);
        ((unsigned char *)Long)[3] = (unsigned char)PutValue;
    }
	else
	{
        ((unsigned char *)Long)[0] = (unsigned char)PutValue;
        ((unsigned char *)Long)[1] = (unsigned char)(PutValue>>8);
		((unsigned char *)Long)[2] = (unsigned char)(PutValue>>16);
        ((unsigned char *)Long)[3] = (unsigned char)(PutValue>>24);
    }
}

void CMetadata::Put32u(void* Long, unsigned long PutValue)
{
    if (m_bMotorolaOrder)
	{
        ((unsigned char *)Long)[0] = (unsigned char)(PutValue>>24);
		((unsigned char *)Long)[1] = (unsigned char)(PutValue>>16);
		((unsigned char *)Long)[2] = (unsigned char)(PutValue>>8);
        ((unsigned char *)Long)[3] = (unsigned char)PutValue;
    }
	else
	{
        ((unsigned char *)Long)[0] = (unsigned char)PutValue;
        ((unsigned char *)Long)[1] = (unsigned char)(PutValue>>8);
		((unsigned char *)Long)[2] = (unsigned char)(PutValue>>16);
        ((unsigned char *)Long)[3] = (unsigned char)(PutValue>>24);
    }
}

/*--------------------------------------------------------------------------
   Endianess Change
//------------------------------------------------------------------------*/
void CMetadata::SwabArrayOfDword(register DWORD* lp, register DWORD n)
{
	register unsigned char *cp;
	register unsigned char t;

	while (n-- > 0)
	{
		cp = (unsigned char *)lp;
		t = cp[3]; cp[3] = cp[0]; cp[0] = t;
		t = cp[2]; cp[2] = cp[1]; cp[1] = t;
		lp++;
	}
}

/*--------------------------------------------------------------------------
   Parse one of the nested EXIF directories
--------------------------------------------------------------------------*/
bool CMetadata::ParseTIFFDir(	int nIFD,
								bool bOnlyParseGivenIFD,
								bool bExifSection,
								unsigned char* pData,
								unsigned char* pOffsetBase,
								int nLength,
								CMetadata::EXIFINFO* pExifInfo)
{
    int de;
    int a;
    unsigned int ThumbnailOffset = 0;
    unsigned int ThumbnailSize = 0;

    int NumDirEntries = Get16u(pData);

    if ((pData+2+NumDirEntries*12) > (pOffsetBase+nLength))
	{
        TRACE(_T("Illegally sized directory\n"));
		return false;
    }

    for (de = 0 ; de < NumDirEntries ; de++)
	{
        int Tag, Format, Components;
		unsigned int OffsetVal;
        unsigned char* ValuePtr;
        /* This actually can point to a variety of things; it must be
           cast to other types when used.  But we use it as a unsigned char-by-unsigned char
           cursor, so we declare it as a pointer to a generic unsigned char here.
        */
        int BytesCount;
        unsigned char * DirEntry;
        DirEntry = pData+2+12*de;

        Tag = Get16u(DirEntry);
        Format = Get16u(DirEntry+2);
        Components = Get32u(DirEntry+4);

        if ((Format-1) >= NUM_FORMATS)
		{
            /* (-1) catches illegal zero case as unsigned underflows to positive large */
            TRACE(_T("Illegal format code in EXIF dir\n"));
			return false;
		}

        BytesCount = Components * m_BytesPerFormat[Format];

        if (BytesCount > 4)
		{
            OffsetVal = Get32u(DirEntry+8);
            /* If its bigger than 4 unsigned chars, the dir entry contains an offset.*/
            if ((int)OffsetVal+BytesCount > nLength)
			{
                /* Bogus pointer offset and / or unsigned char count value */
                TRACE(_T("Illegal pointer offset value in EXIF.\n"));
				return false;
            }
            ValuePtr = pOffsetBase+OffsetVal;
			if (OffsetVal > m_uiMaxOffset)
				m_uiMaxOffset = OffsetVal;
        }
		else
		{
            /* 4 unsigned chars or less and value is in the dir entry itself */
            ValuePtr = DirEntry+8;
        }

        // Extract useful components of tag
        switch (Tag)
		{
            case TAG_MAKE:
                strncpy(pExifInfo->CameraMake, (char*)ValuePtr, 31);
				pExifInfo->CameraMake[31] = '\0';
                break;

            case TAG_MODEL:
                strncpy(pExifInfo->CameraModel, (char*)ValuePtr, 39);
				pExifInfo->CameraModel[39] = '\0';
                break;

			case TAG_EXIF_VERSION:
				strncpy(pExifInfo->Version, (char*)ValuePtr, 4);
				pExifInfo->Version[4] = '\0';
				break;

            case TAG_DATETIME_ORIGINAL:
				if (m_bDoWrite && m_ExifInfoWrite.bDateTime)
				{
					strncpy((char*)ValuePtr, pExifInfo->DateTime, 19);
					((char*)ValuePtr)[19] = '\0';
				}
				else
				{
					strncpy(pExifInfo->DateTime, (char*)ValuePtr, 19);
					pExifInfo->DateTime[19] = '\0';
				}
				break;

			case TAG_DATETIME_DIGITIZED:
            case TAG_DATETIME:
				if (m_bDoWrite && m_ExifInfoWrite.bDateTime)
				{
					strncpy((char*)ValuePtr, pExifInfo->DateTime, 19);
					((char*)ValuePtr)[19] = '\0';
				}
				else if (!pExifInfo->DateTime[0])
				{
					// If we don't already have a DATETIME_ORIGINAL, use whatever
					// time fields we may have
					strncpy(pExifInfo->DateTime, (char*)ValuePtr, 19);
					pExifInfo->DateTime[19] = '\0';
				}
				break;

			case TAG_IMAGE_DESC:
                // Copy the image description which is always ASCII
                strncpy(pExifInfo->ImageDescription, (const char*)ValuePtr,
						(BytesCount > MAX_IMAGE_DESC) ? MAX_IMAGE_DESC  : BytesCount);
				pExifInfo->ImageDescription[(BytesCount > MAX_IMAGE_DESC) ?
											(MAX_IMAGE_DESC - 1)  : (BytesCount - 1)] = '\0';

				// Remove trailing spaces
                for (a = (strlen(pExifInfo->ImageDescription) - 1) ; a >= 0 ; a--)
				{
                    if (pExifInfo->ImageDescription[a] == ' ')
                        pExifInfo->ImageDescription[a] = '\0';
					else
                        break;
                }
                break;

            case TAG_USER_COMMENT:
                /* Copy the comment
				** The first 8 bytes are used to specify the character encoding, see Exif 2.2 spec
				*/
				if (BytesCount < 8)
					break;

                if (memcmp(ValuePtr, m_USER_COMMENT_ASCII, 8) == 0) // ASCII
				{
					mbstowcs(pExifInfo->UserComment, (const char*)(ValuePtr+8),
							((BytesCount - 8) > MAX_USER_COMMENT) ?
							MAX_USER_COMMENT : (BytesCount - 8));
					pExifInfo->UserComment[	((BytesCount - 8) > MAX_USER_COMMENT) ?
										(MAX_USER_COMMENT - 1) : (BytesCount - 9)] = L'\0';
                }
				else if (memcmp(ValuePtr, m_USER_COMMENT_UNDEFINED, 8) == 0) // Undefined
				{
					break;
				}
				else if (memcmp(ValuePtr, m_USER_COMMENT_JIS, 8) == 0) // JIS
				{
					wcscpy(pExifInfo->UserComment, L"JIS Not Supported");
					break;
				}
				else if (memcmp(ValuePtr, m_USER_COMMENT_UNICODE, 8) == 0) // Unicode
				{
					memcpy(	pExifInfo->UserComment, ValuePtr+8,
							((BytesCount - 8) > MAX_USER_COMMENT) ?
							MAX_USER_COMMENT : (BytesCount - 8));
					pExifInfo->UserComment[	((BytesCount - 8) > MAX_USER_COMMENT) ?
										(MAX_USER_COMMENT - 1) : (BytesCount - 9)] = L'\0';
                }
				else
					break;

				// Remove trailing spaces
                for (a = (wcslen(pExifInfo->UserComment) - 1) ; a >= 0 ; a--)
				{
                    if (pExifInfo->UserComment[a] == L' ')
                        pExifInfo->UserComment[a] = L'\0';
					else
                        break;
                }
                break;

			case TAG_SOFTWARE:
				// Copy the Software Version Information which is always ASCII
                strncpy(pExifInfo->Software, (const char*)ValuePtr,
						(BytesCount > MAX_SOFTWARE) ? MAX_SOFTWARE  : BytesCount);
				pExifInfo->Software[(BytesCount > MAX_SOFTWARE) ?
									(MAX_SOFTWARE - 1)  : (BytesCount - 1)] = '\0';
				break;

			case TAG_ARTIST:
				// Copy the Artist Information which is always ASCII
                strncpy(pExifInfo->Artist, (const char*)ValuePtr,
						(BytesCount > MAX_ARTIST) ? MAX_ARTIST  : BytesCount);
				pExifInfo->Artist[	(BytesCount > MAX_ARTIST) ?
									(MAX_ARTIST - 1)  : (BytesCount - 1)] = '\0';
				break;

			case TAG_COPYRIGHT:
			{
				// Copy the Copyright Information which is always ASCII,
				// but uses a '\0' to separate the photographer from the
				// editor copyright! 
				int i, k;
                for (i = 0 ; i < BytesCount ; i++)
				{
					if (ValuePtr[i] == '\0')
					{
						i++;
						memcpy(pExifInfo->CopyrightPhotographer, ValuePtr, i);
						break;
					}
				}
				for (k = i ; k < BytesCount ; k++)
				{
					if (ValuePtr[k] == '\0')
					{
						k++;
						memcpy(pExifInfo->CopyrightEditor, ValuePtr+i, k - i);
						break;
					}
				}
				pExifInfo->CopyrightPhotographer[MAX_COPYRIGHT-1] = '\0';
				pExifInfo->CopyrightEditor[MAX_COPYRIGHT-1] = '\0';
				break;
			}

            case TAG_FNUMBER:
                /* Simplest way of expressing aperture, so I trust it the most.
                   (overwrite previously computd value if there is one)
                   */
                pExifInfo->ApertureFNumber = (float)ConvertFromAnyFormat(ValuePtr, Format);
                break;

            case TAG_APERTURE:
            case TAG_MAXAPERTURE:
                /* More relevant info always comes earlier, so only
                 use this field if we don't have appropriate aperture
                 information yet. 
                */
                if (pExifInfo->ApertureFNumber == 0)
				{
                    pExifInfo->ApertureFNumber = (float)exp(ConvertFromAnyFormat(ValuePtr, Format)*log(2.0)*0.5);
                }
                break;

			case TAG_BRIGHTNESS:
				pExifInfo->Brightness = (float)ConvertFromAnyFormat(ValuePtr, Format);
				break;

			case TAG_AMBIENT_TEMPERATURE:
				pExifInfo->AmbientTemperature = (float)ConvertFromAnyFormat(ValuePtr, Format);
				break;

            case TAG_FOCALLENGTH:
                /* Nice digital cameras actually save the focal length
                   as a function of how farthey are zoomed in. 
                */
                pExifInfo->FocalLength = (float)ConvertFromAnyFormat(ValuePtr, Format);
                break;

            case TAG_SUBJECT_DISTANCE:
                /* Indicates the distance the autofocus camera is focused to.
                   Tends to be less accurate as distance increases.
                */
                pExifInfo->Distance = (float)ConvertFromAnyFormat(ValuePtr, Format);
                break;

            case TAG_EXPOSURETIME:
                /* Simplest way of expressing exposure time, so I
                   trust it most.  (overwrite previously computd value
                   if there is one) 
                */
                pExifInfo->ExposureTime = 
                    (float)ConvertFromAnyFormat(ValuePtr, Format);
                break;

            case TAG_SHUTTERSPEED:
                /* More complicated way of expressing exposure time,
                   so only use this value if we don't already have it
                   from somewhere else.  
                */
                if (pExifInfo->ExposureTime == 0)
				{
                    pExifInfo->ExposureTime = (float)
                        (1/exp(ConvertFromAnyFormat(ValuePtr, Format)*log(2.0)));
                }
                break;

            case TAG_FLASH:
                pExifInfo->Flash = (int)ConvertFromAnyFormat(ValuePtr, Format);
                break;

            case TAG_ORIENTATION:
				if (bExifSection)
				{
					if (nIFD == 0) // IFD0
					{
						if (m_bDoWrite && m_ExifInfoWrite.bOrientation)
							ConvertToValue(ValuePtr, (double)(pExifInfo->Orientation), Format);
						else
						{
							pExifInfo->Orientation = (int)ConvertFromAnyFormat(ValuePtr, Format);
							if (pExifInfo->Orientation < 1 || pExifInfo->Orientation > 8)
							{
								TRACE(_T("Undefined rotation value\n"));
								pExifInfo->Orientation = 0;
							}
						}
					}
					else if (nIFD == 1) // IFD1 (Thumbnail)
					{
						if (m_bDoWrite && m_ExifInfoWrite.bThumbnailOrientation)
							ConvertToValue(ValuePtr, (double)(pExifInfo->ThumbnailOrientation), Format);
						else
						{
							pExifInfo->ThumbnailOrientation = (int)ConvertFromAnyFormat(ValuePtr, Format);
							if (pExifInfo->ThumbnailOrientation < 1 || pExifInfo->ThumbnailOrientation > 8)
							{
								TRACE(_T("Undefined thumbnail rotation value\n"));
								pExifInfo->ThumbnailOrientation = 0;
							}
						}
					}
				}
				else
				{
					if (m_bDoWrite && m_ExifInfoWrite.bOrientation)
						ConvertToValue(ValuePtr, (double)(pExifInfo->Orientation), Format);
					else
					{
						pExifInfo->Orientation = (int)ConvertFromAnyFormat(ValuePtr, Format);
						if (pExifInfo->Orientation < 1 || pExifInfo->Orientation > 8)
						{
							TRACE(_T("Undefined rotation value\n"));
							pExifInfo->Orientation = 0;
						}
					}
				}
                break;

			case TAG_HEIGHT:
				if (bExifSection)
				{
					if (nIFD == 1) // IFD1 (Thumbnail)
					{
						if (m_bDoWrite && m_ExifInfoWrite.bThumbnailHeight)
							ConvertToValue(ValuePtr, (double)(pExifInfo->ThumbnailHeight), Format);
						else
							pExifInfo->ThumbnailHeight = (int)ConvertFromAnyFormat(ValuePtr, Format);
					}
					else if (nIFD == 0) // IFD0
					{
						/* Use largest of height and width to deal with images
						   that have been rotated to portrait format.  
						*/
						a = (int)ConvertFromAnyFormat(ValuePtr, Format);
						if (m_nExifImageWidth < a)
							m_nExifImageWidth = a;
					}
				}
				else
				{
					/* Use largest of height and width to deal with images
					   that have been rotated to portrait format.  
					*/
					a = (int)ConvertFromAnyFormat(ValuePtr, Format);
					if (m_nExifImageWidth < a)
						m_nExifImageWidth = a;
				}
				break;

            case TAG_EXIF_IMAGEHEIGHT:
				if (m_bDoWrite && m_ExifInfoWrite.bHeight)
					ConvertToValue(ValuePtr, (double)(pExifInfo->Height), Format);
				/* Use largest of height and width to deal with images
				   that have been rotated to portrait format.  
				*/
				a = (int)ConvertFromAnyFormat(ValuePtr, Format);
				if (m_nExifImageWidth < a)
					m_nExifImageWidth = a;
                break;

			case TAG_WIDTH:
				if (bExifSection)
				{
					if (nIFD == 1) // IFD1 (Thumbnail)
					{
						if (m_bDoWrite && m_ExifInfoWrite.bThumbnailWidth)
							ConvertToValue(ValuePtr, (double)(pExifInfo->ThumbnailWidth), Format);
						else
							pExifInfo->ThumbnailWidth = (int)ConvertFromAnyFormat(ValuePtr, Format);
					}
					else if (nIFD == 0) // IFD0
					{
						/* Use largest of height and width to deal with images
						   that have been rotated to portrait format.  
						*/
						a = (int)ConvertFromAnyFormat(ValuePtr, Format);
						if (m_nExifImageWidth < a)
							m_nExifImageWidth = a;
					}
				}
				else
				{
					/* Use largest of height and width to deal with images
					   that have been rotated to portrait format.  
					*/
					a = (int)ConvertFromAnyFormat(ValuePtr, Format);
					if (m_nExifImageWidth < a)
						m_nExifImageWidth = a;
				}
				break;

			case TAG_EXIF_IMAGEWIDTH:
				if (m_bDoWrite && m_ExifInfoWrite.bWidth)
					ConvertToValue(ValuePtr, (double)(pExifInfo->Width), Format);
				/* Use largest of height and width to deal with images
				   that have been rotated to portrait format.  
				*/
				a = (int)ConvertFromAnyFormat(ValuePtr, Format);
				if (m_nExifImageWidth < a)
					m_nExifImageWidth = a;
                break;

            case TAG_FOCALPLANEXRES:
                pExifInfo->FocalplaneXRes = (float)ConvertFromAnyFormat(ValuePtr, Format);
                break;

            case TAG_FOCALPLANEYRES:
                pExifInfo->FocalplaneYRes = (float)ConvertFromAnyFormat(ValuePtr, Format);
                break;

            case TAG_FOCALPLANEUNITS:
                switch((int)ConvertFromAnyFormat(ValuePtr, Format))
				{
					case 1: pExifInfo->FocalplaneUnits = 25.4f; break;	// inch
                    case 2: pExifInfo->FocalplaneUnits = 25.4f; break;	// inch
                    case 3: pExifInfo->FocalplaneUnits = 10.0f;	break;  // centimeter
                    case 4: pExifInfo->FocalplaneUnits = 1.0f;	break;  // millimeter
                    case 5: pExifInfo->FocalplaneUnits = 0.001f;break;  // micrometer
                }
                break;

            case TAG_EXPOSURE_BIAS:
                pExifInfo->ExposureBias = (float)ConvertFromAnyFormat(ValuePtr, Format);
                break;

            case TAG_WHITEBALANCE:
                pExifInfo->WhiteBalance = (int)ConvertFromAnyFormat(ValuePtr, Format);
                break;

			case TAG_LIGHT_SOURCE:
                pExifInfo->LightSource = (int)ConvertFromAnyFormat(ValuePtr, Format);
                break;

            case TAG_METERING_MODE:
                pExifInfo->MeteringMode = (int)ConvertFromAnyFormat(ValuePtr, Format);
                break;

            case TAG_EXPOSURE_PROGRAM:
                pExifInfo->ExposureProgram = (int)ConvertFromAnyFormat(ValuePtr, Format);
                break;

			case TAG_EXPOSURE_INDEX:
                if (pExifInfo->ISOequivalent == 0)
				{
                    // Exposure index and ISO equivalent are often used interchangeably,
                    // http://photography.about.com/library/glossary/bldef_ei.htm
                    pExifInfo->ISOequivalent = (unsigned short)ConvertFromAnyFormat(ValuePtr, Format);
                }
                break;

			case TAG_EXPOSURE_MODE:
                pExifInfo->ExposureMode = (int)ConvertFromAnyFormat(ValuePtr, Format);
                break;

            case TAG_ISO_EQUIVALENT:
                pExifInfo->ISOequivalent = (unsigned short)ConvertFromAnyFormat(ValuePtr, Format);
                if (pExifInfo->ISOequivalent < 50)
					pExifInfo->ISOequivalent *= 200;
                break;

            case TAG_COMPRESSION_LEVEL:
                pExifInfo->CompressionLevel = (float)ConvertFromAnyFormat(ValuePtr, Format);
                break;

            case TAG_XRESOLUTION:
                if (!bExifSection || nIFD != 1) // some exif thumbs have a dpi value set...
					pExifInfo->Xresolution = (float)ConvertFromAnyFormat(ValuePtr, Format);
                break;

            case TAG_YRESOLUTION:
                if (!bExifSection || nIFD != 1) // some exif thumbs have a dpi value set...
					pExifInfo->Yresolution = (float)ConvertFromAnyFormat(ValuePtr, Format);
                break;

			case TAG_RESOLUTIONUNIT:
				if (!bExifSection || nIFD != 1) // some exif thumbs have a dpi value set...
				{
					switch ((int)ConvertFromAnyFormat(ValuePtr, Format))
					{
						case 1: pExifInfo->ResolutionUnit = 1.0f;			break;	// inch
						case 2:	pExifInfo->ResolutionUnit = 1.0f;			break;	// inch
						case 3: pExifInfo->ResolutionUnit = 0.393701f;		break;	// centimeter
						case 4: pExifInfo->ResolutionUnit = 0.0393701f;		break;	// millimeter
						case 5: pExifInfo->ResolutionUnit = 0.0000393701f;	break;	// micrometer
					}
				}
                break;

			case TAG_COMPRESSION:
				if (bExifSection && nIFD == 1)
				{
					if (m_bDoWrite && m_ExifInfoWrite.bThumbnailCompression)
						ConvertToValue(ValuePtr, (double)(pExifInfo->ThumbnailCompression), Format);
					else
						pExifInfo->ThumbnailCompression = (int)ConvertFromAnyFormat(ValuePtr, Format);
				}
				break;

			case TAG_COEFFICIENTS:
				if (bExifSection && nIFD == 1)
				{
					pExifInfo->dThumbnailYCbCrCoeff[0] = ConvertFromAnyFormat(ValuePtr, Format);
					pExifInfo->dThumbnailYCbCrCoeff[1] = ConvertFromAnyFormat(ValuePtr+m_BytesPerFormat[Format], Format);
					pExifInfo->dThumbnailYCbCrCoeff[2] = ConvertFromAnyFormat(ValuePtr+2*m_BytesPerFormat[Format], Format);
				}
				break;

			case TAG_YCBCRSUBSAMPL:
				if (bExifSection && nIFD == 1)
				{
					pExifInfo->ThumbnailYCbCrSubSampling[0] = (int)ConvertFromAnyFormat(ValuePtr, Format);
					pExifInfo->ThumbnailYCbCrSubSampling[1] = (int)ConvertFromAnyFormat(ValuePtr+m_BytesPerFormat[Format], Format);
				}
				break;

			case TAG_REFERENCEBW:
				if (bExifSection && nIFD == 1)
				{
					pExifInfo->dThumbnailRefBW[0] = ConvertFromAnyFormat(ValuePtr, Format);
					pExifInfo->dThumbnailRefBW[1] = ConvertFromAnyFormat(ValuePtr+m_BytesPerFormat[Format], Format);
					pExifInfo->dThumbnailRefBW[2] = ConvertFromAnyFormat(ValuePtr+2*m_BytesPerFormat[Format], Format);
					pExifInfo->dThumbnailRefBW[3] = ConvertFromAnyFormat(ValuePtr+3*m_BytesPerFormat[Format], Format);
					pExifInfo->dThumbnailRefBW[4] = ConvertFromAnyFormat(ValuePtr+4*m_BytesPerFormat[Format], Format);
					pExifInfo->dThumbnailRefBW[5] = ConvertFromAnyFormat(ValuePtr+5*m_BytesPerFormat[Format], Format);
				}
				break;

			case TAG_PHOTOMETRIC:
				if (bExifSection && nIFD == 1)
				{
					if (m_bDoWrite && m_ExifInfoWrite.bThumbnailPhotometricInterpretation)
						ConvertToValue(ValuePtr, (double)(pExifInfo->ThumbnailPhotometricInterpretation), Format);
					else
						pExifInfo->ThumbnailPhotometricInterpretation  = (int)ConvertFromAnyFormat(ValuePtr, Format);
				}
				break;

			case TAG_DIGITALZOOMRATIO:
					pExifInfo->DigitalZoomRatio = (float)ConvertFromAnyFormat(ValuePtr, Format);
                break;

            case TAG_JPEGIFOFFSET:
				if (bExifSection && nIFD == 1)
					ThumbnailOffset = (unsigned int)ConvertFromAnyFormat(ValuePtr, Format);
                break;

            case TAG_JPEGIFBYTECOUNT:
				if (bExifSection && nIFD == 1)
					ThumbnailSize = (unsigned int)ConvertFromAnyFormat(ValuePtr, Format);
                break;

			case TAG_STRIPOFFSETS:
				if (bExifSection && nIFD == 1)
					ThumbnailOffset  = (unsigned int)ConvertFromAnyFormat(ValuePtr, Format);
				break;

			case TAG_STRIPBYTECOUNTS:
				if (bExifSection && nIFD == 1)
					ThumbnailSize  = (unsigned int)ConvertFromAnyFormat(ValuePtr, Format);
				break;

			case TAG_FOCALLENGTH_35MM:
                // The focal length equivalent 35 mm is a 2.2 tag (defined as of April 2002)
                // if its present, use it to compute equivalent focal length instead of 
                // computing it from sensor geometry and actual focal length.
                pExifInfo->FocalLength35mmEquiv = (int)ConvertFromAnyFormat(ValuePtr, Format);
                break;

			case TAG_DISTANCE_RANGE:
                // Three possible standard values:
                //   1 = macro, 2 = close, 3 = distant
                pExifInfo->DistanceRange = (int)ConvertFromAnyFormat(ValuePtr, Format);
                break;

			case TAG_XMLPACKET: // Xmp Packet only in Tiff files
			{
				if (m_pXmpData)
				{	
					delete [] m_pXmpData;
					m_pXmpData = NULL;
				}
				m_pXmpData = new BYTE[BytesCount];
				if (m_pXmpData)
				{
					m_dwXmpSize = BytesCount;
					memcpy(m_pXmpData, ValuePtr, BytesCount);
					ParseXMP(m_pXmpData, (int)m_dwXmpSize);
				}
				break;
			}

			case TAG_RICHTIFFIPTC: // Legacy Iptc only in Tiff files
			{
				DWORD dwDWordsCount = (BytesCount % 4) ? (BytesCount / 4) + 1 : BytesCount / 4;
				m_dwIptcLegacySize = 0;
				m_dwIptcLegacyPreSize = 0;
				m_dwIptcLegacyPostSize = 0;
				if (m_pIptcLegacyPreData)
				{
					delete [] m_pIptcLegacyPreData;
					m_pIptcLegacyPreData = NULL;
				}
				if (m_pIptcLegacyPostData)
				{
					delete [] m_pIptcLegacyPostData;
					m_pIptcLegacyPostData = NULL;
				}
				if (m_pIptcLegacyData)
				{	
					delete [] m_pIptcLegacyData;
					m_pIptcLegacyData = NULL;
				}
				m_pIptcLegacyData = (LPBYTE)(new DWORD[dwDWordsCount]);
				if (m_pIptcLegacyData)
				{
					m_dwIptcLegacySize = BytesCount;
					memset(m_pIptcLegacyData, 0, 4 * dwDWordsCount);
					memcpy(m_pIptcLegacyData, ValuePtr, BytesCount);
					if (m_pIptcLegacyData[0] != 0x1C && m_pIptcLegacyData[3] == 0x1C)
						SwabArrayOfDword((DWORD*)(m_pIptcLegacyData), dwDWordsCount);
					ParseIPTCLegacy(m_pIptcLegacyData, (int)m_dwIptcLegacySize);
				}
				break;
			}

			case TAG_ICCPROFILE: // Icc Color Profile only in Tiff files
			{
				if (m_pIccData)
				{	
					delete [] m_pIccData;
					m_pIccData = NULL;
				}
				m_pIccData = new BYTE[BytesCount];
				if (m_pIccData)
				{
					m_dwIccSize = BytesCount;
					memcpy(m_pIccData, ValuePtr, BytesCount);
				}
				break;
			}

			case TAG_PHOTOSHOP: // Photoshop specific data only in Tiff files
			{
				if (m_pPhotoshopData)
				{	
					delete [] m_pPhotoshopData;
					m_pPhotoshopData = NULL;
				}
				m_pPhotoshopData = new BYTE[BytesCount];
				if (m_pPhotoshopData)
				{
					m_dwPhotoshopSize = BytesCount;
					memcpy(m_pPhotoshopData, ValuePtr, BytesCount);
				}
				break;
			}

			case TAG_MAKER_NOTE:
				ParseMakerNote(	ValuePtr,
								pOffsetBase,
								nLength,
								pExifInfo);
				break;

			case TAG_GPS_SUBIFD:
            {
                unsigned char * SubdirStart;
				unsigned int Offset = Get32u(ValuePtr);
				SubdirStart = pOffsetBase + Offset;
                if (SubdirStart < pOffsetBase || 
					SubdirStart > pOffsetBase+nLength)
				{
					TRACE(_T("Illegal GPS directory link\n"));
					return false;
                }
				else
				{
                    ParseGpsInfo(	SubdirStart,
									pOffsetBase,
									nLength,
									pExifInfo);
                }
				break;
            }

			case TAG_EXIF_SUBIFD :
				// Set Exif Flag
				m_ExifInfo.bHasExif = true;
			case TAG_INTEROP_SUBIFD :
			{		
				unsigned char * SubdirStart;
				unsigned int Offset = Get32u(ValuePtr);
				SubdirStart = pOffsetBase + Offset;
				if (SubdirStart < pOffsetBase || 
					SubdirStart > pOffsetBase+nLength)
				{
					TRACE(_T("Illegal subdirectory link\n"));
					return false;
				}
				if (Offset > m_uiMaxOffset)
					m_uiMaxOffset = Offset;
				ParseTIFFDir(	nIFD,
								bOnlyParseGivenIFD,
								bExifSection,
								SubdirStart,
								pOffsetBase,
								nLength,
								pExifInfo);
				break;
			}

			default:
				break;
        }
    }

	// In addition to linking to subdirectories via exif tags,
	// there's also a potential link to another directory at
	// the end of each directory.
	unsigned int NextDirOffset = Get32u(pData+2+NumDirEntries*12);
	if (NextDirOffset > m_uiMaxOffset)
		m_uiMaxOffset = NextDirOffset;
	if (!bOnlyParseGivenIFD)
	{
		unsigned char* pNextDirStart = GetNextDirPointer(pData, pOffsetBase, nLength);
		if (pNextDirStart)
		{
			ParseTIFFDir(	++nIFD,
							bOnlyParseGivenIFD,
							bExifSection,
							pNextDirStart,
							pOffsetBase,
							nLength,
							pExifInfo);
		}
	}

	// Parsing For Thumbnail
    if (ThumbnailSize && ThumbnailOffset)
	{
        if ((int)(ThumbnailSize + ThumbnailOffset) <= nLength)
		{
            // The thumbnail pointer appears to be valid
			if (ThumbnailOffset > m_uiMaxOffset)
				m_uiMaxOffset = ThumbnailOffset;

			if (m_bDoWrite && m_ExifInfoWrite.bThumbnail && 
				(pExifInfo->ThumbnailSize <= ThumbnailSize))
			{
				memcpy(	pOffsetBase + ThumbnailOffset,
						pExifInfo->ThumbnailPointer,
						pExifInfo->ThumbnailSize);
				memset(	pOffsetBase + ThumbnailOffset + pExifInfo->ThumbnailSize,
						0,
						ThumbnailSize - pExifInfo->ThumbnailSize);
			}
			else
			{
				pExifInfo->ThumbnailOffset = ThumbnailOffset;
				pExifInfo->ThumbnailSize = ThumbnailSize;
				pExifInfo->ThumbnailPointer = m_pAllocatedThumbnailPointer = new BYTE[ThumbnailSize];
				memcpy(pExifInfo->ThumbnailPointer, pOffsetBase + ThumbnailOffset, ThumbnailSize);
			}
        }
    }

	return true;
}


/*--------------------------------------------------------------------------
   Convert To int or float (rationals not supported).
--------------------------------------------------------------------------*/
bool CMetadata::ConvertToValue(void* ValuePtr, double Value, int Format)
{
	switch (Format)
	{
		case FMT_SINGLE:
			*(float*)ValuePtr = (float)Value;
			return true;

		case FMT_DOUBLE:
			*(double*)ValuePtr = Value;
			return true;
		
		case FMT_SBYTE:
			*(char*)ValuePtr = (char)Value;
			return true;

		case FMT_BYTE:      
			*(unsigned char*)ValuePtr = (unsigned char)Value;
			return true;

		case FMT_SSHORT:    
			Put16s(ValuePtr, (short)Value);                
			return true;
		
		case FMT_USHORT:    
			Put16u(ValuePtr, (unsigned short)Value);                
			return true;
		
		case FMT_SLONG:     
			Put32s(ValuePtr, (long)Value);                
			return true;

		case FMT_ULONG:
			Put32u(ValuePtr, (unsigned long)Value);                
			return true;

		case FMT_STRING:
		case FMT_URATIONAL:
		case FMT_SRATIONAL:
		case FMT_UNDEFINED:
		default:
			return false;
	}
}

/*--------------------------------------------------------------------------
   Evaluate number, be it int, rational, or float from directory.
--------------------------------------------------------------------------*/
double CMetadata::ConvertFromAnyFormat(void* ValuePtr, int Format)
{
    double Value;
    Value = 0;

    switch (Format)
	{
        case FMT_SBYTE:     Value = *(signed char *)ValuePtr;  break;
        case FMT_BYTE:      Value = *(unsigned char *)ValuePtr;        break;

        case FMT_USHORT:    Value = Get16u(ValuePtr);          break;
        case FMT_ULONG:     Value = Get32u(ValuePtr);          break;

        case FMT_URATIONAL:
        case FMT_SRATIONAL: 
            {
                int Num,Den;
                Num = Get32s(ValuePtr);
                Den = Get32s(4+(char *)ValuePtr);
                if (Den == 0)
				{
                    Value = 0;
                }
				else
				{
                    Value = (double)Num/Den;
                }
                break;
            }

        case FMT_SSHORT:    Value = (signed short)Get16u(ValuePtr);  break;
        case FMT_SLONG:     Value = Get32s(ValuePtr);                break;

        /* Not sure if this is correct (never seen float used in Exif format)
         */
        case FMT_SINGLE:    Value = (double)*(float*)ValuePtr;      break;
        case FMT_DOUBLE:    Value = *(double*)ValuePtr;             break;
    }
    return Value;
}

bool CMetadata::ParseMakerNote(	unsigned char* pDirStart, 
								unsigned char* pOffsetBase,
								int nLength,
								CMetadata::EXIFINFO* pExifInfo)
{
	// Only Canon supported yet!
    if (strstr(pExifInfo->CameraMake, "Canon"))
        return ParseCanonMakerNote(pDirStart, pOffsetBase, nLength, pExifInfo);
	else
		return false;
}

bool CMetadata::ParseCanonMakerNote(unsigned char* pDirStart,
									unsigned char* pOffsetBase, 
									int nLength,
									CMetadata::EXIFINFO* pExifInfo)
{
    int de;
    int NumDirEntries = Get16u(pDirStart);
    
    // Check
    unsigned char* pDirEnd = DIR_ENTRY_ADDR(pDirStart, NumDirEntries);
    if (pDirEnd > (pOffsetBase+nLength))
	{
        TRACE(_T("Illegally sized directory\n"));
        return false;
    }

    for (de = 0 ; de < NumDirEntries ; de++)
	{
        int Tag, Format, Components;
        unsigned char * ValuePtr;
        int ByteCount;
        unsigned char * DirEntry;
        DirEntry = DIR_ENTRY_ADDR(pDirStart, de);

        Tag = Get16u(DirEntry);
        Format = Get16u(DirEntry+2);
        Components = Get32u(DirEntry+4);

        if ((Format-1) >= NUM_FORMATS)
		{
            // (-1) catches illegal zero case as unsigned underflows to positive large
            TRACE(_T("Illegal number format %d for tag %04x\n"), Format, Tag);
            continue;
        }

        if ((unsigned int)Components > 0x10000)
		{
            TRACE(_T("Illegal number of components %d for tag %04x\n"), Components, Tag);
            continue;
        }

        ByteCount = Components * m_BytesPerFormat[Format];

        if (ByteCount > 4)
		{
            unsigned OffsetVal;
            OffsetVal = Get32u(DirEntry+8);
            // If its bigger than 4 bytes, the dir entry contains an offset
            if (OffsetVal+ByteCount > (unsigned int)nLength)
			{
                // Bogus pointer offset and / or bytecount value
                TRACE(_T("Illegal value pointer for tag %04x\n"), Tag);
                continue;
            }
            ValuePtr = pOffsetBase+OffsetVal;
        }
		else
		{
            // 4 bytes or less and value is in the dir entry itself
            ValuePtr = DirEntry+8;
        }

        if (Tag == 1 && ByteCount >= 17*sizeof(unsigned short))
		{
            int IsoCode = Get16u(ValuePtr + 16*sizeof(unsigned short));
            if (IsoCode >= 16 && IsoCode <= 24)
                pExifInfo->ISOequivalent = (unsigned short)(50 << (IsoCode-16));
        }

        if (Tag == 4 && ByteCount >= 8*sizeof(unsigned short))
		{
            int WhiteBalance = Get16u(ValuePtr + 7*sizeof(unsigned short));
            switch (WhiteBalance)
			{
                // 0=Auto, 6=Custom
                case 1: pExifInfo->LightSource = 1; break; // Sunny
                case 2: pExifInfo->LightSource = 1; break; // Cloudy
                case 3: pExifInfo->LightSource = 3; break; // Thungsten
                case 4: pExifInfo->LightSource = 2; break; // Fluorescent
                case 5: pExifInfo->LightSource = 4; break; // Flash
				default: pExifInfo->LightSource = 0; break;
            }
        }
    }

	return true;
}

void CMetadata::GpsNormalizeCoord(float& fDegree, float& fMinutes, float& fSeconds)
{
	int nDegree = (int)fDegree;
	fMinutes += 60.0f * (fDegree -(float)nDegree);

	int nMinutes = (int)fMinutes;
	fSeconds += 60.0f * (fMinutes -(float)nMinutes);
	
	if (fSeconds >= 60.0f)
	{
		fSeconds -= 60.0f;
		fMinutes += 1.0f;
		nMinutes += 1;
	}
	if (fMinutes >= 60.0f)
	{
		fMinutes -= 60.0f;
		fDegree += 1.0f;
		nDegree += 1;
	}

	fDegree = (float)nDegree;
	fMinutes = (float)nMinutes;
}

void CMetadata::ParseGpsInfo(	unsigned char* pDirStart,
								unsigned char* pOffsetBase,
								int nLength,
								CMetadata::EXIFINFO* pExifInfo)
{
    int NumDirEntries = Get16u(pDirStart);
	pExifInfo->bGpsInfoPresent = true; 

    for (int de = 0 ; de < NumDirEntries ; de++)
	{
        unsigned int Tag, Format, Components;
        unsigned char* ValuePtr;
        int ComponentSize;
        unsigned int ByteCount;
        unsigned char* DirEntry = DIR_ENTRY_ADDR(pDirStart, de);

        Tag = Get16u(DirEntry);
        Format = Get16u(DirEntry+2);
        Components = Get32u(DirEntry+4);

        if ((Format-1) >= NUM_FORMATS)
		{
            TRACE(_T("Illegal number format %d for tag %04x\n"), Format, Tag);
            continue;
        }

        ComponentSize = m_BytesPerFormat[Format];
        ByteCount = Components * ComponentSize;

        if (ByteCount > 4)
		{
            unsigned int OffsetVal;
            OffsetVal = Get32u(DirEntry+8);

            // If its bigger than 4 bytes, the dir entry contains an offset.
            if (OffsetVal + ByteCount > (unsigned int)nLength)
			{
                // Bogus pointer offset and / or bytecount value
                TRACE(_T("Illegal value pointer for tag %04x\n"), Tag, 0);
                continue;
            }
            ValuePtr = pOffsetBase + OffsetVal;
        }
		else
		{
            // 4 bytes or less and value is in the dir entry itself
            ValuePtr = DirEntry+8;
        }

        switch (Tag)
		{
			case TAG_GPS_VERSION :
				memcpy(pExifInfo->GpsVersion, ValuePtr, 4);
				break;

            case TAG_GPS_LAT_REF :
				pExifInfo->GpsLatRef[0] = ValuePtr[0];
				pExifInfo->GpsLatRef[1] = '\0';
                break;

            case TAG_GPS_LONG_REF :
				pExifInfo->GpsLongRef[0] = ValuePtr[0];
				pExifInfo->GpsLongRef[1] = '\0';
                break;

            case TAG_GPS_LAT :
				pExifInfo->GpsLat[GPS_DEGREE]  = (float)ConvertFromAnyFormat(ValuePtr, Format);
				pExifInfo->GpsLat[GPS_MINUTES] = (float)ConvertFromAnyFormat(ValuePtr+ComponentSize, Format);
				pExifInfo->GpsLat[GPS_SECONDS] = (float)ConvertFromAnyFormat(ValuePtr+2*ComponentSize, Format);
				GpsNormalizeCoord(pExifInfo->GpsLat[GPS_DEGREE], pExifInfo->GpsLat[GPS_MINUTES], pExifInfo->GpsLat[GPS_SECONDS]);
				break;

            case TAG_GPS_LONG :
				pExifInfo->GpsLong[GPS_DEGREE]  = (float)ConvertFromAnyFormat(ValuePtr, Format);
				pExifInfo->GpsLong[GPS_MINUTES] = (float)ConvertFromAnyFormat(ValuePtr+ComponentSize, Format);
				pExifInfo->GpsLong[GPS_SECONDS] = (float)ConvertFromAnyFormat(ValuePtr+2*ComponentSize, Format);
				GpsNormalizeCoord(pExifInfo->GpsLong[GPS_DEGREE], pExifInfo->GpsLong[GPS_MINUTES], pExifInfo->GpsLong[GPS_SECONDS]);
				break;

            case TAG_GPS_ALT_REF :
				pExifInfo->GpsAltRef = ValuePtr[0];
                break;

            case TAG_GPS_ALT :
				pExifInfo->GpsAlt = (float)ConvertFromAnyFormat(ValuePtr, Format);
                break;

			case TAG_GPS_TIMESTAMP :
				pExifInfo->GpsTime[GPS_HOUR]    = (float)ConvertFromAnyFormat(ValuePtr, Format);
				pExifInfo->GpsTime[GPS_MINUTES] = (float)ConvertFromAnyFormat(ValuePtr+ComponentSize, Format);
				pExifInfo->GpsTime[GPS_SECONDS] = (float)ConvertFromAnyFormat(ValuePtr+2*ComponentSize, Format);
				break;

			case TAG_GPS_MAPDATUM :
				strncpy(pExifInfo->GpsMapDatum, (const char*)ValuePtr, 20);
				pExifInfo->GpsMapDatum[19] = '\0';
				break;

			default :
				break;
        }
    }
}

void CMetadata::ParseCOMSection(const unsigned char* pData, int nLength)
{
	// We do not know if the comment is NULL terminated!
	LPSTR p = new char[nLength+1];
	if (p)
	{
		memcpy(p, pData, nLength);
		p[nLength] = '\0';
		m_sJpegComment = CStringA(p);
		delete [] p;
	}
	else
		m_sJpegComment = "";
}

void CMetadata::ParseSOFnSection(int nMarker, const unsigned char* pData, int nLength)
{
	m_ExifInfo.JpegSOFnMarker = nMarker;

	// Check Size
	if (nLength < 6)
		return;

	m_ExifInfo.Width = Get16m((void*)(pData+3));
	m_ExifInfo.Height = Get16m((void*)(pData+1));
}

BOOL CMetadata::ParseXMP(const unsigned char* pData, int nLength)
{
	// Clear
	m_IptcFromXmpInfo.Clear();
	m_IptcFromXmpInfo.Encoding = IPTCINFO::IPTC_ENCODING_UTF8;
	m_XmpInfo.Clear();

	// Parse Vars
	CString sXml;
	XNode xml;
	PARSEINFO pi;

	// If Existing
	if (pData && nLength > 0)
	{
		// Convert from UTF8 to CString
		sXml = ::FromUTF8(pData, nLength);

		// Parse Xml
		pi.trim_value = true;				// trim value
		sXml.Replace(_T("\\"), _T("\\\\"));	// escape
		if (xml.Load(sXml, &pi))
		{
			// Find the Photoshop Schema
			LPXNode pXPhotoshop = xml.FindByAttrValue(	_T("rdf:Description"),
														_T("http://ns.adobe.com/photoshop/1.0/"));
			if (pXPhotoshop)
			{
				// Read Prefix
				CString sPrefix = pXPhotoshop->GetAttrName(_T("http://ns.adobe.com/photoshop/1.0/"));
				if (sPrefix.Find(_T("xmlns:"), 0) >= 0)
					sPrefix = sPrefix.Right(sPrefix.GetLength() - 6) + _T(":");

				// Read Strings
				m_IptcFromXmpInfo.CaptionWriter = ReadXmpString(pXPhotoshop, sPrefix + _T("CaptionWriter"));
				m_IptcFromXmpInfo.Headline = ReadXmpString(pXPhotoshop, sPrefix + _T("Headline"));
				m_IptcFromXmpInfo.SpecialInstructions = ReadXmpString(pXPhotoshop, sPrefix + _T("Instructions"));
				m_IptcFromXmpInfo.BylineTitle = ReadXmpString(pXPhotoshop, sPrefix + _T("AuthorsPosition"));
				m_IptcFromXmpInfo.Credits = ReadXmpString(pXPhotoshop, sPrefix + _T("Credit"));
				CString sDate = ReadXmpString(pXPhotoshop, sPrefix + _T("DateCreated"));
				if (sDate != _T(""))
				{
					CTime Date = GetDateFromXmpString(sDate);
					m_IptcFromXmpInfo.DateCreated.Format(_T("%04d%02d%02d"),
														Date.GetYear(),
														Date.GetMonth(),
														Date.GetDay());
				}
				m_IptcFromXmpInfo.City = ReadXmpString(pXPhotoshop, sPrefix + _T("City"));
				m_IptcFromXmpInfo.Source = ReadXmpString(pXPhotoshop, sPrefix + _T("Source"));
				m_IptcFromXmpInfo.ProvinceState = ReadXmpString(pXPhotoshop, sPrefix + _T("State"));
				m_IptcFromXmpInfo.Country = ReadXmpString(pXPhotoshop, sPrefix + _T("Country"));
				m_IptcFromXmpInfo.OriginalTransmissionReference = ReadXmpString(pXPhotoshop, sPrefix + _T("TransmissionReference"));
				m_IptcFromXmpInfo.Category = ReadXmpString(pXPhotoshop, sPrefix + _T("Category"));
				m_IptcFromXmpInfo.Urgency = ReadXmpString(pXPhotoshop, sPrefix + _T("Urgency"));
				ReadXmpStringArray(pXPhotoshop, sPrefix + _T("SupplementalCategories"), m_IptcFromXmpInfo.SupplementalCategories);
			}

			// Find the Dublin Core Schema
			LPXNode pXDublinCore = xml.FindByAttrValue(	_T("rdf:Description"),
														_T("http://purl.org/dc/elements/1.1/"));
			if (pXDublinCore)
			{
				// Read Prefix
				CString sPrefix = pXDublinCore->GetAttrName(_T("http://purl.org/dc/elements/1.1/"));
				if (sPrefix.Find(_T("xmlns:"), 0) >= 0)
					sPrefix = sPrefix.Right(sPrefix.GetLength() - 6) + _T(":");

				// Read Arrays

				CStringArray TempArray;

				ReadXmpStringArray(pXDublinCore, sPrefix + _T("description"), TempArray);
				if (TempArray.GetSize() > 0)
					m_IptcFromXmpInfo.Caption = TempArray[0];

				ReadXmpStringArray(pXDublinCore, sPrefix + _T("rights"), TempArray);
				if (TempArray.GetSize() > 0)
					m_IptcFromXmpInfo.CopyrightNotice = TempArray[0];

				ReadXmpStringArray(pXDublinCore, sPrefix + _T("title"), TempArray);
				if (TempArray.GetSize() > 0)
					m_IptcFromXmpInfo.ObjectName = TempArray[0];

				ReadXmpStringArray(pXDublinCore, sPrefix + _T("creator"), TempArray);
				if (TempArray.GetSize() > 0)
					m_IptcFromXmpInfo.Byline = TempArray[0];

				ReadXmpStringArray(pXDublinCore, sPrefix + _T("subject"), m_IptcFromXmpInfo.Keywords);
			}

			// Find the Right Management Schema
			LPXNode pXRightManagement = xml.FindByAttrValue(_T("rdf:Description"),
															_T("http://ns.adobe.com/xap/1.0/rights/"));
			if (pXRightManagement)
			{
				// Read Prefix
				CString sPrefix = pXRightManagement->GetAttrName(_T("http://ns.adobe.com/xap/1.0/rights/"));
				if (sPrefix.Find(_T("xmlns:"), 0) >= 0)
					sPrefix = sPrefix.Right(sPrefix.GetLength() - 6) + _T(":");

				// Read Strings
				m_XmpInfo.CopyrightUrl = ReadXmpString(pXRightManagement, sPrefix + _T("WebStatement"));
				m_XmpInfo.CopyrightMarked = ReadXmpString(pXRightManagement, sPrefix + _T("Marked"));

				// Read Array
				CStringArray TempArray;
				ReadXmpStringArray(pXRightManagement, sPrefix + _T("UsageTerms"), TempArray);
				if (TempArray.GetSize() > 0)
					m_XmpInfo.UsageTerms = TempArray[0];
			}

			// Find the Iptc4Xmp Schema
			LPXNode pXIptc4Xmp = xml.FindByAttrValue(	_T("rdf:Description"),
														_T("http://iptc.org/std/Iptc4xmpCore/1.0/xmlns/"));
			if (pXIptc4Xmp)
			{
				// Read Prefix
				CString sPrefix = pXIptc4Xmp->GetAttrName(_T("http://iptc.org/std/Iptc4xmpCore/1.0/xmlns/"));
				if (sPrefix.Find(_T("xmlns:"), 0) >= 0)
					sPrefix = sPrefix.Right(sPrefix.GetLength() - 6) + _T(":");

				// Read Creator Contact Info Strings
				LPXNode pXCreatorContactInfo = pXIptc4Xmp->Find(sPrefix + _T("CreatorContactInfo"));
				if (pXCreatorContactInfo)
				{
					m_XmpInfo.CiAdrExtadr = ReadXmpString(pXCreatorContactInfo, sPrefix + _T("CiAdrExtadr"));
					m_XmpInfo.CiAdrCity = ReadXmpString(pXCreatorContactInfo, sPrefix + _T("CiAdrCity"));
					m_XmpInfo.CiAdrRegion = ReadXmpString(pXCreatorContactInfo, sPrefix + _T("CiAdrRegion"));
					m_XmpInfo.CiAdrPcode = ReadXmpString(pXCreatorContactInfo, sPrefix + _T("CiAdrPcode"));
					m_XmpInfo.CiAdrCtry = ReadXmpString(pXCreatorContactInfo, sPrefix + _T("CiAdrCtry"));
					m_XmpInfo.CiTelWork = ReadXmpString(pXCreatorContactInfo, sPrefix + _T("CiTelWork"));
					m_XmpInfo.CiEmailWork = ReadXmpString(pXCreatorContactInfo, sPrefix + _T("CiEmailWork"));
					m_XmpInfo.CiUrlWork = ReadXmpString(pXCreatorContactInfo, sPrefix + _T("CiUrlWork"));
				}

				// Read Strings
				m_XmpInfo.IntellectualGenre = ReadXmpString(pXIptc4Xmp, sPrefix + _T("IntellectualGenre"));
				m_XmpInfo.Location = ReadXmpString(pXIptc4Xmp, sPrefix + _T("Location"));
				m_XmpInfo.CountryCode = ReadXmpString(pXIptc4Xmp, sPrefix + _T("CountryCode"));

				// Read Arrays
				ReadXmpStringArray(pXIptc4Xmp, sPrefix + _T("SubjectCode"), m_XmpInfo.SubjectCode);
				ReadXmpStringArray(pXIptc4Xmp, sPrefix + _T("Scene"), m_XmpInfo.Scene);
			}
		}
		else
			return FALSE;
	}
	else
		return FALSE;

	// Make Line-Breaks CR-LF Pairs
	m_IptcFromXmpInfo.MakeCRLF();
	m_XmpInfo.MakeCRLF();

	return TRUE;
}

void CMetadata::ParseIPTCLegacy(const unsigned char* pData, int nLength)
{
	int ret;
	int pos = 0;
	char record;
	char type;
	CString cnt;

	// Clear
	m_IptcLegacyInfo.Clear();
	m_IptcLegacyInfo.Encoding = IPTCINFO::IPTC_ENCODING_UNSPECIFIED;
	
	while (pos < nLength)
	{
		ret = GetNextIptcLegacyField(pData+pos, record, type, cnt);
		if (ret == 0)
			return;

		switch (type)
		{
			// Version
			case '\x00' :
				if (ret == 2)
					m_IptcLegacyInfo.Version =	(((unsigned short)(pData[pos+5]))<<8) |
												((unsigned short)(pData[pos+6]));
				break;

			// Caption
			case '\x78':
				if (record == 2)
					m_IptcLegacyInfo.Caption = cnt;
				break;
			case '\x7A':
				if (record == 2)
					m_IptcLegacyInfo.CaptionWriter = cnt;
				break;
			case '\x69':
				if (record == 2)
					m_IptcLegacyInfo.Headline = cnt;
				break;
			case '\x28':
				if (record == 2)
					m_IptcLegacyInfo.SpecialInstructions = cnt;
				break;

			// Credits
			case '\x50':
				if (record == 2)
					m_IptcLegacyInfo.Byline = cnt;
				break;
			case '\x55':
				if (record == 2)
					m_IptcLegacyInfo.BylineTitle = cnt;
				break;
			case '\x6E':
				if (record == 2)
					m_IptcLegacyInfo.Credits = cnt;
				break;
			case '\x73':
				if (record == 2)
					m_IptcLegacyInfo.Source = cnt;
				break;
			
			// Origin
			case '\x05':
				if (record == 2)
					m_IptcLegacyInfo.ObjectName = cnt;
				break;
			case '\x37':
				if (record == 2)
					m_IptcLegacyInfo.DateCreated = cnt;
				break;
			case '\x5A':
				// Encoding
				if (record == 1)
				{
					m_IptcLegacyInfo.Encoding = IPTCINFO::IPTC_ENCODING_UNKNOWN;
					if (ret >= UTF8_INVOCATION_SIZE)
					{
						if (memcmp(&pData[pos+5], m_UTF8Invocation, UTF8_INVOCATION_SIZE) == 0)
							m_IptcLegacyInfo.Encoding = IPTCINFO::IPTC_ENCODING_UTF8;
					}
				}
				// City
				else if (record == 2)
					m_IptcLegacyInfo.City = cnt;
				break;
			case '\x5F':
				if (record == 2)
					m_IptcLegacyInfo.ProvinceState = cnt;
				break;
			case '\x65':
				if (record == 2)
					m_IptcLegacyInfo.Country = cnt;
				break;
			case '\x67':
				if (record == 2)
					m_IptcLegacyInfo.OriginalTransmissionReference = cnt;
				break;
			
			// Categories
			case '\x0F':
				if (record == 2)
					m_IptcLegacyInfo.Category = cnt;
				break;
			case '\x14':
				if (record == 2)
					m_IptcLegacyInfo.SupplementalCategories.Add(cnt);
				break;
			case '\x0A':
				if (record == 2)
					m_IptcLegacyInfo.Urgency = cnt;
				break;
			
			// Keywords
			case '\x19':
				if (record == 2)
					m_IptcLegacyInfo.Keywords.Add(cnt);
				break;

			// Copyright
			case '\x74':
				if (record == 2)
					m_IptcLegacyInfo.CopyrightNotice = cnt;
				break;
		}
		pos += ret + 5;
	}

	// Make Line-Breaks CR-LF Pairs
	m_IptcLegacyInfo.MakeCRLF();
}

int CMetadata::GetNextIptcLegacyField(const unsigned char* pData, char& Record, char& Type, CString& Content)
{
	// Check
	if (pData[0] != '\x1c')
		return 0;

	// Record
	Record = pData[1];
	if (Record < 1 || Record > 9)
		return 0;

	// Type
	Type = pData[2];
	
	// Length
	int len = (((int)(pData[3]))<<8) | ((int)(pData[4]));
	
	// Decode from UTF-8?
	if (m_IptcLegacyInfo.Encoding == IPTCINFO::IPTC_ENCODING_UTF8)
	{
		Content = ::FromUTF8(&pData[5], len);
		return len;
	}
	else
	{
		if (pData[5+len-1] == '\0')
			Content = CString(&pData[5]);
		else
		{
			char Buf[MAX_IPTC_FIELD_SIZE];
			memset(Buf, 0, MAX_IPTC_FIELD_SIZE);
			memcpy(Buf, &pData[5], MIN(len, MAX_IPTC_FIELD_SIZE - 1));  
			Content = CString(Buf);
		}
		return len;
	}
}

int CMetadata::MakeIptcLegacyData(LPBYTE* ppData)
{
	// Current Position
	int Pos = 0;
	int Size;
	LPBYTE pData = NULL;

	// Calc. Size and Allocate
	if (ppData != NULL)
	{
		if ((*ppData) != NULL)
			return -1;
		Size = MakeIptcLegacyData(NULL);
		DWORD dwDWordsCount = (Size % 4) ? (Size / 4) + 1 : Size / 4;
		pData = *ppData = (LPBYTE)(new DWORD[dwDWordsCount]);
		if (!pData)
			return -1;
		memset(pData, 0, 4 * dwDWordsCount);
	}

	// Using ANSI Code Page if possible, otherwise UTF-8
	if (m_IptcLegacyInfo.IsANSIConvertible())
		m_IptcLegacyInfo.Encoding = IPTCINFO::IPTC_ENCODING_UNSPECIFIED;
	else
		m_IptcLegacyInfo.Encoding = IPTCINFO::IPTC_ENCODING_UTF8;

	// Set Version
	m_IptcLegacyInfo.Version = IPTC_VERSION;

	// Append Version & Encoding
	AppendIptcLegacyVersionAndEncoding(pData, Pos);
	
	// Caption
	if (m_IptcLegacyInfo.Caption != _T(""))
		AppendIptcLegacyString('\x78', m_IptcLegacyInfo.Caption, pData, Pos);
	if (m_IptcLegacyInfo.CaptionWriter != _T(""))
		AppendIptcLegacyString('\x7A', m_IptcLegacyInfo.CaptionWriter, pData, Pos);
	if (m_IptcLegacyInfo.Headline != _T(""))
		AppendIptcLegacyString('\x69', m_IptcLegacyInfo.Headline, pData, Pos);
	if (m_IptcLegacyInfo.SpecialInstructions != _T(""))
		AppendIptcLegacyString('\x28', m_IptcLegacyInfo.SpecialInstructions, pData, Pos);

	// Credits
	if (m_IptcLegacyInfo.Byline != _T(""))
		AppendIptcLegacyString('\x50', m_IptcLegacyInfo.Byline, pData, Pos);
	if (m_IptcLegacyInfo.BylineTitle != _T(""))
		AppendIptcLegacyString('\x55', m_IptcLegacyInfo.BylineTitle, pData, Pos);	
	if (m_IptcLegacyInfo.Credits != _T(""))
		AppendIptcLegacyString('\x6E', m_IptcLegacyInfo.Credits, pData, Pos);
	if (m_IptcLegacyInfo.Source != _T(""))
		AppendIptcLegacyString('\x73', m_IptcLegacyInfo.Source, pData, Pos);

	// Origin
	if (m_IptcLegacyInfo.ObjectName != _T(""))
		AppendIptcLegacyString('\x05', m_IptcLegacyInfo.ObjectName, pData, Pos);
	if (m_IptcLegacyInfo.DateCreated != _T(""))
		AppendIptcLegacyString('\x37', m_IptcLegacyInfo.DateCreated, pData, Pos);
	if (m_IptcLegacyInfo.City != _T(""))
		AppendIptcLegacyString('\x5A', m_IptcLegacyInfo.City, pData, Pos);
	if (m_IptcLegacyInfo.ProvinceState != _T(""))
		AppendIptcLegacyString('\x5F', m_IptcLegacyInfo.ProvinceState, pData, Pos);
	if (m_IptcLegacyInfo.Country != _T(""))
		AppendIptcLegacyString('\x65', m_IptcLegacyInfo.Country, pData, Pos);
	if (m_IptcLegacyInfo.OriginalTransmissionReference != _T(""))
		AppendIptcLegacyString('\x67', m_IptcLegacyInfo.OriginalTransmissionReference, pData, Pos);

	// Categories
	if (m_IptcLegacyInfo.Category != _T(""))
		AppendIptcLegacyString('\x0F', m_IptcLegacyInfo.Category, pData, Pos);
	if (m_IptcLegacyInfo.SupplementalCategories.GetSize() > 0)
	{
		for (int i = 0 ; i < m_IptcLegacyInfo.SupplementalCategories.GetSize() ; i++)
		{
			if (m_IptcLegacyInfo.SupplementalCategories[i] != _T(""))
				AppendIptcLegacyString('\x14', m_IptcLegacyInfo.SupplementalCategories[i], pData, Pos);
		}
	}
	if (m_IptcLegacyInfo.Urgency != _T(""))
		AppendIptcLegacyString('\x0A', m_IptcLegacyInfo.Urgency, pData, Pos);

	// Keywords
	if (m_IptcLegacyInfo.Keywords.GetSize() > 0)
	{
		for (int i = 0 ; i < m_IptcLegacyInfo.Keywords.GetSize() ; i++)
		{
			if (m_IptcLegacyInfo.Keywords[i] != _T(""))
				AppendIptcLegacyString('\x19', m_IptcLegacyInfo.Keywords[i], pData, Pos);
		}
	}

	// Copyright
	if (m_IptcLegacyInfo.CopyrightNotice != _T(""))
		AppendIptcLegacyString('\x74', m_IptcLegacyInfo.CopyrightNotice, pData, Pos);

	// Assert
#ifdef _DEBUG
	if (ppData != NULL)
	{
		ASSERT(Pos == Size);
	}
#endif

	return Pos;
}

void CMetadata::AppendIptcLegacyVersionAndEncoding(LPBYTE pData, int& Pos)
{
	// Put Record 1 only if encoding with UTF-8
	if (m_IptcLegacyInfo.Encoding == IPTCINFO::IPTC_ENCODING_UTF8)
	{
		BYTE tmp;

		// Signature of Record 1
		if (pData)
			memcpy(&pData[Pos], "\x1c\x01", 2);
		Pos += 2;

		// Type
		if (pData)
			pData[Pos] = 0;
		Pos++;

		// Length (Big Endian)
		unsigned short Len = 2;
		if (pData)
		{
			memcpy(&pData[Pos], &Len, 2);
			tmp = pData[Pos]; pData[Pos] = pData[Pos+1]; pData[Pos+1] = tmp;
		}
		Pos += 2;

		// Version Data (Big Endian)
		if (pData)
		{
			memcpy(&pData[Pos], &m_IptcLegacyInfo.Version, 2);
			tmp = pData[Pos]; pData[Pos] = pData[Pos+1]; pData[Pos+1] = tmp;
		}
		Pos += 2;

		// Signature of Record 1
		if (pData)
			memcpy(&pData[Pos], "\x1c\x01", 2);
		Pos += 2;

		// Type
		if (pData)
			pData[Pos] = '\x5A';
		Pos++;

		// Length (Big Endian)
		Len = UTF8_INVOCATION_SIZE;
		if (pData)
		{
			memcpy(&pData[Pos], &Len, 2);
			tmp = pData[Pos]; pData[Pos] = pData[Pos+1]; pData[Pos+1] = tmp;
		}
		Pos += 2;

		// UTF-8 invocation Data
		if (pData)
			memcpy(&pData[Pos], m_UTF8Invocation, UTF8_INVOCATION_SIZE);
		Pos += UTF8_INVOCATION_SIZE;
	}

	// Version to Record 2
	AppendIptcLegacyShort('\x00', m_IptcLegacyInfo.Version, pData, Pos);
}

void CMetadata::AppendIptcLegacyShort(BYTE Type, unsigned short Value, LPBYTE pData, int& Pos)
{
	BYTE tmp;

	// Signature
	if (pData)
		memcpy(&pData[Pos], "\x1c\x02", 2);
	Pos += 2;

	// Type
	if (pData)
		pData[Pos] = Type;
	Pos++;

	// Length (Big Endian)
	unsigned short Len = 2;
	if (pData)
	{
		memcpy(&pData[Pos], &Len, 2);
		tmp = pData[Pos]; pData[Pos] = pData[Pos+1]; pData[Pos+1] = tmp;
	}
	Pos += 2;

	// Data (Big Endian)
	if (pData)
	{
		memcpy(&pData[Pos], &Value, 2);
		tmp = pData[Pos]; pData[Pos] = pData[Pos+1]; pData[Pos+1] = tmp;
	}
	Pos += 2;
}

// Iptc strings are not '\0' terminated!
void CMetadata::AppendIptcLegacyString(BYTE Type, CString sEntry, LPBYTE pData, int& Pos)
{
	BOOL bUtf8 = (m_IptcLegacyInfo.Encoding == IPTCINFO::IPTC_ENCODING_UTF8);
	BYTE tmp;
	LPBYTE pUtf8 = NULL;
	LPSTR pAnsi = NULL;

	// Check
	if (sEntry.GetLength() <= 0)
		return;

	// Signature
	if (pData)
		memcpy(&pData[Pos], "\x1c\x02", 2);
	Pos += 2;

	// Type
	if (pData)
		pData[Pos] = Type;
	Pos++;

	// Linebreak conversion
	sEntry.Replace(_T("\r\n"), OUT_LINEBREAK);

	// Length (Big Endian)
	unsigned short Len;
	if (bUtf8)
		Len = (unsigned short)::ToUTF8(sEntry, &pUtf8);
	else
		Len = (unsigned short)::ToANSI(sEntry, &pAnsi);
	
	if (pData)
	{
		memcpy(&pData[Pos], &Len, 2);
		tmp = pData[Pos]; pData[Pos] = pData[Pos+1]; pData[Pos+1] = tmp;
	}
	Pos += 2;

	// Data
	if (pData)
	{
		if (bUtf8)
			memcpy(&pData[Pos], pUtf8, Len);
		else
			memcpy(&pData[Pos], pAnsi, Len);
	}
	Pos += Len;

	// Free
	if (pUtf8)
		delete [] pUtf8;
	if (pAnsi)
		delete [] pAnsi;
}

int CMetadata::MakeIptcLegacySection(LPBYTE* ppData)
{
	// Vars
	int nPos = 0;
	int nTotalSize;
	int nSizePadded;
	int nSizePos;

	// Check
	if (!ppData)
		return -1;

	// Merge Iptc
	if (m_pIptcLegacyData &&
		m_dwIptcLegacySize > 0)
	{
		// Make Iptc Data
		if (m_pIptcLegacyData)
		{
			delete [] m_pIptcLegacyData;
			m_pIptcLegacyData = NULL;
		}
		m_dwIptcLegacySize = MakeIptcLegacyData(&m_pIptcLegacyData);

		// Even Padded Iptc Size
		nSizePadded = m_dwIptcLegacySize;
		if (nSizePadded & 0x1)
			nSizePadded++;

		// Total Size
		nTotalSize =	m_dwIptcLegacyPreSize + 
						nSizePadded +
						m_dwIptcLegacyPostSize;

		// Allocate	
		*ppData = new BYTE[nTotalSize];
		if (!(*ppData))
			return -1;
		memset(*ppData, 0, nTotalSize);

		// Copy Pre Data
		if (m_pIptcLegacyPreData &&
			m_dwIptcLegacyPreSize > 0)
		{
			memcpy(	*ppData,
					m_pIptcLegacyPreData,
					m_dwIptcLegacyPreSize);
			nPos += (int)m_dwIptcLegacyPreSize;
		}
		nSizePos = nPos - 4;

		// Copy Iptc Data
		memcpy(	(*ppData) + nPos,
				m_pIptcLegacyData,
				m_dwIptcLegacySize);
		nPos += nSizePadded;
		
		// Copy Post Data
		if (m_pIptcLegacyPostData &&
			m_dwIptcLegacyPostSize > 0)
		{
			memcpy(	(*ppData) + nPos,
					m_pIptcLegacyPostData,
					m_dwIptcLegacyPostSize);
			nPos += (int)m_dwIptcLegacyPostSize;
		}

		// Assert
		ASSERT(nPos == nTotalSize);
	}
	// New Iptc
	else
	{	
		// Clean-Up
		if (m_pIptcLegacyPreData)
		{
			delete [] m_pIptcLegacyPreData;
			m_pIptcLegacyPreData = NULL;
		}
		m_dwIptcLegacyPreSize = 0;
		if (m_pIptcLegacyPostData)
		{
			delete [] m_pIptcLegacyPostData;
			m_pIptcLegacyPostData = NULL;
		}
		m_dwIptcLegacyPostSize = 0;

		// Make Iptc Data
		if (m_pIptcLegacyData)
		{
			delete [] m_pIptcLegacyData;
			m_pIptcLegacyData = NULL;
		}
		m_dwIptcLegacySize = MakeIptcLegacyData(&m_pIptcLegacyData);

		// Even Padded Iptc Size
		nSizePadded = m_dwIptcLegacySize;
		if (nSizePadded & 0x1)
			nSizePadded++;

		// Total Size
		nTotalSize = 26 + nSizePadded;

		// Allocate	
		*ppData = new BYTE[nTotalSize];
		if (!(*ppData))
			return -1;
		memset(*ppData, 0, nTotalSize);

		// Header
		memcpy(*ppData, "Photoshop 3.0", 14);
		nPos += 14;
		memcpy(&(*ppData)[nPos], "8BIM", 4);
		nPos += 4;
		(*ppData)[nPos++] = 0x04;
		(*ppData)[nPos++] = 0x04;
		(*ppData)[nPos++] = 0;
		(*ppData)[nPos++] = 0;
		nSizePos = nPos;
		nPos += 4;
		m_dwIptcLegacyPreSize = nPos;
		m_pIptcLegacyPreData = new BYTE[m_dwIptcLegacyPreSize];
		if (!m_pIptcLegacyPreData)
		{
			m_dwIptcLegacyPreSize = 0;
			return -1;
		}

		// Copy Iptc Data
		memcpy(	(*ppData) + nPos,
				m_pIptcLegacyData,
				m_dwIptcLegacySize);
		nPos += nSizePadded;
		
		// Assert
		ASSERT(nPos == nTotalSize);
	}

	// Set Size
	if (nSizePos >= 0 && nTotalSize >= (nSizePos + 4))
	{
		BYTE t;
		memcpy(&(*ppData)[nSizePos], &m_dwIptcLegacySize, 4);
		t = (*ppData)[nSizePos+3]; (*ppData)[nSizePos+3] = (*ppData)[nSizePos]; (*ppData)[nSizePos] = t;
		t = (*ppData)[nSizePos+2]; (*ppData)[nSizePos+2] = (*ppData)[nSizePos+1]; (*ppData)[nSizePos+1] = t;
	}

	// Copy Back Pre Data with new size value
	if (m_pIptcLegacyPreData &&
		m_dwIptcLegacyPreSize > 0)
	{
		memcpy(	m_pIptcLegacyPreData,
				*ppData,
				m_dwIptcLegacyPreSize);
	}

	return nPos;
}

BOOL CMetadata::UpdateXmpData(const CString& sMime)
{
	CString uuid(_T(""));
	CString sXml;
	XDoc xml;
	PARSEINFO pi;

	// If Existing
	if (m_pXmpData && m_dwXmpSize > 0)
	{
		// Convert from UTF8 to CString
		sXml = ::FromUTF8(	(const unsigned char*)m_pXmpData,
							(int)m_dwXmpSize);

		// Parse Xml
		pi.trim_value = true;				// trim value
		sXml.Replace(_T("\\"), _T("\\\\"));	// escape
		if (!xml.Load(sXml, &pi))
			return FALSE;

		// Set a fake BOM, so that when again in UTF8 mode
		// we have 3 chars space to set the correct BOM!
		LPXAttr pAttr = xml.GetChildAttr(_T("xpacket"), _T("begin"));
		if (pAttr)
			pAttr->value = _T("123");

		// Get uuid
		LPXNode pXFirstDescription = xml.Find(_T("rdf:Description"));
		if (pXFirstDescription)
			uuid = CString(pXFirstDescription->GetAttrValue(_T("rdf:about")));
		else
		{
			// Generate uuid
			uuid = ::GetUuidCString();
			if (uuid != _T(""))
				uuid = _T("uuid:") + uuid;
		}

	}
	// Create new Xmp
	else
	{
		// Generate uuid
		uuid = ::GetUuidCString();
		if (uuid != _T(""))
			uuid = _T("uuid:") + uuid;

		// Create Basic Xmp
		sXml.Format(	_T("<?xpacket begin='123' id='W5M0MpCehiHzreSzNTczkc9d'?>")
						_T("<x:xmpmeta xmlns:x='adobe:ns:meta/'>")
						_T("<rdf:RDF xmlns:rdf='http://www.w3.org/1999/02/22-rdf-syntax-ns#' xmlns:iX='http://ns.adobe.com/iX/1.0/'>")
						
						/*
						_T("<rdf:Description rdf:about='%s'	xmlns:exif='http://ns.adobe.com/exif/1.0/'>")
						_T("</rdf:Description>")

						_T("<rdf:Description rdf:about='%s' xmlns:pdf='http://ns.adobe.com/pdf/1.3/'>")
						_T("</rdf:Description>")

						_T("<rdf:Description rdf:about='%s' xmlns:tiff='http://ns.adobe.com/tiff/1.0/'>")
						_T("</rdf:Description>")
						*/

						_T("<rdf:Description rdf:about='%s' xmlns:xmp='http://ns.adobe.com/xap/1.0/'>")
						_T("<xmp:CreatorTool>%s</xmp:CreatorTool>")
						_T("</rdf:Description>")

						_T("<rdf:Description rdf:about='%s' xmlns:xmpMM='http://ns.adobe.com/xap/1.0/mm/'>")
						_T("<xmpMM:DocumentID>%s:docid:%s:%s</xmpMM:DocumentID>")
						_T("</rdf:Description>")

						_T("<rdf:Description rdf:about='%s' xmlns:dc='http://purl.org/dc/elements/1.1/'>")
						_T("</rdf:Description>")
						
						_T("</rdf:RDF>")
						_T("</x:xmpmeta>")
						_T("<?xpacket end='w'?>"), /*uuid, uuid, uuid,*/ uuid, APPNAME_NOEXT, uuid, MYCOMPANY_WEB, APPNAME_NOEXT_LOW, uuid, uuid);
		
		// Parse Xml
		pi.trim_value = true;	// trim value
		if (!xml.Load(sXml, &pi))
			return FALSE;
	}

	// Find the Photoshop Schema
	BOOL bPhotoshopExisting;
	CString sPhotoshopPrefix;
	LPXNode pXPhotoshop = xml.FindByAttrValue(	_T("rdf:Description"),
												_T("http://ns.adobe.com/photoshop/1.0/"));

	// Existing?
	if (pXPhotoshop)
	{
		// Set Flag
		bPhotoshopExisting = TRUE;

		// Read Prefix
		sPhotoshopPrefix = pXPhotoshop->GetAttrName(_T("http://ns.adobe.com/photoshop/1.0/"));
		if (sPhotoshopPrefix.Find(_T("xmlns:"), 0) >= 0)
			sPhotoshopPrefix = sPhotoshopPrefix.Right(sPhotoshopPrefix.GetLength() - 6) + _T(":");
	}
	// Create one if not existing
	else
	{
		bPhotoshopExisting = FALSE;
		sPhotoshopPrefix = _T("photoshop:");
		pXPhotoshop = new XNode;
		if (pXPhotoshop)
		{
			CString sXPhotoshop;
			sXPhotoshop.Format(	_T("<rdf:Description ")
								_T("rdf:about='%s' ") 
								_T("xmlns:photoshop='http://ns.adobe.com/photoshop/1.0/'>")
								_T("<photoshop:History/>")
								_T("</rdf:Description>"), uuid);
			pXPhotoshop->Load(sXPhotoshop);
		}
		else
			return FALSE;
	}
	
	// Write Strings
	WriteXmpString(pXPhotoshop, sPhotoshopPrefix + _T("CaptionWriter"), m_IptcFromXmpInfo.CaptionWriter);
	WriteXmpString(pXPhotoshop, sPhotoshopPrefix + _T("Headline"), m_IptcFromXmpInfo.Headline);
	WriteXmpString(pXPhotoshop, sPhotoshopPrefix + _T("Instructions"), m_IptcFromXmpInfo.SpecialInstructions);
	WriteXmpString(pXPhotoshop, sPhotoshopPrefix + _T("AuthorsPosition"), m_IptcFromXmpInfo.BylineTitle);
	WriteXmpString(pXPhotoshop, sPhotoshopPrefix + _T("Credit"), m_IptcFromXmpInfo.Credits);
	if (m_IptcFromXmpInfo.DateCreated == _T(""))
		WriteXmpString(pXPhotoshop, sPhotoshopPrefix + _T("DateCreated"), _T(""));
	else
	{
		CTime Date = GetDateFromIptcLegacyString(m_IptcFromXmpInfo.DateCreated);
		
		// Photoshop 7 Supports only the simpler format
		CString sDate;
		sDate.Format(_T("%04d-%02d-%02d"), Date.GetYear(), Date.GetMonth(), Date.GetDay());
		//sDate.Format(_T("%04d-%02d-%02dT00:00:00+01:00"), Date.GetYear(), Date.GetMonth(), Date.GetDay());

		WriteXmpString(pXPhotoshop, sPhotoshopPrefix + _T("DateCreated"), sDate);
	}
	WriteXmpString(pXPhotoshop, sPhotoshopPrefix + _T("City"), m_IptcFromXmpInfo.City);
	WriteXmpString(pXPhotoshop, sPhotoshopPrefix + _T("Source"), m_IptcFromXmpInfo.Source);
	WriteXmpString(pXPhotoshop, sPhotoshopPrefix + _T("State"), m_IptcFromXmpInfo.ProvinceState);
	WriteXmpString(pXPhotoshop, sPhotoshopPrefix + _T("Country"), m_IptcFromXmpInfo.Country);
	WriteXmpString(pXPhotoshop, sPhotoshopPrefix + _T("TransmissionReference"), m_IptcFromXmpInfo.OriginalTransmissionReference);
	WriteXmpString(pXPhotoshop, sPhotoshopPrefix + _T("Category"), m_IptcFromXmpInfo.Category);
	WriteXmpString(pXPhotoshop, sPhotoshopPrefix + _T("Urgency"), m_IptcFromXmpInfo.Urgency);
	WriteXmpStringArray(pXPhotoshop, sPhotoshopPrefix + _T("SupplementalCategories"), m_IptcFromXmpInfo.SupplementalCategories, _T("rdf:Bag"));
	
	// Append the Photoshop Schema
	if (!bPhotoshopExisting)
	{
		LPXNode pXRdf = xml.Find(_T("rdf:RDF"));
		if (pXRdf)
			pXRdf->AppendChild(pXPhotoshop);
	}

	// Find the Dublin Core Schema
	BOOL bDublinCore;
	CString sDublinCorePrefix;
	LPXNode pXDublinCore = xml.FindByAttrValue(	_T("rdf:Description"),
												_T("http://purl.org/dc/elements/1.1/"));

	// Existing?
	if (pXDublinCore)
	{
		// Set Flag
		bDublinCore = TRUE;

		// Read Prefix
		sDublinCorePrefix = pXDublinCore->GetAttrName(_T("http://purl.org/dc/elements/1.1/"));
		if (sDublinCorePrefix.Find(_T("xmlns:"), 0) >= 0)
			sDublinCorePrefix = sDublinCorePrefix.Right(sDublinCorePrefix.GetLength() - 6) + _T(":");
	}
	// Create one if not existing
	else
	{
		bDublinCore = FALSE;
		sDublinCorePrefix = _T("dc:");
		pXDublinCore = new XNode;
		if (pXDublinCore)
		{
			CString sXDublinCore;
			sXDublinCore.Format(_T("<rdf:Description ")
								_T("rdf:about='%s' ") 
								_T("xmlns:dc='http://purl.org/dc/elements/1.1/'>")
								_T("</rdf:Description>"), uuid);
			pXDublinCore->Load(sXDublinCore);
		}
		else
			return FALSE;
	}

	// Update Mime
	LPXNode pDcFormat = pXDublinCore->Find(sDublinCorePrefix + _T("format"));
	if (pDcFormat)
		pDcFormat->value = sMime;
	else
		pXDublinCore->AppendChild(sDublinCorePrefix + _T("format"), sMime);

	// Write Arrays
	CStringArray TempArray;
	if (m_IptcFromXmpInfo.Caption != _T(""))
		TempArray.Add(m_IptcFromXmpInfo.Caption);
	WriteXmpStringArray(pXDublinCore,
						sDublinCorePrefix + _T("description"),
						TempArray,
						_T("rdf:Alt"),
						_T("xml:lang"),
						_T("x-default"));
	TempArray.RemoveAll();
	if (m_IptcFromXmpInfo.CopyrightNotice != _T(""))
		TempArray.Add(m_IptcFromXmpInfo.CopyrightNotice);
	WriteXmpStringArray(pXDublinCore,
						sDublinCorePrefix + _T("rights"),
						TempArray,
						_T("rdf:Alt"),
						_T("xml:lang"),
						_T("x-default"));
	TempArray.RemoveAll();
	if (m_IptcFromXmpInfo.ObjectName != _T(""))
		TempArray.Add(m_IptcFromXmpInfo.ObjectName);
	WriteXmpStringArray(pXDublinCore,
						sDublinCorePrefix + _T("title"),
						TempArray,
						_T("rdf:Alt"),
						_T("xml:lang"),
						_T("x-default"));
	TempArray.RemoveAll();
	if (m_IptcFromXmpInfo.Byline != _T(""))
		TempArray.Add(m_IptcFromXmpInfo.Byline);
	WriteXmpStringArray(pXDublinCore,
						sDublinCorePrefix + _T("creator"),
						TempArray,
						_T("rdf:Seq"));
	WriteXmpStringArray(pXDublinCore,
						sDublinCorePrefix + _T("subject"),
						m_IptcFromXmpInfo.Keywords,
						_T("rdf:Bag"));
	
	// Append the Dublin Core Schema
	if (!bDublinCore)
	{
		LPXNode pXRdf = xml.Find(_T("rdf:RDF"));
		if (pXRdf)
			pXRdf->AppendChild(pXDublinCore);
	}

	// Find the Right Management Schema
	BOOL bRightManagement;
	CString sRightManagementPrefix;
	LPXNode pXRightManagement = xml.FindByAttrValue(_T("rdf:Description"),
													_T("http://ns.adobe.com/xap/1.0/rights/"));

	// Existing?
	if (pXRightManagement)
	{
		// Set Flag
		bRightManagement = TRUE;

		// Read Prefix
		sRightManagementPrefix = pXRightManagement->GetAttrName(_T("http://ns.adobe.com/xap/1.0/rights/"));
		if (sRightManagementPrefix.Find(_T("xmlns:"), 0) >= 0)
			sRightManagementPrefix = sRightManagementPrefix.Right(sRightManagementPrefix.GetLength() - 6) + _T(":");
	}
	// Create one if not existing
	else
	{
		bRightManagement = FALSE;
		sRightManagementPrefix = _T("xmpRights:");
		pXRightManagement = new XNode;
		if (pXRightManagement)
		{
			CString sXDublinCore;
			sXDublinCore.Format(_T("<rdf:Description ")
								_T("rdf:about='%s' ") 
								_T("xmlns:xmpRights='http://ns.adobe.com/xap/1.0/rights/'>")
								_T("</rdf:Description>"), uuid);
			pXRightManagement->Load(sXDublinCore);
		}
		else
			return FALSE;
	}

	// Write Web Statement
	WriteXmpString(pXRightManagement, sRightManagementPrefix + _T("WebStatement"), m_XmpInfo.CopyrightUrl);

	// Write Copyright Marker
	WriteXmpString(pXRightManagement, sRightManagementPrefix + _T("Marked"), m_XmpInfo.CopyrightMarked);
	
	// Write Rights Usage Terms
	TempArray.RemoveAll();
	if (m_XmpInfo.UsageTerms != _T(""))
		TempArray.Add(m_XmpInfo.UsageTerms);
	WriteXmpStringArray(pXRightManagement,
						sRightManagementPrefix + _T("UsageTerms"),
						TempArray,
						_T("rdf:Alt"),
						_T("xml:lang"),
						_T("x-default"));

	// Append the Right Management Schema
	if (!bRightManagement)
	{
		LPXNode pXRdf = xml.Find(_T("rdf:RDF"));
		if (pXRdf)
			pXRdf->AppendChild(pXRightManagement);
	}

	// Find the Iptc4Xmp Schema
	BOOL bIptc4XmpExisting;
	CString sIptc4XmpPrefix;
	LPXNode pXIptc4Xmp = xml.FindByAttrValue(	_T("rdf:Description"),
												_T("http://iptc.org/std/Iptc4xmpCore/1.0/xmlns/"));

	// Existing?
	if (pXIptc4Xmp)
	{
		// Set Flag
		bIptc4XmpExisting = TRUE;

		// Read Prefix
		sIptc4XmpPrefix = pXIptc4Xmp->GetAttrName(_T("http://iptc.org/std/Iptc4xmpCore/1.0/xmlns/"));
		if (sIptc4XmpPrefix.Find(_T("xmlns:"), 0) >= 0)
			sIptc4XmpPrefix = sIptc4XmpPrefix.Right(sIptc4XmpPrefix.GetLength() - 6) + _T(":");
	}
	// Create one if not existing
	else
	{
		bIptc4XmpExisting = FALSE;
		sIptc4XmpPrefix = _T("Iptc4xmpCore:");
		pXIptc4Xmp = new XNode;
		if (pXIptc4Xmp)
		{
			CString sXIptc4Xmp;
			sXIptc4Xmp.Format(	_T("<rdf:Description ")
								_T("rdf:about='%s' ") 
								_T("xmlns:Iptc4xmpCore='http://iptc.org/std/Iptc4xmpCore/1.0/xmlns/'>")
								_T("<Iptc4xmpCore:CreatorContactInfo rdf:parseType='Resource'>")
								_T("</Iptc4xmpCore:CreatorContactInfo>")
								_T("</rdf:Description>"), uuid);
			pXIptc4Xmp->Load(sXIptc4Xmp);
		}
		else
			return FALSE;
	}
	
	// Write Creator Contact Info Strings
	LPXNode pXCreatorContactInfo = pXIptc4Xmp->Find(sIptc4XmpPrefix + _T("CreatorContactInfo"));
	if (pXCreatorContactInfo)
	{
		WriteXmpString(pXCreatorContactInfo, sIptc4XmpPrefix + _T("CiAdrExtadr"), m_XmpInfo.CiAdrExtadr);
		WriteXmpString(pXCreatorContactInfo, sIptc4XmpPrefix + _T("CiAdrCity"), m_XmpInfo.CiAdrCity);
		WriteXmpString(pXCreatorContactInfo, sIptc4XmpPrefix + _T("CiAdrRegion"), m_XmpInfo.CiAdrRegion);
		WriteXmpString(pXCreatorContactInfo, sIptc4XmpPrefix + _T("CiAdrPcode"), m_XmpInfo.CiAdrPcode);
		WriteXmpString(pXCreatorContactInfo, sIptc4XmpPrefix + _T("CiAdrCtry"), m_XmpInfo.CiAdrCtry);
		WriteXmpString(pXCreatorContactInfo, sIptc4XmpPrefix + _T("CiTelWork"), m_XmpInfo.CiTelWork);
		WriteXmpString(pXCreatorContactInfo, sIptc4XmpPrefix + _T("CiEmailWork"), m_XmpInfo.CiEmailWork);
		WriteXmpString(pXCreatorContactInfo, sIptc4XmpPrefix + _T("CiUrlWork"), m_XmpInfo.CiUrlWork);
	}

	// Write Strings
	WriteXmpString(pXIptc4Xmp, sIptc4XmpPrefix + _T("IntellectualGenre"), m_XmpInfo.IntellectualGenre);
	WriteXmpString(pXIptc4Xmp, sIptc4XmpPrefix + _T("Location"), m_XmpInfo.Location);
	WriteXmpString(pXIptc4Xmp, sIptc4XmpPrefix + _T("CountryCode"), m_XmpInfo.CountryCode);

	// Write Arrays
	WriteXmpStringArray(pXIptc4Xmp, sIptc4XmpPrefix + _T("SubjectCode"), m_XmpInfo.SubjectCode, _T("rdf:Bag"));
	WriteXmpStringArray(pXIptc4Xmp, sIptc4XmpPrefix + _T("Scene"), m_XmpInfo.Scene, _T("rdf:Bag"));
	
	// Append the Iptc4Xmp Schema
	if (!bIptc4XmpExisting)
	{
		LPXNode pXRdf = xml.Find(_T("rdf:RDF"));
		if (pXRdf)
			pXRdf->AppendChild(pXIptc4Xmp);
	}

	// Copy to m_pXmpData
	DISP_OPT opt;
	opt.newline = true;
	_tcsncpy(opt.newline_type, _T("\n"), 3);
	opt.value_quotation_mark = _T('\'');
	sXml = xml.GetXML(&opt);
	sXml.TrimLeft(_T('\n')); // Remove initial \n
	if (m_pXmpData)
	{
		delete [] m_pXmpData;
		m_pXmpData = NULL;
	}
	m_dwXmpSize = (DWORD)::ToUTF8(sXml, &m_pXmpData);

	// Restore BOM
	int nBOMPos = 0;
	while (nBOMPos < ((int)m_dwXmpSize - 6) && memcmp(&m_pXmpData[nBOMPos], "begin=", 6) != 0)
		nBOMPos++;
	if (nBOMPos < ((int)m_dwXmpSize - 11) && memcmp(&m_pXmpData[nBOMPos], "begin='123'", 11) == 0)
	{
		m_pXmpData[nBOMPos+7] = 0xEF;
		m_pXmpData[nBOMPos+8] = 0xBB;
		m_pXmpData[nBOMPos+9] = 0xBF;
	}

	return TRUE;
}

CString CMetadata::ReadXmpString(	LPXNode pXNode,
									const CString& name)
{
	LPXNode pXTemp = pXNode->Find(name);
	if (pXTemp)
		return pXTemp->value;
	else
		return _T("");
}

void CMetadata::ReadXmpStringArray(	LPXNode pXNode,
									const CString& name,
									CStringArray& array)
{
	array.RemoveAll();
	LPXNode pXTemp = pXNode->Find(name);
	if (pXTemp)
	{
		if (pXTemp->GetChildCount() == 1)
		{
			LPXNode pXArray = pXTemp->GetChild(0);
			for (int i = 0 ; i < pXArray->GetChildCount() ; i++)
				array.Add(pXArray->GetChild(i)->value);
		}
	}
}

void CMetadata::WriteXmpString(LPXNode pXNode,
							   const CString& name,
							   CString value)
{
	// Linebreak conversion
	value.Replace(_T("\r\n"), OUT_LINEBREAK);

	LPXNode pXTemp = pXNode->Find(name);
	if (value == _T(""))
	{
		if (pXTemp)
			pXNode->RemoveChild(pXTemp);
	}
	else
	{
		if (pXTemp)
			pXTemp->value = value;
		else
			pXNode->AppendChild(name, value);
	}
}

void CMetadata::WriteXmpStringArray(LPXNode pXNode,
									const CString& name,
									const CStringArray& array,
									const CString& arraytype,
									const CString& attrname/*=_T("")*/,
									const CString& attrvalue/*=_T("")*/)
{
	LPXNode pXTemp = pXNode->Find(name);
	if (pXTemp)
		pXNode->RemoveChild(pXTemp);
	if (array.GetSize() > 0)
	{
		pXTemp = new XNode;
		if (pXTemp)
		{
			pXTemp->name = name;
			LPXNode pXBag = pXTemp->AppendChild(arraytype);
			for (int i = 0 ; i < array.GetSize() ; i++)
			{
				// Linebreak conversion
				CString value = array[i];
				value.Replace(_T("\r\n"), OUT_LINEBREAK);

				LPXNode pXListItem = NULL;
				if (value != _T(""))
					pXListItem = pXBag->AppendChild(_T("rdf:li"), value);
				if (pXListItem && attrname != _T(""))
					pXListItem->AppendAttr(attrname, attrvalue);
			}
			pXNode->AppendChild(pXTemp);
		}
	}
}

int CMetadata::MakeXmpSection(LPBYTE* ppData)
{
	// Vars
	int nPos = 0;
	int nTotalSize;

	// Check
	if (!ppData)
		return -1;

	// Update Xmp
	if (UpdateXmpData(_T("image/jpeg")))
	{
		// Set
		nTotalSize = XMP_HEADER_SIZE + m_dwXmpSize;

		// Allocate	
		*ppData = new BYTE[nTotalSize];
		if (!(*ppData))
			return -1;
		memset(*ppData, 0, nTotalSize);

		// Header
		nPos = 0;
		memcpy(*ppData, CMetadata::m_XmpHeader, XMP_HEADER_SIZE);
		nPos += XMP_HEADER_SIZE;
	
		// Copy Xmp Data
		memcpy(	(*ppData) + nPos,
				m_pXmpData,
				m_dwXmpSize);
		nPos += m_dwXmpSize;

		// Assert
		ASSERT(nPos == nTotalSize);

		return nPos;
	}
	else
		return -1;
}

void CMetadata::IPTCINFO::CopyVars(const CMetadata::IPTCINFO& iptcinfo)
{
	int i;

	// Encoding
	Encoding = iptcinfo.Encoding;
	
	// Version
	Version = iptcinfo.Version;

	// Credits
	Byline = iptcinfo.Byline;	
	BylineTitle = iptcinfo.BylineTitle;
	Credits = iptcinfo.Credits;
	Source = iptcinfo.Source;
	
	// Caption
	CaptionWriter = iptcinfo.CaptionWriter;
	Caption = iptcinfo.Caption;
	Headline = iptcinfo.Headline;
	SpecialInstructions = iptcinfo.SpecialInstructions;
	
	// Origin
	ObjectName = iptcinfo.ObjectName;
	DateCreated = iptcinfo.DateCreated;
	City = iptcinfo.City;
	ProvinceState = iptcinfo.ProvinceState;
	Country = iptcinfo.Country;
	OriginalTransmissionReference = iptcinfo.OriginalTransmissionReference;
	
	// Categories
	Category = iptcinfo.Category;
	for (i = 0 ; i < iptcinfo.SupplementalCategories.GetSize() ; i++)
		SupplementalCategories.Add(iptcinfo.SupplementalCategories[i]);
	Urgency = iptcinfo.Urgency;

	// Keywords
	for (i = 0 ; i < iptcinfo.Keywords.GetSize() ; i++)
		Keywords.Add(iptcinfo.Keywords[i]);
	
	// Copyright
	CopyrightNotice = iptcinfo.CopyrightNotice;
}

BOOL CMetadata::IPTCINFO::IsEqual(const CMetadata::IPTCINFO& iptcinfo)
{
	// Credits
	if (Byline != iptcinfo.Byline)
		return FALSE;
	if (BylineTitle != iptcinfo.BylineTitle)
		return FALSE;
	if (Credits != iptcinfo.Credits)
		return FALSE;
	if (Source != iptcinfo.Source)
		return FALSE;
	
	// Caption
	if (CaptionWriter != iptcinfo.CaptionWriter)
		return FALSE;
	if (Caption != iptcinfo.Caption)
		return FALSE;
	if (Headline != iptcinfo.Headline)
		return FALSE;
	if (SpecialInstructions != iptcinfo.SpecialInstructions)
		return FALSE;
	
	// Origin
	if (ObjectName != iptcinfo.ObjectName)
		return FALSE;
	if (DateCreated != iptcinfo.DateCreated)
		return FALSE;
	if (City != iptcinfo.City)
		return FALSE;
	if (ProvinceState != iptcinfo.ProvinceState)
		return FALSE;
	if (Country != iptcinfo.Country)
		return FALSE;
	if (OriginalTransmissionReference != iptcinfo.OriginalTransmissionReference)
		return FALSE;
	
	// Categories
	if (Category != iptcinfo.Category)
		return FALSE;
	if (!IsArrayEqual(SupplementalCategories, iptcinfo.SupplementalCategories))
		return FALSE;
	if (Urgency != iptcinfo.Urgency)
		return FALSE;

	// Keywords
	if (!IsArrayEqual(Keywords, iptcinfo.Keywords))
		return FALSE;
	
	// Copyright
	if (CopyrightNotice != iptcinfo.CopyrightNotice)
		return FALSE;

	return TRUE;
}

BOOL CMetadata::IPTCINFO::IsANSIConvertible()
{
	int i;

	// Credits
	if (!::IsANSIConvertible(Byline))
		return FALSE;
	if (!::IsANSIConvertible(BylineTitle))
		return FALSE;
	if (!::IsANSIConvertible(Credits))
		return FALSE;
	if (!::IsANSIConvertible(Source))
		return FALSE;
	
	// Caption
	if (!::IsANSIConvertible(CaptionWriter))
		return FALSE;
	if (!::IsANSIConvertible(Caption))
		return FALSE;
	if (!::IsANSIConvertible(Headline))
		return FALSE;
	if (!::IsANSIConvertible(SpecialInstructions))
		return FALSE;
	
	// Origin
	if (!::IsANSIConvertible(ObjectName))
		return FALSE;
	if (!::IsANSIConvertible(DateCreated))
		return FALSE;
	if (!::IsANSIConvertible(City))
		return FALSE;
	if (!::IsANSIConvertible(ProvinceState))
		return FALSE;
	if (!::IsANSIConvertible(Country))
		return FALSE;
	if (!::IsANSIConvertible(OriginalTransmissionReference))
		return FALSE;
	
	// Categories
	if (!::IsANSIConvertible(Category))
		return FALSE;
	for (i = 0 ; i < SupplementalCategories.GetSize() ; i++)
	{
		if (!::IsANSIConvertible(SupplementalCategories[i]))
			return FALSE;
	}
	if (!::IsANSIConvertible(Urgency))
		return FALSE;

	// Keywords
	for (i = 0 ; i < Keywords.GetSize() ; i++)
	{
		if (!::IsANSIConvertible(Keywords[i]))
			return FALSE;
	}
	
	// Copyright
	if (!::IsANSIConvertible(CopyrightNotice))
		return FALSE;

	return TRUE;
}

BOOL CMetadata::IPTCINFO::IsEmpty()
{	
	// Encoding
	if (Encoding != IPTC_ENCODING_UNSPECIFIED)
		return FALSE;

	// Version
	if (Version != IPTC_VERSION)
		return FALSE;

	// Credits
	if (Byline != _T(""))
		return FALSE;
	if (BylineTitle != _T(""))
		return FALSE;
	if (Credits != _T(""))
		return FALSE;
	if (Source != _T(""))
		return FALSE;
	
	// Caption
	if (CaptionWriter != _T(""))
		return FALSE;
	if (Caption != _T(""))
		return FALSE;
	if (Headline != _T(""))
		return FALSE;
	if (SpecialInstructions != _T(""))
		return FALSE;
	
	// Origin
	if (ObjectName != _T(""))
		return FALSE;
	if (DateCreated != _T(""))
		return FALSE;
	if (City != _T(""))
		return FALSE;
	if (ProvinceState != _T(""))
		return FALSE;
	if (Country != _T(""))
		return FALSE;
	if (OriginalTransmissionReference != _T(""))
		return FALSE;
	
	// Categories
	if (Category != _T(""))
		return FALSE;
	if (SupplementalCategories.GetSize() > 0)
		return FALSE;
	if (Urgency != _T(""))
		return FALSE;

	// Keywords
	if (Keywords.GetSize() > 0)
		return FALSE;
	
	// Copyright
	if (CopyrightNotice != _T(""))
		return FALSE;

	return TRUE;
}

void CMetadata::IPTCINFO::Clear()
{	
	// Encoding
	Encoding = IPTC_ENCODING_UNSPECIFIED;

	// Default to Version IPTC_VERSION
	Version = IPTC_VERSION;

	// Credits
	Byline = _T("");	
	BylineTitle = _T("");
	Credits = _T("");
	Source = _T("");
	
	// Caption
	CaptionWriter = _T("");
	Caption = _T("");
	Headline = _T("");
	SpecialInstructions = _T("");
	
	// Origin
	ObjectName = _T("");
	DateCreated = _T("");
	City = _T("");
	ProvinceState = _T("");
	Country = _T("");
	OriginalTransmissionReference = _T("");
	
	// Categories
	Category = _T("");
	SupplementalCategories.RemoveAll();
	Urgency = _T("");

	// Keywords
	Keywords.RemoveAll();
	
	// Copyright
	CopyrightNotice = _T("");
}

void CMetadata::IPTCINFO::MakeCRLF()
{	
	int i;

	// Credits
	::MakeLineBreakCRLF(Byline);	
	::MakeLineBreakCRLF(BylineTitle);
	::MakeLineBreakCRLF(Credits);
	::MakeLineBreakCRLF(Source);
	
	// Caption
	::MakeLineBreakCRLF(CaptionWriter);
	::MakeLineBreakCRLF(Caption);
	::MakeLineBreakCRLF(Headline);
	::MakeLineBreakCRLF(SpecialInstructions);
	
	// Origin
	::MakeLineBreakCRLF(ObjectName);
	::MakeLineBreakCRLF(DateCreated);
	::MakeLineBreakCRLF(City);
	::MakeLineBreakCRLF(ProvinceState);
	::MakeLineBreakCRLF(Country);
	::MakeLineBreakCRLF(OriginalTransmissionReference);
	
	// Categories
	::MakeLineBreakCRLF(Category);
	for (i = 0 ; i < SupplementalCategories.GetSize() ; i++)
		::MakeLineBreakCRLF(SupplementalCategories[i]);
	::MakeLineBreakCRLF(Urgency);

	// Keywords
	for (i = 0 ; i < Keywords.GetSize() ; i++)
		::MakeLineBreakCRLF(Keywords[i]);
	
	// Copyright
	::MakeLineBreakCRLF(CopyrightNotice);
}

void CMetadata::XMPINFO::CopyVars(const CMetadata::XMPINFO& xmpinfo)
{
	int i;

	// Copyright
	CopyrightUrl = xmpinfo.CopyrightUrl;
	CopyrightMarked = xmpinfo.CopyrightMarked;

	// CreatorContactInfo (IPTC4XMP Core)
	CiAdrExtadr = xmpinfo.CiAdrExtadr;
	CiAdrCity = xmpinfo.CiAdrCity;
	CiAdrRegion = xmpinfo.CiAdrRegion;
	CiAdrPcode = xmpinfo.CiAdrPcode;
	CiAdrCtry = xmpinfo.CiAdrCtry;
	CiTelWork = xmpinfo.CiTelWork;
	CiEmailWork = xmpinfo.CiEmailWork;
	CiUrlWork = xmpinfo.CiUrlWork;

	// Content (IPTC4XMP Core)
	for (i = 0 ; i < xmpinfo.SubjectCode.GetSize() ; i++)
		SubjectCode.Add(xmpinfo.SubjectCode[i]);

	// Image
	IntellectualGenre = xmpinfo.IntellectualGenre;
	for (i = 0 ; i < xmpinfo.Scene.GetSize() ; i++)
		Scene.Add(xmpinfo.Scene[i]);
	Location = xmpinfo.Location;
	CountryCode = xmpinfo.CountryCode;

	// Status
	UsageTerms = xmpinfo.UsageTerms;
}

BOOL CMetadata::XMPINFO::IsEqual(const CMetadata::XMPINFO& xmpinfo)
{
	// Copyright
	if (CopyrightUrl != xmpinfo.CopyrightUrl)
		return FALSE;
	if (CopyrightMarked != xmpinfo.CopyrightMarked)
		return FALSE;

	// CreatorContactInfo (IPTC4XMP Core)
	if (CiAdrExtadr != xmpinfo.CiAdrExtadr)
		return FALSE;
	if (CiAdrCity != xmpinfo.CiAdrCity)
		return FALSE;
	if (CiAdrRegion != xmpinfo.CiAdrRegion)
		return FALSE;
	if (CiAdrPcode != xmpinfo.CiAdrPcode)
		return FALSE;
	if (CiAdrCtry != xmpinfo.CiAdrCtry)
		return FALSE;
	if (CiTelWork != xmpinfo.CiTelWork)
		return FALSE;
	if (CiEmailWork != xmpinfo.CiEmailWork)
		return FALSE;
	if (CiUrlWork != xmpinfo.CiUrlWork)
		return FALSE;

	// Content (IPTC4XMP Core)
	if (!IsArrayEqual(SubjectCode, xmpinfo.SubjectCode))
		return FALSE;

	// Image
	if (IntellectualGenre != xmpinfo.IntellectualGenre)
		return FALSE;
	if (!IsArrayEqual(Scene, xmpinfo.Scene))
		return FALSE;
	if (Location != xmpinfo.Location)
		return FALSE;
	if (CountryCode != xmpinfo.CountryCode)
		return FALSE;

	// Status
	if (UsageTerms != xmpinfo.UsageTerms)
		return FALSE;

	return TRUE;
}

BOOL CMetadata::XMPINFO::IsEmpty()
{
	// Copyright
	if (CopyrightUrl != _T(""))
		return FALSE;
	if (CopyrightMarked != _T(""))
		return FALSE;

	// CreatorContactInfo (IPTC4XMP Core)
	if (CiAdrExtadr != _T(""))
		return FALSE;
	if (CiAdrCity != _T(""))
		return FALSE;
	if (CiAdrRegion != _T(""))
		return FALSE;
	if (CiAdrPcode != _T(""))
		return FALSE;
	if (CiAdrCtry != _T(""))
		return FALSE;
	if (CiTelWork != _T(""))
		return FALSE;
	if (CiEmailWork != _T(""))
		return FALSE;
	if (CiUrlWork != _T(""))
		return FALSE;

	// Content (IPTC4XMP Core)
	if (SubjectCode.GetSize() > 0)
		return FALSE;

	// Image
	if (IntellectualGenre != _T(""))
		return FALSE;
	if (Scene.GetSize() > 0)
		return FALSE;
	if (Location != _T(""))
		return FALSE;
	if (CountryCode != _T(""))
		return FALSE;

	// Status
	if (UsageTerms != _T(""))
		return FALSE;

	return TRUE;
}

void CMetadata::XMPINFO::Clear()
{	
	// Copyright
	CopyrightUrl = _T("");
	CopyrightMarked = _T("");

	// CreatorContactInfo (IPTC4XMP Core)
	CiAdrExtadr = _T("");
	CiAdrCity = _T("");
	CiAdrRegion = _T("");
	CiAdrPcode = _T("");
	CiAdrCtry = _T("");
	CiTelWork = _T("");
	CiEmailWork = _T("");
	CiUrlWork = _T("");

	// Content (IPTC4XMP Core)
	SubjectCode.RemoveAll();

	// Image
	IntellectualGenre = _T("");
	Scene.RemoveAll();
	Location = _T("");
	CountryCode = _T("");

	// Status
	UsageTerms = _T("");
}

void CMetadata::XMPINFO::MakeCRLF()
{	
	int i;

	// Copyright
	::MakeLineBreakCRLF(CopyrightUrl);
	::MakeLineBreakCRLF(CopyrightMarked);

	// CreatorContactInfo (IPTC4XMP Core)
	::MakeLineBreakCRLF(CiAdrExtadr);
	::MakeLineBreakCRLF(CiAdrCity);
	::MakeLineBreakCRLF(CiAdrRegion);
	::MakeLineBreakCRLF(CiAdrPcode);
	::MakeLineBreakCRLF(CiAdrCtry);
	::MakeLineBreakCRLF(CiTelWork);
	::MakeLineBreakCRLF(CiEmailWork);
	::MakeLineBreakCRLF(CiUrlWork);

	// Content (IPTC4XMP Core)
	for (i = 0 ; i < SubjectCode.GetSize() ; i++)
		::MakeLineBreakCRLF(SubjectCode[i]);

	// Image
	::MakeLineBreakCRLF(IntellectualGenre);
	for (i = 0 ; i < Scene.GetSize() ; i++)
		::MakeLineBreakCRLF(Scene[i]);
	::MakeLineBreakCRLF(Location);
	::MakeLineBreakCRLF(CountryCode);

	// Status
	::MakeLineBreakCRLF(UsageTerms);
}