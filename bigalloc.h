// bigalloc.h
//

#if !defined(AFX_BIGALLOC_H__FFD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_)
#define AFX_BIGALLOC_H__FFD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_

#pragma once

// - VirtualAlloc and VirtualFree leaks are not detected by the debugger,
//   we better use the CRT heap functions
// - VirtualAlloc's returned address is 64 KB aligned, 64 bytes alignment
//   is necessary for SIMD ops, _aligned_malloc handles that correctly
// - Today's 32 bits apps suffer from memory space fragmentation and
//   not from RAM shortage, ALLOC_GRANULARITY makes sure we allocate in
//   multiples of g_dwAllocationGranularity which is always 64K (that may
//   change in future if Microsoft decides to support other CPU architectures)
// - The LFH heap is active up to 16 KB, from 16 KB to 512 KB the standard
//   heap is used and above this value the VirtualAlloc function is called.
//   Mjpeg frames have typically a size between 16 KB - 512 KB so it's important
//   to allocate in chunks of 64K to avoid fragmenting the standard heap
// - From ffmpeg's mem.c: av_malloc(size) calls _aligned_malloc(size, ALIGN)
//   with ALIGN equal to 32 if HAVE_AVX is defined, otherwise it is 16
#define BIGALLOC_SAFETY		64U	// must be at least AV_INPUT_BUFFER_PADDING_SIZE
#define BIGALLOC_ALIGN		64U	// 64 to support AVX-512
#define ALLOC_GRANULARITY(Size) (((Size) + (g_dwAllocationGranularity - 1U)) & ~(g_dwAllocationGranularity - 1U))
#define BIGALLOC(Size) _aligned_malloc((size_t)ALLOC_GRANULARITY((Size)+BIGALLOC_SAFETY),BIGALLOC_ALIGN)
#define BIGFREE(lpAddress) _aligned_free((void*)(lpAddress))

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BIGALLOC_H__FFD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_)
