/*
http://tedwvc.wordpress.com/2011/04/16/static-mfc-code-bloat-problem-from-vc2010-is-now-in-vc2008-sp1security-fix/

It appears after installing KB2465361 and then rebuilding your existing statically linked MFC application.
What happened is that themehelper.cpp now uses the function AfxLoadSystemLibraryUsingFullPath which resides
in the afxglobals.cpp file (as an inline function, no less!). As we know if your app has to pull in afxglobals.obj
for any reason, a cascade effect occurs and approximately 1.2 megs are added to your release EXE size.
*/
HMODULE AfxLoadSystemLibraryUsingFullPath(const WCHAR *pszLibrary)
{
	WCHAR wszLoadPath[MAX_PATH+1];
	if (::GetSystemDirectoryW(wszLoadPath, _countof(wszLoadPath)) == 0)
	{
		return NULL;
	}

	if (wszLoadPath[wcslen(wszLoadPath)-1] != L'\\')
	{
		if (wcscat_s(wszLoadPath, _countof(wszLoadPath), L"\\") != 0)
		{
			return NULL;
		}
	}

	if (wcscat_s(wszLoadPath, _countof(wszLoadPath), pszLibrary) != 0)
	{
		return NULL;
	}

	return(::AfxCtxLoadLibraryW(wszLoadPath));
}

/*
http://tedwvc.wordpress.com/2011/04/16/fixing-problems-with-findactctxsectionstring-in-mfc-security-updates/

Bug #1 - Make it running on Windows 2000
Bug #2 - missing cbSize initializer
Bug #3 - memory leak (pass in zero as return handle not freed)
*/
#undef FindActCtxSectionString
#define FindActCtxSectionString MyFindActCtxSectionString
#ifdef _UNICODE
#define _FINDACTCTXSECTIONSTRING "FindActCtxSectionStringW"
#else
#define _FINDACTCTXSECTIONSTRING "FindActCtxSectionStringA"
#endif
typedef BOOL (WINAPI * PFN_FINDAC)(DWORD dwFlags, const GUID *lpExtensionGuid, ULONG ulSectionId, LPCTSTR lpStringToFind, PACTCTX_SECTION_KEYED_DATA ReturnedData);
BOOL WINAPI MyFindActCtxSectionString(
     DWORD dwFlags,
     const GUID *lpExtensionGuid,
     ULONG ulSectionId,
     LPCTSTR lpStringToFind,
     PACTCTX_SECTION_KEYED_DATA ReturnedData)
{
	// Bug #1 - Windows 2000
	PFN_FINDAC pfnFindActCtxSectionString = NULL;
	{
		HINSTANCE hKernel32 = ::GetModuleHandle(_T("kernel32.dll"));
		if (hKernel32 == NULL)
		{
			return FALSE;
		}

		pfnFindActCtxSectionString = (PFN_FINDAC)::GetProcAddress(hKernel32, _FINDACTCTXSECTIONSTRING);

		if (pfnFindActCtxSectionString == NULL)
		{
			/* pre-fusion OS, so no more checking.*/
			return FALSE;
		}
	}

	ReturnedData->cbSize = sizeof(ACTCTX_SECTION_KEYED_DATA); // Bug #2 - missing cbSize initializer
	return pfnFindActCtxSectionString(/* dwFlags */ 0,  // Bug #3 memory leak - pass in zero as return handle not freed
		lpExtensionGuid, ulSectionId, lpStringToFind, ReturnedData);
}
#include "..\src\mfc\appcore.cpp"