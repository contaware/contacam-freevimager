#include "stdafx.h"
#include "mmsystem.h"
#include "wininet.h"
#include "ConvertUTF.h"
#include "Helpers.h"
#include "Round.h"
#include "Rpc.h"
#include <math.h>
#include <tlhelp32.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "Wininet.lib")

#ifndef WC_NO_BEST_FIT_CHARS
#define WC_NO_BEST_FIT_CHARS      0x00000400  // Do not use best fit chars
#endif

// If InitHelpers() is not called use worst case (Win95)
BOOL g_bWin95 = TRUE;
BOOL g_bWin9x = TRUE;
BOOL g_bNT = FALSE;
BOOL g_bNT4OrOlder = FALSE;
BOOL g_bReactOS = FALSE;
BOOL g_bWin2000 = FALSE;
BOOL g_bWin2000OrHigher = FALSE;
BOOL g_bWinXP = FALSE;
BOOL g_bWinXPOrHigher = FALSE;
BOOL g_bWin2003 = FALSE;
BOOL g_bWin2003OrHigher = FALSE;
BOOL g_bWinVista = FALSE;
BOOL g_bWinVistaOrHigher = FALSE;
BOOL g_bMMX = FALSE;
BOOL g_bSSE = FALSE;
BOOL g_bSSE2 = FALSE;
BOOL g_b3DNOW = FALSE;
static BOOL g_bMakeTempFileNameCSInit = FALSE;
static CRITICAL_SECTION g_csMakeTempFileName;
// To Avoid duplicated Temp File Names,
// for example if calling this function
// several times one after the other,
// keep track of the last used timeGetTime
// value and inc. the current timeGetTime
// if they are the same.
static volatile DWORD g_dwLastTimeGetTime = 0;

#ifdef CRACKCHECK
LPBYTE g_pCodeStart = NULL;
DWORD g_dwCodeSize;
#endif

// Cpu Instruction Set Support
#define CPU_FEATURE_MMX		0x0001
#define CPU_FEATURE_SSE		0x0002
#define CPU_FEATURE_SSE2	0x0004
#define CPU_FEATURE_3DNOW	0x0008
int GetCpuInstr();

void InitHelpers()
{
	OSVERSIONINFO ovi = {0};
    ovi.dwOSVersionInfoSize = sizeof(ovi);
	GetVersionEx(&ovi);

	// Win95
	g_bWin95 =				(ovi.dwPlatformId == 1)		&&
							(ovi.dwMajorVersion == 4)	&&
							(ovi.dwMinorVersion < 10);

	// Win9x (Win95, Win98 or WinMe)
	g_bWin9x =				(ovi.dwPlatformId == 1);

	// NT Platform
	g_bNT =					(ovi.dwPlatformId == 2);

	// NT 4 or Older
	g_bNT4OrOlder =			(ovi.dwPlatformId == 2)		&&
							(ovi.dwMajorVersion <= 4);

	// ReactOS
	LPTSTR RosVersion;
	RosVersion = ovi.szCSDVersion + _tcslen(ovi.szCSDVersion) + 1;
	g_bReactOS = (_tcsnicmp(RosVersion, _T("ReactOS"), 7) == 0);                
	
	// Win 2000
	g_bWin2000 =			(ovi.dwPlatformId == 2)		&&
							(ovi.dwMajorVersion == 5)	&&
							(ovi.dwMinorVersion == 0);

	g_bWin2000OrHigher =	(ovi.dwPlatformId == 2)	&&
							(ovi.dwMajorVersion >= 5);

	// XP
	g_bWinXP =				(ovi.dwPlatformId == 2)		&&
							(ovi.dwMajorVersion == 5)	&&
							(ovi.dwMinorVersion == 1);

	g_bWinXPOrHigher =		(ovi.dwPlatformId == 2)		&&
							((ovi.dwMajorVersion == 5 && ovi.dwMinorVersion >= 1) ||
							(ovi.dwMajorVersion > 5));

	// Win 2003 Server
	g_bWin2003 =			(ovi.dwPlatformId == 2)		&&
							(ovi.dwMajorVersion == 5)	&&
							(ovi.dwMinorVersion == 2);

	g_bWin2003OrHigher =	(ovi.dwPlatformId == 2)		&&
							((ovi.dwMajorVersion == 5 && ovi.dwMinorVersion >= 2) ||
							(ovi.dwMajorVersion > 5));


	// Win Vista
	g_bWinVista =			(ovi.dwPlatformId == 2)		&&
							(ovi.dwMajorVersion == 6)	&&
							(ovi.dwMinorVersion == 0);

	g_bWinVistaOrHigher =	(ovi.dwPlatformId == 2)		&&
							(ovi.dwMajorVersion >= 6);

	// Supported Instruction Sets
	int nInstructionSets = GetCpuInstr();
	if (nInstructionSets & CPU_FEATURE_MMX)
		g_bMMX = TRUE;
	if (nInstructionSets & CPU_FEATURE_SSE)
		g_bSSE = TRUE;
	if (nInstructionSets & CPU_FEATURE_SSE2)
		g_bSSE2 = TRUE;
	if (nInstructionSets & CPU_FEATURE_3DNOW)
		g_b3DNOW = TRUE;

	// Init Critical Section
	if (!g_bMakeTempFileNameCSInit)
	{
		InitializeCriticalSection(&g_csMakeTempFileName);
		g_bMakeTempFileNameCSInit = TRUE;
	}
}

void EndHelpers()
{
	// Delete Critical Section
	if (g_bMakeTempFileNameCSInit)
	{
		DeleteCriticalSection(&g_csMakeTempFileName);
		g_bMakeTempFileNameCSInit = FALSE;
	}
}

//
// Example c:\mydir1\mydir2\hello.jpeg
// 
// GetDriveName()			->  c:
// GetDirName()				-> \mydir1\mydir2\
// GetDriveAndDirName()		-> c:\mydir1\mydir2\
// GetShortFileName()		-> hello.jpeg
// GetShortFileNameNoExt()	-> hello
// GetFileNameNoExt()		-> c:\mydir1\mydir2\hello
// GetFileExt()				-> .jpeg
CString GetDriveName(const CString& sFullFilePath)
{
	TCHAR szDrive[_MAX_DRIVE];
	_tsplitpath(sFullFilePath, szDrive, NULL, NULL, NULL);
	return CString(szDrive);
}

CString GetDirName(const CString& sFullFilePath)
{
	TCHAR szDir[_MAX_DIR];
	_tsplitpath(sFullFilePath, NULL, szDir, NULL, NULL);
	return CString(szDir);
}

CString GetDriveAndDirName(const CString& sFullFilePath)
{
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	_tsplitpath(sFullFilePath, szDrive, szDir, NULL, NULL);
	return CString(szDrive) + CString(szDir);
}

CString GetShortFileName(const CString& sFullFilePath)
{
	TCHAR szName[_MAX_FNAME];
	TCHAR szExt[_MAX_EXT];
	_tsplitpath(sFullFilePath, NULL, NULL, szName, szExt);
	return CString(szName) + CString(szExt);
}

CString GetShortFileNameNoExt(const CString& sFullFilePath)
{
	TCHAR szName[_MAX_FNAME];
	_tsplitpath(sFullFilePath, NULL, NULL, szName, NULL);
	return CString(szName);
}

CString GetFileNameNoExt(const CString& sFullFilePath)
{
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	TCHAR szName[_MAX_FNAME];
	_tsplitpath(sFullFilePath, szDrive, szDir, szName, NULL);
	return CString(szDrive) + CString(szDir) + CString(szName);
}

CString GetFileExt(const CString& sFullFilePath)
{
	TCHAR szExt[_MAX_EXT];
	_tsplitpath(sFullFilePath, NULL, NULL, NULL, szExt);
	CString sExt(szExt);
	sExt.MakeLower();
	return sExt;
}

BOOL IsReadonly(LPCTSTR lpszFileName)
{
	DWORD dwAttrib = GetFileAttributes(lpszFileName);
	if ((dwAttrib != 0xFFFFFFFF) && (dwAttrib & FILE_ATTRIBUTE_READONLY))
		return TRUE;
	else
		return FALSE;
}

BOOL HasWriteAccess(LPCTSTR lpszFileName)
{
	HANDLE hFile;
	if (IsExistingFile(lpszFileName))
	{
		hFile = CreateFile(	lpszFileName,
							GENERIC_READ | GENERIC_WRITE,
							FILE_SHARE_READ,
							NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,
							NULL);
		if (hFile == INVALID_HANDLE_VALUE)
			return FALSE;
		else
		{
			CloseHandle(hFile);
			return TRUE;
		}
	}
	else
	{
		hFile = CreateFile(	lpszFileName,
							GENERIC_READ | GENERIC_WRITE,
							FILE_SHARE_READ,
							NULL,
							CREATE_NEW,
							FILE_ATTRIBUTE_NORMAL,
							NULL);
		if (hFile == INVALID_HANDLE_VALUE)
			return FALSE;
		else
		{
			CloseHandle(hFile);
			DeleteFile(lpszFileName);
			return TRUE;
		}
	}
}

// This function is used to take the Owneship of a specified file
BOOL TakeOwnership(LPCTSTR lpszFile)
{
	if (GetVersion() < 0x80000000) // Windows NT or higher
	{
		int file[256];
		TCHAR error[256];
		DWORD description;
		SECURITY_DESCRIPTOR sd;
		SECURITY_INFORMATION info_owner = OWNER_SECURITY_INFORMATION;
			
		TOKEN_USER* owner = (TOKEN_USER*)file;
		HANDLE token;
		
		InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
		if (OpenProcessToken(GetCurrentProcess(), TOKEN_READ | TOKEN_ADJUST_PRIVILEGES, &token))
		{
			// To Get the Current Process Token & opening it to adjust the previleges
			if (SetPrivilege(token, SE_TAKE_OWNERSHIP_NAME, FALSE))
			{
				// Enabling the privilege
				if (GetTokenInformation(token, TokenUser, owner, sizeof(file), &description))
				{
					// Get the information on the opened token
					if (SetSecurityDescriptorOwner(&sd,owner->User.Sid,FALSE))
					{
						// Replace any owner information present on the security descriptor
						if (SetFileSecurity(lpszFile,info_owner, &sd))
							return TRUE;
						else
						{
							_stprintf(error, _T("Error in SetFileSecurity Error No : %d\n"), GetLastError());
							TRACE(error);
						}
					}
					else
					{
						_stprintf(error, _T("Error in SetSecurityDescriptorOwner Error No : %d\n"), GetLastError());
						TRACE(error);
					}
				}
				else
				{
					_stprintf(error, _T("Error in GetTokenInformation Error No : %d\n"), GetLastError());
					TRACE(error);
				}
			}
			else
			{
				_stprintf(error, _T("Error in SetPrivilege No : %d\n"), GetLastError());
				TRACE(error);
			}
		}
		else
		{
			_stprintf(error, _T("Error in OpenProcessToken No : %d\n"), GetLastError());
			TRACE(error);
		}

		SetPrivilege(token, SE_TAKE_OWNERSHIP_NAME, TRUE);// Disabling the set previlege
	}

	return FALSE;
}

// This function is used to enable or disable the privileges of a user
// hToken        : handle of the user token
// lpszPrivilege : name of the privilege to be set
// bChange       : if this flag is FALSE, then it enables the specified
//                 privilege. Otherwise It disables all privileges
BOOL SetPrivilege(HANDLE hToken, LPCTSTR lpszPrivilege, BOOL bChange)
{
	if (GetVersion() < 0x80000000) // Windows NT or higher
	{
		TOKEN_PRIVILEGES tp;
		LUID luid;
		BOOL bReturnValue = FALSE;

		if (lpszPrivilege != NULL && !bChange)
		{
			if (LookupPrivilegeValue( 
				NULL,            // lookup privilege on local system
				lpszPrivilege,   // privilege to lookup 
				&luid )) 
			{      // receives LUID of privilege
				tp.PrivilegeCount = 1;
				tp.Privileges[0].Luid = luid;
				tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
			}
		}

		AdjustTokenPrivileges(hToken,bChange,&tp,
			sizeof(TOKEN_PRIVILEGES), 
			(PTOKEN_PRIVILEGES) NULL,
			(PDWORD) NULL);  
		// Call GetLastError to determine whether the function succeeded.
		if (GetLastError() == ERROR_SUCCESS) 
			bReturnValue = TRUE;
		 
		return bReturnValue;
	}
	else
		return FALSE;
} 

// This function is used to set the Permissions of a file
// lpszFile	    : name of the file for which permissions are to be set
// lpszAccess   : access rights for the specified file
// dwAccessMask : access rights to be granted to the specified SID              
BOOL SetPermission(LPCTSTR lpszFile, LPCTSTR lpszAccess, DWORD dwAccessMask)
{
	if (GetVersion() < 0x80000000) // Windows NT or higher
	{
		int buff[512];
		TCHAR domain[512];
		TCHAR error[256];
		
		DWORD domain_size = 512;
		DWORD acl_size;

		SECURITY_DESCRIPTOR sd1;
		SECURITY_INFORMATION info_dacl = DACL_SECURITY_INFORMATION;
		PSID sid = (PSID)buff;
		ACL* acl = NULL;
		SID_NAME_USE sidname;
		DWORD sid_size = sizeof(buff);
		
		InitializeSecurityDescriptor(&sd1, SECURITY_DESCRIPTOR_REVISION);
		// To get the SID 
		if (LookupAccountName(NULL, lpszAccess, sid, &sid_size, domain, &domain_size, &sidname))
		{
			acl_size = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) + GetLengthSid(buff);
			acl = (ACL*)malloc(acl_size);
			if (!acl)
			{
				_stprintf(error, _T("Error allocating memory for the acl\n"));
				TRACE(error);
				return FALSE;
			}
			InitializeAcl(acl, acl_size, ACL_REVISION);
			if (AddAccessAllowedAce(acl, ACL_REVISION, dwAccessMask, sid))
			{
				if (SetSecurityDescriptorDacl(&sd1, TRUE, acl, FALSE))
				{
					if (SetFileSecurity(lpszFile, info_dacl, &sd1))
					{
						if (acl)
							free(acl);
						return TRUE;
					}
					else
					{
						_stprintf(error, _T("Error in SetFileSecurity Error No : %d\n"), GetLastError());
						TRACE(error);
					}
				}
				else
				{
					_stprintf(error, _T("Error in SetSecurityDescriptorDacl Error No : %d\n"), GetLastError());
					TRACE(error);
				}
			}
			else
			{
				_stprintf(error, _T("Error in AddAccessAllowedAce Error No : %d\n"), GetLastError());
				TRACE(error);
			}
		}
		else
		{
			_stprintf(error, _T("Error in LookupAccountName No : %d\n"), GetLastError());
			TRACE(error);
		}

		if (acl)
			free(acl);
	}

	return FALSE;
}

// This Iterative Solution Should Be Faster than a Recursive One!
BOOL CreateDir(LPCTSTR szNewDir)
{
	TCHAR szFolder[MAX_PATH];
	_tcsncpy(szFolder, szNewDir, MAX_PATH);
	szFolder[MAX_PATH - 1] = _T('\0');
	TCHAR* pStart = szFolder;
	TCHAR* pEnd = pStart + _tcslen(szFolder);
	TCHAR* p = pEnd;

	// Try 16 times to create the directory
	for (int i = 0 ; i < 16 ; i++)
	{
		BOOL bOK = CreateDirectory(szFolder, NULL);
		DWORD dwLastError = GetLastError();
		if (!bOK && dwLastError == ERROR_PATH_NOT_FOUND)
		{
			while (*p != _T('\\'))
			{
				if (p == pStart)
					return FALSE;
				p--;
			}
			*p = NULL;
		}
		else if (bOK || (ERROR_ALREADY_EXISTS == dwLastError))
		{
			if (p == pEnd)
				return TRUE;

			*p = _T('\\');
			while (*p)
				p++;
		}
		else
			break;
	}

	return FALSE;
}

// Recursive Directory Content Copy
#define COPYDIRCONTENT_FREE \
if (pInfo) delete pInfo;\
if (srcname) delete [] srcname;\
if (dstname) delete [] dstname;
BOOL CopyDirContent(LPCTSTR szFromDir, LPCTSTR szToDir, BOOL bOverwriteIfExists/*=TRUE*/, BOOL bContinueOnCopyError/*=TRUE*/)
{
	// Create dir
	if (!IsExistingDir(szToDir))
	{
		if (!CreateDir(szToDir))
			return FALSE;
	}

	// Vars
	HANDLE hp;
	BOOL bSrcBackslashEnding, bDstBackslashEnding;
	// Allocate on heap because we are a recursive function,
	// using the stack can overflow the stack!
	WIN32_FIND_DATA* pInfo = NULL;
	TCHAR* srcname = NULL;
	TCHAR* dstname = NULL;
	pInfo = new WIN32_FIND_DATA;
	if (!pInfo)
	{
		COPYDIRCONTENT_FREE;
		return FALSE;
	}

	// Src
	srcname = new TCHAR[MAX_PATH];
	if (!srcname)
	{
		COPYDIRCONTENT_FREE;
		return FALSE;
	}
	if (_tcslen(szFromDir) > MAX_PATH - 5) // Make sure we have some chars left to add '\\' and '*' and to avoid an auto-recursion!
	{
		COPYDIRCONTENT_FREE;
		return FALSE;
	}
	if (szFromDir[_tcslen(szFromDir) - 1] == _T('\\'))
	{
		bSrcBackslashEnding = TRUE;
		_sntprintf(srcname, MAX_PATH - 1, _T("%s*"), szFromDir);
		srcname[MAX_PATH - 1] = _T('\0');
	}
	else
	{
		bSrcBackslashEnding = FALSE;
		_sntprintf(srcname, MAX_PATH - 1, _T("%s\\*"), szFromDir);
		srcname[MAX_PATH - 1] = _T('\0');
	}

	// Dst
	dstname = new TCHAR[MAX_PATH];
	if (!dstname)
	{
		COPYDIRCONTENT_FREE;
		return FALSE;
	}
	if (szToDir[_tcslen(szToDir) - 1] == _T('\\'))
		bDstBackslashEnding = TRUE;
	else
		bDstBackslashEnding = FALSE;

	// Copy
    hp = FindFirstFile(srcname, pInfo);
    if (!hp || (hp == INVALID_HANDLE_VALUE))
	{
		COPYDIRCONTENT_FREE;
        return FALSE;
	}
    do
    {
        if (pInfo->cFileName[1] == _T('\0') &&
			pInfo->cFileName[0] == _T('.'))
            continue;
        else if (	pInfo->cFileName[2] == _T('\0')	&&
					pInfo->cFileName[1] == _T('.')	&&
					pInfo->cFileName[0] == _T('.'))
            continue;
		if (bSrcBackslashEnding)
			_sntprintf(srcname, MAX_PATH - 1, _T("%s%s"), szFromDir, pInfo->cFileName);
		else
			_sntprintf(srcname, MAX_PATH - 1, _T("%s\\%s"), szFromDir, pInfo->cFileName);
		srcname[MAX_PATH - 1] = _T('\0');
		if (bDstBackslashEnding)
			_sntprintf(dstname, MAX_PATH - 1, _T("%s%s"), szToDir, pInfo->cFileName);
		else
			_sntprintf(dstname, MAX_PATH - 1, _T("%s\\%s"), szToDir, pInfo->cFileName);
		dstname[MAX_PATH - 1] = _T('\0');
		if (pInfo->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (!CopyDirContent(srcname, dstname, bOverwriteIfExists, bContinueOnCopyError))
			{
				FindClose(hp);
				COPYDIRCONTENT_FREE;
				return FALSE;
			}
		}
		else
		{
			if (!CopyFile(srcname, dstname, !bOverwriteIfExists) && !bContinueOnCopyError)
			{
				FindClose(hp);
				COPYDIRCONTENT_FREE;
				return FALSE;
			}
		}
    }
    while (FindNextFile(hp, pInfo));

	// Clean-up
	FindClose(hp);
	COPYDIRCONTENT_FREE;

	return TRUE;
}

// Recursive Directory Content Merge
// Note: the szFromDir directory tree (without files) is left behind!
#define MERGEDIRCONTENT_FREE \
if (pInfo) delete pInfo;\
if (srcname) delete [] srcname;\
if (dstname) delete [] dstname;
BOOL MergeDirContent(LPCTSTR szFromDir, LPCTSTR szToDir, BOOL bOverwriteIfExists/*=TRUE*/, BOOL bContinueOnCopyError/*=TRUE*/)
{
	// Create dir
	if (!IsExistingDir(szToDir))
	{
		if (!CreateDir(szToDir))
			return FALSE;
	}

	// Vars
	HANDLE hp;
	BOOL bSrcBackslashEnding, bDstBackslashEnding;
	// Allocate on heap because we are a recursive function,
	// using the stack can overflow the stack!
	WIN32_FIND_DATA* pInfo = NULL;
	TCHAR* srcname = NULL;
	TCHAR* dstname = NULL;
	pInfo = new WIN32_FIND_DATA;
	if (!pInfo)
	{
		MERGEDIRCONTENT_FREE;
		return FALSE;
	}

	// Src
	srcname = new TCHAR[MAX_PATH];
	if (!srcname)
	{
		MERGEDIRCONTENT_FREE;
		return FALSE;
	}
	if (_tcslen(szFromDir) > MAX_PATH - 5) // Make sure we have some chars left to add '\\' and '*' and to avoid an auto-recursion!
	{
		MERGEDIRCONTENT_FREE;
		return FALSE;
	}
	if (szFromDir[_tcslen(szFromDir) - 1] == _T('\\'))
	{
		bSrcBackslashEnding = TRUE;
		_sntprintf(srcname, MAX_PATH - 1, _T("%s*"), szFromDir);
		srcname[MAX_PATH - 1] = _T('\0');
	}
	else
	{
		bSrcBackslashEnding = FALSE;
		_sntprintf(srcname, MAX_PATH - 1, _T("%s\\*"), szFromDir);
		srcname[MAX_PATH - 1] = _T('\0');
	}

	// Dst
	dstname = new TCHAR[MAX_PATH];
	if (!dstname)
	{
		MERGEDIRCONTENT_FREE;
		return FALSE;
	}
	if (szToDir[_tcslen(szToDir) - 1] == _T('\\'))
		bDstBackslashEnding = TRUE;
	else
		bDstBackslashEnding = FALSE;

	// Merge
    hp = FindFirstFile(srcname, pInfo);
    if (!hp || (hp == INVALID_HANDLE_VALUE))
	{
		MERGEDIRCONTENT_FREE;
        return FALSE;
	}
    do
    {
        if (pInfo->cFileName[1] == _T('\0') &&
			pInfo->cFileName[0] == _T('.'))
            continue;
        else if (	pInfo->cFileName[2] == _T('\0')	&&
					pInfo->cFileName[1] == _T('.')	&&
					pInfo->cFileName[0] == _T('.'))
            continue;
		if (bSrcBackslashEnding)
			_sntprintf(srcname, MAX_PATH - 1, _T("%s%s"), szFromDir, pInfo->cFileName);
		else
			_sntprintf(srcname, MAX_PATH - 1, _T("%s\\%s"), szFromDir, pInfo->cFileName);
		srcname[MAX_PATH - 1] = _T('\0');
		if (bDstBackslashEnding)
			_sntprintf(dstname, MAX_PATH - 1, _T("%s%s"), szToDir, pInfo->cFileName);
		else
			_sntprintf(dstname, MAX_PATH - 1, _T("%s\\%s"), szToDir, pInfo->cFileName);
		dstname[MAX_PATH - 1] = _T('\0');
		if (pInfo->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (!MergeDirContent(srcname, dstname, bOverwriteIfExists, bContinueOnCopyError))
			{
				FindClose(hp);
				MERGEDIRCONTENT_FREE;
				return FALSE;
			}
		}
		else
		{
			if (_tcsicmp(srcname, dstname) != 0)
			{
				if (bOverwriteIfExists)
					DeleteFile(dstname);
				if (!MoveFile(srcname, dstname))
				{
					if (!CopyFile(srcname, dstname, !bOverwriteIfExists))
					{
						if (!bContinueOnCopyError)
						{
							FindClose(hp);
							MERGEDIRCONTENT_FREE;
							return FALSE;
						}
					}
					else
						DeleteFile(srcname);
				}
			}
		}
    }
    while (FindNextFile(hp, pInfo));

	// Clean-up
	FindClose(hp);
	MERGEDIRCONTENT_FREE;

	return TRUE;
}

// Recursive Directory Content Deletion
#define DELETEDIRCONTENT_FREE \
if (pInfo) delete pInfo;\
if (name) delete [] name;
BOOL DeleteDirContent(LPCTSTR szDirName, BOOL bOnlyFiles/*=FALSE*/)
{
	HANDLE hp;
	BOOL bBackslashEnding;
	// Allocate on heap because we are a recursive function,
	// using the stack can overflow the stack!
	WIN32_FIND_DATA* pInfo = NULL;
	TCHAR* name = NULL;
	pInfo = new WIN32_FIND_DATA;
	if (!pInfo)
	{
		DELETEDIRCONTENT_FREE;
		return FALSE;
	}
	name = new TCHAR[MAX_PATH];
	if (!name)
	{
		DELETEDIRCONTENT_FREE;
		return FALSE;
	}
	if (_tcslen(szDirName) > MAX_PATH - 5) // Make sure we have some chars left to add '\\' and '*' and to avoid an auto-recursion!
	{
		DELETEDIRCONTENT_FREE;
		return FALSE;
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
		DELETEDIRCONTENT_FREE;
        return FALSE;
	}
    do
    {
        if (pInfo->cFileName[1] == _T('\0') &&
			pInfo->cFileName[0] == _T('.'))
            continue;
        else if (	pInfo->cFileName[2] == _T('\0')	&&
					pInfo->cFileName[1] == _T('.')	&&
					pInfo->cFileName[0] == _T('.'))
            continue;
		if (bBackslashEnding)
			_sntprintf(name, MAX_PATH - 1, _T("%s%s"), szDirName, pInfo->cFileName);
		else
			_sntprintf(name, MAX_PATH - 1, _T("%s\\%s"), szDirName, pInfo->cFileName);
		name[MAX_PATH - 1] = _T('\0');
		if (pInfo->dwFileAttributes & FILE_ATTRIBUTE_READONLY)
		{
			SetFileAttributes(name, pInfo->dwFileAttributes & ~FILE_ATTRIBUTE_READONLY);
		}
		if (pInfo->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (!bOnlyFiles && !DeleteDir(name))
			{
				FindClose(hp);
				DELETEDIRCONTENT_FREE;
				return FALSE;
			}
		}
		else
		{
			if (!DeleteFile(name))
			{
				if (!TakeOwnership(name))
				{
					FindClose(hp);
					DELETEDIRCONTENT_FREE;
					return FALSE;
				}
				if (!SetPermission(name, _T("everyone"), GENERIC_ALL))
				{
					FindClose(hp);
					DELETEDIRCONTENT_FREE;
					return FALSE;
				}
				if (!DeleteFile(name))
				{
					FindClose(hp);
					DELETEDIRCONTENT_FREE;
					return FALSE;
				}
			}
		}
    }
    while (FindNextFile(hp, pInfo));

	// Clean-up
	FindClose(hp);
	DELETEDIRCONTENT_FREE;

	return TRUE;
}

BOOL DeleteDir(LPCTSTR szDirName)
{
	if (DeleteDirContent(szDirName))
	{
		if (!RemoveDirectory(szDirName))
		{
			if (!TakeOwnership(szDirName))
				return FALSE;
			if (!SetPermission(szDirName, _T("everyone"), GENERIC_ALL))
				return FALSE;
			DWORD dwFileAttributes = GetFileAttributes(szDirName);
			if (dwFileAttributes & FILE_ATTRIBUTE_READONLY)
				SetFileAttributes(szDirName, dwFileAttributes & ~FILE_ATTRIBUTE_READONLY);
			if (!RemoveDirectory(szDirName))
				return FALSE;
			return TRUE;
		}
		else
			return TRUE;
	}
	else
		return FALSE;
}

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
        else if (	pInfo->cFileName[2] == _T('\0')	&&
					pInfo->cFileName[1] == _T('.')	&&
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
    }
    while (FindNextFile(hp, pInfo));

	// Clean-up
	FindClose(hp);
	GETDIRCONTENTSIZE_FREE;
	
	return Size;
}

// Shell deletion
BOOL DeleteToRecycleBin(LPCTSTR szName, BOOL bSilent/*=TRUE*/)
{
	TCHAR pFrom[MAX_PATH+1]; // +1 for double NULL Termination
	memset(pFrom, 0, MAX_PATH+1);
	_tcsncpy(pFrom, szName, MAX_PATH);

	SHFILEOPSTRUCT FileOp;
	memset(&FileOp, 0, sizeof(SHFILEOPSTRUCT));
	FileOp.hwnd = NULL; 
    FileOp.pFrom = pFrom; 
    FileOp.pTo = NULL; 
    FileOp.fFlags = FOF_ALLOWUNDO | (bSilent ? FOF_SILENT | FOF_NOCONFIRMATION : 0);
    FileOp.hNameMappings = NULL; 
    FileOp.lpszProgressTitle = NULL; 
	FileOp.fAnyOperationsAborted = FALSE; 
	FileOp.wFunc = FO_DELETE;

	return (SHFileOperation(&FileOp) == 0);
}

// Shell Rename
BOOL RenameShell(LPCTSTR szOldName, LPCTSTR szNewName, BOOL bSilent/*=TRUE*/)
{
	// pFrom and pTo have to be double NULL terminated! 
	TCHAR pFrom[MAX_PATH+1];
	TCHAR pTo[MAX_PATH+1];
	memset(pFrom, 0, MAX_PATH+1);
	memset(pTo, 0, MAX_PATH+1);
	_tcsncpy(pFrom, szOldName, MAX_PATH);
	_tcsncpy(pTo, szNewName, MAX_PATH);

	SHFILEOPSTRUCT FileOp;
	memset(&FileOp, 0, sizeof(SHFILEOPSTRUCT));
	FileOp.hwnd = NULL; 
    FileOp.pFrom = pFrom;
    FileOp.pTo = pTo;
    FileOp.fFlags = bSilent ? FOF_SILENT | FOF_NOERRORUI | FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR : 0;
    FileOp.hNameMappings = NULL; 
    FileOp.lpszProgressTitle = NULL; 
	FileOp.fAnyOperationsAborted = FALSE; 
	FileOp.wFunc = FO_RENAME;

	return (SHFileOperation(&FileOp) == 0);
}

// Shell Move
BOOL MoveShell(LPCTSTR szFromName, LPCTSTR szToName, BOOL bSilent/*=TRUE*/)
{
	// pFrom and pTo have to be double NULL terminated! 
	TCHAR pFrom[MAX_PATH+1];
	TCHAR pTo[MAX_PATH+1];
	memset(pFrom, 0, MAX_PATH+1);
	memset(pTo, 0, MAX_PATH+1);
	_tcsncpy(pFrom, szFromName, MAX_PATH);
	_tcsncpy(pTo, szToName, MAX_PATH);

	SHFILEOPSTRUCT FileOp;
	memset(&FileOp, 0, sizeof(SHFILEOPSTRUCT));
	FileOp.hwnd = NULL; 
    FileOp.pFrom = pFrom;
    FileOp.pTo = pTo;
    FileOp.fFlags = bSilent ? FOF_SILENT | FOF_NOERRORUI | FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR : 0;
    FileOp.hNameMappings = NULL; 
    FileOp.lpszProgressTitle = NULL; 
	FileOp.fAnyOperationsAborted = FALSE; 
	FileOp.wFunc = FO_MOVE;

	return (SHFileOperation(&FileOp) == 0);
}

// Shell Copy
BOOL CopyShell(LPCTSTR szFromName, LPCTSTR szToName, BOOL bSilent/*=TRUE*/)
{
	// pFrom and pTo have to be double NULL terminated! 
	TCHAR pFrom[MAX_PATH+1];
	TCHAR pTo[MAX_PATH+1];
	memset(pFrom, 0, MAX_PATH+1);
	memset(pTo, 0, MAX_PATH+1);
	_tcsncpy(pFrom, szFromName, MAX_PATH);
	_tcsncpy(pTo, szToName, MAX_PATH);

	SHFILEOPSTRUCT FileOp;
	memset(&FileOp, 0, sizeof(SHFILEOPSTRUCT));
	FileOp.hwnd = NULL; 
    FileOp.pFrom = pFrom;
    FileOp.pTo = pTo;
    FileOp.fFlags = bSilent ? FOF_SILENT | FOF_NOERRORUI | FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR : 0;
    FileOp.hNameMappings = NULL; 
    FileOp.lpszProgressTitle = NULL; 
	FileOp.fAnyOperationsAborted = FALSE; 
	FileOp.wFunc = FO_COPY;

	return (SHFileOperation(&FileOp) == 0);
}

// Shell Directory Content Copy
BOOL CopyDirContentShell(LPCTSTR szFromDir, LPCTSTR szToDir, BOOL bSilent/*=TRUE*/)
{
	// Check
	if (_tcslen(szFromDir) <= 0 ||
		_tcslen(szToDir) <= 0)
		return FALSE;

	// pFrom and pTo have to be double NULL terminated! 
	TCHAR pFrom[MAX_PATH+5];
	TCHAR pTo[MAX_PATH+1];
	memset(pFrom, 0, MAX_PATH+5);
	memset(pTo, 0, MAX_PATH+1);
	_tcsncpy(pFrom, szFromDir, MAX_PATH);
	_tcsncpy(pTo, szToDir, MAX_PATH);
	int pos = _tcslen(szFromDir) - 1;
	if (pFrom[pos++] != _T('\\')) 
		pFrom[pos++] = _T('\\');
	pFrom[pos++] = _T('*');
	pFrom[pos++] = _T('.');
	pFrom[pos++] = _T('*');

	SHFILEOPSTRUCT FileOp;
	memset(&FileOp, 0, sizeof(SHFILEOPSTRUCT));
	FileOp.hwnd = NULL; 
    FileOp.pFrom = pFrom;
    FileOp.pTo = pTo;
    FileOp.fFlags = bSilent ? FOF_SILENT | FOF_NOERRORUI | FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR : 0;
    FileOp.hNameMappings = NULL; 
    FileOp.lpszProgressTitle = NULL; 
	FileOp.fAnyOperationsAborted = FALSE; 
	FileOp.wFunc = FO_COPY;

	return (SHFileOperation(&FileOp) == 0);
}

CString MakeTimeLocalFormat(const CTime& Time,
							BOOL bShowSeconds/*=FALSE*/)
{
	SYSTEMTIME st;
	Time.GetAsSystemTime(st);
	int nSize = GetTimeFormat(	LOCALE_USER_DEFAULT,
								bShowSeconds ? 0 : TIME_NOSECONDS,
								&st,
								NULL,
								NULL,
								0);
	CString sTime;
	GetTimeFormat(	LOCALE_USER_DEFAULT,
					bShowSeconds ? 0 : TIME_NOSECONDS,
					&st,
					NULL,
					sTime.GetBuffer(nSize),
					nSize);
	sTime.ReleaseBuffer();
	return sTime;
}

CString MakeDateLocalFormat(const CTime& Time,
							BOOL bLongDate/*=FALSE*/)
{
	SYSTEMTIME st;
	Time.GetAsSystemTime(st);
	int nSize = GetDateFormat(	LOCALE_USER_DEFAULT,
								bLongDate ? DATE_LONGDATE : DATE_SHORTDATE,
								&st,
								NULL,
								NULL,
								0);
	CString sDate;
	GetDateFormat(	LOCALE_USER_DEFAULT,
					bLongDate ? DATE_LONGDATE : DATE_SHORTDATE,
					&st,
					NULL,
					sDate.GetBuffer(nSize),
					nSize);
	sDate.ReleaseBuffer();
	return sDate;
}

CString GetDateLocalFormat(BOOL bLongDate/*=FALSE*/)
{
	CString sDateFormat;

	// Get Local Date Format
	int nSize = GetLocaleInfo(	LOCALE_USER_DEFAULT,
								bLongDate ? LOCALE_SLONGDATE : LOCALE_SSHORTDATE,
								NULL,
								0);
	LPTSTR lpLCData = new TCHAR[nSize];
	GetLocaleInfo(	LOCALE_USER_DEFAULT,
					LOCALE_SSHORTDATE,
					lpLCData,
					nSize);
	sDateFormat = CString(lpLCData);
	delete [] lpLCData;
	return sDateFormat;
}

CTime ParseShortDateLocalFormat(CString sDate)
{
	int nYear, nMonth, nDay;
	int nDate[6] = {2000,1,1,12,0,0};
	TCHAR cSeparator;
	CString sDateFormat = GetDateLocalFormat();

	// Year
	int nYearPosInit = sDateFormat.Find(_T('y'));
	int nYearPosEnd = nYearPosInit;
	while (	nYearPosEnd < sDateFormat.GetLength() && 
			sDateFormat[nYearPosEnd] == _T('y'))
		nYearPosEnd++;

	// Month
	int nMonthPosInit = sDateFormat.Find(_T('M'));
	int nMonthPosEnd = nMonthPosInit;
	while (	nMonthPosEnd < sDateFormat.GetLength() && 
			sDateFormat[nMonthPosEnd] == _T('M'))
		nMonthPosEnd++;
	
	// Day
	int nDayPosInit = sDateFormat.Find(_T('d'));
	int nDayPosEnd = nDayPosInit;
	while (	nDayPosEnd < sDateFormat.GetLength() && 
			sDateFormat[nDayPosEnd] == _T('d'))
		nDayPosEnd++;

	// Year/Month/Day Format
	if (nYearPosInit < nMonthPosInit && nMonthPosInit < nDayPosInit)
	{
		cSeparator = sDateFormat[nYearPosEnd];
		sDate.Replace(cSeparator, _T(' '));
		_stscanf(sDate, _T("%d %d %d"), &nYear, &nMonth, &nDay);
	}
	// Day/Month/Year Format
	else if (nDayPosInit < nMonthPosInit && nMonthPosInit < nYearPosInit)
	{
		cSeparator = sDateFormat[nDayPosEnd];
		sDate.Replace(cSeparator, _T(' '));
		_stscanf(sDate, _T("%d %d %d"), &nDay, &nMonth, &nYear);
	}
	// Month/Day/Year Format
	else if (nMonthPosInit < nDayPosInit && nDayPosInit < nYearPosInit)
	{
		cSeparator = sDateFormat[nDayPosEnd];
		sDate.Replace(cSeparator, _T(' '));
		_stscanf(sDate, _T("%d %d %d"), &nMonth, &nDay, &nYear);
	}

	// Check
	if (nYear >= 0 && nYear < 70)
	{
		nYear += 2000;
		nDate[0] = nYear;
	}
	else if (nYear >= 70 && nYear < 100)
	{
		nYear += 1900;
		nDate[0] = nYear;
	}
#if _MFC_VER >= 0x0700
	if (nYear >= 1971 && nYear <= 3000) // MFC CTime Limitation (January 1, 1970 00:00:00 crashes also...)
#else
	if (nYear >= 1971 && nYear <= 2037) // MFC CTime Limitation (January 1, 1970 00:00:00 crashes also...)
#endif
		nDate[0] = nYear;
	if (nMonth >= 1 && nMonth <= 12)
		nDate[1] = nMonth;
	if (nDay >= 1 && nDay <= 31)
		nDate[2] = nDay;

	// Make CTime Object
	CTime Time(nDate[0], nDate[1], nDate[2], nDate[3], nDate[4], nDate[5]);
	
	return Time;
}

CString MakeTempFileName(CString sTempPath, LPCTSTR lpszFileName)
{
	// Enter CS for thread safety
	if (g_bMakeTempFileNameCSInit)
		EnterCriticalSection(&g_csMakeTempFileName);

	// Make sure the Temp Path terminates with a backslash
	if (sTempPath.GetLength() > 0)
	{
		if (sTempPath[sTempPath.GetLength() - 1] != _T('\\'))
			sTempPath += _T('\\');
	}

	// Initial temp file
	CString sTempFile = sTempPath + GetShortFileNameNoExt(lpszFileName);
	
	// Get current tick, avoid duplicates
	CString sTime;
	DWORD dwCurrentTimeGetTime = timeGetTime();
	if (dwCurrentTimeGetTime != g_dwLastTimeGetTime)
		sTime.Format(_T("%X"), dwCurrentTimeGetTime);
	else
		sTime.Format(_T("%X"), ++dwCurrentTimeGetTime);
	g_dwLastTimeGetTime = dwCurrentTimeGetTime;
	
	// Full temp file
	sTempFile = (sTempFile + _T("_") + sTime + GetFileExt(lpszFileName));

	// Leave CS
	if (g_bMakeTempFileNameCSInit)
		LeaveCriticalSection(&g_csMakeTempFileName);
	
	return sTempFile;
}

BOOL IsExistingFile(LPCTSTR lpszFileName)
{
	DWORD dwAttrib = GetFileAttributes(lpszFileName);
	if (dwAttrib != 0xFFFFFFFF)
	{
		// Directory
		if (!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
			return TRUE;
		else
			return FALSE;
	}
	else
		return FALSE;
}

BOOL IsExistingDir(LPCTSTR lpszFileName)
{
	DWORD dwAttrib = GetFileAttributes(lpszFileName);
	if (dwAttrib != 0xFFFFFFFF)
	{
		// Directory
		if (dwAttrib & FILE_ATTRIBUTE_DIRECTORY)
			return TRUE;
		else
			return FALSE;
	}
	else
		return FALSE;
}

// Slow
/*
ULARGE_INTEGER GetFileSize64(LPCTSTR lpszFileName)
{
	ULARGE_INTEGER Size;
	Size.QuadPart = 0;

	if (sFileName == _T(""))
		return Size;

	HANDLE hFile = CreateFile(	lpszFileName,
								0, // Only Query Access
								FILE_SHARE_READ |
								FILE_SHARE_WRITE,
								NULL, OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL,
								NULL);

	if (hFile == INVALID_HANDLE_VALUE)
		return Size;
	
	Size.LowPart = GetFileSize(hFile, &(Size.HighPart));

	CloseHandle(hFile);

	return Size;
}
*/

ULARGE_INTEGER GetFileSize64(LPCTSTR lpszFileName)
{
	ULARGE_INTEGER Size;
	Size.QuadPart = 0;
	WIN32_FIND_DATA fileinfo;
	memset(&fileinfo, 0, sizeof(WIN32_FIND_DATA));
	HANDLE hFind = FindFirstFile(lpszFileName, &fileinfo);
	Size.LowPart = fileinfo.nFileSizeLow;
	Size.HighPart = fileinfo.nFileSizeHigh;
	FindClose(hFind);
	return Size;
}

BOOL GetFileTime(	LPCTSTR lpszFileName,
					LPFILETIME lpCreationTime,
					LPFILETIME lpLastAccessTime,
					LPFILETIME lpLastWriteTime)
{
	if (lpszFileName == NULL)
		return FALSE;

	HANDLE hFile = CreateFile(	lpszFileName,
								GENERIC_READ,
								FILE_SHARE_READ |
								FILE_SHARE_WRITE,
								NULL,
								OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL,
								NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;
	
	BOOL res = GetFileTime(	hFile,
							lpCreationTime,
							lpLastAccessTime,
							lpLastWriteTime);

	CloseHandle(hFile);

	return res;
}

BOOL SetFileTime(	LPCTSTR lpszFileName,
					LPFILETIME lpCreationTime,
					LPFILETIME lpLastAccessTime,
					LPFILETIME lpLastWriteTime)
{
	if (lpszFileName == NULL)
		return FALSE;

	HANDLE hFile = CreateFile(	lpszFileName,
								GENERIC_WRITE,
								FILE_SHARE_READ,
								NULL,
								OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL,
								NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;
	
	BOOL res = SetFileTime(	hFile,
							lpCreationTime,
							lpLastAccessTime,
							lpLastWriteTime);

	CloseHandle(hFile);

	return res;
}

// Get File Status, do not use for file size because with
// old MFC Versions the size of CFileStatus is limited to 4GB.
// -> Use GetFileSize64()
//
// Note: The original CFile::GetStatus() is ASSERTING with Files > 4GB
BOOL GetFileStatus(LPCTSTR lpszFileName, CFileStatus& rStatus)
{
	if (lpszFileName == NULL) 
		return FALSE;

	if (lstrlen(lpszFileName) >= _MAX_PATH)
	{
		ASSERT(FALSE); // MFC requires paths with length < _MAX_PATH
		return FALSE;
	}

	WIN32_FIND_DATA findFileData;
	HANDLE hFind = FindFirstFile((LPTSTR)lpszFileName, &findFileData);
	if (hFind == INVALID_HANDLE_VALUE)
		return FALSE;
	VERIFY(FindClose(hFind));

	// strip attribute of NORMAL bit, our API doesn't have a "normal" bit.
	rStatus.m_attribute = (BYTE)
		(findFileData.dwFileAttributes & ~FILE_ATTRIBUTE_NORMAL);

	// Old Buggy MFC Code!!!
	// get just the low DWORD of the file size
	//ASSERT(findFileData.nFileSizeHigh == 0);
	//rStatus.m_size = (LONG)findFileData.nFileSizeLow;

	// Get the Correct Size
#if _MFC_VER >= 0x0700
	rStatus.m_size = (ULONGLONG)findFileData.nFileSizeLow |
					((ULONGLONG)findFileData.nFileSizeHigh) << 32;
#else
	rStatus.m_size = (LONG)findFileData.nFileSizeLow;
#endif

	// convert times as appropriate
#if _MFC_VER >= 0x0700
	if (CTime::IsValidFILETIME(findFileData.ftCreationTime))
		rStatus.m_ctime = CTime(findFileData.ftCreationTime);
	else
		rStatus.m_ctime = CTime();
#else
	rStatus.m_ctime = CTime(findFileData.ftCreationTime);
#endif

#if _MFC_VER >= 0x0700
	if (CTime::IsValidFILETIME(findFileData.ftLastAccessTime))
		rStatus.m_atime = CTime(findFileData.ftLastAccessTime);
	else
		rStatus.m_atime = CTime();
#else
	rStatus.m_atime = CTime(findFileData.ftLastAccessTime);
#endif

#if _MFC_VER >= 0x0700
	if (CTime::IsValidFILETIME(findFileData.ftLastWriteTime))
		rStatus.m_mtime = CTime(findFileData.ftLastWriteTime);
	else
		rStatus.m_mtime = CTime();
#else
	rStatus.m_mtime = CTime(findFileData.ftLastWriteTime);
#endif

	if (rStatus.m_ctime.GetTime() == 0)
		rStatus.m_ctime = rStatus.m_mtime;

	if (rStatus.m_atime.GetTime() == 0)
		rStatus.m_atime = rStatus.m_mtime;

	return TRUE;
}

#ifndef GCT_LFNCHAR
#define GCT_LFNCHAR             0x0001
#endif
CString MakeValidPath(CString sPath)
{
#ifdef _UNICODE
	typedef UINT (WINAPI * FPPATHGETCHARTYPE)(WCHAR ch);
#else
	typedef UINT (WINAPI * FPPATHGETCHARTYPE)(UCHAR ch);
#endif
	FPPATHGETCHARTYPE fpPathGetCharType;
	HINSTANCE h = LoadLibrary(_T("shlwapi.dll"));
	if (!h)
		return sPath;
#ifdef _UNICODE
	fpPathGetCharType = (FPPATHGETCHARTYPE)GetProcAddress(h, "PathGetCharTypeW");
#else
	fpPathGetCharType = (FPPATHGETCHARTYPE)GetProcAddress(h, "PathGetCharTypeA");
#endif
	if (fpPathGetCharType)
	{
		CString sNewPath;
		for (int i = 0 ; i < sPath.GetLength() ; i++)
		{
			if ((fpPathGetCharType(sPath[i]) & GCT_LFNCHAR)	||
				(sPath[i] == _T('\\'))						||
				(i == 1 && sPath[1] == _T(':')))
				sNewPath += sPath[i];
			else
			{
				if (sPath[i] == _T('/'))
					sNewPath += _T("\\");
				else
					sNewPath += _T("_");
			}
		}
		FreeLibrary(h);
		return sNewPath;
	}
	else
	{
		FreeLibrary(h);
		return sPath;
	}
}

CString GetSpecialFolderPath(int nSpecialFolder)
{
	CString sSpecialFolderPath;
	typedef HRESULT (WINAPI * FPSHGETSPECIALFOLDERPATH)(HWND hwndOwner, LPTSTR lpszPath, int nFolder, BOOL fCreate);
	FPSHGETSPECIALFOLDERPATH fpSHGetSpecialFolderPath;
	HINSTANCE h = LoadLibrary(_T("shell32.dll"));
	if (!h)
		return _T("");
#ifdef _UNICODE
	fpSHGetSpecialFolderPath = (FPSHGETSPECIALFOLDERPATH)GetProcAddress(h, "SHGetSpecialFolderPathW");
#else
	fpSHGetSpecialFolderPath = (FPSHGETSPECIALFOLDERPATH)GetProcAddress(h, "SHGetSpecialFolderPathA");
#endif
	if (fpSHGetSpecialFolderPath)
	{
		TCHAR path[MAX_PATH] = {0};
		fpSHGetSpecialFolderPath(NULL, path, nSpecialFolder, FALSE);
		sSpecialFolderPath = path;
	}
	FreeLibrary(h);
	return sSpecialFolderPath;
}

int EnumKillProcByName(CString sProcessName, BOOL bKill/*=FALSE*/)
{	
	// Vars
	int iCount = 0;
	HANDLE hProc;
	HINSTANCE hInstLib;

	// PSAPI
	hInstLib = LoadLibrary(_T("PSAPI.DLL"));
	if (hInstLib)
	{
		TCHAR szName[MAX_PATH];

		// Get procedure addresses
		BOOL (WINAPI *lpfEnumProcesses)(DWORD *, DWORD cb, DWORD *);
		BOOL (WINAPI *lpfEnumProcessModules)(HANDLE, HMODULE *, DWORD, LPDWORD);
		DWORD (WINAPI *lpfGetModuleBaseName)(HANDLE, HMODULE, LPTSTR, DWORD);
		lpfEnumProcesses = (BOOL(WINAPI*)(DWORD *, DWORD, DWORD*))GetProcAddress(hInstLib, "EnumProcesses");
		lpfEnumProcessModules = (BOOL(WINAPI*)(HANDLE, HMODULE *, DWORD, LPDWORD))GetProcAddress(hInstLib, "EnumProcessModules");
#ifdef _UNICODE
		lpfGetModuleBaseName = (DWORD(WINAPI*)(HANDLE, HMODULE, LPTSTR, DWORD))GetProcAddress(hInstLib, "GetModuleBaseNameW");
#else
		lpfGetModuleBaseName = (DWORD(WINAPI*)(HANDLE, HMODULE, LPTSTR, DWORD))GetProcAddress(hInstLib, "GetModuleBaseNameA");
#endif	
		if (lpfEnumProcesses		&&
			lpfEnumProcessModules	&&
			lpfGetModuleBaseName)
		{
			// How many processes are there?
			DWORD aiPID[1024];
			DWORD iCbneeded;
			if (!lpfEnumProcesses(aiPID, sizeof(aiPID), &iCbneeded))
			{
				FreeLibrary(hInstLib);
				return 0;
			}
			DWORD iNumProc = iCbneeded / sizeof(DWORD);

			// Get and match the name of each process
			for (DWORD i = 0 ; i < iNumProc ; i++)
			{
				// Get the process name
				DWORD dwStrLength = 0;
				HMODULE hMod;
				hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aiPID[i]);
				if (hProc && lpfEnumProcessModules(hProc, &hMod, sizeof(hMod), &iCbneeded))
					dwStrLength = lpfGetModuleBaseName(hProc, hMod, szName, MAX_PATH);
				CloseHandle(hProc);

				// We will match regardless of character case
				if (dwStrLength > 0 && sProcessName.CompareNoCase(szName) == 0)
				{
					iCount++;
					if (bKill)
					{
						hProc = OpenProcess(PROCESS_TERMINATE, FALSE, aiPID[i]);
						if (hProc)
						{
							TerminateProcess(hProc, 0);
							CloseHandle(hProc);
						}
					}
				}
			}

			// Free
			FreeLibrary(hInstLib);

			// Return
			return iCount;
		}
		else
			FreeLibrary(hInstLib);
	}

	// Try ToolHelp
	hInstLib = LoadLibrary(_T("Kernel32.DLL"));
	if (hInstLib == NULL)
		return 0;

	// Get procedure addresses
	HANDLE (WINAPI *lpfCreateToolhelp32Snapshot)(DWORD, DWORD);
	BOOL (WINAPI *lpfProcess32First)(HANDLE, LPPROCESSENTRY32);
	BOOL (WINAPI *lpfProcess32Next)(HANDLE, LPPROCESSENTRY32);
	BOOL (WINAPI *lpfModule32First)(HANDLE, LPMODULEENTRY32);
	BOOL (WINAPI *lpfModule32Next)(HANDLE, LPMODULEENTRY32);
	lpfCreateToolhelp32Snapshot= (HANDLE(WINAPI*)(DWORD,DWORD))GetProcAddress(hInstLib, "CreateToolhelp32Snapshot");
#ifdef _UNICODE
	lpfProcess32First = (BOOL(WINAPI*)(HANDLE,LPPROCESSENTRY32))GetProcAddress(hInstLib, "Process32FirstW") ;
	lpfProcess32Next = (BOOL(WINAPI*)(HANDLE,LPPROCESSENTRY32))GetProcAddress(hInstLib, "Process32NextW");
	lpfModule32First = (BOOL(WINAPI*)(HANDLE,LPMODULEENTRY32))GetProcAddress(hInstLib, "Module32FirstW");
	lpfModule32Next = (BOOL(WINAPI*)(HANDLE,LPMODULEENTRY32))GetProcAddress(hInstLib, "Module32NextW");
#else
	lpfProcess32First = (BOOL(WINAPI*)(HANDLE,LPPROCESSENTRY32))GetProcAddress(hInstLib, "Process32First") ;
	lpfProcess32Next = (BOOL(WINAPI*)(HANDLE,LPPROCESSENTRY32))GetProcAddress(hInstLib, "Process32Next");
	lpfModule32First = (BOOL(WINAPI*)(HANDLE,LPMODULEENTRY32))GetProcAddress(hInstLib, "Module32First");
	lpfModule32Next = (BOOL(WINAPI*)(HANDLE,LPMODULEENTRY32))GetProcAddress(hInstLib, "Module32Next");
#endif
	if (lpfProcess32Next == NULL	||
		lpfProcess32First == NULL	||
		lpfModule32Next == NULL		||
		lpfModule32First == NULL	||
		lpfCreateToolhelp32Snapshot == NULL)
	{
		FreeLibrary(hInstLib);
		return 0;
	}
	
	// Get a handle to a Toolhelp snapshot of all the systems processes
	HANDLE hSnapShot = lpfCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapShot == INVALID_HANDLE_VALUE)
	{
		FreeLibrary(hInstLib);
		return 0;
	}
	
    // While there are processes, keep looping and checking
	PROCESSENTRY32 procentry;
    procentry.dwSize = sizeof(PROCESSENTRY32);
    BOOL bResult = lpfProcess32First(hSnapShot, &procentry);
    while (bResult)
    {
		// Get a handle to a Toolhelp snapshot of this process
		HANDLE hSnapShotm = lpfCreateToolhelp32Snapshot(TH32CS_SNAPMODULE, procentry.th32ProcessID);
		if (hSnapShotm != INVALID_HANDLE_VALUE)
		{
			// While there are modules, keep looping and checking
			MODULEENTRY32 modentry;
			modentry.dwSize = sizeof(MODULEENTRY32);
			BOOL bResultm = lpfModule32First(hSnapShotm, &modentry);
			while (bResultm)
			{
				// We will match regardless of character case
				if (sProcessName.CompareNoCase(modentry.szModule) == 0)
				{
					iCount++;
					if (bKill)
					{
						hProc = OpenProcess(PROCESS_TERMINATE, FALSE,procentry.th32ProcessID);
						if (hProc)
						{
							TerminateProcess(hProc, 0);
							CloseHandle(hProc);
						}
					}
				}
			
				// Keep looking
				modentry.dwSize = sizeof(MODULEENTRY32);
				bResultm = lpfModule32Next(hSnapShotm, &modentry);
			}
			CloseHandle(hSnapShotm);
		}

		// Keep looking
        procentry.dwSize = sizeof(PROCESSENTRY32);
        bResult = lpfProcess32Next(hSnapShot, &procentry);
    }

	// Free
	CloseHandle(hSnapShot);
	FreeLibrary(hInstLib);
	return iCount;
}

BOOL SelfDelete()
{
	TCHAR szModule [MAX_PATH],
		  szComspec[MAX_PATH],
		  szParams [MAX_PATH];

	// Get file path names:
	if ((GetModuleFileName(0, szModule, MAX_PATH) != 0) &&
		(GetShortPathName(szModule, szModule, MAX_PATH) != 0) &&
		(GetEnvironmentVariable(_T("COMSPEC"), szComspec, MAX_PATH) != 0))
	{
		// Set command shell parameters
		lstrcpy(szParams, _T(" /c del "));
		lstrcat(szParams, szModule);
		lstrcat(szParams, _T(" > nul"));
		lstrcat(szComspec, szParams);

		// Set struct members
		STARTUPINFO si={0};
		PROCESS_INFORMATION	pi={0};
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;

		// Increase resource allocation to program
		SetPriorityClass(GetCurrentProcess(),
				REALTIME_PRIORITY_CLASS);
		SetThreadPriority(GetCurrentThread(),
				THREAD_PRIORITY_TIME_CRITICAL);

		// Invoke command shell
		if (CreateProcess(	0, szComspec, 0, 0, 0, CREATE_SUSPENDED |
							DETACHED_PROCESS, 0, 0, &si, &pi))
		{
			// suppress command shell process until program exits
			SetPriorityClass(pi.hProcess,IDLE_PRIORITY_CLASS);
			SetThreadPriority(pi.hThread,THREAD_PRIORITY_IDLE); 

			// resume shell process with new low priority
			ResumeThread(pi.hThread);

			// everything seemed to work
			return TRUE;
		}
		else // if error, normalize allocation
		{
			SetPriorityClass(GetCurrentProcess(),
							 NORMAL_PRIORITY_CLASS);
			SetThreadPriority(GetCurrentThread(),
							  THREAD_PRIORITY_NORMAL);
		}
	}
	return FALSE;
}

BOOL ExecHiddenApp(	const CString& sFileName,
					const CString& sParams/*=_T("")*/,
					BOOL bWaitTillDone/*=FALSE*/,
					DWORD dwWaitMillisecondsTimeout/*=INFINITE*/)
{
	SHELLEXECUTEINFO sei;
	memset(&sei, 0, sizeof(sei));
	sei.cbSize = sizeof(sei);
	sei.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_NOCLOSEPROCESS;
	sei.nShow = SW_HIDE;
	sei.lpFile = sFileName;
	CString sDir = GetDriveAndDirName(sFileName);
	sei.lpDirectory = sDir;
	sei.lpParameters = sParams;
	BOOL res = ShellExecuteEx(&sei);
	if (res)
	{
		if (bWaitTillDone)
		{
			if (sei.hProcess)
			{
				if (WaitForSingleObject(sei.hProcess, dwWaitMillisecondsTimeout) != WAIT_OBJECT_0)
					res = FALSE;
			}
			else
				res = FALSE;
		}
	}
	if (sei.hProcess)
		CloseHandle(sei.hProcess);
	return res;
}

BOOL IsRegistryValue(HKEY hOpenKey, LPCTSTR szKey, LPCTSTR szValue)
{
	LONG lRet;

	// Validate Input
	if (!hOpenKey || !szKey || !szKey[0] || !szValue)
		return FALSE;

	HKEY hSubKey;
	lRet = RegOpenKeyEx(
						hOpenKey,		// key handle at root level
						szKey,			// path name of child key
						0,				// reserved
						KEY_READ,		// requesting access
						&hSubKey		// address of key to be returned
						);
	if (lRet != ERROR_SUCCESS)
		return FALSE;

	// Get Type and Get Buffer Size
	DWORD dwType, dwBufSize;
	lRet = RegQueryValueEx(
						hSubKey,		// handle to key to query
						szValue,		// address of name of value to query
						0,				// reserved
						&dwType,		// address of buffer for value type
						NULL,			// address of data buffer
						&dwBufSize		// address of data buffer size
						);
	RegCloseKey(hSubKey);
	if (lRet != ERROR_SUCCESS)
		return FALSE;
	else
		return TRUE;
}

BOOL IsRegistryKey(HKEY hOpenKey, LPCTSTR szKey)
{
	LONG lRet;

	// Validate Input
	if (!hOpenKey || !szKey || !szKey[0])
		return FALSE;

	HKEY hSubKey;
	lRet = RegOpenKeyEx(
						hOpenKey,		// key handle at root level
						szKey,			// path name of child key
						0,				// reserved
						KEY_READ,		// requesting access
						&hSubKey		// address of key to be returned
						);
	if (lRet != ERROR_SUCCESS)
		return FALSE;
	else
	{
		RegCloseKey(hSubKey);
		return TRUE;
	}
}

BOOL SetRegistryEmptyValue(HKEY hOpenKey, LPCTSTR szKey, LPCTSTR szValue)
{
	// Validate Input
	if (!hOpenKey || !szKey || !szKey[0] || !szValue)
		return FALSE;

	BOOL 	bRetVal = FALSE;
	DWORD	dwDisposition;
	DWORD	dwReserved = 0;
	HKEY  	hTempKey = (HKEY)0;

	// Open key of interest
	// Assume all access is okay and that all keys will be stored to file
	// Utilize the default security attributes
	LONG lRet = RegCreateKeyEx(	hOpenKey, szKey, dwReserved,
								(LPTSTR)0, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, 0,
								&hTempKey, &dwDisposition);
	if (lRet != ERROR_SUCCESS && hOpenKey == HKEY_CLASSES_ROOT)
	{
		lRet = RegCreateKeyEx(	HKEY_CURRENT_USER, _T("SOFTWARE\\Classes\\") + CString(szKey), dwReserved,
								(LPTSTR)0, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, 0,
								&hTempKey, &dwDisposition);
	}
	if (lRet == ERROR_SUCCESS)
	{	
		if(ERROR_SUCCESS == RegSetValueEx(hTempKey, (LPTSTR)szValue,
			dwReserved, REG_NONE, (LPBYTE)NULL, 0))
		{
			bRetVal = TRUE;
		}
	}

	// Close Open Key
	if (hTempKey)
		RegCloseKey(hTempKey);

	return bRetVal;
}

BOOL SetRegistryStringValue(HKEY hOpenKey, LPCTSTR szKey, LPCTSTR szValue, LPCTSTR szData)
{
	// Validate Input
	if (!hOpenKey || !szKey || !szKey[0] || !szValue || !szData)
		return FALSE;

	BOOL 	bRetVal = FALSE;
	DWORD	dwDisposition;
	DWORD	dwReserved = 0;
	HKEY  	hTempKey = (HKEY)0;

	// Length specifier is in bytes, and some TCHAR 
	// are more than 1 byte each
	DWORD dwBufferLength = lstrlen(szData) * sizeof(TCHAR);

	// Open key of interest
	// Assume all access is okay and that all keys will be stored to file
	// Utilize the default security attributes
	LONG lRet = RegCreateKeyEx(	hOpenKey, szKey, dwReserved,
								(LPTSTR)0, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, 0,
								&hTempKey, &dwDisposition);
	if (lRet != ERROR_SUCCESS && hOpenKey == HKEY_CLASSES_ROOT)
	{
		lRet = RegCreateKeyEx(	HKEY_CURRENT_USER, _T("SOFTWARE\\Classes\\") + CString(szKey), dwReserved,
								(LPTSTR)0, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, 0,
								&hTempKey, &dwDisposition);
	}
	if (lRet == ERROR_SUCCESS)
	{
		// dwBufferLength must include size of terminating nul 
		// character when using REG_SZ with RegSetValueEx function
		dwBufferLength += sizeof(TCHAR);
		
		if (ERROR_SUCCESS == RegSetValueEx(hTempKey, (LPTSTR)szValue,
			dwReserved, REG_SZ, (LPBYTE)szData, dwBufferLength))
		{
			bRetVal = TRUE;
		}
	}

	// Close Open Key
	if (hTempKey)
		RegCloseKey(hTempKey);

	return bRetVal;
}

CString GetRegistryStringValue(HKEY hOpenKey, LPCTSTR szKey, LPCTSTR szValue)
{
	LONG lRet;

	// Validate Input
	if (!hOpenKey || !szKey || !szKey[0] || !szValue)
		return _T("");

	HKEY hSubKey;
	lRet = RegOpenKeyEx(
						hOpenKey,		// key handle at root level
						szKey,			// path name of child key
						0,				// reserved
						KEY_READ,		// requesting access
						&hSubKey		// address of key to be returned
						);
	if (lRet != ERROR_SUCCESS)
		return _T("");

	// Get Type and Get Buffer Size
	DWORD dwType, dwBufSize;
	lRet = RegQueryValueEx(
						hSubKey,		// handle to key to query
						szValue,		// address of name of value to query
						0,				// reserved
						&dwType,		// address of buffer for value type
						NULL,			// address of data buffer
						&dwBufSize		// address of data buffer size
						);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hSubKey);
		return _T("");
	}

	if (dwType != REG_SZ)
	{
		RegCloseKey(hSubKey);
		return _T("");
	}

	if (dwBufSize == 0)
	{
		RegCloseKey(hSubKey);
		return _T("");
	}

	// Get The String
	LPBYTE lpBuf = new BYTE[dwBufSize];
	lRet = RegQueryValueEx(
						hSubKey,		// handle to key to query
						szValue,		// address of name of value to query
						0,				// reserved
						&dwType,		// address of buffer for value type
						lpBuf,			// address of data buffer
						&dwBufSize		// address of data buffer size
						);
	if (lRet != ERROR_SUCCESS)
	{
		delete [] lpBuf;
		RegCloseKey(hSubKey);
		return _T("");
	}

	// Close Open Key
	RegCloseKey(hSubKey);

	CString Str((TCHAR*)lpBuf);
	delete [] lpBuf;
	return Str;
}

BOOL DeleteRegistryValue(HKEY hOpenKey, LPCTSTR szKey, LPCTSTR szValue)
{
	// Validate Input
	if (!hOpenKey || !szKey || !szKey[0] || !szValue)
		return FALSE;

	HKEY hSubKey;
	LONG lRet = RegOpenKeyEx(
						hOpenKey,				// key handle at root level
						szKey,					// path name of child key
						0,						// reserved
						KEY_SET_VALUE,			// requesting access
						&hSubKey				// address of key to be returned
						);
	if (lRet != ERROR_SUCCESS)
	{
		if (hOpenKey == HKEY_CLASSES_ROOT)
		{
			lRet = RegOpenKeyEx(
							HKEY_CURRENT_USER,		// key handle at root level
							_T("SOFTWARE\\Classes\\") + CString(szKey), // path name of child key
							0,						// reserved
							KEY_SET_VALUE,			// requesting access
							&hSubKey				// address of key to be returned
							);
		}
		if (lRet != ERROR_SUCCESS)
			return FALSE;
	}

	lRet =  RegDeleteValue(
						hSubKey,				// handle to key
						szValue					// address of value name
						);
	RegCloseKey(hSubKey);
	if (lRet != ERROR_SUCCESS)
		return FALSE;
	else
		return TRUE;
}

// Delete the entire key. NOTE: Windows NT cannot delete
// subkeys of the key, so we have to handle it ourself.
BOOL DeleteRegistryKey(HKEY hOpenKey, LPCTSTR szKey)
{
	// Validate Input
	if (!hOpenKey || !szKey || !szKey[0])
		return FALSE;

	CString sKey(szKey);
	HKEY hSubKey;
	LONG lRet = RegOpenKeyEx(
						hOpenKey,				// key handle at root level
						sKey,					// path name of child key
						0,						// reserved
						KEY_WRITE | KEY_READ,	// requesting access
						&hSubKey				// address of key to be returned
						);
	if (lRet != ERROR_SUCCESS)
	{
		if (hOpenKey == HKEY_CLASSES_ROOT)
		{
			hOpenKey = HKEY_CURRENT_USER;
			sKey = _T("SOFTWARE\\Classes\\") + sKey;
			lRet = RegOpenKeyEx(
							hOpenKey,				// key handle at root level
							sKey,					// path name of child key
							0,						// reserved
							KEY_WRITE | KEY_READ,	// requesting access
							&hSubKey				// address of key to be returned
							);
		}
		if (lRet != ERROR_SUCCESS)
			return FALSE;
	}

	// Loop until all subkeys are removed
	DWORD dwSubKeyCnt = 0;
	do
	{
		// Do not use the number of subkeys for the enumeration,
		// because it changes, if a subkey is deleted !!!

		// Get info about this subkey
		DWORD dwMaxSubKey;
		lRet =	RegQueryInfoKey(
						hSubKey,
						0,					// buffer for class name
						0,					// length of class name string
						0,					// reserved
						&dwSubKeyCnt,		// # of subkeys
						&dwMaxSubKey,		// length of longest subkey
						0,					// length of longest class name string
						0,					// # of values
						0,					// length of longest value name
						0,					// length of longest value data
						0,					// security descriptor
						0					// last write time
						);
		if (lRet != ERROR_SUCCESS)
		{
			RegCloseKey(hSubKey);
			return FALSE;
		}

		if (dwSubKeyCnt > 0)
		{
			// Retrieve the first subkey and call DeleteRegistryKey() recursively
			LPTSTR pszKeyName = new TCHAR [dwMaxSubKey + 1];
			DWORD dwKeyNameLen = dwMaxSubKey;
			lRet = RegEnumKey(
						hSubKey,
						0,				// index
						pszKeyName,		// address of buffer for key name string
						dwKeyNameLen+1	// max. length of key name string
					);
			if (lRet != ERROR_SUCCESS)
			{
				delete [] pszKeyName;
				RegCloseKey(hSubKey);
				return FALSE;
			}
			if(!DeleteRegistryKey(hSubKey, pszKeyName))
			{
				delete [] pszKeyName;
				RegCloseKey(hSubKey);
				return FALSE;
			}
			delete [] pszKeyName;
		}
	}
	while (dwSubKeyCnt > 0);
	
	// Delete It!
	RegCloseKey(hSubKey);
	return (RegDeleteKey(hOpenKey, sKey) == ERROR_SUCCESS);
}

void ShowError(DWORD dwErrorCode, BOOL bShowMessageBoxOnError, CString sHeader/*=_T("")*/, CString sFooter/*=_T("")*/)
{
	CString sText(_T(""));
	LPVOID lpMsgBuf = NULL;

	if (FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		dwErrorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR)&lpMsgBuf,
		0,
		NULL) && lpMsgBuf)
	{
		// Replace eventual CRs in the middle of the string with a space
		int i = 0;
		while ((((LPTSTR)lpMsgBuf)[i] != '\0') && (i < 1024)) // i < 1024 security!
		{
			if (((LPTSTR)lpMsgBuf)[i] == '\r') 
				((LPTSTR)lpMsgBuf)[i] = ' ';
			i++;
		}

		// Remove the terminating CR + LF
		i = 0;
		while ((((LPTSTR)lpMsgBuf)[i++] != '\0') && (i < 1024)); // i < 1024 security!
		((LPTSTR)lpMsgBuf)[i-3] = '\0';

		sText = (LPTSTR)lpMsgBuf;

		// Free
		LocalFree(lpMsgBuf);
	}

	// Error Codes For Internet Functions
	if (dwErrorCode == ERROR_INTERNET_EXTENDED_ERROR)
	{
		DWORD dwInetError;
		DWORD dwSize = 0;
		InternetGetLastResponseInfo(&dwInetError, NULL, &dwSize);
		TCHAR* pszResponse = new TCHAR[dwSize+1];
		InternetGetLastResponseInfo(&dwInetError, pszResponse, &dwSize);
		pszResponse[dwSize] = _T('\0');
		if (sText != _T(""))
			sText = sText + _T('\n') + pszResponse;
		else
			sText = pszResponse;
		delete [] pszResponse;
	}
	else if (dwErrorCode == ERROR_INTERNET_NAME_NOT_RESOLVED)
	{
		CString sResponse(ML_STRING(1778, "The server name could not be resolved."));
		if (sText != _T(""))
			sText = sText + _T('\n') + sResponse;
		else
			sText = sResponse;
	}
	else if (dwErrorCode == ERROR_INTERNET_CANNOT_CONNECT)
	{
		CString sResponse(ML_STRING(1779, "The attempt to connect to the server failed."));
		if (sText != _T(""))
			sText = sText + _T('\n') + sResponse;
		else
			sText = sResponse;
	}
	else if (	dwErrorCode == ERROR_INTERNET_INCORRECT_PASSWORD	||
				dwErrorCode == ERROR_INTERNET_INCORRECT_USER_NAME	||
				dwErrorCode == ERROR_INTERNET_LOGIN_FAILURE)
	{
		CString sResponse(ML_STRING(1780, "The request to connect and log on to\nthe server could not be completed\nbecause the supplied user name and / or\npassword are incorrect."));
		if (sText != _T(""))
			sText = sText + _T('\n') + sResponse;
		else
			sText = sResponse;
	}
	else if (dwErrorCode == ERROR_INTERNET_INVALID_PROXY_REQUEST)
	{
		CString sResponse(ML_STRING(1781, "The request to the proxy was invalid."));
		if (sText != _T(""))
			sText = sText + _T('\n') + sResponse;
		else
			sText = sResponse;
	}
	else if (dwErrorCode == ERROR_FTP_NO_PASSIVE_MODE)
	{
		CString sResponse(ML_STRING(1782, "Passive mode is not available on the server."));
		if (sText != _T(""))
			sText = sText + _T('\n') + sResponse;
		else
			sText = sResponse;
	}
	else if (dwErrorCode == ERROR_FTP_DROPPED)
	{
		CString sResponse(ML_STRING(1783, "FTP connection has been dropped."));
		if (sText != _T(""))
			sText = sText + _T('\n') + sResponse;
		else
			sText = sResponse;
	}

	// Avoid empty errors!
	if (sText == _T(""))
		sText.Format(ML_STRING(1784, "Error with code %u."), dwErrorCode);

	// Show Error
	CString sTraceMsg = sHeader + sText + sFooter;
	if (sTraceMsg[sTraceMsg.GetLength() - 1] != _T('\n')) // This is ok because sTraceMsg is never empty
		sTraceMsg += _T('\n');
	TRACE(sTraceMsg);
	if (bShowMessageBoxOnError)
		AfxMessageBox(sHeader + sText + sFooter, MB_ICONSTOP);
}

void ShowLastError(BOOL bShowMessageBoxOnError, CString sHeader/*=_T("")*/, CString sFooter/*=_T("")*/)
{
	ShowError(GetLastError(), bShowMessageBoxOnError, sHeader, sFooter);
}

static void CALLBACK timerCallbackSetEvent(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	SetEvent((HANDLE)dwUser);
}

static void CALLBACK timerCallbackPulseEvent(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	PulseEvent((HANDLE)dwUser);
}

// Win95 Compatible timeSetEvent Function.
// Under Win95 timer events are not working!
#ifndef TIME_KILL_SYNCHRONOUS
#define TIME_KILL_SYNCHRONOUS   0x0100  /* This flag prevents the event from occurring */
                                        /* after the user calls timeKillEvent() to */
                                        /* destroy it. Introduced with Windows XP */
#endif
MMRESULT timeSetEventCompatible(UINT uDelay,                
								UINT uResolution,           
								LPTIMECALLBACK lpTimeProc,  
								DWORD dwUser,               
								UINT fuEvent)
{
	UINT uiOneShot_Periodic_Flag = fuEvent & 0x1;

	if (g_bWin95)
	{
		if ((fuEvent & TIME_CALLBACK_EVENT_SET) == TIME_CALLBACK_EVENT_SET)
			return timeSetEvent(	uDelay,
									uResolution,
									(LPTIMECALLBACK)timerCallbackSetEvent,
									(DWORD)lpTimeProc,
									uiOneShot_Periodic_Flag | TIME_CALLBACK_FUNCTION);
		else if ((fuEvent & TIME_CALLBACK_EVENT_PULSE) == TIME_CALLBACK_EVENT_PULSE)
			return timeSetEvent(	uDelay,
									uResolution,
									(LPTIMECALLBACK)timerCallbackPulseEvent,
									(DWORD)lpTimeProc,
									uiOneShot_Periodic_Flag | TIME_CALLBACK_FUNCTION);
		else
			return timeSetEvent(	uDelay,
									uResolution,
									lpTimeProc,
									dwUser,
									fuEvent);
	}
	else if (g_bWinXPOrHigher)
	{
		return timeSetEvent(	uDelay,
								uResolution,
								lpTimeProc,
								dwUser,
								fuEvent | TIME_KILL_SYNCHRONOUS);
	}
	else
	{
		return timeSetEvent(	uDelay,
								uResolution,
								lpTimeProc,
								dwUser,
								fuEvent);
	}
}

// Plays a specified file using MCI_OPEN and MCI_PLAY. 
// Returns when playback begins.
// Returns the device id on success,
// on error it returns -1.
int MCIPlayFile(HWND hWndNotify, BOOL bStartPlaying, LPCTSTR lpszFileName)
{
    MCI_OPEN_PARMS mciOpenParms;
	ZeroMemory(&mciOpenParms, sizeof(MCI_OPEN_PARMS));

    // Open the device by specifying the filename.
    // MCI will choose a device capable of playing the specified file.
    mciOpenParms.lpstrElementName = lpszFileName;
    if (mciSendCommand(0,
						MCI_OPEN,
						MCI_OPEN_ELEMENT,
						(DWORD)(LPVOID)&mciOpenParms))
    {
        // Failed to open device.
        return -1;
    }

    // Play?
	if (bStartPlaying)
	{
		if (MCIPlayDevice(hWndNotify, (int)mciOpenParms.wDeviceID))
		{
			MCICloseDevice(hWndNotify, (int)mciOpenParms.wDeviceID);
			return -1;
		}
	}

    return (int)mciOpenParms.wDeviceID;
}

// Begin playback. The window procedure function for the parent 
// window will be notified with an MM_MCINOTIFY message when 
// playback is complete. At this time, the window procedure closes 
// the device.
MCIERROR MCIPlayDevice(HWND hWndNotify, int nDeviceID)
{
	if (nDeviceID >= 0)
	{
		MCI_PLAY_PARMS mciPlayParms;
		ZeroMemory(&mciPlayParms, sizeof(MCI_PLAY_PARMS));
		mciPlayParms.dwCallback = (DWORD)hWndNotify;
		return mciSendCommand((MCIDEVICEID)nDeviceID,
								MCI_PLAY, 
								hWndNotify ? MCI_NOTIFY : 0,
								(DWORD)(LPVOID)&mciPlayParms);
	}
	else
		return MCIERR_INVALID_DEVICE_ID;
}

MCIERROR MCIPauseDevice(HWND hWndNotify, int nDeviceID)
{
	if (nDeviceID >= 0)
	{
		MCI_GENERIC_PARMS mciGenericParms;
		mciGenericParms.dwCallback = (DWORD)hWndNotify;
		return mciSendCommand((MCIDEVICEID)nDeviceID,
								MCI_PAUSE, 
								hWndNotify ? MCI_NOTIFY : 0,
								(DWORD)(LPVOID)&mciGenericParms);
	}
	else
		return MCIERR_INVALID_DEVICE_ID;
}

MCIERROR MCICloseDevice(HWND hWndNotify, int nDeviceID)
{
	if (nDeviceID >= 0)
	{
		MCI_GENERIC_PARMS mciGenericParms;
		mciGenericParms.dwCallback = (DWORD)hWndNotify;
		return mciSendCommand((MCIDEVICEID)nDeviceID,
								MCI_CLOSE, 
								hWndNotify ? MCI_NOTIFY : 0,
								(DWORD)(LPVOID)&mciGenericParms);
	}
	else
		return MCIERR_INVALID_DEVICE_ID;
}

static BOOL HasCpuId()
{
    __try
	{
        _asm
		{
            xor eax, eax
            cpuid
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
	{
        return FALSE;
    }
    return TRUE;
}

#define _MMX_FEATURE_BIT	0x00800000
#define _SSE_FEATURE_BIT	0x02000000
#define _SSE2_FEATURE_BIT	0x04000000
#define _3DNOW_FEATURE_BIT	0x80000000

static int GetCpuInstr()
{
    DWORD dwStandard = 0;
    DWORD dwFeature = 0;
    DWORD dwMax = 0;
    DWORD dwExt = 0;
    int feature = 0;
    union
	{
        char cBuf[12+1];
        struct
		{
            DWORD dw0;
            DWORD dw1;
            DWORD dw2;
        } s;
    } Ident;

    if (!HasCpuId())
        return 0;

    _asm
	{
        push ebx
        push ecx
        push edx

        // get the vendor string
        xor eax, eax
        cpuid
        mov dwMax, eax
        mov Ident.s.dw0, ebx
        mov Ident.s.dw1, edx
        mov Ident.s.dw2, ecx

        // get the Standard bits
        mov eax, 1
        cpuid
        mov dwStandard, eax
        mov dwFeature, edx

        // get AMD-specials
        mov eax, 80000000h
        cpuid
        cmp eax, 80000000h
        jc notamd
        mov eax, 80000001h
        cpuid
        mov dwExt, edx

notamd:
        pop ecx
        pop ebx
        pop edx
    }

    if (dwFeature & _MMX_FEATURE_BIT)
        feature |= CPU_FEATURE_MMX;
    if (dwExt & _3DNOW_FEATURE_BIT)
        feature |= CPU_FEATURE_3DNOW;
    if (dwFeature & _SSE_FEATURE_BIT)
        feature |= CPU_FEATURE_SSE;
    if (dwFeature & _SSE2_FEATURE_BIT)
        feature |= CPU_FEATURE_SSE2;

    return feature;
}

int GetTotPhysMemMB()
{
	typedef BOOL (WINAPI * FPGLOBALMEMORYSTATUSEX)(LPMEMORYSTATUSEX lpBuffer);
	HINSTANCE h = LoadLibrary(_T("kernel32.dll"));
	if (!h)
		return 0;
	FPGLOBALMEMORYSTATUSEX fpGlobalMemoryStatusEx = (FPGLOBALMEMORYSTATUSEX)GetProcAddress(h, "GlobalMemoryStatusEx");
	if (fpGlobalMemoryStatusEx)
	{
		MEMORYSTATUSEX MemoryStatusEx;
		MemoryStatusEx.dwLength = sizeof(MemoryStatusEx);
		fpGlobalMemoryStatusEx(&MemoryStatusEx);
		FreeLibrary(h);
		return (int)(MemoryStatusEx.ullTotalPhys >> 20);
	}
	else
	{
		MEMORYSTATUS MemoryStatus;
		GlobalMemoryStatus(&MemoryStatus);
		FreeLibrary(h);
		return (int)(MemoryStatus.dwTotalPhys >> 20);
	}
}

void GetMemoryStats(int* pRegions/*=NULL*/,
					int* pFreeMB/*=NULL*/,
					int* pReservedMB/*=NULL*/,
					int* pCommittedMB/*=NULL*/,
					double* pFragmentation/*=NULL*/)
{
	MEMORY_BASIC_INFORMATION memory_info;
	memset(&memory_info, 0, sizeof (memory_info));
	DWORD region = 0;
	DWORD sum_free = 0, max_free = 0;
	DWORD sum_reserve = 0, max_reserve = 0;
	DWORD sum_commit = 0, max_commit = 0;
	while (::VirtualQuery(memory_info.BaseAddress, &memory_info, sizeof(memory_info)))
	{
		++region;
		switch (memory_info.State)
		{
			case MEM_FREE :
				sum_free += memory_info.RegionSize;
				max_free = MAX(max_free, memory_info.RegionSize);
				break;
			case MEM_RESERVE :
				sum_reserve += memory_info.RegionSize;
				max_reserve = MAX(max_reserve, memory_info.RegionSize);
				break;
			case MEM_COMMIT :
				sum_commit += memory_info.RegionSize;
				max_commit = MAX(max_commit, memory_info.RegionSize);
				break;
			default :
				ASSERT(FALSE);
				break;
		}
		memory_info.BaseAddress = (char*)memory_info.BaseAddress + memory_info.RegionSize;
	}

	// Calc.
	double dFragmentation = 0.0;
	if ((sum_free + sum_reserve + sum_commit) > 0)
		dFragmentation = 100.0 * (1.0 - (double)(max_free + max_reserve + max_commit) /
										(double)(sum_free + sum_reserve + sum_commit));
	sum_free >>= 20;
	sum_reserve >>= 20;
	sum_commit >>= 20;

	// Return params
	if (pRegions) *pRegions = region;
	if (pFreeMB) *pFreeMB = sum_free;
	if (pReservedMB) *pReservedMB = sum_reserve;
	if (pCommittedMB) *pCommittedMB = sum_commit;
	if (pFragmentation) *pFragmentation = dFragmentation;

	// Trace
	CString sMsg;
	sMsg.Format(_T("Regions=%u, Free=%uMB, Reserved=%uMB, Committed=%uMB, Fragmentation=%0.1f%%%%\n"),
					region, sum_free, sum_reserve, sum_commit, dFragmentation);
	TRACE(sMsg);
}

typedef BOOL (WINAPI * FPGETDISKFREESPACEEX)(LPCTSTR lpDirectoryName,
												PULARGE_INTEGER lpFreeBytesAvailableToCaller,
												PULARGE_INTEGER lpTotalNumberOfBytes,
												PULARGE_INTEGER lpTotalNumberOfFreeBytes);

ULONGLONG GetDiskSize(LPCTSTR lpszPath)
{
	HINSTANCE h = LoadLibrary(_T("kernel32.dll"));
	if (!h)
		return 0;
#ifdef _UNICODE
	FPGETDISKFREESPACEEX fpGetDiskFreeSpaceEx = (FPGETDISKFREESPACEEX)GetProcAddress(h, "GetDiskFreeSpaceExW");
#else
	FPGETDISKFREESPACEEX fpGetDiskFreeSpaceEx = (FPGETDISKFREESPACEEX)GetProcAddress(h, "GetDiskFreeSpaceExA");
#endif
	ULARGE_INTEGER FreeBytesAvailableToCaller;	// receives the number of bytes on
												// disk available to the caller
	ULARGE_INTEGER TotalNumberOfBytes;			// receives the number of bytes on disk
	ULARGE_INTEGER TotalNumberOfFreeBytes;		// receives the free bytes on disk
	if (fpGetDiskFreeSpaceEx)
	{
		if (!fpGetDiskFreeSpaceEx(
				GetDriveName(lpszPath) + _T("\\"),
				&FreeBytesAvailableToCaller,
				&TotalNumberOfBytes,
				&TotalNumberOfFreeBytes))
		{
			FreeLibrary(h);
			return 0;
		}
	}
	else
	{
		FreeLibrary(h);
		return 0;
	}

	FreeLibrary(h);

	return TotalNumberOfBytes.QuadPart;
}

ULONGLONG GetDiskSpace(LPCTSTR lpszPath)
{
	HINSTANCE h = LoadLibrary(_T("kernel32.dll"));
	if (!h)
		return 0;
#ifdef _UNICODE
	FPGETDISKFREESPACEEX fpGetDiskFreeSpaceEx = (FPGETDISKFREESPACEEX)GetProcAddress(h, "GetDiskFreeSpaceExW");
#else
	FPGETDISKFREESPACEEX fpGetDiskFreeSpaceEx = (FPGETDISKFREESPACEEX)GetProcAddress(h, "GetDiskFreeSpaceExA");
#endif
	ULARGE_INTEGER FreeBytesAvailableToCaller;	// receives the number of bytes on
												// disk available to the caller
	ULARGE_INTEGER TotalNumberOfBytes;			// receives the number of bytes on disk
	ULARGE_INTEGER TotalNumberOfFreeBytes;		// receives the free bytes on disk
	if (fpGetDiskFreeSpaceEx)
	{
		if (!fpGetDiskFreeSpaceEx(
				GetDriveName(lpszPath) + _T("\\"),
				&FreeBytesAvailableToCaller,
				&TotalNumberOfBytes,
				&TotalNumberOfFreeBytes))
		{
			FreeLibrary(h);
			return 0;
		}
	}
	else
	{
		FreeLibrary(h);
		return 0;
	}

	FreeLibrary(h);

	return FreeBytesAvailableToCaller.QuadPart;
}

CString FileNameToMime(LPCTSTR lpszFileName)
{
	CString sExt = GetFileExt(lpszFileName);

	if (sExt == _T(".jpeg")		||
		sExt == _T(".jpg")		||
		sExt == _T(".jpe")		||
		sExt == _T(".thm"))
		return _T("image/jpeg");
	else if (	sExt == _T(".tif")	||
				sExt == _T(".jfx")	||
				sExt == _T(".tiff"))
		return _T("image/tiff");
	else if (	sExt == _T(".bmp")	||
				sExt == _T(".dib"))
		return _T("image/bmp");
	else if (sExt == _T(".gif"))
		return _T("image/gif");
	else if (sExt == _T(".png"))
		return _T("image/png");
	else if (	sExt == _T(".avi")	||
				sExt == _T(".divx"))
		return _T("video/x-msvideo");
	else
		return _T(""); 
}

void MakeLineBreakCR(CString& s)
{
	s.Replace(_T("\r\n"), _T("\r"));
	s.Replace(_T("\n"), _T("\r"));
}

void MakeLineBreakLF(CString& s)
{
	s.Replace(_T("\r\n"), _T("\n"));
	s.Replace(_T("\r"), _T("\n"));
}

void MakeLineBreakCRLF(CString& s)
{
	s.Replace(_T("\r\n"), _T("\n"));
	s.Replace(_T("\r"), _T("\n"));
	s.Replace(_T("\n"), _T("\r\n"));
}

BOOL IsANSIConvertible(const CString& s)
{
	// Empty String is Convertible!
	if (s.IsEmpty())
		return TRUE;

	// Convert CString to Unicode
	USES_CONVERSION;
	LPCWSTR psuBuff = T2CW(s);
	int nUtf16Len = (int)wcslen(psuBuff);

	// Convert UTF-16 to ANSI
	BOOL bUsedDefaultChar = FALSE;
	int res = WideCharToMultiByte(	CP_ACP,					// ANSI Code Page
									WC_NO_BEST_FIT_CHARS,	// Not supported by Win95 and NT4
									psuBuff,				// wide-character string
									nUtf16Len,				// number of chars in string.
									NULL,					// buffer for new string
									0,						// size of buffer
									NULL,					// default for unmappable chars
									&bUsedDefaultChar);		// set when default char used
	if (res <= 0 && GetLastError() == ERROR_INVALID_FLAGS)
	{
		res = WideCharToMultiByte(	CP_ACP,					// ANSI Code Page
									0, 
									psuBuff,				// wide-character string
									nUtf16Len,				// number of chars in string.
									NULL,					// buffer for new string
									0,						// size of buffer
									NULL,					// default for unmappable chars
									&bUsedDefaultChar);		// set when default char used
	}
	if (res <= 0)
		return FALSE;
	else
		return !bUsedDefaultChar;
}

int ToANSI(const CString& s, LPSTR* ppAnsi, BOOL* pbUsedDefaultChar/*=NULL*/)
{
	// Check
	if (!ppAnsi)
		return 0;

	// Convert CString to Unicode
	USES_CONVERSION;
	LPCWSTR psuBuff = T2CW(s);
	int nUtf16Len = (int)wcslen(psuBuff);

	// Allocate enough buffer
	*ppAnsi = new char[nUtf16Len+1];
	if (*ppAnsi == NULL)
		return 0;

	// Convert UTF-16 to ANSI
	int res = WideCharToMultiByte(	CP_ACP,					// ANSI Code Page
									WC_NO_BEST_FIT_CHARS,	// Not supported by Win95 and NT4
									psuBuff,				// wide-character string
									nUtf16Len,				// number of chars in string.
									*ppAnsi,				// buffer for new string
									nUtf16Len+1,			// size of buffer
									NULL,					// default for unmappable chars
									pbUsedDefaultChar);		// set when default char used
	if (res <= 0 && GetLastError() == ERROR_INVALID_FLAGS)
	{
		res = WideCharToMultiByte(	CP_ACP,					// ANSI Code Page
									0, 
									psuBuff,				// wide-character string
									nUtf16Len,				// number of chars in string.
									*ppAnsi,				// buffer for new string
									nUtf16Len+1,			// size of buffer
									NULL,					// default for unmappable chars
									pbUsedDefaultChar);		// set when default char used
	}
	if (res <= 0)
	{
		delete [] *ppAnsi;
		*ppAnsi = NULL;
		return 0;
	}
	else
	{
		(*ppAnsi)[MIN(res, nUtf16Len)] = '\0';
		return res;
	}
}

CString UrlEncode(const CString& s, BOOL bEncodeReserved)
{
	// Empty String is already ok!
	if (s.IsEmpty())
		return _T("");

	LPSTR c = NULL;
	if (::ToANSI(s, &c) <= 0 || !c)
	{
		if (c)
			delete [] c;
		return _T("");
	}
	CString sHex;
    CString sEscaped(_T(""));
	if (bEncodeReserved)
	{
		for (int i = 0 ; i < (int)strlen(c) ; i++)
		{
			if ( (48 <= c[i] && c[i] <= 57) ||	// 0-9
				 (65 <= c[i] && c[i] <= 90) ||	// ABC...XYZ
				 (97 <= c[i] && c[i] <= 122)||	// abc...xyz
				 (c[i]== '-' || c[i] == '_' ||
				 c[i] == '.' || c[i] == '!' ||
				 c[i] == '*' || c[i] == '\''||
				 c[i] == '(' || c[i]== ')'))
				sEscaped += CString(c[i]);
			else
			{
				sEscaped += _T("%");
				sHex.Format(_T("%02x"), ((int)c[i]) & 0xFF);
				sEscaped += sHex;
			}
		}
	}
	else
	{
		for (int i = 0 ; i < (int)strlen(c) ; i++)
		{
			if ( (48 <= c[i] && c[i] <= 57) || // 0-9
				 (65 <= c[i] && c[i] <= 90) || // ABC...XYZ
				 (97 <= c[i] && c[i] <= 122)|| // abc...xyz
				 (c[i]== '-' || c[i] == '_' ||
				 c[i] == '.' || c[i] == '!' ||
				 c[i] == '*' || c[i] == '\''||
				 c[i] == '(' || c[i]== ')'	||
				 c[i] == '$' || c[i] == '&' || // reserved chars
				 c[i] == '+' || c[i] == ',' || // reserved chars
				 c[i] == '/' || c[i] == ':' || // reserved chars
				 c[i] == ';' || c[i] == '=' || // reserved chars
				 c[i] == '?' || c[i] == '@'))  // reserved chars
				sEscaped += CString(c[i]);
			else
			{
				sEscaped += _T("%");
				sHex.Format(_T("%02x"), ((int)c[i]) & 0xFF);
				sEscaped += sHex;
			}
		}
	}
	delete [] c;
	return sEscaped;
}

CString FromUTF8(const unsigned char* pUtf8, int nUtf8Len)
{
	// Check
	if (!pUtf8 || nUtf8Len <= 0)
		return _T("");

	// Allocate enough buffer
	UTF16* pUtf16 = new UTF16[nUtf8Len+1];
	if (pUtf16 == NULL)
		return _T("");

	// Convert UTF-8 to UTF-16
	UTF8* sourceStart = (UTF8*)pUtf8; 
	UTF8* sourceEnd = sourceStart + nUtf8Len;
	UTF16* targetStart = (UTF16*)(pUtf16);
	UTF16* targetEnd = targetStart + nUtf8Len+1;
	ConversionResult res = ConvertUTF8toUTF16(	(const UTF8**)&sourceStart,
												(const UTF8*)sourceEnd, 
												&targetStart,
												targetEnd,
												lenientConversion);
	if (res != conversionOK)
	{
		delete [] pUtf16;
		pUtf16 = NULL;
		return _T("");
	}
	else
	{
		int nUtf16Len = targetStart - pUtf16;
		ASSERT(nUtf16Len+1 <= nUtf8Len+1);
		pUtf16[nUtf16Len] = L'\0';
		CString s((LPCWSTR)pUtf16);
		delete [] pUtf16;
		return s;
	}
}

int ToUTF8(const CString& s, LPBYTE* ppUtf8)
{
	// Check
	if (!ppUtf8)
		return 0;

	// Convert CString to Unicode
	USES_CONVERSION;
	LPCWSTR psuBuff = T2CW(s);
	int nUtf16Len = (int)wcslen(psuBuff);

	// Allocate enough buffer
	*ppUtf8 = new BYTE[4*nUtf16Len+1];
	if (*ppUtf8 == NULL)
		return 0;

	// Convert UTF-16 to UTF-8
	UTF16* sourceStart = (UTF16*)psuBuff; 
	UTF16* sourceEnd = sourceStart + nUtf16Len;
	UTF8* targetStart = (UTF8*)(*ppUtf8);
	UTF8* targetEnd = targetStart + 4*nUtf16Len+1;
	ConversionResult res = ConvertUTF16toUTF8(	(const UTF16**)&sourceStart,
												(const UTF16*)sourceEnd, 
												&targetStart,
												targetEnd,
												lenientConversion);
	if (res != conversionOK)
	{
		delete [] *ppUtf8;
		*ppUtf8 = NULL;
		return 0;
	}
	else
	{
		int nUtf8Len = targetStart - *ppUtf8;
		ASSERT(nUtf8Len+1 <= 4*nUtf16Len+1);
		(*ppUtf8)[nUtf8Len] = '\0';
		return nUtf8Len;
	}
}

CString GetUuidString()
{
	typedef RPC_STATUS (RPC_ENTRY * FPUUIDCREATE)(UUID*  Uuid);
	typedef RPC_STATUS (RPC_ENTRY * FPUUIDTOSTRINGA)(UUID*  Uuid, unsigned char** StringUuid);
	typedef RPC_STATUS (RPC_ENTRY * FPRPCSTRINGFREEA)(unsigned char** String);
	typedef RPC_STATUS (RPC_ENTRY * FPUUIDTOSTRINGW)(UUID*  Uuid, unsigned short** StringUuid);
	typedef RPC_STATUS (RPC_ENTRY * FPRPCSTRINGFREEW)(unsigned short** String);
	CString sUUID(_T(""));
	HINSTANCE h = LoadLibrary(_T("Rpcrt4.dll"));
	if (!h)
		return _T("");
	FPUUIDCREATE fpUuidCreate = (FPUUIDCREATE)GetProcAddress(h, "UuidCreate");
#ifdef _UNICODE
	FPUUIDTOSTRINGW fpUuidToString = (FPUUIDTOSTRINGW)GetProcAddress(h, "UuidToStringW");
	FPRPCSTRINGFREEW fpRpcStringFree = (FPRPCSTRINGFREEW)GetProcAddress(h, "RpcStringFreeW");
#else
	FPUUIDTOSTRINGA fpUuidToString = (FPUUIDTOSTRINGA)GetProcAddress(h, "UuidToStringA");
	FPRPCSTRINGFREEA fpRpcStringFree = (FPRPCSTRINGFREEA)GetProcAddress(h, "RpcStringFreeA");
#endif
	if (fpUuidCreate && fpUuidToString && fpRpcStringFree)
	{
#ifdef _UNICODE
		unsigned short* sTemp;
#else
		unsigned char* sTemp;
#endif
		UUID* pUUID = new UUID;
		if (pUUID)
		{
			HRESULT hr = fpUuidCreate(pUUID);
			if (hr == (HRESULT)RPC_S_OK || hr == (HRESULT)RPC_S_UUID_LOCAL_ONLY)
			{
				hr = fpUuidToString(pUUID, &sTemp);
				if (hr == RPC_S_OK)
				{
					sUUID = CString((LPCTSTR)sTemp);
					fpRpcStringFree(&sTemp);
				}
			}
			delete pUUID;
		}
	}

	FreeLibrary(h);

	return sUUID;
}

// The PI Constant
#ifndef PI
#define PI  3.14159265358979323846
#endif
int MercatorLatitude(double lati, int maxTile)
{
	double maxlat = PI;
	double lat = lati;
	if (lat > 90) lat = lat - 180;
	if (lat < -90) lat = lat + 180;

	// Conversion degree => radians
	double phi = PI * lat / 180.0;

	// Mercator formula
	double res = 0.5 * log((1.0 + sin(phi)) / (1.0 - sin(phi)));
	
	// Re-center
	int result = (int)(((1.0 - res / maxlat) / 2.0) * maxTile);
	
	return result;
}

// bSatView: Satellite View if set, otherwise Map View
// latitude: -90 (S) .. +90 (N)
// longitude: -180 (W)  .. +180 (E)
// zoom: 17 (fully zoomed out) to 0 (maximum definition)
// Note: Some places have a limited zoom,
//       Sat. Views usually can be zoomed more!
//
// Example Locarno: 46.1682418767, 8.79695522996
CString LatLongToGoogleMapUrl(BOOL bSatView, double latitude, double longitude, int zoom)
{
	// Correct the longitude to go from 0 to 360
	double lon = 180.0 + longitude;

	int maxTile = 1 << (17 - zoom);

	// Find tile size from zoom level
	double longTileSize = 360.0 / (double)maxTile;

	// Find the tile coordinates
	int tilex = (int)(lon/longTileSize);
	int tiley = (int)MercatorLatitude(latitude, maxTile); 

	CString sUrl;
	if (!bSatView)
	{
		// There are other 3 servers: mt2, mt3 and mt4
		sUrl.Format(_T("http://mt1.google.com/mt?n=404&v=w2.12&x=%i&y=%i&zoom=%i"),
					tilex,
					tiley,
					zoom);

		return sUrl;
	}
	else
	{
		CString location = _T("t");
		int CompX = maxTile / 2;
		int CompY = maxTile / 2;

		// Google use a latitude divided by 2;
		double halflat = latitude / 2;
		for (int i = 0 ; i < 17 - zoom ; i++)
		{	
			// Upper part (q or r)
			if (tiley < CompY)
			{
				if (tilex < CompX)
				{ /*q*/
					location+= "q";
				}
				else
				{/*r*/
					location+= "r";
					tilex -= CompX; 
				}
				CompX = CompX / 2;
				CompY = CompY / 2;
			}
			// Lower part (t or s)
			else
			{
				if (tilex < CompX)
				{ /*t*/
					location+= "t";
				}
				else
				{/*s*/
					location+= "s";
					tilex -= CompX;
				}
				CompX = CompX / 2;
				tiley -= CompY; 
				CompY = CompY / 2;
			}
		}

		// There are other 3 servers: kh1, kh2 and kh3
		sUrl.Format(_T("http://kh0.google.com/kh?n=404&v=8&t=%s"), location);

		return sUrl;
	}
}



// Use SecureZeroMemory(PVOID ptr, SIZE_T cnt) to zero
// the given buffer when fineshed using it
#ifndef HCRYPTPROV
typedef ULONG_PTR HCRYPTPROV;    // WinCrypt.h, line 249
#endif
#ifndef CRYPT_VERIFYCONTEXT
#define CRYPT_VERIFYCONTEXT     0xF0000000
#endif
#ifndef CRYPT_SILENT
#define CRYPT_SILENT            0x00000040
#endif
#ifndef PROV_RSA_FULL
#define PROV_RSA_FULL			1
#endif
typedef BOOL (WINAPI * fnCryptGenRandom) (HCRYPTPROV, DWORD, BYTE*); 
typedef BOOL (WINAPI * fnCryptAcquireContext) (HCRYPTPROV*, LPCTSTR, LPCTSTR, DWORD, DWORD);
typedef BOOL (WINAPI * fnCryptReleaseContext) (HCRYPTPROV, DWORD);
BOOL MakeRandomBuffer(PVOID RandomBuffer, ULONG RandomBufferLength)
{
	// Check
	if (!RandomBuffer || RandomBufferLength == 0)
		return FALSE;

	BOOL res;
	HMODULE hLib = LoadLibrary(_T("ADVAPI32.DLL"));
	if (hLib)
	{
		BOOLEAN (APIENTRY *pfn)(void*, ULONG) = (BOOLEAN (APIENTRY *)(void*,ULONG))GetProcAddress(hLib, "SystemFunction036");
		if (pfn)
		{
			res = (BOOL)(pfn(RandomBuffer, RandomBufferLength));		
			FreeLibrary(hLib);
			return res;
		}
		else
		{
			fnCryptAcquireContext lpCryptAcquireContext = NULL;
			fnCryptReleaseContext lpCryptReleaseContext = NULL;
			fnCryptGenRandom lpCryptGenRandom = NULL;
			HCRYPTPROV hProvider = NULL;
         
			// Function Pointer: CryptAcquireContext
			lpCryptAcquireContext = reinterpret_cast< fnCryptAcquireContext >
#ifdef UNICODE
			  ( GetProcAddress(hLib, "CryptAcquireContextW") );
#else
			  ( GetProcAddress(hLib, "CryptAcquireContextA") );
#endif
			if (!lpCryptAcquireContext)
			{
				FreeLibrary(hLib);
				return FALSE;     
			}

			// Function Pointer: CryptReleaseContext
			lpCryptReleaseContext = reinterpret_cast< fnCryptReleaseContext >
			 ( GetProcAddress(hLib, "CryptReleaseContext") );
			if (!lpCryptReleaseContext)
			{
				FreeLibrary(hLib);
				return FALSE;
			}

			// Function Pointer: CryptGenRandom
			lpCryptGenRandom = reinterpret_cast< fnCryptGenRandom >
			  ( GetProcAddress(hLib, "CryptGenRandom") );
			if (!lpCryptGenRandom)
			{
				FreeLibrary(hLib);
				return FALSE;
			}

			// Acquire Context
			if (!(*lpCryptAcquireContext)(	 &hProvider,		
											 NULL,
											 NULL,		
											 PROV_RSA_FULL,
											 CRYPT_VERIFYCONTEXT | CRYPT_SILENT))
			{
				FreeLibrary(hLib);
				return FALSE;
			}

			// Random Buffer
			res = (*lpCryptGenRandom)(hProvider, (DWORD)RandomBufferLength, (LPBYTE)RandomBuffer);
     
			// Free the Context
			(*lpCryptReleaseContext)(hProvider, 0);

			// Free the Library
			FreeLibrary(hLib);

			return res;
		}
	}
	else
		return FALSE;
}

#ifdef CRACKCHECK

BOOL CodeSectionInformation()
{
    __try
	{
        HMODULE hModule = AfxGetInstanceHandle();
        if (NULL == hModule)
            return FALSE;

        /////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////
        PIMAGE_DOS_HEADER pDOSHeader = NULL;
        pDOSHeader = static_cast<PIMAGE_DOS_HEADER>( (PVOID)hModule );
        if (pDOSHeader->e_magic != IMAGE_DOS_SIGNATURE)
            return FALSE;

        /////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////
        PIMAGE_NT_HEADERS pNTHeader = NULL;
        pNTHeader = reinterpret_cast<PIMAGE_NT_HEADERS>(
						(PBYTE)hModule + pDOSHeader->e_lfanew);
        if (pNTHeader->Signature != IMAGE_NT_SIGNATURE)
            return FALSE;

        /////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////
        PIMAGE_FILE_HEADER pFileHeader = NULL;
        pFileHeader = reinterpret_cast<PIMAGE_FILE_HEADER>(
						(PBYTE)&pNTHeader->FileHeader );

        /////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////
        PIMAGE_OPTIONAL_HEADER pOptionalHeader = NULL;
        pOptionalHeader = reinterpret_cast<PIMAGE_OPTIONAL_HEADER>(
						(PBYTE)&pNTHeader->OptionalHeader );

        /////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////
        if (IMAGE_NT_OPTIONAL_HDR32_MAGIC != pNTHeader->OptionalHeader.Magic)
            return FALSE;

        /////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////
        PIMAGE_SECTION_HEADER pSectionHeader = NULL;
        pSectionHeader = reinterpret_cast<PIMAGE_SECTION_HEADER>(
						(PBYTE)&pNTHeader->OptionalHeader +
						pNTHeader->FileHeader.SizeOfOptionalHeader);

        /////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////
        int nSectionCount = pNTHeader->FileHeader.NumberOfSections;
        char szSectionName[IMAGE_SIZEOF_SHORT_NAME + 1];
        memset(szSectionName, 0, IMAGE_SIZEOF_SHORT_NAME + 1);
        for (int i = 0 ; i < nSectionCount ; i++)
        {
            memcpy(szSectionName, pSectionHeader->Name, IMAGE_SIZEOF_SHORT_NAME);
            if (0 == strncmp(".text", szSectionName, IMAGE_SIZEOF_SHORT_NAME))
                break;
            pSectionHeader++;
        }

        /////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////
        if (0 != strncmp(".text", szSectionName, IMAGE_SIZEOF_SHORT_NAME))
            return FALSE;
        
        g_dwCodeSize = pSectionHeader->Misc.VirtualSize; 
        g_pCodeStart = (LPBYTE)hModule + (DWORD)(pSectionHeader->VirtualAddress);
		
		return TRUE;
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
	{
        return FALSE;
    }
}

BOOL IsDebuggerPresentApi()
{
	BOOL res = FALSE;
	HINSTANCE h = LoadLibrary(_T("kernel32.dll"));
	if (h)
	{
		FARPROC lIsDebuggerPresent = GetProcAddress(h, "IsDebuggerPresent");
		if (lIsDebuggerPresent)
			res = IsBPX(lIsDebuggerPresent) || lIsDebuggerPresent();
		else
			res = IsDebuggerPresentAsm();
		FreeLibrary(h);
	}
	return res;
}

#endif
