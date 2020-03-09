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
extern CRITICAL_SECTION g_csLogFile;
extern volatile BOOL g_bTraceLogFileInited;

extern void InitTraceLogFile(	LPCTSTR szLogFileName,
								ULONGLONG ullMaxLogFileSize = 0);	// 0 means unlimited
extern void EndTraceLogFile();
extern CString SingleLine(CString s);
extern void LogLine(const TCHAR* pFormat, ...);

#endif // !defined(AFX_TRACELOGFILE_H__0A6BEBCA_829C_4085_8A12_19A15A12F62C__INCLUDED_)
