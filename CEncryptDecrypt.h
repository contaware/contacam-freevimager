// ------------------------------------------------------------------------------------------------------------------------------------------------------
// -																																					-
// - File:			cencryptdecrypt.h.																													-
// -																																					-
// - Contents: 		Implementation of class CEncryptDecrypt and a few helper functions.																	-
// -																																					-
// - Purpose:  		TEA, XTEA and XXTEA crypto methods.																									-
// -																																					-
// - Remarks:    	No restrictions and free to use open source.																						-
// -																																					-
// - Originator: 	Michael Mogensen, MM-IT Consult 2003, michael-mogensen-danmark@hotmail.com.															-
// -																																					-
// - Compiler:		MS Visual C++ 9.0.																													-
// -																																					-
// - Period:		07.12.08 - 00.00.00.																											    -
// -																																					-
// - Version:		1.00. 07.12.08. Initial version.																									-
// -				1.10. 18.12.08. Replaced MFC Array<T, T> with SLT vector<> to loosen MFC dependence.												-
// -								Added new Encrypt(...)/Decrypt(...) methods working with char*.														-
// -				1.20. 12.08.16. Removed "strconv.h" inclusion (by Oliver Pfister)                                                                   -
// -                                Made TRACE("Unknown error in const const size_t GetSubString(...)") unicode (by Oliver Pfister)                     -
// -																																					-
// ------------------------------------------------------------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------------------------------------------------------------
// - Miscellaneous.																																		-
// ------------------------------------------------------------------------------------------------------------------------------------------------------
using namespace std;
#pragma once

// ------------------------------------------------------------------------------------------------------------------------------------------------------
// - Header(s).																																			-
// ------------------------------------------------------------------------------------------------------------------------------------------------------
#include <vector>

// ------------------------------------------------------------------------------------------------------------------------------------------------------
// - Miscellaneous.																																		-
// ------------------------------------------------------------------------------------------------------------------------------------------------------
inline const size_t GetSubString(const CString cstrSource, CString &cstrTarget, CONST UINT uPart2Lookup, const TCHAR cDelim)
// Get substring no. uPart2Lookup of cstrSource and place it in cstrTarget. Substrings are separated by cDelim. Return length of substring.
{
	TRY
	{
		if(cstrSource.IsEmpty())
			// No work to do.
			return 0;
		cstrTarget.Empty();
		int iLoc_Begin = -1, 
			iLoc_End = -1;
		if(uPart2Lookup == 0)
		{
			if(cstrSource[0] != cDelim)
			{
				iLoc_Begin = 0;
				iLoc_End = cstrSource.Find(cDelim, 1);
			}
		}
		else
		{
			for(UINT uPart = 0; uPart < uPart2Lookup; uPart++)
			{
				iLoc_Begin = cstrSource.Find(cDelim, iLoc_Begin + 1);
				iLoc_End = cstrSource.Find(cDelim, iLoc_Begin + 1);
				if(iLoc_Begin == -1)
					break;
			}
			if(iLoc_Begin != -1)
				iLoc_Begin++;
		}
		if(iLoc_Begin != -1)
		{
			if(iLoc_End == -1)
				iLoc_End = cstrSource.GetLength();
			cstrTarget = cstrSource.Mid(iLoc_Begin, iLoc_End - iLoc_Begin);
		}
		return cstrTarget.GetLength();
	}
	CATCH_ALL(e)
	{
		TRACE(_T("Unknown error in const const size_t GetSubString(...)"));
		THROW_LAST();
	}
	END_CATCH_ALL
}

inline const CString GetSubString(const CString cstrSource, CONST UINT uPart2Lookup, const TCHAR cDelim = TCHAR('|'))
// For docu. see above.
{
	CString cstrTarget(_T(""));
	::GetSubString(cstrSource, cstrTarget, uPart2Lookup, cDelim);
	return cstrTarget;
}

inline const int GetDigit(const int iDigitNo, const long lx)
// 10^n:                                  n = 3210                    3210
// Return digit in long value - like GetDigit(4617, 3) == 4, GetDigit(4617, 0) == 7.
{
	CString cstrlX(_T(""));
	cstrlX.Format(TEXT("%ld"), lx);
	int iDigit = ::_wtoi(CString(cstrlX[cstrlX.GetLength() - iDigitNo - 1]));
	return iDigit;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------
// - CEncryptDecrypt.																																	-
// ------------------------------------------------------------------------------------------------------------------------------------------------------
// 1 WORD are == 32 bit.
#define SIZE_OF_KEY 4 // Keysize are 128 bit (== 4 WORDS)
#define SIZE_OF_VEC_TEA 2 // Blocksize are 64 bit (== 2 WORDS).
#define SIZE_OF_VEC_XTEA 2 // Blocksize are 64 bit (== 2 WORDS).
#define MAGIC_KEY 2956

class CEncryptDecrypt
{
	public:
		// 0. Properties. (alphabetical).
		enum EEncryptDecryptStandard
		{
			eTEA, eXTEA, eXXTEA
		};
		// 1. Constructors. (alphabetical).
		inline CEncryptDecrypt(const enum EEncryptDecryptStandard iEncryptDecryptStandard = eTEA, CONST ULONG uKey = MAGIC_KEY) : 
			m_iEncryptDecryptStandard(iEncryptDecryptStandard), 
			m_ulDelta(0x9e3779b9)
			{
				ClearVec();
				m_uKey[0] = (ULONG)::GetDigit(3, uKey);
				m_uKey[1] = (ULONG)::GetDigit(2, uKey);
				m_uKey[2] = (ULONG)::GetDigit(1, uKey);
				m_uKey[3] = (ULONG)::GetDigit(0, uKey);
			}
		inline ~CEncryptDecrypt()
		{ ClearVec(); }
		// 3. Methods. (alphabetical).
		//
		// ************************************************************
		// * Encrypt methods.										  *
		// ************************************************************
		inline const void Encrypt(const int iVal)
		// Encrypt int.
		{
			ClearVec();
			AddVec((ULONG)iVal);
			AddVec(0L);
			EncryptInner();
		}
		inline const void Encrypt(const TCHAR chVal)
		// Encrypt TCHAR.
		{
			ClearVec();
			AddVec((ULONG)chVal);
			AddVec(0L);
			EncryptInner();
		}
		inline const void Encrypt(const CString &cstrVal)
		// Encrypt CString.
		{
			ClearVec();
			if(m_iEncryptDecryptStandard == eXXTEA)
			{
				ULONG ulLength = (ULONG)cstrVal.GetLength();
				for(ULONG ulId = 0; ulId < ulLength; ulId++)
				{
					AddVec((ULONG)cstrVal[ulId]);
				}
				EncryptInner();
			}
			else
			{
				ULONG ulLength = 2L * (ULONG)cstrVal.GetLength();
				for(ULONG ulId = 0; ulId < ulLength; ulId += 2)
				{
					AddVec((ULONG)cstrVal[ulId / 2]);
					AddVec(0L);
					EncryptInner(ulId);
				}
			}
		}
		inline const void Encrypt(char *pBuf, CONST ULONG ulBufLen)
		// Encrypt char* buffer.
		{
			ClearVec();
			if(m_iEncryptDecryptStandard == eXXTEA)
			{
				for(ULONG ulId = 0; ulId < ulBufLen; ulId++)
				{
					AddVec((ULONG)*(pBuf + ulId));
				}
				EncryptInner();
			}
			else
			{
				for(ULONG ulId = 0; ulId < 2L * ulBufLen; ulId += 2)
				{
					AddVec((ULONG)*(pBuf + ulId / 2));
					AddVec(0L);
					EncryptInner(ulId);
				}
			}
		}
		// ************************************************************
		// * Decrypt methods.										  *
		// ************************************************************
		inline const void Decrypt(int &iVal)
		// Decrypt int.
		{
			DecryptInner();
			iVal = (int)GetVec(0L);
		}
		inline const void Decrypt(TCHAR &chVal)
		// Decrypt TCHAR.
		{
			DecryptInner();
			chVal = (TCHAR)GetVec(0L);
		}
		inline const void Decrypt(CString &cstrVal)
		// Decrypt CString.
		{
			cstrVal.Empty();
			if(m_iEncryptDecryptStandard == eXXTEA)
			{
				DecryptInner();
				for(ULONG uId = 0; uId < GetVecSize(); uId++)
				{
					cstrVal += (TCHAR)GetVec(uId);
				}
			}
			else
			{
				for(ULONG uId = 0; uId < GetVecSize(); uId += 2)
				{
					DecryptInner(uId);
					cstrVal += (TCHAR)GetVec(uId);
				}
			}
		}
		inline const void Decrypt(char *pBuf, CONST ULONG ulBufLen)
		// Decrypt char* buffer.
		{
			::memset(pBuf, 0, ulBufLen);
			if(m_iEncryptDecryptStandard == eXXTEA)
			{
				DecryptInner();
				for(ULONG uId = 0; uId < GetVecSize(); uId++)
				{
					pBuf[uId] = (char)GetVec(uId);
				}
			}
			else
			{
				for(ULONG uId = 0; uId < GetVecSize(); uId += 2)
				{
					DecryptInner(uId);
					pBuf[uId / 2] = (char)GetVec(uId);
				}
			}
		}
		// ************************************************************
		// * Other supp. methods.									  *
		// ************************************************************
		inline const void ClearEncryptedValues()
		// Wipe encrypted values.
		{ ClearVec(); }
		inline const CString GetEncryptedValues()
		// Get encrypted values to comma sep. string. This list can be saved public pretty safe and can be visible.
		{
			TRY
			{
				CString cstrCommaSepValues(_T(""));
				for(ULONG uId = 0; uId < GetVecSize(); uId++)
				{
					// Add.
					CString cstrCommaSepValue(_T(""));
					cstrCommaSepValue.Format(TEXT("%lu"), GetVec(uId));
					if(cstrCommaSepValues.IsEmpty())
						cstrCommaSepValues = cstrCommaSepValue;
					else
						cstrCommaSepValues += CString(_T(", ")) + cstrCommaSepValue;
				}
				return cstrCommaSepValues;
			}
			CATCH_ALL(e)
			{
				// Do something.
				THROW_LAST();
			}
			END_CATCH_ALL
			return _T("");
		}
		inline const void SetEncryptedValues(const CString &cstrCommaSepValues)
		// Set encrypted values from comma sep. string. This is a reconstruction of the encryption obj. from the public saved list.
		{
			TRY
			{
				if(cstrCommaSepValues.IsEmpty())
					return;
				wchar_t *chStop = NULL;
				CString cstrCommaSepValue(_T(""));
				ClearVec();
				for(ULONG uId = 0; 1; uId++)
				{
					cstrCommaSepValue = ::GetSubString(cstrCommaSepValues, uId, TCHAR(','));
					cstrCommaSepValue = cstrCommaSepValue.Trim();
					if(cstrCommaSepValue.IsEmpty())
						// No more to add.
						break;
					else
					{
						// Add.
						ULONG ulVal = ::wcstoul(cstrCommaSepValue, &chStop, 10);
						if(ulVal == 0 || ulVal == ULONG_MAX)
						   throw new CInvalidArgException();
						AddVec(ulVal);
					}
				}
			}
			CATCH(CInvalidArgException, e)
			{
				// Do something.
			}
			CATCH_ALL(e)
			{
				// Do something.
				THROW_LAST();
			}
			END_CATCH_ALL
		}
		inline const void SetEncryptionStandard(const enum EEncryptDecryptStandard iEncryptDecryptStandard)
		{ m_iEncryptDecryptStandard = iEncryptDecryptStandard; }
	protected:
		// 3. Methods. (alphabetical).
		//
		// ************************************************************
		// * Encrypt/decrypt delegate methods.						  *
		// ************************************************************
		inline const void EncryptInner(const int iOffset = 0)
		{
			switch(m_iEncryptDecryptStandard)
			{
				case eTEA:
					TEAEncrypt(iOffset);
					break;
				case eXTEA:
					XTEAEncrypt(iOffset);
					break;
				case eXXTEA:
					XXTEAEncrypt();
					break;
			}
		}
		inline const void DecryptInner(const int iOffset = 0)
		{
			switch(m_iEncryptDecryptStandard)
			{
				case eTEA:
					TEADecrypt(iOffset);
					break;
				case eXTEA:
					XTEADecrypt(iOffset);
					break;
				case eXXTEA:
					XXTEADecrypt();
					break;
			}
		}
		// ********************
		// * TEA.			  *
		// ********************
		inline const void TEAEncrypt(const int iOffset)
		// Core encryption method (TEA) inspired from http://en.wikipedia.org/wiki/Tiny_Encryption_Algorithm.
		{
			ULONG ulSum = 0;
			ULONG ulVec[SIZE_OF_VEC_TEA] = { GetVec(iOffset + 0), GetVec(iOffset + 1) };
			for(register int iBit = 0; iBit < 32; iBit++)
			{
				ulSum += m_ulDelta;
				ulVec[0] += ((ulVec[1] << 4) + m_uKey[0]) ^ (ulVec[1] + ulSum) ^ ((ulVec[1] >> 5) + m_uKey[1]);
				ulVec[1] += ((ulVec[0] << 4) + m_uKey[2]) ^ (ulVec[0] + ulSum) ^ ((ulVec[0] >> 5) + m_uKey[3]);
			}
			SetVec(iOffset + 0, ulVec[0]);
			SetVec(iOffset + 1, ulVec[1]);
		}
		inline const void TEADecrypt(const int iOffset)
		// Core decryption method (TEA) inspired from http://en.wikipedia.org/wiki/Tiny_Encryption_Algorithm.
		{
			ULONG ulSum = 0xc6ef3720;
			ULONG ulVec[SIZE_OF_VEC_TEA] = { GetVec(iOffset + 0), GetVec(iOffset + 1) };
			for(register int iBit = 0; iBit < 32; iBit++)
			{
				ulVec[1] -= ((ulVec[0] << 4) + m_uKey[2]) ^ (ulVec[0] + ulSum) ^ ((ulVec[0] >> 5) + m_uKey[3]);
				ulVec[0] -= ((ulVec[1] << 4) + m_uKey[0]) ^ (ulVec[1] + ulSum) ^ ((ulVec[1] >> 5) + m_uKey[1]);
				ulSum -= m_ulDelta;
			}
			SetVec(iOffset + 0, ulVec[0]);
			SetVec(iOffset + 1, ulVec[1]);
		}
		// ********************
		// * XTEA.			  *
		// ********************
		inline const void XTEAEncrypt(const int iOffset, CONST ULONG ulNumRounds = 32)
		// Core encryption method (XTEA) inspired from http://en.wikipedia.org/wiki/XTEA.
		{
			ULONG ulSum = 0L;
			ULONG ulVec[SIZE_OF_VEC_TEA] = { GetVec(iOffset + 0), GetVec(iOffset + 1) };
			for(register ULONG ulRound = 0L; ulRound < ulNumRounds; ulRound++)
			{
				ulVec[0] += (((ulVec[1] << 4) ^ (ulVec[1] >> 5)) + ulVec[1]) ^ (ulSum + m_uKey[ulSum & 3]);
				ulSum += m_ulDelta;
				ulVec[1] += (((ulVec[0] << 4) ^ (ulVec[0] >> 5)) + ulVec[0]) ^ (ulSum + m_uKey[(ulSum >> 11) & 3]);
			}
			SetVec(iOffset + 0, ulVec[0]);
			SetVec(iOffset + 1, ulVec[1]);
		}
		inline const void XTEADecrypt(const int iOffset, CONST ULONG ulNumRounds = 32)
		// Core decryption method (XTEA) inspired from http://en.wikipedia.org/wiki/XTEA.
		{
			ULONG ulSum = m_ulDelta * ulNumRounds;
			ULONG ulVec[SIZE_OF_VEC_TEA] = { GetVec(iOffset + 0), GetVec(iOffset + 1) };
			for(register ULONG ulRound = 0L; ulRound < ulNumRounds; ulRound++)
			{
				ulVec[1] -= (((ulVec[0] << 4) ^ (ulVec[0] >> 5)) + ulVec[0]) ^ (ulSum + m_uKey[(ulSum >> 11) & 3]);
				ulSum -= m_ulDelta;
				ulVec[0] -= (((ulVec[1] << 4) ^ (ulVec[1] >> 5)) + ulVec[1]) ^ (ulSum + m_uKey[ulSum & 3]);
			}
			SetVec(iOffset + 0, ulVec[0]);
			SetVec(iOffset + 1, ulVec[1]);
		}
		// ********************
		// * XXTEA.			  *
		// ********************
		#define MX ((ulVecLast >> 5 ^ ulVecFirst << 2) + (ulVecFirst >> 3 ^ ulVecLast << 4) ^ (ulSum ^ ulVecFirst) + (m_uKey[ulRound & 3 ^ ulE] ^ ulVecLast))
		inline const void XXTEAEncrypt()
		// Core encryption method (XXTEA) inspired from http://en.wikipedia.org/wiki/XXTEA
		{
			LONG lN = (LONG)GetVecSize();
			ULONG *ulVec = new ULONG[lN];
			::memset(ulVec, 0, lN * sizeof(ULONG));
			for(int iId = 0; iId < lN; iId++)
				ulVec[iId] = GetVec(iId);
			ULONG 
				ulE = 0L, 
				ulRound = 0L, 
				ulSum = 0L, 
				ulVecLast = ulVec[lN - 1], 
				ulVecFirst = ulVec[0];
			LONG lQ = 6 + 52 / (LONG)labs(lN);
			while(lQ-- > 0)
			{
				ulSum += m_ulDelta;
				ulE = (ulSum >> 2) & 3;
				for(ulRound = 0L; ulRound < (ULONG)lN - 1L; ulRound++)
				{
					ulVecFirst = ulVec[ulRound + 1];
					ulVecLast = ulVec[ulRound] += MX;
				}
				ulVecFirst = ulVec[0];
				ulVecLast = ulVec[lN - 1] += MX;
			}
			// Copy from local vec.
			for(int iId = 0; iId < lN; iId++)
				SetVec(iId, ulVec[iId]);
			delete [] ulVec;
		}
		inline const void XXTEADecrypt()
		// Core decryption method (XXTEA) inspired from http://en.wikipedia.org/wiki/XXTEA
		{
			LONG lN = (LONG)GetVecSize();
			ULONG *ulVec = new ULONG[lN];
			::memset(ulVec, 0, lN * sizeof(ULONG));
			for(int iId = 0; iId < lN; iId++)
				ulVec[iId] = GetVec(iId);
			ULONG 
				ulE = 0L, 
				ulRound = 0L, 
				ulSum = 0L, 
				ulVecLast = ulVec[lN - 1], 
				ulVecFirst = ulVec[0];
			LONG lQ = 6 + 52 / (LONG)labs(lN);
			// Decrypting part.
			ulSum = lQ * m_ulDelta ;
			while(ulSum != 0)
			{
				ulE = (ulSum >> 2) & 3;
				for(ulRound = (ULONG)lN - 1L; ulRound > 0L; ulRound--)
				{
					ulVecLast = ulVec[ulRound - 1];
					ulVecFirst = ulVec[ulRound] -= MX;
				}
				ulVecLast = ulVec[lN - 1];
				ulVecFirst = ulVec[0] -= MX;
				ulSum -= m_ulDelta;
			}
			// Copy from local vec.
			for(int iId = 0; iId < lN; iId++)
				SetVec(iId, ulVec[iId]);
			delete [] ulVec;
		}
		// ************************************************************
		// * Other supp. methods.									  *
		// ************************************************************
		inline void AddVec(ULONG ulVal)
		{ m_ulvecEncrypt.push_back(ulVal); }
		inline void ClearVec()
		{ m_ulvecEncrypt.clear(); }
		inline ULONG GetVec(ULONG uId)
		{ ULONG ulCoord = m_ulvecEncrypt.at(uId); return ulCoord; }
		inline ULONG GetVecSize()
		{ const vector <ULONG>::size_type stSize = m_ulvecEncrypt.size(); return stSize; }
		inline void SetVec(ULONG uId, ULONG ulVal)
		{ m_ulvecEncrypt.at(uId) = ulVal; }
	private:
		// 0. Properties. (alphabetical).
		EEncryptDecryptStandard m_iEncryptDecryptStandard;
		CONST ULONG m_ulDelta;
		ULONG m_uKey[SIZE_OF_KEY];
		vector <ULONG> m_ulvecEncrypt;
};

