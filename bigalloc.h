// bigalloc.h
//

#if !defined(AFX_BIGALLOC_H__FFD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_)
#define AFX_BIGALLOC_H__FFD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_

#pragma once

// new16align's address is 16 bytes aligned so that
// we are ok with SSE2 ops in CDib, CAVRec and CAVIPlay
extern LPVOID new16align(SIZE_T size); // size in bytes
extern void delete16align(LPVOID ptr);
#define BIGALLOC(Size) new16align((SIZE_T)(Size))
#define BIGFREE(lpAddress) delete16align((LPVOID)(lpAddress))

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BIGALLOC_H__FFD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_)

