#if !defined(AFX_CPUSPEED_H__E852AF35_48F6_4D85_A507_3A77724D2AE5__INCLUDED_)
#define AFX_CPUSPEED_H__E852AF35_48F6_4D85_A507_3A77724D2AE5__INCLUDED_

#pragma once

extern DWORD GetProcessorSpeedMHz();

typedef	void (*DELAY_FUNC)(unsigned int uiMS);

class CPUSpeed
{
public:
	CPUSpeed(DWORD dwMeasureTimeMs);
	~CPUSpeed();
		
	int m_nCPUSpeedInMHz;
	__int64 __cdecl GetCyclesDifference(DELAY_FUNC, unsigned int);
		
private:
	DWORD m_dwOldThreadAffinity;
	DWORD m_dwOldPriorityClass;
	int m_nOldThreadPriority;
	static void Delay(unsigned int);
	static void DelayOverhead(unsigned int);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CPUSPEED_H__E852AF35_48F6_4D85_A507_3A77724D2AE5__INCLUDED_)
