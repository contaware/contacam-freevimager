#if !defined(AFX_GETDXVER_H__B14182E4_3AF2_4D7A_AED5_B77789819D79__INCLUDED_)
#define AFX_GETDXVER_H__B14182E4_3AF2_4D7A_AED5_B77789819D79__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

HRESULT GetDXVersion(DWORD* pdwDirectXVersion,
					 TCHAR* strDirectXVersion,
					 int cchDirectXVersion );
HRESULT GetDirectXVersionViaDxDiag(DWORD* pdwDirectXVersionMajor,
								   DWORD* pdwDirectXVersionMinor,
								   TCHAR* pcDirectXVersionLetter);
HRESULT GetDirectXVerionViaFileVersions(DWORD* pdwDirectXVersionMajor, 
										DWORD* pdwDirectXVersionMinor,
										TCHAR* pcDirectXVersionLetter);
HRESULT GetFileVersion(TCHAR* szPath,
					   ULARGE_INTEGER* pllFileVersion);
ULARGE_INTEGER MakeInt64(WORD a, WORD b, WORD c, WORD d);
int CompareLargeInts(ULARGE_INTEGER ullParam1,
					 ULARGE_INTEGER ullParam2 );

#endif // !defined(AFX_GETDXVER_H__B14182E4_3AF2_4D7A_AED5_B77789819D79__INCLUDED_)
