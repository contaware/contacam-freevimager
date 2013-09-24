#if !defined(AFX_CPUCOUNT_H__E852AF35_48F6_4D85_A507_3A77724D2AE5__INCLUDED_)
#define AFX_CPUCOUNT_H__E852AF35_48F6_4D85_A507_3A77724D2AE5__INCLUDED_

#pragma once

extern void	DisplayCpuCount(void);
extern unsigned int  CpuIDSupported(void);
extern unsigned int  GenuineIntel(void);
extern unsigned int  HWD_MTSupported(void);
extern unsigned int  MaxLogicalProcPerPhysicalProc(void);
extern unsigned int  MaxCorePerPhysicalProc(void);
extern unsigned int  find_maskwidth(unsigned int);
extern unsigned char GetAPIC_ID(void);
extern unsigned char GetNzbSubID(unsigned char,
								unsigned char,
								unsigned char);
extern unsigned char CPUCount(	unsigned int *,
								unsigned int *,
								unsigned int *);


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CPUCOUNT_H__E852AF35_48F6_4D85_A507_3A77724D2AE5__INCLUDED_)
