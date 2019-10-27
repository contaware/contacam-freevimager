// LicenseHelper.cpp : implementation file
//

#include "stdafx.h"
#include "LicenseHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma comment(lib, "Version.lib") // to support GetFileVersionInfo()

// The one and only CDonorEmailValidateThread object
CDonorEmailValidateThread g_DonorEmailValidateThread;

CString CDonorEmailValidateThread::GetAppLanguage()
{
	CString sLanguageName;
	TCHAR szProgramName[MAX_PATH];
	if (::GetModuleFileName(NULL, szProgramName, MAX_PATH) != 0)
	{
		DWORD dwDummy;
		DWORD dwSize = ::GetFileVersionInfoSize(szProgramName, &dwDummy);
		if (dwSize > 0)
		{
			LPBYTE pData = new BYTE[dwSize];
			if (pData)
			{
				if (::GetFileVersionInfo(szProgramName, NULL, dwSize, pData))
				{
					UINT uiLen = 0;
					struct LANGANDCODEPAGE {
						WORD wLanguage;
						WORD wCodePage;
					} *lpTranslate;
					if (::VerQueryValue(pData, _T("\\VarFileInfo\\Translation"), (void**)&lpTranslate, (UINT*)&uiLen))
					{
						TCHAR szLanguageName[MAX_PATH];
						::VerLanguageName(MAKELANGID(lpTranslate->wLanguage, SUBLANG_NEUTRAL), szLanguageName, MAX_PATH);
						szLanguageName[MAX_PATH - 1] = _T('\0');
						sLanguageName = szLanguageName;
					}
				}
				delete[] pData;
			}
		}
	}
	return sLanguageName;
}

// Note: WinINet functions work also in ContaCam service mode, even if the
//       InternetOpen() documentation states that it should not be used
//       from a service (for that there is the WinHTTP API)
int CDonorEmailValidateThread::DonorEmailValidate()
{
	int ret = -1;

	// Connect to server and verify email
	// Note: in case that our server does not answer correctly, or if it is
	//       busy or when there is no internet connection, leave m_bNoDonation
	//       as it is and leave also ret -1
	CString sURL(_T("https://www.contaware.com/validate-437837653763456231.php"));
	sURL += _T("?email=");
	sURL += ::UrlEncode(m_sDonorEmail, TRUE);
	sURL += _T("&lang=");
	sURL += ::UrlEncode(GetAppLanguage(), TRUE);
	size_t Size;
	LPBYTE p = ::GetURL(sURL, Size, FALSE, FALSE, NULL);
	if (p)
	{
		CString s(::FromUTF8(p, Size));
		free(p);
		if (s.Find(_T("OK")) >= 0)
		{
			ret = 1;	// good email
			m_bNoDonation = FALSE;
		}
		else if (s.Find(_T("BAD")) >= 0)
		{
			ret = 0;	// bad email
			m_bNoDonation = TRUE;
		}
	}

	// But if empty then it is for sure a "bad" email
	if (m_sDonorEmail.IsEmpty())
	{
		ret = 0;		// bad email
		m_bNoDonation = TRUE;
	}

	return ret;
}

int CDonorEmailValidateThread::Work()
{
	::CoInitialize(NULL);

	// Validate after 1 second
	if (::WaitForSingleObject(GetKillEvent(), 1000U) == WAIT_OBJECT_0)
		goto exit;
	if (DonorEmailValidate() >= 0)
		goto exit;

	// Retry after 1 minute
	if (::WaitForSingleObject(GetKillEvent(), 60000U) == WAIT_OBJECT_0)
		goto exit;
	if (DonorEmailValidate() >= 0)
		goto exit;

	// Final check after 1 hour
	if (::WaitForSingleObject(GetKillEvent(), 3600000U) == WAIT_OBJECT_0)
		goto exit;
	DonorEmailValidate();

exit:
	::CoUninitialize();

	return 0;
}
