#if !defined(AFX_HELPERS_H__8FD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_)
#define AFX_HELPERS_H__8FD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_

#pragma once

#include "WinReg.h"
#include "WorkerThread.h"
#include "Round.h"

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
__forceinline int SystemDPIScale(int n) { return MulDiv(n, g_nSystemDPI, 96); };

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
extern ULARGE_INTEGER GetDirContentSize(LPCTSTR szDirName, 
										int* pFilesCount = NULL, // optional files count
										CWorkerThread* pThread = NULL);
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
CSIDL_DRIVES					My Computer virtual folder containing everything on the local computer: storage devices,
								printers, and Control Panel. The folder may also contain mapped network drives. 
CSIDL_FAVORITES					File system directory that serves as a common repository for the user's favorite items. 
CSIDL_FONTS						Virtual folder containing fonts. 
CSIDL_HISTORY					File system directory that serves as a common repository for Internet history items. 
CSIDL_INTERNET					Virtual folder representing the Internet. 
CSIDL_INTERNET_CACHE			File system directory that serves as a common repository for temporary Internet files. 
CSIDL_NETHOOD					File system directory containing objects that appear in the network neighborhood. 
CSIDL_NETWORK					Network Neighborhood virtual folder representing the top level of the network hierarchy. 
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
CSIDL_CDBURN_AREA				CD Burn area
*/
extern CString GetSpecialFolderPath(int nSpecialFolder);

// Kill process by PID
extern BOOL KillProcByPID(DWORD dwProcID);

// Enumerate or eventually kill process(es) by name
// returns the number of found processes
extern int EnumKillProcByName(const CString& sProcessName, BOOL bKill = FALSE);

// Execute an app with hidden window (working for both console and windowed apps)
extern BOOL ExecHiddenApp(	const CString& sFileName,
							const CString& sParams = _T(""),
							BOOL bWaitTillDone = FALSE,
							DWORD dwWaitMillisecondsTimeout = INFINITE);

// Ini file Functions
extern UINT GetProfileIniInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault, LPCTSTR lpszProfileName);
extern BOOL WriteProfileIniInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue, LPCTSTR lpszProfileName);
extern CString GetProfileIniString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault, LPCTSTR lpszProfileName); // it strips quotes
extern BOOL WriteProfileIniString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue, LPCTSTR lpszProfileName);

// Registry Functions
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

// Get the local NetBIOS computer name
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

// Disk total size and available free space.
// Pass a drive letter (like "c:"), a directory path
// (it has not to be the root dir) or a UNC path
extern ULONGLONG GetDiskTotalSize(LPCTSTR lpszPath);
extern ULONGLONG GetDiskAvailableFreeSpace(LPCTSTR lpszPath);

// Is it a positive numeric string (0, 1, 2, ...)?
__forceinline BOOL IsNumeric(const CString& s)
{
	if (s.IsEmpty())
		return FALSE;
	for (int i = 0 ; i < s.GetLength() ; i++)
	{
		if (!_istdigit(s[i])) // if not 0 – 9
			return FALSE;
	}
	return TRUE;
}

// Tiff check functions
__forceinline BOOL IsTIFFExt(CString sExt)
{
	sExt.TrimLeft(_T('.'));
	return ((sExt.CompareNoCase(_T("tif")) == 0)	||
			(sExt.CompareNoCase(_T("tiff")) == 0)	||
			(sExt.CompareNoCase(_T("jfx")) == 0)	||
			IsNumeric(sExt));	// some scan/fax programs save multi-page tiffs
								// using numeric extensions (.001, .002, ...)
								// which indicate the page count
}
__forceinline BOOL IsTIFF(const CString& sFileName)
{
	return IsTIFFExt(GetFileExt(sFileName));
}

// Jpeg check functions
__forceinline BOOL IsJPEGExt(CString sExt)
{
	sExt.TrimLeft(_T('.'));
	return ((sExt.CompareNoCase(_T("jpg")) == 0)	||
			(sExt.CompareNoCase(_T("jpeg")) == 0)	||
			(sExt.CompareNoCase(_T("jpe")) == 0)	||
			(sExt.CompareNoCase(_T("thm")) == 0));
}
__forceinline BOOL IsJPEG(const CString& sFileName)
{
	return IsJPEGExt(GetFileExt(sFileName));
}

// File Name to Mime
extern CString FileNameToMime(LPCTSTR lpszFileName);

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

// Natural order string comparision function
extern int __cdecl CompareNatural(const CString* pstr1, const CString* pstr2);

// Is the given string in the supplied array (case insensitive)?
extern BOOL InStringArray(const CString& s, const CStringArray& arr);

// On some systems GetCursorPos() fails for addresses > 2GB,
// better to use GetCursorInfo() which is called by this wrapper
// (this bug appears to be fixed in Windows 7 and higher)
extern BOOL GetSafeCursorPos(LPPOINT lpPoint);

// Does the given rectangle intersect a valid monitor?
extern BOOL IntersectsValidMonitor(LPCRECT lpRect);

// Scale a font size starting from a minimum reference
__forceinline int ScaleFont(int nWidth, int nHeight,
							int nMinRefFontSize,
							int nMinRefWidth, int nMinRefHeight)
{
	// Check
	if (nMinRefWidth <= 0 || nMinRefHeight <= 0)
		return nMinRefFontSize;

	// Scale
	double dFactorX = (double)nWidth / nMinRefWidth;
	if (dFactorX < 1.0)
		dFactorX = 1.0;
	double dFactorY = (double)nHeight / nMinRefHeight;
	if (dFactorY < 1.0)
		dFactorY = 1.0;
	if (dFactorX > dFactorY)
		return Round(nMinRefFontSize * dFactorX);
	else
		return Round(nMinRefFontSize * dFactorY);
}

// Checks whether the given font is available in the system
extern BOOL IsFontSupported(LPCTSTR szFontFamily);

// Draw big single line Arial text, if text doesn't fit in given rc width
// then a smaller font size is chosen (minimum used font size is 8 points)
// nMaxFontSize:	max. font size in points
// uAlign:			DT_TOP, DT_BOTTOM, DT_LEFT, DT_RIGHT, DT_CENTER, DT_VCENTER
// returns:			the used height in pixels (0 if an error occurred)
extern int DrawBigText(	HDC hDC,
						CRect rc,
						LPCTSTR szText,
						COLORREF crTextColor,
						int nMaxFontSize = 72,
						UINT uAlign = DT_CENTER | DT_VCENTER,
						int nBkMode = TRANSPARENT,
						COLORREF crBkColor = RGB(0,0,0));

// Calculates the shrink size keeping the aspect ratio
// supply a maximum size in pixels or percent
// returns TRUE if shrinking is necessary
extern BOOL CalcShrink(	DWORD dwOrigWidth,
						DWORD dwOrigHeight,
						DWORD dwMaxSize,
						BOOL bMaxSizePercent,
						DWORD& dwShrinkWidth,
						DWORD& dwShrinkHeight);

// For vertical sliders
class CSliderCtrl;
extern int GetRevertedPos(CSliderCtrl* pSliderCtrl);
extern void SetRevertedPos(CSliderCtrl* pSliderCtrl, int nPos);

#endif // !defined(AFX_HELPERS_H__8FD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_)