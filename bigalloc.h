// bigalloc.h
//

#if !defined(AFX_BIGALLOC_H__FFD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_)
#define AFX_BIGALLOC_H__FFD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_

#pragma once

// - VirtualAlloc and VirtualFree leaks are not detected by debugger,
//   we use the CRT heap functions for the debug build 
// - VirtualAlloc's returned address is 64 KB aligned, 16, 32 or 64 bytes
//   alignment is necessary for SIMD ops in CDib and CAVRec, av_malloc
//   handles that correctly
// - Today's 32 bits apps suffer from memory space fragmentation and
//   not from RAM shortage, BIGALLOC_USEDSIZE accounts for the 64 KB
//   allocation granularity and the address space waste
// - LFH heap is active up to 16 KB, from 16 KB to 512 KB the standard
//   heap is used and above this value the VirtualAlloc function is
//   called. For the release build we prefer the virtual functions
//   because mjpeg frames have a size between 16 KB - 512 KB which would
//   fragment and grow the heap (freed heap memory remains reserved wasting
//   address space). For picture data the size is above 512 KB, using the
//   heap functions would add a function call more
// - VirtualAlloc for small mjpeg frames (16 KB - 64 KB) fragments the
//   virtual address space quite a lot, so it's mandatory to reserve heap
//   space when starting the program (see InitInstance() where we reserve
//   512 MB for the heap)
#define BIGALLOC_SAFETY			4096	// must be at least FF_INPUT_BUFFER_PADDING_SIZE, we use one page
#ifdef _DEBUG
#define BIGALLOC(Size) av_malloc((size_t)(Size)+BIGALLOC_SAFETY)
#define BIGFREE(lpAddress) av_free(lpAddress)
// av_malloc wastes some bytes for alignment but we do not account for that here
#define BIGALLOC_USEDSIZE(Size) ((SIZE_T)(Size)+BIGALLOC_SAFETY)
#else
#define BIGALLOC(Size) VirtualAlloc(NULL,(SIZE_T)(Size)+BIGALLOC_SAFETY,MEM_COMMIT,PAGE_READWRITE)
#define BIGFREE(lpAddress) VirtualFree((LPVOID)(lpAddress),0,MEM_RELEASE)
#define BIGALLOC_USEDSIZE(Size) (((SIZE_T)(Size)+BIGALLOC_SAFETY+0xffff)&~0xffff)
#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BIGALLOC_H__FFD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_)

