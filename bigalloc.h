// bigalloc.h
//

#if !defined(AFX_BIGALLOC_H__FFD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_)
#define AFX_BIGALLOC_H__FFD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_

#pragma once

// - VirtualAlloc and VirtualFree leaks are not detected by debugger,
//   we use the CRT heap functions for the debug build 
// - VirtualAlloc's returned address is 64 KB aligned
//   and new16align's address is 16 bytes aligned so that
//   we are ok with SSE2 ops in CDib, CAVRec and CAVIPlay
// - Today's 32 bits apps suffer from memory space fragmentation and
//   not from RAM shortage, BIGSIZE accounts for the 64 KB allocation
//   granularity and the address space waste
// - LFH heap is active up to 16 KB, from 16 KB to 512 KB the standard
//   heap is used and above this value the VirtualAlloc function is
//   called. For the release build we prefer the virtual functions
//   because mjpeg frames have a size between 16 KB - 512 KB, that's
//   the worst range because there the heap gets quite fragmented and
//   grows a lot without shrinking when heap data is freed (heap
//   memory remains reserved in memory wasting address space). For
//   picture data the size is above 512 KB, using the heap functions
//   would add a function call more
#ifdef _DEBUG
extern LPVOID new16align(SIZE_T size); // size in bytes
extern void delete16align(LPVOID ptr);
#define BIGALLOC(Size) new16align((SIZE_T)(Size))
#define BIGFREE(lpAddress) delete16align((LPVOID)(lpAddress))
#define BIGSIZE(Size) ((SIZE_T)(Size)+sizeof(LPVOID)+0xf)
#else
#define BIGALLOC(Size) VirtualAlloc(NULL,(SIZE_T)(Size),MEM_COMMIT,PAGE_READWRITE)
#define BIGFREE(lpAddress) VirtualFree((LPVOID)(lpAddress),0,MEM_RELEASE)
#define BIGSIZE(Size) (((SIZE_T)(Size)+0xffff)&~0xffff)
#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BIGALLOC_H__FFD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_)

