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
			// Init vars
			m_InitCount.QuadPart = 0;
			m_EndCount.QuadPart = 0;
			m_Frequency.QuadPart = 0;
			m_dwDiffMicroSec = 0U;
			m_dwInitTick = 0U;

			// Get the current process core mask
			DWORD dwProcMask;
			DWORD dwSysMask;
			::GetProcessAffinityMask(::GetCurrentProcess(), &dwProcMask, &dwSysMask);
			
			// If dwProcMask is 0, consider there is only one core available
			// (using 0 as dwProcMask will cause an infinite loop below)
			if (dwProcMask == 0U)
				dwProcMask = 1U;

			// Find the lowest core that this process uses
			m_dwFirstCoreMask = 1U;
			while ((m_dwFirstCoreMask & dwProcMask) == 0U)
				m_dwFirstCoreMask <<= 1;
		}
		virtual ~CPerformance()
		{

		}
		__forceinline void Init()
		{
			// Get thread handle
			HANDLE hThread = ::GetCurrentThread();

			// Set affinity to the first core
			DWORD dwOldMask = ::SetThreadAffinityMask(hThread, m_dwFirstCoreMask);

			// Get the constant frequency
			::QueryPerformanceFrequency(&m_Frequency);

			// Query the counters
			m_dwInitTick = ::GetTickCount(); // GetTickCount() is faster than timeGetTime() but a bit less accurate
			::QueryPerformanceCounter(&m_InitCount);

			// Reset affinity
			::SetThreadAffinityMask(hThread, dwOldMask);		
		}
		__forceinline void End()
		{
			// Get thread handle
			HANDLE hThread = ::GetCurrentThread();

			// Set affinity to the first core
			DWORD dwOldMask = ::SetThreadAffinityMask(hThread, m_dwFirstCoreMask);

			// Query the counters
			::QueryPerformanceCounter(&m_EndCount);
			DWORD dwEndTick = ::GetTickCount(); // GetTickCount() is faster than timeGetTime() but a bit less accurate

			// Reset affinity
			::SetThreadAffinityMask(hThread, dwOldMask);

			// Get differences
			if (m_Frequency.QuadPart > 0)
			{
				LONGLONG llDiffMicroSec = 1000000 * (m_EndCount.QuadPart - m_InitCount.QuadPart) /
													m_Frequency.QuadPart;
				// Check (performance counter may also leap backwards)
				if (llDiffMicroSec > 0)
					m_dwDiffMicroSec = (DWORD)llDiffMicroSec;
				else
					m_dwDiffMicroSec = 0U;
			}
			else
				m_dwDiffMicroSec = 0U;
			DWORD dwDiffMilliSec = m_dwDiffMicroSec / 1000;
			DWORD dwDiffTicks = dwEndTick - m_dwInitTick;

			// Detect performance counter leaps
			// (surprisingly common, see Microsoft KB: Q274323)
			int nMilliSecOff = (int)dwDiffMilliSec - (int)dwDiffTicks;
			if (nMilliSecOff < -300 || nMilliSecOff > 300)
				m_dwDiffMicroSec = 1000 * dwDiffTicks;
		}
		__forceinline DWORD GetMicroSecDiff()
		{
			return m_dwDiffMicroSec;
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
		LARGE_INTEGER m_InitCount;
		LARGE_INTEGER m_EndCount;
		LARGE_INTEGER m_Frequency;
		DWORD m_dwDiffMicroSec;
		DWORD m_dwInitTick;
		DWORD m_dwFirstCoreMask;
};

#endif // !defined(AFX_PERFORMANCE_H__0A6BEBCA_829C_4085_8A12_19A15A12F62C__INCLUDED_)
