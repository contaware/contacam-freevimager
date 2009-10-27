//
// Performance.h
//

#if !defined(AFX_PERFORMANCE_H__0A6BEBCA_829C_4085_8A12_19A15A12F62C__INCLUDED_)
#define AFX_PERFORMANCE_H__0A6BEBCA_829C_4085_8A12_19A15A12F62C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TraceLogFile.h"

class CPerformance
{
	public:
		CPerformance()
		{
			m_dwOldThreadAffinity = ::SetThreadAffinityMask(::GetCurrentThread(), 1);
			::QueryPerformanceFrequency(&m_Frequency);
			m_InitCount.QuadPart = 0;
			m_EndCount.QuadPart = 0;
		}
		virtual ~CPerformance()
		{
			::SetThreadAffinityMask(::GetCurrentThread(), m_dwOldThreadAffinity);
		}
		__forceinline void Init()	{::QueryPerformanceCounter(&m_InitCount);}
		__forceinline void End()	{::QueryPerformanceCounter(&m_EndCount);}
		__forceinline DWORD GetMicroSecDiff()
		{
			LARGE_INTEGER Time;
			Time.QuadPart = 1000000 * (	m_EndCount.QuadPart -
										m_InitCount.QuadPart) /
										m_Frequency.QuadPart;
			return (DWORD)Time.QuadPart;
		}
		__forceinline void Wait(DWORD dwWaitMicroSec)
		{
			Init();
			do
			{
				End();
			}
			while (GetMicroSecDiff() < dwWaitMicroSec);
		}
		__forceinline void Trace(const TCHAR* pFormat = NULL, ...)
		{
			CString t(_T(""));
			CString s(_T(""));
			if (pFormat)
			{
				va_list arguments;
				va_start(arguments, pFormat);	
				t.FormatV(pFormat, arguments);
				va_end(arguments);
			}
			if (t != _T(""))
				s.Format(_T("%s : %uus\n"), t, GetMicroSecDiff());
			else
				s.Format(_T("%uus\n"), GetMicroSecDiff());
			TRACE(s);
		}
		__forceinline void MessageBox(const TCHAR* pFormat = NULL, ...)
		{
			CString t(_T(""));
			CString s(_T(""));
			if (pFormat)
			{
				va_list arguments;
				va_start(arguments, pFormat);	
				t.FormatV(pFormat, arguments);
				va_end(arguments);
			}
			if (t != _T(""))
				s.Format(_T("%s : %uus\n"), t, GetMicroSecDiff());
			else
				s.Format(_T("%uus\n"), GetMicroSecDiff());
			TRACE(s);
			::AfxMessageBox(s);
		}
		__forceinline void Log(CString sFileName, const TCHAR* pFormat = NULL, ...)
		{
			CString t(_T(""));
			CString s(_T(""));
			if (pFormat)
			{
				va_list arguments;
				va_start(arguments, pFormat);	
				t.FormatV(pFormat, arguments);
				va_end(arguments);
			}
			if (t != _T(""))
				s.Format(_T("%s : %uus\n"), t, GetMicroSecDiff());
			else
				s.Format(_T("%uus\n"), GetMicroSecDiff());
			TRACE(s);
			FILE* pf = _tfopen(sFileName, _T("at"));
			if (pf)
			{
				_ftprintf(pf, _T("%s"), (LPCTSTR)s);
				fclose(pf);
			}
		}
		__forceinline void LogNewLine(CString sFileName)
		{
			CString s(_T("\n"));
			TRACE(s);
			FILE* pf = _tfopen(sFileName, _T("at"));
			if (pf)
			{
				_ftprintf(pf, _T("%s"), (LPCTSTR)s);
				fclose(pf);
			}
		}

	protected:
		LARGE_INTEGER m_Frequency;
		LARGE_INTEGER m_InitCount;
		LARGE_INTEGER m_EndCount;
		DWORD m_dwOldThreadAffinity;
};

#endif // !defined(AFX_PERFORMANCE_H__0A6BEBCA_829C_4085_8A12_19A15A12F62C__INCLUDED_)
