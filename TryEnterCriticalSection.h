//
// TryEnterCriticalSection.h
//

#if !defined(AFX_TRYENTERCRITICALSECTION_H__0A6BEBCA_829C_4085_8A12_19A15A12F62C__INCLUDED_)
#define AFX_TRYENTERCRITICALSECTION_H__0A6BEBCA_829C_4085_8A12_19A15A12F62C__INCLUDED_

#pragma once

//
// ATTENTION (from the Microsoft docs)
//
// 1. After a thread has ownership of a critical section, it can make additional calls
// to EnterCriticalSection or TryEnterCriticalSection without blocking its execution.
// This prevents a thread from deadlocking itself while waiting for a critical section
// that it already owns. The thread enters the critical section each time EnterCriticalSection
// and TryEnterCriticalSection succeed. A thread must call LeaveCriticalSection once for each
// time that it entered the critical section!
// If a thread calls LeaveCriticalSection when it does not have ownership of the specified
// critical section object, an error condition occurs (the function itself doesn't return an error)
// that may cause another thread using EnterCriticalSection to wait indefinitely!
//
// 2. The thread that owns a mutex can specify the same mutex in repeated wait function calls
// without blocking its execution. Typically, you would not wait repeatedly for the same mutex,
// but this mechanism prevents a thread from deadlocking itself while waiting for a mutex that
// it already owns. However, to release its ownership, the thread must call ReleaseMutex once
// for each time that the mutex satisfied a wait!
// The ReleaseMutex function fails if the calling thread does not own the mutex object (compared
// to critical sections it seems possible to double release, but it is wise to not rely on that
// behavior).
//
// -> *** IT'S IMPORTANT TO BALANCE THE NUMBER OF SUCCESSFULLY ENTERED CALLS WITH THE RELEASED ONES ***
//

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
		HANDLE m_hMutex;
		CRITICAL_SECTION m_cs;
		FPTRYENTERCRITICALSECTION m_fpTryEnterCriticalSection;
};

#endif // !defined(AFX_TRYENTERCRITICALSECTION_H__0A6BEBCA_829C_4085_8A12_19A15A12F62C__INCLUDED_)
