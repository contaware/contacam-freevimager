#include "stdafx.h"
#include "GetDirContentSize.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Recursively Calculate the Dir Tree Size,
// optionally returns the files count
#define GETDIRCONTENTSIZE_FREE \
if (pInfo) delete pInfo;\
if (name) delete [] name;
ULARGE_INTEGER GetDirContentSize(LPCTSTR szDirName,
								int* pFilesCount/*=NULL*/,
								CWorkerThread* pThread/*=NULL*/)
{
	HANDLE hp;
	BOOL bBackslashEnding;
	ULARGE_INTEGER Size, SizeReturned;
	Size.QuadPart = 0;
	SizeReturned.QuadPart = 0;

	// Check
	if (szDirName == NULL || _tcslen(szDirName) == 0)
		return Size;

	// Allocate on heap because we are a recursive function,
	// using the stack can overflow the stack!
	WIN32_FIND_DATA* pInfo = NULL;
	TCHAR* name = NULL;
	pInfo = new WIN32_FIND_DATA;
	if (!pInfo)
	{
		GETDIRCONTENTSIZE_FREE;
		return Size;
	}
	name = new TCHAR[MAX_PATH];
	if (!name)
	{
		GETDIRCONTENTSIZE_FREE;
		return Size;
	}
	if (_tcslen(szDirName) > MAX_PATH - 5) // Make sure we have some chars left to add '\\' and '*' and to avoid an auto-recursion!
	{
		GETDIRCONTENTSIZE_FREE;
		return Size;
	}
	if (szDirName[_tcslen(szDirName) - 1] == _T('\\'))
	{
		bBackslashEnding = TRUE;
		_sntprintf(name, MAX_PATH - 1, _T("%s*"), szDirName);
		name[MAX_PATH - 1] = _T('\0');
	}
	else
	{
		bBackslashEnding = FALSE;
		_sntprintf(name, MAX_PATH - 1, _T("%s\\*"), szDirName);
		name[MAX_PATH - 1] = _T('\0');
	}
	hp = FindFirstFile(name, pInfo);
	if (!hp || (hp == INVALID_HANDLE_VALUE))
	{
		GETDIRCONTENTSIZE_FREE;
		return Size;
	}
	do
	{
		// Do Exit?
		if (pThread && pThread->DoExit())
			break;
		if (pInfo->cFileName[1] == _T('\0') &&
			pInfo->cFileName[0] == _T('.'))
			continue;
		else if (pInfo->cFileName[2] == _T('\0') &&
			pInfo->cFileName[1] == _T('.') &&
			pInfo->cFileName[0] == _T('.'))
			continue;
		if (bBackslashEnding)
			_sntprintf(name, MAX_PATH - 1, _T("%s%s"), szDirName, pInfo->cFileName);
		else
			_sntprintf(name, MAX_PATH - 1, _T("%s\\%s"), szDirName, pInfo->cFileName);
		name[MAX_PATH - 1] = _T('\0');
		if (pInfo->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			SizeReturned = GetDirContentSize(name, pFilesCount, pThread);
		}
		else
		{
			SizeReturned.QuadPart = (((ULONGLONG)pInfo->nFileSizeHigh) << 32) +
				(ULONGLONG)pInfo->nFileSizeLow;
			if (pFilesCount)
				(*pFilesCount)++;
		}
		Size.QuadPart += SizeReturned.QuadPart;
	} while (FindNextFile(hp, pInfo));

	// Clean-up
	FindClose(hp);
	GETDIRCONTENTSIZE_FREE;

	return Size;
}