#pragma once

#ifndef __CMD5_H__
#define __CMD5_H__

#ifndef __WINCRYPT_H__
#pragma message("To avoid this message, put wincrypt.h in your pre compiled header (usually stdafx.h)")
#include <wincrypt.h>
#endif

class CMD5Hash
{
public:
	CMD5Hash()
	{
		memset(m_byHash, 0, sizeof(m_byHash));
	}

	operator BYTE*() 
	{ 
		return m_byHash; 
	};

	CString Format(BOOL bUppercase)
	{
		CString sRet;

		LPTSTR pString = sRet.GetBuffer(33);
		DWORD i;
		for (i = 0 ; i < 16 ; i++)
		{
			int nChar = (m_byHash[i] & 0xF0) >> 4;
			if (nChar <= 9)
				pString[i*2] = static_cast<TCHAR>(nChar + _T('0'));
			else
				pString[i*2] = static_cast<TCHAR>(nChar - 10 + (bUppercase ? _T('A') : _T('a')));

			nChar = m_byHash[i] & 0x0F;
			if (nChar <= 9)
				pString[i*2 + 1] = static_cast<TCHAR>(nChar + _T('0'));
			else
				pString[i*2 + 1] = static_cast<TCHAR>(nChar - 10 + (bUppercase ? _T('A') : _T('a')));
		}
		pString[i*2] = _T('\0');
		sRet.ReleaseBuffer();

		return sRet;
	}

	BYTE m_byHash[16];
};

class CMD5
{
public:
	CMD5()
	{
		m_hProv = NULL;
		::CryptAcquireContext(&m_hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT);
	}
	
	virtual ~CMD5()
	{
		if (m_hProv)
			::CryptReleaseContext(m_hProv, 0);
	}

	BOOL Hash(const BYTE* pbyData, DWORD dwDataSize, CMD5Hash& hash)
	{
		// Create the hash object
		HCRYPTHASH hHash = NULL;
		if (!::CryptCreateHash(m_hProv, CALG_MD5, 0, 0, &hHash)) 
			return FALSE;
		
		// Hash the data
		BOOL bSuccess = ::CryptHashData(hHash, pbyData, dwDataSize, 0);
		if (bSuccess)
		{
			DWORD dwHashSize = sizeof(hash);
			bSuccess = ::CryptGetHashParam(hHash, HP_HASHVAL, hash, &dwHashSize, 0);
		}
		
		// Free
		if (hHash)
			::CryptDestroyHash(hHash);

		return bSuccess;
	}

	BOOL Random(CMD5Hash& hash)
	{
		return ::CryptGenRandom(m_hProv, sizeof(hash), hash);
	}

protected:
	HCRYPTPROV m_hProv;
};

#endif //__CMD5_H__
