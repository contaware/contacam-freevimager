#include "stdafx.h"
#include "IniFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifndef _countof
#define _countof(array) (sizeof(array)/sizeof(array[0]))
#endif

UINT GetProfileIniInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault, LPCTSTR lpszProfileName)
{
	ASSERT(lpszSection != NULL);
	ASSERT(lpszEntry != NULL);
	ASSERT(lpszProfileName != NULL);
	return GetPrivateProfileInt(lpszSection, lpszEntry, nDefault, lpszProfileName);
}

// Attention: GetPrivateProfileString strips quotes!
CString GetProfileIniString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault, LPCTSTR lpszProfileName)
{
	ASSERT(lpszSection != NULL);
	ASSERT(lpszEntry != NULL);
	ASSERT(lpszProfileName != NULL);

	if (lpszDefault == NULL)
		lpszDefault = _T("");    // don't pass in NULL
	TCHAR szT[4096];
	DWORD dw = GetPrivateProfileString(lpszSection, lpszEntry,
		lpszDefault, szT, _countof(szT), lpszProfileName);
	ASSERT(dw < 4095);
	return szT;
}

BOOL GetProfileIniBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry, BYTE** ppData, UINT* pBytes, LPCTSTR lpszProfileName)
{
	ASSERT(lpszSection != NULL);
	ASSERT(lpszEntry != NULL);
	ASSERT(ppData != NULL);
	ASSERT(pBytes != NULL);
	*ppData = NULL;
	*pBytes = 0;
	ASSERT(lpszProfileName != NULL);

	CString str = GetProfileIniString(lpszSection, lpszEntry, NULL, lpszProfileName);
	if (str.IsEmpty())
		return FALSE;
	ASSERT(str.GetLength()%2 == 0);
	int nLen = str.GetLength();
	*pBytes = nLen/2;
	*ppData = new BYTE[*pBytes];
	for (int i=0;i<nLen;i+=2)
	{
		(*ppData)[i/2] = (BYTE)
			(((str[i+1] - 'A') << 4) + (str[i] - 'A'));
	}
	return TRUE;
}

typedef BOOL (WINAPI * FPCRYPTUNPROTECTDATA)(DATA_BLOB*, LPWSTR*, DATA_BLOB*, PVOID*, CRYPTPROTECT_PROMPTSTRUCT*, DWORD, DATA_BLOB*);
CString GetSecureProfileIniString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault, LPCTSTR lpszProfileName)
{
	HINSTANCE h = LoadLibrary(_T("crypt32.dll"));
	if (!h)
		return GetProfileIniString(lpszSection, lpszEntry, lpszDefault, lpszProfileName);
	FPCRYPTUNPROTECTDATA fpCryptUnprotectData = (FPCRYPTUNPROTECTDATA)GetProcAddress(h, "CryptUnprotectData");
	if (fpCryptUnprotectData && g_bWin2000OrHigher) // System version check necessary because win98 is returning a function pointer which does nothing!
	{
		DATA_BLOB blobIn, blobOut, blobEntropy;
		blobIn.cbData = 0;
		blobIn.pbData = NULL;
		blobOut.cbData = 0;
		blobOut.pbData = NULL;
		BYTE Entropy[] = {
			0x6B, 0x31, 0x20, 0x85, 0x08, 0x79, 0xA3, 0x1B, 0x53, 0xAB, 0x3D, 0x08, 0x67, 0xFD, 0x55, 0x66, 
			0x26, 0x7B, 0x46, 0x28, 0x91, 0xBB, 0x11, 0x8D, 0x8E, 0xB0, 0x2C, 0x99, 0x1E, 0x5B, 0x4A, 0x68};
		blobEntropy.pbData = Entropy;
		blobEntropy.cbData = sizeof(Entropy);
		LPWSTR pDescrOut = (LPWSTR)0xbaadf00d ; // Not NULL!

		GetProfileIniBinary(lpszSection, lpszEntry, &blobIn.pbData, (UINT*)&blobIn.cbData, lpszProfileName);
		if (blobIn.pbData && (blobIn.cbData > 0))
		{
			if (fpCryptUnprotectData(	&blobIn,
										&pDescrOut,
										&blobEntropy,
										NULL,
										NULL,
										0,
										&blobOut))
			{
				CString s;
				CString sType(pDescrOut);
				if (sType == L"UNICODE")
					s = CString((LPCWSTR)blobOut.pbData);
				else if (sType == L"ASCII")
					s = CString((LPCSTR)blobOut.pbData);
				delete [] blobIn.pbData;
				LocalFree(pDescrOut);
				LocalFree(blobOut.pbData);
				FreeLibrary(h);
				return s;
			}
			else
			{
				delete [] blobIn.pbData;
				LocalFree(pDescrOut);
				LocalFree(blobOut.pbData);
			}
		}
		else
		{
			if (blobIn.pbData)
				delete [] blobIn.pbData;
		}
		FreeLibrary(h);
		return _T("");
	}
	else
	{
		FreeLibrary(h);
		return GetProfileIniString(lpszSection, lpszEntry, lpszDefault, lpszProfileName);
	}
}

BOOL WriteProfileIniInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue, LPCTSTR lpszProfileName)
{
	ASSERT(lpszSection != NULL);
	ASSERT(lpszEntry != NULL);
	ASSERT(lpszProfileName != NULL);

	TCHAR szT[16];
	_stprintf(szT, _T("%d"), nValue);
	return WritePrivateProfileString(lpszSection, lpszEntry, szT, lpszProfileName);
}

BOOL WriteProfileIniString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue, LPCTSTR lpszProfileName)
{
	ASSERT(lpszSection != NULL);
	ASSERT(lpszProfileName != NULL);
	ASSERT(lstrlen(lpszProfileName) < 4095); // can't read in bigger
	return WritePrivateProfileString(lpszSection, lpszEntry, lpszValue, lpszProfileName);
}

BOOL WriteProfileIniBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPBYTE pData, UINT nBytes, LPCTSTR lpszProfileName)
{
	ASSERT(lpszSection != NULL);

	// convert to string and write out
	LPTSTR lpsz = new TCHAR[nBytes*2+1];
	UINT i;
	for (i = 0; i < nBytes; i++)
	{
		lpsz[i*2] = (TCHAR)((pData[i] & 0x0F) + 'A'); //low nibble
		lpsz[i*2+1] = (TCHAR)(((pData[i] >> 4) & 0x0F) + 'A'); //high nibble
	}
	lpsz[i*2] = 0;

	ASSERT(lpszProfileName != NULL);

	BOOL bResult = WriteProfileIniString(lpszSection, lpszEntry, lpsz, lpszProfileName);
	delete[] lpsz;
	return bResult;
}

typedef BOOL (WINAPI * FPCRYPTPROTECTDATA)(DATA_BLOB*, LPCWSTR, DATA_BLOB*, PVOID, CRYPTPROTECT_PROMPTSTRUCT*, DWORD, DATA_BLOB*);
BOOL WriteSecureProfileIniString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue, LPCTSTR lpszProfileName)
{
	HINSTANCE h = LoadLibrary(_T("crypt32.dll"));
	if (!h)
		return WriteProfileIniString(lpszSection, lpszEntry, lpszValue, lpszProfileName);
	FPCRYPTPROTECTDATA fpCryptProtectData = (FPCRYPTPROTECTDATA)GetProcAddress(h, "CryptProtectData");
	if (fpCryptProtectData && g_bWin2000OrHigher) // System version check necessary because win98 is returning a function pointer which does nothing!
	{
		DATA_BLOB blobIn, blobOut, blobEntropy;
		blobIn.pbData = (BYTE*)lpszValue;
		blobIn.cbData = sizeof(TCHAR) * (_tcslen(lpszValue) + 1);
		blobOut.cbData = 0;
		blobOut.pbData = NULL;
		BYTE Entropy[] = {
			0x6B, 0x31, 0x20, 0x85, 0x08, 0x79, 0xA3, 0x1B, 0x53, 0xAB, 0x3D, 0x08, 0x67, 0xFD, 0x55, 0x66, 
			0x26, 0x7B, 0x46, 0x28, 0x91, 0xBB, 0x11, 0x8D, 0x8E, 0xB0, 0x2C, 0x99, 0x1E, 0x5B, 0x4A, 0x68};
		blobEntropy.pbData = Entropy;
		blobEntropy.cbData = sizeof(Entropy);

		if (fpCryptProtectData(	&blobIn,
#ifdef _UNICODE
								L"UNICODE",	// Windows 2000:  This parameter is required and cannot be set to NULL
#else
								L"ASCII",	// Windows 2000:  This parameter is required and cannot be set to NULL
#endif
								&blobEntropy,
								NULL,
								NULL,
								0,
								&blobOut))
		{
			BOOL res = WriteProfileIniBinary(lpszSection, lpszEntry, (LPBYTE)blobOut.pbData, (UINT)blobOut.cbData, lpszProfileName);
			LocalFree(blobOut.pbData);
			FreeLibrary(h);
			return res;
		}
		else
		{
			LocalFree(blobOut.pbData);
			FreeLibrary(h);
			return FALSE;
		}
	}
	else
	{
		FreeLibrary(h);
		return WriteProfileIniString(lpszSection, lpszEntry, lpszValue, lpszProfileName);
	}
}
