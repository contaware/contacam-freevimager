// ZipException.cpp: implementation of the CZipException class.
//
////////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2000 Tadeusz Dracz.
//  For conditions of distribution and use, see copyright notice in ZipArchive.h
////////////////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include "ZipException.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC( CZipException, CException)

CZipException::CZipException(int iCause, LPCTSTR lpszZipName):CException(TRUE)
{
	m_iZipCause = iCause;

	if (lpszZipName)
		m_sFileName = lpszZipName;	
}

CZipException::~CZipException()
{

}

void AfxThrowZipException(int iZipError, LPCTSTR lpszZipName)
{
	throw new CZipException(CZipException::ZipErrToCause(iZipError), lpszZipName);
}

int CZipException::ZipErrToCause(int iZipError)
{
	switch (iZipError)
	{
	case 2://Z_NEED_DICT:
		return CZipException::needDict;
	case 1://Z_STREAM_END:
		return CZipException::streamEnd;
	case -1://Z_ERRNO:
		return CZipException::errNo;
	case -2://Z_STREAM_ERROR:
		return CZipException::streamError;
	case -3://Z_DATA_ERROR:
		return CZipException::dataError;
	case -4://Z_MEM_ERROR:
		return CZipException::memError;
	case -5://Z_BUF_ERROR:
		return CZipException::bufError;
	case -6://Z_VERSION_ERROR:
		return CZipException::versionError;
	case ZIP_BADZIPFILE:
		return CZipException::badZipFile;
	case ZIP_BADCRC:
		return CZipException::badCrc;
	case ZIP_ABORTED:
		return CZipException::aborted;
	case ZIP_NOCALLBACK:
		return CZipException::noCallback;
	case ZIP_NONREMOVABLE:
		return CZipException::nonRemovable;
	case ZIP_TOOMANYVOLUMES:
		return CZipException::tooManyVolumes;
	case ZIP_TOOLONGFILENAME:
		return CZipException::tooLongFileName;

	default:
		return CZipException::generic;
	}
}

void CZipException::ReportZipError()
{
	::AfxMessageBox(GetZipErrorMessage(), MB_ICONSTOP);
}

CString CZipException::GetZipErrorMessage()
{
	CString sErrorMsg;

	switch (m_iZipCause)
	{
		case noError			:	sErrorMsg = _T("No Error");
									break;
		case generic			:	sErrorMsg = _T("Unknown Error");
									break;
		case streamEnd			:	sErrorMsg = _T("Zlib Library Error: Z_STREAM_END");
									break;
		case needDict			:	sErrorMsg = _T("Zlib Library Error: Z_NEED_DICT");
									break;
		case errNo				:	sErrorMsg = _T("Zlib Library Error: Z_ERRNO");
									break;
		case streamError		:	sErrorMsg = _T("Zlib Library Error: Z_STREAM_ERROR");
									break;
		case dataError			:	sErrorMsg = _T("Zlib Library Error: Z_DATA_ERROR");
									break;
		case memError			:	sErrorMsg = _T("Zlib Library Error: Z_MEM_ERROR");
									break;
		case bufError			:	sErrorMsg = _T("Zlib Library Error: Z_BUF_ERROR");
									break;
		case versionError		:	sErrorMsg = _T("Zlib Library Error: Z_VERSION_ERROR");
									break;
		case badZipFile			:	sErrorMsg = _T("Damaged or unsupported zip file");
									break;
		case badCrc				:	sErrorMsg = _T("Crc Mismatched");
									break;
		case noCallback			:	sErrorMsg = _T("No callback function set");
									break;
		case aborted			:	sErrorMsg = _T("Disk change callback function returned false");
									break;
		case nonRemovable		:	sErrorMsg = _T("The disk selected for pkSpan archive is non removable");
									break;
		case tooManyVolumes		:	sErrorMsg = _T("Limit of the maximum volumes reached (999)");
									break;
		case tooLongFileName	:	sErrorMsg = _T("The file name of the file added to the archive is too long");
									break;
		default					:	sErrorMsg = _T("Unknown Error");
									break;
	}

	return sErrorMsg;
}

