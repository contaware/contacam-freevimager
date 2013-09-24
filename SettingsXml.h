#if !defined(AFX_SETTINGSXML_H__786258A5_8360_4AE4_BDAF_2A52F8E1B877__INCLUDED_)
#define AFX_SETTINGSXML_H__786258A5_8360_4AE4_BDAF_2A52F8E1B877__INCLUDED_

#pragma once

#include "XMLite.h"

#define XML_INIT_STRING		_T("<?xml version=\'1.0\' encoding=\'UTF-8\' standalone=\'yes\'?>\r\n")

class CSettingsXml
{
public:
	CSettingsXml() {PARSEINFO pi;
					pi.trim_value = true;	// trim value
					m_Xml.Close();
					m_Xml.Load(XML_INIT_STRING, &pi);};
	virtual ~CSettingsXml() {;};

	BOOL LoadSettings(const CString& sFileName);
	BOOL StoreSettings(CString sFileName = CString(_T("")));	

	LPXNode GetSection(LPXNode p, const CString& sName);

	CString GetString(LPXNode p, const CString& sName, const CString& sDefault);
	int GetInt(LPXNode p, const CString& sName, int nDefault);
	BOOL WriteString(LPXNode p, const CString& sName, const CString& sValue);
	BOOL WriteInt(LPXNode p, const CString& sName, int nValue);

private:
	CString FromUTF8(const unsigned char* pUtf8, int nUtf8Len);
	int ToUTF8(const CString& s, LPBYTE* ppUtf8);
	CString m_sFileName;
	XDoc m_Xml;
};

#endif // !defined(AFX_SETTINGSXML_H__786258A5_8360_4AE4_BDAF_2A52F8E1B877__INCLUDED_)