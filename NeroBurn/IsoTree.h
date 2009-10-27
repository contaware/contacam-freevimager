#if !defined(AFX_ISOTREE_H__78BE85B9_87B6_11D4_8056_00105ABB05E6__INCLUDED_)
#define AFX_ISOTREE_H__78BE85B9_87B6_11D4_8056_00105ABB05E6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ExitCode.h"

class CIsoTree
{
public:
	CIsoTree() {;};
	virtual ~CIsoTree() {;};
	void DebugPrintIsoTrack(const NERO_ISO_ITEM * pItem, int iLevel);
	void DeleteIsoItemTree(NERO_ISO_ITEM * pItem);
	void FreeOurOwnResources(NERO_ISO_ITEM * pItem);
	CExitCode CreateIsoTree(bool bRecursive, LPCSTR psFilename, NERO_ISO_ITEM ** ppItem, int iLevel = 0);
	void StoreFileName(char (& shortFilename) [252], char * psLongFilename, LPCSTR psFilename);
};


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ISOTREE_H__78BE85B9_87B6_11D4_8056_00105ABB05E6__INCLUDED_)
