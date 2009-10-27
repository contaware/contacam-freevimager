#include "TraceLogFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

TCHAR g_sTraceFileName[MAX_PATH] = _T("");
TCHAR g_sLogFileName[MAX_PATH] = _T("");
ULONGLONG g_ullMaxLogFileSize = 0;
CRITICAL_SECTION g_csTraceFile;
CRITICAL_SECTION g_csLogFile;
BOOL g_bTraceLogFileCSInited = FALSE;

void InitTraceLogFile(LPCTSTR szTraceFileName,
					  LPCTSTR szLogFileName,
					  ULONGLONG ullMaxLogFileSize/*=0*/)
{
	InitializeCriticalSection(&g_csTraceFile);
	InitializeCriticalSection(&g_csLogFile);
	g_ullMaxLogFileSize = ullMaxLogFileSize;
	_tcsncpy(g_sTraceFileName, szTraceFileName, MAX_PATH);
	g_sTraceFileName[MAX_PATH - 1] = _T('\0');
	_tcsncpy(g_sLogFileName, szLogFileName, MAX_PATH);
	g_sLogFileName[MAX_PATH - 1] = _T('\0');
	g_bTraceLogFileCSInited = TRUE;
}

void EndTraceLogFile()
{
	if (g_bTraceLogFileCSInited)
	{
		g_bTraceLogFileCSInited = FALSE;
		DeleteCriticalSection(&g_csLogFile);
		DeleteCriticalSection(&g_csTraceFile);
	}
}

void LogLine(const TCHAR* pFormat, ...)
{
	// Enter CS
	EnterCriticalSection(&g_csLogFile);

	// Check file size
	if (g_ullMaxLogFileSize > 0)
	{
		ULARGE_INTEGER Size;
		Size.QuadPart = 0;
		WIN32_FIND_DATA fileinfo;
		memset(&fileinfo, 0, sizeof(WIN32_FIND_DATA));
		HANDLE hFind = FindFirstFile(g_sLogFileName, &fileinfo);
		Size.LowPart = fileinfo.nFileSizeLow;
		Size.HighPart = fileinfo.nFileSizeHigh;
		FindClose(hFind);
		if (Size.QuadPart > g_ullMaxLogFileSize)
		{
			TCHAR szDrive[_MAX_DRIVE];
			TCHAR szDir[_MAX_DIR];
			TCHAR szName[_MAX_FNAME];
			_tsplitpath(g_sLogFileName, szDrive, szDir, szName, NULL);
			CString sOldFileName = CString(szDrive) + CString(szDir) + CString(szName) + _T(".old");
			::DeleteFile(sOldFileName);
			::MoveFile(g_sLogFileName, sOldFileName);
		}
	}

	// Arguments
	CString s;
	va_list arguments;
	va_start(arguments, pFormat);	
	s.FormatV(pFormat, arguments);
    va_end(arguments);
	s.Replace(_T("\r\n"), _T(" "));
	s.Replace(_T('\r'), _T(' '));
	s.Replace(_T('\n'), _T(' '));
	s.TrimLeft();
	s.TrimRight();
	s += _T('\n');

	// Current Time
	time_t CurrentTime;
	time(&CurrentTime);
	CString sCurrentTime(_tctime(&CurrentTime));
	sCurrentTime.Replace(_T('\n'), _T(' '));

	// Log To File
	FILE* pf = _tfopen(g_sLogFileName, _T("at"));
	if (pf)
	{
		_ftprintf(pf, _T("%s: %s"), (LPCTSTR)sCurrentTime, (LPCTSTR)s);
		fclose(pf);
	}

	// Leave CS
	LeaveCriticalSection(&g_csLogFile);
}

void TraceFile(const TCHAR* pFormat, ...)
{
	CString s;
	va_list arguments;
	va_start(arguments, pFormat);	
	s.FormatV(pFormat, arguments);
    va_end(arguments);

	// Trace
	AfxTrace(s);

	// Log To File
	FILE* pf = _tfopen(g_sTraceFileName, _T("at"));
	if (pf)
	{
		_ftprintf(pf, _T("%s"), (LPCTSTR)s);
		fclose(pf);
	}
}

void TraceFileCS(const TCHAR* pFormat, ...)
{
	CString s;
	va_list arguments;
	va_start(arguments, pFormat);	
	s.FormatV(pFormat, arguments);
    va_end(arguments);

	// Trace
	AfxTrace(s);

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