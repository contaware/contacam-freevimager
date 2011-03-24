#include "stdafx.h"
#include "CPUSpeed.h"
#include "uImager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#if _MSC_VER < 1100
#define rdtsc __asm _emit 0x0f __asm _emit 0x31
#endif // _MSC_VER < 1100

#ifndef MAX
#define MAX(a,b)  ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b)  ((a) < (b) ? (a) : (b))
#endif

DWORD GetProcessorSpeedMHz(DWORD dwMeasureTimeMs)
{
    DWORD dwTime1, dwTime2;
    ULARGE_INTEGER ulInstruct1, ulInstruct2;
	TIMECAPS tc;
	UINT wTimerRes;
	if (timeGetDevCaps(&tc, sizeof(TIMECAPS)) != TIMERR_NOERROR) 
		return 0;
	wTimerRes = MIN(MAX(tc.wPeriodMin, 1), tc.wPeriodMax); // 1 millisecond target resolution
    
	// Make sure everything is running on the same CPU, this because the counters
	// may differ from one CPU to the other on a multi-processor system
	DWORD dwProcMask;
	DWORD dwSysMask;
	GetProcessAffinityMask(GetCurrentProcess(), &dwProcMask, &dwSysMask);
	if (dwProcMask == 0)
		dwProcMask = 1;
	DWORD dwFirstCoreMask = 1;
	while ((dwFirstCoreMask & dwProcMask) == 0)
		dwFirstCoreMask <<= 1;
	DWORD dwOldThreadAffinity = SetThreadAffinityMask(GetCurrentThread(), dwFirstCoreMask);

	// Increase Timer Resolution to 1 ms, if possible
	timeBeginPeriod(wTimerRes);

	// Timer start-up
    dwTime1 = timeGetTime();
	Sleep(20);
    
	// First measure point
    dwTime1 = timeGetTime();
    __try
    {
        __asm
        {
            rdtsc
            mov ulInstruct1.LowPart,  eax
            mov ulInstruct1.HighPart, edx
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        timeEndPeriod(wTimerRes);
		SetThreadAffinityMask(GetCurrentThread(), dwOldThreadAffinity);
		return 0;
    }

    // Measure time
    Sleep(dwMeasureTimeMs);

    // Second measure point
    dwTime2 = timeGetTime();
    __try
    {
        __asm
        {
            rdtsc
            mov ulInstruct2.LowPart,  eax
            mov ulInstruct2.HighPart, edx
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        timeEndPeriod(wTimerRes);
		SetThreadAffinityMask(GetCurrentThread(), dwOldThreadAffinity);
		return 0;
    }

	// Restore Timer Resolution
	timeEndPeriod(wTimerRes);

	// Restore Affinity Mask
	SetThreadAffinityMask(GetCurrentThread(), dwOldThreadAffinity);

    // Check
    if ((dwTime1 >= dwTime2) ||
        ((dwTime2-dwTime1) < (dwMeasureTimeMs/2)) ||
        (ulInstruct1.QuadPart >= ulInstruct2.QuadPart))
        return 0;

    // Calc. Frequency in MHz
    return (DWORD)((ulInstruct2.QuadPart-ulInstruct1.QuadPart) /
			((dwTime2-dwTime1) * 1000));
}


/////////////////////////////
// A faster implementation //
/////////////////////////////

#define	CPUSPEED_I32TO64(x, y)		(((__int64) x << 32) + y)

DWORD GetProcessorSpeedMHzFast()
{
	CPUSpeed Speed1(10);	// Warm-up and wake-up from power-safe
	CPUSpeed Speed2(20);	// Measure
	return Speed2.m_nCPUSpeedInMHz;
}

CPUSpeed::CPUSpeed(DWORD dwMeasureTimeMs)
{
	// Make sure everything is running on the same CPU, this because the counters
	// may differ from one CPU to the other on a multi-processor system
	DWORD dwProcMask;
	DWORD dwSysMask;
	::GetProcessAffinityMask(::GetCurrentProcess(), &dwProcMask, &dwSysMask);
	if (dwProcMask == 0)
		dwProcMask = 1;
	DWORD dwFirstCoreMask = 1;
	while ((dwFirstCoreMask & dwProcMask) == 0)
		dwFirstCoreMask <<= 1;
	m_dwOldThreadAffinity = ::SetThreadAffinityMask(::GetCurrentThread(), dwFirstCoreMask);

	// Set Time Critical Priority
	m_dwOldPriorityClass = ::GetPriorityClass(::GetCurrentProcess());
	m_nOldThreadPriority = ::GetThreadPriority(::GetCurrentThread());
	::SetPriorityClass(::GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
	::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

	unsigned int uiRepetitions = 1;
	unsigned int uiMSecPerRepetition = dwMeasureTimeMs;
	__int64	i64Total = 0, i64Overhead = 0;

	for (unsigned int nCounter = 0; nCounter < uiRepetitions; nCounter ++)
	{
		i64Total += GetCyclesDifference(CPUSpeed::Delay, uiMSecPerRepetition);
		i64Overhead += GetCyclesDifference(CPUSpeed::DelayOverhead, uiMSecPerRepetition);
	}

	// Calculate the MHz speed
	i64Total -= i64Overhead;
	i64Total /= uiRepetitions;
	i64Total /= uiMSecPerRepetition;
	i64Total /= 1000;

	// Save the CPU speed
	m_nCPUSpeedInMHz = (int)i64Total;
}

CPUSpeed::~CPUSpeed()
{
	// Restore Priority
	::SetPriorityClass(::GetCurrentProcess(), m_dwOldPriorityClass);
	::SetThreadPriority(::GetCurrentThread(), m_nOldThreadPriority);

	// Restore Affinity Mask
	::SetThreadAffinityMask(::GetCurrentThread(), m_dwOldThreadAffinity);
}

__int64	__cdecl CPUSpeed::GetCyclesDifference(DELAY_FUNC DelayFunction, unsigned int uiParameter)
{
	unsigned int edx1, eax1;
	unsigned int edx2, eax2;
		
	// Calculate the frequency of the CPU instructions.
	__try
	{
		_asm
		{
			push uiParameter		; push parameter param
			mov ebx, DelayFunction	; store func in ebx

			rdtsc

			mov esi, eax			; esi = eax
			mov edi, edx			; edi = edx

			call ebx				; call the delay functions

			rdtsc

			pop ebx

			mov edx2, edx			; edx2 = edx
			mov eax2, eax			; eax2 = eax

			mov edx1, edi			; edx1 = edi
			mov eax1, esi			; eax1 = esi
		}
	}
	// A generic catch-all just to be sure...
	__except (1)
	{
		return -1;
	}

	return (CPUSPEED_I32TO64 (edx2, eax2) - CPUSPEED_I32TO64 (edx1, eax1));
}

void CPUSpeed::Delay(unsigned int uiMS)
{
	LARGE_INTEGER Frequency, StartCounter, EndCounter;
	__int64 x;

	// Get the frequency of the high performance counter.
	if (!::QueryPerformanceFrequency(&Frequency))
		return;
	x = Frequency.QuadPart / 1000 * uiMS;

	// Get the starting position of the counter.
	::QueryPerformanceCounter(&StartCounter);

	do
	{
		// Get the ending position of the counter.	
		::QueryPerformanceCounter(&EndCounter);
	}
	while (EndCounter.QuadPart - StartCounter.QuadPart < x);
}

void CPUSpeed::DelayOverhead(unsigned int uiMS)
{
	LARGE_INTEGER Frequency, StartCounter, EndCounter;
	__int64 x;

	// Get the frequency of the high performance counter.
	if (!::QueryPerformanceFrequency(&Frequency))
		return;
	x = Frequency.QuadPart / 1000 * uiMS;

	// Get the starting position of the counter.
	::QueryPerformanceCounter(&StartCounter);
	
	do
	{
		// Get the ending position of the counter.	
		::QueryPerformanceCounter(&EndCounter);
	}
	while (EndCounter.QuadPart - StartCounter.QuadPart == x);
}