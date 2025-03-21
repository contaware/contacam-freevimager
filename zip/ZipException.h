// ZipException.h: interface for the CZipException class.
//
////////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2000 Tadeusz Dracz.
//  For conditions of distribution and use, see copyright notice in ZipArchive.h
////////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_ZIPEXCEPTION_H__E3546921_D728_11D3_B7C7_E77339672847__INCLUDED_)
#define AFX_ZIPEXCEPTION_H__E3546921_D728_11D3_B7C7_E77339672847__INCLUDED_

#pragma once

#include <afxwin.h>         // MFC core and standard components

typedef unsigned short uShort;
// errors number definitions
#define ZIP_BADZIPFILE		(-101)
#define ZIP_BADCRC			(-102)
#define ZIP_NOCALLBACK		(-103)
#define ZIP_ABORTED			(-104)
#define ZIP_NONREMOVABLE	(-105)
#define ZIP_TOOMANYVOLUMES	(-106)
#define ZIP_TOOLONGFILENAME (-107)

class CZipException : public CException  
{
public:
	DECLARE_DYNAMIC(CZipException)
	// convert zlib library and internal error code to a ZipException code
	static int ZipErrToCause(int iZipError);
	// name of the zip file where the error occured
	CString m_sFileName;

	enum
	{
		noError,		// no error
		generic,		// unknown error
		streamEnd,			// zlib library errors
		needDict,			//
		errNo,				//
		streamError,		//
		dataError,			//
		memError,			//	
		bufError,			//
		versionError,		// 
		badZipFile,		// damaged or not a zip file
		badCrc,			// crc mismatched
		noCallback,		// no callback function set
		aborted,		// disk change callback function returned false
		nonRemovable,	// the disk selected for pkSpan archive is non removable
		tooManyVolumes,	// limit of the maximum volumes reached (999)
		tooLongFileName	// the file name of the file added to the archive is too long
	};
	// cause - takes one of the codes above
	int m_iZipCause;
	CZipException(int iCause = generic, LPCTSTR lpszZipName = NULL);
	virtual ~CZipException();
	CString GetZipErrorMessage();
	void ReportZipError();
};

// throw zip exception
// Parameters:
//		iZipError	- zlib or internal error code
//		lpszZipName - name of the file where the error occured
void AfxThrowZipException(int iZipError = 1000, LPCTSTR lpszZipName = NULL);

#endif // !defined(AFX_ZIPEXCEPTION_H__E3546921_D728_11D3_B7C7_E77339672847__INCLUDED_)
