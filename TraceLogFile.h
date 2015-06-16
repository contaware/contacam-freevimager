//
// TraceLogFile.h
//

#include <afx.h>

#if !defined(AFX_TRACELOGFILE_H__0A6BEBCA_829C_4085_8A12_19A15A12F62C__INCLUDED_)
#define AFX_TRACELOGFILE_H__0A6BEBCA_829C_4085_8A12_19A15A12F62C__INCLUDED_

extern TCHAR g_sTraceFileName[MAX_PATH];
extern TCHAR g_sLogFileName[MAX_PATH];
extern volatile ULONGLONG g_ullMaxLogFileSize;
#ifdef _DEBUG
extern CRITICAL_SECTION g_csTraceDebug;
extern CString g_sTraceDebugFileAndLine;
#endif
extern CRITICAL_SECTION g_csTraceFile;
extern CRITICAL_SECTION g_csLogFile;
extern volatile BOOL g_bTraceLogFileInited;

extern void InitTraceLogFile(	LPCTSTR szTraceFileName,
								LPCTSTR szLogFileName,
								ULONGLONG ullMaxLogFileSize = 0); // 0 means unlimited
extern void EndTraceLogFile();
extern CString SingleLine(CString s);
extern void LogLine(const TCHAR* pFormat, ...);
extern void TraceFileEnterCS(const TCHAR* pFormat, ...);
extern void TraceFileLeaveCS(const TCHAR* pFormat, ...);
#ifdef _DEBUG
extern void TraceDebugEnterCS(CString sFileName, int nLine);
extern void TraceDebugLeaveCS(const TCHAR* pFormat, ...);
#endif

#endif // !defined(AFX_TRACELOGFILE_H__0A6BEBCA_829C_4085_8A12_19A15A12F62C__INCLUDED_)


#ifdef TRACELOGFILE
	#undef TRACE
	#define TRACE ::TraceFileEnterCS(_T("%s(%i) : "),CString(__FILE__),__LINE__),::TraceFileLeaveCS
#else
	#ifdef _DEBUG
		#undef TRACE
		#define TRACE ::TraceDebugEnterCS(CString(__FILE__),__LINE__),::TraceDebugLeaveCS
	#endif
#endif