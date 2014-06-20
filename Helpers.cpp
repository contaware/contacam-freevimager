#include "stdafx.h"
#include "mmsystem.h"
#include "wininet.h"
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

// If InitHelpers() is not called vars default to WinXP,
// no multimedia instructions and 2GB RAM
BOOL g_bWin2003 = FALSE;
BOOL g_bWin2003OrHigher = FALSE;
BOOL g_bWinVista = FALSE;
BOOL g_bWinVistaOrHigher = FALSE;
BOOL g_bMMX = FALSE;
BOOL g_bSSE = FALSE;
BOOL g_bSSE2 = FALSE;
BOOL g_b3DNOW = FALSE;
int g_nInstalledPhysRamMB = 2048;
int g_nAvailablePhysRamMB = 2048;
#define CPU_FEATURE_MMX		0x0001
#define CPU_FEATURE_SSE		0x0002
#define CPU_FEATURE_SSE2	0x0004
#define CPU_FEATURE_3DNOW	0x0008
int GetCpuInstr();
int GetTotPhysMemMB(BOOL bInstalled);
void InitHelpers()
{
	OSVERSIONINFO ovi = {0};
    ovi.dwOSVersionInfoSize = sizeof(ovi);
	GetVersionEx(&ovi);

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

	// RAM
	g_nInstalledPhysRamMB = GetTotPhysMemMB(TRUE);
	g_nAvailablePhysRamMB = GetTotPhysMemMB(FALSE);
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

	// Try 64 times to create the directory
	for (int i = 0 ; i < 64 ; i++)
	{
		BOOL bOK = CreateDirectory(szFolder, NULL);
		DWORD dwLastError = GetLastError();
		if (!bOK && dwLastError == ERROR_PATH_NOT_FOUND)
		{
			while (*p != _T('\\'))
			{
				if (p <= pStart)
					return FALSE;
				p--;
			}
			*p = NULL;
		}
		else if (bOK || (ERROR_ALREADY_EXISTS == dwLastError))
		{
			if (p >= pEnd)
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
BOOL DeleteToRecycleBin(LPCTSTR szName)
{
	TCHAR pFrom[MAX_PATH+1];			// +1 for double NULL termination
	_tcsncpy(pFrom, szName, MAX_PATH);	// this pads pFrom with NULLs -> already double NULL terminated
	pFrom[MAX_PATH - 1] = _T('\0');		// if szName to big make sure pFrom is NULL terminated
	pFrom[MAX_PATH] = _T('\0');			// if szName to big make sure pFrom is double NULL terminated

	SHFILEOPSTRUCT FileOp;
	memset(&FileOp, 0, sizeof(SHFILEOPSTRUCT)); 
    FileOp.pFrom = pFrom;
    FileOp.pTo = NULL; 
    FileOp.fFlags = FOF_ALLOWUNDO | FOF_SILENT | FOF_NOCONFIRMATION;
    FileOp.hNameMappings = NULL; 
    FileOp.lpszProgressTitle = NULL; 
	FileOp.fAnyOperationsAborted = FALSE; 
	FileOp.wFunc = FO_DELETE;

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
	if (nYear >= 1971 && nYear <= 3000) // MFC CTime Limitation (January 1, 1970 00:00:00 crashes also...)
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
	// Make sure the Temp Path terminates with a backslash
	if (sTempPath.GetLength() > 0)
	{
		if (sTempPath[sTempPath.GetLength() - 1] != _T('\\'))
			sTempPath += _T('\\');
	}
	return sTempPath + GetShortFileNameNoExt(lpszFileName) + _T("_") + GetUuidString() + GetFileExt(lpszFileName);
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

BOOL IsSubDir(CString sDir, CString sSubDir)
{
	sDir.MakeLower();
	sSubDir.MakeLower();
	sDir.Replace(_T('/'), _T('\\'));
	sSubDir.Replace(_T('/'), _T('\\'));
	sDir.TrimRight(_T('\\'));
	sSubDir.TrimRight(_T('\\'));
	int index;
	while ((index = sSubDir.ReverseFind(_T('\\'))) >= 0)
	{
		sSubDir = sSubDir.Left(index);
		if (sSubDir == sDir)
			return TRUE;
	}
	return FALSE;
}

ULARGE_INTEGER GetFileSize64(LPCTSTR lpszFileName)
{
	ULARGE_INTEGER Size;
	Size.QuadPart = 0;
	WIN32_FIND_DATA fileinfo;
	memset(&fileinfo, 0, sizeof(WIN32_FIND_DATA));
	HANDLE hFind = FindFirstFile(lpszFileName, &fileinfo);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		Size.LowPart = fileinfo.nFileSizeLow;
		Size.HighPart = fileinfo.nFileSizeHigh;
		FindClose(hFind);
	}
	return Size;
}

BOOL GetFileTime(	LPCTSTR lpszFileName,
					LPFILETIME lpCreationTime,
					LPFILETIME lpLastAccessTime,
					LPFILETIME lpLastWriteTime)
{
	WIN32_FIND_DATA fileinfo;
	memset(&fileinfo, 0, sizeof(WIN32_FIND_DATA));
	HANDLE hFind = FindFirstFile(lpszFileName, &fileinfo);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		if (lpCreationTime)
			*lpCreationTime = fileinfo.ftCreationTime;
		if (lpLastAccessTime)
			*lpLastAccessTime = fileinfo.ftLastAccessTime;
		if (lpLastWriteTime)
			*lpLastWriteTime = fileinfo.ftLastWriteTime;
		FindClose(hFind);
		return TRUE;
	}
	else
		return FALSE;
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
	rStatus.m_size = (ULONGLONG)findFileData.nFileSizeLow |
					((ULONGLONG)findFileData.nFileSizeHigh) << 32;

	// convert times as appropriate
	if (CTime::IsValidFILETIME(findFileData.ftCreationTime))
		rStatus.m_ctime = CTime(findFileData.ftCreationTime);
	else
		rStatus.m_ctime = CTime();

	if (CTime::IsValidFILETIME(findFileData.ftLastAccessTime))
		rStatus.m_atime = CTime(findFileData.ftLastAccessTime);
	else
		rStatus.m_atime = CTime();

	if (CTime::IsValidFILETIME(findFileData.ftLastWriteTime))
		rStatus.m_mtime = CTime(findFileData.ftLastWriteTime);
	else
		rStatus.m_mtime = CTime();

	if (rStatus.m_ctime.GetTime() == 0)
		rStatus.m_ctime = rStatus.m_mtime;

	if (rStatus.m_atime.GetTime() == 0)
		rStatus.m_atime = rStatus.m_mtime;

	return TRUE;
}

CString GetSpecialFolderPath(int nSpecialFolder)
{
	TCHAR path[MAX_PATH] = {0};
	SHGetSpecialFolderPath(NULL, path, nSpecialFolder, FALSE);
	return CString(path);
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
		lpfGetModuleBaseName = (DWORD(WINAPI*)(HANDLE, HMODULE, LPTSTR, DWORD))GetProcAddress(hInstLib, "GetModuleBaseNameW");	
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
	lpfProcess32First = (BOOL(WINAPI*)(HANDLE,LPPROCESSENTRY32))GetProcAddress(hInstLib, "Process32FirstW") ;
	lpfProcess32Next = (BOOL(WINAPI*)(HANDLE,LPPROCESSENTRY32))GetProcAddress(hInstLib, "Process32NextW");
	lpfModule32First = (BOOL(WINAPI*)(HANDLE,LPMODULEENTRY32))GetProcAddress(hInstLib, "Module32FirstW");
	lpfModule32Next = (BOOL(WINAPI*)(HANDLE,LPMODULEENTRY32))GetProcAddress(hInstLib, "Module32NextW");
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
	DWORD dw = GetPrivateProfileString(lpszSection, lpszEntry, lpszDefault, szT, _countof(szT), lpszProfileName);
	ASSERT(dw < 4095);
	return szT;
}

BOOL WriteProfileIniInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue, LPCTSTR lpszProfileName)
{
	ASSERT(lpszSection != NULL);
	ASSERT(lpszEntry != NULL);
	ASSERT(lpszProfileName != NULL);
	TCHAR szT[16];
	_stprintf_s(szT, _countof(szT), _T("%d"), nValue);
	return WritePrivateProfileString(lpszSection, lpszEntry, szT, lpszProfileName);
}

BOOL WriteProfileIniString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue, LPCTSTR lpszProfileName)
{
	ASSERT(lpszSection != NULL);
	ASSERT(lpszProfileName != NULL);
	ASSERT(lstrlen(lpszProfileName) < 4095); // can't read in bigger
	return WritePrivateProfileString(lpszSection, lpszEntry, lpszValue, lpszProfileName);
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

CString ShowError(DWORD dwErrorCode, BOOL bShowMessageBoxOnError, CString sHeader/*=_T("")*/, CString sFooter/*=_T("")*/)
{
	CString sText;
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
		// Init
		sText = (LPCTSTR)lpMsgBuf;
		
		// Remove terminating CR and LF
		sText.TrimRight(_T("\r\n"));

		// Replace eventual CRs or LFs in the middle of the string with a space
		sText.Replace(_T('\r'), _T(' '));
		sText.Replace(_T('\n'), _T(' '));

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
	else if (dwErrorCode == ERROR_INTERNET_CONNECTION_RESET)
	{
		CString sResponse(ML_STRING(1766, "The connection with the server has been reset."));
		if (sText != _T(""))
			sText = sText + _T('\n') + sResponse;
		else
			sText = sResponse;
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
		CString sResponse(ML_STRING(1780, "The request to connect could not be completed because the supplied user name and/or password are incorrect."));
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

	// Format and show error
	CString sMsg = sHeader + sText + sFooter;
	if (sMsg[sMsg.GetLength() - 1] != _T('\n')) // this is ok because sMsg is never empty
		TRACE(sMsg + _T('\n'));
	else
		TRACE(sMsg);
	if (bShowMessageBoxOnError)
		AfxMessageBox(sMsg, MB_ICONSTOP);
	return sMsg;
}

CString ShowLastError(BOOL bShowMessageBoxOnError, CString sHeader/*=_T("")*/, CString sFooter/*=_T("")*/)
{
	return ShowError(GetLastError(), bShowMessageBoxOnError, sHeader, sFooter);
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

CString GetComputerName()
{
	TCHAR szComputerName[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD dwSize = MAX_COMPUTERNAME_LENGTH + 1;
	if (GetComputerName(szComputerName, &dwSize))
		return CString(szComputerName);
	else
		return _T("");
}

static int GetTotPhysMemMB(BOOL bInstalled)
{
	/* The GetPhysicallyInstalledSystemMemory function retrieves
	the amount of physically installed RAM from the computer's
	SMBIOS firmware tables. This can differ from the amount reported
	by the GlobalMemoryStatusEx function, which sets the ullTotalPhys
	member of the MEMORYSTATUSEX structure to the amount of physical
	memory that is available for the operating system to use. The amount
	of memory available to the operating system can be less than the
	amount of memory physically installed in the computer because the
	BIOS and some drivers may reserve memory as I/O regions for
	memory-mapped devices, making the memory unavailable to the
	operating system and applications. */
	typedef BOOL (WINAPI * FPGLOBALMEMORYSTATUSEX)(LPMEMORYSTATUSEX lpBuffer);
	typedef BOOL (WINAPI * FPGETPHYSICALLYINSTALLEDSYSTEMMEMORY)(PULONGLONG TotalMemoryInKilobytes);
	HINSTANCE h = LoadLibrary(_T("kernel32.dll"));
	if (!h)
		return 0;
	FPGETPHYSICALLYINSTALLEDSYSTEMMEMORY fpGetPhysicallyInstalledSystemMemory;
	FPGLOBALMEMORYSTATUSEX fpGlobalMemoryStatusEx;
	ULONGLONG ullTotalMemoryInKilobytes;
	MEMORYSTATUSEX MemoryStatusEx;
	MemoryStatusEx.dwLength = sizeof(MemoryStatusEx);
	if (bInstalled &&
		(fpGetPhysicallyInstalledSystemMemory = (FPGETPHYSICALLYINSTALLEDSYSTEMMEMORY)GetProcAddress(h, "GetPhysicallyInstalledSystemMemory")) &&
		fpGetPhysicallyInstalledSystemMemory(&ullTotalMemoryInKilobytes))
	{
		FreeLibrary(h);
		return (int)(ullTotalMemoryInKilobytes >> 10);
	}
	else if ((fpGlobalMemoryStatusEx = (FPGLOBALMEMORYSTATUSEX)GetProcAddress(h, "GlobalMemoryStatusEx")) &&
			fpGlobalMemoryStatusEx(&MemoryStatusEx))
	{
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
	memset(&memory_info, 0, sizeof(memory_info));
	DWORD region = 0;
	DWORD sum_free = 0, max_free = 0;
	DWORD sum_reserve = 0, max_reserve = 0;
	DWORD sum_commit = 0, max_commit = 0;
	while (VirtualQuery(memory_info.BaseAddress, &memory_info, sizeof(memory_info))) // it stops when passing >= 0x7fff0000
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
}

BOOL EnableLFHeap()
{
	// Enable the low-fragmenation heap (LFH) for XP and Windows 2003.
	// Starting with Windows Vista the LFH is enabled by default for
	// the default heap and starting with Visual Studio 2010 it is
	// enable by default for the CRT heap
	ULONG HeapInformation = 2;	// LFH heap
	BOOL bResDefaultHeap = HeapSetInformation(GetProcessHeap(),
								HeapCompatibilityInformation,
								&HeapInformation,
								sizeof(HeapInformation));

	HeapInformation = 2;		// LFH heap
	BOOL bResCRTHeap = HeapSetInformation((HANDLE)_get_heap_handle(),
								HeapCompatibilityInformation,
								&HeapInformation,
								sizeof(HeapInformation));

	return (bResDefaultHeap && bResCRTHeap);
}

ULONGLONG GetDiskTotalSize(LPCTSTR lpszPath)
{
	// Receives the total number of bytes on disk available to caller
	ULARGE_INTEGER TotalNumberOfBytesAvailableToCaller;
	if (!GetDiskFreeSpaceEx(GetDriveName(lpszPath) + _T("\\"),
							NULL,
							&TotalNumberOfBytesAvailableToCaller,
							NULL))
		return 0;
	else
		return TotalNumberOfBytesAvailableToCaller.QuadPart;
}

ULONGLONG GetDiskAvailableFreeSpace(LPCTSTR lpszPath)
{
	// Receives the number of free bytes on disk available to caller
	ULARGE_INTEGER FreeBytesAvailableToCaller;
	if (!GetDiskFreeSpaceEx(GetDriveName(lpszPath) + _T("\\"),
							&FreeBytesAvailableToCaller,
							NULL,
							NULL))
		return 0;
	else
		return FreeBytesAvailableToCaller.QuadPart;
}

CString FileNameToMime(LPCTSTR lpszFileName)
{
	CString sExt = GetFileExt(lpszFileName);

	if (IsJPEGExt(sExt))
		return _T("image/jpeg");
	else if (IsTIFFExt(sExt))
		return _T("image/tiff");
	else if (sExt == _T(".bmp")	|| sExt == _T(".dib"))
		return _T("image/bmp");
	else if (sExt == _T(".gif"))
		return _T("image/gif");
	else if (sExt == _T(".png"))
		return _T("image/png");
	else if (sExt == _T(".avi")	|| sExt == _T(".divx"))
		return _T("video/avi");
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

BOOL IsValidFileName(const CString& s, BOOL bShowMessageBoxOnError/*=FALSE*/)
{
	// Not allowed  \ / : * ? " < > |
	if (s.FindOneOf(_T("\\/:*?\"<>|")) >= 0)
	{
		if (bShowMessageBoxOnError)
			ShowError(ERROR_INVALID_NAME, TRUE);
		return FALSE;
	}
	else
		return TRUE;
}

BOOL IsANSIConvertible(const CString& s)
{
	// Empty String is Convertible!
	if (s.IsEmpty())
		return TRUE;

	// UTF16 string length (null termination not included)
	int nUtf16Len = s.GetLength();

	// Convert UTF16 to ANSI
	BOOL bUsedDefaultChar = FALSE;
	int res = WideCharToMultiByte(	CP_ACP,					// ANSI Code Page
									WC_NO_BEST_FIT_CHARS,	// translate any Unicode characters that do not
															// translate directly to multibyte equivalents to
															// the default character specified by lpDefaultChar
									s,						// wide-character string
									nUtf16Len,				// number of chars in string.
									NULL,					// buffer for new string
									0,						// size of buffer
									NULL,					// default for unmappable chars given by system
									&bUsedDefaultChar);		// set when default char used
	if (res <= 0 && GetLastError() == ERROR_INVALID_FLAGS)
	{
		res = WideCharToMultiByte(	CP_ACP,					// ANSI Code Page
									0, 
									s,						// wide-character string
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

	// UTF16 string length (null termination not included)
	int nUtf16Len = s.GetLength();

	// Allocate enough buffer:
	// the MBCS schemes used in Windows contain two character types,
	// single-byte characters and double-byte characters. Since the
	// largest multi-byte character used in Windows is two bytes long,
	// the term double-byte character set, or DBCS, is commonly used
	// in place of MBCS.
	*ppAnsi = new char[2*nUtf16Len + 1];
	if (*ppAnsi == NULL)
		return 0;

	// Convert UTF16 to ANSI
	int nBytesWritten = WideCharToMultiByte(CP_ACP,					// ANSI Code Page
											WC_NO_BEST_FIT_CHARS,	// translate any Unicode characters that do not
																	// translate directly to multibyte equivalents to
																	// the default character specified by lpDefaultChar
											s,						// wide-character string
											nUtf16Len,				// number of chars in string.
											*ppAnsi,				// buffer for new string
											2*nUtf16Len,			// size of allocated buffer minus null termination
											NULL,					// default for unmappable chars given by system
											pbUsedDefaultChar);		// set when default char used
	if (nBytesWritten <= 0 && GetLastError() == ERROR_INVALID_FLAGS)
	{
		nBytesWritten = WideCharToMultiByte(CP_ACP,					// ANSI Code Page
											0, 
											s,						// wide-character string
											nUtf16Len,				// number of chars in string
											*ppAnsi,				// buffer for new string
											2*nUtf16Len,			// size of allocated buffer minus null termination
											NULL,					// default for unmappable chars
											pbUsedDefaultChar);		// set when default char used
	}
	if (nBytesWritten > 0)
	{
		(*ppAnsi)[nBytesWritten] = '\0'; // null terminate
		return nBytesWritten;
	}
	else
	{
		delete [] *ppAnsi;
		*ppAnsi = NULL;
		return 0;
	}
}

BOOL IsASCIICompatiblePath(const CString& sPath)
{
	// Empty Path is already ok!
	if (sPath.IsEmpty())
		return TRUE;

	LPSTR c = NULL;
	if (ToANSI(sPath, &c) <= 0 || !c)
	{
		if (c)
			delete [] c;
		return FALSE;
	}
	for (int i = 0 ; i < (int)strlen(c) ; i++)
	{
		if (!((48 <= c[i] && c[i] <= 57)||	// 0-9
			(65 <= c[i] && c[i] <= 90)	||	// ABC...XYZ
			(97 <= c[i] && c[i] <= 122)	||	// abc...xyz
			c[i] == ' '					||	// space
			c[i] == '-'					||	// minus
			c[i] == '_'					||	// underscore
			c[i] == '~'					||	// tilde
			c[i] == '.'					||	// dot
			c[i] == ':'					||	// column
			c[i] == '\\'				||	// backslash
			c[i] == '/'))					// slash
		{
			delete [] c;
			return FALSE;
		}
	}
	delete [] c;
	return TRUE;
}

/*
GetShortPathName will return an ASCII string if NtfsAllowExtendedCharacterIn8dot3Name
is not set in the registry (the default value is 0 so we are quite ok with the following code)

NtfsAllowExtendedCharacterIn8dot3Name under
HKLM\SYSTEM\CurrentControlSet\Control\FileSystem
specifies whether the characters from the extended character set,
including diacritic characters, can be used in short file names
using the 8.3 naming convention on NTFS volumes.
 
Values:
0: On NTFS volumes, file names using the 8.3 naming convention are limited
   to the standard ASCII character set (minus any reserved values)
1: On NTFS volumes, file names using the 8.3 naming convention may use extended characters

Note: this entry does not exist in the registry by default,
you can add it by using the registry editor.

See: http://technet.microsoft.com/en-us/library/cc781607%28WS.10%29.aspx
*/
CString GetASCIICompatiblePath(const CString& sPath)
{
	if (!IsASCIICompatiblePath(sPath))
	{
		TCHAR lpszShortPath[1024];
		DWORD dwCount = GetShortPathName(sPath, lpszShortPath, 1024);
		lpszShortPath[1023] = _T('\0');
		if (dwCount == 0)
			return sPath;
		else
			return CString(lpszShortPath);
	}
	else
		return sPath;
}

// http://tools.ietf.org/html/rfc3986
CString UrlEncode(const CString& s, BOOL bEncodeReserved)
{
	// Empty String is already ok!
	if (s.IsEmpty())
		return _T("");

	LPSTR c = NULL;
	if (ToANSI(s, &c) <= 0 || !c)
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
				 (c[i]== '-' || c[i] == '.' ||	// unreserved chars
				 c[i] == '_' || c[i] == '~'))	// unreserved chars
				sEscaped += CString(c[i]);
			else
			{
				sEscaped += _T("%");
				sHex.Format(_T("%02X"), ((int)c[i]) & 0xFF);
				sEscaped += sHex;
			}
		}
	}
	else
	{
		for (int i = 0 ; i < (int)strlen(c) ; i++)
		{
			if ( (48 <= c[i] && c[i] <= 57) ||	// 0-9
				 (65 <= c[i] && c[i] <= 90) ||	// ABC...XYZ
				 (97 <= c[i] && c[i] <= 122)||	// abc...xyz
				 (c[i]== '-' || c[i] == '.' ||	// unreserved chars
				 c[i] == '_' || c[i] == '~' ||	// unreserved chars
				 c[i] == ':' || c[i] == '/' ||	// reserved chars
				 c[i] == '?' || c[i] == '#' ||	// reserved chars
				 c[i] == '[' || c[i] == ']' ||	// reserved chars
				 c[i] == '@' || c[i] == '!' ||	// reserved chars
				 c[i] == '$' || c[i] == '&'	||	// reserved chars
				 c[i] == '\''|| c[i] == '(' ||	// reserved chars
				 c[i] == ')' || c[i] == '*'	||	// reserved chars
				 c[i] == '+' || c[i] == ','	||	// reserved chars
				 c[i] == ';' || c[i] == '='))	// reserved chars
				sEscaped += CString(c[i]);
			else
			{
				sEscaped += _T("%");
				sHex.Format(_T("%02X"), ((int)c[i]) & 0xFF);
				sEscaped += sHex;
			}
		}
	}
	delete [] c;
	return sEscaped;
}

CString UrlDecode(const CString& s)
{
	// Empty String is already ok!
	if (s.IsEmpty())
		return _T("");

	// Decode
	CString sDecoded;
	for (int i = 0 ; i < s.GetLength() ; i++)
	{
		if ((s[i] == _T('%')) && ((i + 2) < s.GetLength()))
		{
			unsigned long ulCode = _tcstoul(s.Mid(i + 1, 2), NULL, 16);
			if (ulCode > 0U && ulCode < 256U)
			{
				sDecoded += CString((char)ulCode);
				i += 2; // skip the two hex digits
				continue;
			}
		}
		sDecoded += s[i];
	}

	return sDecoded;
}

CString HtmlEncode(CString s)
{
	s.Replace(_T("&"), _T("&amp;")); // must be first!
	s.Replace(_T("\""), _T("&quot;"));
	s.Replace(_T("\'"), _T("&apos;"));
	s.Replace(_T("<"), _T("&lt;"));
	s.Replace(_T(">"), _T("&gt;"));
	return s;
}

CString HtmlDecode(CString s)
{
	s.Replace(_T("&gt;"), _T(">"));
	s.Replace(_T("&lt;"), _T("<"));
	s.Replace(_T("&apos;"), _T("\'"));
	s.Replace(_T("&quot;"), _T("\""));
	s.Replace(_T("&amp;"), _T("&")); // must be last!
	return s;
}

CString FromUTF8(const unsigned char* pUtf8, int nUtf8Len)
{
	// Check
	if (!pUtf8 || nUtf8Len <= 0)
		return _T("");

	// Convert UTF8 to UTF16
	CString s;
	LPWSTR p = s.GetBuffer(nUtf8Len + 1);	// allocate enough buffer
											// (according to documentation the +1 should not
											// be necessary because the null termination char
											// should already be given, but you never know if
											// that's correct...)
	if (p)
	{
		int nCharsWritten = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)pUtf8, nUtf8Len, p, nUtf8Len);
		if (nCharsWritten > 0)
			p[nCharsWritten] = L'\0'; // null terminate
		s.ReleaseBuffer();
	}
	return s;
}

int ToUTF8(const CString& s, LPBYTE* ppUtf8)
{
	// Check
	if (!ppUtf8)
		return 0;

	// UTF16 string length (null termination not included)
	int nUtf16Len = s.GetLength();
	
	// Allocate enough buffer:
	// in UTF8, characters from the U+0000..U+10FFFF range (the UTF-16 accessible range)
	// are encoded using sequences of 1 to 4 octets.
	*ppUtf8 = new BYTE[4*nUtf16Len + 1];
	if (*ppUtf8 == NULL)
		return 0;
	
	// Convert UTF16 to UTF8
	int nBytesWritten = WideCharToMultiByte(CP_UTF8,
											0,
											s,					// wide-character string
											nUtf16Len,			// number of chars in string
											(LPSTR)(*ppUtf8),	// buffer for new string
											4*nUtf16Len,		// size of allocated buffer minus null termination
											NULL,
											NULL);
	if (nBytesWritten > 0)
	{
		(*ppUtf8)[nBytesWritten] = '\0'; // null terminate
		return nBytesWritten;
	}
	else
	{
		delete [] *ppUtf8;
		*ppUtf8 = NULL;
		return 0;
	}
}

CString UuidToString(const UUID* pUuid)
{
	CString sUuid;
	if (pUuid)
	{
		unsigned short* sTemp;
		if (UuidToString(pUuid, &sTemp) == RPC_S_OK)
		{
			sUuid = CString((LPCTSTR)sTemp);
			RpcStringFree(&sTemp);
		}
	}
	return sUuid;
}

CString GetUuidString()
{
	UUID Uuid;
	HRESULT hr = UuidCreate(&Uuid);
	if (hr == (HRESULT)RPC_S_OK || hr == (HRESULT)RPC_S_UUID_LOCAL_ONLY)
		return UuidToString(&Uuid);
	else
		return _T("");
}

int __cdecl CompareNatural(CString * pstr1, CString * pstr2)
{
	if (pstr1 == NULL || pstr2 == NULL)
		return 0;
	LPCWSTR pwstr1 = (LPCWSTR)(*pstr1);
	LPCWSTR pwstr2 = (LPCWSTR)(*pstr2);
	return StrCmpLogicalW(pwstr1, pwstr2);
}

BOOL InStringArray(const CString& s, const CStringArray& arr)
{
	for (int i = 0 ; i < arr.GetSize() ; i++)
	{
		if (s.CompareNoCase(arr[i]) == 0)
			return TRUE;
	}
	return FALSE;
}

BOOL IntersectsValidMonitor(LPCRECT lpRect)
{
	return (MonitorFromRect(lpRect, MONITOR_DEFAULTTONULL) != NULL ? TRUE : FALSE);
}

// From: http://www.azillionmonkeys.com/qed/random.html
#define RS_SCALE (1.0 / (1.0 + RAND_MAX))
double drand() { // returns a double in the range of [0.0,1.0[  (0.0 included and 1.0 excluded)
    double d;
    do {
       d = (((rand() * RS_SCALE) + rand()) * RS_SCALE + rand()) * RS_SCALE;
    } while (d >= 1.0); // shield in case of an extremely unlikely case of erroneous overflow
    return d;
}

// From: http://www.concentric.net/~Ttwang/tech/inthash.htm
unsigned int makeseed(unsigned int a, unsigned int b, unsigned int c)
{
	a=a-b;  a=a-c;  a=a^(c >> 13);
	b=b-c;  b=b-a;  b=b^(a << 8); 
	c=c-a;  c=c-b;  c=c^(b >> 13);
	a=a-b;  a=a-c;  a=a^(c >> 12);
	b=b-c;  b=b-a;  b=b^(a << 16);
	c=c-a;  c=c-b;  c=c^(b >> 5);
	a=a-b;  a=a-c;  a=a^(c >> 3);
	b=b-c;  b=b-a;  b=b^(a << 10);
	c=c-a;  c=c-b;  c=c^(b >> 15);
	return c;
}

int DrawBigText(HDC hDC,
				CRect rc,
				LPCTSTR szText,
				COLORREF crTextColor,
				int nMaxFontSize/*=72*/,
				UINT uAlign/*=DT_CENTER | DT_VCENTER*/,
				int nBkMode/*=TRANSPARENT*/,
				COLORREF crBkColor/*=RGB(0,0,0)*/)
{
	// Check
	if (!hDC)
		return 0;

	// Vars
	HFONT hFont;
	HFONT hOldFont = NULL;
	LOGFONT lf;
	int nUsedHeightPix;
	nMaxFontSize = MAX(nMaxFontSize, 8); // 8 is min font size

	// Set colors and mode
	COLORREF crOldTextColor = SetTextColor(hDC, crTextColor);
	int nOldBkMode = SetBkMode(hDC, nBkMode);
	COLORREF crOldBkColor = SetBkColor(hDC, crBkColor);

	// Calc. Font Size
	while (TRUE)
	{
		memset(&lf, 0, sizeof(lf));
		_tcscpy(lf.lfFaceName, DEFAULT_FONTFACE);
		lf.lfHeight = -MulDiv(nMaxFontSize, GetDeviceCaps(hDC, LOGPIXELSY), 72);
		lf.lfWeight = FW_MEDIUM;
		lf.lfItalic = 0;
		lf.lfUnderline = 0;
		hFont = CreateFontIndirect(&lf);
		CRect rcText(0,0,0,0);
		hOldFont = (HFONT)SelectObject(hDC, hFont);
		nUsedHeightPix = DrawText(hDC, szText, -1, rcText, DT_CALCRECT | DT_SINGLELINE | DT_NOCLIP);
		if (rcText.Width() > rc.Width())
		{
			if (nMaxFontSize == 8) // 8 is min font size
				break;
			nMaxFontSize = MAX(Round((double)nMaxFontSize * (double)rc.Width() /
								(1.3 * (double)rcText.Width())), 8);
			SelectObject(hDC, hOldFont);
			DeleteObject(hFont);
		}
		else
			break;
	}

	// Draw Message
	DrawText(hDC, szText, -1, rc, uAlign | DT_NOCLIP | DT_SINGLELINE);
	
	// Restore and clean-up
	SetBkColor(hDC, crOldBkColor);
	SetBkMode(hDC, nOldBkMode);
	SetTextColor(hDC, crOldTextColor);
	SelectObject(hDC, hOldFont);
	DeleteObject(hFont);

	return nUsedHeightPix;
}
