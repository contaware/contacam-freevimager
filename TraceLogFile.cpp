#include "stdafx.h"
#include "TraceLogFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

volatile int g_nLogLevel = 0;
CString g_sLogFileName;
volatile ULONGLONG g_ullMaxLogFileSize = 0;
CRITICAL_SECTION g_csLogFile;
volatile BOOL g_bTraceLogFileInited = FALSE;

void InitTraceLogFile(LPCTSTR szLogFileName,
					  ULONGLONG ullMaxLogFileSize/*=0*/)
{
	if (!g_bTraceLogFileInited)
	{
		InitializeCriticalSection(&g_csLogFile);
		g_ullMaxLogFileSize = ullMaxLogFileSize;
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

	// Add current time
	time_t CurrentTime;
	time(&CurrentTime);
	CString sCurrentTime(_tctime(&CurrentTime));
	sCurrentTime.TrimRight(_T('\n'));
	s = _T("[") + sCurrentTime + _T("] ") + s;

	// Trace
	TRACE(_T("%s\n"), s);

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
