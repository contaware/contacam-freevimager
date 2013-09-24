//
// TryEnterCriticalSection.h
//

#if !defined(AFX_TRYENTERCRITICALSECTION_H__0A6BEBCA_829C_4085_8A12_19A15A12F62C__INCLUDED_)
#define AFX_TRYENTERCRITICALSECTION_H__0A6BEBCA_829C_4085_8A12_19A15A12F62C__INCLUDED_

#pragma once

class CTryEnterCriticalSection
{
	public:
		typedef BOOL (WINAPI * FPTRYENTERCRITICALSECTION)(LPCRITICAL_SECTION lpCriticalSection);

		CTryEnterCriticalSection();
		virtual ~CTryEnterCriticalSection();
		void EnableTimeout(); // Must be called once if using timeouts other than INFINITE!

		// Tries to enter the critical section for the given amount of time
		// Returns:
		//   TRUE  : Entered CS
		//   FALSE : still locked after specified timeout
		__forceinline BOOL EnterCriticalSection(DWORD dwTimeout = INFINITE)
		{		
			if (m_fpTryEnterCriticalSection)
			{
				::EnterCriticalSection(&m_cs);
				return TRUE;
			}
			else
				return (::WaitForSingleObject(m_hMutex, dwTimeout) == WAIT_OBJECT_0);
		};
		
		// Try to enter critical section without blocking
		// Returns:
		//   TRUE  : Entered CS
		//   FALSE : already locked
		__forceinline BOOL TryEnterCriticalSection()
		{
			if (m_fpTryEnterCriticalSection)
				return m_fpTryEnterCriticalSection(&m_cs);
			else
				return (::WaitForSingleObject(m_hMutex, 0U) == WAIT_OBJECT_0);
		};

		// For each successful enter call a LeaveCriticalSection()
		__forceinline void LeaveCriticalSection()
		{
			if (m_fpTryEnterCriticalSection)
				::LeaveCriticalSection(&m_cs);
			else
				::ReleaseMutex(m_hMutex);
		};
	
	protected:
		HINSTANCE m_hKernel32;
		HANDLE m_hMutex;		// For Win9x (Because ::TryEnterCriticalSection() is not supported)
		CRITICAL_SECTION m_cs;	// For NT, Win2k, WinXp, Win2k3 and higher
		FPTRYENTERCRITICALSECTION m_fpTryEnterCriticalSection;
};

#endif // !defined(AFX_TRYENTERCRITICALSECTION_H__0A6BEBCA_829C_4085_8A12_19A15A12F62C__INCLUDED_)
