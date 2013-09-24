#if !defined(AFX_INIFILE_H__8FD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_)
#define AFX_INIFILE_H__8FD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_

#pragma once

UINT GetProfileIniInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault, LPCTSTR lpszProfileName);
BOOL WriteProfileIniInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue, LPCTSTR lpszProfileName);
CString GetProfileIniString(LPCTSTR lpszSection, LPCTSTR lpszEntry,
			LPCTSTR lpszDefault, LPCTSTR lpszProfileName);
BOOL WriteProfileIniString(LPCTSTR lpszSection, LPCTSTR lpszEntry,
			LPCTSTR lpszValue, LPCTSTR lpszProfileName);
BOOL GetProfileIniBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry,
			LPBYTE* ppData, UINT* pBytes, LPCTSTR lpszProfileName);
BOOL WriteProfileIniBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry,
			LPBYTE pData, UINT nBytes, LPCTSTR lpszProfileName);
CString GetSecureProfileIniString(LPCTSTR lpszSection, LPCTSTR lpszEntry,
			LPCTSTR lpszDefault, LPCTSTR lpszProfileName);
BOOL WriteSecureProfileIniString(LPCTSTR lpszSection, LPCTSTR lpszEntry,
			LPCTSTR lpszValue, LPCTSTR lpszProfileName);

#endif // !defined(AFX_INIFILE_H__8FD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_)