//
// Performance.h
//

#if !defined(AFX_PERFORMANCE_H__0A6BEBCA_829C_4085_8A12_19A15A12F62C__INCLUDED_)
#define AFX_PERFORMANCE_H__0A6BEBCA_829C_4085_8A12_19A15A12F62C__INCLUDED_

#pragma once

/*
QueryPerformanceCounter (QPC) functions
---------------------------------------

In general, the performance counter results are consistent across all processors
in multi-core and multi-processor systems, even when measured on different threads
or processes. Here are some exceptions to this rule:

1. Pre-Windows Vista operating systems that run on certain processors might violate
   this consistency because of one of these reasons:
   - The hardware processors have a non-invariant TSC and the BIOS doesn't indicate
     this condition correctly
   - The TSC synchronization algorithm that was used wasn't suitable for systems
     with large numbers of processors
    
2. When you compare performance counter results that are acquired from different
   threads, consider values that differ by ± 1 tick to have an ambiguous ordering.
   If the time stamps are taken from the same thread, this ± 1 tick uncertainty
   doesn't apply. In this context, the term tick refers to a period of time equal
   to the frequency of the performance counter obtained from
   QueryPerformanceFrequency.

In conclusion do we need to set the thread affinity to a single core to use the
QPC functions in the following class?

This is neither necessary nor desirable. Performing this scenario might adversely
affect the application's performance by restricting processing to one core or by
creating a bottleneck on a single core if multiple threads set their affinity to
the same core when calling QueryPerformanceCounter.

Reference: http://msdn.microsoft.com/en-us/library/windows/desktop/dn553408%28v=vs.85%29.aspx
*/

class CPerformance
{
	public:
		CPerformance()
		{
			// Init vars
			m_InitCount.QuadPart = 0;
			m_EndCount.QuadPart = 0;
			m_dwDiffMicroSec = 0U;
			m_dwInitTick = 0U;
			::QueryPerformanceFrequency(&m_Frequency); // the frequency will not change while the system is running
		}
		virtual ~CPerformance()
		{

		}
		__forceinline void Init()
		{
			// Query the counters
			m_dwInitTick = ::GetTickCount();
			::QueryPerformanceCounter(&m_InitCount);
		}
		__forceinline void End()
		{
			// Query the counters
			::QueryPerformanceCounter(&m_EndCount);
			DWORD dwEndTick = ::GetTickCount();

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
			::AfxMessageBox(s);
		}
		__forceinline void Log(const TCHAR* pFormat = NULL, ...)
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
			::LogLine(_T("%s"), s);
		}

	protected:
		LARGE_INTEGER m_InitCount;
		LARGE_INTEGER m_EndCount;
		LARGE_INTEGER m_Frequency;
		DWORD m_dwDiffMicroSec;
		DWORD m_dwInitTick;
};

#endif // !defined(AFX_PERFORMANCE_H__0A6BEBCA_829C_4085_8A12_19A15A12F62C__INCLUDED_)
