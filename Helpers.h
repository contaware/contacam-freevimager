#if !defined(AFX_HELPERS_H__8FD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_)
#define AFX_HELPERS_H__8FD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_

#pragma once

#include "WinReg.h"

#ifndef MAX
#  define MAX(a,b)  ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#  define MIN(a,b)  ((a) < (b) ? (a) : (b))
#endif

// Windows thumbnail cache file
#define THUMBS_DB				_T("Thumbs.db")

extern int g_nSystemDPI;
extern TCHAR g_szDefaultFontFace[LF_FACESIZE];
extern BOOL g_bMMX;
extern BOOL g_bSSE;
extern BOOL g_bSSE2;
extern BOOL g_b3DNOW;
extern DWORD g_dwAllocationGranularity;
extern int g_nPCInstalledPhysRamMB;
extern int g_nOSUsablePhysRamMB;
extern int g_nAppUsableAddressSpaceMB;

// Call This Before Using the following Functions!
extern void InitHelpers();

// System DPI Scale
extern int SystemDPIScale(int n);

/*
c:\mydir1\mydir2\hello.jpeg
---------------------------
GetDriveName()			-> c:
GetDirName()			-> \mydir1\mydir2\
GetDriveAndDirName()	-> c:\mydir1\mydir2\
GetShortFileName()		-> hello.jpeg
GetShortFileNameNoExt()	-> hello
GetFileNameNoExt()		-> c:\mydir1\mydir2\hello
GetFileExt()			-> .jpeg

c:\mydir1\mydir2\hello
----------------------
GetDriveName()			-> c:
GetDirName()			-> \mydir1\mydir2\
GetDriveAndDirName()	-> c:\mydir1\mydir2\
GetShortFileName()		-> hello
GetShortFileNameNoExt()	-> hello
GetFileNameNoExt()		-> c:\mydir1\mydir2\hello
GetFileExt()			-> empty string!

c:\mydir1\mydir2\hello\
-----------------------
GetDriveName()			-> c:
GetDirName()			-> \mydir1\mydir2\hello\
GetDriveAndDirName()	-> c:\mydir1\mydir2\hello\
GetShortFileName()		-> empty string!
GetShortFileNameNoExt()	-> empty string!
GetFileNameNoExt()		-> c:\mydir1\mydir2\hello\
GetFileExt()			-> empty string!

\\?\c:\mydir1\mydir2\hello.jpeg
-------------------------------
GetDriveName()			-> empty string!
GetDirName()			-> \\?\c:\mydir1\mydir2\
GetDriveAndDirName()	-> \\?\c:\mydir1\mydir2\
GetShortFileName()		-> hello.jpeg
GetShortFileNameNoExt()	-> hello
GetFileNameNoExt()		-> \\?\c:\mydir1\mydir2\hello
GetFileExt()			-> .jpeg

\\TS109\Public\ContaCam\hello.jpeg
----------------------------------
GetDriveName()			-> empty string!
GetDirName()			-> \\TS109\Public\ContaCam\
GetDriveAndDirName()	-> \\TS109\Public\ContaCam\
GetShortFileName()		-> hello.jpeg
GetShortFileNameNoExt()	-> hello
GetFileNameNoExt()		-> \\TS109\Public\ContaCam\hello
GetFileExt()			-> .jpeg

\\?\UNC\TS109\Public\ContaCam\hello.jpeg
----------------------------------------
GetDriveName()			-> empty string!
GetDirName()			-> \\?\UNC\TS109\Public\ContaCam\
GetDriveAndDirName()	-> \\?\UNC\TS109\Public\ContaCam\
GetShortFileName()		-> hello.jpeg
GetShortFileNameNoExt()	-> hello
GetFileNameNoExt()		-> \\?\UNC\TS109\Public\ContaCam\hello
GetFileExt()			-> .jpeg
*/
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

// If it's a valid drive mount path convert it to a UNC path,
// otherwise return the given sPath unchanged
extern CString UNCPath(const CString& sPath);

// File / directory operations
extern BOOL TakeOwnership(LPCTSTR lpszFile);
extern BOOL SetPrivilege(HANDLE hToken, LPCTSTR lpszPrivilege, BOOL bChange);
extern BOOL SetPermission(LPCTSTR lpszFile, LPCTSTR lpszAccess, DWORD dwAccessMask);
extern BOOL CreateDir(LPCTSTR szNewDir);
extern BOOL CopyDirContent(	LPCTSTR szFromDir,
							LPCTSTR szToDir,
							BOOL bOverwriteIfExists = TRUE,
							BOOL bContinueOnCopyError = TRUE,
							int* pFilesCount = NULL);			// optional files count
extern BOOL MergeDirContent(LPCTSTR szFromDir,
							LPCTSTR szToDir,
							BOOL bOverwriteIfExists = TRUE,
							BOOL bContinueOnCopyError = TRUE,
							int* pFilesCount = NULL);			// optional files count
extern BOOL DeleteDir(LPCTSTR szDirName);
extern BOOL DeleteDirContent(LPCTSTR szDirName);
extern BOOL DeleteToRecycleBin(LPCTSTR szName); // delete file or directory

// Format Integer Number
extern CString FormatIntegerNumber(const CString& sNumber);

// Format as GB, MB, KB or Bytes
extern CString FormatBytes(ULONGLONG ullBytes);

// Date / Time Handling
extern int GetLastDayOfMonth(int nMonth, int nYear);
extern CString MakeTimeLocalFormat(	const CTime& Time,
									BOOL bShowSeconds = FALSE);
extern CString MakeDateLocalFormat(	const CTime& Time,
									BOOL bLongDate = FALSE);
extern CString GetDateLocalFormat(BOOL bLongDate = FALSE);
extern CTime ParseShortDateLocalFormat(CString sDate);

// Make a Unique Temp File Name (file is not created)
extern CString MakeTempFileName(CString sTempPath, LPCTSTR lpszFileName);

// File / Dir Existence Check
extern BOOL IsExistingFile(LPCTSTR lpszFileName);
extern BOOL IsExistingDir(LPCTSTR lpszFileName);

// Is sSubDir a nested subfolder of sDir?
extern BOOL IsSubDir(CString sDir, CString sSubDir);

// Path comparision
extern BOOL AreSamePath(const CString& sPath1, const CString& sPath2);

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
CSIDL_DRIVES					My Computer virtual folder containing everything on the local computer: storage devices,
								printers, and Control Panel. The folder may also contain mapped network drives. 
CSIDL_FAVORITES					File system directory that serves as a common repository for the user's favorite items. 
CSIDL_FONTS						Virtual folder containing fonts. 
CSIDL_HISTORY					File system directory that serves as a common repository for Internet history items. 
CSIDL_INTERNET					Virtual folder representing the Internet. 
CSIDL_INTERNET_CACHE			File system directory that serves as a common repository for temporary Internet files. 
CSIDL_NETHOOD					File system directory containing objects that appear in the network neighborhood. 
CSIDL_NETWORK					Network Neighborhood virtual folder representing the top level of the network hierarchy. 
CSIDL_PRINTERS					Virtual folder containing installed printers. 
CSIDL_PRINTHOOD					File system directory that serves as a common repository for printer links. 
CSIDL_PROGRAMS					File system directory that contains the user's program groups (which are also file system directories). 
CSIDL_RECENT					File system directory that contains the user's most recently used documents. 
CSIDL_SENDTO					File system directory that contains Send To menu items. 
CSIDL_STARTMENU					File system directory containing Start menu items. 
CSIDL_STARTUP					File system directory that corresponds to the user's Startup program group.
								The system starts these programs whenever any user logs onto Windows NT or starts Windows 95. 
CSIDL_TEMPLATES					File system directory that serves as a common repository for document templates.
CSIDL_MYDOCUMENTS				My Documents (same as CSIDL_PERSONAL).
CSIDL_MYVIDEO					My Videos folder
CSIDL_MYMUSIC					My Music folder
CSIDL_MYPICTURES				My Pictures folder
CSIDL_CDBURN_AREA				CD Burn area
*/
extern CString GetSpecialFolderPath(int nSpecialFolder);

// Kill process by PID
// 
// The process ID is a value associated with the process object,
// and as long as the process object is still around, so too will
// its process ID. The process object remains as long as the process
// is still running or as long as somebody still has a handle to the
// process object. This makes sense, because as long as there is still
// a handle to the process, which can already be terminated, somebody
// can call WaitForSingleObject or GetExitCodeProcess or even
// OpenProcess to get another handle.
extern BOOL KillProcByPID(DWORD dwProcID);
extern BOOL KillProcTreeByPID(DWORD dwProcID);

// Enumerate or eventually kill process(es) by name
// returns the number of found processes
extern int EnumKillProcByName(const CString& sProcessName, BOOL bKill = FALSE);

// ShellExecuteEx() given sFileName
// Attention: always CloseHandle() the returned process handle when != NULL
extern HANDLE ExecApp(	const CString& sFileName,
						const CString& sParams = _T(""),			// parameters
						const CString& sStartDirectory = _T(""),	// application start directory, if not provided program's directory is used
						BOOL bShow = TRUE,							// show application window?
						BOOL bElevated = FALSE,						// run the given app with elevated privileges?
						BOOL bWaitTillDone = FALSE,					// do not return until done?
						DWORD dwWaitMillisecondsTimeout = INFINITE);// timeout in ms for the above wait
extern void KillApp(HANDLE& hProcess);								// after killing the app, the function closes and sets hProcess to NULL

// Ini file Functions
extern UINT GetProfileIniInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault, LPCTSTR lpszProfileName);
extern BOOL WriteProfileIniInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue, LPCTSTR lpszProfileName);
extern CString GetProfileIniString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault, LPCTSTR lpszProfileName); // it strips quotes
extern BOOL WriteProfileIniString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue, LPCTSTR lpszProfileName);

// Registry Functions
// samOptional:
// KEY_WOW64_64KEY to access a 64-bit key from either a 32-bit or 64-bit application
// KEY_WOW64_32KEY to access a 32-bit key from either a 32-bit or 64-bit application
// https://support.microsoft.com/en-us/help/305097/how-to-view-the-system-registry-by-using-64-bit-versions-of-windows
extern BOOL IsRegistryValue(HKEY hOpenKey, LPCTSTR szKey, LPCTSTR szValue, REGSAM samOptional = 0);
extern BOOL IsRegistryKey(HKEY hOpenKey, LPCTSTR szKey, REGSAM samOptional = 0);
extern BOOL SetRegistryEmptyValue(HKEY hOpenKey, LPCTSTR szKey, LPCTSTR szValue);
extern BOOL SetRegistryStringValue(HKEY hOpenKey, LPCTSTR szKey, LPCTSTR szValue, LPCTSTR szData);
extern CString GetRegistryStringValue(HKEY hOpenKey, LPCTSTR szKey, LPCTSTR szValue, REGSAM samOptional = 0);
extern BOOL DeleteRegistryValue(HKEY hOpenKey, LPCTSTR szKey, LPCTSTR szValue);
extern BOOL DeleteRegistryKey(HKEY hOpenKey, LPCTSTR szKey);

// LogLine + eventually show MessageBox, format:
// sHeader + GetLastError() in readable format + sFooter
extern CString ShowErrorMsg(DWORD dwErrorCode,
							BOOL bShowMessageBoxOnError,
							CString sHeader = _T(""),
							CString sFooter = _T(""));

// Aler user with a beep and a flashing window
extern void AlertUser(HWND hWnd);

// Get the current user name
extern CString GetUserName();

/*
Get the local NetBIOS computer name
Minimum length 	1
Maximum length 	15
Disallowed characters (like for files/folders):
\ / : * ? " < > |
*/
extern CString GetComputerName();

// Get Memory Stats
extern void GetMemoryStats(	ULONGLONG* pRegions = NULL,
							ULONGLONG* pFree = NULL,
							ULONGLONG* pReserved = NULL,
							ULONGLONG* pCommitted = NULL,
							ULONGLONG* pMaxFree = NULL,
							ULONGLONG* pMaxReserved = NULL,
							ULONGLONG* pMaxCommitted = NULL,
							double* pFragmentation = NULL);

// Disk size functions
// Pass a drive letter (like "c:"), a directory path
// (must not necessarily be the root dir) or a UNC path
extern ULONGLONG GetDiskTotalSize(LPCTSTR lpszPath);
extern ULONGLONG GetDiskAvailableFreeSpace(LPCTSTR lpszPath);
extern ULONGLONG GetDiskUsedSpace(LPCTSTR lpszPath);

// Is it a positive numeric string (0, 1, 2, ...)?
extern BOOL IsNumeric(const CString& s);

// Line-Break Handling
extern void MakeLineBreakCR(CString& s);
extern void MakeLineBreakLF(CString& s);
extern void MakeLineBreakCRLF(CString& s);

// Is Valid File Name, checks whether s contains  \ / : * ? " < > |
extern BOOL IsValidFileName(const CString& s, BOOL bShowMessageBoxOnError = FALSE);

// Is ANSI Convertible?
extern BOOL IsANSIConvertible(const CString& s);

// CString to ANSI conversion
// Allocates a NULL terminated buffer which has to be freed by delete []
// returns the ANSI string size in bytes, NULL termination not included
// (for the empty string a one byte buffer containing a single '\0'
// is allocated)
extern int ToANSI(const CString& s, LPSTR* ppAnsi);

// Short <-> Long path names convertion
extern CString GetLongPathName(const CString& sShortPath);
extern CString GetShortPathName(const CString& sLongPath);

// Get ASCII compatible path, file or directory must exist!
extern BOOL IsASCIICompatiblePath(const CString& sPath);
extern CString GetASCIICompatiblePath(const CString& sPath);

// Url encode given string according to RFC 3986
// set bEncodeReserved to TRUE if encoding GET parameter values
// set bEncodeReserved to FALSE if encoding an entire URL
extern CString UrlEncode(const CString& s, BOOL bEncodeReserved);

// Url decode given string (only decode percent encoded values, the + char is NOT decoded as space)
extern CString UrlDecode(const CString& s);

// Html encode given string
extern CString HtmlEncode(CString s);

// Html decode given string
extern CString HtmlDecode(CString s);

// UTF8 -> UTF16 conversion
// pUtf8 must not be NULL terminated (it doesn't harm if it is)
// nUtf8Len is the size in bytes (optional NULL termination should
// not be included in this size, but it doesn't harm if it is)
extern CString FromUTF8(const unsigned char* pUtf8, int nUtf8Len);

// CString to UTF8 conversion
// Allocates a NULL terminated buffer which has to be freed by delete []
// returns the UTF8 string size in bytes, NULL termination not included
// (for the empty string a one byte buffer containing a single '\0'
// is allocated)
extern int ToUTF8(const CString& s, LPBYTE* ppUtf8);

/*
GUID is Microsoft's UUID, GUID and UUID differ only when exchanged
in binary form between machines with different endianness:

Bytes Name   Endianness  Endianness
             GUID        RFC 4122
----- -----  ----------  ----------
4     Data1  Native      Big
2     Data2  Native      Big
2     Data3  Native      Big
8     Data4  Big         Big

Note: 'Native' is Little Endian on Microsoft platforms
*/
extern CString UuidToCString(const UUID* pUuid); // uuid/guid struct to CString
extern CString GetUuidCString(); // generate a uuid/guid CString

// On some systems GetCursorPos() fails for addresses > 2GB,
// better to use GetCursorInfo() which is called by this wrapper
// (this bug appears to be fixed in Windows 7 and higher)
extern BOOL GetSafeCursorPos(LPPOINT lpPoint);

// Does the given rectangle intersect a valid monitor?
extern BOOL IntersectsValidMonitor(LPCRECT lpRect);

// Checks whether the given font is available in the system
extern BOOL IsFontSupported(LPCTSTR szFontFamily);

// Download given http(s) URL to memory or file
typedef void(CALLBACK* URLDOWNLOADPROGRESSCALLBACK)(int,size_t);	// int nStatus: -1 = error, 0 = downloading, 1 = done
																	// size_t Size: downloaded size in bytes
extern LPBYTE GetURL(LPCTSTR lpszURL, size_t& Size,					// remember to free() the returned buffer
					BOOL bAllowInvalidCert,
					BOOL bShowMessageBoxOnError,
					URLDOWNLOADPROGRESSCALLBACK lpfnCallback);
extern BOOL SaveURL(LPCTSTR lpszURL, LPCTSTR lpszFileName,			// lpszFileName must not exist, otherwise it fails
					BOOL bAllowInvalidCert,
					BOOL bShowMessageBoxOnError,
					URLDOWNLOADPROGRESSCALLBACK lpfnCallback);

#endif // !defined(AFX_HELPERS_H__8FD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_)