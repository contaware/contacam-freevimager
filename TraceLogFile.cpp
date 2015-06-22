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

CString g_sTraceFileName;
CString g_sLogFileName;
volatile ULONGLONG g_ullMaxTraceFileSize = 0;
volatile ULONGLONG g_ullMaxLogFileSize = 0;
#ifdef _DEBUG
CRITICAL_SECTION g_csTraceDebug;
CString g_sTraceDebugFileAndLine;
#endif
CRITICAL_SECTION g_csTraceFile;
CRITICAL_SECTION g_csLogFile;
volatile BOOL g_bTraceLogFileInited = FALSE;

void InitTraceLogFile(LPCTSTR szTraceFileName,
					  LPCTSTR szLogFileName,
					  ULONGLONG ullMaxTraceFileSize/*=0*/,
					  ULONGLONG ullMaxLogFileSize/*=0*/)
{
	if (!g_bTraceLogFileInited)
	{
#ifdef _DEBUG
		InitializeCriticalSection(&g_csTraceDebug);
#endif
		InitializeCriticalSection(&g_csTraceFile);
		InitializeCriticalSection(&g_csLogFile);
		g_ullMaxTraceFileSize = ullMaxTraceFileSize;
		g_ullMaxLogFileSize = ullMaxLogFileSize;
		g_sTraceFileName = szTraceFileName;
		g_sLogFileName = szLogFileName;
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
#ifdef _DEBUG
		DeleteCriticalSection(&g_csTraceDebug);
#endif
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

	// Format
	CString s;
	va_list arguments;
	va_start(arguments, pFormat);	
	s.FormatV(pFormat, arguments);
    va_end(arguments);

	// Make single line with no line ending
	s = SingleLine(s);
	
	// Trace (current time is added by TRACE)
	TRACE(_T("%s\n"), s);

	// Add current time
	time_t CurrentTime;
	time(&CurrentTime);
	CString sCurrentTime(_tctime(&CurrentTime));
	sCurrentTime.TrimRight(_T('\n'));
	s = _T("[") + sCurrentTime + _T("] ") + s;

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

	// Create directory
	CString sPath = GetDriveAndDirName(g_sLogFileName);
	if (!IsExistingDir(sPath))
		CreateDir(sPath);

	// Log To File
	s += _T("\r\n"); // add \r\n line ending
	FILE* pf = _tfopen(g_sLogFileName, _T("ab"));
	if (pf)
	{
		LPBYTE pData = NULL;
		int nSize = ToUTF8(s, &pData);
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

void TraceFile(const TCHAR* pFormat, ...)
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

	// Add current time
	time_t CurrentTime;
	time(&CurrentTime);
	CString sCurrentTime(_tctime(&CurrentTime));
	sCurrentTime.TrimRight(_T('\n'));
	s = _T("[") + sCurrentTime + _T("] ") + s;

	// Check file size
	if (g_ullMaxTraceFileSize > 0)
	{
		ULARGE_INTEGER Size = GetFileSize64(g_sTraceFileName);
		if (Size.QuadPart > g_ullMaxTraceFileSize)
		{
			CString sOldFileName = GetFileNameNoExt(g_sTraceFileName) + _T(".old");
			DeleteFile(sOldFileName);
			MoveFile(g_sTraceFileName, sOldFileName);
		}
	}

	// Create directory
	CString sPath = GetDriveAndDirName(g_sTraceFileName);
	if (!IsExistingDir(sPath))
		CreateDir(sPath);

	// Trace To File
	s.Replace(_T("\r\n"), _T("\n")); // make sure we do not have already \r\n endings
	s.Replace(_T("\n"), _T("\r\n")); // convert to \r\n line endings
	FILE* pf = _tfopen(g_sTraceFileName, _T("ab"));
	if (pf)
	{
		LPBYTE pData = NULL;
		int nSize = ToUTF8(s, &pData);
		if (pData)
		{
			ULARGE_INTEGER Size = GetFileSize64(g_sTraceFileName);
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
	LeaveCriticalSection(&g_csTraceFile);
}

#ifdef _DEBUG
void TraceDebugEnterCS(CString sFileName, int nLine)
{
	// Check
	if (!g_bTraceLogFileInited)
		return;

	// Enter CS
	EnterCriticalSection(&g_csTraceDebug);

	// Format
	g_sTraceDebugFileAndLine.Format(_T("%s(%i)"), sFileName, nLine);
}

void TraceDebugLeaveCS(const TCHAR* pFormat, ...)
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

	// Add current time
	time_t CurrentTime;
	time(&CurrentTime);
	CString sCurrentTime(_tctime(&CurrentTime));
	sCurrentTime.TrimRight(_T('\n'));
	s = _T("[") + sCurrentTime + _T("] ") + s;

	// Add file and line names
	CString sWithFileAndLine;
	sWithFileAndLine.Format(_T("%-") TRACEDEBUG_CHARS_INDENT _T("s : %s"), g_sTraceDebugFileAndLine, s);

	// Trace
	afxDump << sWithFileAndLine;

	// Leave CS
	LeaveCriticalSection(&g_csTraceDebug);
}
#endif