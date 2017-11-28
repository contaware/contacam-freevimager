//
// TraceLogFile.h
//

#include <afx.h>

#if !defined(AFX_TRACELOGFILE_H__0A6BEBCA_829C_4085_8A12_19A15A12F62C__INCLUDED_)
#define AFX_TRACELOGFILE_H__0A6BEBCA_829C_4085_8A12_19A15A12F62C__INCLUDED_

// Log level
// 0: normal
// 1: verbose
// 2: debug
extern volatile int g_nLogLevel;
extern CString g_sLogFileName;
extern volatile ULONGLONG g_ullMaxLogFileSize;
#ifdef _DEBUG
#define TRACEDEBUG_CHARS_INDENT	_T("60")
extern CRITICAL_SECTION g_csTraceDebug;
extern CString g_sTraceDebugFileAndLine;
#endif
extern CRITICAL_SECTION g_csLogFile;
extern volatile BOOL g_bTraceLogFileInited;

extern void InitTraceLogFile(	LPCTSTR szLogFileName,
								ULONGLONG ullMaxLogFileSize = 0);	// 0 means unlimited
extern void EndTraceLogFile();
extern CString SingleLine(CString s);
extern void LogLine(const TCHAR* pFormat, ...);
#ifdef _DEBUG
extern void TraceDebugEnterCS(CString sFileName, int nLine);
extern void TraceDebugLeaveCS(const TCHAR* pFormat, ...);
#endif

#endif // !defined(AFX_TRACELOGFILE_H__0A6BEBCA_829C_4085_8A12_19A15A12F62C__INCLUDED_)

#ifdef _DEBUG
	#undef TRACE
	#define TRACE ::TraceDebugEnterCS(CString(__FILE__),__LINE__),::TraceDebugLeaveCS
#endif