#include "TraceLogFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern BOOL CreateDir(LPCTSTR szNewDir);
extern BOOL IsExistingDir(LPCTSTR lpszFileName);
extern CString GetDriveAndDirName(const CString& sFullFilePath);
extern CString GetFileNameNoExt(const CString& sFullFilePath);
extern ULARGE_INTEGER GetFileSize64(LPCTSTR lpszFileName);
extern int ToUTF8(const CString& s, LPBYTE* ppUtf8);

TCHAR g_sTraceFileName[MAX_PATH] = _T("");
TCHAR g_sLogFileName[MAX_PATH] = _T("");
volatile ULONGLONG g_ullMaxLogFileSize = 0;
CRITICAL_SECTION g_csTraceFile;
CRITICAL_SECTION g_csLogFile;
volatile BOOL g_bTraceLogFileInited = FALSE;

void InitTraceLogFile(LPCTSTR szTraceFileName,
					  LPCTSTR szLogFileName,
					  ULONGLONG ullMaxLogFileSize/*=0*/)
{
	if (!g_bTraceLogFileInited)
	{
		InitializeCriticalSection(&g_csTraceFile);
		InitializeCriticalSection(&g_csLogFile);
		g_ullMaxLogFileSize = ullMaxLogFileSize;
		_tcsncpy(g_sTraceFileName, szTraceFileName, MAX_PATH);
		g_sTraceFileName[MAX_PATH - 1] = _T('\0');
		_tcsncpy(g_sLogFileName, szLogFileName, MAX_PATH);
		g_sLogFileName[MAX_PATH - 1] = _T('\0');
		g_bTraceLogFileInited = TRUE;
	}
}

void EndTraceLogFile()
{
	if (g_bTraceLogFileInited)
	{
		g_bTraceLogFileInited = FALSE;
		DeleteCriticalSection(&g_csLogFile);
		DeleteCriticalSection(&g_csTraceFile);
	}
}

CString SingleLine(CString s)
{
	s.Replace(_T("\r\n\r\n"), _T(" "));
	s.Replace(_T("\r\r"), _T(" "));
	s.Replace(_T("\n\n"), _T(" "));
	s.Replace(_T("\r\n"), _T(" "));
	s.Replace(_T('\r'), _T(' '));
	s.Replace(_T('\n'), _T(' '));
	s.TrimLeft();
	s.TrimRight();
	return s;
}

void LogLine(const TCHAR* pFormat, ...)
{
	// Check
	if (!g_bTraceLogFileInited)
		return;

	// Enter CS
	EnterCriticalSection(&g_csLogFile);

	// Check file size
	if (g_ullMaxLogFileSize > 0)
	{
		ULARGE_INTEGER Size = GetFileSize64(g_sLogFileName);
		if (Size.QuadPart > g_ullMaxLogFileSize)
		{
			CString sOldFileName = GetFileNameNoExt(g_sLogFileName) + _T(".old");
			DeleteFile(sOldFileName);
			MoveFile(g_sLogFileName, sOldFileName);
		}
	}

	// Format
	CString s;
	va_list arguments;
	va_start(arguments, pFormat);	
	s.FormatV(pFormat, arguments);
    va_end(arguments);

	// Single line
	s = SingleLine(s);
	
	// Current Time
	time_t CurrentTime;
	time(&CurrentTime);
	CString sCurrentTime(_tctime(&CurrentTime));
	sCurrentTime.Replace(_T('\n'), _T(' '));

	// Trace
#ifdef _DEBUG
	AfxTrace(_T("%s: %s\n"), sCurrentTime, s);
#endif

	// Create directory
	CString sPath = GetDriveAndDirName(g_sLogFileName);
	if (!IsExistingDir(sPath))
		CreateDir(sPath);

	// Log To File
	FILE* pf = _tfopen(g_sLogFileName, _T("ab"));
	if (pf)
	{
		LPBYTE pData = NULL;
		int nSize = ToUTF8(sCurrentTime + _T(": ") + s + _T("\r\n"), &pData);
		if (pData)
		{
			ULARGE_INTEGER Size = GetFileSize64(g_sLogFileName);
			if (Size.QuadPart == 0)
			{
				const BYTE BOM[3] = {0xEF, 0xBB, 0xBF};
				fwrite(BOM, sizeof(BYTE), 3, pf);
			}
			fwrite(pData, sizeof(BYTE), nSize, pf);
			delete [] pData;
		}
		fclose(pf);
	}

	// Leave CS
	LeaveCriticalSection(&g_csLogFile);
}

void TraceFileEnterCS(const TCHAR* pFormat, ...)
{
	// Check
	if (!g_bTraceLogFileInited)
		return;

	// Enter CS
	EnterCriticalSection(&g_csTraceFile);

	// Format
	CString s;
	va_list arguments;
	va_start(arguments, pFormat);	
	s.FormatV(pFormat, arguments);
    va_end(arguments);

	// Trace
#ifdef _DEBUG
	AfxTrace(_T("%s"), s);
#endif

	// Create directory
	CString sPath = GetDriveAndDirName(g_sTraceFileName);
	if (!IsExistingDir(sPath))
		CreateDir(sPath);

	// Log To File
	FILE* pf = _tfopen(g_sTraceFileName, _T("at"));
	if (pf)
	{
		_ftprintf(pf, _T("%s"), (LPCTSTR)s);
		fclose(pf);
	}
}

void TraceFileLeaveCS(const TCHAR* pFormat, ...)
{
	// Check
	if (!g_bTraceLogFileInited)
		return;

	// Format
	CString s;
	va_list arguments;
	va_start(arguments, pFormat);	
	s.FormatV(pFormat, arguments);
    va_end(arguments);

	// Trace
#ifdef _DEBUG
	AfxTrace(_T("%s"), s);
#endif

	// Create directory
	CString sPath = GetDriveAndDirName(g_sTraceFileName);
	if (!IsExistingDir(sPath))
		CreateDir(sPath);

	// Log To File
	FILE* pf = _tfopen(g_sTraceFileName, _T("at"));
	if (pf)
	{
		_ftprintf(pf, _T("%s"), (LPCTSTR)s);
		fclose(pf);
	}

	// Leave CS
	LeaveCriticalSection(&g_csTraceFile);
}