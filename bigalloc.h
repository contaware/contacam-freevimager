// bigalloc.h
//

#if !defined(AFX_BIGALLOC_H__FFD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_)
#define AFX_BIGALLOC_H__FFD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_

#pragma once

// Default list size must be a multiple of 1048576
#ifdef VIDEODEVICEDOC
#define BIGALLOC_DEFAULT_LIST_SIZE	134217728	// 128MB
#else
#define BIGALLOC_DEFAULT_LIST_SIZE	33554432	// 32MB
#endif

typedef CList<LPVOID,LPVOID> BIGALLOCLIST;
extern void InitBigAlloc();
extern void EndBigAlloc();
extern LPVOID BigAlloc(SIZE_T Size, CString sFileName, int nLine);
extern BOOL BigFree(LPBYTE p, CString sFileName, int nLine);
extern SIZE_T BigAllocUsedSize(LPBYTE p, SIZE_T Size);
extern void GetBigAllocStats(double* p64kUsed = NULL,
							double* p128kUsed = NULL,
							double* p256kUsed = NULL,
							double* p512kUsed = NULL,
							double* p1024kUsed = NULL);
extern void BigAllocVMMapSnapshot();

// - VirtualAlloc and VirtualFree leaks are not detected by the debugger,
//   we use the CRT heap functions for the debug build 
// - VirtualAlloc's returned address is 64 KB aligned, 32 bytes alignment
//   is necessary for SIMD ops, av_malloc handles that correctly
// - Today's 32 bits apps suffer from memory space fragmentation and
//   not from RAM shortage, BIGALLOC_USEDSIZE accounts for the
//   allocation granularity and the address space waste
// - If using the heap functions the LFH heap is active up to 16 KB, from
//   16 KB to 512 KB the standard heap is used and above this value the
//   VirtualAlloc function is called. For the release build we prefer the
//   virtual functions because mjpeg frames have a size between 16 KB - 512 KB
//   which fragments and grows the heap (freed heap memory remains reserved
//   wasting address space). VirtualAllocs for sizes 16 KB - 512 KB fragment
//   the virtual address space, to solve that problem we use fixed sizes
//   allocator lists implemented in BigAlloc() and BigFree()
#define BIGALLOC_SAFETY		32	// must be at least FF_INPUT_BUFFER_PADDING_SIZE
#ifdef _DEBUG
extern "C"
{
#include "libavutil/mem.h"
}
#define BIGALLOC(Size) av_malloc((size_t)(Size)+BIGALLOC_SAFETY)
#define BIGFREE(p) av_free(p)
// av_malloc wastes some bytes for alignment but we do not account for that here
#define BIGALLOC_USEDSIZE(p,Size) ((SIZE_T)(Size)+BIGALLOC_SAFETY)
#else
#define BIGALLOC(Size) BigAlloc((SIZE_T)(Size)+BIGALLOC_SAFETY,CString(__FILE__),__LINE__)
#define BIGFREE(p) BigFree((LPBYTE)(p),CString(__FILE__),__LINE__)
#define BIGALLOC_USEDSIZE(p,Size) BigAllocUsedSize((LPBYTE)(p),(SIZE_T)(Size)+BIGALLOC_SAFETY)
#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BIGALLOC_H__FFD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_)

