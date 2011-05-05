#if !defined(AFX_HELPERS_H__8FD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_)
#define AFX_HELPERS_H__8FD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MMSystem.h"
#include "WorkerThread.h"

#ifndef MAX
#  define MAX(a,b)  ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#  define MIN(a,b)  ((a) < (b) ? (a) : (b))
#endif

extern BOOL g_bWin95;
extern BOOL g_bWin9x;
extern BOOL g_bNT;
extern BOOL g_bNT4OrOlder;
extern BOOL g_bWin2000;
extern BOOL g_bWin2000OrHigher;
extern BOOL g_bWinXP;
extern BOOL g_bWinXPOrHigher;
extern BOOL g_bWin2003;
extern BOOL g_bWin2003OrHigher;
extern BOOL g_bWinVista;
extern BOOL g_bWinVistaOrHigher;
extern BOOL g_bMMX;
extern BOOL g_bSSE;
extern BOOL g_bSSE2;
extern BOOL g_b3DNOW;

// Call This Once Before Using the following Functions!
extern void InitHelpers();

// Call This Once When finished Using the following Functions!
extern void EndHelpers();

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
extern CString GetDriveName(const CString& sFullFilePath);
extern CString GetDirName(const CString& sFullFilePath);
extern CString GetDriveAndDirName(const CString& sFullFilePath);
extern CString GetShortFileName(const CString& sFullFilePath);
extern CString GetShortFileNameNoExt(const CString& sFullFilePath);
extern CString GetFileNameNoExt(const CString& sFullFilePath);
extern CString GetFileExt(const CString& sFullFilePath);

// Is readonly flag set
extern BOOL IsReadonly(LPCTSTR lpszFileName);

// Do we have write access to the given file
extern BOOL HasWriteAccess(LPCTSTR lpszFileName);

// API file / directory operations
extern BOOL TakeOwnership(LPCTSTR lpszFile);
extern BOOL SetPrivilege(HANDLE hToken,LPCTSTR lpszPrivilege,BOOL bChange);
extern BOOL SetPermission(LPCTSTR lpszFile, LPCTSTR lpszAccess, DWORD dwAccessMask);
extern BOOL CreateDir(LPCTSTR szNewDir);
extern BOOL CopyDirContent(LPCTSTR szFromDir, LPCTSTR szToDir, BOOL bOverwriteIfExists = TRUE, BOOL bContinueOnCopyError = TRUE);
extern BOOL MergeDirContent(LPCTSTR szFromDir, LPCTSTR szToDir, BOOL bOverwriteIfExists = TRUE, BOOL bContinueOnCopyError = TRUE);
extern BOOL DeleteDir(LPCTSTR szDirName);
extern BOOL DeleteDirContent(LPCTSTR szDirName, BOOL bOnlyFiles = FALSE);
extern ULARGE_INTEGER GetDirContentSize(LPCTSTR szDirName, 
										int* pFilesCount = NULL, // optional Files Count
										CWorkerThread* pThread = NULL);

// Shell file / directory operations
extern BOOL DeleteToRecycleBin(LPCTSTR szName, BOOL bSilent = TRUE);				// delete file or directory
extern BOOL RenameShell(LPCTSTR szOldName, LPCTSTR szNewName, BOOL bSilent = TRUE);	// rename file or directory
extern BOOL MoveShell(LPCTSTR szFromName, LPCTSTR szToName, BOOL bSilent = TRUE);	// move file or directory
extern BOOL CopyShell(LPCTSTR szFromName, LPCTSTR szToName, BOOL bSilent = TRUE);	// copy file or directory
extern BOOL CopyDirContentShell(LPCTSTR szFromDir, LPCTSTR szToDir, BOOL bSilent = TRUE);

// Date / Time Formatting According to Local Settings
extern CString MakeTimeLocalFormat(	const CTime& Time,
									BOOL bShowSeconds = FALSE);
extern CString MakeDateLocalFormat(	const CTime& Time,
									BOOL bLongDate = FALSE);
extern CString GetDateLocalFormat(BOOL bLongDate = FALSE);
extern CTime ParseShortDateLocalFormat(CString sDate);

// Create a Temp File Name
extern CString MakeTempFileName(CString sTempPath, LPCTSTR lpszFileName);

// File / Dir Existence Check
extern BOOL IsExistingFile(LPCTSTR lpszFileName);
extern BOOL IsExistingDir(LPCTSTR lpszFileName);

// File Size
extern ULARGE_INTEGER GetFileSize64(LPCTSTR lpszFileName);

// Get File Times
extern BOOL GetFileTime(LPCTSTR lpszFileName,
						LPFILETIME lpCreationTime,
						LPFILETIME lpLastAccessTime,
						LPFILETIME lpLastWriteTime);

// Set File Times
extern BOOL SetFileTime(LPCTSTR lpszFileName,
						LPFILETIME lpCreationTime,
						LPFILETIME lpLastAccessTime,
						LPFILETIME lpLastWriteTime);

// Get File Status, do not use for file size because with
// old MFC Versions the size of CFileStatus is limited to 4GB.
// -> Use GetFileSize64()
extern BOOL GetFileStatus(LPCTSTR lpszFileName, CFileStatus& rStatus);

// Wrapper for SHGetSpecialFolderPath, returns the path with no trailing backslash
/*
CSIDL_ALTSTARTUP				File system directory that corresponds to the user's nonlocalized Startup program group. 
CSIDL_APPDATA					The file system directory that serves as a common repository for application-specific data.
								A typical path is C:\Documents and Settings\username\Application Data.
CSIDL_LOCAL_APPDATA             The file system directory that serves as a data repository for local (nonroaming) applications.
								A typical path is C:\Documents and Settings\username\Local Settings\Application Data.
CSIDL_COMMON_APPDATA			The file system directory that contains application data for all users.
								A typical path is C:\Documents and Settings\All Users\Application Data.
								This folder is used for application data that is not user specific.
CSIDL_BITBUCKET					File system directory containing file objects in the user's Recycle Bin.
								The location of this directory is not in the registry;
								it is marked with the hidden and system attributes to prevent the user from moving or deleting it. 
CSIDL_COMMON_ALTSTARTUP			File system directory that corresponds to the nonlocalized Startup program group for all users. 
CSIDL_COMMON_DESKTOPDIRECTORY	File system directory that contains files and folders that appear on the desktop for all users. 
CSIDL_COMMON_FAVORITES			File system directory that serves as a common repository for all users' favorite items. 
CSIDL_COMMON_PROGRAMS			File system directory that contains the directories for the common program groups that appear
								on the Start menu for all users. 
CSIDL_COMMON_STARTMENU			File system directory that contains the programs and folders that appear on the Start menu for all users. 
CSIDL_COMMON_STARTUP			File system directory that contains the programs that appear in the Startup folder for all users.  
CSIDL_CONTROLS					Virtual folder containing icons for the Control Panel applications. 
CSIDL_COOKIES					File system directory that serves as a common repository for Internet cookies. 
CSIDL_DESKTOP					Windows Desktop, virtual folder at the root of the namespace. 
CSIDL_DESKTOPDIRECTORY			File system directory used to physically store file objects on the desktop
								(not to be confused with the desktop folder itself). 
CSIDL_DRIVES					My Computer—virtual folder containing everything on the local computer: storage devices,
								printers, and Control Panel. The folder may also contain mapped network drives. 
CSIDL_FAVORITES					File system directory that serves as a common repository for the user's favorite items. 
CSIDL_FONTS						Virtual folder containing fonts. 
CSIDL_HISTORY					File system directory that serves as a common repository for Internet history items. 
CSIDL_INTERNET					Virtual folder representing the Internet. 
CSIDL_INTERNET_CACHE			File system directory that serves as a common repository for temporary Internet files. 
CSIDL_NETHOOD					File system directory containing objects that appear in the network neighborhood. 
CSIDL_NETWORK					Network Neighborhood Folder—virtual folder representing the top level of the network hierarchy. 
CSIDL_PERSONAL					File system directory that serves as a common repository for documents. 
CSIDL_PRINTERS					Virtual folder containing installed printers. 
CSIDL_PRINTHOOD					File system directory that serves as a common repository for printer links. 
CSIDL_PROGRAMS					File system directory that contains the user's program groups (which are also file system directories). 
CSIDL_RECENT					File system directory that contains the user's most recently used documents. 
CSIDL_SENDTO					File system directory that contains Send To menu items. 
CSIDL_STARTMENU					File system directory containing Start menu items. 
CSIDL_STARTUP					File system directory that corresponds to the user's Startup program group.
								The system starts these programs whenever any user logs onto Windows NT or starts Windows 95. 
CSIDL_TEMPLATES					File system directory that serves as a common repository for document templates.
CSIDL_MYVIDEO					My Videos folder
CSIDL_MYMUSIC					My Music folder
CSIDL_MYPICTURES				My Pictures folder
CSIDL_PERSONAL					My Documents folder
CSIDL_CDBURN_AREA				CD Burn area
*/
extern CString GetSpecialFolderPath(int nSpecialFolder);

// Make a valid file path
extern CString MakeValidPath(CString sPath);

// Enumerate or eventually kill process(es) by name
// returns the number of found processes
// On NT4 you need to install psapi.dll from the Platform SDK
extern int EnumKillProcByName(CString sProcessName, BOOL bKill = FALSE);

// Execute an app with hidden window (working for both console and windowed apps)
extern BOOL ExecHiddenApp(	const CString& sFileName,
							const CString& sParams = _T(""),
							BOOL bWaitTillDone = FALSE,
							DWORD dwWaitMillisecondsTimeout = INFINITE);

// Registry Functions
extern BOOL IsRegistryValue(HKEY hOpenKey, LPCTSTR szKey, LPCTSTR szValue);
extern BOOL IsRegistryKey(HKEY hOpenKey, LPCTSTR szKey);
extern BOOL SetRegistryEmptyValue(HKEY hOpenKey, LPCTSTR szKey, LPCTSTR szValue);
extern BOOL SetRegistryStringValue(HKEY hOpenKey, LPCTSTR szKey, LPCTSTR szValue, LPCTSTR szData);
extern CString GetRegistryStringValue(HKEY hOpenKey, LPCTSTR szKey, LPCTSTR szValue);
extern BOOL DeleteRegistryValue(HKEY hOpenKey, LPCTSTR szKey, LPCTSTR szValue);
extern BOOL DeleteRegistryKey(HKEY hOpenKey, LPCTSTR szKey);

// TRACE and eventually Pop Message Box of String:
// sHeader + GetLastError() in Readable Format + sFooter
extern void ShowError(	DWORD dwErrorCode,
						BOOL bShowMessageBoxOnError,
						CString sHeader = _T(""),
						CString sFooter = _T(""));
extern void ShowLastError(	BOOL bShowMessageBoxOnError,
						   CString sHeader = _T(""),
						   CString sFooter = _T(""));

// Win95 Compatible timeSetEvent Function.
// Under Win95 timer events are not working!
extern MMRESULT timeSetEventCompatible(	UINT uDelay,                
										UINT uResolution,           
										LPTIMECALLBACK lpTimeProc,  
										DWORD dwUser,               
										UINT fuEvent);

// Just Play An Audio File
extern int MCIPlayFile(HWND hWndNotify, BOOL bStartPlaying, LPCTSTR lpszFileName);
extern MCIERROR MCIPlayDevice(HWND hWndNotify, int nDeviceID);
extern MCIERROR MCIPauseDevice(HWND hWndNotify, int nDeviceID);
extern MCIERROR MCICloseDevice(HWND hWndNotify, int nDeviceID);

// 16 bytes aligned new / delete
extern LPVOID new16align(unsigned int size); // size in bytes
extern void delete16align(LPVOID ptr);

// Get the Total Physical Memory in MB installed on the computer
// or available to the OS
extern int GetTotPhysMemMB(BOOL bInstalled);

// Get Memory Stats
extern void GetMemoryStats(	int* pRegions = NULL,
							int* pFreeMB = NULL,
							int* pReservedMB = NULL,
							int* pCommittedMB = NULL,
							double* pFragmentation = NULL);

// Disk Size and Available Space, pass "c:"
// or a path that has a drive letter in it
extern ULONGLONG GetDiskSize(LPCTSTR lpszPath);
extern ULONGLONG GetDiskSpace(LPCTSTR lpszPath);

// File Name to Mime
extern CString FileNameToMime(LPCTSTR lpszFileName);

// Line-Break Handling
extern void MakeLineBreakCR(CString& s);
extern void MakeLineBreakLF(CString& s);
extern void MakeLineBreakCRLF(CString& s);

// Is ANSI Convertible?
extern BOOL IsANSIConvertible(const CString& s);

// CString to ANSI Conversion
// The allocated buffer is always NULL terminated and has to be freed with delete []
// The function returns the number of written bytes
// (the terminating NULL char is not included in this returned bytes count)
extern int ToANSI(const CString& s, LPSTR* ppAnsi, BOOL* pbUsedDefaultChar = NULL);

// Url encode given string
// set bEncodeReserved to TRUE if encoding GET values
// set bEncodeReserved to FALSE if encoding an entire URL
extern CString UrlEncode(const CString& s, BOOL bEncodeReserved);

// Unicode (=UTF16) <-> UTF8 Conversion
extern CString FromUTF8(const unsigned char* pUtf8, int nUtf8Len);	// Note: pUtf8 must not be NULL terminated.
extern int ToUTF8(const CString& s, LPBYTE* ppUtf8);				// Allocates a NULL terminated buffer,
																	// returns the Utf-8 size (null termination not included!)

// Get uuid
extern CString GetUuidString();

// Does the given rectangle intersect a valid monitor?
extern BOOL IntersectsValidMonitor(LPCRECT lpRect);

#endif // !defined(AFX_HELPERS_H__8FD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_)