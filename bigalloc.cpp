#include "stdafx.h"
#include "Helpers.h"
#include "Round.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static BIGALLOCLIST g_BigAlloc64kList;
static BIGALLOCLIST g_BigAlloc128kList;
static BIGALLOCLIST g_BigAlloc256kList;
static BIGALLOCLIST g_BigAlloc512kList;
static BIGALLOCLIST g_BigAlloc1024kList;
static BIGALLOCLIST g_BigAlloc2048kList;
static CRITICAL_SECTION g_csBigAlloc64k = {0};
static CRITICAL_SECTION g_csBigAlloc128k = {0};
static CRITICAL_SECTION g_csBigAlloc256k = {0};
static CRITICAL_SECTION g_csBigAlloc512k = {0};
static CRITICAL_SECTION g_csBigAlloc1024k = {0};
static CRITICAL_SECTION g_csBigAlloc2048k = {0};
static LPBYTE g_pBigAlloc64kBase = NULL;
static SIZE_T g_BigAlloc64kSize = 0;
static LPBYTE g_pBigAlloc128kBase = NULL;
static SIZE_T g_BigAlloc128kSize = 0;
static LPBYTE g_pBigAlloc256kBase = NULL;
static SIZE_T g_BigAlloc256kSize = 0;
static LPBYTE g_pBigAlloc512kBase = NULL;
static SIZE_T g_BigAlloc512kSize = 0;
static LPBYTE g_pBigAlloc1024kBase = NULL;
static SIZE_T g_BigAlloc1024kSize = 0;
static LPBYTE g_pBigAlloc2048kBase = NULL;
static SIZE_T g_BigAlloc2048kSize = 0;
static BOOL g_bOneBigMemoryChunk = FALSE;
static BOOL g_bBigAllocInited = FALSE;

void InitBigAlloc()
{
	if (!g_bBigAllocInited)
	{
		SIZE_T i;

		// Init critical sections
		InitializeCriticalSection(&g_csBigAlloc64k);
		InitializeCriticalSection(&g_csBigAlloc128k);
		InitializeCriticalSection(&g_csBigAlloc256k);
		InitializeCriticalSection(&g_csBigAlloc512k);
		InitializeCriticalSection(&g_csBigAlloc1024k);
		InitializeCriticalSection(&g_csBigAlloc2048k);

		// Init sizes
		g_BigAlloc64kSize = BIGALLOC_DEFAULT_LIST_SIZE;
		g_BigAlloc128kSize = BIGALLOC_DEFAULT_LIST_SIZE;
		g_BigAlloc256kSize = BIGALLOC_DEFAULT_LIST_SIZE;
		g_BigAlloc512kSize = BIGALLOC_DEFAULT_LIST_SIZE;
		g_BigAlloc1024kSize = BIGALLOC_DEFAULT_LIST_SIZE;
		g_BigAlloc2048kSize = BIGALLOC_DEFAULT_LIST_SIZE;

		// Try reserving one big chunk of address space
		g_pBigAlloc64kBase = (LPBYTE)VirtualAlloc(	NULL,
													g_BigAlloc64kSize	+
													g_BigAlloc128kSize	+
													g_BigAlloc256kSize	+
													g_BigAlloc512kSize	+
													g_BigAlloc1024kSize	+
													g_BigAlloc2048kSize,
													MEM_RESERVE,
													PAGE_NOACCESS);
		if (g_pBigAlloc64kBase)
		{
			g_bOneBigMemoryChunk = TRUE;
			g_pBigAlloc128kBase = g_pBigAlloc64kBase + g_BigAlloc64kSize;	
			g_pBigAlloc256kBase = g_pBigAlloc128kBase + g_BigAlloc128kSize;
			g_pBigAlloc512kBase = g_pBigAlloc256kBase + g_BigAlloc256kSize;
			g_pBigAlloc1024kBase = g_pBigAlloc512kBase + g_BigAlloc512kSize;
			g_pBigAlloc2048kBase = g_pBigAlloc1024kBase + g_BigAlloc1024kSize;
		}
		// Reserve individual chunks
		else
		{
			g_bOneBigMemoryChunk = FALSE;
			g_pBigAlloc64kBase = (LPBYTE)VirtualAlloc(NULL, g_BigAlloc64kSize, MEM_RESERVE, PAGE_NOACCESS);
			g_pBigAlloc128kBase = (LPBYTE)VirtualAlloc(NULL, g_BigAlloc128kSize, MEM_RESERVE, PAGE_NOACCESS);
			g_pBigAlloc256kBase = (LPBYTE)VirtualAlloc(NULL, g_BigAlloc256kSize, MEM_RESERVE, PAGE_NOACCESS);
			g_pBigAlloc512kBase = (LPBYTE)VirtualAlloc(NULL, g_BigAlloc512kSize, MEM_RESERVE, PAGE_NOACCESS);
			g_pBigAlloc1024kBase = (LPBYTE)VirtualAlloc(NULL, g_BigAlloc1024kSize, MEM_RESERVE, PAGE_NOACCESS);
			g_pBigAlloc2048kBase = (LPBYTE)VirtualAlloc(NULL, g_BigAlloc2048kSize, MEM_RESERVE, PAGE_NOACCESS);
		}

		// 64k
		for (i = 0 ; i < g_BigAlloc64kSize / 65536 ; i++)
			g_BigAlloc64kList.AddTail(g_pBigAlloc64kBase + i * 65536);

		// 128k
		for (i = 0 ; i < g_BigAlloc128kSize / 131072 ; i++)
			g_BigAlloc128kList.AddTail(g_pBigAlloc128kBase + i * 131072);
		
		// 256k
		for (i = 0 ; i < g_BigAlloc256kSize / 262144 ; i++)
			g_BigAlloc256kList.AddTail(g_pBigAlloc256kBase + i * 262144);

		// 512k
		for (i = 0 ; i < g_BigAlloc512kSize / 524288 ; i++)
			g_BigAlloc512kList.AddTail(g_pBigAlloc512kBase + i * 524288);
		
		// 1024k
		for (i = 0 ; i < g_BigAlloc1024kSize / 1048576 ; i++)
			g_BigAlloc1024kList.AddTail(g_pBigAlloc1024kBase + i * 1048576);

		// 2048k
		for (i = 0 ; i < g_BigAlloc2048kSize / 2097152 ; i++)
			g_BigAlloc2048kList.AddTail(g_pBigAlloc2048kBase + i * 2097152);

		// Set init flag
		g_bBigAllocInited = TRUE;
	}
}

void EndBigAlloc()
{
	if (g_bBigAllocInited)
	{
		// Clear init flag
		g_bBigAllocInited = FALSE;

		// Free memory
		if (g_bOneBigMemoryChunk)
		{
			if (g_pBigAlloc64kBase)
			{
				VirtualFree(g_pBigAlloc64kBase, 0, MEM_RELEASE);
				g_pBigAlloc64kBase = NULL;
				g_pBigAlloc128kBase = NULL;
				g_pBigAlloc256kBase = NULL;
				g_pBigAlloc512kBase = NULL;
				g_pBigAlloc1024kBase = NULL;
				g_pBigAlloc2048kBase = NULL;
			}
		}
		else
		{
			if (g_pBigAlloc64kBase)
			{
				VirtualFree(g_pBigAlloc64kBase, 0, MEM_RELEASE);
				g_pBigAlloc64kBase = NULL;
			}
			if (g_pBigAlloc128kBase)
			{
				VirtualFree(g_pBigAlloc128kBase, 0, MEM_RELEASE);
				g_pBigAlloc128kBase = NULL;
			}
			if (g_pBigAlloc256kBase)
			{
				VirtualFree(g_pBigAlloc256kBase, 0, MEM_RELEASE);
				g_pBigAlloc256kBase = NULL;
			}
			if (g_pBigAlloc512kBase)
			{
				VirtualFree(g_pBigAlloc512kBase, 0, MEM_RELEASE);
				g_pBigAlloc512kBase = NULL;
			}
			if (g_pBigAlloc1024kBase)
			{
				VirtualFree(g_pBigAlloc1024kBase, 0, MEM_RELEASE);
				g_pBigAlloc1024kBase = NULL;
			}
			if (g_pBigAlloc2048kBase)
			{
				VirtualFree(g_pBigAlloc2048kBase, 0, MEM_RELEASE);
				g_pBigAlloc2048kBase = NULL;
			}
		}

		// Reset sizes
		g_BigAlloc64kSize = 0;
		g_BigAlloc128kSize = 0;
		g_BigAlloc256kSize = 0;
		g_BigAlloc512kSize = 0;
		g_BigAlloc1024kSize = 0;
		g_BigAlloc2048kSize = 0;

		// Delete critical sections
		DeleteCriticalSection(&g_csBigAlloc64k);
		DeleteCriticalSection(&g_csBigAlloc128k);
		DeleteCriticalSection(&g_csBigAlloc256k);
		DeleteCriticalSection(&g_csBigAlloc512k);
		DeleteCriticalSection(&g_csBigAlloc1024k);
		DeleteCriticalSection(&g_csBigAlloc2048k);
	}
}

LPVOID BigAlloc(SIZE_T Size, CString sFileName, int nLine)
{
	LPVOID p;

	// 64k
	if (Size <= 65536)
	{
		EnterCriticalSection(&g_csBigAlloc64k);
		if (!g_BigAlloc64kList.IsEmpty()) // note: try next allocation size if list empty
		{
			LPVOID pHead = g_BigAlloc64kList.RemoveHead();
			LeaveCriticalSection(&g_csBigAlloc64k);
			p = VirtualAlloc(pHead, 65536, MEM_COMMIT, PAGE_READWRITE);
			if (p)
				return p;
			else
			{
				// Re-add it if failing
				EnterCriticalSection(&g_csBigAlloc64k);
				g_BigAlloc64kList.AddHead(pHead);
			}
		}
		LeaveCriticalSection(&g_csBigAlloc64k);
	}

	// 128k
	if (Size <= 131072)
	{
		EnterCriticalSection(&g_csBigAlloc128k);
		if (!g_BigAlloc128kList.IsEmpty()) // note: try next allocation size if list empty
		{
			LPVOID pHead = g_BigAlloc128kList.RemoveHead();
			LeaveCriticalSection(&g_csBigAlloc128k);
			p = VirtualAlloc(pHead, 131072, MEM_COMMIT, PAGE_READWRITE);
			if (p)
				return p;
			else
			{
				// Re-add it if failing
				EnterCriticalSection(&g_csBigAlloc128k);
				g_BigAlloc128kList.AddHead(pHead);
			}
		}
		LeaveCriticalSection(&g_csBigAlloc128k);
	}

	// 256k
	if (Size <= 262144)
	{
		EnterCriticalSection(&g_csBigAlloc256k);
		if (!g_BigAlloc256kList.IsEmpty()) // note: try next allocation size if list empty
		{
			LPVOID pHead = g_BigAlloc256kList.RemoveHead();
			LeaveCriticalSection(&g_csBigAlloc256k);
			p = VirtualAlloc(pHead, 262144, MEM_COMMIT, PAGE_READWRITE);
			if (p)
				return p;
			else
			{
				// Re-add it if failing
				EnterCriticalSection(&g_csBigAlloc256k);
				g_BigAlloc256kList.AddHead(pHead);
			}
		}
		LeaveCriticalSection(&g_csBigAlloc256k);
	}

	// 512k
	if (Size <= 524288)
	{
		EnterCriticalSection(&g_csBigAlloc512k);
		if (!g_BigAlloc512kList.IsEmpty()) // note: try next allocation size if list empty
		{
			LPVOID pHead = g_BigAlloc512kList.RemoveHead(); 
			LeaveCriticalSection(&g_csBigAlloc512k);
			p = VirtualAlloc(pHead, 524288, MEM_COMMIT, PAGE_READWRITE);
			if (p)
				return p;
			else
			{
				// Re-add it if failing
				EnterCriticalSection(&g_csBigAlloc512k);
				g_BigAlloc512kList.AddHead(pHead);
			}
		}
		LeaveCriticalSection(&g_csBigAlloc512k);
	}

	// 1024k
	if (Size <= 1048576)
	{
		EnterCriticalSection(&g_csBigAlloc1024k);
		if (!g_BigAlloc1024kList.IsEmpty()) // note: try next allocation size if list empty
		{
			LPVOID pHead = g_BigAlloc1024kList.RemoveHead();
			LeaveCriticalSection(&g_csBigAlloc1024k);
			p = VirtualAlloc(pHead, 1048576, MEM_COMMIT, PAGE_READWRITE);
			if (p)
				return p;
			else
			{
				// Re-add it if failing
				EnterCriticalSection(&g_csBigAlloc1024k);
				g_BigAlloc1024kList.AddHead(pHead);
			}
		}
		LeaveCriticalSection(&g_csBigAlloc1024k);
	}

	// 2048k
	if (Size <= 2097152)
	{
		EnterCriticalSection(&g_csBigAlloc2048k);
		if (!g_BigAlloc2048kList.IsEmpty()) // note: reserve + commit in one step if list empty
		{
			LPVOID pHead = g_BigAlloc2048kList.RemoveHead();
			LeaveCriticalSection(&g_csBigAlloc2048k);
			p = VirtualAlloc(pHead, 2097152, MEM_COMMIT, PAGE_READWRITE);
			if (p)
				return p;
			else
			{
				// Re-add it if failing
				EnterCriticalSection(&g_csBigAlloc2048k);
				g_BigAlloc2048kList.AddHead(pHead);
			}
		}
		LeaveCriticalSection(&g_csBigAlloc2048k);
	}

	// Reserving + committing in one step
	p = VirtualAlloc(NULL, Size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (!p)
	{
		DWORD dwLastError = GetLastError();
		LogLine(_T("VirtualAlloc(%Iu bytes) in %s(%i) returned ") + ML_STRING(1784, "Error with code %u."),
				Size, GetShortFileName(sFileName), nLine, dwLastError);
	}

	return p;
}

BOOL BigFree(LPBYTE p, CString sFileName, int nLine)
{
	BOOL res;

	// 64k
	if (g_pBigAlloc64kBase <= p && p < g_pBigAlloc64kBase + g_BigAlloc64kSize)
	{
		res = VirtualFree(p, 65536, MEM_DECOMMIT);
		if (res)
		{
			EnterCriticalSection(&g_csBigAlloc64k);
			g_BigAlloc64kList.AddHead(p);
			LeaveCriticalSection(&g_csBigAlloc64k);
		}
	}
	// 128k
	else if (g_pBigAlloc128kBase <= p && p < g_pBigAlloc128kBase + g_BigAlloc128kSize)
	{
		res = VirtualFree(p, 131072, MEM_DECOMMIT);
		if (res)
		{
			EnterCriticalSection(&g_csBigAlloc128k);
			g_BigAlloc128kList.AddHead(p);
			LeaveCriticalSection(&g_csBigAlloc128k);
		}
	}
	// 256k
	else if (g_pBigAlloc256kBase <= p && p < g_pBigAlloc256kBase + g_BigAlloc256kSize)
	{
		res = VirtualFree(p, 262144, MEM_DECOMMIT);
		if (res)
		{
			EnterCriticalSection(&g_csBigAlloc256k);
			g_BigAlloc256kList.AddHead(p);
			LeaveCriticalSection(&g_csBigAlloc256k);
		}
	}
	// 512k
	else if (g_pBigAlloc512kBase <= p && p < g_pBigAlloc512kBase + g_BigAlloc512kSize)
	{
		res = VirtualFree(p, 524288, MEM_DECOMMIT);
		if (res)
		{
			EnterCriticalSection(&g_csBigAlloc512k);
			g_BigAlloc512kList.AddHead(p);
			LeaveCriticalSection(&g_csBigAlloc512k);
		}
	}
	// 1024k
	else if (g_pBigAlloc1024kBase <= p && p < g_pBigAlloc1024kBase + g_BigAlloc1024kSize)
	{
		res = VirtualFree(p, 1048576, MEM_DECOMMIT);
		if (res)
		{
			EnterCriticalSection(&g_csBigAlloc1024k);
			g_BigAlloc1024kList.AddHead(p);
			LeaveCriticalSection(&g_csBigAlloc1024k);
		}
	}
	// 2048k
	else if (g_pBigAlloc2048kBase <= p && p < g_pBigAlloc2048kBase + g_BigAlloc2048kSize)
	{
		res = VirtualFree(p, 2097152, MEM_DECOMMIT);
		if (res)
		{
			EnterCriticalSection(&g_csBigAlloc2048k);
			g_BigAlloc2048kList.AddHead(p);
			LeaveCriticalSection(&g_csBigAlloc2048k);
		}
	}
	else
		res = VirtualFree(p, 0, MEM_RELEASE);

	if (!res)
	{
		DWORD dwLastError = GetLastError();
		LogLine(_T("VirtualFree(0x%08IX) in %s(%i) returned ") + ML_STRING(1784, "Error with code %u."),
				(SIZE_T)p, GetShortFileName(sFileName), nLine, dwLastError);
	}

	return res;
}

SIZE_T BigAllocUsedSize(LPBYTE p, SIZE_T Size)
{
	// 64k
	if (g_pBigAlloc64kBase <= p && p < g_pBigAlloc64kBase + g_BigAlloc64kSize)
		return 65536;
	// 128k
	else if (g_pBigAlloc128kBase <= p && p < g_pBigAlloc128kBase + g_BigAlloc128kSize)
		return 131072;
	// 256k
	else if (g_pBigAlloc256kBase <= p && p < g_pBigAlloc256kBase + g_BigAlloc256kSize)
		return 262144;
	// 512k
	else if (g_pBigAlloc512kBase <= p && p < g_pBigAlloc512kBase + g_BigAlloc512kSize)
		return 524288;
	// 1024k
	else if (g_pBigAlloc1024kBase <= p && p < g_pBigAlloc1024kBase + g_BigAlloc1024kSize)
		return 1048576;
	// 2048k
	else if (g_pBigAlloc2048kBase <= p && p < g_pBigAlloc2048kBase + g_BigAlloc2048kSize)
		return 2097152;
	else
		return ((Size + 0xffff) & ~0xffff);
}

void GetBigAllocStats(	double* p64kUsed/*=NULL*/,
						double* p128kUsed/*=NULL*/,
						double* p256kUsed/*=NULL*/,
						double* p512kUsed/*=NULL*/,
						double* p1024kUsed/*=NULL*/,
						double* p2048kUsed/*=NULL*/)
{
	int nTot;
	if (p64kUsed)
	{
		EnterCriticalSection(&g_csBigAlloc64k);
		nTot = g_BigAlloc64kSize / 65536;
		if (nTot > 0)
			*p64kUsed = 100.0 * (double)(nTot - g_BigAlloc64kList.GetCount()) / (double)nTot;
		else
			*p64kUsed = 0.0;
		LeaveCriticalSection(&g_csBigAlloc64k);
	}
	if (p128kUsed)
	{
		EnterCriticalSection(&g_csBigAlloc128k);
		nTot = g_BigAlloc128kSize / 131072;
		if (nTot > 0)
			*p128kUsed = 100.0 * (double)(nTot - g_BigAlloc128kList.GetCount()) / (double)nTot;
		else
			*p128kUsed = 0.0;
		LeaveCriticalSection(&g_csBigAlloc128k);
	}
	if (p256kUsed)
	{
		EnterCriticalSection(&g_csBigAlloc256k);
		nTot = g_BigAlloc256kSize / 262144;
		if (nTot > 0)
			*p256kUsed = 100.0 * (double)(nTot - g_BigAlloc256kList.GetCount()) / (double)nTot;
		else
			*p256kUsed = 0.0;
		LeaveCriticalSection(&g_csBigAlloc256k);
	}
	if (p512kUsed)
	{
		EnterCriticalSection(&g_csBigAlloc512k);
		nTot = g_BigAlloc512kSize / 524288;
		if (nTot > 0)
			*p512kUsed = 100.0 * (double)(nTot - g_BigAlloc512kList.GetCount()) / (double)nTot;
		else
			*p512kUsed = 0.0;
		LeaveCriticalSection(&g_csBigAlloc512k);
	}
	if (p1024kUsed)
	{
		EnterCriticalSection(&g_csBigAlloc1024k);
		nTot = g_BigAlloc1024kSize / 1048576;
		if (nTot > 0)
			*p1024kUsed = 100.0 * (double)(nTot - g_BigAlloc1024kList.GetCount()) / (double)nTot;
		else
			*p1024kUsed = 0.0;
		LeaveCriticalSection(&g_csBigAlloc1024k);
	}
	if (p2048kUsed)
	{
		EnterCriticalSection(&g_csBigAlloc2048k);
		nTot = g_BigAlloc2048kSize / 2097152;
		if (nTot > 0)
			*p2048kUsed = 100.0 * (double)(nTot - g_BigAlloc2048kList.GetCount()) / (double)nTot;
		else
			*p2048kUsed = 0.0;
		LeaveCriticalSection(&g_csBigAlloc2048k);
	}
}