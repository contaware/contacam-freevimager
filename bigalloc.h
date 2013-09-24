// bigalloc.h
//

#if !defined(AFX_BIGALLOC_H__FFD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_)
#define AFX_BIGALLOC_H__FFD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_

#pragma once

// VirtualAlloc / VirtualFree debug for leaks
// ------------------------------------------
// - VirtualAlloc and VirtualFree leaks are not detected by debugger
// - VirtualAlloc's returned address is page aligned
//   and new16align's address is 16 bytes aligned so that
//   we are ok with SSE2 ops in CDib, CAVRec and CAVIPlay
#ifdef _DEBUG
extern LPVOID new16align(unsigned int size);
extern void delete16align(LPVOID ptr);
#define BIGALLOC(Size) new16align((unsigned int)(Size))
#define BIGFREE(lpAddress) delete16align((LPVOID)(lpAddress))
#else
#define BIGALLOC(Size) VirtualAlloc(NULL,Size,MEM_COMMIT,PAGE_READWRITE)
#define BIGFREE(lpAddress) VirtualFree((LPVOID)(lpAddress),0,MEM_RELEASE)
#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BIGALLOC_H__FFD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_)

