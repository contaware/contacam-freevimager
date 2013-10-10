#include "stdafx.h"
#include "TraceLogFile.h"
#include "SettingsXml.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CString CSettingsXml::GetString(LPXNode p, const CString& sName, const CString& sDefault)
{
	// Top Node
	if (p == NULL)
		p = &m_Xml;

	if (p)
	{
		LPXNode pNode = p->Find(sName, TRUE);
		if (pNode)
			return pNode->value;
	}
	
	return sDefault;
}

int CSettingsXml::GetInt(LPXNode p, const CString& sName, int nDefault)
{
	// Top Node
	if (p == NULL)
		p = &m_Xml;

	if (p)
	{
		LPXNode pNode = p->Find(sName, TRUE);
		if (pNode)
		{
			if (pNode->value != _T(""))
				return _ttoi(pNode->value);
		}
	}
	
	return nDefault;
}

BOOL CSettingsXml::LoadSettings(const CString& sFileName)
{
	LPBYTE pData = NULL;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD dwSizeFile;
	DWORD dwNumberOfBytesRead = 0;
	CString sXml;
	PARSEINFO pi;
	pi.trim_value = true;	// trim value
	m_sFileName = sFileName;

	// Note:
	// pi.escape_value = _T('\0') is giving an error
	// when a dir path ends with a _T('\\')
	// -> Keep escape_value = _T('\\')
	// -> Make Escapes in sXml

	// Open File, if not existing init m_Xml
	hFile = ::CreateFile(sFileName,
						GENERIC_READ,
						FILE_SHARE_READ,
						NULL, OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL,
						NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		// Set Default
		m_Xml.Close();
		m_Xml.Load(XML_INIT_STRING, &pi);
		return TRUE;
	}

	// Get File Size
	dwSizeFile = GetFileSize(hFile, NULL);
	if (dwSizeFile == 0 || dwSizeFile == 0xFFFFFFFF)
		goto error;

	// Allocate Memory
	pData = new BYTE[dwSizeFile];
	if (!pData)
		goto error;

	// Go read the file
	ReadFile(hFile, pData, dwSizeFile, &dwNumberOfBytesRead, NULL);
	if (dwNumberOfBytesRead != dwSizeFile)
		goto error;

	// Convert from UTF8
	sXml = ::FromUTF8(	(const unsigned char*)pData,
						(int)dwSizeFile);
	if (sXml == _T(""))
		goto error;

	// Make Escapes
	sXml.Replace(_T("\\"), _T("\\\\"));

	// Parse
	m_Xml.Close();
	if (m_Xml.Load(sXml, &pi))
	{
		// Free
		::CloseHandle(hFile);
		if (pData)
			delete [] pData;

		return TRUE;
	}

error:
	if (hFile != INVALID_HANDLE_VALUE)
		::CloseHandle(hFile);
	if (pData)
		delete [] pData;
	return FALSE;
}

LPXNode CSettingsXml::GetSection(LPXNode p, const CString& sName)
{
	// Top Node
	if (p == NULL)
		p = &m_Xml;
	return p->Find(sName);
}

BOOL CSettingsXml::WriteString(LPXNode p, const CString& sName, const CString& sValue)
{
	CString s;

	// Top Node
	if (p == NULL)
		p = &m_Xml;

	if (p)
	{
		LPXNode pNode = p->Find(sName, TRUE);
		if (pNode)
		{
			pNode->value = sValue;
			return TRUE;
		}
		else
		{
			pNode = new XNode;
			if (pNode)
			{
				s.Format(_T("<%s>%s</%s>"), sName, sValue, sName);
				pNode->Load(s);
				p->AppendChild(pNode);
				return TRUE;
			}
		}
	}

	return FALSE;
}

BOOL CSettingsXml::WriteInt(LPXNode p, const CString& sName, int nValue)
{
	CString s;

	// Top Node
	if (p == NULL)
		p = &m_Xml;

	if (p)
	{
		LPXNode pNode = p->Find(sName, TRUE);
		if (pNode)
		{
			s.Format(_T("%d"), nValue);
			pNode->value = s;
			return TRUE;
		}
		else
		{
			pNode = new XNode;
			if (pNode)
			{
				s.Format(_T("<%s>%d</%s>"), sName, nValue, sName);
				pNode->Load(s);
				p->AppendChild(pNode);
				return TRUE;
			}
		}
	}

	return FALSE;
}

BOOL CSettingsXml::StoreSettings(CString sFileName/*=CString(_T(""))*/)
{
	if (sFileName == _T(""))
		sFileName = m_sFileName;

	if (sFileName == _T(""))
		return FALSE;

	LPBYTE pData = NULL;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD dwNumberOfBytesWritten;
	int nSize;
	DISP_OPT opt;
	PARSEINFO pi;
	pi.trim_value = true; // trim value
	CString sXml;

	// Get Xml
	opt.newline = true;
	_tcsncpy(opt.newline_type, _T("\r\n"), 3);
	opt.value_quotation_mark = _T('\'');
	sXml = m_Xml.GetXML(&opt);
	sXml.TrimLeft(_T("\r\n")); // Remove initial \r\n
	
	// Convert to UTF8
	nSize = ::ToUTF8(sXml, &pData);
	if (nSize > 0)
	{
		// Create File
		hFile = ::CreateFile(sFileName,
							GENERIC_WRITE,
							FILE_SHARE_READ,
							NULL, CREATE_ALWAYS,
							FILE_ATTRIBUTE_NORMAL,
							NULL);
		if (hFile == INVALID_HANDLE_VALUE)
			goto error;

		// Write to file
		WriteFile(hFile, pData, nSize, &dwNumberOfBytesWritten, NULL);
		if (dwNumberOfBytesWritten != nSize)
			goto error;

		// Free
		::CloseHandle(hFile);
		if (pData)
			delete [] pData;

		return TRUE;
	}

error:
	if (hFile != INVALID_HANDLE_VALUE)
		::CloseHandle(hFile);
	if (pData)
		delete [] pData;
	return FALSE;
}