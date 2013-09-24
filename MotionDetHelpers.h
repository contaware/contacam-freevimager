#if !defined(AFX_MOTIONDETHELPERS_H__8FD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_)
#define AFX_MOTIONDETHELPERS_H__8FD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_

#pragma once

extern "C" void Mix7To1MMX(void* pBackgnd, void* pSrc, int nSize);
extern "C" void Mix3To1MMX(void* pBackgnd, void* pSrc, int nSize);
extern "C" void DiffMMX(void* pDst, void* pSrc1, void* pSrc2, int nSize, void* pMinDiffs);

#endif // !defined(AFX_MOTIONDETHELPERS_H__8FD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_)


