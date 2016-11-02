#pragma once

#ifndef __CSHA256_H__
#define __CSHA256_H__

#ifndef __WINCRYPT_H__
#pragma message("To avoid this message, put wincrypt.h in your pre compiled header (usually stdafx.h)")
#include <wincrypt.h>
#endif

// SHA256 support in:
// - Windows XP SP3
// - Windows Server 2003 Service Pack 2 + KB938397 installed
#ifndef CALG_SHA_256
#define ALG_SID_SHA_256         12
#define CALG_SHA_256            (ALG_CLASS_HASH | ALG_TYPE_ANY | ALG_SID_SHA_256)
#endif

class CSHA256Hash
{
public:
	CSHA256Hash()
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

		LPTSTR pString = sRet.GetBuffer(65);
		DWORD i;
		for (i = 0 ; i < 32 ; i++)
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

	BYTE m_byHash[32];
};

class CSHA256
{
public:
	CSHA256()
	{
		m_hProv = NULL;
		::CryptAcquireContext(&m_hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT | CRYPT_SILENT);
	}
	
	virtual ~CSHA256()
	{
		if (m_hProv)
			::CryptReleaseContext(m_hProv, 0);
	}

	BOOL Hash(const BYTE* pbyData, DWORD dwDataSize, CSHA256Hash& hash)
	{
		// Create the hash object
		HCRYPTHASH hHash = NULL;
		if (!::CryptCreateHash(m_hProv, CALG_SHA_256, 0, 0, &hHash)) 
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

	BOOL Random(CSHA256Hash& hash)
	{
		return ::CryptGenRandom(m_hProv, sizeof(hash), hash);
	}

protected:
	HCRYPTPROV m_hProv;
};

#endif //__CSHA256_H__
